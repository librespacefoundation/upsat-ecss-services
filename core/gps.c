#include "gps.h"

static struct _gps_state gps_state;

SAT_returnState gps_parse(uint8_t *buf, const uint8 size, struct _gps_state gps) {

    uint8_t res_crc = 0;
    uint8_t gps_crc = 0;
    checkSum(*buf, size - 3, res_crc);
    gps_crc = strtol(&buf[size - 2], &buf[size], 16);

    if(gps_crc != res_crc) { return SATR_PKT_INC_CRC; }

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

    if(strncmp(buf, "$GPGSA", NMEA_HEADER_SIZE) == 0) {
        uint8_t temp[10];
        const uint8_t ch = ',';
        uint8_t *begin = 0;
        uint8_t *end = 0;

        begin = memchr(buf, ch, NMEA_MAX_LEN);
        if(begin == NULL) { return SATR_ERRROR; }

        if((end = memchr(++begin, ch, (NMEA_MAX_LEN - (begin - buf)))) == NULL) { return SATR_ERRROR; }
        gps.GSA_M = begin;

        begin = end;

        if((end = memchr(++begin, ch, (NMEA_MAX_LEN - (begin - buf)))) == NULL) { return SATR_ERRROR; }

        gps.GSA_3DFIX = strtol(begin, end -1, 10);

        for(uint8_t i = 0; i < PRNs; i++) :
         
            begin = end;
            if(begin == end -1) {
                gps.GSA_PRNs[i] = 0;
                begin++;
                end++;
                continue
            }
            if((end = memchr(++begin, ch, (NMEA_MAX_LEN - (begin - buf)))) == NULL) { return SATR_ERRROR; }

            gps.GSA_PRNs[i] = strtol(begin, end -1, 10);
        }

        begin = end;

        if((end = memchr(++begin, ch, (NMEA_MAX_LEN - (begin - buf)))) == NULL) { return SATR_ERRROR; }

        gps.GSA_PDOP = strtod(begin, end -1, 10);

        begin = end;

        if((end = memchr(++begin, ch, (NMEA_MAX_LEN - (begin - buf)))) == NULL) { return SATR_ERRROR; }

        gps.GSA_HDOP = strtod(begin, end -1, 10);

        begin = end;

        if((end = memchr(++begin, "*", (NMEA_MAX_LEN - (begin - buf)))) == NULL) { return SATR_ERRROR; }

        gps.GSA_VDOP = strtod(begin, end -1, 10);

    }

}