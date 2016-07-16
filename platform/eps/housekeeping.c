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

        uint16_t size = 1;

        cnv32_8( HAL_sys_GetTick(), &pkt->data[1]);
        size += 4;

        /*batterypack health status*/
        pkt->data[size] = (uint8_t)(eps_board_state.batterypack_health_status);
        size += 1;

        /* heater status*/
        EPS_switch_control_status heaters_status = EPS_get_control_switch_status(BATTERY_HEATERS);
        pkt->data[size] = (uint8_t)heaters_status;
        size += 1;

        /*power module top*/
    	cnv16_8( power_module_top.voltage, &pkt->data[size]);
        size += 2;
    	cnv16_8( power_module_top.current, &pkt->data[size]);
        size += 2;
    	pkt->data[size] = (uint8_t)power_module_top.pwm_duty_cycle;
        size += 1;

        /*power module bottom*/
    	cnv16_8( power_module_bottom.voltage, &pkt->data[size]);
        size += 2;
    	cnv16_8( power_module_bottom.current, &pkt->data[size]);
        size += 2;
    	pkt->data[size] = (uint8_t)power_module_bottom.pwm_duty_cycle;
        size += 1;

        /*power module left*/
    	cnv16_8( power_module_left.voltage, &pkt->data[size]);
        size += 2;
    	cnv16_8( power_module_left.current, &pkt->data[size]);
        size += 2;
    	pkt->data[size] = (uint8_t)power_module_left.pwm_duty_cycle;
        size += 1;

        /*power module right*/
    	cnv16_8( power_module_right.voltage, &pkt->data[size]);
        size += 2;
    	cnv16_8( power_module_right.current, &pkt->data[size]);
        size += 2;
    	pkt->data[size] = (uint8_t)power_module_right.pwm_duty_cycle;
        size += 1;

    	/* deployment status*/
    	EPS_deployment_status deployment_status = EPS_check_deployment_status();
    	pkt->data[size] = (uint8_t)deployment_status;
        size += 1;

    	/* battery voltage safety */
    	pkt->data[size] = (uint8_t)(eps_board_state.EPS_safety_battery_mode );
        size += 1;

    	/* battery voltage safety */
    	pkt->data[size] = (uint8_t)(eps_board_state.EPS_safety_temperature_mode );
        size += 1;

        /*edo vale fash*/
        pkt->len = size;
    }

    return SATR_OK;
}
