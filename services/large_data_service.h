#ifndef LARGE_DATA_SERVICE_H
#define LARGE_DATA_SERVICE_H

#include <stdint.h>
#include <math.h>
#include "services.h"
#include "service_utilities.h"
#include "pkt_pool.h"

#define LD_PKT_DATA             198 /*MAX_PKT_DATA*/
#define LD_PKT_DATA_HDR_SIZE    3

#define LD_TIMEOUT              1000 /*sec*/

#define LD_MAX_TRANSFER_TIME    1000 //random

typedef enum {
    LD_STATE_FREE           = 1,
    LD_STATE_RECEIVING      = 2,
    LD_STATE_TRANSMITING    = 3,
    LD_STATE_REPORT         = 4,
    LD_STATE_DOWNLINK       = 5,
    LAST_STATE              = 6
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

extern SAT_returnState route_pkt(tc_tm_pkt *pkt);

//ToDo
//  check again if app_id and dest_id are ok.
//  assert, require.
//  finish definitions, types, subtypes, documentation and doc.
//  check again if finished segmentation

//Finito
//  check types definitions
//  add function definition in .h
//  check size and packet len. reconfigure for pkt len instead of pack.
//  check definition of n.
//  add pack in pack
//  what happens when new packet arrives, when the state is not free.
//  when to change iterator to next.
//  first tx packet, what header sould be.
//  check downlink, uplink subtypes.
//  what happens in timeout.
//  implement timeout and abort.
//  finish timeout.
//  check sequence numbers.
//  in tx when to make it FREE, maybe should ack every packet and then send it free.

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

void large_data_INIT();

void large_data_IDLE();

SAT_returnState large_data_timeout();

#endif
