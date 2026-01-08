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
 * @note 用于传递结构化天气信息给UI层
 */
typedef struct {
    char city[32];        // 城市名称，如"重庆"
    char weather[32];     // 天气状态，如"阴"
    char temperature[16]; // 温度数值，如"9"
    char code[8];         // 天气代码，如"9"（用于图标映射）
    char update_time[32]; // 更新时间，如"11:47:24"
    char date[16];        // 日期，如"2026-01-07"
    int weekday;          // 星期（0=周日, 1=周一, ..., 6=周六）
} weather_data_t;

typedef void (* weather_callback_fun)(weather_data_t *data);

/**
 * @brief 初始化HTTP请求模块
 * @return 0-成功，-1-失败
 * @note 创建网络请求线程和消息队列
 */
int http_request_create(void);

/**
 * @brief 异步获取天气数据
 * @param key API密钥
 * @param city 城市名称
 * @note 如果网络未连接，请求将被加入待处理队列，网络恢复后自动重试
 */
void http_get_weather_async(char *key,char *city);

/**
 * @brief 注册天气数据回调函数
 * @param func 回调函数指针
 */
void http_set_weather_callback(weather_callback_fun func);

/**
 * @brief 启动网络状态监听定时器
 * @note 用于检测网络恢复并自动重试待处理请求
 *       在main.c的LVGL主循环前调用
 */
void http_start_network_monitor(void);

#endif