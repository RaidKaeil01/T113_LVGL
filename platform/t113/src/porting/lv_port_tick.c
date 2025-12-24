#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

#include "lv_port_tick.h"

uint32_t lvgl_port_tick(void)
{
  struct timespec tv = {0, 0};
  uint32_t time = 0;

  clock_gettime(CLOCK_MONOTONIC, &tv);
  time = tv.tv_sec*1000 + tv.tv_nsec/1000000;
  return time;
}
