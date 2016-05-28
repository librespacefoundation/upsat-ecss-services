#include "adcs.h"
#include "adcs_control.h"
#include "adcs_state.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

const uint8_t services_verification_ADCS_TC[MAX_SERVICES][MAX_SUBTYPES] =
  {
  /*    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 }, /*TC_VERIFICATION_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,
	0 }, /*TC_HOUSEKEEPING_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 }, /*TC_FUNCTION_MANAGEMENT_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 }, /*TC_SCHEDULING_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0 }, /*TC_LARGE_DATA_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 }, /*TC_MASS_STORAGE_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 }, /*TC_TEST_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0 } };

struct _adcs_data adcs_data;
static struct _sys_data sys_data;

SAT_returnState
route_pkt (tc_tm_pkt *pkt)
{

  SAT_returnState res;
  TC_TM_app_id id;

  if (!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) {
    verification_app (pkt);
    free_pkt (pkt);
    return SATR_ERROR;
  }
  if (!C_ASSERT(pkt->type == TC || pkt->type == TM) == true) {
    verification_app (pkt);
    free_pkt (pkt);
    return SATR_ERROR;
  }
  if (!C_ASSERT(pkt->app_id < LAST_APP_ID && pkt->dest_id < LAST_APP_ID) == true) {
    verification_app (pkt);
    free_pkt (pkt);
    return SATR_ERROR;
  }

  if (pkt->type == TC) {
    id = pkt->app_id;
  }
  else if (pkt->type == TM) {
    id = pkt->dest_id;
  }

  if (id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
    //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
    res = hk_app (pkt);
  }
  else if (id == SYSTEM_APP_ID
      && pkt->ser_type == TC_FUNCTION_MANAGEMENT_SERVICE) {
    res = function_management_app (pkt);
  }
  else if (id == SYSTEM_APP_ID && pkt->ser_type == TC_TEST_SERVICE) {
    //C_ASSERT(pkt->ser_subtype == 1 || pkt->ser_subtype == 2 || pkt->ser_subtype == 9 || pkt->ser_subtype == 11 || pkt->ser_subtype == 12 || pkt->ser_subtype == 13) { free_pkt(pkt); return SATR_ERROR; }
    res = test_app (pkt);
  }
  else if (id == EPS_APP_ID) {
    export_pkt (OBC_APP_ID, pkt, &adcs_data.obc_uart);
  }
  else if (id == ADCS_APP_ID) {
    export_pkt (OBC_APP_ID, pkt, &adcs_data.obc_uart);
  }
  else if (id == COMMS_APP_ID) {
    export_pkt (OBC_APP_ID, pkt, &adcs_data.obc_uart);
  }
  else if (id == IAC_APP_ID) {
    export_pkt (OBC_APP_ID, pkt, &adcs_data.obc_uart);
  }
  else if (id == GND_APP_ID) {
    export_pkt (OBC_APP_ID, pkt, &adcs_data.obc_uart);
  }
  else if (id == DBG_APP_ID) {
    export_pkt (OBC_APP_ID, pkt, &adcs_data.obc_uart);
  }

  verification_app (pkt);
  free_pkt (pkt);
  return SATR_OK;
}

SAT_returnState
event_log (uint8_t *buf, const uint16_t size)
{
  return SATR_OK;
}

SAT_returnState
check_timeouts ()
{

}

SAT_returnState
time_management_app (tc_tm_pkt *pkt)
{
  return SATR_ERROR;
}

void
set_reset_source (const uint8_t rsrc)
{
  sys_data.rsrc = rsrc;
}

void
get_reset_source (uint8_t *rsrc)
{
  *rsrc = sys_data.rsrc;
}

void
HAL_adcs_GPS_ON ()
{
  adcs_switch (SWITCH_ON, GPS, &adcs_state);
}

void
HAL_adcs_GPS_OFF ()
{
  adcs_switch (SWITCH_OFF, GPS, &adcs_state);
}

void
HAL_adcs_SENSORS_ON ()
{
  adcs_switch (SWITCH_ON, SENSORS, &adcs_state);
}

void
HAL_adcs_SENSORS_OFF ()
{
  adcs_switch (SWITCH_OFF, SENSORS, &adcs_state);
}

void
HAL_adcs_SPIN (int32_t RPM)
{
  if (abs (RPM) > MAX_RPM) {
    return;
  }
  adcs_actuator.RPM = RPM;
  adcs_actuator.rampTime = 0;
  update_spin_torquer (&adcs_actuator);
}

void
HAL_adcs_MAGNETO (int32_t current_x, int32_t current_y)
{
  if (abs (current_x) > MAX_CURR_MAGNETO_TORQUER
      || abs (current_y) > MAX_CURR_MAGNETO_TORQUER) {
    return;
  }
  adcs_actuator.current_x = current_x;
  adcs_actuator.current_y = current_y;
  update_magneto_torquer (&adcs_actuator);
}

