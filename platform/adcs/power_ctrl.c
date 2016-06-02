#include "power_ctrl.h"


#undef __FILE_ID__
#define __FILE_ID__ 11

/*Must use real pins*/
SAT_returnState power_control_api(FM_dev_id did, FM_fun_id fid, uint8_t *state) {

    if(!C_ASSERT(did == ADCS_SD_DEV_ID || did == ADCS_SENSORS || did == ADCS_GPS || did == ADCS_MAGNETO || did == ADCS_SPIN || did == SYS_DBG) == true)                             { return SATR_ERROR; }
    if(!C_ASSERT(fid == P_OFF || fid == P_ON || fid == P_RESET || fid == SET_VAL) == true)    { return SATR_ERROR; }

    if(did == ADCS_SD_DEV_ID && fid == P_ON)         { HAL_adcs_SD_ON(); }
    else if(did == ADCS_SD_DEV_ID && fid == P_OFF)   { HAL_adcs_SD_OFF(); }
    else if(did == ADCS_SD_DEV_ID && fid == P_RESET) {
        HAL_adcs_SD_OFF();
        HAL_sys_delay(60);
        HAL_adcs_SD_ON();
    }
    else if(did == ADCS_SENSORS && fid == P_ON)    { HAL_adcs_SENSORS_ON(); }
    else if(did == ADCS_SENSORS && fid == P_OFF)   { HAL_adcs_SENSORS_OFF(); }

    else if(did == ADCS_GPS && fid == P_ON)    { HAL_adcs_GPS_ON(); }
    else if(did == ADCS_GPS && fid == P_OFF)   { HAL_adcs_GPS_OFF(); }

    else if(did == ADCS_SPIN && fid == SET_VAL)    {
        int32_t rpm = 0;
        cnv8_32(state, &rpm);
        HAL_adcs_SPIN(rpm); 
    }
    else if(did == ADCS_MAGNETO && fid == SET_VAL)    {
        int32_t current_x = 0;
        int32_t current_y = 0;

        cnv8_32(state, &current_x);
        cnv8_32(&state[4], &current_y);
        HAL_adcs_MAGNETO (current_x, current_y);
    }
    else if(did == SYS_DBG && fid == SET_VAL)    {

        HAL_adcs_DBG(state[0], state[1]);
    }

    return SATR_OK;
}
