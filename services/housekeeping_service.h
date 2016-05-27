#ifndef __HOUSEKEEPING_SERVICE_H
#define __HOUSEKEEPING_SERVICE_H

#include <stdint.h>
#include "services.h"
#include "service_utilities.h"
#include "pkt_pool.h"
#include "housekeeping.h"

extern SAT_returnState route_pkt(tc_tm_pkt *pkt);

//ToDo
//	verify sid reports
//  finish sid

//finished
//  sid to enum
//  when the get packet happens in crt pkt

SAT_returnState hk_app(tc_tm_pkt *pkt);

SAT_returnState hk_crt_pkt_TC(tc_tm_pkt *pkt, TC_TM_app_id app_id, HK_struct_id sid);

SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, TC_TM_app_id app_id, HK_struct_id sid);

SAT_returnState hk_crt_empty_pkt_TM(tc_tm_pkt **pkt, TC_TM_app_id app_id, HK_struct_id sid);

#endif
