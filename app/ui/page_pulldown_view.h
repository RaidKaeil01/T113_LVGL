#ifndef _PAGE_PULLDOWM_VIEW_H_
#define _PAGE_PULLDOWM_VIEW_H_

#include "lvgl/lvgl.h"

typedef enum{
    SYSTEM_SETTING_BACKLIGHT = 0,
    SYSTEM_SETTING_VOLUME,
}SYSTEM_SETTING_TYPE_E;

typedef struct{
    SYSTEM_SETTING_TYPE_E type;
    char name[10];
    char *img_url;
    int value;
    lv_obj_t *label;
}system_setting_info_t;

void init_page_pulldowm_view(lv_obj_t *view);
void deinit_page_pulldowm_view();
bool get_is_pull_down();

#endif
