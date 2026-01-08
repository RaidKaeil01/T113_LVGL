#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "osal_thread.h"
#include "osal_conf.h"

/**
 * 创建线程
 * @param thandle 输出参数，用于存储线程句柄
 * @param thread_fun 线程函数
 * @param arg 传递给线程函数的参数
 * @return 操作结果
 */
OSAL_RESULT_T osal_thread_create(osal_thread_t *thandle, 
                               void* (*thread_fun)(void *), 
                               void *arg)
{
    // 参数有效性检查
    if (thandle == NULL || thread_fun == NULL) {
        printf("osal_thread_create: Invalid parameters\n");
        return OSAL_INVALID_PARAM;
    }

    // 分配线程ID存储内存
    pthread_t *tid = (pthread_t *)malloc(sizeof(pthread_t));
    if (tid == NULL) {
        printf("osal_thread_create: Memory allocation failed\n");
        return OSAL_MEMORY_ERROR;
    }

    // 创建线程
    int ret = pthread_create(tid, NULL, thread_fun, arg);
    if (ret != 0) {
        printf("osal_thread_create: pthread_create failed, errno=%d\n", ret);
        free(tid);
        return OSAL_SYSTEM_ERROR;
    }

    *thandle = tid;
    return OSAL_SUCCESS;
}

/**
 * 取消线程
 * @param thandle 线程句柄
 * @return 操作结果
 */
OSAL_RESULT_T osal_thread_cancel(osal_thread_t *thandle)
{
    // 参数有效性检查
    if (thandle == NULL || *thandle == NULL) {
        printf("osal_thread_cancel: Invalid thread handle\n");
        return OSAL_INVALID_PARAM;
    }

    pthread_t *tid = (pthread_t *)*thandle;
    int ret = pthread_cancel(*tid);
    if (ret != 0) {
        printf("osal_thread_cancel: pthread_cancel failed, errno=%d\n", ret);
        return OSAL_SYSTEM_ERROR;
    }

    return OSAL_SUCCESS;
}

/**
 * 等待线程结束并释放资源
 * @param thandle 线程句柄
 * @param thread_return 用于存储线程返回值
 */
void osal_thread_join(osal_thread_t *thandle, void **thread_return)
{
    // 参数有效性检查
    if (thandle == NULL || *thandle == NULL) {
        printf("osal_thread_join: Invalid thread handle\n");
        return;
    }

    pthread_t *tid = (pthread_t *)*thandle;
    int ret = pthread_join(*tid, thread_return);
    if (ret != 0) {
        printf("osal_thread_join: pthread_join failed, errno=%d\n", ret);
    }

    // 释放线程ID内存
    free(*thandle);
    *thandle = NULL;
}

/**
 * 销毁线程资源
 * @param thandle 线程句柄
 * @return 操作结果
 */
OSAL_RESULT_T osal_thread_delete(osal_thread_t *thandle)
{
    // 参数有效性检查
    if (thandle != NULL && *thandle != NULL) {
        free(*thandle);
        *thandle = NULL;
    }

    // 线程自身调用时退出
    pthread_exit(NULL);
    // 理论上不会执行到这里
    return OSAL_SUCCESS;
}

/**
 * 线程休眠
 * @param msecs 休眠时间，单位：毫秒
 */
void osal_thread_sleep(int32_t msecs)
{
    usleep(msecs*1000);
}
