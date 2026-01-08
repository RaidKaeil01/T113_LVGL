#ifndef _OSAL_THREAD_H_
#define _OSAL_THREAD_H_

#include <stdint.h>
#include <pthread.h>
#include "osal_conf.h"

OSAL_RESULT_T osal_thread_create(osal_thread_t *thandle,void* (*thread_fun)(void *arg), void *arg);

OSAL_RESULT_T osal_thread_delete(osal_thread_t *thandle);

OSAL_RESULT_T osal_thread_cancel(osal_thread_t *thandle);

void osal_thread_join(osal_thread_t *thandle, void **__thread_return);

void osal_thread_sleep(int32_t msecs);

#endif

