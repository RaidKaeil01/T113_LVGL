#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lvgl.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "ble_mesh.h"
#include "utils.h"
#include "http_manager.h"
#include "audio_player_async.h"

extern void lv_port_disp_init(bool is_disp_orientation);
extern void lv_port_indev_init(void);
static uint32_t time_till_next = 1;

int main() {
    system_signal_init();
    init_device_state();
    device_state_t *state = get_device_state();
    lv_init();
    lv_port_disp_init(state->is_disp_orientation);
    lv_port_indev_init();
    FONT_INIT();
    device_timer_init();
    init_async_audio_player();
#ifdef SIMULATOR_LINUX
    // Linux模拟器无ble mesh
    printf("Running in Linux simulator mode.\n");
#else
    printf("Running in T113 mode.111\n");
    wpa_manager_open();
    ble_mesh_init();
#endif
    http_request_create();
    init_page_main();
    while (1) {
        time_till_next = lv_task_handler();
        /**
         * 延时，保证cpu占有率不会过高
         */
        usleep(time_till_next*1000);
    }
    return 0;
}