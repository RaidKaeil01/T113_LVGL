      
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"

/* ========== 硬件控制函数 ========== */
/**
 * @brief 设置屏幕背光亮度
 * @param value 亮度值 0-100（百分比）
 */
static void em_hal_brightness_set_value(int value)
{
    char cmd[128];
    
    // 边界检查
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    
    // 0-100的值映射到0-200（硬件亮度范围）
    int brightness = 200 * value / 100;
    printf("Setting brightness: %d%% -> hardware value: %d\n", value, brightness);
    
    snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/backlight/backlight/brightness", brightness);
    system(cmd);
}

//声明通用样式
static lv_style_t com_style;
//初始化通用样式
static void com_style_init(){
    //初始化样式
    lv_style_init(&com_style);
    //判断如果样式非空，那就先重置，再设置
    if(lv_style_is_empty(&com_style) == false)
        lv_style_reset(&com_style);
    //样式背景设置为黑色，圆角设置为0，边框宽度设置为0，填充区域设置为0
    lv_style_set_bg_color(&com_style,lv_color_hex(0x000000));
    lv_style_set_radius(&com_style,0);
    lv_style_set_border_width(&com_style,0);
    lv_style_set_pad_all(&com_style,0);
    lv_style_set_outline_width(&com_style,0);
}

//封装字库获取函数
static void obj_font_set(lv_obj_t *obj,int type, uint16_t weight){
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}

/**
 * @brief 清理设置页面资源
 */
void cleanup_page_setting(void)
{
    printf("Cleaning up page_setting resources...\n");
    
    // 1. 清理通用样式
    if(lv_style_is_empty(&com_style) == false) {
        lv_style_reset(&com_style);
        printf("Style reset\n");
    }
    
    // 2. 移除屏幕上所有事件回调（防止手势事件残留）
    lv_obj_remove_event_cb(lv_scr_act(), NULL);
    
    // 3. 清理所有定时器（LVGL会自动处理已删除对象的定时器，这里做保险）
    // 注意：如果有全局定时器变量，需要在这里显式删除
    
    printf("page_setting cleanup completed\n");
}

/**
 * @brief 返回按钮点击事件回调 - 返回菜单页面
 */
static void back_click_event_cb(lv_event_t * e){
    printf("Back button clicked, returning to Menu page\n");
    
    // 清理当前页面资源
    cleanup_page_setting();
    
    // 清空屏幕
    lv_obj_clean(lv_scr_act());
    
    // 切换到菜单页面
    init_pageMenu();
}

static void init_back_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont,&com_style,LV_PART_MAIN);
    lv_obj_align(cont,LV_ALIGN_TOP_LEFT,0,10);
    
    // 禁用返回按钮容器的滚动和手势冒泡
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_t * img_back = lv_img_create(cont);
    lv_img_set_src(img_back, GET_IMAGE_PATH("main/back.png"));
    lv_obj_align(img_back,LV_ALIGN_LEFT_MID,20,0);

    lv_obj_t * img_set = lv_img_create(cont);
    lv_img_set_src(img_set, GET_IMAGE_PATH("icon_setting.png"));
    lv_obj_align_to(img_set,img_back,LV_ALIGN_OUT_RIGHT_MID,15,0);

    lv_obj_t *label = lv_label_create(cont);
    obj_font_set(label,FONT_TYPE_CN,24);
    lv_label_set_text(label, "系统设置");
    lv_obj_align_to(label,img_set,LV_ALIGN_OUT_RIGHT_MID,5,-5);
    lv_obj_set_style_text_color(label,lv_color_hex(0xffffff),LV_PART_MAIN);

    // 为返回按钮区域添加点击事件
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, back_click_event_cb, LV_EVENT_CLICKED, NULL);
}

typedef struct 
{
    int type;
    char *img_url;
    char name[10];
    int value;
}system_setting_info_t;

/* ========== 设置值记忆（静态变量，程序运行期间保持） ========== */
static int g_brightness_value = 50;  // 默认亮度50%
static int g_volume_value = 50;      // 默认音量50%

static system_setting_info_t info[2] = {
    {0, GET_IMAGE_PATH("icon_brightness.png"), "亮度", 0},  // value将在初始化时设置
    {1, GET_IMAGE_PATH("icon_volume.png"), "音量", 0},
};

// 滑动条类型标识
#define SLIDER_TYPE_BRIGHTNESS  0
#define SLIDER_TYPE_VOLUME      1

// 滑动条用户数据结构
typedef struct {
    lv_obj_t *label;    // 显示百分比的标签
    int type;           // 滑动条类型（亮度/音量）
} slider_user_data_t;

static slider_user_data_t slider_data[2];  // 存储两个滑动条的数据

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int value = (int)lv_slider_get_value(slider);
    slider_user_data_t *data = (slider_user_data_t *)lv_event_get_user_data(e);
    
    // 更新标签显示
    lv_label_set_text_fmt(data->label, "%d%%", value);
    // 重新对齐，避免数值不同不能居中
    lv_obj_align_to(data->label, slider, LV_ALIGN_OUT_RIGHT_MID, 15, -5);
    
    // 根据类型执行对应的硬件控制
    switch(data->type) {
        case SLIDER_TYPE_BRIGHTNESS:
            g_brightness_value = value;  // 保存当前亮度值
            em_hal_brightness_set_value(value);  // 调用背光设置函数
            break;
        case SLIDER_TYPE_VOLUME:
            g_volume_value = value;  // 保存当前音量值
            // TODO: 调用音量设置函数
            printf("Setting volume: %d%%\n", value);
            break;
        default:
            break;
    }
}

static lv_obj_t * init_slider_view(lv_obj_t *parent,int type){
    // 从静态变量获取当前值
    int current_value = (type == SLIDER_TYPE_BRIGHTNESS) ? g_brightness_value : g_volume_value;
    
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont,&com_style,LV_PART_MAIN);

    lv_obj_t * icon = lv_img_create(cont);
    lv_img_set_src(icon, info[type].img_url);
    lv_obj_align(icon,LV_ALIGN_LEFT_MID,20,0);

    lv_obj_t *label_name = lv_label_create(cont);
    obj_font_set(label_name,FONT_TYPE_CN,24);
    lv_label_set_text(label_name, info[type].name);
    lv_obj_align_to(label_name,icon,LV_ALIGN_OUT_RIGHT_MID,15,-10);
    lv_obj_set_style_text_color(label_name,lv_color_hex(0xffffff),LV_PART_MAIN);

    lv_obj_t * slider = lv_slider_create(cont);
    lv_slider_set_value(slider, current_value, LV_ANIM_OFF);  // 设置滑动条初始值
    lv_obj_align_to(slider,label_name,LV_ALIGN_OUT_RIGHT_MID,37,5);

    lv_obj_t *label_value = lv_label_create(cont);
    obj_font_set(label_value,FONT_TYPE_CN,24);
    lv_label_set_text_fmt(label_value, "%d%%", current_value);  // 显示当前值
    lv_obj_align_to(label_value,slider,LV_ALIGN_OUT_RIGHT_MID,15,-5);
    lv_obj_set_style_text_color(label_value,lv_color_hex(0xffffff),LV_PART_MAIN);

    // 设置滑动条用户数据（包含标签和类型）
    slider_data[type].label = label_value;
    slider_data[type].type = type;
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, &slider_data[type]);

    return cont;
}

static lv_obj_t * init_setting_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont,&com_style,LV_PART_MAIN);  
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(cont);
    lv_obj_set_style_pad_row(cont,50,LV_PART_MAIN);

    init_slider_view(cont,0);
    init_slider_view(cont,1);

    return cont;
}

void init_page_setting(){
    printf("Initializing page_setting...\n");
    
    // 初始化样式
    com_style_init();
    
    // 创建主容器
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont,LV_PCT(100),LV_PCT(100));
    lv_obj_add_style(cont,&com_style,LV_PART_MAIN);
    
    // ⚠️ 关键：禁用主容器的滚动和手势冒泡，防止误触发滑动返回
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 禁用屏幕对象的手势冒泡（防止pageStart的手势事件残留）
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 初始化子视图
    init_back_view(cont);
    init_setting_view(cont);
    
    printf("page_setting initialized successfully\n");
}

    