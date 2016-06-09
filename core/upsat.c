#include "upsat.h"

#include "pkt_pool.h"
#include "service_utilities.h"
#include "hldlc.h"
#include "verification_service.h"

#undef __FILE_ID__
#define __FILE_ID__ 1

extern SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data);


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
            else { verification_app(pkt); free_pkt(pkt); }
        }
    }

    return SATR_OK;
}

//WIP
SAT_returnState export_pkt(TC_TM_app_id app_id, tc_tm_pkt *pkt, struct uart_data *data) {

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) { return SATR_ERROR; }

    uint16_t size = 0;
    SAT_returnState res;    

    pack_pkt(data->uart_pkted_buf, pkt, &size);

    HAL_uart_tx_check(app_id);

    res = HLDLC_frame(data->uart_pkted_buf, data->framed_buf, &size);
    if(res == SATR_ERROR) { return SATR_ERROR; }

    if(!C_ASSERT(size > 0) == true) { return SATR_ERROR; }

    HAL_uart_tx(app_id, data->framed_buf, size);

    return SATR_OK;
}
