/* 
 * File:   scheduling_service.c
 * Author: 
 *
 * Created on March 8, 2016, 9:05 PM
 * This is the implementation of scheduling service as is
 * documented at pages 99-118 of ECSS-E-70-41A document.
 * Service Type 11 
 * (some restrictions apply)
 */

#undef __FILE_ID__
#define __FILE_ID__ 6

#include "scheduling_service.h"

uint8_t sche_tc_buffer[MAX_PKT_LEN+14+1]; /*224+1 for checksum*/

extern void wdg_reset_SCH();
extern SAT_returnState mass_storage_schedule_load_api(MS_sid sid, uint32_t sch_number, uint8_t *buf);
extern SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);
extern uint32_t HAL_GetTick(void);

SAT_returnState handle_sch_reporting(uint8_t *tc_tm_data);
SAT_returnState scheduling_service_report_summary(tc_tm_pkt *pkt, TC_TM_app_id dest_id);
SAT_returnState scheduling_service_report_detailed(tc_tm_pkt *pkt, TC_TM_app_id dest_id, uint8_t apid, uint8_t seqcnt);
SAT_returnState scheduling_service_load_schedules();
SAT_returnState scheduling_service_crt_pkt_TM(tc_tm_pkt *pkt, uint8_t sid, TC_TM_app_id dest_app_id );
SAT_returnState time_shift_sel_schedule(uint8_t *data_v);
SAT_returnState copy_inner_tc(const uint8_t *buf, tc_tm_pkt *pkt, const uint16_t size);

uint8_t check_existing(uint8_t apid, uint8_t seqc);

SC_pkt* find_schedule_pos();

Scheduling_service_state sc_s_state;
Schedule_pkt_pool sch_mem_pool;

/**
 * Initializes the scheduling service.
 * 
 * @return the execution state.
 */
SAT_returnState scheduling_service_init(){
    
    /* Initialize schedules memory.
     * Assign proper memory allocation to inner TC of ScheduleTC for its data payload.
     */
    for(uint8_t s=0;s<SC_MAX_STORED_SCHEDULES;s++){
        sch_mem_pool.sc_mem_array[s].tc_pck.data 
                         = sch_mem_pool.innerd_tc_data[s];
        
        /* Marks every schedule as invalid, so its position
         * can be taken by a request to the Schedule packet pool.
         */
        sch_mem_pool.sc_mem_array[s].pos_taken = false;
    }
        
    sc_s_state.nmbr_of_ld_sched = 0;
    sc_s_state.sch_arr_full = false;
    
    /*Enable scheduling release for every APID*/
    for(uint8_t s=0;s<LAST_APP_ID-1;s++){
        sc_s_state.schs_apids_enabled[s] = true;
    }
    
    /* Load Schedules from storage.
     * 
     */
    scheduling_service_load_schedules();
    return SATR_OK;
}

/**
 * Serves requests to Scheduling service, unique entry point to the Service.
 * @param spacket
 * @return 
 */
SAT_returnState scheduling_app( tc_tm_pkt *tc_tm_packet){
    
    /*TODO: add assertions*/
    SAT_returnState exec_state = SATR_ERROR;
    SC_pkt *sc_packet;
    
    if(!C_ASSERT(tc_tm_packet != NULL) == true) { return SATR_ERROR; }

    switch( tc_tm_packet->ser_subtype){
        case SCHS_ENABLE_RELEASE:
            exec_state = enable_disable_schedule_apid_release( SCHS_ENABLE_RELEASE, tc_tm_packet->data[3] );
            break;
        case SCHS_DISABLE_RELEASE:
            exec_state = enable_disable_schedule_apid_release( SCHS_DISABLE_RELEASE, tc_tm_packet->data[3] );
            break;
        case SCHS_RESET_SCH:
            exec_state = operations_scheduling_reset_schedule_api();
            break;
        case SCHS_INSERT_TC_IN_SCH:
            if( (sc_packet = find_schedule_pos()) == NULL){
                exec_state = SATR_SCHS_FULL;
            }
            else{
                    exec_state = parse_sch_packet(sc_packet, tc_tm_packet);
                    if(exec_state == SATR_OK){
                        /*Place the packet into the scheduling array*/
                        sc_s_state.nmbr_of_ld_sched++;
                        if (sc_s_state.nmbr_of_ld_sched == SC_MAX_STORED_SCHEDULES){
                            /*schedule array has become full*/
                            sc_s_state.sch_arr_full = true;
                            SYSVIEW_PRINT("SCHS PKT ADDED");
                        }
                    }
                }
            break;
        case SCHS_DELETE_TC_FROM_SCH: /*selection criteria is destined APID and Seq.Count*/
            if(!C_ASSERT( tc_tm_packet->data[1] < LAST_APP_ID) == true) { return SATR_ERROR; }
            /*if(!C_ASSERT( seqcnt <= MAX_SEQ_CNT) == true) { return SATR_ERROR; }*/
            exec_state = scheduling_remove_schedule_api( tc_tm_packet->data[1], tc_tm_packet->data[2]);
            break;
        case SCHS_DELETE_TC_FROM_SCH_OTP:
            /*unimplemented*/
            exec_state = SATR_SCHS_NOT_IMLP;
            break;
        case SCHS_TIME_SHIFT_SEL_TC:
            if(!C_ASSERT( tc_tm_packet->data[5] < LAST_APP_ID) == true) { return SATR_ERROR; }
            /*if(!C_ASSERT( seqcnt <= MAX_SEQ_CNT) == true) { return SATR_ERROR; }*/
            exec_state = time_shift_sel_schedule(tc_tm_packet->data);
            break;
        case SCHS_TIME_SHIFT_SEL_TC_OTP:
            exec_state = SATR_SCHS_NOT_IMLP;
            break;
        case SCHS_TIME_SHIFT_ALL_TCS:
            exec_state = time_shift_all_tcs(tc_tm_packet->data);
            break;
        case SCHS_REPORT_SCH_DETAILED:
            tc_tm_pkt *sch_rep_d_pkt = get_pkt(PKT_NORMAL);
            if(!C_ASSERT(sch_rep_d_pkt != NULL)==true) { return SATR_ERROR; }
            exec_state = scheduling_service_report_detailed(sch_rep_d_pkt, 
                         (TC_TM_app_id)tc_tm_packet->dest_id, tc_tm_packet->data[0], tc_tm_packet->data[1]);
            tc_tm_packet->verification_state = exec_state;
            route_pkt(sch_rep_d_pkt);
            break;
        case SCHS_REPORT_SCH_SUMMARY:
            tc_tm_pkt *sch_rep_s_pkt = get_pkt(PKT_NORMAL);
            if(!C_ASSERT(sch_rep_s_pkt != NULL)==true) { return SATR_ERROR; }
            exec_state = scheduling_service_report_summary(sch_rep_s_pkt, (TC_TM_app_id)tc_tm_packet->dest_id);
            tc_tm_packet->verification_state = exec_state;
            route_pkt(sch_rep_s_pkt);
            break;
        case SCHS_LOAD_SCHEDULES: /*Load TCs from permanent storage*/
            exec_state = scheduling_service_load_schedules();
            break;
        case SCHS_SAVE_SCHEDULES: /*Save TCs to permanent storage*/
            exec_state = scheduling_service_save_schedules();
            break;
    }

    tc_tm_packet->verification_state = exec_state;
    return exec_state;
}

/* Cross schedules array, 
 * in every pass check if scheduling for specific APID is enabled,
 * if enabled,
 *  if it is then check if its relative or absolute and check the time.
 *  if time_now >= release time, then execute it.
 * else !enabled
 *  if time >= release time, then mark it as !valid
 */
SAT_returnState cross_schedules() {
    
    for (uint8_t i = 0; i < SC_MAX_STORED_SCHEDULES; i++) {
        if (sch_mem_pool.sc_mem_array[i].pos_taken == true && /*if a valid schedule exists*/
            sch_mem_pool.sc_mem_array[i].enabled &&
            sc_s_state.schs_apids_enabled[(sch_mem_pool.sc_mem_array[i].app_id) - 1] == true){ /*if scheduling enabled for this APID */

            switch(sch_mem_pool.sc_mem_array[i].sch_evt){
                /*case ABSOLUTE:
                    uint32_t boot_secs = HAL_GetTick();
                    if(sch_mem_pool.sc_mem_array[i].release_time <= (boot_secs / 1000)){
                        route_pkt(&(sch_mem_pool.sc_mem_array[i].tc_pck));
                        sch_mem_pool.sc_mem_array[i].pos_taken = false;
                        sc_s_state.nmbr_of_ld_sched--;
                        sc_s_state.sch_arr_full = false;
                    }
                    break;*/
                case REPETITIVE:                    
                    uint32_t qb_time = return_time_QB50();
                    if(!C_ASSERT(qb_time >= MIN_VALID_QB50_SECS) == true ) { 
                        wdg_reset_SCH();
                        return SATR_QBTIME_INVALID; }
                    SYSVIEW_PRINT("SCHS CHECKING SCH TIME");
                    if(sch_mem_pool.sc_mem_array[i].release_time <= qb_time){ /*time to execute*/
                        SYSVIEW_PRINT("SCHS ROUTING PKT");
                        route_pkt(&(sch_mem_pool.sc_mem_array[i].tc_pck));
                        if(!(sch_mem_pool.sc_mem_array[i].timeout <=0)){ /*to save the else, and go for rescheduling*/
                            sch_mem_pool.sc_mem_array[i].release_time =
                                (sch_mem_pool.sc_mem_array[i].release_time + sch_mem_pool.sc_mem_array[i].timeout);
                            sch_mem_pool.sc_mem_array[i].pos_taken = true;
                            sch_mem_pool.sc_mem_array[i].enabled = true;
                            SYSVIEW_PRINT("SCHS SCHEDULE RESCHEDULED");
                            continue;
                        }/*timeout field is positive */
                        sch_mem_pool.sc_mem_array[i].pos_taken = false;
                        sch_mem_pool.sc_mem_array[i].enabled = false;
                        sc_s_state.nmbr_of_ld_sched--;
                        sc_s_state.sch_arr_full = false;
                        SYSVIEW_PRINT("SCHS SCHEDULE POS FREED");
                    }
                    break;
             }
        }
    }/*go to check next schedule*/
    wdg_reset_SCH();
    return SATR_OK;
}

/**
 * Handles the simple report request.
 * This reports just the scheduling position (0-14) and if something is in it.
 * 
 * @param tc_tm_data.
 * @return the execution state.
 */
SAT_returnState scheduling_service_report_summary(tc_tm_pkt *pkt, TC_TM_app_id dest_id){
    
    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }
    scheduling_service_crt_pkt_TM(pkt, SCHS_SIMPLE_SCH_REPORT, dest_id);
    uint8_t base = 0;
    for(uint8_t i = 0; i < SC_MAX_STORED_SCHEDULES; i++){
        pkt->data[base] = sch_mem_pool.sc_mem_array[i].pos_taken;
        base+=1;
        pkt->data[base] = sch_mem_pool.sc_mem_array[i].enabled;
        base+=1;
        pkt->data[base] = sch_mem_pool.sc_mem_array[i].app_id;
        base+=1;
        pkt->data[base] = sch_mem_pool.sc_mem_array[i].seq_count;
        base+=1;
        pkt->data[base] = sch_mem_pool.sc_mem_array[i].sch_evt;
        base+=1;
        cnv32_8(sch_mem_pool.sc_mem_array[i].release_time, &pkt->data[base]);
        base+=4;
        cnv32_8(sch_mem_pool.sc_mem_array[i].timeout, &pkt->data[base]);
        base+=4;
    }
    pkt->len = 13*SC_MAX_STORED_SCHEDULES;
    return SATR_OK;
}

/**
 * Handles the full report request.
 * @param tc_tm_data.
 * @return the execution state.
 */
SAT_returnState scheduling_service_report_detailed(tc_tm_pkt *pkt, TC_TM_app_id dest_id, uint8_t apid, uint8_t seqcnt){
    
    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }
    if(!C_ASSERT( apid < LAST_APP_ID) == true) { return SATR_ERROR; }
    /*if(!C_ASSERT( seqcnt <= MAX_SEQ_CNT) == true) { return SATR_ERROR; }*/
    
    scheduling_service_crt_pkt_TM(pkt, SCHS_DETAILED_SCH_REPORT, dest_id);
    for(uint8_t i = 0; i < SC_MAX_STORED_SCHEDULES; i++){
        if(sch_mem_pool.sc_mem_array[i].app_id == apid &&
           sch_mem_pool.sc_mem_array[i].seq_count == seqcnt){
            uint8_t base =0;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.app_id;
            base+=1;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.type;
            base+=1;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.seq_flags;
            base+=1;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.seq_count;
            base+=1;
            cnv16_8(sch_mem_pool.sc_mem_array[i].tc_pck.len, &pkt->data[base]);
            base+=2;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.ack;
            base+=1;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.ser_type;
            base+=1;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.ser_subtype;
            base+=1;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.dest_id;
            base+=1;            
            for(uint16_t p=0;p<sch_mem_pool.sc_mem_array[i].tc_pck.len;p++){
                pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.data[p];
                base+=1;
            }
            base+=1;
            pkt->data[base] = sch_mem_pool.sc_mem_array[i].tc_pck.verification_state;
            pkt->len = base;
            break;
        }
    }
    return SATR_OK;
}

SAT_returnState scheduling_service_crt_pkt_TM(tc_tm_pkt *pkt, uint8_t sid, TC_TM_app_id dest_app_id ){

    if(!C_ASSERT(dest_app_id < LAST_APP_ID) == true)  { return SATR_ERROR; }
    crt_pkt(pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_SCHEDULING_SERVICE, sid, dest_app_id);
    pkt->len = 0;
    return SATR_OK;
}

/**
 * Time shifts forward/backward all currently active schedules.
 * For repetitive schedules add /substract the repetition time (restrictions apply).
 * For one time schedules add / substract the execution time (QB50).
 * @param time_v
 * @return the execution state.
 */
SAT_returnState time_shift_all_tcs(uint8_t *time_v){
    
    uint32_t ushift_time = 0;
    cnv8_32(time_v, &ushift_time);    
    for(uint8_t pos = 0; pos < SC_MAX_STORED_SCHEDULES; pos++) {
        if( sch_mem_pool.sc_mem_array[pos].pos_taken == true){
            switch(sch_mem_pool.sc_mem_array[pos].sch_evt){
                case REPETITIVE:
                    uint32_t rele_time = sch_mem_pool.sc_mem_array[pos].release_time;
                    uint32_t qb_time_now = return_time_QB50();
                    uint8_t neg = (ushift_time >> 31) & 0x1;
                    uint32_t shift_time_val = (~ushift_time)+ 1;
                    uint32_t new_release_t = 0;
                    if(neg){ /*then subtract it from release time*/
                        if(shift_time_val >= rele_time){ /*subtraction not possible, erroneous state*/
                            break;
                        }
                        new_release_t = rele_time - shift_time_val;
                        if((new_release_t < qb_time_now)){
                            break;
                        }
                        sch_mem_pool.sc_mem_array[pos].release_time = new_release_t;
                        break;
                    }
                    /*then add it to release time*/
                    new_release_t = rele_time + ushift_time;
                    if( new_release_t <= MAX_QB_SECS ){ /*to far to execute, we will not exist until then*/
                        sch_mem_pool.sc_mem_array[pos].release_time = new_release_t;
                        return SATR_OK;
                    }
                    break;
                break;
            }
        }
    }
    return SATR_OK;
}

/**
 * Enable / Disable the APID releases.
 * If the release for a specific APID is enabled (true) then the Sch_pkt(s) destined
 * for this APID are normally scheduled and executed.
 * False otherwise.
 * @param subtype
 * @param apid
 * @return the execution state.
 */
SAT_returnState enable_disable_schedule_apid_release( uint8_t subtype, uint8_t apid  ){
    
    if(!C_ASSERT( subtype == SCHS_ENABLE_RELEASE ||
                  subtype == SCHS_DISABLE_RELEASE ) == true) { return SATR_ERROR; }
    if(!C_ASSERT( apid < LAST_APP_ID) == true)               { return SATR_ERROR; }

    if( subtype == SCHS_ENABLE_RELEASE ){
        sc_s_state.schs_apids_enabled[apid-1] = true; }
    else{
        sc_s_state.schs_apids_enabled[apid-1] = false; }

    return SATR_OK;
}

/**
 * Reset the schedule memory pool.
 * Marks every schedule position as invalid and eligible for allocation to a new
 * Sch_packet request. Also, release to every APID will be enabled.
 * @return the execution state.
 */
SAT_returnState operations_scheduling_reset_schedule_api(){
    
    uint8_t g = 0;
    sc_s_state.nmbr_of_ld_sched = 0;
    sc_s_state.sch_arr_full = false;
    
    /*mark every pos as !valid, = available*/
    for( ;g<SC_MAX_STORED_SCHEDULES;g++ ){
        sch_mem_pool.sc_mem_array[g].pos_taken = false;
    }
    /*enable release for all apids*/
    for( g=0;g<LAST_APP_ID-1;g++ ){
        sc_s_state.schs_apids_enabled[g] = true;
    }
    
    //TODO: reload schedules from storage?
    return SATR_OK;
}

/**
 * Inserts a given Schedule_pck on the schedule array
 * Service Subtype 4
 * @param posit, position of schedule to set.
 * @param theSchpck, the SC_pkt to insert in the schedule.
 * @return the execution state.
 */
SAT_returnState scheduling_insert_api( uint8_t posit, SC_pkt theSchpck){
    
    sch_mem_pool.sc_mem_array[posit].app_id = theSchpck.app_id;
    sch_mem_pool.sc_mem_array[posit].assmnt_type = theSchpck.assmnt_type;
    sch_mem_pool.sc_mem_array[posit].enabled = theSchpck.enabled;
    sch_mem_pool.sc_mem_array[posit].intrlck_set_id = theSchpck.intrlck_set_id;
    sch_mem_pool.sc_mem_array[posit].intrlck_ass_id = theSchpck.intrlck_ass_id;
    sch_mem_pool.sc_mem_array[posit].num_of_sch_tc = theSchpck.num_of_sch_tc;
    sch_mem_pool.sc_mem_array[posit].release_time = theSchpck.release_time;
    sch_mem_pool.sc_mem_array[posit].sch_evt = theSchpck.sch_evt;
    sch_mem_pool.sc_mem_array[posit].seq_count = theSchpck.seq_count;
    sch_mem_pool.sc_mem_array[posit].sub_schedule_id = theSchpck.sub_schedule_id;
    sch_mem_pool.sc_mem_array[posit].timeout = theSchpck.timeout;
    sch_mem_pool.sc_mem_array[posit].pos_taken = theSchpck.pos_taken;
    sch_mem_pool.sc_mem_array[posit].timeout = theSchpck.timeout;
    
    sch_mem_pool.sc_mem_array[posit].tc_pck.ack = theSchpck.tc_pck.ack;
    sch_mem_pool.sc_mem_array[posit].tc_pck.app_id = theSchpck.tc_pck.app_id;
    uint8_t i=0;
    for( ;i<theSchpck.tc_pck.len;i++){
        sch_mem_pool.sc_mem_array[posit].tc_pck.data[i] = theSchpck.tc_pck.data[i];
    }
    sch_mem_pool.sc_mem_array[posit].tc_pck.dest_id = theSchpck.tc_pck.dest_id;
    sch_mem_pool.sc_mem_array[posit].tc_pck.len = theSchpck.tc_pck.len;
    sch_mem_pool.sc_mem_array[posit].tc_pck.seq_count = theSchpck.tc_pck.seq_count;
    sch_mem_pool.sc_mem_array[posit].tc_pck.seq_flags = theSchpck.tc_pck.seq_flags;
    sch_mem_pool.sc_mem_array[posit].tc_pck.ser_subtype = theSchpck.tc_pck.ser_subtype;
    sch_mem_pool.sc_mem_array[posit].tc_pck.ser_type = theSchpck.tc_pck.ser_type;
    sch_mem_pool.sc_mem_array[posit].tc_pck.verification_state = theSchpck.tc_pck.verification_state;
    
    return SATR_OK;
}

/**
 * 
 * @return the execution state.
 */
SAT_returnState scheduling_state_api(){

    return (scheduling_enabled ? SATR_OK : SATR_SCHS_DISABLED);
}

/**
 * Removes a given Schedule_pck from the schedule array
 * Service Subtype 5.
 * Selection Criteria is destined APID and Sequence Count.
 * @param apid
 * @param seqc
 * @return the execution state.
 */
SAT_returnState scheduling_remove_schedule_api(uint8_t apid, uint16_t seqc){
    
    for(uint8_t i=0;i<SC_MAX_STORED_SCHEDULES;i++){
        if (sch_mem_pool.sc_mem_array[i].seq_count == seqc &&   
            sch_mem_pool.sc_mem_array[i].app_id == apid ){
            sch_mem_pool.sc_mem_array[i].pos_taken = false;
            sc_s_state.nmbr_of_ld_sched--;
            sc_s_state.sch_arr_full = false;
            return SATR_OK;
        }
    }
    return SATR_ERROR; /*selection criteria not met*/
}

/**
 * 
 * @param sch_mem_pool
 * @return the execution state.
 */
SAT_returnState scheduling_reset_schedule_api(SC_pkt* sch_mem_pool){
    
    for (uint8_t pos = 0; pos < SC_MAX_STORED_SCHEDULES; pos++) {
        sch_mem_pool[pos++].pos_taken = false;
        
    }
    return SATR_OK;
}

/** 
 * Time shifts selected Schedule_pck(s) on the Schedule 
 * Service Subtype 7
 * @param sch_mem_pool
 * @param apid
 * @param seqcount
 * @return the execution state.
 */
SAT_returnState time_shift_sel_schedule(uint8_t *data_v){
    
    uint8_t apid = data_v[5];
    uint8_t seqc = data_v[6];
    uint32_t ushift_time = 0;
    cnv8_32(data_v, &ushift_time);
    if(!C_ASSERT( apid < LAST_APP_ID) == true) { return SATR_ERROR; }
    /*if(!C_ASSERT( seqcnt <= MAX_SEQ_CNT) == true) { return SATR_ERROR; }*/
    for (uint8_t pos = 0; pos < SC_MAX_STORED_SCHEDULES; pos++) {
        if (sch_mem_pool.sc_mem_array[pos].seq_count == seqc &&   
            sch_mem_pool.sc_mem_array[pos].app_id == apid &&
            sch_mem_pool.sc_mem_array[pos].enabled == true ){
            switch(sch_mem_pool.sc_mem_array[pos].sch_evt){
                case REPETITIVE:
                    uint32_t rele_time = sch_mem_pool.sc_mem_array[pos].release_time;
                    uint32_t qb_time_now = return_time_QB50();
                    uint8_t neg = (ushift_time >> 31) & 0x1;
                    uint32_t shift_time_val = (~ushift_time)+ 1;
                    uint32_t new_release_t = 0;
                    if(neg){ /*then subtract it from release time*/
                        if(shift_time_val >= rele_time){ /*subtraction not possible, erroneous state*/
                            return SATR_ERROR;
                        }
                        new_release_t = rele_time - shift_time_val;
                        if((new_release_t < qb_time_now)){
                            return SATR_ERROR;
                        }
                        sch_mem_pool.sc_mem_array[pos].release_time = new_release_t;
                        return SATR_OK;
                    }
                    /*then add it to release time*/
                    new_release_t = rele_time + ushift_time;
                    if( new_release_t <= MAX_QB_SECS ){ /*to far to execute, we will not exist until then*/
                        sch_mem_pool.sc_mem_array[pos].release_time = new_release_t;
                        return SATR_OK;
                    }
                    return SATR_ERROR;
                    break;
            }
        }
    }
    return SATR_ERROR; /*schedule not found*/
}

/* Find a 'free' (non-valid schedule) position in the Schedule_pck array to
 * insert the Scheduling packet, and return its address.
 */
SC_pkt* find_schedule_pos(/*SC_pkt* sche_mem_pool*/) {

    for (uint8_t i = 0; i < SC_MAX_STORED_SCHEDULES; i++) {
        if (!sch_mem_pool.sc_mem_array[i].pos_taken) {
            return &(sch_mem_pool.sc_mem_array[i]);
        }
    }
    return NULL;
}

/**
 * Reports summary info of all telecommands from the Schedule 
 * Service Subtype 17
 * @param theSchpck
 * @return the execution state.
 */
SAT_returnState report_summary_all( SC_pkt theSchpck ){
    
    return SATR_OK;
}

/**
 * Reports detailed info about every telecommand the Schedule 
 * Service Subtype 16 
 * @param theSchpck
 * @return the execution state.
 */
SAT_returnState report_detailed( SC_pkt theSchpck ){
    
    return SATR_OK;
}

/**
 * Time shifts selected telecommands over a time period on the Schedule 
 * Service Subtype 8 
 * @param theSchpck
 * @return the execution state.
 */
SAT_returnState time_shift_sel_scheduleOTP( SC_pkt* theSchpck ){
    
    return SATR_SCHS_NOT_IMLP;
}

/**
 *  Reports summary info of a subset of telecommands from the Schedule 
 * Service Subtype 12
 * @param theSchpck
 * @return the execution state.
 */
SAT_returnState report_summary_subset( SC_pkt theSchpck ){
    
    return SATR_SCHS_NOT_IMLP;
}

/**
 * Reports detailed info about a subset of telecommands from the Schedule 
 * Service Subtype 9
 * @param theSchpck
 * @return the execution state.
 */
SAT_returnState report_detailed_subset( SC_pkt theSchpck ){
    
    return SATR_SCHS_NOT_IMLP;
}

/**
 * Loads the schedules from persistent storage. 
 * @return the execution state.
 */
SAT_returnState scheduling_service_load_schedules(){

    SAT_returnState state = SATR_ERROR;
    
    for(uint8_t s=0;s<SC_MAX_STORED_SCHEDULES;s++){

        memset(sche_tc_buffer,0x00,MAX_PKT_LEN+14+1);
        state = mass_storage_schedule_load_api(SCHS, s, sche_tc_buffer);

        if( state == SATR_OK){
            uint16_t f_s=0;
            /*read the tc's data length from the first 2 bytes*/
            cnv8_16LE(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].tc_pck.len);
            f_s+=2;
            /*start loading the sch packet*/
            sch_mem_pool.sc_mem_array[s].app_id = (TC_TM_app_id)sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].seq_count = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].enabled = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].sub_schedule_id = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].num_of_sch_tc = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].intrlck_set_id = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].intrlck_ass_id = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].assmnt_type = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].sch_evt = (SC_event_time_type) sche_tc_buffer[f_s];
            f_s+=1;
            cnv8_32(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].release_time);
            f_s+=4;
            cnv8_32(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].timeout);
            f_s+=4;
            /*TC parsing begins here*/
            sch_mem_pool.sc_mem_array[s].tc_pck.app_id = (TC_TM_app_id) sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.type = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.seq_flags = sche_tc_buffer[f_s];
            f_s+=1;
            cnv8_16LE(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].tc_pck.seq_count);
            f_s+=2;
            cnv8_16LE(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].tc_pck.len);
            f_s+=2;
            sch_mem_pool.sc_mem_array[s].tc_pck.ack = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.ser_type = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.ser_subtype = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.dest_id = (TC_TM_app_id) sche_tc_buffer[f_s];
            f_s+=1;
            /*copy tc payload data*/
            uint16_t i = 0;
            for(;i<sch_mem_pool.sc_mem_array[s].tc_pck.len;i++){
                sch_mem_pool.sc_mem_array[s].tc_pck.data[i] = sche_tc_buffer[f_s];
                f_s+=1;
            }
            sch_mem_pool.sc_mem_array[s].tc_pck.verification_state = (SAT_returnState) sche_tc_buffer[f_s];
            f_s+=1;
            uint8_t l_chk = sche_tc_buffer[f_s];
            
            uint8_t chk = 0;
            for(uint16_t l=0;l<f_s-1;l++){
                chk = chk ^ sche_tc_buffer[l];
            }
            
            if( l_chk == chk && chk!= 0x0 ){
                sch_mem_pool.sc_mem_array[s].pos_taken = true;
                sch_mem_pool.sc_mem_array[s].enabled = true;
            }
            else{ 
                sch_mem_pool.sc_mem_array[s].pos_taken = false;
                sch_mem_pool.sc_mem_array[s].enabled = false;
            }
        }
    }
    return SATR_OK;
}

/**
 * Saves current active schedules on permanent storage.
 * @return the execution state.
 */
SAT_returnState scheduling_service_save_schedules(){

    /*convert the Schedule packet from Schedule_pkt_pool format to an array of linear bytes*/
    for(uint8_t s=0;s<SC_MAX_STORED_SCHEDULES;s++){
        if( sch_mem_pool.sc_mem_array[s].pos_taken == true &&
            sch_mem_pool.sc_mem_array[s].enabled   == true){
            
            memset(sche_tc_buffer,0x00,MAX_PKT_LEN+14+1);
            uint16_t f_s=0;
            /*save the tc's data length in the first 2 bytes*/
            cnv16_8(sch_mem_pool.sc_mem_array[s].tc_pck.len, &sche_tc_buffer[f_s]);
            f_s+=2;
            /*start saving sch packet*/
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].app_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].seq_count;
            f_s+=1;
    //        cnv16_8(sch_mem_pool.sc_mem_array[s].seq_count, &sche_tc_buffer[3]);
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].enabled;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].sub_schedule_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].num_of_sch_tc;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].intrlck_set_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].intrlck_ass_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].assmnt_type;
            f_s+=1;
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].sch_evt;
            f_s+=1;
            cnv32_8(sch_mem_pool.sc_mem_array[s].release_time,&sche_tc_buffer[f_s]); //11
            f_s+=4;
            cnv32_8(sch_mem_pool.sc_mem_array[s].timeout,&sche_tc_buffer[f_s]); //15
            f_s+=4;
            /*TC parsing begins here*/
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].tc_pck.app_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.type;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.seq_flags;
            f_s+=1;
            cnv16_8(sch_mem_pool.sc_mem_array[s].tc_pck.seq_count,&sche_tc_buffer[f_s]);
            f_s+=2;
            cnv16_8(sch_mem_pool.sc_mem_array[s].tc_pck.len,&sche_tc_buffer[f_s]);
            f_s+=2;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.ack;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.ser_type;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.ser_subtype;
            f_s+=1;
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].tc_pck.dest_id;
            f_s+=1;
            /*copy tc payload data*/
            uint16_t i = 0;
            for(;i<sch_mem_pool.sc_mem_array[s].tc_pck.len;i++){
                sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.data[i];
                f_s+=1;
            }
//            f_s+=f_s+i;
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].tc_pck.verification_state;
            f_s+=1;
            uint8_t chk = 0;
            for(uint16_t l=0;l<f_s-1;l++){
                chk = chk ^ sche_tc_buffer[l];
            }
            sche_tc_buffer[f_s] = chk;
            f_s+=1;
            mass_storage_storeFile(SCHS,s,sche_tc_buffer,&f_s);
        }
    }
    return SATR_OK;
}

/**
 * Extracts the inner TC packet from the Sch_pkt structure
 * @param buf is the source of the data.
 * @param pkt the inner tc_tm_pkt to be created.
 * @param size
 * @return the execution state.
 */
SAT_returnState copy_inner_tc(const uint8_t *buf, tc_tm_pkt *pkt, const uint16_t size) {

    uint8_t tmp_crc[2];
    uint8_t ver, dfield_hdr, ccsds_sec_hdr, tc_pus;
    if(!C_ASSERT(buf != NULL && pkt != NULL && pkt->data != NULL) == true)  { return SATR_ERROR; }
    if(!C_ASSERT(size < MAX_PKT_SIZE) == true)                              { return SATR_ERROR; }

    tmp_crc[0] = buf[size - 1];
    checkSum(buf, size-2, &tmp_crc[1]); /* -2 for excluding the checksum bytes*/
    ver = buf[0] >> 5;
    pkt->type = (buf[0] >> 4) & 0x01;
    dfield_hdr = (buf[0] >> 3) & 0x01;

    pkt->app_id = (TC_TM_app_id)buf[1];

    pkt->seq_flags = buf[2] >> 6;

    cnv8_16((uint8_t*)&buf[2], &pkt->seq_count);
    pkt->seq_count &= 0x3FFF;

    cnv8_16((uint8_t*)&buf[4], &pkt->len);

    ccsds_sec_hdr = buf[6] >> 7;

    tc_pus = buf[6] >> 4;

    pkt->ack = 0x04 & buf[6];

    pkt->ser_type = buf[7];
    pkt->ser_subtype = buf[8];
    pkt->dest_id = (TC_TM_app_id)buf[9];

    pkt->verification_state = SATR_PKT_INIT;

    if(!C_ASSERT(pkt->app_id < LAST_APP_ID) == true) {
        pkt->verification_state = SATR_PKT_ILLEGAL_APPID;
        return SATR_PKT_ILLEGAL_APPID; 
    }

    if(!C_ASSERT(pkt->len == size - ECSS_HEADER_SIZE - 1) == true) {
        pkt->verification_state = SATR_PKT_INV_LEN;
        return SATR_PKT_INV_LEN; 
    }
    pkt->len = pkt->len - ECSS_DATA_HEADER_SIZE - ECSS_CRC_SIZE + 1;

    if(!C_ASSERT(tmp_crc[0] == tmp_crc[1]) == true) {
        pkt->verification_state = SATR_PKT_INC_CRC;
        return SATR_PKT_INC_CRC; 
    }

    if(!C_ASSERT(services_verification_TC_TM[pkt->ser_type][pkt->ser_subtype][pkt->type] == 1) == true) { 
        pkt->verification_state = SATR_PKT_ILLEGAL_PKT_TP;
        return SATR_PKT_ILLEGAL_PKT_TP; 
    }

    if(!C_ASSERT(ver == ECSS_VER_NUMBER) == true) {
        pkt->verification_state = SATR_ERROR;
        return SATR_ERROR; 
    }

    if(!C_ASSERT(tc_pus == ECSS_PUS_VER) == true) {
        pkt->verification_state = SATR_ERROR;
        return SATR_ERROR;
    }

    if(!C_ASSERT(ccsds_sec_hdr == ECSS_SEC_HDR_FIELD_FLG) == true) {
        pkt->verification_state = SATR_ERROR;
        return SATR_ERROR;
    }

    if(!C_ASSERT(pkt->type == TC || pkt->type == TM) == true) {
        pkt->verification_state = SATR_ERROR;
        return SATR_ERROR;
    }

    if(!C_ASSERT(dfield_hdr == ECSS_DATA_FIELD_HDR_FLG) == true) {
        pkt->verification_state = SATR_ERROR;
        return SATR_ERROR;
    }

    if(!C_ASSERT(pkt->ack == TC_ACK_NO || pkt->ack == TC_ACK_ACC) == true) {
        pkt->verification_state = SATR_ERROR;
        return SATR_ERROR;
    }

    if(!C_ASSERT(pkt->seq_flags == TC_TM_SEQ_SPACKET) == true) {
        pkt->verification_state = SATR_ERROR;
        return SATR_ERROR;
    }    

    for(int i = 0; i < pkt->len; i++) {
        pkt->data[i] = buf[ECSS_DATA_OFFSET+i];
    }

    return SATR_OK;
}

/**
 * 
 * @param sc_pkt
 * @param tc_pkt
 * @return the execution state.
 */
SAT_returnState parse_sch_packet(SC_pkt *sc_pkt, tc_tm_pkt *tc_pkt) {

    /*extract the packet and route accordingly*/
    uint32_t time = 0;
    uint32_t exec_timeout = 0;
    uint8_t offset = 14;

    /*extract the scheduling packet from the data pointer*/
    (*sc_pkt).sub_schedule_id = tc_pkt->data[0];
    if (!C_ASSERT((*sc_pkt).sub_schedule_id == 1) == true) {
        return SATR_SCHS_ID_INVALID;
    }

    (*sc_pkt).num_of_sch_tc = tc_pkt->data[1];
    if (!C_ASSERT((*sc_pkt).num_of_sch_tc == 1) == true) {

        return SATR_SCHS_NMR_OF_TC_INVLD;
    }

    (*sc_pkt).intrlck_set_id = tc_pkt->data[2];
    if (!C_ASSERT((*sc_pkt).intrlck_set_id == 0) == true) {

        return SATR_SCHS_INTRL_ID_INVLD;
    }

    (*sc_pkt).intrlck_ass_id = tc_pkt->data[3];
    if (!C_ASSERT((*sc_pkt).intrlck_ass_id == 0) == true) {

        return SATR_SCHS_ASS_INTRL_ID_INVLD;
    }

    (*sc_pkt).assmnt_type = tc_pkt->data[4];
    if (!C_ASSERT((*sc_pkt).assmnt_type == 1) == true) {

        return SATR_SCHS_ASS_TP_ID_INVLD;
    }

    (*sc_pkt).sch_evt = (SC_event_time_type) tc_pkt->data[5];
    if (!C_ASSERT((*sc_pkt).sch_evt < LAST_EVENTTIME) == true) {

        return SATR_SCHS_RLS_TT_ID_INVLD;
    }
    
    /*7,8,9,10th bytes are the time fields, combine them to a uint32_t*/
    time = (time | tc_pkt->data[9]) << 8;
    time = (time | tc_pkt->data[8]) << 8;
    time = (time | tc_pkt->data[7]) << 8;
    time = (time | tc_pkt->data[6]);
    /*read execution time out fields*/
    exec_timeout = (exec_timeout | tc_pkt->data[13]) << 8;
    exec_timeout = (exec_timeout | tc_pkt->data[12]) << 8;
    exec_timeout = (exec_timeout | tc_pkt->data[11]) << 8;
    exec_timeout = (exec_timeout | tc_pkt->data[10]);

    if( exec_timeout < MIN_SCH_REP_INTRVL ) { 
        SYSVIEW_PRINT("SCHS PKT REJECTED LOW REP INTRVL");
        return SATR_SCHS_TIM_INVLD; }
    
    /*extract data from internal TC packet ( app_id )*/
    (*sc_pkt).app_id = (TC_TM_app_id)tc_pkt->data[offset + 1];
    if (!C_ASSERT((*sc_pkt).app_id < LAST_APP_ID) == true){
        return SATR_PKT_ILLEGAL_APPID;
    }
    (*sc_pkt).seq_count = (*sc_pkt).seq_count | (tc_pkt->data[offset + 2] >> 2);
    (*sc_pkt).seq_count << 8;
    (*sc_pkt).seq_count = (*sc_pkt).seq_count | (tc_pkt->data[offset + 3]);
    if(check_existing((*sc_pkt).app_id, (*sc_pkt).seq_count)){ 
        SYSVIEW_PRINT("SCHS PKT REJECTED, ALREADY EXISTS");
        return SATR_SCHS_INTRL_LGC_ERR; }
    
    (*sc_pkt).release_time = time;
    (*sc_pkt).timeout = exec_timeout;
    (*sc_pkt).pos_taken = true;
    (*sc_pkt).enabled = true;

    /*copy the internal TC packet for future use*/
    /*  tc_pkt is a TC containing 14 bytes of data related to scheduling service.
     *  After those 14 bytes, a 'whole_inner_tc' packet starts.
     *  
     *  The 'whole_inner_tc' offset in the tc_pkt's data payload is: 15 (16th byte).
     *  
     *  The length of the 'whole_inner_tc' is tc_pkt->data - 14 bytes
     *  
     *  Within the 'whole_inner_tc' the length of the 'inner' goes for:
     *  16+16+16+32+(tc_pkt->len - 11)+16 bytes.
     */
    return copy_inner_tc( &(tc_pkt->data[14]), &((*sc_pkt).tc_pck), (uint16_t) tc_pkt->len - 14);
}

/**
 * Checks if a scheduling packets with the same APID and sequence count exists
 * on schedules pool and it is enabled/valid.
 * @param apid
 * @param seqc
 * @return 
 */
uint8_t check_existing(uint8_t apid, uint8_t seqc){
    for(uint8_t i = 0; i < SC_MAX_STORED_SCHEDULES; i++){
        if(sch_mem_pool.sc_mem_array[i].pos_taken == true && /*if a valid schedule exists*/
            sch_mem_pool.sc_mem_array[i].enabled == true){
            if( sch_mem_pool.sc_mem_array[i].app_id == apid &&
                sch_mem_pool.sc_mem_array[i].seq_count == seqc ){
                return true;
            }
        }
    }
    return false;
}