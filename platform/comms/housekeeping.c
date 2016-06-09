#include "housekeeping.h"
#include "sensors.h"
#include "comms_manager.h"
#include "flash.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {

   if(sid == WOD_REP) {
        //data[98] = 0xFE;
        //data[99] = 0xED;
        //data[100] = 0xBE;
        //data[101] = 0xEF;
        
        send_payload(&data[1], (size_t)MAX_PKT_DATA, COMMS_DEFAULT_TIMEOUT_MS);
   }

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
        cnv32_8(flash_read_trasmit(), &pkt->data[5]);

        pkt->len = 9;

    } 

    return SATR_OK;
}