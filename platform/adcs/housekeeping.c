#include "housekeeping.h"

#include "adcs_sensors.h"
#include "adcs_actuators.h"
#include "sgp4.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {
   
   return SATR_ERROR;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {

	pkt->data[0] = (HK_struct_id) sid;

	if (sid == EX_HEALTH_REP) {

		cnv32_8(HAL_sys_GetTick(), &pkt->data[1]);
		cnv16_8(adcs_sensors.lsm9ds0_sensor.gyr_raw[0], &pkt->data[5]);
		cnv16_8(adcs_sensors.lsm9ds0_sensor.gyr_raw[1], &pkt->data[7]);
		cnv16_8(adcs_sensors.lsm9ds0_sensor.gyr_raw[2], &pkt->data[9]);
		cnv16_8(adcs_sensors.lsm9ds0_sensor.xm_raw[0], &pkt->data[11]);
		cnv16_8(adcs_sensors.lsm9ds0_sensor.xm_raw[1], &pkt->data[13]);
		cnv16_8(adcs_sensors.lsm9ds0_sensor.xm_raw[2], &pkt->data[17]);
		cnv32_8(adcs_sensors.magn_sensor.rm_raw[0], &pkt->data[19]);
		cnv32_8(adcs_sensors.magn_sensor.rm_raw[1], &pkt->data[23]);
		cnv32_8(adcs_sensors.magn_sensor.rm_raw[2], &pkt->data[27]);
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[0], &pkt->data[29]);
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[1], &pkt->data[31]);
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[2], &pkt->data[33]);
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[3], &pkt->data[35]);
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[4], &pkt->data[37]);
		cnv16_8((int16_t)adcs_actuator.spin_torquer.m_RPM, &pkt->data[39]);
		pkt->len = 40;
	} else if (sid == SU_SCI_HDR_REP) {
		cnv16_8((int16_t)(40.0 / 0.01), &pkt->data[1]); // Roll in deg
		cnv16_8((int16_t)(-45.0 / 0.01), &pkt->data[3]); // Pitch in deg
		cnv16_8((int16_t)(90.0 / 0.01), &pkt->data[5]); // Yaw in deg
		cnv16_8((int16_t)(1.0 / 0.001), &pkt->data[7]); // Roll Dot in deg/sec
		cnv16_8((int16_t)(-2.0 / 0.001), &pkt->data[9]); // Pitch Dot in deg/sec
		cnv16_8((int16_t)(3.0 / 0.001), &pkt->data[11]); // Yaw Dot in deg/sec
		cnv16_8((int16_t)(p_eci.x / 0.5), &pkt->data[13]); // X ECI in km
		cnv16_8((int16_t)(p_eci.y / 0.5), &pkt->data[15]); // Y ECI in km
		cnv16_8((int16_t)(p_eci.z / 0.5), &pkt->data[17]); // Z ECI in km
		pkt->len = 19;
//	} else if (sid == ADCS_TLE_REP) {
//		for (uint8_t i = 1; i < TLE_SIZE + 1; i++) {
//			tle_string[i] = &pkt->data[i];
//		}
	}

	return SATR_OK;
}
