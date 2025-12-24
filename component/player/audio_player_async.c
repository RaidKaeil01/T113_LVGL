#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "osal_thread.h"    // 操作系统抽象层线程接口
#include "osal_queue.h"     // 操作系统抽象层消息队列接口
#include "em_hal_audio.h"   // 音频硬件抽象层接口

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

// 全局句柄：消息队列和线程（静态变量限制作用域为当前文件）
static osal_queue_t audio_queue = NULL;    // 音频命令消息队列
static osal_thread_t audio_thread = NULL;  // 音频处理线程

// 音频处理线程函数
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
        if (osal_queue_recv(&audio_queue, &obj, 100) == OSAL_SUCCESS) {
            printf("Received cmd: %d\n", obj.id);
            // 根据命令ID执行对应操作
            if (obj.id == AUDIO_COMM_ID_START) {
                em_stop_play_audio();
                em_play_audio(obj.file_name);
            }
        }
        osal_thread_sleep(1);
    }
}

// 异步启动音频播放接口
void start_play_audio_async(char *url) {
    // 初始化音频消息（指定启动命令ID）
    audio_obj obj = {.id = AUDIO_COMM_ID_START};
    // 拷贝音频文件名
    strncpy(obj.file_name, url, sizeof(obj.file_name)-1); 
    // 发送消息到队列（超时时间1000ms）
    if (osal_queue_send(&audio_queue, &obj, sizeof(audio_obj), 1000) == OSAL_ERROR) {
        printf("Failed to send start cmd\n");
    }
}

// 初始化异步音频播放器
int init_async_audio_player(void) {
    // 创建音频命令消息队列
    // 参数：队列句柄、队列名称、消息大小、队列长度（最多缓存50条消息）
    if (osal_queue_create(&audio_queue, "audio_queue", sizeof(audio_obj), 50) != OSAL_SUCCESS) {
        printf("Init audio play failed\n");
        return -1;
    }
    // 创建音频处理线程
    // 参数：线程句柄、线程函数、线程参数（NULL表示无参数）
    if(osal_thread_create(&audio_thread, audio_thread_fun, NULL) != OSAL_SUCCESS){
        printf("Init audio play failed\n");
        // 线程创建失败时，清理已创建的队列
        osal_queue_delete(&audio_queue);
        audio_queue = NULL;
        return -1;
    }
    return 0;
}