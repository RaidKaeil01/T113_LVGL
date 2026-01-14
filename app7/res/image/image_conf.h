#ifndef _IMAGE_CONF_H_
#define _IMAGE_CONF_H_

#include "res_conf.h"

// 使用LVGL文件系统驱动，A:映射到/usr/，所以路径是 A:res/image/xxx
#define  DRIVER_LETTER  "A:"
#define GET_IMAGE_PATH(name) (DRIVER_LETTER "res/image/" name)

#endif