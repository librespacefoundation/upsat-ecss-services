#ifndef __OBC_HAL_H
#define __OBC_HAL_H

#include <stdint.h>
#include "services.h"

SAT_returnState queuePush(tc_tm_pkt *pkt, TC_TM_app_id app_id);

tc_tm_pkt * queuePop(TC_TM_app_id app_id);

uint8_t queueSize(TC_TM_app_id app_id);

tc_tm_pkt * queuePeak(TC_TM_app_id app_id);

void HAL_obc_SD_ON();

void HAL_obc_SD_OFF();

void HAL_sys_delay(uint32_t sec);

void HAL_reset_source(uint8_t *src);

void HAL_sys_setTime(uint8_t hours, uint8_t mins, uint8_t sec);

void HAL_sys_getTime(uint8_t *hours, uint8_t *mins, uint8_t *sec);

void HAL_sys_setDate(uint8_t weekday, uint8_t mon, uint8_t date, uint8_t year);

void HAL_sys_getDate(uint8_t *weekday, uint8_t *mon, uint8_t *date, uint8_t *year);

void HAL_obc_enableBkUpAccess();

uint32_t * HAL_obc_BKPSRAM_BASE();

uint32_t HAL_sys_GetTick();

void wdg_reset();

#endif
