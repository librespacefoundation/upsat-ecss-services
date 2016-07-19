#include "su_mnlp.h"

#undef __FILE_ID__
#define __FILE_ID__ 33

extern SAT_returnState function_management_pctrl_crt_pkt_api(tc_tm_pkt **pkt, TC_TM_app_id dest_id, FM_fun_id fun_id, FM_dev_id did);
extern SAT_returnState route_pkt(tc_tm_pkt *pkt);
extern SAT_returnState HAL_su_uart_rx();
extern SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);
extern SAT_returnState mass_storage_su_load_api(MS_sid sid, uint8_t *buf);
extern SAT_returnState su_nmlp_app( tc_tm_pkt *spacket);
extern void HAL_su_uart_tx(uint8_t *buf, uint16_t size);
extern void HAL_sys_delay(uint32_t sec);
extern osThreadId su_schHandle;

struct time_utc tt_utc;
uint32_t qb_50_time;

uint32_t obc_day_moment_secs = 0;
uint32_t tt_day_moment_secs = 0; /*script time-table moment in seconds from start of current day*/
uint32_t moment_diff = 0;        /**/

//uint32_t first_tt_exec_moment = 0;
//uint32_t last_tt_exec_moment = 0;
su_mnlp_returnState state = su_sche_last;

struct _MNLP_data MNLP_data;
struct time_utc obc_day_utc_time;
//mnlp_response_science_header flight_data;

uint8_t su_sci_header[SU_SCI_HEADER_SIZE];
uint8_t su_log_buff[SU_LOG_SIZE];
uint8_t obc_su_err_seq_cnt = 1;
uint16_t last_su_incoming_time;
uint16_t current_tt_pointer = 0;    /*points to the current start byte of a time's table begining, into the loaded script*/
uint16_t current_ss_pointer = 0;;    /*points to the current start byte of a script's sequence begining, into the loaded script*/
uint32_t previousWakeTime = 0;
uint32_t sleep_until_msecs = 0;

/* if true all commands are routed to nmlp serial,
 * if false some commands are routed to cubesat subsystems. (obc_su_on, obc_su_off, )
 */
uint8_t mnlp_sim_active = false;

SAT_returnState tt_call_state;  /*time-table calling state*/
SAT_returnState ss_call_state;  /*script sequence calling state*/  
SAT_returnState scom_call_state;/*script command calling state*/    

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
            if(mnlp_sim_active){ HAL_su_uart_tx( s_seq.command, s_seq.len+2); } else{ su_power_ctrl(P_ON); }
            break;
        case 2: /*Power off mnlp unit*/
            if(mnlp_sim_active){ HAL_su_uart_tx( s_seq.command, s_seq.len+2); } else{ su_power_ctrl(P_OFF); }
            break;
        case 3: /*Power Reset mnlp unit*/
            if(mnlp_sim_active){ HAL_su_uart_tx( s_seq.command, s_seq.len+2); }else{ su_power_ctrl(P_RESET); }
            break;
        case 4: /*su load parameters*/ 
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
            break;
        case 6: /*su health check*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
            break;
        case 8: /*su calibrate*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
            break;
        case 10: /*su science*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
            break;
        case 12: /*su housekeep*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
            break;
        case 13: /*su reset mnlp state*/
            *MNLP_data.su_nmlp_last_active_script = (uint8_t) SU_NOSCRIPT;
            *MNLP_data.su_nmlp_script_scheduler_active = (uint8_t) false;
//            su_power_ctrl(P_OFF);
            //TODO: add other actios to do, eg: close the mnlp power?
            break;
        case 14: /*su stm*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
            break;
        case 16:/*su dump*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2);
            break;
        case 18: /*bias on*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
            break;            
        case 19: /*bias off*/
            HAL_su_uart_tx( s_seq.command, s_seq.len+2); 
            break;            
        case 20: /*mtee on*/
            if(mnlp_sim_active){ 
                ; }
            else{ 
                HAL_su_uart_tx( s_seq.command, s_seq.len+2); }
            break;            
        case 21: /*mtee off*/
            if(mnlp_sim_active){ 
                ; }
            else{
                HAL_su_uart_tx( s_seq.command, s_seq.len+2); }
            break;
        case 22: /*su scheduler task notify*/
                xTaskNotifyGive(su_schHandle);
            break;      
        case 23: /*MNLP status report*/
            //TODO: implement it!
            if(mnlp_sim_active){ 
                ; }
            else{ 
                ; }
            break;
        case 24: /*Manage su nmlp service scheduler*/
            if( spacket->data[0]){ /*enable nmlp service scheduler*/
                *MNLP_data.su_service_sheduler_active = (uint8_t) true;
                 spacket->verification_state = SATR_OK;
            }
            else{ /*disable nmlp service scheduler*/
                *MNLP_data.su_service_sheduler_active = (uint8_t) false;
                spacket->verification_state = SATR_OK;
            }
            break;
    }//switch ends here
    
    return SATR_OK;
}

SAT_returnState su_incoming_rx(){
    
    SAT_returnState res;
    SAT_returnState fres = SATR_OK;
    uint8_t su_resp_pos=0;
    uint16_t size = SU_LOG_SIZE;
    uint32_t current_time=0;
    
    res = HAL_su_uart_rx();
    current_time = return_time_QB50();
    
    /*detect responses timeout*/
    if( MNLP_data.su_state == SU_POWERED_ON //TODO: crseate timeout set service
            && (current_time - MNLP_data.last_su_response_time) > SU_TIMEOUT){ //10 for tests
        handle_su_timeout();
    }
    
    if(res == SATR_EOT){
        
        MNLP_data.last_su_response_time = return_time_QB50();
        MNLP_data.su_timed_out = (uint8_t) false;
        
        /*science header request to ADCS subsystem*/
//        hk_crt_pkt_TC( test_pkt, ADCS_APP_ID, SU_SCI_HDR_REP);
        
        /*science header*/
        uint32_t qb_50_secs;
        get_time_QB50(&qb_50_secs);
        /*the rest of su science header if filled from housekeeping service, when handling of SU_SCI_HDR_REP is done*/
        cnv32_8( qb_50_secs, &su_sci_header[0]);
//        int16_t roll = 10; 
//        cnv16_8(roll, &su_sci_header[4]);
//        int16_t pitch = 20;
//        cnv16_8(pitch, &su_sci_header[6]);
//        int16_t yaw = 30;
//        cnv16_8(yaw, &su_sci_header[8]);
//        int16_t rolldot = 400;
//        cnv16_8(rolldot, &su_sci_header[10]);
//        int16_t pitchdot = 500;
//        cnv16_8(pitchdot, &su_sci_header[12]);
//        int16_t yawdot = 600;
//        cnv16_8(yawdot, &su_sci_header[14]);
//        uint16_t xeci = 6700;
//        cnv16_8(xeci, &su_sci_header[16]);
//        uint16_t yeci = 6800;
//        cnv16_8(yeci, &su_sci_header[18]);
//        uint16_t zeci = 6900;
//        cnv16_8(zeci, &su_sci_header[20]);
        
        /**/
        if(MNLP_data.su_inc_resp[0]==0){ /*then we have a byte shift on the su-nmlp response*/
            for(su_resp_pos;su_resp_pos<7;su_resp_pos++){
                if(MNLP_data.su_inc_resp[su_resp_pos]==0){continue;}else{break;}}}
        
        /*zip the two responses to one buffer*/
        memcpy(su_log_buff, su_sci_header,SU_SCI_HEADER_SIZE);
        memcpy(&su_log_buff[SU_SCI_HEADER_SIZE], &MNLP_data.su_inc_resp[su_resp_pos],SU_RSP_SIZE);
        
        switch(MNLP_data.su_inc_resp[su_resp_pos])
        {
            case (uint8_t)SU_LDP_RSP_ID:
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                break;
            case (uint8_t)SU_HC_RSP_ID:
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                break;
            case (uint8_t)SU_CAL_RSP_ID:
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                break;
            case (uint8_t)SU_SCI_RSP_ID:
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                break;
            case (uint8_t)SU_HK_RSP_ID:
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                break;    
            case (uint8_t)SU_STM_RSP_ID:
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                break;
            case (uint8_t)SU_DUMP_RSP_ID:
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                break;
            case (uint8_t)SU_ERR_RSP_ID:
            /*indicates that nmlp is in reset state, power cycle must be done as of 13.4.3 M-NLP ICD issue 6.2, page 49*/
                /*save the su_error packet as is*/
                fres = mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
                /*generate the obc_su_error packet, and cycle power to mnlp*/
                handle_su_error_response();
                break;
        }
    }
    
    memset(su_sci_header,0x00,SU_SCI_HEADER_SIZE);
    memset(su_log_buff,0x00,SU_LOG_SIZE);
    /*set response to zero, in order to detect shifts in next responses*/
    memset(MNLP_data.su_inc_resp,0x00,180);
    return SATR_OK;
}

void su_INIT(){
    
    su_power_ctrl(P_OFF); /*the su may remain powered on, in case that OBC reset occurs and a OBC_SU_OFF command is not executed*/
    MNLP_data.su_state = SU_POWERED_OFF;
    MNLP_data.su_timed_out = (uint8_t) false;
    MNLP_data.current_tt = 0;
    MNLP_data.current_sip = 0x0;
    MNLP_data.tt_exec_on_span_count = 0;
    MNLP_data.tt_lost_count = 0;
    MNLP_data.tt_norm_exec_count = 0;
    *MNLP_data.su_init_func_run_time = return_time_QB50();
    
    mnlp_sim_active = false;
    su_load_all_scripts();
    for (MS_sid i = SU_SCRIPT_1; i <= SU_SCRIPT_7; i++){
        if (MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_str == true &&
            MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_logi == true){
            /*populate only script headers that are structural OK*/    
            //TODO: add a byte to store invalid scripts, LSB for script1, 0 for ok, 1 for checksum error
            su_populate_header(&(MNLP_data.su_scripts[(uint8_t) i - 1].scr_header), 
                                 MNLP_data.su_scripts[(uint8_t) i - 1].file_load_buf);
        }
    }
    /*Enable the script scheduler*/
//    *MNLP_data.su_nmlp_scheduler_active = (uint8_t) false;
    *MNLP_data.su_nmlp_script_scheduler_active = (uint8_t) true;
}

/*
 * Loads the scripts from permanent storage.
 */
void su_load_all_scripts(){
    
    SAT_returnState res ;
    for( MS_sid i = SU_SCRIPT_1; i <= SU_SCRIPT_7; i++) {
        /*mark every script as non-valid, to check it later on memory and mark it (if it is, structurally) valid*/
        MNLP_data.su_scripts[(uint8_t) i - 1].valid_str = false;
        /*load scripts on memory but, parse them at a later time, in order to unlock access to the storage medium for other users*/
        res = mass_storage_su_load_api( i, MNLP_data.su_scripts[(uint8_t) i - 1].file_load_buf);
//        SAT_returnState res = mass_storage_su_load_api( SU_SCRIPT_1, MNLP_data.su_scripts[(uint8_t) i - 1].file_load_buf);
        if( res != SATR_OK ){ //if( res == SATR_ERROR || res == SATR_CRC_ERROR){
            /*script is kept marked as invalid because of CRC error, or invalid length, or some other error*/
            continue;
        }
        else{
            /* Mark the script as both structural/logical valid, to be parsed afterwards inside the su_scheduler. */
            /* This is the first load from memory, maybe after a reset, so we assume that scripts are
             * logically valid (they are purposed to be scheduled). 
             */
            MNLP_data.su_scripts[(uint8_t) i - 1].valid_str = true;
            MNLP_data.su_scripts[(uint8_t) i - 1].valid_logi = true;
            /*populate only script headers that are structural OK*/    
            su_populate_header(&(MNLP_data.su_scripts[(uint8_t) i - 1].scr_header), 
                               MNLP_data.su_scripts[(uint8_t) i - 1].file_load_buf);
            
        }
    }
}

/**
 * Loads a specific script from permanent storage.
 */
SAT_returnState su_load_specific_script(MS_sid sid){
    
    SAT_returnState res;
        /*mark every script as non-valid, to check it later on memory and mark it (if it is, structurally) valid*/
        MNLP_data.su_scripts[(uint8_t) sid - 1].valid_str = false;
        /*load scripts on memory but, parse them at a later time, in order to unlock access to the storage medium for other users*/
        res = mass_storage_su_load_api( sid, MNLP_data.su_scripts[(uint8_t) sid - 1].file_load_buf);
        if( res != SATR_OK ){ //if( res == SATR_ERROR || res == SATR_CRC_ERROR){
            /*script is kept marked as invalid because of CRC error, or invalid length, or some other error*/
        return res;
        }else{
            /* Mark the script as both structural/logical valid, to be parsed afterwards inside the su_scheduler. */
            /* This is the first load from memory, maybe after a reset, so we assume that scripts are
             * logically valid (they are purposed to be scheduled). 
             */
            MNLP_data.su_scripts[(uint8_t) sid - 1].valid_str = true;
            MNLP_data.su_scripts[(uint8_t) sid - 1].valid_logi = true;
            /*populate only script headers that are structural OK*/    
            su_populate_header(&(MNLP_data.su_scripts[(uint8_t) sid - 1].scr_header), 
                               MNLP_data.su_scripts[(uint8_t) sid - 1].file_load_buf);
    return res;
        }
}

/**
 * Selects the appropriate script that is eligible to run, and marks
 * it as the ''running script''.
 * @param sleep_val_secs
 * @return 
 */
su_mnlp_returnState su_script_selector(uint32_t *sleep_val_secs){
    /*script chooser code segment*/
    uint32_t least_act_time = 4000000000  ;//4294967296;
    for(MS_sid i = SU_SCRIPT_1; i <= SU_SCRIPT_7; i++){
        if(MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_str == true &&
            MNLP_data.su_scripts[(uint8_t)(i) - 1].valid_logi == true){
            uint32_t qb_f_time_now = 0;
            uint32_t time_diff = 4000000000; /*used as a flag*/
            get_time_QB50(&qb_f_time_now);
            if( qb_f_time_now <= MNLP_data.su_scripts[(uint8_t) i - 1].scr_header.start_time){
                /**/
                time_diff = MNLP_data.su_scripts[(uint8_t) i - 1].scr_header.start_time - qb_f_time_now;
                if( time_diff < least_act_time){ least_act_time = time_diff; }
            }
            /* if time_diff is > 0(zero) means that the current checking script
             * start time has passed over us, so the closest value bigger than (>) 0 and closest to zero is the script to be activated
             * before others. Now, if time_diff > zero AND time_diff >=0 && time_diff <=60 (secs)
             * mark it as active script, with a max. activation delay of 1 min.
             * also see page 35 on mnlp icd, for general script selection concept
             */
            if(time_diff >= 0 && time_diff <= 60){ //TODO: maybe open this span a little this a little
                MNLP_data.active_script = (MS_sid) i;
                /*Save the last chosen active script to sram*/
                *MNLP_data.su_nmlp_last_active_script = (MS_sid) i;
                /*set first time table to be executed*/
                *MNLP_data.su_next_time_table = 1;
                /*set script sequence 1 (0x41) to be executed*/
                *MNLP_data.su_next_script_seq = (uint8_t) 0x41;                
                /*don't check other scripts, we have just choose one as the active script*/
                /*enable the su scheduler, because a script is about to become active*/
                *MNLP_data.su_nmlp_script_scheduler_active = (uint8_t) true;
//                break;
//                *sleep_val_secs = time_diff; //TODO substract something
                *sleep_val_secs = 5;
                trace_SCR_MARKED_ACTIVE(i);
                return su_new_scr_selected;
            }
        }
        else{ /*here log if you want the invalid script's number*/
                continue;
        }
    }
    
    if(*MNLP_data.su_nmlp_last_active_script == SU_NOSCRIPT){ /*means that we have never activated a script*/
        if( least_act_time ==  4000000000 ){ /*if least_act_time untouched, sleep for 50 seconds*/
            *sleep_val_secs = 50; }
        else{ *sleep_val_secs = (least_act_time-30); } /*no chance to go < 0, because its always >=61 seconds*/
        
        trace_SCR_NON_ELIG();
        return su_no_scr_eligible;
    }
    /* in case that no newer script is eligible to be activated,
     * we run the last activated script continuously.
     */
    MNLP_data.active_script = *MNLP_data.su_nmlp_last_active_script;
    
    /* assert that active script is not zero (SU_NOSCRIPT), if it is and we have reached this point something is not quite right
     * so hard-set script 1 as the running script
     */
    if(!C_ASSERT(MNLP_data.active_script != (uint8_t) 0) ){ 
       *MNLP_data.su_nmlp_last_active_script = 1; MNLP_data.active_script = 1; }
    
    trace_SCR_NO_NEW_TO_BACT(*MNLP_data.su_nmlp_last_active_script);
    return su_no_new_scr_selected;
}

/**
 * Handles the science unit scripts.
 */
su_mnlp_returnState su_SCH(uint32_t *sleep_val_secs){
    
    /*uint32_t */ qb_50_time = 0;
    get_time_QB50(&qb_50_time);
    /*uint32_t*/ obc_day_moment_secs = 0;
    /*uint32_t*/ tt_day_moment_secs = 0; /*script time-table moment in seconds from start of current day*/
    /*uint32_t*/ moment_diff = 0;        /**/
    /*su_mnlp_returnState*/ state = su_sche_last;
    
    if(!C_ASSERT(MNLP_data.active_script != (MS_sid) 0) ) { *sleep_val_secs = 55; return su_sche_script_ended; }
    if(MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].valid_str == true &&
       MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].valid_logi == true){

        /* the first byte after the 12-byte sized script header,
         * points to the first time table, on the active_script script.
         */
        current_tt_pointer = SU_TT_OFFSET;
        MNLP_data.current_tt = 0; /*set TimeTable index to zero*/
        /*finds the next tt that needs to be executed, it iterates all the tt to find the correct one*/
        for(uint16_t b = current_tt_pointer; b < SU_MAX_FILE_SIZE; b++){ /*due to 'no while' policy*/
            tt_call_state =
                polulate_next_time_table(
                 MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].file_load_buf,
                &MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header,
                &current_tt_pointer);
            if( tt_call_state == SATR_EOT){
                /*reached the last time table, go for another script, or this script, but on the NEXT day*/
                *sleep_val_secs = 5;
                 /*reset the script index*/
                MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.script_index = SU_SCR_TT_SNONE;
                MNLP_data.current_tt = 0; /*set TimeTable index to zero*/
		trace_SCR_ENDED();                
		return su_sche_script_ended;
            }
            else
            if(tt_call_state == SATR_ERROR){
                /*non valid time table, go for next time table*/
                MNLP_data.current_tt++;
                continue;
            }
            else
            { /*we have a time table to handle*/
                MNLP_data.current_tt++;
                /*check this day's moment time against time_table's start time, and wait for it to come.*/
                for(uint32_t i = 1; i < 86400; i++){ /*due to 'no while' policy*/
                    tt_utc.hour= MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.hours;
                    tt_utc.min = MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.min;
                    tt_utc.sec = MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.sec;
                    get_time_UTC(&obc_day_utc_time);
                    cnv_utc_to_secs(&obc_day_utc_time, &obc_day_moment_secs); //TODO change to return qb50
                    cnv_utc_to_secs(&tt_utc, &tt_day_moment_secs); //TODO change to return qb50
                    /* the difference in here should be max. of 24 hours e.g 86400 seconds,
                     * because we have choose the right active script
                     */
                    if(obc_day_moment_secs > tt_day_moment_secs){ /*we have passed over the time table's start time*/
                        moment_diff = obc_day_moment_secs - tt_day_moment_secs;
                        if(moment_diff > 0 && moment_diff <= 6){
                            /*f us 6 seconds, and execute a little late*/
                            MNLP_data.tt_exec_on_span_count++;
                            serve_tt();
                            break;
                        }
                        else
                        if(moment_diff > 6){
                            /*for some reason we have lost this time-table, go to next one*/                  
                            MNLP_data.tt_lost_count++;
                            break;
                        }
                    }
                    else
                    if(obc_day_moment_secs < tt_day_moment_secs){
                        uint32_t to_set = 0;
                        /*time table is in the future, delay until that moment/2*/
                        moment_diff = tt_day_moment_secs - obc_day_moment_secs;
                        to_set = moment_diff / 2;                        
                        if( to_set == 0){ /*some times half a second earlier*/                            
                            MNLP_data.tt_norm_exec_count++;
                            serve_tt();
                            break;
                        }
                        else{                                    
                            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS((to_set)*1000));
                        }
                    }
                    else{ /*execute at once*/
                        MNLP_data.tt_norm_exec_count++;
                        serve_tt();
                        break;
                    }
                }//for 86400 ends here
            }
            //go for next time table
        }
        //script handling for ends here, at this point every time table in (the current) script has been served.
        //su state is updated in script command handler
        //            MNLP_data.su_state = SU_POWERED_OFF
    }
}

void serve_tt(){
    /*find the script sequence pointed by *time_table->script_index, and execute it */
    /*start every search after current_tt_pointer, current_tt_pointer now points to the start of the next tt_header*/
    current_ss_pointer = current_tt_pointer - 1;
    ss_call_state =
            su_goto_script_seq(&current_ss_pointer,
            &MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.script_index);
    if( ss_call_state == SATR_OK){
        for( uint16_t p = 0; p < SU_MAX_FILE_SIZE; p++){ /*start executing commands in a script sequence*/ /*due to 'no while' policy*/
            scom_call_state =
                su_next_cmd(MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].file_load_buf,
                &MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header,
                &current_ss_pointer);
            if( scom_call_state == SATR_EOT){
                /*no more commands on script sequences to be executed*/
                MNLP_data.current_sip = 0x55;
                /*reset seq_header->cmd_if field to something else other than 0xFE*/
                MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header.cmd_id = 0x0;               
                break;
            }else
            if( scom_call_state == SATR_ERROR){
                MNLP_data.current_sip = 0x66;
                /*an unknown command has been encountered in the script sequences, so go for the next command*/
                uint8_t stop_here = 0;
                continue;
            }else{
                /*handle script sequence command*/
                if( MNLP_data.su_timed_out == (uint8_t) true){
                    
                    break;
                }
                else{ //TODO: add an extra check here, for valid seq_header state
                    MNLP_data.current_sip = MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].tt_header.script_index;
                    su_cmd_handler( &MNLP_data.su_scripts[(uint8_t) (MNLP_data.active_script - 1)].seq_header);
                }
            }
        }
    }
    else
    if (ss_call_state == SATR_EOT){
        
    }
}

/* 
 * traverses the script sequences to find the one requested by
 * *ss_to_go parameter, the resulting offset is stored on 
 * *script_sequence_pointer.
 */
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
    
    //TODO Add more strict assertions
    if(!C_ASSERT(time_table->sec < 59) == true) { return SATR_ERROR; }
    if(!C_ASSERT(time_table->min < 59) == true) { return SATR_ERROR; }
    if(!C_ASSERT(time_table->hours < 24) == true) { return SATR_ERROR; }
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
    
    if( mnlp_sim_active){    //route cmd to su nmlp simulator        
        if( cmd->cmd_id == 0xF1){
            HAL_su_uart_tx( cmd->command, cmd->len+2);
            HAL_sys_delay(1000);            
        }else
        if( cmd->cmd_id == 0x05){
            HAL_su_uart_tx( cmd->command, cmd->len+1);
            uint8_t su_out[105];
            su_out[0]= 0x05;
            su_out[1]= 0x63; //len
            su_out[2]= 2;    //seq_coun
            HAL_su_uart_tx( su_out, 102);  
        }
        else{ HAL_su_uart_tx( cmd->command, cmd->len+2); }
    }
    else{ //route to real su mnlp unit
    if(cmd->cmd_id == SU_OBC_SU_ON_CMD_ID){
        su_power_ctrl(P_ON);
        MNLP_data.su_state = SU_POWERED_ON;
        MNLP_data.last_su_response_time = return_time_QB50();
        
    }else
    if(cmd->cmd_id == SU_OBC_SU_OFF_CMD_ID){
        su_power_ctrl(P_OFF);
        MNLP_data.su_state = SU_POWERED_OFF;
        
    }else 
    if(cmd->cmd_id == SU_OBC_EOT_CMD_ID){
        return SATR_EOT;
    }else{ 
        HAL_su_uart_tx(cmd->command, cmd->len + 2); }
    }
    return SATR_OK;
}

/* 
 * TITLE: QB50 m-NLP Science Unit Interface Control Document
 * Document Number: QB50-UiO-ID-0001 M-NLP ICD Issue 6.2
 * Date 22.10.2015, page 51.
 * Header fixed size is 12 bytes.
 */
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

/**
 * 
 * @param err_source designates the reason for the error.
 *              
 */
void handle_su_error_response(){
    
    //steps to do as described in page 42 of m-nlp icd, issue 6.2
    //abort current running script
    //turn off su, 
    //generate OBC_SU_ERR packet
    //wait 60 secs, rerun same script from NEXT time tables, 
    //(because propably the previous command gave you an error state))
//    uint16_t size = SU_RSP_SIZE;
    uint16_t size = SU_LOG_SIZE;
    /*clear the su_log_buffer to be used for error reporting*/
    memset(su_log_buff, 0x00, SU_LOG_SIZE);
    
    MNLP_data.su_timed_out = (uint8_t) true; //to break any other execution
    su_power_ctrl(P_OFF);
    MNLP_data.su_state = SU_POWERED_OFF;
    *MNLP_data.su_nmlp_script_scheduler_active = false;
    
    /*generate error with 0xBB code*/
    generate_obc_su_error(su_log_buff, SU_ERR_RSP_ID);
    mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
    HAL_sys_delay(SU_ERR_TIM_SLEEP_TIME); //TODO check this    
    
//    su_power_ctrl(P_ON); /*su will be powered on again from next time table*/
//    MNLP_data.su_state = SU_POWERED_ON;
//    MNLP_data.last_su_response_time = return_time_QB50();
    MNLP_data.su_timed_out = (uint8_t) false;
    *MNLP_data.su_nmlp_script_scheduler_active = true;
}

/**
 * Indicates that nmlp has timed out,
 * power cycle must be done as of 13.4.3 M-NLP ICD issue 6.2, page 42
 * @param error
 */
void handle_su_timeout(){
    
//    uint16_t size = SU_RSP_SIZE;
    uint16_t size = SU_LOG_SIZE;  
    /*clear the su_log_buffer to be used for error reporting*/
    memset(su_log_buff, 0x00, SU_LOG_SIZE);
    
    MNLP_data.su_timed_out = (uint8_t) true;
    su_power_ctrl(P_OFF);
    MNLP_data.su_state = SU_POWERED_OFF;
    *MNLP_data.su_nmlp_script_scheduler_active = false;
        
    /*generate error with 0xF0 code*/
    generate_obc_su_error(su_log_buff, SU_TIMEOUT_ERR_ID);
    mass_storage_storeFile( SU_LOG, 0 ,su_log_buff, &size);
    HAL_sys_delay(SU_ERR_TIM_SLEEP_TIME); //TODO: check this
    
//    su_power_ctrl(P_ON); /*su will be powered on again from next time table*/
//    MNLP_data.su_state = SU_POWERED_ON;
//    MNLP_data.last_su_response_time = return_time_QB50();
    MNLP_data.su_timed_out = (uint8_t) false;
    *MNLP_data.su_nmlp_script_scheduler_active = true;
}

/**
 * Generate OBC_SU ERR(or) packet, as of m-nlp icd issue 6.2, page 43.
 * @param buffer
 * @param err_source Is the event the generates the error code:
 *          0xF0 for su timeout error,          
 *          0xF1 for packet lenght error(?),
 *          0xBB for su_err received from mnlp science unit          
 * @return 
 */
SAT_returnState generate_obc_su_error(uint8_t *buffer, uint8_t rsp_error_code){
    
    /*save only the first 174 bytes of the generated error*/
    buffer[0] = (uint8_t)OBC_SU_ERR_RSP_ID; //0xFA
    buffer[1] = obc_su_err_seq_cnt++; //add our own packet. seq number, ++for next one
    buffer[2] = rsp_error_code;   //code that indicates the error source
    cnv16_8(MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].scr_header.xsum, &buffer[3]); 
    cnv32_8(MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].scr_header.start_time, &buffer[5]);
    cnv32_8(MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].scr_header.file_sn, &buffer[9]);
    buffer[13] = MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].file_load_buf[10];
    buffer[14] = MNLP_data.su_scripts[(uint8_t) MNLP_data.active_script - 1].file_load_buf[11];
    uint8_t base = 15; /*sixteenth byte*/
    for( uint8_t p=0;p<7;p++){ /*run seven times*/
        
        cnv16_8(MNLP_data.su_scripts[p].scr_header.xsum, &buffer[base]);
        cnv32_8(MNLP_data.su_scripts[p].scr_header.start_time, &buffer[base+2]);
        cnv32_8(MNLP_data.su_scripts[p].scr_header.file_sn, &buffer[base+6]);
        buffer[base+10] = MNLP_data.su_scripts[p].file_load_buf[10];
        buffer[base+11] = MNLP_data.su_scripts[p].file_load_buf[11];
        base +=12;
    }    
    return SATR_OK;
}

SAT_returnState handle_script_upload(MS_sid sid){

    //TODO: disable the spck on scheduling service
    if(MNLP_data.active_script == sid ){
        /*if the active script is being updated, requires special handling*/        
        if( MNLP_data.su_state == SU_POWERED_ON){
            su_power_ctrl(P_OFF);
            MNLP_data.su_state = SU_POWERED_OFF;
        }
    }
    return su_load_specific_script(sid);
}
