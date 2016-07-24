#ifndef __VERIFICATION_SERVICE_H
#define __VERIFICATION_SERVICE_H

#include "services.h"


SAT_returnState verification_app(const tc_tm_pkt *pkt);

SAT_returnState verification_crt_pkt(const tc_tm_pkt *pkt, tc_tm_pkt **out, SAT_returnState res);

#endif
