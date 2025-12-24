#ifndef _BLE_MESH_H_
#define _BLE_MESH_H_

#include "device_data.h"

void send_tt_cmd(uint8_t *cmd,uint8_t len);

void ctrl_rgb_and_switch(uint8_t r,uint8_t g, uint8_t b,uint8_t switch_state);

void scan_adv_ctl(bool state);

void ble_mesh_init(void);

#endif

