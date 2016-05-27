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

struct _pkt_pool{
    tc_tm_pkt pkt[POOL_PKT_SIZE];
    uint8_t free[POOL_PKT_SIZE];
    uint32_t time[POOL_PKT_SIZE];
    uint8_t data[POOL_PKT_SIZE][MAX_PKT_DATA];
    uint32_t time_delta[POOL_PKT_SIZE];

    tc_tm_pkt pkt_ext[POOL_PKT_EXT_SIZE];
    uint8_t free_ext[POOL_PKT_EXT_SIZE];
    uint32_t time_ext[POOL_PKT_EXT_SIZE];
    uint8_t data_ext[POOL_PKT_EXT_SIZE][MAX_PKT_EXT_DATA];
    uint32_t time_delta_ext[POOL_PKT_EXT_SIZE];

};

tc_tm_pkt * get_pkt(uint16_t size);

SAT_returnState free_pkt(tc_tm_pkt *pkt);

SAT_returnState pkt_pool_INIT();

void pkt_pool_IDLE();

#endif
