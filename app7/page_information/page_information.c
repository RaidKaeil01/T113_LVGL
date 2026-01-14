#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "page_conf.h"
#include "image_conf.h"
#include "font_conf.h"

/* ========== 全局变量 ========== */
static lv_style_t com_style;
static lv_obj_t *content_label = NULL;  // 右侧内容显示标签
static lv_obj_t *btn_sysinfo = NULL;    // 系统信息按钮
static lv_obj_t *btn_syslog = NULL;     // 系统日志按钮
static int current_tab = 0;             // 当前选中的tab: 0=系统信息, 1=系统日志

/* ========== 样式初始化 ========== */
static void com_style_init(void)
{
    lv_style_init(&com_style);
    lv_style_set_bg_color(&com_style, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&com_style, 0);
    lv_style_set_border_width(&com_style, 0);
    lv_style_set_pad_all(&com_style, 0);
    lv_style_set_outline_width(&com_style, 0);
}

/* ========== 字体设置 ========== */
static void obj_font_set(lv_obj_t *obj, int type, uint16_t weight)
{
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}

/**
 * @brief 清理信息页面资源
 */
void cleanup_pageInformation(void)
{
    printf("Cleaning up pageInformation resources...\n");
    
    // 清理样式
    if(lv_style_is_empty(&com_style) == false) {
        lv_style_reset(&com_style);
        printf("Style reset\n");
    }
    
    // 移除事件回调
    lv_obj_remove_event_cb(lv_scr_act(), NULL);
    
    // 清空指针
    content_label = NULL;
    btn_sysinfo = NULL;
    btn_syslog = NULL;
    current_tab = 0;
    
    printf("pageInformation cleanup completed\n");
}

/**
 * @brief 返回按钮点击回调 - 返回菜单页面
 */
static void back_btn_event_cb(lv_event_t * e)
{
    printf("Back button clicked, returning to Menu page\n");
    
    // 清理当前页面资源
    cleanup_pageInformation();
    
    // 清空屏幕
    lv_obj_clean(lv_scr_act());
    
    // 返回菜单页面
    init_pageMenu();
}

/**
 * @brief 更新按钮选中状态
 */
static void update_button_state(int selected_tab)
{
    current_tab = selected_tab;
    
    if(selected_tab == 0) {
        // 系统信息按钮选中
        lv_obj_set_style_bg_color(btn_sysinfo, lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_text_color(btn_sysinfo, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        
        // 系统日志按钮未选中
        lv_obj_set_style_bg_color(btn_syslog, lv_color_hex(0xECF0F1), LV_PART_MAIN);
        lv_obj_set_style_text_color(btn_syslog, lv_color_hex(0x2C3E50), LV_PART_MAIN);
        
        // 更新右侧内容
        lv_label_set_text(content_label, 
            "📱 设备信息\n"
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
            "设备型号:    T113-S3\n\n"
            "内核版本:    Linux 5.4.61\n\n"
            "CPU架构:     ARM Cortex-A7 @ 1.2GHz\n\n"
            "CPU核心数:   双核\n\n"
            "内存信息:    128MB / 256MB (已用/总量)\n\n"
            "存储空间:    32MB / 128MB (已用/总量)\n\n"
            "系统运行:    5天 12小时 35分钟\n\n"
            "IP地址:      192.168.1.100\n\n"
            "MAC地址:     00:11:22:33:44:55\n\n"
            "LVGL版本:    v8.3.0\n\n"
            "编译时间:    2026-01-13 10:30:00\n"
        );
        
        printf("Switched to System Info tab\n");
    } 
    else if(selected_tab == 1) {
        // 系统日志按钮选中
        lv_obj_set_style_bg_color(btn_syslog, lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_text_color(btn_syslog, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        
        // 系统信息按钮未选中
        lv_obj_set_style_bg_color(btn_sysinfo, lv_color_hex(0xECF0F1), LV_PART_MAIN);
        lv_obj_set_style_text_color(btn_sysinfo, lv_color_hex(0x2C3E50), LV_PART_MAIN);
        
        // 更新右侧内容
        lv_label_set_text(content_label,
            "📋 系统日志\n"
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
            "[2026-01-13 10:30:01] 系统启动完成\n\n"
            "[2026-01-13 10:30:05] LVGL初始化成功\n\n"
            "[2026-01-13 10:30:10] 触摸屏校准完成\n\n"
            "[2026-01-13 10:30:15] WiFi模块初始化\n\n"
            "[2026-01-13 10:30:20] 连接到WiFi: Hunexi-2.4G\n\n"
            "[2026-01-13 10:30:25] 获取IP地址: 192.168.1.100\n\n"
            "[2026-01-13 10:30:30] 背光设置为50%\n\n"
            "[2026-01-13 10:30:35] 进入主菜单\n\n"
            "[2026-01-13 10:30:40] 音频系统就绪\n\n"
            "[2026-01-13 10:30:45] 所有模块加载完成\n"
        );
        
        printf("Switched to System Log tab\n");
    }
}

/**
 * @brief 系统信息按钮点击回调
 */
static void sysinfo_btn_event_cb(lv_event_t * e)
{
    update_button_state(0);
}

/**
 * @brief 系统日志按钮点击回调
 */
static void syslog_btn_event_cb(lv_event_t * e)
{
    update_button_state(1);
}

/**
 * @brief 创建顶部标题栏
 */
static void init_header_view(lv_obj_t *parent)
{
    // 创建顶部容器
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_set_size(header, 1424, 50);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(header, &com_style, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x34495E), LV_PART_MAIN);
    
    /* ========== 左侧：返回按钮 ========== */
    lv_obj_t *back_btn_cont = lv_obj_create(header);
    lv_obj_set_size(back_btn_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(back_btn_cont, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_style(back_btn_cont, &com_style, LV_PART_MAIN);
    lv_obj_add_flag(back_btn_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(back_btn_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(back_btn_cont, lv_color_hex(0x34495E), LV_PART_MAIN);
    
    lv_obj_t *back_img = lv_img_create(back_btn_cont);
    lv_img_set_src(back_img, GET_IMAGE_PATH("main/back.png"));
    lv_obj_align(back_img, LV_ALIGN_CENTER, 0, 0);
    
    lv_obj_add_event_cb(back_btn_cont, back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* ========== 中间：标题 ========== */
    lv_obj_t *title_label = lv_label_create(header);
    obj_font_set(title_label, FONT_TYPE_CN, 24);
    lv_label_set_text(title_label, "信息中心");
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

/**
 * @brief 创建左侧按钮区域
 */
static void init_left_buttons(lv_obj_t *parent)
{
    // 创建左侧容器
    lv_obj_t *left_cont = lv_obj_create(parent);
    lv_obj_set_size(left_cont, 400, 230);
    lv_obj_align(left_cont, LV_ALIGN_TOP_LEFT, 0, 50);
    lv_obj_add_style(left_cont, &com_style, LV_PART_MAIN);
    lv_obj_clear_flag(left_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(left_cont, lv_color_hex(0xF8F9FA), LV_PART_MAIN);
    lv_obj_set_style_pad_all(left_cont, 20, LV_PART_MAIN);
    
    /* ========== 系统信息按钮 ========== */
    btn_sysinfo = lv_btn_create(left_cont);
    lv_obj_set_size(btn_sysinfo, 200, 50);
    lv_obj_align(btn_sysinfo, LV_ALIGN_TOP_LEFT, 5, 10);
    lv_obj_set_style_radius(btn_sysinfo, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn_sysinfo, 5, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(btn_sysinfo, LV_OPA_30, LV_PART_MAIN);
    
    lv_obj_t *label_sysinfo = lv_label_create(btn_sysinfo);
    obj_font_set(label_sysinfo, FONT_TYPE_CN, 22);
    lv_label_set_text(label_sysinfo, "系统信息");
    lv_obj_center(label_sysinfo);
    
    lv_obj_add_event_cb(btn_sysinfo, sysinfo_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* ========== 系统日志按钮 ========== */
    btn_syslog = lv_btn_create(left_cont);
    lv_obj_set_size(btn_syslog, 200, 50);
    lv_obj_align(btn_syslog, LV_ALIGN_TOP_LEFT, 5, 65);
    lv_obj_set_style_radius(btn_syslog, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn_syslog, 5, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(btn_syslog, LV_OPA_30, LV_PART_MAIN);
    
    lv_obj_t *label_syslog = lv_label_create(btn_syslog);
    obj_font_set(label_syslog, FONT_TYPE_CN, 22);
    lv_label_set_text(label_syslog, "系统日志");
    lv_obj_center(label_syslog);
    
    lv_obj_add_event_cb(btn_syslog, syslog_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

/**
 * @brief 创建右侧内容显示区域
 */
static void init_right_content(lv_obj_t *parent)
{
    // 创建右侧容器
    lv_obj_t *right_cont = lv_obj_create(parent);
    lv_obj_set_size(right_cont, 1024, 230);
    lv_obj_align(right_cont, LV_ALIGN_TOP_RIGHT, 0, 50);
    lv_obj_add_style(right_cont, &com_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(right_cont, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(right_cont, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(right_cont, lv_color_hex(0xBDC3C7), LV_PART_MAIN);
    lv_obj_set_style_radius(right_cont, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(right_cont, 15, LV_PART_MAIN);
    
    // 创建可滚动的文本标签
    content_label = lv_label_create(right_cont);
    lv_obj_set_width(content_label, 984);
    obj_font_set(content_label, FONT_TYPE_CN, 18);
    lv_label_set_long_mode(content_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(content_label, lv_color_hex(0x2C3E50), LV_PART_MAIN);
    lv_obj_set_style_text_line_space(content_label, 8, LV_PART_MAIN);
    
    // 初始显示系统信息
    lv_label_set_text(content_label, 
        "📱 设备信息\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
        "请点击左侧按钮选择要查看的内容..."
    );
}

/**
 * @brief 初始化信息显示页面
 */
void init_pageInformation(void)
{
    printf("Initializing pageInformation...\n");
    
    // 初始化样式
    com_style_init();
    
    // 创建主容器
    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 1424, 280);
    lv_obj_add_style(main_cont, &com_style, LV_PART_MAIN);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(main_cont, lv_color_hex(0xECF0F1), LV_PART_MAIN);
    
    // 初始化顶部标题栏
    init_header_view(main_cont);
    
    // 初始化左侧按钮区域
    init_left_buttons(main_cont);
    
    // 初始化右侧内容显示区域
    init_right_content(main_cont);
    
    // 设置默认选中系统信息
    update_button_state(0);
    
    printf("pageInformation initialized successfully\n");
}
