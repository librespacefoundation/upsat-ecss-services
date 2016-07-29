#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief      { Include this init in main in order flash write to work}
 *
 * @return     { description_of_the_return_value }
 */
uint32_t flash_INIT();

uint32_t flash_read_trasmit(size_t offset);

void flash_write_trasmit(uint32_t data, size_t offset);

#endif
