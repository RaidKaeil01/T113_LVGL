/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:32:02 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 02:40:14
 */

#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "music_conf.h"
#include "em_hal_audio.h"
#include "device_data.h"

typedef struct{
    ALARM_TYPE_E type;
    char msg[50];
    char *img_url;
}alarm_info_t;

static alarm_info_t alarm_info_list[] = {
    {ALARM_ALARM_CLOCK_TRIGGER,"嘿,闹铃时间到啦",GET_IMAGE_PATH("icon_alarm.png")},
};

static lv_style_t com_style;
static lv_obj_t * page = NULL;

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

static void select_btn_click_event_cb(lv_event_t * e){
    lv_obj_t * act_scr = page;
    lv_disp_t * d = lv_obj_get_disp(act_scr);
	if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
	{
		lv_obj_del(act_scr);
        lv_style_reset(&com_style);
        page = NULL;
	}
}

static lv_obj_t * init_message_view(lv_obj_t *parent,ALARM_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(cont,30,0);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img,alarm_info_list[type].img_url);

    lv_obj_t *msg = lv_label_create(cont);
    obj_font_set(msg,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(msg,alarm_info_list[type].msg);
    lv_obj_set_style_text_color(msg,APP_COLOR_WHITE,0);

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
    lv_obj_add_event_cb(btn,select_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    lv_label_set_text(btn_label,"确定");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

void init_page_alarm_remind(ALARM_TYPE_E type)
{
    if(page != NULL)
        return;
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_layer_top());
    page = cont;
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t * message_view = init_message_view(cont,type);

    lv_obj_t * btn = init_select_btn(cont);
    lv_obj_align_to(btn,message_view,LV_ALIGN_OUT_BOTTOM_MID,0,50);
    device_state_t* device_state = get_device_state();
    if(type == ALARM_ALARM_CLOCK_TRIGGER){
        start_play_audio_async(GET_MUSIC_PATH("audio_finish2.wav"));
    }
}