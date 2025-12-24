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
#include "wpa_manager.h"
#include "http_manager.h"
#include "device_data.h"
#include "em_hal_time.h"

static lv_style_t com_style;

static lv_obj_t * page = NULL;

static lv_obj_t  * title_label = NULL;

static lv_obj_t *state_img = NULL;

static lv_obj_t  * btn_label = NULL;

static bool is_cancle = false;

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

static void anim_cb(void * var, int32_t v) {
    lv_img_set_angle(var,v);
}

static void select_btn_click_event_cb(lv_event_t * e){
    lv_obj_t * act_scr = page;
    lv_disp_t * d = lv_obj_get_disp(act_scr);
    if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
    {
        is_cancle = true;
        lv_obj_del(act_scr);
        lv_style_reset(&com_style);
        page = NULL;
    }
}

static void connect_status_callback_func(WPA_WIFI_CONNECT_STATUS_E status){
    printf("loading----->connect_status_callback_func: %d\n", status);
    if(is_cancle == true)
        return;
    if(status == WPA_WIFI_CONNECT){
        lv_label_set_text(btn_label,"确定");
        lv_label_set_text(title_label,"连接成功");
        lv_img_set_src(state_img,GET_IMAGE_PATH("icon_connect.png"));
        device_state_t* device_state = get_device_state();
        device_state->wifi_connect_state = WPA_WIFI_CONNECT;
        http_get_weather_async(WEATHER_KEY,device_state->weather_city);
        em_hal_time_ntpd_update();
        lv_anim_del_all();
        lv_img_set_angle(state_img,0);
    }else if(status == WPA_WIFI_WRONG_KEY){
        lv_label_set_text(btn_label,"确定");
        lv_label_set_text(title_label,"密码错误");
        lv_img_set_src(state_img,GET_IMAGE_PATH("icon_disconnect.png"));
        lv_anim_del_all();
        lv_img_set_angle(state_img,0);
    }
}

static void wifi_status_callback_func(WPA_WIFI_STATUS_E status){
    printf("----->wifi_status_callback_func: %d\n", status);
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

    btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    lv_label_set_text(btn_label,"取消");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

void init_page_connect_loading(const char *name,const char *psw)
{
    if(page != NULL)
        return;
    is_cancle = false;
    if(strlen(name) == 0){
        printf("wifi name null\n");
        return;
    }
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_layer_top());
    page = cont;
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    title_label = lv_label_create(cont);
    obj_font_set(title_label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title_label,"连接中");
    lv_obj_set_style_text_color(title_label,APP_COLOR_WHITE,0);
    lv_obj_set_align(title_label,LV_ALIGN_TOP_MID);
    lv_obj_set_style_pad_top(title_label,20,0);

    lv_obj_t  * info_label = lv_label_create(cont);
    obj_font_set(info_label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text_fmt(info_label,"WiFi:%s 密码:%s 注意:检查大小写是否正确 仅支持2.4G WiFi",name,psw);
    lv_obj_set_style_text_color(info_label,APP_COLOR_WHITE,0);
    lv_obj_align_to(info_label,title_label,LV_ALIGN_OUT_BOTTOM_MID,0,10);

    state_img = lv_img_create(cont);
    lv_img_set_src(state_img,GET_IMAGE_PATH("icon_loading1.png"));
    lv_obj_align_to(state_img,info_label,LV_ALIGN_OUT_BOTTOM_MID,0,20);

    lv_obj_t * btn = init_select_btn(cont);
    lv_obj_align_to(btn,state_img,LV_ALIGN_OUT_BOTTOM_MID,0,30);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, state_img);
    lv_anim_set_values(&anim, 0, 3600);
    lv_anim_set_time(&anim, 2000);
    lv_anim_set_exec_cb(&anim, anim_cb);
    lv_anim_set_repeat_count(&anim,LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&anim,lv_anim_path_ease_in_out);
    lv_anim_start(&anim);

    wpa_manager_add_callback(wifi_status_callback_func,connect_status_callback_func);

    wpa_ctrl_wifi_info_t wifi_info;
    if(strlen(name) > 32 || strlen(psw) > 32){
        printf("wifi str len error\n");
        return;
    }
    memset(wifi_info.ssid,'\0',sizeof(wifi_info.ssid));
    memset(wifi_info.psw,'\0',sizeof(wifi_info.psw));
    memcpy(wifi_info.ssid,name,strlen(name));
    memcpy(wifi_info.psw,psw,strlen(psw));
    wpa_manager_wifi_connect(&wifi_info);
    device_state_t* device_state = get_device_state();
    device_state->wifi_connect_state = WPA_WIFI_DISCONNECT;
}