#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>

/**
 * @brief      { Include this init in main in order flash write to work}
 *
 * @return     { description_of_the_return_value }
 */
uint32_t flash_INIT();

uint32_t flash_read_trasmit();

void flash_write_trasmit(uint32_t data);

#endif
