#include <chip/osal.h>

#include "bwps.h"
#include "bwps_port.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

static bwps_list_node_t bwps_data_cache_list;
static struct bwps_map_unit bwps_map[BWPS_TERMINAL_NUMBERS];
static struct chip_os_mutex bwps_data_cache_mutex;

void bwps_data_cache_list_init(bwps_list_node_t *l)
{
    l->next = l->prev = l;
}

void bwps_data_cache_list_insert_after(bwps_list_node_t *l, bwps_list_node_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

void bwps_data_cache_list_insert_before(bwps_list_node_t *l, bwps_list_node_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

void bwps_data_cache_list_remove(bwps_list_node_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

int bwps_data_cache_list_isempty(const bwps_list_node_t *l)
{
    return l->next == l;
}

int bwps_data_cache_list_len(const bwps_list_node_t *l)
{
    unsigned int len = 0;
    const bwps_list_node_t *p = l;
    while (p->next != l)
    {
        p = p->next;
        len++;
    }

    return len;
}

struct bwps_map_unit *bwps_get_map(void)
{
    return bwps_map;
}

bwps_error_t bwps_map_update_sequence(struct bwps_control_logic_data *data)
{
    uint32_t mac;
    uint16_t time_slot;
    mac = data->mac;
    time_slot = data->time_slot - 1;

    if (mac == bwps_map[time_slot].mac)
    {
        if (data->sequence != bwps_map[time_slot].sequence)
        {
            bwps_map[time_slot].sequence = data->sequence;
            return BWPS_OK;
        }
        else
        {
            LOG_D("Terminal data duplication. mac:0x%08X time_slot:%d sequence:%d", data->mac, data->time_slot, data->sequence);
            return BWPS_NOTHING;
        }
    }
    else
    {
        // bwps_map_add_mac(data->mac);
        if (bwps_map[time_slot].mac == 0)
        {
            LOG_W("Terminal mac add. mac:0x%08X time_slot:%d sequence:%d", data->mac, data->time_slot, data->sequence);
            bwps_map[time_slot].mac = mac;
            bwps_map[time_slot].sequence = data->sequence;
            return BWPS_OK;
        }
        else
        {
            LOG_E("Terminal transmission time slot problem. mac:0x%08X time_slot:%d sequence:%d", data->mac, data->time_slot, data->sequence);
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

bwps_error_t bwps_get_sequence_beacon_data(struct bwps_beacon_data *data)
{
    data->type = 0;
    data->mac = 0x00000000;

    for (int i = 0; i < (sizeof(bwps_map) / sizeof(struct bwps_map_unit)); i++)
    {
        data->union_data.sequence_data.sequence_buf[i] = bwps_map[i].sequence;
    }
    return BWPS_OK;
}

bwps_error_t bwps_get_mac_beacon_data_1(struct bwps_beacon_data *data)
{
    data->type = 1;
    data->mac = 0x00000000;

    for (int i = 0; i < 100; i++)
    {
        data->union_data.mac_data.mac_buf[i] = bwps_map[i].mac;
    }
    return BWPS_OK;
}

bwps_error_t bwps_get_mac_beacon_data_2(struct bwps_beacon_data *data)
{
    data->type = 2;
    data->mac = 0x00000000;

    for (int i = 0; i < 100; i++)
    {
        data->union_data.mac_data.mac_buf[i] = bwps_map[i + 100].mac;
    }
    return BWPS_OK;
}

bwps_error_t bwps_map_delete_mac(uint32_t mac)
{
    for (int i = 0; i < sizeof(bwps_map) / sizeof(struct bwps_map_unit); i++)
    {
        if (bwps_map[i].mac == mac)
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
    for (int i = 0; i < sizeof(bwps_map) / sizeof(struct bwps_map_unit); i++)
    {
        if (bwps_map[i].mac == 0)
        {
            bwps_map[i].mac = mac;
            bwps_map[i].sequence = 0;
            LOG_I("add mac:%08X index :%d", bwps_map[i].mac, i + 1);
            return BWPS_OK;
        }
    }

    return BWPS_ERROR;
}

void bwps_data_cache_add(struct bwps_control_logic_data *data)
{
    struct bwps_data_cache_node *new_node;
    new_node = 0;
    chip_os_mutex_take(&bwps_data_cache_mutex, CHIP_OS_TIME_FOREVER);
    new_node = bwps_malloc(sizeof(struct bwps_data_cache_node));

    if (new_node)
    {
        new_node->data.mac = data->mac;
        new_node->data.sequence = data->sequence;
        new_node->data.time_slot = data->time_slot;
        new_node->data.len = data->len;
        memcpy(&(new_node->data.buf), data->buf, data->len);
        bwps_data_cache_list_insert_after(&bwps_data_cache_list, &(new_node->list));
    }
    chip_os_mutex_give(&bwps_data_cache_mutex);
}

bwps_error_t bwps_data_cache_get(struct bwps_control_logic_data *out_data)
{
    struct bwps_data_cache_node *temp;
    bwps_error_t ret;
    ret = BWPS_ERROR;
    chip_os_mutex_take(&bwps_data_cache_mutex, CHIP_OS_TIME_FOREVER);
    if (out_data)
    {
        if (!bwps_data_cache_list_isempty(&bwps_data_cache_list))
        {
            temp = (struct bwps_data_cache_node *)(bwps_data_cache_list.prev);
            LOG_D("prev mac:0x%08X", temp->data.mac);
            memcpy(out_data, temp, sizeof(struct bwps_control_logic_data));
            bwps_free(bwps_data_cache_list.prev);
            bwps_data_cache_list_remove(bwps_data_cache_list.prev);
            ret = BWPS_OK;
        }
    }
    chip_os_mutex_give(&bwps_data_cache_mutex);
    return ret;
}

int bwps_data_cache_init(void)
{
    chip_os_mutex_init(&bwps_data_cache_mutex);
    bwps_data_cache_list_init(&bwps_data_cache_list);
    return BWPS_OK;
}
