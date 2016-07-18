#ifndef __EVENT_REPORTING_SERVICE_H
#define __EVENT_REPORTING_SERVICE_H

#include <stdint.h>
#include "services.h"


SAT_returnState event_app(tc_tm_pkt * pkt);

SAT_returnState event_boot(const uint8_t reset_source, const uint32_t boot_counter);

SAT_returnState event_eps_voltage_state(const uint8_t battery_status);

SAT_returnState event_pkt_pool_timeout();

SAT_returnState event_dbg_api(uint8_t *buf, uint8_t *str, uint16_t *size);

SAT_returnState event_crt_pkt_api(uint8_t *buf, uint8_t *f, uint16_t fi, uint32_t l, uint8_t *e, uint16_t *size, SAT_returnState mode);

SAT_returnState event_crt_pkt(tc_tm_pkt **pkt, EV_event event);

#endif
