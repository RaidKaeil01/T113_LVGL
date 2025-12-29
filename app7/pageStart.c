#include <stdio.h>
#include "lvgl.h"
#include "page_conf.h"
#include "image_conf.h"

/* ========== 全局变量 ========== */
// 时间显示标签
static lv_obj_t * label_time = NULL;
static lv_obj_t * label_date = NULL;
static lv_obj_t * label_week = NULL;

// 天气信息标签（天气状态和温度合并显示）
static lv_obj_t * label_weather = NULL;

// 状态提示标签
static lv_obj_t * label_status = NULL;

// 定时器
static lv_timer_t * time_timer = NULL;

/**
 * @brief 清理页面资源
 */
void cleanup_pageStart(void)
{
    // 删除定时器
    if(time_timer != NULL) {
        lv_timer_del(time_timer);
        time_timer = NULL;
    }
    
    // 清空全局变量
    label_time = NULL;
    label_date = NULL;
    label_week = NULL;
    label_weather = NULL;
    label_status = NULL;
    
    printf("pageStart cleanup completed\n");
}

/**
 * @brief 滑动事件回调函数 - 切换到菜单页面
 */
static void swipe_event_cb(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    
    if(dir == LV_DIR_LEFT) {
        printf("Swipe LEFT detected, switching to Menu page\n");
        
        // 清理当前页面资源
        cleanup_pageStart();
        
        // 清空屏幕
        lv_obj_clean(lv_scr_act());
        
        // 切换到菜单页面
        init_pageMenu();
    }
}

/**
 * @brief 定时器回调函数 - 更新时间显示
 * @param timer 定时器对象指针
 * @note 每秒更新一次时间显示（实际项目中应从RTC获取真实时间）
 */
static void time_update_timer_cb(lv_timer_t * timer)
{
    static int hour = 14;
    static int minute = 30;
    static int second = 0;
    
    // 模拟时间更新（实际项目中应从RTC获取）
    second++;
    if(second >= 60) {
        second = 0;
        minute++;
        if(minute >= 60) {
            minute = 0;
            hour++;
            if(hour >= 24) {
                hour = 0;
            }
        }
    }
    
    // 更新时间显示
    if(label_time != NULL) {
        lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", hour, minute, second);
    }
}

/**
 * @brief 初始化开始页面
 * @note 创建包含头像、时间、天气和状态信息的启动页面
 */
void init_pageStart(void)
{
    /* ========== 0. 设置屏幕背景颜色 ========== */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN);  // 黑色背景
    
    /* ========== 1. 头像区域（左上角）========== */
    lv_obj_t * avatar_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(avatar_container,280, 280);  // 头像容器大小
    lv_obj_align(avatar_container, LV_ALIGN_TOP_LEFT,0, 0);  // 左上角，偏移0
    
    // 设置头像容器样式
    lv_obj_set_style_radius(avatar_container, 60, LV_PART_MAIN);  // 圆形
    lv_obj_set_style_border_width(avatar_container, 0, LV_PART_MAIN);  // 无边框
    lv_obj_set_style_pad_all(avatar_container, 0, LV_PART_MAIN);  // 无内边距
    
    // 创建头像图片对象（用户需自行添加图片路径）
    lv_obj_t * img_avatar = lv_img_create(avatar_container);
    // TODO: 用户添加头像图片路径，例如：
    lv_img_set_src(img_avatar, "A:res/image/start/watch1.png");
    // 或使用图片数组：lv_img_set_src(img_avatar, &avatar_img);
    lv_obj_center(img_avatar);  // 头像居中
    
    // // 临时占位标签（实际使用时可删除）- 位于头像右侧的侧
    // lv_obj_t * label_avatar_placeholder = lv_label_create(lv_scr_act());
    // lv_label_set_text(label_avatar_placeholder, "HuneXi");
    // lv_obj_set_style_text_color(label_avatar_placeholder, lv_color_hex(0x808080), LV_PART_MAIN);
    // lv_obj_align_to(label_avatar_placeholder, avatar_container, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 在头像右侧，间隔10px
    
    
    /* ========== 2. 天气+时间信息区（整体容器 - 左右布局）========== */
lv_obj_t * info_container = lv_obj_create(lv_scr_act());
lv_obj_set_size(info_container, 300, 280);       // 整体容器尺寸 300x280
lv_obj_align(info_container, LV_ALIGN_TOP_LEFT, 280, 0);

// 整体容器样式（透明）
lv_obj_set_style_bg_opa(info_container, LV_OPA_TRANSP, LV_PART_MAIN);
lv_obj_set_style_border_width(info_container, 0, LV_PART_MAIN);
lv_obj_set_style_radius(info_container, 0, LV_PART_MAIN);
lv_obj_set_style_pad_all(info_container, 0, LV_PART_MAIN);  // 无内边距


/* ---------- 2.1 左侧：时间显示区 ---------- */
lv_obj_t * time_container = lv_obj_create(info_container);
lv_obj_set_size(time_container, 120, 280);       // 时间容器尺寸 120x280
lv_obj_align(time_container, LV_ALIGN_TOP_LEFT, 0, 0);  // 左上角对齐

// 时间容器样式
lv_obj_set_style_bg_opa(time_container, LV_OPA_TRANSP, LV_PART_MAIN);
lv_obj_set_style_border_width(time_container, 0, LV_PART_MAIN);
lv_obj_set_style_radius(time_container, 0, LV_PART_MAIN);
lv_obj_set_style_pad_all(time_container, 0, LV_PART_MAIN);  // 无内边距

// 统一字体（时间 & 日期）
const lv_font_t * info_font = &lv_font_montserrat_20;

// 时间显示
label_time = lv_label_create(time_container);
lv_label_set_text(label_time, "14:30:00");
lv_obj_set_style_text_font(label_time, info_font, LV_PART_MAIN);
lv_obj_set_style_text_color(label_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
lv_obj_align(label_time, LV_ALIGN_TOP_RIGHT, -5,100);

// 日期显示
label_date = lv_label_create(time_container);
lv_label_set_text(label_date, "2025-12-23");
lv_obj_set_style_text_font(label_date, info_font, LV_PART_MAIN);
lv_obj_set_style_text_color(label_date, lv_color_hex(0xBDC3C7), LV_PART_MAIN);
lv_obj_align(label_date, LV_ALIGN_TOP_RIGHT, -5, 130);

// 星期显示
label_week = lv_label_create(time_container);
lv_label_set_text(label_week, "Monday");
lv_obj_set_style_text_font(label_week, info_font, LV_PART_MAIN);
lv_obj_set_style_text_color(label_week, lv_color_hex(0xBDC3C7), LV_PART_MAIN);
lv_obj_align(label_week, LV_ALIGN_TOP_RIGHT,-5, 150);

/* ---------- 2.2 右侧：天气信息区 ---------- */
lv_obj_t * weather_container = lv_obj_create(info_container);
lv_obj_set_size(weather_container, 120, 280);    // 天气容器尺寸 120x280
lv_obj_align(weather_container, LV_ALIGN_TOP_LEFT,120, 0);  // 左上角对齐,偏移120px

// 天气容器样式
lv_obj_set_style_bg_opa(weather_container, LV_OPA_TRANSP, LV_PART_MAIN);
lv_obj_set_style_border_width(weather_container, 0, LV_PART_MAIN);
lv_obj_set_style_radius(weather_container, 0, LV_PART_MAIN);
lv_obj_set_style_pad_all(weather_container, 0, LV_PART_MAIN);  // 无内边距



// 图标内符号（临时）
lv_obj_t * icon_label = lv_label_create(weather_container);
lv_label_set_text(icon_label, LV_SYMBOL_CALL);
//lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_28, LV_PART_MAIN);
lv_obj_align(icon_label,LV_ALIGN_TOP_LEFT,15,100); 

//城市
lv_obj_t * label_city = lv_label_create(weather_container);
lv_label_set_text(label_city, "Beijing");
lv_obj_set_style_text_font(label_city, info_font, LV_PART_MAIN);
lv_obj_set_style_text_color(label_city, lv_color_hex(0xECF0F1), LV_PART_MAIN);
lv_obj_align(label_city, LV_ALIGN_TOP_LEFT,5,140);

// 天气状态 + 温度
label_weather = lv_label_create(weather_container);

// 测试用随机天气
const char * weather_states[] = {"雾", "晴", "多云", "雨", "雪", "阴"};
int random_index = lv_rand(0, 5);
int random_temp  = lv_rand(15, 30);

lv_label_set_text_fmt(label_weather, "%s %d°C",
                      weather_states[random_index], random_temp);

lv_obj_set_style_text_font(label_weather, info_font, LV_PART_MAIN);
lv_obj_set_style_text_color(label_weather, lv_color_hex(0xECF0F1), LV_PART_MAIN);
lv_obj_align(label_weather, LV_ALIGN_TOP_LEFT,5,160);

    
    /* ========== 4. 状态/提示区（右上角）========== */
    lv_obj_t * status_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_container, 250, 100);  // 状态区域大小
    lv_obj_align(status_container, LV_ALIGN_TOP_RIGHT, -20, 20);  // 右上角
    
    // 设置状态容器样式
    lv_obj_set_style_bg_color(status_container, lv_color_hex(0x34495E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_container, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_radius(status_container, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(status_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(status_container, 10, LV_PART_MAIN);
    
    // 状态图标（可选）
    lv_obj_t * status_icon = lv_label_create(status_container);
    lv_label_set_text(status_icon, LV_SYMBOL_WIFI " " LV_SYMBOL_BLUETOOTH " " LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(status_icon, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(status_icon, LV_ALIGN_TOP_LEFT, 5, 5);
    
    // // 状态提示文本
    // label_status = lv_label_create(status_container);
    // lv_label_set_text(label_status, "System Ready\nTouch to Continue");
    // lv_obj_set_style_text_color(label_status, lv_color_hex(0xECF0F1), LV_PART_MAIN);
    // lv_obj_set_style_text_align(label_status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    // lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    
    /* ========== 5. 创建定时器更新时间 ========== */
    time_timer = lv_timer_create(time_update_timer_cb, 1000, NULL);  // 每秒更新
    // 定时器将无限循环执行
    
    
    /* ========== 6. 添加滑动手势检测 ========== */
    lv_obj_add_event_cb(lv_scr_act(), swipe_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_GESTURE_BUBBLE);
}

