#include <chip/osal.h>

#include "bwps.h"
#include "bwps_port.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

static struct chip_os_task data_control_logic_layer_task;
static struct chip_os_queue data_control_logic_layer_queue;

bwps_error_t bwps_data_control_logic_layer_put(struct bwps_control_logic_data *data)
{
    chip_os_queue_put(&data_control_logic_layer_queue, data);

    return BWPS_OK;
}

void *bwps_data_control_logic_thread(void *args)
{
    static struct bwps_control_logic_data data;
    while (1)
    {
        chip_os_queue_get(&data_control_logic_layer_queue, &data, CHIP_OS_TIME_FOREVER);
        if(data.time_slot == 0) /* 广播时隙，需要分配时隙。 */
        {
            LOG_W("mac:%08X time_slot:%d len:%d",data.mac,data.time_slot,data.len);
            bwps_map_add_mac(data.mac);
        }
        else
        {
            if (BWPS_OK == bwps_map_update_sequence(&data))
            {   /* 序列号有变化，可以做处理。 */
                LOG_I("mac:%08X time_slot:%d sequence:%d", data.mac, data.time_slot, data.sequence);
            }
            else
            {   /* 终端数据重复或者发送时隙存在问题。 */
                LOG_E("Terminal data duplication or transmission time slot problem.");
            }
        }
        chip_os_task_sleep_ms(10);
    }
}

int bwps_data_control_logic_layer_init(void)
{
    bwps_error_t osal_err;

    osal_err = chip_os_queue_init(&data_control_logic_layer_queue, sizeof(struct bwps_control_logic_data), 10);
    osal_err = chip_os_task_init(&data_control_logic_layer_task, "bwps_data_control_logic_layer", bwps_data_control_logic_thread, &data_control_logic_layer_queue, 23, 2048);

    return (osal_err == BWPS_OK) ? 0 : -1;
}
