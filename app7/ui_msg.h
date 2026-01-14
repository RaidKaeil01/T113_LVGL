/**
 * @file ui_msg.h
 * @brief UI消息队列定义 - 线程安全的UI更新机制
 * @note 网络线程、WiFi线程通过消息队列通知主线程更新UI
 *       所有LVGL操作只在主线程执行，保证线程安全
 */

#ifndef _UI_MSG_H_
#define _UI_MSG_H_

#include <stdbool.h>

/* ========== UI消息类型枚举 ========== */
typedef enum {
    UI_MSG_NONE = 0,
    
    // 天气相关消息
    UI_MSG_WEATHER_OK,          // 天气数据获取成功
    UI_MSG_WEATHER_FAIL,        // 天气数据获取失败
    
    // WiFi相关消息
    UI_MSG_WIFI_CONNECTED,      // WiFi连接成功
    UI_MSG_WIFI_DISCONNECTED,   // WiFi断开连接
    UI_MSG_WIFI_WRONG_KEY,      // WiFi密码错误
    UI_MSG_WIFI_SCANNING,       // WiFi扫描中
    
    // 系统消息
    UI_MSG_NETWORK_ERROR,       // 网络错误（DNS解析失败等）
    
} ui_msg_type_t;

/* ========== 天气数据结构（消息携带） ========== */
typedef struct {
    char city[32];           // 城市名称
    char weather[32];        // 天气状态（晴/阴/雨/雪）
    char temperature[16];    // 温度（如"25"）
    char code[8];            // 天气代码（用于匹配图标）
    char update_time[32];    // 更新时间 "HH:MM:SS"
    char date[16];           // 日期 "2026-01-08"
    int weekday;             // 星期 (0=周日, 1=周一, ..., 6=周六)
} ui_weather_data_t;

/* ========== UI消息结构体 ========== */
typedef struct {
    ui_msg_type_t type;          // 消息类型
    union {
        ui_weather_data_t weather;   // 天气数据
        int error_code;              // 错误码
    } data;
} ui_msg_t;

/* ========== API函数声明 ========== */

/**
 * @brief 初始化UI消息队列
 * @return 0-成功，-1-失败
 */
int ui_msg_init(void);

/**
 * @brief 发送UI消息（供网络线程/WiFi线程调用）
 * @param msg 消息指针
 * @return 0-成功，-1-失败
 */
int ui_msg_send(ui_msg_t *msg);

/**
 * @brief 接收UI消息（供主线程调用，非阻塞）
 * @param msg 消息指针（输出）
 * @return 0-成功获取消息，-1-队列为空
 */
int ui_msg_recv(ui_msg_t *msg);

/**
 * @brief 处理UI消息（在主线程中调用）
 * @param msg 消息指针
 */
void ui_msg_handle(ui_msg_t *msg);

#endif /* _UI_MSG_H_ */
