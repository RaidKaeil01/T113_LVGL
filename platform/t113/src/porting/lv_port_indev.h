
#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

int evdev_init(void);

void lv_port_indev_init(void);

void lv_port_indev_deinit();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

