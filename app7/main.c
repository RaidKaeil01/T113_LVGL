#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lvgl.h"
//#include "page_test.h"
#include "font_conf.h"
#include "page_conf.h"
#include "lv_demos.h"
#include "net/http_manager.h"
#include "wifi/wpa_manager.h"
#include "ui_msg.h"  // 引入UI消息队列

extern void lv_port_disp_init(bool is_disp_orientation);
extern void lv_port_indev_init(void);

// pageStart的天气数据回调函数
extern void pageStart_weather_callback(weather_data_t *data);

// // 天气数据回调函数 - 打印到终端用于测试
// void my_weather_callback(char* weather_str) {
//     printf("\n========== 天气数据回调 ==========\n");
//     printf("接收到天气信息: %s\n", weather_str);
//     printf("=================================\n\n");
// }

int main() {
    //LVGL框架初始化
    lv_init();
    //LVGL显示屏幕初始化
   
    lv_port_disp_init(true);
    //LVGL输入设备初始化
    lv_port_indev_init();
    //初始化字体库（注册外部.OTF字体文件路径）
    FONT_INIT();

    //lv_example_hello_world();//执行功能函数

   // init_page1();//页面初始化
   // init_pageStart();//启动页面初始化
   // init_pageWifi();//WiFi页面初始化
    
    /* ========== 初始化UI消息队列（线程安全通信） ========== */
    printf("🔧 Initializing UI message queue...\n");
    if (ui_msg_init() != 0) {
        printf("❌ Failed to initialize UI message queue!\n");
        return -1;
    }
    
    /* ========== 先初始化界面（避免阻塞） ========== */
    init_pageStart();
    
    /* ========== 网络模块初始化（后台异步） ========== */
    // 临时禁用网络功能进行测试
    printf("⚠️  Network modules temporarily disabled for testing\n");
    
    // http_request_create();  // HTTP模块初始化
    // http_set_weather_callback(pageStart_weather_callback);  // 注册pageStart天气回调
    
    // // T113嵌入式环境：后台初始化WiFi（非阻塞）
    // printf("📱 Running on T113, initializing WiFi in background...\n");
    
    // // 后台启动WiFi管理器（不阻塞主线程）
    // if (wpa_manager_open() == 0) {
    //     printf("✅ WiFi manager thread started\n\n");
    //     
    //     // 尝试连接初始WiFi（非阻塞，仅发起连接命令）
    //     // 💡 修改默认WiFi请编辑: wifi/wpa_manager.h 中的 DEFAULT_WIFI_SSID 和 DEFAULT_WIFI_PSW
    //     wpa_ctrl_wifi_info_t default_wifi;
    //     memset(&default_wifi, 0, sizeof(default_wifi));
    //     strncpy(default_wifi.ssid, DEFAULT_WIFI_SSID, sizeof(default_wifi.ssid) - 1);
    //     strncpy(default_wifi.psw, DEFAULT_WIFI_PSW, sizeof(default_wifi.psw) - 1);
    //     
    //     // 只发起连接命令，不等待结果（立即返回）
    //     wpa_manager_auto_connect_default_wifi(&default_wifi, 0);
    //     
    //     printf("💡 Tip: Connection result will be shown in console\n");
    //     printf("   - Weather will auto-update after WiFi connected\n");
    //     printf("   - Or use Menu → WiFi Settings to connect manually\n\n");
    // } else {
    //     printf("❌ Failed to start WiFi manager\n\n");
    // }
    
    // // 立即发送天气请求（异步队列，不阻塞）
    // // WiFi连接成功后会自动执行
    // http_get_weather_async("SPhQ7CZNrk6Rzn8_H", "成都");
   // init_pageMenu();
   //init_pageClock();
   //init_page_setting();
   
    /* ========== 主循环（UI线程） ========== */
    printf("🚀 Entering main loop (UI thread)...\n\n");
    ui_msg_t msg;
    
    while (1) {
        // 1. 处理LVGL任务
        lv_task_handler();
        
        // 2. 处理UI消息队列（非阻塞）
        //    ⚠️ 这是唯一操作LVGL的地方（主线程）
        while (ui_msg_recv(&msg) == 0) {
            ui_msg_handle(&msg);
        }
        
        // 3. 延时，保证cpu占有率不会过高
        usleep(1000);
    }
    return 0;
}