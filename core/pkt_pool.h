#ifndef __PKT_POOL_H
#define __PKT_POOL_H

#include <stdint.h>
#include "services.h"
#include "system.h"

#define PKT_TIMEOUT 60000 /*in mseconds*/
#define PKT_NORMAL  198   /*MAX_PKT_DATA*/

#if (SYSTEM_APP_ID == _EPS_APP_ID_)
#define POOL_PKT_SIZE       10
#define POOL_PKT_EXT_SIZE   0
#else
#define POOL_PKT_SIZE       25
#define POOL_PKT_EXT_SIZE   4
#endif


tc_tm_pkt * get_pkt(uint16_t size);

SAT_returnState free_pkt(tc_tm_pkt *pkt);

SAT_returnState pkt_pool_INIT();

void pkt_pool_IDLE();

#endif
