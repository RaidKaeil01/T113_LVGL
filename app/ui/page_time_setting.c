/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 01:46:05
 */
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "lv_clock.h"
#include "http_manager.h"
#include "em_hal_time.h"

static lv_style_t com_style;

static lv_obj_t * roller_obj = NULL;
static uint16_t roller_index = 1;

static lv_obj_t * show_view = NULL;

static lv_obj_t *clock0_img = NULL;
static lv_obj_t *clock1_img = NULL;
static lv_obj_t *clock2_img = NULL;

static lv_clock_t lv_clock0;
static lv_clock_t lv_clock1;
static lv_clock_t lv_clock2;

static int clock_type = 0;

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

static void clean_show_view(lv_obj_t *parent){
    printf("clean_show_view");
    lv_obj_t * act_scr = parent;
    lv_disp_t * d = lv_obj_get_disp(act_scr);
	if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
	{
		lv_obj_clean(act_scr);
	}
}

static void clock0_type_click_event_cb(lv_event_t * e){
    clock_type = 0;
    lv_img_set_src(clock0_img,GET_IMAGE_PATH("icon_select.png"));
    lv_img_set_src(clock1_img,GET_IMAGE_PATH("icon_unselect.png"));
    lv_img_set_src(clock2_img,GET_IMAGE_PATH("icon_unselect.png"));
} 

static void clock1_type_click_event_cb(lv_event_t * e){
    clock_type = 1;
    lv_img_set_src(clock0_img,GET_IMAGE_PATH("icon_unselect.png"));
    lv_img_set_src(clock1_img,GET_IMAGE_PATH("icon_select.png"));
    lv_img_set_src(clock2_img,GET_IMAGE_PATH("icon_unselect.png"));
}

static void clock2_type_click_event_cb(lv_event_t * e){
    clock_type = 2;
    lv_img_set_src(clock0_img,GET_IMAGE_PATH("icon_unselect.png"));
    lv_img_set_src(clock1_img,GET_IMAGE_PATH("icon_unselect.png"));
    lv_img_set_src(clock2_img,GET_IMAGE_PATH("icon_select.png"));
}

static void init_item_view(lv_obj_t *parent,TIME_SHOW_TYPE_E type){
    if(type == TIME_TYPE_1){
        lv_obj_t *time_obj = lv_label_create(parent);
        obj_font_set(time_obj,FONT_TYPE_LETTER, FONT_SIZE_TIME_1);
        lv_label_set_text(time_obj,"10:20");
        lv_obj_set_style_text_color(time_obj,APP_COLOR_WHITE,0);
        lv_obj_align(time_obj,LV_ALIGN_TOP_MID,0,50);

        lv_obj_t *weather_obj = lv_label_create(parent);
        obj_font_set(weather_obj,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
        lv_label_set_text(weather_obj,"广州 多云 26°C");
        lv_obj_set_style_text_color(weather_obj,APP_COLOR_WHITE,0);
        lv_obj_align_to(weather_obj,time_obj,LV_ALIGN_OUT_BOTTOM_MID,0,10);
    }else if(type == TIME_TYPE_2){
        lv_obj_t *time_obj = lv_label_create(parent);
        obj_font_set(time_obj,FONT_TYPE_NUMBER, FONT_SIZE_MAX);
        lv_label_set_text(time_obj,"10:20");
        lv_obj_set_style_text_color(time_obj,APP_COLOR_WHITE,0);
        lv_obj_align(time_obj,LV_ALIGN_CENTER,0,0);
    }else if(type == TIME_TYPE_3){
        lv_obj_t *clock0 = init_clock0_obj(parent,&lv_clock0);
        clock0_img = lv_img_create(parent);
        lv_img_set_src(clock0_img,GET_IMAGE_PATH("icon_select.png"));
        lv_obj_align_to(clock0_img,clock0,LV_ALIGN_OUT_BOTTOM_MID,0,0);
        lv_obj_set_style_pad_top(clock0_img,10,LV_PART_MAIN);
        
        lv_obj_t *clock1 = init_clock1_obj(parent,&lv_clock1);
        lv_obj_align_to(clock1,clock0,LV_ALIGN_OUT_RIGHT_MID,20,0);
        clock1_img = lv_img_create(parent);
        lv_img_set_src(clock1_img,GET_IMAGE_PATH("icon_unselect.png"));
        lv_obj_align_to(clock1_img,clock1,LV_ALIGN_OUT_BOTTOM_MID,0,0);
        lv_obj_set_style_pad_top(clock1_img,10,LV_PART_MAIN);

        lv_obj_t *clock2 = init_clock2_obj(parent,&lv_clock2);
        lv_obj_align_to(clock2,clock1,LV_ALIGN_OUT_RIGHT_MID,20,0);
        clock2_img = lv_img_create(parent);
        lv_img_set_src(clock2_img,GET_IMAGE_PATH("icon_unselect.png"));
        lv_obj_align_to(clock2_img,clock2,LV_ALIGN_OUT_BOTTOM_MID,0,0);
        lv_obj_set_style_pad_top(clock2_img,10,LV_PART_MAIN);

        lv_obj_add_flag(clock0,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(clock1,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(clock2,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(clock0,clock0_type_click_event_cb,LV_EVENT_CLICKED,NULL);
        lv_obj_add_event_cb(clock1,clock1_type_click_event_cb,LV_EVENT_CLICKED,NULL);
        lv_obj_add_event_cb(clock2,clock2_type_click_event_cb,LV_EVENT_CLICKED,NULL);
    }
}


static void back_btn_click_event_cb(lv_event_t * e){
    delete_current_page(&com_style);
    deinit_clock0_obj(&lv_clock0);
    deinit_clock1_obj(&lv_clock1);
    deinit_clock2_obj(&lv_clock2);
    init_page_main();
}

static void roller_event_handler(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        // char buf[10];
        // lv_roller_get_selected_str(obj, buf, sizeof(buf));
        roller_index = lv_roller_get_selected(obj);
        printf("roller index = %d\n",roller_index);
        if(roller_index != 2){
            deinit_clock0_obj(&lv_clock0);
            deinit_clock1_obj(&lv_clock1);
            deinit_clock2_obj(&lv_clock2);
        }
        clean_show_view(show_view);
        init_item_view(show_view,roller_index);
    }
}

static void select_btn_click_event_cb(lv_event_t * e){
    get_device_state()->clock_type = clock_type;
    em_hal_time_ntpd_update();
    lv_event_code_t code = lv_event_get_code(e);
    device_state_t* device_state = get_device_state();
    device_state->time_type = roller_index;
    deinit_clock0_obj(&lv_clock0);
    deinit_clock1_obj(&lv_clock1);
    deinit_clock2_obj(&lv_clock2);
    delete_current_page(&com_style);
    init_page_main();
    device_param_write();
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
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_clock.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"时间显示设置");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    return cont;
}

static lv_obj_t * init_select_btn(lv_obj_t *parent){
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_style(btn,&com_style,0);
    lv_obj_set_size(btn,171,66);
    lv_obj_clear_state(btn,LV_STATE_FOCUS_KEY);
    lv_obj_set_style_border_width(btn, 0,0);
    lv_obj_set_style_shadow_width(btn, 0,0);
    lv_obj_set_style_radius(btn,35,0);
    lv_obj_set_style_bg_color(btn,APP_COLOR_BUTTON_DEFALUT,0);
    // lv_obj_set_style_opa(btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn,select_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t  * btn_label = lv_label_create(btn);
    obj_font_set(btn_label,FONT_TYPE_CN, FONT_SIZE_TEXT_1);
    lv_obj_set_style_text_color(btn_label,APP_COLOR_WHITE,0);
    lv_label_set_text(btn_label,"确定更新");
    lv_obj_align(btn_label,LV_ALIGN_CENTER,0,-5);

    return btn;
}

static lv_obj_t * init_setting_param_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    // lv_obj_set_style_bg_color(cont,APP_COLOR_BUTTON_UNSELECT,0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    //弹性布局对齐方式
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,30,0);
    // lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    lv_obj_t *setting_img = lv_img_create(cont);
    lv_img_set_src(setting_img,GET_IMAGE_PATH("icon_clock.png"));
    lv_obj_set_style_pad_top(setting_img,10,LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"显示方式:");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);

    roller_obj  = lv_roller_create(cont);
    lv_obj_set_style_border_width(roller_obj,0,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,0);
    lv_obj_set_style_text_color(roller_obj,APP_COLOR_WHITE,LV_PART_SELECTED);
    lv_obj_set_style_bg_color(roller_obj,lv_color_black(),0);
    lv_obj_set_style_bg_opa(roller_obj, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_obj,LV_OPA_TRANSP,LV_PART_MAIN);
    lv_roller_set_options(roller_obj,
                        "数字+文字显示\n仅数字显示\n表盘显示",
                        LV_ROLLER_MODE_NORMAL);
    lv_obj_add_event_cb(roller_obj, roller_event_handler, LV_EVENT_ALL, NULL);
    lv_font_t* font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_2);
    if(font_select != NULL)
        lv_obj_set_style_text_font(roller_obj, font_select, LV_PART_SELECTED);
    lv_font_t* un_font_select = get_font(FONT_TYPE_CN_LIGHT, FONT_SIZE_TITLE_3);
	if(un_font_select != NULL)
		lv_obj_set_style_text_font(roller_obj, un_font_select, 0);
    lv_obj_set_style_text_line_space(roller_obj,10,0);
    lv_obj_set_height(roller_obj,400);
    lv_obj_set_width(roller_obj,250);
    lv_roller_set_visible_row_count(roller_obj, 3);
    lv_roller_set_selected(roller_obj,1,LV_ANIM_ON);
    lv_obj_clear_state(roller_obj,LV_STATE_FOCUS_KEY);


    return cont;
}

static lv_obj_t * init_show_view(lv_obj_t *parent,TIME_SHOW_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 600, 280);
    lv_obj_add_style(cont, &com_style, 0);
    // lv_obj_set_style_bg_color(cont,APP_COLOR_BUTTON_DEFALUT,0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    
    init_item_view(cont,type);
    return cont;
}

void init_page_time_setting()
{
    clock_type = 0;
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_1.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);
    
    lv_obj_t * setting_param_view = init_setting_param_view(cont);
    lv_obj_align_to(setting_param_view,title_view,LV_ALIGN_OUT_BOTTOM_LEFT,50,10);

    show_view = init_show_view(cont,TIME_TYPE_2);
    lv_obj_align_to(show_view,setting_param_view,LV_ALIGN_OUT_RIGHT_MID,20,-10);


    lv_obj_t * btn = init_select_btn(cont);
    lv_obj_align_to(btn,show_view,LV_ALIGN_OUT_RIGHT_MID,50,10);
    
}