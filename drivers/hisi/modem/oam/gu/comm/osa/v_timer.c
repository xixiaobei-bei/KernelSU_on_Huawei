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
 * 功能描述: OSA不可唤醒定时器功能实现
 * 生成日期: 2006年10月3日
 */

#include "v_typdef.h"
#include "v_timer.h"
#include "v_msg.h"
#include "v_task.h"
#include "v_id_def.h"
#include "v_io.h"
#include "v_id.h"
#include "time.h"
#include "v_sem.h"
#include "mdrv.h"
#include "vos_task_prio_def.h"
#include "vos.h"
#include "v_private.h"
#include "pam_tag.h"

#if (VOS_OS_VER != VOS_WIN32)
#include <securec.h>
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_V_TIMER_C
#define THIS_MODU mod_pam_osa

/* the state of Timer */
#define VOS_TIMER_CTRL_BLK_RUNNIG 0
#define VOS_TIMER_CTRL_BLK_PAUSE 1
#define VOS_TIMER_CTRL_BLK_STOP 2

/* the tag of  VOS_TimerCtrlBlkFree */
#define THE_FIRST_TIMER_TAG 1
#define THE_SECOND_TIMER_TAG 2

/* timer task's stack size */
#define VOS_TIMER_TASK_STACK_SIZE 3072

/* 32 bits */
#define VOS_TICK_MAX 0xffffffff

#define REL_TIMER_MESSAGE_LENGTH (sizeof(REL_TimerMsgBlock) - VOS_MSG_HEAD_LENGTH)

typedef struct VOS_TimerControl {
    VOS_UINT32     timerId;    /* timer ID */
    VOS_UINT32     usedFlag; /* whether be used or not */
    VOS_PID        pid;        /* who allocate the timer */
    VOS_UINT32     name;       /* timer's name */
    VOS_UINT32     para;       /* timer's paremate */
    VOS_UINT8      rsv[4];
    REL_TimerFunc callBackFunc; /* timer's callback function */
    HTIMER        *phTm;         /* user's pointer which point the real timer room */
    VOS_UINT32     timeOutValueInMilliSeconds;
    VOS_UINT32     timeOutValueInTicks;
    /* the message which be sent when timer expire */
    struct VOS_TimerControl *next;
    struct VOS_TimerControl *previous;
    VOS_UINT8                      mode;        /* timer's mode */
    VOS_UINT8                      state;       /* timer's state */
    VOS_UINT8                      reserved[2]; /* for 4 byte aligned */

#if VOS_YES == VOS_TIMER_CHECK
    VOS_UINT32 allocTick; /* CPU tick of block allocation */
    VOS_UINT32 fileId;    /* alloc file ID */
    VOS_UINT32 lineNo;    /* alloc line no. */
#endif
    VOS_UINT32 backUpTimerId; /* timer ID */
} VOS_TimerControlBlock;

/* the number of task's control block */
VOS_UINT32 g_vosTimerCtrlBlkNumber;

/* the number of free task's control block */
VOS_UINT32 g_vosTimerIdleCtrlBlkNumber;

/* the start address of task's control block */
VOS_TimerControlBlock *g_vosTimerCtrlBlk = VOS_NULL_PTR;

/* the start address of free task's control block list */
VOS_TimerControlBlock *g_vosTimerIdleCtrlBlk = VOS_NULL_PTR;

/* the begin address of timer control block */
VOS_VOID *g_vosTimerCtrlBlkBegin = VOS_NULL_PTR;

/* the end address of timer control block */
VOS_VOID *g_vosTimerCtrlBlkEnd = VOS_NULL_PTR;

/* the head of the running timer list */
VOS_TimerControlBlock *g_vosTimerHeadPtr = VOS_NULL_PTR;

/* the task ID of timer's task */
VOS_UINT32 g_vosTimerTaskId;

/* the Min usage of timer */
VOS_UINT32 g_vosTimerMinTimerIdUsed;

/* record elapsed cpu ticks when system boot */
CPU_TickEx g_vosCpuTick;

/* record current system time */
SYS_T g_sysTime;

#define VOS_TIMER_CTRL_BUF_SIZE (sizeof(VOS_TimerControlBlock) * VOS_MAX_TIMER_NUMBER)

VOS_CHAR g_vosTimerCtrlBuf[VOS_TIMER_CTRL_BUF_SIZE];

VOS_VOID VOS_TimerDump(VOS_INT modId, VOS_UINT32 fileId, VOS_UINT32 lineNo);

/* 自旋锁，用来作Timer的临界资源保护 */
VOS_SPINLOCK g_vosTimerSpinLock;

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#define VOS_26M_TIMER_ID (TIMER_ACPU_OM_TCXO_ID)
#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
#define VOS_26M_TIMER_ID (TIMER_CCPU_OM_TCXO_ID)
#endif

/* 记录 VOS 26 timer 可维可测信息 */
VOS_TimerSocTimerInfo g_26MSocTimerInfo;

/* the queue will be given when 26M's interrupt occures */
VOS_UINT32 g_vos26MTimerQueue;

/* record start value */
VOS_UINT32 g_vos26MStartValue = ELAPESD_TIME_INVAILD;

extern VOS_VOID OM_RecordInfoEnd(VOS_ExcDumpMemNumUint32 number);
extern VOS_VOID OM_RecordInfoStart(VOS_ExcDumpMemNumUint32 number, VOS_UINT32 sendPid, VOS_UINT32 rcvPid,
                                   VOS_UINT32 msgName);
extern VOS_VOID OM_OSAEvent(VOS_TimerOmEvent *data, VOS_UINT32 length);

/* Just for Sparse checking. */
VOS_VOID   VOS_Start26MHardTimer(VOS_UINT32 value);
VOS_UINT32 VOS_Get26MHardTimerElapsedTime(VOS_VOID);

/* 记录上报RTC信息的时间点 */
VOS_UINT32 g_rtcTimeReportSlice = 0;

#define VOS_RTC_REPORT_TIME_INTERVAL (327680)

/*
 * Description: handle timer's list
 */
VOS_VOID VOS_26MDualTimerIsrEntry(VOS_UINT32 elapsedCycles)
{
    VOS_TimerControlBlock *headPtr = VOS_NULL_PTR;
    /* the timer control which expire */
    VOS_TimerControlBlock *vosTimerCtrlBlkCurrent = VOS_NULL_PTR;
    /* the timer head control which expire */
    VOS_TimerControlBlock *vosTimerExpireHeadPtr = VOS_NULL_PTR;
    VOS_TimerControlBlock *vosTimerExpireTailPtr = VOS_NULL_PTR;
    VOS_UINT32               tempCount;

    if (VOS_NULL_PTR == g_vosTimerHeadPtr) {
        return;
    }

    headPtr = g_vosTimerHeadPtr;

    /* sub timer value */
    headPtr->timeOutValueInTicks -= elapsedCycles;

    tempCount = 0;

    /* check the left timer */
    while ((VOS_NULL_PTR != headPtr) && (0 == headPtr->timeOutValueInTicks)) {
        tempCount++;
        if (g_vosTimerCtrlBlkNumber < tempCount) {
            return;
        }

        vosTimerCtrlBlkCurrent = headPtr;

        *(headPtr->phTm) = VOS_NULL_PTR;

        headPtr = vosTimerCtrlBlkCurrent->next;

        vosTimerCtrlBlkCurrent->next = VOS_NULL_PTR;
        if (VOS_NULL_PTR == vosTimerExpireHeadPtr) {
            vosTimerExpireHeadPtr = vosTimerCtrlBlkCurrent;
            vosTimerExpireTailPtr = vosTimerCtrlBlkCurrent;
        } else {
            if (vosTimerExpireTailPtr != VOS_NULL_PTR) {
                vosTimerExpireTailPtr->next = vosTimerCtrlBlkCurrent;
            }
            vosTimerExpireTailPtr = vosTimerCtrlBlkCurrent;
        }

        g_vosTimerHeadPtr = headPtr;
    }

    if (VOS_NULL_PTR != g_vosTimerHeadPtr) {
        g_vosTimerHeadPtr->previous = VOS_NULL_PTR;

        /* 上面已经把为0的都过滤了，这里不会再有为0的 */
        if (0 == g_vosTimerHeadPtr->timeOutValueInTicks) {
            g_vosTimerHeadPtr->timeOutValueInTicks += 1;
        }

        VOS_Start26MHardTimer(g_vosTimerHeadPtr->timeOutValueInTicks);
    }

    if (VOS_NULL_PTR != vosTimerExpireHeadPtr) {
        if (VOS_OK != VOS_FixedQueueWriteDirect(g_vos26MTimerQueue, (VOS_VOID *)vosTimerExpireHeadPtr,
                                                VOS_NORMAL_PRIORITY_MSG)) {
            /* blank */
        }
    }
}

/*
 * Description: ISR of DualTimer
 */
VOS_INT32 VOS_26MDualTimerIsr(VOS_INT para)
{
    VOS_UINT32 elapsedCycles;
    VOS_ULONG  lockLevel;

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    g_26MSocTimerInfo.expireCount++;

    elapsedCycles = VOS_Get26MHardTimerElapsedTime();

    if (VOS_OK != mdrv_timer_stop((VOS_UINT)para)) {
        g_26MSocTimerInfo.stopErrCount++;
    }

    VOS_26MDualTimerIsrEntry(elapsedCycles);

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    return 0;
}

/*
 * Description: stop hard timer
 */
VOS_VOID VOS_Stop26MHardTimer(VOS_VOID)
{
    g_26MSocTimerInfo.stopCount++;

    if (VOS_OK != mdrv_timer_stop(VOS_26M_TIMER_ID)) {
        g_26MSocTimerInfo.stopErrCount++;
    }

    g_vos26MStartValue = ELAPESD_TIME_INVAILD;

    return;
}

/*
 * Description: start hard timer
 */
VOS_VOID VOS_Start26MHardTimer(VOS_UINT32 value)
{
    g_26MSocTimerInfo.startCount++;

    VOS_Stop26MHardTimer();

    g_vos26MStartValue = value;

    if (VOS_OK != mdrv_timer_start(VOS_26M_TIMER_ID, (timer_handle_cb)VOS_26MDualTimerIsr, VOS_26M_TIMER_ID, value,
                                   TIMER_ONCE_COUNT, TIMER_UNIT_NONE)) {
        g_26MSocTimerInfo.startErrCount++;
    }

    return;
}

/*
 * Description: get the elapsed time from hard timer
 */
VOS_UINT32 VOS_Get26MHardTimerElapsedTime(VOS_VOID)
{
    VOS_UINT32 tempValue = 0;

    if (ELAPESD_TIME_INVAILD == g_vos26MStartValue) {
        return 0;
    }

    if (VOS_OK != mdrv_timer_get_rest_time(VOS_26M_TIMER_ID, TIMER_UNIT_NONE, (VOS_UINT *)&tempValue)) {
        g_26MSocTimerInfo.elapsedErrCount++;
    }

    if (g_vos26MStartValue < tempValue) {
        g_26MSocTimerInfo.elapsedContentErrCount++;

        return g_vos26MStartValue;
    }

    return g_vos26MStartValue - tempValue;
}

/*
 * Description: Init timer's control block
 */
VOS_UINT32 VOS_TimerCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32 i;

    g_vosTimerCtrlBlkNumber     = VOS_MAX_TIMER_NUMBER;
    g_vosTimerIdleCtrlBlkNumber = VOS_MAX_TIMER_NUMBER;

    g_vosTimerCtrlBlk = (VOS_TimerControlBlock *)g_vosTimerCtrlBuf;

    g_vosTimerIdleCtrlBlk  = g_vosTimerCtrlBlk;
    g_vosTimerCtrlBlkBegin = (VOS_VOID *)g_vosTimerCtrlBlk;
    g_vosTimerCtrlBlkEnd =
        (VOS_VOID *)((VOS_UINT_PTR)(g_vosTimerCtrlBlk) + g_vosTimerCtrlBlkNumber * sizeof(VOS_TimerControlBlock));

    for (i = 0; i < g_vosTimerCtrlBlkNumber - 1; i++) {
        g_vosTimerCtrlBlk[i].state           = VOS_TIMER_CTRL_BLK_STOP;
        g_vosTimerCtrlBlk[i].usedFlag      = VOS_NOT_USED;
        g_vosTimerCtrlBlk[i].timerId         = i;
        g_vosTimerCtrlBlk[i].backUpTimerId = i;
        g_vosTimerCtrlBlk[i].phTm            = VOS_NULL_PTR;
        g_vosTimerCtrlBlk[i].callBackFunc    = VOS_NULL_PTR;
        g_vosTimerCtrlBlk[i].previous        = VOS_NULL_PTR;
        /*lint -e679*/
        g_vosTimerCtrlBlk[i].next = &(g_vosTimerCtrlBlk[i + 1]);
        /*lint +e679*/
    }

    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].state           = VOS_TIMER_CTRL_BLK_STOP;
    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].usedFlag      = VOS_NOT_USED;
    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].timerId         = g_vosTimerCtrlBlkNumber - 1;
    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].backUpTimerId = g_vosTimerCtrlBlkNumber - 1;
    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].phTm            = VOS_NULL_PTR;
    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].callBackFunc    = VOS_NULL_PTR;
    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].previous        = VOS_NULL_PTR;
    g_vosTimerCtrlBlk[g_vosTimerCtrlBlkNumber - 1].next            = VOS_NULL_PTR;

    g_vosTimerMinTimerIdUsed = g_vosTimerCtrlBlkNumber;

    VOS_SpinLockInit(&g_vosTimerSpinLock);

    if (memset_s(&g_26MSocTimerInfo, sizeof(VOS_TimerSocTimerInfo), 0x0,
                 sizeof(VOS_TimerSocTimerInfo)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    /* 1 -> only one queue */
    if (VOS_OK !=
        VOS_FixedQueueCreate(VOS_FID_QUEUE_LENGTH, &g_vos26MTimerQueue, VOS_MSG_Q_FIFO, VOS_FID_MAX_MSG_LENGTH, 1)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

/*
 * Description: allocte a block
 */
VOS_TimerControlBlock *VOS_TimerCtrlBlkGet(VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_TimerControlBlock *tempTimerCtrlPtr = VOS_NULL_PTR;

    if (0 == g_vosTimerIdleCtrlBlkNumber) {
        return ((VOS_TimerControlBlock *)VOS_NULL_PTR);
    } else {
        g_vosTimerIdleCtrlBlkNumber--;

        tempTimerCtrlPtr             = g_vosTimerIdleCtrlBlk;
        tempTimerCtrlPtr->usedFlag = VOS_USED;
        g_vosTimerIdleCtrlBlk            = g_vosTimerIdleCtrlBlk->next;
    }

    /* record the usage of timer control block */
    if (g_vosTimerIdleCtrlBlkNumber < g_vosTimerMinTimerIdUsed) {
        g_vosTimerMinTimerIdUsed = g_vosTimerIdleCtrlBlkNumber;
    }

    tempTimerCtrlPtr->next     = VOS_NULL_PTR;
    tempTimerCtrlPtr->previous = VOS_NULL_PTR;

#if VOS_YES == VOS_TIMER_CHECK
    tempTimerCtrlPtr->fileId    = fileId;
    tempTimerCtrlPtr->lineNo    = (VOS_UINT32)lineNo;
    tempTimerCtrlPtr->allocTick = VOS_GetSlice();
#endif

    return tempTimerCtrlPtr;
}

/*
 * Description: free a block
 */
VOS_UINT32 VOS_TimerCtrlBlkFree(VOS_TimerControlBlock *ptr, VOS_UINT8 tag)
{
    if ((VOS_UINT_PTR)ptr < (VOS_UINT_PTR)g_vosTimerCtrlBlkBegin ||
        (VOS_UINT_PTR)ptr > (VOS_UINT_PTR)g_vosTimerCtrlBlkEnd) {
        return VOS_ERR;
    }

    if (VOS_NOT_USED == ptr->usedFlag) {
        return VOS_ERRNO_RELTM_FREE_RECEPTION;
    }

    ptr->usedFlag      = VOS_NOT_USED;
    ptr->reserved[0]     = tag;
    ptr->next            = g_vosTimerIdleCtrlBlk;
    g_vosTimerIdleCtrlBlk = ptr;

    g_vosTimerIdleCtrlBlkNumber++;

    return VOS_OK;
}

/*
 * Description: timer task entry
 */
VOS_VOID VOS_TimerTaskFunc(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3)
{
    VOS_TimerControlBlock *headPtr = VOS_NULL_PTR;
    /* the timer head control which expire */
    VOS_TimerControlBlock *vosTimerExpireHeadPtr = VOS_NULL_PTR;
    VOS_ULONG                lockLevel;
    REL_TimerMsgBlock       *expireMsg = VOS_NULL_PTR;
    VOS_UINT_PTR             tempValue;
    VOS_UINT_PTR             ctrlBlkAddress;

    /* record slice for RTC OM report */
    VOS_UINT32 currentSlice = 0;


    g_vosCpuTick.high = 0;
    g_vosCpuTick.low  = 0;

    /* here should clear the count SEM when use 10ms INT */

    for (;;) {
        if (VOS_ERR == VOS_FixedQueueRead(g_vos26MTimerQueue, 0, &ctrlBlkAddress, VOS_FID_MAX_MSG_LENGTH)) {
            continue;
        }

        vosTimerExpireHeadPtr = (VOS_TimerControlBlock *)ctrlBlkAddress;

        while (VOS_NULL_PTR != vosTimerExpireHeadPtr) {
            headPtr = vosTimerExpireHeadPtr;

            if (VOS_RELTIMER_LOOP == headPtr->mode) {
                if (VOS_NULL_PTR == headPtr->callBackFunc) {
                    V_Start26MRelTimer(headPtr->phTm, headPtr->pid, headPtr->timeOutValueInMilliSeconds,
                                       headPtr->name, headPtr->para, VOS_RELTIMER_LOOP,
                                       headPtr->fileId, (VOS_INT32)headPtr->lineNo);
                } else {
                    V_Start26MCallBackRelTimer(headPtr->phTm, headPtr->pid, headPtr->timeOutValueInMilliSeconds,
                                               headPtr->name, headPtr->para, VOS_RELTIMER_LOOP,
                                               headPtr->callBackFunc,
                                               headPtr->fileId, (VOS_INT32)headPtr->lineNo);
                }
            }

            tempValue = (VOS_UINT_PTR)(vosTimerExpireHeadPtr->callBackFunc);

            /* CallBackFunc需要用32位传入，所以和name互换位置保证数据不丢失 */
            OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_4, (VOS_UINT32)(vosTimerExpireHeadPtr->pid),
                               vosTimerExpireHeadPtr->name, (VOS_UINT32)tempValue);

            if (VOS_NULL_PTR == vosTimerExpireHeadPtr->callBackFunc) {
                /* Alloc expires's Msg */
                expireMsg = VOS_TimerPreAllocMsg(vosTimerExpireHeadPtr->pid);
                if (VOS_NULL_PTR != expireMsg) {
                    expireMsg->name = vosTimerExpireHeadPtr->name;
                    expireMsg->para = vosTimerExpireHeadPtr->para;

                    tempValue         = (VOS_UINT_PTR)(vosTimerExpireHeadPtr->timeOutValueInMilliSeconds);
                    expireMsg->hTm = (HTIMER)tempValue;

#if (VOS_YES == VOS_TIMER_CHECK)
                    tempValue           = (VOS_UINT_PTR)(vosTimerExpireHeadPtr->allocTick);
                    expireMsg->next = (struct REL_TimerMsg *)tempValue;

                    tempValue           = (VOS_UINT_PTR)VOS_GetSlice();
                    expireMsg->prev = (struct REL_TimerMsg *)tempValue;
#endif

                    if (VOS_OK != VOS_SendMsg(DOPRA_PID_TIMER, expireMsg)) {
                        mdrv_err("<VOS_TimerTaskFunc> OSA Timer task Send Msg Fail.\n");
                    }
                }
            } else {
                vosTimerExpireHeadPtr->callBackFunc(vosTimerExpireHeadPtr->para,
                                                        vosTimerExpireHeadPtr->name);
            }

            OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_4);

            vosTimerExpireHeadPtr = vosTimerExpireHeadPtr->next;

            VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

            (VOS_VOID)VOS_TimerCtrlBlkFree(headPtr, THE_FIRST_TIMER_TAG);

            VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);
        }

        /* 获取当前的slice */
        currentSlice = VOS_GetSlice();
        /* 超过10s或出现反转时上报一次RTC可维可测信息 */
        if ((currentSlice - g_rtcTimeReportSlice) > VOS_RTC_REPORT_TIME_INTERVAL) {
            /* 更新RTC信息上报的时间 */
            g_rtcTimeReportSlice = currentSlice;

            /* 上报RTC可维可测信息 */
            RTC_ReportOmInfo();
        }
    }
}

/*
 * Description: allocte timer expire message's memory
 */
REL_TimerMsgBlock *VOS_TimerPreAllocMsg(VOS_PID pid)
{
    REL_TimerMsgBlock *msg = VOS_NULL_PTR;

    msg = (REL_TimerMsgBlock *)VOS_AllocTimerMsg(pid, REL_TIMER_MESSAGE_LENGTH);
    if (VOS_NULL_PTR == msg) {
        return VOS_NULL_PTR;
    }

    msg->type      = VOS_RELATIVE_TIMER;
    msg->valid     = VOS_TM_INVALID_MSG;
    msg->reserved[0] = 0;
    msg->reserved[1] = 0;
    msg->next       = (struct REL_TimerMsg *)VOS_NULL_PTR;
    msg->prev       = (struct REL_TimerMsg *)VOS_NULL_PTR;

    return msg;
}

/*
 * Description: create timer task
 */
VOS_UINT32 VOS_TimerTaskCreat(VOS_VOID)
{
    VOS_UINT32 timerArguments[4] = { 0, 0, 0, 0 };

    return (VOS_CreateTask("VOS_TIMER", &g_vosTimerTaskId, VOS_TimerTaskFunc, COMM_TIMER_TASK_PRIO,
                           VOS_TIMER_TASK_STACK_SIZE, timerArguments));
}

/*
 * Description: add a timer to list
 */
VOS_VOID Add_Timer_To_List(VOS_TimerControlBlock *timer)
{
    VOS_TimerControlBlock *tempPtr      = VOS_NULL_PTR;
    VOS_TimerControlBlock *preTempPtr  = VOS_NULL_PTR;
    VOS_UINT32               elapsedCycles = 0;

    if (VOS_NULL_PTR == g_vosTimerHeadPtr) {
        g_vosTimerHeadPtr = timer;
    } else {
        elapsedCycles = VOS_Get26MHardTimerElapsedTime();

        timer->timeOutValueInTicks += elapsedCycles;

        /*  find the location to insert */
        tempPtr = preTempPtr = g_vosTimerHeadPtr;
        while ((tempPtr != VOS_NULL_PTR) && (timer->timeOutValueInTicks >= tempPtr->timeOutValueInTicks)) {
            timer->timeOutValueInTicks -= tempPtr->timeOutValueInTicks;
            preTempPtr = tempPtr;
            tempPtr     = tempPtr->next;
        }

        /* insert timer < head timer */
        if (tempPtr == g_vosTimerHeadPtr) {
            timer->next        = g_vosTimerHeadPtr;
            g_vosTimerHeadPtr = timer;
        } else {
            timer->next        = tempPtr;
            preTempPtr->next = timer;
            timer->previous    = preTempPtr;
        }

        if (tempPtr != VOS_NULL_PTR) {
            tempPtr->timeOutValueInTicks = tempPtr->timeOutValueInTicks - timer->timeOutValueInTicks;
            tempPtr->previous            = timer;
        }
    }

    /* restart RTC timer */
    if (g_vosTimerHeadPtr == timer) {
        /* judge timer value when the new timer at head */
        timer->timeOutValueInTicks -= elapsedCycles;

        if (0 == timer->timeOutValueInTicks) {
            timer->timeOutValueInTicks += 1;
        }

        VOS_Start26MHardTimer(timer->timeOutValueInTicks);
    }
}

/*
 * 功能描述: 检查TIMER是否正确
 */
VOS_UINT32 VOS_CheckTimer(HTIMER *phTm, VOS_UINT32 *timerID, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_TimerControlBlock *timer = VOS_NULL_PTR;

    if (((VOS_UINT_PTR)*phTm >= (VOS_UINT_PTR)g_vosTimerCtrlBlkBegin) &&
        ((VOS_UINT_PTR)*phTm < (VOS_UINT_PTR)g_vosTimerCtrlBlkEnd)) {
        timer = (VOS_TimerControlBlock *)(*phTm);

        if (phTm == timer->phTm) {
            *timerID = timer->backUpTimerId;

            if (timer->backUpTimerId != timer->timerId) {
                timer->timerId = timer->backUpTimerId;
            }

            return VOS_OK;
        }

        VOS_SIMPLE_FATAL_ERROR(VOS_CHECK_TIMER_ID);
    } else {
        VOS_ProtectionReboot(VOS_CHECK_TIMER_RANG, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)phTm, sizeof(VOS_CHAR *));
    }

    return VOS_ERR;
}

/*
 * 功能描述: 申请并启动一个定时器
 */
MODULE_EXPORTED VOS_UINT32 V_StartRelTimer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name,
                                           VOS_UINT32 param, VOS_UINT8 mode,
                                           VOS_TimerPrecisionUint32  precision, VOS_UINT32 fileId,
                                           VOS_INT32 lineNo)
{
    VOS_TimerOmEvent timerEvent;
    VOS_UINT32              result;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    VOS_UINT32 bit64Timer = VOS_FALSE;
#endif

    if ((mode != VOS_RELTIMER_LOOP) && (mode != VOS_RELTIMER_NOLOOP)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_START_INPUTMODEINVALID);

        mdrv_err("<V_StartRelTimer> ucMode.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_RELTIMER_FLASE_MODE);

        return (VOS_ERRNO_RELTM_START_INPUTMODEINVALID);
    }

    if (VOS_NULL_PTR == phTm) {
        mdrv_err("<VOS_StartRelTimer Error> phTm null.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_RELTIMER_NULL);

        return VOS_ERR;
    }

    if (length > VOS_TIMER_MAX_LENGTH) {
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
        if (VOS_RELTIMER_LOOP == mode) {
            mdrv_err("<VOS_StartRelTimer Error> Timer's length and Mode is wrong.F=%d L=%d.\n", (VOS_INT)fileId,
                     lineNo);

            return VOS_ERR;
        } else {
            bit64Timer = VOS_TRUE;
        }
#else
        mdrv_err("<VStartRelTimer> Timer's length is too big.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_RELTIMER_TOO_BIG);

        return VOS_ERR;
#endif
    }

    if (VOS_TIMER_PRECISION_BUTT <= precision) {
        mdrv_err("<VStartRelTimer> Timer's Precision is too big.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_RELTIMER_PRECISION_TOO_BIG);

        return VOS_ERR;
    }

    if ((VOS_TIMER_PRECISION_100 == precision) && (VOS_RELTIMER_LOOP == mode)) {
        mdrv_err("<VStartRelTimer> Timer's para is wrong.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        return VOS_ERR;
    }

    if (length < MILLISECONDS_PER_TICK) {
        length = MILLISECONDS_PER_TICK;
    }

    if ((pid < VOS_PID_DOPRAEND) || (pid >= VOS_PID_BUTT)) {
        mdrv_err("<VStartRelTimer> Timer's pid is not belong to local cpu.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_RELTIMER_WRONG_CPUPID);

        return VOS_ERR;
    }

    timerEvent.action    = VOS_TIMER_OM_START;
    timerEvent.mode      = mode;
    timerEvent.type      = VOS_DELTM_ON_TIMEOUTMSG;
    timerEvent.pid         = pid;
    timerEvent.length    = length;
    timerEvent.name      = name;
    timerEvent.param     = param;
    timerEvent.precision = precision;

    /* 获取RTC的可维可测信息(Action和Slice) */
    RTC_GetDebugSocInfo(&(timerEvent.rTCAction), &(timerEvent.rTCSlice), &(timerEvent.coreId));

#if (VOS_32K_TIMER_ENABLE == VOS_YES)

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if (VOS_TRUE == bit64Timer) {
        result = VOS_StartBit64Timer(phTm, pid, length, name, param, fileId, lineNo);

        timerEvent.result        = (VOS_UINT8)result;
        timerEvent.hTimerAddress = *phTm;

        OM_OSAEvent((VOS_VOID *)&timerEvent, sizeof(VOS_TimerOmEvent));

        return result;
    }
#endif

#if ((VOS_LINUX == VOS_OS_VER) || (VOS_RTOSCK == VOS_OS_VER))
    if (VOS_TIMER_NO_PRECISION != precision) {
        result = V_Start32KRelTimer(phTm, pid, length, name, param, mode, precision, fileId, lineNo);

        timerEvent.result        = (VOS_UINT8)result;
        timerEvent.hTimerAddress = *phTm;

        OM_OSAEvent((VOS_VOID *)&timerEvent, sizeof(VOS_TimerOmEvent));

        return result;
    }
#endif

#endif

    result = V_Start26MRelTimer(phTm, pid, length, name, param, mode, fileId, lineNo);

    timerEvent.result        = (VOS_UINT8)result;
    timerEvent.hTimerAddress = *phTm;

    OM_OSAEvent((VOS_VOID *)&timerEvent, sizeof(VOS_TimerOmEvent));

    return result;
}

/*
 * Description: Allocate and start a 26M relative timer.
 */
VOS_UINT32 V_Start26MRelTimer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name, VOS_UINT32 param,
                              VOS_UINT8 mode, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_TimerControlBlock *timer   = VOS_NULL_PTR;
    VOS_UINT32               timerId = 0;
    VOS_ULONG                lockLevel;

    /* stop the timer if exists */
    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    if (VOS_NULL_PTR != *phTm) {
        if (VOS_OK == VOS_CheckTimer(phTm, &timerId, fileId, lineNo)) {
            if (VOS_OK != V_Stop26MRelTimer(phTm, fileId, lineNo, VOS_NULL_PTR)) {
                VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

                VOS_SIMPLE_FATAL_ERROR(START_26M_RELTIMER_FAIL_TO_STOP);

                return VOS_ERR;
            }
        }
    }

    timer = VOS_TimerCtrlBlkGet(fileId, lineNo);
    if (VOS_NULL_PTR == timer) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_START_MSGNOTINSTALL);

        VOS_TimerDump(START_26M_RELTIMER_FAIL_TO_ALLOCATE, fileId, (VOS_UINT32)lineNo);

        return (VOS_ERRNO_RELTM_START_MSGNOTINSTALL);
    }

    timer->pid                        = pid;
    timer->name                       = name;
    timer->para                       = param;
    timer->mode                       = mode;
    timer->phTm                       = phTm;
    timer->timeOutValueInMilliSeconds = length;

    timer->timeOutValueInTicks = (VOS_UINT32)RTC_MUL_32_DOT_768(length, fileId, lineNo);

    timer->callBackFunc = VOS_NULL_PTR;

    *phTm = (HTIMER)(&(timer->timerId));

    /* add the timer to the running list */
    Add_Timer_To_List(timer);

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    return (VOS_OK);
}

/*
 * Description: del a timer from list
 */
VOS_VOID Del_Timer_From_List(VOS_TimerControlBlock *timer)
{
    VOS_BOOL bIsHead = VOS_FALSE;

    /* deletet this timer from list */
    if (timer == g_vosTimerHeadPtr) {
        bIsHead = VOS_TRUE;

        g_vosTimerHeadPtr = timer->next;
        if (VOS_NULL_PTR != g_vosTimerHeadPtr) {
            g_vosTimerHeadPtr->previous = VOS_NULL_PTR;
        }
    } else {
        (timer->previous)->next = timer->next;
        if (VOS_NULL_PTR != timer->next) {
            (timer->next)->previous = timer->previous;
        }
    }

    /* adjust the time_val after this timer */
    if (timer->next != NULL) {
        timer->next->timeOutValueInTicks += timer->timeOutValueInTicks;

        if (VOS_TRUE == bIsHead) {
            timer->next->timeOutValueInTicks -= VOS_Get26MHardTimerElapsedTime();

            if (0 == timer->next->timeOutValueInTicks) {
                timer->next->timeOutValueInTicks += 1;
            }

            VOS_Start26MHardTimer(timer->next->timeOutValueInTicks);
        }
    }

    /* Stop soc timer if no timer */
    if (VOS_NULL_PTR == g_vosTimerHeadPtr) {
        VOS_Stop26MHardTimer();
    }
}

/*
 * Description: stop a relative timer which was previously started.
 */
VOS_UINT32 V_Stop26MRelTimer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo, VOS_TimerOmEvent *event)
{
    VOS_UINT32               timerId = 0;
    VOS_TimerControlBlock *timer   = VOS_NULL_PTR;

    if (VOS_NULL_PTR == *phTm) {
        return VOS_OK;
    }

    if (VOS_OK != VOS_CheckTimer(phTm, &timerId, fileId, lineNo)) {
        return (VOS_ERRNO_RELTM_STOP_TIMERINVALID);
    }

    timer = &g_vosTimerCtrlBlk[timerId];

    /* del the timer from the running list */
    Del_Timer_From_List(timer);

    *(timer->phTm) = VOS_NULL_PTR;

    /* OM */
    if (VOS_NULL_PTR != event) {
        event->mode      = timer->mode;
        event->pid         = timer->pid;
        event->length    = timer->timeOutValueInMilliSeconds;
        event->name      = timer->name;
        event->param     = timer->para;
        event->precision = VOS_TIMER_NO_PRECISION;
    }

    return VOS_TimerCtrlBlkFree(timer, THE_SECOND_TIMER_TAG);
}

/*
 * Description: stop a relative timer which was previously started.
 */
MODULE_EXPORTED VOS_UINT32 V_StopRelTimer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_ULONG               lockLevel;
    VOS_UINT32              returnValue;
    VOS_TimerOmEvent timerEvent;

    if (VOS_NULL_PTR == phTm) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_STOP_INPUTISNULL);
        return (VOS_ERRNO_RELTM_STOP_INPUTISNULL);
    }

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    timerEvent.hTimerAddress = *phTm;

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if (VOS_TRUE == VOS_Bit64TimerCheckTimer(phTm)) {
        returnValue = VOS_StopBit64Timer(phTm, fileId, lineNo, &timerEvent);
    } else if (VOS_TRUE != VOS_Is26MTimer(phTm)) {
#else
    if (VOS_TRUE != VOS_Is26MTimer(phTm)) {
#endif
        returnValue = R_Stop32KTimer(phTm, fileId, lineNo, &timerEvent);
    } else {
        returnValue = V_Stop26MRelTimer(phTm, fileId, lineNo, &timerEvent);
    }

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    timerEvent.action = VOS_TIMER_OM_STOP;
    timerEvent.result = (VOS_UINT8)returnValue;

    /* 获取RTC的可维可测信息(Action和Slice) */
    RTC_GetDebugSocInfo(&(timerEvent.rTCAction), &(timerEvent.rTCSlice), &(timerEvent.coreId));

    OM_OSAEvent((VOS_VOID *)&timerEvent, sizeof(VOS_TimerOmEvent));

    return returnValue;
}

/*
 * Description: Restart a relative timer which was previously started
 */
VOS_UINT32 V_RestartRelTimer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32               timerId = 0;
    VOS_TimerControlBlock *timer   = VOS_NULL_PTR;
    VOS_ULONG                lockLevel;
    VOS_UINT32               returnValue;
    VOS_TimerOmEvent  timerEvent;

    if (VOS_NULL_PTR == phTm) {
        mdrv_err("<V_RestartRelTimer> phTm is null.");

        VOS_SIMPLE_FATAL_ERROR(RESTART_RELTIMER_NULL);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_RESTART_INPUTISNULL);
        return (VOS_ERRNO_RELTM_RESTART_INPUTISNULL);
    }

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if (VOS_TRUE == VOS_Bit64TimerCheckTimer(phTm)) {
        returnValue = VOS_RestartBit64Timer(phTm, fileId, lineNo);

        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return returnValue;
    }
#endif

    if (VOS_TRUE != VOS_Is26MTimer(phTm)) {
        returnValue = R_Restart32KRelTimer(phTm, fileId, lineNo);

        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return returnValue;
    }

    if (VOS_NULL_PTR == *phTm) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);

        VOS_SIMPLE_FATAL_ERROR(RESTART_RELTIMER_NOT_RUNNING);

        return (VOS_ERRNO_RELTM_RESTART_TIMERINVALID);
    }

    if (VOS_OK != VOS_CheckTimer(phTm, &timerId, fileId, lineNo)) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);

        VOS_SIMPLE_FATAL_ERROR(RESTART_RELTIMER_FAIL_TO_CHECK);

        return (VOS_ERRNO_RELTM_RESTART_TIMERINVALID);
    }

    timer = &g_vosTimerCtrlBlk[timerId];

    /* Del the old timer but not free timer control block */
    Del_Timer_From_List(timer);

    timer->timeOutValueInTicks = (VOS_UINT32)RTC_MUL_32_DOT_768(timer->timeOutValueInMilliSeconds, fileId, lineNo);

    timer->next     = VOS_NULL_PTR;
    timer->previous = VOS_NULL_PTR;

    /* add the new timer to list */
    Add_Timer_To_List(timer);

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    timerEvent.action        = VOS_TIMER_OM_RESTART;
    timerEvent.result        = (VOS_UINT8)VOS_OK;
    timerEvent.hTimerAddress = *phTm;

    /* 获取RTC的可维可测信息(Action和Slice) */
    RTC_GetDebugSocInfo(&(timerEvent.rTCAction), &(timerEvent.rTCSlice), &(timerEvent.coreId));

    OM_OSAEvent((VOS_VOID *)&timerEvent, sizeof(VOS_TimerOmEvent));

    return VOS_OK;
}

/*
 * Description: get left time
 */
MODULE_EXPORTED VOS_UINT32 V_GetRelTmRemainTime(HTIMER *phTm, VOS_UINT32 *tick, VOS_UINT32 fileId,
                                                VOS_INT32 lineNo)
{
    VOS_UINT32               timerId      = 0;
    VOS_UINT32               remainValue = 0;
    VOS_TimerControlBlock *headPtr     = VOS_NULL_PTR;
    VOS_TimerControlBlock *tempPtr     = VOS_NULL_PTR;
    VOS_ULONG                lockLevel;
    VOS_UINT32               tempTick = 0;

    if (VOS_NULL_PTR == phTm) {
        mdrv_err("<VOS_GetRelTmRemainTime> phTm null.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_STOP_INPUTISNULL);
        return (VOS_ERRNO_RELTM_STOP_INPUTISNULL);
    }

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if (VOS_TRUE == VOS_Bit64TimerCheckTimer(phTm)) {
        if (VOS_OK == VOS_GetBit64RemainTime(phTm, &tempTick, fileId, lineNo)) {
            *tick = tempTick / MILLISECONDS_PER_TICK;

            VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

            return VOS_OK;
        }

        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return VOS_ERR;
    }
#endif

    if (VOS_TRUE != VOS_Is26MTimer(phTm)) {
        if (VOS_OK == R_Get32KRelTmRemainTime(phTm, &tempTick, fileId, lineNo)) {
            *tick = tempTick / MILLISECONDS_PER_TICK;

            VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

            return VOS_OK;
        }

        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return VOS_ERR;
    }

    if (VOS_NULL_PTR == *phTm) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_STOP_INPUTISNULL);
        return (VOS_ERRNO_RELTM_STOP_INPUTISNULL);
    }

    if (VOS_OK != VOS_CheckTimer(phTm, &timerId, fileId, lineNo)) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_STOP_TIMERINVALID);
        return (VOS_ERRNO_RELTM_STOP_TIMERINVALID);
    }

    headPtr = g_vosTimerHeadPtr;

    while ((VOS_NULL_PTR != headPtr) && (headPtr->timerId != timerId)) {
        remainValue += (headPtr->timeOutValueInTicks);

        tempPtr = headPtr;
        headPtr = tempPtr->next;
    }

    if ((VOS_NULL_PTR == headPtr) || (headPtr->timerId != timerId)) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return VOS_ERR;
    } else {
        remainValue += (headPtr->timeOutValueInTicks);

        tempTick = VOS_Get26MHardTimerElapsedTime();

        *tick =
            (VOS_UINT32)(RTC_DIV_32_DOT_768((remainValue - tempTick), fileId, lineNo) / MILLISECONDS_PER_TICK);

        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return (VOS_OK);
    }
}

/*
 * Description: Check time is 32K or 26M.
 */
VOS_BOOL VOS_Is26MTimer(const HTIMER *phTm)
{
    if (((VOS_UINT_PTR)*phTm >= (VOS_UINT_PTR)g_vosTimerCtrlBlkBegin) &&
        ((VOS_UINT_PTR)*phTm < (VOS_UINT_PTR)g_vosTimerCtrlBlkEnd)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

#if (VOS_RTOSCK == VOS_OS_VER)
/*
 * Description: get tick
 */
MODULE_EXPORTED VOS_UINT32 VOS_GetTick(VOS_VOID)
{
    return (VOS_UINT32)SRE_TickCountGet();
}
#endif

#if (VOS_WIN32 == VOS_OS_VER)
/*
 * Description: get tick
 */
MODULE_EXPORTED VOS_UINT32 VOS_GetTick(VOS_VOID)
{
    return g_vosCpuTick.low;
}
#endif

#if (VOS_LINUX == VOS_OS_VER)
/*
 * Description: get tick
 */
MODULE_EXPORTED VOS_UINT32 VOS_GetTick(VOS_VOID)
{
    return (VOS_UINT32)jiffies;
}
#endif

/*
 * 功能描述: 申请一个带有回调的定时器
 */
MODULE_EXPORTED VOS_UINT32 V_StartCallBackRelTimer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name,
                                                   VOS_UINT32 param, VOS_UINT8 mode, REL_TimerFunc timeOutRoutine,
                                                   VOS_TimerPrecisionUint32  precision, VOS_UINT32 fileId,
                                                   VOS_INT32 lineNo)
{
    VOS_TimerOmEvent timerEvent;
    VOS_UINT32              result;

    if ((mode != VOS_RELTIMER_LOOP) && (mode != VOS_RELTIMER_NOLOOP)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_START_INPUTMODEINVALID);

        mdrv_err("<V_StartCallBackRelTimer> ucMode.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_CALLBACK_RELTIMER_FALSE_MODE);

        return (VOS_ERRNO_RELTM_START_INPUTMODEINVALID);
    }

    if (VOS_NULL_PTR == phTm) {
        mdrv_err("<V_StartCallBackRelTimer>  phTm null.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_CALLBACK_RELTIMER_NULL);

        return VOS_ERR;
    }

    if (length > VOS_TIMER_MAX_LENGTH) {
        mdrv_err("<V_StartCallBackRelTimer> Timer's length is too big.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_CALLBACK_RELTIMER_TOO_BIG);

        return VOS_ERR;
    }

    if (VOS_NULL_PTR == timeOutRoutine) {
        mdrv_err("<V_StartCallBackRelTimer> Timer no callback.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_CALLBACK_RELTIMER_FUN_NULL);

        return VOS_ERR;
    }

    if (VOS_TIMER_PRECISION_BUTT <= precision) {
        mdrv_err("<V_StartCallBackRelTimer> Timer's Precision is too big.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        VOS_SIMPLE_FATAL_ERROR(START_CALLBACK_RELTIMER_PRECISION_TOO_BIG);

        return VOS_ERR;
    }

    if (length < MILLISECONDS_PER_TICK) {
        length = MILLISECONDS_PER_TICK;
    }

    timerEvent.action    = VOS_TIMER_OM_START;
    timerEvent.mode      = mode;
    timerEvent.type      = VOS_RELTIMER_CALLBACK;
    timerEvent.pid         = pid;
    timerEvent.length    = length;
    timerEvent.name      = name;
    timerEvent.param     = param;
    timerEvent.precision = precision;

    /* 获取RTC的可维可测信息(Action和Slice) */
    RTC_GetDebugSocInfo(&(timerEvent.rTCAction), &(timerEvent.rTCSlice), &(timerEvent.coreId));

#if (VOS_32K_TIMER_ENABLE == VOS_YES)

#if ((VOS_LINUX == VOS_OS_VER) || (VOS_RTOSCK == VOS_OS_VER))
    if (VOS_TIMER_NO_PRECISION != precision) {
        result = V_Start32KCallBackRelTimer(phTm, pid, length, name, param, mode, timeOutRoutine, precision,
                                              fileId, lineNo);

        timerEvent.result        = (VOS_UINT8)result;
        timerEvent.hTimerAddress = *phTm;

        OM_OSAEvent((VOS_VOID *)&timerEvent, sizeof(VOS_TimerOmEvent));

        return result;
    }
#endif

#endif

    result = V_Start26MCallBackRelTimer(phTm, pid, length, name, param, mode, timeOutRoutine, fileId,
                                          lineNo);

    timerEvent.result        = (VOS_UINT8)result;
    timerEvent.hTimerAddress = *phTm;

    OM_OSAEvent((VOS_VOID *)&timerEvent, sizeof(VOS_TimerOmEvent));

    return result;
}

/*
 * Description: allocate and start a 26M relative timer using callback function.
 */
VOS_UINT32 V_Start26MCallBackRelTimer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name,
                                      VOS_UINT32 param, VOS_UINT8 mode, REL_TimerFunc timeOutRoutine,
                                      VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_TimerControlBlock *timer   = VOS_NULL_PTR;
    VOS_UINT32               timerId = 0;
    VOS_ULONG                lockLevel;

    /* stop the timer if exists */
    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    if (VOS_NULL_PTR != *phTm) {
        if (VOS_OK == VOS_CheckTimer(phTm, &timerId, fileId, lineNo)) {
            if (VOS_OK != V_Stop26MRelTimer(phTm, fileId, lineNo, VOS_NULL_PTR)) {
                VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

                VOS_SIMPLE_FATAL_ERROR(START_CALLBACK_RELTIMER_FAIL_TO_STOP);

                return VOS_ERR;
            }
        }
    }

    timer = VOS_TimerCtrlBlkGet(fileId, lineNo);
    if (timer == VOS_NULL_PTR) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_START_MSGNOTINSTALL);

        VOS_TimerDump(START_CALLBACK_RELTIMER_FAIL_TO_ALLOCATE, fileId, (VOS_UINT32)lineNo);

        return (VOS_ERRNO_RELTM_START_MSGNOTINSTALL);
    }

    timer->pid                        = pid;
    timer->name                       = name;
    timer->para                       = param;
    timer->mode                       = mode;
    timer->phTm                       = phTm;
    timer->timeOutValueInMilliSeconds = length;

    timer->timeOutValueInTicks = (VOS_UINT32)RTC_MUL_32_DOT_768(length, fileId, lineNo);

    timer->callBackFunc = timeOutRoutine;

    *phTm = (HTIMER)(&(timer->timerId));

    Add_Timer_To_List(timer);

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    return (VOS_OK);
}

/*
 * Description: Convert ticks to milliseconds
 */
MODULE_EXPORTED VOS_UINT32 VOS_TmTickToMillSec(VOS_UINT32 ticks)
{
    VOS_UINT32 tempMillSec;

#if (0 == MILLISECONDS_PER_TICK)
    tempMillSec = VOS_NULL_DWORD;

    (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_SYSTIME_TK2MS_MSECPERTICKISZERO);
#else
    if (ticks > (VOS_NULL_DWORD / MILLISECONDS_PER_TICK)) {
        tempMillSec = VOS_NULL_DWORD;
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_SYSTIME_TK2MS_INPUTTICKISTOOBIG);
    } else {
        tempMillSec = ticks * MILLISECONDS_PER_TICK;
    }
#endif

    return tempMillSec;
}

/*
 * Description: a 26M timer is running or not
 */
VOS_UINT32 VOS_Is26MTimerRunning(VOS_VOID)
{
    if (VOS_NULL_PTR != g_vosTimerHeadPtr) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * Description: print the usage info of timer
 */
MODULE_EXPORTED VOS_VOID VOS_show_Timer_info(VOS_VOID)
{
    mdrv_debug("<VOS_show_Timer_info> Max be used timer is %x.\n", g_vosTimerCtrlBlkNumber - g_vosTimerMinTimerIdUsed);
}

/*
 * Description: print the usage info of timer
 */
VOS_BOOL VOS_CalcTimerInfo(VOS_VOID)
{
    if (VOS_UPPER_TIMER_NUMBER > g_vosTimerMinTimerIdUsed) {
        g_vosTimerMinTimerIdUsed = g_vosTimerCtrlBlkNumber;
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * Description: Record the info of used timer when UE reboots.
 */
VOS_VOID VOS_TimerDump(VOS_INT modId, VOS_UINT32 fileId, VOS_UINT32 lineNo)
{
    VOS_ULONG                lockLevel;
    VOS_UINT32              *dumpBuffer = VOS_NULL_PTR;
    VOS_TimerControlBlock *timer      = VOS_NULL_PTR;

    dumpBuffer = (VOS_UINT32 *)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == dumpBuffer) {
        VOS_ProtectionReboot(modId, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&g_vosTimerMinTimerIdUsed,
                             sizeof(g_vosTimerMinTimerIdUsed));
        return;
    }

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    timer = g_vosTimerHeadPtr;
    while (VOS_NULL_PTR != timer) {
        *(dumpBuffer++) = timer->pid;
        *(dumpBuffer++) = timer->name;
#if VOS_YES == VOS_TIMER_CHECK
        *(dumpBuffer++) = timer->fileId;
        *(dumpBuffer++) = timer->lineNo;
#endif
        timer = timer->next;
    }

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    VOS_ProtectionReboot(modId, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&g_vosTimerMinTimerIdUsed,
                         sizeof(g_vosTimerMinTimerIdUsed));

    return;
}

/*
 * Description: print debug info
 */
MODULE_EXPORTED VOS_VOID Show26MTimerLog(VOS_VOID)
{
    mdrv_debug("<Show26MTimerLog> g_26MSocTimerInfo.ulStartCount=%d. (call DRV Start timer num)\n",
               g_26MSocTimerInfo.startCount);
    mdrv_debug("<Show26MTimerLog> g_26MSocTimerInfo.ulStopCount=%d. (call DRV Stop timer num)\n",
               g_26MSocTimerInfo.stopCount);
    mdrv_debug("<Show26MTimerLog> g_26MSocTimerInfo.ulExpireCount=%d. (receive DRV ISR of DualTimer num)\n",
               g_26MSocTimerInfo.expireCount);
    mdrv_debug("<Show26MTimerLog> g_26MSocTimerInfo.ulStartErrCount=%d. (call DRV Stop timer err num)\n",
               g_26MSocTimerInfo.startErrCount);
    mdrv_debug("<Show26MTimerLog> g_26MSocTimerInfo.ulStopErrCount=%d. (call DRV Start timer err num)\n",
               g_26MSocTimerInfo.stopErrCount);
    mdrv_debug("<Show26MTimerLog> g_26MSocTimerInfo.ulElapsedErrCount=%d. (call DRV get rest timer num)\n",
               g_26MSocTimerInfo.elapsedErrCount);
    mdrv_debug("<Show26MTimerLog> g_26MSocTimerInfo.ulElapsedContentErrCount=%d. (call DRV get rest timer err num)\n",
               g_26MSocTimerInfo.elapsedContentErrCount);
}

