#include "pkt_pool.h"


#undef __FILE_ID__
#define __FILE_ID__ 10

struct _pkt_pool pkt_pool;

tc_tm_pkt * get_pkt() {

    for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
        if(pkt_pool.free[i] == true) {
            pkt_pool.free[i] = false;
            pkt_pool.time[i] = HAL_sys_GetTick();
            pkt_pool.pkt[i].verification_state = SATR_PKT_INIT;
            return &pkt_pool.pkt[i];
        }
    }

    return NULL;
}

#ifdef POOL_PKT_EXT
tc_tm_pkt * get_pkt_ext() {

    for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
        if(pkt_pool.free_ext[i] == true) {
            pkt_pool.free_ext[i] = false;
            pkt_pool.time_ext[i] = HAL_sys_GetTick();
            pkt_pool.pkt_ext[i].verification_state = SATR_PKT_INIT;
            return &pkt_pool.pkt_ext[i];
        }
    }

    return NULL;
}
#endif

SAT_returnState free_pkt(tc_tm_pkt *pkt) {

    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }

    for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
        if(&pkt_pool.pkt[i] == pkt) {
            pkt_pool.free[i] = true;
            pkt_pool.time_delta[i]= HAL_sys_GetTick() - pkt_pool.time[i];
            return SATR_OK;
        }
    }

#ifdef POOL_PKT_EXT
    for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
        if(&pkt_pool.pkt_ext[i] == pkt) {
            pkt_pool.free_ext[i] = true;
            pkt_pool.time_delta_ext[i]= HAL_sys_GetTick() - pkt_pool.time_ext[i];
            return SATR_OK;
        }
    }
#endif    

    return SATR_ERROR;
}

SAT_returnState pkt_pool_INIT() {

    for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
       pkt_pool.free[i] = true;
       pkt_pool.pkt[i].data = &pkt_pool.data[i][0];
    }

#ifdef POOL_PKT_EXT
    for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
       pkt_pool.free_ext[i] = true;
       pkt_pool.pkt_ext[i].data = &pkt_pool.data_ext[i][0];
    }
#endif

    return SATR_OK;
}

void pkt_pool_GC() {

    for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
        if(pkt_pool.free[i] == false && pkt_pool.time[i] - time_now() > PKT_TIMEOUT) {
            //pkt_pool.free[i] = 0;
            // error
        }
    }

#ifdef POOL_PKT_EXT
    for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
        if(pkt_pool.free_ext[i] == false && pkt_pool.time_ext[i] - time_now() > PKT_TIMEOUT) {
            //pkt_pool.free[i] = 0;
            // error
        }
    }
#endif

}
