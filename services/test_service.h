#ifndef TEST_SERVICE_H
#define TEST_SERVICE_H

#include "services.h"


SAT_returnState test_app(tc_tm_pkt *pkt);

SAT_returnState test_crt_pkt(tc_tm_pkt **pkt, TC_TM_app_id dest_id);

#endif