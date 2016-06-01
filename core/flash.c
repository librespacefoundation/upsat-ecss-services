#include "flash.h"
#include "stm32f4xx_hal.h"

uint32_t flash_read_trasmit() {

    uint32_t *val = (uint32_t*)TRANSMIT_VAR_ADD;

    return *val;
}

void flash_write_trasmit(uint32_t data) {

    HAL_FLASH_Unlock();

    //HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_Erase_Sector(FLASH_SECTOR_3, VOLTAGE_RANGE_3);

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, TRANSMIT_VAR_ADD, data);

    HAL_FLASH_Lock();
}