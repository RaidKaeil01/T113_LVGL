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

#include "http_manager.h"
#include "osal_thread.h"
#include "osal_queue.h"

static osal_queue_t net_queue = NULL;
static osal_thread_t net_thread = NULL;
static weather_callback_fun weather_callback_func = NULL;

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
        if (code == CURLE_COULDNT_RESOLVE_HOST) {
            printf("⚠️  DNS resolution failed - Check network connection!\n");
            printf("💡 Tip: Connect WiFi via Menu → WiFi Settings\n");
        }
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
    
    // 安全获取字段（防止NULL指针）
    cJSON *name_item = cJSON_GetObjectItem(location, "name");
    cJSON *text_item = cJSON_GetObjectItem(now, "text");
    cJSON *temp_item = cJSON_GetObjectItem(now, "temperature");
    
    if (!name_item || !text_item || !temp_item) {
        fprintf(stderr, "Missing required weather fields.\n");
        cJSON_Delete(root);
        return;
    }
    
    // 打印 location 字段
    printf("Location Name: %s\n", name_item->valuestring);
    // 打印 now 字段
    printf("Current Weather: %s\n", text_item->valuestring);
    printf("Temperature: %s\n", temp_item->valuestring);

    // 提取更多字段
    cJSON *last_update = cJSON_GetObjectItem(result, "last_update");
    cJSON *code_item = cJSON_GetObjectItem(now, "code");
    
    // 填充天气数据结构体
    weather_data_t weather_data;
    memset(&weather_data, 0, sizeof(weather_data));
    
    // 基本信息
    strncpy(weather_data.city, name_item->valuestring, sizeof(weather_data.city) - 1);
    strncpy(weather_data.weather, text_item->valuestring, sizeof(weather_data.weather) - 1);
    strncpy(weather_data.temperature, temp_item->valuestring, sizeof(weather_data.temperature) - 1);
    
    // 天气代码
    if (code_item && code_item->valuestring) {
        strncpy(weather_data.code, code_item->valuestring, sizeof(weather_data.code) - 1);
    }
    
    // 更新时间和日期解析
    if (last_update && last_update->valuestring) {
        strncpy(weather_data.update_time, last_update->valuestring, sizeof(weather_data.update_time) - 1);
        
        // 从"2026-01-08T11:47:24+08:00"提取日期和时间
        int year, month, day, hour, min, sec;
        if (sscanf(last_update->valuestring, "%d-%d-%dT%d:%d:%d", 
                   &year, &month, &day, &hour, &min, &sec) == 6) {
            // 格式化日期
            snprintf(weather_data.date, sizeof(weather_data.date), "%04d-%02d-%02d", year, month, day);
            
            // 格式化时间
            snprintf(weather_data.update_time, sizeof(weather_data.update_time), "%02d:%02d:%02d", hour, min, sec);
            
            // 计算星期（Zeller公式）
            if (month < 3) {
                month += 12;
                year -= 1;
            }
            int c = year / 100;
            int y = year % 100;
            int w = (y + y/4 + c/4 - 2*c + (26*(month+1))/10 + day - 1) % 7;
            weather_data.weekday = (w + 7) % 7;  // 确保非负
            
            printf("✅ Date: %s, Weekday: %d\n", weather_data.date, weather_data.weekday);
        }
    }
    
    // 回调通知
    if(weather_callback_func != NULL) {
        weather_callback_func(&weather_data);
    }
    
    cJSON_Delete(root);
}

//网络模块线程
static void* net_thread_fun(void *arg)
{
    int ret = OSAL_ERROR;
    net_obj obj;
    memset(&obj, 0, sizeof(net_obj));
    char *response_json_str;
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
                    http_request_method(obj.host,obj.path,obj.type,obj.data,&response_json_str);
                    if (response_json_str != NULL){
                        parseWeatherData(response_json_str);
                        free(response_json_str);
                    }
                    break;
                default:
                    break;
            }
        }
        osal_thread_sleep(500);
    }
}

//异步获取天气
void http_get_weather_async(char *key,char *city){
    net_obj obj;    
    memset(&obj, 0, sizeof(net_obj));
    strcpy(obj.host,"https://api.seniverse.com");
    sprintf(obj.path, "/v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c", key,city);
    obj.id = NET_GET_WEATHER;
    strcpy(obj.data,"");
    strcpy(obj.type,"GET");
    int ret = osal_queue_send(&net_queue, &obj, sizeof(net_obj), 1000);
    if(ret == OSAL_ERROR)
    {
        printf("queue send error");
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
