#ifndef _WPA_MANAGER_H_
#define _WPA_MANAGER_H_

/**
 * @file wpa_manager.h
 * @brief WiFi管理模块头文件
 * @note 基于wpa_supplicant实现WiFi连接管理
 */

/* ========== 宏定义 ========== */
#define STA_IFNAME  "wlan0"  // WiFi网卡名称
#define STA_CONFIG_PATH "/etc/wifi/wpa_supplicant/wpa_supplicant.conf"  // wpa配置文件路径

/* ========== 枚举类型 ========== */
/**
 * @brief WiFi连接状态枚举
 */
typedef enum {
    WPA_WIFI_INACTIVE = 0,    // 未启动
    WPA_WIFI_SCANNING,        // 扫描中
    WPA_WIFI_DISCONNECT,      // 未连接
    WPA_WIFI_CONNECT,         // 连接成功
    WPA_WIFI_WRONG_KEY,       // 密码错误
} WPA_WIFI_CONNECT_STATUS_E;

/**
 * @brief WiFi开关状态枚举
 */
typedef enum {
    WPA_WIFI_CLOSE = 0,       // WiFi关闭
    WPA_WIFI_OPEN,            // WiFi打开
} WPA_WIFI_STATUS_E;

/* ========== 结构体定义 ========== */
/**
 * @brief WiFi连接信息结构体
 */
typedef struct {
    char ssid[32];    // WiFi名称（SSID）
    char psw[32];     // WiFi密码
} wpa_ctrl_wifi_info_t;

/**
 * @brief WiFi扫描结果结构体
 */
typedef struct {
    char ssid[64];           // WiFi名称（SSID）
    char bssid[20];          // MAC地址
    int frequency;           // 频率（MHz）
    int signal_level;        // 信号强度（dBm）
    char flags[128];         // 加密类型标志（WPA/WPA2/WEP等）
} wpa_scan_result_t;

/* ========== 回调函数类型定义 ========== */
/**
 * @brief WiFi连接状态变化回调函数类型
 * @param status 连接状态
 */
typedef void (*connect_status_callback_fun)(WPA_WIFI_CONNECT_STATUS_E status);

/**
 * @brief WiFi开关状态变化回调函数类型
 * @param status 开关状态
 */
typedef void (*wifi_status_callback_fun)(WPA_WIFI_STATUS_E status);

/**
 * @brief WiFi扫描结果回调函数类型
 * @param results 扫描结果数组
 * @param count 扫描结果数量
 */
typedef void (*wifi_scan_callback_fun)(wpa_scan_result_t *results, int count);

/* ========== 函数声明 ========== */
/**
 * @brief 初始化WiFi管理器，启动后台监听线程
 * @return 0-成功，-1-失败
 */
int wpa_manager_open(void);

/**
 * @brief 查询当前WiFi连接状态
 * @note 查询结果通过回调函数返回
 */
void wpa_manager_wifi_status(void);

/**
 * @brief 同步检测WiFi是否已连接
 * @return 1-已连接，0-未连接
 * @note 使用全局缓存状态，快速返回结果
 */
int wpa_manager_is_connected(void);

/**
 * @brief 连接到指定WiFi
 * @param wifi_info WiFi信息指针（包含SSID和密码）
 * @return 0-成功，非0-失败
 */
int wpa_manager_wifi_connect(wpa_ctrl_wifi_info_t *wifi_info);

/**
 * @brief 上电自动连接初始WiFi（带超时等待）
 * @param wifi_info WiFi信息指针
 * @param timeout_sec 超时时间（秒）
 * @return 1-连接成功，0-连接失败或超时
 */
int wpa_manager_auto_connect_default_wifi(wpa_ctrl_wifi_info_t *wifi_info, int timeout_sec);

/**
 * @brief 断开当前WiFi连接
 * @return 0-命令发送成功，非0-失败
 */
int wpa_manager_wifi_disconnect(void);

/**
 * @brief 开始扫描附近的WiFi网络
 * @return 0-命令发送成功，非0-失败
 */
int wpa_manager_wifi_scan(void);

/**
 * @brief 获取WiFi扫描结果
 * @param results 扫描结果数组（调用者分配）
 * @param max_results 最大结果数量
 * @return 实际扫描到的WiFi数量，失败返回-1
 */
int wpa_manager_get_scan_results(wpa_scan_result_t *results, int max_results);

/**
 * @brief 注册WiFi扫描结果回调函数
 * @param scan_callback_f 扫描结果回调函数
 */
void wpa_manager_set_scan_callback(wifi_scan_callback_fun scan_callback_f);

/**
 * @brief 注册WiFi状态回调函数
 * @param wifi_status_f WiFi开关状态回调函数（可传NULL）
 * @param connect_status_f WiFi连接状态回调函数
 */
void wpa_manager_add_callback(wifi_status_callback_fun wifi_status_f,
                               connect_status_callback_fun connect_status_f);

/**
 * @brief 保存WiFi配置到文件
 */
void wpa_manager_wifi_save_config(void);

#endif // _WPA_MANAGER_H_
