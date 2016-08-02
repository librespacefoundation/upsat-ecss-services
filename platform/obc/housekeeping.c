#include "housekeeping.h"

#include "obc.h"
#include "time_management_service.h"
#include "wdg.h"
#include "mass_storage_service.h"
#include "su_mnlp.h"
#include "stm32f4xx_hal.h"
#include "obc_hal.h"
#include "uart_hal.h"

#undef __FILE_ID__
#define __FILE_ID__ 16

extern UART_HandleTypeDef huart3;

extern void HAL_sys_delay(uint32_t sec);

extern SAT_returnState hk_crt_pkt_TC(tc_tm_pkt *pkt, TC_TM_app_id app_id, HK_struct_id sid);
extern SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, TC_TM_app_id app_id, HK_struct_id sid);

extern void get_time_QB50(uint32_t *qb);
extern SAT_returnState wod_log();
extern SAT_returnState wod_log_load(uint8_t *buf);

struct _sat_ext_status {
    uint32_t comms_sys_epoch;

    uint32_t comms_sys_time;
    uint32_t adcs_sys_time;
    uint32_t eps_sys_time;
    uint32_t obc_sys_time;

    float adcs_gyro[3];
    float adcs_rm_mag[3];
    float adcs_vsun[5];
    float adcs_long_sun;
    float adcs_lat_sun;
    int32_t adcs_m_RPM; 

    uint32_t comms_tx_state;

    uint8_t eps_batterypack_health_status;
    uint8_t eps_heaters_status;

    uint16_t eps_top_voltage;
    uint16_t eps_top_current;

    uint8_t eps_top_pwm_duty_cycle;

    uint16_t eps_bottom_voltage;
    uint16_t eps_bottom_current;

    uint8_t eps_bottom_pwm_duty_cycle;

    uint16_t eps_left_voltage;
    uint16_t eps_left_current;

    uint8_t eps_left_pwm_duty_cycle;

    uint16_t eps_right_voltage;
    uint16_t eps_right_current;

    uint8_t eps_right_pwm_duty_cycle;

    uint8_t eps_deployment_status;

    uint8_t eps_safety_battery_mode;

    uint8_t eps_safety_temperature_mode;

};

static struct _sat_ext_status sat_ext_status;
struct _sat_status sat_status;

static tc_tm_pkt hk_pkt;
static uint8_t hk_pkt_data[264];

void hk_INIT() {
   hk_pkt.data = hk_pkt_data;
}

void hk_SCH() {
    
    hk_crt_pkt_TC(&hk_pkt, EPS_APP_ID, HEALTH_REP);
    route_pkt(&hk_pkt);
    wake_uart_task();
    HAL_sys_delay(1000);

    hk_crt_pkt_TC(&hk_pkt, COMMS_APP_ID, HEALTH_REP);
    route_pkt(&hk_pkt);
    wake_uart_task();

    wdg.hk_valid = true;
    HAL_sys_delay(14000);

    wdg.hk_valid = true;
    HAL_sys_delay(14000);

    wod_log();
    clear_wod();
    hk_crt_pkt_TM(&hk_pkt, COMMS_APP_ID, WOD_REP);
    route_pkt(&hk_pkt);
    wake_uart_task();
    HAL_sys_delay(1000);


    hk_crt_pkt_TC(&hk_pkt, EPS_APP_ID, EX_HEALTH_REP);
    route_pkt(&hk_pkt);
    wake_uart_task();
    HAL_sys_delay(1000);

    hk_crt_pkt_TC(&hk_pkt, COMMS_APP_ID, EX_HEALTH_REP);
    route_pkt(&hk_pkt);
    wake_uart_task();
    HAL_sys_delay(1000);

    hk_crt_pkt_TC(&hk_pkt, ADCS_APP_ID, EX_HEALTH_REP);
    route_pkt(&hk_pkt);
    wake_uart_task();
    HAL_sys_delay(1000);

    // wdg.hk_valid = true;
    //HAL_sys_delay(12500);

    // wdg.hk_valid = true;
    HAL_sys_delay(26000);  

    //hk_crt_pkt_TM(&hk_pkt, GND_APP_ID, EXT_WOD_REP);
    hk_crt_pkt_TM(&hk_pkt, DBG_APP_ID, EXT_WOD_REP);
    route_pkt(&hk_pkt);
    wake_uart_task();
    // clear_ext_wod();
    HAL_sys_delay(1000);
}

void clear_wod() {
    sat_status.batt_curr = 0;
    sat_status.batt_volt = 0;
    sat_status.bus_3v3_curr = 0;
    sat_status.bus_5v_curr = 0;
    sat_status.temp_eps = 0;
    sat_status.temp_batt = 0;
    sat_status.temp_comms = 0;
}

void clear_ext_wod() {
    sat_ext_status.comms_sys_epoch = 0;

    sat_ext_status.comms_sys_time = 0;
    sat_ext_status.adcs_sys_time = 0;
    sat_ext_status.eps_sys_time = 0;
    sat_ext_status.obc_sys_time = 0;

    sat_ext_status.adcs_gyro[0] = 0;
    sat_ext_status.adcs_gyro[1] = 0;
    sat_ext_status.adcs_gyro[2] = 0;
    sat_ext_status.adcs_rm_mag[0] = 0;
    sat_ext_status.adcs_rm_mag[1] = 0;
    sat_ext_status.adcs_rm_mag[2] = 0;
    sat_ext_status.adcs_vsun[0] = 0;
    sat_ext_status.adcs_vsun[1] = 0;
    sat_ext_status.adcs_vsun[2] = 0;
    sat_ext_status.adcs_vsun[3] = 0;
    sat_ext_status.adcs_vsun[4] = 0;
    sat_ext_status.adcs_long_sun = 0;
    sat_ext_status.adcs_lat_sun = 0;
    sat_ext_status.adcs_m_RPM = 0; 

    sat_ext_status.comms_tx_state = 0;
}

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data, uint8_t len) {
    
    if(app_id == EPS_APP_ID && sid == HEALTH_REP) {
        sat_status.batt_volt = data[1];
        sat_status.batt_curr = data[2];
        sat_status.bus_3v3_curr = data[3];
        sat_status.bus_5v_curr = data[4];
        sat_status.temp_batt = data[5];
        sat_status.temp_eps = data[6];

    } else if(app_id == COMMS_APP_ID && sid == HEALTH_REP) {
        sat_status.temp_comms = data[1];
    } else if(app_id == ADCS_APP_ID && sid == EX_HEALTH_REP) {

        cnv8_32(&data[1], &sat_ext_status.adcs_sys_time);
        //cnv8_F(&data[1], &sat_ext_status.adcs_gyro[0]);
        //cnv8_F(&data[5], &sat_ext_status.adcs_gyro[0]);
        //cnv8_F(&data[9], &sat_ext_status.adcs_gyro[1]);
        //cnv8_F(&data[13], &sat_ext_status.adcs_gyro[2]);
        //cnv8_F(&data[17], &sat_ext_status.adcs_rm_mag[0]);
        //cnv8_F(&data[21], &sat_ext_status.adcs_rm_mag[1]);
        //cnv8_F(&data[25], &sat_ext_status.adcs_rm_mag[2]);
        //cnv8_F(&data[29], &sat_ext_status.adcs_vsun[0]);
        //cnv8_F(&data[33], &sat_ext_status.adcs_vsun[1]);
        //cnv8_F(&data[37], &sat_ext_status.adcs_vsun[2]);
        //cnv8_F(&data[41], &sat_ext_status.adcs_vsun[3]);
        //cnv8_F(&data[45], &sat_ext_status.adcs_vsun[4]);
        //cnv8_F(&data[49], &sat_ext_status.adcs_long_sun);
        //cnv8_F(&data[53], &sat_ext_status.adcs_lat_sun);
        //cnv8_32(&data[57], &sat_ext_status.adcs_m_RPM);

    } else if(app_id == EPS_APP_ID && sid == EX_HEALTH_REP) {

        cnv8_32(&data[1], &sat_ext_status.eps_sys_time);
        // pkt->data[5] = (uint8_t)(eps_board_state.batterypack_health_status);

        // /* heater status*/
        // EPS_switch_control_status heaters_status = EPS_get_control_switch_status(BATTERY_HEATERS);
        // pkt->data[6] = (uint8_t)heaters_status;


        // /*power module top*/
        // cnv16_8( power_module_top.voltage, &pkt->data[7]);
        // cnv16_8( power_module_top.current, &pkt->data[9]);
        // pkt->data[11] = (uint8_t)power_module_top.pwm_duty_cycle;

        // /*power module bottom*/
        // cnv16_8( power_module_bottom.voltage, &pkt->data[12]);
        // cnv16_8( power_module_bottom.current, &pkt->data[14]);
        // pkt->data[16] = (uint8_t)power_module_bottom.pwm_duty_cycle;

        // /*power module left*/
        // cnv16_8( power_module_left.voltage, &pkt->data[17]);
        // cnv16_8( power_module_left.current, &pkt->data[19]);
        // pkt->data[21] = (uint8_t)power_module_left.pwm_duty_cycle;

        // /*power module right*/
        // cnv16_8( power_module_right.voltage, &pkt->data[22]);
        // cnv16_8( power_module_right.current, &pkt->data[24]);
        // pkt->data[26] = (uint8_t)power_module_right.pwm_duty_cycle;

        // /* deployment status*/
        // EPS_deployment_status deployment_status = EPS_check_deployment_status();
        // pkt->data[27] = (uint8_t)deployment_status;

        // /* battery voltage safety */
        // pkt->data[28] = (uint8_t)(eps_board_state.EPS_safety_battery_mode );

        // /* battery voltage safety */
        // pkt->data[29] = (uint8_t)(eps_board_state.EPS_safety_temperature_mode );
    } else if(app_id == COMMS_APP_ID && sid == EX_HEALTH_REP) {
        cnv8_32(&data[1], &sat_ext_status.comms_sys_time);
        //cnv8_32(&data[5], &sat_ext_status.comms_tx_state);
    } else {
        return SATR_ERROR; // this should change to inv pkt
    }
    
    return SATR_OK;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {
    
    pkt->data[0] = (HK_struct_id)sid;
    
    if(sid == EX_HEALTH_REP) {

        struct time_utc temp_time;

        get_time_UTC(&temp_time);

        uint16_t size = 1;
 
        cnv32_8( HAL_sys_GetTick(), &pkt->data[size]);
        size += 4;
        
        //pkt->data[5] = temp_time.day;
        //pkt->data[6] = temp_time.month;
        //pkt->data[7] = temp_time.year;
        
        //pkt->data[8] = temp_time.hour;
        //pkt->data[9] = temp_time.min;
        //pkt->data[10] = temp_time.sec;

        //cnv32_8(task_times.uart_time, &pkt->data[11]);
        //cnv32_8(task_times.idle_time, &pkt->data[15]);
        //cnv32_8(task_times.hk_time, &pkt->data[19]);
        //cnv32_8(task_times.su_time, &pkt->data[23]);
        //cnv32_8(task_times.sch_time, &pkt->data[27]);

        pkt->data[size] = *MNLP_data.su_nmlp_script_scheduler_active;
        size += 1;
        
        pkt->data[size] = *MNLP_data.su_service_sheduler_active;
        size += 1;
        
        pkt->len = size;

    } else if(sid == WOD_REP) {

        uint32_t time_temp;
        get_time_QB50(&time_temp);

        cnv32_8(time_temp, &pkt->data[1]);
        wod_log_load(&pkt->data[5]);

        uint16_t size = 4+(32*7);
        mass_storage_storeFile(WOD_LOG, 0, &pkt->data[1], &size);
        pkt->len = 1+4+(32*7);
    } else if(sid == EXT_WOD_REP) {

        uint16_t size = 1;
        get_time_QB50(&sat_ext_status.comms_sys_epoch);
        sat_ext_status.obc_sys_time = HAL_sys_GetTick();

        cnv32_8(sat_ext_status.comms_sys_epoch, &pkt->data[size]);
        size += 4;
        cnv32_8(sat_ext_status.obc_sys_time, &pkt->data[size]);
        size += 4;
        cnv32_8(sat_ext_status.comms_sys_time, &pkt->data[size]);
        size += 4;
        cnv32_8(sat_ext_status.eps_sys_time, &pkt->data[size]);
        size += 4;
        cnv32_8(sat_ext_status.adcs_sys_time, &pkt->data[size]);
        size += 4;

        cnv32_8(task_times.uart_time, &pkt->data[size]);
        size += 4;
        cnv32_8(task_times.idle_time, &pkt->data[size]);
        size += 4;
        cnv32_8(task_times.hk_time, &pkt->data[size]);
        size += 4;
        cnv32_8(task_times.su_time, &pkt->data[size]);
        size += 4;
        cnv32_8(task_times.sch_time, &pkt->data[size]);
        size += 4;

        cnv32_8(obc_data.vbat, &pkt->data[size]);
        size += 2;
        
        pkt->data[size] = *MNLP_data.su_nmlp_script_scheduler_active;
        size += 1;
        
        pkt->data[size] = *MNLP_data.su_service_sheduler_active;
        size += 1;
        //pkt->data[size] = (uint8_t)HAL_UART_GetState(&huart3);
        //size++;
        // pkt->data[5] = (uint8_t)(eps_board_state.batterypack_health_status);

        // /* heater status*/
        // EPS_switch_control_status heaters_status = EPS_get_control_switch_status(BATTERY_HEATERS);
        // pkt->data[6] = (uint8_t)heaters_status;


        // /*power module top*/
        // cnv16_8( power_module_top.voltage, &pkt->data[7]);
        // cnv16_8( power_module_top.current, &pkt->data[9]);
        // pkt->data[11] = (uint8_t)power_module_top.pwm_duty_cycle;

        // /*power module bottom*/
        // cnv16_8( power_module_bottom.voltage, &pkt->data[12]);
        // cnv16_8( power_module_bottom.current, &pkt->data[14]);
        // pkt->data[16] = (uint8_t)power_module_bottom.pwm_duty_cycle;

        // /*power module left*/
        // cnv16_8( power_module_left.voltage, &pkt->data[17]);
        // cnv16_8( power_module_left.current, &pkt->data[19]);
        // pkt->data[21] = (uint8_t)power_module_left.pwm_duty_cycle;

        // /*power module right*/
        // cnv16_8( power_module_right.voltage, &pkt->data[22]);
        // cnv16_8( power_module_right.current, &pkt->data[24]);
        // pkt->data[26] = (uint8_t)power_module_right.pwm_duty_cycle;

        // /* deployment status*/
        // EPS_deployment_status deployment_status = EPS_check_deployment_status();
        // pkt->data[27] = (uint8_t)deployment_status;

        // /* battery voltage safety */
        // pkt->data[28] = (uint8_t)(eps_board_state.EPS_safety_battery_mode );

        // /* battery voltage safety */
        // pkt->data[29] = (uint8_t)(eps_board_state.EPS_safety_temperature_mode );

        // cnvF_8(sat_ext_status.adcs_gyro[0], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_gyro[1], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_gyro[2], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_rm_mag[0], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_rm_mag[1], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_rm_mag[2], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_vsun[0], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_vsun[1], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_vsun[2], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_vsun[3], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_vsun[4], &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_long_sun, &pkt->data[size]);
        // size += 4;
        // cnvF_8(sat_ext_status.adcs_lat_sun, &pkt->data[size]);
        // size += 4;
        // cnv32_8(sat_ext_status.adcs_m_RPM, &pkt->data[size]);
        // size += 4;

        // cnv32_8(sat_ext_status.comms_tx_state, &pkt->data[size]);
        // size += 4;
        
        //mass_storage_storeFile(EXT_WOD_LOG, 0, &pkt->data[1], &size);
        pkt->len = size;
    }

    return SATR_OK;
}
