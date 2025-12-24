/**
 * @file lv_load.c
 *
 */

/**
 * TODO Remove these instructions
 * Search and replace: load -> object short name with lower case(e.g. btn, label etc)
 *                    LOAD -> object short name with upper case (e.g. BTN, LABEL etc.)
 *
 * You can remove the defined() clause from the #if statement below. This exists because
 * LV_USE_LOAD is not in lv_conf.h or lv_conf_loadate.h by default.
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_load.h" /*TODO uncomment this*/
#include <stdio.h>

#if defined(LV_USE_LOAD) && LV_USE_LOAD != 0

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_load_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_load_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_load_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_load_event(const lv_obj_class_t * class_p, lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_load_class = {
    .constructor_cb = lv_load_constructor,
    .destructor_cb = lv_load_destructor,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .event_cb = lv_load_event,
    .instance_size = sizeof(lv_load_t),
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .base_class = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_load_create(lv_obj_t * parent)
{

    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*======================
 * Add/remove functions
 *=====================*/

/*
 * New object specific "add" or "remove" functions come here
 */

/*=====================
 * Setter functions
 *====================*/
void lv_load_set_point_width(lv_obj_t * obj,uint16_t width){
    lv_load_t * load_obj = (lv_load_t *)obj;
    load_obj->point_width = width;
}

void lv_load_set_point_pad(lv_obj_t * obj,uint16_t pad){
    lv_load_t * load_obj = (lv_load_t *)obj;
    load_obj->point_pad = pad;
}

/*
 * New object specific "set" functions come here
 */

/*=====================
 * Getter functions
 *====================*/

/*
 * New object specific "get" functions come here
 */

/*=====================
 * Other functions
 *====================*/

/*
 * New object specific "other" functions come here
 */

/**********************
 *   STATIC FUNCTIONS
 **********************/
// 定时器回调
static void lv_load_obj_timer_cb(lv_timer_t * timer) {
    lv_obj_t * obj = timer->user_data;
    lv_load_t * load_obj = (lv_load_t *)obj;
    // 更新圆点状态
    load_obj->dots[load_obj->current_dot] = true;
    load_obj->current_dot = (load_obj->current_dot + 1) % MAX_LOAD_POINT;
    load_obj->dots[load_obj->current_dot] = false;
    for(int index =0;index < MAX_LOAD_POINT;index ++){
        printf("%d ",load_obj->dots[index]);
    }
    printf("\n");
    // 触发重绘
    lv_obj_invalidate(obj);
}

static void lv_load_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_load_t * load_obj = (lv_load_t *)obj;
    // 初始化圆点状态
    memset(load_obj->dots,0,sizeof(load_obj->dots));
    load_obj->current_dot = 0;
    load_obj->point_width = LOAD_POINT_WIDTH;
    load_obj->point_pad = LOAD_POINT_PAD;

    //重要，没有样式会导致绘制不出内容
    //初始化LV_PART_MAIN样式
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj,lv_color_hex(0x2f3237),LV_PART_MAIN);
    lv_obj_set_style_radius(obj,30,LV_PART_MAIN);
    //初始化LV_PART_INDICATOR样式
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,LV_PART_INDICATOR);
    lv_obj_set_style_radius(obj,LV_RADIUS_CIRCLE,LV_PART_INDICATOR);
    // 启动定时器
    lv_timer_t * timer = lv_timer_create(lv_load_obj_timer_cb, 1000, obj);
    lv_timer_ready(timer);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_load_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    lv_load_t * load = (lv_load_t *)obj;
    /*Free the widget specific data*/
}

static void lv_load_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);
    lv_res_t res;
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RES_OK) return;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if (code == LV_EVENT_DRAW_MAIN) {
        lv_load_t * load_obj = (lv_load_t *)obj;
        //获取绘制上下文，包含了绘制操作所需的各种信息和资源，例如画布等。
        lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

        /*绘制LV_PART_MAIN*/
        lv_area_t main_area;
        //设置绘制的区域，这里直接赋值对象区域
        lv_area_copy(&main_area, &obj->coords);
        //printf("x1=%d x2=%d y1=%d y2=%d\n",main_area.x1,main_area.x2,main_area.y1,main_area.y2);
        //初始化绘制样式描述符，包含了绘制操作所需的边框、背景颜色、圆角信息
        lv_draw_rect_dsc_t draw_main_dsc;
        lv_draw_rect_dsc_init(&draw_main_dsc);
        //这里直接用LV_PART_MAIN部分的样式
        lv_obj_init_draw_rect_dsc(obj, LV_PART_MAIN, &draw_main_dsc);
        if(load_obj->pressed)
        {
            draw_main_dsc.bg_color = lv_color_hex(0x00ff00);
        }
        //绘制函数
        lv_draw_rect(draw_ctx, &draw_main_dsc, &main_area);

        /*绘制LV_PART_INDICATOR*/
        for(int index = 0;index < MAX_LOAD_POINT ; index++){
            lv_draw_rect_dsc_t draw_indic_dsc;
            lv_draw_rect_dsc_init(&draw_indic_dsc);
            lv_obj_init_draw_rect_dsc(obj, LV_PART_INDICATOR, &draw_indic_dsc);
            //根据定时器更新的点信息，设置应该绘制的颜色
            if(load_obj->dots[index] == true){
                draw_indic_dsc.bg_color = lv_color_hex(0x00ff00);
            }else{
                draw_indic_dsc.bg_color = lv_color_hex(0x155B80);
            }
            //计算绘制位置和大小，x1 y1为绘制起始位置，x2-x1 y2-y1为绘制的宽高
            lv_area_t indic_area;
            indic_area.x1 = main_area.x1 + index*load_obj->point_pad;
            indic_area.x2 = indic_area.x1 + load_obj->point_width;
            indic_area.y1 = (main_area.y2 - main_area.y1)/2 - load_obj->point_width / 2; //y轴中心位置
            indic_area.y2 = indic_area.y1 + load_obj->point_width;
            lv_draw_rect(draw_ctx, &draw_indic_dsc, &indic_area);
        }
    }else if(code == LV_EVENT_PRESSED)
    {
        lv_load_t * load_obj = (lv_load_t*)obj;
        load_obj->pressed = true;
        lv_obj_invalidate(obj);
    }
    else if(code == LV_EVENT_RELEASED)
    {
        lv_load_t * load_obj = (lv_load_t*)obj;
        load_obj->pressed = false;
        lv_obj_invalidate(obj);
    }
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
