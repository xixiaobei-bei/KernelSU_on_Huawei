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


#ifdef __KERNEL__
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
	struct task_struct* tsk;
	struct sched_param	sch_para;

	sch_para.sched_priority = (int)priority;

	tsk =  kthread_run(entry, para, name);
	if (IS_ERR(tsk))
	{
		printk("create kthread %s failed!\n", name);
		return ERROR;
	}
	if (OK != sched_setscheduler(tsk, SCHED_FIFO, &sch_para))
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
#elif defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)
#include "sre_task.h"
#include <sre_sys.h>
#include <sre_tick.h>
#include <osl_printf.h>
#include <mdrv_sys.h>

#define OSL_TASK_FUNC   MDRV_TASK_FUNC
typedef mdrv_task_id          OSL_TASK_ID;


static inline  s32 osl_task_init(
        char *        name,         /* name of new task (stored at pStackBase) */
        int           priority,     /* priority of new task */
        int           stackSize,    /* size (bytes) of stack needed plus name */
        OSL_TASK_FUNC       entryPt,      /* entry point of new task */
        void *para,         /* 1st of 10 req'd args to pass to entryPt */
        OSL_TASK_ID *     tskid
       )
{
	struct mdrv_task_attribute tsk_attr= {.entryfunc =  entryPt,\
		                                  .prio   =  (unsigned int)priority,\
		                                  .qNum   =  0u,\
		                                  .args[0] =  (unsigned int)para,\
		                                  .stackSize =  (unsigned int)stackSize};
    unsigned int ret;
    ret = mdrv_task_create(name, &tsk_attr, tskid);
	if(ret != OK)
    {
        (void)osl_printf("osl_task_init fail name:%s,ret=0x%x!\n", name, ret);
        return ERROR;
    }
	ret = mdrv_task_resume(*tskid);
    if(ret != OK)
    {
        (void)osl_printf("osl_task_init resume task name:%s,ret=0x%x!\n", name, ret);
        return ERROR;
    }
    return OK;
}
 
static inline int osl_task_delay(int ticks )
{
	return (int)mdrv_task_delay((unsigned int)ticks);
}

static inline void osl_task_lock(void)
{
    mdrv_task_lock();
    return;
}

static inline void osl_task_unlock(void)
 {
     mdrv_task_unlock();
     return;
 }
 static inline int osl_task_delete(OSL_TASK_ID * tskid )
 {
     int ret = OK;
     if(*tskid != 0xffffffff)
     {
         ret = (int)mdrv_task_delete(*tskid);
         *tskid = 0xffffffff;
     }
     if(ret != OK)
     {
         (void)osl_printf("osl_task_delete fail ret=0x%x!\n", ret);
         return ERROR;
     }
     return OK;
 }
static inline int osl_task_self(void)
{
     unsigned int id;
     int ret;
     ret = (int)mdrv_task_self((mdrv_task_id *)&id);
     if(OK == ret)
     {
         return (int)id;
     }
     (void)osl_printf("osl_task_self fail ret=0x%x!\n", ret);
     return ERROR;
 }
static inline int osl_task_name_get(int taskID,char **pName)
{
    unsigned int ret = 0;
    ret = mdrv_task_name_get((mdrv_task_id)taskID,pName);
    if(OK != ret)
    {
        (void)osl_printf("osl_task_name_get fail ret=0x%x!\n", ret);
        return ERROR;
    }
    return OK;
}

#define kthread_stop(id)    \
do{     \
	(void)mdrv_task_delete((mdrv_task_id)id); \
}while(0)

static inline int osl_task_check(unsigned int taskid)
{
    return ERROR;
}

//lint -esym(528,*)
/*lint -save -e528*/
static __inline__ u64 osl_tick_get(void)
{
    return mdrv_get_tick_count(); 
}
static __inline__ void osl_tick_set(u64 swTick)
{
    return;
}
/*only create a task,not active it*/
static inline  s32 osl_task_create_only(
        char *        name,         /* name of new task (stored at pStackBase) */
        int           priority,     /* priority of new task */
        int           stackSize,    /* size (bytes) of stack needed plus name */
        OSL_TASK_FUNC       entryPt,      /* entry point of new task */
        void *para,         /* 1st of 10 req'd args to pass to entryPt */
        OSL_TASK_ID *     tskid
       )
{
	struct mdrv_task_attribute tsk_attr= {.entryfunc =  entryPt,\
		                                  .prio   =  (unsigned int)priority,\
		                                  .qNum   =  0u,\
		                                  .args[0] =  (unsigned int)para,\
		                                  .stackSize =  (unsigned int)stackSize};
    unsigned int ret;
    ret = mdrv_task_create(name, &tsk_attr, tskid);

    if(ret != OK)
    {
        (void)osl_printf("mdrv_task_create name:%s,ret:0x%x!", name, ret);
        return ERROR;
    }
    ret = mdrv_task_suspend(*tskid);
    if(ret != OK)
    {
        (void)osl_printf("mdrv_task_suspend name:%s,ret:0x%x!", name, ret);
        return ERROR;
    }
    return OK;
}
/*suspend a task*/
static __inline__ u32 osl_task_suspend(OSL_TASK_ID uwTaskPID)
{
	return mdrv_task_suspend(uwTaskPID);
}
/*resume/reactive a task*/
static __inline__ u32 osl_task_resume(OSL_TASK_ID uwTaskPID)
{
	return mdrv_task_resume(uwTaskPID);
}

static __inline__ u32 osl_task_core_bind(OSL_TASK_ID uwTaskPID, UINT32 uwCoreMask)
{
    return mdrv_task_core_bind(uwTaskPID,uwCoreMask);
}
static __inline__ u32 osl_task_core_unbind(OSL_TASK_ID uwTaskPID)
{
    return mdrv_task_core_unbind(uwTaskPID);
}

#elif defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__)
#include "sre_task.h"
#include <sre_sys.h>
#include <sre_tick.h>
#include <osl_printf.h>
#include <osl_types.h>
/*此处用于存放任务优先级 ---begin*/
#define  ICC_TASK_PRIVATE_PRI         (4)
#define  ICC_TASK_SHARED_PRI          (4)

#define OS_MAX_TASK_ID     62
/*此处用于存放任务优先级 ---end*/

#define VX_DEFAULT_PRIORITY      63
#define OSL_TASK_FUNC   TskEntryFunc
typedef TskHandle          OSL_TASK_ID;


 static inline  s32 osl_task_init(
        char *        name,         /* name of new task (stored at pStackBase) */
        int           priority,     /* priority of new task */
        int           stackSize,    /* size (bytes) of stack needed plus name */
        OSL_TASK_FUNC       entryPt,      /* entry point of new task */
        void *para,         /* 1st of 10 req'd args to pass to entryPt */
        OSL_TASK_ID *     tskid
       )
{
    struct TskInitParam tsk;
    UINT32 id;
    tsk.taskEntry  = (OSL_TASK_FUNC)entryPt;
    tsk.stackSize  = (UINT32)stackSize;
    tsk.taskPrio   = (TskPrior)priority;
    tsk.name       = name;
    tsk.queNum     = 0;
    tsk.args[0]    = (uintptr_t)para;
    tsk.stackAddr  = 0;
    UINT32 ret = SRE_TaskCreate((UINT32 *)&id,&tsk);
    if(ret != OK)
    {
        (void)osl_printf("osl_task_init fail name:%s,ret=0x%x!\n", name,ret);
        return ERROR;
    }
    *tskid = id;
    return OK;
}
static inline int osl_task_delay(int ticks )
 {
     return (int)SRE_TaskDelay((unsigned int)ticks);
 }
static inline void osl_task_lock(void)
 {
     SRE_TaskLock();
     return;
 }
static inline void osl_task_lock_disirq(void)
 {
     #ifdef __OS_RTOSCK_SMP__
     SRE_TaskLockNoIntLock();
     #else
     SRE_TaskLock();
     #endif
     return;
 }
 static inline void osl_task_unlock(void)
 {
     SRE_TaskUnlock();
     return;
 }
 static inline int osl_task_delete(OSL_TASK_ID * tskid )
 {
     int ret = OK;
     if(*tskid != 0xffffffff)
     {
         ret = (int)SRE_TaskDelete(*tskid);
         *tskid = 0xffffffff;
     }
     if(ret != OK)
     {
         (void)osl_printf("osl_task_delete fail ret=0x%x!\n", ret);
         return ERROR;
     }
     return OK;
 }
static inline int osl_task_self(void)
{
     unsigned int id;
     int ret;
     ret = (int)SRE_TaskSelf((unsigned int *)&id);
     if(OK == ret)
     {
         return (int)id;
     }
     (void)osl_printf("osl_task_self fail ret=0x%x!\n", ret);
     return ERROR;
 }
static inline int osl_task_name_get(int taskID,char **pName)
{
    unsigned int ret = 0;
    ret = SRE_TaskNameGet((unsigned int)taskID,pName);
    if(OK != ret)
    {
        (void)osl_printf("osl_task_name_get fail ret=0x%x!\n", ret);
        return ERROR;
    }
    return OK;
}

#define kthread_stop(id)    \
do{     \
	(void)SRE_TaskDelete(id); \
}while(0)

static inline int  osl_task_check(unsigned int taskid)
{
	if(TRUE == SRE_TaskIDCheck(taskid))
		return 0;
	else
		return -1;
}

//lint -esym(528,*)
/*lint -save -e528*/
static __inline__ u64 osl_tick_get(void){
	return SRE_TickCountGet();
}
static __inline__ void osl_tick_set(u64 swTick){
	u64* tick_addr=NULL;
	tick_addr = (u64*)SRE_GetTickAddr();
	*tick_addr = swTick;
}
/*only create a task,not active it*/
static inline  s32 osl_task_create_only(
        char *        name,         /* name of new task (stored at pStackBase) */
        int           priority,     /* priority of new task */
        int           stackSize,    /* size (bytes) of stack needed plus name */
        OSL_TASK_FUNC       entryPt,      /* entry point of new task */
        void *para,         /* 1st of 10 req'd args to pass to entryPt */
        OSL_TASK_ID *     tskid
       )
{
    struct TskInitParam tsk;
    UINT32 id;
    tsk.taskEntry  = (OSL_TASK_FUNC)entryPt;
    tsk.stackSize  = (UINT32)stackSize;
    tsk.taskPrio   = (TskPrior)priority;
    tsk.name       = name;
    tsk.queNum     = 0;
    tsk.args[0]    = (uintptr_t)para;
    tsk.stackAddr  = 0;
    UINT32 ret = SRE_TaskCreateOnly((UINT32 *)&id,&tsk);
    if(ret != OK)
    {
        (void)osl_printf("SRE_TaskCreateOnlyl name:%s,ret:0x%x!", name,ret);
        return ERROR;
    }
    *tskid = id;
    return OK;
}
/*suspend a task*/
static __inline__ u32 osl_task_suspend(OSL_TASK_ID uwTaskPID)
{
	return SRE_TaskSuspend(uwTaskPID);
}
/*resume/reactive a task*/
static __inline__ u32 osl_task_resume(OSL_TASK_ID uwTaskPID)
{
	return SRE_TaskResume(uwTaskPID);
}
#if defined(__OS_RTOSCK_SMP__) ||defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)
static __inline__ u32 osl_task_core_bind(OSL_TASK_ID uwTaskPID, UINT32 uwCoreMask)
{
#if defined(__OS_RTOSCK_TVP__)
	return 0;
#else
	return SRE_TaskCoreBind(uwTaskPID,uwCoreMask);
#endif
}
static __inline__ u32 osl_task_core_unbind(OSL_TASK_ID uwTaskPID)
{
#if defined(__OS_RTOSCK_TVP__)
	return 0;
#else
	return SRE_TaskCoreUnBind(uwTaskPID);
#endif
}

#else
static __inline__ u32 osl_task_core_bind(OSL_TASK_ID uwTaskPID, UINT32 uwCoreMask)
{
	return 0;
}
static __inline__ u32 osl_task_core_unbind(OSL_TASK_ID uwTaskPID)
{
	return 0;
}

#endif
#endif /* __KERNEL__ */

#endif /* __OSL_THREAD_H */

