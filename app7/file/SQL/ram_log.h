/**
 * @file ram_log.h
 * @brief RAM循环缓冲日志后端 - 实时日志缓冲
 * 
 * 特性：
 * - 纯内存操作，零磁盘IO
 * - 循环覆盖，保留最新日志
 * - 超高速写入，适合高频日志
 * - 异常时可导出到文件
 * - 支持实时查看
 */

#ifndef RAM_LOG_H
#define RAM_LOG_H

#include <stdint.h>
#include <stdbool.h>

/* ================= 配置参数 ================= */

#define RAM_LOG_ENTRY_MAX_SIZE   128   // 单条日志最大长度
#define RAM_LOG_ENTRY_COUNT      512   // 循环缓冲区容量（512条）
#define RAM_LOG_EXPORT_PATH      "/usr/data/log/ram_dump.log"  // 导出路径

/* ================= 数据结构 ================= */

/**
 * @brief RAM日志条目
 */
typedef struct {
    char content[RAM_LOG_ENTRY_MAX_SIZE];  // 日志内容
    uint32_t timestamp;                    // 时间戳
    uint8_t valid;                         // 是否有效（1=有效，0=无效）
} ram_log_entry_t;

/**
 * @brief RAM日志统计信息
 */
typedef struct {
    uint32_t total_writes;     // 总写入次数
    uint32_t overwrite_count;  // 覆盖次数
    uint32_t current_count;    // 当前有效日志数量
    uint32_t write_index;      // 当前写入索引
    uint32_t oldest_index;     // 最老日志索引
} ram_log_stats_t;

/* ================= 接口函数 ================= */

/**
 * @brief 初始化RAM日志模块
 * 
 * @return 0=成功，-1=失败
 */
int ram_log_init(void);

/**
 * @brief 反初始化RAM日志模块
 */
void ram_log_deinit(void);

/**
 * @brief 写入一条日志到RAM缓冲区
 * 
 * @param log_entry 日志内容（不超过128字节）
 * @return 0=成功，-1=失败
 */
int ram_log_write(const char *log_entry);

/**
 * @brief 获取最新的N条日志
 * 
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @param count 要获取的日志条数（0=全部）
 * @return 实际获取的日志条数，-1=失败
 */
int ram_log_read_latest(char *buffer, uint32_t size, uint32_t count);

/**
 * @brief 导出所有日志到文件
 * 
 * @param filepath 导出路径（NULL=使用默认路径）
 * @return 0=成功，-1=失败
 */
int ram_log_export(const char *filepath);

/**
 * @brief 清空RAM缓冲区
 * 
 * @return 0=成功，-1=失败
 */
int ram_log_clear(void);

/**
 * @brief 获取RAM日志统计信息
 * 
 * @param stats 输出统计信息
 * @return 0=成功，-1=失败
 */
int ram_log_get_stats(ram_log_stats_t *stats);

#endif // RAM_LOG_H
