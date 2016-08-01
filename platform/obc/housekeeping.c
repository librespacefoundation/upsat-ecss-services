#include "housekeeping.h"

#include "housekeeping_service.h"

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

static uint8_t ext_wod_buffer[SUB_SYS_EXT_WOD_SIZE];

struct _sat_ext_status {
    uint32_t comms_sys_time;
    uint32_t eps_sys_time;
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

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data, uint8_t len) {
    
    if(!C_ASSERT(data != NULL) == true) { return SATR_ERROR; }

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
        
        if(!C_ASSERT(len != ADCS_EXT_WOD_SIZE - 1) == true) { return SATR_ERROR; }
        
        for(uint8_t i = 0; i < SCI_REP_SIZE; i++) {
            su_sci_header[i + SCI_ARR_OFFSET] = data[i + ADCS_EXT_SCI_OFFSET]; 
        }
        memcpy(&ext_wod_buffer[ADCS_EXT_WOD_OFFSET], &data[1], ADCS_EXT_WOD_SIZE);
    
    } else if(app_id == EPS_APP_ID && sid == EX_HEALTH_REP) {
      
        if(!C_ASSERT(len != EPS_EXT_WOD_SIZE - 1) == true) { return SATR_ERROR; }

        cnv8_32(&data[1], &sat_ext_status.eps_sys_time);
        memcpy(&ext_wod_buffer[EPS_EXT_WOD_OFFSET], &data[1], EPS_EXT_WOD_SIZE);
    
    } else if(app_id == COMMS_APP_ID && sid == EX_HEALTH_REP) {
    
        if(!C_ASSERT(len != COMMS_EXT_WOD_SIZE - 1) == true) { return SATR_ERROR; }

        cnv8_32(&data[1], &sat_ext_status.comms_sys_time);
        memcpy(&ext_wod_buffer[COMMS_EXT_WOD_OFFSET], &data[1], COMMS_EXT_WOD_SIZE);
    
    } else {
        return SATR_ERROR; // this should change to inv pkt
    }
    
    return SATR_OK;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {
    
    pkt->data[0] = (HK_struct_id)sid;
    
    if(sid == EX_HEALTH_REP) {

        uint16_t size = 1;

        uint32_t sys_epoch = 0;
        uint8_t rsrc = 0;
        uint32_t boot_counter = 0;

        get_time_QB50(&sys_epoch);

        get_reset_source(&rsrc);
        get_boot_counter(&boot_counter);

        cnv32_8( HAL_sys_GetTick(), &pkt->data[size]);
        size += 4;

        cnv32_8( sys_epoch, &pkt->data[size]);
        size += 4;

        pkt->data[size] = rsrc;
        size += 1;

        cnv32_8( boot_counter, &pkt->data[size]);
        size += 4;

        //Boot Counter comms
        size += 2;
        //Boot Counter eps
        size += 2;

        pkt->data[size] = assertion_last_file;
        size += 1;
        cnv16_8(assertion_last_line,&pkt->data[size]);
        size += 2;

        cnv16_8(obc_data.vbat, &pkt->data[size]);
        size += 2;

        cnv32_8(task_times.uart_time, &pkt->data[size]);
        size += 4;
        cnv16_8((uint16_t)task_times.idle_time, &pkt->data[size]);
        size += 2;
        cnv16_8((uint16_t)task_times.hk_time, &pkt->data[size]);
        size += 2;
        cnv16_8((uint16_t)task_times.su_time, &pkt->data[size]);
        size += 2;
        cnv16_8((uint16_t)task_times.sch_time, &pkt->data[size]);
        size += 2;

        //add su,sch data
        pkt->data[size] = MS_data.last_err;
        size += 1;

        pkt->data[size] = MS_data.enabled;
        size += 1;

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

        uint32_t sys_epoch = 0;
        uint8_t rsrc = 0;
        uint32_t boot_counter = 0;

        get_time_QB50(&sys_epoch);

        get_reset_source(&rsrc);
        get_boot_counter(&boot_counter);

        cnv32_8( HAL_sys_GetTick(), &pkt->data[size]);
        size += 4;

        cnv32_8( sys_epoch, &pkt->data[size]);
        size += 4;

        pkt->data[size] = rsrc;
        size += 1;

        cnv32_8( boot_counter, &pkt->data[size]);
        size += 4;

        //Boot Counter comms
        size += 2;
        //Boot Counter eps
        size += 2;

        pkt->data[size] = assertion_last_file;
        size += 1;
        cnv16_8(assertion_last_line,&pkt->data[size]);
        size += 2;

        cnv16_8(obc_data.vbat, &pkt->data[size]);
        size += 2;

        cnv32_8(task_times.uart_time, &pkt->data[size]);
        size += 4;
        cnv16_8((uint16_t)task_times.idle_time, &pkt->data[size]);
        size += 2;
        cnv16_8((uint16_t)task_times.hk_time, &pkt->data[size]);
        size += 2;
        cnv16_8((uint16_t)task_times.su_time, &pkt->data[size]);
        size += 2;
        cnv16_8((uint16_t)task_times.sch_time, &pkt->data[size]);
        size += 2;

        pkt->data[size] = MS_data.last_err;
        size += 1;

        pkt->data[size] = MS_data.enabled;
        size += 1;

        //add su,sch data
        pkt->data[size] = *MNLP_data.su_nmlp_script_scheduler_active;
        size += 1;

        pkt->data[size] = *MNLP_data.su_service_sheduler_active;
        size += 1;
 
        memcpy( &pkt->data[size], ext_wod_buffer, SUB_SYS_EXT_WOD_SIZE);

        pkt->len = size;
    }

    return SATR_OK;
}
