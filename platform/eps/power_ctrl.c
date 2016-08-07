#include "power_ctrl.h"
#include "eps_non_volatile_mem_handling.h"

#undef __FILE_ID__
#define __FILE_ID__ 19

/*Must use real pins*/
SAT_returnState power_control_api(FM_dev_id did, FM_fun_id fid, uint8_t *state) {

    if(!C_ASSERT(did == OBC_DEV_ID || did == ADCS_DEV_ID || did == COMMS_DEV_ID || did == SU_DEV_ID || did == EPS_WRITE_FLS ) == true)                             { return SATR_ERROR; }
    if(!C_ASSERT(fid == P_OFF || fid == P_ON || fid == P_RESET || fid == SET_VAL) == true)    { return SATR_ERROR; }

    if(did == OBC_DEV_ID && fid == P_ON)         { HAL_eps_OBC_ON(); }
    else if(did == OBC_DEV_ID && fid == P_OFF)   { HAL_eps_OBC_OFF(); }
    else if(did == OBC_DEV_ID && fid == P_RESET) {
        HAL_eps_OBC_OFF();
        HAL_sys_delay(60);
        HAL_eps_OBC_ON();
    } else if(did == ADCS_DEV_ID && fid == P_ON)  { HAL_eps_ADCS_ON(); }
    else if(did == ADCS_DEV_ID && fid == P_OFF)   { HAL_eps_ADCS_OFF(); }
    else if(did == ADCS_DEV_ID && fid == P_RESET) {
        HAL_eps_ADCS_OFF();
        HAL_sys_delay(60);
        HAL_eps_ADCS_ON();
    } else if(did == COMMS_DEV_ID && fid == P_ON)  { HAL_eps_COMMS_ON(); }
    else if(did == COMMS_DEV_ID && fid == P_OFF)   { HAL_eps_COMMS_OFF(); }
    else if(did == COMMS_DEV_ID && fid == P_RESET) {
        HAL_eps_COMMS_OFF();
        HAL_sys_delay(60);
        HAL_eps_COMMS_ON();
    } else if(did == SU_DEV_ID && fid == P_ON)  { HAL_eps_SU_ON(); }
    else if(did == SU_DEV_ID && fid == P_OFF)   { HAL_eps_SU_OFF(); }
    else if(did == SU_DEV_ID && fid == P_RESET) {
        HAL_eps_SU_OFF();
        HAL_sys_delay(60);
        HAL_eps_SU_ON();
    }else if(did == EPS_WRITE_FLS && fid == SET_VAL) {

    	uint32_t memory_write_value;
    	uint32_t memory_write_address;

        cnv8_32(state, &memory_write_address);
        cnv8_32(&state[4], &memory_write_value);

		if (memory_write_address == LIMIT_BATTERY_LOW_ADDRESS
		        || memory_write_address == LIMIT_BATTERY_HIGH_ADDRESS
		        || memory_write_address == LIMIT_BATTERY_CRITICAL_ADDRESS
		        || memory_write_address == LIMIT_BATTERY_TEMPERATURE_LOW_ADDRESS
		        || memory_write_address == LIMIT_BATTERY_TEMPERATURE_HIGH_ADDRESS) {

			/*sanity check in ranges*/
			if (memory_write_address == LIMIT_BATTERY_LOW_ADDRESS){
				if( (memory_write_value>=LIMIT_BATTERY_LOW_MAX)||(memory_write_value<=LIMIT_BATTERY_LOW_MIN)){
					memory_write_value = LIMIT_BATTERY_VOLTAGE_LOW_DEFAULT;
				}
			}

			if (memory_write_address == LIMIT_BATTERY_HIGH_ADDRESS){
				if( (memory_write_value>=LIMIT_BATTERY_HIGH_MAX)||(memory_write_value<=LIMIT_BATTERY_HIGH_MIN)){
					memory_write_value = LIMIT_BATTERY_VOLTAGE_HIGH_DEFAULT;
				}
			}

			if (memory_write_address == LIMIT_BATTERY_CRITICAL_ADDRESS){
				if( (memory_write_value>=LIMIT_BATTERY_CRITICAL_MAX)||(memory_write_value<=LIMIT_BATTERY_CRITICAL_MIN)){
					memory_write_value = LIMIT_BATTERY_VOLTAGE_CRITICAL_DEFAULT;
				}
			}

			if (memory_write_address == LIMIT_BATTERY_TEMPERATURE_LOW_ADDRESS){
				if( (memory_write_value>=LIMIT_BATTERY_TEMPERATURE_LOW__MAX)||(memory_write_value<=LIMIT_BATTERY_TEMPERATURE_LOW__MIN)){
					memory_write_value = LIMIT_BATTERY_TEMPERATURE_LOW_DEFAULT;
				}
			}

			if (memory_write_address == LIMIT_BATTERY_TEMPERATURE_HIGH_ADDRESS){
				if( (memory_write_value>=LIMIT_BATTERY_TEMPERATURE_HIGH__MAX)||(memory_write_value<=LIMIT_BATTERY_TEMPERATURE_HIGH__MIN)){
					memory_write_value = LIMIT_BATTERY_TEMPERATURE_HIGH_DEFAULT;
				}
			}

			EPS_set_memory_word(memory_write_address, &memory_write_value);
		}
    }

    return SATR_OK;
}
