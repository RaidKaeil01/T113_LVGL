#include <stdlib.h>
#include <stdio.h>

#include "lv_port_disp.h"
#include "lvgl.h"
#include "sunxifb.h"
#include "port_conf.h"


static lv_color_t *draw_buf;
static lv_color_t *draw_buf_1;

void lv_port_disp_init(bool is_disp_orientation)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
	uint32_t rotated = DISP_ORIENTATION;
    if(is_disp_orientation == true){
        rotated = LV_DISP_ROT_270;
    }else{
        rotated = LV_DISP_ROT_90;
    }
	 
    sunxifb_init(rotated);

    /*A buffer for LittlevGL to draw the screen's content*/
    static uint32_t width, height;
    sunxifb_get_sizes(&width, &height);
    width = 280;
	
	int draw_buf_size = width * height * sizeof(lv_color_t);
	
    draw_buf = (lv_color_t*) sunxifb_alloc(draw_buf_size, "lv_examples");

    if (draw_buf == NULL) 
	{
        sunxifb_exit();
        LV_LOG_ERROR("sunxifb_alloc error");
        return;
    }

    draw_buf_1 = (lv_color_t*) sunxifb_alloc(draw_buf_size, "lv_examples_1");

    if (draw_buf_1 == NULL) 
	{
        sunxifb_exit();
        LV_LOG_ERROR("sunxifb_alloc error");
        return;
    }
	
    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    static lv_disp_draw_buf_t draw_buf_dsc;                         
    lv_disp_draw_buf_init(&draw_buf_dsc, draw_buf, draw_buf_1, width * height);   

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    /*Initialize and register a display driver*/
	
    static lv_disp_drv_t disp_drv;                     
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf_dsc;
	disp_drv.flush_cb = sunxifb_flush;
	disp_drv.hor_res = width;
    disp_drv.ver_res = height;
	disp_drv.rotated = rotated;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

void lv_port_disp_deinit(void)
{
    sunxifb_free((void **)&draw_buf,"lv_examples");
    sunxifb_free((void **)&draw_buf_1,"lv_examples_1");
}