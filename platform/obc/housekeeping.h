#ifndef __HOUSEKEEPING_H
#define __HOUSEKEEPING_H

#include <stdint.h>
#include "services.h"


void hk_SCH();

void hk_INIT();

void clear_wod();

void clear_ext_wod();

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data);

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt);

#endif
