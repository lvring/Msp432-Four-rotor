#ifndef __GUI_REFRESH_H
#define __GUI_REFRESH_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
extern void GUI_Refresh(void);
extern void refr_init(void);
extern void inv_area(const area_t * area_p,bool reverse);
extern void refr_set_monitor_cb(void (*cb)(uint32_t, uint32_t));
extern void refr_set_round_cb(void(*cb)(area_t*));
extern uint16_t refr_get_buf_size(void);
extern void refr_pop_from_buf(uint16_t num);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
