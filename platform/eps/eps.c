#include "eps.h"
#include "eps_state.h"
#include "eps_time.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#undef __FILE_ID__
#define __FILE_ID__ 21


//extern SAT_returnState export_pkt(TC_TM_app_id app_id, struct uart_data *data);

extern SAT_returnState free_pkt(tc_tm_pkt *pkt);

extern SAT_returnState verification_app(tc_tm_pkt *pkt);
extern SAT_returnState hk_app(tc_tm_pkt *pkt);
extern SAT_returnState function_management_app(tc_tm_pkt *pkt);
extern SAT_returnState test_app(tc_tm_pkt *pkt);

extern EPS_State eps_board_state;


const uint8_t services_verification_EPS_TC[MAX_SERVICES][MAX_SUBTYPES] = {
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
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_MASS_STORAGE_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /*TC_TEST_SERVICE*/
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

struct _eps_data eps_data;

static struct _sys_data sys_data;

uint32_t comms_update=0;
uint32_t adcs_update=0;
uint32_t obc_update=0;

SAT_returnState route_pkt(tc_tm_pkt *pkt) {

    SAT_returnState res;
    TC_TM_app_id id;

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)                         { return SATR_ERROR; }
    if(!C_ASSERT(pkt->type == TC || pkt->type == TM) == true)                       { return SATR_ERROR; }
    if(!C_ASSERT(pkt->app_id < LAST_APP_ID && pkt->dest_id < LAST_APP_ID) == true)  { return SATR_ERROR; }

    if(pkt->type == TC)         { id = pkt->app_id; }
    else if(pkt->type == TM)    { id = pkt->dest_id; }

    if(id == COMMS_APP_ID)      { comms_update = EPS_time_counter_get(); }
    else if(id == ADCS_APP_ID)  { adcs_update = EPS_time_counter_get(); }
    else if(id == OBC_APP_ID)   { obc_update = EPS_time_counter_get(); }

    if(id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
        res = hk_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_FUNCTION_MANAGEMENT_SERVICE) {
        res = function_management_app(pkt);
    } else if(id == SYSTEM_APP_ID && pkt->ser_type == TC_TEST_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 1 || pkt->ser_subtype == 2 || pkt->ser_subtype == 9 || pkt->ser_subtype == 11 || pkt->ser_subtype == 12 || pkt->ser_subtype == 13) { free_pkt(pkt); return SATR_ERROR; }
        res = test_app(pkt);
    }
    else if(id == COMMS_APP_ID)    { queuePush(pkt, OBC_APP_ID); }
    else if(id == ADCS_APP_ID)     { queuePush(pkt, OBC_APP_ID); }
    else if(id == OBC_APP_ID)      { queuePush(pkt, OBC_APP_ID); }
    else if(id == GND_APP_ID)      { queuePush(pkt, OBC_APP_ID); }
    else if(id == DBG_APP_ID)      { queuePush(pkt, OBC_APP_ID); }

    return SATR_OK;
}

SAT_returnState event_log(uint8_t *buf, const uint16_t size) {
    return SATR_OK;
}

SAT_returnState check_timeouts() {

	/*subsystem reset flags - set clear once in startup*/
	static subsystem_reset_state obc_reset_flag = SUBSYSTEM_RESET_CLEAR;
	static subsystem_reset_state adcs_reset_flag = SUBSYSTEM_RESET_CLEAR;
	static subsystem_reset_state comms_reset_flag = SUBSYSTEM_RESET_CLEAR;

	/*get current systick time*/
	uint32_t t_now = EPS_time_counter_get();

	/*check reset flags for all subsystems*/

	/*if OBC reset flag is set:*/
	if(obc_reset_flag==SUBSYSTEM_RESET_SET){

		/*clear reset flag*/
		obc_reset_flag=SUBSYSTEM_RESET_CLEAR;
		/*power on subsystem*/
		EPS_set_rail_switch(OBC, EPS_SWITCH_RAIL_ON, &eps_board_state);
		/*update reset time*/
		obc_update = t_now;
	}

	/*if ADCS reset flag is set:*/
	if(adcs_reset_flag==SUBSYSTEM_RESET_SET){

		/*clear reset flag*/
		adcs_reset_flag=SUBSYSTEM_RESET_CLEAR;
		/*power on subsystem*/
		EPS_set_rail_switch(ADCS, EPS_SWITCH_RAIL_ON, &eps_board_state);
		/*update reset time*/
		adcs_update = t_now;
	}

	/*if COMMS reset flag is set:*/
	if(comms_reset_flag==SUBSYSTEM_RESET_SET){

		/*clear reset flag*/
		comms_reset_flag=SUBSYSTEM_RESET_CLEAR;
		/*power on subsystem*/
		EPS_set_rail_switch(COMM, EPS_SWITCH_RAIL_ON, &eps_board_state);
		/*update reset time*/
		comms_update = t_now;
	}



	/*check time elapsed for all subsystems*/


	/*if OBC time passed >10min since last time update (as performed in route packet)
	 * ad if OBC subsystem is powered on - initiate reset and set subsystem reset flag*/
	if( t_now - obc_update > SUBSYSTEM_TIMEOUT_PERIOD ) {

		/*reset subsystem*/
		/*if obc subsystem is powered on - set reset flag and power off - in order to avoid to power up a subsystem that should be turned down!*/
		if(EPS_get_rail_switch_status(OBC)==EPS_SWITCH_RAIL_ON){
			obc_reset_flag=SUBSYSTEM_RESET_SET;
			/*power down subsystem*/
			EPS_set_rail_switch(OBC, EPS_SWITCH_RAIL_OFF, &eps_board_state);
		}
	}
	/*if ADCS time passed >10min since last time update (as performed in route packet)
	 * ad if ADCS subsystem is powered on - initiate reset and set subsystem reset flag*/
	if( t_now - adcs_update > SUBSYSTEM_TIMEOUT_PERIOD ) {

		/*reset subsystem*/
		/*if adcs subsystem is powered on - set reset flag and power off - in order to avoid to power up a subsystem that should be turned down!*/
		if(EPS_get_rail_switch_status(ADCS)==EPS_SWITCH_RAIL_ON){
			adcs_reset_flag=SUBSYSTEM_RESET_SET;
			/*power down subsystem*/
			EPS_set_rail_switch(ADCS, EPS_SWITCH_RAIL_OFF, &eps_board_state);
		}
	}
	/*if COMMS time passed >10min since last time update (as performed in route packet)
	 * ad if COMM subsystem is powered on - initiate reset and set subsystem reset flag*/
	if( t_now - comms_update > SUBSYSTEM_TIMEOUT_PERIOD ) {

		/*reset subsystem*/
		/*if comms subsystem is powered on - set reset flag and power off - in order to avoid to power up a subsystem that should be turned down!*/
		if(EPS_get_rail_switch_status(COMM)==EPS_SWITCH_RAIL_ON){
			comms_reset_flag=SUBSYSTEM_RESET_SET;
			/*power down subsystem*/
			EPS_set_rail_switch(COMM, EPS_SWITCH_RAIL_OFF, &eps_board_state);
		}
	}

	return SATR_OK;

}

SAT_returnState time_management_app(tc_tm_pkt *pkt) {
    return SATR_ERROR;
}
