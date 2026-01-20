#include "system_log.h"
#include "text_log.h"
#include "ram_log.h"
// 暂时禁用SQLite，等交叉编译环境配置好再启用
// #include "event_log.h"
#define ENABLE_EVENT_LOG 0  // 0=禁用，1=启用

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

/* ================= 全局配置 ================= */
static log_config_t g_log_config = {
    .enable_text_log = true,
    .enable_ram_log = true,
    .enable_event_log = false,  // SQLite默认关闭，需要手动启用
    .min_level = LOG_DEBUG,
    .ram_buffer_size = 512,
    .text_max_size = 100 * 1024,  // 100KB
    .text_max_files = 5
};

static bool g_log_initialized = false;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ================= 字符串转换表 ================= */
static const char* level_strings[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "EVENT"
};

static const char* module_strings[] = {
    "SYSTEM", "WIFI", "UI", "FILE", "AUDIO", 
    "NOTEBOOK", "HTTP", "SETTING"
};

static const char* event_type_strings[] = {
    "SYSTEM_BOOT", "SYSTEM_SHUTDOWN", "WIFI_CONNECT", 
    "WIFI_DISCONNECT", "FILE_SAVE", "FILE_DELETE",
    "CONFIG_CHANGE", "USER_OPERATION", "ERROR_OCCURRED"
};

/* ================= 工具函数实现 ================= */
const char* log_level_to_string(log_level_t level)
{
    if (level < 0 || level >= sizeof(level_strings)/sizeof(level_strings[0])) {
        return "UNKNOWN";
    }
    return level_strings[level];
}

const char* log_module_to_string(log_module_t module)
{
    if (module < 0 || module >= LOG_MODULE_MAX) {
        return "UNKNOWN";
    }
    return module_strings[module];
}

const char* log_event_type_to_string(log_event_type_t type)
{
    if (type < 0 || type >= LOG_EVENT_MAX) {
        return "UNKNOWN";
    }
    return event_type_strings[type];
}

/**
 * @brief 获取当前时间戳字符串
 */
static void get_timestamp_string(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* ================= 核心接口实现 ================= */

/**
 * @brief 初始化日志系统
 */
int system_log_init(const log_config_t *config)
{
    pthread_mutex_lock(&g_log_mutex);
    
    if (g_log_initialized) {
        printf("⚠️  Log system already initialized\n");
        pthread_mutex_unlock(&g_log_mutex);
        return 0;
    }
    
    // 使用用户配置或默认配置
    if (config != NULL) {
        memcpy(&g_log_config, config, sizeof(log_config_t));
    }
    
    int ret = 0;
    
    // 初始化文本日志后端
    if (g_log_config.enable_text_log) {
        text_log_config_t text_config = {
            .max_file_size = g_log_config.text_max_size,
            .max_file_count = g_log_config.text_max_files
        };
        
        if (text_log_init(&text_config) != 0) {
            printf("❌ Failed to initialize text log\n");
            ret = -1;
        } else {
            printf("✅ Text log initialized\n");
        }
    }
    
    // 初始化内存日志后端
    if (g_log_config.enable_ram_log) {
        if (ram_log_init() != 0) {
            printf("❌ Failed to initialize RAM log\n");
            ret = -1;
        } else {
            printf("✅ RAM log initialized\n");
        }
    }
    
    // 初始化事件日志后端（SQLite）
#if ENABLE_EVENT_LOG
    if (g_log_config.enable_event_log) {
        if (event_log_init() != 0) {
            printf("❌ Failed to initialize event log (SQLite)\n");
            ret = -1;
        } else {
            printf("✅ Event log (SQLite) initialized\n");
        }
    }
#else
    printf("⚠️  Event log (SQLite) disabled\n");
#endif
    
    g_log_initialized = true;
    pthread_mutex_unlock(&g_log_mutex);
    
    // 记录系统启动日志
    if (ret == 0) {
        system_log_event(LOG_MODULE_SYSTEM, LOG_EVENT_SYSTEM_BOOT,
                        "日志系统初始化完成", NULL);
        printf("🚀 System log initialized successfully\n");
    }
    
    return ret;
}

/**
 * @brief 反初始化日志系统
 */
void system_log_deinit(void)
{
    pthread_mutex_lock(&g_log_mutex);
    
    if (!g_log_initialized) {
        pthread_mutex_unlock(&g_log_mutex);
        return;
    }
    
    printf("Shutting down log system...\n");
    
    // 记录系统关闭日志
    system_log_event(LOG_MODULE_SYSTEM, LOG_EVENT_SYSTEM_SHUTDOWN,
                    "日志系统关闭", NULL);
    
    // 反初始化各后端
    if (g_log_config.enable_text_log) {
        text_log_deinit();
    }
    
    if (g_log_config.enable_ram_log) {
        ram_log_deinit();
    }
    
#if ENABLE_EVENT_LOG
    if (g_log_config.enable_event_log) {
        event_log_deinit();
    }
#endif
    
    g_log_initialized = false;
    pthread_mutex_unlock(&g_log_mutex);
    
    printf("✅ Log system shutdown completed\n");
}

/**
 * @brief 写入日志（核心分发函数）
 */
void system_log_write(log_level_t level, log_module_t module, 
                      const char *format, ...)
{
    if (!g_log_initialized) {
        return;  // 日志系统未初始化，静默返回
    }
    
    // 检查日志级别过滤
    if (level < g_log_config.min_level) {
        return;
    }
    
    // 格式化日志消息
    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // 生成完整日志条目
    char timestamp[32];
    get_timestamp_string(timestamp, sizeof(timestamp));
    
    char log_entry[512];
    snprintf(log_entry, sizeof(log_entry), "[%s] [%s] [%s] %s",
             timestamp,
             log_level_to_string(level),
             log_module_to_string(module),
             message);
    
    pthread_mutex_lock(&g_log_mutex);
    
    // 根据日志级别分发到不同后端
    switch (level) {
        case LOG_DEBUG:
        case LOG_INFO:
            // DEBUG/INFO → 仅文本日志
            if (g_log_config.enable_text_log) {
                text_log_write(log_entry);
            }
            break;
            
        case LOG_WARN:
            // WARN → 文本日志 + 内存缓冲
            if (g_log_config.enable_text_log) {
                text_log_write(log_entry);
            }
            if (g_log_config.enable_ram_log) {
                ram_log_write(log_entry);
            }
            break;
            
        case LOG_ERROR:
            // ERROR → 文本日志 + 内存缓冲 + SQLite
            if (g_log_config.enable_text_log) {
                text_log_write(log_entry);
            }
            if (g_log_config.enable_ram_log) {
                ram_log_write(log_entry);
            }
#if ENABLE_EVENT_LOG
            if (g_log_config.enable_event_log) {
                event_log_write(module, LOG_EVENT_ERROR_OCCURRED, 
                               message, NULL);
            }
#endif
            break;
            
        case LOG_FATAL:
            // FATAL → 所有后端
            if (g_log_config.enable_text_log) {
                text_log_write(log_entry);
                text_log_flush();  // 立即刷新
            }
            if (g_log_config.enable_ram_log) {
                ram_log_write(log_entry);
            }
#if ENABLE_EVENT_LOG
            if (g_log_config.enable_event_log) {
                event_log_write(module, LOG_EVENT_ERROR_OCCURRED,
                               message, NULL);
            }
#endif
            break;
            
        case LOG_EVENT:
            // 不在这里处理，使用system_log_event
            break;
    }
    
    pthread_mutex_unlock(&g_log_mutex);
}

/**
 * @brief 写入事件日志（关键事件）
 */
void system_log_event(log_module_t module, log_event_type_t event_type,
                      const char *message, const char *extra_data)
{
    if (!g_log_initialized) {
        return;
    }
    
    // 生成日志条目（同时写入文本日志）
    char timestamp[32];
    get_timestamp_string(timestamp, sizeof(timestamp));
    
    char log_entry[512];
    snprintf(log_entry, sizeof(log_entry), "[%s] [EVENT] [%s] [%s] %s",
             timestamp,
             log_module_to_string(module),
             log_event_type_to_string(event_type),
             message);
    
    pthread_mutex_lock(&g_log_mutex);
    
    // 写入文本日志
    if (g_log_config.enable_text_log) {
        text_log_write(log_entry);
    }
    
    // 写入SQLite数据库
#if ENABLE_EVENT_LOG
    if (g_log_config.enable_event_log) {
        event_log_write(module, event_type, message, extra_data);
    }
#endif
    
    pthread_mutex_unlock(&g_log_mutex);
}

/**
 * @brief 获取内存日志内容
 */
int system_log_get_ram_buffer(char *buffer, uint32_t size)
{
    if (!g_log_initialized || !g_log_config.enable_ram_log) {
        return -1;
    }
    
    // 获取最新的日志（0=全部）
    return ram_log_read_latest(buffer, size, 0);
}

/**
 * @brief 读取文本日志文件
 */
int system_log_read_text(char *buffer, uint32_t size)
{
    if (!g_log_initialized || !g_log_config.enable_text_log) {
        return -1;
    }
    
    return text_log_read(buffer, size);
}

/**
 * @brief 导出内存日志到文件（异常时）
 */
int system_log_dump_ram_to_file(const char *filepath)
{
    if (!g_log_initialized || !g_log_config.enable_ram_log) {
        return -1;
    }
    
    return ram_log_export(filepath);
}
