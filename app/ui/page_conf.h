/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:06 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:24:44
 */
#ifndef _PAGE_CONF_H_
#define _PAGE_CONF_H_

#include "lvgl.h"
#include "music_conf.h"
#include "em_hal_audio.h"
#include "audio_player_async.h"

//setting page param: body_sensor, smart_coaster, flame alert.
typedef enum{   
    TYPE_BODY_SENSOR = 0,
    TYPE_SMART_COASTER,
    TYPE_FLAME_SENSOR,
}SETTING_PARAM_TYPE_E;

typedef enum{
    WARN_BODY_SENSOR_TRIGGER = 0,
    WARN_SMART_COASTER_TRIGGER,
    WARN_FLAME_SENSOR_TRIGGER,
}WRANING_TYPE_E;

typedef enum{
    ALARM_ALARM_CLOCK_TRIGGER = 0,
}ALARM_TYPE_E;

void delete_current_page(lv_style_t *style);

void init_page_main(void);
void init_page_linkage(void);
void init_page_linkage_setting(SETTING_PARAM_TYPE_E type);
void init_page_warn(WRANING_TYPE_E type);
void init_page_wifi_setting(void);
void init_page_city_setting(void);
void init_page_time_setting(void);
void init_page_tomatotime_setting(void);
void init_page_tomato_learn(void);
void init_page_tomato_rest(void);
void init_page_tomato_finish(void);
void init_page_system_setting(void);
void init_page_about_us(void);
void init_page_pulldowm_view(lv_obj_t *view);
void deinit_page_pulldowm_view(void);
void init_page_sleep(void);
bool get_is_pull_down(void);
void init_page_connect_loading(const char *name,const char *psw);
void init_page_alarm_clock_setting(void);
void init_page_alarm_remind(ALARM_TYPE_E type);
void init_page_shortcutkey(void);
void init_page_language_game(void);
void init_page_simulate_iphone(void);

#endif
