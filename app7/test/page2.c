#include <stdio.h>
#include "lvgl.h"
#include "page_conf.h"

/**
 * @brief 页面2按钮点击事件回调函数
 * @param e 事件对象指针
 * @note 点击按钮后清空当前屏幕内容，并返回到页面1
 *       实现页面2到页面1的反向导航
 */
static void btn_click_event_cb_func(lv_event_t * e){
    printf("page2 btn click\n");  // 打印调试信息
    
    // 1. 获取当前活动的屏幕对象
    // lv_scr_act() 返回当前正在显示的屏幕
    lv_obj_t * act_scr = lv_scr_act();
    
    // 2. 清除屏幕上的所有子对象
    // lv_obj_clean() 删除指定对象的所有子对象，但保留屏幕对象本身
    // 这样可以在同一屏幕上显示新内容，避免频繁创建销毁屏幕
    lv_obj_clean(act_scr);
    
    // 3. 初始化并显示页面1的内容
    // 在已清空的屏幕上重新创建页面1的UI元素，实现返回导航
    init_page1();
}

/**
 * @brief 初始化页面2的UI界面
 * @note 创建页面2的所有UI元素：标签和返回按钮
 *       页面布局：标签居中显示，按钮在标签下方20像素处
 *       与页面1结构相同，仅文本内容不同
 */
void init_page2()
{
    // 1. 创建标签显示页面标题
    lv_obj_t * label = lv_label_create(lv_scr_act());  // 在当前活动屏幕上创建标签
    lv_obj_center(label);                               // 将标签在屏幕中心显示
    lv_label_set_text(label, "page 2");                 // 设置标签文本为"page 2"

    // 2. 创建按钮用于返回页面1
    lv_obj_t * btn = lv_btn_create(lv_scr_act());      // 在当前活动屏幕上创建按钮
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);          // 先将按钮居中对齐（后续会重新定位）
    
    // 3. 为按钮添加点击事件监听
    // 当按钮被点击时，调用 btn_click_event_cb_func 回调函数返回页面1
    lv_obj_add_event_cb(btn, btn_click_event_cb_func, LV_EVENT_CLICKED, NULL);

    // 4. 在按钮上创建标签显示按钮文字
    lv_obj_t * btn_label = lv_label_create(btn);       // 创建标签作为按钮的子对象
    lv_label_set_text(btn_label, "Open page1");        // 设置按钮文字为"Open page1"
    lv_obj_center(btn_label);                          // 将按钮文字在按钮内居中

    // 5. 重新定位按钮位置
    // 将按钮放置在标签下方20像素处，水平居中对齐
    // 参数说明：
    //   - btn: 要对齐的按钮对象
    //   - label: 参考对象（标签）
    //   - LV_ALIGN_OUT_BOTTOM_MID: 在参考对象外部底部中间位置
    //   - 0: x方向偏移0像素（保持水平居中）
    //   - 20: y方向偏移20像素（向下偏移）
    lv_obj_align_to(btn, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

}