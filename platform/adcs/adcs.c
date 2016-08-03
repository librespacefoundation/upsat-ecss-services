#include "adcs.h"
#include "queue.h"

#include "adcs_actuators.h"
#include "adcs_switch.h"
#include "adcs_gps.h"
#include <stdlib.h>

uint8_t dbg_msg;

#undef __FILE_ID__
#define __FILE_ID__ 29

const uint8_t services_verification_ADCS_TC[MAX_SERVICES][MAX_SUBTYPES] = {
   /* 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_VERIFICATION_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0 }, /*TC_HOUSEKEEPING_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_FUNCTION_MANAGEMENT_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_SCHEDULING_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 }, /*TC_LARGE_DATA_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_MASS_STORAGE_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_TEST_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

struct _adcs_data adcs_data;
static struct _sys_data sys_data;

SAT_returnState route_pkt(tc_tm_pkt *pkt) {

    SAT_returnState res;
    TC_TM_app_id id;

    if (!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) {
        return SATR_ERROR;
    }
    if (!C_ASSERT(pkt->type == TC || pkt->type == TM) == true) {
        return SATR_ERROR;
    }
    if (!C_ASSERT(
            pkt->app_id < LAST_APP_ID && pkt->dest_id < LAST_APP_ID) == true) {
        return SATR_ERROR;
    }

    if (pkt->type == TC) {
        id = pkt->app_id;
    } else if (pkt->type == TM) {
        id = pkt->dest_id;
    }

    if (id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
        res = hk_app(pkt);
    } else if (id == SYSTEM_APP_ID
            && pkt->ser_type == TC_FUNCTION_MANAGEMENT_SERVICE) {
        res = function_management_app(pkt);
    } else if (id == SYSTEM_APP_ID && pkt->ser_type == TC_TEST_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 1 || pkt->ser_subtype == 2 || pkt->ser_subtype == 9 || pkt->ser_subtype == 11 || pkt->ser_subtype == 12 || pkt->ser_subtype == 13) { free_pkt(pkt); return SATR_ERROR; }
        res = test_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_TIME_MANAGEMENT_SERVICE) {
        //TODO: ADD C_ASSERT
        res = time_management_app(pkt);
    } else if (id == EPS_APP_ID) {
        queuePush(pkt, OBC_APP_ID);
    } else if (id == OBC_APP_ID) {
        queuePush(pkt, OBC_APP_ID);
    } else if (id == COMMS_APP_ID) {
        queuePush(pkt, OBC_APP_ID);
    } else if (id == GND_APP_ID) {
        queuePush(pkt, OBC_APP_ID);
    } else if (id == DBG_APP_ID) {
        queuePush(pkt, OBC_APP_ID);
    }

    return SATR_OK;
}

SAT_returnState event_log(uint8_t *buf, const uint16_t size) {
    return SATR_OK;
}

SAT_returnState check_timeouts() {
    return SATR_OK;
}

void set_reset_source(const uint8_t rsrc) {
    sys_data.rsrc = rsrc;
}

void get_reset_source(uint8_t *rsrc) {
    *rsrc = sys_data.rsrc;
}

void HAL_adcs_GPS_ON() {
    adcs_pwr_switch(SWITCH_ON, GPS);
    gps_state.status = GPS_UNLOCK;
    struct time_utc gps_utc_on_gnd;
    get_time_UTC(&gps_utc_on_gnd);
    gps_state.status = HAL_SetAlarm_GPS_LOCK(gps_utc_on_gnd, GPS_ALARM_UNLOCK);
}

void HAL_adcs_GPS_OFF() {
    adcs_pwr_switch(SWITCH_OFF, GPS);
}

void HAL_adcs_SENSORS_ON() {
    adcs_pwr_switch(SWITCH_ON, SENSORS);
}

void HAL_adcs_SENSORS_OFF() {
    adcs_pwr_switch(SWITCH_OFF, SENSORS);
}

void HAL_adcs_SPIN(int32_t RPM) {
    if (abs(RPM) > MAX_RPM) {
        return;
    }
    adcs_actuator.spin_torquer.RPM = RPM;
    adcs_actuator.spin_torquer.rampTime = 0;
    update_spin_torquer(&adcs_actuator);
}

void HAL_adcs_MAGNETO(int8_t current_z, int8_t current_y) {
    if (abs(current_z) > MAX_CURR_MAGNETO_TORQUER
            || abs(current_y) > MAX_CURR_MAGNETO_TORQUER) {
        return;
    }
    adcs_actuator.magneto_torquer.current_z = current_z;
    adcs_actuator.magneto_torquer.current_y = current_y;
    update_magneto_torquer(&adcs_actuator);
}

void HAL_adcs_DBG(uint8_t var, uint8_t val) {
    dbg_msg = val;
}
