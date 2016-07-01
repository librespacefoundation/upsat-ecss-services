#ifndef __SYSVIEW_H
#define __SYSVIEW_H

#define SYSVIEW  0 /* define as 1 if you want to use systemview */

#if(SYSVIEW == 1)

#include "segger_sysview.h"

#define ID_GETPKT  0
#define ID_FREEPKT 1

#define traceGET_PKT(ID) SEGGER_SYSVIEW_RecordU32(ID_GETPKT + PKT_POOL_module.EventOffset, ID)
#define traceFREE_PKT(ID) SEGGER_SYSVIEW_RecordU32(ID_FREEPKT + PKT_POOL_module.EventOffset, ID)

extern SEGGER_SYSVIEW_MODULE PKT_POOL_module;

void sysview_init();

#else

#define traceGET_PKT(ID)   ;
#define traceFREE_PKT(ID)  ;

#endif

#endif
