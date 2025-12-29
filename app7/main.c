#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lvgl.h"
//#include "page_test.h"
#include "font_conf.h"
#include "page_conf.h"
#include "lv_demos.h"
#include "net/http_manager.h"

extern void lv_port_disp_init(bool is_disp_orientation);
extern void lv_port_indev_init(void);

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
    
   // http_request_create();//HTTP模块初始化
   // http_set_weather_callback(my_weather_callback);  // 先注册回调
    //http_get_weather_async("SPhQ7CZNrk6Rzn8_H", "上海");//异步获取天气数据

   // init_pageMenu();
   init_pageStart();
   //init_pageClock();
   //init_page_setting();
   
    while (1) {
        lv_task_handler();
        //延时，保证cpu占有率不会过高
        usleep(1000);
    }
    return 0;
}