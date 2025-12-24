/*
 * @Author: xiaozhi 
 * @Date: 2024-10-08 00:19:08 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-10-08 02:56:53
 */
#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "page_pulldown_view.h"
#include "em_hal_brightness.h"
#include "em_hal_audio.h"
#include "device_data.h"
#include "music_conf.h"

//触发下拉窗口的按下起始位置y坐标偏移值
#define PULL_VIEW_START_Y_OFFSET 30
#define PULL_VIEW_END_Y_OFFSET 200
//触发下拉滑动动作的y坐标偏移值
#define PULL_ACTION_TRIGGER_Y_OFFSET 5
//触发上拉回收的y坐标偏移值
#define PULL_UP_TRIGGER_Y_OFFSET 20

static lv_obj_t *view_obj = NULL;

static lv_point_t press_point;
static lv_point_t pressing_point;
static lv_point_t released_point;
static bool is_pull_down = false;

static lv_style_t com_style;

static system_setting_info_t system_setting_info_list[] = {
    {SYSTEM_SETTING_BACKLIGHT,"亮度",GET_IMAGE_PATH("icon_brightness.png"),50,NULL},
    {SYSTEM_SETTING_VOLUME,"音量",GET_IMAGE_PATH("icon_volume.png"),30,NULL},
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

static void anim_cb(void *var, int32_t v)
{
    lv_obj_set_pos(var, 0, v);
}

static void run_anim(int32_t y_start,int32_t y_end){
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, view_obj);
    lv_anim_set_values(&anim, y_start, y_end);
    lv_anim_set_exec_cb(&anim, anim_cb);
    lv_anim_set_time(&anim, 200);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_start(&anim);
}

static void feedback_cb(struct _lv_indev_drv_t *indev_drv, uint8_t code)
{
    static int offset;
    lv_indev_t *indev = lv_indev_get_act();
    switch (code)
    {
    case LV_EVENT_PRESSED:
        lv_indev_get_point(indev,&press_point);
        // printf("press_point y = %d\n",press_point.y);
        break;
    case LV_EVENT_PRESSING:
        lv_indev_get_point(indev,&pressing_point);
        offset = abs(pressing_point.y - press_point.y);
        // printf("pressing_point y = %d offset= %d\n",pressing_point.y,offset);
        if(offset > PULL_ACTION_TRIGGER_Y_OFFSET){
            if(is_pull_down == false){
                if(press_point.y < PULL_VIEW_START_Y_OFFSET){
                    // printf("is_pull_down\n");
                    lv_coord_t view_height = lv_obj_get_height(view_obj);
                    lv_obj_set_pos(view_obj, 0, -view_height + pressing_point.y);
                }
            }else{
                if(press_point.y > PULL_VIEW_END_Y_OFFSET){
                    lv_coord_t view_height = lv_obj_get_height(view_obj);
                    lv_obj_set_pos(view_obj, 0, -view_height + pressing_point.y);
                }
            }
        }
        break;  
    case LV_EVENT_RELEASED:
        lv_indev_get_point(indev,&released_point);
        lv_coord_t view_y = lv_obj_get_y(view_obj)+lv_obj_get_height(view_obj);
        lv_coord_t view_height = lv_obj_get_height(view_obj);
        // printf("is_pull_down = %d offset = %d\n",is_pull_down,offset);
        if(offset > PULL_ACTION_TRIGGER_Y_OFFSET){
            if(is_pull_down == false){
                //当前为收起状态
                if(press_point.y < PULL_VIEW_START_Y_OFFSET){
                    if(view_y > view_height/3){
                        //need run down anim
                        is_pull_down = true;
                        run_anim(released_point.y - view_height,0);
                    }else{
                        //need run up anim
                        is_pull_down = false;
                        run_anim(released_point.y - view_height,- view_height);
                    }
                }
            }else{
                if(press_point.y > PULL_VIEW_END_Y_OFFSET){
                    int direction = pressing_point.y - press_point.y;
                    // printf("direction = %d\n",direction);
                    if(offset > PULL_UP_TRIGGER_Y_OFFSET && direction < 0){
                        is_pull_down = false;
                        run_anim(released_point.y - view_height,- view_height);
                    }else{
                        is_pull_down = true;
                        run_anim(released_point.y - view_height,0);
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}

static void slider_release_cb(lv_event_t * e){
    lv_obj_t * slider = lv_event_get_target(e);
    char* menu_name  = (char *)lv_event_get_user_data(e);
    device_state_t* device_state = get_device_state();
    if(strstr(menu_name,"亮度") != NULL){
        em_hal_brightness_set_value(lv_slider_get_value(slider)+5);
        device_state->brightness_value  = lv_slider_get_value(slider);
    }else if(strstr(menu_name,"音量") != NULL){
        em_set_audio_vol(lv_slider_get_value(slider));
        start_play_audio_async(GET_MUSIC_PATH("audio_msg.wav"));
        device_state->volume_value  = lv_slider_get_value(slider);
    }
    device_param_write();
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    char* menu_name  = (char *)lv_event_get_user_data(e);
    device_state_t* device_state = get_device_state();
    if(strstr(menu_name,"亮度") != NULL){
        lv_label_set_text(system_setting_info_list[SYSTEM_SETTING_BACKLIGHT].label, buf);
        em_hal_brightness_set_value(lv_slider_get_value(slider)+5);
    }else if(strstr(menu_name,"音量") != NULL){
        lv_label_set_text(system_setting_info_list[SYSTEM_SETTING_VOLUME].label, buf);
    }
}

static lv_obj_t *init_scroll_item_view(lv_obj_t *parent,SYSTEM_SETTING_TYPE_E type){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont,30,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);

    lv_obj_t *icon = lv_img_create(cont);
    lv_img_set_src(icon,system_setting_info_list[type].img_url);
    lv_obj_set_align(icon,LV_ALIGN_LEFT_MID);

    lv_obj_t *title_label = lv_label_create(cont);
    obj_font_set(title_label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(title_label,system_setting_info_list[type].name);
    lv_obj_set_style_text_color(title_label,APP_COLOR_WHITE,0);
    lv_obj_set_style_pad_top(title_label,-10,LV_PART_MAIN);

    lv_obj_t * slider = lv_slider_create(cont);
    lv_slider_set_value(slider,system_setting_info_list[type].value,LV_ANIM_OFF);
    lv_slider_set_range(slider, 0, 100);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)system_setting_info_list[type].name);
    lv_obj_add_event_cb(slider,slider_release_cb,LV_EVENT_RELEASED, (void *)system_setting_info_list[type].name);

    system_setting_info_list[type].label = lv_label_create(cont);
    obj_font_set(system_setting_info_list[type].label,FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text_fmt(system_setting_info_list[type].label,"%d%%",system_setting_info_list[type].value);
    lv_obj_set_style_text_color(system_setting_info_list[type].label,APP_COLOR_WHITE,0);
    lv_obj_set_style_pad_top(system_setting_info_list[type].label,-10,LV_PART_MAIN);

    return cont;
}

bool get_is_pull_down(){
    return is_pull_down;
}

void init_page_pulldowm_view(lv_obj_t *view)
{
    device_state_t* device_state = get_device_state();
    system_setting_info_list[SYSTEM_SETTING_BACKLIGHT].value = device_state->brightness_value;
    system_setting_info_list[SYSTEM_SETTING_VOLUME].value = device_state->volume_value;

    com_style_init();
    view_obj = view;
    lv_obj_t* backlight_view = init_scroll_item_view(view,SYSTEM_SETTING_BACKLIGHT);
    lv_obj_align(backlight_view,LV_ALIGN_TOP_MID,0,50);
    
    lv_obj_t* volume_view = init_scroll_item_view(view,SYSTEM_SETTING_VOLUME);
    lv_obj_align_to(volume_view,backlight_view,LV_ALIGN_OUT_BOTTOM_MID,0,50);

    lv_obj_t *up_icon = lv_img_create(view);
    lv_img_set_src(up_icon,GET_IMAGE_PATH("icon_pullup.png"));
    lv_obj_align_to(up_icon,volume_view,LV_ALIGN_OUT_BOTTOM_MID,0,50);

    lv_indev_t *indev = NULL;
    do
    {
        indev = lv_indev_get_next(indev);
        if (indev != NULL && indev->driver->type == LV_INDEV_TYPE_POINTER)
        {
            indev->driver->feedback_cb = feedback_cb;
            break;
        }
    } while (indev != NULL);
}

void deinit_page_pulldowm_view()
{
    lv_style_reset(&com_style);
    lv_indev_t *indev = NULL;
    do
    {
        indev = lv_indev_get_next(indev);
        if (indev != NULL && indev->driver->type == LV_INDEV_TYPE_POINTER)
        {
            indev->driver->feedback_cb = NULL;
            break;
        }
    } while (indev != NULL);
}