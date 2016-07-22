#include "gps.h"

#undef __FILE_ID__
#define __FILE_ID__ 37

_gps_state gps;

SAT_returnState gps_parse_fields(uint8_t *buf, const uint8_t size,  uint8_t (*res)[NMEA_MAX_FIELDS][NMEA_MAX_FIELD_SIZE]) {

    uint8_t gps_crc = 0;
    uint8_t field = 0;
    uint8_t cnt = 0;

    if(!C_ASSERT(size < NMEA_MAX_LEN) == true) { return SATR_ERROR; }

    for(uint8_t i = 0; i < size; i++) {

        gps_crc = gps_crc ^ buf[i];

        if(buf[i] == ',') {

            *res[field++][cnt] = 0;
            cnt = 0;

        } else if(buf[i] == '*') {

            *res[field][cnt] = 0;

            uint8_t res_crc = strtol(&buf[i+1], &buf[i+2], 16);
            if(gps_crc != res_crc) { return SATR_PKT_INC_CRC; }
            else { return SATR_OK; }

        } else {
            *res[field][cnt++] = buf[i];
        }

        if(!C_ASSERT(field < NMEA_MAX_FIELDS) == true)   { return SATR_ERROR; }
        if(!C_ASSERT(cnt < NMEA_MAX_FIELD_SIZE) == true) { return SATR_ERROR; }
    }

    return SATR_OK;
}

SAT_returnState gps_parse_logic(const uint8_t (*res)[NMEA_MAX_FIELDS][NMEA_MAX_FIELD_SIZE], _gps_state *gps_state) {

   /*
    $GPGSA,M,3,31,32,22,24,19,11,17,14,20,,,,1.6,0.9,1.3*3E
 
        GSA Satellite status
        M Auto selection of 2D or 3D fix (M = manual) 
        3 3D fix - values include: 1 = No Fix
                                   2 = 2D Fix
                                   3 = 3D Fix 
        31,32... PRNs of satellites used for Fix (space for 12)
        1.6 Position Dilution of Precision (PDOP) 
        0.9 Horizontal Dilution of Precision (HDOP) 
        1.3 Vertical Dilution of Precision (VDOP) 
        *3e The checksum data, always begin with *
    */

    if(strncmp("$GPGSA", &res[NMEA_HEADER][0], NMEA_HEADER_SIZE) == 0) {
        gps_state->d3fix = strtol(&res[NMEA_GSA_3DFIX][0], &res[NMEA_GSA_3DFIX][NMEA_MAX_FIELD_SIZE], 10);
    }
    /*
     LSP – LEO Satellite Position – The piNAV SENTENCE
     $PSLSP,193772.0585851,780,3963889.204,1001383.917,4879428.991,5,4.5*72
     LEO satellite position
     GPS time [s]
     GPS week
     X position referenced to WGS-84 [m]
     Y position referenced to WGS-84 [m]
     Z position referenced to WGS-84 [m]
     Number of satellites used for PVT
     Position Dilution of Precision (PDOP)
     The checksum data, always begin with *
     */

    else if(strncmp("$PSLSP", &res[NMEA_HEADER][0], NMEA_HEADER_SIZE) == 0) {
        gps_state->gps_time = strtof(&res[NMEA_GPS_TIME][0], &res[NMEA_GPS_TIME][NMEA_MAX_FIELD_SIZE]);
        gps_state->gps_week = strtol(&res[NMEA_GPS_WEEK][0], &res[NMEA_GPS_WEEK][NMEA_MAX_FIELD_SIZE], 10);
        gps_state->x_wgs = strtof(&res[NMEA_LSP_X_WGS][0], &res[NMEA_LSP_X_WGS][NMEA_MAX_FIELD_SIZE]);
        gps_state->y_wgs = strtof(&res[NMEA_LSP_Y_WGS][0], &res[NMEA_LSP_Y_WGS][NMEA_MAX_FIELD_SIZE]);
        gps_state->z_wgs = strtof(&res[NMEA_LSP_Z_WGS][0], &res[NMEA_LSP_Z_WGS][NMEA_MAX_FIELD_SIZE]);
    }

    /*
     LSV – LEO Satellite Velocity – The piNAV SENTENCE
     $PSLSV,193772.0585851,780,0.051,0.017,0.034,5,4.5*7B
     LEO satellite velocity
     GPS time [s] to which the rising edge of the Valid Position Pulse (VPP) was calculated
     GPS week
     v X velocity referenced to WGS-84 [m/s]
     v Y velocity referenced to WGS-84 [m/s]
     v Z velocity referenced to WGS-84 [m/s]
     Number of satellites used for PVT
     Position Dilution of Precision (PDOP)
     The checksum data, always begin with *
     */

    else if(strncmp("$PSLSV", &res[NMEA_HEADER][0], NMEA_HEADER_SIZE) == 0) {
        gps_state->vx_wgs = strtof(&res[NMEA_LSV_VX_WGS][0], &res[NMEA_LSV_VX_WGS][NMEA_MAX_FIELD_SIZE]);
        gps_state->vy_wgs = strtof(&res[NMEA_LSV_VY_WGS][0], &res[NMEA_LSV_VY_WGS][NMEA_MAX_FIELD_SIZE]);
        gps_state->vz_wgs = strtof(&res[NMEA_LSV_VZ_WGS][0], &res[NMEA_LSV_VZ_WGS][NMEA_MAX_FIELD_SIZE]);
    }

    /*
     GGA - Fix Data - The NMEA SENTENCE
     $GPGGA,172120.384,5219.0671,N,05117.0926,E,1,9,0.9,371262.1,M,0,M,,,*54
     Global Positioning System Fix Data
     Fix taken at 17:21:20.384 UTC
     Latitude 52 deg 19.0671' N
     Longitude 51 deg 17.0926' E
     Fix quality:
     0 = Invalid
     1 = GPS Fix (SPS)
     Number of satellites being tracked
     Horizontal Dilution of Precision (HDOP)
     Altitude, meters, above WGS84 ellipsoid 1
     Height of the Geoid (mean sea level) above WGS84 ellipsoid
     Time in seconds since last DGPS update
     DGPS station ID number
     The checksum data, always begin with *
     */

    else if(strncmp("$GPGGA", &res[NMEA_HEADER][0], NMEA_HEADER_SIZE) == 0) {
        gps_state->time = strtof(&res[NMEA_GGA_TIME][0], &res[NMEA_GGA_TIME][NMEA_MAX_FIELD_SIZE]);
        gps_state->num_sat = strtof(&res[NMEA_GGA_NUM_SAT][0], &res[NMEA_GGA_NUM_SAT][NMEA_MAX_FIELD_SIZE]);
    }

    return SATR_OK;

}
