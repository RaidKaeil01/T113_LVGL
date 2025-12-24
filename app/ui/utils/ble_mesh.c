/*
 * @Author: xiaozhi
 * @Date: 2024-09-28 20:16:14
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-29 01:28:10
 */
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include "ble_mesh.h"
#include "em_hal_uart.h"
#include "device_data.h"

/**
主机初始化
-设置BLE名称-SET_BLE_NAME-主机为ss_master 从机为ss_device
-查询BLE名称-GET_BLE_NAME
-设置自动重连MAC列表SET_BLE_RECONNECT_LIST ssdevice0001/0002
-开启自动重连 SET_BLE_AUTO_RECON
-开启循环
--监听是否连接成功-接收到CONNECTED:0,MAC
--扫描自定义广播GET_BLE_SCAN_ADV-解析内容
--超过10s未接收到设备状态，判断离线

从机初始化
-判断设备类型1/2/3/4/5
-设置BLE名称-SET_BLE_NAME-主机为ss_master 从机为ss_device_000n
-查询BLE名称-GET_BLE_NAME
-设置MAC-SET_BLE_MAC  12位字符串 ssdevice0001/0002/0003/0004/0005
-查询MAC-GET_BLE_MAC 
-可控制设备-等待连接成功-接收到CONNECTED:1,MAC
---上报状态&等待控制-检测控制指令是否是控制当前设备类型
-不可控制设备
---开启自定义广播 SET_BLE_SCAN_ADV，上报状态

协议
包头 包长度 设备类型 数据 包尾/crc8
0xAA 0xnn 0xnn ... 0x55
RGB-可控制-设备类型1
CTL  0xAA 0x06 0x01 R G B state 0x55
REP 0xAA 0x07 0x01 R G B state battery 0x55
舵机-可控制-设备类型2 暂时不用，合并到设备类型1中
CTL  0xAA 0x03 0x02 angle 0x55
REP 0xAA 0x04 0x02 angle battery 0x55
火焰-仅上报-设备类型3
REP 0xAA 0x04 0x03 status battery 0x55
人体-仅上报-设备类型4
REP 0xAA 0x04 0x04 status battery 0x55
杯垫-仅上报-设备类型5
REP 0xAA 0x04 0x05 status battery 0x55
 */

#ifdef SIMULATOR_LINUX
    #define BLE_UART_NUM "/dev/ttyUSB0"
#else
    #define BLE_UART_NUM "/dev/ttyS0"
#endif

#define BLE_UART_BAUD_RATE 115200
#define BLE_UART_TIMEOUTS 0

#define BLE_ACK_OK "OK\r\n"
#define BLE_ACK_ERROR "ERROR\r\n"

#define GET_BLE_NAME "AT+NAME?\r\n"                 // 查询BLE名称
#define SET_BLE_NAME "AT+NAME=%s\r\n"               // 修改BLE名称
#define GET_BLE_MAC "AT+MAC?\r\n"                   // 查询MAC
#define SET_BLE_MAC "AT+MAC=%s\r\n"                 // 设置MAC 12位字符串
#define SET_BLE_REBOOT "AT+REBOOT=1\r\n"            // 重启模组
#define SET_BLE_DISONNECT "AT+DISCONN=1\r\n"        // 0-断开所有连接的从设备 1-断开与主机端连接
#define SET_BLE_CONNECT "AT+CONN=%s,0\r\n"          // 连接从机 <MAC TYP>地址类型，0-静态地址1-随机地址 <MAC TYP>值为0时，输入TYP参数可缺省，即输入AT+CONN=<MAC><CR><LF>即可
#define GET_BLE_CON_DEV "AT+DEV?\r\n"               // 查询当前已连接设备
#define SET_BLE_RECONNECT_LIST "AT+AUTO_MAC=%s,0\r\n" // 设置自动重连MAC列表
#define SET_BLE_AUTO_RECON "AT+AUTO_CFG=1\r\n"      // 开启自动重连
#define BLE_DEVICE_MAC1 "FFFFFFFF0001"              // ble从设备1
#define BLE_DEVICE_MAC2 "FFFFFFFF0002"              // ble从设备2
#define BLE_DEVICE_MAC3 "FFFFFFFF0003"              // ble从设备3
#define BLE_DEVICE_MAC4 "FFFFFFFF0004"              // ble从设备4
#define BLE_DEVICE_MAC5 "FFFFFFFF0005"              // ble从设备5


/*
模组默认服务UUID
透传数据通道【服务 UUID：0xFFF0】
0xFFF2 Write 写入的数据将会从串口TX 输出
0xFFF1 notify 从串口RX 输入的数据将会在此通道产生通知发给移动设备
*/
// 备注：16bit 格式 UUID 示例：FFF0
// 128bit 格式 UUID 示例：11223344556677889900112233445566
#define SET_BLE_UUIDS "AT+UUIDS=%s\r\n" // 设置主UUID 16bit 格式或 128bit 格式的 UUID
#define SET_BLE_UUIDR "AT+UUIDN=%s\r\n" // 设置写UUID
#define SET_BLE_UUIDW "AT+UUIDW=%s\r\n" // 设置读UUID

#define SET_BLE_ADV_INTVL "AT+AINTVL=%s\r\n"  // 设置广播间隔 支持参数：20-10240 单位毫秒
#define GET_BLE_ADV_INTVL "AT+AINTVL?\r\n"    // 查询广播间隔
#define GET_BLE_VERSION "AT+VER?\r\n"         // 查询软件版本
#define SET_BLE_ADV_POWER "AT+TXPOWER=%s\r\n" // 设置广播功率 支持参数：-43/ -38/ -33/ -30/ -25/ -20/ -16/ -10/ -8/ -6/ -5/ -4/ -3/ -1/ 0/ 2/ 4/ 6/7/8/9/10
#define GET_BLE_ADV_POWER "AT+TXPOWER?\r\n"   // 查询广播功率

/**
+SCAN:<CR><LF>
<MAC>空格<TYP>空格<RSSI>空格<DEVICE NAME><CR><LF>…
+SCAN END<CR><LF>
 */
#define SET_BLE_SCAN "AT+SCAN=1\r\n" // 0-立即关闭扫描功能 1-开启扫描功能（扫描持续时间为 6S，6S 后停止扫描）
/**
+SCAN_MANU:<CR><LF>
<MAC>空格<TYP>空格<RSSI>空格<Manufacturer SpecificData><CR><LF> …
+SCAN END<CR><LF>
 */
#define OPEN_BLE_SCAN_ADV "AT+SCAN_MANU=1\r\n"  // 开启扫描蓝牙自定义广播数据
#define CLOSE_BLE_SCAN_ADV "AT+SCAN_MANU=0\r\n" // 关闭扫描蓝牙自定义广播数据
#define SET_BLE_SCAN_ADV "AT+AMDATA=%s\r\n"   // 设置自定义广播数据 <HEX>为 0-29 字节长度的 HEX 数值，如设置广播数据为5 个字节“12345”，则对应格为“AT+AMDATA=3132333435\r\n”


#define SET_BLE_RESET "AT+RESET=1\r\n"

// transparent transmission
#define BLE_TRANSPARENT_TRANSMISSION_PACK_HEAD 0xAA
#define BLE_TRANSPARENT_TRANSMISSION_PACK_END 0x55

#define BLE_ADV_MAX_LINES 30
#define BLE_ADV_MAX_LINES_LENGTH 100
#define BLE_ADV_MAX_MSG_LENGTH 200

typedef enum
{
    BLE_MESH_DEVICE_TYPE_RGB = 1,
    BLE_MESH_DEVICE_TYPE_SWITCH,
    BLE_MESH_DEVICE_TYPE_BODY_SENSOR,
    BLE_MESH_DEVICE_TYPE_SMART_COASTER,
    BLE_MESH_DEVICE_TYPE_FLAME_SENSOR,
} BLE_MESH_DEVICE_TYPE_E;

typedef struct
{
    long type;       // 消息类型
    int len;
    unsigned char buffer[BLE_ADV_MAX_MSG_LENGTH];
} ble_msg_t;

unsigned char adv_scan_buffer[2000];
bool is_adv_scan_runing = false;

static int dev_uart_fd;
static uart_dev_t dev;
static CONNECT_STATE_E ble_state = CONNECTING;

// 消息队列id
static int msgid;
// 互斥量
static pthread_mutex_t mutex;
// 线程
static pthread_t ble_mesh_readthread;   //读取串口接收数据，发送到消息队列
static pthread_t ble_mesh_handlethread; //读取消息队列数据，进行处理

static void read_uart_msg_to_queue()
{
    int ret = 1;
    // unsigned char hex_array[] = {0xaa, 0x03, 0x04, 0x00, 0x55};
    ble_msg_t msg;
    msg.type = 1;
    msg.len = BLE_ADV_MAX_MSG_LENGTH;
    // memcpy(msg.buffer,hex_array,sizeof(hex_array));
    // ret = sizeof(hex_array);
    ret = em_hal_uart_read(dev_uart_fd,msg.buffer,msg.len,BLE_UART_TIMEOUTS);
    if (ret >= 0)
    {
        msg.len = ret;
        printf("receive msg len %d\n",ret);
        ret = msgsnd(msgid, &msg, sizeof(int) + msg.len, 0);
        if (ret == -1)
        {
            printf("queue msgsnd error");
            return;
        }
    }
}

static void hex_to_string()
{
    unsigned char hex_array[] = {0xaa, 0x03, 0x04, 0x00, 0x55};
    char str[10];
    for (int i = 0; i < sizeof(hex_array); i++)
    {
        sprintf(str + i * 2, "%02x", hex_array[i]);
    }
    printf("%s\n", str);
}

static void string_to_hex(char *input, unsigned char *output, size_t output_len)
{
    size_t i;
    size_t len = strlen(input);
    if (len % 2 != 0)
    {
        printf("Error: Input string length must be even.\n");
        return;
    }
    if (len / 2 > output_len)
    {
        printf("Error: Output buffer size too small.\n");
        return;
    }
    for (i = 0; i < len; i += 2)
    {
        sscanf(input + i, "%2hhx", &output[i / 2]);
    }
}

/**
 * 透传消息处理
 */
static void handle_tt_msg(ble_msg_t msg)
{
    printf("cmd = ");
    for (int i = 0; i < msg.len; i++)
    {
        printf("%x ", msg.buffer[i]);
    }
    if (msg.buffer[0] == BLE_TRANSPARENT_TRANSMISSION_PACK_HEAD)
    { // 校验包头
        int pack_len = msg.buffer[1];
        if (msg.len != pack_len + 2)
        {
            printf("msg len error\n");
            return;
        }
        device_state_t *state = get_device_state();
        if (msg.buffer[pack_len + 1] == BLE_TRANSPARENT_TRANSMISSION_PACK_END)
        { // 校验包尾
            int device_id = msg.buffer[2];
            switch (device_id)
            {
            case BLE_MESH_DEVICE_TYPE_RGB:
                state->led_connect_state = CONNECT;
                state->switch_connect_state = CONNECT;
                printf("msg from BLE_MESH_DEVICE_TYPE_RGB\n");
                state->led_r = msg.buffer[3];
                state->led_g = msg.buffer[4];
                state->led_b = msg.buffer[5];
                state->mesh_switch_state = msg.buffer[6];
                state->switch_battery_level = msg.buffer[7];
                break;
            case BLE_MESH_DEVICE_TYPE_SWITCH:
                state->switch_connect_state = CONNECT;
                printf("msg from BLE_MESH_DEVICE_TYPE_SWITCH\n");
                state->mesh_switch_state = msg.buffer[3];
                state->switch_battery_level = msg.buffer[4];
                break;
            case BLE_MESH_DEVICE_TYPE_BODY_SENSOR:
                state->body_sensor_connect_state = CONNECT;
                printf("msg from BLE_MESH_DEVICE_TYPE_BODY_SENSOR\n");
                state->body_sensor_data = msg.buffer[3];
                state->body_sensor_battery_level = msg.buffer[4];
                break;
            case BLE_MESH_DEVICE_TYPE_SMART_COASTER:
                state->smart_coaster_connect_state = CONNECT;
                printf("msg from BLE_MESH_DEVICE_TYPE_SMART_COASTER\n");
                state->smart_coaster_data = msg.buffer[3];
                state->smart_coaster_battery_level = msg.buffer[4];
                break;
            case BLE_MESH_DEVICE_TYPE_FLAME_SENSOR:
                state->flame_sensor_connect_state = CONNECT;
                printf("msg from BLE_MESH_DEVICE_TYPE_FLAME_SENSOR\n");
                state->flame_sensor_data = msg.buffer[3];
                state->flame_sensor_battery_level = msg.buffer[4];
                break;
            default:
                break;
            }
        }
        else
        {
            printf("msg pack end error !!! \n");
        }
    }
    else
    {
        printf("msg pack head error !!! \n");
    }
}

void handle_adv_data(char *data)
{
    char lines[BLE_ADV_MAX_LINES][BLE_ADV_MAX_LINES_LENGTH];
    char items[BLE_ADV_MAX_LINES][BLE_ADV_MAX_LINES_LENGTH];
    char *line;
    char *item;
    int line_count = 0;
    int item_count = 0;

    // 逐行分割字符串
    line = strtok(data, "\r\n");
    while (line != NULL && line_count < BLE_ADV_MAX_LINES)
    {
        strcpy(lines[line_count], line);
        line = strtok(NULL, "\r\n");
        line_count++;
    }
    // 逐行解析
    for (int i = 0; i < line_count; i++)
    {
        item_count = 0;
        item = strtok(lines[i], " ");
        while (item != NULL && item_count < BLE_ADV_MAX_LINES)
        {
            strcpy(items[item_count], item);
            item = strtok(NULL, " ");
            item_count++;
        }
        bool found_data = false;
        for (int j = 0; j < item_count; j++)
        {
            found_data = false;
            if (strcmp(items[j], BLE_DEVICE_MAC3) == 0)
            {
                if (j + 3 < item_count)
                {
                    printf("Found '%s', cmd: %s\n", BLE_DEVICE_MAC3, items[j + 3]);
                    found_data = true;
                }
            }
            else if (strcmp(items[j], BLE_DEVICE_MAC4) == 0)
            {
                if (j + 3 < item_count)
                {
                    printf("Found '%s', cmd: %s\n", BLE_DEVICE_MAC4, items[j + 3]);
                    found_data = true;
                }
            }
            else if (strcmp(items[j], BLE_DEVICE_MAC5) == 0)
            {
                if (j + 3 < item_count)
                {
                    printf("Found '%s', cmd: %s\n", BLE_DEVICE_MAC5, items[j + 3]);
                    found_data = true;
                }
            }
            if (found_data)
            {
                char *input = items[j + 3];
                ble_msg_t msg;
                msg.len = strlen(items[j + 3]) / 2;
                string_to_hex(input, msg.buffer, msg.len);
                handle_tt_msg(msg);
            }
        }
    }
    return;
}

/**
 * 串口消息处理
 */
static void handle_uart_msg()
{
    int ret;
    ble_msg_t msg;
    memset(msg.buffer,0,sizeof(msg.buffer));
    ret = msgrcv(msgid, &msg, sizeof(msg), 1, 0);
    if (ret == -1)
    {
        printf("queue msgrcv error");
        return;
    }
    msg.buffer[msg.len] = '\0';
    printf("handle message: len = %d %s\n", msg.len,msg.buffer);
    device_state_t *state = get_device_state();
    if (strstr((const char *)msg.buffer, "CONNECTED:") != NULL)
    {
        printf("handle connect msg = %s\n", msg.buffer);
        if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC1) != NULL)
        {
            // 已连接设备1、2，设备1&2合二为一
            printf("已连接设备1&2 %s\n",BLE_DEVICE_MAC1);
            state->led_connect_state = CONNECT;
            state->switch_connect_state = CONNECT;
        }
        else if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC3) != NULL)
        {
            printf("已连接设备3 %s\n",BLE_DEVICE_MAC3);
            state->body_sensor_connect_state = CONNECT;
        }
        else if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC4) != NULL)
        {
            printf("已连接设备4 %s\n",BLE_DEVICE_MAC4);
            state->smart_coaster_connect_state = CONNECT;
        }
        else if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC5) != NULL)
        {
            printf("已连接设备5 %s\n",BLE_DEVICE_MAC5);
            state->flame_sensor_connect_state = CONNECT;
        }
    }
    else if (strstr((const char *)msg.buffer, "DISCONN:") != NULL)
    {
        printf("handle disconnect msg = %s\n", msg.buffer);
        if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC1) != NULL)
        {
            // 已断开设备1&2连接
            printf("已断开设备1&2连接 %s\n",BLE_DEVICE_MAC1);
            state->led_connect_state = DISCONNECT;
            state->switch_connect_state = DISCONNECT;
            state->switch_battery_level = 0;
        }
        else if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC3) != NULL)
        {
            printf("已断开设备3连接 %s\n",BLE_DEVICE_MAC3);
            state->body_sensor_connect_state = DISCONNECT;
            state->body_sensor_battery_level = 0;
        }
        else if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC4) != NULL)
        {
            printf("已断开设备4连接 %s\n",BLE_DEVICE_MAC4);
            state->smart_coaster_connect_state = DISCONNECT;
            state->smart_coaster_battery_level = 0;
        }
        else if (strstr((const char *)msg.buffer, BLE_DEVICE_MAC5) != NULL)
        {
            printf("已断开设备5连接 %s\n",BLE_DEVICE_MAC5);
            state->flame_sensor_connect_state = DISCONNECT;
            state->flame_sensor_battery_level = 0;
        }
    }
    else if (strstr((const char *)msg.buffer, "SCAN_MANU:") != NULL)
    {
        printf("handle adv msg\n");
        is_adv_scan_runing = true;
        memset(adv_scan_buffer,0,sizeof(adv_scan_buffer));
        strcpy(adv_scan_buffer,msg.buffer);
        // 扫描自定义广播数据
        // handle_adv_data(msg.buffer);
    }
    else if (strstr((const char *)msg.buffer, "SCAN END") != NULL)
    {
        printf("handle adv end\n");
        is_adv_scan_runing = false;
        strcat(adv_scan_buffer,msg.buffer);
        // 扫描自定义广播数据
        handle_adv_data(adv_scan_buffer);
    }
    else if(strstr((const char *)msg.buffer, "\r\n") != NULL){
        if(is_adv_scan_runing){
            strcat(adv_scan_buffer,msg.buffer);
        }else{
            printf("unhandle cmd\n");
        }
    }
    else
    {
        printf("handle tt msg\n");
        // 透传数据
        // char *input = msg.buffer;
        // ble_msg_t msg_temp;
        // msg_temp.len = strlen(msg.buffer) / 2;
        // string_to_hex(input, msg_temp.buffer, msg_temp.len);
        handle_tt_msg(msg);
    }
}

static bool send_cmd(uint8_t *cmd, uint8_t *ack)
{
    int ret;
    int time_out = 5;
    printf("send ----> %s\n", cmd);
    em_hal_uart_write(dev_uart_fd, cmd, strlen(cmd));
    while (time_out--)
    {
        usleep(200 * 1000); // 200ms
        ble_msg_t msg;
        ret = msgrcv(msgid, &msg, sizeof(msg), 1, IPC_NOWAIT);
        if (ret == -1)
        {
            em_hal_uart_write(dev_uart_fd, cmd, strlen(cmd));
        }
        else
        {
            msg.buffer[msg.len] = '\0';
            printf("rec message: len = %d data = %s\n", msg.len, msg.buffer);
            if (strstr((const char *)msg.buffer, (const char *)ack) != NULL)
            {
                printf("ack ok\n");
            }
            // usleep(1000 * 1000);
            return true;
        }
    }
    printf("ack fail\n");
    ble_state = DISCONNECT;
    return false;
}

static void init_at_device()
{
    char tx_buf[100];
    
    send_cmd(SET_BLE_REBOOT, "OK");

    send_cmd(SET_BLE_RESET, "OK");

    memset(tx_buf, 0, sizeof(tx_buf));
    sprintf((char *)tx_buf, SET_BLE_NAME, "SS_MASTER"); // smart screen master device
    send_cmd(tx_buf, BLE_ACK_OK);
    send_cmd(GET_BLE_NAME, "MAC:");

    memset(tx_buf, 0, sizeof(tx_buf));
    sprintf((char *)tx_buf, SET_BLE_ADV_POWER, "10");
    send_cmd(tx_buf, BLE_ACK_OK);
    
    memset(tx_buf, 0, sizeof(tx_buf));
    sprintf((char *)tx_buf, SET_BLE_RECONNECT_LIST, BLE_DEVICE_MAC1);
    send_cmd(tx_buf, BLE_ACK_OK);

    // memset(tx_buf, 0, sizeof(tx_buf));
    // sprintf((char *)tx_buf, SET_BLE_RECONNECT_LIST, BLE_DEVICE_MAC2);
    // send_cmd(tx_buf, BLE_ACK_OK);

    memset(tx_buf, 0, sizeof(tx_buf));
    sprintf((char *)tx_buf, SET_BLE_RECONNECT_LIST, BLE_DEVICE_MAC3);
    send_cmd(tx_buf, BLE_ACK_OK);

    memset(tx_buf, 0, sizeof(tx_buf));
    sprintf((char *)tx_buf, SET_BLE_RECONNECT_LIST, BLE_DEVICE_MAC4);
    send_cmd(tx_buf, BLE_ACK_OK);

    memset(tx_buf, 0, sizeof(tx_buf));
    sprintf((char *)tx_buf, SET_BLE_RECONNECT_LIST, BLE_DEVICE_MAC5);
    send_cmd(tx_buf, BLE_ACK_OK);

    send_cmd(SET_BLE_AUTO_RECON, BLE_ACK_OK);

    if(ble_state != DISCONNECT){
        ble_state = CONNECT;
    }
    get_device_state()->ble_mesh_state = ble_state;
}

static void *readthread_function(void *arg)
{
    printf("ble mesh readthread_function run!\n");
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    while (1)
    {
        read_uart_msg_to_queue();
        usleep(100);
    }
    pthread_exit(NULL);
}

static void *handlethread_function(void *arg)
{
    printf("ble mesh handlethread_function run!\n");
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    init_at_device();
    while (1)
    {
        handle_uart_msg();
        usleep(10);
    }
    pthread_exit(NULL);
}

void ble_mesh_test(){
    char input[] = "+SCAN_MANU:\r\n"
                   "5EDC856415C 0 -50 \r\n"
                   "ssdevice0003 0 -50 aa03030055\r\n"
                   "ssdevice0004 0 -80 aa03040155\r\n"
                   "ssdevice0005 0 -100 aa03050055\r\n"
                   "+SCAN_END\r\n";
    handle_adv_data(input);

}

void send_tt_cmd(uint8_t *cmd,uint8_t len){
    printf("------------>send cmd\n");
    em_hal_uart_write(dev_uart_fd, cmd, len);
}

void ctrl_rgb_and_switch(uint8_t r,uint8_t g, uint8_t b,uint8_t switch_state){
    uint8_t cmd[] = {0xaa,0x06,0x01,0x00,0x00,0x00,0x00,0x55};
    cmd[3] = r;
    cmd[4] = g;
    cmd[5] = b;
    cmd[6] = switch_state;
    send_tt_cmd(cmd,sizeof(cmd));
}

void scan_adv_ctl(bool state){
    send_cmd(OPEN_BLE_SCAN_ADV, "SCAN_MANU:");
    // if(state){
    //     em_hal_uart_write(dev_uart_fd, OPEN_BLE_SCAN_ADV, strlen(OPEN_BLE_SCAN_ADV));
    // }else{
    //     em_hal_uart_write(dev_uart_fd, CLOSE_BLE_SCAN_ADV, strlen(CLOSE_BLE_SCAN_ADV));
    // }
}


void ble_mesh_init()
{
    int ret = 0;
    dev.speed = BLE_UART_BAUD_RATE;
    dev.path = BLE_UART_NUM;
    dev.bits = 8;
    dev.parity = 'N';
    dev.stop = 1;
    dev_uart_fd = em_hal_uart_open(&dev);

    // 创建消息队列
    msgid = msgget((key_t)123, IPC_CREAT | 0666);
    // 创建互斥量
    ret = pthread_mutex_init(&mutex, NULL);
    if (ret != 0)
    {
        printf("Error: pthread_mutex_init() failed\n");
        return;
    }
    // 创建接收和处理线程，接收和处理分离，避免处理耗时丢数据
    ret = pthread_create(&ble_mesh_readthread, NULL, readthread_function, NULL);
    if (ret != 0)
    {
        printf("Error: pthread_create() ble_mesh_readthread failed\n");
        return;
    }
    ret = pthread_create(&ble_mesh_handlethread, NULL, handlethread_function, NULL);
    if (ret != 0)
    {
        printf("Error: pthread_create() ble_mesh_handlethread failed\n");
        return;
    }
}