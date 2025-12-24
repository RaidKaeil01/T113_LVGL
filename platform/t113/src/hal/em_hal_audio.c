#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_AUDIO_CMD_LEN 200
#define PLAYER_MAX_URL_LENGTH 200

volatile pid_t aplay_pid = -1;

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
    //0-200的值映射到0-100
    return (volume * 100) / 200;;
}

int em_set_audio_vol(int vol){
    int ret = 0;
    char cmd[MAX_AUDIO_CMD_LEN];
    memset(cmd, 0, sizeof(cmd));
    //0-100的值映射到0-200
	int volume = 200 * vol / 100;
    sprintf(cmd, "amixer -D hw:audiocodec cset name='DAC volume' 0,%d", volume);       
    ret = system(cmd);  
    printf("set_audio_vol %s,ret %d\n", cmd, ret);
    return 0;
}

void em_stop_play_audio(){
    int ret = 0;
    char cmd[200];
    memset(cmd, 0, 200);
    sprintf(cmd, "killall -9 aplay");      
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
    sprintf(cmd, "aplay %s", url);  
    // 如果已经在播放，先停止
    if (aplay_pid > 0) {
        printf("kill %d\n",aplay_pid);
        em_stop_play_audio();
        waitpid(aplay_pid, NULL, 0);
        aplay_pid = -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        ret = system("echo 0 > /sys/class/gpio/gpio34/value");
        ret = system(cmd);  
        printf("play_local_audio %s,ret %d\n", cmd, ret);
        ret = system("echo 1 > /sys/class/gpio/gpio34/value");
        exit(1);
    } else if (pid > 0) {
        aplay_pid = pid;
    } else {
        perror("fork failed");
    }
    return 0;
}