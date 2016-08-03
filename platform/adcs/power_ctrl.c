#include "power_ctrl.h"
#include "adcs.h"
#include "service_utilities.h"

#include "sysview.h"

#include "sgp4.h"
#include "adcs_control.h"
#include "adcs_error_handler.h"

#undef __FILE_ID__
#define __FILE_ID__ 27

/*Must use real pins*/
SAT_returnState power_control_api(FM_dev_id did, FM_fun_id fid, uint8_t *state) {

    if(!C_ASSERT(did == ADCS_SD_DEV_ID ||
                 did == ADCS_SENSORS ||
                 did == ADCS_GPS ||
                 did == ADCS_MAGNETO ||
                 did == ADCS_SPIN ||
                 did == ADCS_TLE ||
                 did == ADCS_SET_POINT ||
                 did == ADCS_CTRL_GAIN ||
                 did == SYS_DBG) == true) { return SATR_ERROR; }
    if(!C_ASSERT(fid == P_OFF ||
                 fid == P_ON ||
                 fid == P_RESET ||
                 fid == SET_VAL) == true) { return SATR_ERROR; }

    if(did == ADCS_SD_DEV_ID && fid == P_ON)         { HAL_adcs_SD_ON(); }
    else if(did == ADCS_SD_DEV_ID && fid == P_OFF)   { HAL_adcs_SD_OFF(); }
    else if(did == ADCS_SD_DEV_ID && fid == P_RESET) {
        HAL_adcs_SD_OFF();
        HAL_sys_delay(60);
        HAL_adcs_SD_ON();
    }
    else if(did == ADCS_SENSORS && fid == P_ON) {
        HAL_adcs_SENSORS_ON();
        SYSVIEW_PRINT("SET SENSORS ON");
    }
    else if(did == ADCS_SENSORS && fid == P_OFF) {
        HAL_adcs_SENSORS_OFF();
        SYSVIEW_PRINT("SET SENSORS OFF");
    }

    else if(did == ADCS_GPS && fid == P_ON) {
        HAL_adcs_GPS_ON();
        SYSVIEW_PRINT("SET GPS ON");
    }
    else if(did == ADCS_GPS && fid == P_OFF) {
        HAL_adcs_GPS_OFF();
        SYSVIEW_PRINT("SET GPS OFF");
    }

    else if(did == ADCS_SPIN && fid == SET_VAL)    {
        SYSVIEW_PRINT("SET SPIN");
        int32_t rpm = 0;
        cnv8_32(state, &rpm);
        HAL_adcs_SPIN(rpm); 
    }
    else if(did == ADCS_MAGNETO && fid == SET_VAL)    {
        SYSVIEW_PRINT("SET MAGNETO");
        int8_t current_x = 0;
        int8_t current_y = 0;

        current_x = state[0];
        current_y = state[1];
        HAL_adcs_MAGNETO (current_x, current_y);
    }
    else if (did == ADCS_TLE && fid == SET_VAL) {
        SYSVIEW_PRINT("SET TLE");
        for (uint8_t i = 1; i < TLE_SIZE + 1; i++) {
            tle_string[i] = state[i];
        }
        temp_tle = read_tle(tle_string);
        if ( update_tle(&upsat_tle, temp_tle) == TLE_NORMAL) {
            if (flash_write_tle(&upsat_tle) == FLASH_ERROR) {
                error_status = ERROR_FLASH;
            }
        } else {
            error_status = ERROR_TLE;
        }
    }
    else if (did == ADCS_CTRL_GAIN && fid == SET_VAL) {
        cnv8_16(&state[0], &control.gain[0]);
        cnv8_16(&state[2], &control.gain[1]);
        cnv8_16(&state[4], &control.gain[2]);
        SYSVIEW_PRINT("SET GAINS");
    }
    else if(did == ADCS_SET_POINT && fid == SET_VAL) {
        cnv8_16(&state[0], &control.set_point[0]);
        cnv8_16(&state[2], &control.set_point[1]);
        cnv8_16(&state[4], &control.set_point[2]);
        SYSVIEW_PRINT("SET POINT");
    }
    else if(did == SYS_DBG && fid == SET_VAL)    {
        HAL_adcs_DBG(state[0], state[1]);
    }

    return SATR_OK;
}
