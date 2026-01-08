/**
 * @file audio_player_async.c
 * @brief 异步音频播放器实现
 * 
 * 使用生产者-消费者模式实现异步音频播放：
 * - 生产者：UI线程通过start_play_audio_async()发送播放命令
 * - 消费者：音频线程从队列取出命令并执行播放
 * - 缓冲区：消息队列（最多缓存50条命令）
 * 
 * 线程安全性：
 * - 使用OSAL消息队列保证线程间通信安全
 * - 静态全局变量限制作用域，避免外部访问冲突
 * 
 * @date 2026年1月6日
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "osal_thread.h"    // 操作系统抽象层线程接口
#include "osal_queue.h"     // 操作系统抽象层消息队列接口
// #include "em_hal_audio.h"   // 音频硬件抽象层接口 (暂时注释，等待实现)

/**
 * @brief 音频控制命令ID枚举
 * 定义了所有支持的音频操作命令类型
 */
typedef enum {
    AUDIO_COMM_ID_START,  // 启动音频播放命令
    AUDIO_COMM_ID_STOP,   // 停止音频播放命令
} AUDIO_COMM_ID;

/**
 * @brief 音频消息结构体
 * 用于在线程间传递音频控制命令及相关参数
 */
typedef struct {
    AUDIO_COMM_ID id;        // 命令ID，指定要执行的操作
    char file_name[256];     // 音频文件名/路径，仅启动命令时有效
} audio_obj;

/**
 * 全局句柄：消息队列和线程
 * 
 * 使用静态变量限制作用域为当前文件，防止外部直接访问。
 * 这些句柄在init_async_audio_player()中创建，
 * 在整个程序生命周期内保持有效。
 */
static osal_queue_t audio_queue = NULL;    // 音频命令消息队列句柄
static osal_thread_t audio_thread = NULL;  // 音频处理线程句柄

/**
 * @brief 音频处理线程函数
 * 
 * 这是音频播放器的核心工作线程，循环执行以下任务：
 * 1. 从消息队列接收播放命令（阻塞等待，超时100ms）
 * 2. 根据命令类型执行相应操作（启动/停止播放）
 * 3. 短暂休眠1ms，避免CPU占用过高
 * 
 * @param arg 线程参数（当前未使用，传入NULL）
 * @return NULL（线程不返回值）
 * 
 * @note 线程设置为可取消状态，支持通过pthread_cancel终止
 * @note 使用延迟取消模式，确保在安全点取消，避免资源泄露
 */
static void* audio_thread_fun(void *arg) {
    // 设置线程可被取消（支持pthread_cancel终止线程）
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // 设置延迟取消类型（取消请求在取消点执行，避免资源泄露）
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1) {
        // 初始化消息结构体（清零操作，避免残留数据影响判断）
        audio_obj obj;
        memset(&obj, 0, sizeof(audio_obj));
        
        // 从队列接收消息（超时时间100ms）
        // 如果队列为空，函数会等待最多100ms后返回失败
        if (osal_queue_recv(&audio_queue, &obj, 100) == OSAL_SUCCESS) {
            printf("Received cmd: %d\n", obj.id);
            
            // 根据命令ID执行对应操作
            if (obj.id == AUDIO_COMM_ID_START) {
                // 先停止当前播放（避免多个音频同时播放）
                em_stop_play_audio();
                // 启动新的音频播放
                em_play_audio(obj.file_name);
            }
            // TODO: 可扩展AUDIO_COMM_ID_STOP等其他命令处理
        }
        
        // 短暂休眠1ms，降低CPU占用率
        // 不休眠会导致空转时CPU占用过高
        osal_thread_sleep(1);
    }
}

/**
 * @brief 异步启动音频播放接口
 * 
 * 将播放命令封装成消息发送到队列，音频线程会异步处理。
 * 该函数立即返回，不等待音频播放完成。
 * 
 * @param url 音频文件路径
 * 
 * @note 如果队列已满（50条消息），发送会等待最多1000ms
 * @note 发送失败会打印错误信息，但不影响程序运行
 */
void start_play_audio_async(char *url) {
    // 初始化音频消息（指定启动命令ID）
    audio_obj obj = {.id = AUDIO_COMM_ID_START};
    
    // 安全地拷贝音频文件名（防止缓冲区溢出）
    // 保留最后一个字节用于\0终止符
    strncpy(obj.file_name, url, sizeof(obj.file_name)-1);
    obj.file_name[sizeof(obj.file_name)-1] = '\0';  // 确保字符串终止
    
    // 发送消息到队列（超时时间1000ms）
    // 如果队列满，会等待最多1秒
    if (osal_queue_send(&audio_queue, &obj, sizeof(audio_obj), 1000) == OSAL_ERROR) {
        printf("Failed to send start cmd\n");
    }
}

/**
 * @brief 初始化异步音频播放器
 * 
 * 执行初始化流程：
 * 1. 创建消息队列（容量50条消息）
 * 2. 创建音频处理线程
 * 3. 失败时自动清理已创建的资源
 * 
 * @return 0  初始化成功
 * @return -1 初始化失败
 * 
 * @note 该函数应在程序启动时调用一次
 * @note 不要重复调用，否则会创建多个线程和队列导致资源泄露
 */
int init_async_audio_player(void) {
    // 步骤1: 创建音频命令消息队列
    // 参数说明：
    //   - audio_queue: 输出参数，存储队列句柄
    //   - "audio_queue": 队列名称，用于生成唯一标识
    //   - sizeof(audio_obj): 单条消息大小
    //   - 50: 队列最大容量（可缓存50条待处理命令）
    if (osal_queue_create(&audio_queue, "audio_queue", sizeof(audio_obj), 50) != OSAL_SUCCESS) {
        printf("Init audio play failed: queue creation error\n");
        return -1;
    }
    
    // 步骤2: 创建音频处理线程
    // 参数说明：
    //   - audio_thread: 输出参数，存储线程句柄
    //   - audio_thread_fun: 线程执行函数
    //   - NULL: 线程参数（当前未使用）
    if(osal_thread_create(&audio_thread, audio_thread_fun, NULL) != OSAL_SUCCESS){
        printf("Init audio play failed: thread creation error\n");
        
        // 线程创建失败时，必须清理已创建的队列，避免资源泄露
        osal_queue_delete(&audio_queue);
        audio_queue = NULL;
        return -1;
    }
    
    printf("Audio player initialized successfully\n");
    return 0;
}