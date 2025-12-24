#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>

#include "base_utils.h"
#include "em_hal_brightness.h"

int brightness_default = 0;

void em_hal_screen_resume()
{
	em_hal_brightness_set_value(brightness_default);
}

void em_hal_screen_suspend()
{
	brightness_default = em_hal_brightness_get_value();
	em_hal_brightness_set_value(0);
}

void em_hal_get_cpu_temp(char* buf, int len){
   	int bytes;
    sprintf(buf, "cat /sys/devices/virtual/thermal/thermal_zone0/temp");
    bytes = base_utils_shell_exec(buf, buf, len);
	if (bytes > 0 )
	{
        printf("temp : %s",buf);
	}
}

void em_hal_get_cpu_clk(char* buf, int len){
   	int bytes;
    sprintf(buf, "cat /sys/kernel/debug/clk/pll-cpux/clk_rate");
    bytes = base_utils_shell_exec(buf, buf, len);
	if (bytes > 0 )
	{
        printf("version : %s",buf);
	}
}

void em_hal_get_free_storage(char* buf, int len){
    int bytes;
    sprintf(buf, "df -h");
    bytes = base_utils_shell_exec(buf, buf, len);
	if (bytes > 0 )
	{
        printf("version : %s",buf);
	}
}

void em_hal_reboot(void){
	// system("reboot");
	system("/usr/bin/demo &");
	exit(0);
}
