#include "ecss_stats.h"
#include "service_utilities.h"
#include "sysview.h"

#undef __FILE_ID__
#define __FILE_ID__ 39

struct _ecss_stats
{
    uint16_t inbound_packet[LAST_APP_ID][LAST_APP_ID];
    uint16_t outbound_packet[LAST_APP_ID][LAST_APP_ID];
    uint16_t dropped_hldlc;
    uint16_t dropped_unpack;
};

static struct _ecss_stats ecss_stats = { .inbound_packet = {{ 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0}} ,
                                        .outbound_packet = {{ 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0},
                                                            { 0, 0}} ,
                                         .dropped_hldlc = 0,
                                         .dropped_unpack = 0 }; 

void stats_inbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt) {

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)               { return ; }
    if(!C_ASSERT(app_id < LAST_APP_ID && dest_id < LAST_APP_ID) == true)  { return ; }

    TC_TM_app_id source = 0;
    TC_TM_app_id dest = 0;

    if(type == TC) {
        source = app_id;
        dest = dest_id;
    }
    else if(type == TM) {
        dest = app_id;
        source = dest_id;
    }

    ecss_stats.inbound_packet[source][dest]++;

    SYSVIEW_PRINT("IN %u,%u,%u,%u", source, dest, pkt->seq_count, pkt->ser_type);
}

void stats_outbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt) {

    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true)               { return ; }
    if(!C_ASSERT(app_id < LAST_APP_ID && dest_id < LAST_APP_ID) == true)  { return ; }

    TC_TM_app_id source = 0;
    TC_TM_app_id dest = 0;

    if(type == TC) {
        source = app_id;
        dest = dest_id;
    }
    else if(type == TM) {
        dest = app_id;
        source = dest_id;
    }

    ecss_stats.outbound_packet[source][dest]++;

    SYSVIEW_PRINT("OUT %u,%u,%u,%u", source, dest, pkt->seq_count, pkt->ser_type);
}

void stats_dropped_hldlc() {
    ecss_stats.dropped_hldlc++;
}

void stats_dropped_upack() {
    ecss_stats.dropped_unpack++;
}

uint16_t ecss_stats_hk(uint8_t *buffer) {
    uint16_t pointer = 0;

    cnv16_8(ecss_stats.dropped_hldlc, &buffer[pointer]);
    pointer += sizeof(uint16_t);

    cnv16_8(ecss_stats.dropped_unpack, &buffer[pointer]);
    pointer += sizeof(uint16_t);

    for(uint8_t a = 1; a < LAST_APP_ID; a++) {
        for(uint8_t b = 1; b < LAST_APP_ID; b++) {
            cnv16_8(ecss_stats.inbound_packet[a][b], &buffer[pointer]);
            pointer += sizeof(uint16_t);
        }
    }
    for(uint8_t a = 1; a < LAST_APP_ID; a++) {
        for(uint8_t b = 1; b < LAST_APP_ID; b++) {
            cnv16_8(ecss_stats.outbound_packet[a][b], &buffer[pointer]);
            pointer += sizeof(uint16_t);
        }
    }

    return pointer;
}

