#include "ram_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>

/* ================= 全局变量 ================= */
static ram_log_entry_t g_ram_buffer[RAM_LOG_ENTRY_COUNT];
static uint32_t g_write_index = 0;       // 当前写入位置
static uint32_t g_oldest_index = 0;      // 最老日志位置
static uint32_t g_total_writes = 0;      // 总写入次数
static uint32_t g_overwrite_count = 0;   // 覆盖次数
static bool g_is_full = false;           // 缓冲区是否已满
static pthread_mutex_t g_ram_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_ram_log_initialized = false;

/* ================= 接口实现 ================= */

/**
 * @brief 初始化RAM日志模块
 */
int ram_log_init(void)
{
    pthread_mutex_lock(&g_ram_log_mutex);
    
    if (g_ram_log_initialized) {
        printf("⚠️  RAM log already initialized\n");
        pthread_mutex_unlock(&g_ram_log_mutex);
        return 0;
    }
    
    // 清空缓冲区
    memset(g_ram_buffer, 0, sizeof(g_ram_buffer));
    
    // 初始化索引
    g_write_index = 0;
    g_oldest_index = 0;
    g_total_writes = 0;
    g_overwrite_count = 0;
    g_is_full = false;
    
    g_ram_log_initialized = true;
    pthread_mutex_unlock(&g_ram_log_mutex);
    
    printf("✅ RAM log initialized (capacity: %d entries, %d bytes each)\n",
           RAM_LOG_ENTRY_COUNT, RAM_LOG_ENTRY_MAX_SIZE);
    
    return 0;
}

/**
 * @brief 反初始化RAM日志模块
 */
void ram_log_deinit(void)
{
    pthread_mutex_lock(&g_ram_log_mutex);
    
    if (!g_ram_log_initialized) {
        pthread_mutex_unlock(&g_ram_log_mutex);
        return;
    }
    
    g_ram_log_initialized = false;
    pthread_mutex_unlock(&g_ram_log_mutex);
    
    printf("✅ RAM log deinitialized\n");
}

/**
 * @brief 写入一条日志到RAM缓冲区
 */
int ram_log_write(const char *log_entry)
{
    if (!g_ram_log_initialized || log_entry == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ram_log_mutex);
    
    // 写入日志内容（截断过长的日志）
    ram_log_entry_t *entry = &g_ram_buffer[g_write_index];
    strncpy(entry->content, log_entry, RAM_LOG_ENTRY_MAX_SIZE - 1);
    entry->content[RAM_LOG_ENTRY_MAX_SIZE - 1] = '\0';
    entry->timestamp = (uint32_t)time(NULL);
    entry->valid = 1;
    
    // 更新索引
    g_write_index = (g_write_index + 1) % RAM_LOG_ENTRY_COUNT;
    g_total_writes++;
    
    // 检查是否覆盖
    if (g_is_full) {
        g_oldest_index = (g_oldest_index + 1) % RAM_LOG_ENTRY_COUNT;
        g_overwrite_count++;
    } else if (g_write_index == 0) {
        // 第一次写满
        g_is_full = true;
    }
    
    pthread_mutex_unlock(&g_ram_log_mutex);
    return 0;
}

/**
 * @brief 获取最新的N条日志
 */
int ram_log_read_latest(char *buffer, uint32_t size, uint32_t count)
{
    if (!g_ram_log_initialized || buffer == NULL || size == 0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ram_log_mutex);
    
    // 计算有效日志数量
    uint32_t valid_count = g_is_full ? RAM_LOG_ENTRY_COUNT : g_write_index;
    
    // 确定要读取的数量
    uint32_t read_count = (count == 0 || count > valid_count) ? valid_count : count;
    if (read_count == 0) {
        buffer[0] = '\0';
        pthread_mutex_unlock(&g_ram_log_mutex);
        return 0;
    }
    
    // 从最新开始往前读
    uint32_t buffer_used = 0;
    for (uint32_t i = 0; i < read_count; i++) {
        // 计算索引（从最新往前）
        uint32_t index;
        if (g_write_index >= i + 1) {
            index = g_write_index - i - 1;
        } else {
            index = RAM_LOG_ENTRY_COUNT + g_write_index - i - 1;
        }
        
        ram_log_entry_t *entry = &g_ram_buffer[index];
        if (!entry->valid) {
            continue;
        }
        
        // 格式化输出
        char line[RAM_LOG_ENTRY_MAX_SIZE + 32];
        int line_len = snprintf(line, sizeof(line), "[%u] %s\n", 
                                entry->timestamp, entry->content);
        
        // 检查缓冲区空间
        if (buffer_used + line_len >= size - 1) {
            break;  // 缓冲区满了
        }
        
        strcpy(buffer + buffer_used, line);
        buffer_used += line_len;
    }
    
    buffer[buffer_used] = '\0';
    
    pthread_mutex_unlock(&g_ram_log_mutex);
    return (int)read_count;
}

/**
 * @brief 导出所有日志到文件
 */
int ram_log_export(const char *filepath)
{
    if (!g_ram_log_initialized) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ram_log_mutex);
    
    // 使用默认路径
    const char *export_path = filepath ? filepath : RAM_LOG_EXPORT_PATH;
    
    // 打开文件
    FILE *fp = fopen(export_path, "w");
    if (fp == NULL) {
        printf("❌ Failed to open export file: %s\n", export_path);
        pthread_mutex_unlock(&g_ram_log_mutex);
        return -1;
    }
    
    // 写入头部信息
    fprintf(fp, "========== RAM Log Export ==========\n");
    fprintf(fp, "Total Writes: %u\n", g_total_writes);
    fprintf(fp, "Overwrites: %u\n", g_overwrite_count);
    fprintf(fp, "Capacity: %d entries\n", RAM_LOG_ENTRY_COUNT);
    fprintf(fp, "====================================\n\n");
    
    // 计算有效日志数量
    uint32_t valid_count = g_is_full ? RAM_LOG_ENTRY_COUNT : g_write_index;
    
    // 从最老的日志开始写入
    uint32_t start_index = g_is_full ? g_oldest_index : 0;
    for (uint32_t i = 0; i < valid_count; i++) {
        uint32_t index = (start_index + i) % RAM_LOG_ENTRY_COUNT;
        ram_log_entry_t *entry = &g_ram_buffer[index];
        
        if (!entry->valid) {
            continue;
        }
        
        fprintf(fp, "[%u] %s\n", entry->timestamp, entry->content);
    }
    
    fclose(fp);
    
    pthread_mutex_unlock(&g_ram_log_mutex);
    
    printf("✅ RAM log exported to: %s (%u entries)\n", export_path, valid_count);
    return 0;
}

/**
 * @brief 清空RAM缓冲区
 */
int ram_log_clear(void)
{
    if (!g_ram_log_initialized) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ram_log_mutex);
    
    // 清空缓冲区
    memset(g_ram_buffer, 0, sizeof(g_ram_buffer));
    
    // 重置索引
    g_write_index = 0;
    g_oldest_index = 0;
    g_total_writes = 0;
    g_overwrite_count = 0;
    g_is_full = false;
    
    pthread_mutex_unlock(&g_ram_log_mutex);
    
    printf("✅ RAM log cleared\n");
    return 0;
}

/**
 * @brief 获取RAM日志统计信息
 */
int ram_log_get_stats(ram_log_stats_t *stats)
{
    if (!g_ram_log_initialized || stats == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ram_log_mutex);
    
    stats->total_writes = g_total_writes;
    stats->overwrite_count = g_overwrite_count;
    stats->current_count = g_is_full ? RAM_LOG_ENTRY_COUNT : g_write_index;
    stats->write_index = g_write_index;
    stats->oldest_index = g_oldest_index;
    
    pthread_mutex_unlock(&g_ram_log_mutex);
    
    return 0;
}
