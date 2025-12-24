#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include "lv_port_indev.h"
#include "lvgl.h"
#include "port_conf.h"

int evdev_fd = -1;
int lv_indev_x;
int lv_indev_y;
int lv_indev_state;
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev;

int evdev_init(void)
{
     evdev_fd = open(EVDEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY);
     if(evdev_fd == -1) 
     {
        perror("unable to open evdev interface:");
        return -1;
     }

     fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
     
     lv_indev_x = 0;
     lv_indev_y = 0;
     lv_indev_state = LV_INDEV_STATE_REL;
     return 0;
}

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 */
void evdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    struct input_event in;
    while(read(evdev_fd, &in, sizeof(struct input_event)) > 0) 
    {
        if(in.type == EV_ABS) 
        {
            if(in.code == ABS_MT_POSITION_X)
            {
            	lv_indev_x = in.value;
            }
            else if(in.code == ABS_MT_POSITION_Y)
            {
            	lv_indev_y = in.value;
            }
        }
        else if(in.type == EV_KEY)
        {
            if(in.code == BTN_TOUCH) 
            {
                if(in.value == 0)
                {
                    lv_indev_state = LV_INDEV_STATE_REL;
                }
                else if(in.value == 1)
                {
                    lv_indev_state = LV_INDEV_STATE_PR;
                }
            }
        }
    }
    
    data->point.x = lv_indev_x;//(400 * lv_indev_x)/480 ;
    data->point.y = lv_indev_y;
    data->state = lv_indev_state;
    // printf("lv_indev_x=%d x,y=%d,%d  state=%d\n",lv_indev_x,data->point.x,data->point.y,data->state);
}

void lv_port_indev_init(void)
{

    /*Initialize your touchpad if you have*/
    evdev_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    indev = lv_indev_drv_register(&indev_drv);
}

void lv_port_indev_deinit(){
    lv_indev_delete(indev);
}
