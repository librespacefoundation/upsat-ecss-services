#ifndef __HOUSEKEEPING_SERVICE_H
#define __HOUSEKEEPING_SERVICE_H

#include "services.h"

//finalize sizes

#define COMMS_EXT_WOD_SIZE   1
#define COMMS_EXT_WOD_OFFSET 1
#define ADCS_EXT_WOD_SIZE    1
#define ADCS_EXT_WOD_OFFSET  1
#define EPS_EXT_WOD_SIZE     1
#define EPS_EXT_WOD_OFFSET   1

#define SUB_SYS_EXT_WOD_SIZE 1

SAT_returnState hk_app(tc_tm_pkt *pkt);

SAT_returnState hk_crt_pkt_TC(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid);

SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid);

SAT_returnState hk_crt_empty_pkt_TM(tc_tm_pkt **pkt, const TC_TM_app_id app_id, const HK_struct_id sid);

#endif
