/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:03 
 * @Last Modified by:   xiaozhi 
 * @Last Modified time: 2024-09-24 23:31:03 
 */
#include <stdio.h>
#include "page_conf.h"

void delete_current_page(lv_style_t *style){
    lv_obj_t * act_scr = lv_scr_act();
    lv_disp_t * d = lv_obj_get_disp(act_scr);
	if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
	{
		lv_obj_clean(act_scr);
        lv_style_reset(style);
	}
}