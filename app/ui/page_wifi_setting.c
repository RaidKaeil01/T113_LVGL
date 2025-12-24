/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 22:34:38
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"

static lv_style_t com_style;
static lv_obj_t * wifi_ta = NULL;
static lv_obj_t * psw_ta = NULL;
static lv_obj_t *kb = NULL;

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

static void select_btn_click_event_cb(lv_event_t * e){
    // lv_event_code_t code = lv_event_get_code(e);
    // SETTING_PARAM_TYPE_E type  = (SETTING_PARAM_TYPE_E)lv_event_get_user_data(e);
    // device_state_t* device_state = get_device_state();
    // delete_current_page(&com_style);
    // init_page_linkage();
    printf("select_btn_click_event_cb wifi:%s psw:%s\n",lv_textarea_get_text(wifi_ta),lv_textarea_get_text(psw_ta));
    init_page_connect_loading(lv_textarea_get_text(wifi_ta),lv_textarea_get_text(psw_ta));
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if(kb != NULL) lv_keyboard_set_textarea(kb, ta);
    }

    else if(code == LV_EVENT_READY) {
        printf("ta_event_cb LV_EVENT_READY wifi:%s psw:%s\n",lv_textarea_get_text(wifi_ta),lv_textarea_get_text(psw_ta));
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
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_wifi.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"WiFi设置");
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

static lv_obj_t * init_wifi_setting_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_set_style_pad_top(cont,50,0);

    lv_obj_t *wifi_title = lv_label_create(cont);
    obj_font_set(wifi_title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(wifi_title,"WiFi账号:");
    lv_obj_set_style_text_color(wifi_title,APP_COLOR_WHITE,0);

    wifi_ta = lv_textarea_create(cont);
    lv_obj_set_size(wifi_ta,300,66);
    obj_font_set(wifi_ta,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_obj_align_to(wifi_ta,wifi_title,LV_ALIGN_OUT_RIGHT_MID,10,5);
    lv_obj_set_style_border_width(wifi_ta, 2,0);
    lv_obj_set_style_shadow_width(wifi_ta, 0,0);
    lv_obj_set_style_radius(wifi_ta,20,0);
    lv_obj_set_style_border_color(wifi_ta,APP_COLOR_WHITE,LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(wifi_ta,APP_COLOR_BUTTON_DEFALUT,LV_PART_MAIN);
    lv_obj_set_style_text_color(wifi_ta,APP_COLOR_WHITE,LV_PART_MAIN);
    lv_obj_set_style_pad_top(wifi_ta,10,0);
    lv_obj_set_style_pad_left(wifi_ta,10,0);
    lv_obj_set_style_pad_right(wifi_ta,10,0);
    lv_obj_set_style_bg_opa(wifi_ta,LV_OPA_20,0);
    lv_obj_set_style_bg_color(wifi_ta,lv_color_hex(0xD9D9D9),0);
    lv_obj_add_event_cb(wifi_ta, ta_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *wifi_psw = lv_label_create(cont);
    obj_font_set(wifi_psw,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(wifi_psw,"WiFi密码:");
    lv_obj_set_style_text_color(wifi_psw,APP_COLOR_WHITE,0);
    lv_obj_align_to(wifi_psw,wifi_title,LV_ALIGN_OUT_BOTTOM_MID,0,50);

    psw_ta = lv_textarea_create(cont);
    lv_obj_set_size(psw_ta,300,66);
    obj_font_set(psw_ta,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_obj_align_to(psw_ta,wifi_psw,LV_ALIGN_OUT_RIGHT_MID,10,5);
    lv_obj_set_style_border_width(psw_ta, 2,0);
    lv_obj_set_style_shadow_width(psw_ta, 0,0);
    lv_obj_set_style_radius(psw_ta,20,0);
    lv_obj_set_style_border_color(psw_ta,APP_COLOR_WHITE,LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(psw_ta,APP_COLOR_BUTTON_DEFALUT,LV_PART_MAIN);
    lv_obj_set_style_text_color(psw_ta,APP_COLOR_WHITE,LV_PART_MAIN);
    lv_obj_set_style_pad_top(psw_ta,10,0);
    lv_obj_set_style_pad_left(psw_ta,10,0);
    lv_obj_set_style_pad_right(psw_ta,10,0);
    lv_obj_set_style_bg_opa(psw_ta,LV_OPA_20,0);
    lv_obj_set_style_bg_color(psw_ta,lv_color_hex(0xD9D9D9),0);
    lv_obj_add_event_cb(psw_ta, ta_event_cb, LV_EVENT_ALL, NULL);


    lv_obj_t * btn = init_select_btn(cont);
    lv_obj_align_to(btn,wifi_psw,LV_ALIGN_OUT_BOTTOM_MID,200,35);

    return cont;
}

static lv_obj_t * init_keyboard_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_RIGHT);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    kb = lv_keyboard_create(cont);
    lv_obj_set_size(kb, 700, 280);
    lv_keyboard_set_textarea(kb, wifi_ta); /*Focus it on one of the text areas to start*/
    lv_obj_set_style_bg_opa(kb,LV_OPA_30,LV_PART_ITEMS);
    lv_obj_set_style_bg_color(kb,lv_color_hex(0xFFFFFF),LV_PART_ITEMS);
    lv_obj_set_style_text_color(kb,APP_COLOR_WHITE,LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(kb,LV_OPA_30,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(kb,lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(kb,APP_COLOR_WHITE,LV_PART_ITEMS| LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(kb,APP_COLOR_BLACK,0);
    lv_obj_set_style_bg_opa(kb,LV_OPA_0,0);

    return cont;
    
}

void init_page_wifi_setting()
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_3.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t *setting_view =  init_wifi_setting_view(cont);
    lv_obj_align_to(setting_view,title_view,LV_ALIGN_OUT_RIGHT_TOP,0,0);

    init_keyboard_view(cont);

    
}