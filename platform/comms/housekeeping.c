#include "housekeeping.h"
#include "sensors.h"
#include "comms_manager.h"
#include "flash.h"
#include "stats.h"
#include "wod_handling.h"
#include "sysview.h"

extern comms_rf_stat_t comms_stats;

#undef __FILE_ID__
#define __FILE_ID__ 24

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {

   if(sid == WOD_REP) {
        //data[98] = 0xFE;
        //data[99] = 0xED;
        //data[100] = 0xBE;
        //data[101] = 0xEF;
        
        send_payload(&data[1], (size_t)MAX_PKT_DATA, 0, COMMS_DEFAULT_TIMEOUT_MS);
   }

   return SATR_ERROR;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {

    pkt->data[0] = (HK_struct_id)sid;
    if(sid == HEALTH_REP) {
        float temp = comms_rf_stats_get_temperature(&comms_stats);

        if(isnanf(temp)){
            pkt->data[1] = 0;
        } else {
            pkt->data[1] = wod_convert_temperature(temp);
        }
        pkt->len = 2;

        SYSVIEW_PRINT("COMMS %u", pkt->data[1]);

    } else if(sid == EX_HEALTH_REP ||
              sid == COMMS_EXT_WOD_REP) {

        uint16_t size = 1;
        //cnv.cnv32 = time.now();
        cnv32_8(HAL_sys_GetTick(), &pkt->data[size]);
        size += 4;
        cnv32_8(flash_read_trasmit(), &pkt->data[size]);
        size += 4;

        cnv32_8(comms_stats.rx_failed_cnt, &pkt->data[size]);
        size += 4;
        cnv32_8(comms_stats.rx_crc_failed_cnt, &pkt->data[size]);
        size += 4;
        cnv32_8(comms_stats.tx_failed_cnt, &pkt->data[size]);
        size += 4;
        cnv32_8(comms_stats.tx_frames_cnt, &pkt->data[size]);
        size += 4;
        cnv32_8(comms_stats.rx_frames_cnt, &pkt->data[size]);
        size += 4;

        cnv32_8(comms_stats.last_tx_error_code, &pkt->data[size]);
        size += 4;
        cnv32_8(comms_stats.last_rx_error_code, &pkt->data[size]);
        size += 4;
        cnv32_8(comms_stats.invalid_dest_frames_cnt, &pkt->data[size]);
        size += 4;

        pkt->len = size;

    }

    return SATR_OK;
}
