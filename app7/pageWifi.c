#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "page_conf.h"
#include "image_conf.h"
#include "wifi/wpa_manager.h"

/* ========== 全局变量 ========== */
// WiFi 相关UI对象
static lv_obj_t * ta_ssid = NULL;       // SSID输入框
static lv_obj_t * ta_password = NULL;   // 密码输入框
static lv_obj_t * keyboard = NULL;      // 键盘对象
static lv_obj_t * label_status = NULL;  // 状态提示标签
static lv_obj_t * scan_list = NULL;     // WiFi扫描结果列表

/**
 * @brief 清理WiFi页面资源
 */
void cleanup_pageWifi(void)
{
    // 断开WiFi连接（注：wpa_manager没有提供close函数，使用disconnect代替）
    wpa_manager_wifi_disconnect();
    
    // 清空全局变量
    ta_ssid = NULL;
    ta_password = NULL;
    keyboard = NULL;
    label_status = NULL;
    scan_list = NULL;
    
    printf("pageWifi cleanup completed\n");
}

/**
 * @brief 返回按钮点击事件回调 - 返回菜单页面
 */
static void back_btn_click_event_cb(lv_event_t * e)
{
    printf("Back button clicked, returning to Menu page\n");
    
    // 清理当前页面资源
    cleanup_pageWifi();
    
    // 清空屏幕
    lv_obj_clean(lv_scr_act());
    
    // 切换到菜单页面
    init_pageMenu();
}

/**
 * @brief WiFi扫描结果回调函数
 * @param results 扫描结果数组
 * @param count 扫描结果数量
 * @note 由 wpa_manager 调用，更新UI显示扫描结果
 */
static void wifi_scan_results_callback(wpa_scan_result_t *results, int count)
{
    printf("WiFi scan callback: found %d networks\n", count);
    
    if(scan_list == NULL) {
        return;
    }
    
    // 清空列表
    lv_obj_clean(scan_list);
    
    // 添加扫描结果到列表
    for(int i = 0; i < count && i < 20; i++) {
        char item_text[128];
        
        // 判断加密类型
        const char *security = "Open";
        if(strstr(results[i].flags, "WPA2") != NULL) {
            security = "WPA2";
        } else if(strstr(results[i].flags, "WPA") != NULL) {
            security = "WPA";
        } else if(strstr(results[i].flags, "WEP") != NULL) {
            security = "WEP";
        }
        
        // 格式化显示：SSID | 信号强度 | 加密类型
        snprintf(item_text, sizeof(item_text), "%s\n  %d dBm | %s",
                 results[i].ssid[0] ? results[i].ssid : "<Hidden>",
                 results[i].signal_level,
                 security);
        
        // 创建列表项标签
        lv_obj_t * label = lv_label_create(scan_list);
        lv_label_set_text(label, item_text);
        lv_obj_set_width(label, lv_pct(100));
        
        // 根据信号强度设置颜色
        if(results[i].signal_level > -50) {
            lv_obj_set_style_text_color(label, lv_color_hex(0x27AE60), LV_PART_MAIN); // 强信号-绿色
        } else if(results[i].signal_level > -70) {
            lv_obj_set_style_text_color(label, lv_color_hex(0xF39C12), LV_PART_MAIN); // 中等信号-橙色
        } else {
            lv_obj_set_style_text_color(label, lv_color_hex(0xE74C3C), LV_PART_MAIN); // 弱信号-红色
        }
    }
    
    // 更新状态
    if(label_status != NULL) {
        char status_text[64];
        snprintf(status_text, sizeof(status_text), "Found %d networks", count);
        lv_label_set_text(label_status, status_text);
        lv_obj_set_style_text_color(label_status, lv_color_hex(0x3498DB), LV_PART_MAIN);
    }
}

/**
 * @brief WiFi连接状态变化回调函数
 * @param status 连接状态
 * @note 由 wpa_manager 后台线程调用，更新UI状态显示
 */
static void wifi_connect_status_callback(WPA_WIFI_CONNECT_STATUS_E status)
{
    printf("WiFi status changed: %d\n", status);
    
    if(label_status == NULL) {
        return;
    }
    
    // 根据不同状态更新UI提示
    switch(status) {
        case WPA_WIFI_INACTIVE:
            lv_label_set_text(label_status, "WiFi Inactive");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0x95A5A6), LV_PART_MAIN);
            break;
            
        case WPA_WIFI_SCANNING:
            lv_label_set_text(label_status, "Scanning networks...");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0x3498DB), LV_PART_MAIN);
            break;
            
        case WPA_WIFI_DISCONNECT:
            lv_label_set_text(label_status, "Disconnected");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            break;
            
        case WPA_WIFI_CONNECT:
            lv_label_set_text(label_status, "√ Connected Successfully!");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0x27AE60), LV_PART_MAIN);
            break;
            
        case WPA_WIFI_WRONG_KEY:
            lv_label_set_text(label_status, "X Wrong Password");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            break;
            
        default:
            lv_label_set_text(label_status, "Unknown Status");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0x95A5A6), LV_PART_MAIN);
            break;
    }
}

/**
 * @brief 连接按钮点击事件回调函数
 * @param e 事件对象
 * @note 用户点击连接按钮后，获取SSID和密码（后续添加连接逻辑）
 */
static void btn_connect_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        // 获取SSID和密码
        const char * ssid = lv_textarea_get_text(ta_ssid);
        const char * password = lv_textarea_get_text(ta_password);
        
        // 检查输入是否为空
        if(ssid == NULL || strlen(ssid) == 0) {
            printf("Error: SSID is empty\n");
            if(label_status != NULL) {
                lv_label_set_text(label_status, "X SSID cannot be empty");
                lv_obj_set_style_text_color(label_status, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            }
            return;
        }
        
        // 打印输入内容（调试用）
        printf("Connecting to WiFi...\n");
        printf("SSID: %s\n", ssid);
        printf("Password: %s\n", password);
        
        // 更新状态提示
        if(label_status != NULL) {
            lv_label_set_text(label_status, "Connecting...");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0xFFA500), LV_PART_MAIN); // 橙色
        }
        
        // 准备WiFi连接信息
        wpa_ctrl_wifi_info_t wifi_info;
        memset(&wifi_info, 0, sizeof(wifi_info));
        
        // 复制SSID和密码（限制长度避免溢出）
        strncpy(wifi_info.ssid, ssid, sizeof(wifi_info.ssid) - 1);
        strncpy(wifi_info.psw, password, sizeof(wifi_info.psw) - 1);
        
        // 调用WiFi连接接口
        int ret = wpa_manager_wifi_connect(&wifi_info);
        if(ret != 0) {
            printf("WiFi connection failed, error code: %d\n", ret);
            if(label_status != NULL) {
                lv_label_set_text(label_status, "X Connection failed");
                lv_obj_set_style_text_color(label_status, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            }
        }
    }
}

/**
 * @brief 取消按钮点击事件回调函数
 * @param e 事件对象
 * @note 清空输入内容，隐藏键盘
 */
static void btn_cancel_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        // 清空输入框
        lv_textarea_set_text(ta_ssid, "");
        lv_textarea_set_text(ta_password, "");
        
        // 重置状态提示
        if(label_status != NULL) {
            lv_label_set_text(label_status, "Ready to connect");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0xA0A0A0), LV_PART_MAIN); // 灰色
        }
        
        printf("Cancelled\n");
    }
}

/**
 * @brief 断开按钮点击事件回调函数
 * @param e 事件对象
 * @note 断开当前WiFi连接
 */
static void btn_disconnect_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        printf("Disconnecting WiFi...\n");
        
        // 更新状态提示
        if(label_status != NULL) {
            lv_label_set_text(label_status, "Disconnecting...");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0xFFA500), LV_PART_MAIN); // 橙色
        }
        
        // 调用WiFi断开接口
        int ret = wpa_manager_wifi_disconnect();
        if(ret != 0) {
            printf("WiFi disconnect failed, error code: %d\n", ret);
            if(label_status != NULL) {
                lv_label_set_text(label_status, "X Disconnect failed");
                lv_obj_set_style_text_color(label_status, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            }
        }
    }
}

/**
 * @brief 扫描按钮点击事件回调函数
 * @param e 事件对象
 * @note 触发WiFi扫描
 */
static void btn_scan_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        printf("Starting WiFi scan...\n");
        
        // 更新状态提示
        if(label_status != NULL) {
            lv_label_set_text(label_status, " Scanning...");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0x3498DB), LV_PART_MAIN); // 蓝色
        }
        
        // 清空扫描结果列表
        if(scan_list != NULL) {
            lv_obj_clean(scan_list);
            lv_obj_t * label = lv_label_create(scan_list);
            lv_label_set_text(label, "Scanning...");
            lv_obj_set_style_text_color(label, lv_color_hex(0x95A5A6), LV_PART_MAIN);
        }
        
        // 触发扫描
        int ret = wpa_manager_wifi_scan();
        if(ret != 0) {
            printf("WiFi scan failed, error code: %d\n", ret);
            if(label_status != NULL) {
                lv_label_set_text(label_status, "X Scan failed");
                lv_obj_set_style_text_color(label_status, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            }
            return;
        }
        
        // 等待3秒后获取扫描结果
        // 注意：在实际应用中应该使用定时器而不是sleep
        sleep(3);
        
        // 获取扫描结果
        wpa_scan_result_t results[50];
        int count = wpa_manager_get_scan_results(results, 50);
        
        if(count < 0) {
            printf("Failed to get scan results\n");
            if(label_status != NULL) {
                lv_label_set_text(label_status, "X No results");
                lv_obj_set_style_text_color(label_status, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            }
        }
    }
}

/**
 * @brief 文本框获得焦点事件回调函数
 * @param e 事件对象
 * @note 当文本框获得焦点时，将键盘绑定到该文本框
 */
static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    
    if(code == LV_EVENT_FOCUSED) {
        // 绑定键盘到当前文本框
        if(keyboard != NULL) {
            lv_keyboard_set_textarea(keyboard, ta);
        }
    }
}

/**
 * @brief 初始化WiFi连接页面
 * @note 创建包含SSID输入框、密码输入框、键盘和按钮的WiFi连接界面
 */
void init_pageWifi(void)
{
    /* ========== 0. 初始化 WiFi 管理器 ========== */
    printf("Initializing WiFi manager...\n");
    
    // 启动WiFi管理器后台线程
    if(wpa_manager_open() == 0) {
        printf("WiFi manager started successfully\n");
        
        // 注册WiFi连接状态回调函数
        wpa_manager_add_callback(NULL, wifi_connect_status_callback);
        
        // 注册WiFi扫描结果回调函数
        wpa_manager_set_scan_callback(wifi_scan_results_callback);
    } else {
        printf("Failed to start WiFi manager\n");
    }
    
    /* ========== 1. 设置屏幕背景颜色 ========== */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xF0F0F0), LV_PART_MAIN);  // 浅灰色背景
    
    
    /* ========== 1.5 返回按钮区域（左上角）========== */
    lv_obj_t * back_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(back_container, 200, 50);
    lv_obj_align(back_container, LV_ALIGN_TOP_LEFT, 10, 5);
    lv_obj_set_style_bg_opa(back_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(back_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(back_container, 0, LV_PART_MAIN);
    lv_obj_add_flag(back_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(back_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 返回图标
    lv_obj_t * back_img = lv_img_create(back_container);
    lv_img_set_src(back_img, GET_IMAGE_PATH("main/back.png"));
    lv_obj_align(back_img, LV_ALIGN_LEFT_MID, 10, 0);
    
    // 标题文字
    lv_obj_t * title_label = lv_label_create(back_container);
    lv_label_set_text(title_label, "WiFi Settings");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x2C3E50), LV_PART_MAIN);
    lv_obj_align_to(title_label, back_img, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // 添加返回按钮点击事件
    lv_obj_add_event_cb(back_container, back_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    
    
    /* ========== 2. 输入区域容器（左侧） ========== */
    lv_obj_t * input_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(input_container, 420, 220); 
    lv_obj_align(input_container, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    
    // 设置容器样式
    lv_obj_set_style_bg_color(input_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(input_container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(input_container, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(input_container, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(input_container, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_pad_all(input_container, 15, LV_PART_MAIN);
    
    
    /* ---------- 2.1 SSID输入框 ---------- */
    // SSID标签
    lv_obj_t * label_ssid = lv_label_create(input_container);
    lv_label_set_text(label_ssid, "WiFi Name (SSID):");
    lv_obj_set_style_text_color(label_ssid, lv_color_hex(0x34495E), LV_PART_MAIN);
    lv_obj_align(label_ssid, LV_ALIGN_TOP_LEFT, 0, 5);
    
    // SSID输入框
    ta_ssid = lv_textarea_create(input_container);
    lv_obj_set_size(ta_ssid, 260, 40); 
    lv_obj_align(ta_ssid, LV_ALIGN_TOP_LEFT, 0, 28);
    lv_textarea_set_one_line(ta_ssid, true);  // 单行模式
    lv_textarea_set_placeholder_text(ta_ssid, "Enter WiFi name");
    
    // 设置SSID输入框样式
    lv_obj_set_style_bg_color(ta_ssid, lv_color_hex(0xF8F9FA), LV_PART_MAIN);
    lv_obj_set_style_border_width(ta_ssid, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(ta_ssid, lv_color_hex(0xDEE2E6), LV_PART_MAIN);
    lv_obj_set_style_border_color(ta_ssid, lv_color_hex(0x3498DB), LV_PART_MAIN | LV_STATE_FOCUSED);
    
    // 添加焦点事件
    lv_obj_add_event_cb(ta_ssid, ta_event_cb, LV_EVENT_FOCUSED, NULL);
    
    
    /* ---------- 2.2 密码输入框 ---------- */
    // 密码标签
    lv_obj_t * label_password = lv_label_create(input_container);
    lv_label_set_text(label_password, "Password:");
    lv_obj_set_style_text_color(label_password, lv_color_hex(0x34495E), LV_PART_MAIN);
    lv_obj_align(label_password, LV_ALIGN_TOP_LEFT, 0, 80);
    
    // 密码输入框
    ta_password = lv_textarea_create(input_container);
    lv_obj_set_size(ta_password, 260, 40);  
    lv_obj_align(ta_password, LV_ALIGN_TOP_LEFT, 0, 103);
    lv_textarea_set_one_line(ta_password, true);  // 单行模式
    lv_textarea_set_password_mode(ta_password, true);  // 密码模式
    lv_textarea_set_placeholder_text(ta_password, "Enter password");
    
    // 设置密码输入框样式
    lv_obj_set_style_bg_color(ta_password, lv_color_hex(0xF8F9FA), LV_PART_MAIN);
    lv_obj_set_style_border_width(ta_password, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(ta_password, lv_color_hex(0xDEE2E6), LV_PART_MAIN);
    lv_obj_set_style_border_color(ta_password, lv_color_hex(0x3498DB), LV_PART_MAIN | LV_STATE_FOCUSED);
    
    // 添加焦点事件
    lv_obj_add_event_cb(ta_password, ta_event_cb, LV_EVENT_FOCUSED, NULL);
    
    
    /* ========== 3. 键盘区域（输入区域右侧20px） ========== */
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, 615, 260);  
    lv_obj_align_to(keyboard, input_container, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    
    // 默认绑定到SSID输入框
    lv_keyboard_set_textarea(keyboard, ta_ssid);
    
    // 设置键盘样式
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(keyboard, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(keyboard, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    
    
    /* ========== 3.5 扫描结果显示区域（键盘右侧） ========== */
    lv_obj_t * scan_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scan_container, 330, 260);
    lv_obj_align_to(scan_container, keyboard, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    
    // 设置扫描结果容器样式
    lv_obj_set_style_bg_color(scan_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scan_container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(scan_container, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(scan_container, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(scan_container, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_pad_all(scan_container, 10, LV_PART_MAIN);
    
    // 标题
    lv_obj_t * scan_title = lv_label_create(scan_container);
    lv_label_set_text(scan_title, "WiFi Networks");
    lv_obj_set_style_text_color(scan_title, lv_color_hex(0x34495E), LV_PART_MAIN);
    lv_obj_set_style_text_font(scan_title, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(scan_title, LV_ALIGN_TOP_MID, 0, 0);
    
    // 创建滚动列表
    scan_list = lv_obj_create(scan_container);
    lv_obj_set_size(scan_list, lv_pct(100), 210);
    lv_obj_align(scan_list, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(scan_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scan_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 设置列表样式
    lv_obj_set_style_bg_color(scan_list, lv_color_hex(0xF8F9FA), LV_PART_MAIN);
    lv_obj_set_style_border_width(scan_list, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(scan_list, lv_color_hex(0xDEE2E6), LV_PART_MAIN);
    lv_obj_set_style_radius(scan_list, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scan_list, 8, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(scan_list, LV_SCROLLBAR_MODE_AUTO);
    
    // 初始提示
    lv_obj_t * hint_label = lv_label_create(scan_list);
    lv_label_set_text(hint_label, "Click 'Scan' to\nsearch WiFi networks");
    lv_obj_set_style_text_color(hint_label, lv_color_hex(0x95A5A6), LV_PART_MAIN);
    lv_obj_set_style_text_align(hint_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    
    
    /* ========== 4. 按钮区域 ========== */
    // 连接按钮
    lv_obj_t * btn_connect = lv_btn_create(input_container);
    lv_obj_set_size(btn_connect, 115, 38); 
    lv_obj_align(btn_connect, LV_ALIGN_TOP_RIGHT, -5, 28);
    
    // 设置连接按钮样式
    lv_obj_set_style_bg_color(btn_connect, lv_color_hex(0x27AE60), LV_PART_MAIN);  // 绿色
    lv_obj_set_style_bg_color(btn_connect, lv_color_hex(0x229954), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_connect, 8, LV_PART_MAIN);
    
    // 连接按钮标签
    lv_obj_t * label_connect = lv_label_create(btn_connect);
    lv_label_set_text(label_connect, "Connect");
    lv_obj_set_style_text_color(label_connect, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(label_connect);
    
    // 添加点击事件
    lv_obj_add_event_cb(btn_connect, btn_connect_event_cb, LV_EVENT_CLICKED, NULL);
    
    
    // 取消按钮
    lv_obj_t * btn_cancel = lv_btn_create(input_container);
    lv_obj_set_size(btn_cancel, 115, 38); 
    lv_obj_align(btn_cancel, LV_ALIGN_TOP_RIGHT, -5, 75);
    
    // 设置取消按钮样式
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xE74C3C), LV_PART_MAIN);  // 红色
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xC0392B), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_cancel, 8, LV_PART_MAIN);
    
    // 取消按钮标签
    lv_obj_t * label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, "Cancel");
    lv_obj_set_style_text_color(label_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(label_cancel);
    
    // 添加点击事件
    lv_obj_add_event_cb(btn_cancel, btn_cancel_event_cb, LV_EVENT_CLICKED, NULL);
    
    
    // 断开按钮
    lv_obj_t * btn_disconnect = lv_btn_create(input_container);
    lv_obj_set_size(btn_disconnect, 115, 38); 
    lv_obj_align(btn_disconnect, LV_ALIGN_TOP_RIGHT, -5, 122);
    
    // 设置断开按钮样式
    lv_obj_set_style_bg_color(btn_disconnect, lv_color_hex(0xF39C12), LV_PART_MAIN);  // 橙色
    lv_obj_set_style_bg_color(btn_disconnect, lv_color_hex(0xD68910), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_disconnect, 8, LV_PART_MAIN);
    
    // 断开按钮标签
    lv_obj_t * label_disconnect = lv_label_create(btn_disconnect);
    lv_label_set_text(label_disconnect, "Disconnect");
    lv_obj_set_style_text_color(label_disconnect, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(label_disconnect);
    
    // 添加点击事件
    lv_obj_add_event_cb(btn_disconnect, btn_disconnect_event_cb, LV_EVENT_CLICKED, NULL);
    
    
    // 扫描按钮
    lv_obj_t * btn_scan = lv_btn_create(input_container);
    lv_obj_set_size(btn_scan, 115, 38); 
    lv_obj_align(btn_scan, LV_ALIGN_TOP_RIGHT, -5, 169);
    
    // 设置扫描按钮样式
    lv_obj_set_style_bg_color(btn_scan, lv_color_hex(0x3498DB), LV_PART_MAIN);  // 蓝色
    lv_obj_set_style_bg_color(btn_scan, lv_color_hex(0x2980B9), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_scan, 8, LV_PART_MAIN);
    
    // 扫描按钮标签
    lv_obj_t * label_scan = lv_label_create(btn_scan);
    lv_label_set_text(label_scan, "Scan");
    lv_obj_set_style_text_color(label_scan, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(label_scan);
    
    // 添加点击事件
    lv_obj_add_event_cb(btn_scan, btn_scan_event_cb, LV_EVENT_CLICKED, NULL);
    
    
    /* ========== 5. 状态提示区域 ========== */
    label_status = lv_label_create(input_container);
    lv_label_set_text(label_status, "Ready to connect");
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x95A5A6), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_LEFT, 3, -8);
}

