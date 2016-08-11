#include "time_management_service.h"

#undef __FILE_ID__
#define __FILE_ID__ 2


#if (SYSTEM_APP_ID == ADCS_APP_ID_)
#include "adcs_hal.h"
#else if (SYSTEM_APP_ID == OBC_APP_ID_)
#include "obc_hal.h"
#endif


static const uint32_t UTC_QB50_YM[MAX_YEAR][13] = {    
 /*2000*/{0 , 0, 2678400, 5184000, 7862400, 10454400, 13132800, 15724800, 18403200, 21081600, 23673600, 26352000, 28944000},
 /*2001*/{0 , 31622400, 34300800, 36720000, 39398400, 41990400, 44668800, 47260800, 49939200, 52617600, 55209600, 57888000, 60480000},
 /*2002*/{0 , 63158400, 65836800, 68256000, 70934400, 73526400, 76204800, 78796800, 81475200, 84153600, 86745600, 89424000, 92016000},
 /*2003*/{0 , 94694400, 97372800, 99792000, 102470400, 105062400, 107740800, 110332800, 113011200, 115689600, 118281600, 120960000, 123552000},
 /*2004*/{0 , 126230400, 128908800, 131414400, 134092800, 136684800, 139363200, 141955200, 144633600, 147312000, 149904000, 152582400, 155174400},
 /*2005*/{0 , 157852800, 160531200, 162950400, 165628800, 168220800, 170899200, 173491200, 176169600, 178848000, 181440000, 184118400, 186710400},
 /*2006*/{0 , 189388800, 192067200, 194486400, 197164800, 199756800, 202435200, 205027200, 207705600, 210384000, 212976000, 215654400, 218246400},
 /*2007*/{0 , 220924800, 223603200, 226022400, 228700800, 231292800, 233971200, 236563200, 239241600, 241920000, 244512000, 247190400, 249782400},
 /*2008*/{0 , 252460800, 255139200, 257644800, 260323200, 262915200, 265593600, 268185600, 270864000, 273542400, 276134400, 278812800, 281404800},
 /*2009*/{0 , 284083200, 286761600, 289180800, 291859200, 294451200, 297129600, 299721600, 302400000, 305078400, 307670400, 310348800, 312940800},
 /*2010*/{0 , 315619200, 318297600, 320716800, 323395200, 325987200, 328665600, 331257600, 333936000, 336614400, 339206400, 341884800, 344476800},
 /*2011*/{0 , 347155200, 349833600, 352252800, 354931200, 357523200, 360201600, 362793600, 365472000, 368150400, 370742400, 373420800, 376012800},
 /*2012*/{0 , 378691200, 381369600, 383875200, 386553600, 389145600, 391824000, 394416000, 397094400, 399772800, 402364800, 405043200, 407635200},
 /*2013*/{0 , 410313600, 412992000, 415411200, 418089600, 420681600, 423360000, 425952000, 428630400, 431308800, 433900800, 436579200, 439171200},
 /*2014*/{0 , 441849600, 444528000, 446947200, 449625600, 452217600, 454896000, 457488000, 460166400, 462844800, 465436800, 468115200, 470707200},
 /*2015*/{0 , 473385600, 476064000, 478483200, 481161600, 483753600, 486432000, 489024000, 491702400, 494380800, 496972800, 499651200, 502243200},
 /*2016*/{0 , 504921600, 507600000, 510105600, 512784000, 515376000, 518054400, 520646400, 523324800, 526003200, 528595200, 531273600, 533865600},
 /*2017*/{0 , 536544000, 539222400, 541641600, 544320000, 546912000, 549590400, 552182400, 554860800, 557539200, 560131200, 562809600, 565401600},
 /*2018*/{0 , 568080000, 570758400, 573177600, 575856000, 578448000, 581126400, 583718400, 586396800, 589075200, 591667200, 594345600, 596937600},
 /*2019*/{0 , 599616000, 602294400, 604713600, 607392000, 609984000, 612662400, 615254400, 617932800, 620611200, 623203200, 625881600, 628473600},
 /*2020*/{0 , 631152000, 633830400, 636336000, 639014400, 641606400, 644284800, 646876800, 649555200, 652233600, 654825600, 657504000, 660096000},  
                                                   };

static const uint32_t UTC_QB50_D[32] = 
        { 0, 86400, 172800, 259200, 345600, 432000, 518400, 604800, 691200, 777600, 864000, 950400, 1036800, 1123200, 1209600,
          1296000, 1382400, 1468800, 1555200, 1641600, 1728000, 1814400, 1900800, 1987200, 2073600, 2160000, 2246400, 2332800,
          2419200, 2505600, 2592000, 2678400
        };

const uint32_t UTC_QB50_H[25] = 
        { 0, 3600, 7200, 10800, 14400, 18000, 21600, 25200, 28800, 32400, 36000, 39600, 43200, 46800, 50400, 54000, 57600,
          61200, 64800, 68400, 72000, 75600, 79200, 82800, 86400
        };

SAT_returnState time_management_app(tc_tm_pkt *pkt){
    
    uint8_t ser_subtype;
    struct time_utc temp_time;

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) { return SATR_ERROR; }
	
    ser_subtype = pkt->ser_subtype;
    
    if(!C_ASSERT(ser_subtype == TM_TIME_SET_IN_UTC     ||\
                 ser_subtype == TM_TIME_SET_IN_QB50    ||\
                 ser_subtype == TM_REPORT_TIME_IN_QB50 ||\
                 ser_subtype == TM_REPORT_TIME_IN_UTC  ||\
                 ser_subtype == TM_TIME_REPORT_IN_UTC  ||\
                 ser_subtype == TM_TIME_REPORT_IN_QB50 ) == true) { return SATR_ERROR; }
    
    if( ser_subtype == TM_TIME_SET_IN_QB50){
        /*set time from 2000 epoch*/
        pkt->verification_state = SATR_ERROR;
    }
    else
    if( ser_subtype == TM_TIME_SET_IN_UTC){
        /*set time in utc mode*/        
        if(!(C_ASSERT(pkt->data[0]>= 1)&&C_ASSERT(pkt->data[0] < 8)) == true)  { return SATR_ERROR; } /*weekday1to7*/
        if(!(C_ASSERT(pkt->data[1] > 0)&&C_ASSERT(pkt->data[1] < 32)) == true) { return SATR_ERROR; } /*day1to31*/
        if(!(C_ASSERT(pkt->data[2] > 0)&&C_ASSERT(pkt->data[2] < 13)) == true) { return SATR_ERROR; } /*month1to12*/
        if(!(C_ASSERT(pkt->data[3]>= 0)&&C_ASSERT(pkt->data[3] < 100)) == true){ return SATR_ERROR; } /*year0to99*/
        if(!(C_ASSERT(pkt->data[4]>= 0)&&C_ASSERT(pkt->data[4] < 24)) == true) { return SATR_ERROR; } /*hours0to23*/
        if(!(C_ASSERT(pkt->data[5]>= 0)&&C_ASSERT(pkt->data[5] < 60)) == true) { return SATR_ERROR; } /*minutes0to59*/
        if(!(C_ASSERT(pkt->data[6]>= 0)&&C_ASSERT(pkt->data[6] < 60)) == true) { return SATR_ERROR; } /*seconds0to59*/
        temp_time.weekday = pkt->data[0];
        temp_time.day = pkt->data[1];  temp_time.month = pkt->data[2];
        temp_time.year = pkt->data[3]; temp_time.hour = pkt->data[4];
        temp_time.min = pkt->data[5];  temp_time.sec = pkt->data[6];
        set_time_UTC(temp_time);
        pkt->verification_state = SATR_OK;
    }
    else
    if( ser_subtype == TM_REPORT_TIME_IN_QB50){
        
        tc_tm_pkt *time_rep_pkt = get_pkt(PKT_NORMAL);
        /*make the packet to send*/
        time_management_report_time_in_qb50( time_rep_pkt, (TC_TM_app_id)pkt->dest_id);
        pkt->verification_state = SATR_OK;
        if(!C_ASSERT(time_rep_pkt != NULL) == true) { return SATR_ERROR; }
        route_pkt(time_rep_pkt);
    }
    else
    if( ser_subtype == TM_REPORT_TIME_IN_UTC){
        
        tc_tm_pkt *time_rep_pkt = get_pkt(PKT_NORMAL);
        if(!C_ASSERT(time_rep_pkt != NULL) == true) { return SATR_ERROR; }
        time_management_report_time_in_utc( time_rep_pkt, (TC_TM_app_id)pkt->dest_id);
        pkt->verification_state = SATR_OK;
        route_pkt(time_rep_pkt);
    }
    else
    if( ser_subtype == TM_TIME_REPORT_IN_UTC){
        /* time report from a time_management_service implementor in UTC format exists here,
         * user should implement his own code to handle the time report response*/
        /*set time in utc mode*/
        if(!(C_ASSERT(pkt->data[0]>= 1) &&C_ASSERT(pkt->data[0] < 8)) == true)  { return SATR_ERROR; } /*weekday1to7*/
        if(!(C_ASSERT(pkt->data[1] > 0) &&C_ASSERT(pkt->data[1] < 32)) == true) { return SATR_ERROR; } /*day1to31*/
        if(!(C_ASSERT(pkt->data[2] > 0) &&C_ASSERT(pkt->data[2] < 13)) == true) { return SATR_ERROR; } /*month1to12*/
        if(!(C_ASSERT(pkt->data[3]>= 15)&&C_ASSERT(pkt->data[3] < 100)) == true){ return SATR_ERROR; } /*assert if year before or equal 2015*/
        if(!(C_ASSERT(pkt->data[4]>= 0) &&C_ASSERT(pkt->data[4] < 24)) == true) { return SATR_ERROR; } /*hours0to23*/
        if(!(C_ASSERT(pkt->data[5]>= 0) &&C_ASSERT(pkt->data[5] < 60)) == true) { return SATR_ERROR; } /*minutes0to59*/
        if(!(C_ASSERT(pkt->data[6]>= 0) &&C_ASSERT(pkt->data[6] < 60)) == true) { return SATR_ERROR; } /*seconds0to59*/
        temp_time.weekday = pkt->data[0];
        temp_time.day = pkt->data[1];  temp_time.month = pkt->data[2];
        temp_time.year = pkt->data[3]; temp_time.hour = pkt->data[4];
        temp_time.min = pkt->data[5];  temp_time.sec = pkt->data[6];
        set_time_UTC(temp_time);
        pkt->verification_state = SATR_OK;
    }
    else
    if( ser_subtype == TM_TIME_REPORT_IN_QB50){
        /*time report from a time_management_service implementor in QB50 format exists here,
         * user should implement his own code to handle the time report response*/
        pkt->verification_state = SATR_ERROR;
    }
    
    return SATR_OK;
}

/**
 * Reports time in QB50 epoch format (seconds from 2000)
 * @param pkt
 * @param dest_id
 * @return 
 */
SAT_returnState time_management_report_time_in_qb50(tc_tm_pkt *pkt, TC_TM_app_id dest_id){

    uint32_t qb_temp_secs = 0;
    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }
    get_time_QB50(&qb_temp_secs);    
    crt_pkt(pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_TIME_MANAGEMENT_SERVICE, TM_TIME_REPORT_IN_QB50, dest_id);
    cnv32_8(qb_temp_secs, pkt->data);
    pkt->len = 4;
    return SATR_OK;
}

/**
 * Reports the time in UTC format.
 * @param pkt
 * @param dest_id
 * @return 
 */
SAT_returnState time_management_report_time_in_utc(tc_tm_pkt *pkt, TC_TM_app_id dest_id){

    struct time_utc temp_time;
    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }
    get_time_UTC(&temp_time);
    time_management_crt_pkt_TM(pkt, TM_TIME_REPORT_IN_UTC, dest_id);
    pkt->data[0] = temp_time.weekday;
    pkt->data[1] = temp_time.day;
    pkt->data[2] = temp_time.month;
    pkt->data[3] = temp_time.year;
    pkt->data[4] = temp_time.hour;
    pkt->data[5] = temp_time.min;
    pkt->data[6] = temp_time.sec;
    pkt->len = 7;

    return SATR_OK;
}

/**
 * Requests time in UTC format from a time_management_service implementor.
 * @param dest_id is the on-board time_management service implementor to request time from.
 * @return 
 */
SAT_returnState time_management_request_time_in_utc( TC_TM_app_id dest_id){
    
    tc_tm_pkt *time_req_pkt;
    time_req_pkt = get_pkt(PKT_NORMAL);
    
    if(!C_ASSERT( time_req_pkt != NULL) == true) { return SATR_ERROR; }
    time_management_crt_pkt_TC(time_req_pkt, TM_REPORT_TIME_IN_UTC, dest_id ); 
    route_pkt(time_req_pkt); 
    return SATR_OK;
}

/** 
 * Forces time update in UTC format to a time_management_service implementor.
 * @param dest_id is the on-board time_management service implementor to force time update on.
 * @return 
 */
SAT_returnState time_management_force_time_update( TC_TM_app_id dest_id){
    
    struct time_utc temp_time;    
    tc_tm_pkt *time_req_pkt;
    time_req_pkt = get_pkt(PKT_NORMAL);
    
    get_time_UTC(&temp_time);
    if(!C_ASSERT( time_req_pkt != NULL) == true) { return SATR_ERROR; }
    
    time_management_crt_pkt_TM(time_req_pkt, TM_TIME_SET_IN_UTC, dest_id );
    (time_req_pkt)->data[0] = temp_time.weekday;
    (time_req_pkt)->data[1] = temp_time.day;
    (time_req_pkt)->data[2] = temp_time.month;
    (time_req_pkt)->data[3] = temp_time.year;
    (time_req_pkt)->data[4] = temp_time.hour;
    (time_req_pkt)->data[5] = temp_time.min;
    (time_req_pkt)->data[6] = temp_time.sec;
    (time_req_pkt)->len = 7;
    
    route_pkt(time_req_pkt);
    return SATR_OK;
}

SAT_returnState time_management_crt_pkt_TC(tc_tm_pkt *pkt, uint8_t sid, TC_TM_app_id dest_app_id){

    if(!C_ASSERT(dest_app_id < LAST_APP_ID) == true)  { return SATR_ERROR; }
    crt_pkt(pkt, dest_app_id, TC, TC_ACK_NO, TC_TIME_MANAGEMENT_SERVICE, sid, SYSTEM_APP_ID);
    pkt->len = 0;

    return SATR_OK;
}

SAT_returnState time_management_crt_pkt_TM(tc_tm_pkt *pkt, uint8_t sid, TC_TM_app_id dest_app_id){

    if(!C_ASSERT(dest_app_id < LAST_APP_ID) == true)  { return SATR_ERROR; }
    crt_pkt(pkt, SYSTEM_APP_ID, TM, TC_ACK_NO, TC_TIME_MANAGEMENT_SERVICE, sid, dest_app_id);
    pkt->len = 0;

    return SATR_OK;
}

/**
 * converts a day's moment (Hour, Minutes, Seconds) in this day's seconds.
 * @param moment
 * @param daysecs
 * @return 
 */
SAT_returnState cnv_utc_to_secs( struct time_utc *moment, uint32_t *daysecs ){ //1 day = 86400 secs
    
    if(!C_ASSERT( moment->hour >= 0  && moment->hour <= 24 ) == true) { return SATR_ERROR; }
    if(!C_ASSERT( moment->min  >= 0  && moment->min  <= 60 ) == true) { return SATR_ERROR; }
    if(!C_ASSERT( moment->sec  >= 0  && moment->sec  <= 60 ) == true) { return SATR_ERROR; }
     *daysecs = ( moment->hour * 3600 ) +
                ( moment->min  * 60   ) +
                ( moment->sec);
    
    return SATR_OK;
}

void cnv_UTC_QB50(struct time_utc utc, uint32_t *qb){
    
    *qb = (UTC_QB50_YM[utc.year][utc.month] +  
          UTC_QB50_D[utc.day]               + 
          UTC_QB50_H[utc.hour]              + 
          (utc.min*60) + utc.sec) - UTC_QB50_D[1];
}

void set_time_QB50(uint32_t qb){
  /*no general meaning(?)*/
}

void set_time_UTC(struct time_utc utc){
    
    HAL_sys_setTime(utc.hour, utc.min, utc.sec);
    HAL_sys_setDate(utc.weekday, utc.month, utc.day, utc.year);
}

/**
 * Fills the destination uint32_t pointer with
 * QB50 epoch (seconds from 2000)
 * @param qb
 */
void get_time_QB50(uint32_t *qb){

    struct time_utc utc;
    HAL_sys_getTime(&utc.hour, &utc.min, &utc.sec);
    HAL_sys_getDate(&utc.weekday, &utc.month, &utc.day, &utc.year);
    cnv_UTC_QB50(utc, qb);
}

/**
 * Returns time in QB50 epoch (seconds from 2000)
 * @return 
 */
uint32_t return_time_QB50(){
    
    struct time_utc utc;
    uint32_t qb_secs;
    HAL_sys_getTime(&utc.hour, &utc.min, &utc.sec);
    HAL_sys_getDate(&utc.weekday, &utc.month, &utc.day, &utc.year);
    cnv_UTC_QB50(utc, &qb_secs);
    return qb_secs;
}

void get_time_UTC(struct time_utc *utc){

    HAL_sys_getTime(&utc->hour, &utc->min, &utc->sec);
    HAL_sys_getDate(&utc->weekday, &utc->month, &utc->day, &utc->year);
}
