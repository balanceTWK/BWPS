#include <chip/osal.h>

#include "bwps.h"
#include "bwps_port.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

static struct bwps_map_unit bwps_map[BWPS_TERMINAL_NUMBERS];

struct bwps_map_unit* bwps_get_map(void)
{
    return bwps_map;
}

bwps_error_t bwps_map_update_sequence(struct bwps_control_logic_data* data)
{
    uint32_t mac;
    uint16_t time_slot;
    mac = data->mac;
    time_slot = data->time_slot - 1;

    if(mac == bwps_map[time_slot].mac)
    {
        if(data->sequence != bwps_map[time_slot].sequence)
        {
            bwps_map[time_slot].sequence = data->sequence;
            return BWPS_OK;
        }
        else
        {
            LOG_D("Terminal data duplication. mac:0x%08X time_slot:%d sequence:%d",data->mac,data->time_slot,data->sequence);
            return BWPS_NOTHING;
        }
    }
    else
    {
        // bwps_map_add_mac(data->mac);
        if(bwps_map[time_slot].mac == 0)
        {
            LOG_W("Terminal mac add. mac:0x%08X time_slot:%d sequence:%d",data->mac,data->time_slot,data->sequence);
            bwps_map[time_slot].mac = mac;
            bwps_map[time_slot].sequence = data->sequence;
            return BWPS_OK;
        }
        else
        {
            LOG_E("Terminal transmission time slot problem. mac:0x%08X time_slot:%d sequence:%d",data->mac,data->time_slot,data->sequence);
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

bwps_error_t bwps_get_sequence_beacon_data(struct bwps_beacon_data* data)
{
    data->type = 0;
    data->mac = 0x00000000;

    for (int i = 0; i < (sizeof(bwps_map)/sizeof(struct bwps_map_unit)); i++)
    {
        data->union_data.sequence_data.sequence_buf[i] = bwps_map[i].sequence;
    }
    return BWPS_OK;
}

bwps_error_t bwps_get_mac_beacon_data_1(struct bwps_beacon_data* data)
{
    data->type = 1;
    data->mac = 0x00000000;

    for (int i = 0; i < 100; i++)
    {
        data->union_data.mac_data.mac_buf[i] = bwps_map[i].mac;
    }
    return BWPS_OK;
}

bwps_error_t bwps_get_mac_beacon_data_2(struct bwps_beacon_data* data)
{
    data->type = 2;
    data->mac = 0x00000000;

    for (int i = 0; i < 100; i++)
    {
        data->union_data.mac_data.mac_buf[i] = bwps_map[i+100].mac;
    }
    return BWPS_OK;
}

bwps_error_t bwps_map_delete_mac(uint32_t mac)
{
    for (int i = 0; i < sizeof(bwps_map)/sizeof(struct bwps_map_unit); i++)
    {
        if(bwps_map[i].mac == mac)
        {
            bwps_map[i].mac = 0;
            bwps_map[i].sequence = 0;
        }
    }

    return BWPS_OK;
}

bwps_error_t bwps_map_add_mac(uint32_t mac)
{
    bwps_map_delete_mac(mac);
    for (int i = 0; i < sizeof(bwps_map)/sizeof(struct bwps_map_unit); i++)
    {
        if(bwps_map[i].mac == 0)
        {
            bwps_map[i].mac = mac;
            bwps_map[i].sequence = 0;
            LOG_I("add mac:%08X index :%d",bwps_map[i].mac ,i + 1);
            return BWPS_OK;
        }
    }

    return BWPS_ERROR;
}



int bwps_data_cache_init(void)
{
    return BWPS_OK;
}

