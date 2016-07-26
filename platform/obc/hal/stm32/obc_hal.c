#include "obc_hal.h"

#include "stm32f4xx_hal.h"
#include <cmsis_os.h>
#include "task.h"
#include "obc.h"
#include "sysview.h"

extern RTC_HandleTypeDef hrtc;
extern IWDG_HandleTypeDef hiwdg;

extern struct _obc_data obc_data;

extern struct _wdg_state wdg;

extern osMessageQId queueCOMMS;
extern osMessageQId queueADCS;
extern osMessageQId queueDBG;
extern osMessageQId queueEPS;

extern TaskHandle_t xTask_UART;

#undef __FILE_ID__
#define __FILE_ID__ 18

void wake_uart_task() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(xTask_UART, &xHigherPriorityTaskWoken);
}


SAT_returnState queuePush(tc_tm_pkt *pkt, TC_TM_app_id app_id) {

    osMessageQId *xQueue;

    if(app_id == EPS_APP_ID) { xQueue = &queueEPS; }
    else if(app_id == DBG_APP_ID) { xQueue = &queueDBG; }
    else if(app_id == COMMS_APP_ID) { xQueue = &queueCOMMS; }
    else if(app_id == ADCS_APP_ID) { xQueue = &queueADCS; }

    xQueueSend(*xQueue, (void *) &pkt, (TickType_t) 10);

    return SATR_OK;
}

tc_tm_pkt * queuePop(TC_TM_app_id app_id) {

    tc_tm_pkt *pkt;
    osMessageQId *xQueue = 0;

    if(app_id == EPS_APP_ID) { xQueue = &queueEPS; }
    else if(app_id == DBG_APP_ID) { xQueue = &queueDBG; }
    else if(app_id == COMMS_APP_ID) { xQueue = &queueCOMMS; }
    else if(app_id == ADCS_APP_ID) { xQueue = &queueADCS; }

    if(uxQueueMessagesWaiting(*xQueue) == 0) { return NULL; }
    if(xQueueReceive(*xQueue, &(pkt), (TickType_t) 0) == pdFALSE) { return NULL; }

    return pkt;
}

uint8_t queueSize(TC_TM_app_id app_id) {

    return 0;
}

tc_tm_pkt * queuePeak(TC_TM_app_id app_id) {

    tc_tm_pkt *pkt;
    osMessageQId *xQueue = 0;

    if(app_id == EPS_APP_ID) { xQueue = &queueEPS; }
    else if(app_id == DBG_APP_ID) { xQueue = &queueDBG; }
    else if(app_id == COMMS_APP_ID) { xQueue = &queueCOMMS; }
    else if(app_id == ADCS_APP_ID) { xQueue = &queueADCS; }

    if(xQueuePeek(*xQueue, &(pkt), (TickType_t) 0) == pdFALSE) { return (tc_tm_pkt *)NULL; }

    return pkt;
}

void queue_IDLE(TC_TM_app_id app_id) {

    tc_tm_pkt *pkt;

    pkt = queuePeak(app_id);
    if(!C_ASSERT(pkt != NULL) == true) { return; }

    if(is_free_pkt(pkt) == true) {
        queuePop(app_id);
        traceGC_QUEUE_PKT();
    }

}

void HAL_sys_delay(uint32_t msec) {
	osDelay(msec);
}

void HAL_obc_SD_ON() {

    for(uint16_t i = 0; i < 1000; i++){
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
        osDelay(1);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
        osDelay(1);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}

void HAL_obc_SD_OFF() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
    osDelay(200);
}

void HAL_obc_IAC_ON() {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); /*DART*/
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); /*CAM*/
}

void HAL_obc_IAC_OFF() {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); /*DART*/
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET); /*CAM*/
}

void HAL_reset_source(uint8_t *src) {

    *src = __HAL_RCC_GET_FLAG(RCC_FLAG_BORRST);
    *src |= (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) << 1);
    *src |= (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) << 2);
    *src |= (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) << 3);
    *src |= (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) << 4);
    *src |= (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) << 5);
    *src |= (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) << 6);
 
    __HAL_RCC_CLEAR_RESET_FLAGS();

}

void HAL_sys_setTime(uint8_t hours, uint8_t mins, uint8_t sec) {

  RTC_TimeTypeDef sTime;

  sTime.Hours = hours;
  sTime.Minutes = mins;
  sTime.Seconds = sec;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  
}

void HAL_sys_getTime(uint8_t *hours, uint8_t *mins, uint8_t *sec) {

  RTC_TimeTypeDef sTime;

  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

   *hours = sTime.Hours;
   *mins = sTime.Minutes;
   *sec = sTime.Seconds;  
}

void HAL_sys_setDate(uint8_t weekday, uint8_t mon, uint8_t date, uint8_t year) {

  RTC_DateTypeDef sDate;
//  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.WeekDay = weekday;
  sDate.Month = mon;
  sDate.Date = date;
  sDate.Year = year;

  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

}

void HAL_sys_getDate(uint8_t *weekday, uint8_t *mon, uint8_t *date, uint8_t *year) {

  RTC_DateTypeDef sDate;

  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  *mon = sDate.Month;
  *date = sDate.Date;
  *year = sDate.Year;
  *weekday = sDate.WeekDay;

}

uint32_t HAL_sys_GetTick() {
  return HAL_GetTick();
}

TickType_t HAL_xTaskGetTickCount(){
    return xTaskGetTickCount();
}

void HAL_obc_enableBkUpAccess() {
  HAL_PWR_EnableBkUpAccess();
  HAL_PWREx_EnableBkUpReg();
  __HAL_RCC_BKPSRAM_CLK_ENABLE();
  
}

uint32_t * HAL_obc_BKPSRAM_BASE() {
  return (uint32_t *)BKPSRAM_BASE;
}

void HAL_obc_IWDG_Refresh() {
  HAL_IWDG_Refresh(&hiwdg);
}