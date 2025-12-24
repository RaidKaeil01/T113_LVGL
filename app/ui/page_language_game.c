#include <stdio.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "audio_player_async.h"

static lv_style_t com_style;

typedef struct{
    int type;
    char *img_url;
}shortcutkey_info_t;

static shortcutkey_info_t shortcutkey_info_list[] = {
    {0,GET_IMAGE_PATH("icon_game_1.png")},
    {1,GET_IMAGE_PATH("icon_game_2.png")},
    {2,GET_IMAGE_PATH("icon_game_3.png")},
    {3,GET_IMAGE_PATH("icon_game_4.png")},
    {4,GET_IMAGE_PATH("icon_game_5.png")},
    {5,GET_IMAGE_PATH("icon_game_6.png")},
    {6,GET_IMAGE_PATH("icon_game_7.png")},
    {7,GET_IMAGE_PATH("icon_game_8.png")},
    {8,GET_IMAGE_PATH("icon_game_9.png")},
    {9,GET_IMAGE_PATH("icon_game_10.png")},
    {10,GET_IMAGE_PATH("icon_game_11.png")},
    {11,GET_IMAGE_PATH("icon_game_12.png")},
    {12,GET_IMAGE_PATH("icon_game_13.png")},
    {13,GET_IMAGE_PATH("icon_game_14.png")},
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

static void menu_click_event_cb(lv_event_t * e){
    int index  = (int)(intptr_t)lv_event_get_user_data(e);
    printf("index = %d is selected\n", index);
    // stop_play_audio_async();
    if(index == 0){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_01.WAV"));
    }else if(index == 1){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_02.WAV"));
    }else if(index == 2){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_03.WAV"));
    }else if(index == 3){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_04.WAV"));
    }else if(index == 4){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_05.WAV"));
    }else if(index == 5){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_06.WAV"));
    }else if(index == 6){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_07.WAV"));
    }else if(index == 7){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_08.WAV"));
    }else if(index == 8){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_09.WAV"));
    }else if(index == 9){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_10.WAV"));
    }else if(index == 10){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_11.WAV"));
    }else if(index == 11){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_12.WAV"));
    }else if(index == 12){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_13.WAV"));
    }else if(index == 13){
        start_play_audio_async(GET_MUSIC_PATH("audio_game_14.WAV"));
    }
}

static void back_btn_click_event_cb(lv_event_t * e){
    delete_current_page(&com_style);
    init_page_main();
}

// 动画回调函数，用于更新图片的大小
static void img_size_anim_cb(void * var, int32_t v)
{
    lv_img_set_zoom((lv_obj_t *)var,v);
}

// 图片事件回调函数
static void img_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * img = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSED) {
        // 创建一个动画，使图片逐渐变大
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_values(&a, 256, 300);
        lv_anim_set_time(&a, 200);
        lv_anim_set_exec_cb(&a, img_size_anim_cb);
        lv_anim_start(&a);
    } else if(code == LV_EVENT_RELEASED) {
        // 创建一个动画，使图片恢复原来的大小
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_values(&a, lv_img_get_zoom(img), 256);
        lv_anim_set_time(&a, 200);
        lv_anim_set_exec_cb(&a, img_size_anim_cb);
        lv_anim_start(&a);
    }
}

static lv_obj_t * init_item(lv_obj_t *parent,int index,char *imgurl){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    if((index+1)%2)
        lv_obj_set_style_pad_top(cont,5,LV_PART_MAIN);
    else
        lv_obj_set_style_pad_top(cont,75,LV_PART_MAIN);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img,imgurl);
    lv_obj_add_flag(img,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(img,LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(img,img_event_cb,LV_EVENT_ALL,NULL);

    lv_obj_add_event_cb(cont,menu_click_event_cb,LV_EVENT_CLICKED,(void *)(intptr_t)index);
}

static lv_obj_t * init_menu_list(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_flex_flow(cont,LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(cont,20,0);
    lv_obj_set_style_pad_left(cont,50,0);
    lv_obj_set_style_pad_right(cont,50,0);
    lv_obj_set_style_bg_opa(cont,LV_OPA_0,0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    int length = sizeof(shortcutkey_info_list) / sizeof(shortcutkey_info_t);
    for(int index = 0;index < length;index ++){
        init_item(cont,index,shortcutkey_info_list[index].img_url);
    }
    return cont;
}

static lv_obj_t * init_title_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(cont,APP_COLOR_GRAY_BG,0);

    lv_obj_t *back_img = lv_img_create(cont);
    lv_img_set_src(back_img,GET_IMAGE_PATH("icon_back.png"));
    lv_obj_set_align(back_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_left(back_img,20,LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img,20,LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"让我们一起说中文吧");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,back_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);
    return cont;
}

void init_page_language_game(void)
{
    com_style_init();
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(cont,APP_COLOR_GRAY_BG,0);

    lv_obj_t *title_view =  init_title_view(cont);

    lv_obj_t * menu_obj = init_menu_list(cont);
    lv_obj_align(menu_obj,LV_ALIGN_TOP_MID,-10,50);
}