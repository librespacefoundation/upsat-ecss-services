#include "gps.h"

static struct _gps_state gps_state;

SAT_returnState gps_parse(uint8_t *buf, const uint8 size, struct _gps_state gps) {

    if(strncmp(buf, "$GGA", NMEA_HEADER_SIZE) == 0) {
        uint8_t temp[10];
        const uint8_t ch = ',';
        uint8_t *begin = 0;
        uint8_t *end = 0;

        begin = memchr(buf, ch, NMEA_MAX_LEN);
        if(begin == NULL) { return SATR_ERRROR; }

        end = memchr(++begin, ch, (NMEA_MAX_LEN - (begin - buf)));
        if(end == NULL) { return SATR_ERRROR; }

        memcpy(temp,begin, (end - 1 - begin));


        begin = end;

        end = memchr(++begin, ch, (NMEA_MAX_LEN - (begin - buf)));
        if(end == NULL) { return SATR_ERRROR; }

        memcpy(temp,begin, (end - 1 - begin));


    }

}