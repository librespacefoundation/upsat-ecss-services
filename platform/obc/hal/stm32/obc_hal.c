#include "obc_hal.h"

#include "stm32f4xx_hal.h"
#include <cmsis_os.h>
#include "services.h"
#include "task.h"
#include "obc.h"

extern RTC_HandleTypeDef hrtc;
extern IWDG_HandleTypeDef hiwdg;

extern uint8_t su_inc_buffer[197];

extern struct _obc_data obc_data;

extern struct _wdg_state wdg;

#undef __FILE_ID__
#define __FILE_ID__ 13

void HAL_sys_delay(uint32_t msec){
    osDelay(msec);
}

void HAL_sys_delay_until(uint32_t *PreviousWakeTime, uint32_t millisec){
    osDelayUntil(PreviousWakeTime, millisec);
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

void HAL_sys_setDate(uint8_t mon, uint8_t date, uint8_t year) {

  RTC_DateTypeDef sDate;

  //sDate.WeekDay = RTC_WEEKDAY_FRIDAY;
  sDate.Month = mon;
  sDate.Date = date;
  sDate.Year = year;

  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

}

void HAL_sys_getDate(uint8_t *mon, uint8_t *date, uint8_t *year) {

  RTC_DateTypeDef sDate;

  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  *mon = sDate.Month;
  *date = sDate.Date;
  *year = sDate.Year;

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

void HAL_obc_IWDG_Start() {
  HAL_IWDG_Start(&hiwdg);;
}

void HAL_obc_IWDG_Refresh() {
  HAL_IWDG_Refresh(&hiwdg);
}