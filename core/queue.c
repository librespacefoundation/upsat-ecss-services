#include "queue.h"

#include "pkt_pool.h"
#include "sysview.h"

#undef __FILE_ID__
#define __FILE_ID__ 34

struct _queue {
    tc_tm_pkt *fifo[POOL_PKT_TOTAL_SIZE];
    uint8_t head;
    uint8_t tail;
};

static struct _queue queue = { .head = 0, .tail = 0};

SAT_returnState queuePush(tc_tm_pkt *pkt, TC_TM_app_id app_id) {

    if(queue.head == (((queue.tail - 1) + POOL_PKT_TOTAL_SIZE) % POOL_PKT_TOTAL_SIZE)) {
        return SATR_QUEUE_FULL;
    }

    queue.fifo[queue.head] = pkt;

    queue.head = (queue.head + 1) % POOL_PKT_TOTAL_SIZE;

    return SATR_OK;
}

tc_tm_pkt * queuePop(TC_TM_app_id app_id) {

    tc_tm_pkt *pkt;

    if(queue.head == queue.tail) { return 0; }

    pkt = queue.fifo[queue.tail];
    queue.tail = (queue.tail + 1) % POOL_PKT_TOTAL_SIZE;

    return pkt;
}

uint8_t queueSize(TC_TM_app_id app_id) {

    if(queue.head == queue.tail) { return 0; }

    return queue.head - queue.tail;
}

tc_tm_pkt * queuePeak(TC_TM_app_id app_id) {

    if(queue.head == queue.tail) { return 0; }

    return queue.fifo[queue.head];
}

void queue_IDLE(TC_TM_app_id app_id) {

    tc_tm_pkt *pkt;

    pkt = queuePeak(app_id);
    if(!C_ASSERT(pkt != NULL) == true) { return; }

    if(is_free_pkt(pkt) == true) {
        queuePop(app_id);
        traceGC_QUEUE_PKT();

    }

}
