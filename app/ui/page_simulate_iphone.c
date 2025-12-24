
#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"

static lv_style_t com_style;

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

static lv_obj_t * page1;
static lv_obj_t * page2;

static void page_hide(lv_obj_t *page,int32_t start, int32_t end){
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, page);
    lv_anim_set_time(&anim, 500);
    lv_anim_set_delay(&anim, 0);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

static void page_show(lv_obj_t *page,int32_t start, int32_t end){
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, page);
    lv_anim_set_time(&anim, 500);
    lv_anim_set_delay(&anim, 0);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

static void get_click_pos(){
    lv_point_t pos;
    lv_indev_t* indev = lv_indev_get_act();
    lv_indev_get_point(indev, &pos);
    printf("Screen touched at position: x = %d, y = %d\n", pos.x, pos.y);
    if(pos.x < 60 && pos.y < 60){
        delete_current_page(&com_style);
        init_page_main();
    }
}

static void page1_click_event_cb(lv_event_t * e) {
    page_hide(page2,0,LV_VER_RES);
    page_show(page1,-LV_VER_RES,0);
    get_click_pos();
}

static void page2_click_event_cb(lv_event_t * e) {
    page_hide(page1,0, -LV_VER_RES);
    page_show(page2,LV_VER_RES, 0);
    get_click_pos();
}

void init_page_simulate_iphone(void)
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(cont,APP_COLOR_GREEN,0);
    // lv_obj_set_style_bg_opa(cont,LV_OPA_50,0);
    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_iphone.png"));
    lv_obj_align(bg_img,LV_ALIGN_CENTER,-2,0);

    
    page2 = lv_obj_create(cont);
    lv_obj_add_style(page2, &com_style, 0);
    lv_obj_set_size(page2, LV_PCT(100), LV_PCT(100));
    lv_obj_set_y(page2,-LV_VER_RES);
    lv_obj_set_style_bg_opa(page2,LV_OPA_0,0);
    lv_obj_t *img2 = lv_img_create(page2);
    lv_img_set_src(img2,GET_IMAGE_PATH("icon_iphone_2.png"));
    lv_obj_add_flag(img2,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(img2,LV_OBJ_FLAG_EVENT_BUBBLE);

    page1 = lv_obj_create(cont);
    lv_obj_add_style(page1, &com_style, 0);
    lv_obj_set_style_bg_opa(page1,LV_OPA_0,0);
    lv_obj_set_size(page1, LV_PCT(100), LV_PCT(100));
    lv_obj_t *img1 = lv_img_create(page1);
    lv_img_set_src(img1,GET_IMAGE_PATH("icon_iphone_1.png"));
    lv_obj_add_flag(img1,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(img1,LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_add_event_cb(page1, page2_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(page2, page1_click_event_cb, LV_EVENT_CLICKED, NULL);

}