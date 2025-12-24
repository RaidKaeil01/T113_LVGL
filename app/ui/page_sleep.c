/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 01:46:05
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "lv_clock.h"
#include "http_manager.h"

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

static lv_style_t com_style;

static lv_obj_t * show_view = NULL;

static lv_obj_t * time_label = NULL;

static lv_obj_t * weather_label = NULL;

static TIME_SHOW_TYPE_E page_type = TIME_TYPE_1;
static int clock_type = 0;

static lv_obj_t * page = NULL;

static time_t timep;
static struct tm time_temp;
static lv_timer_t * refresh_timer = NULL;
static char time_str[20];

static lv_clock_t lv_clock0;
static lv_clock_t lv_clock1;
static lv_clock_t lv_clock2;

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

static bool get_system_time(){
    time(&timep);
    memcpy(&time_temp, localtime(&timep), sizeof(struct tm));
    // printf("h=%d m=%d s=%d\n",time_temp.tm_hour,time_temp.tm_min,time_temp.tm_sec);
    if(time_temp.tm_hour < 10){
        if(time_temp.tm_min < 10){
            sprintf(time_str,"0%d:0%d",time_temp.tm_hour,time_temp.tm_min);
        }else{
            sprintf(time_str,"0%d:%d",time_temp.tm_hour,time_temp.tm_min);
        }
    }else{
        if(time_temp.tm_min < 10){
            sprintf(time_str,"%d:0%d",time_temp.tm_hour,time_temp.tm_min);
        }else{
            sprintf(time_str,"%d:%d",time_temp.tm_hour,time_temp.tm_min);
        }
    }
    if(time_temp.tm_hour == 0 && time_temp.tm_min == 0 && time_temp.tm_sec == 1){
        return true;
    }
    return false;
}

static void refresh(lv_event_t* event){
    // printf("sleep refresh");
    if(page_type == TIME_TYPE_1 || page_type == TIME_TYPE_2){
        if(get_system_time()){
            device_state_t* device_state = get_device_state();
            http_get_weather_async(WEATHER_KEY,device_state->weather_city);
        }
        lv_label_set_text(time_label,time_str);
        if(page_type == TIME_TYPE_1){
            lv_label_set_text(weather_label,get_device_state()->weather_info);
        }
    }else if(page_type == TIME_TYPE_3){
        //nothing to do , time update in lv_clock
    }
}

static void init_timer(){
    if(refresh_timer == NULL)
        refresh_timer = lv_timer_create((void*)refresh, 1000, NULL);
}

static void deinit_timer(){
    if(refresh_timer != NULL)
        lv_timer_del(refresh_timer);
    refresh_timer = NULL;
}

static void init_item_view(lv_obj_t *parent,TIME_SHOW_TYPE_E type){
    if(type == TIME_TYPE_1){
        lv_obj_t *time_obj = lv_label_create(parent);
        time_label = time_obj;
        obj_font_set(time_obj,FONT_TYPE_LETTER, FONT_SIZE_MAX);
        lv_label_set_text(time_label,time_str);
        lv_obj_set_style_text_color(time_obj,APP_COLOR_WHITE,0);
        lv_obj_align(time_obj,LV_ALIGN_TOP_MID,0,50);

        lv_obj_t *weather_obj = lv_label_create(parent);
        weather_label = weather_obj;
        obj_font_set(weather_obj,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
        lv_label_set_text(weather_obj,get_device_state()->weather_info);
        lv_obj_set_style_text_color(weather_obj,APP_COLOR_WHITE,0);
        lv_obj_align_to(weather_obj,time_obj,LV_ALIGN_OUT_BOTTOM_MID,0,10);
    }else if(type == TIME_TYPE_2){
        lv_obj_t *time_obj = lv_label_create(parent);
        time_label = time_obj;
        obj_font_set(time_obj,FONT_TYPE_NUMBER, FONT_SIZE_TIME);
        lv_label_set_text(time_label,time_str);
        lv_obj_set_style_text_color(time_obj,APP_COLOR_WHITE,0);
        lv_obj_align(time_obj,LV_ALIGN_CENTER,0,-15);
    }else if(type == TIME_TYPE_3){
        // lv_obj_t * cont = lv_obj_create(parent);
        // lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        // lv_obj_center(cont);
        lv_obj_t *clock = NULL;
        if(clock_type == 0){
            clock = init_clock0_obj(parent,&lv_clock0);
        }else if(clock_type == 1){
            clock = init_clock1_obj(parent,&lv_clock1);
        }else{
            clock = init_clock2_obj(parent,&lv_clock2);
        }
        lv_obj_align(clock,LV_ALIGN_CENTER,0,0);
    }
}

static void stop_sleep_btn_click_event_cb(lv_event_t * e){
    deinit_timer();
    lv_obj_t * act_scr = page;
    lv_disp_t * d = lv_obj_get_disp(act_scr);
    if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
    {
        lv_obj_del(act_scr);
        lv_style_reset(&com_style);
        page = NULL;
    }
    deinit_clock0_obj(&lv_clock0);
    deinit_clock1_obj(&lv_clock1);
    deinit_clock2_obj(&lv_clock2);
}

static lv_obj_t * init_show_view(lv_obj_t *parent,TIME_SHOW_TYPE_E type){
    init_item_view(parent,type);
}

void init_page_sleep()
{
    if(page != NULL)
        return;
    page_type = get_device_state()->time_type;
    clock_type = get_device_state()->clock_type;
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_layer_top());
    page = cont;
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont,stop_sleep_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    get_system_time();

    if(page_type == TIME_TYPE_2){
        lv_obj_t* gif_obj = lv_gif_create(cont);
        lv_gif_set_src(gif_obj, GET_IMAGE_PATH("earth02.gif"));
        lv_obj_set_align(gif_obj,LV_ALIGN_LEFT_MID);
    }
    
    init_show_view(cont,page_type);
    init_timer();
}