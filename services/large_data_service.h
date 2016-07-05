#ifndef LARGE_DATA_SERVICE_H
#define LARGE_DATA_SERVICE_H

#include <stdint.h>
#include "services.h"




SAT_returnState large_data_app(tc_tm_pkt *pkt);


SAT_returnState large_data_firstRx_api(tc_tm_pkt *pkt);

SAT_returnState large_data_intRx_api(tc_tm_pkt *pkt);

SAT_returnState large_data_lastRx_api(tc_tm_pkt *pkt);


SAT_returnState large_data_downlinkTx_api(tc_tm_pkt *pkt);

SAT_returnState large_data_ackTx_api(tc_tm_pkt *pkt);

SAT_returnState large_data_retryTx_api(tc_tm_pkt *pkt);

SAT_returnState large_data_abort_api(tc_tm_pkt *pkt);


SAT_returnState large_data_updatePkt(tc_tm_pkt *pkt, uint16_t size, uint8_t subtype);

SAT_returnState large_data_downlinkPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t n, uint16_t dest_id);

SAT_returnState large_data_verifyPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t n, uint16_t dest_id);

SAT_returnState large_data_abortPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t dest_id, uint8_t subtype);

void large_data_IDLE();

void large_data_init();

SAT_returnState large_data_timeout();

#endif
