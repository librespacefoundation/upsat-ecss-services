#include "comms.h"
#include "large_data_service.h"
#include "service_utilities.h"
#include "config.h"
#include "log.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "pkt_pool.h"
#include "queue.h"
#include "stats.h"
#include "wod_handling.h"
#include "sysview.h"

#undef __FILE_ID__
#define __FILE_ID__ 25

extern SAT_returnState
verification_app (tc_tm_pkt *pkt);
extern SAT_returnState
hk_app (tc_tm_pkt *pkt);
extern SAT_returnState
function_management_app (tc_tm_pkt *pkt);
extern SAT_returnState
test_app (tc_tm_pkt *pkt);
extern int32_t
send_payload (const uint8_t *in, size_t len, uint8_t is_wod, size_t timeout_ms);
extern uint8_t dbg_msg;
extern UART_HandleTypeDef huart5;

static uint8_t send_buf[TC_MAX_PKT_SIZE];
struct _comms_data comms_data;
extern comms_rf_stat_t comms_stats;

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

SAT_returnState
route_pkt (tc_tm_pkt *pkt)
{

  SAT_returnState res;
  TC_TM_app_id id;

  if ( !C_ASSERT(pkt != NULL && pkt->data != NULL)) {
    free_pkt(pkt);
    return SATR_ERROR;
  }
  if (!C_ASSERT(pkt->type == TC || pkt->type == TM)) {
    free_pkt(pkt);
    return SATR_ERROR;
  }
  if (!C_ASSERT(pkt->app_id < LAST_APP_ID && pkt->dest_id < LAST_APP_ID)) {
    free_pkt(pkt);
    return SATR_ERROR;
  }

  if (pkt->type == TC) {
    id = pkt->app_id;
  }
  else if (pkt->type == TM) {
    id = pkt->dest_id;
  }
  else {
    return SATR_ERROR;
  }

  if (id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE
      && pkt->ser_subtype == TM_HK_PARAMETERS_REPORT
      && pkt->data[0] == WOD_REP) {
    /*
     * A new WOD arrived from the OBC. Store it and extract the information.
     * The transmission of each WOD is handled by the COMMS dispatcher function
     */
    SYSVIEW_PRINT("WOD from OBC");
    store_wod_obc(pkt->data + 1, pkt->len - 1);
  }
  else if (id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE
      && pkt->ser_subtype == TM_HK_PARAMETERS_REPORT
      && pkt->data[0] == EXT_WOD_REP) {
    /*
     * A new exWOD arrived from the OBC. Store it and extract the information.
     * The transmission of each exWOD is handled by the COMMS dispatcher function
     */
    SYSVIEW_PRINT("exWOD from OBC");
    store_ex_wod_obc(pkt->data, pkt->len);
  }
  else if (id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
    res = hk_app (pkt);
  }
  else if (id == SYSTEM_APP_ID
      && pkt->ser_type == TC_FUNCTION_MANAGEMENT_SERVICE) {
    res = function_management_app (pkt);
  }
  else if (id == SYSTEM_APP_ID && pkt->ser_type == TC_LARGE_DATA_SERVICE) {
    res = large_data_app (pkt);
    if (res == SATR_OK) {
      free_pkt (pkt);
      return SATR_OK;
    }
  }
  else if (id == SYSTEM_APP_ID && pkt->ser_type == TC_TEST_SERVICE) {
    //C_ASSERT(pkt->ser_subtype == 1 || pkt->ser_subtype == 2 || pkt->ser_subtype == 9 || pkt->ser_subtype == 11 || pkt->ser_subtype == 12 || pkt->ser_subtype == 13) { free_pkt(pkt); return SATR_ERROR; }
    res = test_app (pkt);
  }
  else if (id == EPS_APP_ID) {
    queuePush (pkt, OBC_APP_ID);
  }
  else if (id == ADCS_APP_ID) {
    queuePush (pkt, OBC_APP_ID);
  }
  else if (id == OBC_APP_ID) {
    queuePush (pkt, OBC_APP_ID);
  }
  else if (id == IAC_APP_ID) {
    queuePush (pkt, OBC_APP_ID);
  }
  else if (id == GND_APP_ID) {
    if (pkt->len > MAX_PKT_DATA) {
      large_data_downlinkTx_api (pkt);
    }
    else {
      tx_ecss (pkt);
    }
  }
  else if (id == DBG_APP_ID) {
    queuePush (pkt, OBC_APP_ID);
  } else {
    free_pkt(pkt);
  }

  return SATR_OK;
}

/**
 * This functions handles an incoming ECSS packet.
 * If the ECSS packet is part of a large data transfer consisting from
 * several sequential ECSS packets, it handles them automatically.
 *
 * In other words, there is no need to explicitly check for a fragmented data
 * transfer.
 *
 * @param payload the received payload
 * @param payload_size the size of the payload
 * @return SATR_OK if all went ok or appropriate error code
 */
SAT_returnState
rx_ecss (uint8_t *payload, const uint16_t payload_size)
{
  SAT_returnState ret;
  tc_tm_pkt *pkt;

  pkt = get_pkt (payload_size);

  if (!C_ASSERT(pkt != NULL)) {
    return SATR_ERROR;
  }
  if (unpack_pkt (payload, pkt, payload_size) == SATR_OK) {
    ret = route_pkt (pkt);
  }

  TC_TM_app_id dest = 0;

  if(pkt->type == TC) {
    dest = pkt->app_id;
  }
  else if(pkt->type == TM) {
    dest = pkt->dest_id;
  }

  if(dest == SYSTEM_APP_ID) {
      free_pkt(pkt);
  }

  return ret;
}


SAT_returnState tx_ecss(tc_tm_pkt *pkt) {
    int32_t ret = 0;
    uint16_t size = 0;
    SAT_returnState res;

    if(pkt == NULL){
      comms_rf_stats_frame_transmitted(&comms_stats, 0, SATR_ERROR);
      return SATR_ERROR;
    }

    res = pack_pkt(send_buf, pkt, &size);
    if(res != SATR_OK){
      comms_rf_stats_frame_transmitted(&comms_stats, 0, res);
      return ret;
    }

    ret = send_payload(send_buf, (size_t)size, 0,COMMS_DEFAULT_TIMEOUT_MS);
    if(ret < 1){
      return SATR_ERROR;
    }
    free_pkt (pkt);
    return SATR_OK;
}

SAT_returnState event_log(uint8_t *buf, const uint16_t size) {
    return SATR_OK;
}
SAT_returnState check_timeouts() {
    return SATR_OK;
}
