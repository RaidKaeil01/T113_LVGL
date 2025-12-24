#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "em_hal_brightness.h"

#define MAX_BRIGHTNESS_CMD_LEN 200

void em_hal_brightness_set_value(int value)
{
	char cmd[MAX_BRIGHTNESS_CMD_LEN];
	//0-100的值映射到0-200
	int brightness = 200 * value / 100;
	printf("em_hal_brightness_set_value = %d\n",brightness);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "echo %d > /sys/class/backlight/backlight/brightness", brightness);
	system(cmd);
}

int em_hal_brightness_get_value(){
	char command[MAX_BRIGHTNESS_CMD_LEN];
    char output[MAX_BRIGHTNESS_CMD_LEN];
    int brightness = 0;
    sprintf(command, "cat /sys/class/backlight/backlight/brightness");
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        return 0;
    }

    while(fgets(output, MAX_BRIGHTNESS_CMD_LEN, fp) != NULL){
		brightness = atoi(output);
        printf("em_hal_brightness_get_value = %d\n",brightness);
    }
    pclose(fp);
	//0-200的值映射到0-100
    return (brightness * 100) / 200;;
}