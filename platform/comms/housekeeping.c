#include "housekeeping.h"
#include "sensors.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {
   return SATR_ERROR;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {
    
    pkt->data[0] = (HK_struct_id)sid;

	if(sid == HEALTH_REP) {
        pkt->data[1] = (uint8_t)(get_temp_adt7420());

        pkt->len = 2;
    } else if(sid == EX_HEALTH_REP) {

        //cnv.cnv32 = time.now();
        cnv32_8(HAL_sys_GetTick(), &pkt->data[1]);
        pkt->len = 5;
    }

    return SATR_OK;
}