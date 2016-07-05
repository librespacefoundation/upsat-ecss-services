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

void sysview_init() {
    SEGGER_SYSVIEW_RegisterModule(&PKT_POOL_module);

}

#else

void sysview_init() {

}

#endif