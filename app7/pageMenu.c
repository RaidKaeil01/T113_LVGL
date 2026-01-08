#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "page_conf.h"

/* ========== 全局变量 ========== */
// 通用样式
static lv_style_t com_style;

// 前向声明
static void swipe_event_cb(lv_event_t * e);

/**
 * @brief 清理菜单页面资源
 */
void cleanup_pageMenu(void)
{
    printf("Cleaning up pageMenu resources...\n");
    
    // 1. 清理通用样式
    if(com_style.prop_cnt > 0) {
        lv_style_reset(&com_style);
        printf("Style reset\n");
    }
    
    // 2. 移除屏幕上的所有事件回调（特别是手势事件）
    lv_obj_remove_event_cb(lv_scr_act(), swipe_event_cb);
    printf("Gesture event removed\n");
    
    printf("pageMenu cleanup completed\n");
}

/**
 * @brief 滑动事件回调函数 - 切换到启动页面
 */
static void swipe_event_cb(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    
    if(dir == LV_DIR_RIGHT) {
        printf("Swipe RIGHT detected, switching to Start page\n");
        
        // 清理当前页面资源
        cleanup_pageMenu();
        
        // 清空屏幕
        lv_obj_clean(lv_scr_act());
        
        // 切换到启动页面
        init_pageStart();
    }
}

/**
 * @brief 菜单点击事件回调函数
 * @param e 事件对象
 */
static void menu_click_event_cb(lv_event_t * e)
{
    // 获取传入的数据，转换为字符串
    char* menu_name = (char *)lv_event_get_user_data(e);
    printf("--->select menu = %s\n", menu_name);
    
    // 判断点击的是哪个菜单项
    if(strcmp(menu_name, "Setting") == 0) {
        printf("Switching to Setting page\n");
        
        // 清理当前页面资源
        cleanup_pageMenu();
        
        // 清空屏幕
        lv_obj_clean(lv_scr_act());
        
        // 切换到设置页面
        init_page_setting();
    }
    else if(strcmp(menu_name, "Clock") == 0) {
        printf("Switching to Clock page\n");
        
        // 清理当前页面资源
        cleanup_pageMenu();
        
        // 清空屏幕
        lv_obj_clean(lv_scr_act());
        
        // 切换到闹钟页面
        init_pageClock();
    }
    else if(strcmp(menu_name, "WiFi") == 0) {
        printf("Switching to WiFi page\n");
        
        // 清理当前页面资源
        cleanup_pageMenu();
        
        // 清空屏幕
        lv_obj_clean(lv_scr_act());
        
        // 切换到WiFi页面
        init_pageWifi();
    }
    else if(strcmp(menu_name, "Music") == 0) {
        printf("Switching to Music page\n");
        
        // 清理当前页面资源
        cleanup_pageMenu();
        
        // 清空屏幕
        lv_obj_clean(lv_scr_act());
        
        // 切换到音乐页面
        init_pageMusic();
    }
}

/**
 * @brief 初始化单个菜单项
 * @param parent 父对象
 * @param imgurl 图片路径
 * @param str 菜单名称
 * @return 菜单项对象
 */
static lv_obj_t * init_item(lv_obj_t *parent, char *imgurl, char *str)
{
    // 初始化对象，作为容器，装载菜单内容
    lv_obj_t * cont = lv_obj_create(parent);
    // 设置对象大小为自适应内容大小
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    // 添加样式
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    // 添加可点击标志
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    // 清除可滚动标志
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    // 设置顶部填充距离
    lv_obj_set_style_pad_top(cont,0, LV_PART_MAIN);

    // 初始化图像控件
    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img, imgurl);
    // 设置图标大小为110x110
    lv_img_set_zoom(img, 256);
    lv_obj_set_size(img, 110, 110);
    
    // 初始化标签控件
    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(label, str);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    
    // 添加点击事件监听，同时传递标签内容
    lv_obj_add_event_cb(cont, menu_click_event_cb, LV_EVENT_CLICKED, lv_label_get_text(label));
    
    return cont;
}

/**
 * @brief 初始化菜单列表
 * @param parent 父对象
 * @return 菜单列表对象
 */
static lv_obj_t * init_menu_list(lv_obj_t *parent)
{
    // 初始化菜单父对象，作为容器，装载菜单
    lv_obj_t * cont = lv_obj_create(parent);
    // 设置大小
    lv_obj_set_size(cont,1424, 280);
    // 设置位置：左上角对齐
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 0);
    // 添加样式
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    // 设置线性布局（横向排列）
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    // 设置flex对齐方式
    lv_obj_set_style_pad_row(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(cont, 50, LV_PART_MAIN);
    // 设置背景颜色透明
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_MAIN);
    // 设置边框为0
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);

    // 初始化菜单 - 蓝牙设置
    init_item(cont, "A:res/image/menu/menu_bluetooth.png", "Bluetooth");
    
    // 初始化菜单 - WiFi设置
    init_item(cont, "A:res/image/menu/menu_wifi1.png", "WiFi");
    
    // 初始化菜单 - 设置
    init_item(cont, "A:res/image/menu/menu_setting.png", "Setting");
    
    // 初始化菜单 - 闹钟
    init_item(cont, "A:res/image/menu/menu_clock.png", "Clock");
    
    // 初始化菜单 - 日历
    init_item(cont, "A:res/image/menu/menu_calendar.png", "Calendar");
    
    // 初始化菜单 - 收音机
    init_item(cont, "A:res/image/menu/menu_radio.png", "Radio");
    
    // 初始化菜单 - 音乐播放器
    init_item(cont, "A:res/image/menu/menu_music.png", "Music");
    
 
    // 初始化菜单 - 相册
    init_item(cont, "A:res/image/menu/menu_photo.png", "Photo");
    
    // 初始化菜单 - 信息
    init_item(cont, "A:res/image/menu/menu_information.png", "Information");

       // 初始化菜单 - 游戏
    init_item(cont, "A:res/image/menu/menu_game.png", "Game");
    
    
    // TODO: 后续添加更多菜单项
    // init_item(cont, "A:res/image/menu/xxx.png", "菜单名称");
    
    return cont;
}

/**
 * @brief 初始化通用样式
 */
static void init_common_style(void)
{
    lv_style_init(&com_style);
    // 设置背景透明
    lv_style_set_bg_opa(&com_style, LV_OPA_TRANSP);
    // 设置边框宽度为0
    lv_style_set_border_width(&com_style, 0);
    // 设置圆角半径
    lv_style_set_radius(&com_style, 10);
    // 设置内边距
    lv_style_set_pad_all(&com_style, 10);
}

/**
 * @brief 初始化菜单页面
 */
void init_pageMenu(void)
{
    // 设置屏幕背景颜色为白色
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    // 初始化通用样式
    init_common_style();
    
    // 初始化菜单列表
    init_menu_list(lv_scr_act());
    
    // 添加滑动手势检测
    lv_obj_add_event_cb(lv_scr_act(), swipe_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_GESTURE_BUBBLE);
}
