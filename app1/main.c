
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lvgl.h"

extern void lv_port_disp_init(bool is_disp_orientation);
extern void lv_port_indev_init(void);

void lv_example_hello_world(void) {
    // 获取活动屏幕对象
    lv_obj_t *scr = lv_scr_act();
    // 创建一个标签对象
    lv_obj_t *label = lv_label_create(scr);
    // 设置标签的文本内容
    lv_label_set_text(label, "Hello, World!");
    // 将标签居中对齐到屏幕
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

int main() {
    //LVGL框架初始化
    lv_init();
    //LVGL显示屏幕初始化
    lv_port_disp_init(true);
    //LVGL输入设备初始化
    lv_port_indev_init();

    lv_example_hello_world();

    while (1) {
        lv_task_handler();
        //延时，保证cpu占有率不会过高
        usleep(1000);
    }
    return 0;
}