#include "gps.h"

static struct _gps_state gps_state;

SAT_returnState gps_parse_fields(uint8_t *buf, const uint8_t size,  uint8_t *res[][]) {

    uint8_t gps_crc = 0;
    uint8_t field = 0;
    uint8_t cnt = 0;

    if(!C_ASSERT(size < NMEA_MAX_LEN) == true) { return SATR_ERROR; }

    for(uint8 i = 0; i < size; i++) {

        gps_crc = gps_crc ^ buf[i];

        if(buf[i] == ',') {

            data[field++][cnt] = 0;
            cnt = 0;

        } else if(buf[i] == '*') {

            data[field][cnt] = 0;

            uint8_t res_crc = strtol(&buf[i+1], &buf[i+2], 16);
            if(gps_crc != res_crc) { return SATR_PKT_INC_CRC; }
            else { return SATR_OK; }

        } else {
            data[field][cnt++] = buf[i];
        }

        if(!C_ASSERT(field < NMEA_MAX_FIELDS) == true)   { return SATR_ERROR; }
        if(!C_ASSERT(cnt < NMEA_MAX_FIELD_SIZE) == true) { return SATR_ERROR; }
    }
}

SAT_returnState gps_parse_logic(const uint8_t *res[][], struct _gps_state *gps_state) {

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
        gps_state.3dfix = strtol(&res[NMEA_GSA_3DFIX][0], &res[NMEA_GSA_3DFIX][MAX_FIELD_LEN], 10);
        gps_state.PDOP = strtod(&res[NMEA_GSA_PDOP][0], &res[NMEA_GSA_3DFIX][MAX_FIELD_LEN],10);
        gps_state.HDOP = strtod(&res[NMEA_GSA_HDOP][0], &res[NMEA_GSA_3DFIX][MAX_FIELD_LEN],10);
        gps_state.VDOP = strtod(&res[NMEA_GSA_VDOP][0], &res[NMEA_GSA_3DFIX][MAX_FIELD_LEN],10);
    }

}
