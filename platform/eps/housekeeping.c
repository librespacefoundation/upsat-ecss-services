#include "housekeeping.h"

#include "eps_state.h"
#include "eps_configuration.h"
#include "eps_power_module.h"
#include "eps_non_volatile_mem_handling.h"
#include "sysview.h"
#include "ecss_stats.h"

#include "eps_soft_error_handling.h"





#undef __FILE_ID__
#define __FILE_ID__ 20

extern EPS_State eps_board_state;
extern EPS_PowerModule power_module_top, power_module_bottom, power_module_left, power_module_right;


SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data, uint8_t len) {
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
        pkt->data[5]  = (uint8_t)( (cpu_temperature_buffer+15)<<2 );



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
        pkt->data[6]  = (uint8_t)( (battery_temperature_buffer+15)<<2 );

        /*packet length*/
        pkt->len = 7;
        SYSVIEW_PRINT("EPS %u,%u,%u,%u,%u,%u", pkt->data[1], pkt->data[2], pkt->data[3], pkt->data[4], pkt->data[5],  pkt->data[6]);

    } else if(sid == EX_HEALTH_REP) {

        uint16_t size = 1;

        cnv32_8( HAL_sys_GetTick(), &pkt->data[1]);
        size += 4;

        //reset sourc assert source + line placeholder.
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

        uint8_t paketoma_buffer = 0x0000;
        uint8_t tempA = 0;
        uint8_t tempB = 0;
        uint8_t tempC = 0;
        uint8_t tempD = 0;

        /*| deployment_status (2bits) | EPS_safety_battery_mode (3bits) | EPS_safety_temperature_mode (3bits) |*/
        EPS_deployment_status deployment_status = EPS_check_deployment_status();
        tempA = ( ((uint8_t) deployment_status) << 6 );
        tempB = ((uint8_t)(eps_board_state.EPS_safety_battery_mode ))<<4;
        tempC = ((uint8_t)(eps_board_state.EPS_safety_temperature_mode ));
        paketoma_buffer =  tempA | tempB;
        pkt->data[size] = (paketoma_buffer  | tempC );
		size += 1;


        /*| SU power_switch (2bits) | OBC power_switch (2bits) | ADCS power_switch (2bits) | COMM power_switch (2bits) |*/
        paketoma_buffer = 0x0000;
        tempA = (uint8_t)(EPS_get_rail_switch_status(SU) )<<6;
        tempB = (uint8_t)(EPS_get_rail_switch_status(OBC) )<<4;
        tempC = (uint8_t)(EPS_get_rail_switch_status(ADCS) )<<2;
        tempD = (uint8_t)(EPS_get_rail_switch_status(COMM) );
        paketoma_buffer =  tempA | tempB;
        paketoma_buffer =  paketoma_buffer | tempC;
        pkt->data[size] = (paketoma_buffer  | tempD );
        size += 1;

        /* Temp sensor power_switch (2bits) | 6bits padding*/
        pkt->data[size] = (uint8_t)(EPS_get_rail_switch_status(TEMP_SENSOR) );
        size += 1;

    	/* soft error status*/
    	pkt->data[size] = (uint8_t)(error_status);
        size += 1;

        pkt->len = size;
    } else if(sid == EPS_FLS_REP) {

    	uint16_t size = 1;
    	uint32_t memory_read_value;

    	/* LIMIT_BATTERY_LOW*/
    	EPS_get_memory_word( LIMIT_BATTERY_LOW_ADDRESS, &memory_read_value );
    	cnv32_8( memory_read_value, &pkt->data[size]);
    	size += 4;

    	/* LIMIT_BATTERY_HIGH*/
    	EPS_get_memory_word( LIMIT_BATTERY_HIGH_ADDRESS, &memory_read_value );
    	cnv32_8( memory_read_value, &pkt->data[size]);
    	size += 4;

    	/* LIMIT_BATTERY_CRITICAL*/
    	EPS_get_memory_word( LIMIT_BATTERY_CRITICAL_ADDRESS, &memory_read_value );
    	cnv32_8( memory_read_value, &pkt->data[size]);
    	size += 4;

    	/* LIMIT_BATTERY_TEMPERATURE_LOW*/
    	EPS_get_memory_word( LIMIT_BATTERY_TEMPERATURE_LOW_ADDRESS, &memory_read_value );
    	cnv32_8( memory_read_value, &pkt->data[size]);
    	size += 4;

    	/* LIMIT_BATTERY_TEMPERATURE_HIGH*/
    	EPS_get_memory_word( LIMIT_BATTERY_TEMPERATURE_HIGH_ADDRESS, &memory_read_value );
    	cnv32_8( memory_read_value, &pkt->data[size]);
    	size += 4;


    	pkt->len = size;
    }

    } else if(sid == ECSS_STATS_REP) {

        uint16_t size = ecss_stats_hk(&pkt->data[1]);

        pkt->len = size + 1;
    }

    return SATR_OK;
}
