#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>

#define SECTOR_3_ADD_START 0x0800C000
#define SECTOR_3_ADD_FIN   0x0800FFFF
#define SECTOR_3_SIZE	   0xFFF

#define TRANSMIT_VAR_ADD   0x0800C000

#pragma location = SECTOR_3_ADD_START;
const uint32_t occupy_sector[SECTOR_3_SIZE] = { 0x0800FFFF };

uint32_t flash_read_trasmit();

void flash_write_trasmit(uint32_t data);

#endif