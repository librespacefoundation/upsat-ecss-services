#include "su_mnlp.h"

#include "time_management_service.h"
#include "upsat.h"
#include "uart_hal.h"

#undef __FILE_ID__
#define __FILE_ID__ 3

extern uint8_t uart_temp[];

extern SAT_returnState function_management_pctrl_crt_pkt_api(tc_tm_pkt **pkt, TC_TM_app_id dest_id, FM_fun_id fun_id, FM_dev_id did);
extern SAT_returnState route_pkt(tc_tm_pkt *pkt);

extern SAT_returnState HAL_su_uart_rx();

extern void HAL_su_uart_tx(uint8_t *buf, uint16_t size);
extern void HAL_sys_delay(uint32_t sec);

extern SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);

extern SAT_returnState mass_storage_su_load_api(MS_sid sid, uint8_t *buf);

extern SAT_returnState su_nmlp_app( tc_tm_pkt *spacket);

mnlp_response_science_header flight_data;

//science_unit_script_inst su_scripts[1];

struct _MNLP_data MNLP_data;

/* if true all commands are routed to nmlp serial,
 * if false some commands are routed to cubesat subsystems. (obc_su_on, obc_su_off, )
 */
uint8_t mnlp_sim_active = true;

/*Current sat time in qb50 secs*/
uint32_t qb_f_time_now = 0;

uint32_t obc_day_moment_secs = 0;
struct time_utc obc_day_utc_time;

/*174 response data + 22 for obc extra header and */
uint8_t su_inc_buffer[197];//198

/*su scheduler various calling states*/
SAT_returnState tt_call_state;
SAT_returnState ss_call_state;
SAT_returnState scom_call_state;

/*points to the current start byte of a time's table begining, into the loaded script*/
uint16_t current_tt_pointer;

/*points to the current start byte of a script's sequence begining, into the loaded script*/
uint16_t current_ss_pointer;

SAT_returnState su_nmlp_app( tc_tm_pkt *spacket){
    
    uint16_t size = 0;
    science_unit_script_sequence s_seq;
    s_seq.cmd_id = spacket->data[0];
    s_seq.command[0] = spacket->data[0];
    s_seq.len = spacket->data[1]; //to tx on uart len+2
    s_seq.command[1] = spacket->data[1];
    
    for(uint8_t o=2;o<spacket->len;o++){
        s_seq.command[o] = spacket->data[o];
    }
    
    switch( spacket->ser_subtype){
        case 1: /*Power on mnlp unit*/
            if(mnlp_sim_active){ 
                HAL_su_uart_tx( s_seq.command, s_seq.len+2); }
            else{ 
                su_power_ctrl(P_ON); }
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_POWER_SET_ACTIVE_SEND", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 2: /*Power off mnlp unit*/
            if(mnlp_sim_active){ 
                HAL_su_uart_tx( s_seq.command, s_seq.len+2); }
            else{ 
                su_power_ctrl(P_OFF); }
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_POWER_SET_NON-ACTIVE_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 3: /*Power Reset mnlp unit*/
            if(mnlp_sim_active){ 
                HAL_su_uart_tx( s_seq.command, s_seq.len+2); }
            else{ 
                su_power_ctrl(P_RESET); }
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_POWER_RESET_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 4: /*su load parameters*/ 
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);  //to check it !!!
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_LOAD_PARAMETERS_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 6: /*su health check*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_HEALTH_CHECK_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 8: /*su calibrate*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_CAL(IBRATION)_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 10: /*su science*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_SCI(ENCE_DATA)_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 12: /*su housekeep*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_HK(HOUSEKEEP)_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 14: /*su stm*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_STM_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 16:/*su dump*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_DUMP_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 18: /*bias on*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_BIAS_ON_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;            
        case 19: /*bias off*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_BIAS_OFF_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;            
        case 20: /*mtee on*/
            if(mnlp_sim_active){ 
                ; }
            else{ 
                HAL_su_uart_tx( s_seq.command, s_seq.len+2); }
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_MTEE_ON_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;            
        case 21: /*mtee off*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_MTEE_OFF_SEND", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;  
        case 23:
            if(mnlp_sim_active){ 
                ; }
            else{ 
                ; }
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_SCHEDULER_STATUS_REPORT_RECEIVED", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
            case 24: /*Enable su nmlp scheduler*/
            *MNLP_data.su_nmlp_scheduler_active = (uint8_t) true;
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_SCHEDULER_SET_ACTIVE_(SEND_AND_EXECUTED)", 969,969, "", &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
        case 25: /*Disable su nmlp scheduler*/
            *MNLP_data.su_nmlp_scheduler_active = (uint8_t) false;
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_SCHEDULER_SET_NON-ACTIVE_(SEND_AND_EXECUTED)", 969,969,"" , &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            break;
    }//switch ends here
    
    return SATR_OK;
}

SAT_returnState su_incoming_rx() {

    uint16_t size = SU_LOG_SIZE;
    SAT_returnState res;
    uint8_t error_array[SU_RSP_PCKT_SIZE];
    
    res = HAL_su_uart_rx();
    if( res == SATR_EOT ) {
        
//        UTC = 7th June 2016 12:33
//        roll = 10 deg
//        pitch = 20 deg
//        yaw = 30 deg
//        rolldot = 400 milli-deg/sec
//        pitchdot = 500 milli-deg/sec
//        yawdot = 600 milli-deg/sec
//        X_ECI = 6700 km
//        Y_ECI = 6800 km
//        Z_ECI = 6900 km
        
        /*science header*/ 
        uint32_t qb_50_secs;
        get_time_QB50(&qb_50_secs);
        cnv32_8( qb_50_secs, &su_inc_buffer[0]);
//        cnv16_8(flight_data.roll, &su_inc_buffer[4]);
        int16_t roll = 10; 
        cnv16_8(roll, &su_inc_buffer[4]);
        int16_t pitch = 20;
        cnv16_8(pitch, &su_inc_buffer[6]);
        int16_t yaw = 30;
        cnv16_8(yaw, &su_inc_buffer[8]);
        int16_t rolldot = 400;
        cnv16_8(rolldot, &su_inc_buffer[10]);
        int16_t pitchdot = 500;
        cnv16_8(pitchdot, &su_inc_buffer[12]);
        int16_t yawdot = 600;
        cnv16_8(yawdot, &su_inc_buffer[14]);
        uint16_t xeci = 6700;
        cnv16_8(xeci, &su_inc_buffer[16]);
        uint16_t yeci = 6800;
        cnv16_8(yeci, &su_inc_buffer[18]);
        uint16_t zeci = 6900;
        cnv16_8(zeci, &su_inc_buffer[20]);
        
        switch( su_inc_buffer[22] )
        {
            case (uint8_t)SU_LDP_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_LDP_RECEIVED(0x05)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
            case (uint8_t)SU_HC_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_HC_RECEIVED(0x06)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
            case (uint8_t)SU_CAL_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_CAL_RSP_RECEIVED(0x07)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
            case (uint8_t)SU_SCI_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_SCI_RECEIVED(0x08)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
            case (uint8_t)SU_HK_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_HK_RECEIVED(0x09)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;    
            case (uint8_t)SU_STM_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_STM_RECEIVED(0x0A)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
            case (uint8_t)SU_DUMP_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_DUMP_RECEIVED(0x0B)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
            case (uint8_t)SU_ERR_RSP_ID: /*indicates tha nmlp is in reset state, power cycle must be done*/
//                handle_su_error();
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "SU_ERROR_RECEIVED(0xBB)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
            case (uint8_t)OBC_SU_ERR_RSP_ID:
                mass_storage_storeFile( SU_LOG, 0 ,su_inc_buffer, &size);
#if nMNLP_DEBUGGING_ACTIVE == 1
            event_crt_pkt_api(uart_temp, "OBC_SU_ERROR_RECEIVED(0xFA)", 969,969, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
            HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                break;
        }
    }
    return SATR_OK;
}

void handle_su_error(){
    
    //steps to do as described in page 42 of m-nlp-icd, issus 6v2
    //abort current running script
    //turn off su, 
    //generate OBC_SU_ERR packet
    //wait 60 secs, rerun same script from NEXT time tables, 
    //(because propably the previous command gave you an error state))
    
}

void su_INIT(){

    MNLP_data.su_state = SU_POWER_OFF;
    mnlp_sim_active = true;
    su_load_scripts();
    for (MS_sid i = SU_SCRIPT_1; i <= SU_SCRIPT_7; i++) {
        if (MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_str == true &&
            MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_logi == true) {
            /*populate only script headers that are structural OK*/    
            su_populate_header(&(MNLP_data.su_scripts[(uint8_t) i - 1].scr_header), 
                           MNLP_data.su_scripts[(uint8_t) i - 1].file_load_buf);
        }
    }
    /*sort the scripts by increasing T_STARTTIME field (?)*/
    /*Enable the script scheduler*/
    *MNLP_data.su_nmlp_scheduler_active = (uint8_t) true;
}

void su_load_scripts(){
    
    SAT_returnState res ;
    for( MS_sid i = SU_SCRIPT_1; i <= SU_SCRIPT_7; i++) {
        /*mark every script as non-valid, to check it later on memory and mark it (if it is, structurally) valid*/
        MNLP_data.su_scripts[(uint8_t) i - 1].valid_str = false;
        /*load scripts on memory but, parse them at a later time, in order to unlock access to the storage medium for other users*/
        res = mass_storage_su_load_api( i, MNLP_data.su_scripts[(uint8_t) i - 1].file_load_buf);
//        SAT_returnState res = mass_storage_su_load_api( SU_SCRIPT_1, MNLP_data.su_scripts[(uint8_t) i - 1].file_load_buf);
        if( res != SATR_OK ){ //if( res == SATR_ERROR || res == SATR_CRC_ERROR){
            /*script is kept marked as invalid because of CRC error, or invalid length, or some other error*/
#if nMNLP_DEBUGGING_ACTIVE == 1
    uint16_t size;
    event_crt_pkt_api(uart_temp, "OH! INVALID SCRIPT, Scr number is:", (uint8_t) i ,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
            continue;
        }
        else{
            /* Mark the script as both structural/logical valid, to be parsed afterwards inside the su_scheduler. */
            /* This is the first load from memory, maybe after a reset, so we assume that scripts are
             * logically valid (they are purposed to be scheduled). 
             */
            MNLP_data.su_scripts[(uint8_t) i - 1].valid_str = true;
            MNLP_data.su_scripts[(uint8_t) i - 1].valid_logi = true;
#if nMNLP_DEBUGGING_ACTIVE == 1
    uint16_t size;
    event_crt_pkt_api(uart_temp, "SCRIPT LOADED OK, NO:", (uint8_t) i ,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
        }
    }
}

void su_script_selector() {

    /*script chooser code segment*/
    for (MS_sid i = SU_SCRIPT_1; i <= SU_SCRIPT_7; i++) {
        if (MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_str == true &&
            MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_logi == true) {
            uint32_t time_diffuint32;
            int32_t time_diffint32;
            get_time_QB50(&qb_f_time_now);
            time_diffuint32 = (uint32_t) qb_f_time_now - MNLP_data.su_scripts[(uint8_t) i - 1].scr_header.start_time;
            time_diffint32 = (int32_t) time_diffuint32;

            /* if time_diffint32 is > 0(zero) means that the current checking script
             * start time has passed over us, so the closest value > 0 and closest to zero is the script to be activated
             * before others. Now, if time_diffint32 > zero AND time_diffint32 >=0 && time_diffint32 <=60 (secs)
             * mark it as active script, with a max. activation delay of 1 min.
             */
            //to become 60 secs, eg: 1 minute late max. and to sleepa after selection for 50 secs,
            //also see page 35 on mnlp icd.
            if (time_diffint32 >= 0 && time_diffint32 <= 500) {
#if nMNLP_DEBUGGING_ACTIVE == 1
                uint16_t size;
                event_crt_pkt_api(uart_temp, "SCRIPT MARKED AS RUNNING SCRIPT IS:", (uint8_t) i, 0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
                HAL_uart_tx(DBG_APP_ID, (uint8_t *) uart_temp, size);
#endif
                MNLP_data.active_script = (MS_sid) i;
                /*Save the last chosen active script to sram*/
                *MNLP_data.su_nmlp_last_active_script = (MS_sid) i;
                /*set first time table to be executed*/
                *MNLP_data.su_next_time_table = 1;
                /*set script sequence 1 (0x41) to be executed*/
                *MNLP_data.su_next_script_seq = (uint8_t) 0x41;                
                /*don't check other scripts, we have just choose one as the active script*/
                break;
            }
        }
        else{ 
#if nMNLP_DEBUGGING_ACTIVE == 1
                uint16_t size;
                event_crt_pkt_api(uart_temp, "SCRIPT SELECTION CHECK OMMITED FOR STRUCTURAL/LOGICAL REASONS:", (uint8_t) i, 0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
                HAL_uart_tx(DBG_APP_ID, (uint8_t *) uart_temp, size);
#endif
                continue;
        }
    }
#if nMNLP_DEBUGGING_ACTIVE == 1
                uint16_t size;
                event_crt_pkt_api(uart_temp, "NO NEWER SCRIPT TO SELECT, USING LAST ACTIVE FROM SRAM:", (uint8_t) 0, 0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
                HAL_uart_tx(DBG_APP_ID, (uint8_t *) uart_temp, size);
#endif
    /* in case that no newer script is eligible to be activated,
     * we run the last activated script continuously.
     */
    MNLP_data.active_script = *MNLP_data.su_nmlp_last_active_script;
}

void su_SCH(){
    
    if((*MNLP_data.su_nmlp_scheduler_active)) {
        
        /* check if the active script is number zero, if it is, we have an issue on sram regions
         * so hard-set script 1 as the running script
         */
        if(!C_ASSERT(MNLP_data.active_script != (uint8_t) 0) ) { *MNLP_data.su_nmlp_last_active_script = 1; MNLP_data.active_script = 1; }
                
        if(MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].valid_str == true &&
           MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].valid_logi == true) {

            //TODO: add a check here for the following.
            /*this script has not been executed on this day &&*/
            
            //TODO: check here on non-volatile mem that we are not executing the same script on the same day,
            //due to a reset.
            //if a reset occured when we have executed a script from start to end, then on the next boot,
            //we will execute it again, we don't want that.

            /* the first byte after the 12-byte sized script header,
             * points to the first time table, on the active_script script.
             */
            current_tt_pointer = SU_TT_OFFSET;
            /*finds the next tt that needs to be executed, it iterates all the tt to find the correct one*/
            for (uint16_t b = current_tt_pointer;
                    b < SU_MAX_FILE_SIZE; b++) { //TODO: check until when for will run

                /*if the script has been deleted/updated abort this old instance of it*/
                if (MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].valid_logi != true) {
                    break;
                }
                tt_call_state =
                        polulate_next_time_table(MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].file_load_buf,
                        &MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header,
                        &current_tt_pointer);
                if (tt_call_state == SATR_EOT) {
                    /*reached the last time table, go for another script, or this script, but on the NEXT day*/
                    //TODO: here to save on non-volatile mem that we have finished executing all ss'es od currentnt active script
                    
#if nMNLP_DEBUGGING_ACTIVE == 1
    uint16_t size;
    event_crt_pkt_api(uart_temp, "REACHED TIME TABLE END(see you tomorrow):", (uint8_t) MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.script_index  ,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                    break;
                } else if (tt_call_state == SATR_ERROR) {
                    /*non valid time table, go for next time table*/
                    continue;
                } else {
                    /*if the script has been deleted/updated abort this old instance of it*/
                    if (MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].valid_logi != true) {
                        break;
                    }
#if nMNLP_DEBUGGING_ACTIVE == 1
    uint16_t size;
    event_crt_pkt_api(uart_temp, "MOVED TO TIME TABLE:", (uint8_t) MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.script_index  ,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                /*check this day's moment time against time_table's start time, and wait for it to come.*/
                struct time_utc tt_utc;
                uint32_t tt_day_moment_secs = 0; /*script time-table moment in seconds from start of current day*/
                uint32_t secs_diff = 0;          /**/
                int32_t casted_secs_diff = 0;    /**/
                tt_utc.hour = MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.hours;
                tt_utc.min = MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.min;
                tt_utc.sec = MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.sec;
                get_time_UTC(&obc_day_utc_time);                
                cnv_utc_to_secs(&tt_utc, &tt_day_moment_secs);                    
                cnv_utc_to_secs(&obc_day_utc_time, &obc_day_moment_secs);
                secs_diff = obc_day_moment_secs - tt_day_moment_secs; 
                casted_secs_diff = (int32_t) secs_diff;
                /*the difference in here should be max. of 24 hours*/
                if( casted_secs_diff < 0 ){
                    /*time table execution is in future, sleep for casted_secs_diff-something*/
#if nMNLP_DEBUGGING_ACTIVE == 1
    uint16_t size;
    event_crt_pkt_api(uart_temp, "Time Table time in future, delaying...:", (uint8_t) 0 ,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                    //HAL_sys_delay( casted_secs_diff*(-1)); //and *1000-
                    HAL_sys_delay( 3000); //three secs
                }else if( casted_secs_diff > 0) { /*for some reason we have lost this time-table, go to next one*/
#if nMNLP_DEBUGGING_ACTIVE == 1
    uint16_t size;
    event_crt_pkt_api(uart_temp, "Time Table lost, moving to next one...:", (uint8_t) 0 ,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
//                    *MNLP_data.su_next_time_table ++;
                    continue;
                }
#if nMNLP_DEBUGGING_ACTIVE == 1
    //uint16_t size;
    event_crt_pkt_api(uart_temp, "End Of Time Table Delay:", (uint8_t) 0,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif                                 
                /*find the script sequence pointed by *time_table->script_index, and execute it */
                /*start every search after current_tt_pointer, current_tt_pointer now points to the start of the next tt_header*/
                current_ss_pointer = current_tt_pointer - 1;
                ss_call_state =
                    su_goto_script_seq(&current_ss_pointer,
                    &MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.script_index);
#if nMNLP_DEBUGGING_ACTIVE == 1
    //uint16_t size;
    event_crt_pkt_api(uart_temp, "MOVED TO SCRIPT SEQUENCE:", (uint8_t) MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.script_index ,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif
                    if (ss_call_state == SATR_OK) {
                        for (uint16_t p = 0; p < SU_MAX_FILE_SIZE; p++) { /*start executing commands in a script sequence*/

                            /*if the script has been deleted/updated abort this old instance of it*/
                            if (MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].valid_logi != true) {
                                break;
                            }
                            scom_call_state =
                                    su_next_cmd(MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].file_load_buf,
                                    &MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header,
                                    &current_ss_pointer);
#if nMNLP_DEBUGGING_ACTIVE == 1
                            //uint16_t size;
                            event_crt_pkt_api(uart_temp, "MOVED TO SCRIPT COMMAND:", (uint8_t) MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header.cmd_id, 0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
                            HAL_uart_tx(DBG_APP_ID, (uint8_t *) uart_temp, size);
#endif
                            if (scom_call_state == SATR_EOT) {
                                /*no more commands on script sequences to be executed*/
                                /*go for the next command in the same script sequence*/
                                /*reset seq_header->cmd_if field to something else other than 0xFE*/
                                MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header.cmd_id = 0x0;
                                break;
                            } else
                                if (scom_call_state == SATR_ERROR) {
                                /*an unknown command has been encountered in the script sequences, so go for the next command*/
                                continue;
                            } else {
                                /*handle script sequence command*/
#if nMNLP_DEBUGGING_ACTIVE == 1
                                //uint16_t size;
                                event_crt_pkt_api(uart_temp, "EXECUTING COMMAND:", (uint8_t) MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header.cmd_id, 0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
                                HAL_uart_tx(DBG_APP_ID, (uint8_t *) uart_temp, size);
#endif
                                su_cmd_handler(&MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header);
                            }
                        }
                    }else if(ss_call_state == SATR_EOT ){
#if nMNLP_DEBUGGING_ACTIVE == 1
    //uint16_t size;
    event_crt_pkt_api(uart_temp, "REACHED SCRIPT EOT, GO TO NEXT TIME TABLE:", 0,0, (uint8_t*) mnlp_sim_active, &size, SATR_OK);
    HAL_uart_tx(DBG_APP_ID, (uint8_t *)uart_temp, size);
#endif                        

                    }
            }//go for next time table
                
            //script handling for ends here, at this point every time table in (the current) script has been served.
            MNLP_data.su_state = SU_IDLE;
        }//script run validity check if ends here
    }else{
        /*script marked as active is not valid either logicly || structural*/
    }
}
}

SAT_returnState su_goto_script_seq(uint16_t *script_sequence_pointer, uint8_t *ss_to_go) {
    
    if( *ss_to_go == SU_SCR_TT_EOT ){
        /*if searching to go to script 0x55*/
        return SATR_EOT;
    }
    /*go until the end of the time tables entries*/
    while (MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].file_load_buf[(*script_sequence_pointer)++] !=
            /*su_scripts[(uint8_t) active_script - 1].tt_header.script_index*/ SU_SCR_TT_EOT) {
        //                            current_ss_pointer++;
    }/*now current_ss_pointer points to start of S1*/
    if( *ss_to_go == 0x41){ return SATR_OK;}
    
    /*an alternative implementation can count how many 0xFE we are passing over*/
    for (uint8_t i = 0x41; i <*ss_to_go; i++) {
        while (MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].file_load_buf[(*script_sequence_pointer)++] !=
                /*su_scripts[(uint8_t) active_script - 1].tt_header.script_index*/ SU_OBC_EOT_CMD_ID) {
            //                            current_ss_pointer++;
        }/*now current_ss_pointer points to start of S<times>*/
        /*move the pointer until end of 0xFE (SU_OBC_EOT_CMD_ID) command*/
        (*script_sequence_pointer) = (*script_sequence_pointer) + 2;
    }
    return SATR_OK;
}

SAT_returnState polulate_next_time_table( uint8_t *file_buffer, science_unit_script_time_table *time_table, uint16_t *tt_pointer) {

    if(!C_ASSERT(file_buffer != NULL && time_table != NULL && tt_pointer != NULL) == true) { return SATR_ERROR; }
    if(!C_ASSERT(time_table->script_index != SU_SCR_TT_EOT) == true) { return SATR_EOT; }
    
    time_table->sec = file_buffer[(*tt_pointer)++];
    time_table->min = file_buffer[(*tt_pointer)++];
    time_table->hours = file_buffer[(*tt_pointer)++];
    time_table->script_index = file_buffer[(*tt_pointer)++];
    
    if(!C_ASSERT(time_table->sec < 59) == true) { return SATR_ERROR; }
    if(!C_ASSERT(time_table->min < 59) == true) { return SATR_ERROR; }
    if(!C_ASSERT(time_table->hours < 23) == true) { return SATR_ERROR; }
    if(!C_ASSERT(time_table->script_index == SU_SCR_TT_S1 || \
                 time_table->script_index == SU_SCR_TT_S2 || \
                 time_table->script_index == SU_SCR_TT_S3 || \
                 time_table->script_index == SU_SCR_TT_S4 || \
                 time_table->script_index == SU_SCR_TT_S5 || \
                 time_table->script_index == SU_SCR_TT_EOT) == true) { return SATR_ERROR; }

    return SATR_OK;
}

SAT_returnState su_goto_time_table( uint16_t *tt_pointer, uint8_t *tt_to_go ){
    uint8_t tt_count = 0;
    if(*tt_to_go == 1) { *tt_pointer = SU_TT_OFFSET; return SATR_OK; } /*go to first time table*/
    
    for(uint8_t i=0;i<2048;i++){
//        MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].file_load_buf[]
    }
    return SATR_OK;
}

SAT_returnState su_cmd_handler( science_unit_script_sequence *cmd) {

    HAL_sys_delay( ((cmd->dt_min * 60) + cmd->dt_sec) * 1000);
    
//    if( cmd->cmd_id == 0x05){
//      HAL_su_uart_tx( cmd->command, cmd->len+1);
        
//        uint8_t su_out[105];
//        su_out[0]= 0x05;
//        su_out[1]= 0x63; //len
//        su_out[2]= 2; //seq_coun
//        HAL_UART_Transmit( &huart2, su_out, 102 , 10); //ver ok
//        HAL_su_uart_tx( su_out, 102);  
//    }
//    else{ HAL_su_uart_tx( cmd->command, cmd->len+2); }
//    else{
//    //-------------------------------------------------------------------------
    if( cmd->cmd_id == SU_OBC_SU_ON_CMD_ID)         { su_power_ctrl(P_ON); } 
    else if(cmd->cmd_id == SU_OBC_SU_OFF_CMD_ID)    { su_power_ctrl(P_OFF); }
    else if(cmd->cmd_id == SU_OBC_EOT_CMD_ID)       { return SATR_EOT; } 
    else{
//            if(cmd->cmd_id == 0x05) {
//                HAL_su_uart_tx( cmd->command, cmd->len+2);
//            }
            HAL_su_uart_tx( cmd->command, cmd->len+2); }
    
    return SATR_OK;
}

SAT_returnState su_populate_header( science_unit_script_header *su_script_hdr, uint8_t *buf) {

    if(!C_ASSERT(buf != NULL && su_script_hdr != NULL) == true) { return SATR_ERROR; }
    
    union _cnv cnv;
    cnv.cnv8[0] = buf[0];
    cnv.cnv8[1] = buf[1];
    su_script_hdr->script_len = cnv.cnv16[0];

    cnv.cnv8[0] = buf[2];
    cnv.cnv8[1] = buf[3];
    cnv.cnv8[2] = buf[4];
    cnv.cnv8[3] = buf[5];
    su_script_hdr->start_time = cnv.cnv32;
    
    cnv.cnv8[0] = buf[6];
    cnv.cnv8[1] = buf[7];
    cnv.cnv8[2] = buf[8];
    cnv.cnv8[3] = buf[9];
    su_script_hdr->file_sn = cnv.cnv32;

    su_script_hdr->sw_ver = 0x1F & buf[10];    
    su_script_hdr->su_id = 0x03 & (buf[10] >> 5); //need to check this, seems ok
    su_script_hdr->script_type = 0x1F & buf[11];
    su_script_hdr->su_md = 0x03 & (buf[11] >> 5); //need to check this, seems ok

    cnv.cnv8[0] = buf[su_script_hdr->script_len-2]; /*to check?*/
    cnv.cnv8[1] = buf[su_script_hdr->script_len-1]; /*to check?*/
    su_script_hdr->xsum = cnv.cnv16[0];

    return SATR_OK;
}  
                                          
SAT_returnState su_next_cmd(uint8_t *file_buffer, science_unit_script_sequence *script_sequence, uint16_t *ss_pointer) {
    
//     for(uint8_t i = 0; i < 255; i++) {
//        script_sequence->command[i] = 254;
//     }
    
    if(!C_ASSERT(file_buffer != NULL && script_sequence != NULL && ss_pointer != NULL) == true) { return SATR_ERROR; }
    if(!C_ASSERT(script_sequence->cmd_id != SU_OBC_EOT_CMD_ID) == true) { return SATR_EOT; }

    script_sequence->dt_sec = file_buffer[(*ss_pointer)++];
    script_sequence->dt_min = file_buffer[(*ss_pointer)++];
    script_sequence->cmd_id = file_buffer[(*ss_pointer)++];
    script_sequence->command[0] = script_sequence->cmd_id;
//    script_sequence->pointer = *ss_pointer;
    script_sequence->len = file_buffer[(*ss_pointer)++];
    script_sequence->command[1] = script_sequence->len;
    //script_sequence->seq_cnt = file_buffer[(*ss_pointer)++];
    //script_sequence->command[2] = script_sequence->seq_cnt;
    
    //if( script_sequence->cmd_id == 0x05) { script_sequence->len = script_sequence->len + 2; }
    
    for(uint8_t i = 0; i < script_sequence->len; i++) {
        script_sequence->command[i+2] = file_buffer[(*ss_pointer)++]; 
    }

    if(!C_ASSERT(script_sequence->dt_sec < 59) == true) { return SATR_ERROR; }
    if(!C_ASSERT(script_sequence->dt_min < 59) == true) { return SATR_ERROR; }
    if(!C_ASSERT(script_sequence->cmd_id == SU_OBC_SU_ON_CMD_ID || \
                 script_sequence->cmd_id == SU_OBC_SU_OFF_CMD_ID || \
                 script_sequence->cmd_id == SU_RESET_CMD_ID || \
                 script_sequence->cmd_id == SU_LDP_CMD_ID || \
                 script_sequence->cmd_id == SU_HC_CMD_ID || \
                 script_sequence->cmd_id == SU_CAL_CMD_ID || \
                 script_sequence->cmd_id == SU_SCI_CMD_ID || \
                 script_sequence->cmd_id == SU_HK_CMD_ID || \
                 script_sequence->cmd_id == SU_STM_CMD_ID || \
                 script_sequence->cmd_id == SU_DUMP_CMD_ID || \
                 script_sequence->cmd_id == SU_BIAS_ON_CMD_ID || \
                 script_sequence->cmd_id == SU_BIAS_OFF_CMD_ID || \
                 script_sequence->cmd_id == SU_MTEE_ON_CMD_ID || \
                 script_sequence->cmd_id == SU_MTEE_OFF_CMD_ID || \
                 script_sequence->cmd_id == SU_OBC_EOT_CMD_ID) == true) { return SATR_ERROR; }
    
    return SATR_OK;
}

SAT_returnState su_power_ctrl(FM_fun_id fid) {
    
    tc_tm_pkt *temp_pkt = 0;
    function_management_pctrl_crt_pkt_api(&temp_pkt, EPS_APP_ID, fid, SU_DEV_ID);
    if(!C_ASSERT(temp_pkt != NULL) == true) { return SATR_ERROR; }
    route_pkt(temp_pkt);
    return SATR_OK;
}

SAT_returnState generate_obc_su_error(uint8_t *buffer) {
    uint8_t point =  22;
    buffer[point] = su_inc_buffer[point++];
    buffer[point] = su_inc_buffer[point++];
    buffer[point] = su_inc_buffer[point++];
    buffer[point] = su_inc_buffer[point++];
    
    
}
//void su_timeout_handler(uint8_t error) {
//    
//    //cnv32_8(time_now(), &obc_su_scripts.rx_buf[0]);
//    //cnv16_8(flight_data.roll, &obc_su_scripts.rx_buf[4]);
//    //cnv16_8(flight_data.pitch, &obc_su_scripts.rx_buf[6]);
//    //cnv16_8(flight_data.yaw, &obc_su_scripts.rx_buf[8]);
//    //cnv16_8(flight_data.roll_dot, &obc_su_scripts.rx_buf[10]);
//    //cnv16_8(flight_data.pitch_dot, &obc_su_scripts.rx_buf[12]);
//    //cnv16_8(flight_data.yaw_dot, &obc_su_scripts.rx_buf[14]);
//    //cnv16_8(flight_data.x_eci, &obc_su_scripts.rx_buf[16]);
//    //cnv16_8(flight_data.y_eci, &obc_su_scripts.rx_buf[18]);
//    //cnv16_8(flight_data.z_eci, &obc_su_scripts.rx_buf[20]);
//
//    uint16_t buf_pointer = SU_SCI_HEADER;
//
//    su_scripts.rx_buf[buf_pointer++] = OBC_SU_ERR_RSP_ID;
//    buf_pointer++;
//    buf_pointer++;
//
//    su_scripts.rx_buf[buf_pointer++] = su_scripts.scripts[su_scripts.active_script].header.xsum;
//    cnv32_8(su_scripts.scripts[su_scripts.active_script].header.start_time, &su_scripts.rx_buf[buf_pointer++]);
//    buf_pointer += 4;
//    cnv32_8(su_scripts.scripts[su_scripts.active_script].header.file_sn, &su_scripts.rx_buf[buf_pointer++]);
//    buf_pointer += 4;
//    su_scripts.rx_buf[buf_pointer++] = (su_scripts.scripts[su_scripts.active_script].header.su_id << 5) | su_scripts.scripts[su_scripts.active_script].header.sw_ver;
//    su_scripts.rx_buf[buf_pointer++] = (su_scripts.scripts[su_scripts.active_script].header.su_md << 5) | su_scripts.scripts[su_scripts.active_script].header.script_type;
//
//    for(uint16_t i = SU_SCRIPT_1; i <= SU_SCRIPT_7; i++) {
//
//        su_scripts.rx_buf[buf_pointer++] = su_scripts.scripts[i].header.xsum;
//        cnv32_8(su_scripts.scripts[i].header.start_time, &su_scripts.rx_buf[buf_pointer++]);
//        buf_pointer += 4;
//        cnv32_8(su_scripts.scripts[i].header.file_sn, &su_scripts.rx_buf[buf_pointer++]);
//        buf_pointer += 4;
//        su_scripts.rx_buf[buf_pointer++] = (su_scripts.scripts[i].header.su_id << 5) | su_scripts.scripts[i].header.sw_ver;
//        su_scripts.rx_buf[buf_pointer++] = (su_scripts.scripts[i].header.su_md << 5) | su_scripts.scripts[i].header.script_type;
//    }
//
//    for(uint16_t i = 99; i < 173; i++) {
//        su_scripts.rx_buf[i] = 0;
//    }
//
//    uint16_t size = SU_MAX_RSP_SIZE;
//    mass_storage_storeLogs(SU_LOG, su_scripts.rx_buf, &size);
//    su_power_ctrl(P_RESET);
//    su_scripts.timeout = time_now();
//
//    su_next_tt(su_scripts.active_buf, &su_scripts.tt_header, &su_scripts.tt_pointer_curr);
//
//}
