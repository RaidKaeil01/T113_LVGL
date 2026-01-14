/*
 * @Author: xiaozhi
 * @Date: 2024-09-30 00:21:03
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-10-08 23:45:16
 */

#include <stdio.h>
#include <stdlib.h>
#include "cJSON/cJSON.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "lvgl.h"  // 用于定时器

#include "http_manager.h"
#include "osal_thread.h"
#include "osal_queue.h"

#ifndef __linux__
#include "wifi/wpa_manager.h"  // T113环境需要WiFi检测
#endif

/* ========== 全局变量 ========== */
static osal_queue_t net_queue = NULL;
static osal_thread_t net_thread = NULL;
static weather_callback_fun weather_callback_func = NULL;

/* ========== 网络监控相关变量 ========== */
static lv_timer_t * network_monitor_timer = NULL;  // 网络状态监听定时器

// 待处理请求队列结构体
typedef struct {
    bool pending;           // 是否有待处理请求
    char api_key[64];       // API密钥
    char city[64];          // 城市名称
    NET_COMM_ID request_type;  // 请求类型
} pending_request_t;

static pending_request_t g_pending_request = {
    .pending = false,
    .api_key = {0},
    .city = {0},
    .request_type = NET_GET_WEATHER
};

/**
 * @brief 组装HTTP请求URL
 */
static int assemble_url(const char *host, const char *path, char **url)
{
    *url = malloc(strlen(host) + strlen(path) + 1);
    strcpy(*url, host);
    strcat(*url, path);
    return 0;
}

/**
 * @brief CURL数据接收回调函数
 */
static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    http_resp_data_t *mem = (http_resp_data_t *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) return 0; // 内存分配失败

    mem->data = ptr;
    memcpy(mem->data + mem->size, data, realsize);
    mem->size += realsize;
    mem->data[mem->size] = '\0';
    return realsize;
}

int http_request_method(const char *host, const char *path,  const char *method, const char *request_json, char **response_json)
{
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    // 组装并设置URL
    char *url = NULL;
    assemble_url(host, path, &url);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // 通用配置
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);           // 调试模式：启用详细输出模式
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);          // 设置请求超时时间（单位：秒），20L表示超过20秒无响应则终止请求
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);    // 禁用SSL证书验证（0L表示关闭），跳过对服务器SSL证书的有效性检查
    // 设置响应处理
    http_resp_data_t response_data = {0};
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);  // 注册响应数据接收回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);      // 指定回调函数的用户数据

    // POST方法特殊处理
    if (strcmp(method, "POST") == 0)
    {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_json);
    }
    // 设置HTTP头部
    struct curl_slist *header = curl_slist_append(NULL, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    // 执行请求
    CURLcode code = curl_easy_perform(curl);
    int ret = (code == CURLE_OK) ? 0 : -1;
    // 处理响应
    if (ret == 0)
    {
        printf("Response len: %ld, data: %s\n", response_data.size, response_data.data);
        *response_json = response_data.data; // 转移内存所有权
    }
    else
    {
        printf("Request failed: %s (%d)\n", curl_easy_strerror(code), code);
        free(response_data.data); // 失败时释放内存
    }
    // 资源清理
    curl_slist_free_all(header);
    free(url);
    curl_easy_cleanup(curl);
    return ret;
}

void parseWeatherData(const char *json_data) {
    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        fprintf(stderr, "Error parsing JSON data.\n");
        return;
    }
    // 获取 results 数组
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (!results || !cJSON_IsArray(results)) {
        fprintf(stderr, "Invalid JSON format: missing 'results' array.\n");
        cJSON_Delete(root);
        return;
    }
    int num_results = cJSON_GetArraySize(results);
    if (num_results <= 0) {
        fprintf(stderr, "No results found.\n");
        cJSON_Delete(root);
        return;
    }
    // 处理第一个结果
    cJSON *result = cJSON_GetArrayItem(results, 0);
    if (!result) {
        fprintf(stderr, "Invalid JSON format: missing first result.\n");
        cJSON_Delete(root);
        return;
    }
    // 获取 location 对象
    cJSON *location = cJSON_GetObjectItem(result, "location");
    if (!location || !cJSON_IsObject(location)) {
        fprintf(stderr, "Invalid JSON format: missing 'location' object.\n");
        cJSON_Delete(root);
        return;
    }
    // 获取 now 对象
    cJSON *now = cJSON_GetObjectItem(result, "now");
    if (!now || !cJSON_IsObject(now)) {
        fprintf(stderr, "Invalid JSON format: missing 'now' object.\n");
        cJSON_Delete(root);
        return;
    }
    // 打印 location 字段
    printf("Location Name: %s\n", cJSON_GetObjectItem(location, "name")->valuestring);
    // 打印 now 字段
    printf("Current Weather: %s\n", cJSON_GetObjectItem(now, "text")->valuestring);
    printf("Temperature: %s\n", cJSON_GetObjectItem(now, "temperature")->valuestring);

    // 填充天气数据结构体
    weather_data_t weather_data;
    memset(&weather_data, 0, sizeof(weather_data));
    
    // 提取城市名称
    strncpy(weather_data.city, cJSON_GetObjectItem(location, "name")->valuestring, 
            sizeof(weather_data.city) - 1);
    
    // 提取天气状态
    strncpy(weather_data.weather, cJSON_GetObjectItem(now, "text")->valuestring, 
            sizeof(weather_data.weather) - 1);
    
    // 提取温度
    strncpy(weather_data.temperature, cJSON_GetObjectItem(now, "temperature")->valuestring, 
            sizeof(weather_data.temperature) - 1);
    
    // 提取天气代码（用于图标映射）
    strncpy(weather_data.code, cJSON_GetObjectItem(now, "code")->valuestring, 
            sizeof(weather_data.code) - 1);
    
    // 提取更新时间、日期和星期（可选）
    cJSON *last_update = cJSON_GetObjectItem(result, "last_update");
    if(last_update != NULL && cJSON_IsString(last_update)) {
        // ISO 8601格式: "2026-01-07T11:47:24+08:00"
        const char *datetime_str = last_update->valuestring;
        
        // 提取日期部分 "2026-01-07"
        strncpy(weather_data.date, datetime_str, 10);
        weather_data.date[10] = '\0';
        
        // 提取时间部分 "11:47:24"
        const char *time_part = strchr(datetime_str, 'T');
        if(time_part != NULL) {
            time_part++; // 跳过 'T'
            strncpy(weather_data.update_time, time_part, 8);
            weather_data.update_time[8] = '\0';
        }
        
        // 计算星期（使用Zeller公式）
        int year, month, day;
        if(sscanf(datetime_str, "%d-%d-%d", &year, &month, &day) == 3) {
            // Zeller公式计算星期
            if(month < 3) {
                month += 12;
                year--;
            }
            int century = year / 100;
            int year_of_century = year % 100;
            int weekday = (day + (13 * (month + 1)) / 5 + year_of_century + 
                          year_of_century / 4 + century / 4 - 2 * century) % 7;
            // 转换为标准格式：0=周日, 1=周一, ..., 6=周六
            weather_data.weekday = (weekday + 6) % 7;
        }
    }
    
    // 打印调试信息
    printf("Weather data extracted: city=%s, weather=%s, temp=%s, code=%s, date=%s, weekday=%d, time=%s\n",
           weather_data.city, weather_data.weather, weather_data.temperature, 
           weather_data.code, weather_data.date, weather_data.weekday, weather_data.update_time);
    
    // 通过回调传递结构化数据
    if(weather_callback_func != NULL) {
        weather_callback_func(&weather_data);
    }
    
    cJSON_Delete(root);
}

/**
 * @brief 检测网络连接状态
 * @return 1-已连接，0-未连接
 * @note Linux环境默认返回1（假设eth0有网），T113环境检测WiFi状态
 */
static int is_network_connected(void)
{
#ifdef __linux__
    // Linux开发环境：假设eth0有网
    return 1;
#else
    // T113嵌入式环境：检测WiFi连接状态
    return wpa_manager_is_connected();
#endif
}

/**
 * @brief 网络状态监听定时器回调函数
 * @param timer 定时器对象指针
 * @note 每5秒检测一次网络状态，网络恢复后自动执行待处理请求
 */
static void network_monitor_timer_cb(lv_timer_t * timer)
{
    // 检查是否有待处理请求
    if (!g_pending_request.pending) {
        return;  // 无待处理请求，继续等待
    }
    
    // 检测网络连接状态
    if (!is_network_connected()) {
        printf("⏳ Network not ready, waiting... (checking every 5s)\n");
        return;  // 网络未连接，继续等待
    }
    
    // 网络已恢复，执行待处理请求
    printf("✅ Network recovered! Processing pending request...\n");
    printf("   API Key: %s\n", g_pending_request.api_key);
    printf("   City: %s\n", g_pending_request.city);
    
    // 根据请求类型执行对应操作
    switch(g_pending_request.request_type) {
        case NET_GET_WEATHER:
            // 重新发送天气请求（不再检测网络，直接发送）
            {
                net_obj *obj = (net_obj *)malloc(sizeof(net_obj));
                if (obj != NULL) {
                    obj->id = NET_GET_WEATHER;
                    strcpy(obj->host, "https://api.seniverse.com");
                    snprintf(obj->path, sizeof(obj->path), 
                            "/v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c",
                            g_pending_request.api_key, g_pending_request.city);
                    strcpy(obj->type, "GET");
                    obj->loop_flag = 1;
                    
                    // 发送消息到网络线程
                    int ret = osal_queue_send(&net_queue, obj, sizeof(net_obj), 1000);
                    if (ret == OSAL_SUCCESS) {
                        printf("📤 Weather request resent successfully\n");
                        // 清空待处理标志
                        g_pending_request.pending = false;
                        memset(g_pending_request.api_key, 0, sizeof(g_pending_request.api_key));
                        memset(g_pending_request.city, 0, sizeof(g_pending_request.city));
                        
                        // 停止监听定时器（请求已发送）
                        if (network_monitor_timer != NULL) {
                            lv_timer_del(network_monitor_timer);
                            network_monitor_timer = NULL;
                            printf("⏹️  Network monitor stopped\n");
                        }
                    }
                    free(obj);
                }
            }
            break;
            
        case NET_GET_TIME:
            // 预留：未来可扩展时间同步请求
            printf("⚠️  NET_GET_TIME not implemented yet\n");
            g_pending_request.pending = false;
            break;
            
        default:
            printf("❌ Unknown request type: %d\n", g_pending_request.request_type);
            g_pending_request.pending = false;
            break;
    }
}

/**
 * @brief 启动网络状态监听定时器
 * @note 每5秒检测一次网络状态，用于自动重试待处理请求
 */
void http_start_network_monitor(void)
{
    // 避免重复创建定时器
    if (network_monitor_timer != NULL) {
        printf("⚠️  Network monitor already running\n");
        return;
    }
    
    // 创建定时器：每5秒执行一次
    network_monitor_timer = lv_timer_create(network_monitor_timer_cb, 5000, NULL);
    if (network_monitor_timer != NULL) {
        printf("🔍 Network monitor started (checking every 5s)\n");
    } else {
        printf("❌ Failed to create network monitor timer\n");
    }
}

//网络模块线程
static void* net_thread_fun(void *arg)
{
    int ret = OSAL_ERROR;
    net_obj obj;
    memset(&obj, 0, sizeof(net_obj));
    char *response_json_str = NULL;  // ⚠️ 必须初始化为NULL，避免野指针
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    while(1)
    {
        ret = osal_queue_recv(&net_queue, (void*)&obj, 100);
        if (ret == OSAL_SUCCESS)
        {
            NET_COMM_ID id = obj.id;
            switch(id)
            {
                case NET_GET_WEATHER:
                    printf("handle NET_GET_WEATHER\n");
                    response_json_str = NULL;  // 每次请求前重置为NULL
                    int http_ret = http_request_method(obj.host,obj.path,obj.type,obj.data,&response_json_str);
                    if (http_ret == 0 && response_json_str != NULL){
                        parseWeatherData(response_json_str);
                        free(response_json_str);
                        response_json_str = NULL;
                    } else {
                        printf("❌ Weather request failed (ret=%d), skipping parse\n", http_ret);
                    }
                    break;
                default:
                    break;
            }
        }
        osal_thread_sleep(500);
    }
}

/**
 * @brief 异步获取天气数据
 * @param key API密钥
 * @param city 城市名称
 * @note 如果网络未连接，请求将被加入待处理队列，网络恢复后自动重试
 */
void http_get_weather_async(char *key, char *city)
{
    printf("\n========== 天气请求开始 ==========\n");
    printf("API Key: %s\n", key);
    printf("City: %s\n", city);
    
    // 检测网络连接状态
    if (!is_network_connected()) {
        printf("⚠️  Network not connected!\n");
        printf("📋 Request queued, will retry when network is available\n");
        printf("=================================\n\n");
        
        // 保存请求参数到待处理队列
        g_pending_request.pending = true;
        strncpy(g_pending_request.api_key, key, sizeof(g_pending_request.api_key) - 1);
        strncpy(g_pending_request.city, city, sizeof(g_pending_request.city) - 1);
        g_pending_request.request_type = NET_GET_WEATHER;
        
        // 启动网络监听定时器（如果还未启动）
        http_start_network_monitor();
        return;
    }
    
    // 网络已连接，立即发送请求
    printf("✅ Network connected, sending request...\n");
    printf("=================================\n\n");
    
    net_obj obj;    
    memset(&obj, 0, sizeof(net_obj));
    strcpy(obj.host, "https://api.seniverse.com");
    snprintf(obj.path, sizeof(obj.path), 
            "/v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c", 
            key, city);
    obj.id = NET_GET_WEATHER;
    strcpy(obj.data, "");
    strcpy(obj.type, "GET");
    
    int ret = osal_queue_send(&net_queue, &obj, sizeof(net_obj), 1000);
    if (ret == OSAL_ERROR) {
        printf("❌ Queue send error\n");
    } else {
        printf("📤 Weather request sent to network thread\n");
    }
}

//设置获取天气回调函数
void http_set_weather_callback(weather_callback_fun func){
    weather_callback_func = func;                       
}

//HTTP模块创建
int http_request_create()
{
    int ret = OSAL_ERROR;
    ret = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (ret != 0)
        return -1;
    ret = osal_queue_create(&net_queue,"net_queue",sizeof(net_obj),50);
    if(ret == OSAL_ERROR)
    {
        printf("create queue error");
        return -1;
    }   
    ret = osal_thread_create(&net_thread,net_thread_fun, NULL);
    if(ret == OSAL_ERROR)
    {
        printf("create thread error");
        return -1;
    }
    return 0;
}
