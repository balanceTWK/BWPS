#ifndef H_BWPS_DATA_CACHE_
#define H_BWPS_DATA_CACHE_

#ifdef __cplusplus
extern "C"
{
#endif

#include "bwps.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

struct bwps_list_node
{
    struct bwps_list_node *next; /**< point to next node. */
    struct bwps_list_node *prev; /**< point to prev node. */
};
typedef struct bwps_list_node bwps_list_node_t; /**< Type for lists. */

struct bwps_data_cache_node
{
    bwps_list_node_t list;
    bwps_control_logic_data_t data;
};

struct bwps_map_unit
{
    uint32_t mac;
    uint16_t sequence;
};

int bwps_data_cache_init(void);
bwps_error_t bwps_map_update_sequence(struct bwps_control_logic_data *data);
bwps_error_t bwps_get_sequence_beacon_data(struct bwps_beacon_data *data);
bwps_error_t bwps_get_mac_beacon_data_1(struct bwps_beacon_data *data);
bwps_error_t bwps_get_mac_beacon_data_2(struct bwps_beacon_data *data);
bwps_error_t bwps_map_add_mac(uint32_t mac);
bwps_error_t bwps_map_delete_mac(uint32_t mac);
struct bwps_map_unit *bwps_get_map(void);
void bwps_data_cache_add(struct bwps_control_logic_data *data);
bwps_error_t bwps_data_cache_get(struct bwps_control_logic_data *out_data);

#ifdef __cplusplus
}
#endif

#endif /* H_BWPS_DATA_CACHE_ */
