#ifndef _HTTP_MANAGER_H
#define _HTTP_MANAGER_H

typedef enum
{
    NET_GET_WEATHER = 0,
    NET_GET_TIME,
}NET_COMM_ID;

typedef struct
{
    NET_COMM_ID id;
    char host[100];
    char path[100];
    char data[50];
    char type[10];
    int loop_flag;
}net_obj;

typedef struct
{
    char *data;
    size_t size;
} http_resp_data_t;

/**
 * @brief 天气数据结构体
 */
typedef struct {
    char city[32];           // 城市名称
    char weather[32];        // 天气状态（晴/阴/雨/雪）
    char temperature[16];    // 温度（如"25"）
    char code[8];            // 天气代码（用于匹配图标）
    char update_time[32];    // 更新时间 "2026-01-08T11:47:24+08:00"
    char date[16];           // 日期 "2026-01-08"
    int weekday;             // 星期 (0=周日, 1=周一, ..., 6=周六)
} weather_data_t;

typedef void (* weather_callback_fun)(weather_data_t* data);

int http_request_create(void);

void http_get_weather_async(char *key,char *city);

void http_set_weather_callback(weather_callback_fun func);

#endif