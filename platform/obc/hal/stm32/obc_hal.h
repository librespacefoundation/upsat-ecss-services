#ifndef __OBC_HAL_H
#define __OBC_HAL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <cmsis_os.h>
#include "services.h"
#include "task.h"
#include "obc.h"

extern SPI_HandleTypeDef hspi3;
extern RTC_HandleTypeDef hrtc;
extern IWDG_HandleTypeDef hiwdg;

extern uint8_t su_inc_buffer[197];

extern struct _obc_data obc_data;

extern struct _wdg_state wdg;



void HAL_obc_SD_ON();

void HAL_obc_SD_OFF();

void HAL_sys_delay(uint32_t sec);

void HAL_reset_source(uint8_t *src);

void HAL_sys_setTime(uint8_t hours, uint8_t mins, uint8_t sec);

void HAL_sys_getTime(uint8_t *hours, uint8_t *mins, uint8_t *sec);

void HAL_sys_setDate(uint8_t mon, uint8_t date, uint8_t year);

void HAL_sys_getDate(uint8_t *mon, uint8_t *date, uint8_t *year);

void HAL_obc_enableBkUpAccess();

uint32_t * HAL_obc_BKPSRAM_BASE();

uint32_t HAL_sys_GetTick();

void wdg_reset();

#endif
