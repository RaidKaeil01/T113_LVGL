#ifndef _USB_HID_UTILS_H_
#define _USB_HID_UTILS_H_

void hid_keyboard_init();

void hid_keyboard_deinit();

// 模拟按下 CTRL + ALT + 字符 的组合键
void send_ctrl_alt_combination(char c);

// 模拟按下 CTRL + 字符 的组合键
void send_ctrl_combination(char c);

#endif
