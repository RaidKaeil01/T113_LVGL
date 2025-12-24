#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>

#include "em_hal_console.h"

#define PTS_FILE_NAME "/dev/pts/0"

#define PTS_MAX_BUF_LEN 4096

static int pts_fd = -1;

static char pts_buffer[PTS_MAX_BUF_LEN];

static char pts_critical_buffer[PTS_MAX_BUF_LEN];

static void get_time_fmt(char* str, int len)
{
	memset(str, 0, len);

	time_t cur_time;
	struct tm* cur_tm;
	cur_time = time(NULL);
	cur_tm = localtime(&cur_time);
	snprintf(str, len, "%04u-%02u-%02u %02u:%02u:%02u.%03u",
			 cur_tm->tm_year + 1900, cur_tm->tm_mon + 1, cur_tm->tm_mday,
			 cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec, 0);
}

static void pts_console_print_inner(const char* str)
{
	int ret = 0;	
	if(pts_fd < 0)
	{
		pts_fd = open(PTS_FILE_NAME, O_WRONLY | O_NONBLOCK, 0666);
		if (pts_fd < 0) 
		{
			return;
		}
	}
	
	ret = write(pts_fd, str, strlen(str));
	if(ret <= 0)
	{
		close(pts_fd);
		pts_fd = -1;
	}
}

static void pts_console_print_inner_time(const char* str)
{
	static time_t pts_open_time = 0;

	if(pts_fd < 0)
	{
		time_t cur_time = time(NULL);
		int time_offset = abs(cur_time - pts_open_time);
		if(time_offset < 5)
		{
			return;
		}
		pts_open_time = cur_time;	
	}
	
	pts_console_print_inner(str);
}

void pts_console_print_fmt(int critical_flag, int time_flag, const char *fun, char *fmt, ...)
{
	char time[64];
	get_time_fmt(time, 64);
	
	char* buffer = NULL;
	if(critical_flag == 0)
	{
		buffer = pts_buffer;
	}
	else
	{
		buffer = pts_critical_buffer;
	}
	
	memset(buffer, 0, PTS_MAX_BUF_LEN);
	snprintf(buffer, PTS_MAX_BUF_LEN, "\n%s %s ", time, fun);
	if(time_flag == 0)
	{
		pts_console_print_inner(buffer);
	}
	else
	{
		pts_console_print_inner_time(buffer);	
	}	
	usleep(2000);
	
	va_list vaList;
	va_start(vaList, fmt);
	
	memset(buffer, 0, PTS_MAX_BUF_LEN);
	vsnprintf(buffer, PTS_MAX_BUF_LEN-1 , fmt, vaList);
	if(time_flag == 0)
	{
		pts_console_print_inner(buffer);
	}
	else
	{
		pts_console_print_inner_time(buffer);	
	}
	usleep(2000);
	
	va_end(vaList);
}

void em_hal_console_print(const char* str)
{
	//pts_console_print_inner_time(str);
}
