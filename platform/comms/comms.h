#ifndef __COMMS_H
#define __COMMS_H

#include <stdint.h>
#include "upsat.h"
#include "services.h"

//temp
#define TEST_ARRAY 1024

/*restriction for 8 char filename, for conversion from num to file name*/
#define MAX_FILE_NUM 0x5F5E0FF

struct _comms_data
{
    uint16_t comms_seq_cnt;

    struct uart_data obc_uart;
};

extern struct _comms_data comms_data;

SAT_returnState tx_ecss(tc_tm_pkt *pkt);
SAT_returnState rx_ecss(uint8_t *payload, const uint16_t payload_size);

SAT_returnState route_pkt(tc_tm_pkt *pkt);

SAT_returnState check_timeouts();

#endif
