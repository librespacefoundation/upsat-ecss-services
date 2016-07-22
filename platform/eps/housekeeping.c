#include "housekeeping.h"

#include "eps_state.h"
#include "eps_configuration.h"
#include "eps_power_module.h"
#include "eps_non_volatile_mem_handling.h"

#undef __FILE_ID__
#define __FILE_ID__ 20

extern EPS_State eps_board_state;
extern EPS_PowerModule power_module_top, power_module_bottom, power_module_left, power_module_right;


SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {
   return SATR_ERROR;
}

uint8_t wod_test[6] = { 1,2,3,4,5,6 };

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {


 	/*EPS housekeeping WOD
	 * the order of transmission is:
	 *  Battery Voltage  - uint8_t - range:3V-15.75V resolution:50mv - reading*71,6mv+3 is the voltage - not 50mv resolution due to voltage divider.
	 *  Battery Current  - uint8_t - range:-1A - 1.008A resolution:7.87mA - reading*4.601mA+1 is the current
	 *  3v3 bus Current  - uint8_t - range: 0A - 6.375A resolution:25mA - reading*11.72mA is the current
	 *  5v  bus Current  - uint8_t - range: 0A - 6.375A resolution:25mA - reading*11.72mA is the current
	 *  EPS cpu temp     - uint8_t - range:-15oC - 48.75oC resolution: 0.25oC - reading + 15 is the temperature
	 *  EPS battery temp - uint8_t - range:-15oC - 48.75oC resolution: 0.25oC - reading + 15 is the temperature  */


    pkt->data[0] = (HK_struct_id)sid;

    if(sid == HEALTH_REP) {

     	/* battery voltage
    	 *
    	 * eps_board_state.battery_voltage is measured in eps_update_state from the internal 12bit adc.
    	 * arithmetic range: 0-4095
    	 * voltage measurement range:  0 - 18.333V for adc range 0-3.3V (0.18V/V)
    	 * expected measurement range: (2.5*3=)7.5V - (4.2*3=)12.6V
    	 * */
//    	pkt->data[1]  = (uint8_t)((eps_board_state.battery_voltage - ADC_VALUE_3V_BAT_VOLTAGE)>>4);/*shift from 12 bit resolution uint to 8bit uint and typecast*/

    	uint32_t battery_voltage_buffer_32bit = 23*eps_board_state.battery_voltage;//multiplication in 32 bit to avoid overflow (23*4095 = 94185)

    	pkt->data[1]  = (uint8_t)( (battery_voltage_buffer_32bit>>8) - 60) ;/*shift from 12 bit resolution uint to 8bit uint and typecast*/

     	/* battery current
    	 *
    	 * eps_board_state.battery_current_plus and battery_current_minus are measured in eps_update_state from the internal 12bit adc.
    	 * arithmetic range: 0-4095
    	 * current measurement range:  0 - 1,178A for adc range 0-3.3V (2.8V/A) so I+ - I- will have a range of -1.178 to 1.178
    	 * expected measurement range: the above full scale
    	 * */
//        uint16_t battery_current_buffer = eps_board_state.battery_current_plus - eps_board_state.battery_current_minus + ADC_VALUE_1A_BAT_CURRENT;
//        /*if negative, saturate to zero*/
//        if(battery_current_buffer<0){battery_current_buffer=0;}
//
//        pkt->data[2]  = (uint8_t)( battery_current_buffer>>4);/*shift from 12 bit resolution uint to 8bit uint and typecast*/
        volatile uint32_t battery_current_buffer = (uint32_t)(eps_board_state.battery_current_plus +4095);
        battery_current_buffer = battery_current_buffer - eps_board_state.battery_current_minus;
        battery_current_buffer = 2408*battery_current_buffer;

        uint8_t battery_current_buffer_8bit = (uint8_t)( battery_current_buffer>>24);
        if(battery_current_buffer_8bit<24){battery_current_buffer_8bit=23;}

        pkt->data[2]  = battery_current_buffer_8bit - 23;

     	/* 5v and 3v3 rails current
    	 *
    	 * eps_board_state.v3_3_current_avg and v5_current_avg are measured in eps_update_state from the internal 12bit adc.
    	 * arithmetic range: 0-4095
    	 * current measurement range:  0 - 3A for adc range 0-3.3V (1.1V/A)
    	 * expected measurement range: the above full scale - 3A is the maximum output current of the buck module.
    	 * */
//        pkt->data[3]  = (uint8_t)(eps_board_state.v3_3_current_avg>>4);
//        pkt->data[4]  = (uint8_t)(eps_board_state.v5_current_avg>>4);

        uint32_t v3_3_current_buffer_32bit = 489895*eps_board_state.v3_3_current_avg;
        pkt->data[3]  = (uint8_t)(v3_3_current_buffer_32bit>>24);

        uint32_t v5_current_buffer_32bit = 489895*eps_board_state.v5_current_avg;
        pkt->data[4]  = (uint8_t)(v5_current_buffer_32bit>>24);

     	/* cpu_temperature
    	 *
    	 * eps_board_state.cpu_temperature is measured in eps_update_state from  the internal 12bit adc.
    	 * arithmetic range: 0-4095
    	 * temperature measurement range: the temperature is returned in proper signed format in celcius degrees.
    	 * expected measurement range:  the expected range is supposed to be -X to a max of 65 Which is the battery temperature limit.
    	 * */
        int32_t cpu_temperature_buffer = eps_board_state.cpu_temperature;

        if(cpu_temperature_buffer<-15){cpu_temperature_buffer=-15;}//clamp to -15


//        pkt->data[5]  = (uint8_t)(cpu_temperature_buffer+15);
        pkt->data[5]  = (uint8_t)( (cpu_temperature_buffer+15)>>2 );


     	/* battery temperature
    	 *
    	 * eps_board_state.battery_temp is measured in eps_update_state from  the tc74 i2c temperature sensors.
    	 * arithmetic range: -128 - +128
    	 * temperature measurement range: the temperature is returned in proper signed format in celcius degrees.
    	 * expected measurement range:  the expected range is supposed to be -X to a max of 65 Which is the battery temperature limit.
    	 * */
        int16_t battery_temperature_buffer = eps_board_state.battery_temp;
        if(battery_temperature_buffer<-15){battery_temperature_buffer=-15;}//clamp to -15

        //pkt->data[6]  = (uint8_t)( battery_temperature_buffer +15);
        pkt->data[6]  = (uint8_t)( (battery_temperature_buffer+15)>>2 );

        /*packet length*/
        pkt->len = 7;
        SYSVIEW_PRINT("EPS %u,%u,%u,%u,%u,%u", pkt->data[1], pkt->data[2], pkt->data[3], pkt->data[4], pkt->data[5],  pkt->data[6]);

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

    	/* battery temperature safety */
    	pkt->data[size] = (uint8_t)(eps_board_state.EPS_safety_temperature_mode );
        size += 1;


        /*edo vale fash*/
        pkt->len = size;
    }

    return SATR_OK;
}
