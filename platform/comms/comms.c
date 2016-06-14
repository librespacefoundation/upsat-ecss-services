#include "comms.h"
#include "large_data_service.h"
#include "config.h"
#include "log.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "pkt_pool.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

extern SAT_returnState export_pkt(TC_TM_app_id app_id, tc_tm_pkt *pkt, struct uart_data *data);

extern SAT_returnState free_pkt(tc_tm_pkt *pkt);

extern SAT_returnState verification_app(tc_tm_pkt *pkt);
extern SAT_returnState hk_app(tc_tm_pkt *pkt);
extern SAT_returnState function_management_app(tc_tm_pkt *pkt);
extern SAT_returnState test_app(tc_tm_pkt *pkt);

extern uint8_t dbg_msg;

const uint8_t services_verification_COMMS_TC[MAX_SERVICES][MAX_SUBTYPES] = { 
/*    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_VERIFICATION_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0 }, /*TC_HOUSEKEEPING_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_FUNCTION_MANAGEMENT_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_SCHEDULING_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 }, /*TC_LARGE_DATA_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_MASS_STORAGE_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_TEST_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

struct _comms_data comms_data;

SAT_returnState route_pkt(tc_tm_pkt *pkt) {

    SAT_returnState res;
    TC_TM_app_id id;

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)                         { verification_app(pkt); free_pkt(pkt); return SATR_ERROR; }
    if(!C_ASSERT(pkt->type == TC || pkt->type == TM) == true)                       { verification_app(pkt); free_pkt(pkt); return SATR_ERROR; }
    if(!C_ASSERT(pkt->app_id < LAST_APP_ID && pkt->dest_id < LAST_APP_ID) == true)  { verification_app(pkt); free_pkt(pkt); return SATR_ERROR; }

    if(pkt->type == TC)         { id = pkt->app_id; } 
    else if(pkt->type == TM)    { id = pkt->dest_id; }
    else{
      return SATR_ERROR;
    }

    if(id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
        res = hk_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_FUNCTION_MANAGEMENT_SERVICE) {
        res = function_management_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_LARGE_DATA_SERVICE) {
        //res = large_data_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_TEST_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 1 || pkt->ser_subtype == 2 || pkt->ser_subtype == 9 || pkt->ser_subtype == 11 || pkt->ser_subtype == 12 || pkt->ser_subtype == 13) { free_pkt(pkt); return SATR_ERROR; }
        res = test_app(pkt);
    } 
    else if(id == EPS_APP_ID)      { queuePush(pkt, OBC_APP_ID); }
    else if(id == ADCS_APP_ID)     { queuePush(pkt, OBC_APP_ID); }
    else if(id == OBC_APP_ID)      { queuePush(pkt, OBC_APP_ID); }
    else if(id == GND_APP_ID)      {

      if(pkt->len > MAX_PKT_DATA) { large_data_downlinkTx_api(pkt); }
      else { tx_ecss(pkt); }
    }
    else if(id == DBG_APP_ID)      { queuePush(pkt, OBC_APP_ID); }

    return SATR_OK;
}

extern UART_HandleTypeDef huart5;
static uint8_t payload[TC_MAX_PKT_SIZE];

void rx_ecss(uint8_t *payload, const uint16_t payload_size) {

    tc_tm_pkt *pkt;
    uint16_t size = 0;

    SAT_returnState res;
    SAT_returnState res_deframe;

    pkt = get_pkt(payload_size);

    if(!C_ASSERT(pkt != NULL) == true) { return; }
    if(unpack_pkt(payload, pkt, payload_size) == SATR_OK) { route_pkt(pkt); } 
    else { verification_app(pkt); free_pkt(pkt); }

}

SAT_returnState tx_ecss(tc_tm_pkt *pkt) {

    int ret = 0;
    
    uint16_t size = 0;
    SAT_returnState res;    

    pack_pkt(payload, pkt, &size);

    //if(!C_ASSERT(size > 0) == true) { return SATR_ERROR; }

    ret = send_payload(payload, (size_t)size, COMMS_DEFAULT_TIMEOUT_MS);
    if (ret > 0) {
      HAL_Delay (50);
      LOG_UART_DBG(&huart5, "Frame transmitted ECSS Ret %d", ret);
    }
    else {
      LOG_UART_DBG(&huart5, "Error at AX.25 encoding");
    }
}

SAT_returnState event_log(uint8_t *buf, const uint16_t size) {
    return SATR_OK;
}
SAT_returnState check_timeouts() {
    
}

void
HAL_comms_DBG (uint8_t var,uint8_t val)
{
  dbg_msg = val;
}
