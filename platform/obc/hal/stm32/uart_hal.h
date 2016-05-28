#ifndef __UART_HAL_H
#define __UART_HAL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <cmsis_os.h>
#include "services.h"
#include "task.h"
#include "obc.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart6;
extern TaskHandle_t xTask_UART;

struct _uart_timeout
{
    uint32_t su;
    uint32_t adcs;
    uint32_t eps;
    uint32_t comms;
};

void uart_timeout_start(UART_HandleTypeDef *huart);

void uart_timeout_stop(UART_HandleTypeDef *huart);

void uart_timeout_check(UART_HandleTypeDef *huart);


void HAL_OBC_UART_IRQHandler(UART_HandleTypeDef *huart);

void UART_OBC_Receive_IT(UART_HandleTypeDef *huart);

HAL_StatusTypeDef UART_OBC_SU_Receive_IT( UART_HandleTypeDef *huart);

void HAL_uart_tx(TC_TM_app_id app_id, uint8_t *buf, uint16_t size);

SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data);

void HAL_su_uart_tx(uint8_t *buf, uint16_t size);

SAT_returnState HAL_su_uart_rx();

#endif
