/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:32:02 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:26:42
 */

#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "utils.h"

static lv_style_t com_style;

static lv_obj_t * time_label = NULL;
static lv_timer_t * refresh_timer = NULL;
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

static void back_btn_click_event_cb(lv_event_t * e){
    is_cancle = true;
    lv_timer_del(refresh_timer);
    refresh_timer = NULL;
    delete_current_page(&com_style);
    init_page_tomatotime_setting();
}

static void select_btn_click_event_cb(lv_event_t * e){
    printf("tomato learn select_btn_click_event_cb\n");
    is_cancle = true;
    lv_timer_del(refresh_timer);
    refresh_timer = NULL;
    delete_current_page(&com_style);
    init_page_tomato_learn();
}

static void refresh_timer_cb_func(lv_timer_t * timer)
{
    if(is_cancle == true)
        return;
    device_state_t* device_state = get_device_state();
    if(is_symbol){//闪烁实现
        is_symbol = false;
        lv_label_set_text_fmt(time_label,"%s",get_time_str(device_state->tomato_rest_remaining_time));
    }else{
        is_symbol = true;
        lv_label_set_text_fmt(time_label,"%s",get_time_str_nosymbol(device_state->tomato_rest_remaining_time));
    }
    if(device_state->tomato_rest_remaining_time == 0){
        is_cancle = true;
        lv_timer_del(refresh_timer);
        refresh_timer = NULL;
        delete_current_page(&com_style);
        init_page_tomato_finish();
    }
}

static lv_obj_t * init_title_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_LEFT);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_set_style_bg_color(cont,APP_COLOR_BUTTON_DEFALUT,0);

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
    lv_label_set_text(title,"番茄时钟-休息中");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);
    return cont;
}

static lv_obj_t * init_info_view(lv_obj_t *parent){
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
    lv_img_set_src(setting_img,GET_IMAGE_PATH(""));
    lv_obj_set_style_pad_top(setting_img,10,LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TIME_1);
    lv_label_set_text(title,"休息倒计时:");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);

    lv_obj_t *time = lv_label_create(cont);
    time_label = time;
    lv_label_set_recolor(time, true); 
    obj_font_set(time,FONT_TYPE_LETTER, FONT_SIZE_TIME_1);
    device_state_t* device_state = get_device_state();
    device_state->tomato_rest_remaining_time = device_state->tomato_rest_time;
    lv_label_set_text_fmt(time,"%s",get_time_str(device_state->tomato_rest_remaining_time));    
    lv_obj_set_style_text_color(time,APP_COLOR_WHITE,0);

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
    lv_label_set_text(btn_label,"开始学习");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

void init_page_tomato_rest(void)
{
    is_cancle = false;
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t *info_view =  init_info_view(cont);
    lv_obj_align_to(info_view,title_view,LV_ALIGN_OUT_BOTTOM_MID,500,20);

    lv_obj_t * btn = init_select_btn(cont);
    lv_obj_align_to(btn,info_view,LV_ALIGN_OUT_RIGHT_MID,100,18);

    if(refresh_timer == NULL)
        refresh_timer = lv_timer_create((void*)refresh_timer_cb_func, 1000, NULL);
    start_play_audio_async(GET_MUSIC_PATH("audio_finish.wav"));
}