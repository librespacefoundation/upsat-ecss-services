#ifndef __GPS_H
#define __GPS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "services.h"


struct _gps_state
{
	uint8_t GSA_M;
};

SAT_returnState gps_parse_fields(uint8_t *buf, const uint8_t size,  uint8_t *res[][]);

SAT_returnState gps_parse_logic(const uint8_t *res[][], struct _gps_state *gps_state);

#endif
