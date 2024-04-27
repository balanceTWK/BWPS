#include <rtthread.h>
#include <stdlib.h>
#include <string.h>

#include "bwps.h"
#include "bwps_port.h"
#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

static rt_device_t uart1;
static rt_mq_t turmass_mq_A;
static rt_device_t uart2;
static rt_mq_t turmass_mq_B;
static struct rt_memheap *bwps_memheap;

static rt_err_t uart1_input(rt_device_t dev, rt_size_t size)
{
    int receive_len;
    static char buff[256];

    if(size<=sizeof(buff))
    {
        receive_len = rt_device_read(dev, -1, buff, sizeof(buff));

        for (uint32_t i = 0; i < receive_len; i++)
        {
            rt_mq_send_wait(turmass_mq_A, &buff[i], 1, 0);
        }
    }

    return RT_EOK;
}

static rt_err_t uart2_input(rt_device_t dev, rt_size_t size)
{
    int receive_len;
    static char buff[256];

    if(size<=sizeof(buff))
    {
        receive_len = rt_device_read(dev, -1, buff, sizeof(buff));

        for (uint32_t i = 0; i < receive_len; i++)
        {
            rt_mq_send_wait(turmass_mq_B, &buff[i], 1, 0);
        }
    }

    return RT_EOK;
}

int turmass_module_send_with_response(int module ,const char * send_str,const char * wait_str, int wait_ms)
{
    char buf[256];
    memset(buf, 0, sizeof(buf));
    if(module == 1)
    {
        rt_device_write(uart1, 0, send_str, strlen(send_str));

        rt_thread_mdelay(wait_ms);
        for (int x = 0; x < sizeof(buf); x++)
        {
            if(RT_EOK != rt_mq_recv(turmass_mq_A, &buf[x], 1, rt_tick_from_millisecond(100)))
            {
                break;
            }
        }
        if (strstr((char *)(buf), (char *)wait_str) != NULL)
        {
            return 0;
        }
        else
        {
            buf[sizeof(buf) - 1] = 0;
            LOG_W("%128s",buf);
            return 1;
        }
    }
    else if(module == 2)
    {
        rt_device_write(uart2, 0, send_str, strlen(send_str));

        rt_thread_mdelay(wait_ms);
        for (int x = 0; x < sizeof(buf); x++)
        {
            if(RT_EOK != rt_mq_recv(turmass_mq_B, &buf[x], 1, rt_tick_from_millisecond(100)))
            {
                break;
            }
        }
        if (strstr((char *)(buf), (char *)wait_str) != NULL)
        {
            return 0;
        }
        else
        {
            buf[sizeof(buf) - 1] = 0;
            LOG_W("%128s",buf);
            return 1;
        }
    }

    return 0;
}

static void turmass_A_low_level_processing_thread(void* parameter)
{
    rt_err_t ret;
    int i;
    int receive_size;
    int head_ok = 0;
    static char buf[2048];
    static struct bwps_raw_data data;

    turmass_mq_A = rt_mq_create("(A)turmass_from_terminal", 1, 3 * 1024, RT_IPC_FLAG_FIFO);

    uart1 = rt_device_find("uart1");
    rt_device_open(uart1, RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(uart1, uart1_input);

    turmass_module_send_with_response(1, "AT+WORKMODE=82\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+RSTPARA\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+FREQ=451125000\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+WORKMODE=31\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+CH=0,1,451125000\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+FRAMECFG=7,600,7,600,8,600,7,600,7,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+TXP=13\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+RATE=18\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+RST\r\n","AT_OK",500);
    turmass_module_send_with_response(1, "AT+WORKMODE=81\r\n","AT_OK",500);
    while (1)
    {
        i = 0;
        ret = rt_mq_recv(turmass_mq_A, &buf[i], 1, RT_WAITING_FOREVER);
        for ( i = 1; i < sizeof(buf); i++)
        {
            ret = rt_mq_recv(turmass_mq_A, &buf[i], 1, rt_tick_from_millisecond(30));
            if(ret != RT_EOK)
            {
                break;
            }
        }
        // LOG_I("turmass A size:%d",i);
        receive_size = i;
        head_ok = 0;
        for (i = 0; i < receive_size; i++)
        {
            if(buf[i] == 0xA5)
            {
                head_ok++;
                if(head_ok == 4)
                {
                    rt_memcpy(&data, (&buf[i-3]), sizeof(data));
                    bwps_link_layer_put(&data);
                    i = i + (sizeof(data) - 4);
                }
            }
            else
            {
                head_ok = 0;
            }
        }
    }
    
}

static void turmass_B_low_level_processing_thread(void* parameter)
{
    rt_err_t ret;
    int i;
    int receive_size;
    int head_ok = 0;
    static char buf[2048];
    static struct bwps_raw_data data;

    turmass_mq_B = rt_mq_create("(B)turmass_from_terminal", 1, 3 * 1024, RT_IPC_FLAG_FIFO);

    uart2 = rt_device_find("uart2");
    rt_device_open(uart2, RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(uart2, uart2_input);

    turmass_module_send_with_response(2, "AT+WORKMODE=82\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+RSTPARA\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+FREQ=451125000\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+WORKMODE=32\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+CH=0,1,451125000\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+FRAMECFG=8,600,8,600,7,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600,8,600\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+TXP=13\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+RATE=18\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+NETSCAN=1\r\n","+EVT_SEARCH:1",5000);
    turmass_module_send_with_response(2, "AT+RST\r\n","AT_OK",500);
    turmass_module_send_with_response(2, "AT+WORKMODE=81\r\n","AT_OK",500);
    while (1)
    {
        i = 0;
        ret = rt_mq_recv(turmass_mq_B, &buf[i], 1, RT_WAITING_FOREVER);
        for ( i = 1; i < sizeof(buf); i++)
        {
            ret = rt_mq_recv(turmass_mq_B, &buf[i], 1, rt_tick_from_millisecond(30));
            if(ret != RT_EOK)
            {
                break;
            }
        }
        // LOG_I("turmass B size:%d",i);
        receive_size = i;
        head_ok = 0;
        for (i = 0; i < receive_size; i++)
        {
            if(buf[i] == 0xA5)
            {
                head_ok++;
                if(head_ok == 4)
                {
                    rt_memcpy(&data, (&buf[i-3]), sizeof(data));
                    bwps_link_layer_put(&data);
                    i = i + (sizeof(data) - 4);
                }
            }
            else
            {
                head_ok = 0;
            }
        }
    }
}

bwps_error_t bwps_turmass_low_level_send(struct bwps_raw_data* data, int mode)
{
    rt_size_t ret1 = 0;
    rt_size_t ret2 = 0;
    if(mode == 2)
    {
        ret1 = rt_device_write(uart1, 0, data, sizeof(struct bwps_raw_data));
        ret2 = rt_device_write(uart2, 0, data, sizeof(struct bwps_raw_data));
        if(ret1 == sizeof(struct bwps_raw_data))
        {
            return BWPS_OK;
        }
        if(ret2 == sizeof(struct bwps_raw_data))
        {
            return BWPS_OK;
        }
    }
    else if (mode == 1)
    {
        ret2 = rt_device_write(uart2, 0, data, sizeof(struct bwps_raw_data));
        if(ret2 == sizeof(struct bwps_raw_data))
        {
            return BWPS_OK;
        }
    }
    else if (mode == 0)
    {
        ret1 = rt_device_write(uart1, 0, data, sizeof(struct bwps_raw_data));
        if(ret1 == sizeof(struct bwps_raw_data))
        {
            return BWPS_OK;
        }
    }

    return BWPS_ERROR;
}

void* bwps_malloc(int nbytes)
{
    return rt_memheap_alloc(bwps_memheap, nbytes);
}

void bwps_free(void *ptr)
{
    rt_memheap_free(ptr);
}

int bwps_user_init(void)
{
    rt_thread_t tid1 = RT_NULL;
    rt_thread_t tid2 = RT_NULL;
    struct rt_object *obj;

    obj = rt_object_find("heap_cma", RT_Object_Class_MemHeap);

    bwps_memheap = (struct rt_memheap *)obj;

    tid1 = rt_thread_create("turmass_A",
                            turmass_A_low_level_processing_thread, 0,
                            1024,
                            16, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }

    tid2 = rt_thread_create("turmass_B",
                            turmass_B_low_level_processing_thread, 0,
                            1024,
                            17, 10);
    if (tid2 != RT_NULL)
    {
        rt_thread_startup(tid2);
    }

    bwps_link_layer_init(bwps_turmass_low_level_send);
    bwps_data_cache_init();
    bwps_data_control_logic_layer_init();

    return 0;
}

// if (!strcmp(argv[i], "-p"))
// int bsize = atol(argv[2]);
void turmass(uint8_t argc, char **argv)
{
    int mode;
    if (argc == 3) 
    {
        mode = atol(argv[1]);
        if(mode == 0)
        {
            
        }
        // // if (!strcmp(argv[1], "-p"))
        // rt_kprintf("%s\r\n", argv[0]);
        // rt_kprintf("%s\r\n", argv[1]);
        // rt_kprintf("%s\r\n", argv[2]);
    }
}
MSH_CMD_EXPORT(turmass, turmass test);
