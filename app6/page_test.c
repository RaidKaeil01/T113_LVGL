#include "lvgl.h"
#include "font_conf.h"
#include <stdio.h>
#include "image_conf.h"

#include "lv_load.h"  // 包含自定义load控件头文件
//extern const lv_font_t test_font;
extern const lv_img_dsc_t favicon;//图片资源声明




/**
 * @brief LVGL示例主函数 - 演示自定义load控件的使用
 */
void lv_example_hello_world(void) {
    // 1. 创建自定义load控件对象
    // 父对象为当前活动屏幕lv_scr_act()
    lv_obj_t * load = lv_load_create(lv_scr_act());
    
    // 2. 设置控件对齐方式
    // LV_ALIGN_TOP_MID: 顶部居中对齐
    lv_obj_set_align(load, LV_ALIGN_TOP_MID);
    
    // 3. 设置控件大小
    // 宽度92像素，高度40像素
    lv_obj_set_size(load,100,50);

} 


/*
 * ============================================================================
 * LVGL 常用函数参考手册
 * ============================================================================
 */

/*
 * 一、基础对象创建
 * ============================================================================
 */

// 1. 创建基础对象（容器）
// lv_obj_t *obj = lv_obj_create(parent);
// 参数：parent - 父对象，通常是 lv_scr_act() 获取当前屏幕

// 2. 创建标签（文本显示）
// lv_obj_t *label = lv_label_create(parent);
// lv_label_set_text(label, "文本内容");              // 设置文本
// lv_label_set_text_fmt(label, "数字: %d", 100);    // 格式化文本

// 3. 创建按钮
// lv_obj_t *btn = lv_btn_create(parent);
// lv_obj_t *label = lv_label_create(btn);           // 在按钮上创建标签
// lv_label_set_text(label, "按钮");

// 4. 创建图片
// lv_obj_t *img = lv_img_create(parent);
// lv_img_set_src(img, "路径/图片.png");              // 设置图片源
// lv_img_set_src(img, &img_variable);               // 使用变量

// 5. 创建滑动条
// lv_obj_t *slider = lv_slider_create(parent);
// lv_slider_set_value(slider, 50, LV_ANIM_OFF);    // 设置值 (0-100)
// lv_slider_set_range(slider, 0, 200);             // 设置范围

// 6. 创建开关
// lv_obj_t *sw = lv_switch_create(parent);
// lv_obj_add_state(sw, LV_STATE_CHECKED);          // 设置为开启状态

// 7. 创建复选框
// lv_obj_t *cb = lv_checkbox_create(parent);
// lv_checkbox_set_text(cb, "同意协议");

// 8. 创建下拉列表
// lv_obj_t *dd = lv_dropdown_create(parent);
// lv_dropdown_set_options(dd, "选项1\n选项2\n选项3");

// 9. 创建文本输入框
// lv_obj_t *ta = lv_textarea_create(parent);
// lv_textarea_set_text(ta, "初始文本");
// lv_textarea_set_placeholder_text(ta, "请输入...");

// 10. 创建弧形进度条
// lv_obj_t *arc = lv_arc_create(parent);
// lv_arc_set_value(arc, 75);                       // 设置值 (0-100)
// lv_arc_set_range(arc, 0, 360);                   // 设置范围

/*
 * 二、位置和大小设置
 * ============================================================================
 */

// 1. 设置位置（绝对坐标）
// lv_obj_set_pos(obj, x, y);                       // x, y 是像素坐标
// lv_obj_set_x(obj, x);                            // 只设置 x 坐标
// lv_obj_set_y(obj, y);                            // 只设置 y 坐标

// 2. 设置大小
// lv_obj_set_size(obj, width, height);             // 设置宽度和高度
// lv_obj_set_width(obj, width);                    // 只设置宽度
// lv_obj_set_height(obj, height);                  // 只设置高度
// lv_obj_set_content_width(obj, width);            // 自动适应内容宽度
// lv_obj_set_content_height(obj, height);          // 自动适应内容高度

// 3. 对齐方式
// lv_obj_align(obj, LV_ALIGN_CENTER, x_ofs, y_ofs);           // 相对父对象对齐
// lv_obj_align_to(obj, target, align, x_ofs, y_ofs);         // 相对其他对象对齐

// 常用对齐参数：
// LV_ALIGN_CENTER          - 居中
// LV_ALIGN_TOP_LEFT        - 左上角
// LV_ALIGN_TOP_MID         - 顶部居中
// LV_ALIGN_TOP_RIGHT       - 右上角
// LV_ALIGN_BOTTOM_LEFT     - 左下角
// LV_ALIGN_BOTTOM_MID      - 底部居中
// LV_ALIGN_BOTTOM_RIGHT    - 右下角
// LV_ALIGN_LEFT_MID        - 左侧居中
// LV_ALIGN_RIGHT_MID       - 右侧居中
// LV_ALIGN_OUT_TOP_LEFT    - 外部左上
// LV_ALIGN_OUT_BOTTOM_MID  - 外部底部居中

// 示例：
// lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);               // 屏幕居中
// lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);           // 左上角，偏移(10,10)
// lv_obj_align_to(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // btn2在btn1下方10像素

/*
 * 三、样式设置
 * ============================================================================
 */

// 1. 背景颜色
// lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), 0);     // 红色
// lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);                    // 透明度50%

// 2. 边框
// lv_obj_set_style_border_width(obj, 2, 0);                      // 边框宽度
// lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), 0);// 边框颜色

// 3. 圆角
// lv_obj_set_style_radius(obj, 10, 0);                           // 圆角半径

// 4. 内边距
// lv_obj_set_style_pad_all(obj, 10, 0);                          // 所有方向
// lv_obj_set_style_pad_left(obj, 5, 0);                          // 左侧
// lv_obj_set_style_pad_top(obj, 5, 0);                           // 顶部

// 5. 文本样式
// lv_obj_set_style_text_color(label, lv_color_hex(0x0000FF), 0);// 文本颜色
// lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0); // 字体

// 6. 阴影
// lv_obj_set_style_shadow_width(obj, 10, 0);                     // 阴影宽度
// lv_obj_set_style_shadow_color(obj, lv_color_hex(0x000000), 0);// 阴影颜色

/*
 * 四、事件处理
 * ============================================================================
 */

// 1. 添加事件回调
// lv_obj_add_event_cb(obj, event_handler, LV_EVENT_CLICKED, NULL);

// 事件回调函数示例：
// void event_handler(lv_event_t *e) {
//     lv_event_code_t code = lv_event_get_code(e);           // 获取事件类型
//     lv_obj_t *obj = lv_event_get_target(e);               // 获取触发对象
//     
//     if(code == LV_EVENT_CLICKED) {
//         printf("对象被点击\n");
//     }
// }

// 常用事件类型：
// LV_EVENT_PRESSED         - 按下
// LV_EVENT_RELEASED        - 释放
// LV_EVENT_CLICKED         - 点击
// LV_EVENT_VALUE_CHANGED   - 值改变
// LV_EVENT_FOCUSED         - 获得焦点
// LV_EVENT_READY           - 准备完成

/*
 * 五、动画
 * ============================================================================
 */

// 1. 创建动画
// lv_anim_t a;
// lv_anim_init(&a);
// lv_anim_set_var(&a, obj);                              // 设置动画对象
// lv_anim_set_time(&a, 1000);                            // 持续时间(毫秒)
// lv_anim_set_values(&a, 0, 100);                        // 起始值和结束值
// lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x); // 执行回调
// lv_anim_start(&a);                                     // 启动动画

// 2. 动画路径
// lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);    // 缓动效果

/*
 * 六、屏幕管理
 * ============================================================================
 */

// 1. 获取当前屏幕
// lv_obj_t *scr = lv_scr_act();

// 2. 创建新屏幕
// lv_obj_t *new_scr = lv_obj_create(NULL);

// 3. 加载屏幕
// lv_scr_load(new_scr);                                  // 立即切换
// lv_scr_load_anim(new_scr, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false); // 动画切换

/*
 * 七、颜色定义
 * ============================================================================
 */

// lv_color_hex(0xFF0000)           // 红色
// lv_color_hex(0x00FF00)           // 绿色
// lv_color_hex(0x0000FF)           // 蓝色
// lv_color_hex(0xFFFFFF)           // 白色
// lv_color_hex(0x000000)           // 黑色
// lv_color_hex(0xFFFF00)           // 黄色
// lv_palette_main(LV_PALETTE_RED)  // 使用调色板

/*
 * 八、常用宏和常量
 * ============================================================================
 */

// 透明度：
// LV_OPA_TRANSP    - 完全透明
// LV_OPA_0         - 0%
// LV_OPA_50        - 50%
// LV_OPA_COVER     - 完全不透明
// LV_OPA_100       - 100%

// 动画选项：
// LV_ANIM_OFF      - 无动画
// LV_ANIM_ON       - 有动画

/*
 * 九、实用工具函数
 * ============================================================================
 */

// 1. 删除对象
// lv_obj_del(obj);                                       // 删除对象
// lv_obj_clean(parent);                                  // 删除所有子对象

// 2. 隐藏/显示
// lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);             // 隐藏
// lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);           // 显示

// 3. 启用/禁用点击
// lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);          // 可点击
// lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);        // 不可点击

// 4. 滚动控制
// lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE);         // 可滚动
// lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);       // 不可滚动

/*
 * 十、完整示例
 * ============================================================================
 */

/*
void create_ui_example(void) {
    // 1. 创建一个容器
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 300, 200);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xF0F0F0), 0);
    
    // 2. 创建标题标签
    lv_obj_t *title = lv_label_create(cont);
    lv_label_set_text(title, "示例界面");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    
    // 3. 创建按钮
    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "点击我");
    lv_obj_center(btn_label);
    
    // 4. 创建滑动条
    lv_obj_t *slider = lv_slider_create(cont);
    lv_obj_set_width(slider, 200);
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
}

void button_event_handler(lv_event_t *e) {
    printf("按钮被点击了！\n");
}
*/


