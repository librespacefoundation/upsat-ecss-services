#include "flash.h"
#include "stm32f4xx_hal.h"

#undef __FILE_ID__
#define __FILE_ID__ 38

#define SECTOR_3_SIZE	   0x1000

#define TRANSMIT_VAR_ADD   0x0800C000

#ifdef __GNUC__
const uint32_t __attribute__((section (".comms_storage_section"))) occupy_sector[SECTOR_3_SIZE] __attribute__ ((aligned (4)))
    =
      { 0x16264e84, 0xa2 };
#else
#pragma location = 0x0800C000
const uint32_t occupy_sector[SECTOR_3_SIZE] = { 0x16264e84, 0xa2 };
#endif

uint32_t flash_INIT() {
    return occupy_sector[0];
}

uint32_t flash_read_trasmit(size_t offset) {

    uint32_t *val = (uint32_t*)(TRANSMIT_VAR_ADD + offset);

    return *val;
}

void flash_write_trasmit(uint32_t data, size_t offset) {

    HAL_FLASH_Unlock();

    //HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_Erase_Sector(FLASH_SECTOR_3, VOLTAGE_RANGE_3);

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, TRANSMIT_VAR_ADD + offset, data);

    HAL_FLASH_Lock();
}
