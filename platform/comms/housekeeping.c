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

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data, uint8_t len) {

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
    float temp;
    size_t i;
    uint8_t b;
    pkt->data[0] = (HK_struct_id)sid;
    if(sid == HEALTH_REP) {
        temp = comms_rf_stats_get_temperature(&comms_stats);
        if(isnanf(temp)){
          return SATR_ERROR;
        }
        pkt->data[1] = wod_convert_temperature(temp);

        pkt->len = 2;

        SYSVIEW_PRINT("COMMS %u", pkt->data[1]);

    } else if(sid == EX_HEALTH_REP) {
	i = 1;
        cnv32_8(HAL_sys_GetTick(), &pkt->data[i]);
        i += sizeof(uint32_t);
        /* Store the reset source */
        b = comms_stats.rst_src;
        pkt->data[i] = b;
        i += sizeof(uint8_t);
        /* FIXME: Add the last assert line */
        cnv32_8(0x0, &pkt->data[i]);
        i += sizeof(uint32_t);
        cnv32_8(flash_read_trasmit(__COMMS_RF_KEY_FLASH_OFFSET),
		&pkt->data[i]);
        i += sizeof(uint32_t);
        b = flash_read_trasmit(__COMMS_HEADLESS_TX_FLASH_OFFSET);
        pkt->data[i] = b;
        i += sizeof(uint8_t);
        cnv16_8(comms_stats.rx_failed_cnt, &pkt->data[i]);
        i += sizeof(uint16_t);
        cnv16_8(comms_stats.rx_crc_failed_cnt, &pkt->data[i]);
        i += sizeof(uint16_t);
        cnv16_8(comms_stats.tx_failed_cnt, &pkt->data[i]);
        i += sizeof(uint16_t);
        cnv16_8(comms_stats.tx_frames_cnt, &pkt->data[i]);
        i += sizeof(uint16_t);
        cnv16_8(comms_stats.rx_frames_cnt, &pkt->data[i]);
        i += sizeof(uint16_t);
        cnv16_8(comms_stats.last_tx_error_code, &pkt->data[i]);
        i += sizeof(int16_t);
        cnv16_8(comms_stats.last_rx_error_code, &pkt->data[i]);
        i += sizeof(int16_t);
        cnv16_8(comms_stats.invalid_dest_frames_cnt, &pkt->data[i]);
        i += sizeof(uint16_t);
        pkt->len = i;
    } 

    return SATR_OK;
}
