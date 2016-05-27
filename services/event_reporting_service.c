#include "event_reporting_service.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

static uint8_t strNo[] = "No";

SAT_returnState event_app(tc_tm_pkt * pkt) {

    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }

    uint8_t ev_id = pkt->data[0];

    if(!C_ASSERT(ev_id < LAST_EV_EVENT) == true) { return SATR_ERROR; }

    event_log(pkt->data, EV_DATA_SIZE);

    return SATR_OK;
}


SAT_returnState event_crt_pkt_api(uint8_t *buf, uint8_t *f, uint16_t fi, uint32_t l, uint8_t *e, uint16_t *size, SAT_returnState mode) {

    uint8_t sub_type;
    uint8_t res_crc;

    if(mode == SATR_OK) { sub_type = TM_EV_NORMAL_REPORT; }
    else { sub_type = TM_EV_ERROR_REPORT; }

    buf[0] = HLDLC_START_FLAG;
    buf[1] = 0x08;
    buf[2] = SYSTEM_APP_ID;
    buf[3] = 0xC0;
    buf[4] = 5;

    buf[7] = 16;
    buf[8] = TC_EVENT_SERVICE;
    buf[9] = sub_type;
    buf[10] = DBG_APP_ID;

    if(strnlen(e, 200) > 200) { e = strNo; }

    if(mode == SATR_OK){ 
        sprintf((char*)&buf[11], "Event %s,%d,%d,%s\n", f, fi, l, e); }
    else{ 
        sprintf((char*)&buf[11], "Error %s,%d,%d,%s\n", f, fi, l, e); }
    
    *size = strnlen(&buf[11], 200);
    event_log(&buf[11], *size);

    *size += 11 + 1;
    buf[*size] = 0;

    buf[5] = 0;
    buf[6] = *size - 6 -2 + 2;
    checkSum(&buf[1], *size - 1, &res_crc);
    buf[(*size)+1] = res_crc;
    buf[(*size)+2] = HLDLC_START_FLAG;

    *size += 3;

    return SATR_OK;
}

SAT_returnState event_boot(const uint8_t reset_source, const uint8_t boot_counter) {

    tc_tm_pkt *temp_pkt = 0;

    if(event_crt_pkt(&temp_pkt, EV_sys_boot) != SATR_OK) { return SATR_ERROR; }
    temp_pkt->data[5] = reset_source;
    temp_pkt->data[6] = boot_counter;

    /*zero padding for fixed length*/
    for(uint8_t i = 7; i < EV_DATA_SIZE; i++) { temp_pkt->data[i] = 0; }

    if(SYSTEM_APP_ID == OBC_APP_ID) {
        event_log(temp_pkt->data, EV_DATA_SIZE);
    } else {
        route_pkt(temp_pkt);

    }
    return SATR_OK;
}

SAT_returnState event_pkt_pool_timeout() {

    tc_tm_pkt *temp_pkt = 0;

    if(event_crt_pkt(&temp_pkt, EV_pkt_pool_timeout) != SATR_OK) { return SATR_ERROR; }

    /*zero padding for fixed length*/
    for(uint8_t i = 6; i < EV_DATA_SIZE; i++) { temp_pkt->data[i] = 0; }
    
    if(SYSTEM_APP_ID == OBC_APP_ID) {
        event_log(temp_pkt->data, EV_DATA_SIZE);
    } else {
        route_pkt(temp_pkt);

    }
    return SATR_OK;
}

SAT_returnState event_crt_pkt(tc_tm_pkt **pkt, const EV_event event) {

    *pkt = get_pkt(PKT_NORMAL);
    if(!C_ASSERT(*pkt != NULL) == true) { return SATR_ERROR; }

    crt_pkt(*pkt, OBC_APP_ID, TC, TC_ACK_NO, TC_EVENT_SERVICE, TM_EV_NORMAL_REPORT, SYSTEM_APP_ID);

    uint32_t time_temp = HAL_sys_GetTick();
    (*pkt)->data[0] = event;
    cnv32_8(time_temp, &((*pkt)->data[1]));

    (*pkt)->len = 12;

    return SATR_OK;
}
