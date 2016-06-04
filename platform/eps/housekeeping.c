#include "housekeeping.h"

#include "eps_state.h"
#include "eps_configuration.h"
#include "eps_power_module.h"
#include "eps_non_volatile_mem_handling.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

extern EPS_State eps_board_state;
extern EPS_PowerModule power_module_top, power_module_bottom, power_module_left, power_module_right;


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

        pkt->len = 7;
    } else if(sid == EX_HEALTH_REP) {

        //cnv.cnv32 = time.now();
        cnv32_8( HAL_sys_GetTick(), &pkt->data[1]);

        /*batterypack health status*/
        pkt->data[5] = (uint8_t)(eps_board_state.batterypack_health_status);

        /* heater status*/
        EPS_switch_control_status heaters_status = EPS_get_control_switch_status(BATTERY_HEATERS);
        pkt->data[6] = (uint8_t)heaters_status;


        /*power module top*/
    	cnv16_8( power_module_top.voltage, &pkt->data[7]);
    	cnv16_8( power_module_top.current, &pkt->data[9]);
    	pkt->data[11] = (uint8_t)power_module_top.pwm_duty_cycle;

        /*power module bottom*/
    	cnv16_8( power_module_bottom.voltage, &pkt->data[12]);
    	cnv16_8( power_module_bottom.current, &pkt->data[14]);
    	pkt->data[16] = (uint8_t)power_module_bottom.pwm_duty_cycle;

        /*power module left*/
    	cnv16_8( power_module_left.voltage, &pkt->data[17]);
    	cnv16_8( power_module_left.current, &pkt->data[19]);
    	pkt->data[21] = (uint8_t)power_module_left.pwm_duty_cycle;

        /*power module right*/
    	cnv16_8( power_module_right.voltage, &pkt->data[22]);
    	cnv16_8( power_module_right.current, &pkt->data[24]);
    	pkt->data[26] = (uint8_t)power_module_right.pwm_duty_cycle;

    	/* deployment status*/
    	EPS_deployment_status deployment_status = EPS_check_deployment_status();
    	pkt->data[27] = (uint8_t)deployment_status;

    	/* battery voltage safety */
    	pkt->data[28] = (uint8_t)(eps_board_state.EPS_safety_battery_mode );

    	/* battery voltage safety */
    	pkt->data[29] = (uint8_t)(eps_board_state.EPS_safety_temperature_mode );


        /*edo vale fash*/
        pkt->len = 30;
    }

    return SATR_OK;
}
