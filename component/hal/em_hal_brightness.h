#ifndef _EM_HAL_BRIGHTNESS_H
#define _EM_HAL_BRIGHTNESS_H

#ifdef __cplusplus
extern "C" {
#endif

void em_hal_brightness_set_value(int value);

int em_hal_brightness_get_value(void);

#ifdef __cplusplus
}
#endif

#endif