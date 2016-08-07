#include "flash.h"
#include "stm32f4xx_hal.h"

#undef __FILE_ID__
#define __FILE_ID__ 38

#define SECTOR_3_SIZE	   0x1000

#define TRANSMIT_VAR_ADD   0x0800C000

#define COMMS_FLASH_CONF_WORDS 2

#ifdef __GNUC__
const uint32_t __attribute__((section (".comms_storage_section"))) occupy_sector[SECTOR_3_SIZE] __attribute__ ((aligned (4)))
    =
      { 0x16264e84, 0x48};
#else
#pragma location = 0x0800C000
const uint32_t occupy_sector[SECTOR_3_SIZE] = { 0x16264e84, 0x48 };
#endif

static uint8_t temp_mem[COMMS_FLASH_CONF_WORDS * sizeof(uint32_t)] = {0};

uint32_t flash_INIT() {
    return occupy_sector[0];
}

uint32_t flash_read_trasmit(size_t offset) {

    uint32_t *val = (uint32_t*)(TRANSMIT_VAR_ADD + offset);

    return *val;
}

void flash_write_trasmit(uint32_t data, size_t offset) {
    size_t i;
    uint32_t word;
    /* Store first the contents of the memory */
    memcpy(temp_mem, (void *)TRANSMIT_VAR_ADD,
	   COMMS_FLASH_CONF_WORDS * sizeof(uint32_t));
    /* Write the new value */
    memcpy(temp_mem + offset, (void *)&data, sizeof(uint32_t));

    HAL_FLASH_Unlock();

    //HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_Erase_Sector(FLASH_SECTOR_3, VOLTAGE_RANGE_3);

    for(i = 0; i < COMMS_FLASH_CONF_WORDS; i++){
      memcpy((void *)(&word), temp_mem + i * sizeof(uint32_t), sizeof(uint32_t));
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
			TRANSMIT_VAR_ADD + i * sizeof(uint32_t),
			word);
    }

    HAL_FLASH_Lock();
}
