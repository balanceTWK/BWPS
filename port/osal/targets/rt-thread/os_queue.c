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

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <chip/osal.h>
#include "os_hw.h"

chip_os_error_t chip_os_queue_init(struct chip_os_queue * msgq, size_t msg_size, size_t max_msgs)
{
    if (!msgq)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    msgq->handle = rt_mq_create("chip_msgq", (rt_size_t)msg_size, (rt_size_t)max_msgs, RT_IPC_FLAG_PRIO);
    assert(msgq->handle);

    return (msgq->handle == NULL) ? CHIP_OS_ERROR : CHIP_OS_OK;
}

chip_os_error_t chip_os_queue_get(struct chip_os_queue * queue, void * data, chip_os_time_t tmo)
{
    rt_err_t ret;

    assert(queue->handle);

    ret = rt_mq_recv(queue->handle,data ,queue->handle->msg_size, tmo);

    return ret == RT_EOK ? CHIP_OS_OK : CHIP_OS_ERROR;
}

chip_os_error_t chip_os_queue_put(struct chip_os_queue * queue, void * data)
{
    if (!queue)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    assert(queue->handle);

    if (rt_mq_send(queue->handle, data, queue->handle->msg_size) != RT_EOK)
    {
        return CHIP_OS_ERROR;
    }

    return CHIP_OS_OK;
}
