#ifndef __QUEUE_H
#define __QUEUE_H

#include <stdint.h>
#include "services.h"

SAT_returnState queuePush(tc_tm_pkt *pkt, TC_TM_app_id app_id);

tc_tm_pkt * queuePop(TC_TM_app_id app_id);

uint8_t queueSize(TC_TM_app_id app_id);

tc_tm_pkt * queuePeak(TC_TM_app_id app_id);

#endif
