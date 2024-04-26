#ifndef H_BWPS_DATA_CONTROL_LOGIC_LAYER_
#define H_BWPS_DATA_CONTROL_LOGIC_LAYER_

#ifdef __cplusplus
extern "C" {
#endif

#include "bwps.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"
typedef struct bwps_control_logic_data
{
    uint32_t mac;
    uint16_t sequence;
    uint16_t time_slot;
    uint16_t len;
    uint8_t buf[580];
} bwps_control_logic_data_t;

int bwps_data_control_logic_layer_init(void);
bwps_error_t bwps_data_control_logic_layer_put(struct bwps_control_logic_data* data);

#ifdef __cplusplus
}
#endif

#endif /* H_BWPS_DATA_CONTROL_LOGIC_LAYER_ */
