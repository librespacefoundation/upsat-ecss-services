#ifndef __HOUSEKEEPING_SERVICE_H
#define __HOUSEKEEPING_SERVICE_H

#include "services.h"

#define OBC_EXT_WOD_SIZE      51
#define OBC_EXT_WOD_OFFSET     1
#define COMMS_EXT_WOD_SIZE    29
#define COMMS_EXT_WOD_OFFSET  51
#define ADCS_EXT_WOD_SIZE     87
#define ADCS_EXT_WOD_OFFSET   80
#define EPS_EXT_WOD_SIZE      34
#define EPS_EXT_WOD_OFFSET   167

#define SYS_EXT_WOD_SIZE     201
#define SUB_SYS_EXT_WOD_SIZE 150

#define SCI_REP_SIZE		  18
#define SCI_ARR_OFFSET		   4
#define ADCS_EXT_SCI_OFFSET	  18


SAT_returnState hk_app(tc_tm_pkt *pkt);

SAT_returnState hk_crt_pkt_TC(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid);

SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid);

SAT_returnState hk_crt_empty_pkt_TM(tc_tm_pkt **pkt, const TC_TM_app_id app_id, const HK_struct_id sid);

#endif
