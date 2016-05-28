#ifndef __FUNCTION_MANAGEMENT_SERVICE_H
#define __FUNCTION_MANAGEMENT_SERVICE_H

#include <stdint.h>
#include "services.h"
#include "service_utilities.h"
#include "pkt_pool.h"
#include "power_ctrl.h"
#include "time_management.h"

extern SAT_returnState time_management_app(tc_tm_pkt *pkt);

SAT_returnState function_management_app(tc_tm_pkt *pkt);

SAT_returnState function_management_pctrl_crt_pkt_api(tc_tm_pkt **pkt, TC_TM_app_id dest_id, FM_fun_id fun_id, FM_dev_id did);

#endif