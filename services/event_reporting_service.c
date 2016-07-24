#include "event_reporting_service.h"

#include "system.h"
#include "service_utilities.h"
#include "pkt_pool.h"
#include "hldlc.h"

#undef __FILE_ID__
#define __FILE_ID__ 11

#define EV_DATA_SIZE 16

extern SAT_returnState route_pkt(tc_tm_pkt *pkt);
extern uint32_t HAL_sys_GetTick();


SAT_returnState event_app(tc_tm_pkt * pkt) {

    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }

    uint8_t ev_id = pkt->data[0];

    if(!C_ASSERT(ev_id < LAST_EV_EVENT) == true) { return SATR_ERROR; }

    return SATR_OK;
}

SAT_returnState event_boot(const uint8_t reset_source, const uint32_t boot_counter) {

    tc_tm_pkt *temp_pkt = 0;

    if(event_crt_pkt(&temp_pkt, EV_sys_boot) != SATR_OK) { return SATR_ERROR; }
    temp_pkt->data[10] = reset_source;
    cnv32_8(boot_counter, &(temp_pkt->data[11]));

    for(uint8_t i = 15; i < EV_DATA_SIZE; i++) { temp_pkt->data[i] = 0; }

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
    for(uint8_t i = 10; i < EV_DATA_SIZE; i++) { temp_pkt->data[i] = 0; }

    if(SYSTEM_APP_ID == OBC_APP_ID) {
        event_log(temp_pkt->data, EV_DATA_SIZE);
    } else {
        route_pkt(temp_pkt);

    }
    return SATR_OK;
}

SAT_returnState event_ms_err(uint8_t err, uint16_t l) {

    tc_tm_pkt *temp_pkt = 0;

    if(event_crt_pkt(&temp_pkt, EV_ms_err) != SATR_OK) { return SATR_ERROR; }

    /*zero padding for fixed length*/
    temp_pkt->data[10] = err;
    cnv16_8(l, &(temp_pkt->data[11]));

    for(uint8_t i = 14; i < EV_DATA_SIZE; i++) { temp_pkt->data[i] = 0; }

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

    crt_pkt(*pkt, OBC_APP_ID, TC, TC_ACK_NO, TC_EVENT_SERVICE, TM_EV_NORMAL_REPORT, (TC_TM_app_id)SYSTEM_APP_ID);

    uint32_t time_temp = HAL_sys_GetTick();
    (*pkt)->data[0] = (TC_TM_app_id)SYSTEM_APP_ID;
    (*pkt)->data[1] = event;
    cnv32_8(time_temp, &((*pkt)->data[2]));
    (*pkt)->data[6] = 0;
    (*pkt)->data[7] = 0;
    (*pkt)->data[8] = 0;
    (*pkt)->data[9] = 0;

    (*pkt)->len = EV_DATA_SIZE;

    return SATR_OK;
}
