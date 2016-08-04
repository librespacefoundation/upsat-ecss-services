#include "housekeeping.h"
#include "housekeeping_service.h"
#include "service_utilities.h"
#include "sensors.h"
#include "comms_manager.h"
#include "flash.h"
#include "stats.h"
#include "wod_handling.h"
#include "sysview.h"
#include <math.h>
#include "ecss_stats.h"

extern comms_rf_stat_t comms_stats;

#undef __FILE_ID__
#define __FILE_ID__ 24

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data, uint8_t len) {

   if(sid == EXT_WOD_REP) {
        SYSVIEW_PRINT("Storing exWOD of size %u\n");
        store_ex_wod_obc(data, len);
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
          /* Shit hack */
          pkt->data[1] = 1;
        }
        else {
          pkt->data[1] = wod_convert_temperature(temp);
        }
        pkt->len = 2;
        SYSVIEW_PRINT("COMMS HEALTH: Temp %u", pkt->data[1]);
    } else if(sid == EX_HEALTH_REP) {
        SYSVIEW_PRINT("COMMS EX HEALTH REP");
        i = 1;
        cnv32_8(HAL_sys_GetTick(), &pkt->data[i]);
        i += sizeof(uint32_t);
        /* Store the reset source */
        b = comms_stats.rst_src;
        pkt->data[i] = b;
        i += sizeof(uint8_t);
        pkt->data[i] = assertion_last_file;
        i += sizeof(uint8_t);
        cnv16_8(assertion_last_line, &pkt->data[i]);
        i += sizeof(uint16_t);
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
    } else if(sid == EXT_WOD_REP) {
      SYSVIEW_PRINT("COMMS ONLY EX WOD");
        /*zero padding*/
        memset(&pkt->data[1], 0, SYS_EXT_WOD_SIZE);

        i = COMMS_EXT_WOD_OFFSET;
        cnv32_8(HAL_sys_GetTick(), &pkt->data[i]);
        i += sizeof(uint32_t);
        /* Store the reset source */
        b = comms_stats.rst_src;
        pkt->data[i] = b;
        i += sizeof(uint8_t);
        pkt->data[i] = assertion_last_file;
        i += sizeof(uint8_t);
        cnv16_8(assertion_last_line, &pkt->data[i]);
        i += sizeof(uint16_t);
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
        pkt->len = SYS_EXT_WOD_SIZE + 1;
    }
    return SATR_OK;
}
