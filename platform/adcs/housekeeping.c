#include "housekeeping.h"

#include "adcs_sensors.h"
#include "adcs_actuators.h"
#include "adcs_common.h"
#include "adcs_flash.h"
#include "sgp4.h"
#include "WahbaRotM.h"
#include "adcs_error_handler.h"
#include "gps.h"

#undef __FILE_ID__
#define __FILE_ID__ 28

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {
   
   return SATR_ERROR;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {

	pkt->data[0] = (HK_struct_id) sid;

	if (sid == EX_HEALTH_REP) {

		uint16_t size = 1;
        /* System specific */
		cnv32_8(HAL_sys_GetTick(), &pkt->data[size]);
		size += 4;
		cnv32_8(adcs_boot_cnt, &pkt->data[size]);
		size += 4;
        pkt->data[size] = trasmit_error_status;
        size += 1;
        /* Sensors */
		cnv16_8(adcs_sensors.imu.gyr_raw[0], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.imu.gyr_raw[1], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.imu.gyr_raw[2], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.imu.xm_raw[0], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.imu.xm_raw[1], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.imu.xm_raw[2], &pkt->data[size]);
		size += 2;
		cnv32_8(adcs_sensors.mgn.rm_raw[0], &pkt->data[size]);
		size += 4;
		cnv32_8(adcs_sensors.mgn.rm_raw[1], &pkt->data[size]);
		size += 4;
		cnv32_8(adcs_sensors.mgn.rm_raw[2], &pkt->data[size]);
		size += 4;
		cnv16_8(adcs_sensors.sun.v_sun_raw[0], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun.v_sun_raw[1], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun.v_sun_raw[2], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun.v_sun_raw[3], &pkt->data[size]);
		size += 2;
		cnv16_8(adcs_sensors.sun.v_sun_raw[4], &pkt->data[size]);
		size += 2;
        /* Actuators */
		cnv16_8((int16_t)adcs_actuator.spin_torquer.m_RPM, &pkt->data[size]);
		size += 2;
        cnv32_8(adcs_actuator.magneto_torquer.current_y, &pkt->data[size]);
        size += 4;
        cnv32_8(adcs_actuator.magneto_torquer.current_z, &pkt->data[size]);
        size += 4;
        /* SU hearder */
		cnv16_8((int16_t)(WahbaRot.Euler[0]*RAD2DEG / 0.01), &pkt->data[size]); // Roll in deg
		size += 2;
		cnv16_8((int16_t)(WahbaRot.Euler[1]*RAD2DEG / 0.01), &pkt->data[size]); // Pitch in deg
		size += 2;
		cnv16_8((int16_t)(WahbaRot.Euler[2]*RAD2DEG / 0.01), &pkt->data[size]); // Yaw in deg
		size += 2;
		cnv16_8((int16_t)(WahbaRot.W[0]*RAD2DEG / 0.001), &pkt->data[size]); // Roll Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(WahbaRot.W[1]*RAD2DEG / 0.001), &pkt->data[size]); // Pitch Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(WahbaRot.W[2]*RAD2DEG / 0.001), &pkt->data[size]); // Yaw Dot in deg/sec
		size += 2;
		cnv16_8((int16_t)(p_eci.x / 0.5), &pkt->data[size]); // X ECI in km
		size += 2;
		cnv16_8((int16_t)(p_eci.y / 0.5), &pkt->data[size]); // Y ECI in km
		size += 2;
		cnv16_8((int16_t)(p_eci.z / 0.5), &pkt->data[size]); // Z ECI in km
		size += 2;
        /* GPS status */
        pkt->data[size] = gps.d3fix;
        size += 1;
        pkt->data[size] = gps.num_sat;
        size += 1;
        cnv16_8(gps.gps_week, &pkt->data[size]);
        size += 2;
        cnv32_8((uint32_t)gps.gps_time, &pkt->data[size]);
        size += 4;

		pkt->len = size;

	} else if (sid == SU_SCI_HDR_REP) {

        uint16_t size = 1;

        cnv16_8((int16_t)(WahbaRot.Euler[0] / 0.01), &pkt->data[size]); // Roll in deg
        size += 2;
        cnv16_8((int16_t)(WahbaRot.Euler[1] / 0.01), &pkt->data[size]); // Pitch in deg
        size += 2;
        cnv16_8((int16_t)(WahbaRot.Euler[2]/ 0.01), &pkt->data[size]); // Yaw in deg
        size += 2;
        cnv16_8((int16_t)(WahbaRot.W[0]*RAD2DEG / 0.001), &pkt->data[size]); // Roll Dot in deg/sec
        size += 2;
        cnv16_8((int16_t)(WahbaRot.W[1]*RAD2DEG / 0.001), &pkt->data[size]); // Pitch Dot in deg/sec
        size += 2;
        cnv16_8((int16_t)(WahbaRot.W[2]*RAD2DEG / 0.001), &pkt->data[size]); // Yaw Dot in deg/sec
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

        cnv32_8((int32_t)(upsat_tle.argp/0.01), &pkt->data[size]);
        size += 4;
        cnv32_8((int32_t)(upsat_tle.ascn/0.01), &pkt->data[size]);
        size += 4;
        cnv32_8((int32_t)(upsat_tle.bstar*1.0e12), &pkt->data[size]);
        size += 4;
        cnv32_8((int32_t)(upsat_tle.ecc*1.0e6), &pkt->data[size]);
        size += 4;
        cnv32_8((uint32_t)(upsat_tle.ep_day*1.0e4), &pkt->data[size]);
        size += 4;
        cnv32_8((int32_t)(upsat_tle.eqinc/0.01), &pkt->data[size]);
        size += 4;
        cnv32_8((int32_t)(upsat_tle.mnan/0.01), &pkt->data[size]);
        size += 4;
        cnv32_8((uint32_t)(upsat_tle.rev/0.1), &pkt->data[size]);
        size += 4;
        cnv16_8((uint16_t)upsat_tle.satno, &pkt->data[size]);
        size += 2;
        cnv16_8((uint16_t)upsat_tle.ep_year, &pkt->data[size]);
        size += 2;
        cnv32_8((uint32_t)upsat_tle.norb, &pkt->data[size]);
        size += 4;

        pkt->len = size;
	}

	return SATR_OK;
}
