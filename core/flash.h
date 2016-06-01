#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>

#define SECTOR_3_ADD_START 
#define SECTOR_3_ADD_FIN   0x0800FFFF
#define SECTOR_3_SIZE	   0xFFF

#define TRANSMIT_VAR_ADD   0x0800C000

/**
 * @brief      { Include this init in main in order flash write to work}
 *
 * @return     { description_of_the_return_value }
 */
uint32_t flash_INIT();

uint32_t flash_read_trasmit();

void flash_write_trasmit(uint32_t data);

#endif