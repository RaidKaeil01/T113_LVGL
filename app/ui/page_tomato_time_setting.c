/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:06:34
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"

static lv_style_t com_style;

typedef enum{   
    TOMATO_TYPE_STUDY = 0,
    TOMATO_TYPE_REST,
}TOMATO_SETTING_PARAM_TYPE_E;


typedef struct{
    char title[50];
    char *img_url;
    char param[100];
    lv_obj_t * obj;
}tomato_setting_param_t;

static tomato_setting_param_t tomato_setting_param[] = {
    {"学习时间:",GET_IMAGE_PATH("icon_clock.png"),"1min\n10min\n20min\n30min\n40min\n50min\n60min\n70min\n80min\n90min",NULL},
    {"休息时间:",GET_IMAGE_PATH("icon_clock.png"),"1min\n10min\n20min\n30min\n40min\n50min\n60min\n70min\n80min\n90min",NULL},
};

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
    init_page_main();
}

static void roller_event_handler(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        // char buf[10];
        // lv_roller_get_selected_str(obj, buf, sizeof(buf));
        // uint16_t roller_index = lv_roller_get_selected(obj);
    }
}

static void select_btn_click_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    uint16_t roller0_index = lv_roller_get_selected(tomato_setting_param[TOMATO_TYPE_STUDY].obj);
    uint16_t roller1_index = lv_roller_get_selected(tomato_setting_param[TOMATO_TYPE_REST].obj);
    device_state_t* device_state = get_device_state();
    device_state->tomato_learn_time = (roller0_index)*10;
    device_state->tomato_rest_time = (roller1_index)*10;
    if(device_state->tomato_learn_time  == 0){
        device_state->tomato_learn_time = 1;
        device_state->tomato_rest_time = 1;
    }
    device_state->tomato_time_state = ON;
    printf("roller study = %dmin  rest = %dmin \n",device_state->tomato_learn_time,device_state->tomato_rest_time);

    delete_current_page(&com_style);
    init_page_tomato_learn();
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
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_tomato.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"番茄时钟设置");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    return cont;
}

static lv_obj_t * init_select_btn(lv_obj_t *parent){
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_style(btn,&com_style,0);
    lv_obj_set_size(btn,171,66);
    lv_obj_clear_state(btn,LV_STATE_FOCUS_KEY);
    lv_obj_set_style_border_width(btn, 0,0);
    lv_obj_set_style_shadow_width(btn, 0,0);
    lv_obj_set_style_radius(btn,35,0);
    lv_obj_set_style_bg_color(btn,APP_COLOR_BUTTON_DEFALUT,0);
    // lv_obj_set_style_opa(btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn,select_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    lv_label_set_text(btn_label,"确定");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

static lv_obj_t * init_setting_param_view(lv_obj_t *parent,TOMATO_SETTING_PARAM_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
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
    lv_img_set_src(setting_img,tomato_setting_param[type].img_url);
    lv_obj_set_style_pad_top(setting_img,10,LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,tomato_setting_param[type].title);
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);

    lv_obj_t *roller_obj  = lv_roller_create(cont);
    tomato_setting_param[type].obj = roller_obj;
    lv_obj_set_style_border_width(roller_obj,0,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,LV_PART_SELECTED);
    lv_obj_set_style_bg_color(roller_obj,lv_color_black(),0);
    lv_obj_set_style_bg_opa(roller_obj, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_obj,LV_OPA_TRANSP,LV_PART_MAIN);
    lv_roller_set_options(roller_obj,
                        tomato_setting_param[type].param,
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
    lv_obj_set_width(roller_obj,100);
    lv_roller_set_visible_row_count(roller_obj, 3);
    lv_roller_set_selected(roller_obj,1,LV_ANIM_ON);
    lv_obj_clear_state(roller_obj,LV_STATE_FOCUS_KEY);

    return cont;
}

void init_page_tomatotime_setting()
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_tomato_time.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);

    lv_obj_t * study_setting_view = init_setting_param_view(cont,TOMATO_TYPE_STUDY);
    lv_obj_align_to(study_setting_view,title_view,LV_ALIGN_OUT_BOTTOM_LEFT,200,10);
    lv_obj_t * rest_setting_view = init_setting_param_view(cont,TOMATO_TYPE_REST);
    lv_obj_align_to(rest_setting_view,study_setting_view,LV_ALIGN_OUT_RIGHT_MID,150,0);

    lv_obj_t * btn = init_select_btn(cont);
    lv_obj_align_to(btn,rest_setting_view,LV_ALIGN_OUT_RIGHT_MID,100,0);

}