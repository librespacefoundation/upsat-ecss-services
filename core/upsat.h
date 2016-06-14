#ifndef __UPSAT_H
#define __UPSAT_H

#include "services.h"
#include "pkt_pool.h"

#define EV_MAX_BUFFER  1024
#define EV_BUFFER_PART 205

#define WOD_MAX_BUFFER 256

#define IAC_PKT_SIZE 205 /*1 cmd, 4 fname, 210 data*/

/*restriction for 8 char filename, for conversion from num to file name*/
#define MAX_FILE_NUM 0x5F5E0FF

#if (SYSTEM_APP_ID == _EPS_APP_ID_)
#define UART_BUF_SIZE 410 /*(MAX_PKT_DATA*2)*/
#else
#define UART_BUF_SIZE 4096 /*(POOL_PKT_EXT*2)*/
#endif

/*cubesat subsystem's timeouts*/
#define TIMEOUT_V_COMMS     5000
#define TIMEOUT_V_ADCS      5000
#define TIMEOUT_V_IAC       5000
#define TIMEOUT_V_EPS       5000
#define TIMEOUT_V_SU_BYTE   5000
#define TIMEOUT_V_DBG       5000

struct uart_data {
    uint8_t uart_buf[UART_BUF_SIZE];
    uint8_t uart_unpkt_buf[UART_BUF_SIZE];

    uint8_t deframed_buf[MAX_PKT_SIZE];

    uint8_t uart_pkted_buf[UART_BUF_SIZE];
    uint8_t framed_buf[UART_BUF_SIZE];
    uint16_t uart_size;
    
    uint32_t last_com_time;
};

/* These values represent the time of last complete packet
 * received by the OBC subsystem.
 */
struct _subs_last_comm {
    uint32_t last_com_comms;
    uint32_t last_com_adcs;
    uint32_t last_com_iac;
    uint32_t last_com_eps;
};

SAT_returnState import_pkt(TC_TM_app_id app_id, struct uart_data *data);

SAT_returnState export_pkt(TC_TM_app_id app_id, struct uart_data *data);

SAT_returnState sys_data_INIT();

#endif
