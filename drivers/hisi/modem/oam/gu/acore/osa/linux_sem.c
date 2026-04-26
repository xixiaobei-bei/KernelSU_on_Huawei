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

/*
 * 版权所有（c）华为技术有限公司 2012-2019
 * 功能描述: OSA linux操作系统上信号量功能实现
 * 生成日期: 2006年10月3日
 */

#include "vos_config.h"
#include "v_sem.h"
#include "v_io.h"
#include "v_task.h"
#include "v_int.h"
#include "v_private.h"
#include "mdrv.h"
#include "pam_tag.h"

#if (VOS_OS_VER != VOS_WIN32)
#include <securec.h>
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#if (VOS_LINUX == VOS_OS_VER) || defined(LLT_OS_LINUX)

#define THIS_FILE_ID PS_FILE_ID_V_SEM_C
#define THIS_MODU mod_pam_osa

#define VOS_SEM_DELAY_MAX_LEN 16777216 /* unit is ms 2^24 */

typedef struct SEM_CONTROL_STRU {
    int flag; /* control block's state */
    VOS_UINT8 rsv[4];
    struct SEM_CONTROL_STRU *semId; /* the ID return to User */
    int semType;                    /* type of SEM */
    int semFlags;
    int semInitCount;
    char name[VOS_MAX_SEM_NAME_LENGTH];
    struct semaphore sem;
} SEM_ControlBlock;

/* the number of queue's control block */
VOS_UINT32 g_vosSemCtrlBlkNumber;

/* the start address of queue's control block */
SEM_ControlBlock *g_vosSemCtrlBlk = VOS_NULL_PTR;

/* the Max usage of queue */
VOS_UINT32 g_vosSemMaxSemId;

#define VOS_SEM_CTRL_BUF_SIZE (sizeof(SEM_ControlBlock) * VOS_MAX_SEM_NUMBER)

VOS_CHAR g_vosSemCtrlBuf[VOS_SEM_CTRL_BUF_SIZE];

/* 自旋锁，用来作信号量的临界资源保护 */
VOS_SPINLOCK g_vosSemSpinLock;

/*
 * Description: Init semaphore's control block
 */
VOS_VOID VOS_SemCtrlBlkInit(VOS_VOID)
{
    int i;

    g_vosSemCtrlBlkNumber = VOS_MAX_SEM_NUMBER;

    g_vosSemCtrlBlk = (SEM_ControlBlock *)g_vosSemCtrlBuf;

    for (i = 0; i < (int)g_vosSemCtrlBlkNumber; i++) {
        g_vosSemCtrlBlk[i].flag = VOS_SEM_CTRL_BLK_IDLE;
        g_vosSemCtrlBlk[i].semId = g_vosSemCtrlBlk + i;
    }

    g_vosSemMaxSemId = 0;

    VOS_SpinLockInit(&g_vosSemSpinLock);

    return;
}

/*
 * Description: allocate a control block
 */
SEM_ControlBlock *VOS_SemCtrlBlkGet(VOS_VOID)
{
    VOS_UINT32 i;
    VOS_ULONG lockLevel;

    VOS_SpinLockIntLock(&g_vosSemSpinLock, lockLevel);

    for (i = 0; i < g_vosSemCtrlBlkNumber; i++) {
        if (g_vosSemCtrlBlk[i].flag == VOS_SEM_CTRL_BLK_IDLE) {
            g_vosSemCtrlBlk[i].flag = VOS_SEM_CTRL_BLK_BUSY;
            break;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosSemSpinLock, lockLevel);

    if (i < g_vosSemCtrlBlkNumber) {
        /* record the max usage of SEM */
        if (i > g_vosSemMaxSemId) {
            g_vosSemMaxSemId = i;
        }

        return g_vosSemCtrlBlk + i;
    } else {
        mdrv_err("<VOS_SemCtrlBlkGet> no Idle.\n");

        VOS_SetErrorNo(VOS_ERRNO_SEMA4_FULL);

        return (VOS_MAX_SEM_ID_NULL);
    }
}

/*
 * Description: fress a block
 */
VOS_UINT32 VOS_SemCtrlBlkFree(SEM_ControlBlock *semAddress)
{
    VOS_ULONG lockLevel;

    if (semAddress == semAddress->semId) {
        if (semAddress->flag == VOS_SEM_CTRL_BLK_IDLE) {
            return (VOS_ERR);
        } else {
            VOS_SpinLockIntLock(&g_vosSemSpinLock, lockLevel);

            semAddress->flag = VOS_SEM_CTRL_BLK_IDLE;

            VOS_SpinUnlockIntUnlock(&g_vosSemSpinLock, lockLevel);
        }

        return (VOS_OK);
    } else {
        mdrv_err("<VOS_FreeSemCtrlBlk> free NULL Sem.\n");

        return (VOS_ERR);
    }
}

/*
 * Description: To create a counting semaphore;
 */
VOS_UINT32 VOS_SmCCreate(const VOS_CHAR *smName, VOS_UINT32 smInit, VOS_UINT32 flags, VOS_SEM *smID)
{
    SEM_ControlBlock *iSemId = VOS_NULL_PTR;
    VOS_UINT32 len;
    VOS_UINT32 i;

    if (smID == VOS_NULL_PTR) {
        VOS_SetErrorNo(VOS_ERRNO_SEMA4_CCREATE_INVALID_SMID);
        return (VOS_ERRNO_SEMA4_CCREATE_INVALID_SMID);
    }

    iSemId = VOS_SemCtrlBlkGet();
    if (iSemId == VOS_MAX_SEM_ID_NULL) {
        VOS_SetErrorNo(VOS_ERRNO_SEMA4_CCREATE_OBJTFULL);
        return (VOS_ERRNO_SEMA4_CCREATE_OBJTFULL);
    }

    *smID = (VOS_SEM)iSemId;

    if (smInit == 0xFFFFFFFF) {
        iSemId->semType = VOS_SEM_TYPE_MUTEX;
        sema_init(&(iSemId->sem), 1); /* 互斥信号量该值为1 */
    } else {
        iSemId->semType = VOS_SEM_TYPE_COUNT;
        sema_init(&(iSemId->sem), smInit);
    }

    len = VOS_StrNLen(smName, VOS_SEM_NAME_PARA_LENGTH);
    if (len > VOS_NULL) {
        for (i = 0; i < len; i++) {
            iSemId->name[i] = smName[i];
        }
        iSemId->name[len] = '\0';
    } else {
        iSemId->name[0] = '\0';
    }

    iSemId->semFlags = flags;
    iSemId->semInitCount = (int)smInit;

    return (VOS_OK);
}

/*
 * Description: To create a Mutex semaphore, can be null
 */
VOS_UINT32 VOS_SmMCreate(const VOS_CHAR *smName, VOS_UINT32 flags, VOS_SEM *smId)
{
    return (VOS_SmCCreate(smName, 0xFFFFFFFF, flags, smId));
}

/*
 * Description: To create a counting semaphore;
 */
VOS_UINT32 VOS_SmCreate(const VOS_CHAR *smName, VOS_UINT32 smInit, VOS_UINT32 flags, VOS_SEM *smId)
{
    return (VOS_SmCCreate(smName, smInit, flags, smId));
}

/*
 * Description: To create a Binary semaphore;
 */
VOS_UINT32 VOS_SmBCreate(const VOS_CHAR *smName, VOS_UINT32 smInit, VOS_UINT32 flags, VOS_SEM *smId)
{
    return (VOS_SmCCreate(smName, smInit, flags, smId));
}

/*
 * Description: to delete the exiting semaphore according to the ulSmID
 */
VOS_UINT32 VOS_SmDelete(VOS_SEM smId)
{
    SEM_ControlBlock *tempPtr = VOS_NULL_PTR;

    if (smId == VOS_NULL) {
        VOS_SetErrorNo(VOS_ERRNO_SEMA4_CCREATE_INVALID_SMID);
        return (VOS_ERRNO_SEMA4_CCREATE_INVALID_SMID);
    }

    tempPtr = (SEM_ControlBlock *)smId;

    if (tempPtr == tempPtr->semId) {
        if (tempPtr->flag == VOS_SEM_CTRL_BLK_IDLE) {
            VOS_SetErrorNo(VOS_ERRNO_SEMA4_CCREATE_OBJTFULL);
            return (VOS_ERRNO_SEMA4_CCREATE_INVALID_SMID);
        }

        return VOS_SemCtrlBlkFree(tempPtr);
    } else {
        VOS_SetErrorNo(VOS_ERRNO_SEMA4_CCREATE_INVALID_SMID);
        return (VOS_ERRNO_SEMA4_CCREATE_INVALID_SMID);
    }
}

/*
 * Description: Lock the resource for synchronization, if the resource is none
 * then block, otherwise the number of the resource --
 */
VOS_UINT32 VOS_SmP(VOS_SEM smId, VOS_UINT32 timeOutInMillSec)
{
    SEM_ControlBlock *tempPtr = VOS_NULL_PTR;
    VOS_INT32 timeintick;
    VOS_INT retValue;

    if (smId == VOS_NULL) {
        return (VOS_ERRNO_SEMA4_P_IDERR);
    }

    if (timeOutInMillSec > VOS_SEM_DELAY_MAX_LEN) {
        return VOS_ERRNO_SEMA4_P_TYPEERR;
    }

    tempPtr = (SEM_ControlBlock *)smId;

    if (tempPtr != tempPtr->semId) {
        return (VOS_ERRNO_SEMA4_P_IDERR);
    }

    if (tempPtr->flag == VOS_SEM_CTRL_BLK_IDLE) {
        return (VOS_ERRNO_SEMA4_P_NOTACTIVE);
    }

    if (timeOutInMillSec == 0) {
        down(&(tempPtr->sem));

        return VOS_OK;
    }

    timeintick = ((timeOutInMillSec * HZ) / 1000);

    retValue = down_timeout(&(tempPtr->sem), timeintick);
    if (retValue == VOS_OK) {
        return VOS_OK;
    } else if (retValue == -ETIME) {
        return VOS_ERRNO_SEMA4_P_TIMEOUT;
    } else {
    }

    return VOS_ERRNO_SEMA4_P_CANOTP;
}

/*
 * Description: Release/Unlock the sema4 that has locked a resource
 */
VOS_UINT32 VOS_SmV(VOS_SEM smId)
{
    SEM_ControlBlock *tempPtr = VOS_NULL_PTR;

    if (smId == VOS_NULL) {
        return (VOS_ERRNO_SEMA4_V_IDERR);
    }

    tempPtr = (SEM_ControlBlock *)smId;

    if (tempPtr != tempPtr->semId) {
        return (VOS_ERRNO_SEMA4_V_IDERR);
    }

    if (tempPtr->flag == VOS_SEM_CTRL_BLK_IDLE) {
        return (VOS_ERRNO_SEMA4_V_NOTACTIVE);
    }

    up(&(tempPtr->sem));

    return VOS_OK;
}

/*
 * Description: to lock the resource asynchronous
 */
VOS_UINT32 VOS_Sm_AsyP(VOS_SEM smId)
{
    return (VOS_SmP(smId, 1));
}

/*
 * Description: print the usage info of Sem
 */
VOS_VOID VOS_show_sem_info(VOS_VOID)
{
    mdrv_debug("<VOS_show_sem_info> Max be used sem is %x.\n", g_vosSemMaxSemId);
}

#endif

