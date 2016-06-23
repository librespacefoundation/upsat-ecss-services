#include "housekeeping.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {
   
   if(sid == EX_HEALTH_REP) {

        struct time_utc temp_time;

        temp_time.day = pkt->data[1];
        temp_time.month = pkt->data[2];
        temp_time.year = pkt->data[3];
        
        temp_time.hour = pkt->data[4];
        temp_time.min = pkt->data[5];
        temp_time.sec = pkt->data[6];
    }

   return SATR_OK;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {
    
    pkt->data[0] = (HK_struct_id)sid;
    
    if(sid == EX_HEALTH_REP) {

        //cnv.cnv32 = time.now();
        cnv32_8(HAL_sys_GetTick(), &pkt->data[1]);
        cnvF_8(adcs_state.gyr[0], &pkt->data[5]);
        cnvF_8(adcs_state.gyr[1], &pkt->data[9]);
        cnvF_8(adcs_state.gyr[2], &pkt->data[13]);
        cnvF_8(adcs_state.rm_mag[0], &pkt->data[17]);
        cnvF_8(adcs_state.rm_mag[1], &pkt->data[21]);
        cnvF_8(adcs_state.rm_mag[1], &pkt->data[25]);
        cnvF_8(adcs_state.v_sun[0], &pkt->data[29]);
        cnvF_8(adcs_state.v_sun[1], &pkt->data[33]);
        cnvF_8(adcs_state.v_sun[2], &pkt->data[37]);
        cnvF_8(adcs_state.v_sun[3], &pkt->data[41]);
        cnvF_8(adcs_state.v_sun[4], &pkt->data[45]);
        cnvF_8(adcs_state.long_sun, &pkt->data[49]);
        cnvF_8(adcs_state.lat_sun, &pkt->data[53]);
        cnv32_8(adcs_actuator.m_RPM, &pkt->data[57]);
        pkt->len = 61;
    }else if(sid == SU_SCI_HED_REP){
        
    } 

    return SATR_OK;
}
