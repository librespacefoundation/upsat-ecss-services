#ifndef __EVENT_REPORTING_SERVICE_H
#define __EVENT_REPORTING_SERVICE_H

#include <stdint.h>
#include "services.h"
#include "system.h"

#define EV_DATA_SIZE 12

extern tc_tm_pkt * get_pkt();
extern SAT_returnState crt_pkt(tc_tm_pkt *pkt, TC_TM_app_id app_id, uint8_t type, uint8_t ack, uint8_t ser_type, uint8_t ser_subtype, TC_TM_app_id dest_id);


extern SAT_returnState checkSum(const uint8_t *data, const uint16_t size, uint8_t *res_crc);

//ToDo
SAT_returnState event_boot(uint8_t reset_source);

SAT_returnState event_crt_pkt_api(uint8_t *buf, uint8_t *f, uint16_t fi, uint32_t l, uint8_t *e, uint16_t *size, SAT_returnState mode);

SAT_returnState event_crt_pkt(tc_tm_pkt **pkt, EV_event event);

#endif
