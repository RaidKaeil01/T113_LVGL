/**
 * @file lv_load.h
 *
 */

/**
 * TODO Remove these instructions
 * Search and replace: load -> object short name with lower case(e.g. btn, label etc)
 *                    LOAD -> object short name with upper case (e.g. BTN, LABEL etc.)
 *
 */

#ifndef LV_LOAD_H
#define LV_LOAD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_conf_internal.h"
#define LV_USE_LOAD 1
#if LV_USE_LOAD != 0

#include "../core/lv_obj.h"

/*********************
 *      DEFINES
 *********************/
#define MAX_LOAD_POINT 5
#define LOAD_POINT_WIDTH 10
#define LOAD_POINT_PAD 20
/**********************
 *      TYPEDEFS
 **********************/
/*Data of loadate*/
typedef struct {
    lv_obj_t obj;                   //必须，存储base obj
    uint16_t point_width;           //点的宽度
    uint16_t point_pad;             //点的间距
    bool dots[MAX_LOAD_POINT];      //点的显示状态
    uint8_t current_dot;            //当前显示的点索引
    bool pressed;
} lv_load_t;

extern const lv_obj_class_t lv_load_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a load object
 * @param parent    pointer to an object, it will be the parent of the new load
 * @return          pointer to the created bar
 */
lv_obj_t * lv_load_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 *=====================*/

/*=====================
 * Setter functions
 *====================*/
void lv_load_set_point_width(lv_obj_t * obj,uint16_t width);

void lv_load_set_point_pad(lv_obj_t * obj,uint16_t pad);
/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LOAD*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LOAD_H*/
