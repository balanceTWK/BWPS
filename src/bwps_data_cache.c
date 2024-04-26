#include <chip/osal.h>

#include "bwps.h"
#include "bwps_port.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

static struct bwps_map_unit bwps_map[200];

bwps_error_t bwps_map_update_sequence(struct bwps_control_logic_data* data)
{
    uint32_t mac;
    uint16_t time_slot;
    mac = data->mac;
    time_slot = data->time_slot;

    if(mac == bwps_map[time_slot].mac)
    {
        if(data->sequence != bwps_map[time_slot].sequence)
        {
            bwps_map[time_slot].sequence = data->sequence;
            return BWPS_OK;
        }
    }
    return BWPS_ERROR;
}

bwps_error_t bwps_map_update_mac(uint32_t mac, uint16_t time_slot)
{
    bwps_map[time_slot].mac = mac;
    bwps_map[time_slot].sequence = 0;

    return BWPS_OK;
}

bwps_error_t bwps_get_map_beacon_data(struct bwps_beacon_data* data)
{
    data->type = 0;
    data->mac = 0x00000000;

    for (int i = 0; i < (sizeof(bwps_map)/sizeof(struct bwps_map_unit)); i++)
    {
        data->buf[i] = bwps_map[i].sequence;
    }
    return BWPS_OK;
}

int bwps_data_cache_init(void)
{
    bwps_map[1].mac = 0x11111111;
    bwps_map[1].sequence = 0;

    return BWPS_OK;
}

