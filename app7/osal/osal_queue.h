#ifndef _OSAL_QUEUE_H_
#define _OSAL_QUEUE_H_

#include <stdint.h>
#include "osal_conf.h"

#define M_QUEUE_ID 1
#define M_FIFO_NAME_PREFIX "/tmp"

/* 消息结构定义 */
typedef struct {
    long type;               // 消息类型
    char data[1];            // 消息数据(柔性数组)
} msg_t;

/* 消息队列句柄结构 */
typedef struct {
    int msg_id;              // 消息队列ID
    uint32_t msgsize;        // 消息大小
    char filename[64];       // 关联的文件名，用于清理
} msg_handle_t;

OSAL_RESULT_T osal_queue_create(osal_queue_t *qhandle, const char *name, 
                              uint32_t msgsize, uint32_t queue_length);

OSAL_RESULT_T osal_queue_send(osal_queue_t *qhandle, const void *msg, 
                            uint32_t len, uint32_t msecs);

OSAL_RESULT_T osal_queue_recv(osal_queue_t *qhandle, void *msg, uint32_t msecs);

OSAL_RESULT_T osal_queue_delete(osal_queue_t *qhandle);

#endif
