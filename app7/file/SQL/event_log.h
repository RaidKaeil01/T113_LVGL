/**
 * @file event_log.h
 * @brief SQLite事件日志后端 - 关键事件持久化
 * 
 * 特性：
 * - SQLite数据库存储，支持SQL查询
 * - 仅记录关键事件（ERROR/FATAL/EVENT）
 * - 低频写入，减少Flash磨损
 * - 支持按时间、模块、事件类型查询
 * - 自动清理旧日志（保留最近1000条）
 */

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdint.h>
#include <stdbool.h>
#include "system_log.h"  // 使用相同的枚举定义

/* ================= 配置参数 ================= */

#define EVENT_LOG_DB_PATH          "/usr/data/log/events.db"
#define EVENT_LOG_MAX_RECORDS      1000   // 最多保留1000条记录
#define EVENT_LOG_CLEANUP_THRESHOLD 1100  // 超过1100条时清理

/* ================= 数据结构 ================= */

/**
 * @brief 事件日志记录
 */
typedef struct {
    uint64_t id;               // 自增主键
    uint32_t timestamp;        // 时间戳
    log_level_t level;         // 日志级别
    log_module_t module;       // 模块
    log_event_type_t event;    // 事件类型
    char message[256];         // 事件描述
} event_log_record_t;

/**
 * @brief 事件日志查询条件
 */
typedef struct {
    uint32_t start_time;       // 起始时间（0=不限制）
    uint32_t end_time;         // 结束时间（0=不限制）
    log_level_t min_level;     // 最低级别（LOG_DEBUG=不限制）
    log_module_t module;       // 模块过滤（-1=不限制）
    log_event_type_t event;    // 事件类型（-1=不限制）
    uint32_t max_count;        // 最大返回数量（0=不限制）
} event_log_query_t;

/**
 * @brief 事件日志统计信息
 */
typedef struct {
    uint32_t total_count;      // 总记录数
    uint32_t error_count;      // 错误数量
    uint32_t fatal_count;      // 严重错误数量
    uint32_t event_count;      // 事件数量
    uint32_t oldest_timestamp; // 最老记录时间戳
    uint32_t newest_timestamp; // 最新记录时间戳
} event_log_stats_t;

/* ================= 接口函数 ================= */

/**
 * @brief 初始化事件日志模块
 * 
 * @return 0=成功，-1=失败
 */
int event_log_init(void);

/**
 * @brief 反初始化事件日志模块
 */
void event_log_deinit(void);

/**
 * @brief 写入一条事件日志
 * 
 * @param level 日志级别
 * @param module 模块
 * @param event 事件类型
 * @param message 事件描述
 * @return 0=成功，-1=失败
 */
int event_log_write(log_level_t level, log_module_t module, 
                   log_event_type_t event, const char *message);

/**
 * @brief 查询事件日志
 * 
 * @param query 查询条件
 * @param records 输出记录数组
 * @param max_records 数组最大容量
 * @return 实际返回的记录数，-1=失败
 */
int event_log_query(const event_log_query_t *query, 
                   event_log_record_t *records, uint32_t max_records);

/**
 * @brief 删除指定时间之前的事件日志
 * 
 * @param before_time 时间戳（删除此时间之前的记录）
 * @return 删除的记录数，-1=失败
 */
int event_log_delete_before(uint32_t before_time);

/**
 * @brief 清空所有事件日志
 * 
 * @return 0=成功，-1=失败
 */
int event_log_clear(void);

/**
 * @brief 获取事件日志统计信息
 * 
 * @param stats 输出统计信息
 * @return 0=成功，-1=失败
 */
int event_log_get_stats(event_log_stats_t *stats);

/**
 * @brief 获取最新的N条事件日志
 * 
 * @param records 输出记录数组
 * @param count 要获取的数量
 * @return 实际返回的记录数，-1=失败
 */
int event_log_get_latest(event_log_record_t *records, uint32_t count);

#endif // EVENT_LOG_H
