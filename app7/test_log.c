/**
 * @file log_test.c
 * @brief 日志系统功能测试程序
 */

#include "system_log.h"
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("========== 日志系统测试 ==========\n\n");
    
    // 1. 初始化日志系统
    printf("1. 初始化日志系统...\n");
    if (system_log_init(NULL) != 0) {
        printf("❌ 日志系统初始化失败\n");
        return -1;
    }
    printf("\n");
    
    // 2. 测试各级别日志
    printf("2. 测试各级别日志...\n");
    LOG_D(LOG_MODULE_SYSTEM, "这是一条DEBUG日志，包含调试信息：value=%d", 123);
    LOG_I(LOG_MODULE_WIFI, "WiFi连接成功：SSID=TestAP, IP=192.168.1.100");
    LOG_W(LOG_MODULE_FILE, "文件大小接近限制：current=%d KB, limit=%d KB", 95, 100);
    LOG_E(LOG_MODULE_AUDIO, "音频播放失败：file not found");
    LOG_F(LOG_MODULE_SYSTEM, "系统严重错误：内存不足！");
    printf("✅ 各级别日志写入完成\n\n");
    
    // 3. 测试模块日志
    printf("3. 测试不同模块日志...\n");
    LOG_I(LOG_MODULE_UI, "界面加载完成");
    LOG_I(LOG_MODULE_NOTEBOOK, "记事本保存成功：notebook.txt");
    LOG_I(LOG_MODULE_HTTP, "HTTP请求成功：status=200");
    LOG_I(LOG_MODULE_SETTING, "设置已更新：brightness=80%%");
    printf("✅ 模块日志写入完成\n\n");
    
    // 4. 测试RAM日志统计
    printf("4. 测试RAM日志统计...\n");
    ram_log_stats_t stats;
    if (ram_log_get_stats(&stats) == 0) {
        printf("  总写入次数: %u\n", stats.total_writes);
        printf("  当前日志数: %u\n", stats.current_count);
        printf("  覆盖次数: %u\n", stats.overwrite_count);
        printf("  写入索引: %u\n", stats.write_index);
    }
    printf("\n");
    
    // 5. 测试日志刷新
    printf("5. 测试日志刷新...\n");
    text_log_flush();
    printf("✅ 文本日志已刷新到磁盘\n\n");
    
    // 6. 读取文本日志（前500字节）
    printf("6. 读取文本日志（最新内容）...\n");
    char log_buffer[500];
    int read_size = text_log_read(log_buffer, sizeof(log_buffer));
    if (read_size > 0) {
        printf("--- 文本日志内容 ---\n%s\n", log_buffer);
        printf("--- 共读取 %d 字节 ---\n", read_size);
    }
    printf("\n");
    
    // 7. 读取RAM日志（最新10条）
    printf("7. 读取RAM日志（最新10条）...\n");
    char ram_buffer[1024];
    int ram_count = ram_log_read_latest(ram_buffer, sizeof(ram_buffer), 10);
    if (ram_count > 0) {
        printf("--- RAM日志内容 (%d条) ---\n%s\n", ram_count, ram_buffer);
        printf("--- 共 %d 条记录 ---\n", ram_count);
    }
    printf("\n");
    
    // 8. 测试RAM日志导出
    printf("8. 测试RAM日志导出...\n");
    if (ram_log_export(NULL) == 0) {
        printf("✅ RAM日志已导出到: %s\n", RAM_LOG_EXPORT_PATH);
    }
    printf("\n");
    
    // 9. 反初始化日志系统
    printf("9. 关闭日志系统...\n");
    system_log_deinit();
    printf("✅ 日志系统已关闭\n\n");
    
    printf("========== 测试完成 ==========\n");
    printf("日志文件位置：\n");
    printf("  - 文本日志: /usr/data/log/system.log\n");
    printf("  - RAM导出: /usr/data/log/ram_dump.log\n");
    
    return 0;
}
