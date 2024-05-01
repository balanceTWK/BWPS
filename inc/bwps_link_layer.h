#ifndef H_BWPS_LINK_LAYER_
#define H_BWPS_LINK_LAYER_

#ifdef __cplusplus
extern "C" {
#endif

#include "bwps.h"

typedef struct bwps_raw_data
{
    uint32_t head;
    uint32_t mac;
    uint16_t time_slot;
    int16_t rssi;
    uint16_t sequence;
    uint16_t len;
    uint32_t crc;
    uint8_t buf[580];
} bwps_raw_data_t;

typedef struct bwps_beacon_sequence_data
{
    uint8_t reserve_1[72];
    uint16_t sequence_buf[200];
    uint16_t reserve_2[50];
} bwps_beacon_sequence_data_t;

typedef struct bwps_beacon_mac_data
{
    uint8_t reserve_1[72];
    uint32_t mac_buf[100];
    uint32_t reserve_2[25];
} bwps_beacon_mac_data_t;

union bwps_beacon_data_union
{
    bwps_beacon_sequence_data_t sequence_data;
    bwps_beacon_mac_data_t mac_data;
};

typedef struct bwps_beacon_data
{
    uint32_t type;
    uint32_t mac;
    union bwps_beacon_data_union union_data;
} bwps_beacon_data_t;


typedef bwps_error_t (*bwps_low_level_send_func)(struct bwps_raw_data* data, int mode);

int bwps_link_layer_init(bwps_low_level_send_func func);
bwps_error_t bwps_link_layer_put(struct bwps_raw_data* data);

#ifdef __cplusplus
}
#endif

#endif /* H_BWPS_LINK_LAYER_ */
