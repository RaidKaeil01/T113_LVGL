/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:32:02 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:24:22
 */

#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"

static lv_style_t com_style;

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

static void restart_btn_click_event_cb(lv_event_t * e){
    printf("tomato learn restart_btn_click_event_cb\n");
    delete_current_page(&com_style);
    init_page_tomato_learn();
}

static void finish_btn_click_event_cb(lv_event_t * e){
    printf("tomato learn select_btn_click_event_cb\n");
    delete_current_page(&com_style);
    init_page_main();
}

static lv_obj_t * init_message_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(cont,30,0);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img,GET_IMAGE_PATH(""));

    lv_obj_t *msg = lv_label_create(cont);
    obj_font_set(msg,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(msg,"本轮学习完成啦！");
    lv_obj_set_style_text_color(msg,APP_COLOR_WHITE,0);

    return cont;
}

static lv_obj_t * init_restart_btn(lv_obj_t *parent){
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_style(btn,&com_style,0);
    lv_obj_set_size(btn,171,66);
    lv_obj_clear_state(btn,LV_STATE_FOCUS_KEY);
    lv_obj_set_style_border_width(btn, 0,0);
    lv_obj_set_style_shadow_width(btn, 0,0);
    lv_obj_set_style_radius(btn,35,0);
    lv_obj_set_style_bg_color(btn,APP_COLOR_BUTTON_DEFALUT,0);
    lv_obj_add_event_cb(btn,restart_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    lv_label_set_text(btn_label,"重新开始");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

static lv_obj_t * init_finish_btn(lv_obj_t *parent){
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_style(btn,&com_style,0);
    lv_obj_set_size(btn,200,66);
    lv_obj_clear_state(btn,LV_STATE_FOCUS_KEY);
    lv_obj_set_style_border_width(btn, 0,0);
    lv_obj_set_style_shadow_width(btn, 0,0);
    lv_obj_set_style_radius(btn,35,0);
    lv_obj_set_style_bg_color(btn,APP_COLOR_BUTTON_DEFALUT,0);
    lv_obj_add_event_cb(btn,finish_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    lv_label_set_text(btn_label,"今天就到这里吧");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

void init_page_tomato_finish(void)
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t * message_view = init_message_view(cont);

    lv_obj_t * btn_cont = lv_obj_create(cont);
    lv_obj_set_size(btn_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(btn_cont, &com_style, 0);
    lv_obj_align_to(btn_cont,message_view,LV_ALIGN_OUT_BOTTOM_MID,0,50);
    lv_obj_set_flex_flow(btn_cont,LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(btn_cont,30,0);
    lv_obj_set_style_bg_opa(btn_cont,LV_OPA_0,0);

    lv_obj_t * restart_btn = init_restart_btn(btn_cont);

    lv_obj_t * finish_btn = init_finish_btn(btn_cont);
}