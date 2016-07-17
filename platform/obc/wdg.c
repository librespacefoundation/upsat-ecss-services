#include "wdg.h"

#include <stdbool.h>
#include "obc_hal.h"

#undef __FILE_ID__
#define __FILE_ID__ 12

static struct _wdg_state wdg;

void wdg_INIT() {
    HAL_obc_IWDG_Start();
}

void wdg_reset_HK() {
    wdg.hk_valid = true;
}

void wdg_reset_UART() {
    wdg.hk_valid = true;
}

void wdg_IDLE() {

  if(wdg.hk_valid == true && wdg.uart_valid == true ) {
      HAL_obc_IWDG_Refresh();
      wdg.hk_valid = false;
      wdg.uart_valid = false; 
  }
}
