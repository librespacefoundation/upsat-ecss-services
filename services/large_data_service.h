#ifndef LARGE_DATA_SERVICE_H
#define LARGE_DATA_SERVICE_H

#include <stdint.h>
#include "services.h"

#define LD_PKT_DATA             195 /*MAX_PKT_DATA - LD_PKT_DATA_HDR_SIZE*/
#define LD_PKT_DATA_HDR_SIZE    3

#define LD_TIMEOUT              8000 /*sec*/

#define LD_MAX_TRANSFER_TIME    60000 /*60 seconds */

typedef enum {
    LD_STATE_FREE           = 1,
    LD_STATE_RECEIVING      = 2,
    LD_STATE_TRANSMITING    = 3,
    LD_STATE_REPORT         = 4,
    LD_STATE_DOWNLINK       = 5,
    LD_STATE_RECV_OK        = 6,
    LAST_STATE              = 7
}LD_states;

struct _ld_status {
    LD_states state;        /*service state machine, state variable*/
    TC_TM_app_id app_id;    /*destination app id*/
    uint8_t ld_num;         /**/
    uint32_t timeout;       /**/
    uint8_t started;        /**/

    uint8_t buf[MAX_PKT_DATA];         /**/
    uint16_t rx_size;         /**/
    uint8_t rx_lid;         /**/
    uint8_t tx_lid;         /**/
    uint8_t tx_pkt;         /**/
    uint16_t tx_size;         /**/
};

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
