#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <fcntl.h>

#include "osal_queue.h"

/* 空指针检查宏 */
#define CHECK_NULL(ptr, ret) do { \
    if (!ptr) { errno = EINVAL; return ret; } \
} while (0)

OSAL_RESULT_T osal_queue_create(osal_queue_t *qhandle, const char *name, 
                              uint32_t msgsize, uint32_t queue_length)
{
    int fd = -1;
    key_t key;
    msg_handle_t *msg_handle;
    char fileName[64];
    struct msqid_ds msg_info;

    // 参数校验
    CHECK_NULL(qhandle, OSAL_ERROR);
    CHECK_NULL(name, OSAL_ERROR);
    if (msgsize == 0 || queue_length == 0) {
        fprintf(stderr, "Invalid msg size or queue length\n");
        return OSAL_ERROR;
    }

    // 分配句柄内存
    if (!(msg_handle = malloc(sizeof(msg_handle_t)))) {
        perror("malloc msg_handle failed");
        return OSAL_ERROR;
    }
    memset(msg_handle, 0, sizeof(msg_handle_t));

    // 构建文件路径
    snprintf(fileName, sizeof(fileName), "%s/%s", M_FIFO_NAME_PREFIX, name);
    strncpy(msg_handle->filename, fileName, sizeof(msg_handle->filename)-1);

    // 创建目录（若不存在）
    if (access(M_FIFO_NAME_PREFIX, F_OK) && 
        mkdir(M_FIFO_NAME_PREFIX, 0755) == -1) {
        perror("mkdir failed");
        goto error;
    }

    // 创建文件（若不存在）
    if (access(fileName, F_OK)) {
        unlink(fileName);
        if ((fd = open(fileName, O_RDONLY | O_CREAT, 0644)) == -1) {
            perror("open file failed");
            goto error;
        }
        close(fd);
    }

    // 生成键值与创建消息队列
    if ((key = ftok(fileName, M_QUEUE_ID)) == -1 ||
        (msg_handle->msg_id = msgget(key, IPC_CREAT | 0644)) == -1) {
        perror("ftok or msgget failed");
        goto error;
    }

    // 设置队列容量
    if (!msgctl(msg_handle->msg_id, IPC_STAT, &msg_info)) {
        msg_info.msg_qbytes = msgsize * queue_length;
        if (msgctl(msg_handle->msg_id, IPC_SET, &msg_info) == -1)
            perror("Warning: set queue size failed");
    } else {
        perror("Warning: get queue info failed");
    }

    msg_handle->msgsize = msgsize;
    *qhandle = msg_handle;
    return OSAL_SUCCESS;

error:
    if (fd != -1) close(fd);
    free(msg_handle);
    return OSAL_ERROR;
}

OSAL_RESULT_T osal_queue_send(osal_queue_t *qhandle, const void *msg, 
                            uint32_t len, uint32_t msecs)
{
    msg_t *qmsg;
    msg_handle_t *msg_handle;

    // 参数校验
    CHECK_NULL(qhandle, OSAL_ERROR);
    CHECK_NULL(msg, OSAL_ERROR);
    CHECK_NULL((msg_handle = *qhandle), OSAL_ERROR);

    // 消息长度校验
    if (len > msg_handle->msgsize) {
        fprintf(stderr, "Msg length exceeds limit\n");
        return OSAL_ERROR;
    }

    // 分配消息内存并发送
    if (!(qmsg = malloc(sizeof(long) + msg_handle->msgsize))) {
        perror("malloc qmsg failed");
        return OSAL_ERROR;
    }

    qmsg->type = 1;
    memset(qmsg->data, 0, msg_handle->msgsize);
    memcpy(qmsg->data, msg, len);

    if (msgsnd(msg_handle->msg_id, qmsg, msg_handle->msgsize, IPC_NOWAIT) == -1) {
        perror("msgsnd failed");
        free(qmsg);
        return OSAL_ERROR;
    }

    free(qmsg);
    return OSAL_SUCCESS;
}

OSAL_RESULT_T osal_queue_recv(osal_queue_t *qhandle, void *msg, uint32_t msecs)
{
    msg_t *qmsg;
    msg_handle_t *msg_handle;

    // 参数校验
    CHECK_NULL(qhandle, OSAL_ERROR);
    CHECK_NULL(msg, OSAL_ERROR);
    CHECK_NULL((msg_handle = *qhandle), OSAL_ERROR);

    // 分配消息内存并接收
    if (!(qmsg = malloc(sizeof(long) + msg_handle->msgsize))) {
        perror("malloc qmsg failed");
        return OSAL_ERROR;
    }

    if (msgrcv(msg_handle->msg_id, qmsg, msg_handle->msgsize, 0, IPC_NOWAIT) != -1) {
        memcpy(msg, qmsg->data, msg_handle->msgsize);
        free(qmsg);
        return OSAL_SUCCESS;
    }

    // 处理接收失败（忽略正常无消息情况）
    if (errno != ENOMSG)
        perror("msgrcv failed");
    free(qmsg);
    return OSAL_ERROR;
}

OSAL_RESULT_T osal_queue_delete(osal_queue_t *qhandle)
{
    msg_handle_t *msg_handle;

    // 参数校验
    CHECK_NULL(qhandle, OSAL_ERROR);
    CHECK_NULL((msg_handle = *qhandle), OSAL_ERROR);

    // 清理资源
    if (msgctl(msg_handle->msg_id, IPC_RMID, NULL) == -1)
        perror("msgctl remove failed");
    if (msg_handle->filename[0])
        unlink(msg_handle->filename);
    
    free(msg_handle);
    *qhandle = NULL;
    return OSAL_SUCCESS;
}