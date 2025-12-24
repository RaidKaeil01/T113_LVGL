#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "osal_thread.h"
#include "osal_queue.h"
#include "usb_hid_utils.h"

typedef enum
{
    HDI_TYPE_ID_CRTL_ALT = 0,
    HDI_TYPE_ID_CRTL,
}HDI_TYPE_ID_E;

typedef struct
{
    HDI_TYPE_ID_E id;
    char c;
}hid_obj;

static osal_queue_t hid_queue = NULL;
static osal_thread_t hid_thread = NULL;

static void handle_hid(osal_queue_t *hid_queue)
{
    int ret = OSAL_ERROR;
    
    hid_obj obj;
    memset(&obj, 0, sizeof(hid_obj));

    ret = osal_queue_recv(hid_queue, (void*)&obj, 100);
    if (ret == OSAL_SUCCESS)
    {
        HDI_TYPE_ID_E id = obj.id;
        switch(id)
        {
            case HDI_TYPE_ID_CRTL_ALT:
                send_ctrl_alt_combination(obj.c);
                break;
            case HDI_TYPE_ID_CRTL:
                send_ctrl_combination(obj.c);
                break;
            default:
                break;
        }
    }
}

static void* hid_thread_fun(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    while(1)
    {
        handle_hid(&hid_queue);
        osal_thread_sleep(1);
    }
}

// 模拟按下 CTRL + ALT + 字符 的组合键
void send_ctrl_alt_combination_async(char c){
    hid_obj obj;    
    memset(&obj, 0, sizeof(hid_obj));
    obj.id = HDI_TYPE_ID_CRTL_ALT;
    obj.c = c;
    int ret = osal_queue_send(&hid_queue, &obj, sizeof(hid_obj), 1000);
    if(ret == OSAL_ERROR)
    {
        printf("queue send error");
    }
}

// 模拟按下 CTRL + 字符 的组合键
void send_ctrl_combination_async(char c){
    hid_obj obj;    
    memset(&obj, 0, sizeof(hid_obj));
    obj.id = HDI_TYPE_ID_CRTL;
    obj.c = c;
    int ret = osal_queue_send(&hid_queue, &obj, sizeof(hid_obj), 1000);
    if(ret == OSAL_ERROR)
    {
        printf("queue send error");
    }
}

int init_async_usb_hid(void)
{
    static int ret = OSAL_ERROR;
    if(ret == OSAL_SUCCESS)
        return 0;
    ret = osal_queue_create(&hid_queue,"hid_queue",sizeof(hid_obj),50);
    if(ret == OSAL_ERROR)
    {
        printf("create queue error");
        return -1;
    }   
    ret = osal_thread_create(&hid_thread,hid_thread_fun, NULL);
    if(ret == OSAL_ERROR)
    {
        printf("create thread error");
        return -1;
    }
    hid_keyboard_init();
    ret = OSAL_SUCCESS;
    return 0;
}