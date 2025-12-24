
#include <stdio.h>
#include "lvgl.h"

static lv_style_t com_style;

//初始化通用样式，方便复用
static void init_com_style(){
    lv_style_init(&com_style);
    lv_style_set_radius(&com_style,0);
    lv_style_set_border_width(&com_style,0);
    lv_style_set_pad_all(&com_style,0);
    lv_style_set_bg_opa(&com_style,0);
}

//按钮点击回调事件
static void back_btn_click_event_cb(lv_event_t * e){
    printf("back_btn_click_event_cb");
    lv_obj_t * act_scr = lv_layer_top();
    //清除layer层所有的对象
    lv_obj_clean(act_scr);
    lv_style_reset(&com_style);
}

//按钮初始化
static lv_obj_t * init_back_btn(lv_obj_t *parent){
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_set_size(btn,194,44);
    lv_obj_set_style_border_width(btn, 2,LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0,LV_PART_MAIN);
    lv_obj_set_style_radius(btn,22,LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn,lv_color_hex(0xFFFFFF),LV_PART_MAIN);
    lv_obj_add_event_cb(btn,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t  * btn_label = lv_label_create(btn);
    lv_obj_center(btn_label);
    lv_obj_set_style_text_color(btn_label,lv_color_hex(0x000000),LV_PART_MAIN);
    lv_label_set_text(btn_label,"Back");
    return btn;
}

void init_dialog()
{
    //初始化通用样式
    init_com_style();
    //初始化一个铺满页面的对象，仅方便演示lv_layer_top层
    lv_obj_t * cont = lv_obj_create(lv_layer_top());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(cont, &com_style, LV_PART_MAIN);

    //初始化一个弹窗对象
    lv_obj_t *msgbox_obj = lv_obj_create(cont);
    lv_obj_set_size(msgbox_obj, 400, 250);
    lv_obj_set_style_bg_opa(msgbox_obj, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_bg_color(msgbox_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_radius(msgbox_obj,25,LV_PART_MAIN);
    lv_obj_center(msgbox_obj);

    //初始化弹窗内的标题label
    lv_obj_t * label = lv_label_create(msgbox_obj);
    lv_obj_set_style_text_color(label,lv_color_hex(0x000000),LV_PART_MAIN);
    lv_label_set_text(label,"Dialog");
    lv_obj_align(label,LV_ALIGN_TOP_MID,0,15);

    //初始化弹窗的返回按钮
    lv_obj_t * back_btn = init_back_btn(msgbox_obj);
    lv_obj_align_to(back_btn,label,LV_ALIGN_OUT_BOTTOM_MID,0,38);
}