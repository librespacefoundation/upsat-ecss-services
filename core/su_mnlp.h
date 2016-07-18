#ifndef SU_MNLP_H
#define SU_MNLP_H

#include <stdint.h>
#include "services.h"
#include "time_management_service.h"
#include "scheduling_service.h"
#include "upsat.h"
#include "uart_hal.h"

#define SU_SCRIPTS_POPU 7

#define DAYSECS 86400

#define SU_MAX_FILE_SIZE 2048
#define SU_CMD_SEQ 5
#define SU_EOT_CMD_HEADER_SIZE 5
#define SU_TT_HEADER_SIZE 4

#define SU_RSP_SIZE 174
#define SU_SCI_HEADER_SIZE 22
#define SU_LOG_SIZE 196

/* 
 * REQ: MNLP-013
 * Science unit timeout in seconds.
*/
#define SU_TIMEOUT 400

#define SU_ERR_TIM_SLEEP_TIME 60000 /*60 seconds in millisecs*/

/* 
section 13.6
*/
#define SU_SCR_HDR_RES   0
#define SU_SCR_HDR_INMS  1
#define SU_SCR_HDR_MNLP  2
#define SU_SCR_HDR_FIPEX 3

/*
REQ: MNLP-027
*/
#define SU_SCR_TT_S1            0x41
#define SU_SCR_TT_S2            0x42
#define SU_SCR_TT_S3            0x43
#define SU_SCR_TT_S4            0x44
#define SU_SCR_TT_S5            0x45
#define SU_SCR_TT_SNONE         0x00
#define SU_SCR_TT_EOT           0x55

/*
Section 13.10

when 0x00 is undefined
*/

/*SU command IDs*/
#define SU_OBC_SU_ON_CMD_ID     0xF1
#define SU_OBC_SU_OFF_CMD_ID    0xF2
#define SU_RESET_CMD_ID         0x02
#define SU_LDP_CMD_ID           0x05
#define SU_HC_CMD_ID            0x06
#define SU_CAL_CMD_ID           0x07
#define SU_SCI_CMD_ID           0x08
#define SU_HK_CMD_ID            0x09
#define SU_STM_CMD_ID           0x0A
#define SU_DUMP_CMD_ID          0x0B
#define SU_BIAS_ON_CMD_ID       0x53
#define SU_BIAS_OFF_CMD_ID      0xC9
#define SU_MTEE_ON_CMD_ID       0x35
#define SU_MTEE_OFF_CMD_ID      0x9C
#define SU_ERR_CMD_ID           0x00
#define SU_OBC_SU_ERR_CMD_ID    0x00
#define SU_OBC_EOT_CMD_ID       0xFE

/*SU responses IDs*/
#define OBC_SU_ON_RSP_ID        0x00
#define OBC_SU_OFF_RSP_ID       0x00
#define SU_RESET_RSP_ID         0x00
#define SU_LDP_RSP_ID           0x05
#define SU_HC_RSP_ID            0x06
#define SU_CAL_RSP_ID           0x07
#define SU_SCI_RSP_ID           0x08
#define SU_HK_RSP_ID            0x09
#define SU_STM_RSP_ID           0x0A
#define SU_DUMP_RSP_ID          0x0B
#define SU_BIAS_ON_RSP_ID       0x00
#define SU_BIAS_OFF_RSP_ID      0x00
#define SU_MTEE_ON_RSP_ID       0x00
#define SU_MTEE_OFF_RSP_ID      0x00
#define SU_ERR_RSP_ID           0xBB
#define OBC_SU_ERR_RSP_ID       0xFA
#define SU_TIMEOUT_ERR_ID       0xF0 /*d. mas mail*/
#define OBC_EOT_RSP_ID          0x00

/*
REQ: MNLP-031
*/
#define SU_RSP_PCKT_DATA_SIZE   172
#define SU_RSP_PCKT_SIZE        174

/* the 13th byte is the first byte 
 * of a time table script record.
 */
#define SU_TT_OFFSET            12

/*
 *This header is to be attached in every response from the m-nlp scientific instrument.
 *needs to be populated in every response in order to have up to date info.
 */
//typedef struct{    
//    uint32_t time_epoch;    /*qb50_epoch, seconds since Year 2000*/
//    int16_t  roll;          /*-180to+180 deg*/
//    int16_t  pitch;         /*-180to+180 deg*/
//    int16_t  yaw;           /*-180to+180 deg*/
//    int16_t  roll_dot;      /*deg per sec*/
//    int16_t  pitch_dot;     /*deg per sec*/
//    int16_t  yaw_dot;       /*deg per sec*/
//    int16_t x_eci;          /*km*/
//    int16_t y_eci;          /*km*/
//    int16_t z_eci;          /*km*/    
//}mnlp_response_science_header;

typedef enum{
    su_sche_script_ended    = 1,
    su_sche_go_to_sleep     = 2,
    su_new_scr_selected     = 3, /*when a new script is eligible to run*/
    su_no_new_scr_selected  = 4, /*when a script is eligible to run*/
    su_no_scr_eligible      = 5,  /*when no script is eligible to run*/
    su_sche_last            = 6        
}su_mnlp_returnState;

/*
fixed size: 12 bytes
section 13.6
time format: UTC
ID_SWver: b4-b0 sw ver,b6-b5 su id 
MD_type:  b4-b0 script type, b6-b5 su model
*/
typedef struct{   
    uint16_t script_len;    /*complete script length, 2 bytes*/    
    uint32_t start_time;    /*UTC time at which script execution begins, 4 bytes*/    
    uint32_t file_sn;       /*file serial number, 4 bytes*/    
    uint8_t  sw_ver;        /*SW_VER-> 5bits, SU_ID-> 2 bits, b7->1 bit "0", 1 byte*/
    uint8_t  su_id;    
    uint8_t  script_type;   /*usage of script, script type-> 5 bits, su_model-> 2 bits, b7->1 bit "0" */    
    uint8_t  su_md;         /*su model*/    
    uint16_t xsum;          /*checksum*/    
    uint8_t  script_sequences_popu; /*number of script sequences in the times table*/
}science_unit_script_header;

/* 
REQ: MNLP-027
*/
typedef struct{    
    uint8_t sec;    /*range: 00-59*/    
    uint8_t min;    /*range: 00-59*/    
    uint8_t hours;  /*range: 00-23*/    
    uint8_t script_index;   /*range: 0x41->S1, 0x42->S2, 0x43->S3, 0x44->S4, 0x45->S5, 0x55->EndOfTable*/
}science_unit_script_time_table;

/* 
REQ: MNLP-027
*/
typedef struct{
    
    uint8_t dt_sec;     /*deltaTIME seconds, range: 00-59*/    
    uint8_t dt_min;     /*deltaTIME minutes, range: 00-59*/    
    uint8_t cmd_id;     /*CMD_ID, command id*/    
    uint8_t len;        /*LEN, length in BYTES of parameter field, range: 0 - 255*/    
    uint8_t seq_cnt;    /*SEQ_CNT, command counter, range: 0 - 255*/
    
    /* holds the command payload to send to the mNLP science unit
     * first byte is the cmd_id to send to the mNLP,
     * //second byte is this command's length,
     * second byte is command's sequence count,
     * and from the third byte the command's parameters starts.
     * Maximum command's length is 140 bytes ( SU_LDP cmd 70 16-bit words ), 
     */
    uint8_t command[255];

}science_unit_script_sequence;

/*represents a science unit script, with its header, time table and command sequences*/
typedef struct{
    science_unit_script_header scr_header;
    science_unit_script_time_table tt_header;
    science_unit_script_sequence seq_header;
    uint8_t file_load_buf[SU_MAX_FILE_SIZE];    /*where the data will be kept after loading*/
    uint8_t valid_str;  /*a script is valid_str(uctural) if it have passed the checksums checks*/
    uint8_t valid_logi; /*a script is valid_logi(cal) if it is to be scheduled for execution by su scheduler*/
}science_unit_script_inst;

struct _MNLP_data{
    uint32_t *su_init_func_run_time;     /*contains the time that su initialization occured -su_init- run (QB50 epoch)*/
    uint32_t *su_nmlp_perm_state_pnt;
    uint8_t *su_nmlp_last_active_script; /*last active script chosen by the scheduler, and saved on sram region*/
    uint8_t *su_next_time_table;         /*points to the time table that needs to be executed when the scheduler will run*/
    uint8_t *su_next_script_seq;         /*point to the script sequence that needs to be executed when the scheduler will run*/
    uint8_t *su_nmlp_script_scheduler_active;   /*True if mNLP scheduler script is active/running, false otherwise*/    
    uint8_t *su_service_sheduler_active; /*True if mNLP scheduler script is active/running, false otherwise*/
    
    uint8_t su_timed_out;
    MS_sid active_script;                /*the current (runtime) active script*/
    SU_state su_state;                   /*the state of the science unit*/
    uint32_t last_su_response_time;      /*the qb50 epoch time of last su response reception*/
    
    science_unit_script_inst su_scripts[SU_SCRIPTS_POPU]; /*holds references to science unit scripts, loaded from permanent storage*/    
    uint8_t su_inc_resp[180];            /*174 response data + 6 bytes to detect nmlp response offsets*/
    
//    mnlp_response_science_header mnlp_science_header;
};

extern struct _MNLP_data MNLP_data;
extern uint8_t su_sci_header[22];

su_mnlp_returnState su_SCH(uint32_t *sleep_val);
su_mnlp_returnState su_script_selector(uint32_t *sleep_val);

void su_INIT();
void serve_tt();
void su_load_all_scripts();
void handle_su_error_response();
void handle_su_timeout();
SAT_returnState su_load_specific_script(MS_sid sid);
SAT_returnState su_goto_script_seq( uint16_t *script_sequence_pointer, uint8_t *ss_to_go);
SAT_returnState su_incoming_rx();
SAT_returnState su_cmd_handler( science_unit_script_sequence *cmd);
SAT_returnState su_populate_header( science_unit_script_header *su_script_hdr, uint8_t *buf);
SAT_returnState polulate_next_time_table(uint8_t *buf, science_unit_script_time_table *tt, uint16_t *tt_pointer);
SAT_returnState su_next_cmd(uint8_t *buf,  science_unit_script_sequence *cmd, uint16_t *pointer);
SAT_returnState su_power_ctrl(FM_fun_id fid);
SAT_returnState generate_obc_su_error(uint8_t *buffer, uint8_t err_source);
SAT_returnState handle_script_upload(MS_sid sid);
#endif
