#ifndef __VERIFICATION_SERVICE_H
#define __VERIFICATION_SERVICE_H

#include "services.h"

#define ECSS_VR_DATA_LEN_SUCCESS 4
#define ECSS_VR_DATA_LEN_FAILURE 5


SAT_returnState verification_app(tc_tm_pkt *pkt); 

SAT_returnState verification_crt_pkt(tc_tm_pkt *pkt, tc_tm_pkt **out, SAT_returnState res);

#endif
