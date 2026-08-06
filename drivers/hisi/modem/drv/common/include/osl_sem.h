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

#ifndef __OSL_SEM_H
#define __OSL_SEM_H

#include "osl_common.h"

#ifdef __KERNEL__
#include <linux/semaphore.h>

typedef struct semaphore osl_sem_id;

static inline void osl_sem_init(u32 val, osl_sem_id* sem)
{
    sema_init(sem, val);
}

static inline void osl_sem_up(osl_sem_id* sem)
{
    up(sem);
}

static inline void osl_sem_down(osl_sem_id* sem)
{
    //down(sem);
    while(down_interruptible(sem)!=0);
}

static inline int osl_sem_downtimeout(osl_sem_id* sem, long jiffies)
{
    return down_timeout(sem, jiffies);
}

static inline s32 osl_sema_delete(osl_sem_id*sem)
{
    return 0;
}

static inline s32 sema_delete(struct semaphore *sem)
{
    return 0;
}


#elif defined(__OS_VXWORKS__)
#include <semLib.h>

#define OSL_SEM_Q_FIFO         SEM_Q_FIFO    /* first in first out queue */
#define OSL_SEM_Q_PRIORITY     SEM_Q_PRIORITY    /* priority sorted queue */
#define OSL_SEM_DELETE_SAFE     SEM_DELETE_SAFE    /* owner delete safe (mutex opt.) */

typedef SEM_ID osl_sem_id;

static __inline__ void osl_sem_init(u32 val, osl_sem_id* sem)
{
    if(val == 0)
    {
        *sem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    }
    else
    {
        *sem = semBCreate(SEM_Q_FIFO, SEM_FULL);
    }
}

static __inline__ void osl_sem_up(osl_sem_id* sem)
{
    semGive(*sem);
}

static __inline__ void osl_sem_down(osl_sem_id* sem)
{
    semTake(*sem, WAIT_FOREVER);
}

static __inline__ int osl_sem_downtimeout(osl_sem_id* sem, long jiffies)
{
    return semTake(*sem, jiffies);
}

static __inline__ s32 osl_sema_delete(osl_sem_id*sem)
{
    return semDelete(*sem);
}

static __inline__ void sema_init(struct semaphore *sem, int val)
{
    if(val == 0)
    {
        sem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    }
    else
    {
        sem = semBCreate(SEM_Q_FIFO, SEM_FULL);
    }
}

static __inline__ void up(struct semaphore *sem)
{
    semGive(sem);
}

static __inline__ void down(struct semaphore *sem)
{
    semTake(sem, WAIT_FOREVER);
}

static __inline__ int down_timeout(struct semaphore *sem, long jiffies)
{
    return semTake(sem, jiffies);
}

static __inline__ s32 osl_sem_delete(struct semaphore *sem)
{
    return semDelete(sem);
}
static __inline__ int osl_sem_mcreate (osl_sem_id * mutex, int options)
{
    osl_sem_id semId;
    semId = semMCreate(options);
    if (!semId)
    {
        return ERROR;
    }
    *mutex = semId;
    return OK;
}
static __inline__ int osl_sem_bcreate (osl_sem_id *mutex,int count, int options)
{
    osl_sem_id semId;
    semId = semBCreate(options,count);
    if (!semId)
    {
        return  ERROR;
    }
    *mutex = semId;
    return OK;

}
static __inline__ int osl_sem_ccreate (osl_sem_id *mutex,int count, int options)
{
    osl_sem_id semId;
    semId = semCCreate(options,count);
    if (!semId)
    {
        return  ERROR;
    }
    *mutex = semId;
    return OK;
}

#elif defined(__OS_RTOSCK__)||defined(__OS_RTOSCK_SMP__)
#include "sre_sem.h"
#include "sre_shell.h"
#include "sre_symbol_api.h"
#include "sre_callstack.h"
#include "osl_complier.h"

#ifndef SEM_FULL
#define SEM_FULL       OS_SEM_FULL
#endif
#ifndef SEM_EMPTY
#define SEM_EMPTY      OS_SEM_EMPTY
#endif
#ifndef WAIT_FOREVER
#define WAIT_FOREVER   OS_WAIT_FOREVER
#endif
#define OSL_SEM_INVERSION_SAFE  0x08

#define OSL_SEM_Q_FIFO         SEM_MODE_FIFO    /* first in first out queue */
#define OSL_SEM_Q_PRIORITY     SEM_MODE_PRIOR    /* priority sorted queue */
#define OSL_SEM_DELETE_SAFE     0x0    /* owner delete safe (mutex opt.) RTOSCK no this*/

typedef  SEM_HANDLE_T osl_sem_id;
#if (OS_HARDWARE_PLATFORM != OS_CORTEX_RX)
static inline unsigned int osl_sem_init(u32 val, osl_sem_id* mutex)
{
    osl_sem_id semId = 0;
    UINT32 ret = SRE_OK;
    if(val != SEM_FULL)
    {
        ret = SRE_SemBCreate(val, &semId, SEM_MODE_PRIOR);
        if(SRE_OK == ret)
            *mutex = semId;
        return ret;
    }
    else
    {
        ret = SRE_SemCCreate(SEM_FULL, &semId, SEM_MODE_PRIOR);
        if(SRE_OK != ret)
            return ret;
        ret = SRE_SemCMaxCountSet(semId,SEM_FULL);
        if(SRE_OK != ret)
        {
            (void)SRE_Printf("SRE_SemCMaxCountSet failed 0x%x\n",ret);
            ret = SRE_SemDelete(semId);
            return ret;
        }
    }
    *mutex = semId;
    return ret;
}
static inline int osl_sem_mcreate (osl_sem_id * mutex, int options)
{
    osl_sem_id semId = 0;
    UINT32 ret = OK;
    if(options&OSL_SEM_INVERSION_SAFE)
    {
        ret = SRE_SemBCreate(SEM_FULL, &semId, (OS_SEM_MODE_E)(options&~OSL_SEM_INVERSION_SAFE));
        if(ret)
        {
            (void)SRE_Printf("SRE_SemBCreate fail!,ret=0x%x\n",ret);
            return ERROR;
        }
    }
    else
    {
        ret = SRE_SemCCreate(SEM_FULL, &semId, (OS_SEM_MODE_E)options);
        if(ret)
        {
            (void)SRE_Printf("SRE_SemCCreate fail!,ret=0x%x\n",ret);
            return ERROR;
        }
        ret = SRE_SemCMaxCountSet(semId,SEM_FULL);
        if(ret)
        {
            (void)SRE_Printf("SRE_SemCMaxCountSet fail!,ret=0x%x\n",ret);
            return ERROR;
        }
    }
    *mutex = semId;
    return OK;
}
static inline int osl_sem_bcreate (osl_sem_id *mutex,int count, int options)
{
    osl_sem_id semId = 0;
    UINT32 ret = OK;

    ret = SRE_SemBCreate(count, &semId, (OS_SEM_MODE_E)options);
    if(ret)
    {
        (void)SRE_Printf("SRE_SemBCreate fail!,ret=0x%x\n",ret);
        return ERROR;
    }

    *mutex = semId;
    return OK;
}
 static inline int osl_sem_ccreate (osl_sem_id *mutex,int count, int options)
{
    osl_sem_id semId = 0;
    unsigned int ret = OK;
    ret = SRE_SemCCreate(count, &semId, (OS_SEM_MODE_E)options);
    if(ret != SRE_OK)
    {
        (void)SRE_Printf("SRE_SemCCreate fail!,ret=0x%x\n",ret);
        return  ERROR;
    }
    *mutex = semId;
    return OK;
}

#else
static inline void osl_callback_stack(void)
{
    unsigned int ret = 0,i=0,puwMaxNum=10, func_stack[10] = {0,};
    char* pc_fun_name = NULL;
    ret = SRE_GetCallStack(&puwMaxNum,func_stack);
    if(ret == SRE_OK)
    {
        for(i = 0; i < puwMaxNum;i++)
        {
            ret = SRE_SymFindNameByAddr(((char*)func_stack[i]), (&pc_fun_name));
            if(ret == SRE_OK)
            {
                 (void)SRE_Printf("layer[%d]%s\n",i,pc_fun_name);
            }
        }
     }
     else
     {
        (void)SRE_Printf("SRE_GetCallStack failed 0x%x\n",ret);
     }
}

static inline unsigned int osl_sem_init(u32 val, osl_sem_id* mutex)
{
    osl_sem_id semId = 0;
    UINT32 ret = SRE_OK;
    u32 cookis=0;
    asm volatile("mov %0,pc":"=r"(cookis)::"memory");
    if(val != SEM_FULL)
    {
        ret = SRE_SemBCreateDebug(val, &semId, SEM_MODE_PRIOR, cookis);
        if(SRE_OK == ret)
            *mutex = semId;
        return ret;
    }
    else
    {
        ret = SRE_SemCCreateDebug(SEM_FULL, &semId, SEM_MODE_PRIOR,cookis);
        if(SRE_OK != ret)
            return ret;
        ret = SRE_SemCMaxCountSet(semId,SEM_FULL);
        if(SRE_OK != ret)
        {
            (void)SRE_Printf("SRE_SemCMaxCountSet failed 0x%x\n",ret);
            ret = SRE_SemDelete(semId);
            return ret;
        }
    }
    *mutex = semId;
    return ret;
}
static inline int osl_sem_mcreate (osl_sem_id * mutex, int options)
{
    osl_sem_id semId = 0;
    UINT32 ret = OK;
    u32 cookis=0;
    asm volatile("mov %0,pc":"=r"(cookis)::"memory");
    if(options&OSL_SEM_INVERSION_SAFE)
    {
        ret = SRE_SemBCreateDebug(SEM_FULL, &semId, (OS_SEM_MODE_E)(options&~OSL_SEM_INVERSION_SAFE),cookis);
        if(ret)
        {
            (void)SRE_Printf("SRE_SemBCreate fail!,ret=0x%x\n",ret);
            return ERROR;
        }
    }
    else
    {
        ret = SRE_SemCCreateDebug(SEM_FULL, &semId, (OS_SEM_MODE_E)options,cookis);
        if(ret)
        {
            (void)SRE_Printf("SRE_SemCCreate fail!,ret=0x%x\n",ret);
            return ERROR;
        }
        ret = SRE_SemCMaxCountSet(semId,SEM_FULL);
        if(ret)
        {
            (void)SRE_Printf("SRE_SemCMaxCountSet fail!,ret=0x%x\n",ret);
            return ERROR;
        }
    }
    *mutex = semId;
    return OK;
}
static inline int osl_sem_bcreate (osl_sem_id *mutex,int count, int options)
{
    osl_sem_id semId = 0;
    UINT32 ret = OK;
    u32 cookis=0;
    asm volatile("mov %0,pc":"=r"(cookis)::"memory");
    ret = SRE_SemBCreateDebug(count, &semId, (OS_SEM_MODE_E)options,cookis);
    if(ret)
    {
        (void)SRE_Printf("SRE_SemBCreate fail!,ret=0x%x\n",ret);
        return ERROR;
    }

    *mutex = semId;
    return OK;
}
 static inline int osl_sem_ccreate (osl_sem_id *mutex,int count, int options)
{
    osl_sem_id semId = 0;
    unsigned int ret = OK;
    u32 cookis=0;
    asm volatile("mov %0,pc":"=r"(cookis)::"memory");
    ret = SRE_SemCCreateDebug(count, &semId, (OS_SEM_MODE_E)options,cookis);
    if(ret != SRE_OK)
    {
        (void)SRE_Printf("SRE_SemCCreate fail!,ret=0x%x\n",ret);
        return  ERROR;
    }
    *mutex = semId;
    return OK;
}
#endif
static inline unsigned int osl_sem_up(osl_sem_id* sem)
{
   return SRE_SemPost(*sem);
}

static inline unsigned int osl_sem_down(osl_sem_id* sem)
{
    return SRE_SemPend(*sem, WAIT_FOREVER);
}

static inline unsigned int osl_sem_downtimeout(osl_sem_id* sem, long jiffies)
{
    return SRE_SemPend(*sem, jiffies);
}

static inline unsigned int osl_sema_delete(osl_sem_id *sem)
{
    return SRE_SemDelete(*sem);
}

struct semaphore{
    SEM_HANDLE_T handle;
};

static inline unsigned int sema_init(struct semaphore *sem, int val)
{
   return osl_sem_init(val,(osl_sem_id*)(&(sem->handle)));
}

static inline unsigned int up(struct semaphore *sem)
{
    return osl_sem_up(&(sem->handle));
}

static inline unsigned int  down(struct semaphore *sem)
{
    return osl_sem_down(&(sem->handle));
}

static inline unsigned int down_timeout(struct semaphore *sem, long jiffies)
{
    return SRE_SemPend(sem->handle, (UINT32)jiffies);
}

static inline unsigned int  osl_sem_delete(struct semaphore *sem)
{
    return SRE_SemDelete(sem->handle);
}


#else

#endif /* __KERNEL__ */

#endif

