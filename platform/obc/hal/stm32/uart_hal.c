#include "uart_hal.h"

#include "task.h"
#include "obc.h"
#include "hldlc.h"
#include "su_mnlp.h"

extern SPI_HandleTypeDef hspi3;
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

static struct _uart_timeout uart_timeout; 

extern uint8_t uart_temp[];
uint8_t spi_data_buf[MAX_PKT_DATA];

SAT_returnState import_spi() {
  static uint8_t cnt;
  HAL_StatusTypeDef res;

  if(obc_data.iac_flag == true) {
      uint16_t size = 198;
      if((mass_storage_storeFile(FOTOS, 0, &obc_data.iac_in[5], &size)) != SATR_OK) { return SATR_ERROR; } 
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
      obc_data.iac_out[0] = cnt++;
      obc_data.iac_out[1] = cnt++;
      obc_data.iac_in[0] = 0xFA;
      obc_data.iac_in[1] = 0xAF;
      obc_data.iac_flag = false;
      res = HAL_SPI_TransmitReceive_IT(&hspi3, obc_data.iac_out, obc_data.iac_in, 16);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
      snprintf(spi_data_buf, MAX_PKT_DATA, "IAC Rec %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n", \
                                                                            obc_data.iac_in[0], \
                                                                            obc_data.iac_in[1], \
                                                                            obc_data.iac_in[2], \
                                                                            obc_data.iac_in[3], \
                                                                            obc_data.iac_in[4], \
                                                                            obc_data.iac_in[5], \
                                                                            obc_data.iac_in[6], \
                                                                            obc_data.iac_in[7], \
                                                                            obc_data.iac_in[8], \
                                                                            obc_data.iac_in[9], \
                                                                            obc_data.iac_in[10], \
                                                                            obc_data.iac_in[11], \
                                                                            obc_data.iac_in[12], \
                                                                            obc_data.iac_in[13], \
                                                                            obc_data.iac_in[14], \
                                                                            obc_data.iac_in[15] );
      event_dbg_api(uart_temp, spi_data_buf, &size);
      HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
  } else if( hspi3.State == HAL_SPI_STATE_READY) {
      res = HAL_SPI_TransmitReceive_IT(&hspi3, obc_data.iac_out, obc_data.iac_in, 16);
  }

}

SAT_returnState HAL_uart_tx_check(TC_TM_app_id app_id) {
    
    HAL_UART_StateTypeDef res;
    UART_HandleTypeDef *huart;

    if(app_id == EPS_APP_ID) { huart = &huart1; }
    else if(app_id == DBG_APP_ID) { huart = &huart3; }
    else if(app_id == COMMS_APP_ID) { huart = &huart4; }
    else if(app_id == ADCS_APP_ID) { huart = &huart6; }


    res = HAL_UART_GetState(huart);
    if(res != HAL_UART_STATE_BUSY && \
       res != HAL_UART_STATE_BUSY_TX && \
       res != HAL_UART_STATE_BUSY_TX_RX) { return SATR_ALREADY_SERVICING; }

    return SATR_OK;
}

void HAL_uart_tx(TC_TM_app_id app_id, uint8_t *buf, uint16_t size) {
    
    UART_HandleTypeDef *huart;

    if(app_id == EPS_APP_ID) { huart = &huart1; }
    else if(app_id == DBG_APP_ID) { huart = &huart3; }
    else if(app_id == COMMS_APP_ID) { huart = &huart4; }
    else if(app_id == ADCS_APP_ID) { huart = &huart6; }

    HAL_UART_Transmit_DMA(huart, buf, size);

}

SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data) {

    UART_HandleTypeDef *huart;

    if(app_id == EPS_APP_ID) { huart = &huart1; }
    else if(app_id == DBG_APP_ID) { huart = &huart3; }
    else if(app_id == COMMS_APP_ID) { huart = &huart4; }
    else if(app_id == ADCS_APP_ID) { huart = &huart6; }

    if(huart->RxState == HAL_UART_STATE_READY) {
        data->uart_size = huart->RxXferSize - huart->RxXferCount;
        for(uint16_t i = 0; i < data->uart_size; i++) { data->uart_unpkt_buf[i] = data->uart_buf[i]; }
        HAL_UART_Receive_IT(huart, data->uart_buf, UART_BUF_SIZE);
        return SATR_EOT;
    }
    return SATR_OK;
}

/**
  * @brief  This function handles UART interrupt request.
  * @param  huart: pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_OBC_UART_IRQHandler(UART_HandleTypeDef *huart)
{
  uint32_t tmp1 = 0U, tmp2 = 0U;

  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  { 
    UART_OBC_Receive_IT(huart);
  }
}
uint16_t err;
/**
  * @brief  Receives an amount of data in non blocking mode 
  * @param  huart: pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
void UART_OBC_Receive_IT(UART_HandleTypeDef *huart)
{
    uint8_t c;

    c = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FFU);
    if(huart->RxXferSize == huart->RxXferCount && c == HLDLC_START_FLAG) {
      *huart->pRxBuffPtr++ = c;
      huart->RxXferCount--;
      uart_timeout_start(huart);
    } else if(c == HLDLC_START_FLAG && (huart->RxXferSize - huart->RxXferCount) < TC_MIN_PKT_SIZE) {
      err++;
      huart->pRxBuffPtr -= huart->RxXferSize - huart->RxXferCount;
      huart->RxXferCount = huart->RxXferSize - 1;
      *huart->pRxBuffPtr++ = c;

      uart_timeout_start(huart);
    } else if(c == HLDLC_START_FLAG) {
      *huart->pRxBuffPtr++ = c;
      huart->RxXferCount--;
      
      uart_timeout_stop(huart);

      __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

      /* Disable the UART Parity Error Interrupt */
      __HAL_UART_DISABLE_IT(huart, UART_IT_PE);

      /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
      __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

	  /* Rx process is completed, restore huart->RxState to Ready */
      huart->RxState = HAL_UART_STATE_READY;

      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR(xTask_UART, &xHigherPriorityTaskWoken);
      
    } else if(huart->RxXferSize > huart->RxXferCount) {
      *huart->pRxBuffPtr++ = c;
      huart->RxXferCount--;
    } else {
      err++;
    }

    if(huart->RxXferCount == 0U) // errror
    {
        huart->pRxBuffPtr -= huart->RxXferSize - huart->RxXferCount;
        huart->RxXferCount = huart->RxXferSize;
        err++;
    }
}

void uart_timeout_start(UART_HandleTypeDef *huart) {

  // uint32_t t = HAL_GetTick();
  // if(huart == &huart1)      { uart_timeout. = t; }
  // else if(huart == &huart2) { uart_timeout. = t; }
  // else if(huart == &huart3) { uart_timeout. = t; }
  // else if(huart == &huart4) { uart_timeout. = t; }
  // else if(huart == &huart6) { uart_timeout. = t; }
}

void uart_timeout_stop(UART_HandleTypeDef *huart) {

  // uint32_t t = HAL_GetTick();
  // if(huart == &huart1)      { uart_timeout. = 0; }
  // else if(huart == &huart2) { uart_timeout. = 0; }
  // else if(huart == &huart3) { uart_timeout. = 0; }
  // else if(huart == &huart4) { uart_timeout. = 0; }
  // else if(huart == &huart6) { uart_timeout. = 0; }
}

void uart_timeout_check(UART_HandleTypeDef *huart) {

  // uint32_t t = HAL_GetTick();
  // if(huart == &huart1 && uart_timeout. != 0 && (t - uart_timeout. > TIMEOUT)) { 
  //   HAL_UART_Receive_IT(huart, data->uart_buf, UART_BUF_SIZE);
  // } else if(huart == &huart2 && uart_timeout. != 0 && (t - uart_timeout. > TIMEOUT)) { 
  // } else if(huart == &huart3 && uart_timeout. != 0 && (t - uart_timeout. > TIMEOUT)) { 
  // } else if(huart == &huart4 && uart_timeout. != 0 && (t - uart_timeout. > TIMEOUT)) { 
  // } else if(huart == &huart6 && uart_timeout. != 0 && (t - uart_timeout. > TIMEOUT)) { 
  //   HAL_UART_Receive_IT(huart, &su_inc_buffer[22], 173);
  // }
}

void HAL_OBC_SU_UART_IRQHandler(UART_HandleTypeDef *huart)
{
  uint32_t tmp1 = 0U, tmp2 = 0U;

  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  { 
    UART_OBC_SU_Receive_IT(huart);
  }
}

uint16_t err;

HAL_StatusTypeDef UART_OBC_SU_Receive_IT( UART_HandleTypeDef *huart)
{

    uart_timeout_start(huart);
    *huart->pRxBuffPtr++ = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FFU);
    if(--huart->RxXferCount == 0U)
    {

      uart_timeout_stop(huart);

      __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

      /* Disable the UART Parity Error Interrupt */
      __HAL_UART_DISABLE_IT(huart, UART_IT_PE);

      /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
      __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

	  /* Rx process is completed, restore huart->RxState to Ready */
      huart->RxState = HAL_UART_STATE_READY;
     
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      vTaskNotifyGiveFromISR(xTask_UART, &xHigherPriorityTaskWoken);

      return HAL_OK;
    }
}

void HAL_su_uart_tx(uint8_t *buf, uint16_t size) {
    //HAL_UART_Transmit(&huart2, buf, size, 10);
    HAL_UART_Transmit_DMA(&huart2, buf, size);
}

SAT_returnState HAL_su_uart_rx() {

    UART_HandleTypeDef *huart;

    huart = &huart2;

    if(huart->RxState == HAL_UART_STATE_READY) {
        //HAL_UART_Receive_IT(huart, &su_scripts.rx_buf[SU_SCI_HEADER], UART_SU_SIZE);
      HAL_UART_Receive_IT(huart, &su_inc_buffer[22], 174);//&22,174
      return SATR_EOT;
    }
    return SATR_OK;
}