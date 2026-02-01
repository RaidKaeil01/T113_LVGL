#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "lvgl.h"
#include "page_conf.h"
#include "image_conf.h"
#include "font_conf.h"

/* ========== 文件路径定义 ========== */
#define RECORD_DIR "/usr/data/record"
#define RECORD_FILE_PATH "/usr/data/record/voice.wav"

/* ========== 全局变量 ========== */
static lv_style_t com_style;
static lv_obj_t *start_btn = NULL;
static lv_obj_t *stop_btn = NULL;
static lv_obj_t *play_btn = NULL;
static lv_obj_t *status_label = NULL;

// 录音状态
static bool is_recording = false;
static bool is_playing = false;
static pid_t record_pid = -1;
static pid_t play_pid = -1;

/**
 * @brief 初始化通用样式
 */
static void com_style_init(void)
{
    lv_style_init(&com_style);
    lv_style_set_bg_opa(&com_style, LV_OPA_0);
    lv_style_set_border_width(&com_style, 0);
    lv_style_set_pad_all(&com_style, 0);
    lv_style_set_radius(&com_style, 0);
}

/**
 * @brief 设置对象字体
 */
static void obj_font_set(lv_obj_t *obj, int type, uint16_t weight)
{
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}

/**
 * @brief 确保录音目录存在
 */
static void ensure_record_dir_exists(void)
{
    struct stat st = {0};
    if (stat(RECORD_DIR, &st) == -1) {
        mkdir(RECORD_DIR, 0755);
        printf("Created record directory: %s\n", RECORD_DIR);
    }
}

/**
 * @brief 更新状态标签
 */
static void update_status_label(const char *text)
{
    if(status_label != NULL) {
        lv_label_set_text(status_label, text);
    }
}

/**
 * @brief 返回按钮点击回调
 */
static void back_btn_event_cb(lv_event_t * e)
{
    printf("Back button clicked, returning to Menu page\n");
    
    // 清理当前页面资源
    cleanup_pageMicrophone();
    
    // 清空屏幕
    lv_obj_clean(lv_scr_act());
    
    // 返回菜单页面
    init_pageMenu();
}

/**
 * @brief 开始录音按钮回调
 */
static void start_btn_event_cb(lv_event_t * e)
{
    printf("Start recording button clicked\n");
    
    if(is_recording) {
        printf("Already recording\n");
        return;
    }
    
    if(is_playing) {
        printf("Cannot record while playing\n");
        return;
    }
    
    // 确保录音目录存在
    ensure_record_dir_exists();
    
    // 设置麦克风增益和音量
    system("amixer -c 0 cset numid=11 31");  // MIC3增益最大
    system("amixer -c 0 cset numid=8 255");  // ADC3音量最大
    system("amixer -c 0 cset numid=4 63");   // 数字音量最大
    printf("Audio input levels set to maximum\n");
    
    // 启动录音进程（后台运行）
    record_pid = fork();
    if(record_pid == 0) {
        // 子进程：执行录音命令
        // arecord -D hw:0,0 -c 1 -f S16_LE -r 48000 /usr/data/record/voice.wav
        execlp("arecord", "arecord",
               "-D", "hw:0,0",
               "-c", "1",
               "-f", "S16_LE",
               "-r", "48000",
               RECORD_FILE_PATH,
               NULL);
        // 如果execlp失败
        exit(1);
    } else if(record_pid > 0) {
        // 父进程
        is_recording = true;
        update_status_label("录音中...");
        printf("Recording started (PID: %d)\n", record_pid);
    } else {
        printf("Failed to start recording\n");
    }
}

/**
 * @brief 停止录音按钮回调
 */
static void stop_btn_event_cb(lv_event_t * e)
{
    printf("Stop recording button clicked\n");
    
    if(!is_recording) {
        printf("Not recording\n");
        return;
    }
    
    // 停止录音进程
    if(record_pid > 0) {
        char kill_cmd[64];
        snprintf(kill_cmd, sizeof(kill_cmd), "kill %d", record_pid);
        system(kill_cmd);
        record_pid = -1;
        is_recording = false;
        update_status_label("录音已停止");
        printf("Recording stopped\n");
    }
}

/**
 * @brief 播放录音按钮回调
 */
static void play_btn_event_cb(lv_event_t * e)
{
    printf("Play recording button clicked\n");
    
    if(is_recording) {
        printf("Cannot play while recording\n");
        return;
    }
    
    if(is_playing) {
        printf("Already playing\n");
        return;
    }
    
    // 检查录音文件是否存在
    if(access(RECORD_FILE_PATH, F_OK) != 0) {
        printf("No recording file found\n");
        update_status_label("未找到录音文件");
        return;
    }
    
    // 打印文件信息
    struct stat st;
    if(stat(RECORD_FILE_PATH, &st) == 0) {
        printf("Recording file size: %ld bytes\n", st.st_size);
    }
    
    // 设置播放音量到最大
    system("amixer -c 0 cset numid=17 7");   // 耳机音量最大
    system("amixer -c 0 cset numid=5 255,255"); // DAC音量最大
    system("amixer -c 0 cset numid=30 on");  // 确保耳机输出启用
    system("amixer -c 0 cset numid=31 on");  // 确保扬声器启用
    printf("Audio output levels set to maximum\n");
    
    // 启动播放进程（后台运行）
    play_pid = fork();
    if(play_pid == 0) {
        // 子进程：执行播放命令（添加-v参数显示详细信息）
        execlp("aplay", "aplay", "-v", RECORD_FILE_PATH, NULL);
        exit(1);
    } else if(play_pid > 0) {
        // 父进程
        is_playing = true;
        update_status_label("播放中...");
        printf("Playing started (PID: %d)\n", play_pid);
        
        // 等待播放完成（非阻塞）
        // 注意：这里简化处理，实际应该使用定时器轮询或信号处理
        // 播放完成后需要在其他地方重置 is_playing 状态
    } else {
        printf("Failed to start playing\n");
    }
}

/**
 * @brief 创建顶部标题栏
 */
static void init_header_view(lv_obj_t *parent)
{
    // 创建顶部容器
    lv_obj_t *header_cont = lv_obj_create(parent);
    lv_obj_set_size(header_cont, 1424, 40);
    lv_obj_align(header_cont, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(header_cont, &com_style, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header_cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(header_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建返回图标按钮
    lv_obj_t *back_btn = lv_img_create(header_cont);
    lv_img_set_src(back_btn, "A:res/image/main/back.png");
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 创建标题标签
    lv_obj_t *title_label = lv_label_create(header_cont);
    obj_font_set(title_label, FONT_TYPE_CN, 24);
    lv_label_set_text(title_label, "录音");
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x333333), LV_PART_MAIN);
}

/**
 * @brief 创建中央内容区（麦克风图标）
 */
static void init_content_view(lv_obj_t *parent)
{
    // 创建内容容器
    lv_obj_t *content_cont = lv_obj_create(parent);
    lv_obj_set_size(content_cont, 1424, 220);
    lv_obj_align(content_cont, LV_ALIGN_TOP_LEFT, 0, 60);
    lv_obj_add_style(content_cont, &com_style, LV_PART_MAIN);
    lv_obj_set_style_bg_color(content_cont, lv_color_hex(0xF5F5F5), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(content_cont, LV_OPA_100, LV_PART_MAIN);
    lv_obj_clear_flag(content_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建麦克风图标容器（居中显示）
    lv_obj_t *icon_cont = lv_obj_create(content_cont);
    lv_obj_set_size(icon_cont, 180, 180);
    lv_obj_align(icon_cont, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_bg_color(icon_cont, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_radius(icon_cont, 90, LV_PART_MAIN);
    lv_obj_set_style_border_width(icon_cont, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(icon_cont, lv_color_hex(0x3498DB), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(icon_cont, 15, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(icon_cont, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(icon_cont, LV_OPA_30, LV_PART_MAIN);
    lv_obj_clear_flag(icon_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建麦克风图标
    lv_obj_t *mic_icon = lv_img_create(icon_cont);
    lv_img_set_src(mic_icon, "A:res/image/menu/microphone.png");
    lv_obj_center(mic_icon);
    
    // 创建状态标签（显示录音/播放状态）
    status_label = lv_label_create(content_cont);
    obj_font_set(status_label, FONT_TYPE_CN, 18);
    lv_label_set_text(status_label, "就绪");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x2ECC71), LV_PART_MAIN);
    
    // 创建按钮容器（放在图标下方）
    lv_obj_t *btn_cont = lv_obj_create(content_cont);
    lv_obj_set_size(btn_cont,400, 50);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0,0);
    lv_obj_add_style(btn_cont, &com_style, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btn_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建"开始"图标按钮
    start_btn = lv_img_create(btn_cont);
    lv_img_set_src(start_btn, "A:res/image/music/music_pause.png");
    lv_obj_add_flag(start_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(start_btn, start_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 创建"暂停"图标按钮
    stop_btn = lv_img_create(btn_cont);
    lv_img_set_src(stop_btn, "A:res/image/music/music_start.png");
    lv_obj_add_flag(stop_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(stop_btn, stop_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 创建"播放"图标按钮
    play_btn = lv_img_create(btn_cont);
    lv_img_set_src(play_btn, "A:res/image/music/aplay.png");
    lv_obj_add_flag(play_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(play_btn, play_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

/**
 * @brief 清理录音页面资源
 */
void cleanup_pageMicrophone(void)
{
    printf("Cleaning up pageMicrophone resources...\n");
    
    // 停止录音进程（如果正在录音）
    if(is_recording && record_pid > 0) {
        char kill_cmd[64];
        snprintf(kill_cmd, sizeof(kill_cmd), "kill %d", record_pid);
        system(kill_cmd);
        record_pid = -1;
        is_recording = false;
        printf("Stopped recording process\n");
    }
    
    // 停止播放进程（如果正在播放）
    if(is_playing && play_pid > 0) {
        char kill_cmd[64];
        snprintf(kill_cmd, sizeof(kill_cmd), "kill %d", play_pid);
        system(kill_cmd);
        play_pid = -1;
        is_playing = false;
        printf("Stopped playing process\n");
    }
    
    // 清理样式
    if(lv_style_is_empty(&com_style) == false) {
        lv_style_reset(&com_style);
        printf("Style reset\n");
    }
    
    // 移除事件回调
    lv_obj_remove_event_cb(lv_scr_act(), NULL);
    
    // 清空按钮指针
    start_btn = NULL;
    stop_btn = NULL;
    play_btn = NULL;
    status_label = NULL;
    
    printf("pageMicrophone cleanup completed\n");
}

/**
 * @brief 初始化录音页面
 */
void init_pageMicrophone(void)
{
    printf("Initializing pageMicrophone...\n");
    
    // 重置状态
    is_recording = false;
    is_playing = false;
    record_pid = -1;
    play_pid = -1;
    
    // 初始化样式
    com_style_init();
    
    // 设置屏幕背景颜色
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xF5F5F5), LV_PART_MAIN);
    
    // 创建主容器
    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 1424, 280);
    lv_obj_add_style(main_cont, &com_style, LV_PART_MAIN);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // 初始化顶部标题栏
    init_header_view(main_cont);
    
    // 初始化中央内容区
    init_content_view(main_cont);
    
    printf("pageMicrophone initialized successfully\n");
}