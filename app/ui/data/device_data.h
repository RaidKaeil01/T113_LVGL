/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 18:24:33 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 01:14:51
 */
#ifndef _DEVICE_DATA_H_
#define _DEVICE_DATA_H_

#include <stdio.h>
#include "lvgl.h"
#include "wpa_manager.h"

#define WEATHER_CITY "guangzhou"
#define WEATHER_KEY "SmazqPcltzTft-X3v"

typedef enum{
    OFF = 0,
    ON,
}SWITCH_STATE_E;

typedef enum{
    DISCONNECT = 0,
    CONNECTING,
    CONNECT,
}CONNECT_STATE_E;

typedef enum{
    TIME_TYPE_1 = 0,
    TIME_TYPE_2,
    TIME_TYPE_3,
}TIME_SHOW_TYPE_E;

typedef enum{
    NOTICE_TYPE_ONLY_WIN = 0,
    NOTICE_TYPE_WIN_AND_AUDIO,
}NOTICE_TYPE_E;

typedef struct{

    CONNECT_STATE_E ble_mesh_state; //ble mesh模组连接状态
    CONNECT_STATE_E led_connect_state;
    CONNECT_STATE_E switch_connect_state;
    CONNECT_STATE_E body_sensor_connect_state;
    CONNECT_STATE_E smart_coaster_connect_state;
    CONNECT_STATE_E flame_sensor_connect_state;

    uint8_t rgbled_battery_level;
    uint8_t switch_battery_level;
    uint8_t body_sensor_battery_level;
    uint8_t smart_coaster_battery_level;
    uint8_t flame_sensor_battery_level;

    SWITCH_STATE_E led_r;
    SWITCH_STATE_E led_g;
    SWITCH_STATE_E led_b;

    SWITCH_STATE_E mesh_switch_state;

    SWITCH_STATE_E body_sensor_state;
    uint8_t body_sensor_data;
    NOTICE_TYPE_E body_sensor_notice_type;

    SWITCH_STATE_E smart_coaster_state;
    uint8_t smart_coaster_data;
    uint32_t smart_coaster_time; //单位：min
    uint32_t smart_coaster_remaining_time;

    SWITCH_STATE_E flame_sensor_state;
    uint8_t flame_sensor_data;
    uint32_t flame_sensor_time; //单位：min
    uint32_t flame_sensor_remaining_time;

    SWITCH_STATE_E tomato_time_state;
    uint32_t tomato_learn_time;
    uint32_t tomato_learn_remaining_time;

    uint32_t tomato_rest_time;
    uint32_t tomato_rest_remaining_time;

    char weather_info[50];
    char weather_city[20];

    TIME_SHOW_TYPE_E time_type;
    int clock_type;

    WPA_WIFI_STATUS_E wifi_state;  //wifi开启状态
    WPA_WIFI_CONNECT_STATUS_E wifi_connect_state; //wifi连接状态

    int brightness_value;
    int volume_value;

    bool is_open_type1;
    bool is_open_type2;
    int alarm_time1;
    int alarm_time2;

    bool is_disp_orientation;

}device_state_t;



void init_device_state(void);

device_state_t* get_device_state(void);

void device_timer_init();

void device_param_write(void);

void device_param_read(void);

void update_wpa_manager_callback_fun(void);

#endif
