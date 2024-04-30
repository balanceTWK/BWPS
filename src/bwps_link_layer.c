#include <chip/osal.h>

#include "bwps.h"
#include "bwps_port.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

static struct chip_os_task link_layer_task;
static struct chip_os_queue link_layer_queue;
static struct chip_os_sem link_layer_sem;

uint32_t bwps_timeslot_map[200];

bwps_error_t bwps_ll_crc(struct bwps_raw_data* data)
{
    uint32_t current_crc = 0;
    for (int i = 0; i < data->len; i++)
    {
        current_crc+=data->buf[i];
    }
    current_crc = ~current_crc;
    if(data->crc == current_crc)
    {
        return BWPS_OK;
    }
    else
    {
        return BWPS_ERROR;
    }
}

bwps_error_t bwps_link_layer_put(struct bwps_raw_data* data)
{
    chip_os_queue_put(&link_layer_queue, data);

    return BWPS_OK;
}

void * bwps_link_layer_thread(void * args)
{
    static struct chip_os_queue* mq;
    static struct bwps_raw_data data;
    static struct bwps_control_logic_data up_data;

    mq = (struct chip_os_queue*)args;
    while (1)
    {
        chip_os_queue_get(mq, &data, CHIP_OS_TIME_FOREVER);

        /* TODO:1.做 CRC 校验；2.时隙检查；3.信号强度检测； */
        if(BWPS_OK == bwps_ll_crc(&data))
        {
            up_data.mac = data.mac;
            up_data.sequence = data.sequence;
            up_data.time_slot = data.time_slot;
            up_data.len = data.len;
            rt_memcpy(up_data.buf, data.buf, up_data.len);
            LOG_D("data.mac%08X CRC OK:%08X", data.mac, data.crc);
        }
        else
        {
            LOG_W("data.mac%08X CRC ERROR:%08X", data.mac, data.crc);
            continue;
        }

        if(data.mac == 0x00000000)
        {
            static uint16_t last_sequence;
            static uint32_t last_time;
            static uint32_t now_time;
            if(last_sequence!=data.sequence)
            {
                if((data.sequence-last_sequence)>1)
                {
                    LOG_W("mac:%08X current sequence:%d lost:%d",data.mac,data.sequence, data.sequence - last_sequence - 1);    
                }
                now_time = chip_os_time_get_ms();
                LOG_D("mac:%08X sequence:%d time:%ld",data.mac,data.sequence,(int64_t)((int64_t)now_time - (int64_t)last_time));
                last_sequence = data.sequence;
                last_time = now_time;
                chip_os_sem_give(&link_layer_sem);
            }
        }
        else
        {
            bwps_data_control_logic_layer_put(&up_data);
        }

        if(data.mac == 0x11111111)
        {
            static uint16_t last_sequence;
            static uint16_t start_sequence = 0;
            static uint16_t lost_sequence = 0;
            if(start_sequence == 0)
            {
                start_sequence = data.sequence;
                last_sequence = data.sequence;
            }
            else
            {
                if(last_sequence!=data.sequence)
                {
                    if((data.sequence-last_sequence)>2)
                    {
                        lost_sequence = lost_sequence + data.sequence - last_sequence - 1;
                        LOG_W("mac:%08X current sequence:%d lost:%d/%d=%d\%\r\n",data.mac,data.sequence, lost_sequence, data.sequence-start_sequence, lost_sequence*100/(data.sequence-start_sequence));
                    }
                    else
                    {
                        LOG_D("mac:%08X sequence:%d len:%d\r\n",data.mac,data.sequence,data.len);
                    }
                    last_sequence = data.sequence;
                }
            }
        }
    }
}

void * bwps_beacon_thread(void * args)
{
    static struct bwps_raw_data beacon_raw_data;
    bwps_low_level_send_func bwps_ll_send;
    static uint32_t last_time = 0;
    static uint32_t now_time = 0;

    bwps_ll_send = (bwps_low_level_send_func)args;
    beacon_raw_data.head = 0xA5A5A5A5;
    beacon_raw_data.mac = 0x00000000;
    beacon_raw_data.time_slot = 0;
    beacon_raw_data.buf[0] = 0xAA;
    beacon_raw_data.crc = 0;
    beacon_raw_data.len = sizeof(beacon_raw_data.buf);
    memset(beacon_raw_data.buf,0x00,sizeof(beacon_raw_data.buf));
    chip_os_task_sleep_ms(1000);
    while (1)
    {
        if(CHIP_OS_OK == chip_os_sem_take(&link_layer_sem,chip_os_time_ms_to_ticks(1700)))
        {
            now_time = chip_os_time_get_ms();

            if((now_time - last_time) < 500)
            {
                chip_os_task_sleep_ms(500-(now_time - last_time));
            }

            beacon_raw_data.sequence++;
            if(beacon_raw_data.sequence >= 10)
            {
                beacon_raw_data.sequence = 0;
            }

            switch (beacon_raw_data.sequence%3)
            {
            case 0:
                bwps_get_sequence_beacon_data((struct bwps_beacon_sequence_data*)beacon_raw_data.buf);
                break;
            case 1:
                bwps_get_mac_beacon_data_1((struct bwps_beacon_mac_data*)beacon_raw_data.buf);
                break;
            case 2:
                bwps_get_mac_beacon_data_2((struct bwps_beacon_mac_data*)beacon_raw_data.buf);
                break;
            default:
                bwps_get_sequence_beacon_data((struct bwps_beacon_sequence_data*)beacon_raw_data.buf);
                break;
            }

            beacon_raw_data.crc = 0;
            for (int i = 0; i < beacon_raw_data.len; i++)
            {
                beacon_raw_data.crc = beacon_raw_data.crc + beacon_raw_data.buf[i];
            }
            beacon_raw_data.crc = ~beacon_raw_data.crc;
            LOG_D("CRC:0x%08X",beacon_raw_data.crc);
            bwps_ll_send(&beacon_raw_data, 1);
        }
        else
        {
            bwps_ll_send(&beacon_raw_data, 1);
        }
        last_time = now_time;
    }
}

int bwps_link_layer_init(bwps_low_level_send_func func)
{
    bwps_error_t osal_err;

    osal_err = chip_os_queue_init(&link_layer_queue, sizeof(struct bwps_raw_data), 10);
    osal_err = chip_os_sem_init(&link_layer_sem, 1);

    osal_err = chip_os_task_init(&link_layer_task, "bwps_link_layer", bwps_link_layer_thread, &link_layer_queue, CHIP_OS_PRIORITY_APP, 2048);
    osal_err = chip_os_task_init(&link_layer_task, "bwps_beacon", bwps_beacon_thread, func, 9, 2048);

    return (osal_err == 0) ? 0 : -1;
}
