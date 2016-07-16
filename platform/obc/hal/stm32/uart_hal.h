#ifndef __UART_HAL_H
#define __UART_HAL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <cmsis_os.h>
#include "services.h"
#include "upsat.h"

void uart_timeout_start(UART_HandleTypeDef *huart);

void uart_timeout_stop(UART_HandleTypeDef *huart);

void uart_timeout_check(UART_HandleTypeDef *huart);

SAT_returnState hal_kill_uart(TC_TM_app_id app_id);


void HAL_OBC_UART_IRQHandler(UART_HandleTypeDef *huart);

void UART_OBC_Receive_IT(UART_HandleTypeDef *huart);

HAL_StatusTypeDef UART_OBC_SU_Receive_IT( UART_HandleTypeDef *huart);

SAT_returnState HAL_uart_tx_check(TC_TM_app_id app_id);

void HAL_uart_tx(TC_TM_app_id app_id, uint8_t *buf, uint16_t size);

SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data);

void HAL_su_uart_tx(uint8_t *buf, uint16_t size);

SAT_returnState HAL_su_uart_rx();

#endif
