#include "upsat.h"

#include "pkt_pool.h"
#include "service_utilities.h"
#include "hldlc.h"
#include "verification_service.h"

#undef __FILE_ID__
#define __FILE_ID__ 1

#define HOUR 360000

extern SAT_returnState hal_kill_uart(TC_TM_app_id app_id);

extern SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data);

extern tc_tm_pkt * queuePeak(TC_TM_app_id app_id);
extern tc_tm_pkt * queuePop(TC_TM_app_id app_id);

SAT_returnState import_pkt(TC_TM_app_id app_id, struct uart_data *data) {

    tc_tm_pkt *pkt;
    uint16_t size = 0;

    SAT_returnState res;    
    SAT_returnState res_deframe;

    res = HAL_uart_rx(app_id, data);
    if( res == SATR_EOT ) {
        
        size = data->uart_size;
        res_deframe = HLDLC_deframe(data->uart_unpkt_buf, data->deframed_buf, &size);
        if(res_deframe == SATR_EOT) {
            
            data->last_com_time = HAL_sys_GetTick();/*update the last communication time, to be used for timeout discovery*/

            pkt = get_pkt(size);

            if(!C_ASSERT(pkt != NULL) == true) { return SATR_ERROR; }            
            if(unpack_pkt(data->deframed_buf, pkt, size) == SATR_OK) { route_pkt(pkt); } 
            verification_app(pkt);
            free_pkt(pkt);
        }
    }

    return SATR_OK;
}

//WIP
SAT_returnState export_pkt(TC_TM_app_id app_id, struct uart_data *data) {

    tc_tm_pkt *pkt = 0;
    uint16_t size = 0;
    SAT_returnState res = SATR_ERROR;

    /* Checks if the tx is busy */
    if((res = HAL_uart_tx_check(app_id)) == SATR_ALREADY_SERVICING) { return res; }

    /* Checks if that the pkt that was transmitted is still in the queue */
    if((pkt = queuePop(app_id)) ==  NULL) { return SATR_OK; }

    pack_pkt(data->uart_pkted_buf,  pkt, &size);

    res = HLDLC_frame(data->uart_pkted_buf, data->framed_buf, &size);
    if(res == SATR_ERROR) { return SATR_ERROR; }

    if(!C_ASSERT(size > 0) == true) { return SATR_ERROR; }

    HAL_uart_tx(app_id, data->framed_buf, size);

    free_pkt(pkt);

    return SATR_OK;
}

void uart_killer(TC_TM_app_id app_id, struct uart_data *data, uint32_t time) {

    if(time - data->init_time > 160000) {

        SAT_returnState res = hal_kill_uart(app_id);
        if(res == SATR_OK) {
            data->init_time = time;
        }
    }
}
