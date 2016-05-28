#include "housekeeping.h"

#include "eps_state.h"
#include "eps_configuration.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

extern EPS_State eps_board_state;

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {
   return SATR_ERROR;
}

uint8_t wod_test[6] = { 1,2,3,4,5,6 };

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {
    


//	uint16_t v5_current_avg;
//	uint16_t v3_3_current_avg;
//	int16_t battery_voltage;
//	uint16_t battery_current_plus;
//	uint16_t battery_current_minus;
//	int16_t  battery_temp;
//	int32_t cpu_temperature;
//	EPS_battery_tempsense_health batterypack_health_status;

    pkt->data[0] = (HK_struct_id)sid;
    
    if(sid == HEALTH_REP) {
        pkt->data[1] = (uint8_t)(eps_board_state.battery_voltage - ADC_VALUE_3V_BAT_VOLTAGE) ;
        pkt->data[2] = (int8_t)((int8_t)eps_board_state.battery_current_plus - (int8_t)eps_board_state.battery_current_minus);
        pkt->data[3] = (uint8_t)(eps_board_state.v3_3_current_avg );
        pkt->data[4] = (uint8_t)(eps_board_state.v5_current_avg);
        pkt->data[5] = (uint8_t)(eps_board_state.battery_temp);
        pkt->data[6] = (uint8_t)(eps_board_state.cpu_temperature);


        wod_test[0] += 10;
        wod_test[1] += 10;
        wod_test[2] += 10;
        wod_test[3] += 10;
        wod_test[4] += 10;
        wod_test[5] += 10;

        pkt->len = 7;
    } else if(sid == EX_HEALTH_REP) {

        //cnv.cnv32 = time.now();
        cnv32_8( HAL_GetTick(), &pkt->data[1]);
        pkt->data[5] = (uint8_t)(eps_board_state.batterypack_health_status);
        /*edo vale fash*/
        pkt->len = 6;
    }

    return SATR_OK;
}
