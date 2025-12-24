/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:06:34
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"

static lv_style_t com_style;

static lv_obj_t * time_view_btn = NULL;
static lv_obj_t * time_view_label = NULL;
static lv_obj_t * hm_time_view_btn = NULL;
static lv_obj_t * hm_time_view_label = NULL;
static lv_timer_t * refresh_timer = NULL;
static bool is_cancle = false;

typedef struct{
    char title[50];
    char *img_url;
    char param[300];
    lv_obj_t * obj;
}alarm_setting_param_t;

static alarm_setting_param_t alarm_setting_param[] = {
    {"分钟后提醒我",GET_IMAGE_PATH("icon_alarm.png"),"0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60",NULL},
    {"",GET_IMAGE_PATH("icon_alarm.png"),"00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24",NULL},
    {"时提醒我",GET_IMAGE_PATH("icon_alarm.png"),"00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60",NULL},
};

static void com_style_init(){
    lv_style_init(&com_style);
    if(lv_style_is_empty(&com_style) == false)
        lv_style_reset(&com_style);
    lv_style_set_bg_color(&com_style,APP_COLOR_BLACK);
    lv_style_set_radius(&com_style,0);
    lv_style_set_border_width(&com_style,0);
    lv_style_set_pad_all(&com_style,0);
}

static void obj_font_set(lv_obj_t *obj,int type, uint16_t weight){
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, 0);
}

static void refresh_timer_cb_func(lv_timer_t * timer)
{
    if(is_cancle == true)
        return;
    device_state_t* device_state = get_device_state();
    lv_label_set_text_fmt(time_view_label,"设置成功,%d分钟后提醒",device_state->alarm_time1);
    if(device_state->alarm_time2 >= 60){
        int hour = device_state->alarm_time2 / 60;
        int min = device_state->alarm_time2 % 60;
        lv_label_set_text_fmt(hm_time_view_label,"设置成功,%d小时%d分钟后提醒",hour,min);
    }else{
        lv_label_set_text_fmt(hm_time_view_label,"设置成功,%d分钟后提醒",device_state->alarm_time2);
    }
    lv_obj_align_to(time_view_label, time_view_btn,LV_ALIGN_OUT_BOTTOM_MID,0,10);
    lv_obj_align_to(hm_time_view_label, hm_time_view_btn,LV_ALIGN_OUT_BOTTOM_MID,0,10);

    if(device_state->is_open_type1){
        lv_label_set_text(lv_obj_get_child(time_view_btn,0),"取消");
        lv_obj_clear_flag(time_view_label,LV_OBJ_FLAG_HIDDEN);
    }else{
        lv_label_set_text(lv_obj_get_child(time_view_btn,0),"设置");
        lv_obj_add_flag(time_view_label,LV_OBJ_FLAG_HIDDEN);
    }
    if(device_state->is_open_type2){
        lv_label_set_text(lv_obj_get_child(hm_time_view_btn,0),"取消");
        lv_obj_clear_flag(hm_time_view_label,LV_OBJ_FLAG_HIDDEN);
    }else{
        lv_label_set_text(lv_obj_get_child(hm_time_view_btn,0),"设置");
        lv_obj_add_flag(hm_time_view_label,LV_OBJ_FLAG_HIDDEN);
    }
}   

static void back_btn_click_event_cb(lv_event_t * e){
    is_cancle = true;
    delete_current_page(&com_style);
    init_page_main();
}

static void roller_event_handler(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        // char buf[10];
        // lv_roller_get_selected_str(obj, buf, sizeof(buf));
        // uint16_t roller_index = lv_roller_get_selected(obj);
    }
}

static void select_btn_click_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    device_state_t* device_state = get_device_state();
    uint16_t roller_index = lv_roller_get_selected(alarm_setting_param[0].obj);
    if(get_device_state()->is_open_type1){
        get_device_state()->is_open_type1 = false;
        lv_label_set_text(lv_obj_get_child(time_view_btn,0),"设置");
    }else{
        if(roller_index == 0)
            return;
        printf("select_btn_click_event_cb roller = %d\n",roller_index);
        get_device_state()->alarm_time1 = roller_index;
        get_device_state()->is_open_type1 = true;
        lv_label_set_text(lv_obj_get_child(time_view_btn,0),"取消");
    }
}

static void select_hm_btn_click_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    device_state_t* device_state = get_device_state();
    uint16_t roller2_index = lv_roller_get_selected(alarm_setting_param[1].obj);
    uint16_t roller3_index = lv_roller_get_selected(alarm_setting_param[2].obj);
    if(get_device_state()->is_open_type2){
        get_device_state()->is_open_type2 = false;
        lv_label_set_text(lv_obj_get_child(hm_time_view_btn,0),"设置");
    }else{
        printf("select_hm_btn_click_event_cb roller = %d:%d\n",roller2_index,roller3_index);
        get_device_state()->alarm_time2 = roller2_index*60+roller3_index;
        get_device_state()->is_open_type2 = true;
        lv_label_set_text(lv_obj_get_child(hm_time_view_btn,0),"取消");
    }
}

static lv_obj_t * init_title_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *back_img = lv_img_create(cont);
    lv_img_set_src(back_img,GET_IMAGE_PATH("icon_back.png"));
    lv_obj_set_align(back_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_left(back_img,20,LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img,20,LV_PART_MAIN);

    lv_obj_t *menu_img = lv_img_create(cont);
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_alarm.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"设置");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    return cont;
}

static lv_obj_t * init_select_btn(lv_obj_t *parent,bool is_open){
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_style(btn,&com_style,0);
    lv_obj_set_size(btn,171,66);
    lv_obj_clear_state(btn,LV_STATE_FOCUS_KEY);
    lv_obj_set_style_border_width(btn, 0,0);
    lv_obj_set_style_shadow_width(btn, 0,0);
    lv_obj_set_style_radius(btn,35,0);
    lv_obj_set_style_bg_color(btn,APP_COLOR_BUTTON_DEFALUT,0);
    // lv_obj_set_style_opa(btn,LV_OPA_80,LV_STATE_PRESSED);

    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    if(is_open){
        lv_label_set_text(btn_label,"取消");
    }else
        lv_label_set_text(btn_label,"设置");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

static lv_obj_t * init_setting_param_view(lv_obj_t *parent,int type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    // lv_obj_set_style_bg_color(cont,APP_COLOR_BUTTON_UNSELECT,0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    //弹性布局对齐方式
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,10,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    lv_obj_t *setting_img = lv_img_create(cont);
    lv_img_set_src(setting_img,alarm_setting_param[type].img_url);
    lv_obj_set_style_pad_top(setting_img,10,LV_PART_MAIN);

    lv_obj_t *roller_obj  = lv_roller_create(cont);
    alarm_setting_param[type].obj = roller_obj;
    lv_obj_set_style_border_width(roller_obj,0,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,LV_PART_SELECTED);
    lv_obj_set_style_bg_color(roller_obj,lv_color_black(),0);
    lv_obj_set_style_bg_opa(roller_obj, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_obj,LV_OPA_TRANSP,LV_PART_MAIN);
    lv_roller_set_options(roller_obj,
                        alarm_setting_param[type].param,
                        LV_ROLLER_MODE_NORMAL);
    lv_obj_add_event_cb(roller_obj, roller_event_handler, LV_EVENT_ALL, NULL);
    lv_font_t* font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_1);
    if(font_select != NULL)
        lv_obj_set_style_text_font(roller_obj, font_select, LV_PART_SELECTED);
    lv_font_t* un_font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_3);
    if(un_font_select != NULL)
        lv_obj_set_style_text_font(roller_obj, un_font_select, 0);
    lv_obj_set_style_text_line_space(roller_obj,10,0);
    lv_obj_set_height(roller_obj,400);
    lv_obj_set_width(roller_obj,50);
    lv_roller_set_visible_row_count(roller_obj, 3);
    lv_roller_set_selected(roller_obj,1,LV_ANIM_ON);
    lv_obj_clear_state(roller_obj,LV_STATE_FOCUS_KEY);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,alarm_setting_param[type].title);
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    return cont;
}

static lv_obj_t * init_setting_param_view_hm(lv_obj_t *parent,int type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    // lv_obj_set_style_bg_color(cont,APP_COLOR_BUTTON_UNSELECT,0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    //弹性布局对齐方式
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,5,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    lv_obj_t *setting_img = lv_img_create(cont);
    lv_img_set_src(setting_img,alarm_setting_param[type].img_url);
    lv_obj_set_style_pad_top(setting_img,10,LV_PART_MAIN);

    lv_obj_t *roller_obj_h  = lv_roller_create(cont);
    alarm_setting_param[type].obj = roller_obj_h;
    lv_obj_set_style_border_width(roller_obj_h,0,0);
    lv_obj_set_style_text_color(roller_obj_h,APP_COLOR_WHITE,0);
    lv_obj_set_style_text_color(roller_obj_h,APP_COLOR_WHITE,LV_PART_SELECTED);
    lv_obj_set_style_bg_color(roller_obj_h,lv_color_black(),0);
    lv_obj_set_style_bg_opa(roller_obj_h, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_obj_h,LV_OPA_TRANSP,LV_PART_MAIN);
    lv_roller_set_options(roller_obj_h,
                        alarm_setting_param[type].param,
                        LV_ROLLER_MODE_NORMAL);
    lv_obj_add_event_cb(roller_obj_h, roller_event_handler, LV_EVENT_ALL, NULL);
    lv_font_t* font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_1);
    if(font_select != NULL)
        lv_obj_set_style_text_font(roller_obj_h, font_select, LV_PART_SELECTED);
    lv_font_t* un_font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_3);
    if(un_font_select != NULL)
        lv_obj_set_style_text_font(roller_obj_h, un_font_select, 0);
    lv_obj_set_style_text_line_space(roller_obj_h,10,0);
    lv_obj_set_height(roller_obj_h,400);
    lv_obj_set_width(roller_obj_h,50);
    lv_roller_set_visible_row_count(roller_obj_h, 3);
    lv_roller_set_selected(roller_obj_h,9,LV_ANIM_ON);
    lv_obj_clear_state(roller_obj_h,LV_STATE_FOCUS_KEY);

    lv_obj_t *symbol = lv_label_create(cont);
    obj_font_set(symbol,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(symbol,":");
    lv_obj_set_style_text_color(symbol,APP_COLOR_WHITE,0);

    lv_obj_t *roller_obj_m  = lv_roller_create(cont);
    alarm_setting_param[type+1].obj = roller_obj_m;
    lv_obj_set_style_border_width(roller_obj_m,0,0);
    lv_obj_set_style_text_color(roller_obj_m,APP_COLOR_WHITE,0);
    lv_obj_set_style_text_color(roller_obj_m,APP_COLOR_WHITE,LV_PART_SELECTED);
    lv_obj_set_style_bg_color(roller_obj_m,lv_color_black(),0);
    lv_obj_set_style_bg_opa(roller_obj_m, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_obj_m,LV_OPA_TRANSP,LV_PART_MAIN);
    lv_roller_set_options(roller_obj_m,
                        alarm_setting_param[type+1].param,
                        LV_ROLLER_MODE_NORMAL);
    lv_obj_add_event_cb(roller_obj_m, roller_event_handler, LV_EVENT_ALL, NULL);
    if(font_select != NULL)
        lv_obj_set_style_text_font(roller_obj_m, font_select, LV_PART_SELECTED);
    if(un_font_select != NULL)
        lv_obj_set_style_text_font(roller_obj_m, un_font_select, 0);
    lv_obj_set_style_text_line_space(roller_obj_m,10,0);
    lv_obj_set_height(roller_obj_m,400);
    lv_obj_set_width(roller_obj_m,50);
    lv_roller_set_visible_row_count(roller_obj_m, 3);
    lv_roller_set_selected(roller_obj_m,19,LV_ANIM_ON);
    lv_obj_clear_state(roller_obj_m,LV_STATE_FOCUS_KEY);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,alarm_setting_param[type+1].title);
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    return cont;
}

void init_page_alarm_clock_setting()
{
    is_cancle = false;
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_tomato_time.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);
    
    lv_obj_t * time_view = init_setting_param_view(cont,0);
    lv_obj_align_to(time_view,title_view,LV_ALIGN_OUT_BOTTOM_LEFT,200,10);
    time_view_btn = init_select_btn(cont,get_device_state()->is_open_type1);
    lv_obj_align_to(time_view_btn,time_view,LV_ALIGN_OUT_RIGHT_MID,30,5);
    lv_obj_add_event_cb(time_view_btn,select_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    time_view_label = lv_label_create(cont);
    obj_font_set(time_view_label,FONT_TYPE_CN, FONT_SIZE_TEXT_3);
    lv_obj_set_style_text_color(time_view_label,APP_COLOR_WHITE,0);
    lv_obj_align_to(time_view_label, time_view_btn,LV_ALIGN_OUT_BOTTOM_MID,0,10);
    lv_obj_add_flag(time_view_label,LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * hm_time_view = init_setting_param_view_hm(cont,1);
    lv_obj_align_to(hm_time_view,time_view,LV_ALIGN_OUT_RIGHT_MID,350,0);
    hm_time_view_btn = init_select_btn(cont,get_device_state()->is_open_type2);
    lv_obj_align_to(hm_time_view_btn,hm_time_view,LV_ALIGN_OUT_RIGHT_MID,30,5);
    lv_obj_add_event_cb(hm_time_view_btn,select_hm_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    hm_time_view_label = lv_label_create(cont);
    obj_font_set(hm_time_view_label,FONT_TYPE_CN, FONT_SIZE_TEXT_3);
    lv_obj_set_style_text_color(hm_time_view_label,APP_COLOR_WHITE,0);
    lv_obj_align_to(hm_time_view_label, hm_time_view_btn,LV_ALIGN_OUT_BOTTOM_MID,0,10);
    lv_obj_add_flag(hm_time_view_label,LV_OBJ_FLAG_HIDDEN);

    if(refresh_timer == NULL)
        refresh_timer = lv_timer_create((void*)refresh_timer_cb_func, 1000, NULL);

}