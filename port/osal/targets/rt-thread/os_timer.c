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
#include "os_hw.h"

#ifdef RT_USING_TIMER_SOFT

chip_os_error_t chip_os_timer_init(struct chip_os_timer * timer, chip_os_timer_fn * tm_cb, void * tm_arg)
{
    memset(timer, 0, sizeof(*timer));
    timer->func   = tm_cb;
    timer->arg    = tm_arg;
    timer->handle = rt_timer_create("chip_timer", timer->func, timer->arg, 10, RT_TIMER_FLAG_ONE_SHOT);

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_timer_start(struct chip_os_timer * timer, chip_os_time_t ticks)
{
    if (ticks < 0)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    if (ticks == 0)
    {
        ticks = 1;
    }
    rt_timer_control(timer->handle, RT_TIMER_CTRL_SET_TIME, &ticks);
    rt_timer_start(timer->handle);
    return CHIP_OS_OK;
}

chip_os_time_t chip_os_timer_remaining_ticks(struct chip_os_timer * timer, chip_os_time_t now)
{
    chip_os_time_t rt;

    rt_timer_control(timer->handle, RT_TIMER_CTRL_GET_REMAIN_TIME, &now);
    rt = now;

    return rt;
}

#endif
