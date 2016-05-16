#include "large_data_service.h"


#undef __FILE_ID__
#define __FILE_ID__ 7

struct _ld_status LD_status;

SAT_returnState large_data_app(tc_tm_pkt *pkt) {

    if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_FIRST_UPLINK)              { large_data_firstRx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_INT_UPLINK)           { large_data_intRx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_LAST_UPLINK)          { large_data_lastRx_api(pkt); }
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_ABORT_SE_UPLINK)      { large_data_abort_api(pkt); } 

    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_ACK_DOWNLINK)         { large_data_ackTx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_REPEAT_DOWNLINK)      { large_data_retryTx_api(pkt); } 
    else if(pkt->ser_type == TC_LARGE_DATA_SERVICE && pkt->ser_subtype == TC_LD_ABORT_RE_DOWNLINK)    { large_data_abort_api(pkt); }

    return SATR_OK;
}

/*downlink*/
SAT_returnState large_data_firstRx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    TC_TM_app_id app_id;
    MS_sid sid;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    lid = pkt->data[0]; 

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) { return SATR_ERROR; } 
    if(!C_ASSERT(LD_status.state == LD_STATE_FREE) == true) { 
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TM_LD_ABORT_RE_UPLINK); 
        if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    cnv8_16(&pkt->data[1], &ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;
    size = pkt->len; //ldata headers

    if(!C_ASSERT(app_id == IAC_APP_ID || app_id == GND_APP_ID) == true)                                             { return SATR_ERROR; }
    if(!C_ASSERT(ld_num == 0) == true)                                                                              { return SATR_ERROR; }
    if(!C_ASSERT(size > 0) == true)                                                                                 { return SATR_ERROR; } 
    //if(!C_ASSERT((app_id == IAC_APP_ID && sid == FOTOS) || (app_id == GND_APP_ID && sid <= SU_SCRIPT_7 )) == true) { return SATR_ERROR; } 

    LD_status.ld_num = ld_num;
    LD_status.rx_lid = lid;
    LD_status.state = LD_STATE_RECEIVING;
    LD_status.started = time_now();

    for(uint16_t i = LD_PKT_DATA_HDR_SIZE; i < size; i++) { LD_status.buf[(LD_status.ld_num * LD_PKT_DATA) + i] = pkt->data[i]; }

    LD_status.ld_num = size;
    LD_status.timeout = time_now();

    large_data_verifyPkt(&temp_pkt, LD_status.rx_lid, LD_status.ld_num, app_id);
    route_pkt(temp_pkt);

    return SATR_OK;
}

SAT_returnState large_data_intRx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    TC_TM_app_id app_id;
    MS_sid sid;
    tc_tm_pkt *temp_pkt = 0;
    MS_mode mode = NO_MODE;
    uint8_t lid;

    lid = pkt->data[0];
    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)         { return SATR_ERROR; } 
    if(!C_ASSERT(LD_status.state == LD_STATE_RECEIVING) == true)    { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.rx_lid == lid) == true) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TM_LD_ABORT_RE_UPLINK); 
        if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    cnv8_16(&pkt->data[1], &ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;
    size = pkt->len; //ldata headers

    if(!C_ASSERT(app_id == IAC_APP_ID || app_id == GND_APP_ID) == true)   { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.ld_num + 1 <= ld_num) == true)                 { return SATR_ERROR; }
    if(!C_ASSERT(size > 0) == true)                                       { return SATR_ERROR; } 
    //if(!C_ASSERT((app_id == IAC_APP_ID && sid == FOTOS) || (app_id == GND_APP_ID && sid <= SU_SCRIPT_7 )) == true) { return SATR_ERROR; } 

    LD_status.ld_num = ld_num;
    LD_status.ld_num += size;

    for(uint16_t i = LD_PKT_DATA_HDR_SIZE; i < size; i++) { LD_status.buf[(LD_status.ld_num * LD_PKT_DATA) + i] = pkt->data[i]; }

    LD_status.timeout = time_now();

    large_data_verifyPkt(&temp_pkt, LD_status.rx_lid, LD_status.ld_num, app_id);
    route_pkt(temp_pkt);

    return SATR_OK;
}

SAT_returnState large_data_lastRx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    TC_TM_app_id app_id;
    MS_sid sid;
    tc_tm_pkt *temp_pkt = 0;
    MS_mode mode = NO_MODE;
    uint8_t lid;

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)         { return SATR_ERROR; } 
    if(!C_ASSERT(LD_status.state == LD_STATE_RECEIVING) == true)    { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.rx_lid == lid) == true) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TM_LD_ABORT_RE_UPLINK); 
        if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    cnv8_16(&pkt->data[1], &ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;
    size = pkt->len; //ldata headers

    if(!C_ASSERT(app_id == IAC_APP_ID || app_id == GND_APP_ID) == true)   { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.ld_num + 1 <= ld_num) == true)                 { return SATR_ERROR; }
    if(!C_ASSERT(size > 0) == true)                                       { return SATR_ERROR; } 
    //if(!C_ASSERT((app_id == IAC_APP_ID && sid == FOTOS) || (app_id == GND_APP_ID && sid <= SU_SCRIPT_7 )) == true) { return SATR_ERROR; } 

    LD_status.ld_num = ld_num;

    for(uint16_t i = LD_PKT_DATA_HDR_SIZE; i < size; i++) { LD_status.buf[(LD_status.ld_num * LD_PKT_DATA) + i] = pkt->data[i]; }

    size += LD_status.ld_num;
    LD_status.state = LD_STATE_FREE;
    LD_status.ld_num = 0;
    LD_status.timeout = 0;
    LD_status.started = 0;

    large_data_verifyPkt(&temp_pkt, LD_status.rx_lid, LD_status.ld_num, app_id);
    route_pkt(temp_pkt);

    temp_pkt = get_pkt();
    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }
    if(unpack_pkt(LD_status.buf, temp_pkt, size) == SATR_OK) { route_pkt(pkt); } 
    else { verification_app(pkt); free_pkt(pkt); }

    return SATR_OK;
}



SAT_returnState large_data_downlinkTx_api(tc_tm_pkt *pkt) {

    uint16_t size;
    uint32_t from;
    uint32_t to;
    uint8_t subtype;
    TC_TM_app_id app_id;
    MS_sid sid;
    SAT_returnState res;
    tc_tm_pkt *temp_pkt = 0;

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) { return SATR_ERROR; }

    app_id = (TC_TM_app_id)pkt->dest_id; //check if this is ok

    if(!C_ASSERT(LD_status.state == LD_STATE_FREE && app_id == GND_APP_ID) == true) {
        return SATR_ERROR; 
    }

    if(!C_ASSERT(sid == FOTOS || sid == EVENT_LOG || sid == SU_LOG) == true)    { return SATR_ERROR; }

    pack_pkt(LD_status.buf, temp_pkt, &size);

    LD_status.app_id = app_id;

    LD_status.ld_num = 0;
    LD_status.tx_size = size;
    LD_status.tx_pkt = (float)size + (float)(LD_PKT_DATA / 2) / LD_PKT_DATA;

    LD_status.state = LD_STATE_TRANSMITING;
    LD_status.started = time_now();

    LD_status.timeout = time_now();
    LD_status.tx_lid++;

    for(uint8_t i = 0; i < LD_status.tx_pkt; i++) {

        large_data_downlinkPkt(&temp_pkt, LD_status.tx_lid, i, app_id);

        size = LD_status.tx_size - (i * LD_PKT_DATA);
        if(size > LD_PKT_DATA) { size = LD_PKT_DATA; }

        for(uint16_t b = LD_PKT_DATA_HDR_SIZE; b < size; b++) { pkt->data[b] = LD_status.buf[(i * LD_PKT_DATA) + b]; }

        temp_pkt->len = size;

        if(i == 0) { subtype = TM_LD_FIRST_DOWNLINK; }
        else if(i == LD_status.tx_pkt - 1) { subtype = TM_LD_LAST_DOWNLINK; }
        else { subtype = TM_LD_INT_DOWNLINK; }

        large_data_updatePkt(temp_pkt, size, subtype);
        route_pkt(temp_pkt);

        HAL_sys_delay(1);
    }
    return SATR_OK;
}

SAT_returnState large_data_ackTx_api(tc_tm_pkt *pkt) {

    uint16_t ld_num;
    uint16_t size;
    uint8_t subtype;
    uint32_t fnext;
    SAT_returnState res;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    lid = pkt->data[0];
    cnv8_16(&pkt->data[1], &ld_num);

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)                                             { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.app_id != pkt->dest_id) == true)                                             { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.state == LD_STATE_TRANSMITING && LD_status.app_id == GND_APP_ID) == true)    { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.tx_lid == lid) == true) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TC_LD_ABORT_RE_DOWNLINK); 
        if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    if(!C_ASSERT(LD_status.tx_pkt == ld_num) == true) { return SATR_ERROR; }

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
    uint32_t fnext;
    uint8_t subtype;
    TC_TM_app_id app_id;
    tc_tm_pkt *temp_pkt = 0;
    uint8_t lid;

    lid = pkt->data[0];
    cnv8_16(&pkt->data[1], &ld_num);

    app_id = (TC_TM_app_id)pkt->dest_id;

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)                                             { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.app_id != pkt->dest_id) == true)                                             { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.state == LD_STATE_TRANSMITING && LD_status.app_id == GND_APP_ID) == true)    { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.tx_pkt < ld_num) == true)                                               { return SATR_ERROR; }
    if(!C_ASSERT(LD_status.tx_lid == lid) == true) {
        large_data_abortPkt(&temp_pkt, pkt->dest_id, lid, TC_LD_ABORT_RE_DOWNLINK); 
        if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    large_data_downlinkPkt(&temp_pkt, LD_status.tx_lid, ld_num, app_id);

    size = LD_status.tx_size - (ld_num * LD_PKT_DATA);
    if(size > LD_PKT_DATA) { size = LD_PKT_DATA; }

    for(uint16_t b = LD_PKT_DATA_HDR_SIZE; b < size; b++) { pkt->data[b] = LD_status.buf[(ld_num * LD_PKT_DATA) + b]; }

    temp_pkt->len = size;

    if(ld_num== 0) { subtype = TM_LD_FIRST_DOWNLINK; }
    else if(ld_num == LD_status.tx_pkt - 1) { subtype = TM_LD_LAST_DOWNLINK; }
    else { subtype = TM_LD_INT_DOWNLINK; }

    large_data_updatePkt(temp_pkt, size, subtype);
    route_pkt(temp_pkt);

    LD_status.timeout = time_now();

    route_pkt(temp_pkt);

    return SATR_OK;
}

SAT_returnState large_data_updatePkt(tc_tm_pkt *pkt, uint16_t size, uint8_t subtype) {

    pkt->ser_subtype = subtype;
    pkt->len = size+LD_PKT_DATA_HDR_SIZE;

    return SATR_OK;
}

SAT_returnState large_data_downlinkPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t n, uint16_t dest_id) {

    *pkt = get_pkt();
    if(!C_ASSERT(*pkt != NULL) == true) { return SATR_ERROR; }
    crt_pkt(*pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_LARGE_DATA_SERVICE, 0, dest_id); //what dest_id ?

    (*pkt)->data[0] = lid;
    cnv16_8(n, &(*pkt)->data[1]);

    return SATR_OK;
}

SAT_returnState large_data_verifyPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t n, uint16_t dest_id) {

    *pkt = get_pkt();
    if(!C_ASSERT(*pkt != NULL) == true) { return SATR_ERROR; }
    crt_pkt(*pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_LARGE_DATA_SERVICE, TM_LD_ACK_UPLINK, dest_id);

    (*pkt)->data[0] = lid;
    cnv16_8(n, &(*pkt)->data[1]);

    (*pkt)->len = 2;

    return SATR_OK;
}

SAT_returnState large_data_abortPkt(tc_tm_pkt **pkt, uint8_t lid, uint16_t dest_id, uint8_t subtype) {

    *pkt = get_pkt();
    if(!C_ASSERT(*pkt != NULL) == true) { return SATR_ERROR; }
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
        large_data_abortPkt(&temp_pkt, LD_status.tx_lid, LD_status.app_id, TM_LD_ABORT_SE_DOWNLINK); 
        if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }
    else if(LD_status.state == LD_STATE_RECEIVING) {
        large_data_abortPkt(&temp_pkt, LD_status.rx_lid, LD_status.app_id, TM_LD_ABORT_RE_UPLINK); 
        if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }

        route_pkt(temp_pkt);
        return SATR_OK; 
    }

    LD_status.state = LD_STATE_FREE;
    LD_status.ld_num = 0;
    LD_status.timeout = 0;
    LD_status.started = 0;

    return SATR_OK;
}

void large_data_INIT() {

    LD_status.state = LD_STATE_FREE;
    LD_status.ld_num = 0;
    LD_status.timeout = 0;
    LD_status.started = 0;

}
