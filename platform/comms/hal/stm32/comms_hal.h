#ifndef __COMMS_HAL_H
#define __COMMS_HAL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "services.h"
#include "hldlc.h"
#include "upsat.h"

//ToDo
//  improve return types
//  need to make the change from the two different typedefs

extern struct _comms_data comms_data;


SAT_returnState HAL_takeMutex(TC_TM_app_id app_id);

SAT_returnState HAL_giveMutex(TC_TM_app_id app_id);

void HAL_comms_SD_ON();

void HAL_comms_SD_OFF();

void HAL_sys_delay(uint32_t sec);

void HAL_COMMS_UART_IRQHandler(UART_HandleTypeDef *huart);

void UART_COMMS_Receive_IT(UART_HandleTypeDef *huart);

SAT_returnState hal_kill_uart(TC_TM_app_id app_id);

SAT_returnState HAL_uart_tx_check(TC_TM_app_id app_id);

void HAL_uart_tx(TC_TM_app_id app_id, uint8_t *buf, uint16_t size);

SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data);

void HAL_reset_source(uint8_t *src);

uint32_t HAL_sys_GetTick();

#endif
