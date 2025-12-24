/*
 * @Author: xiaozhi 
 * @Date: 2024-09-25 00:07:46 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:56:32
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "lvgl.h"
#include "utils.h"

static char time[20];

/**
 * 返回分钟转换的小时、分钟字符串 单位数补0
 */
char *get_time_str(int time_min){
    memset(time,'\0',sizeof(time));
    int time_h = time_min / 60;
    int time_m = time_min % 60;
    if(time_h >= 10){
        if(time_m >= 10){
            sprintf(time,"%d:%d",time_h,time_m);
        }else{
            sprintf(time,"%d:0%d",time_h,time_m);
        }
    }else{
        if(time_m >= 10){
            sprintf(time,"0%d:%d",time_h,time_m);
        }else{
            sprintf(time,"0%d:0%d",time_h,time_m);
        }
    }
    return time;
}   

/**
 * 返回分钟转换的小时、分钟字符串 单位数补0
 */
char *get_time_str_nosymbol(int time_min){
    memset(time,'\0',sizeof(time));
    int time_h = time_min / 60;
    int time_m = time_min % 60;
    if(time_h >= 10){
        if(time_m >= 10){
            sprintf(time,"%d#000000 :#%d",time_h,time_m);
        }else{
            sprintf(time,"%d#000000 :#0%d",time_h,time_m);
        }
    }else{
        if(time_m >= 10){
            sprintf(time,"0%d#000000 :#%d",time_h,time_m);
        }else{
            sprintf(time,"0%d#000000 :#0%d",time_h,time_m);
        }
    }
    return time;
}


static void signal_callback_func(int sig_no)
{
    printf("signal %d, exiting ...\n", sig_no);
    _exit(1);
    printf("retry _exit ...\n");
}

void system_signal_init(){
    signal(SIGINT, signal_callback_func);
    signal(SIGQUIT, signal_callback_func);
    signal(SIGTERM, signal_callback_func);
    signal(SIGSEGV, signal_callback_func);
    signal(SIGABRT, signal_callback_func);
    signal(SIGBUS, signal_callback_func);
    signal(SIGFPE, signal_callback_func);
    signal(SIGILL, signal_callback_func);
    signal(SIGTSTP, signal_callback_func);
}

// typedef enum {
//     WIFI_DISCONNECTED=0,
//     WIFI_CONNECTED,
//     INVAILD_STATUS,
// }WIFI_STATUS_E;

// #define READ_WIFI_STATUS_CMD  "cat /sys/class/net/%s/carrier"
// #define WIFI_INTERFACE  "wlan0"

// WIFI_STATUS_E  get_wifi_status(void)
// {
//     int status = 0;
//     int bytes = 0;
//     char cmd[100];
//     char buf[500];
//     char *p =NULL;
//     memset(cmd,0,sizeof(cmd));
//     memset(buf,0,sizeof(buf));
//     sprintf(cmd,READ_WIFI_STATUS_CMD,WIFI_INTERFACE);
//     if(bytes > 0){
//         status = atoi(buf);
//         if(status == 1){
//             return WIFI_CONNECTED;
//         }
//         else if(status == 0){
//              return WIFI_DISCONNECTED;
//         }
//         else
//             return INVAILD_STATUS;
//     }
// }