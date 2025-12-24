#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

int base_utils_system(char *cmd)
{
	int ret = 0;
	pid_t status = system(cmd);
	if (status == -1)
	{
		printf("system(\"%s\") error",cmd);
		return 1;
	}
	else
	{
		if (WIFEXITED(status))
		{
			ret = WEXITSTATUS(status);
			if (ret == 0)
			{
				return 0;
			}
			else
			{
				printf("%s return error: %d", cmd, ret);
				return ret;
			}
		}
		else
		{
			printf("%s exec error",cmd);
			return 2;
		}
	}
}

int base_utils_shell_exec(const char* exe, char* buf, int len)
{
	int count = 0;
    FILE* stream = popen(exe, "r");
    if(stream == NULL) 
	{
		return count;
	}
	
    if(buf != NULL)
	{
        count = fread(buf, sizeof(char), len, stream);
    }
	
    pclose(stream);
    return count;
}

int base_utils_get_process_state(const char *name,int length){
    int bytes;
	char buf[10];
	char cmd[60];
    FILE   *strea;
	if(length > 20){
		printf("process nae is too long!");
		return -1;
	}
	sprintf(cmd,"ps | grep %s | grep -v grep",name);
    strea = popen(cmd, "r" ); //CHECK_WIFI_SHELL-->> FILE* strea
    if(!strea) return -1;
    bytes = fread( buf, sizeof(char), sizeof(buf), strea);
    pclose(strea);
    if(bytes > 0){
        printf("%s :process exist",name);
		return 1;
    }else {
        printf("%s :process not exist",name);
        return -1;
    }
}