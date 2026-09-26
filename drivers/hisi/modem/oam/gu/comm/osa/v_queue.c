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
 * 功能描述: OSA队列功能实现
 * 生成日期: 2006年10月3日
 */
#include "vos_config.h"
#include "v_typdef.h"
#include "v_queue.h"
#include "v_msg.h"
#include "v_io.h"
#include "v_task.h"
#include "v_sem.h"
#include "vos_id.h"
#include "v_int.h"
#include "v_blk_mem.h"
#include "mdrv.h"
#include "pam_tag.h"


#define THIS_FILE_ID PS_FILE_ID_V_QUEUE_C
#define THIS_MODU mod_pam_osa

/* the state of control block */
#define VOS_QUEUE_CTRL_BLK_IDLE 0
#define VOS_QUEUE_CTRL_BLK_BUSY 1

/* errno define */
#define VOS_QUEUE_CTRL_BLK_NULL 0xffffffff

#define VOS_MAX_PID_PRI VOS_PRIORITY_NUM

typedef struct {
    VOS_UINT_PTR *qStart;
    VOS_UINT_PTR *qEnd;
    VOS_UINT_PTR *qIn;
    VOS_UINT_PTR *qOut;
    VOS_UINT32    qSize;
    VOS_UINT32    qEntries;
    VOS_UINT32    qUrgentSize;
    VOS_UINT8     rsv[4];
} VOS_Q;

typedef struct {
    int        flag;        /* control block's state */
    VOS_UINT32 qid;         /* queue ID */
    VOS_UINT32 length;      /* the MAX stored message number */
    VOS_UINT32 queueOption; /* FIFO or priority */
    VOS_UINT32 maxMsgSize;  /* the MAX stored message size */
    VOS_UINT8  rsv[4];
    VOS_SEM    semId; /* pend this Sem to get queue */
    /* which should be del when only one FID exists */
    VOS_UINT32 qNum; /* number of Q */
    VOS_Q q[VOS_MAX_PID_PRI];
#if (VOS_DEBUG == VOS_DOPRA_VER)
    VOS_UINT32 queueFullTag; /* queue is full or not */
#endif
} VOS_QueueControlBlock;

typedef struct {
    VOS_UINT16 sendPid;
    VOS_UINT16 rcvPid;
    VOS_UINT32 msgName;
} VOS_DumpQueueContent;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
} VOS_MsgBlockBak;

/* the number of queue's control block */
VOS_UINT32 g_vosQueueCtrlBlcokNumber;

/* the Max usage of queue */
VOS_UINT32 g_maxUseQueueNumber;

/* the start address of queue's control block */
VOS_QueueControlBlock *g_vosQueueCtrlBlcok = VOS_NULL_PTR;

/* the definition of control info */
#define VOS_QUEUE_CTRL_BUF_SIZE (sizeof(VOS_QueueControlBlock) * VOS_MAX_QUEUE_NUMBER)

VOS_CHAR g_vosQueueCtrlBuf[VOS_QUEUE_CTRL_BUF_SIZE];

/* the definition of buf 1->RTC task */
#define VOS_QUEUE_BUF_SIZE \
    ((VOS_FID_BUTT + 1) * (sizeof(VOS_UINT_PTR) * VOS_FID_QUEUE_LENGTH)) /* VOS_FID_BUFF + 1 for vos_timer */

VOS_CHAR g_vosQueueBuf[VOS_QUEUE_BUF_SIZE] = {0};

/* the location of buf which should be allocated */
VOS_UINT32 g_vosQueueBufSuffix = 0;

/* 自旋锁，用来作queue的临界资源保护 */
VOS_SPINLOCK g_vosQueueSpinLock;

/*
 * Description: Init queue's control block
 */
VOS_VOID VOS_QueueCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32 i;

    g_vosQueueCtrlBlcok = (VOS_QueueControlBlock *)g_vosQueueCtrlBuf;

    g_maxUseQueueNumber = 0;

    g_vosQueueCtrlBlcokNumber = VOS_MAX_QUEUE_NUMBER;

    for (i = 0; i < g_vosQueueCtrlBlcokNumber; i++) {
        g_vosQueueCtrlBlcok[i].flag = VOS_QUEUE_CTRL_BLK_IDLE;
        g_vosQueueCtrlBlcok[i].qid  = i;
#if (VOS_DEBUG == VOS_DOPRA_VER)
        g_vosQueueCtrlBlcok[i].queueFullTag = VOS_FALSE;
#endif
    }

    VOS_SpinLockInit(&g_vosQueueSpinLock);

    return;
}

/*
 * Description: allocte a queue control block
 */
VOS_UINT32 VOS_QueueCtrlBlkGet(VOS_VOID)
{
    VOS_UINT32 i;
    VOS_ULONG  lockLevel;

    VOS_SpinLockIntLock(&g_vosQueueSpinLock, lockLevel);

    for (i = 0; i < g_vosQueueCtrlBlcokNumber; i++) {
        if (g_vosQueueCtrlBlcok[i].flag == VOS_QUEUE_CTRL_BLK_IDLE) {
            g_vosQueueCtrlBlcok[i].flag = VOS_QUEUE_CTRL_BLK_BUSY;

            break;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

    if (i < g_vosQueueCtrlBlcokNumber) {
        /* record the max usage of queue */
        if (i > g_maxUseQueueNumber) {
            g_maxUseQueueNumber = i;
        }

#if (VOS_DEBUG == VOS_DOPRA_VER)
        g_vosQueueCtrlBlcok[i].queueFullTag = VOS_FALSE;
#endif

        return i;
    } else {
        mdrv_err("<VOS_QueueCtrlBlkGet> no Idle.\n");

        VOS_SetErrorNo(VOS_ERRNO_QUEUE_FULL);

        return (VOS_QUEUE_CTRL_BLK_NULL);
    }
}

/*
 * Description: free a queue control block
 */
VOS_UINT32 VOS_QueueCtrlBlkFree(VOS_UINT32 qid)
{
    VOS_ULONG lockLevel;

    if (qid < g_vosQueueCtrlBlcokNumber) {
        if (g_vosQueueCtrlBlcok[qid].flag == VOS_QUEUE_CTRL_BLK_IDLE) {
            mdrv_debug("<OS_QueueCtrlBlkFree> free Idle Queue.\n");
        } else {
            VOS_SpinLockIntLock(&g_vosQueueSpinLock, lockLevel);

            g_vosQueueCtrlBlcok[qid].flag = VOS_QUEUE_CTRL_BLK_IDLE;

            VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);
        }

        return VOS_OK;
    } else {
        mdrv_err("<OS_QueueCtrlBlkFree> Qid exceed g_vosQueueCtrlBlcokNumber.\n");

        return VOS_ERR;
    }
}

/*
 * Description: Create queue
 */
VOS_UINT32 VOS_FixedQueueCreate(VOS_UINT32 length, VOS_UINT32 *queueID, VOS_UINT32 queueOption,
                                VOS_UINT32 maxMsgSize, VOS_UINT32 queueNum)
{
    int i;
    VOS_UINT32 iQid;
    VOS_UINT32 iQueueOption;
    VOS_UINT32 queueLength;
    VOS_UINT32 allocSize;

    if (length == 0) {
        queueLength = VOS_DEFAULT_QUEUE_SIZE;
        allocSize   = VOS_DEFAULT_QUEUE_SIZE * sizeof(VOS_UINT_PTR);
    } else {
        queueLength = length;
        allocSize   = length * sizeof(VOS_UINT_PTR);
    }

    if (queueOption == VOS_MSG_Q_FIFO) {
        iQueueOption = VOS_SEMA4_FIFO;
    } else if (queueOption == VOS_MSG_Q_PRIORITY) {
        iQueueOption = VOS_SEMA4_PRIOR;
    } else {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_QUEUE_CREATE_OPTINVALID);
        return (VOS_ERRNO_QUEUE_CREATE_OPTINVALID);
    }

    if (maxMsgSize == 0) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_QUEUE_CREATE_SIZEISZERO);
        return (VOS_ERRNO_QUEUE_CREATE_SIZEISZERO);
    }

    if (queueID == NULL) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_QUEUE_CREATE_OUTPUTISNULL);
        return (VOS_ERRNO_QUEUE_CREATE_OUTPUTISNULL);
    }

    /* create queue */
    iQid = VOS_QueueCtrlBlkGet();
    if (iQid == VOS_QUEUE_CTRL_BLK_NULL) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_QUEUE_CREATE_NOFREECB);
        return (VOS_ERRNO_QUEUE_CREATE_NOFREECB);
    }

    /* which should be del when only one FID exists */
    for (i = 0; i < (VOS_INT)queueNum; i++) {
        g_vosQueueCtrlBlcok[iQid].q[i].qStart = (VOS_UINT_PTR *)VOS_StaticMemAlloc(g_vosQueueBuf,
                                                                                  sizeof(g_vosQueueBuf), allocSize,
                                                                                  &g_vosQueueBufSuffix);

        if (VOS_NULL_PTR == g_vosQueueCtrlBlcok[iQid].q[i].qStart) {
            mdrv_err("<VOS_FixedQueueCreatealloc> static mem alloc fail.\n");

            (VOS_VOID)VOS_QueueCtrlBlkFree(iQid);
            return (VOS_ERR);
        }

        g_vosQueueCtrlBlcok[iQid].q[i].qEnd =
            (VOS_UINT_PTR *)((VOS_UINT_PTR)g_vosQueueCtrlBlcok[iQid].q[i].qStart + allocSize);
        g_vosQueueCtrlBlcok[iQid].q[i].qIn         = g_vosQueueCtrlBlcok[iQid].q[i].qStart;
        g_vosQueueCtrlBlcok[iQid].q[i].qOut        = g_vosQueueCtrlBlcok[iQid].q[i].qStart;
        g_vosQueueCtrlBlcok[iQid].q[i].qSize       = queueLength;
        g_vosQueueCtrlBlcok[iQid].q[i].qEntries    = 0;
        g_vosQueueCtrlBlcok[iQid].q[i].qUrgentSize = 0;
    }

    if (VOS_OK != VOS_SmCCreate("tmp", 0, iQueueOption, &g_vosQueueCtrlBlcok[iQid].semId)) {
        /* which should be del when only one FID exists */
        (VOS_VOID)VOS_QueueCtrlBlkFree(iQid);
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_QUEUE_CREATE_OSALFAIL);
        return (VOS_ERRNO_QUEUE_CREATE_OSALFAIL);
    } else {
        *queueID = iQid;

        g_vosQueueCtrlBlcok[iQid].length      = length;
        g_vosQueueCtrlBlcok[iQid].queueOption = queueOption;
        g_vosQueueCtrlBlcok[iQid].maxMsgSize  = maxMsgSize;
        /* which should be del when only one FID exists */
        g_vosQueueCtrlBlcok[iQid].qNum = queueNum;

        return (VOS_OK);
    }
}

/*
 * Description: Write a message to a specific queue's list
 */
VOS_UINT32 VOS_AddQueue(VOS_UINT32 queueID, int suffix, VOS_UINT_PTR addressValue, VOS_UINT32 pri)
{
    VOS_ULONG     lockLevel;
    VOS_UINT32    urgentSize;
    VOS_UINT32    i;
    VOS_UINT_PTR *tmpAddr = VOS_NULL_PTR;
    VOS_UINT_PTR  tmpValue;

    VOS_SpinLockIntLock(&g_vosQueueSpinLock, lockLevel);

    if (g_vosQueueCtrlBlcok[queueID].q[suffix].qEntries >= g_vosQueueCtrlBlcok[queueID].q[suffix].qSize) {
        VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

        return VOS_ERR;
    }

    if (VOS_EMERGENT_PRIORITY_MSG == pri) {
        urgentSize = g_vosQueueCtrlBlcok[queueID].q[suffix].qUrgentSize;

        if (0 < urgentSize) {
            tmpAddr = g_vosQueueCtrlBlcok[queueID].q[suffix].qOut;

            tmpValue = *tmpAddr;

            for (i = 1; i < urgentSize; i++) {
                if (tmpAddr == g_vosQueueCtrlBlcok[queueID].q[suffix].qEnd - 1) {
                    *tmpAddr = *(VOS_UINT_PTR *)(g_vosQueueCtrlBlcok[queueID].q[suffix].qStart);
                    tmpAddr  = g_vosQueueCtrlBlcok[queueID].q[suffix].qStart;
                    continue;
                }

                *tmpAddr = *(tmpAddr + 1);
                tmpAddr++;
            }

            *tmpAddr = addressValue;
        } else {
            tmpValue = addressValue;
        }

        if (g_vosQueueCtrlBlcok[queueID].q[suffix].qOut == g_vosQueueCtrlBlcok[queueID].q[suffix].qStart) {
            g_vosQueueCtrlBlcok[queueID].q[suffix].qOut = g_vosQueueCtrlBlcok[queueID].q[suffix].qEnd - 1;
        } else {
            g_vosQueueCtrlBlcok[queueID].q[suffix].qOut--;
        }

        *g_vosQueueCtrlBlcok[queueID].q[suffix].qOut = tmpValue;
        g_vosQueueCtrlBlcok[queueID].q[suffix].qEntries++;
        g_vosQueueCtrlBlcok[queueID].q[suffix].qUrgentSize++;
    } else {
        *g_vosQueueCtrlBlcok[queueID].q[suffix].qIn++ = addressValue;
        g_vosQueueCtrlBlcok[queueID].q[suffix].qEntries++;
        if (g_vosQueueCtrlBlcok[queueID].q[suffix].qIn == g_vosQueueCtrlBlcok[queueID].q[suffix].qEnd) {
            g_vosQueueCtrlBlcok[queueID].q[suffix].qIn = g_vosQueueCtrlBlcok[queueID].q[suffix].qStart;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

    return VOS_OK;
}

/*
 * Description: Write a message to a specific queue synchronuslly
 */
/*lint -e456 -e454 */
VOS_UINT32 VOS_FixedQueueWrite(VOS_UINT32 queueID, VOS_VOID *bufferAddr, VOS_UINT32 bufferSize, VOS_UINT32 pri,
                               VOS_UINT32 pid)
{
    int                suffix;
    VOS_UINT32         returnValue;
    VOS_MsgBlockBak  msgBlcokBak;
    VOS_MsgBlockBak *temp = VOS_NULL_PTR;
#if (VOS_RTOSCK == VOS_OS_VER)
    VOS_UINT32 interruptFlag = VOS_FALSE;

    interruptFlag = VOS_CheckInterrupt();
#endif
#if (VOS_RTOSCK == VOS_OS_VER)
    if (VOS_FALSE == interruptFlag) {
        (VOS_VOID)VOS_TaskLock();
    }
#endif

    /* Copy Msg Parameters to local stub */
    temp = (VOS_MsgBlockBak *)((VOS_UINT_PTR)bufferAddr + VOS_MSG_BLK_HEAD_LEN);

    msgBlcokBak.ulSenderCpuId   = temp->ulSenderCpuId;
    msgBlcokBak.ulSenderPid     = temp->ulSenderPid;
    msgBlcokBak.ulReceiverCpuId = temp->ulReceiverCpuId;
    msgBlcokBak.ulReceiverPid   = temp->ulReceiverPid;
    msgBlcokBak.ulLength        = sizeof(VOS_UINT32);
    msgBlcokBak.msgName       = temp->msgName;

    suffix   = g_vosPidRecords[pid - VOS_PID_DOPRAEND].priority;
    returnValue = VOS_AddQueue(queueID, suffix, (VOS_UINT_PTR)bufferAddr, pri);
    if (VOS_ERR == returnValue) {
#if (VOS_RTOSCK == VOS_OS_VER)
        if (VOS_FALSE == interruptFlag) {
            (VOS_VOID)VOS_TaskUnlock();
        }
#endif
        mdrv_err("<VOS_FixedQueueWrite> Queue ID %d is full.\n", (int)queueID);

        return VOS_ERR;
    } else {
        if (VOS_OK != VOS_SmV(g_vosQueueCtrlBlcok[queueID].semId)) {
#if (VOS_RTOSCK == VOS_OS_VER)
            if (VOS_FALSE == interruptFlag) {
                (VOS_VOID)VOS_TaskUnlock();
            }
#endif
            mdrv_err("<VOS_FixedQueueWrite> VOS_SmV Fail.\n");

            return VOS_ERR;
        } else {
            VOS_ExecuteAwakeFun((MsgBlock *)&msgBlcokBak);

#if (VOS_RTOSCK == VOS_OS_VER)
            if (VOS_FALSE == interruptFlag) {
                (VOS_VOID)VOS_TaskUnlock();
            }
#endif
        }
    }

    return VOS_OK;
}
/*lint +e456 +e454 */

/*
 * Description: Write a message to a specific queue synchronuslly
 */
VOS_UINT32 VOS_FixedQueueWriteDirect(VOS_UINT32 queueID, const VOS_VOID *bufferAddr, VOS_UINT32 pri)
{
    VOS_UINT32 returnValue;

    /* only one queue */
    returnValue = VOS_AddQueue(queueID, 0, (VOS_UINT_PTR)bufferAddr, pri);
    if (VOS_ERR == returnValue) {
        return VOS_ERR;
    }

    if (VOS_OK != VOS_SmV(g_vosQueueCtrlBlcok[queueID].semId)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

/*
 * Description: Reading message from a queue synchronouslly
 */
VOS_UINT32 VOS_FixedQueueRead(VOS_UINT32 queueID, VOS_UINT32 timeOutInMillSec, VOS_UINT_PTR *bufferAddr,
                              VOS_UINT32 bufferSize)
{
    int          i;
    VOS_ULONG    lockLevel;
    VOS_UINT_PTR tempValue;

    if (VOS_OK != VOS_SmP(g_vosQueueCtrlBlcok[queueID].semId, timeOutInMillSec)) {
        mdrv_err("<VOS_FixedQueueRead> msgQReceive error.\n");
        return VOS_ERR;
    } else {
        VOS_SpinLockIntLock(&g_vosQueueSpinLock, lockLevel);

        /* which should be del when only one FID exists */
        /* g_vosQueueCtrlBlcok[ulQueueID].QNum is VOS_MAX_PID_PRI */
        for (i = (VOS_INT)(g_vosQueueCtrlBlcok[queueID].qNum - 1); i >= 0; i--) {
            if (g_vosQueueCtrlBlcok[queueID].q[i].qEntries > 0) {
                tempValue = *g_vosQueueCtrlBlcok[queueID].q[i].qOut++;
                g_vosQueueCtrlBlcok[queueID].q[i].qEntries--;
                if (g_vosQueueCtrlBlcok[queueID].q[i].qOut == g_vosQueueCtrlBlcok[queueID].q[i].qEnd) {
                    g_vosQueueCtrlBlcok[queueID].q[i].qOut = g_vosQueueCtrlBlcok[queueID].q[i].qStart;
                }

                if (0 < g_vosQueueCtrlBlcok[queueID].q[i].qUrgentSize) {
                    g_vosQueueCtrlBlcok[queueID].q[i].qUrgentSize--;
                }

                VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

                *bufferAddr = tempValue;
                return VOS_OK;
            }
        }

        VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

        return VOS_ERR;
    }
}

/*
 * 功能描述: 通过FID获取该FID消息队列中首条消息地址
 */
VOS_VOID *VOS_FixedQueueReadMsg(VOS_UINT32 fidID)
{
    VOS_INT      i;
    VOS_ULONG    lockLevel;
    VOS_UINT_PTR tempValue;
    VOS_UINT32   queueID;

    VOS_SpinLockIntLock(&g_vosQueueSpinLock, lockLevel);

    queueID = VOS_GetQueueIDFromFid(fidID);

    for (i = (VOS_INT)(g_vosQueueCtrlBlcok[queueID].qNum - 1); i >= 0; i--) {
        if (g_vosQueueCtrlBlcok[queueID].q[i].qEntries > 0) {
            tempValue = *g_vosQueueCtrlBlcok[queueID].q[i].qOut;

            VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

            tempValue += VOS_MSG_BLK_HEAD_LEN;

            return (VOS_VOID *)tempValue;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

    return VOS_NULL_PTR;
}

/*
 * Description: get the Sem ID of a queue
 */
VOS_SEM VOS_GetSemIDFromQueue(VOS_UINT32 queue)
{
    if (queue >= g_vosQueueCtrlBlcokNumber) {
        mdrv_err("<VOS_GetSemIDFromQueue> ulQueue is invaild.\n");

        return 0xffffffff;
    }

    return g_vosQueueCtrlBlcok[queue].semId;
}

/*
 * Description: get a ready message from the message queue.
 */
VOS_VOID *VOS_OutMsg(VOS_UINT32 queueID)
{
    int          i;
    VOS_ULONG    lockLevel;
    VOS_UINT_PTR tempValue;

    VOS_SpinLockIntLock(&g_vosQueueSpinLock, lockLevel);

    /* which should be del when only one FID exists */
    /* g_vosQueueCtrlBlcok[ulQueueID].QNum is VOS_MAX_PID_PRI */
    for (i = (VOS_INT)(g_vosQueueCtrlBlcok[queueID].qNum - 1); i >= 0; i--) {
        if (g_vosQueueCtrlBlcok[queueID].q[i].qEntries > 0) {
            tempValue = *g_vosQueueCtrlBlcok[queueID].q[i].qOut++;
            g_vosQueueCtrlBlcok[queueID].q[i].qEntries--;
            if (g_vosQueueCtrlBlcok[queueID].q[i].qOut == g_vosQueueCtrlBlcok[queueID].q[i].qEnd) {
                g_vosQueueCtrlBlcok[queueID].q[i].qOut = g_vosQueueCtrlBlcok[queueID].q[i].qStart;
            }

            if (0 < g_vosQueueCtrlBlcok[queueID].q[i].qUrgentSize) {
                g_vosQueueCtrlBlcok[queueID].q[i].qUrgentSize--;
            }

            VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

            tempValue += VOS_MSG_BLK_HEAD_LEN;

            return (VOS_VOID *)tempValue;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

    return VOS_NULL_PTR;
}

/*
 * Description: get a ready message from the message queue of the task.
 */
MODULE_EXPORTED VOS_VOID *VOS_GetMsg(VOS_UINT32 taskID)
{
    VOS_UINT32 queueID;

    queueID = VOS_GetQueueIDFromTID(taskID);

    return VOS_OutMsg(queueID);
}

/*
 * Description: get the queue size of Fid.
 */
MODULE_EXPORTED VOS_UINT32 VOS_GetQueueSizeFromFid(VOS_UINT32 fid)
{
    VOS_UINT32 qid;

    qid = VOS_GetQueueIDFromFid(fid);

    return g_vosQueueCtrlBlcok[qid].q[0].qEntries;
}

/*
 * Description: print usage info of queue
 */
MODULE_EXPORTED VOS_VOID VOS_show_queue_info(VOS_VOID)
{
    mdrv_debug("<VOS_show_queue_info> Max be used queue is %d.\n", g_maxUseQueueNumber);
}

/*
 * Description: print queue info which is full
 */
VOS_VOID VOS_QueuePrintFull(VOS_UINT32 queue, VOS_CHAR *pcBuf, VOS_UINT32 len)
{
    VOS_UINT32 count = 0;
#if (VOS_DEBUG == VOS_DOPRA_VER)
    VOS_UINT32 *content = VOS_NULL_PTR;
#endif
    VOS_UINT32                   number;
    MSG_CB                      *msg    = VOS_NULL_PTR;
    VOS_DumpQueueContent *dump = VOS_NULL_PTR;

#if (VOS_DEBUG == VOS_DOPRA_VER)
    if (VOS_TRUE == g_vosQueueCtrlBlcok[queue].queueFullTag) {
        if (len >= sizeof(VOS_UINT32)) {
            content  = (VOS_UINT32 *)pcBuf;
            *content = 0x55aa5a5a; /* special value */
        }

        return;
    }

    g_vosQueueCtrlBlcok[queue].queueFullTag = VOS_TRUE;
#endif

    msg = (MSG_CB *)VOS_OutMsg(queue);

    number = len / sizeof(VOS_DumpQueueContent);
    dump  = (VOS_DumpQueueContent *)pcBuf;

    while (VOS_NULL_PTR != msg) {
        count++;

        if ((VOS_FID_QUEUE_LENGTH < count) || (count > number)) {
            mdrv_debug("<VOS_QueuePrintFull> count=%d.\n", (VOS_INT)count);

            return;
        }

        dump->sendPid = (VOS_UINT16)(msg->ulSenderPid);
        dump->rcvPid  = (VOS_UINT16)(msg->ulReceiverPid);
        dump->msgName = *(VOS_UINT32 *)(msg->value);

        dump++;

        msg = (MSG_CB *)VOS_OutMsg(queue);
    }

    return;
}

/*
 * Description: check queue's msg number
 */
MODULE_EXPORTED VOS_UINT32 VOS_CheckTaskQueue(VOS_UINT32 pid, VOS_UINT32 entries)
{
    int        fid;
    VOS_UINT32 qid;
    int        suffix;

    fid  = g_vosPidRecords[pid - VOS_PID_DOPRAEND].fid;
    qid  = g_vosFidCtrlBlk[fid].qid;
    suffix = g_vosPidRecords[pid - VOS_PID_DOPRAEND].priority;
    if (g_vosQueueCtrlBlcok[qid].q[suffix].qEntries > entries) {
        return VOS_ERR;
    }
    return VOS_OK;
}

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
/*
 * Description: delete message from a queue
 */
VOS_VOID VOS_DelQueueInfo(VOS_UINT32 queueID, VOS_UINT32 timeOutInMillSec)
{
    int          i;
    VOS_UINT32   lockLevel;
    VOS_UINT_PTR tempValue;
    VOS_UINT32   iThePid;
    MSG_CB      *msg = VOS_NULL_PTR;

    VOS_QueueControlBlock *queueCtrlBlk = &g_vosQueueCtrlBlcok[queueID];

    /* 北研添加的只在C核使用所以不用替换为自旋锁 */
    VOS_SpinLockIntLock(&g_vosQueueSpinLock, lockLevel);

    /* which should be del when only one FID exists */
    for (i = (VOS_INT)(queueCtrlBlk->qNum - 1); i >= 0; i--) {
        while (0 != queueCtrlBlk->q[i].qEntries) {
            if (VOS_OK != VOS_SmP(queueCtrlBlk->semId, timeOutInMillSec)) {
                VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);

                return;
            }

            tempValue = *(queueCtrlBlk->q[i].qOut++);
            queueCtrlBlk->q[i].qEntries--;
            if (queueCtrlBlk->q[i].qOut == queueCtrlBlk->q[i].qEnd) {
                queueCtrlBlk->q[i].qOut = queueCtrlBlk->q[i].qStart;
            }

            msg    = (MSG_CB *)(tempValue + VOS_MSG_BLK_HEAD_LEN);
            iThePid = msg->ulReceiverPid;
            (VOS_VOID)VOS_FreeMsg(iThePid, msg);
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosQueueSpinLock, lockLevel);
}

/*
 * Description: delete message from a FID's queue
 */
MODULE_EXPORTED VOS_VOID VOS_ClearQueueInfoByFid(VOS_UINT32 fid)
{
    VOS_DelQueueInfo(VOS_GetQueueIDFromFid(fid), VOS_FID_MAX_MSG_LENGTH);
}
#endif

