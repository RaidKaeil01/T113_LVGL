/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 02:34:34
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"

static lv_style_t com_style;

typedef struct{
    SETTING_PARAM_TYPE_E type;
    char title[50];
    char *img_url;
    char *bg_img_url;
    int param_num;
    char param[100];
}setting_param_t;

static setting_param_t setting_param[3] = {
    {TYPE_BODY_SENSOR,"人体感应触发方式：",GET_IMAGE_PATH("icon_body_sensor.png"),GET_IMAGE_PATH("bg_1.png"),3,"仅弹窗提醒\n弹窗+声音提醒"},
    {TYPE_SMART_COASTER,"喝水提醒间隔时间：",GET_IMAGE_PATH("icon_smart_coaster.png"),GET_IMAGE_PATH("bg_2.png"),9,"10min\n20min\n30min\n40min\n50min\n60min\n70min\n80min\n90min"},
    {TYPE_FLAME_SENSOR,"火焰监测时长：",GET_IMAGE_PATH("icon_flame_sensor.png"),GET_IMAGE_PATH("bg_3.png"),9,"10min\n20min\n30min\n40min\n50min\n60min\n70min\n80min\n90min"},
};

static lv_obj_t * roller_obj = NULL;
static uint16_t roller_index = 1;

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

static void back_btn_click_event_cb(lv_event_t * e){
    delete_current_page(&com_style);
    init_page_linkage();
}

static void roller_event_handler(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        // char buf[10];
        // lv_roller_get_selected_str(obj, buf, sizeof(buf));
        roller_index = lv_roller_get_selected(obj);
        printf("roller index = %d\n",roller_index);
    }
}

static void select_btn_click_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    SETTING_PARAM_TYPE_E type  = (SETTING_PARAM_TYPE_E)lv_event_get_user_data(e);
    device_state_t* device_state = get_device_state();
    switch (type)
    {
    case TYPE_BODY_SENSOR:
        device_state->body_sensor_notice_type = roller_index;
        printf("set body_sensor_notice_type-->%d\n",device_state->body_sensor_notice_type);
        break;
    case TYPE_SMART_COASTER:
        device_state->smart_coaster_time = 10*(roller_index+1);
        device_state->smart_coaster_remaining_time = device_state->smart_coaster_time;
        printf("set smart_coaster_time-->%d\n",device_state->smart_coaster_time);
        break;
    case TYPE_FLAME_SENSOR:
        device_state->flame_sensor_time = 10*(roller_index+1);
        device_state->flame_sensor_remaining_time = device_state->flame_sensor_time;
        printf("set flame_sensor_time-->%d\n",device_state->flame_sensor_time);
        break;    
    default:
        break;
    }
    device_param_write();
    delete_current_page(&com_style);
    init_page_linkage();
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
    lv_obj_set_style_pad_left(back_img,20,LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img,20,LV_PART_MAIN);

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

static lv_obj_t * init_select_btn(lv_obj_t *parent,SETTING_PARAM_TYPE_E type){
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_style(btn,&com_style,0);
    lv_obj_set_size(btn,171,66);
    lv_obj_clear_state(btn,LV_STATE_FOCUS_KEY);
    lv_obj_set_style_border_width(btn, 0,0);
    lv_obj_set_style_shadow_width(btn, 0,0);
    lv_obj_set_style_radius(btn,35,0);
    lv_obj_set_style_bg_color(btn,APP_COLOR_BUTTON_DEFALUT,0);
    // lv_obj_set_style_opa(btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn,select_btn_click_event_cb,LV_EVENT_CLICKED,(void *)type);

    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    lv_label_set_text(btn_label,"确定");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

static lv_obj_t * init_setting_param_view(lv_obj_t *parent,SETTING_PARAM_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    // lv_obj_set_style_bg_color(cont,APP_COLOR_BUTTON_UNSELECT,0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    //弹性布局对齐方式
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,30,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    lv_obj_t *setting_img = lv_img_create(cont);
    lv_img_set_src(setting_img,setting_param[type].img_url);
    lv_obj_set_style_pad_top(setting_img,10,LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,setting_param[type].title);
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);

    roller_obj  = lv_roller_create(cont);
    lv_obj_set_style_border_width(roller_obj,0,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,LV_PART_SELECTED);
    lv_obj_set_style_bg_color(roller_obj,lv_color_black(),0);
    lv_obj_set_style_bg_opa(roller_obj, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_obj,LV_OPA_TRANSP,LV_PART_MAIN);
    lv_roller_set_options(roller_obj,
                        setting_param[type].param,
                        LV_ROLLER_MODE_NORMAL);
    lv_obj_add_event_cb(roller_obj, roller_event_handler, LV_EVENT_ALL, NULL);
    lv_font_t* font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_2);
    if(font_select != NULL)
        lv_obj_set_style_text_font(roller_obj, font_select, LV_PART_SELECTED);
    lv_font_t* un_font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_3);
	if(un_font_select != NULL)
		lv_obj_set_style_text_font(roller_obj, un_font_select, 0);
    lv_obj_set_style_text_line_space(roller_obj,10,0);
    lv_obj_set_height(roller_obj,400);
    lv_obj_set_width(roller_obj,250);
    lv_roller_set_visible_row_count(roller_obj, 3);
    lv_roller_set_selected(roller_obj,1,LV_ANIM_ON);
    lv_obj_clear_state(roller_obj,LV_STATE_FOCUS_KEY);

    init_select_btn(cont,type);

    return cont;

}

void init_page_linkage_setting(SETTING_PARAM_TYPE_E type)
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);


    lv_obj_t *title_view =  init_title_view(cont);
    
    lv_obj_t * setting_param_view = init_setting_param_view(cont,type);
    lv_obj_align_to(setting_param_view,title_view,LV_ALIGN_OUT_BOTTOM_MID,0,30);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,setting_param[type].bg_img_url);
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);
    
}