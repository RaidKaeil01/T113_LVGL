#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lvgl.h"
#include "page_test.h"
#include "font_conf.h"
#include "page_conf.h"
#include "lv_demos.h"

extern void lv_port_disp_init(bool is_disp_orientation);
extern void lv_port_indev_init(void);



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
    
 lv_demo_music();
    while (1) {
        lv_task_handler();
        //延时，保证cpu占有率不会过高
        usleep(1000);
    }
    return 0;
}