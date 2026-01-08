#include <stdio.h>
#include "lvgl.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "music/audio_player_async.h"

/* ========== 静态变量 ========== */
static lv_style_t com_style;
static lv_obj_t * play_btn = NULL;           // 播放/暂停按钮
static lv_obj_t * prev_btn = NULL;           // 上一曲按钮
static lv_obj_t * next_btn = NULL;           // 下一曲按钮
static lv_obj_t * progress_bar = NULL;       // 进度条
static lv_obj_t * song_title_label = NULL;   // 歌曲标题
static lv_obj_t * artist_label = NULL;       // 艺术家
static lv_obj_t * current_time_label = NULL; // 当前时间
static lv_obj_t * total_time_label = NULL;   // 总时间
static lv_obj_t * album_cover = NULL;        // 专辑封面
static lv_obj_t * main_container = NULL;     // 主容器
static bool is_playing = false;              // 播放状态
static lv_timer_t * update_timer = NULL;     // 进度更新定时器

/* ========== 音频文件路径配置 ========== */
#ifdef SIMULATOR_LINUX
    #define AUDIO_PATH_START "./res/music/audio_start.wav"
    #define AUDIO_PATH_WARN  "./res/music/audio_warn2.wav"
#else
    #define AUDIO_PATH_START "/usr/res/music/audio_start.wav"
    #define AUDIO_PATH_WARN  "/usr/res/music/audio_warn2.wav"
#endif

/* ========== 当前播放的音频索引 ========== */
static int current_track = 0;  // 0: audio_start, 1: audio_warn2

/* ========== 函数前向声明 ========== */
static lv_obj_t * create_control_btn(lv_obj_t *parent, const char *img_path, int size);
static void update_song_info(void);
static void play_current_track(void);

/* ========== 初始化通用样式 ========== */
static void com_style_init(void)
{
    lv_style_init(&com_style);
    if(lv_style_is_empty(&com_style) == false)
        lv_style_reset(&com_style);
    
    lv_style_set_bg_color(&com_style, lv_color_hex(0x000000));
    lv_style_set_radius(&com_style, 0);
    lv_style_set_border_width(&com_style, 0);
    lv_style_set_pad_all(&com_style, 0);
}

/* ========== 封装字库获取函数 ========== */
static void obj_font_set(lv_obj_t *obj, int type, uint16_t weight)
{
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}

/* ========== 清理音乐页面资源 ========== */
void cleanup_pageMusic(void)
{
    printf("Cleaning up pageMusic resources...\n");
    
    // 删除定时器
    if(update_timer != NULL) {
        lv_timer_del(update_timer);
        update_timer = NULL;
        printf("Update timer deleted\n");
    }
    
    // 清理通用样式
    if(lv_style_is_empty(&com_style) == false) {
        lv_style_reset(&com_style);
        printf("Common style reset\n");
    }
    
    // 清空全局对象指针
    play_btn = NULL;
    prev_btn = NULL;
    next_btn = NULL;
    progress_bar = NULL;
    song_title_label = NULL;
    artist_label = NULL;
    current_time_label = NULL;
    total_time_label = NULL;
    album_cover = NULL;
    main_container = NULL;
    is_playing = false;
    
    printf("pageMusic cleanup completed\n");
}

/* ========== 返回按钮点击事件 ========== */
static void back_btn_click_event_cb(lv_event_t * e)
{
    printf("Back button clicked, returning to Menu page\n");
    
    // 清理当前页面资源
    cleanup_pageMusic();
    
    // 清空屏幕
    lv_obj_clean(lv_scr_act());
    
    // 切换到菜单页面
    init_pageMenu();
}

/* ========== 更新歌曲信息显示 ========== */
static void update_song_info(void)
{
    if(song_title_label != NULL) {
        if(current_track == 0) {
            lv_label_set_text(song_title_label, "启动音效");
        } else {
            lv_label_set_text(song_title_label, "警告音效");
        }
    }
    
    if(artist_label != NULL) {
        lv_label_set_text(artist_label, "系统音频");
    }
}

/* ========== 播放当前曲目 ========== */
static void play_current_track(void)
{
    const char *audio_path = (current_track == 0) ? AUDIO_PATH_START : AUDIO_PATH_WARN;
    
    printf("Playing track %d: %s\n", current_track, audio_path);
    start_play_audio_async(audio_path);
    
    // 更新歌曲信息显示
    update_song_info();
}

/* ========== 播放/暂停按钮点击事件 ========== */
static void play_btn_click_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * img = lv_obj_get_child(btn, 0);
    
    is_playing = !is_playing;
    
    if(is_playing) {
        lv_img_set_src(img, GET_IMAGE_PATH("music/music_start.png"));
        printf("Music: Playing\n");
        
        play_current_track();
    } else {
        lv_img_set_src(img, GET_IMAGE_PATH("music/music_pause.png"));
        printf("Music: Paused\n");
        
        // 停止播放（通过播放空音频实现暂停效果）
        // 注意：当前audio_player_async没有STOP命令实现，实际会停止当前播放
    }
}

/* ========== 上一曲按钮点击事件 ========== */
static void prev_btn_click_event_cb(lv_event_t * e)
{
    printf("Music: Previous track\n");
    
    // 切换到上一曲
    current_track = (current_track == 0) ? 1 : 0;
    
    // 如果正在播放，则播放新曲目
    if(is_playing) {
        play_current_track();
    } else {
        // 只更新显示信息
        update_song_info();
    }
}

/* ========== 下一曲按钮点击事件 ========== */
static void next_btn_click_event_cb(lv_event_t * e)
{
    printf("Music: Next track\n");
    
    // 切换到下一曲
    current_track = (current_track == 0) ? 1 : 0;
    
    // 如果正在播放，则播放新曲目
    if(is_playing) {
        play_current_track();
    } else {
        // 只更新显示信息
        update_song_info();
    }
}

/* ========== 初始化标题栏 ========== */
static lv_obj_t * init_title_view(lv_obj_t *parent)
{
    // 创建容器
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    lv_obj_set_align(cont, LV_ALIGN_TOP_MID);
    
    // 禁用容器的滑动和滚动，防止误触发返回
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 返回图标
    lv_obj_t *back_img = lv_img_create(cont);
    lv_img_set_src(back_img, GET_IMAGE_PATH("main/back.png"));
    lv_obj_set_align(back_img, LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_left(back_img, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img, 20, LV_PART_MAIN);
    
    // 为返回图标添加点击区域
    lv_obj_add_flag(back_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back_img, back_btn_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 标题文字
    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title, FONT_TYPE_CN, 24);
    lv_label_set_text(title, "音乐播放");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_obj_align_to(title, back_img, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    
    return cont;
}

/* ========== 初始化专辑封面区域 ========== */
static lv_obj_t * init_album_cover_view(lv_obj_t *parent)
{
    // 创建圆形容器作为专辑封面背景（适配280高度屏幕）
    lv_obj_t * cover_bg = lv_obj_create(parent);
    lv_obj_set_size(cover_bg, 150, 150);
    lv_obj_set_style_radius(cover_bg, 50, LV_PART_MAIN);  // 圆形
    lv_obj_set_style_bg_color(cover_bg, lv_color_hex(0x1F94D2), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cover_bg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(cover_bg, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(cover_bg, lv_color_hex(0x3498DB), LV_PART_MAIN);
    lv_obj_clear_flag(cover_bg, LV_OBJ_FLAG_SCROLLABLE);
    
    // 在圆形背景中添加音乐符号图标
    lv_obj_t * music_symbol = lv_label_create(cover_bg);
    lv_label_set_text(music_symbol, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_font(music_symbol, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(music_symbol, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_center(music_symbol);
    
    album_cover = cover_bg;
    return cover_bg;
}

/* ========== 初始化歌曲信息区域 ========== */
static lv_obj_t * init_song_info_view(lv_obj_t *parent)
{
    // 创建容器
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 450,260);
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 10, LV_PART_MAIN);
    
    // 测试用边框 - 改为蓝色以便观察
    lv_obj_set_style_border_width(cont, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x0000FF), LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x0000FF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_30, LV_PART_MAIN);
    
    // 歌曲标题
    song_title_label = lv_label_create(cont);
    obj_font_set(song_title_label, FONT_TYPE_CN, 18);
    lv_label_set_text(song_title_label, "未知歌曲");
    lv_obj_set_style_text_color(song_title_label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_label_set_long_mode(song_title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(song_title_label, 420);
    lv_obj_set_style_text_align(song_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    
    // 艺术家名称
    artist_label = lv_label_create(cont);
    obj_font_set(artist_label, FONT_TYPE_CN, 16);
    lv_label_set_text(artist_label, "未知艺术家");
    lv_obj_set_style_text_color(artist_label, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_align(artist_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    
    return cont;
}

/* ========== 初始化进度条区域 ========== */
static lv_obj_t * init_progress_view(lv_obj_t *parent)
{
    // 创建容器
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(cont, 5, LV_PART_MAIN);
    
    // 进度条
    progress_bar = lv_slider_create(cont);
    lv_obj_set_size(progress_bar, 400, 4);
    lv_obj_align(progress_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_slider_set_range(progress_bar, 0, 100);
    lv_slider_set_value(progress_bar, 35, LV_ANIM_OFF);  // 示例进度
    
    // 进度条样式
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x1F94D2), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x1F94D2), LV_PART_KNOB);
    lv_obj_set_style_pad_all(progress_bar, 3, LV_PART_KNOB);
    
    // 当前播放时间
    current_time_label = lv_label_create(cont);
    obj_font_set(current_time_label, FONT_TYPE_CN, 12);
    lv_label_set_text(current_time_label, "01:25");
    lv_obj_set_style_text_color(current_time_label, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align_to(current_time_label, progress_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    
    // 总时长
    total_time_label = lv_label_create(cont);
    obj_font_set(total_time_label, FONT_TYPE_CN, 12);
    lv_label_set_text(total_time_label, "03:45");
    lv_obj_set_style_text_color(total_time_label, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align_to(total_time_label, progress_bar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);
    
    // ===== 控制按钮区域(在进度条下方)=====
    // 上一曲按钮(32x32图标)
    prev_btn = create_control_btn(cont, GET_IMAGE_PATH("music/music_left.png"), 40);
    lv_obj_t * prev_img = lv_obj_get_child(prev_btn, 0);
    lv_img_set_angle(prev_img, 1800);  // 旋转108度(LVGL使用0.1度为单位)
    lv_obj_align_to(prev_btn, progress_bar, LV_ALIGN_OUT_BOTTOM_MID, -60, 25);
    lv_obj_add_event_cb(prev_btn, prev_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 播放/暂停按钮（32x32图标）
    play_btn = create_control_btn(cont, GET_IMAGE_PATH("music/music_pause.png"), 50);
    lv_obj_align_to(play_btn, progress_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_add_event_cb(play_btn, play_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 下一曲按钮（32x32图标）
    next_btn = create_control_btn(cont, GET_IMAGE_PATH("music/music_right.png"), 40);
    lv_obj_align_to(next_btn, progress_bar, LV_ALIGN_OUT_BOTTOM_MID, 60, 25);
    lv_obj_add_event_cb(next_btn, next_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    
    return cont;
}

/* ========== 创建控制按钮 ========== */
static lv_obj_t * create_control_btn(lv_obj_t *parent, const char *img_path, int size)
{
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_set_size(btn, size, size);
    lv_obj_set_style_radius(btn, size / 2, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_clear_state(btn, LV_STATE_FOCUS_KEY);
    
    // 按下效果
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_PRESSED);
    
    // 添加图标
    lv_obj_t * img = lv_img_create(btn);
    lv_img_set_src(img, img_path);
    lv_obj_center(img);
    
    return btn;
}

/* ========== 初始化控制按钮区域 ========== */
static lv_obj_t * init_control_view(lv_obj_t *parent)
{
    // 创建容器
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 200, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(cont, 5, LV_PART_MAIN);
    
    // 上一曲按钮（32x32图标，按钮稍大一点方便点击）
    prev_btn = create_control_btn(cont, GET_IMAGE_PATH("music/music_left.png"), 45);
    lv_obj_add_event_cb(prev_btn, prev_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 播放/暂停按钮（32x32图标）
    play_btn = create_control_btn(cont, GET_IMAGE_PATH("music/music_pause.png"), 55);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x1F94D2), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(play_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(play_btn, play_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 下一曲按钮（32x32图标）
    next_btn = create_control_btn(cont, GET_IMAGE_PATH("music/music_right.png"), 45);
    lv_obj_add_event_cb(next_btn, next_btn_click_event_cb, LV_EVENT_CLICKED, NULL);
    
    return cont;
}

/* ========== 初始化音乐播放器页面 ========== */
void init_pageMusic(void)
{
    printf("Initializing pageMusic...\n");
    
    // 初始化音频播放器（如果尚未初始化）
    static bool audio_inited = false;
    if(!audio_inited) {
        if(init_async_audio_player() == 0) {
            printf("Audio player initialized successfully\n");
            audio_inited = true;
        } else {
            printf("Failed to initialize audio player\n");
        }
    }
    
    // 初始化样式
    com_style_init();
    
    // 创建主容器
    main_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_container, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(main_container, &com_style, LV_PART_MAIN);
    
    // 禁用主容器的滚动和手势冒泡，防止误触发滑动返回
    lv_obj_clear_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(main_container, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 初始化标题栏（保留返回功能）
    lv_obj_t * title_view = init_title_view(main_container);
    
    // 显示进度条区域，靠近底部
    lv_obj_t * progress_view = init_progress_view(main_container);
    lv_obj_align(progress_view, LV_ALIGN_BOTTOM_MID, 0, 5);
    
    // 显示专辑封面，在进度条正上方
    lv_obj_t * album_view = init_album_cover_view(main_container);
    lv_obj_align_to(album_view, progress_view, LV_ALIGN_OUT_TOP_MID, 0, -30);
    
    // 显示歌曲信息区域，在进度条右侧
    lv_obj_t * song_info_view = init_song_info_view(main_container);
    lv_obj_align_to(song_info_view, progress_bar, LV_ALIGN_OUT_RIGHT_MID, 40, -70);
    
    // 更新初始歌曲信息
    update_song_info();
    
    printf("pageMusic initialized successfully\n");
}
