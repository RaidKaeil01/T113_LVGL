#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_AUDIO_CMD_LEN 200
#define PLAYER_MAX_URL_LENGTH 200

int em_get_audio_vol() {
    char command[MAX_AUDIO_CMD_LEN];
    char output[MAX_AUDIO_CMD_LEN];
    int volume = 0;
    sprintf(command, "amixer -D hw:audiocodec cget name='DAC volume'");

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        return 0;
    }

    while(fgets(output, MAX_AUDIO_CMD_LEN, fp) != NULL){
        char* volume_str = strstr(output, " values=");
        if (volume_str != NULL) {
            char* volstr = strtok(volume_str, ",");
            while (volstr != NULL) {
                volstr = strtok(NULL, ",");
                if (volstr != NULL) {
                    volume = atoi(volstr);
                }
            }
            printf("get_audio_vol %d\n", volume);
        }
    }
    pclose(fp);
    return volume;
}

int em_set_audio_vol(int vol){
    int ret = 0;
    char cmd[MAX_AUDIO_CMD_LEN];
    memset(cmd, 0, MAX_AUDIO_CMD_LEN);
    sprintf(cmd, "amixer -D hw:audiocodec cset name='DAC volume' 0,%d", vol);       
    ret = system(cmd);  
    printf("set_audio_vol %s,ret %d\n", cmd, ret);
    return 0;
}

void em_stop_play_audio(){
    int ret = 0;
    char cmd[200];
    memset(cmd, 0, 200);
    sprintf(cmd, "killall aplay");      
    ret = system(cmd);
    printf("stop_play_local_audio %s,ret %d\n", cmd, ret);
}

int em_play_audio(const char *url)
{
    int ret = 0;
    char cmd[PLAYER_MAX_URL_LENGTH+10];
    if(url == NULL){
        return 0;
    }
    memset(cmd, 0, PLAYER_MAX_URL_LENGTH);
    sprintf(cmd, "aplay %s &", url);        
    ret = system(cmd);  
    printf("play_local_audio %s,ret %d\n", cmd, ret);
    return 0;
}