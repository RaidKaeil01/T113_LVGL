#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // atoi函数
#include "lvgl.h"
#include "page_conf.h"
#include "image_conf.h"
#include "font_conf.h"  // 引入中文字体配置
#include "net/http_manager.h"  // 引入天气数据结构体定义
#include "ui_msg.h"  // 引入UI消息队列

/* ========== 全局变量 ========== */
// 时间显示标签
static lv_obj_t * label_time = NULL;
static lv_obj_t * label_date = NULL;
static lv_obj_t * label_week = NULL;

// 天气信息标签（天气状态和温度合并显示）
static lv_obj_t * label_weather = NULL;
static lv_obj_t * label_city = NULL;     // 城市名称标签
static lv_obj_t * icon_label = NULL;     // 天气图标标签

// 状态提示标签
static lv_obj_t * label_status = NULL;

// 定时器
static lv_timer_t * time_timer = NULL;

/**
 * @brief 天气数据全局缓存（页面切换不丢失）
 * @note 保存API返回的所有数据，用于页面重新进入时恢复UI
 */
static struct {
    // 时间数据
    int hour;
    int minute;
    int second;
    
    // 日期数据
    char date[16];        // "2026-01-08"
    int weekday;          // 0=周日, 1=周一, ..., 6=周六
    
    // 天气数据
    char city[32];        // "成都"
    char weather[32];     // "阴"
    char temperature[16]; // "9"
    char weather_code[8]; // "9"
    
    bool initialized;     // 标记是否已从API初始化
} g_weather_state = {
    .hour = 0,
    .minute = 0,
    .second = 0,
    .date = "",
    .weekday = 0,
    .city = "",
    .weather = "",
    .temperature = "",
    .weather_code = "",
    .initialized = false
};

// 前向声明
static void swipe_event_cb(lv_event_t * e);

/**
 * @brief 封装的字体设置函数
 * @param obj LVGL对象
 * @param type 字体类型（FONT_TYPE_CN等）
 * @param weight 字体大小
 */
static void obj_font_set(lv_obj_t *obj, int type, uint16_t weight) {
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}

/**
 * @brief 根据天气代码映射天气图标图片路径
 * @param code 天气代码字符串（如"9"）
 * @return 天气图标PNG图片路径
 */
static const char* get_weather_icon(const char *code) {
    int code_int = atoi(code);
    
    // 根据心知天气代码映射PNG图标路径
    switch(code_int) {
        case 0:  // 晴（白天）
        case 1:  // 晴（夜间）
            return "A:res/image/start/weather_cloudy.png";  // 晴天（暂用weather_cloudy.png）
            
        case 4:  // 多云
        case 5:  // 多云转阴
            return "A:res/image/start/weather_cloudy.png";
            
        case 9:  // 阴天
            return "A:res/image/start/weather_cloudy.png";
            
        case 10: // 阵雨
        case 13: // 小雨
            return "A:res/image/start/weather_smallrain.png";
            
        case 14: // 中雨
        case 15: // 大雨
        case 16: // 暴雨
        case 17: // 大暴雨
        case 18: // 特大暴雨
            return "A:res/image/start/weather_heavyrain.png";
            
        case 19: // 冰雹
            return "A:res/image/start/weather_heavyrain.png";
            
        case 20: // 雨夹雪
            return "A:res/image/start/weather_snow.png";
            
        case 21: // 雷阵雨
        case 22: // 雷阵雨伴有冰雹
            return "A:res/image/start/weather_thunder.png";
            
        case 26: // 小雪
        case 27: // 中雪
        case 28: // 大雪
        case 29: // 暴雪
            return "A:res/image/start/weather_snow.png";
            
        case 30: // 雾
        case 31: // 霾
        case 32: // 沙尘
        case 33: // 扬沙
        case 34: // 强沙尘暴
        case 35: // 大雾
        case 49: // 浓雾
        case 53: // 霾
            return "A:res/image/start/weather_fog.png";
            
        default:
            return "A:res/image/start/weather_cloudy.png";  // 默认多云图标
    }
}

/**
 * @brief 天气数据回调函数（网络线程调用）
 * @param data 天气数据结构体指针
 * @note ⚠️ 此函数在网络线程中运行！
 *       不能直接操作LVGL，只能发送消息到UI队列
 */
void pageStart_weather_callback(weather_data_t *data) {
    printf("\n========== 天气数据回调（网络线程） ==========\n");
    printf("接收到天气信息:\n");
    printf("  城市: %s\n", data->city);
    printf("  天气: %s\n", data->weather);
    printf("  温度: %s°C\n", data->temperature);
    printf("  代码: %s\n", data->code);
    printf("  日期: %s\n", data->date);
    printf("  星期: %d (0=周日)\n", data->weekday);
    printf("  更新时间: %s\n", data->update_time);
    printf("=================================\n\n");
    
    /* ========== 构造UI消息并发送 ========== */
    ui_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = UI_MSG_WEATHER_OK;
    
    // 复制天气数据到消息
    strncpy(msg.data.weather.city, data->city, sizeof(msg.data.weather.city) - 1);
    strncpy(msg.data.weather.weather, data->weather, sizeof(msg.data.weather.weather) - 1);
    strncpy(msg.data.weather.temperature, data->temperature, sizeof(msg.data.weather.temperature) - 1);
    strncpy(msg.data.weather.code, data->code, sizeof(msg.data.weather.code) - 1);
    strncpy(msg.data.weather.update_time, data->update_time, sizeof(msg.data.weather.update_time) - 1);
    strncpy(msg.data.weather.date, data->date, sizeof(msg.data.weather.date) - 1);
    msg.data.weather.weekday = data->weekday;
    
    // 发送消息到UI队列（线程安全）
    if (ui_msg_send(&msg) == 0) {
        printf("📤 天气消息已发送到UI队列\n\n");
    } else {
        printf("❌ 天气消息发送失败\n\n");
    }
}

/**
 * @brief 更新天气UI显示（主线程调用）
 * @param weather 天气数据指针
 * @note ⚠️ 此函数只能在主线程中调用！
 */
static void update_weather_ui(ui_weather_data_t *weather) {
    printf("\n========== 更新天气UI（主线程） ==========\n");
    
    /* ========== 第一步：保存数据到全局缓存 ========== */
    // 保存日期
    if(strlen(weather->date) > 0) {
        strncpy(g_weather_state.date, weather->date, sizeof(g_weather_state.date) - 1);
        g_weather_state.date[sizeof(g_weather_state.date) - 1] = '\0';
    }
    
    // 保存星期
    g_weather_state.weekday = weather->weekday;
    
    // 保存城市
    strncpy(g_weather_state.city, weather->city, sizeof(g_weather_state.city) - 1);
    g_weather_state.city[sizeof(g_weather_state.city) - 1] = '\0';
    
    // 保存天气状态
    strncpy(g_weather_state.weather, weather->weather, sizeof(g_weather_state.weather) - 1);
    g_weather_state.weather[sizeof(g_weather_state.weather) - 1] = '\0';
    
    // 保存温度
    strncpy(g_weather_state.temperature, weather->temperature, sizeof(g_weather_state.temperature) - 1);
    g_weather_state.temperature[sizeof(g_weather_state.temperature) - 1] = '\0';
    
    // 保存天气代码
    strncpy(g_weather_state.weather_code, weather->code, sizeof(g_weather_state.weather_code) - 1);
    g_weather_state.weather_code[sizeof(g_weather_state.weather_code) - 1] = '\0';
    
    // 保存时间（仅首次）
    if(!g_weather_state.initialized && strlen(weather->update_time) > 0) {
        if(sscanf(weather->update_time, "%d:%d:%d", 
                  &g_weather_state.hour, 
                  &g_weather_state.minute, 
                  &g_weather_state.second) == 3) {
            g_weather_state.initialized = true;
            printf("✅ 时间已从天气API同步: %02d:%02d:%02d\n", 
                   g_weather_state.hour, g_weather_state.minute, g_weather_state.second);
        } else {
            printf("⚠️  时间解析失败: %s\n", weather->update_time);
        }
    }
    
    printf("💾 数据已保存到全局缓存\n\n");
    
    /* ========== 第二步：更新UI显示（仅主线程可执行） ========== */
    // 更新日期
    if(label_date != NULL && strlen(g_weather_state.date) > 0) {
        lv_label_set_text(label_date, g_weather_state.date);
        printf("✅ 日期标签已更新: %s\n", g_weather_state.date);
    }
    
    // 更新星期
    if(label_week != NULL) {
        const char *weekday_names[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
        if(g_weather_state.weekday >= 0 && g_weather_state.weekday <= 6) {
            lv_label_set_text(label_week, weekday_names[g_weather_state.weekday]);
            printf("✅ 星期标签已更新: %s\n", weekday_names[g_weather_state.weekday]);
        }
    }
    
    // 更新城市名称
    if(label_city != NULL) {
        lv_label_set_text(label_city, g_weather_state.city);
        printf("✅ 城市标签已更新: %s\n", g_weather_state.city);
    }
    
    // 更新天气状态和温度
    if(label_weather != NULL) {
        char weather_display[64];
        snprintf(weather_display, sizeof(weather_display), "%s %s°C", 
                 g_weather_state.weather, g_weather_state.temperature);
        lv_label_set_text(label_weather, weather_display);
        printf("✅ 天气标签已更新: %s\n", weather_display);
    }
    
    // 更新天气图标
    if(icon_label != NULL) {
        const char *icon_path = get_weather_icon(g_weather_state.weather_code);
        lv_img_set_src(icon_label, icon_path);
        printf("✅ 天气图标已更新: code=%s -> icon=%s\n", g_weather_state.weather_code, icon_path);
    }
    
    // 更新时间显示
    if(label_time != NULL && g_weather_state.initialized) {
        lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", 
            g_weather_state.hour, g_weather_state.minute, g_weather_state.second);
        printf("✅ 时间标签已更新: %02d:%02d:%02d\n", 
               g_weather_state.hour, g_weather_state.minute, g_weather_state.second);
    }
    
    printf("=================================\n\n");
}

/**
 * @brief UI消息处理函数（主线程调用）
 * @param msg UI消息指针
 * @note ⚠️ 此函数是唯一能操作LVGL的地方！必须在主线程调用
 */
void ui_msg_handle(ui_msg_t *msg) {
    if (msg == NULL) return;
    
    switch (msg->type) {
        case UI_MSG_WEATHER_OK:
            printf("📥 处理天气消息: UI_MSG_WEATHER_OK\n");
            update_weather_ui(&msg->data.weather);
            break;
            
        case UI_MSG_WEATHER_FAIL:
            printf("📥 处理天气消息: UI_MSG_WEATHER_FAIL\n");
            if (label_weather != NULL) {
                lv_label_set_text(label_weather, "获取失败");
            }
            break;
            
        case UI_MSG_WIFI_CONNECTED:
            printf("📥 处理WiFi消息: UI_MSG_WIFI_CONNECTED\n");
            if (label_status != NULL) {
                lv_label_set_text(label_status, "WiFi已连接");
            }
            break;
            
        case UI_MSG_WIFI_DISCONNECTED:
            printf("📥 处理WiFi消息: UI_MSG_WIFI_DISCONNECTED\n");
            if (label_status != NULL) {
                lv_label_set_text(label_status, "WiFi未连接");
            }
            break;
            
        case UI_MSG_NETWORK_ERROR:
            printf("📥 处理网络消息: UI_MSG_NETWORK_ERROR\n");
            if (label_weather != NULL) {
                lv_label_set_text(label_weather, "网络错误");
            }
            break;
            
        default:
            printf("⚠️  未知消息类型: %d\n", msg->type);
            break;
    }
}

/**
 * @brief 清理页面资源
 */
void cleanup_pageStart(void)
{
    printf("Cleaning up pageStart resources...\n");
    
    // 1. 删除定时器
    if(time_timer != NULL) {
        lv_timer_del(time_timer);
        time_timer = NULL;
        printf("Time timer deleted\n");
    }
    
    // 2. 移除屏幕上的所有事件回调（特别是手势事件）
    lv_obj_remove_event_cb(lv_scr_act(), swipe_event_cb);
    printf("Gesture event removed\n");
    
    // 3. 清空全局变量
    label_time = NULL;
    label_date = NULL;
    label_week = NULL;
    label_weather = NULL;
    label_city = NULL;
    icon_label = NULL;
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
 * @note 每秒递增时间，初始值来自天气API的update_time字段
 */
static void time_update_timer_cb(lv_timer_t * timer)
{
    // 如果时间未初始化，等待天气API回调
    if(!g_weather_state.initialized) {
        return;
    }
    
    // 时间递增
    g_weather_state.second++;
    if(g_weather_state.second >= 60) {
        g_weather_state.second = 0;
        g_weather_state.minute++;
        if(g_weather_state.minute >= 60) {
            g_weather_state.minute = 0;
            g_weather_state.hour++;
            if(g_weather_state.hour >= 24) {
                g_weather_state.hour = 0;
            }
        }
    }
    
    // 更新UI显示
    if(label_time != NULL) {
        lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", 
            g_weather_state.hour, g_weather_state.minute, g_weather_state.second);
    }
}

/**
 * @brief 初始化开始页面
 * @note 创建包含头像、时间、天气和状态信息的启动页面
 */
void init_pageStart(void)
{
    printf("🔹 [DEBUG] init_pageStart() ENTER\n");
    
    /* ========== 0. 设置屏幕背景颜色 ========== */
    printf("🔹 [DEBUG] Step 0: Setting background color\n");
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN);  // 黑色背景
    
    /* ========== 1. 头像区域（左上角）========== */
    printf("🔹 [DEBUG] Step 1: Creating avatar container\n");
    lv_obj_t * avatar_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(avatar_container,280, 280);  // 头像容器大小
    lv_obj_align(avatar_container, LV_ALIGN_TOP_LEFT,0, 0);  // 左上角，偏移0
    
    // 设置头像容器样式
    lv_obj_set_style_radius(avatar_container, 60, LV_PART_MAIN);  // 圆形
    lv_obj_set_style_border_width(avatar_container, 0, LV_PART_MAIN);  // 无边框
    lv_obj_set_style_pad_all(avatar_container, 0, LV_PART_MAIN);  // 无内边距
    
    // 创建头像图片对象（用户需自行添加图片路径）
    printf("🔹 [DEBUG] Step 1.1: Creating avatar image\n");
    lv_obj_t * img_avatar = lv_img_create(avatar_container);
    // TODO: 用户添加头像图片路径，例如：
    printf("🔹 [DEBUG] Step 1.2: Loading watch1.png\n");
    lv_img_set_src(img_avatar, "A:res/image/start/watch1.png");
    // 或使用图片数组：lv_img_set_src(img_avatar, &avatar_img);
    lv_obj_center(img_avatar);  // 头像居中
    printf("🔹 [DEBUG] Step 1.3: Avatar loaded successfully\n");
    
    // // 临时占位标签（实际使用时可删除）- 位于头像右侧的侧
    // lv_obj_t * label_avatar_placeholder = lv_label_create(lv_scr_act());
    // lv_label_set_text(label_avatar_placeholder, "HuneXi");
    // lv_obj_set_style_text_color(label_avatar_placeholder, lv_color_hex(0x808080), LV_PART_MAIN);
    // lv_obj_align_to(label_avatar_placeholder, avatar_container, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 在头像右侧，间隔10px
    
    
    /* ========== 2. 时间显示区（独立容器）========== */
printf("🔹 [DEBUG] Step 2: Creating time container\n");
lv_obj_t * time_container = lv_obj_create(lv_scr_act());
lv_obj_set_size(time_container, 120, 280);       // 时间容器尺寸 120x280
lv_obj_align(time_container, LV_ALIGN_TOP_LEFT, 280, 0);  // 紧挨着头像容器右侧

// 时间容器样式
lv_obj_set_style_bg_opa(time_container, LV_OPA_TRANSP, LV_PART_MAIN);
lv_obj_set_style_border_width(time_container, 0, LV_PART_MAIN);
lv_obj_set_style_radius(time_container, 0, LV_PART_MAIN);
lv_obj_set_style_pad_all(time_container, 0, LV_PART_MAIN);  // 无内边距

// 时间显示
label_time = lv_label_create(time_container);
// 从缓存恢复时间，如果有的话
if(g_weather_state.initialized) {
    lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", 
        g_weather_state.hour, g_weather_state.minute, g_weather_state.second);
} else {
    lv_label_set_text(label_time, "--:--:--");  // 等待天气API同步时间
}
obj_font_set(label_time, FONT_TYPE_NUMBER, 20);  // 使用数字字体
lv_obj_set_style_text_color(label_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
lv_obj_align(label_time, LV_ALIGN_TOP_RIGHT, -5,100);

// 日期显示
label_date = lv_label_create(time_container);
// 从缓存恢复日期，如果有的话
if(strlen(g_weather_state.date) > 0) {
    lv_label_set_text(label_date, g_weather_state.date);
} else {
    lv_label_set_text(label_date, "----/--/--");  // 等待API同步
}
obj_font_set(label_date, FONT_TYPE_NUMBER, 20);  // 使用数字字体
lv_obj_set_style_text_color(label_date, lv_color_hex(0xBDC3C7), LV_PART_MAIN);
lv_obj_align(label_date, LV_ALIGN_TOP_RIGHT, -5, 130);

// 星期显示
label_week = lv_label_create(time_container);
// 从缓存恢复星期，如果有的话
if(g_weather_state.initialized && g_weather_state.weekday >= 0 && g_weather_state.weekday <= 6) {
    const char *weekday_names[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
    lv_label_set_text(label_week, weekday_names[g_weather_state.weekday]);
} else {
    lv_label_set_text(label_week, "---");  // 等待API同步
}
obj_font_set(label_week, FONT_TYPE_CN, 20);  // 使用中文字体
lv_obj_set_style_text_color(label_week, lv_color_hex(0xBDC3C7), LV_PART_MAIN);
lv_obj_align(label_week, LV_ALIGN_TOP_RIGHT,-5, 150);

/* ========== 3. 天气信息区（独立容器）========== */
lv_obj_t * weather_container = lv_obj_create(lv_scr_act());
lv_obj_set_size(weather_container, 120, 280);    // 天气容器尺寸 120x280
lv_obj_align(weather_container, LV_ALIGN_TOP_LEFT, 400, 0);  // 位于时间容器右侧 (280+120=400)

// 天气容器样式
lv_obj_set_style_bg_opa(weather_container, LV_OPA_TRANSP, LV_PART_MAIN);
lv_obj_set_style_border_width(weather_container, 0, LV_PART_MAIN);
lv_obj_set_style_radius(weather_container, 0, LV_PART_MAIN);
lv_obj_set_style_pad_all(weather_container, 0, LV_PART_MAIN);  // 无内边距

// 天气图标（使用PNG图片）
icon_label = lv_img_create(weather_container);
// 从缓存恢复天气图标，如果有的话
if(g_weather_state.initialized && strlen(g_weather_state.weather_code) > 0) {
    const char *icon_path = get_weather_icon(g_weather_state.weather_code);
    lv_img_set_src(icon_label, icon_path);
} else {
    lv_img_set_src(icon_label, "A:res/image/start/weather_cloudy.png");  // 默认多云图标
}
lv_obj_set_size(icon_label,36,36);  // 设置图标大小
lv_obj_align(icon_label, LV_ALIGN_TOP_LEFT, 15, 90); 

//城市
label_city = lv_label_create(weather_container);
// 从缓存恢复城市，如果有的话
if(strlen(g_weather_state.city) > 0) {
    lv_label_set_text(label_city, g_weather_state.city);
} else {
    lv_label_set_text(label_city, "北京");  // 默认占位符
}
obj_font_set(label_city, FONT_TYPE_CN, 20);  // 使用中文字体
lv_obj_set_style_text_color(label_city, lv_color_hex(0xECF0F1), LV_PART_MAIN);
lv_obj_align(label_city, LV_ALIGN_TOP_LEFT,5,140);

// 天气状态 + 温度
label_weather = lv_label_create(weather_container);
// 从缓存恢复天气数据，如果有的话
if(g_weather_state.initialized && strlen(g_weather_state.weather) > 0) {
    char weather_display[64];
    snprintf(weather_display, sizeof(weather_display), "%s %s°C", 
             g_weather_state.weather, g_weather_state.temperature);
    lv_label_set_text(label_weather, weather_display);
} else {
    // 测试用随机天气（仅首次显示）
    const char * weather_states[] = {"雾", "晴", "多云", "雨", "雪", "阴"};
    int random_index = lv_rand(0, 5);
    int random_temp  = lv_rand(15, 30);
    lv_label_set_text_fmt(label_weather, "%s %d°C",
                          weather_states[random_index], random_temp);
}
obj_font_set(label_weather, FONT_TYPE_CN, 20);  // 使用中文字体
lv_obj_set_style_text_color(label_weather, lv_color_hex(0xECF0F1), LV_PART_MAIN);
lv_obj_align(label_weather, LV_ALIGN_TOP_LEFT,5,160);
lv_obj_align(label_city, LV_ALIGN_TOP_LEFT,5,140);

// 天气状态 + 温度
label_weather = lv_label_create(weather_container);

// 测试用随机天气
const char * weather_states[] = {"雾", "晴", "多云", "雨", "雪", "阴"};
int random_index = lv_rand(0, 5);
int random_temp  = lv_rand(15, 30);

lv_label_set_text_fmt(label_weather, "%s %d°C",
                      weather_states[random_index], random_temp);

obj_font_set(label_weather, FONT_TYPE_CN, 20);  // 使用中文字体
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

