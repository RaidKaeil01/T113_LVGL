#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#ifdef __cplusplus
extern "C" {
#endif
#include<stdbool.h>

void lv_port_disp_init(bool is_disp_orientation);
void lv_port_disp_deinit(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
