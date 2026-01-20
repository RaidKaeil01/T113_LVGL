#ifndef __TEXT_LOG_H__
#define __TEXT_LOG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================= 文本日志配置 ================= */
#define TEXT_LOG_PATH "/usr/data/log/system.log"
#define TEXT_LOG_BUFFER_SIZE 4096  // 4KB内存缓冲

typedef struct {
    uint32_t max_file_size;   // 单个日志文件最大大小（字节）
    uint32_t max_file_count;  // 最多保留的日志文件数
} text_log_config_t;

/* ================= 接口函数 ================= */

/**
 * @brief 初始化文本日志模块
 * @param config 配置参数
 * @return 0成功，-1失败
 */
int text_log_init(const text_log_config_t *config);

/**
 * @brief 反初始化文本日志模块
 */
void text_log_deinit(void);

/**
 * @brief 写入一条日志
 * @param log_entry 日志内容（已格式化）
 * @return 0成功，-1失败
 */
int text_log_write(const char *log_entry);

/**
 * @brief 刷新缓冲区到磁盘
 * @return 0成功，-1失败
 */
int text_log_flush(void);

/**
 * @brief 读取当前日志文件内容
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 实际读取字节数，-1失败
 */
int text_log_read(char *buffer, uint32_t size);

/**
 * @brief 日志轮转（内部函数，自动触发）
 * @return 0成功，-1失败
 */
int text_log_rotate(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEXT_LOG_H__ */
