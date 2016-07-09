#include "large_data_service.h"

#include "service_utilities.h"
#include "pkt_pool.h"
#include "verification_service.h"
#include "stm32f4xx_hal.h"
#include <math.h>

#undef __FILE_ID__
#define __FILE_ID__ 7

extern SAT_returnState route_pkt(tc_tm_pkt *pkt);

struct _ld_status LD_status = { .state = LD_STATE_FREE,
                                .ld_num = 0,
                                .timeout = 0,
                                .started = 0 } ;


SAT_returnState large_data_app(tc_tm_pkt *pkt) {

    if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_FIRST_UPLINK)              { large_data_firstRx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_INT_UPLINK)           { large_data_intRx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_LAST_UPLINK)          { large_data_lastRx_api(pkt); }
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_ABORT_SE_UPLINK)      { large_data_abort_api(pkt); } 

    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_ACK_DOWNLINK)         { large_data_ackTx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_REPEAT_DOWNLINK)      { large_data_retryTx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_ABORT_RE_DOWNLINK)    { large_data_abort_api(pkt); }
    else {
      return SATR_ERROR;
    }

    return SATR_OK;
}

/*downlink*/
SAT_returnState large_data_firstRx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    TC_TM_app_id app_id;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    lid = pkt->data[0]; 

    if(C_ASSERT(pkt == NULL || pkt->data == NULL)) {
      return SATR_ERROR;
    }

    /* Check if there is a previous, successfully ended large data session */
    if(LD_status.state == LD_STATE_RECV_OK){
      large_data_init();
    }

    /*
     * Convert properly the sequence number.
     * NOTE: We assume that the transmitter complies with the network byte order
     */
    memcpy(&ld_num, &pkt->data[1], sizeof(uint16_t));
    ld_num = ntohs(ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;
    size = pkt->len; //ldata headers

    if(C_ASSERT(app_id != DBG_APP_ID && app_id != GND_APP_ID)) {
      return SATR_ERROR;
    }

    if(C_ASSERT(size > LD_PKT_DATA)){
      return SATR_ERROR;
    }

    /*
     * The ACK from a previous first large data frame may never reached the
     * destination....
     */
    if(LD_status.state == LD_STATE_RECEIVING && LD_status.ld_num == ld_num
	&& LD_status.rx_lid == lid) {
        /*Re-send the ACK */
        large_data_verifyPkt(&temp_pkt, LD_status.rx_lid,
			     LD_status.ld_num, app_id);
        route_pkt(temp_pkt);
        return SATR_OK;
    }
    else if(C_ASSERT(LD_status.state != LD_STATE_FREE)) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid,
			    TM_LD_ABORT_RE_UPLINK);

        if(C_ASSERT(temp_pkt == NULL)) {
          return SATR_ERROR;
        }

        route_pkt(temp_pkt);
        return SATR_OK;
    }

    size -= LD_PKT_DATA_HDR_SIZE;
    LD_status.ld_num = ld_num;
    LD_status.rx_size = size;
    LD_status.ld_num = ld_num;
    LD_status.rx_lid = lid;
    LD_status.state = LD_STATE_RECEIVING;
    LD_status.started = HAL_GetTick();

    for(uint16_t i = 0; i < size; i++) { 
        LD_status.buf[i] = pkt->data[i + LD_PKT_DATA_HDR_SIZE]; 
    }

    LD_status.timeout = HAL_GetTick();

    large_data_verifyPkt(&temp_pkt, LD_status.rx_lid, LD_status.ld_num, app_id);
    route_pkt(temp_pkt);

    return SATR_OK;
}

SAT_returnState large_data_intRx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    TC_TM_app_id app_id;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    if(C_ASSERT(pkt == NULL || pkt->data == NULL)){
      return SATR_ERROR;
    }

    lid = pkt->data[0];

    if(C_ASSERT(LD_status.state != LD_STATE_RECEIVING)){
      return SATR_ERROR;
    }

    if(C_ASSERT(LD_status.rx_lid != lid)) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TM_LD_ABORT_RE_UPLINK); 
        if(C_ASSERT(temp_pkt == NULL)) {
          return SATR_ERROR;
        }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    /*
     * Convert properly the sequence number.
     * NOTE: We assume that the transmitter complies with the network byte order
     */
    memcpy(&ld_num, &pkt->data[1], sizeof(uint16_t));
    ld_num = ntohs(ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;
    size = pkt->len; //ldata headers

    if(C_ASSERT(app_id != DBG_APP_ID && app_id != GND_APP_ID)) {
      return SATR_ERROR;
    }

    if (C_ASSERT(LD_status.ld_num + 1 < ld_num)) {
      return SATR_ERROR;
    }

    if(C_ASSERT(size > LD_PKT_DATA)) {
      return SATR_ERROR;
    }
    //if(!C_ASSERT((app_id == IAC_APP_ID && sid == FOTOS) || (app_id == GND_APP_ID && sid <= SU_SCRIPT_7 )) == true) { return SATR_ERROR; } 
    size -= LD_PKT_DATA_HDR_SIZE;

    LD_status.ld_num = ld_num;
    LD_status.rx_size += size;

    for(uint16_t i = 0; i < size; i++) { 
        LD_status.buf[(LD_status.ld_num * LD_PKT_DATA) + i] = pkt->data[i + LD_PKT_DATA_HDR_SIZE]; 
    }

    LD_status.timeout = HAL_GetTick();

    large_data_verifyPkt(&temp_pkt, LD_status.rx_lid, LD_status.ld_num, app_id);
    route_pkt(temp_pkt);

    return SATR_OK;
}

SAT_returnState large_data_lastRx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    TC_TM_app_id app_id;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    if(C_ASSERT(pkt == NULL || pkt->data == NULL)){
      return SATR_ERROR;
    }

    if(C_ASSERT(LD_status.state != LD_STATE_RECEIVING
		 && LD_status.state != LD_STATE_RECV_OK)) {
      return SATR_ERROR;
    }

    lid = pkt->data[0];

    if(C_ASSERT(LD_status.rx_lid != lid)) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TM_LD_ABORT_RE_UPLINK); 
        if(C_ASSERT(temp_pkt == NULL)) {
          return SATR_ERROR;
        }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    /*
     * Convert properly the sequence number.
     * NOTE: We assume that the transmitter complies with the network byte order
     */
    memcpy(&ld_num, &pkt->data[1], sizeof(uint16_t));
    ld_num = ntohs(ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;
    size = pkt->len; //ldata headers

    if(C_ASSERT(app_id != DBG_APP_ID && app_id != GND_APP_ID)) {
      return SATR_ERROR;
    }

    if(C_ASSERT(LD_status.ld_num + 1 < ld_num)) {
      return SATR_ERROR;
    }

    if(C_ASSERT(size <= LD_PKT_DATA_HDR_SIZE)) {
      return SATR_ERROR;
    }

    size -= LD_PKT_DATA_HDR_SIZE;

    /*
     * If the last frame has been successfully received just send back an ACK.
     * The data have been already sent to the proper subsystem
     */
    if(LD_status.state == LD_STATE_RECV_OK) {
      large_data_verifyPkt(&temp_pkt, LD_status.rx_lid, LD_status.ld_num, app_id);
      route_pkt(temp_pkt);
      return SATR_OK;
    }

    LD_status.ld_num = ld_num;
    LD_status.rx_size += size;

    for(uint16_t i = 0; i < size; i++) { 
        LD_status.buf[(LD_status.ld_num * LD_PKT_DATA) + i] = pkt->data[i + LD_PKT_DATA_HDR_SIZE]; 
    }

    large_data_verifyPkt(&temp_pkt, LD_status.rx_lid, LD_status.ld_num, app_id);
    route_pkt(temp_pkt);

    /*
     * The last ACK may be lost. So we set the state of the large data uplink
     * indicating that all frames succesfull received. With this way, any
     * future frames with the last frame, will receive an ACK
     */
    LD_status.state = LD_STATE_RECV_OK;

    temp_pkt = get_pkt(LD_status.rx_size);
    if(C_ASSERT(pkt == NULL)) {
      return SATR_ERROR;
    }

    if(unpack_pkt(LD_status.buf, temp_pkt, LD_status.rx_size) == SATR_OK) {
      route_pkt(pkt);
    }
    else {
      verification_app(pkt);
      free_pkt(pkt);
    }

    return SATR_OK;
}

SAT_returnState large_data_downlinkTx_api(tc_tm_pkt *pkt) {

    uint32_t i;
    uint16_t size;
    uint8_t subtype;
    TC_TM_app_id app_id;
    SAT_returnState res;
    tc_tm_pkt *temp_pkt = 0;

    if(C_ASSERT(pkt == NULL || pkt->data == NULL)) {
      return SATR_ERROR;
    }

    if (pkt->type == TC) {
      app_id = pkt->app_id;
    }
    else if (pkt->type == TM) {
      app_id = pkt->dest_id;
    }
    else {
      return SATR_ERROR;
    }

    res = pack_pkt(LD_status.buf, pkt, &size);
    if(C_ASSERT(res != SATR_OK)) {
      return SATR_ERROR;
    }

    if(C_ASSERT(size <= MAX_PKT_DATA)) {
      return SATR_ERROR;
    }

    LD_status.app_id = app_id;

    LD_status.ld_num = 0;
    LD_status.tx_size = size;
    LD_status.tx_pkt = ceil((float) size / LD_PKT_DATA);

    LD_status.state = LD_STATE_TRANSMITING;
    LD_status.started = HAL_GetTick();

    LD_status.timeout = HAL_GetTick();
    LD_status.tx_lid++;

    for(i = 0; i < LD_status.tx_pkt; i++) {

        large_data_downlinkPkt(&temp_pkt, LD_status.tx_lid, i, app_id);

        size = LD_status.tx_size - (i * LD_PKT_DATA);
	if (size > LD_PKT_DATA) {
	  size = LD_PKT_DATA;
	}

        /*
         * Copy the proper amount of data into the sub-frames and send them!
         */
        memcpy(temp_pkt->data + LD_PKT_DATA_HDR_SIZE,
	       LD_status.buf + (i * LD_PKT_DATA), size);

        temp_pkt->len = size;

	if (i == 0) {
	  subtype = TM_LD_FIRST_DOWNLINK;
	}
	else if (i == LD_status.tx_pkt - 1) {
	  subtype = TM_LD_LAST_DOWNLINK;
	}
	else {
	  subtype = TM_LD_INT_DOWNLINK;
	}

        large_data_updatePkt(temp_pkt, size, subtype);
        route_pkt(temp_pkt);

        /*FIXME: Why a so large delay? */
        HAL_Delay(1);
    }
    return SATR_OK;
}

SAT_returnState large_data_ackTx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    if (C_ASSERT(pkt == NULL || pkt->data == NULL)) {
      return SATR_ERROR;
    }

    lid = pkt->data[0];
    cnv8_16(&pkt->data[1], &ld_num);
    //if(!C_ASSERT(LD_status.app_id != pkt->dest_id) == true)                                             { return SATR_ERROR; }
    //if(!C_ASSERT(LD_status.state == LD_STATE_TRANSMITING && (app_id == GND_APP_ID || app_id == DBG_APP_ID)) == true)    { return SATR_ERROR; }
    if(C_ASSERT(LD_status.tx_lid != lid)) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id,
			    lid, TC_LD_ABORT_RE_DOWNLINK);
        if(C_ASSERT(temp_pkt == NULL)) {
          return SATR_ERROR;
        }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    if(C_ASSERT(LD_status.tx_pkt != ld_num)) {
      return SATR_ERROR;
    }

    LD_status.state = LD_STATE_FREE;
    LD_status.tx_lid++;
    LD_status.ld_num = 0;
    LD_status.timeout = 0;
    LD_status.started = 0;

    return SATR_OK;
}


SAT_returnState large_data_retryTx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    uint8_t subtype;
    TC_TM_app_id app_id;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    if (C_ASSERT(pkt == NULL || pkt->data == NULL)) {
      return SATR_ERROR;
    }

    lid = pkt->data[0];
    cnv8_16(&pkt->data[1], &ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;

    //if(!C_ASSERT(LD_status.app_id != pkt->dest_id) == true)                                             { return SATR_ERROR; }
    //if(!C_ASSERT(LD_status.state == LD_STATE_TRANSMITING && (app_id == GND_APP_ID || app_id == DBG_APP_ID)) == true)    { return SATR_ERROR; }
    //if(!C_ASSERT(LD_status.tx_pkt < ld_num) == true)                                               { return SATR_ERROR; }
    if(C_ASSERT(LD_status.tx_lid != lid)) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TC_LD_ABORT_RE_DOWNLINK); 
        if(C_ASSERT(temp_pkt == NULL)) {
          return SATR_ERROR;
        }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    large_data_downlinkPkt(&temp_pkt, LD_status.tx_lid, ld_num, app_id);

    size = LD_status.tx_size - (ld_num * LD_PKT_DATA);
    if(size > MAX_PKT_DATA) {
      size = LD_PKT_DATA;
    }

    memcpy(temp_pkt->data + LD_PKT_DATA_HDR_SIZE,
	   LD_status.buf + (ld_num * LD_PKT_DATA), size);
    temp_pkt->len = size;

    if(ld_num== 0) {
      subtype = TM_LD_FIRST_DOWNLINK;
    }
    else if(ld_num == LD_status.tx_pkt - 1) {
      subtype = TM_LD_LAST_DOWNLINK;
    }
    else {
      subtype = TM_LD_INT_DOWNLINK;
    }

    large_data_updatePkt(temp_pkt, size, subtype);
    route_pkt(temp_pkt);

    LD_status.timeout = HAL_GetTick();

    return SATR_OK;
}

SAT_returnState large_data_updatePkt(tc_tm_pkt *pkt, uint16_t size, uint8_t subtype) {

    pkt->ser_subtype = subtype;
    pkt->len = size+LD_PKT_DATA_HDR_SIZE;

    return SATR_OK;
}

SAT_returnState large_data_downlinkPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t n,
				       uint16_t dest_id) {
    if(C_ASSERT(pkt == NULL)) {
      return SATR_ERROR;
    }
    *pkt = get_pkt(PKT_NORMAL);
    if(C_ASSERT(*pkt == NULL)) {
      return SATR_ERROR;
    }
    crt_pkt(*pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_LARGE_DATA_SERVICE, 0, dest_id); //what dest_id ?

    (*pkt)->data[0] = lid;
    cnv16_8(n, &(*pkt)->data[1]);

    return SATR_OK;
}

SAT_returnState large_data_verifyPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t n,
				     uint16_t dest_id) {
    if(C_ASSERT(pkt == NULL)) {
      return SATR_ERROR;
    }
    *pkt = get_pkt(PKT_NORMAL);
    if(C_ASSERT(*pkt == NULL)) {
      return SATR_ERROR;
    }

    crt_pkt(*pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_LARGE_DATA_SERVICE,
	    TM_LD_ACK_UPLINK, dest_id);

    (*pkt)->data[0] = lid;
    cnv16_8(n, &(*pkt)->data[1]);

    (*pkt)->len = 3;

    return SATR_OK;
}

SAT_returnState large_data_abortPkt(tc_tm_pkt **pkt, uint8_t lid,
				    uint16_t dest_id, uint8_t subtype) {

    if(C_ASSERT(*pkt == NULL)) {
      return SATR_ERROR;
    }
    *pkt = get_pkt(PKT_NORMAL);
    if(C_ASSERT(*pkt == NULL)) {
      return SATR_ERROR;
    }
    crt_pkt(*pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_LARGE_DATA_SERVICE, subtype, dest_id);

    (*pkt)->data[0] = lid;
    (*pkt)->data[1] = SATR_ALREADY_SERVICING;

    (*pkt)->len = 1;

    return SATR_OK;
}

SAT_returnState large_data_abort_api(tc_tm_pkt *pkt) {

    LD_status.state = LD_STATE_FREE;
    LD_status.ld_num = 0;
    LD_status.timeout = 0;
    LD_status.started = 0;

    return SATR_OK;
}

SAT_returnState large_data_timeout() {

    tc_tm_pkt *temp_pkt = 0;

    if(LD_status.state == LD_STATE_TRANSMITING) {
        large_data_abortPkt(&temp_pkt, LD_status.tx_lid, LD_status.app_id,
			    TM_LD_ABORT_SE_DOWNLINK);
        if(C_ASSERT(temp_pkt == NULL)) {
          return SATR_ERROR;
        }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }
    else if(LD_status.state == LD_STATE_RECEIVING) {
        large_data_abortPkt(&temp_pkt, LD_status.rx_lid, LD_status.app_id,
			    TM_LD_ABORT_RE_UPLINK);
        if(C_ASSERT(temp_pkt == NULL)) {
          return SATR_ERROR;
        }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    return SATR_OK;
}

void
large_data_IDLE ()
{

  uint32_t tmp_time = HAL_GetTick ();

  if (LD_status.timeout != 0
      && (((tmp_time - LD_status.timeout) > LD_TIMEOUT)
	  || ((tmp_time - LD_status.started) > LD_MAX_TRANSFER_TIME))) {
    large_data_timeout ();

    LD_status.state = LD_STATE_FREE;
    LD_status.ld_num = 0;
    LD_status.timeout = 0;
    LD_status.started = 0;
  }
}

/**
 * Resets all the internal large data structures
 */
void
large_data_init ()
{
  LD_status.state = LD_STATE_FREE;
  LD_status.ld_num = 0;
  LD_status.timeout = 0;
  LD_status.started = 0;
}
