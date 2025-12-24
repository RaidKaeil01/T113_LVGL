#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define DELAY_WAIT_TIME usleep(100*1000); 

// 定义 HID 报告长度
#define HID_REPORT_LENGTH 8

static int fd = -1;

// 发送 HID 报告到设备
void send_hid_report(int fd, unsigned char report[HID_REPORT_LENGTH])
{
    ssize_t written;
    int retries = 3; // 重试次数
    if(fd == -1)
        return;
    while (retries > 0)
    {
        printf("send_hid_report\n");
        written = write(fd, report, HID_REPORT_LENGTH);
        if (written == HID_REPORT_LENGTH)
        {
            return;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            DELAY_WAIT_TIME; // 等待一段时间后重试
            retries--;
        }
        else
        {
            perror("write");
        }
    }
    printf("Failed to send HID report after multiple attempts.\n");
}

// 字符到 HID 键码的映射
unsigned char char_to_hid_keycode(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 0x04;
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 0x04;
    }
    else if (c >= '0' && c <= '9')
    {
        if (c == '0') {
            return 0x27; // 数字 0 对应的 HID 键码
        }
        return c - '1' + 0x1E; // 数字 1 - 9 对应的 HID 键码
    }
    return 0;
}

/*
   键位对应协议说明
 * buffer[0] - bit0: Left CTRL
 *           -bit1: Left SHIFT
 *           -bit2: Left ALT
 *           -bit3: Left GUI(win)
 *           -bit4: Right CTRL
 *           -bit5: Right SHIFT
 *           -bit6: Right ALT
 *           -bit7: Right GUI ()
 * buffer[1] - Padding = Always 0x00
 * buffer[2] - Key 1
 * buffer[3] - Key 2
 * buffer[4] - Key 3
 * buffer[5] - Key 4
 * buffer[6] - Key 5
 * buffer[7] - Key 6
 */
// 模拟按下 CTRL + 字符 的组合键
void send_ctrl_combination(char c)
{
    unsigned char report[HID_REPORT_LENGTH] = {0};

    // 按下 CTRL + 字符
    // 第 1 个字节设置为 0x01 表示按下左 CTRL 键
    report[0] = 0x01;
    // 根据字符获取对应的 HID 键码
    report[2] = char_to_hid_keycode(c);
    send_hid_report(fd, report);

    // 短暂延迟，模拟按键按下时间
    DELAY_WAIT_TIME;

    // 释放 CTRL + 字符
    // 清空报告，所有按键释放
    for (int i = 0; i < HID_REPORT_LENGTH; i++)
    {
        report[i] = 0;
    }
    send_hid_report(fd, report);
    DELAY_WAIT_TIME;
}

// 模拟按下 CTRL + ALT + 字符 的组合键
void send_ctrl_alt_combination(char c)
{
    printf("Send HID Cmd = CTRL+ALT+%c\n",c);
    unsigned char report[HID_REPORT_LENGTH] = {0};

    // 按下 CTRL + ALT + 字符
    // 第 1 个字节设置为 0x05 表示按下左 CTRL 键和左 ALT 键
    report[0] = 0x05;
    // 根据字符获取对应的 HID 键码
    report[2] = char_to_hid_keycode(c);
    send_hid_report(fd, report);

    // 短暂延迟，模拟按键按下时间
    DELAY_WAIT_TIME;

    // 释放 CTRL + ALT + 字符
    // 清空报告，所有按键释放
    for (int i = 0; i < HID_REPORT_LENGTH; i++)
    {
        report[i] = 0;
    }
    send_hid_report(fd, report);
    DELAY_WAIT_TIME;
}

// 模拟按下单个字符键
void send_single_char(int fd, char c)
{
    unsigned char report[HID_REPORT_LENGTH] = {0};
    unsigned char modifier = 0;

    // 如果是大写字母，按下 SHIFT 键
    if (c >= 'A' && c <= 'Z')
    {
        modifier = 0x02;
    }

    report[0] = modifier;
    report[2] = char_to_hid_keycode(c);

    // 发送按下按键的报告
    send_hid_report(fd, report);

    // 短暂延迟，模拟按键按下时间
    DELAY_WAIT_TIME;

    // 清空报告，模拟按键释放
    for (int i = 0; i < HID_REPORT_LENGTH; i++)
    {
        report[i] = 0;
    }
    send_hid_report(fd, report);
    DELAY_WAIT_TIME;
}

// 输入字符串的功能接口
void send_string(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        send_single_char(fd, str[i]);
    }
}

void hid_keyboard_init(){
    printf("hid_keyboard_init\n");
    fd = open("/dev/hidg0", O_WRONLY);
    if (fd == -1)
    {
        perror("open");
    }
}

void hid_keyboard_deinit(){
    close(fd);
    printf("hid_keyboard_deinit\n");
}

int test_hid_utils()
{
    usleep(2000000);
    // 打开 HID 设备文件
    hid_keyboard_init();

    // 定义要输入的字符串
    const char *input_str = "Hello, World!";
    // 调用输入字符串的函数
    send_string(input_str);
    printf("CTRL+A\n");
    send_ctrl_combination('A');
    printf("CTRL+ALT+I\n");
    send_ctrl_alt_combination('I');

    // 关闭 HID 设备文件
    hid_keyboard_deinit();

    return EXIT_SUCCESS;
}