#ifndef __EVENT_REPORTING_SERVICE_H
#define __EVENT_REPORTING_SERVICE_H

#include <stdint.h>
#include "services.h"

#define EV_DATA_SIZE 12

SAT_returnState event_boot(const uint8_t reset_source, const uint8_t boot_counter);

SAT_returnState event_dbg_api(uint8_t *buf, uint8_t *str, uint16_t *size);

SAT_returnState event_crt_pkt_api(uint8_t *buf, uint8_t *f, uint16_t fi, uint32_t l, uint8_t *e, uint16_t *size, SAT_returnState mode);

SAT_returnState event_crt_pkt(tc_tm_pkt **pkt, EV_event event);

#endif
