#ifndef __ECSS_STATS_H
#define __ECSS_STATS_H

#include "services.h"

void stats_inbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt);

void stats_outbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt);

void stats_dropped_hldlc();

void stats_dropped_upack();

uint16_t ecss_stats_hk(uint8_t *buffer);

#endif
