#ifndef TIME_MANAGEMENT_H
#define TIME_MANAGEMENT_H

#include <stdint.h>
#include "services.h"
#include "pkt_pool.h"

#define TM_MONTH_JANUARY        ((uint8_t)0x01U)
#define TM_MONTH_FEBRUARY       ((uint8_t)0x02U)
#define TM_MONTH_MARCH          ((uint8_t)0x03U)
#define TM_MONTH_APRIL          ((uint8_t)0x04U)
#define TM_MONTH_MAY            ((uint8_t)0x05U)
#define TM_MONTH_JUNE           ((uint8_t)0x06U)
#define TM_MONTH_JULY           ((uint8_t)0x07U)
#define TM_MONTH_AUGUST         ((uint8_t)0x08U)
#define TM_MONTH_SEPTEMBER      ((uint8_t)0x09U)
#define TM_MONTH_OCTOBER        ((uint8_t)0x10U)
#define TM_MONTH_NOVEMBER       ((uint8_t)0x11U)
#define TM_MONTH_DECEMBER       ((uint8_t)0x12U)

#define MAX_YEAR 21

struct time_utc {
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

struct time_keeping {
    uint32_t epoch;
    uint32_t elapsed;
    struct time_utc utc;
};

void cnv_UTC_QB50(struct time_utc utc, uint32_t *qb);

void set_time_QB50(uint32_t qb);

void set_time_UTC(struct time_utc utc);

void get_time_QB50(uint32_t *qb);

void get_time_UTC(struct time_utc *utc);

uint32_t return_time_QB50();

SAT_returnState time_management_app(tc_tm_pkt *pck);

SAT_returnState time_management_report_time_in_utc(tc_tm_pkt *pkt, TC_TM_app_id dest_id);

SAT_returnState time_management_report_time_in_qb50(tc_tm_pkt *pkt, TC_TM_app_id dest_id);

SAT_returnState time_management_crt_pkt_TC(tc_tm_pkt *pkt, uint8_t sid, TC_TM_app_id app_id);

SAT_returnState time_management_crt_pkt_TM(tc_tm_pkt *pkt, uint8_t sid, TC_TM_app_id app_id);

SAT_returnState time_management_request_time_in_utc( TC_TM_app_id dest_id);

SAT_returnState time_management_force_time_update( TC_TM_app_id dest_id);
#endif
