#ifndef H_BWPS_HOST_LAYER_
#define H_BWPS_HOST_LAYER_

#ifdef __cplusplus
extern "C" {
#endif

#include "bwps.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

typedef bwps_error_t (*bwps_host_exe)(struct bwps_control_logic_data* data);
bwps_error_t bwps_host_start(bwps_host_exe func);

#ifdef __cplusplus
}
#endif

#endif /* H_BWPS_HOST_LAYER_ */
