#include "wdg.h"

#include <stdbool.h>
#include "obc_hal.h"

#undef __FILE_ID__
#define __FILE_ID__ 12

struct _wdg_state {
    uint8_t hk_valid;
    uint8_t uart_valid;
    uint8_t sch_valid;  
};

static struct _wdg_state wdg = { .hk_valid = false,
								 .uart_valid = false,
								 .sch_valid = false };

void wdg_reset_HK() {
    wdg.hk_valid = true;
}

void wdg_reset_UART() {
    wdg.uart_valid = true;
}

void wdg_reset_SCH() {
    wdg.sch_valid = true;
}


void wdg_IDLE() {

  if(wdg.hk_valid == true &&
     wdg.uart_valid == true && 
  	 wdg.sch_valid == true ) {

      HAL_obc_IWDG_Refresh();
      wdg.hk_valid = false;
      wdg.uart_valid = false; 
      wdg.sch_valid = false;
  }
}
