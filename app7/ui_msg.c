/**
 * @file ui_msg.c
 * @brief UI消息队列实现 - 线程安全的UI更新机制
 */

#include <stdio.h>
#include <string.h>
#include "ui_msg.h"
#include "osal_conf.h"   // OSAL_RESULT_T 等类型定义
#include "osal_queue.h"  // component/osal 已在include路径中

/* ========== 全局变量 ========== */
static osal_queue_t g_ui_queue = NULL;

/* ========== API实现 ========== */

/**
 * @brief 初始化UI消息队列
 */
int ui_msg_init(void)
{
    if (g_ui_queue != NULL) {
        printf("⚠️  UI message queue already initialized\n");
        return 0;
    }
    
    OSAL_RESULT_T ret = osal_queue_create(&g_ui_queue, "ui_msg_queue", sizeof(ui_msg_t), 20);
    if (ret != OSAL_SUCCESS) {
        printf("❌ Failed to create UI message queue\n");
        return -1;
    }
    
    printf("✅ UI message queue initialized (capacity: 20)\n");
    return 0;
}

/**
 * @brief 发送UI消息（线程安全）
 */
int ui_msg_send(ui_msg_t *msg)
{
    if (g_ui_queue == NULL) {
        printf("❌ UI queue not initialized\n");
        return -1;
    }
    
    if (msg == NULL) {
        return -1;
    }
    
    OSAL_RESULT_T ret = osal_queue_send(&g_ui_queue, msg, sizeof(ui_msg_t), 100);
    if (ret != OSAL_SUCCESS) {
        printf("⚠️  UI queue send failed (queue full?)\n");
        return -1;
    }
    
    return 0;
}

/**
 * @brief 接收UI消息（非阻塞）
 */
int ui_msg_recv(ui_msg_t *msg)
{
    if (g_ui_queue == NULL || msg == NULL) {
        return -1;
    }
    
    // 非阻塞接收（timeout=0）
    OSAL_RESULT_T ret = osal_queue_recv(&g_ui_queue, msg, 0);
    return (ret == OSAL_SUCCESS) ? 0 : -1;
}
