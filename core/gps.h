#ifndef __GPS_H
#define __GPS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "services.h"

#define NMEA_MAX_FIELD_SIZE 15
#define NMEA_MAX_FIELDS     20
#define NMEA_HEADER         0
#define NMEA_HEADER_SIZE    6
#define NMEA_MAX_LEN        80

#define NMEA_GSA_3DFIX      2

#define NMEA_GPS_TIME       1
#define NMEA_GPS_WEEK       2

#define NMEA_LSP_X_WGS      3
#define NMEA_LSP_Y_WGS      4
#define NMEA_LSP_Z_WGS      5

#define NMEA_LSV_VX_WGS     3
#define NMEA_LSV_VY_WGS     4
#define NMEA_LSV_VZ_WGS     5

#define NMEA_GGA_TIME       1
#define NMEA_GGA_NUM_SAT    7

typedef struct {
    uint8_t d3fix;
    float x_wgs;
    float y_wgs;
    float z_wgs;
    float vx_wgs;
    float vy_wgs;
    float vz_wgs;
    float time;
    double gps_time;
    uint16_t gps_week;
    uint8_t num_sat;
} _gps_state;

extern _gps_state gps;

SAT_returnState gps_parse_fields(uint8_t *buf, const uint8_t size,  uint8_t (*res)[NMEA_MAX_FIELDS][NMEA_MAX_FIELD_SIZE]);

SAT_returnState gps_parse_logic(const uint8_t (*res)[NMEA_MAX_FIELDS][NMEA_MAX_FIELD_SIZE], _gps_state *gps_state);

#endif
