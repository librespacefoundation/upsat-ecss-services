#ifndef __ADCS_H
#define __ADCS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "services.h"

#include "upsat.h"

struct _adcs_data {
	uint16_t adcs_seq_cnt;
	uint8_t rsrc;
	uint32_t *boot_counter;
	uint32_t *log;
	uint32_t *log_cnt;
	uint32_t *log_state;

	struct uart_data obc_uart;
};

struct _sys_data {
	uint8_t rsrc;
	uint32_t *boot_counter;
};

extern struct _adcs_data adcs_data;

extern const uint8_t services_verification_ADCS_TC[MAX_SERVICES][MAX_SUBTYPES];

extern uint32_t * HAL_obc_BKPSRAM_BASE();

extern SAT_returnState free_pkt(tc_tm_pkt *pkt);

extern SAT_returnState verification_app(tc_tm_pkt *pkt);
extern SAT_returnState hk_app(tc_tm_pkt *pkt);
extern SAT_returnState function_management_app(tc_tm_pkt *pkt);
extern SAT_returnState mass_storage_app(tc_tm_pkt *pkt);
extern SAT_returnState mass_storage_storeLogs(MS_sid sid, uint8_t *buf,
		uint16_t *size);
extern SAT_returnState large_data_app(tc_tm_pkt *pkt);
extern SAT_returnState test_app(tc_tm_pkt *pkt);

SAT_returnState route_pkt(tc_tm_pkt *pkt);

SAT_returnState event_log(uint8_t *buf, const uint16_t size);

SAT_returnState check_timeouts();

void HAL_adcs_SENSORS_ON();
void HAL_adcs_SENSORS_OFF();
void HAL_adcs_SPIN(int32_t RPM);
void HAL_adcs_MAGNETO(int32_t current_x, int32_t current_y);
void HAL_adcs_DBG(uint8_t var, uint8_t val);

#endif
