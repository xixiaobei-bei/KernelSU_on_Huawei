/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __OSL_THREAD_H
#define __OSL_THREAD_H

#include "osl_common.h"

#ifndef ERROR
#define ERROR (-1)
#endif

#ifndef OK
#define OK (0)
#endif

#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/types.h>
#include <linux/sched/debug.h>
#include <linux/sched/signal.h>
#endif
#include <linux/delay.h>

/*此处用于存放任务优先级 ---begin*/
#define  ICC_TASK_PRIVATE_PRI         (91)
#define  ICC_TASK_SHARED_PRI          (90)

/*此处用于存放任务优先级 ---end*/

typedef struct task_struct* OSL_TASK_ID;
typedef int (*OSL_TASK_FUNC)(void* para);
static inline s32  osl_task_init(
                char* name,
                u32 priority,
                u32 size,
                OSL_TASK_FUNC entry,
                void *para,
                OSL_TASK_ID* task_id
                )
{
    struct task_struct* tsk = NULL;
    struct sched_param sch_para;

    sch_para.sched_priority = (int)priority;

    tsk =  kthread_run(entry, para, name);
    if (IS_ERR(tsk))
    {
        printk("create kthread %s failed!\n", name);
        return ERROR;
    }
    if (sched_setscheduler(tsk, SCHED_FIFO, &sch_para) != OK)
    {
        printk("create kthread %s sched_setscheduler failed!", name);
        return ERROR;
    }
    *task_id =tsk;

    return OK;
}
static __inline__ void osl_task_delay(int ticks )
{
    msleep(ticks);
}

#endif /* __OSL_THREAD_H */

