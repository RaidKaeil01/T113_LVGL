
#include <stdio.h>
#include "lvgl.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"

static lv_style_t com_style;
static lv_obj_t * count_time_btn = NULL;
static lv_obj_t * count_time_label = NULL;
static lv_obj_t * timer_roller = NULL;
static lv_timer_t * refresh_timer = NULL;
static int alarm_time = 0;
static int time_count = 0;
static lv_obj_t * countdown_msgbox = NULL;
static lv_obj_t * countdown_label = NULL;

//初始化通用样式
static void com_style_init(){
    //初始化样式
    lv_style_init(&com_style);
    //判断如果样式非空，那就先重置，再设置
    if(lv_style_is_empty(&com_style) == false)
        lv_style_reset(&com_style);
    //样式背景设置为黑色，圆角设置为0，边框宽度设置为0，填充区域设置为0
    lv_style_set_bg_color(&com_style,lv_color_hex(0x000000));
    lv_style_set_radius(&com_style,0);
    lv_style_set_border_width(&com_style,0);
    lv_style_set_pad_all(&com_style,0);
}

//封装字库获取函数
static void obj_font_set(lv_obj_t *obj,int type, uint16_t weight){
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}

/**
 * @brief 清理闹钟页面资源
 */
void cleanup_pageClock(void)
{
    printf("Cleaning up pageClock resources...\n");
    
    // 1. 删除定时器
    if(refresh_timer != NULL) {
        lv_timer_del(refresh_timer);
        refresh_timer = NULL;
        printf("Refresh timer deleted\n");
    }
    
    // 2. 关闭弹窗（如果存在）
    if(countdown_msgbox != NULL) {
        lv_msgbox_close(countdown_msgbox);
        countdown_msgbox = NULL;
        countdown_label = NULL;
        printf("Countdown msgbox closed\n");
    }
    
    // 3. 移除屏幕上的所有事件回调
    lv_obj_remove_event_cb(lv_scr_act(), NULL);
    
    // 4. 清理通用样式
    if(lv_style_is_empty(&com_style) == false) {
        lv_style_reset(&com_style);
        printf("Style reset\n");
    }
    
    // 5. 清空全局变量
    count_time_btn = NULL;
    count_time_label = NULL;
    timer_roller = NULL;
    alarm_time = 0;
    time_count = 0;
    
    printf("pageClock cleanup completed\n");
}

static void refresh_timer_cb_func(lv_timer_t * timer)
{
    if(time_count > 0)
        time_count --;
    
    if(time_count == 0 && alarm_time > 0){
        // 倒计时结束，关闭弹窗（DELETE回调会自动清理状态）
        if(countdown_msgbox != NULL){
            lv_msgbox_close(countdown_msgbox);
        }
    }
    
    // 更新倒计时弹窗显示
    if(countdown_msgbox != NULL && countdown_label != NULL){
        int minutes = time_count / 60;
        int seconds = time_count % 60;
        lv_label_set_text_fmt(countdown_label, "%02d:%02d", minutes, seconds);
    }
    
    lv_label_set_text_fmt(count_time_label,"设置成功,%d分钟后提醒",alarm_time);
    if(alarm_time > 0){
        lv_label_set_text(lv_obj_get_child(count_time_btn,0),"取消");
        lv_obj_clear_flag(count_time_label,LV_OBJ_FLAG_HIDDEN);
    }else{
        lv_label_set_text(lv_obj_get_child(count_time_btn,0),"设置");
        lv_obj_add_flag(count_time_label,LV_OBJ_FLAG_HIDDEN);
    }
}  

// 弹窗删除事件回调 - 统一处理所有关闭场景
static void countdown_msgbox_delete_cb(lv_event_t * e){
    // 无论何种方式关闭弹窗，都会触发此回调
    countdown_msgbox = NULL;
    countdown_label = NULL;
    alarm_time = 0;
    time_count = 0;
    // 重置主界面按钮状态
    lv_label_set_text(lv_obj_get_child(count_time_btn,0),"设置");
    lv_obj_add_flag(count_time_label,LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief 返回按钮点击事件回调 - 返回菜单页面
 */
static void back_btn_click_event_cb(lv_event_t * e){
    printf("Back button clicked, returning to Menu page\n");
    
    // 清理当前页面资源
    cleanup_pageClock();
    
    // 清空屏幕
    lv_obj_clean(lv_scr_act());
    
    // 切换到菜单页面
    init_pageMenu();
}

static void select_btn_click_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    // 获取第一个子对象（为btn的label）
    lv_obj_t * label = lv_obj_get_child(btn, 0); 
    const char * text = lv_label_get_text(label);
    if (strcmp(text, "设置") == 0) { // 判断是否为"设置"
        //获取设置的时间0-60min
        alarm_time = lv_roller_get_selected(timer_roller);
        if(alarm_time == 0){
            printf("请选择大于0的时间\n");
            return;
        }
        //转换为秒
        time_count = alarm_time * 60;
        lv_label_set_text(label, "取消"); // 修改为"取消"
        
        // 创建倒计时弹窗
        static const char * btns[] = {"取消", ""};
        countdown_msgbox = lv_msgbox_create(NULL, "倒计时提醒", NULL, btns, true);
        lv_obj_set_size(countdown_msgbox, 400, 300);
        lv_obj_center(countdown_msgbox);
        // 设置弹窗背景为黑色
        lv_obj_set_style_bg_color(countdown_msgbox, lv_color_hex(0x000000), LV_PART_MAIN);
        lv_obj_set_style_border_width(countdown_msgbox, 0, LV_PART_MAIN);
        
        // 在弹窗中创建倒计时标签
        lv_obj_t * content = lv_msgbox_get_content(countdown_msgbox);
        countdown_label = lv_label_create(content);
        obj_font_set(countdown_label, FONT_TYPE_CN, 48);
        lv_obj_set_style_text_color(countdown_label, lv_color_hex(0x1F94D2), 0);
        int minutes = time_count / 60;
        int seconds = time_count % 60;
        lv_label_set_text_fmt(countdown_label, "%02d:%02d", minutes, seconds);
        lv_obj_center(countdown_label);
        
        // 添加说明文字
        lv_obj_t * desc_label = lv_label_create(content);
        obj_font_set(desc_label, FONT_TYPE_CN, 18);
        lv_obj_set_style_text_color(desc_label, lv_color_hex(0xffffff), 0);
        lv_label_set_text(desc_label, "剩余时间");
        lv_obj_align_to(desc_label, countdown_label, LV_ALIGN_OUT_TOP_MID, 0, -10);
        
        // 添加弹窗删除事件回调 - 统一处理所有关闭方式
        lv_obj_add_event_cb(countdown_msgbox, countdown_msgbox_delete_cb, LV_EVENT_DELETE, NULL);
        
    }else{
        // 点击取消按钮，关闭弹窗（DELETE回调会自动清理状态）
        if(countdown_msgbox != NULL){
            lv_msgbox_close(countdown_msgbox);
        }
    }
}
    

static lv_obj_t * init_title_view(lv_obj_t *parent){
    //初始化容器对象
    lv_obj_t * cont = lv_obj_create(parent);
    //设置大小 样式 对齐
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    //添加可点击标志
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);

    //初始化返回图像控件
    lv_obj_t *back_img = lv_img_create(cont);
    //设置显示的图片（返回图标）
    lv_img_set_src(back_img,GET_IMAGE_PATH("main/back.png"));
    //设置对齐方式为，父对象的左上角对齐
    lv_obj_set_align(back_img,LV_ALIGN_TOP_LEFT);
    //设置左侧、顶部填充距离为20
    lv_obj_set_style_pad_left(back_img,20,LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img,20,LV_PART_MAIN);

    //初始化菜单图像控件
    lv_obj_t *menu_img = lv_img_create(cont);
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_alarm.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    //设置控件对齐back_img，x轴偏移20
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    //初始化标签控件
    lv_obj_t *title = lv_label_create(cont);
    //设置字体样式和字体大小
    obj_font_set(title,FONT_TYPE_CN, 24);
    //设置显示内容
    lv_label_set_text(title,"闹铃设置");
    //设置颜色
    lv_obj_set_style_text_color(title,lv_color_hex(0xffffff),0);
    //设置对齐方式
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    //添加点击事件监听
    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);
    return cont;
}

static lv_obj_t * init_select_btn(lv_obj_t *parent){
    //初始化按钮控件
    lv_obj_t * btn = lv_btn_create(parent);
    //设置按钮样式
    lv_obj_add_style(btn,&com_style,LV_PART_MAIN);
    //设置按钮大小
    lv_obj_set_size(btn,171,66);
    //清除焦点状态
    lv_obj_clear_state(btn,LV_STATE_FOCUS_KEY);
    //设置边框、阴影为0
    lv_obj_set_style_border_width(btn, 0,LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0,LV_PART_MAIN);
    //设置圆角为35
    lv_obj_set_style_radius(btn,35,LV_PART_MAIN);
    //设置背景颜色为蓝色
    lv_obj_set_style_bg_color(btn,lv_color_hex(0x1F94D2),0);

    //初始化按钮显示文字
    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, 24);
    lv_obj_set_style_text_color(btn_label,lv_color_hex(0xffffff),0);
    lv_label_set_text(btn_label,"设置");
    //进行偏移对齐
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);
    return btn;
}

static lv_obj_t * init_setting_param_view(lv_obj_t *parent){
    //创建对象容器
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    //设置线性布局
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,10,LV_PART_MAIN);

    //创建图标控件
    lv_obj_t *setting_img = lv_img_create(cont);
    lv_img_set_src(setting_img,GET_IMAGE_PATH("icon_alarm.png"));
    lv_obj_set_style_pad_top(setting_img,10,LV_PART_MAIN);

    //创建滚轮
    lv_obj_t *roller_obj  = lv_roller_create(cont);
    timer_roller = roller_obj;
    //设置大小
    lv_obj_set_size(roller_obj, 50, 150);
    //清除焦点状态
    lv_obj_clear_state(roller_obj,LV_STATE_FOCUS_KEY);
    //设置边框宽度为0
    lv_obj_set_style_border_width(roller_obj,0,LV_PART_MAIN);
    //设置文字为白色、文字为黑色
    lv_obj_set_style_text_color(roller_obj,lv_color_hex(0xffffff),LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_obj,lv_color_hex(0x000000),LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_obj,lv_color_hex(0xffffff),LV_PART_SELECTED);
    lv_obj_set_style_bg_color(roller_obj,lv_color_hex(0x000000),LV_PART_SELECTED);
    //设置选项，非循环
    lv_roller_set_options(roller_obj,
                        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60",
                        LV_ROLLER_MODE_NORMAL);
    //设置选中项的字体大小
    lv_font_t* font_select = get_font(FONT_TYPE_CN_LIGHT, 36);
    lv_obj_set_style_text_font(roller_obj, font_select, LV_PART_SELECTED);
    //设置非选中项的字体大小
    lv_font_t* un_font_select = get_font(FONT_TYPE_CN_LIGHT, 28);
    lv_obj_set_style_text_font(roller_obj, un_font_select, LV_PART_MAIN);

    //创建标签
    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_CN, 24);
    lv_label_set_text(title,"分钟后提醒我");
    lv_obj_set_style_text_color(title,lv_color_hex(0xffffff),0);
    return cont;
}

void init_pageClock()
{
    printf("Initializing pageClock...\n");
    
    // 初始化样式
    com_style_init();
    
    // 创建页面对象
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);
    
    // ⚠️ 关键：禁用主容器的滚动和手势冒泡，防止误触发滑动返回
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 禁用屏幕对象的手势冒泡（防止其他页面的手势事件残留）
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 初始化标题界面
    lv_obj_t *title_view =  init_title_view(cont);
    //初始化闹铃设置界面
    lv_obj_t * time_view = init_setting_param_view(cont);
    lv_obj_align(time_view,LV_ALIGN_TOP_RIGHT,-230,50);

    //初始化设置按钮
    count_time_btn = init_select_btn(cont);
    lv_obj_align(count_time_btn,LV_ALIGN_TOP_RIGHT,-30,100);
    lv_obj_add_event_cb(count_time_btn,select_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    //初始化标签
    count_time_label = lv_label_create(cont);
    obj_font_set(count_time_label,FONT_TYPE_CN, 18);
    lv_obj_set_style_text_color(count_time_label,lv_color_hex(0xffffff),0);
    lv_label_set_text_fmt(count_time_label,"设置成功,%d分钟后提醒",alarm_time);
    lv_obj_align_to(count_time_label, count_time_btn,LV_ALIGN_OUT_BOTTOM_MID,0,10);
    lv_obj_add_flag(count_time_label,LV_OBJ_FLAG_HIDDEN);

    if(refresh_timer == NULL)
        refresh_timer = lv_timer_create((void*)refresh_timer_cb_func, 1000, NULL);
}