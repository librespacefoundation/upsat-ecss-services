#include "housekeeping.h"

#include "adcs_sensors.h"
#include "adcs_actuators.h"
#include "sgp4.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid,
		uint8_t *data) {
	return SATR_ERROR;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {

	pkt->data[0] = (HK_struct_id) sid;

	if (sid == EX_HEALTH_REP) {
		//cnv.cnv32 = time.now();
		cnv32_8(HAL_sys_GetTick(), &pkt->data[1]);
		cnvF_8(adcs_sensors.lsm9ds0_sensor.gyr[0], &pkt->data[5]);
		cnvF_8(adcs_sensors.lsm9ds0_sensor.gyr[1], &pkt->data[9]);
		cnvF_8(adcs_sensors.lsm9ds0_sensor.gyr[2], &pkt->data[13]);
		cnvF_8(adcs_sensors.magn_sensor.rm_mag[0], &pkt->data[17]);
		cnvF_8(adcs_sensors.magn_sensor.rm_mag[1], &pkt->data[21]);
		cnvF_8(adcs_sensors.magn_sensor.rm_mag[1], &pkt->data[25]);
		cnvF_8(adcs_sensors.sun_sensor.v_sun[0], &pkt->data[29]);
		cnvF_8(adcs_sensors.sun_sensor.v_sun[1], &pkt->data[33]);
		cnvF_8(adcs_sensors.sun_sensor.v_sun[2], &pkt->data[37]);
		cnvF_8(adcs_sensors.sun_sensor.v_sun[3], &pkt->data[41]);
		cnvF_8(adcs_sensors.sun_sensor.v_sun[4], &pkt->data[45]);
		cnvF_8(adcs_sensors.sun_sensor.long_sun, &pkt->data[49]);
		cnvF_8(adcs_sensors.sun_sensor.lat_sun, &pkt->data[53]);
		cnv32_8(adcs_actuator.spin_torquer.m_RPM, &pkt->data[57]);
		pkt->len = 61;
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
