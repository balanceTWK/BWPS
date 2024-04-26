/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2022 tangweikang, <BalanceTWK@yeah.net>
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <chip/osal.h>

#include <assert.h>

typedef void (*rtt_task_func_t)(void *);

void chip_os_task_dispatch(void * arg)
{
    struct chip_os_task * task = (struct chip_os_task *) arg;

    assert(task);
    assert(task->func);
    task->func(task->arg);
}

chip_os_error_t chip_os_task_init(struct chip_os_task * task, const char * name, chip_os_task_func_t func, void * arg, uint8_t prio,
                                  uint16_t stack_size)
{
    rt_err_t err;

    err = RT_ERROR;

    if ((task == NULL) || (func == NULL))
    {
        return CHIP_OS_INVALID_PARAM;
    }

    task->arg  = arg;
    task->func = func;
    task->handle = rt_thread_create(name, (rtt_task_func_t)(task->func), task->arg, stack_size, prio, 10);

    if (task->handle != RT_NULL)
    {
        err = rt_thread_startup(task->handle);
    }
    return (err == RT_EOK) ? CHIP_OS_OK : CHIP_OS_ERROR;
}

void chip_os_task_yield(void)
{
    rt_thread_yield();
}

void chip_os_task_sleep(chip_os_time_t ticks)
{
    rt_thread_delay(ticks);
}

void chip_os_task_sleep_ms(chip_os_time_t ms)
{
    rt_thread_mdelay((rt_int32_t)ms);
}

/* 函数返回值是否可以换成 task->handle */
void * chip_os_get_current_task_id(void)
{
    return (void*)rt_thread_self();
}

chip_os_error_t chip_os_task_remove(struct chip_os_task * t)
{
    rt_err_t err;
    err = rt_thread_delete(t->handle);

    return (err == RT_EOK) ? CHIP_OS_OK : CHIP_OS_ERROR;
}
