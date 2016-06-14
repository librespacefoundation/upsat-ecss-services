#ifndef LARGE_DATA_SERVICE_H
#define LARGE_DATA_SERVICE_H

#include <stdint.h>
#include "services.h"

typedef enum {
    LD_STATE_FREE           = 1,
    LD_STATE_RECEIVING      = 2,
    LD_STATE_TRANSMITING    = 3,
    LD_STATE_REPORT         = 4,
    LD_STATE_DOWNLINK       = 5,
    LAST_STATE              = 6
}LD_states;

/**
 * Status of the large data transfer
 */
struct _ld_status {
    LD_states state;        		/**< Service state machine, state variable*/
    TC_TM_app_id app_id;    		/**< Destination app id */
    uint8_t ld_num;         		/**< Sequence number of last fragmented packet stored */
    uint32_t timeout;       /**/
    uint8_t started;        /**/

    uint8_t buf[MAX_PKT_EXT_DATA]; 	/**< Buffer that holds the sequential fragmented packets */
    uint16_t rx_size;         		/**< The number of bytes stored already in the buffer */
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

SAT_returnState large_data_timeout();

#endif
