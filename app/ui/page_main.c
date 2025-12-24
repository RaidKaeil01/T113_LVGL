/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:17 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:39:11
 */

#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "http_manager.h"
#include "lv_clock.h"
#include "em_hal_time.h"

#include <time.h>
#include <sys/time.h>

static lv_obj_t * time_label = NULL;
static lv_obj_t * weather_label = NULL;
static lv_obj_t *wifi_img = NULL;
static lv_obj_t *ble_img = NULL;
static lv_obj_t *vol_img = NULL;
static lv_obj_t *alarm_img = NULL;
static int sleep_time_count = 0; //60S Sleep

static time_t timep;
static struct tm time_temp;
static lv_timer_t * refresh_timer = NULL;
static char time_str[20];

static TIME_SHOW_TYPE_E page_type = TIME_TYPE_1;
static int clock_type = 0;

static lv_clock_t lv_clock0;
static lv_clock_t lv_clock1;
static lv_clock_t lv_clock2;

typedef enum{
    MENU_LINKAGE = 0,
    MENU_TOMATO_TIME,
    MENU_CLOCK,
    MENU_TIME,
    MENU_CITY,
    MENU_WIFI,
    MENU_SETTING,
    MENU_SHORTCUT_KEY,
    MENU_LANGUAGE,
    MENU_APPLE,
    MENU_ABOUT,
}MENU_TYPE_E;

typedef struct{
    MENU_TYPE_E type;
    char name[50];
    char *img_url;
}menu_info_t;

static menu_info_t menu_info_list[] = {
    {MENU_TOMATO_TIME,"番茄时钟",GET_IMAGE_PATH("icon_menu_tomato_time.png")},
    {MENU_CLOCK,"闹钟设置",GET_IMAGE_PATH("icon_menu_time.png")},
    {MENU_TIME,"表盘设置",GET_IMAGE_PATH("icon_menu_dial.png")},
    {MENU_SHORTCUT_KEY,"快捷键",GET_IMAGE_PATH("icon_stm32cubemx.png")},
    {MENU_CITY,"城市设置",GET_IMAGE_PATH("icon_menu_city.png")},
    {MENU_LANGUAGE,"小游戏",GET_IMAGE_PATH("icon_language.png")},
    {MENU_WIFI,"WiFi设置",GET_IMAGE_PATH("icon_menu_wifi.png")},
    {MENU_SETTING,"系统设置",GET_IMAGE_PATH("icon_menu_setting.png")},
    {MENU_APPLE,"手机模拟",GET_IMAGE_PATH("icon_menu_apple.png")},
    {MENU_LINKAGE,"场景联动",GET_IMAGE_PATH("icon_menu_linkage.png")},
    {MENU_ABOUT,"关于",GET_IMAGE_PATH("icon_menu_about.png")},
};
    
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
    device_state_t* device_state = get_device_state();
    if(page_type == TIME_TYPE_1 || page_type == TIME_TYPE_2){
        if(get_system_time()){
            http_get_weather_async(WEATHER_KEY,device_state->weather_city);
        }
        lv_label_set_text(time_label,time_str);
        if(page_type == TIME_TYPE_1){
            lv_label_set_text(weather_label,get_device_state()->weather_info);
            lv_obj_align_to(weather_label, time_label,LV_ALIGN_OUT_BOTTOM_MID,0,10);
        }
    }else if(page_type == TIME_TYPE_3){
        //nothing to do , time update in lv_clock
    }
    sleep_time_count ++;
    if(sleep_time_count >= 60){
        sleep_time_count = 0;
        init_page_sleep();
    }
    static bool is_first_init = true;
    if(device_state->wifi_connect_state == WPA_WIFI_CONNECT && is_first_init){
        http_get_weather_async(WEATHER_KEY,device_state->weather_city);
        em_hal_time_ntpd_update();
        is_first_init = false;
    }
    //TODO check wifi state
    if(device_state->wifi_connect_state == WPA_WIFI_CONNECT){
        lv_img_set_src(wifi_img,GET_IMAGE_PATH("icon_wifi_connect.png"));
    }else{
        lv_img_set_src(wifi_img,GET_IMAGE_PATH("icon_wifi_disconnect.png"));
    }
    if(device_state->ble_mesh_state == CONNECT){
        lv_img_set_src(ble_img,GET_IMAGE_PATH("icon_ble_connect.png"));
    }else{
        lv_img_set_src(ble_img,GET_IMAGE_PATH("icon_ble_disconnect.png"));
    }
    if(device_state->volume_value == 0){
        lv_img_set_src(vol_img,GET_IMAGE_PATH("icon_volx.png"));
    }else{
        lv_img_set_src(vol_img,GET_IMAGE_PATH("icon_vol.png"));
    }
    if(device_state->is_open_type1 == true || device_state->is_open_type2 == true){
        lv_img_set_src(alarm_img,GET_IMAGE_PATH("icon_alarm_state.png"));
    }else{
        lv_img_set_src(alarm_img,GET_IMAGE_PATH(""));
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

static void time_sync_click_event_cb(lv_event_t * e){
    printf("time_sync_click_event_cb\n");
    device_state_t* device_state = get_device_state();
    if(device_state->wifi_connect_state == WPA_WIFI_CONNECT){
        em_hal_time_ntpd_update();
        http_get_weather_async(WEATHER_KEY,device_state->weather_city);
    }
}

static void screen_click_event_cb(lv_event_t * e){
    printf("screen_click_event_cb\n");
    sleep_time_count = 0;
}

static void menu_click_event_cb(lv_event_t * e){
    if(get_is_pull_down())
        return;
    char* menu_name  = (char *)lv_event_get_user_data(e);
    printf("--->select menu = %s\n",menu_name);
    printf("----->%d",strcmp(menu_name,"场景联动\0"));
    deinit_page_pulldowm_view();
    deinit_timer();
    deinit_clock0_obj(&lv_clock0);
    deinit_clock1_obj(&lv_clock2);
    deinit_clock2_obj(&lv_clock1);
    if(strcmp(menu_name,"场景联动\0") == 0){
        delete_current_page(&com_style);
        init_page_linkage();
    }else if(strcmp(menu_name,"番茄时钟\0") == 0){
        delete_current_page(&com_style);
        init_page_tomatotime_setting();
    }else if(strcmp(menu_name,"闹钟设置\0") == 0){
        delete_current_page(&com_style);
        init_page_alarm_clock_setting();
    }else if(strcmp(menu_name,"表盘设置\0") == 0){
        delete_current_page(&com_style);
        init_page_time_setting();
    }else if(strcmp(menu_name,"城市设置\0") == 0){
        delete_current_page(&com_style);
        init_page_city_setting();
    }else if(strcmp(menu_name,"WiFi设置\0") == 0){
        delete_current_page(&com_style);
        init_page_wifi_setting();
    }else if(strcmp(menu_name,"系统设置\0") == 0){
        delete_current_page(&com_style);
        init_page_system_setting();
    }else if(strcmp(menu_name,"快捷键\0") == 0){
        delete_current_page(&com_style);
        init_page_shortcutkey();
    }else if(strcmp(menu_name,"小游戏\0") == 0){
        delete_current_page(&com_style);
        init_page_language_game();
    }else if(strcmp(menu_name,"手机模拟\0") == 0){
        delete_current_page(&com_style);
        init_page_simulate_iphone();
    }else if(strcmp(menu_name,"关于\0") == 0){
        delete_current_page(&com_style);
        init_page_about_us();
    }
}

static lv_obj_t * init_user_avatar(lv_obj_t *parent){
    lv_obj_t *user_img = lv_img_create(parent);
    lv_img_set_src(user_img,GET_IMAGE_PATH("icon_user1.png"));
    lv_obj_set_align(user_img,LV_ALIGN_LEFT_MID);
    lv_obj_set_style_pad_left(user_img,35,LV_PART_MAIN);
    return user_img;
}

static void init_item_view(lv_obj_t *parent,TIME_SHOW_TYPE_E type){
    if(type == TIME_TYPE_1){
        lv_obj_t *time_obj = lv_label_create(parent);
        time_label = time_obj;
        obj_font_set(time_obj,FONT_TYPE_LETTER, FONT_SIZE_TIME_1);
        lv_label_set_text(time_label,time_str);
        lv_obj_set_style_text_color(time_obj,APP_COLOR_WHITE,0);
        lv_obj_align(time_obj,LV_ALIGN_TOP_MID,0,60);

        lv_obj_t *weather_obj = lv_label_create(parent);
        weather_label = weather_obj;
        obj_font_set(weather_obj,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
        device_state_t* device_state = get_device_state();
        lv_label_set_text(weather_obj,device_state->weather_info);
        lv_obj_set_style_text_color(weather_obj,APP_COLOR_WHITE,0);
        lv_obj_align_to(weather_obj,time_obj,LV_ALIGN_OUT_BOTTOM_MID,0,10);
    }else if(type == TIME_TYPE_2){
        lv_obj_t *time_obj = lv_label_create(parent);
        time_label = time_obj;
        obj_font_set(time_obj,FONT_TYPE_NUMBER, FONT_SIZE_MAX);
        lv_label_set_text(time_label,time_str);
        lv_obj_set_style_text_color(time_obj,APP_COLOR_WHITE,0);
        lv_obj_align(time_obj,LV_ALIGN_CENTER,0,-10);
    }else if(type == TIME_TYPE_3){
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

static lv_obj_t * init_info_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 240, LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont,time_sync_click_event_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(cont,screen_click_event_cb,LV_EVENT_PRESSED,NULL);
    lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);

    init_item_view(cont,page_type);
    // lv_obj_t *time_obj = lv_label_create(cont);
    // time_label = time_obj;
    // obj_font_set(time_obj,FONT_TYPE_LETTER, FONT_SIZE_TIME_1);
    // lv_label_set_text(time_obj,"10:20");
    // lv_obj_set_style_text_color(time_obj,APP_COLOR_WHITE,0);
    // lv_obj_align(time_obj,LV_ALIGN_TOP_MID,0,50);

    // lv_obj_t *weather_obj = lv_label_create(cont);
    // weather_label = weather_obj;
    // obj_font_set(weather_obj,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    // lv_label_set_text(weather_obj,get_device_state()->weather_info);
    // lv_obj_set_style_text_color(weather_obj,APP_COLOR_WHITE,0);
    // lv_obj_align_to(weather_obj,time_obj,LV_ALIGN_OUT_BOTTOM_MID,0,30);
    return cont;
}

static lv_obj_t * init_item(lv_obj_t *parent,char *imgurl ,char *str){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_pad_top(cont,60,LV_PART_MAIN);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont,screen_click_event_cb,LV_EVENT_PRESSED,NULL);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img,imgurl);
    // printf("imgurl = %s\n",imgurl);

    lv_obj_t *label = lv_label_create(cont);
    obj_font_set(label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(label,str);
    lv_obj_set_style_text_color(label,APP_COLOR_WHITE,0);
    lv_obj_align_to(label,img,LV_ALIGN_OUT_BOTTOM_MID,0,10);

    lv_obj_add_event_cb(cont,menu_click_event_cb,LV_EVENT_CLICKED,lv_label_get_text(label));
}

static lv_obj_t * init_menu_list(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 900, LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(cont,30,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_pad_left(cont,40,0);
    lv_obj_set_style_pad_right(cont,40,0);
    lv_obj_add_event_cb(cont,screen_click_event_cb,LV_EVENT_PRESSED,NULL);

    int length = sizeof(menu_info_list) / sizeof(menu_info_t);
    for(int index = 0;index < length;index ++){
        init_item(cont,menu_info_list[index].img_url,menu_info_list[index].name);
    }

    // init_item(cont,GET_IMAGE_PATH("icon_menu_linkage.png"),"场景联动");
    // init_item(cont,GET_IMAGE_PATH("icon_menu_wifi.png"),"WiFi设置");
    // init_item(cont,GET_IMAGE_PATH("icon_menu_time.png"),"时间设置");
    // init_item(cont,GET_IMAGE_PATH("icon_menu_tomato_time.png"),"番茄时钟");
    // init_item(cont,GET_IMAGE_PATH("icon_menu_city.png"),"城市设置");
    // init_item(cont,GET_IMAGE_PATH("icon_menu_setting.png"),"系统设置");
    // init_item(cont,GET_IMAGE_PATH("icon_menu_about.png"),"关于");

    return cont;
}

static lv_obj_t * screen = NULL;
static lv_obj_t * scroll_view = NULL;

static lv_obj_t *init_scroll_view(){
    scroll_view = lv_obj_create(lv_scr_act()); 
    lv_obj_set_size(scroll_view, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(scroll_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scroll_view, APP_COLOR_BLACK, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_add_style(scroll_view,&com_style,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(scroll_view, LV_OPA_90 , 0);
    lv_obj_set_pos(scroll_view,0,-280);

    init_page_pulldowm_view(scroll_view);
}

static void init_status_icon(lv_obj_t *parent){
    wifi_img = lv_img_create(parent);
    if(get_device_state()->wifi_connect_state == WPA_WIFI_CONNECT){
        lv_img_set_src(wifi_img,GET_IMAGE_PATH("icon_wifi_connect.png"));
    }else{
        lv_img_set_src(wifi_img,GET_IMAGE_PATH("icon_wifi_disconnect.png"));
    }
    lv_obj_set_align(wifi_img,LV_ALIGN_TOP_RIGHT);
    lv_obj_set_style_pad_right(wifi_img,40,LV_PART_MAIN);
    lv_obj_set_style_pad_top(wifi_img,10,LV_PART_MAIN);

    ble_img = lv_img_create(parent);
    lv_img_set_src(ble_img,GET_IMAGE_PATH("icon_ble_disconnect.png"));
    lv_obj_align_to(ble_img,wifi_img,LV_ALIGN_OUT_LEFT_BOTTOM,-10,0);

    vol_img = lv_img_create(parent);
    if(get_device_state()->volume_value == 0){
        lv_img_set_src(vol_img,GET_IMAGE_PATH("icon_volx.png"));
    }else{
        lv_img_set_src(vol_img,GET_IMAGE_PATH("icon_vol.png"));
    }
    lv_obj_align_to(vol_img,ble_img,LV_ALIGN_OUT_LEFT_BOTTOM,-10,0);

    alarm_img = lv_img_create(parent);
    if(get_device_state()->is_open_type1 == true || get_device_state()->is_open_type2 == true){
        lv_img_set_src(alarm_img,GET_IMAGE_PATH("icon_alarm_state.png"));
    }else{
        lv_img_set_src(alarm_img,GET_IMAGE_PATH(""));
    }
    lv_obj_align_to(alarm_img,vol_img,LV_ALIGN_OUT_LEFT_BOTTOM,-10,0);

}

void init_page_main(void)
{
    update_wpa_manager_callback_fun();
    page_type = get_device_state()->time_type;
    clock_type = get_device_state()->clock_type;
    get_system_time();

    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont,screen_click_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t * user_avatar_obj = NULL;
    if(page_type == TIME_TYPE_2){
        lv_obj_t* gif_obj = lv_gif_create(cont);
        lv_gif_set_src(gif_obj, GET_IMAGE_PATH("earth02.gif"));
        lv_obj_set_align(gif_obj,LV_ALIGN_LEFT_MID);
        lv_obj_align(gif_obj,LV_ALIGN_LEFT_MID,-30,20);
        user_avatar_obj = gif_obj;
    }else{
        user_avatar_obj = init_user_avatar(cont);
    }

    init_scroll_view();

    lv_obj_t * bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("main_bg.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);    

    lv_obj_t * info_obj = init_info_view(cont);
    lv_obj_align_to(info_obj,user_avatar_obj,LV_ALIGN_OUT_RIGHT_MID,40,0);

    lv_obj_t * menu_obj = init_menu_list(cont);
    lv_obj_align_to(menu_obj,info_obj,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t * left_bg_img = lv_img_create(cont);
    lv_img_set_src(left_bg_img,GET_IMAGE_PATH("main_menu_bg_left.png"));
    lv_obj_align_to(left_bg_img,info_obj,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t * right_bg_img = lv_img_create(cont);
    lv_img_set_src(right_bg_img,GET_IMAGE_PATH("main_menu_bg_right.png"));
    lv_obj_set_align(right_bg_img,LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_pad_right(right_bg_img,0,LV_PART_MAIN);

    init_status_icon(cont);

    init_timer();
}