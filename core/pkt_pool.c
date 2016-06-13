#include "pkt_pool.h"


#undef __FILE_ID__
#define __FILE_ID__ 10

struct _pkt_pool{
    tc_tm_pkt pkt[POOL_PKT_SIZE];
    uint8_t free[POOL_PKT_SIZE];
    uint32_t time[POOL_PKT_SIZE];
    uint8_t data[POOL_PKT_SIZE][MAX_PKT_DATA];
    uint32_t time_delta[POOL_PKT_SIZE];

    tc_tm_pkt pkt_ext[POOL_PKT_EXT_SIZE];
    uint8_t free_ext[POOL_PKT_EXT_SIZE];
    uint32_t time_ext[POOL_PKT_EXT_SIZE];
    uint8_t data_ext[POOL_PKT_EXT_SIZE][MAX_PKT_EXT_DATA];
    uint32_t time_delta_ext[POOL_PKT_EXT_SIZE];

};

static struct _pkt_pool pkt_pool;

tc_tm_pkt * get_pkt(const uint16_t size) {

    if(size <= PKT_NORMAL) {
        for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
            if(pkt_pool.free[i] == true) {
                pkt_pool.free[i] = false;
                pkt_pool.time[i] = HAL_sys_GetTick();
                pkt_pool.pkt[i].verification_state = SATR_PKT_INIT;
                return &pkt_pool.pkt[i];
            }
        }
    } else {
        for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
            if(pkt_pool.free_ext[i] == true) {
                pkt_pool.free_ext[i] = false;
                pkt_pool.time_ext[i] = HAL_sys_GetTick();
                pkt_pool.pkt_ext[i].verification_state = SATR_PKT_INIT;
                return &pkt_pool.pkt_ext[i];
            }
        }
    }
    return NULL;
}

SAT_returnState free_pkt(tc_tm_pkt *pkt) {

    if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }

    for(uint8_t i = 0; i <= POOL_PKT_SIZE; i++) {
        if(&pkt_pool.pkt[i] == pkt) {
            pkt_pool.free[i] = true;
            pkt_pool.time_delta[i]= HAL_sys_GetTick() - pkt_pool.time[i];
            return SATR_OK;
        }
    }

    for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
        if(&pkt_pool.pkt_ext[i] == pkt) {
            pkt_pool.free_ext[i] = true;
            pkt_pool.time_delta_ext[i]= HAL_sys_GetTick() - pkt_pool.time_ext[i];
            return SATR_OK;
        }
    }

    return SATR_ERROR;
}

SAT_returnState pkt_pool_INIT() {

    for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
       pkt_pool.free[i] = true;
       pkt_pool.pkt[i].data = &pkt_pool.data[i][0];
    }

    for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
       pkt_pool.free_ext[i] = true;
       pkt_pool.pkt_ext[i].data = &pkt_pool.data_ext[i][0];
    }

    return SATR_OK;
}

void pkt_pool_IDLE() {

    uint32_t tmp_time = HAL_sys_GetTick();

    for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
        if(pkt_pool.free[i] == false && pkt_pool.time[i] - tmp_time > PKT_TIMEOUT) {
            pkt_pool.free[i] = 0;
            event_pkt_pool_timeout();
        }
    }

    for(uint8_t i = 0; i < POOL_PKT_EXT_SIZE; i++) {
        if(pkt_pool.free_ext[i] == false && pkt_pool.time_ext[i] - tmp_time > PKT_TIMEOUT) {
            pkt_pool.free[i] = 0;
            event_pkt_pool_timeout();
        }
    }

}

SAT_returnState queuePush(struct queue *uart_queue, tc_tm_pkt *pkt) {

    //mutex take

    if(uart_queue->head == (((uart_queue->tail - 1) + POOL_PKT_TOTAL_SIZE) % POOL_PKT_TOTAL_SIZE)) {
        return SATR_QUEUE_FULL;
    }

    uart_queue[uart_queue->head] = pkt;

    uart_queue->head = (uart_queue->head + 1) % POOL_PKT_TOTAL_SIZE;

    //mutex left

    return SATR_OK;
}

SAT_returnState queuePop(struct queue *uart_queue) {

    if(uart_queue->head == uart_queue->tail) { return SATR_QUEUE_EMPTY; }

    uart_queue->tail = (uart_queue->tail + 1) % POOL_PKT_TOTAL_SIZE;

    return SATR_OK;
}

uint8_t queueSize(struct queue *uart_queue)
{
    if(uart_queue->head == QueueOut) { return 0; }

    return uart_queue->head - uart_queue->tail;
}

