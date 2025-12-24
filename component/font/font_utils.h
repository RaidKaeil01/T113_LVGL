/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:32:10 
 * @Last Modified by:   xiaozhi 
 * @Last Modified time: 2024-09-24 23:32:10 
 */
#ifndef _FONT_UTILS_H_
#define _FONT_UTILS_H_

#include "lvgl/lvgl.h"

/**
 * 添加字库
 */
void add_font(int type ,char* font_url);

/**
 * 获取字库
 */
lv_font_t* get_font(int type, uint16_t size);

#endif
