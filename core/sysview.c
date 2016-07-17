#include "sysview.h"

#if(SYSVIEW == 1)

SEGGER_SYSVIEW_MODULE PKT_POOL_module = {
    "M=PKT_POOL, " \
    "0 GetPKT ID=%u, " \
    "1 FreePKT ID=%u", // sModule
    2, // NumEvents
    0,
    // EventOffset, Set by SEGGER_SYSVIEW_RegisterModule() NULL,
    // pfSendModuleDesc, NULL: No additional module description NULL,
    // pNext, Set by SEGGER_SYSVIEW_RegisterModule()
};

SEGGER_SYSVIEW_MODULE MS_module = {
    "M=MS, " \
    "0 Downlink started, " \
    "1 Downlink stoped, " \
    "2 STORE started, " \
    "3 STORE stoped, " \
    "4 REP started, " \
    "5 REP stoped, " \
    "6 LIST started, " \
    "7 LIST stoped, " \
    "8 DEL started, " \
    "9 DEL stoped, " \
    "10 FORM started, " \
    "11 FORM stoped", // sModule
    12, // NumEvents
    0,
    // EventOffset, Set by SEGGER_SYSVIEW_RegisterModule() NULL,
    // pfSendModuleDesc, NULL: No additional module description NULL,
    // pNext, Set by SEGGER_SYSVIEW_RegisterModule()
};

SEGGER_SYSVIEW_MODULE COMMS_module = {
    "M=COMMS, " \
    "0 IMPORT ID=%u, " \
    "1 EXPORT ID=%u", // sModule
    2, // NumEvents
    0,
    // EventOffset, Set by SEGGER_SYSVIEW_RegisterModule() NULL,
    // pfSendModuleDesc, NULL: No additional module description NULL,
    // pNext, Set by SEGGER_SYSVIEW_RegisterModule()
};

SEGGER_SYSVIEW_MODULE ASSERTION_module = {
    "M=ASSERTION, " \
    "0 ASSERTION ID=%u,%u,%u",
    1, // NumEvents
    0,
    // EventOffset, Set by SEGGER_SYSVIEW_RegisterModule() NULL,
    // pfSendModuleDesc, NULL: No additional module description NULL,
    // pNext, Set by SEGGER_SYSVIEW_RegisterModule()
};

void sysview_init() {
    SEGGER_SYSVIEW_RegisterModule(&PKT_POOL_module);
    SEGGER_SYSVIEW_RegisterModule(&MS_module);
    SEGGER_SYSVIEW_RegisterModule(&COMMS_module);
    SEGGER_SYSVIEW_RegisterModule(&ASSERTION_module);
}

#else

void sysview_init() {

}

#endif