/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 22:46:54
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"

static lv_style_t com_style;

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
    init_page_main();
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
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_about_us.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"关于智能助手屏&小智学长");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    return cont;
}

void init_page_about_us()
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view =  init_title_view(cont);
    
    lv_obj_t *qr_code = lv_img_create(cont);
    lv_img_set_src(qr_code,GET_IMAGE_PATH("icon_qrcode.png"));
    lv_obj_set_align(qr_code,LV_ALIGN_TOP_MID);
    lv_obj_set_style_pad_top(qr_code,50,LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"扫码了解更多课程");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,qr_code,LV_ALIGN_OUT_BOTTOM_MID,0,10);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_about_us.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);
    
}