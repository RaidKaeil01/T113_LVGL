/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:41 
 * @Last Modified by:   xiaozhi 
 * @Last Modified time: 2024-09-24 23:31:41 
 */
#ifndef _IMAGE_CONF_H_
#define _IMAGE_CONF_H_

#include "res_conf.h"

#define  DRIVER_LETTER  "A:"
#define GET_IMAGE_PATH(name) (DRIVER_LETTER IMAGE_PATH name)

#define IMAGE_PATH_IC_PARAM_BODY_SENSOR   DRIVER_LETTER IMAGE_PATH "icon_body_sensor.png"
#define IMAGE_PATH_IC_PARAM_SMART_COASTER   DRIVER_LETTER IMAGE_PATH "icon_smart_coaster.png"
#define IMAGE_PATH_IC_PARAM_FLAME_SENSOR   DRIVER_LETTER IMAGE_PATH "icon_flame_sensor.png"


#endif