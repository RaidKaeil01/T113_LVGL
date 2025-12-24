#ifndef _EM_HAL_SYSTEM_H
#define _EM_HAL_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

void em_hal_screen_resume(void);

void em_hal_screen_suspend(void);

void em_hal_get_cpu_temp(char* buf, int len);

void em_hal_get_cpu_clk(char* buf, int len);

void em_hal_get_free_storage(char* buf, int len);

void em_hal_reboot(void);

#ifdef __cplusplus
}
#endif

#endif