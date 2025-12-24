
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "common/wpa_ctrl.h"
#include "osal_thread.h"
#include "wpa_manager.h"
#include "base_utils.h"

//WiFi线程句柄
static osal_thread_t event_thread = NULL;
//wpa句柄
static struct wpa_ctrl *g_pstWpaCtrl = NULL;
//WiFi状态
static WPA_WIFI_STATUS_E g_wifi_status = WPA_WIFI_CLOSE;
//连接状态
static WPA_WIFI_CONNECT_STATUS_E g_connect_status = WPA_WIFI_INACTIVE;
//回调函数
connect_status_callback_fun connect_status_func = NULL;
wifi_status_callback_fun wifi_status_func = NULL;

// 发送WiFi指令
static int wifi_send_command(const char *cmd, char *reply, size_t *reply_len)
{
    int ret;
    if (g_pstWpaCtrl == NULL) {
        printf("Not connected to wpa_supplicant - \"%s\" command dropped.\n", cmd);
        return -1;
    }
    ret = wpa_ctrl_request(g_pstWpaCtrl, cmd, strlen(cmd), reply, reply_len, NULL);
    if (ret < 0) {
        printf("'%s' command error.\n", cmd);
        return ret;
    } 
    return 0;
}

//打开WiFi
static void wpa_manager_wifi_on(){
    printf("wpa_manager_wifi_on\n");
    char cmdstr[200];
    if(base_utils_get_process_state("wpa_supplicant",14) == 1)
     return;
    sprintf(cmdstr,"ifconfig %s up", STA_IFNAME);
    system(cmdstr);
    sprintf(cmdstr, "wpa_supplicant -i %s -c %s -B",STA_IFNAME,STA_CONFIG_PATH);
    system(cmdstr);
}

//连接Socket
static int wpa_manager_connect_socket(void)
{
    char path[128];
    snprintf(path, sizeof(path), "/etc/wifi/wpa_supplicant/sockets/%s", STA_IFNAME);
    //打开与wpa_supplicant的控制接口连接
    g_pstWpaCtrl = wpa_ctrl_open(path);
    if (g_pstWpaCtrl == NULL) {
        return -1;
    }
    // 附加监听wpa_supplicant事件（如连接成功、断开、扫描完成等）
    // 调用wpa_ctrl_attach后，可通过wpa_ctrl_recv接收事件通知
    return wpa_ctrl_attach(g_pstWpaCtrl);
}

void wpa_manager_wifi_save_config(void)
{
    printf("wpa_manager_wifi_save_config\n");
    char reply_buf[256] = {0};
    size_t reply_len = sizeof(reply_buf);
    if (wifi_send_command("SAVE_CONFIG", reply_buf, &reply_len) == 0) {
        printf("SAVE_CONFIG--->status = %s\n",reply_buf);
    }
}

//查询WiFi状态
void wpa_manager_wifi_status(void)
{
    printf("wpa_manager_wifi_status\n");
    char reply_buf[256] = {0};
    size_t reply_len = sizeof(reply_buf);
    //发送STATUS，查询WiFi状态
    if (wifi_send_command("STATUS", reply_buf, &reply_len) == 0) {
        if (strstr(reply_buf, "COMPLETED") != NULL) {
            g_connect_status = WPA_WIFI_CONNECT;
        } else if (strstr(reply_buf, "DISCONNECTED") != NULL) {
            g_connect_status = WPA_WIFI_DISCONNECT;
        } else if (strstr(reply_buf, "SCANNING") != NULL) {
            g_connect_status = WPA_WIFI_SCANNING;
        } else if (strstr(reply_buf, "INACTIVE") != NULL) {
            g_connect_status = WPA_WIFI_INACTIVE;
        }
        printf("--->status = %d\n",g_connect_status);
        if(connect_status_func != NULL)
            connect_status_func(g_connect_status);
    }
}

//连接WiFi
int wpa_manager_wifi_connect(wpa_ctrl_wifi_info_t *wifi_info)
{
    char reply_buf[128] = {0};
    size_t reply_len;
    int ret;
    int net_id = -1;
    // 清理现有网络配置
    wifi_send_command("REMOVE_NETWORK all", reply_buf, &reply_len);
    wifi_send_command("SAVE_CONFIG", reply_buf, &reply_len);
    // 添加新网络
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command("ADD_NETWORK", reply_buf, &reply_len);
    if (ret != 0) {
        return ret;
    }
    reply_buf[reply_len] = '\0';
    net_id = atoi(reply_buf);
    // 设置SSID
    char cmd_buf[128];
    snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d ssid \"%s\"", net_id, wifi_info->ssid);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    if (ret != 0) {
        return ret;
    }
    // 设置PSK密码
    snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d psk \"%s\"", net_id, wifi_info->psw);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    if (ret != 0) {
        return ret;
    }
    // 启用网络
    snprintf(cmd_buf, sizeof(cmd_buf), "ENABLE_NETWORK %d", net_id);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    if (ret != 0) {
        return ret;
    }
    // 选择网络
    snprintf(cmd_buf, sizeof(cmd_buf), "SELECT_NETWORK %d", net_id);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    return ret;
}

//WiFi线程
void *wpa_manager_event_thread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    //打开WiFi
    wpa_manager_wifi_on();
    //连接wpa_supplicant
    for (int count = 0; count < 10; count++) {
        if (wpa_manager_connect_socket() == 0) break;
        osal_thread_sleep(1000);
        printf("wpa_manager_connect_socket\n");

    }
    //查询WiFi状态
    wpa_manager_wifi_status();
    // 事件处理循环
    while (1) {
        // 检查是否已建立与wpa_supplicant的连接，且有未处理的事件
        if (g_pstWpaCtrl && wpa_ctrl_pending(g_pstWpaCtrl) > 0) {
            char buf[512];
            size_t len = sizeof(buf);
            // 接收wpa_supplicant发送的事件数据
            if (wpa_ctrl_recv(g_pstWpaCtrl, buf, &len) == 0) {
                buf[len] = '\0';
                // 处理"已连接"事件
                if (strstr(buf, "CTRL-EVENT-CONNECTED")) {
                    char cmd[64];
                    snprintf(cmd, sizeof(cmd), "udhcpc -i %s -t 5 -T 2 -A 5 -q", STA_IFNAME);
                    system(cmd);
                    wpa_manager_wifi_save_config();
                    g_connect_status = WPA_WIFI_CONNECT;
                } else if(strstr(buf, "CTRL-EVENT-DISCONNECTED") != NULL){
                    g_connect_status = WPA_WIFI_DISCONNECT;
                } else if (strstr(buf, "CTRL-EVENT-SSID-TEMP-DISABLED")) {
                    g_connect_status = WPA_WIFI_WRONG_KEY;
                }
                printf("--->status = %d\n",g_connect_status);
                if(connect_status_func != NULL)
                    connect_status_func(g_connect_status);
            }
        }
        // 休眠10毫秒，降低CPU占用
        osal_thread_sleep(10);
    }
    return NULL;
}

//注册WiFi状态监听函数
void wpa_manager_add_callback(wifi_status_callback_fun wifi_status_f,
                                connect_status_callback_fun connect_status_f){
    wifi_status_func = wifi_status_f;                       
    connect_status_func = connect_status_f;
}

//初始化WiFi，创建WiFi线程
int wpa_manager_open(){
    int ret = osal_thread_create(&event_thread,wpa_manager_event_thread, NULL);
    if(ret == -1)
    {
        printf("create thread error\n");
        return ret;
    }
    return ret;
}
