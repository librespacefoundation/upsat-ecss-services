#include "obc.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

const uint8_t services_verification_OBC_TC[MAX_SERVICES][MAX_SUBTYPES] = { 
/*    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 */
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
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_MASS_STORAGE_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_TEST_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

struct _obc_data obc_data;
struct _sat_status sat_status;
struct _wdg_state wdg = { .hk_valid = false, .uart_valid = false };
static struct _sys_data sys_data;

SAT_returnState route_pkt(tc_tm_pkt *pkt) {

    SAT_returnState res;
    TC_TM_app_id id;

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)                         { verification_app(pkt); free_pkt(pkt); return SATR_ERROR; }
    if(!C_ASSERT(pkt->type == TC || pkt->type == TM) == true)                       { verification_app(pkt); free_pkt(pkt); return SATR_ERROR; }
    if(!C_ASSERT(pkt->app_id < LAST_APP_ID && pkt->dest_id < LAST_APP_ID) == true)  { verification_app(pkt); free_pkt(pkt); return SATR_ERROR; }

    if(pkt->type == TC)         { id = pkt->app_id; } 
    else if(pkt->type == TM)    { id = pkt->dest_id; }

    if(id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
        res = event_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_EVENT_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
        res = hk_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_FUNCTION_MANAGEMENT_SERVICE) {
        res = function_management_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_MASS_STORAGE_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 1 || pkt->ser_subtype == 2 || pkt->ser_subtype == 9 || pkt->ser_subtype == 11 || pkt->ser_subtype == 12 || pkt->ser_subtype == 13) { free_pkt(pkt); return SATR_ERROR; }
        res = mass_storage_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_TEST_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 1 || pkt->ser_subtype == 2 || pkt->ser_subtype == 9 || pkt->ser_subtype == 11 || pkt->ser_subtype == 12 || pkt->ser_subtype == 13) { free_pkt(pkt); return SATR_ERROR; }
        res = test_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_SCHEDULING_SERVICE) {
        //TODO: ADD C_ASSERT
        res = scheduling_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_SU_MNLP_SERVICE) {
        //TODO: ADD C_ASSERT
        res = su_nmlp_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_TIME_MANAGEMENT_SERVICE) {
        //TODO: ADD C_ASSERT
        res = time_management_app(pkt);
    }
    else if(id == EPS_APP_ID)      { export_pkt(EPS_APP_ID, pkt, &obc_data.eps_uart); }
    else if(id == ADCS_APP_ID)     { export_pkt(ADCS_APP_ID, pkt, &obc_data.adcs_uart); }
    else if(id == COMMS_APP_ID)    { export_pkt(COMMS_APP_ID, pkt, &obc_data.comms_uart); }
    else if(id == IAC_APP_ID)      { export_pkt(DBG_APP_ID, pkt, &obc_data.dbg_uart); }
    else if(id == GND_APP_ID)      { export_pkt(COMMS_APP_ID, pkt, &obc_data.comms_uart); }
    else if(id == DBG_APP_ID)      { export_pkt(DBG_APP_ID, pkt, &obc_data.dbg_uart); }

    verification_app(pkt);
    free_pkt(pkt);
    return SATR_OK;
}

SAT_returnState obc_INIT() {

    pkt_pool_INIT();
    HAL_obc_enableBkUpAccess();
    bkup_sram_INIT();

    HAL_reset_source(&sys_data.rsrc);
    update_boot_counter();

    uint32_t cnt = 0;
    get_boot_counter(&cnt);
    event_boot(sys_data.rsrc, cnt);

    HAL_obc_SD_ON();
   
    mass_storage_init();

    su_INIT();

    scheduling_init_service();
    return SATR_OK;
}

void bkup_sram_INIT() {

    obc_data.log_cnt = HAL_obc_BKPSRAM_BASE();
    obc_data.log_state = HAL_obc_BKPSRAM_BASE() + 1;
    sys_data.boot_counter = HAL_obc_BKPSRAM_BASE() + 2;
    obc_data.wod_cnt = HAL_obc_BKPSRAM_BASE() + 3;
    obc_data.file_id_su = HAL_obc_BKPSRAM_BASE() + 4;
    obc_data.file_id_wod = HAL_obc_BKPSRAM_BASE() + 5;
    obc_data.file_id_ext = HAL_obc_BKPSRAM_BASE() + 6;
    obc_data.file_id_ev = HAL_obc_BKPSRAM_BASE() + 7;
    obc_data.file_id_fotos = HAL_obc_BKPSRAM_BASE() + 8;
    
//    *obc_data.log_cnt = 0;
//    *obc_data.log_state = 0;
//    *sys_data.boot_counter = 0;
//    *obc_data.wod_cnt = 0;
//    *obc_data.file_id_su = 0;
//    *obc_data.file_id_wod = 0;
//    *obc_data.file_id_ext =0;
//    *obc_data.file_id_ev = 0;
//    *obc_data.file_id_fotos =0;
            
    obc_data.log = (uint8_t *)HAL_obc_BKPSRAM_BASE() + 9;
    obc_data.wod_log = (uint8_t *)HAL_obc_BKPSRAM_BASE() + 10 + (EV_MAX_BUFFER);
    
    if(!C_ASSERT(*obc_data.log_cnt < EV_MAX_BUFFER) == true)      { *obc_data.log_cnt = 0; }
    if(!C_ASSERT(*obc_data.wod_cnt < EV_MAX_BUFFER) == true)      { *obc_data.wod_cnt = 0; }
    if(!C_ASSERT(*obc_data.file_id_su < MS_MAX_FILES) == true)    { *obc_data.file_id_su = 0; }
    if(!C_ASSERT(*obc_data.file_id_wod < MS_MAX_FILES) == true)   { *obc_data.file_id_wod = 0; }
    if(!C_ASSERT(*obc_data.file_id_ext < MS_MAX_FILES) == true)   { *obc_data.file_id_ext = 0; }
    if(!C_ASSERT(*obc_data.file_id_ev < MS_MAX_FILES) == true)    { *obc_data.file_id_ev = 0; }
    if(!C_ASSERT(*obc_data.file_id_fotos < MS_MAX_FILES) == true) { *obc_data.file_id_fotos = 0; }

}

uint32_t get_new_fileId(MS_sid sid) {

    if(!C_ASSERT(sid == SU_LOG || sid == WOD_LOG || sid == EXT_WOD_LOG || sid == EVENT_LOG || sid == FOTOS) == true) { return 0; }

    if(sid == SU_LOG) {
        (*obc_data.file_id_su)++;
        if(*obc_data.file_id_su > MAX_FILE_NUM) {
            *obc_data.file_id_su = 1;
        }
        return *obc_data.file_id_su;

    } else if(sid == WOD_LOG) {
        (*obc_data.file_id_wod)++;
        if(*obc_data.file_id_wod > MAX_FILE_NUM) {
            *obc_data.file_id_wod = 1;
        }
        return *obc_data.file_id_wod;

    } else if(sid == EXT_WOD_LOG) {
        (*obc_data.file_id_ext)++;
        if(*obc_data.file_id_ext > MAX_FILE_NUM) {
            *obc_data.file_id_ext = 1;
        }
        return *obc_data.file_id_ext;

    } else if(sid == EVENT_LOG) {
        (*obc_data.file_id_ev)++;
        if(*obc_data.file_id_ev > MAX_FILE_NUM) {
            *obc_data.file_id_ev = 1;
        }
        return *obc_data.file_id_ev;

    } else if(sid == FOTOS) {   //need to change this
        (*obc_data.file_id_fotos)++;
        if(*obc_data.file_id_fotos > MAX_FILE_NUM) {
            *obc_data.file_id_fotos = 1;
        }
        return *obc_data.file_id_fotos;
    }
}

SAT_returnState event_log(uint8_t *buf, const uint16_t size) {
  
    uint32_t tmp_time;

    union _cnv cnv;

    get_time_QB50(&tmp_time);

    cnv.cnv32 = tmp_time;

    for(uint16_t i = 0; i < 4; i++) {
        obc_data.log[*obc_data.log_cnt] = cnv.cnv8[i];
        if(++(*obc_data.log_cnt) >= EV_MAX_BUFFER) { *obc_data.log_cnt = 0; }
    }

    for(uint16_t i = 0; i < size; i++) {
        obc_data.log[*obc_data.log_cnt] = buf[i];
        (*obc_data.log_cnt)++;
        if(*obc_data.log_cnt >= EV_MAX_BUFFER) { *obc_data.log_cnt = 0; }
    }

    return SATR_OK;
}

SAT_returnState event_log_load(uint8_t *buf, const uint16_t pointer, const uint16_t size) {
   
   if(!C_ASSERT(size < EV_MAX_BUFFER) == true) { return SATR_ERROR; }

   for(uint16_t i = 0; i < size; i++) {
        buf[i] = obc_data.log[i];
   }
   return SATR_OK;
}

    uint16_t cnt = 0;
    uint8_t buf[TEST_ARRAY];

SAT_returnState event_log_IDLE() {

    if(!C_ASSERT(*obc_data.log_state < LAST_EV_STATE) == true) { *obc_data.log_state = EV_P0; }

    int16_t len = *obc_data.log_cnt - (*obc_data.log_state * EV_BUFFER_PART);
    if(len < 0) { len = EV_MAX_BUFFER - len; }
    if(len > EV_BUFFER_PART) {

        uint16_t size = EV_BUFFER_PART;

        mass_storage_storeFile(EVENT_LOG, 0, &obc_data.log[*obc_data.log_state * EV_BUFFER_PART], &size);
        if(++(*obc_data.log_state) > EV_P4) { *obc_data.log_state = EV_P0; }

    } 
    
     return SATR_OK;
}

SAT_returnState wod_log() {

//check endianess
    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.mode;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.batt_volt;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.batt_curr;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.bus_3v3_curr;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }
    
    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.bus_5v_curr;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.temp_comms;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.temp_eps;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.temp_batt;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    return SATR_OK;
}

SAT_returnState wod_log_load(uint8_t *buf) {

   uint8_t rev_wod_cnt = *obc_data.wod_cnt;
   if(rev_wod_cnt == 0) { rev_wod_cnt = WOD_MAX_BUFFER; }

   uint16_t buf_cnt = 0;
   for(uint16_t i = 0; i < WOD_MAX_BUFFER; i++) {

        rev_wod_cnt--;
        buf[buf_cnt++] = obc_data.wod_log[rev_wod_cnt]; 
        if(rev_wod_cnt == 0) { rev_wod_cnt = WOD_MAX_BUFFER; }
   }
   return SATR_OK;
}

SAT_returnState check_subsystems_timeouts() {
    
    uint32_t sys_t_now = HAL_sys_GetTick();
    
    if( (sys_t_now - obc_data.adcs_uart.last_com_time) >= TIMEOUT_V_ADCS ) { 
        /*Handle ADCS subsystem's timeout*/
        tc_tm_pkt *tmp_pkt = 0;

        SAT_returnState res = function_management_pctrl_crt_pkt_api(&tmp_pkt, EPS_APP_ID, P_RESET, ADCS_DEV_ID);
        if(res == SATR_OK) { 
            route_pkt(tmp_pkt); 
            obc_data.adcs_uart.last_com_time = sys_t_now;
        }
        else { free_pkt(tmp_pkt); }
    }
    
    if( (sys_t_now - obc_data.comms_uart.last_com_time) >= TIMEOUT_V_COMMS ) { 
        /*Handle COMMS subsystem's timeout*/
        tc_tm_pkt *tmp_pkt = 0;

        SAT_returnState res = function_management_pctrl_crt_pkt_api(&tmp_pkt, EPS_APP_ID, P_RESET, COMMS_DEV_ID);
        if(res == SATR_OK) { 
            route_pkt(tmp_pkt); 
            obc_data.comms_uart.last_com_time = sys_t_now;
        }
        else { free_pkt(tmp_pkt); }
    }
    
    if( (sys_t_now - obc_data.eps_uart.last_com_time) >= TIMEOUT_V_EPS ) { 
        /*Handle EPS subsystem's timeout*/
        //here we drink it, nothing we can do.
    }

    if( (sys_t_now - obc_data.dbg_uart.last_com_time) >= TIMEOUT_V_DBG ) { 
        /*Handle UMBILICAL (dbg's port) subsystem's timeout*/
        //no need for handling
    }

    return SATR_OK;
}

void set_reset_source(const uint8_t rsrc) {
    sys_data.rsrc = rsrc;
}

void get_reset_source(uint8_t *rsrc) {
    *rsrc = sys_data.rsrc;
}

void update_boot_counter() {
    (*sys_data.boot_counter)++;
}

void get_boot_counter(uint32_t *cnt) {
    *cnt = *sys_data.boot_counter;
}
