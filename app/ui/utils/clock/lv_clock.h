#ifndef _LV_CLOCK_H_
#define _LV_CLOCK_H_

#include <stdio.h>
#include "lvgl.h"
#include <time.h>
#include <sys/time.h>
#include "image_conf.h"

typedef struct 
{
    time_t timep;
    struct tm time_temp;
    lv_obj_t * dial_img;
    lv_obj_t * hour_img;
    lv_obj_t * minute_img;
    lv_obj_t * sec_img;
    lv_timer_t * refresh_timer;
}lv_clock_t;

lv_obj_t * init_clock0_obj(lv_obj_t* parent,lv_clock_t *lv_clock);
lv_obj_t * init_clock1_obj(lv_obj_t* parent,lv_clock_t *lv_clock);
lv_obj_t * init_clock2_obj(lv_obj_t* parent,lv_clock_t *lv_clock);

void deinit_clock0_obj(lv_clock_t *lv_clock);
void deinit_clock1_obj(lv_clock_t *lv_clock);
void deinit_clock2_obj(lv_clock_t *lv_clock);

#endif
