
#include "file_save.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PARAM_SAVE_PATH PROJECT_RES_URL
#define PARAM_SAVE_PATH_MAX_LEN 300

int file_param_read(const char *name,void *data,int len){
    int fd = 0;
    int seek_offset = 0;
    char file_name[PARAM_SAVE_PATH_MAX_LEN];
    char file_name_copy[PARAM_SAVE_PATH_MAX_LEN];
    memset(file_name,0,sizeof(file_name));
    memset(file_name_copy,0,sizeof(file_name_copy));
    sprintf(file_name, "%s%s", PARAM_SAVE_PATH, name);
    sprintf(file_name_copy, "%s%s_copy", PARAM_SAVE_PATH, name);
    //如果不存在file
    if(access(file_name,F_OK) != 0){
        //如果存在临时文件temp
        if(access(file_name_copy,F_OK) == 0){
            //把file_temp_name文件民修改为file_name
            rename(file_name_copy,file_name);
            unlink(file_name_copy);
        }else{
            printf("open file error\n");
            return -1;
        }
    }
    fd = open(file_name, O_RDONLY, 0666);
    if (fd < 0){
        printf("open file error\n");
        return -1;
    }
    if (lseek(fd, seek_offset, SEEK_SET) == -1){
        return -1;
    }
    if (read(fd, data, len) <= 0){
        return -1;
    }
    return 0;
}

int file_param_write(const char *name,void *data,int len){
    int fd = 0;
    int seek_offset = 0;
    char file_name[PARAM_SAVE_PATH_MAX_LEN];
    char file_name_copy[PARAM_SAVE_PATH_MAX_LEN];
    memset(file_name,0,sizeof(file_name));
    memset(file_name_copy,0,sizeof(file_name_copy));
    sprintf(file_name, "%s%s", PARAM_SAVE_PATH, name);
    sprintf(file_name_copy, "%s%s_copy", PARAM_SAVE_PATH, name);
    printf("path = %s\n",file_name);
    fd = open(file_name_copy, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd < 0){
        printf("open file error\n");
        return -1;
    }
    if(lseek(fd, seek_offset, SEEK_SET) == -1){
        fsync(fd);
        close(fd);
        return -1;
    }
    if(write(fd, data, len) != len){
        fsync(fd);
        close(fd);
        return -1;
    }
    rename(file_name_copy, file_name);
    unlink(file_name_copy);
    sync();
    return 0;
}

typedef struct 
{
    char name[10];
    int data1;
    char data2;
}file_param_test_data_t;

void file_param_test(){
    file_param_test_data_t wdata;
    file_param_test_data_t rdata;

    int ret = file_param_read("param.cfg",&rdata,sizeof(rdata));
    if(ret == 0)
        printf("rdata name = %s %d %d\n",rdata.name,rdata.data1,rdata.data2);

    memcpy(wdata.name,"xiaozhi",strlen("xiaozhi"));
    wdata.data1 = 10;
    wdata.data2 = 20;
    file_param_write("param.cfg",&wdata,sizeof(wdata));
    ret = file_param_read("param.cfg",&rdata,sizeof(rdata));
    if(ret == 0)
        printf("rdata name = %s %d %d\n",rdata.name,rdata.data1,rdata.data2);

    memcpy(wdata.name,"xiaohei",strlen("xiaohei"));
    wdata.data1 = 40;
    wdata.data2 = 10;
    file_param_write("param.cfg",&wdata,sizeof(wdata));
    ret = file_param_read("param.cfg",&rdata,sizeof(rdata));
    if(ret == 0)
        printf("rdata name = %s %d %d\n",rdata.name,rdata.data1,rdata.data2);

}