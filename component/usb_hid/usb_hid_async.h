
#ifndef _HID_ASYNC_H_
#define _HID_ASYNC_H_

int init_async_usb_hid(void);

void send_ctrl_combination_async(char c);

void send_ctrl_alt_combination_async(char c);

#endif