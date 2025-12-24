#ifndef _EM_HAL_TIME_H
#define _EM_HAL_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/time.h>

void em_hal_time_ntpd_update(void);

void em_hal_time_set_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t min, uint32_t second);

void em_hal_time_get_time(uint32_t* year, uint32_t* month, uint32_t* day, 
						 uint32_t* hour, uint32_t* min, uint32_t* second, uint32_t* day_week);

void em_hal_time_get_time_by_input(time_t *timep, uint32_t* year, uint32_t* month, uint32_t* day,
	uint32_t* hour, uint32_t* min, uint32_t* second, uint32_t* day_week);

#ifdef __cplusplus
}
#endif

#endif