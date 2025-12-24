/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:15 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 19:09:33
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "utils.h"
#include "ble_mesh.h"

typedef struct{
    CONNECT_STATE_E state;
    char str[50];
    char *img_url;
}connect_state_t;

static connect_state_t connect_state_list[] = {
    {DISCONNECT,"未连接",GET_IMAGE_PATH("icon_state_disconnect.png")},
    {CONNECTING,"连接中",GET_IMAGE_PATH("icon_state_connect.png")},
    {CONNECT,"已连接",GET_IMAGE_PATH("icon_state_connect.png")},
};

typedef enum{
    ITEM_RGB = 0,
    ITEM_SWITCH,
    ITEM_BODY_SENSOR,
    ITEM_SMART_COASTER,
    ITEM_FLAME_SENSOR,
}ITEM_TYPE_E;

typedef struct{
    ITEM_TYPE_E state;
    char name[50];
    char *img_url;
    lv_obj_t *connect_img_obj;
    lv_obj_t *connect_str_obj;
}item_info_t;

static item_info_t item_info_list[] = {
    {ITEM_RGB,"RGB灯",GET_IMAGE_PATH("icon_rgb.png"),NULL,NULL},
    {ITEM_SWITCH,"mesh开关",GET_IMAGE_PATH("icon_switch.png"),NULL,NULL},
    {ITEM_BODY_SENSOR,"人体感应",GET_IMAGE_PATH("icon_body_sensor.png"),NULL,NULL},
    {ITEM_SMART_COASTER,"智能杯垫",GET_IMAGE_PATH("icon_smart_coaster.png"),NULL,NULL},
    {ITEM_FLAME_SENSOR,"火焰监测",GET_IMAGE_PATH("icon_flame_sensor.png"),NULL,NULL},
};

//当前界面支持的所有按键类型
typedef enum{
    BTN_RGB_R = 0,
    BTN_RGB_G,
    BTN_RGB_B,
    BTN_SWITCH,
    BTN_BODY_SENSOR_SWITCH,
    BTN_BODY_SENSOR_EDIT,
    BTN_SMART_COASTER_SWITCH,
    BTN_SMART_COASTER_EDIT,
    BTN_FLAME_SENSOR_SWITCH,
    BTN_FLAME_SENSOR_EDIT,
}BTN_TYPE_E;


static lv_style_t com_style;
static lv_timer_t * refresh_timer = NULL;
static lv_obj_t *smart_coaster_time_label = NULL;
static lv_obj_t *flame_sensor_time_label = NULL;
static lv_obj_t *flame_state_label = NULL;
static bool is_cancle = false;
static bool is_symbol = false;

static void com_style_init(){
    lv_style_init(&com_style);
    if(lv_style_is_empty(&com_style) == false)
        lv_style_reset(&com_style);
    lv_style_set_bg_color(&com_style,APP_COLOR_BLACK);
    lv_style_set_radius(&com_style,0);
    lv_style_set_border_width(&com_style,0);
    lv_style_set_pad_all(&com_style,0);
}

static void obj_font_set(lv_obj_t *obj,int type, uint16_t weight){
    lv_font_t* font = get_font(type, weight);
	if(font != NULL)
		lv_obj_set_style_text_font(obj, font, 0);
}

static void refresh_timer_cb_func(lv_timer_t * timer)
{
    // printf("refresh_timer_cb_func timer run ...\n");
    if(is_cancle == true)
        return;
    device_state_t* device_state = get_device_state();

    if(is_symbol){//闪烁实现
        is_symbol = false;
        if(device_state->flame_sensor_state == ON)
            lv_label_set_text_fmt(flame_sensor_time_label,"关火倒计时 %s",get_time_str(device_state->flame_sensor_remaining_time));
        if(device_state->smart_coaster_state == ON)
            lv_label_set_text_fmt(smart_coaster_time_label,"提醒倒计时 %s",get_time_str(device_state->smart_coaster_remaining_time));
    }else{
        is_symbol = true;
        if(device_state->flame_sensor_state == ON)
            lv_label_set_text_fmt(flame_sensor_time_label,"关火倒计时 %s",get_time_str_nosymbol(device_state->flame_sensor_remaining_time));
        if(device_state->smart_coaster_state == ON)
            lv_label_set_text_fmt(smart_coaster_time_label,"提醒倒计时 %s",get_time_str_nosymbol(device_state->smart_coaster_remaining_time));
    }

    lv_img_set_src(item_info_list[0].connect_img_obj,connect_state_list[device_state->led_connect_state].img_url);
    lv_label_set_text_fmt(item_info_list[0].connect_str_obj,"%s %d%%",connect_state_list[device_state->led_connect_state].str,device_state->switch_battery_level);

    lv_img_set_src(item_info_list[1].connect_img_obj,connect_state_list[device_state->switch_connect_state].img_url);
    lv_label_set_text_fmt(item_info_list[1].connect_str_obj,"%s %d%%",connect_state_list[device_state->switch_connect_state].str,device_state->switch_battery_level);

    lv_img_set_src(item_info_list[2].connect_img_obj,connect_state_list[device_state->body_sensor_connect_state].img_url);
    lv_label_set_text_fmt(item_info_list[2].connect_str_obj,"%s %d%%",connect_state_list[device_state->body_sensor_connect_state].str,device_state->body_sensor_battery_level);

    lv_img_set_src(item_info_list[3].connect_img_obj,connect_state_list[device_state->smart_coaster_connect_state].img_url);
    lv_label_set_text_fmt(item_info_list[3].connect_str_obj,"%s %d%%",connect_state_list[device_state->smart_coaster_connect_state].str,device_state->smart_coaster_battery_level);

    lv_img_set_src(item_info_list[4].connect_img_obj,connect_state_list[device_state->flame_sensor_connect_state].img_url);
    lv_label_set_text_fmt(item_info_list[4].connect_str_obj,"%s %d%%",connect_state_list[device_state->flame_sensor_connect_state].str,device_state->flame_sensor_battery_level);

    if(device_state->smart_coaster_data == 1){
        device_state->smart_coaster_remaining_time = device_state->smart_coaster_time;
    }
    lv_label_set_text_fmt(flame_state_label,"火焰状态:%d",device_state->flame_sensor_data);
}

static void back_btn_click_event_cb(lv_event_t * e){
    is_cancle = true;
    lv_timer_del(refresh_timer);
    refresh_timer = NULL;
    delete_current_page(&com_style);
    init_page_main();
}

static void btn_click_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    BTN_TYPE_E type  = (BTN_TYPE_E)lv_event_get_user_data(e);

    device_state_t* device_state = get_device_state();
 
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * swbtn = lv_event_get_target(e);
        printf("sw btn type = %d\n",type);
        SWITCH_STATE_E state = OFF;
        if(lv_obj_has_state(swbtn, LV_STATE_CHECKED)){
            state = ON;
            printf("sw btn open\n");
        }else{
            state = OFF;
            printf("sw btn close\n");
        }
        switch (type)
        {
        case BTN_RGB_R:
            device_state->led_r = state;
            ctrl_rgb_and_switch(device_state->led_r,device_state->led_g,device_state->led_b,device_state->mesh_switch_state);
            break;
        case BTN_RGB_G:
            device_state->led_g = state;
            ctrl_rgb_and_switch(device_state->led_r,device_state->led_g,device_state->led_b,device_state->mesh_switch_state);
            break;
        case BTN_RGB_B:
            device_state->led_b = state;
            ctrl_rgb_and_switch(device_state->led_r,device_state->led_g,device_state->led_b,device_state->mesh_switch_state);
            break;
        case BTN_SWITCH:
            device_state->mesh_switch_state = state;
            ctrl_rgb_and_switch(device_state->led_r,device_state->led_g,device_state->led_b,device_state->mesh_switch_state);
            break;
        case BTN_BODY_SENSOR_SWITCH:
            device_state->body_sensor_state = state;
            break;
        case BTN_SMART_COASTER_SWITCH:
            device_state->smart_coaster_state = state;
            break;
        case BTN_FLAME_SENSOR_SWITCH:
            device_state->flame_sensor_state = state;
            break;        
        default:
            break;
        }

    }else if(code == LV_EVENT_CLICKED){
        printf("edit btn type = %d\n",type);
        is_cancle = true;
        lv_timer_del(refresh_timer);
        refresh_timer = NULL;
        delete_current_page(&com_style);
        switch (type)
        {
        case BTN_BODY_SENSOR_EDIT:
            init_page_linkage_setting(TYPE_BODY_SENSOR);
            break;
        case BTN_SMART_COASTER_EDIT:
            init_page_linkage_setting(TYPE_SMART_COASTER);
            break;
        case BTN_FLAME_SENSOR_EDIT:
            init_page_linkage_setting(TYPE_FLAME_SENSOR);
            break;        
        default:
            break;
        }
    }
}

static lv_obj_t * init_title_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *back_img = lv_img_create(cont);
    lv_img_set_src(back_img,GET_IMAGE_PATH("icon_back.png"));
    lv_obj_set_align(back_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_left(back_img,5,LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img,5,LV_PART_MAIN);

    lv_obj_t *menu_img = lv_img_create(cont);
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_linkage.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"设备联动");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);
    return cont;
}

/**
 * 初始化连接状态界面
 */
static lv_obj_t * init_item_connect_state_view(lv_obj_t *parent,ITEM_TYPE_E type, CONNECT_STATE_E state){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_style_pad_top(cont,5,LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    lv_obj_t *state_img = lv_img_create(cont);
    lv_img_set_src(state_img,connect_state_list[state].img_url);
    lv_obj_set_align(state_img,LV_ALIGN_LEFT_MID);

    lv_obj_t *state_str = lv_label_create(cont);
    obj_font_set(state_str,FONT_TYPE_LETTER, FONT_SIZE_TEXT_5);
    lv_label_set_text(state_str,connect_state_list[state].str);
    lv_obj_set_style_text_color(state_str,APP_COLOR_WHITE,0);
    lv_obj_align_to(state_str,state_img,LV_ALIGN_OUT_RIGHT_MID,5,-3);

    item_info_list[type].connect_img_obj = state_img;
    item_info_list[type].connect_str_obj = state_str;

    return cont;
}

static lv_obj_t * init_item_title_view(lv_obj_t *parent,ITEM_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_style_pad_top(cont,5,LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img,item_info_list[type].img_url);
    lv_obj_set_align(img,LV_ALIGN_LEFT_MID);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,item_info_list[type].name);
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,img,LV_ALIGN_OUT_RIGHT_MID,5,-5);

    return cont;
}


static lv_obj_t * init_item_btn_view(lv_obj_t *parent,char *name_str,BTN_TYPE_E type,lv_state_t state){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,name_str);
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_set_align(title,LV_ALIGN_LEFT_MID);

    lv_obj_t* sw = lv_switch_create(cont);
    lv_obj_set_size(sw,80,35);
    lv_obj_align_to(sw,title,LV_ALIGN_OUT_RIGHT_MID,20,15);
    //选中状态
	lv_obj_set_style_border_width(sw, 2, LV_PART_MAIN);
	lv_obj_set_style_bg_color(sw, APP_COLOR_BUTTON_DEFALUT, LV_PART_INDICATOR | LV_STATE_CHECKED);
	lv_obj_set_style_bg_opa(sw, LV_OPA_100, LV_PART_INDICATOR | LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(sw, APP_COLOR_WHITE, LV_PART_KNOB | LV_STATE_CHECKED);
	lv_obj_set_style_bg_opa(sw, LV_OPA_100, LV_PART_KNOB | LV_STATE_CHECKED);
	lv_obj_set_style_pad_all(sw, -5, LV_PART_KNOB | LV_STATE_CHECKED);
	lv_obj_set_style_border_color(sw, APP_COLOR_BUTTON_DEFALUT, LV_PART_MAIN | LV_STATE_CHECKED);
	lv_obj_set_style_border_opa(sw, LV_OPA_100, LV_PART_MAIN | LV_STATE_CHECKED);
	//未选中状态
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x999999), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(sw, LV_OPA_10, LV_PART_MAIN);
	lv_obj_set_style_border_color(sw, lv_color_hex(0x999999), LV_PART_MAIN);
	lv_obj_set_style_border_opa(sw, LV_OPA_100, LV_PART_MAIN);
	lv_obj_set_style_bg_color(sw, lv_color_hex(0x999999), LV_PART_KNOB);
	lv_obj_set_style_bg_opa(sw, LV_OPA_100, LV_PART_KNOB);
	lv_obj_set_style_pad_all(sw, -10, LV_PART_KNOB);
    lv_obj_add_flag(sw, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_clear_state(sw,LV_STATE_FOCUS_KEY);
    lv_obj_add_event_cb(sw, btn_click_event_cb, LV_EVENT_VALUE_CHANGED, (void *)type);
    
    lv_obj_add_state(sw,state);

    return cont;
}

static lv_obj_t * init_item_edit_view(lv_obj_t *parent,BTN_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 150,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_style_pad_top(cont,0,LV_PART_MAIN);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, btn_click_event_cb, LV_EVENT_CLICKED, (void *)type);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img,GET_IMAGE_PATH("icon_edit.png"));
    lv_obj_set_align(img,LV_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_5);
    lv_label_set_text(title,"编辑配置");
    lv_obj_set_style_text_color(title,APP_COLOR_GRAY,0);
    lv_obj_align_to(title,img,LV_ALIGN_OUT_BOTTOM_MID,0,15);

    return cont;
}

/**
 * RGB 界面
 */
static lv_obj_t * init_rgb_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_COLUMN);
    //弹性布局对齐方式
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,0,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    device_state_t* device_state = get_device_state();
    init_item_connect_state_view(cont,ITEM_RGB,device_state->led_connect_state);
    init_item_title_view(cont,ITEM_RGB);
    init_item_btn_view(cont,"R:",BTN_RGB_R,device_state->led_r);
    init_item_btn_view(cont,"G:",BTN_RGB_G,device_state->led_g);
    init_item_btn_view(cont,"B:",BTN_RGB_B,device_state->led_b);

    return cont;
}

/**
 * 开关界面
 */
static lv_obj_t * init_switch_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,0,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    device_state_t* device_state = get_device_state();
    init_item_connect_state_view(cont,ITEM_SWITCH,device_state->switch_connect_state);
    init_item_title_view(cont,ITEM_SWITCH);
    init_item_btn_view(cont,"开关",BTN_SWITCH,device_state->mesh_switch_state);

    return cont;
}

/**
 * 人体感应界面
 */
static lv_obj_t *init_body_sensor_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,0,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    device_state_t* device_state = get_device_state();
    init_item_connect_state_view(cont,ITEM_BODY_SENSOR,device_state->body_sensor_connect_state);
    init_item_title_view(cont,ITEM_BODY_SENSOR);
    init_item_btn_view(cont,"开关",BTN_BODY_SENSOR_SWITCH,device_state->body_sensor_state);

    init_item_edit_view(cont,BTN_BODY_SENSOR_EDIT);
    return cont;
}

/**
 * 智能杯垫界面
 */
static lv_obj_t *init_smart_coaster_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,0,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    device_state_t* device_state = get_device_state();
    init_item_connect_state_view(cont,ITEM_SMART_COASTER,device_state->smart_coaster_connect_state);
    init_item_title_view(cont,ITEM_SMART_COASTER);
    init_item_btn_view(cont,"开关",BTN_SMART_COASTER_SWITCH,device_state->smart_coaster_state);

    lv_obj_t *time = lv_label_create(cont);
    smart_coaster_time_label = time;
    obj_font_set(time,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_recolor(time, true); 
    lv_obj_set_style_text_color(time,APP_COLOR_WHITE,0);
    lv_label_set_text_fmt(time,"提醒倒计时 %s",get_time_str(device_state->smart_coaster_remaining_time));
    init_item_edit_view(cont,BTN_SMART_COASTER_EDIT);

    return cont;
}  

/**
 * 火焰监测倒计时
 */
static lv_obj_t *init_flame_sensor_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,0,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    device_state_t* device_state = get_device_state();

    init_item_connect_state_view(cont,ITEM_FLAME_SENSOR,device_state->flame_sensor_connect_state);
    init_item_title_view(cont,ITEM_FLAME_SENSOR);
    init_item_btn_view(cont,"开关",BTN_FLAME_SENSOR_SWITCH,device_state->flame_sensor_state);

    lv_obj_t *state_label = lv_label_create(cont);
    flame_state_label = state_label;
    obj_font_set(state_label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_recolor(state_label, true); 
    lv_obj_set_style_text_color(state_label,APP_COLOR_WHITE,0);
    lv_label_set_text_fmt(state_label,"火焰状态:%d",device_state->flame_sensor_data);

    lv_obj_t *time = lv_label_create(cont);
    flame_sensor_time_label = time;
    obj_font_set(time,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_recolor(time, true); 
    lv_obj_set_style_text_color(time,APP_COLOR_WHITE,0);
    lv_label_set_text_fmt(time,"关火倒计时 %s",get_time_str(device_state->flame_sensor_remaining_time));

    init_item_edit_view(cont,BTN_FLAME_SENSOR_EDIT);
    return cont;
}

void init_page_linkage(void)
{
    is_cancle = false;
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_linkage.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);

    lv_obj_t * item_cont = lv_obj_create(cont);
    lv_obj_set_size(item_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_center(item_cont);
    lv_obj_add_style(item_cont, &com_style, 0);
    lv_obj_set_flex_flow(item_cont,LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(item_cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_gap(item_cont,90,0);
    lv_obj_set_style_bg_opa(item_cont,LV_OPA_0,0);
    lv_obj_set_style_pad_left(item_cont,50,0);

    // lv_obj_set_style_pad_top(item_cont,45,LV_PART_MAIN);
    lv_obj_align_to(item_cont,title_view,LV_ALIGN_OUT_BOTTOM_MID,0,-10);

    init_rgb_view(item_cont);
    init_switch_view(item_cont);
    init_body_sensor_view(item_cont);
    init_smart_coaster_view(item_cont);
    init_flame_sensor_view(item_cont);

    if(refresh_timer == NULL){
	    refresh_timer = lv_timer_create(refresh_timer_cb_func, 500, NULL);
    }
    
}