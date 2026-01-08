#ifndef _OSAL_CONF_H_
#define _OSAL_CONF_H_

typedef void* osal_queue_t;

typedef void* osal_thread_t;

typedef enum
{
    OSAL_SUCCESS = 0,
    OSAL_ERROR = -1,
    OSAL_INVALID_PARAM = -2,
    OSAL_MEMORY_ERROR = -3,
    OSAL_SYSTEM_ERROR = -4
}OSAL_RESULT_T;

#endif

