#ifndef SCHEDULING_SERVICE_H
#define SCHEDULING_SERVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "service_utilities.h"
#include "time_management_service.h"
#include "services.h"
#include "pkt_pool.h"

#define SCHEDULING_SERVICE_V 0.1

#define MIN_VALID_QB50_SECS 2678400


/* Declares the maximum available space for 
 * on-memory loaded schedule commands
 */
#define SC_MAX_STORED_SCHEDULES 15

/* For every APID there is a number of maximum 255 sequence counts,
 * The combination of APID and seq_count acts as 'primary key' for actions
 * that require selections upon the scheduling packets. 
 */
#define MAX_SEQ_CNT             256

typedef enum {
    /* The 'release_time' member
     * specified on the Scheduling_pck is absolute to OBC boot time.
     */
    ABSOLUTE        = 0, 
    
    /* The 'release_time' member
     * specified on the Scheduling_pck is relative to the schedule's
     * activation time.
     */
    SCHEDULE        = 1, 
    
    /* The 'release_time' member
     * specified on the Scheduling_pck is relative to the sub-schedule's
     * activation time.
     */
    SUBSCHEDULE     = 2, 
    
    /* The 'release_time' member
     * specified on the Scheduling_pck is relative to the notification time
     * of success of failure of interlocked schedule.
     * time.
     */
    INTERLOCK       = 3,
    
    /* The 'release_time' member
     * specified on the Scheduling_pck is absolute to OBC QB50 time, but when
     * it is fired for execution its automatically re-sets itself for future
     * execution. In case that repetition time is zero, its on-time only.
     */
    REPETITIVE      = 4,
    LAST_EVENTTIME  = 5
            
}SC_event_time_type;
 
/* (Page 105-106 of ECSS-E-70-41A document)
 * Schedule command structure:
 */
typedef struct {    
        /* This is the application id that the telecommand it is destined to.
         * This info will be extracted from the encapsulated TC packet.
         */
    TC_TM_app_id app_id;
    
        /* This is the sequence count of the telecommand packet.
         * This info will be extracted from the encapsulated TC packet.
         */
    uint16_t seq_count;
    
        /* If the specific schedule command is enabled.
         * Enabled = 1, Disabled = 0.
         */
    uint8_t enabled;
    
        /* Currently not supported by this implementation.*
         * For this specific implementation is set to 1 (one)
         * for every schedule packet.
         */
    uint8_t sub_schedule_id;
    
        /* Number of telecommands in the schedule.
         * For this specific implementation is set to 1 (one) Telecommand
         * per Schedule Packet.
         */
    uint8_t num_of_sch_tc;
    
        /* The number of interlock id to be set by this telecommand.
         * For this specific implementation is set to 0 (zero)
         * for every schedule packet.
         */
    uint8_t intrlck_set_id ;
    
        /* The number of interlock that this telecommand is dependent on.
         * For this specific implementation is set to 0 (zero)
         * for every schedule packet.
         */
    uint8_t intrlck_ass_id ;
    
        /* Success or failure of the dependent telecommand, Success=1, Failure=0
         * For this specific implementation is set to 1 (success)
         * for every schedule packet.
         */
    uint8_t assmnt_type;
    
        /* Determines the release type for the telecommand.
         * See: SC_event_time_type
         */
    SC_event_time_type sch_evt;
    
        /* Absolute or relative time of telecommand execution,
         * this field has meaning relative to sch_evt member.
         */
    uint32_t release_time;
    
        /* This is a delta time which when added to the release time of the scheduled telecommand, the command
         * is expected to complete execution.
         * Timeout execution is only set if telecommand sets interlocks, so for our
         * current implementation will be always 0 (zero)
         */
    uint32_t timeout;
    
        /* The actual telecommand packet to be scheduled and executed
         * 
         */
    tc_tm_pkt tc_pck;
        
        /* Declares a schedule position as pos_taken or !pos_taken.
         * If a schedule position is noted as !pos_taken, it can be replaced 
         * by a new SC_pkt packet.
         * When a schedule goes for execution, 
         * automatically its position becomes !pos_taken (exception for repetitive SC_pkt(s) ).
         * pos_taken=true, 1, !pos_taken=false ,0
         */
    uint8_t pos_taken;
    
}SC_pkt;

typedef struct{    
    /*Holds structures, containing Scheduling Telecommands*/
    SC_pkt sc_mem_array[SC_MAX_STORED_SCHEDULES];
    
    /* Memory array for inner TC data payload
     * One to one mapping with the sc_mem_array
     */
    uint8_t innerd_tc_data[SC_MAX_STORED_SCHEDULES][MAX_PKT_LEN];
    
}Schedule_pkt_pool;

extern Schedule_pkt_pool sch_mem_pool;

typedef struct {
    /*Number of loaded schedules*/
    uint8_t nmbr_of_ld_sched;
    
    /* Defines the state of the Scheduling service,
    * if enabled the release of TC is running.
    * Enable = true, 1
    * Disabled = false, 0
    */
    uint8_t scheduling_service_enabled;
    
    /* Schedules memory pool is full.
     * Full = true, 1
     * space avaliable = false, 0 
     */
    uint8_t schedule_arr_full;
    
    /* This array holds the value 1 (True), if 
     * the specified APID scheduling is enabled.
     * OBC_APP_ID = 1, starts at zero index.
     */
    uint8_t scheduling_apids_enabled[LAST_APP_ID-1];
    
}Scheduling_service_state;

extern Scheduling_service_state sc_s_state;

static uint8_t scheduling_enabled = true;

SAT_returnState cross_schedules();

SAT_returnState scheduling_service_init();

SAT_returnState scheduling_app(tc_tm_pkt* spacket);

SAT_returnState scheduling_state_api();

/* Enables / Disables the scheduling execution as a service.
 * Enable state = 1
 * Disable state = 0
 * Return R_OK, on successful state alteration.
 */
SAT_returnState edit_schedule_stateAPI(tc_tm_pkt* spacket);

SAT_returnState enable_disable_schedule_apid_release( uint8_t subtype, uint8_t apid );

SAT_returnState operations_scheduling_reset_schedule_api();

SAT_returnState scheduling_insert_api( uint8_t posit, SC_pkt theSchpck );

SAT_returnState scheduling_remove_schedule_api( /*SC_pkt* sch_mem_pool,  
                                                SC_pkt* theSchpck, */ uint8_t apid, uint16_t seqc );

/* Remove Schedule_pck from schedule over a time period (OTP)
 * * Service Subtype 6
 */
SAT_returnState remove_from_scheduleOTPAPI( SC_pkt theSchpck );

SAT_returnState scheduling_time_shift_all_schedules_api( SC_pkt* sch_mem_pool, int32_t secs );

SAT_returnState parse_sch_packet( SC_pkt *sc_pkt, tc_tm_pkt *tc_pkt );

SAT_returnState time_shift_all_tcs(uint8_t *time_v);

SAT_returnState scheduling_service_save_schedules();

#endif

