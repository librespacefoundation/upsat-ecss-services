#ifndef __WDG_H
#define __WDG_H

#include <stdint.h>

struct _wdg_state {
    uint8_t hk_valid;
    uint8_t uart_valid; 
};

void wdg_reset_INIT();

void wdg_reset_HK();

void wdg_reset_UART();

void wdg_IDLE();

#endif