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

		uint16_t size = 1;

		cnv32_8(HAL_sys_GetTick(), &pkt->data[size]);
		size += 4;
		cnv16_8(adcs_sensors.lsm9ds0_sensor.gyr_raw[0], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.lsm9ds0_sensor.gyr_raw[1], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.lsm9ds0_sensor.gyr_raw[2], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.lsm9ds0_sensor.xm_raw[0], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.lsm9ds0_sensor.xm_raw[1], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.lsm9ds0_sensor.xm_raw[2], &pkt->data[size]);
		size += 2;
		cnv32_8(adcs_sensors.magn_sensor.rm_raw[0], &pkt->data[size]);
		size += 4;
		cnv32_8(adcs_sensors.magn_sensor.rm_raw[1], &pkt->data[size]);
		size += 4;
		cnv32_8(adcs_sensors.magn_sensor.rm_raw[2], &pkt->data[size]);
		size += 4;
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[0], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[1], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[2], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[3], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun_sensor.v_sun_raw[4], &pkt->data[size]);
		size += 2;
		cnv16_8((int16_t)adcs_actuator.spin_torquer.m_RPM, &pkt->data[size]);
		size += 2;

		cnv16_8((int16_t)(40.0 / 0.01), &pkt->data[size]); // Roll in deg
		size += 2;
		cnv16_8((int16_t)(-45.0 / 0.01), &pkt->data[size]); // Pitch in deg
		size += 2;
		cnv16_8((int16_t)(90.0 / 0.01), &pkt->data[size]); // Yaw in deg
		size += 2;
		cnv16_8((int16_t)(1.0 / 0.001), &pkt->data[size]); // Roll Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(-2.0 / 0.001), &pkt->data[size]); // Pitch Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(3.0 / 0.001), &pkt->data[size]); // Yaw Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(p_eci.x / 0.5), &pkt->data[size]); // X ECI in km
		size += 2;
		cnv16_8((int16_t)(p_eci.y / 0.5), &pkt->data[size]); // Y ECI in km
		size += 2;
		cnv16_8((int16_t)(p_eci.z / 0.5), &pkt->data[size]); // Z ECI in km
		size += 2;

		pkt->len = size;

	} else if (sid == SU_SCI_HDR_REP) {

		uint16_t size = 1;

		cnv16_8((int16_t)(40.0 / 0.01), &pkt->data[size]); // Roll in deg
		size += 2;
		cnv16_8((int16_t)(-45.0 / 0.01), &pkt->data[size]); // Pitch in deg
		size += 2;
		cnv16_8((int16_t)(90.0 / 0.01), &pkt->data[size]); // Yaw in deg
		size += 2;
		cnv16_8((int16_t)(1.0 / 0.001), &pkt->data[size]); // Roll Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(-2.0 / 0.001), &pkt->data[size]); // Pitch Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(3.0 / 0.001), &pkt->data[size]); // Yaw Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(p_eci.x / 0.5), &pkt->data[size]); // X ECI in km
		size += 2;
		cnv16_8((int16_t)(p_eci.y / 0.5), &pkt->data[size]); // Y ECI in km
		size += 2;
		cnv16_8((int16_t)(p_eci.z / 0.5), &pkt->data[size]); // Z ECI in km
		size += 2;

		pkt->len = size;

	} else if (sid == ADCS_TLE_REP) {

		uint16_t size = 1;

		pkt->len = size;

	}

	return SATR_OK;
}
