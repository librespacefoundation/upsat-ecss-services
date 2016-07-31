#include "power_ctrl.h"

#undef __FILE_ID__
#define __FILE_ID__ 23

/*Must use real pins*/
SAT_returnState power_control_api(FM_dev_id did, FM_fun_id fid, uint8_t *state) {

    if(!C_ASSERT(did == COMMS_WOD_PAT ||
                 did == SYS_DBG ) == true)   { return SATR_ERROR; }

    if(!C_ASSERT(fid == P_OFF ||
                 fid == P_ON ||
                 fid == P_RESET ||
                 fid == SET_VAL) == true)    { return SATR_ERROR; }

    if (did == COMMS_WOD_PAT && fid == SET_VAL) {

        pattern = state[0];
    }
    else if(did == SYS_DBG && fid == SET_VAL)    {
      //FIXME
    }

    return SATR_OK;
}
