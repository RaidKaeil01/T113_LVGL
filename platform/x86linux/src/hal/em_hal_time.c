#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "em_hal_time.h"

void em_hal_time_ntpd_update(){
	system("ntpd -q -p pool.ntp.org ");
}

void em_hal_time_set_time(uint32_t year, uint32_t month, uint32_t day, 
						 uint32_t hour, uint32_t min, uint32_t second)
{
	printf("em_hal_time_set_time %d/%d/%d %d:%d:%d\n",year,month,day,hour,min,second);
	struct tm _tm;
    struct timeval tv;
    time_t timep;

    _tm.tm_year = year - 1900;
	_tm.tm_mon = month - 1;
	_tm.tm_mday = day;
	_tm.tm_hour = hour;
	_tm.tm_min = min;
	_tm.tm_sec = second;
	
    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
	
    if(settimeofday(&tv, (struct timezone *) 0) < 0)
    {
        printf("settimeofday error\n");
    }
}

void em_hal_time_get_time(uint32_t* year, uint32_t* month, uint32_t* day,
	uint32_t* hour, uint32_t* min, uint32_t* second, uint32_t* day_week)
{
	time_t timep;   
    struct tm* _tm;
	
    timep = time(NULL);	
    _tm = localtime(&timep);

	*year = _tm->tm_year + 1900;
	*month = _tm->tm_mon + 1;
	*day = _tm->tm_mday;
	*hour = _tm->tm_hour;
	*min = _tm->tm_min;
	*second = _tm->tm_sec;
	*day_week = _tm->tm_wday;	

}


void em_hal_time_get_time_by_input(time_t *timep, uint32_t* year, uint32_t* month, uint32_t* day,
	uint32_t* hour, uint32_t* min, uint32_t* second, uint32_t* day_week)
{

}
