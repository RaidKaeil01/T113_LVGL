/**
 * @file wpa_manager.c
 * @brief WiFi管理模块实现文件
 * @note 通过wpa_supplicant实现WiFi连接、断开、状态监听等功能
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "wpa_ctrl.h"
#include "wpa_manager.h"

/* ========== 全局变量 ========== */
// WiFi后台监听线程句柄
static pthread_t event_thread = 0;

// wpa_supplicant控制句柄
static struct wpa_ctrl *g_pstWpaCtrl = NULL;

// WiFi开关状态
static WPA_WIFI_STATUS_E g_wifi_status = WPA_WIFI_CLOSE;

// WiFi连接状态
static WPA_WIFI_CONNECT_STATUS_E g_connect_status = WPA_WIFI_INACTIVE;

// 回调函数指针
static connect_status_callback_fun connect_status_func = NULL;
static wifi_status_callback_fun wifi_status_func = NULL;
static wifi_scan_callback_fun wifi_scan_func = NULL;

/* ========== 内部辅助函数 ========== */
/**
 * @brief 检查进程是否正在运行
 * @param name 进程名称
 * @return 1-进程存在，0-进程不存在
 */
static int is_process_running(const char *name)
{
    char cmd[128];
    char buf[256];
    FILE *fp;
    
    snprintf(cmd, sizeof(cmd), "ps | grep %s | grep -v grep", name);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return 0;
    }
    
    int bytes = fread(buf, sizeof(char), sizeof(buf), fp);
    pclose(fp);
    
    if (bytes > 0) {
        printf("%s: process exist\n", name);
        return 1;
    } else {
        printf("%s: process not exist\n", name);
        return 0;
    }
}

/**
 * @brief 向wpa_supplicant发送命令
 * @param cmd 命令字符串
 * @param reply 回复缓冲区
 * @param reply_len 回复缓冲区长度（输入输出参数）
 * @return 0-成功，-1-失败
 */
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

/**
 * @brief 启动WiFi网卡和wpa_supplicant进程
 */
static void wpa_manager_wifi_on(void)
{
    printf("wpa_manager_wifi_on\n");
    char cmdstr[256];
    
    // 检查wpa_supplicant是否已经在运行
    if (is_process_running("wpa_supplicant")) {
        printf("wpa_supplicant already running\n");
        return;
    }
    
    // 启动WiFi网卡
    snprintf(cmdstr, sizeof(cmdstr), "ifconfig %s up", STA_IFNAME);
    system(cmdstr);
    
    // 启动wpa_supplicant守护进程
    snprintf(cmdstr, sizeof(cmdstr), "wpa_supplicant -i %s -c %s -B", 
             STA_IFNAME, STA_CONFIG_PATH);
    system(cmdstr);
    
    printf("WiFi interface and wpa_supplicant started\n");
}

/**
 * @brief 连接到wpa_supplicant的控制接口
 * @return 0-成功，-1-失败
 */
static int wpa_manager_connect_socket(void)
{
    char path[128];
    
    snprintf(path, sizeof(path), "/etc/wifi/wpa_supplicant/sockets/%s", STA_IFNAME);
    
    // 打开与wpa_supplicant的控制接口连接
    g_pstWpaCtrl = wpa_ctrl_open(path);
    if (g_pstWpaCtrl == NULL) {
        printf("Failed to connect to wpa_supplicant socket: %s\n", path);
        return -1;
    }
    
    // 附加监听wpa_supplicant事件
    if (wpa_ctrl_attach(g_pstWpaCtrl) != 0) {
        printf("Failed to attach to wpa_supplicant\n");
        wpa_ctrl_close(g_pstWpaCtrl);
        g_pstWpaCtrl = NULL;
        return -1;
    }
    
    printf("Connected to wpa_supplicant socket: %s\n", path);
    return 0;
}

/* ========== 外部接口函数实现 ========== */
/**
 * @brief 保存WiFi配置到文件
 */
void wpa_manager_wifi_save_config(void)
{
    printf("wpa_manager_wifi_save_config\n");
    char reply_buf[256] = {0};
    size_t reply_len = sizeof(reply_buf);
    
    if (wifi_send_command("SAVE_CONFIG", reply_buf, &reply_len) == 0) {
        reply_buf[reply_len] = '\0';
        printf("SAVE_CONFIG ---> status = %s\n", reply_buf);
    }
}

/**
 * @brief 查询WiFi连接状态
 */
void wpa_manager_wifi_status(void)
{
    printf("wpa_manager_wifi_status\n");
    char reply_buf[512] = {0};
    size_t reply_len = sizeof(reply_buf);
    
    // 发送STATUS命令查询WiFi状态
    if (wifi_send_command("STATUS", reply_buf, &reply_len) == 0) {
        reply_buf[reply_len] = '\0';
        
        // 解析状态字符串
        if (strstr(reply_buf, "wpa_state=COMPLETED") != NULL) {
            g_connect_status = WPA_WIFI_CONNECT;
        } else if (strstr(reply_buf, "wpa_state=DISCONNECTED") != NULL) {
            g_connect_status = WPA_WIFI_DISCONNECT;
        } else if (strstr(reply_buf, "wpa_state=SCANNING") != NULL) {
            g_connect_status = WPA_WIFI_SCANNING;
        } else if (strstr(reply_buf, "wpa_state=INACTIVE") != NULL) {
            g_connect_status = WPA_WIFI_INACTIVE;
        }
        
        printf("---> WiFi status = %d\n", g_connect_status);
        
        // 触发回调函数
        if (connect_status_func != NULL) {
            connect_status_func(g_connect_status);
        }
    }
}

/**
 * @brief 同步检测WiFi是否已连接
 * @return 1-已连接，0-未连接
 * @note 使用全局缓存状态，快速返回结果
 */
int wpa_manager_is_connected(void)
{
    // 方式1：使用缓存状态（快速，适合频繁查询）
    if (g_connect_status == WPA_WIFI_CONNECT) {
        return 1;
    }
    
    // 方式2：实时查询确认（更准确，但较慢）
    char reply_buf[512] = {0};
    size_t reply_len = sizeof(reply_buf);
    
    if (wifi_send_command("STATUS", reply_buf, &reply_len) == 0) {
        reply_buf[reply_len] = '\0';
        if (strstr(reply_buf, "wpa_state=COMPLETED") != NULL) {
            g_connect_status = WPA_WIFI_CONNECT;
            printf("✅ WiFi is connected\n");
            return 1;
        }
    }
    
    printf("⚠️  WiFi is NOT connected (status=%d)\n", g_connect_status);
    return 0;
}

/**
 * @brief 连接到指定WiFi
 * @param wifi_info WiFi信息指针
 * @return 0-成功，非0-失败
 */
int wpa_manager_wifi_connect(wpa_ctrl_wifi_info_t *wifi_info)
{
    char reply_buf[128] = {0};
    size_t reply_len;
    int ret;
    int net_id = -1;
    char cmd_buf[256];
    
    if (wifi_info == NULL) {
        printf("Error: wifi_info is NULL\n");
        return -1;
    }
    
    printf("Connecting to WiFi: SSID=%s\n", wifi_info->ssid);
    
    // 步骤1: 清除所有现有网络配置
    reply_len = sizeof(reply_buf);
    wifi_send_command("REMOVE_NETWORK all", reply_buf, &reply_len);
    
    reply_len = sizeof(reply_buf);
    wifi_send_command("SAVE_CONFIG", reply_buf, &reply_len);
    
    // 步骤2: 添加新网络
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command("ADD_NETWORK", reply_buf, &reply_len);
    if (ret != 0) {
        printf("ADD_NETWORK failed\n");
        return ret;
    }
    
    reply_buf[reply_len] = '\0';
    net_id = atoi(reply_buf);
    printf("Added network ID: %d\n", net_id);
    
    // 步骤3: 设置SSID
    snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d ssid \"%s\"", net_id, wifi_info->ssid);
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    if (ret != 0) {
        printf("SET_NETWORK ssid failed\n");
        return ret;
    }
    
    // 步骤4: 设置密码（PSK）
    snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d psk \"%s\"", net_id, wifi_info->psw);
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    if (ret != 0) {
        printf("SET_NETWORK psk failed\n");
        return ret;
    }
    
    // 步骤5: 启用网络
    snprintf(cmd_buf, sizeof(cmd_buf), "ENABLE_NETWORK %d", net_id);
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    if (ret != 0) {
        printf("ENABLE_NETWORK failed\n");
        return ret;
    }
    
    // 步骤6: 选择网络并开始连接
    snprintf(cmd_buf, sizeof(cmd_buf), "SELECT_NETWORK %d", net_id);
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command(cmd_buf, reply_buf, &reply_len);
    if (ret != 0) {
        printf("SELECT_NETWORK failed\n");
        return ret;
    }
    
    printf("WiFi connection command sent successfully\n");
    return 0;
}

/**
 * @brief 断开当前WiFi连接
 * @return 0-成功，非0-失败
 */
int wpa_manager_wifi_disconnect(void)
{
    char reply_buf[128] = {0};
    size_t reply_len;
    int ret;
    
    printf("Disconnecting from WiFi...\n");
    
    // 步骤1: 发送 DISCONNECT 命令
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command("DISCONNECT", reply_buf, &reply_len);
    if (ret != 0) {
        printf("DISCONNECT command failed\n");
        return ret;
    }
    
    // 步骤2: 移除所有网络配置
    reply_len = sizeof(reply_buf);
    wifi_send_command("REMOVE_NETWORK all", reply_buf, &reply_len);
    
    // 步骤43: 保存配置
    reply_len = sizeof(reply_buf);
    wifi_send_command("SAVE_CONFIG", reply_buf, &reply_len);
    
    printf("WiFi disconnect command sent successfully\n");
    
    // 更新状态
    g_connect_status = WPA_WIFI_DISCONNECT;
    if (connect_status_func != NULL) {
        connect_status_func(g_connect_status);
    }
    
    return 0;
}

/**
 * @brief 开始扫描附近的WiFi网络
 * @return 0-成功，非0-失败
 */
int wpa_manager_wifi_scan(void)
{
    char reply_buf[128] = {0};
    size_t reply_len;
    int ret;
    
    printf("Starting WiFi scan...\n");
    
    // 发送 SCAN 命令触发扫描
    reply_len = sizeof(reply_buf);
    ret = wifi_send_command("SCAN", reply_buf, &reply_len);
    if (ret != 0) {
        printf("SCAN command failed\n");
        return ret;
    }
    
    reply_buf[reply_len] = '\0';
    printf("SCAN command sent: %s\n", reply_buf);
    
    return 0;
}

/**
 * @brief 获取WiFi扫描结果
 * @param results 扫描结果数组
 * @param max_results 最大结果数量
 * @return 实际扫描到的WiFi数量，失败返回-1
 */
int wpa_manager_get_scan_results(wpa_scan_result_t *results, int max_results)
{
    char reply_buf[4096] = {0};  // 大缓冲区用于扫描结果
    size_t reply_len;
    int ret;
    int count = 0;
    
    if (results == NULL || max_results <= 0) {
        printf("Invalid parameters\n");
        return -1;
    }
    
    printf("Getting WiFi scan results...\n");
    
    // 发送 SCAN_RESULTS 命令获取扫描结果
    reply_len = sizeof(reply_buf) - 1;
    ret = wifi_send_command("SCAN_RESULTS", reply_buf, &reply_len);
    if (ret != 0) {
        printf("SCAN_RESULTS command failed\n");
        return -1;
    }
    
    reply_buf[reply_len] = '\0';
    
    printf("Raw scan results:\n%s\n", reply_buf);
    
    // 解析扫描结果
    // 格式: bssid / frequency / signal level / flags / ssid
    // 例如: 00:11:22:33:44:55\t2437\t-45\t[WPA2-PSK-CCMP][ESS]\tMyWiFi
    
    // 按行分割
    char *line_start = reply_buf;
    char *line_end;
    int line_num = 0;
    
    while ((line_end = strchr(line_start, '\n')) != NULL && count < max_results) {
        *line_end = '\0';  // 临时替换换行符
        
        // 跳过第一行标题
        if (line_num > 0 && strlen(line_start) > 0) {
            wpa_scan_result_t *result = &results[count];
            memset(result, 0, sizeof(wpa_scan_result_t));
            
            // 手动解析每个字段（避免 strtok 修改原始数据）
            char *field_start = line_start;
            char *field_end;
            int field = 0;
            
            while (field_start && field < 5) {
                // 查找制表符
                field_end = strchr(field_start, '\t');
                
                // 如果是最后一个字段（SSID），可能没有制表符
                if (field_end == NULL && field == 4) {
                    field_end = field_start + strlen(field_start);
                }
                
                if (field_end) {
                    int field_len = field_end - field_start;
                    
                    switch(field) {
                        case 0:  // BSSID
                            if (field_len < sizeof(result->bssid)) {
                                strncpy(result->bssid, field_start, field_len);
                                result->bssid[field_len] = '\0';
                            }
                            break;
                        case 1:  // Frequency
                            result->frequency = atoi(field_start);
                            break;
                        case 2:  // Signal level
                            result->signal_level = atoi(field_start);
                            break;
                        case 3:  // Flags
                            if (field_len < sizeof(result->flags)) {
                                strncpy(result->flags, field_start, field_len);
                                result->flags[field_len] = '\0';
                            }
                            break;
                        case 4:  // SSID
                            if (field_len < sizeof(result->ssid)) {
                                strncpy(result->ssid, field_start, field_len);
                                result->ssid[field_len] = '\0';
                            }
                            break;
                    }
                    
                    field++;
                    
                    // 移动到下一个字段
                    if (*field_end == '\t') {
                        field_start = field_end + 1;
                    } else {
                        break;  // 已到行尾
                    }
                } else {
                    break;
                }
            }
            
            // 验证并添加结果
            if (field >= 4) {  // 至少有BSSID、频率、信号强度、加密类型
                printf("Found WiFi [%d]: SSID='%s', Signal=%d dBm, Flags=%s, BSSID=%s\n",
                       count + 1, result->ssid[0] ? result->ssid : "<Hidden>", 
                       result->signal_level, result->flags, result->bssid);
                count++;
            }
        }
        
        line_num++;
        line_start = line_end + 1;
    }
    
    printf("Total WiFi networks found: %d\n", count);
    
    // 触发扫描结果回调
    if (wifi_scan_func != NULL && count > 0) {
        wifi_scan_func(results, count);
    }
    
    return count;
}

/**
 * @brief 注册WiFi扫描结果回调函数
 * @param scan_callback_f 扫描结果回调函数
 */
void wpa_manager_set_scan_callback(wifi_scan_callback_fun scan_callback_f)
{
    wifi_scan_func = scan_callback_f;
    printf("WiFi scan callback function registered\n");
}

/**
 * @brief WiFi事件监听线程
 * @param arg 线程参数（未使用）
 * @return NULL
 */
static void *wpa_manager_event_thread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    
    printf("WiFi event thread started\n");
    
    // 步骤1: 启动WiFi网卡和wpa_supplicant
    wpa_manager_wifi_on();
    
    // 步骤2: 连接到wpa_supplicant控制接口（最多重试10次）
    for (int count = 0; count < 10; count++) {
        if (wpa_manager_connect_socket() == 0) {
            printf("Connected to wpa_supplicant successfully\n");
            break;
        }
        sleep(1);
        printf("Retry connecting to wpa_supplicant... (%d/10)\n", count + 1);
    }
    
    if (g_pstWpaCtrl == NULL) {
        printf("Failed to connect to wpa_supplicant after 10 retries\n");
        return NULL;
    }
    
    // 步骤3: 查询初始WiFi状态
    wpa_manager_wifi_status();
    
    // 步骤4: 进入事件循环，监听wpa_supplicant事件
    printf("Entering WiFi event loop...\n");
    while (1) {
        // 检查是否有待处理的事件
        if (g_pstWpaCtrl && wpa_ctrl_pending(g_pstWpaCtrl) > 0) {
            char buf[512];
            size_t len = sizeof(buf) - 1;
            
            // 接收wpa_supplicant发送的事件
            if (wpa_ctrl_recv(g_pstWpaCtrl, buf, &len) == 0) {
                buf[len] = '\0';
                printf("WiFi Event: %s\n", buf);
                
                // 解析并处理事件
                if (strstr(buf, "CTRL-EVENT-CONNECTED")) {
                    // WiFi连接成功事件
                    printf("✅ WiFi connected, acquiring IP address...\n");
                    
                    // 使用udhcpc获取IP地址
                    char cmd[128];
                    snprintf(cmd, sizeof(cmd), "udhcpc -i %s -t 5 -T 2 -A 5 -q", STA_IFNAME);
                    system(cmd);
                    
                    // 保存配置
                    wpa_manager_wifi_save_config();
                    
                    g_connect_status = WPA_WIFI_CONNECT;
                    
                } else if (strstr(buf, "CTRL-EVENT-DISCONNECTED") != NULL) {
                    // WiFi断开连接事件
                    printf("❌ WiFi disconnected\n");
                    g_connect_status = WPA_WIFI_DISCONNECT;
                    
                } else if (strstr(buf, "CTRL-EVENT-SSID-TEMP-DISABLED")) {
                    // 密码错误事件
                    printf("🔑 WiFi authentication failed (wrong password)\n");
                    g_connect_status = WPA_WIFI_WRONG_KEY;
                }
                
                // 触发回调函数通知应用层
                if (connect_status_func != NULL) {
                    connect_status_func(g_connect_status);
                }
            }
        }
        
        // 休眠10毫秒，降低CPU占用
        usleep(10 * 1000);
    }
    
    return NULL;
}

/**
 * @brief 注册WiFi状态回调函数
 * @param wifi_status_f WiFi开关状态回调函数
 * @param connect_status_f WiFi连接状态回调函数
 */
void wpa_manager_add_callback(wifi_status_callback_fun wifi_status_f,
                               connect_status_callback_fun connect_status_f)
{
    wifi_status_func = wifi_status_f;
    connect_status_func = connect_status_f;
    printf("WiFi callback functions registered\n");
}

/**
 * @brief 获取当前WiFi连接状态
 * @return WiFi连接状态枚举值
 */
WPA_WIFI_CONNECT_STATUS_E wpa_manager_get_connect_status(void)
{
    return g_connect_status;
}

/**
 * @brief 上电自动连接初始WiFi（非阻塞版本）
 * @param wifi_info WiFi信息指针
 * @param timeout_sec 超时时间（秒）- 已废弃，保留兼容性
 * @return 0-命令发送成功，-1-发送失败
 * @note 仅发起连接命令，不等待连接结果，结果通过回调通知
 */
int wpa_manager_auto_connect_default_wifi(wpa_ctrl_wifi_info_t *wifi_info, int timeout_sec)
{
    if (wifi_info == NULL) {
        printf("❌ Error: wifi_info is NULL\n");
        return -1;
    }
    
    printf("\n========== 自动连接初始WiFi ==========\n");
    printf("📡 SSID: %s\n", wifi_info->ssid);
    printf("🚀 Mode: Non-blocking (async)\n");
    printf("=================================\n\n");
    
    // 等待WiFi模块初始化完成（1秒）
    printf("⏳ Waiting 1 second for WiFi module ready...\n");
    sleep(1);
    
    // 发起连接命令（不等待结果）
    int ret = wpa_manager_wifi_connect(wifi_info);
    if (ret != 0) {
        printf("❌ WiFi connection command failed\n\n");
        return -1;
    }
    
    printf("✅ WiFi connection request sent\n");
    printf("💡 Connection result will be notified via callback\n");
    printf("   - Check console for connection status\n");
    printf("   - Or use Menu → WiFi Settings to check manually\n\n");
    
    return 0;  // 命令发送成功，不等待结果
}

/**
 * @brief 初始化WiFi管理器，启动后台监听线程
 * @return 0-成功，-1-失败
 */
int wpa_manager_open(void)
{
    printf("wpa_manager_open: Initializing WiFi manager...\n");
    
    // 创建WiFi事件监听线程
    int ret = pthread_create(&event_thread, NULL, wpa_manager_event_thread, NULL);
    if (ret != 0) {
        printf("Failed to create WiFi event thread, error: %d\n", ret);
        return -1;
    }
    
    // 分离线程，自动回收资源
    pthread_detach(event_thread);
    
    printf("WiFi manager initialized successfully\n");
    return 0;
}
