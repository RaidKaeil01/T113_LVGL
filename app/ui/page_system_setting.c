/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-10-08 18:56:25
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "page_pulldown_view.h"
#include "em_hal_brightness.h"
#include "em_hal_audio.h"
#include "em_hal_system.h"
#include "music_conf.h"

static lv_style_t com_style;

static system_setting_info_t system_setting_info_list[] = {
    {SYSTEM_SETTING_BACKLIGHT,"亮度",GET_IMAGE_PATH("icon_brightness.png"),50,NULL},
    {SYSTEM_SETTING_VOLUME,"音量",GET_IMAGE_PATH("icon_volume.png"),30,NULL},
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

static void slider_release_cb(lv_event_t * e){
    lv_obj_t * slider = lv_event_get_target(e);
    char* menu_name  = (char *)lv_event_get_user_data(e);
    device_state_t* device_state = get_device_state();
    if(strstr(menu_name,"亮度") != NULL){
        em_hal_brightness_set_value(lv_slider_get_value(slider)+5);
        device_state->brightness_value  = lv_slider_get_value(slider);
    }else if(strstr(menu_name,"音量") != NULL){
        em_set_audio_vol(lv_slider_get_value(slider));
        start_play_audio_async(GET_MUSIC_PATH("audio_msg.wav"));
        device_state->volume_value  = lv_slider_get_value(slider);
    }
    device_param_write();
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    char* menu_name  = (char *)lv_event_get_user_data(e);
    device_state_t* device_state = get_device_state();
    if(strstr(menu_name,"亮度") != NULL){
        lv_label_set_text(system_setting_info_list[SYSTEM_SETTING_BACKLIGHT].label, buf);
        em_hal_brightness_set_value(lv_slider_get_value(slider)+5);
    }else if(strstr(menu_name,"音量") != NULL){
        lv_label_set_text(system_setting_info_list[SYSTEM_SETTING_VOLUME].label, buf);
    }
}

static void btn_click_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    device_state_t* device_state = get_device_state();
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * swbtn = lv_event_get_target(e);
        SWITCH_STATE_E state = OFF;
        if(lv_obj_has_state(swbtn, LV_STATE_CHECKED)){
            state = ON;
            printf("sw btn open\n");
            device_state->is_disp_orientation = true;
        }else{
            state = OFF;
            printf("sw btn close\n");
            device_state->is_disp_orientation = false;
        }
    }
    device_param_write();
}

static void select_btn_click_event_cb(lv_event_t * e){
    em_hal_reboot();
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
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_setting.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"系统设置");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    return cont;
}

static lv_obj_t *init_scroll_item_view(lv_obj_t *parent,SYSTEM_SETTING_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,30,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    lv_obj_t *icon = lv_img_create(cont);
    lv_img_set_src(icon,system_setting_info_list[type].img_url);
    lv_obj_set_align(icon,LV_ALIGN_LEFT_MID);

    lv_obj_t *title_label = lv_label_create(cont);
    obj_font_set(title_label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(title_label,system_setting_info_list[type].name);
    lv_obj_set_style_text_color(title_label,APP_COLOR_WHITE,0);
    lv_obj_set_style_pad_top(title_label,-10,LV_PART_MAIN);

    lv_obj_t * slider = lv_slider_create(cont);
    lv_slider_set_value(slider,system_setting_info_list[type].value,LV_ANIM_OFF);
    lv_slider_set_range(slider, 0, 100);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)system_setting_info_list[type].name);
    lv_obj_add_event_cb(slider,slider_release_cb,LV_EVENT_RELEASED, (void *)system_setting_info_list[type].name);

    system_setting_info_list[type].label = lv_label_create(cont);
    obj_font_set(system_setting_info_list[type].label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text_fmt(system_setting_info_list[type].label,"%d%%",system_setting_info_list[type].value);
    lv_obj_set_style_text_color(system_setting_info_list[type].label,APP_COLOR_WHITE,0);
    lv_obj_set_style_pad_top(system_setting_info_list[type].label,-10,LV_PART_MAIN);

    return cont;
}

static lv_obj_t * init_item_btn_view(lv_obj_t *parent,char *name_str,lv_state_t state){
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
    lv_obj_add_event_cb(sw, btn_click_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    lv_obj_add_state(sw,state);

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
    lv_label_set_text(btn_label,"点我重启");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

void init_page_system_setting()
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    device_state_t* device_state = get_device_state();
    system_setting_info_list[SYSTEM_SETTING_BACKLIGHT].value = device_state->brightness_value;
    system_setting_info_list[SYSTEM_SETTING_VOLUME].value = device_state->volume_value;

    // lv_obj_t *bg_img = lv_img_create(cont);
    // lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_1.png"));
    // lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t* backlight_view = init_scroll_item_view(cont,SYSTEM_SETTING_BACKLIGHT);
    lv_obj_align(backlight_view,LV_ALIGN_TOP_MID,0,50);
    
    lv_obj_t* volume_view = init_scroll_item_view(cont,SYSTEM_SETTING_VOLUME);
    lv_obj_align_to(volume_view,backlight_view,LV_ALIGN_OUT_BOTTOM_MID,0,40);

    lv_obj_t* orientation_btn = init_item_btn_view(cont,"界面旋转(设置后重启生效)",device_state->is_disp_orientation);
    lv_obj_align_to(orientation_btn,volume_view,LV_ALIGN_OUT_BOTTOM_MID,0,40);

    lv_obj_t * btn = init_select_btn(cont);
    lv_obj_align_to(btn,orientation_btn,LV_ALIGN_OUT_RIGHT_MID,100,0);
}