#ifndef __SERVICES_H
#define __SERVICES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "subsystems_ids.h"
#include "system.h"

/* TM TC services*/
#define ECSS_VER_NUMBER             0
#define ECSS_DATA_FIELD_HDR_FLG     1
// never used
//#define TC_TM_SER_TC_VER            1

#define ECSS_PUS_VER            1
#define ECSS_SEC_HDR_FIELD_FLG  0

/*sequence definitions*/
/*we only support TC_TM_SEQ_SPACKET*/
#define TC_TM_SEQ_FPACKET 0x01
#define TC_TM_SEQ_CPACKET 0x00
#define TC_TM_SEQ_LPACKET 0x02
#define TC_TM_SEQ_SPACKET 0x03

/*services ack req*/
/*should confirm endianess*/
#define TC_ACK_NO           0x00
#define TC_ACK_ACC          0x01
#define TC_ACK_EXE_START    0x02
#define TC_ACK_EXE_PROG     0x04
#define TC_ACK_EXE_COMP     0x08
#define TC_ACK_ALL          0x0F

#define ECSS_HEADER_SIZE        6
#define ECSS_DATA_HEADER_SIZE   4
#define ECSS_CRC_SIZE           2

#define ECSS_DATA_OFFSET        10  /*ECSS_HEADER_SIZE + ECSS_DATA_HEADER_SIZE*/

#define MAX_APP_ID      20
#define MAX_SERVICES    20
#define MAX_SUBTYPES    26

#define TC 1
#define TM 0

//needs to redifine
#define MAX_PKT_LEN         210 /*ECSS_HEADER_SIZE + ECSS_DATA_HEADER_SIZE + MAX_PKT_DATA + ECSS_CRC_SIZE*/

#define MAX_PKT_DATA        198
#define TC_MAX_PKT_SIZE     210
#define TC_MIN_PKT_SIZE     11 //12  /*ECSS_HEADER_SIZE + ECSS_DATA_HEADER_SIZE + ECSS_CRC_SIZE*/

#define MAX_PKT_EXT_DATA    2051
#define TC_MAX_PKT_EXT_SIZE 2063

#if (SYSTEM_APP_ID == _EPS_APP_ID_)
#define MAX_PKT_SIZE  210
#else
#define MAX_PKT_SIZE  2063
#endif

#define MS_SU_LOG_SIZE  196
#define MS_WOD_LOG_SIZE  228

typedef enum {
    OBC_APP_ID      = _OBC_APP_ID_,
    EPS_APP_ID      = _EPS_APP_ID_,
    ADCS_APP_ID     = _ADCS_APP_ID_,
    COMMS_APP_ID    = _COMMS_APP_ID_,
    IAC_APP_ID      = _IAC_APP_ID_,
    GND_APP_ID      = _GND_APP_ID_,
    DBG_APP_ID      = _DBG_APP_ID_,
    LAST_APP_ID     = _LAST_APP_ID_
}TC_TM_app_id;

typedef enum {
    SATR_PKT_ILLEGAL_APPID     = 0,
    SATR_PKT_INV_LEN           = 1,
    SATR_PKT_INC_CRC           = 2,
    SATR_PKT_ILLEGAL_PKT_TP    = 3,
    SATR_PKT_ILLEGAL_PKT_STP   = 4,
    SATR_PKT_ILLEGAL_APP_DATA  = 5,
    SATR_OK                    = 6,
    SATR_ERROR                 = 7,
    SATR_EOT                   = 8,
    SATR_CRC_ERROR             = 9,
    SATR_PKT_ILLEGAL_ACK       = 10,
    SATR_ALREADY_SERVICING     = 11,
    SATR_MS_MAX_FILES          = 12,
    SATR_PKT_INIT              = 13,
    SATR_INV_STORE_ID          = 14,
    SATR_INV_DATA_LEN          = 15,
    /* Scheduling Service Error State Codes
    * from
    */
    SATR_SCHEDULE_FULL         = 16, /*Schedule array is full */
    SATR_SSCH_ID_INVALID       = 17, /*Subschedule ID invalid */
    SATR_NMR_OF_TC_INVALID     = 18, /*Number of telecommands invalid */
    SATR_INTRL_ID_INVALID      = 19, /*Interlock ID invalid */
    SATR_ASS_INTRL_ID_INVALID  = 20, /*Assess Interlock ID invalid */
    SATR_ASS_TYPE_ID_INVALID   = 21, /*Assesment type id invalid*/
    SATR_RLS_TIMET_ID_INVALID  = 22, /*Relese time type ID invalid */
    SATR_DEST_APID_INVALID     = 23, /*Destination APID in embedded TC is invalid */
    SATR_TIME_INVALID          = 24, /*Release time of TC is invalid */
    SATR_QBTIME_INVALID        = 25, /*Time management reported erroneous time*/
    SATR_TIME_SPEC_INVALID     = 26, /*Release time of TC is specified in a invalid representation*/
    SATR_INTRL_LOGIC_ERROR     = 27, /*The release time of telecommand is in the execution window of its interlocking telecommand.*/
    SATR_SCHEDULE_DISABLED     = 28,
    SATR_SCHEDULE_NOT_IMLP     = 29, /*Not implemented function of scheduling service*/
    /*FatFs*/
    SATRF_OK                   = 30, /* (0) Succeeded */
    SATRF_DISK_ERR             = 31, /* (1) A hard error occurred in the low level disk I/O layer */
    SATRF_INT_ERR              = 32, /* (2) Assertion failed */
    SATRF_NOT_READY            = 33, /* (3) The physical drive cannot work */
    SATRF_NO_FILE              = 34, /* (4) Could not find the file */
    SATRF_NO_PATH              = 35, /* (5) Could not find the path */
    SATRF_INVALID_NAME         = 36, /* (6) The path name format is invalid */
    SATRF_DENIED               = 37, /* (7) Access denied due to prohibited access or directory full */
    SATRF_EXIST                = 38, /* (8) Access denied due to prohibited access */
    SATRF_INVALID_OBJECT       = 39, /* (9) The file/directory object is invalid */
    SATRF_WRITE_PROTECTED      = 40, /* (10) The physical drive is write protected */
    SATRF_INVALID_DRIVE        = 41, /* (11) The logical drive number is invalid */
    SATRF_NOT_ENABLED          = 42, /* (12) The volume has no work area */
    SATRF_NO_FILESYSTEM        = 43, /* (13) There is no valid FAT volume */
    SATRF_MKFS_ABORTED         = 44, /* (14) The f_mkfs() aborted due to any parameter error */
    SATRF_TIMEOUT              = 45, /* (15) Could not get a grant to access the volume within defined period */
    SATRF_LOCKED               = 46, /* (16) The operation is rejected according to the file sharing policy */
    SATRF_NOT_ENOUGH_CORE      = 47, /* (17) LFN working buffer could not be allocated */
    SATRF_TOO_MANY_OPEN_FILES  = 48, /* (18) Number of open files > _FS_SHARE */
    SATRF_INVALID_PARAMETER    = 49, /* (19) Given parameter is invalid */
    SATRF_DIR_ERROR            = 50,
    SATR_SD_DISABLED           = 51,
    SATR_QUEUE_FULL            = 52,
    SATR_WRONG_DOWNLINK_OFFSET = 53,
    SATR_VER_ERROR             = 54,
	SATR_FIREWALLED            = 55,
    /*LAST*/
    SATR_LAST                  = 56
}SAT_returnState;

/*services types*/
#define TC_VERIFICATION_SERVICE         1
#define TC_HOUSEKEEPING_SERVICE         3
#define TC_EVENT_SERVICE                5
#define TC_FUNCTION_MANAGEMENT_SERVICE  8
#define TC_TIME_MANAGEMENT_SERVICE      9
#define TC_SCHEDULING_SERVICE           11
#define TC_LARGE_DATA_SERVICE           13
#define TC_MASS_STORAGE_SERVICE         15
#define TC_TEST_SERVICE                 17
#define TC_SU_MNLP_SERVICE              18 /*service number out of ECSS standard, mission specific for mnlp su*/

/*services subtypes*/
#define TM_VR_ACCEPTANCE_SUCCESS        1
#define TM_VR_ACCEPTANCE_FAILURE        2

#define TC_HK_REPORT_PARAMETERS         21
#define TM_HK_PARAMETERS_REPORT         23

#define TM_EV_NORMAL_REPORT         	1
#define TM_EV_ERROR_REPORT         	4

#define TC_FM_PERFORM_FUNCTION          1

#define TC_SC_ENABLE_RELEASE            1
#define TC_SC_DISABLE_RELEASE           2
#define TC_SC_RESET_SCHEDULE            3
#define TC_SC_INSERT_TC                 4
#define TC_SC_DELETE_TC                 5
#define TC_SC_TIME_SHIFT_SPECIFIC       7
#define TC_SC_TIME_SHIFT_SELECTED_OTP   8
#define TC_SC_TIME_SHIFT_ALL            15

#define TM_LD_FIRST_DOWNLINK            1
#define TC_LD_FIRST_UPLINK              9
#define TM_LD_INT_DOWNLINK              2
#define TC_LD_INT_UPLINK                10
#define TM_LD_LAST_DOWNLINK             3
#define TC_LD_LAST_UPLINK               11
#define TC_LD_ACK_DOWNLINK              5
#define TM_LD_ACK_UPLINK                14
#define TC_LD_REPEAT_DOWNLINK           6
#define TM_LD_REPEAT_UPLINK             15
#define TM_LD_REPEATED_DOWNLINK         7
#define TC_LD_REPEATED_UPLINK           12
#define TM_LD_ABORT_SE_DOWNLINK         4
#define TC_LD_ABORT_SE_UPLINK           13
#define TC_LD_ABORT_RE_DOWNLINK         8
#define TM_LD_ABORT_RE_UPLINK           16

#define TC_MS_ENABLE                    1
#define TC_MS_DISABLE                   2
#define TM_MS_CONTENT                   8
#define TC_MS_DOWNLINK                  9
#define TC_MS_DELETE                    11
#define TC_MS_REPORT                    12
#define TM_MS_CATALOGUE_REPORT          13
#define TC_MS_UPLINK                    14
#define TC_MS_FORMAT                    15 /*custom service*/
#define TC_MS_LIST                      16 /*custom service*/
#define TM_MS_CATALOGUE_LIST            17

#define TC_CT_PERFORM_TEST              1
#define TM_CT_REPORT_TEST               2

/*mNLP science unit sub-service definitions*/
#define TC_SU_ON                        1
#define TC_SU_OFF                       2
#define TC_SU_RESET                     3
#define TC_SU_LOAD_P                    4 //subservice 4
#define TM_SU_LOAD_P                    5
#define TC_SU_HC                        6 //subservice 5
#define TM_SU_HC                        7
#define TC_SU_CAL                       8 //subservice 6
#define TM_SU_CAL                       9
#define TC_SU_SCI                       10 //subservice 7
#define TM_SU_SCI                       11
#define TC_SU_HK                        12 //subservice 8
#define TM_SU_HK                        13
#define TC_SU_STM                       14 //subservice 9
#define TM_SU_STM                       15
#define TC_SU_DUMP                      16 //subservice 10
#define TM_SU_DUMP                      17
#define TC_SU_BIAS_ON                   18 //subservice 11
#define TC_SU_BIAS_OFF                  19 //subservice 12
#define TC_SU_MTEE_ON                   20 //subservice 13
#define TC_SU_MTEE_OFF                  21 //subservice 14
#define TM_SU_ERR                       22 //subservice 15
#define TM_OBC_SU_ERR                   23 //subservice 16
#define TC_OBC_EOT                      24 //subservice 17
#define TC_SU_SCHE_ON                   25 //subservice 24
#define TC_SU_SCHE_OFF                  26 //subservice 25

/*TIME MANAGEMENT SERVICE*/
#define TM_TIME_SET_IN_UTC              1 /*subservice 1*/
#define TM_TIME_SET_IN_QB50             2 /*subservice 2*/
#define TM_REPORT_TIME_IN_UTC           3 /*subservice 3, Telecommand to report time in UTC*/
#define TM_REPORT_TIME_IN_QB50          4 /*subservice 4, Telecommand to report time in QB50*/
#define TM_TIME_REPORT_IN_UTC           5 /*subservice 5, Telemetry response time in UTC*/
#define TM_TIME_REPORT_IN_QB50          6 /*subservice 6, Telemetry response time in QB50*/

/*SCHEDULING SERVICE*/
#define SCHS_ENABLE_RELEASE             1 /*subservice 01, Telecommand to enable the release of telecommands from schedule pool*/
#define SCHS_DISABLE_RELEASE            2 /*subservice 02, Telecommand to disable the release of telecommands from schedule pool*/
#define SCHS_RESET_SCH                  3 /*subservice 03, Telecommand to reset the schedule pool*/
#define SCHS_INSERT_TC_IN_SCH           4 /*subservice 04, Telecommand to insert a tc_tm_pkt in schedule pool*/
#define SCHS_DELETE_TC_FROM_SCH         5 /*subservice 05, Telecommand to delete a tc_tm_pkt from schedule pool*/
#define SCHS_DELETE_TC_FROM_SCH_OTP     6 /*subservice 06, Telecommand to delete tc_tm_pkts from schedule pool over a time period*/
#define SCHS_TIME_SHIFT_SEL_TC          7 /*subservice 07, Telecommand to time shift (+/-) selected active schedule packet*/
#define SCHS_TIME_SHIFT_SEL_TC_OTP      8 /*subservice 08, Telecommand to time shift (+/-) selected active schedule packets over a time period*/
#define SCHS_DETAILED_SCH_REPORT        10 /*subservice 10, Telemerty response to (to TC no:16) report schedules in detailed form*/
#define SCHS_SIMPLE_SCH_REPORT          13 /*subservice 13, Telemerty response to (to TC no:17) report schedules in summary form*/
#define SCHS_TIME_SHIFT_ALL_TCS         15 /*subservice 15, Telecommand to time shift (+/-) all active schedule packets*/
#define SCHS_REPORT_SCH_DETAILED        16 /*subservice 16, Telecommand to report schedules in detailed form*/
#define SCHS_REPORT_SCH_SIMPLE          17 /*subservice 17, Telecommand to report schedules in summary form*/

typedef enum {
    HEALTH_REP      = 1,
    EX_HEALTH_REP   = 2,
    EVENTS_REP      = 3,
    WOD_REP         = 4,
    EXT_WOD_REP     = 5,
    SU_SCI_HDR_REP  = 6,
    ADCS_TLE_REP    = 7,
    EPS_FLS_REP     = 8,
    ECSS_STATS_REP  = 9,
    LAST_STRUCT_ID  = 10
}HK_struct_id;

typedef enum {
    P_OFF       = 0,
    P_ON        = 1,
    P_RESET     = 2,
    SET_VAL     = 3,
    LAST_FUN_ID = 4
}FM_fun_id;

typedef enum {
    OBC_DEV_ID      = 1,
    EPS_DEV_ID      = 2,
    ADCS_DEV_ID     = 3,
    COMMS_DEV_ID    = 4,
    IAC_DEV_ID      = 5,
    SU_DEV_ID       = 6,
    GPS_DEV_ID      = 7,
    OBC_SD_DEV_ID   = 8,
    ADCS_SD_DEV_ID  = 9,
    ADCS_SENSORS    = 10,
    ADCS_GPS        = 11,
    ADCS_MAGNETO    = 12,
    ADCS_SPIN       = 13,
    ADCS_TLE        = 14,
    ADCS_CTRL_GAIN  = 15,
    ADCS_SET_POINT  = 16,
    EPS_WRITE_FLS   = 17,
    SYS_DBG         = 18,
    COMMS_WOD_PAT   = 19,
    LAST_DEV_ID     = 20
}FM_dev_id;

/*Mass storage ids*/
typedef enum {  
    SU_NOSCRIPT     = 0, /*used when no script has ever been chosen to run, saved on SRAM*/
    SU_SCRIPT_1     = 1,
    SU_SCRIPT_2     = 2,
    SU_SCRIPT_3     = 3,
    SU_SCRIPT_4     = 4,
    SU_SCRIPT_5     = 5,
    SU_SCRIPT_6     = 6,
    SU_SCRIPT_7     = 7,
    SU_LOG          = 8,
    WOD_LOG         = 9,
    EXT_WOD_LOG     = 10,
    EVENT_LOG       = 11,
    FOTOS           = 12,
    SCHS            = 13,
    SRAM            = 14,
    LAST_SID        = 15
}MS_sid;

typedef enum {
    ALL         = 0,
    TO          = 1,
    BETWEEN     = 2,
    SPECIFIC    = 3,
    LAST_PART   = 4,
    NO_MODE     = 5,
    HARD_DELETE = 6,
    DELETE_ALL  = 7,
    FATFS_RESET = 8,
    LAST_MODE   = 9
}MS_mode;

typedef enum {
    SU_POWERED_OFF = 1,
    SU_POWERED_ON  = 2,
    SU_IDLE        = 3,
    SU_FINISHED    = 4,
    LAST_SU_STATE  = 5
}SU_state;

typedef enum {
    EV_inc_pkt           = 1,
    EV_pkt_ack_er        = 2,
    EV_sys_boot          = 3,
    EV_pwr_level         = 4,
    EV_comms_tx_off      = 5,
    EV_sys_timeout       = 6,
    EV_sys_shutdown      = 7,
    EV_assertion         = 8,
    EV_su_error          = 9,
    EV_su_scr_start      = 10,
    EV_pkt_pool_timeout  = 11,
    EV_ms_err            = 12,
    LAST_EV_EVENT        = 13
}EV_event;

//TODO: REMOVE THEM
typedef enum {
    SET_DTIME_UTC       = 1,
    SET_DTIME_QB50      = 2,
    REPORT_TIME_IN_UTC  = 3,
    REPORT_TIME_IN_QB50 = 4,
    LAST_TIME_ID        = 5
}TIME_MAN_MODE;

#define C_ASSERT(e)    ((e) ? (true) : (tst_debugging(__FILE_ID__, __LINE__, #e)))

union _cnv {
    double cnvD;
    float cnvF;
    uint32_t cnv32;
    uint16_t cnv16[4];
    uint8_t cnv8[8];
};

extern void HAL_uart_tx(TC_TM_app_id app_id, uint8_t *buf, uint16_t size);
extern SAT_returnState event_log(uint8_t *buf, const uint16_t size);
extern SAT_returnState event_crt_pkt_api(uint8_t *buf, uint8_t *f, uint16_t fi, uint32_t l, uint8_t *e, uint16_t *size, SAT_returnState mode);

typedef struct {
    /* packet id */
    //uint8_t ver; /* 3 bits, should be equal to 0 */

    //uint8_t data_field_hdr; /* 1 bit, data_field_hdr exists in data = 1 */
    TC_TM_app_id app_id; /* TM: app id = 0 for time packets, = 0xff for idle packets. should be 11 bits only 8 are used though */
    uint8_t type; /* 1 bit, tm = 0, tc = 1 */

    /* packet sequence control */
    uint8_t seq_flags; /* 3 bits, definition in TC_SEQ_xPACKET */
    uint16_t seq_count; /* 14 bits, packet counter, should be unique for each app id */

    uint16_t len; /* 16 bits, C = (Number of octets in packet data field) - 1, on struct is the size of data without the headers. on array is with the headers */

    uint8_t ack; /* 4 bits, definition in TC_ACK_xxxx 0 if its a TM */
    uint8_t ser_type; /* 8 bit, service type */
    uint8_t ser_subtype; /* 8 bit, service subtype */

    /*optional*/
    //uint8_t pckt_sub_cnt; /* 8 bits*/
    TC_TM_app_id dest_id;   /*on TC is the source id, on TM its the destination id*/

    uint8_t *data; /* pkt data */

    /*this is not part of the header. it is used from the software and the verification service,
     *when the packet wants ACK.
     *the type is SAT_returnState and it either stores R_OK or has the error code (failure reason).
     *it is initialized as R_ERROR and the service should be responsible to make it R_OK or put the corresponding error.
     */
    SAT_returnState verification_state;
/*  uint8_t padding;  x bits, padding for word alligment */

//  uint16_t crc; /* CRC or checksum, mission specific*/
}tc_tm_pkt;

/*Lookup table that returns if a service with its subtype with TC or TM is supported and valid*/
extern const uint8_t services_verification_TC_TM[MAX_SERVICES][MAX_SUBTYPES][2];
extern uint8_t assertion_last_file;
extern uint16_t assertion_last_line;

uint8_t tst_debugging(uint16_t fi, uint32_t l, char *e);

#endif
