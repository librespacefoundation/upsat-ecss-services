#ifndef MASS_STORAGE_SERVICE_H
#define MASS_STORAGE_SERVICE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fatfs.h"
#include "services.h"
#include "su_mnlp.h"
#include "pkt_pool.h"

#define MS_SD_PATH "0:"

#define MS_SU_LOG          "/SU_LOG"
#define MS_WOD_LOG         "/WOD_LOG"
#define MS_EXT_WOD_LOG     "/EXT_WOD"
#define MS_SU_SCRIPT_1     "/SU_SCR_1/SCR1.bin"
#define MS_SU_SCRIPT_2     "/SU_SCR_2/SCR2.bin"
#define MS_SU_SCRIPT_3     "/SU_SCR_3/SCR3.bin"
#define MS_SU_SCRIPT_4     "/SU_SCR_4/SCR4.bin"
#define MS_SU_SCRIPT_5     "/SU_SCR_5/SCR5.bin"
#define MS_SU_SCRIPT_6     "/SU_SCR_6/SCR6.bin"
#define MS_SU_SCRIPT_7     "/SU_SCR_7/SCR7.bin"
#define MS_EVENT_LOG       "/EV_LOG"
#define MS_FOTOS           "/FOTOS"
#define MS_SCHS            "/SCHS"

#define MS_MAX_PATH             40 //random num
#define MS_MAX_FILES            0x5F5E0FF //random num
#define MS_MAX_FNAME            30 //random num
#define MS_MAX_LOG_FILE_SIZE    198 /*MAX_PKT_DATA*/ 
#define MS_MAX_SU_FILE_SIZE     2048 //2k
#define MS_FILE_SECTOR          512
#define MS_STORES               3
#define MS_SU_FSIZE             174
#define MS_MIN_SU_FILE 			1	//min is the header.
#define MAX_F_RETRIES			3

struct _MS_data {
    FATFS Fs;
    uint8_t enabled;
};

extern struct _MNLP_data MNLP_data;

extern uint32_t get_new_fileId(MS_sid sid);

//extern SAT_returnState su_populate_header( science_unit_script_header *hdr, uint8_t *buf);

//extern science_unit_script_inst su_scripts[];

//ToDo
//  add format for sd
//	check type casting for snprintf & %d conversions
//	error checking for sprintf
//  strtol proper checking, return value range is ok
//  error checking in return values of fatfs
//  check various equations
//  assert, require.
//  finish definitions, documentation and doc.
//  check for EOF
//  maybe file paths should be const variable instead of definitions.

//Finished
//  add global counters for file and size, add check for array limits.
//  add check for MAX_FILE for loop, hard limit.
//  check if sprintf is ok for the job, used snprintf

SAT_returnState mass_storage_init();

SAT_returnState mass_storage_app(tc_tm_pkt *pkt);

SAT_returnState mass_storage_delete_api(MS_sid sid, uint32_t to, MS_mode mode);

SAT_returnState mass_storage_delete_su_scr(MS_sid sid);

SAT_returnState mass_storage_downlink_api(tc_tm_pkt *pkt, MS_sid sid, uint32_t file) ;

SAT_returnState mass_storage_store_api(MS_sid sid, MS_mode mode, uint8_t *buf, uint16_t *size, uint32_t part);

SAT_returnState mass_storage_report_api(tc_tm_pkt *pkt, MS_sid sid);

SAT_returnState mass_storage_report(MS_sid sid, uint8_t *buf, uint16_t *size, uint32_t *iter);

SAT_returnState mass_storage_report_su_scr(MS_sid sid, uint8_t *buf, uint16_t *size);

SAT_returnState mass_storage_su_checksum_api(MS_sid sid);

SAT_returnState mass_storage_su_load_api(MS_sid sid, uint8_t *buf);


SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);

SAT_returnState mass_storage_downlinkLogs(MS_sid sid, MS_mode mode, uint32_t from, uint32_t to, uint8_t *buf, uint16_t *size, uint32_t *part);

SAT_returnState mass_storage_downlinkFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);

SAT_returnState mass_storage_getLog(MS_sid sid, uint8_t *fn);

SAT_returnState mass_storage_findLog(MS_sid sid, uint32_t *fn);

SAT_returnState mass_storage_getFileName(uint8_t *fn);

SAT_returnState mass_storage_getFileSizeCount(MS_sid sid);


SAT_returnState mass_storage_crtPkt(tc_tm_pkt **pkt, const uint16_t dest_id, const uint16_t size);

SAT_returnState mass_storage_updatePkt(tc_tm_pkt *pkt, uint16_t size, uint8_t subtype);


SAT_returnState mass_storage_FORMAT();

SAT_returnState mass_storage_dirCheck();

void mass_storage_getState(uint8_t *state);

#endif
