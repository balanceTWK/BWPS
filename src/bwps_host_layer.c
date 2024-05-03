#include <chip/osal.h>

#include "bwps.h"
#include "bwps_port.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"
#include "bwps_host_layer.h"

static struct chip_os_task bwps_host_task;

void * bwps_host_layer_thread(void * args)
{
    bwps_error_t status;
    struct bwps_control_logic_data data;
    bwps_host_exe bwps_host_exe_func;

    bwps_host_exe_func = (bwps_host_exe)args;
    while (1)
    {
        chip_os_task_sleep_ms(100);
        status = bwps_data_cache_get(&data);
        if(status == BWPS_OK)
        {
            if(bwps_host_exe_func)
            {
                bwps_host_exe_func(&data);
            }
        }
    }
}

bwps_error_t bwps_host_start(bwps_host_exe func)
{
    bwps_error_t osal_err;

    osal_err = chip_os_task_init(&bwps_host_task, "bwps_host_task", bwps_host_layer_thread, func, 20, 2048);

    return (osal_err == BWPS_OK) ? 0 : -1;
}
