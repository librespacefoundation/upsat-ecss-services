#ifndef __OBC_H
#define __OBC_H

#include <stdint.h>
#include "services.h"
#include "upsat.h"

//temp
#define TEST_ARRAY 1024

typedef enum {  
    EV_P0         = 0,
    EV_P1         = 1,
    EV_P2         = 2,
    EV_P3         = 3,
    EV_P4         = 4,
    LAST_EV_STATE = 5
}EV_state;

struct _obc_data
{
    uint16_t obc_seq_cnt;
    uint32_t *file_id_su;
    uint32_t *file_id_wod;
    uint32_t *file_id_ext;
    uint32_t *file_id_ev;
    uint32_t *file_id_fotos;
    uint8_t *log;
    uint32_t *log_cnt;
    uint32_t *log_state;
    uint8_t *wod_log;
    uint32_t *wod_cnt;
    
    uint8_t iac_in[IAC_PKT_SIZE];
    uint8_t iac_out[IAC_PKT_SIZE];
    uint8_t iac_flag;
    uint32_t iac_timeout;

    struct uart_data dbg_uart;
    struct uart_data comms_uart;
    struct uart_data adcs_uart;
    struct uart_data eps_uart;   
};

struct _sys_data {
    uint8_t rsrc;
    uint32_t *boot_counter;
};

extern struct _obc_data obc_data;
extern struct _wdg_state wdg;

SAT_returnState route_pkt(tc_tm_pkt *pkt);

SAT_returnState obc_INIT();


void bkup_sram_INIT();

uint32_t get_new_fileId(MS_sid sid);

SAT_returnState event_log(uint8_t *buf, const uint16_t size);

SAT_returnState event_log_load(uint8_t *buf, const uint16_t pointer, const uint16_t size);

SAT_returnState event_log_IDLE();

SAT_returnState wod_log();

SAT_returnState wod_log_load(uint8_t *buf);

void set_reset_source(const uint8_t rsrc);

void get_reset_source(uint8_t *rsrc);

void update_boot_counter();

void get_boot_counter(uint32_t *cnt);

#endif
