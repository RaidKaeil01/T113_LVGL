#ifndef __SYSTEM_LOG_H__
#define __SYSTEM_LOG_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================= 日志级别定义 ================= */
typedef enum {
    LOG_DEBUG = 0,   // 调试信息 → 文本日志
    LOG_INFO,        // 普通信息 → 文本日志
    LOG_WARN,        // 警告信息 → 文本日志 + 内存缓冲
    LOG_ERROR,       // 错误信息 → 文本日志 + 内存缓冲 + SQLite
    LOG_FATAL,       // 致命错误 → 所有后端
    LOG_EVENT        // 关键事件 → SQLite + 文本日志
} log_level_t;

/* ================= 模块标识定义 ================= */
typedef enum {
    LOG_MODULE_SYSTEM = 0,   // 系统
    LOG_MODULE_WIFI,         // WiFi
    LOG_MODULE_UI,           // 界面
    LOG_MODULE_FILE,         // 文件系统
    LOG_MODULE_AUDIO,        // 音频
    LOG_MODULE_NOTEBOOK,     // 记事本
    LOG_MODULE_HTTP,         // 网络
    LOG_MODULE_SETTING,      // 设置
    LOG_MODULE_MAX
} log_module_t;

/* ================= 事件类型定义 ================= */
typedef enum {
    LOG_EVENT_SYSTEM_BOOT = 0,      // 系统启动
    LOG_EVENT_SYSTEM_SHUTDOWN,      // 系统关闭
    LOG_EVENT_WIFI_CONNECT,         // WiFi连接
    LOG_EVENT_WIFI_DISCONNECT,      // WiFi断开
    LOG_EVENT_FILE_SAVE,            // 文件保存
    LOG_EVENT_FILE_DELETE,          // 文件删除
    LOG_EVENT_CONFIG_CHANGE,        // 配置变更
    LOG_EVENT_USER_OPERATION,       // 用户操作
    LOG_EVENT_ERROR_OCCURRED,       // 错误发生
    LOG_EVENT_MAX
} log_event_type_t;

/* ================= 日志配置 ================= */
typedef struct {
    bool enable_text_log;      // 启用文本日志
    bool enable_ram_log;       // 启用内存日志
    bool enable_event_log;     // 启用事件日志(SQLite)
    log_level_t min_level;     // 最小日志级别
    uint32_t ram_buffer_size;  // 内存缓冲区大小
    uint32_t text_max_size;    // 文本日志最大大小
    uint32_t text_max_files;   // 文本日志保留文件数
} log_config_t;

/* ================= 核心接口 ================= */

/**
 * @brief 初始化日志系统
 * @param config 日志配置，NULL则使用默认配置
 * @return 0成功，-1失败
 */
int system_log_init(const log_config_t *config);

/**
 * @brief 反初始化日志系统
 */
void system_log_deinit(void);

/**
 * @brief 写入日志（核心接口）
 * @param level 日志级别
 * @param module 模块标识
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void system_log_write(log_level_t level, log_module_t module, 
                      const char *format, ...);

/**
 * @brief 写入事件日志（关键事件）
 * @param module 模块标识
 * @param event_type 事件类型
 * @param message 事件消息
 * @param extra_data 额外数据（JSON格式或NULL）
 */
void system_log_event(log_module_t module, log_event_type_t event_type,
                      const char *message, const char *extra_data);

/**
 * @brief 获取内存日志内容
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 实际复制的字节数
 */
int system_log_get_ram_buffer(char *buffer, uint32_t size);

/**
 * @brief 读取文本日志文件
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 实际读取的字节数
 */
int system_log_read_text(char *buffer, uint32_t size);

/**
 * @brief 导出内存日志到文件（异常时）
 * @param filepath 导出文件路径
 * @return 0成功，-1失败
 */
int system_log_dump_ram_to_file(const char *filepath);

/* ================= 便捷宏定义 ================= */
#define LOG_D(module, ...) system_log_write(LOG_DEBUG, module, __VA_ARGS__)
#define LOG_I(module, ...) system_log_write(LOG_INFO, module, __VA_ARGS__)
#define LOG_W(module, ...) system_log_write(LOG_WARN, module, __VA_ARGS__)
#define LOG_E(module, ...) system_log_write(LOG_ERROR, module, __VA_ARGS__)
#define LOG_F(module, ...) system_log_write(LOG_FATAL, module, __VA_ARGS__)

#define LOG_EVENT_WRITE(module, type, msg, data) \
    system_log_event(module, type, msg, data)

/* ================= 工具函数 ================= */
const char* log_level_to_string(log_level_t level);
const char* log_module_to_string(log_module_t module);
const char* log_event_type_to_string(log_event_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_LOG_H__ */
