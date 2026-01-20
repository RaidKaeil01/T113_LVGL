#include "text_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>

/* ================= 全局变量 ================= */
static text_log_config_t g_text_config = {
    .max_file_size = 100 * 1024,  // 默认100KB
    .max_file_count = 5            // 默认保留5个文件
};

static FILE *g_log_file = NULL;
static char g_write_buffer[TEXT_LOG_BUFFER_SIZE];
static uint32_t g_buffer_used = 0;
static uint32_t g_current_file_size = 0;
static pthread_mutex_t g_text_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_text_log_initialized = false;

static time_t g_last_flush_time = 0;
#define FLUSH_INTERVAL 30  // 每30秒自动刷新一次

/* ================= 内部函数 ================= */

/**
 * @brief 确保日志目录存在
 */
static int ensure_log_directory(void)
{
    struct stat st = {0};
    
    // 检查 /usr/data 目录
    if (stat("/usr/data", &st) == -1) {
        if (mkdir("/usr/data", 0755) != 0) {
            printf("❌ Failed to create /usr/data\n");
            return -1;
        }
    }
    
    // 检查 /usr/data/log 目录
    if (stat("/usr/data/log", &st) == -1) {
        if (mkdir("/usr/data/log", 0755) != 0) {
            printf("❌ Failed to create /usr/data/log\n");
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief 获取当前日志文件大小
 */
static uint32_t get_file_size(const char *filepath)
{
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return (uint32_t)st.st_size;
    }
    return 0;
}

/**
 * @brief 日志轮转
 * system.log → system.log.1 → system.log.2 → ... → system.log.N (删除)
 */
int text_log_rotate(void)
{
    if (g_log_file != NULL) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
    
    // 删除最老的日志文件
    char oldest_log[256];
    snprintf(oldest_log, sizeof(oldest_log), "%s.%u", 
             TEXT_LOG_PATH, g_text_config.max_file_count);
    remove(oldest_log);  // 忽略返回值
    
    // 依次重命名旧日志文件
    for (int i = g_text_config.max_file_count - 1; i >= 1; i--) {
        char old_name[256], new_name[256];
        snprintf(old_name, sizeof(old_name), "%s.%d", TEXT_LOG_PATH, i);
        snprintf(new_name, sizeof(new_name), "%s.%d", TEXT_LOG_PATH, i + 1);
        rename(old_name, new_name);  // 忽略返回值
    }
    
    // 将当前日志文件重命名为 .1
    char new_name[256];
    snprintf(new_name, sizeof(new_name), "%s.1", TEXT_LOG_PATH);
    rename(TEXT_LOG_PATH, new_name);
    
    // 重新打开日志文件
    g_log_file = fopen(TEXT_LOG_PATH, "a");
    if (g_log_file == NULL) {
        printf("❌ Failed to reopen log file after rotation\n");
        return -1;
    }
    
    g_current_file_size = 0;
    printf("✅ Log file rotated successfully\n");
    
    return 0;
}

/* ================= 接口实现 ================= */

/**
 * @brief 初始化文本日志模块
 */
int text_log_init(const text_log_config_t *config)
{
    pthread_mutex_lock(&g_text_log_mutex);
    
    if (g_text_log_initialized) {
        printf("⚠️  Text log already initialized\n");
        pthread_mutex_unlock(&g_text_log_mutex);
        return 0;
    }
    
    // 使用用户配置或默认配置
    if (config != NULL) {
        memcpy(&g_text_config, config, sizeof(text_log_config_t));
    }
    
    // 确保日志目录存在
    if (ensure_log_directory() != 0) {
        pthread_mutex_unlock(&g_text_log_mutex);
        return -1;
    }
    
    // 打开日志文件（追加模式）
    g_log_file = fopen(TEXT_LOG_PATH, "a");
    if (g_log_file == NULL) {
        printf("❌ Failed to open log file: %s\n", TEXT_LOG_PATH);
        pthread_mutex_unlock(&g_text_log_mutex);
        return -1;
    }
    
    // 获取当前文件大小
    g_current_file_size = get_file_size(TEXT_LOG_PATH);
    
    // 初始化缓冲区
    memset(g_write_buffer, 0, sizeof(g_write_buffer));
    g_buffer_used = 0;
    g_last_flush_time = time(NULL);
    
    g_text_log_initialized = true;
    pthread_mutex_unlock(&g_text_log_mutex);
    
    printf("✅ Text log initialized: %s (size: %u bytes)\n", 
           TEXT_LOG_PATH, g_current_file_size);
    
    return 0;
}

/**
 * @brief 反初始化文本日志模块
 */
void text_log_deinit(void)
{
    pthread_mutex_lock(&g_text_log_mutex);
    
    if (!g_text_log_initialized) {
        pthread_mutex_unlock(&g_text_log_mutex);
        return;
    }
    
    // 刷新缓冲区
    if (g_buffer_used > 0 && g_log_file != NULL) {
        fwrite(g_write_buffer, 1, g_buffer_used, g_log_file);
        fflush(g_log_file);
    }
    
    // 关闭文件
    if (g_log_file != NULL) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
    
    g_text_log_initialized = false;
    pthread_mutex_unlock(&g_text_log_mutex);
    
    printf("✅ Text log deinitialized\n");
}

/**
 * @brief 写入一条日志
 */
int text_log_write(const char *log_entry)
{
    if (!g_text_log_initialized || log_entry == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_text_log_mutex);
    
    size_t entry_len = strlen(log_entry);
    
    // 添加换行符
    char entry_with_newline[512];
    snprintf(entry_with_newline, sizeof(entry_with_newline), 
             "%s\n", log_entry);
    size_t total_len = strlen(entry_with_newline);
    
    // 检查是否需要轮转
    if (g_current_file_size + total_len > g_text_config.max_file_size) {
        // 先刷新当前缓冲区
        if (g_buffer_used > 0) {
            fwrite(g_write_buffer, 1, g_buffer_used, g_log_file);
            g_buffer_used = 0;
        }
        
        // 执行日志轮转
        if (text_log_rotate() != 0) {
            pthread_mutex_unlock(&g_text_log_mutex);
            return -1;
        }
    }
    
    // 写入缓冲区
    if (g_buffer_used + total_len < TEXT_LOG_BUFFER_SIZE) {
        // 缓冲区有空间，先缓存
        memcpy(g_write_buffer + g_buffer_used, entry_with_newline, total_len);
        g_buffer_used += total_len;
        g_current_file_size += total_len;
    } else {
        // 缓冲区满了，先刷新
        if (g_buffer_used > 0) {
            fwrite(g_write_buffer, 1, g_buffer_used, g_log_file);
            g_buffer_used = 0;
        }
        
        // 直接写入文件
        fwrite(entry_with_newline, 1, total_len, g_log_file);
        g_current_file_size += total_len;
    }
    
    // 定时自动刷新（每30秒）
    time_t now = time(NULL);
    if (now - g_last_flush_time >= FLUSH_INTERVAL) {
        text_log_flush();
        g_last_flush_time = now;
    }
    
    pthread_mutex_unlock(&g_text_log_mutex);
    return 0;
}

/**
 * @brief 刷新缓冲区到磁盘
 */
int text_log_flush(void)
{
    if (!g_text_log_initialized || g_log_file == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_text_log_mutex);
    
    // 写入缓冲区内容
    if (g_buffer_used > 0) {
        fwrite(g_write_buffer, 1, g_buffer_used, g_log_file);
        g_buffer_used = 0;
    }
    
    // 刷新到磁盘
    fflush(g_log_file);
    fsync(fileno(g_log_file));
    
    pthread_mutex_unlock(&g_text_log_mutex);
    return 0;
}

/**
 * @brief 读取当前日志文件内容
 */
int text_log_read(char *buffer, uint32_t size)
{
    if (!g_text_log_initialized || buffer == NULL || size == 0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_text_log_mutex);
    
    // 先刷新缓冲区
    text_log_flush();
    
    // 打开文件读取
    FILE *fp = fopen(TEXT_LOG_PATH, "r");
    if (fp == NULL) {
        pthread_mutex_unlock(&g_text_log_mutex);
        return -1;
    }
    
    // 读取文件内容
    size_t read_size = fread(buffer, 1, size - 1, fp);
    buffer[read_size] = '\0';
    
    fclose(fp);
    
    pthread_mutex_unlock(&g_text_log_mutex);
    return (int)read_size;
}
