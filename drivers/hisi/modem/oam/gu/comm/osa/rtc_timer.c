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
 * 功能描述: OSA可唤醒定时器功能实现
 * 生成日期: 2006年10月3日
 */

#include "vos.h"
#include "v_io.h"
#include "v_private.h"
#include "msp_diag_comm.h"
#include "mdrv.h"
#include "pam_tag.h"

#if (VOS_OS_VER != VOS_WIN32)
#include <securec.h>
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_V_RTC_TIMER_C
#define THIS_MODU mod_pam_osa

/* the state of Timer */
#define RTC_TIMER_CTRL_BLK_RUNNIG 0
#define RTC_TIMER_CTRL_BLK_PAUSE 1
#define RTC_TIMER_CTRL_BLK_STOP 2

/* the tag of  RTC_TimerCtrlBlkFree */
#define THE_FIRST_RTC_TIMER_TAG 1
#define THE_SECOND_RTC_TIMER_TAG 2

/* can't make a mistake,add threshold */
#define RTC_TIMER_CHECK_PRECISION 10

#define RTC_SOC_TIMER_CHECK_PRECISION 100
#define RTC_TIMER_CHECK_LONG_PRECISION 32768

/* the tag of  RTC_Add_Timer_To_List */
enum {
    VOS_ADD_TIMER_TAG_1,
    VOS_ADD_TIMER_TAG_2,
    VOS_ADD_TIMER_TAG_3,
    VOS_ADD_TIMER_TAG_4,
    VOS_ADD_TIMER_TAG_5,
    VOS_ADD_TIMER_TAG_BUTT
};

/* timer task's stack size */
#define RTC_TIMER_TASK_STACK_SIZE 3072

/* 1s->1000ms */
#define RTC_ONE_SECOND 1000

/* 32.768K */
#define RTC_PRECISION_CYCLE_LENGTH 0.32768

#define RTC_TIMER_NORMAL_COUNT 5

#ifndef BIT
#define BIT(x) ((unsigned)0x1 << (x))
#endif

typedef struct RTC_TimerControl {
    VOS_UINT32                     timerId;    /* timer ID */
    VOS_UINT32                     usedFlag; /* whether be used or not */
    VOS_PID                        pid;        /* who allocate the timer */
    VOS_UINT32                     name;       /* timer's name */
    VOS_UINT32                     para;       /* timer's paremate */
    VOS_UINT8                      rsv[4];
    REL_TimerFunc                 callBackFunc; /* timer's callback function */
    HTIMER                        *phTm;         /* user's pointer which point the real timer room */
    VOS_UINT32                     timeOutValueInMilliSeconds;
    VOS_UINT32                     timeOutValueInCycle;
    struct RTC_TimerControl *next;
    struct RTC_TimerControl *previous;
    VOS_UINT8                      mode;               /* timer's mode */
    VOS_UINT8                      state;              /* timer's state */
    VOS_UINT8                      reserved[2];        /* for 4 byte aligned */
    VOS_UINT32                     precision;        /* record only */
    VOS_UINT32                     precisionInCycle; /* unit is 32K cycle */

#if VOS_YES == VOS_TIMER_CHECK
    VOS_UINT32 allocTick; /* CPU tick of block allocation */
    VOS_UINT32 freeTick;  /* CPU tick of block free */
    VOS_UINT32 fileId;    /* alloc file ID */
    VOS_UINT32 lineNo;    /* alloc line no. */
    VOS_UINT32 backUpTimeOutValueInCycle;
#endif
    VOS_UINT32 backUpTimerId; /* timer ID */
} RTC_TimerControlBlock;

typedef struct DRX_TimerControl {
    VOS_UINT32 usedFlag;                   /* whether be used or not */
    VOS_PID    pid;                        /* who allocate the timer */
    VOS_UINT32 name;                       /* timer's name */
    VOS_UINT32 para;                       /* timer's paremate */
    HTIMER    *phTm;                         /* user's pointer which point the real timer room */
    VOS_UINT32 timeOutValueInMilliSeconds; /* timer's length(ms) */
    VOS_UINT32 timeOutValueSlice;          /* timer's length(32k) */
    VOS_UINT32 timeEndSlice;               /* the end slice time of timer */

#if VOS_YES == VOS_TIMER_CHECK
    VOS_UINT32 allocTick; /* CPU tick of block allocation */
    VOS_UINT32 fileId;    /* alloc file ID */
    VOS_UINT32 lineNo;    /* alloc line no. */
#endif
} DRX_TimerControlBlock;

typedef struct BIT64_TimerControl {
    VOS_UINT32 usedFlag;                   /* whether be used or not */
    VOS_PID    pid;                        /* who allocate the timer */
    VOS_UINT32 name;                       /* timer's name */
    VOS_UINT32 para;                       /* timer's paremate */
    HTIMER    *phTm;                         /* user's pointer which point the real timer room */
    VOS_UINT32 timeOutValueInMilliSeconds; /* timer's length(ms) */
    VOS_UINT64 timeOutValueSlice;         /* timer's length(64k) */
    VOS_UINT64 timeEndSlice;              /* the end slice time of timer */

#if VOS_YES == VOS_TIMER_CHECK
    VOS_UINT64 allocTick; /* CPU tick of block allocation */
    VOS_UINT32 fileId;     /* alloc file ID */
    VOS_UINT32 lineNo;     /* alloc line no. */
#endif
} BIT64_TimerControlBlock;

/* the number of task's control block */
VOS_UINT32 g_rtcTimerCtrlBlkNumber;

/* the number of free task's control block */
VOS_UINT32 g_rtcTimerIdleCtrlBlkNumber;

/* the start address of task's control block */
RTC_TimerControlBlock *g_rtcTimerCtrlBlk = VOS_NULL_PTR;

/* the start address of free task's control block list */
RTC_TimerControlBlock *g_rtcTimerIdleCtrlBlk = VOS_NULL_PTR;

/* the begin address of timer control block */
VOS_VOID *g_rtcTimerCtrlBlkBegin = VOS_NULL_PTR;

/* the end address of timer control block */
VOS_VOID *g_rtcTimerCtrlBlkEnd = VOS_NULL_PTR;

/* the head of the running timer list */
RTC_TimerControlBlock *g_rtcTimerHeadPtr = VOS_NULL_PTR;

/* the task ID of timer's task */
VOS_UINT32 g_rtcTimerTaskId;

/* the Min usage of timer */
VOS_UINT32 g_rtcTimerMinTimerIdUsed;

/* the queue will be given when RTC's interrupt occures */
VOS_UINT32 g_rtcTaskQueueId;

/* record start value */
VOS_UINT32 g_rtcStartValue = ELAPESD_TIME_INVAILD;

#define RTC_TIMER_CTRL_BUF_SIZE (sizeof(RTC_TimerControlBlock) * RTC_MAX_TIMER_NUMBER)

VOS_CHAR g_rtcTimerCtrlBuf[RTC_TIMER_CTRL_BUF_SIZE];

/* 循环记录SOC Timer的启停记录 */
enum {
    RTC_SOC_TIMER_SEND_ERR = 0xfffffffd,
    RTC_SOC_TIMER_EXPIRED  = 0xfffffffe,
    RTC_SOC_TIMER_STOPED   = 0xffffffff
};

VOS_UINT32 g_rtcSocTimerDebugInfoSuffix = 0;

RTC_SocTimerDebugInfo g_rtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER];

/* 记录 RTC timer 可维可测信息 */
VOS_TimerSocTimerInfo g_rtcSocTimerInfo;

/* the array of DRX timer's control block */
DRX_TimerControlBlock g_drxTimerCtrlBlk[DRX_TIMER_MAX_NUMBER];

/* the semaphore will be given when DRX's interrupt occures */
VOS_SEM g_drxSem;

/* the task ID of DRX timer's task */
VOS_UINT32 g_drxTimerTaskId;

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)

VOS_UINT32 g_bit64TimerTaskId;

BIT64_TimerControlBlock g_bit64TimerCtrlBlk[BIT64_TIMER_MAX_NUMBER];

VOS_SEM g_bit64TimerSem;

VOS_UINT32 g_bit64NextHardTimerSlice = 0;

#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

/* 自旋锁，用来作DRX Timer的临界资源保护 */
VOS_SPINLOCK g_drxTimerSpinLock;

/* flight mode max mode number */
#define DRX_TIMER_WAKE_SRC_MODE_NUM (8)

VOS_UINT32 g_flightModeVoteMap = 0; /* DRX TIMER在飞行模式投票 */

VOS_SPINLOCK g_flightModeVoteMapSpinLock; /* 用于投票的自旋锁 */

#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#define VOS_RTC_TIMER_ID (TIMER_ACPU_OSA_ID)
#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
#define VOS_RTC_TIMER_ID (TIMER_CCPU_OSA_ID)
#endif

extern VOS_VOID OM_RecordInfoEnd(VOS_ExcDumpMemNumUint32 number);
extern VOS_VOID OM_RecordInfoStart(VOS_ExcDumpMemNumUint32 number, VOS_UINT32 sendPid, VOS_UINT32 rcvPid,
                                   VOS_UINT32 msgName);
#if (VOS_OSA_CPU == OSA_CPU_CCPU)
extern VOS_VOID PAMOM_DrxTimer_Event(VOS_VOID *data, VOS_UINT32 length);
#endif

/* Just for Sparse checking. */
VOS_VOID                 StartHardTimer(VOS_UINT32 value);
VOS_VOID                 StopHardTimer(VOS_VOID);
VOS_INT32                RTC_DualTimerIsr(VOS_INT para);
RTC_TimerControlBlock *RTC_TimerCtrlBlkGet(VOS_UINT32 fileId, VOS_INT32 lineNo);
VOS_UINT32               RTC_TimerCtrlBlkFree(RTC_TimerControlBlock *ptr, VOS_UINT8 tag);
VOS_UINT32               GetHardTimerElapsedTime(VOS_VOID);
VOS_VOID                 RTC_TimerTaskFunc(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3);
VOS_VOID                 RTC_Add_Timer_To_List(RTC_TimerControlBlock *timer);
VOS_VOID                 RTC_Del_Timer_From_List(RTC_TimerControlBlock *timer);
VOS_VOID                 ShowRtcTimerLog(VOS_VOID);

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
VOS_UINT64 BIT64_MUL_32_DOT_768(VOS_UINT32 value, VOS_UINT32 fileId, VOS_INT32 lineNo);
VOS_UINT64 BIT64_DIV_32_DOT_768(VOS_UINT64 value, VOS_UINT32 fileId, VOS_INT32 lineNo);
VOS_VOID   VOS_Bit64TimerIsr(VOS_VOID);
VOS_VOID   VOS_StartBit64HardTimer(VOS_UINT32 value);
VOS_UINT32 VOS_GetNextBit64Timer(VOS_UINT64 curSlice, VOS_UINT32 *needStartTimer);
VOS_VOID   VOS_Bit64TimerTaskFunc(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3);
#endif

typedef struct  {
    PM_LOG_CosaPamUint32  pamType;
    VOS_UINT32                  pid;
    VOS_UINT32                  timerId;
} RTC_TimerPmlog;

VOS_UINT32 g_timerlpm = VOS_FALSE;

VOS_INT VOS_TimerLpmCb(VOS_INT x)
{
    g_timerlpm = VOS_TRUE;

    return VOS_OK;
}


/*
 * Description: 乘以32.768
 */
VOS_UINT32 RTC_MUL_32_DOT_768(VOS_UINT32 value, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 productHigh;
    VOS_UINT32 productLow;
    VOS_UINT32 quotientHigh;
    VOS_UINT32 quotientLow;
    VOS_UINT32 rtcRemainder;
    VOS_UINT32 returnValue;

    returnValue = VOS_64Multi32(0, value, RTC_TIMER_CHECK_LONG_PRECISION, &productHigh, &productLow);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    returnValue = VOS_64Div32(productHigh, productLow, 1000, &quotientHigh, &quotientLow, &rtcRemainder);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    if (VOS_NULL != quotientHigh) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    return quotientLow;
}

/*
 * Description: 除以32.768
 */
VOS_UINT32 RTC_DIV_32_DOT_768(VOS_UINT32 value, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 productHigh;
    VOS_UINT32 productLow;
    VOS_UINT32 quotientHigh;
    VOS_UINT32 quotientLow;
    VOS_UINT32 rtcRemainder;
    VOS_UINT32 returnValue;

    returnValue = VOS_64Multi32(0, value, 1000, &productHigh, &productLow);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    returnValue = VOS_64Div32(productHigh, productLow, RTC_TIMER_CHECK_LONG_PRECISION, &quotientHigh, &quotientLow,
                           &rtcRemainder);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    if (VOS_NULL != quotientHigh) {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    return quotientLow;
}

/*
 * Description: 乘以0.32768
 */
VOS_UINT32 RTC_MUL_DOT_32768(VOS_UINT32 value, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 productHigh;
    VOS_UINT32 productLow;
    VOS_UINT32 quotientHigh;
    VOS_UINT32 quotientLow;
    VOS_UINT32 rtcRemainder;
    VOS_UINT32 returnValue;

    returnValue = VOS_64Multi32(0, value, RTC_TIMER_CHECK_LONG_PRECISION, &productHigh, &productLow);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_DOT_32768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    returnValue = VOS_64Div32(productHigh, productLow, 100000, &quotientHigh, &quotientLow, &rtcRemainder);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_DOT_32768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    if (VOS_NULL != quotientHigh) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_DOT_32768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    return quotientLow;
}


/*
 * Description: record the track of Soc timer
 */
MODULE_EXPORTED VOS_VOID RTC_DebugSocInfo(VOS_UINT32 action, VOS_UINT32 slice)
{
    g_rtcSocTimerDebugInfo[g_rtcSocTimerDebugInfoSuffix].action = action;
    g_rtcSocTimerDebugInfo[g_rtcSocTimerDebugInfoSuffix].slice  = slice;

    g_rtcSocTimerDebugInfo[g_rtcSocTimerDebugInfoSuffix].coreId = 0;

    g_rtcSocTimerDebugInfoSuffix++;

    if (RTC_MAX_TIMER_NUMBER <= g_rtcSocTimerDebugInfoSuffix) {
        g_rtcSocTimerDebugInfoSuffix = 0;
    }
}

/*
 * Description: get the track of Soc timer
 */
VOS_VOID RTC_GetDebugSocInfo(VOS_UINT32 *action, VOS_UINT32 *slice, VOS_UINT32 *coreId)
{
    VOS_UINT32 currentInfoSuffix = g_rtcSocTimerDebugInfoSuffix;

    if (0 == currentInfoSuffix) {
        *action = g_rtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER - 1].action;
        *slice  = g_rtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER - 1].slice;
        *coreId = g_rtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER - 1].coreId;
    } else {
        *action = g_rtcSocTimerDebugInfo[currentInfoSuffix - 1].action;
        *slice  = g_rtcSocTimerDebugInfo[currentInfoSuffix - 1].slice;
        *coreId = g_rtcSocTimerDebugInfo[currentInfoSuffix - 1].coreId;
    }
}

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
/*
 * Description: Record the SOC timer's info.
 */
VOS_VOID RTC_SocTimerMemDump(VOS_VOID)
{
    /*lint -e438 屏蔽pucDumpBuffer没有使用的错误*/
    VOS_UINT8 *dumpBuffer = VOS_NULL_PTR;
    VOS_UINT32 bufferSize;

    (VOS_VOID)VOS_TaskLock();

    dumpBuffer = (VOS_UINT8 *)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == dumpBuffer) {
        (VOS_VOID)VOS_TaskUnlock();
        return;
    }

    bufferSize = VOS_DUMP_MEM_TOTAL_SIZE;

    if (memset_s((VOS_VOID *)dumpBuffer, bufferSize, 0, VOS_DUMP_MEM_TOTAL_SIZE) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, VOS_DUMP_MEM_TOTAL_SIZE, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__),
                             0, 0);
    }

    if (memcpy_s((VOS_VOID *)dumpBuffer, bufferSize, (VOS_VOID *)g_rtcTimerCtrlBuf, RTC_TIMER_CTRL_BUF_SIZE) !=
        EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, RTC_TIMER_CTRL_BUF_SIZE, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__),
                             0, 0);
    }

    dumpBuffer += RTC_TIMER_CTRL_BUF_SIZE;
    bufferSize -= RTC_TIMER_CTRL_BUF_SIZE;

    if (memcpy_s((VOS_VOID *)dumpBuffer, bufferSize, (VOS_VOID *)g_rtcSocTimerDebugInfo,
                 sizeof(g_rtcSocTimerDebugInfo)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, (VOS_INT)sizeof(g_rtcSocTimerDebugInfo),
                             (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    dumpBuffer += sizeof(g_rtcSocTimerDebugInfo);
    bufferSize -= sizeof(g_rtcSocTimerDebugInfo);

    if (memcpy_s((VOS_VOID *)dumpBuffer, bufferSize, (VOS_VOID *)(&g_rtcSocTimerInfo),
                 sizeof(g_rtcSocTimerInfo)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, (VOS_INT)sizeof(g_rtcSocTimerInfo),
                             (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    (VOS_VOID)VOS_TaskUnlock();
    return;
    /*lint +e438 */
}
#endif

/*
 * Description: handle timer's list
 */
VOS_VOID RTC_DualTimerIsrEntry(VOS_UINT32 elapsedCycles)
{
    RTC_TimerControlBlock *headPtr = VOS_NULL_PTR;
    /* the timer control which expire */
    RTC_TimerControlBlock *rtcTimerCtrlBlkCurrent = VOS_NULL_PTR;
    /* the timer head control which expire */
    RTC_TimerControlBlock *rtcTimerCtrlBlkexpired     = VOS_NULL_PTR;
    RTC_TimerControlBlock *rtcTimerCtrlBlkexpiredTail = VOS_NULL_PTR;
    VOS_UINT32               tempCount;

    if (VOS_NULL_PTR == g_rtcTimerHeadPtr) {
        return;
    }

    headPtr = g_rtcTimerHeadPtr;

    /* sub timer value */
    headPtr->timeOutValueInCycle -= elapsedCycles;

    tempCount = 0;

    /* check the left timer */
    while ((VOS_NULL_PTR != headPtr) && (0 == headPtr->timeOutValueInCycle)) {
        tempCount++;
        if (g_rtcTimerCtrlBlkNumber < tempCount) {
            return;
        }

        rtcTimerCtrlBlkCurrent = headPtr;

        *(headPtr->phTm) = VOS_NULL_PTR;

        headPtr = rtcTimerCtrlBlkCurrent->next;

        rtcTimerCtrlBlkCurrent->next = VOS_NULL_PTR;
        if (VOS_NULL_PTR == rtcTimerCtrlBlkexpired) {
            rtcTimerCtrlBlkexpired     = rtcTimerCtrlBlkCurrent;
            rtcTimerCtrlBlkexpiredTail = rtcTimerCtrlBlkCurrent;
        } else {
            if (rtcTimerCtrlBlkexpiredTail != VOS_NULL_PTR) {
                rtcTimerCtrlBlkexpiredTail->next = rtcTimerCtrlBlkCurrent;
            }
            rtcTimerCtrlBlkexpiredTail = rtcTimerCtrlBlkCurrent;
        }

        g_rtcTimerHeadPtr = headPtr;
    }

    if (VOS_NULL_PTR != g_rtcTimerHeadPtr) {
        g_rtcTimerHeadPtr->previous = VOS_NULL_PTR;

        /* 上面已经把为0的都过滤了，这里不会再有为0的 */
        if (0 == g_rtcTimerHeadPtr->timeOutValueInCycle) {
            g_rtcTimerHeadPtr->timeOutValueInCycle += 1;
        }

        StartHardTimer(g_rtcTimerHeadPtr->timeOutValueInCycle);
    }

    if (VOS_NULL_PTR != rtcTimerCtrlBlkexpired) {
        if (VOS_OK != VOS_FixedQueueWriteDirect(g_rtcTaskQueueId, (VOS_VOID *)rtcTimerCtrlBlkexpired,
                                                VOS_NORMAL_PRIORITY_MSG)) {
            g_rtcSocTimerInfo.expiredSendErrCount++;
            g_rtcSocTimerInfo.expiredSendErrSlice = VOS_GetSlice();

            RTC_DebugSocInfo((VOS_UINT32)RTC_SOC_TIMER_SEND_ERR, VOS_GetSlice());
        }
    }

    return;
}

/*
 * Description: ISR of DualTimer
 */
VOS_INT32 RTC_DualTimerIsr(VOS_INT para)
{
    VOS_UINT32 currentSlice;
    VOS_ULONG  lockLevel;
    VOS_UINT32 elapsedCycles;

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    g_rtcSocTimerInfo.expireCount++;

    currentSlice = VOS_GetSlice();
    if ((currentSlice - g_rtcSocTimerInfo.startSlice) < g_rtcStartValue) {
        g_rtcSocTimerInfo.expiredShortErrCount++;
        g_rtcSocTimerInfo.expiredShortErrSlice = VOS_GetSlice();
    }

    if ((currentSlice - g_rtcSocTimerInfo.startSlice) > (g_rtcStartValue + RTC_SOC_TIMER_CHECK_PRECISION)) {
        g_rtcSocTimerInfo.expiredLongErrCount++;
        g_rtcSocTimerInfo.expiredLongErrSlice = VOS_GetSlice();
    }

    elapsedCycles = GetHardTimerElapsedTime();

    if (VOS_OK != mdrv_timer_stop((unsigned int)para)) {
        g_rtcSocTimerInfo.stopErrCount++;
    }

    RTC_DebugSocInfo((VOS_UINT32)RTC_SOC_TIMER_EXPIRED, currentSlice);

    RTC_DualTimerIsrEntry(elapsedCycles);

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    return 0;
}

/*
 * Description: stop hard timer
 */
VOS_VOID StopHardTimer(VOS_VOID)
{
    g_rtcSocTimerInfo.stopCount++;

    if (VOS_OK != mdrv_timer_stop(VOS_RTC_TIMER_ID)) {
        g_rtcSocTimerInfo.stopErrCount++;
    }

    g_rtcStartValue = ELAPESD_TIME_INVAILD;

    RTC_DebugSocInfo((VOS_UINT32)RTC_SOC_TIMER_STOPED, VOS_GetSlice());

    return;
}

/*
 * Description: start hard timer
 */
VOS_VOID StartHardTimer(VOS_UINT32 value)
{
    g_rtcSocTimerInfo.startCount++;

    StopHardTimer();

    g_rtcSocTimerInfo.startSlice = VOS_GetSlice();

    g_rtcStartValue = value;

    if (VOS_OK != mdrv_timer_start(VOS_RTC_TIMER_ID, (timer_handle_cb)RTC_DualTimerIsr, VOS_RTC_TIMER_ID, value,
                                   TIMER_ONCE_COUNT, TIMER_UNIT_NONE)) {
        g_rtcSocTimerInfo.startErrCount++;
    }

    RTC_DebugSocInfo(value, g_rtcSocTimerInfo.startSlice);

    return;
}

/*
 * Description: get the elapsed time from hard timer
 */
VOS_UINT32 GetHardTimerElapsedTime(VOS_VOID)
{
    VOS_UINT32 tempValue = 0;

    if (ELAPESD_TIME_INVAILD == g_rtcStartValue) {
        return 0;
    }

    if (VOS_OK != mdrv_timer_get_rest_time(VOS_RTC_TIMER_ID, TIMER_UNIT_NONE, (VOS_UINT *)&tempValue)) {
        g_rtcSocTimerInfo.elapsedErrCount++;
    }

    if (g_rtcStartValue < tempValue) {
        g_rtcSocTimerInfo.elapsedContentErrCount++;
        g_rtcSocTimerInfo.elapsedContentErrSlice = VOS_GetSlice();

        return g_rtcStartValue;
    }

    return g_rtcStartValue - tempValue;
}

/*
 * Description: Init timer's control block
 */
VOS_UINT32 RTC_TimerCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32 i;

    g_rtcTimerCtrlBlkNumber     = RTC_MAX_TIMER_NUMBER;
    g_rtcTimerIdleCtrlBlkNumber = RTC_MAX_TIMER_NUMBER;

    g_rtcTimerCtrlBlk = (RTC_TimerControlBlock *)g_rtcTimerCtrlBuf;

    g_rtcTimerIdleCtrlBlk  = g_rtcTimerCtrlBlk;
    g_rtcTimerCtrlBlkBegin = (VOS_VOID *)g_rtcTimerCtrlBlk;
    g_rtcTimerCtrlBlkEnd =
        (VOS_VOID *)((VOS_UINT_PTR)(g_rtcTimerCtrlBlk) + g_rtcTimerCtrlBlkNumber * sizeof(RTC_TimerControlBlock));

    for (i = 0; i < g_rtcTimerCtrlBlkNumber - 1; i++) {
        g_rtcTimerCtrlBlk[i].state           = RTC_TIMER_CTRL_BLK_STOP;
        g_rtcTimerCtrlBlk[i].usedFlag      = VOS_NOT_USED;
        g_rtcTimerCtrlBlk[i].timerId         = i;
        g_rtcTimerCtrlBlk[i].backUpTimerId = i;
        g_rtcTimerCtrlBlk[i].phTm            = VOS_NULL_PTR;
        g_rtcTimerCtrlBlk[i].callBackFunc    = VOS_NULL_PTR;
        g_rtcTimerCtrlBlk[i].previous        = VOS_NULL_PTR;
        /*lint -e679*/
        g_rtcTimerCtrlBlk[i].next = &(g_rtcTimerCtrlBlk[i + 1]);
        /*lint +e679*/
    }

    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].state           = RTC_TIMER_CTRL_BLK_STOP;
    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].usedFlag      = VOS_NOT_USED;
    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].timerId         = g_rtcTimerCtrlBlkNumber - 1;
    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].backUpTimerId = g_rtcTimerCtrlBlkNumber - 1;
    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].phTm            = VOS_NULL_PTR;
    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].callBackFunc    = VOS_NULL_PTR;
    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].previous        = VOS_NULL_PTR;
    g_rtcTimerCtrlBlk[g_rtcTimerCtrlBlkNumber - 1].next            = VOS_NULL_PTR;

    g_rtcTimerMinTimerIdUsed = g_rtcTimerCtrlBlkNumber;

    if (memset_s(&g_rtcSocTimerInfo, sizeof(VOS_TimerSocTimerInfo), 0x0,
                 sizeof(VOS_TimerSocTimerInfo)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    if (memset_s((VOS_VOID *)g_rtcSocTimerDebugInfo, sizeof(g_rtcSocTimerDebugInfo), 0x0,
                 sizeof(g_rtcSocTimerDebugInfo)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    /* 1 -> only one queue */
    if (VOS_OK !=
        VOS_FixedQueueCreate(VOS_FID_QUEUE_LENGTH, &g_rtcTaskQueueId, VOS_MSG_Q_FIFO, VOS_FID_MAX_MSG_LENGTH, 1)) {
        VOS_ProtectionReboot(VOS_ERRNO_RELTM_CTRLBLK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }

    mdrv_timer_debug_register(VOS_RTC_TIMER_ID, (timer_handle_cb)VOS_TimerLpmCb, 0);

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if (VOS_OK != VOS_DrxTimerCtrlBlkInit()) {
        VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_RESOURCE_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if (VOS_OK != VOS_Bit64RtcTimerCtrlBlkInit()) {
        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_RESOURCE_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif

    return (VOS_OK);
}

/*
 * Description: allocte a block
 */
RTC_TimerControlBlock *RTC_TimerCtrlBlkGet(VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    RTC_TimerControlBlock *tempTimerCtrlPtr = VOS_NULL_PTR;

    VOS_VOID *dumpBuffer = VOS_NULL_PTR;

    if (0 == g_rtcTimerIdleCtrlBlkNumber) {
        dumpBuffer = (VOS_VOID *)VOS_EXCH_MEM_MALLOC;

        if (VOS_NULL_PTR == dumpBuffer) {
            return ((RTC_TimerControlBlock *)VOS_NULL_PTR);
        }

        if (memset_s(dumpBuffer, VOS_DUMP_MEM_TOTAL_SIZE, 0, VOS_DUMP_MEM_TOTAL_SIZE) != EOK) {
            mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
        }

        /* 防止拷贝内存越界，取最小值 */
        /*lint -e506 */
        if (memcpy_s(dumpBuffer, VOS_DUMP_MEM_TOTAL_SIZE, (VOS_VOID *)g_rtcTimerCtrlBuf,
                     ((VOS_DUMP_MEM_TOTAL_SIZE < RTC_TIMER_CTRL_BUF_SIZE) ? VOS_DUMP_MEM_TOTAL_SIZE
                      : RTC_TIMER_CTRL_BUF_SIZE)) != EOK) {
            mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
        }
        /*lint +e506 */

        return ((RTC_TimerControlBlock *)VOS_NULL_PTR);
    } else {
        g_rtcTimerIdleCtrlBlkNumber--;

        tempTimerCtrlPtr             = g_rtcTimerIdleCtrlBlk;
        tempTimerCtrlPtr->usedFlag = VOS_USED;
        g_rtcTimerIdleCtrlBlk            = g_rtcTimerIdleCtrlBlk->next;
    }

    /* record the usage of timer control block */
    if (g_rtcTimerIdleCtrlBlkNumber < g_rtcTimerMinTimerIdUsed) {
        g_rtcTimerMinTimerIdUsed = g_rtcTimerIdleCtrlBlkNumber;
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
VOS_UINT32 RTC_TimerCtrlBlkFree(RTC_TimerControlBlock *ptr, VOS_UINT8 tag)
{
    if ((VOS_UINT_PTR)ptr < (VOS_UINT_PTR)g_rtcTimerCtrlBlkBegin ||
        (VOS_UINT_PTR)ptr > (VOS_UINT_PTR)g_rtcTimerCtrlBlkEnd) {
        return VOS_ERR;
    }

    if (VOS_NOT_USED == ptr->usedFlag) {
        return VOS_RTC_ERRNO_RELTM_FREE_RECEPTION;
    }

    ptr->usedFlag      = VOS_NOT_USED;
    ptr->reserved[0]     = tag;
    ptr->next            = g_rtcTimerIdleCtrlBlk;
    g_rtcTimerIdleCtrlBlk = ptr;

    g_rtcTimerIdleCtrlBlkNumber++;

#if VOS_YES == VOS_TIMER_CHECK
    ptr->freeTick = VOS_GetSlice();
#endif

    return VOS_OK;
}

/*
 * Description: RTC timer task entry
 */
VOS_VOID RTC_TimerTaskFunc(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3)
{
    RTC_TimerControlBlock *headPtr                = VOS_NULL_PTR;
    RTC_TimerControlBlock *rtcTimerCtrlBlkexpired = VOS_NULL_PTR;
    VOS_ULONG                lockLevel;
    REL_TimerMsgBlock           *expireMsg = VOS_NULL_PTR;
    VOS_UINT32               currentSlice;
    VOS_UINT32               intervalSlice;
    VOS_UINT_PTR             ctrlBlkAddress;
    VOS_UINT_PTR             tempValue;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
    RTC_TimerPmlog pmLog;
#endif

#if (VOS_WIN32 == VOS_OS_VER)
    VOS_UINT32 i;

    for (i = 0; i < 1; i++) {
#else
    for (;;) {
#endif

        if (VOS_ERR == VOS_FixedQueueRead(g_rtcTaskQueueId, 0, &ctrlBlkAddress, VOS_FID_MAX_MSG_LENGTH)) {
            continue;
        }

        currentSlice = VOS_GetSlice();

        rtcTimerCtrlBlkexpired = (RTC_TimerControlBlock *)ctrlBlkAddress;

        while (VOS_NULL_PTR != rtcTimerCtrlBlkexpired) {
            headPtr = rtcTimerCtrlBlkexpired;

            if (VOS_RELTIMER_LOOP == headPtr->mode) {
                if (VOS_NULL_PTR == headPtr->callBackFunc) {
                    (VOS_VOID)V_Start32KRelTimer(headPtr->phTm, headPtr->pid, headPtr->timeOutValueInMilliSeconds,
                                                 headPtr->name, headPtr->para, VOS_RELTIMER_LOOP,
                                                 headPtr->precision,
                                                 headPtr->fileId, (VOS_INT32)headPtr->lineNo);
                } else {
                    (VOS_VOID)V_Start32KCallBackRelTimer(headPtr->phTm, headPtr->pid,
                                                         headPtr->timeOutValueInMilliSeconds, headPtr->name,
                                                         headPtr->para, VOS_RELTIMER_LOOP, headPtr->callBackFunc,
                                                         headPtr->precision,
                                                         headPtr->fileId, (VOS_INT32)headPtr->lineNo);
                }
            }

            tempValue = (VOS_UINT_PTR)(rtcTimerCtrlBlkexpired->callBackFunc);

            /* CallBackFunc需要用32位传入，所以和name互换位置保证数据不丢失 */
            OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_3, (VOS_UINT32)(rtcTimerCtrlBlkexpired->pid),
                               rtcTimerCtrlBlkexpired->name, (VOS_UINT32)tempValue);

            if (VOS_NULL_PTR == rtcTimerCtrlBlkexpired->callBackFunc) {
                /* Alloc expires's Msg */
                expireMsg = VOS_TimerPreAllocMsg(rtcTimerCtrlBlkexpired->pid);
                if (VOS_NULL_PTR != expireMsg) {
                    expireMsg->name = rtcTimerCtrlBlkexpired->name;
                    expireMsg->para = rtcTimerCtrlBlkexpired->para;
                    tempValue            = (VOS_UINT_PTR)(rtcTimerCtrlBlkexpired->timeOutValueInMilliSeconds);
                    expireMsg->hTm    = (HTIMER)tempValue;

#if (VOS_YES == VOS_TIMER_CHECK)
                    tempValue = (VOS_UINT_PTR)rtcTimerCtrlBlkexpired->allocTick;

                    expireMsg->next = (struct REL_TimerMsg *)tempValue;

                    tempValue = (VOS_UINT_PTR)currentSlice;

                    expireMsg->prev = (struct REL_TimerMsg *)tempValue;

                    intervalSlice = (rtcTimerCtrlBlkexpired->backUpTimeOutValueInCycle -
                                       rtcTimerCtrlBlkexpired->precisionInCycle);

                    intervalSlice = ((intervalSlice > RTC_TIMER_CHECK_PRECISION)
                                       ? (intervalSlice - RTC_TIMER_CHECK_PRECISION)
                                       : intervalSlice);

                    if ((currentSlice - rtcTimerCtrlBlkexpired->allocTick) < intervalSlice) {
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
#if (FEATURE_ON == FEATURE_RTC_TIMER_DBG)
                        RTC_SocTimerMemDump();

                        VOS_ProtectionReboot(RTC_TIMER_EXPIRED_TOO_SHORT, 0, (VOS_INT)currentSlice,
                                             (VOS_CHAR *)rtcTimerCtrlBlkexpired, sizeof(RTC_TimerControlBlock));
#endif
#endif
                    }

                    intervalSlice = (rtcTimerCtrlBlkexpired->backUpTimeOutValueInCycle +
                                       rtcTimerCtrlBlkexpired->precisionInCycle);

                    if ((currentSlice - rtcTimerCtrlBlkexpired->allocTick) >
                        (intervalSlice + RTC_TIMER_CHECK_LONG_PRECISION)) {
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
#if (FEATURE_ON == FEATURE_RTC_TIMER_DBG)

#endif
#endif
                    }
#endif
                    (VOS_VOID)VOS_SendMsg(DOPRA_PID_TIMER, expireMsg);
                }
            } else {
                rtcTimerCtrlBlkexpired->callBackFunc(rtcTimerCtrlBlkexpired->para, rtcTimerCtrlBlkexpired->name);
            }

            OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_3);

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
            if (VOS_TRUE == g_timerlpm) {
                g_timerlpm = VOS_FALSE;

                pmLog.pamType = PM_LOG_COSA_PAM_TIMER;
                pmLog.pid     = rtcTimerCtrlBlkexpired->pid;
                pmLog.timerId = rtcTimerCtrlBlkexpired->name;

                (VOS_VOID)mdrv_pm_log(PM_MOD_CP_OSA, sizeof(RTC_TimerPmlog), &pmLog);
            }
#else
            if (VOS_TRUE == g_timerlpm) {
                g_timerlpm = VOS_FALSE;

                mdrv_err("<RTC_TimerTaskFunc> rtc_timer: allocpid=%d, timername = 0x%x.\n",
                         rtcTimerCtrlBlkexpired->pid, rtcTimerCtrlBlkexpired->name);
            }

#endif

            rtcTimerCtrlBlkexpired = rtcTimerCtrlBlkexpired->next;

            VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

            (VOS_VOID)RTC_TimerCtrlBlkFree(headPtr, THE_FIRST_RTC_TIMER_TAG);

            VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);
        }
    }
}

/*
 * Description: create RTC timer task
 */
VOS_UINT32 RTC_TimerTaskCreat(VOS_VOID)
{
    VOS_UINT32 timerArguments[4] = { 0, 0, 0, 0 };

    if (VOS_OK != VOS_CreateTask("RTC_TIMER", &g_rtcTimerTaskId, RTC_TimerTaskFunc, COMM_RTC_TIMER_TASK_PRIO,
                                 RTC_TIMER_TASK_STACK_SIZE, timerArguments)) {
        VOS_ProtectionReboot(VOS_ERRNO_RELTM_TASK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if (VOS_OK != VOS_DrxTimerTaskCreat()) {
        VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_TASK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if (VOS_OK != VOS_Bit64TimerTaskCreat()) {
        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_TASK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif

    return VOS_OK;
}

/*
 * Description: add a timer to list
 */
VOS_VOID RTC_Add_Timer_To_List(RTC_TimerControlBlock *timer)
{
    RTC_TimerControlBlock *tempPtr      = VOS_NULL_PTR;
    RTC_TimerControlBlock *preTempPtr  = VOS_NULL_PTR;
    VOS_UINT32               elapsedCycles = 0;

    if (VOS_NULL_PTR == g_rtcTimerHeadPtr) {
        g_rtcTimerHeadPtr = timer;

        timer->reserved[1] = VOS_ADD_TIMER_TAG_1;
    } else {
        elapsedCycles = GetHardTimerElapsedTime();

        timer->timeOutValueInCycle += elapsedCycles;

        /*  find the location to insert */
        tempPtr = preTempPtr = g_rtcTimerHeadPtr;

        while (tempPtr != VOS_NULL_PTR) {
            if (timer->timeOutValueInCycle >= tempPtr->timeOutValueInCycle) {
                timer->timeOutValueInCycle -= tempPtr->timeOutValueInCycle;

                if (timer->timeOutValueInCycle <= timer->precisionInCycle) {
                    /* forward adjust; do nothindg when TimeOutValueInCycle == 0 */
                    timer->timeOutValueInCycle = 0;
                    timer->reserved[1]         = VOS_ADD_TIMER_TAG_2;

                    preTempPtr = tempPtr;
                    tempPtr     = tempPtr->next;

                    while ((tempPtr != VOS_NULL_PTR) &&
                           (timer->timeOutValueInCycle == tempPtr->timeOutValueInCycle)) {
                        preTempPtr = tempPtr;
                        tempPtr     = tempPtr->next;
                    } /* make sure the order of expiry */

                    break;
                }

                preTempPtr = tempPtr;
                tempPtr     = tempPtr->next;

                timer->reserved[1] = VOS_ADD_TIMER_TAG_3;
            } else {
                if (tempPtr->timeOutValueInCycle - timer->timeOutValueInCycle <= timer->precisionInCycle) {
                    /* backward adjust */
                    timer->timeOutValueInCycle = 0;
                    timer->reserved[1]         = VOS_ADD_TIMER_TAG_4;

                    preTempPtr = tempPtr;
                    tempPtr     = tempPtr->next;

                    while ((tempPtr != VOS_NULL_PTR) &&
                           (timer->timeOutValueInCycle == tempPtr->timeOutValueInCycle)) {
                        preTempPtr = tempPtr;
                        tempPtr     = tempPtr->next;
                    } /* make sure the order of expiry */

                    break;
                }

                timer->reserved[1] = VOS_ADD_TIMER_TAG_5;

                /* can't adjust */
                break;
            }
        }

        /* insert timer < head timer */
        if (tempPtr == g_rtcTimerHeadPtr) {
            timer->next        = g_rtcTimerHeadPtr;
            g_rtcTimerHeadPtr = timer;
        } else {
            timer->next        = tempPtr;
            preTempPtr->next = timer;
            timer->previous    = preTempPtr;
        }

        if (tempPtr != VOS_NULL_PTR) {
            tempPtr->timeOutValueInCycle = tempPtr->timeOutValueInCycle - timer->timeOutValueInCycle;
            tempPtr->previous            = timer;
        }
    }

    /* restart RTC timer */
    if (g_rtcTimerHeadPtr == timer) {
        /* judge timer value when the new timer at head */
        timer->timeOutValueInCycle -= elapsedCycles;

        if (0 == timer->timeOutValueInCycle) {
            timer->timeOutValueInCycle += 1;
        }

        StartHardTimer(timer->timeOutValueInCycle);
    }
}

/*
 * Description: del a timer from list
 */
VOS_VOID RTC_Del_Timer_From_List(RTC_TimerControlBlock *timer)
{
    VOS_BOOL bIsHead = VOS_FALSE;

    /* deletet this timer from list */
    if (timer == g_rtcTimerHeadPtr) {
        bIsHead = VOS_TRUE;

        g_rtcTimerHeadPtr = timer->next;
        if (VOS_NULL_PTR != g_rtcTimerHeadPtr) {
            g_rtcTimerHeadPtr->previous = VOS_NULL_PTR;
        }
    } else {
        (timer->previous)->next = timer->next;
        if (VOS_NULL_PTR != timer->next) {
            (timer->next)->previous = timer->previous;
        }
    }

    /* adjust the time_val after this timer */
    if (timer->next != NULL) {
        timer->next->timeOutValueInCycle += timer->timeOutValueInCycle;

        if (VOS_TRUE == bIsHead) {
            timer->next->timeOutValueInCycle -= GetHardTimerElapsedTime();

            if (0 == timer->next->timeOutValueInCycle) {
                timer->next->timeOutValueInCycle += 1;
            }

            StartHardTimer(timer->next->timeOutValueInCycle);
        }
    }

    /* Stop timer3 if no timer */
    if (VOS_NULL_PTR == g_rtcTimerHeadPtr) {
        StopHardTimer();
    }
}

/*
 * Description: stop a 32K relative timer which was previously started.
 */
VOS_UINT32 R_Stop32KTimer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo, VOS_TimerOmEvent *event)
{
    VOS_UINT32               timerId = 0;
    RTC_TimerControlBlock *timer   = VOS_NULL_PTR;

    if (VOS_NULL_PTR == *phTm) {
        return VOS_OK;
    }

    if (VOS_OK != RTC_CheckTimer(phTm, &timerId, fileId, lineNo)) {
        return (VOS_RTC_ERRNO_RELTM_STOP_TIMERINVALID);
    }

    timer = &g_rtcTimerCtrlBlk[timerId];

    /* del the timer from the running list */
    RTC_Del_Timer_From_List(timer);

    *(timer->phTm) = VOS_NULL_PTR;

    /* OM */
    if (VOS_NULL_PTR != event) {
        event->mode      = timer->mode;
        event->pid         = timer->pid;
        event->length    = timer->timeOutValueInMilliSeconds;
        event->name      = timer->name;
        event->param     = timer->para;
        event->precision = (VOS_TimerPrecisionUint32 )timer->precision;
    }

    return RTC_TimerCtrlBlkFree(timer, THE_SECOND_RTC_TIMER_TAG);
}

/*
 * Description: get left time
 */
VOS_UINT32 R_Get32KRelTmRemainTime(HTIMER *phTm, VOS_UINT32 *vosTime, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32               timerId      = 0;
    VOS_UINT32               remainValue = 0;
    RTC_TimerControlBlock *headPtr     = VOS_NULL_PTR;
    RTC_TimerControlBlock *tempPtr     = VOS_NULL_PTR;
    VOS_UINT32               tempValue;

    if (VOS_NULL_PTR == *phTm) {
        return (VOS_RTC_ERRNO_RELTM_STOP_INPUTISNULL);
    }

    if (VOS_OK != RTC_CheckTimer(phTm, &timerId, fileId, lineNo)) {
        return (VOS_RTC_ERRNO_RELTM_STOP_TIMERINVALID);
    }

    headPtr = g_rtcTimerHeadPtr;

    while ((VOS_NULL_PTR != headPtr) && (headPtr->timerId != timerId)) {
        remainValue += (headPtr->timeOutValueInCycle);

        tempPtr = headPtr;
        headPtr = tempPtr->next;
    }

    if ((VOS_NULL_PTR == headPtr) || (headPtr->timerId != timerId)) {
        return VOS_ERR;
    } else {
        remainValue += (headPtr->timeOutValueInCycle);

        tempValue = GetHardTimerElapsedTime();

        *vosTime = (VOS_UINT32)RTC_DIV_32_DOT_768((remainValue - tempValue), fileId, lineNo);

        return (VOS_OK);
    }
}

/*
 * 功能描述: 检查TIMER是否正确
 */
VOS_UINT32 RTC_CheckTimer(HTIMER *phTm, VOS_UINT32 *timerID, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    RTC_TimerControlBlock *timer = VOS_NULL_PTR;

    if (((VOS_UINT_PTR)*phTm >= (VOS_UINT_PTR)g_rtcTimerCtrlBlkBegin) &&
        ((VOS_UINT_PTR)*phTm < (VOS_UINT_PTR)g_rtcTimerCtrlBlkEnd)) {
        timer = (RTC_TimerControlBlock *)(*phTm);

        if (phTm == timer->phTm) {
            *timerID = timer->backUpTimerId;

            if (timer->backUpTimerId != timer->timerId) {
                timer->timerId = timer->backUpTimerId;
            }

            return VOS_OK;
        }

        VOS_SIMPLE_FATAL_ERROR(RTC_CHECK_TIMER_ID);
    } else {
        VOS_ProtectionReboot(RTC_CHECK_TIMER_RANG, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)phTm, sizeof(VOS_CHAR *));
    }

    return VOS_ERR;
}

/*
 * Description: allocate and start a relative timer using callback function.
 */
VOS_UINT32 V_Start32KCallBackRelTimer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name,
                                      VOS_UINT32 param, VOS_UINT8 mode, REL_TimerFunc timeOutRoutine,
                                      VOS_UINT32 precision, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    RTC_TimerControlBlock *timer   = VOS_NULL_PTR;
    VOS_UINT32               timerId = 0;
    VOS_ULONG                lockLevel;

    /* stop the timer if exists */
    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    if (VOS_NULL_PTR != *phTm) {
        if (VOS_OK == RTC_CheckTimer(phTm, &timerId, fileId, lineNo)) {
            if (VOS_OK != R_Stop32KTimer(phTm, fileId, lineNo, VOS_NULL_PTR)) {
                VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

                VOS_SIMPLE_FATAL_ERROR(START_32K_CALLBACK_RELTIMER_FAIL_TO_STOP);

                return VOS_ERR;
            }
        }
    }

    timer = RTC_TimerCtrlBlkGet(fileId, lineNo);
    if (timer == VOS_NULL_PTR) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(START_32K_CALLBACK_RELTIMER_FAIL_TO_ALLOCATE, (VOS_INT)fileId, (VOS_INT)lineNo,
                             (VOS_CHAR *)(&g_rtcSocTimerInfo), sizeof(VOS_TimerSocTimerInfo));

        return (VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);
    }

    timer->pid                        = pid;
    timer->name                       = name;
    timer->para                       = param;
    timer->mode                       = mode;
    timer->phTm                       = phTm;
    timer->timeOutValueInMilliSeconds = length;
    timer->precision                = precision;
    timer->timeOutValueInCycle = (VOS_UINT32)RTC_MUL_32_DOT_768(length, fileId, lineNo);
    timer->precisionInCycle  = (VOS_UINT32)RTC_MUL_DOT_32768((length * precision), fileId, lineNo);
    timer->backUpTimeOutValueInCycle = timer->timeOutValueInCycle;
    timer->callBackFunc                = timeOutRoutine;
    *phTm                              = (HTIMER)(&(timer->timerId));

    RTC_Add_Timer_To_List(timer);

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    return (VOS_OK);
}

/*
 * Description: allocate and start a relative timer using callback function.
 */
VOS_UINT32 V_Start32KRelTimer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name, VOS_UINT32 param,
                              VOS_UINT8 mode, VOS_UINT32 precision, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    RTC_TimerControlBlock *timer   = VOS_NULL_PTR;
    VOS_UINT32               timerId = 0;
    VOS_ULONG                lockLevel;

    /* stop the timer if exists */
    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    if (VOS_NULL_PTR != *phTm) {
        if (VOS_OK == RTC_CheckTimer(phTm, &timerId, fileId, lineNo)) {
            if (VOS_OK != R_Stop32KTimer(phTm, fileId, lineNo, VOS_NULL_PTR)) {
                VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

                VOS_SIMPLE_FATAL_ERROR(START_32K_RELTIMER_FAIL_TO_STOP);

                return VOS_ERR;
            }
        }
    }

    timer = RTC_TimerCtrlBlkGet(fileId, lineNo);
    if (VOS_NULL_PTR == timer) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(START_32K_CALLBACK_RELTIMER_FAIL_TO_ALLOCATE, (VOS_INT)fileId, (VOS_INT)lineNo,
                             (VOS_CHAR *)(&g_rtcSocTimerInfo), sizeof(VOS_TimerSocTimerInfo));

        return (VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);
    }

    timer->pid                        = pid;
    timer->name                       = name;
    timer->para                       = param;
    timer->mode                       = mode;
    timer->phTm                       = phTm;
    timer->timeOutValueInMilliSeconds = length;
    timer->precision                = precision;
    timer->timeOutValueInCycle = (VOS_UINT32)RTC_MUL_32_DOT_768(length, fileId, lineNo);
    timer->precisionInCycle  = (VOS_UINT32)RTC_MUL_DOT_32768((length * precision), fileId, lineNo);
    timer->backUpTimeOutValueInCycle = timer->timeOutValueInCycle;
    timer->callBackFunc                = VOS_NULL_PTR;

    *phTm = (HTIMER)(&(timer->timerId));

    /* add the timer to the running list */
    RTC_Add_Timer_To_List(timer);

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    return (VOS_OK);
}

/*
 * Description: Restart a relative timer which was previously started
 */
VOS_UINT32 R_Restart32KRelTimer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32               timerId = 0;
    RTC_TimerControlBlock *timer   = VOS_NULL_PTR;

    if (VOS_NULL_PTR == *phTm) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);

        VOS_SIMPLE_FATAL_ERROR(RESTART_32K_RELTIMER_NULL);

        return (VOS_ERRNO_RELTM_RESTART_TIMERINVALID);
    }

    if (VOS_OK != RTC_CheckTimer(phTm, &timerId, fileId, lineNo)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);

        VOS_SIMPLE_FATAL_ERROR(RESTART_32K_RELTIMER_FAIL_TO_CHECK);

        return (VOS_ERRNO_RELTM_RESTART_TIMERINVALID);
    }

    timer = &g_rtcTimerCtrlBlk[timerId];

    /* Del the old timer but not free timer control block */
    RTC_Del_Timer_From_List(timer);

    /* reset timer value */
    timer->timeOutValueInCycle = (VOS_UINT32)RTC_MUL_32_DOT_768(timer->timeOutValueInMilliSeconds, fileId, lineNo);
    timer->backUpTimeOutValueInCycle = timer->timeOutValueInCycle;

    timer->next     = VOS_NULL_PTR;
    timer->previous = VOS_NULL_PTR;

    /* add the new timer to list */
    RTC_Add_Timer_To_List(timer);

    return VOS_OK;
}

/*
 * Description: a APP RTC timer is running or not
 */
VOS_UINT32 RTC_timer_running(VOS_VOID)
{
    RTC_TimerControlBlock *headPtr = VOS_NULL_PTR;
    VOS_ULONG                lockLevel;

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    headPtr = g_rtcTimerHeadPtr;

    if (headPtr != VOS_NULL_PTR) {
        while (VOS_NULL_PTR != headPtr) {
            if (APP_PID != headPtr->pid) {
                headPtr = headPtr->next;
            } else {
                VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

                return VOS_TRUE;
            }
        }

        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return VOS_FALSE;
    } else {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        return VOS_FALSE;
    }
}

/*
 * Description: get left time of the first timer.Unit is 30us
 */
MODULE_EXPORTED VOS_UINT32 VOS_GetSlice(VOS_VOID)
{
#if (VOS_WIN32 != VOS_OS_VER)
    return mdrv_timer_get_normal_timestamp();
#else
    return VOS_GetTick();
#endif
}

/*
 * 功能描述: 获取64位slice
 */
MODULE_EXPORTED VOS_UINT64 VOS_Get64BitSlice(VOS_VOID)
{
#if (VOS_WIN32 != VOS_OS_VER)
    VOS_UINT32 highBitValue = 0;
    VOS_UINT32 lowBitValue  = 0;
    VOS_UINT64 slice;
    VOS_INT    result;

    result = mdrv_timer_get_accuracy_timestamp(&highBitValue, &lowBitValue);
    if (VOS_OK != result) {
        VOS_ProtectionReboot(VOS_GET_64BIT_SLICE_ERROR, (VOS_INT)highBitValue, (VOS_INT)lowBitValue,
                             (VOS_CHAR *)&result, sizeof(VOS_INT));
    }

    slice = (((VOS_UINT64)highBitValue << 32) & 0xffffffff00000000) |
               ((VOS_UINT64)lowBitValue & 0x00000000ffffffff);

    return slice;
#else
    return VOS_GetTick();
#endif
}

/*
 * Function: VOS_GetSliceUnit
 */
MODULE_EXPORTED VOS_UINT32 VOS_GetSliceUnit(VOS_VOID)
{
    return RTC_TIMER_CHECK_LONG_PRECISION;
}

/*
 * Description: print the usage info of timer
 */
VOS_BOOL RTC_CalcTimerInfo(VOS_VOID)
{
    if (RTC_UPPER_TIMER_NUMBER > g_rtcTimerMinTimerIdUsed) {
        g_rtcTimerMinTimerIdUsed = g_rtcTimerCtrlBlkNumber;
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
/*
 * 功能描述: 检测输入的timer句柄合法性
 */
VOS_UINT32 VOS_DrxCheckTimer(const HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 i;

    for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++) {
        if (*phTm == (HTIMER)(&g_drxTimerCtrlBlk[i])) {
            return VOS_OK;
        }
    }

    VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_ERROR_TIMERHANDLE, (VOS_INT)fileId, (VOS_INT)lineNo, VOS_NULL_PTR,
                         VOS_NULL);

    return VOS_ERR;
}

/*
 * Description: ISR of DRX Timer.
 */
VOS_VOID VOS_DrxTimerIsr(VOS_VOID)
{
    g_rtcSocTimerInfo.stopCount++;

    if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_DRX_TIMER_ID)) {
        g_rtcSocTimerInfo.stopErrCount++;
    }

    (VOS_VOID)VOS_SmV(g_drxSem);

    return;
}

/*
 * Description: Start SOC hard timer by DRV interface.
 */
VOS_VOID VOS_StartDrxHardTimer(VOS_UINT32 value)
{
    g_rtcSocTimerInfo.stopCount++;
    if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_DRX_TIMER_ID)) {
        g_rtcSocTimerInfo.stopErrCount++;
    }

    g_rtcSocTimerInfo.startCount++;
    if (VOS_OK != mdrv_timer_start(TIMER_CCPU_DRX_TIMER_ID, (timer_handle_cb)VOS_DrxTimerIsr, TIMER_CCPU_DRX_TIMER_ID,
                                   value, TIMER_ONCE_COUNT, TIMER_UNIT_NONE)) {
        g_rtcSocTimerInfo.startErrCount++;
    }

    return;
}

/*
 * Description: Init timer's control block
 */
VOS_UINT32 VOS_DrxTimerCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32 i;

    for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++) {
        g_drxTimerCtrlBlk[i].usedFlag = DRX_TIMER_NOT_USED_FLAG;
    }

    if (VOS_OK != VOS_SmCreate("DRX", 0, VOS_SEMA4_FIFO, &g_drxSem)) {
        mdrv_err("<VOS_DrxTimerCtrlBlkInit> create semaphore error.\n");

        return VOS_ERR;
    }

    return VOS_OK;
}

/*
 * Description: Get the shortest timer near now.
 */
VOS_UINT32 VOS_GetNextDrxTimer(VOS_UINT32 curSlice)
{
    VOS_UINT32 i;
    VOS_UINT32 interval;
    VOS_UINT32 minValue;

    minValue = VOS_NULL_DWORD;

    for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++) {
        if (DRX_TIMER_USED_FLAG == g_drxTimerCtrlBlk[i].usedFlag) {
            interval = g_drxTimerCtrlBlk[i].timeEndSlice - curSlice;

            /* The interval must be smaller than timer's length. */
            if (g_drxTimerCtrlBlk[i].timeOutValueSlice >= interval) {
                if (minValue > interval) {
                    minValue = interval;
                }
            }
        }
    }

    if (0 == minValue) {
        minValue += 1;
    }

    return minValue;
}

/*
 * Description: DRX timer task entry
 */
VOS_VOID VOS_DrxTimerTaskFunc(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3)
{
    VOS_UINT32     lockLevel;
    VOS_UINT32     curSlice;
    VOS_UINT32     i;
    VOS_UINT32     nextTimer;
    REL_TimerMsgBlock *expireMsg = VOS_NULL_PTR;

#if (VOS_WIN32 == VOS_OS_VER)
    VOS_UINT32 j;

    for (j = 0; j < 1; j++) {
#else
    for (;;) {
#endif
        /* SemTake SEM when release */
        (VOS_VOID)VOS_SmP(g_drxSem, 0);

        VOS_SpinLockIntLock(&g_drxTimerSpinLock, lockLevel);

        curSlice = VOS_GetSlice();

        for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++) {
            if (DRX_TIMER_USED_FLAG == g_drxTimerCtrlBlk[i].usedFlag) {
                /* Check timer is timeout. */
                if (DRX_TIMER_TIMEOUT_INTERVAL >= (curSlice - g_drxTimerCtrlBlk[i].timeEndSlice)) {
                    /* timer is timeout then notify user by sending msg */
                    expireMsg = VOS_TimerPreAllocMsg(g_drxTimerCtrlBlk[i].pid);
                    if (VOS_NULL_PTR != expireMsg) {
                        expireMsg->name = g_drxTimerCtrlBlk[i].name;
                        expireMsg->para = g_drxTimerCtrlBlk[i].para;

#if (VOS_YES == VOS_TIMER_CHECK)
                        expireMsg->next =
                            (struct REL_TimerMsg *)((VOS_UINT_PTR)(g_drxTimerCtrlBlk[i].allocTick));
                        expireMsg->prev = (struct REL_TimerMsg *)((VOS_UINT_PTR)curSlice);
#endif
                        (VOS_VOID)VOS_SendMsg(DOPRA_PID_TIMER, expireMsg);
                    }

                    *(g_drxTimerCtrlBlk[i].phTm)    = VOS_NULL_PTR;
                    g_drxTimerCtrlBlk[i].usedFlag = DRX_TIMER_NOT_USED_FLAG;
                }
            }
        }

        nextTimer = VOS_GetNextDrxTimer(curSlice);
        if (VOS_NULL_DWORD != nextTimer) {
            VOS_StartDrxHardTimer(nextTimer);
        }

        VOS_SpinUnlockIntUnlock(&g_drxTimerSpinLock, lockLevel);
    }
}

/*
 * Description: create DRX timer task
 */
VOS_UINT32 VOS_DrxTimerTaskCreat(VOS_VOID)
{
    VOS_UINT32 timerArguments[4] = { 0, 0, 0, 0 };

    /* DrxTimer自旋锁的初始化 */
    VOS_SpinLockInit(&g_drxTimerSpinLock);

    /* Wake Src投票中使用的自旋锁的初始化 */
    VOS_SpinLockInit(&g_flightModeVoteMapSpinLock);

    return (VOS_CreateTask("DRX_TIMER", &g_drxTimerTaskId, VOS_DrxTimerTaskFunc, COMM_RTC_TIMER_TASK_PRIO,
                           RTC_TIMER_TASK_STACK_SIZE, timerArguments));
}

/*
 * Description: stop a DRX timer which was previously started.
 */
VOS_UINT32 V_StopDrxTimerFunc(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    DRX_TimerControlBlock *timerCtrl = VOS_NULL_PTR;
    VOS_UINT32               nextTime;

    if (VOS_NULL_PTR == *phTm) {
        return VOS_OK;
    }

    if (VOS_ERR == VOS_DrxCheckTimer(phTm, fileId, lineNo)) {
        return VOS_ERRNO_DRXTIME_ERROR_TIMERHANDLE;
    }

    timerCtrl = (DRX_TimerControlBlock *)(*phTm);
    *phTm        = VOS_NULL_PTR;

    if (DRX_TIMER_USED_FLAG == timerCtrl->usedFlag) {
        timerCtrl->usedFlag = DRX_TIMER_NOT_USED_FLAG;

        nextTime = VOS_GetNextDrxTimer(VOS_GetSlice());
        if (VOS_NULL_DWORD == nextTime) {
            g_rtcSocTimerInfo.stopCount++;
            if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_DRX_TIMER_ID)) {
                g_rtcSocTimerInfo.stopErrCount++;
            }
        } else {
            VOS_StartDrxHardTimer(nextTime);
        }
    }

    return VOS_OK;
}

/*
 * Description: stop a DRX timer which was previously started.
 */
MODULE_EXPORTED VOS_UINT32 V_StopDrxTimer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 lockLevel;
    VOS_UINT32 returnValue;

    if (VOS_NULL_PTR == phTm) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_DRXTIME_STOP_INPUTISNULL);

        return (VOS_ERRNO_DRXTIME_STOP_INPUTISNULL);
    }

    VOS_SpinLockIntLock(&g_drxTimerSpinLock, lockLevel);

    returnValue = V_StopDrxTimerFunc(phTm, fileId, lineNo);

    VOS_SpinUnlockIntUnlock(&g_drxTimerSpinLock, lockLevel);

    return returnValue;
}

/*
 * Description: allocate and start a DRX timer.
 */
MODULE_EXPORTED VOS_UINT32 V_StartDrxTimer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name,
                                           VOS_UINT32 param, VOS_UINT32 fileId, VOS_INT32 lineNo)

{
    VOS_UINT32 i;
    VOS_UINT32 lockLevel;
    VOS_UINT32 curSlice;
    VOS_UINT32 nextTime;

    if (VOS_NULL_PTR == phTm) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_DRXTIME_START_INPUTISNULL);
        return (VOS_ERRNO_DRXTIME_START_INPUTISNULL);
    }

    VOS_SpinLockIntLock(&g_drxTimerSpinLock, lockLevel);

    /* stop the timer if exists */
    if (VOS_NULL_PTR != *phTm) {
        if (VOS_OK != V_StopDrxTimerFunc(phTm, fileId, lineNo)) {
            VOS_SpinUnlockIntUnlock(&g_drxTimerSpinLock, lockLevel);

            return VOS_ERRNO_DRXTIME_START_STOP_FAIL;
        }
    }

    for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++) {
        if (DRX_TIMER_NOT_USED_FLAG == g_drxTimerCtrlBlk[i].usedFlag) {
            break;
        }
    }

    /* All DRX timer are used. */
    if (DRX_TIMER_MAX_NUMBER == i) {
        VOS_SpinUnlockIntUnlock(&g_drxTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_DRXTIME_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_START_MSGNOTINSTALL, (VOS_INT)pid, (VOS_INT)name,
                             (VOS_CHAR *)g_drxTimerCtrlBlk, sizeof(g_drxTimerCtrlBlk));

        return (VOS_ERRNO_DRXTIME_START_MSGNOTINSTALL);
    }

    g_drxTimerCtrlBlk[i].pid                        = pid;
    g_drxTimerCtrlBlk[i].name                       = name;
    g_drxTimerCtrlBlk[i].para                       = param;
    g_drxTimerCtrlBlk[i].phTm                         = phTm;
    g_drxTimerCtrlBlk[i].timeOutValueInMilliSeconds = length;

    g_drxTimerCtrlBlk[i].timeOutValueSlice = (VOS_UINT32)RTC_MUL_32_DOT_768(length, fileId, lineNo);

    curSlice = VOS_GetSlice();

    g_drxTimerCtrlBlk[i].timeEndSlice = g_drxTimerCtrlBlk[i].timeOutValueSlice + curSlice;

#if VOS_YES == VOS_TIMER_CHECK
    g_drxTimerCtrlBlk[i].fileId    = fileId;
    g_drxTimerCtrlBlk[i].lineNo    = (VOS_UINT32)lineNo;
    g_drxTimerCtrlBlk[i].allocTick = VOS_GetSlice();
#endif

    g_drxTimerCtrlBlk[i].usedFlag = DRX_TIMER_USED_FLAG;

    *phTm = (HTIMER)(&g_drxTimerCtrlBlk[i]);

    nextTime = VOS_GetNextDrxTimer(curSlice);
    if (VOS_NULL_DWORD != nextTime) {
        VOS_StartDrxHardTimer(nextTime);
    }

    VOS_SpinUnlockIntUnlock(&g_drxTimerSpinLock, lockLevel);

    return VOS_OK;
}

/*
 * Description: 清除当前modem上所有的票
 */
MODULE_EXPORTED VOS_VOID OM_SetDrxTimerWakeSrcAllVote(ModemIdUint16 modem)
{
    VOS_ULONG                    lockLevel;
    VOS_UINT32                   voteBit;
    VOS_UINT32                   bitPos;
    DRX_TimerWakeSrcVote         drxTimerWakeSrcVoteInfo;
    VOS_UINT32                   voteMap;

    /* 参数检查 */
    if (modem >= MODEM_ID_BUTT) {
        return;
    }

    VOS_SpinLockIntLock(&g_flightModeVoteMapSpinLock, lockLevel);

    /* 当前modem不在drx，进飞行模式，销整个modem上的票 */
    for (bitPos = 0; bitPos < DRX_TIMER_WAKE_SRC_MODE_NUM; bitPos++) {
        voteBit = modem * DRX_TIMER_WAKE_SRC_MODE_NUM + bitPos;
        g_flightModeVoteMap &= (~BIT(voteBit));
    }
    voteMap = g_flightModeVoteMap;

    VOS_SpinUnlockIntUnlock(&g_flightModeVoteMapSpinLock, lockLevel);

    /* 所有mode都退出了drx，就设为唤醒源 */
    if (0 == voteMap) {
        mdrv_pm_set_wakesrc(PM_WAKE_SRC_DRX_TIMER);
    }

    /* Mntn Event */
    drxTimerWakeSrcVoteInfo.voteType  = DRX_TIMER_WAKE_SRC_VOTE_SET_ALL;
    drxTimerWakeSrcVoteInfo.voteModem = modem;
    drxTimerWakeSrcVoteInfo.voteMode  = VOS_RATMODE_GSM;
    drxTimerWakeSrcVoteInfo.voteValue = voteMap;
    drxTimerWakeSrcVoteInfo.slice     = VOS_GetSlice();
    PAMOM_DrxTimer_Event((VOS_VOID *)&drxTimerWakeSrcVoteInfo, sizeof(DRX_TimerWakeSrcVote));

    return;
}

/*
 * Description: 设置DRX timer作为唤醒源
 */
MODULE_EXPORTED VOS_VOID OM_SetDrxTimerWakeSrc(ModemIdUint16 modem, VOS_RatModeUint32 mode)
{
    VOS_ULONG                    lockLevel;
    VOS_UINT32                   voteBit;
    DRX_TimerWakeSrcVote drxTimerWakeSrcVoteInfo;
    VOS_UINT32                   voteMap;

    /* 参数检查 */
    if ((modem >= MODEM_ID_BUTT) || (mode >= VOS_RATMODE_BUTT)) {
        return;
    }

    VOS_SpinLockIntLock(&g_flightModeVoteMapSpinLock, lockLevel);

    /* 当前mode退出drx，要销票 */
    voteBit = modem * DRX_TIMER_WAKE_SRC_MODE_NUM + mode;
    g_flightModeVoteMap &= (~BIT(voteBit));
    voteMap = g_flightModeVoteMap;

    VOS_SpinUnlockIntUnlock(&g_flightModeVoteMapSpinLock, lockLevel);

    /* 所有mode都退出了drx，就设为唤醒源 */
    if (0 == voteMap) {
        mdrv_pm_set_wakesrc(PM_WAKE_SRC_DRX_TIMER);
    }

    /* Mntn Event */
    drxTimerWakeSrcVoteInfo.voteType  = DRX_TIMER_WAKE_SRC_VOTE_SET;
    drxTimerWakeSrcVoteInfo.voteModem = modem;
    drxTimerWakeSrcVoteInfo.voteMode  = mode;
    drxTimerWakeSrcVoteInfo.voteValue = voteMap;
    drxTimerWakeSrcVoteInfo.slice     = VOS_GetSlice();
    PAMOM_DrxTimer_Event((VOS_VOID *)&drxTimerWakeSrcVoteInfo, sizeof(DRX_TimerWakeSrcVote));

    return;
}

/*
 * Description: 设置DRX timer不作为唤醒源
 */
MODULE_EXPORTED VOS_VOID OM_DelDrxTimerWakeSrc(ModemIdUint16 modem, VOS_RatModeUint32 mode)
{
    VOS_ULONG                    lockLevel;
    VOS_UINT32                   voteBit;
    DRX_TimerWakeSrcVote drxTimerWakeSrcVoteInfo;
    VOS_UINT32                   voteMap;

    /* 参数检查 */
    if ((modem >= MODEM_ID_BUTT) || (mode >= VOS_RATMODE_BUTT)) {
        return;
    }

    VOS_SpinLockIntLock(&g_flightModeVoteMapSpinLock, lockLevel);

    /* 当前mode进drx，要投票 */
    voteBit = modem * DRX_TIMER_WAKE_SRC_MODE_NUM + mode;
    g_flightModeVoteMap |= BIT(voteBit);
    voteMap = g_flightModeVoteMap;

    VOS_SpinUnlockIntUnlock(&g_flightModeVoteMapSpinLock, lockLevel);

    /* 只要有mode进drx，就设成不作为唤醒源 */
    if (0 != voteMap) {
        mdrv_pm_clear_wakesrc(PM_WAKE_SRC_DRX_TIMER);
    }

    /* Mntn Event */
    drxTimerWakeSrcVoteInfo.voteType  = DRX_TIMER_WAKE_SRC_VOTE_CLEAR;
    drxTimerWakeSrcVoteInfo.voteModem = modem;
    drxTimerWakeSrcVoteInfo.voteMode  = mode;
    drxTimerWakeSrcVoteInfo.voteValue = voteMap;
    drxTimerWakeSrcVoteInfo.slice     = VOS_GetSlice();
    PAMOM_DrxTimer_Event((VOS_VOID *)&drxTimerWakeSrcVoteInfo, sizeof(DRX_TimerWakeSrcVote));

    return;
}

#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
/*
 * Description: 乘以32.768
 */
VOS_UINT64 BIT64_MUL_32_DOT_768(VOS_UINT32 value, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 productHigh;
    VOS_UINT32 productLow;
    VOS_UINT32 quotientHigh;
    VOS_UINT32 quotientLow;
    VOS_UINT32 rtcRemainder;
    VOS_UINT32 returnValue;
    VOS_UINT64 ull64Value;

    returnValue = VOS_64Multi32(0, value, RTC_TIMER_CHECK_LONG_PRECISION, &productHigh, &productLow);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    returnValue = VOS_64Div32(productHigh, productLow, 1000, &quotientHigh, &quotientLow, &rtcRemainder);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    ull64Value = (((VOS_UINT64)quotientHigh << 32) & 0xffffffff00000000) |
                 ((VOS_UINT64)quotientLow & 0x00000000ffffffff);

    return ull64Value;
}

/*
 * Description: 除以32.768
 */
VOS_UINT64 BIT64_DIV_32_DOT_768(VOS_UINT64 value, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 productHigh;
    VOS_UINT32 productLow;
    VOS_UINT32 quotientHigh;
    VOS_UINT32 quotientLow;
    VOS_UINT32 rtcRemainder;
    VOS_UINT32 returnValue;
    VOS_UINT64 ull64Value;

    returnValue = VOS_64Multi32((VOS_UINT32)(value >> 32), (VOS_UINT32)(value & 0xffffffff), 1000, &productHigh,
                             &productLow);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    returnValue = VOS_64Div32(productHigh, productLow, RTC_TIMER_CHECK_LONG_PRECISION, &quotientHigh, &quotientLow,
                           &rtcRemainder);
    if (VOS_OK != returnValue) {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&value,
                             sizeof(value));
        return VOS_ERR;
    }

    ull64Value = (((VOS_UINT64)quotientHigh << 32) & 0xffffffff00000000) |
                 ((VOS_UINT64)quotientLow & 0x00000000ffffffff);

    return ull64Value;
}

/*
 * 功能描述: 检测输入的timer句柄合法性
 */
VOS_UINT32 VOS_Bit64TimerCheckTimer(const HTIMER *phTm)
{
    VOS_UINT32 i;

    for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++) {
        if (*phTm == (HTIMER)(&g_bit64TimerCtrlBlk[i])) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

/*
 * 功能描述: 64Bit定时器中断处理接口
 */
VOS_VOID VOS_Bit64TimerIsr(VOS_VOID)
{
    g_rtcSocTimerInfo.bit64TimerStopCount++;
    if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_OSA_TIMER2_ID)) {
        g_rtcSocTimerInfo.bit64TimerStopErrCount++;
    }

    (VOS_VOID)VOS_SmV(g_bit64TimerSem);

    return;
}

/*
 * 功能描述: 64Bit定时器启动硬件定时器接口
 */
VOS_VOID VOS_StartBit64HardTimer(VOS_UINT32 value)
{
    g_rtcSocTimerInfo.bit64TimerStopCount++;
    if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_OSA_TIMER2_ID)) {
        g_rtcSocTimerInfo.bit64TimerStopErrCount++;
    }

    g_rtcSocTimerInfo.bit64TimerStartCount++;
    if (VOS_OK != mdrv_timer_start(TIMER_CCPU_OSA_TIMER2_ID, (timer_handle_cb)VOS_Bit64TimerIsr, TIMER_CCPU_OSA_TIMER2_ID,
                                   value, TIMER_ONCE_COUNT, TIMER_UNIT_NONE)) {
        g_rtcSocTimerInfo.bit64TimerStartErrCount++;
    }

    return;
}

/*
 * 功能描述: 64BIT定时器控制块初始化
 */
VOS_UINT32 VOS_Bit64RtcTimerCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32 i;

    for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++) {
        g_bit64TimerCtrlBlk[i].usedFlag = BIT64_TIMER_NOT_USED_FLAG;
    }

#if (OSA_CPU_NRCPU == VOS_OSA_CPU) && (VOS_WIN32 == VOS_OS_VER)
#else
    if (VOS_OK != VOS_SmCreate("B64", 0, VOS_SEMA4_FIFO, &g_bit64TimerSem)) {
        mdrv_err("<VOS_64BitRtcTimerCtrlBlkInit> create semaphore error.\n");

        return VOS_ERR;
    }
#endif

    return VOS_OK;
}

/*
 * 功能描述: 计算下个定时器超时时间
 */
VOS_UINT32 VOS_GetNextBit64Timer(VOS_UINT64 curSlice, VOS_UINT32 *needStartTimer)
{
    VOS_UINT32 i;
    VOS_UINT64 interval;
    VOS_UINT32 minValue;

    /* 64BIT定时器资源池列表中所有定时器距离当前系统时间超时时长超过18小时，
       则需要启动18小时最大的定时计数。换算成32K硬件定时器时长为0x7E900000 */
    minValue         = 0x7E900000;
    *needStartTimer = VOS_FALSE;

    for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++) {
        if (BIT64_TIMER_USED_FLAG == g_bit64TimerCtrlBlk[i].usedFlag) {
            *needStartTimer = VOS_TRUE;

            if (g_bit64TimerCtrlBlk[i].timeEndSlice >= curSlice) {
                interval = g_bit64TimerCtrlBlk[i].timeEndSlice - curSlice;
            } else {
                interval = 0;
            }

            if ((VOS_UINT64)minValue > interval) {
                minValue = (VOS_UINT32)interval;
            }
        }
    }

    /* 规避芯片timerbug，不能启动0步长定时器 */
    if (0 == minValue) {
        minValue += 1;
    }

    g_bit64NextHardTimerSlice = minValue;

    return minValue;
}

/*
 * 功能描述: 64BIT定时器任务处理接口
 */
VOS_VOID VOS_Bit64TimerTaskFunc(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3)
{
    VOS_UINT32     lockLevel;
    VOS_UINT64     curSlice;
    VOS_UINT32     i;
    VOS_UINT32     nextTimer;
    REL_TimerMsgBlock *expireMsg = VOS_NULL_PTR;
    VOS_UINT32     needStartTimer;

#if (VOS_WIN32 == VOS_OS_VER)
    VOS_UINT32 j;

    for (j = 0; j < 1; j++) {
#else
    for (;;) {
#endif

        /* SemTake SEM when release */
        (VOS_VOID)VOS_SmP(g_bit64TimerSem, 0);

        VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

        curSlice = VOS_Get64BitSlice();

        for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++) {
            if (BIT64_TIMER_USED_FLAG == g_bit64TimerCtrlBlk[i].usedFlag) {
                /* 64位Slice时间是个非常大的时间，按照正常使用该时间是几乎不会
                  发生反转的，所以此处判断可以不用考虑反转场景 */
                if (curSlice >= g_bit64TimerCtrlBlk[i].timeEndSlice) {
                    /* 定时器超时后，给调用组件发送消息通知 */
                    expireMsg = VOS_TimerPreAllocMsg(g_bit64TimerCtrlBlk[i].pid);
                    if (VOS_NULL_PTR != expireMsg) {
                        expireMsg->name = g_bit64TimerCtrlBlk[i].name;
                        expireMsg->para = g_bit64TimerCtrlBlk[i].para;

                        (VOS_VOID)VOS_SendMsg(DOPRA_PID_TIMER, expireMsg);
                    }

                    *(g_bit64TimerCtrlBlk[i].phTm)    = VOS_NULL_PTR;
                    g_bit64TimerCtrlBlk[i].usedFlag = BIT64_TIMER_NOT_USED_FLAG;
                }
            }
        }

        nextTimer = VOS_GetNextBit64Timer(curSlice, &needStartTimer);

        if (VOS_TRUE == needStartTimer) {
            VOS_StartBit64HardTimer(nextTimer);
        }

        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);
    }
}

/*
 * 功能描述: 64BIT定时器任务创建
 */
VOS_UINT32 VOS_Bit64TimerTaskCreat(VOS_VOID)
{
    VOS_UINT32 timerArguments[4] = { 0, 0, 0, 0 };

    return (VOS_CreateTask("B64_TIMER", &g_bit64TimerTaskId, VOS_Bit64TimerTaskFunc, COMM_RTC_TIMER_TASK_PRIO,
                           RTC_TIMER_TASK_STACK_SIZE, timerArguments));
}

/*
 * 功能描述: 64BIT定时器停止处理接口
 */
VOS_UINT32 VOS_StopBit64Timer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo, VOS_TimerOmEvent *event)
{
    BIT64_TimerControlBlock *timerCtrl = VOS_NULL_PTR;
    VOS_UINT64                 curSlice;
    VOS_UINT32                 nextTime;
    VOS_UINT32                 needStartTimer;

    timerCtrl = (BIT64_TimerControlBlock *)(*phTm);
    *phTm        = VOS_NULL_PTR;

    if (BIT64_TIMER_USED_FLAG == timerCtrl->usedFlag) {
        timerCtrl->usedFlag = BIT64_TIMER_NOT_USED_FLAG;

        curSlice = VOS_Get64BitSlice();

        nextTime = VOS_GetNextBit64Timer(curSlice, &needStartTimer);

        if (VOS_FALSE == needStartTimer) {
            g_rtcSocTimerInfo.bit64TimerStopCount++;
            if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_OSA_TIMER2_ID)) {
                g_rtcSocTimerInfo.bit64TimerStopErrCount++;
            }
        } else {
            VOS_StartBit64HardTimer(nextTime);
        }
    } else {
        if (VOS_NULL_PTR != event) {
            event->mode      = 0;
            event->pid         = timerCtrl->pid;
            event->length    = timerCtrl->timeOutValueInMilliSeconds;
            event->name      = timerCtrl->name;
            event->param     = timerCtrl->para;
            event->precision = 0;
        }

        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED, (VOS_INT)fileId, (VOS_INT)lineNo,
                             (VOS_CHAR *)phTm, sizeof(VOS_CHAR *));

        /* 补充异常处理流程 */
        return VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED;
    }

    /* OM */
    if (VOS_NULL_PTR != event) {
        event->mode      = 0;
        event->pid         = timerCtrl->pid;
        event->length    = timerCtrl->timeOutValueInMilliSeconds;
        event->name      = timerCtrl->name;
        event->param     = timerCtrl->para;
        event->precision = 0;
    }

    return VOS_OK;
}

/*
 * 功能描述: 64BIT定时器启动接口
 */
MODULE_EXPORTED VOS_UINT32 VOS_StartBit64Timer(HTIMER *phTm, VOS_PID pid, VOS_UINT32 length, VOS_UINT32 name,
                                               VOS_UINT32 param, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 i;
    VOS_UINT32 lockLevel;
    VOS_UINT64 curSlice;
    VOS_UINT32 nextTime;
    VOS_UINT32 needStartTimer;

    VOS_SpinLockIntLock(&g_vosTimerSpinLock, lockLevel);

    /* 如果定时器已经启动，则先停止该定时器 */
    if (VOS_NULL_PTR != *phTm) {
        if (VOS_OK != VOS_StopBit64Timer(phTm, fileId, lineNo, VOS_NULL_PTR)) {
            VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

            (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_BIT64TIME_START_STOP_FAIL);

            VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_START_STOP_FAIL, (VOS_INT)pid, (VOS_INT)name,
                                 (VOS_CHAR *)g_bit64TimerCtrlBlk, sizeof(g_bit64TimerCtrlBlk));

            return VOS_ERRNO_BIT64TIME_START_STOP_FAIL;
        }
    }

    /* 查找资源池中空闲的定时器资源 */
    for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++) {
        if (BIT64_TIMER_NOT_USED_FLAG == g_bit64TimerCtrlBlk[i].usedFlag) {
            break;
        }
    }

    /* 所有定时器资源都被使用，OSA发起保护性复位保留现场确认资源使用情况 */
    if (BIT64_TIMER_MAX_NUMBER == i) {
        VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_BIT64TIME_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_START_MSGNOTINSTALL, (VOS_INT)pid, (VOS_INT)name,
                             (VOS_CHAR *)g_bit64TimerCtrlBlk, sizeof(g_bit64TimerCtrlBlk));

        return (VOS_ERRNO_BIT64TIME_START_MSGNOTINSTALL);
    }

    g_bit64TimerCtrlBlk[i].pid                        = pid;
    g_bit64TimerCtrlBlk[i].name                       = name;
    g_bit64TimerCtrlBlk[i].para                       = param;
    g_bit64TimerCtrlBlk[i].phTm                         = phTm;
    g_bit64TimerCtrlBlk[i].timeOutValueInMilliSeconds = length;

    g_bit64TimerCtrlBlk[i].timeOutValueSlice = BIT64_MUL_32_DOT_768(length, fileId, lineNo);

    curSlice = VOS_Get64BitSlice();

    g_bit64TimerCtrlBlk[i].timeEndSlice = g_bit64TimerCtrlBlk[i].timeOutValueSlice + curSlice;

#if VOS_YES == VOS_TIMER_CHECK
    g_bit64TimerCtrlBlk[i].fileId     = fileId;
    g_bit64TimerCtrlBlk[i].lineNo     = (VOS_UINT32)lineNo;
    g_bit64TimerCtrlBlk[i].allocTick = VOS_Get64BitSlice();
#endif

    g_bit64TimerCtrlBlk[i].usedFlag = BIT64_TIMER_USED_FLAG;

    *phTm = (HTIMER)(&g_bit64TimerCtrlBlk[i]);

    /* 遍历资源池，确认是否需要启动硬件定时器 */
    nextTime = VOS_GetNextBit64Timer(curSlice, &needStartTimer);

    if (VOS_TRUE == needStartTimer) {
        VOS_StartBit64HardTimer(nextTime);
    }

    VOS_SpinUnlockIntUnlock(&g_vosTimerSpinLock, lockLevel);

    return VOS_OK;
}

/*
 * 功能描述: 64BIT定时器启动接口
 */
VOS_UINT32 VOS_RestartBit64Timer(HTIMER *phTm, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    BIT64_TimerControlBlock *timerCtrl = VOS_NULL_PTR;
    VOS_UINT64                 curSlice;
    VOS_UINT32                 nextTime;
    VOS_UINT32                 needStartTimer;

    timerCtrl = (BIT64_TimerControlBlock *)(*phTm);

    if (BIT64_TIMER_NOT_USED_FLAG == timerCtrl->usedFlag) {
        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED, (VOS_INT)fileId, (VOS_INT)lineNo,
                             (VOS_CHAR *)phTm, sizeof(VOS_CHAR *));

        return VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED;
    }

    curSlice = VOS_Get64BitSlice();

    /* 重置定时器预期超时物理时间 */
    timerCtrl->timeEndSlice = timerCtrl->timeOutValueSlice + curSlice;

    /* 遍历资源池，确认是否需要启动硬件定时器 */
    nextTime = VOS_GetNextBit64Timer(curSlice, &needStartTimer);

    if (VOS_TRUE == needStartTimer) {
        VOS_StartBit64HardTimer(nextTime);
    }

    return VOS_OK;
}

/*
 * 功能描述: 64BIT定时器启动接口
 */
VOS_UINT32 VOS_GetBit64RemainTime(HTIMER *phTm, VOS_UINT32 *vosTime, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    BIT64_TimerControlBlock *timerCtrl = VOS_NULL_PTR;
    VOS_UINT64                 curSlice;
    VOS_UINT64                 tempTime;

    timerCtrl = (BIT64_TimerControlBlock *)(*phTm);

    if (BIT64_TIMER_NOT_USED_FLAG == timerCtrl->usedFlag) {
        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED, (VOS_INT)fileId, (VOS_INT)lineNo,
                             (VOS_CHAR *)phTm, sizeof(VOS_CHAR *));

        return VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED;
    }

    curSlice = VOS_Get64BitSlice();
    if (timerCtrl->timeEndSlice >= curSlice) {
        tempTime = timerCtrl->timeEndSlice - curSlice;
    } else {
        tempTime = 0;
    }

    *vosTime = (VOS_UINT32)BIT64_DIV_32_DOT_768(tempTime, fileId, usLineNo);

    return VOS_OK;
}

#endif

/*
 * Description: print rtc's debug info
 */
MODULE_EXPORTED VOS_VOID ShowRtcTimerLog(VOS_VOID)
{
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulStartCount=%d. (call DRV Start timer num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.startCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulStopCount=%d. (call DRV Stop timer num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.stopCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulExpireCount=%d. (receive DRV ISR of DualTimer num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.expireCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulStartErrCount=%d. (call DRV Stop timer err num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.startErrCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulStopErrCount=%d. (call DRV Start timer err num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.stopErrCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulElapsedErrCount=%d. (call DRV get rest timer num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.elapsedErrCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulElapsedContentErrCount=%d. (call DRV get rest timer err num)\n",
               __FUNCTION__, g_rtcSocTimerInfo.elapsedContentErrCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulBit64TimerStartCount=%d. (call DRV get start timer num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.bit64TimerStartCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulBit64TimerStopCount=%d. (call DRV get stop timer num)\n", __FUNCTION__,
               g_rtcSocTimerInfo.bit64TimerStopCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulBit64TimerStartErrCount=%d. (call DRV get start timer err num)\n",
               __FUNCTION__, g_rtcSocTimerInfo.bit64TimerStartErrCount);
    mdrv_debug("<%s> g_rtcSocTimerInfo.ulBit64TimerStopErrCount=%d. (call DRV get stop timer err num)\n",
               __FUNCTION__, g_rtcSocTimerInfo.bit64TimerStopErrCount);
}

/*
 * Description: report RTC OM info
 */
VOS_VOID RTC_ReportOmInfo(VOS_VOID)
{
    VOS_UINT32            rTCLength               = 0;
    VOS_UINT32            rTCTimerDebugInfoLength = 0;
    VOS_UINT32            rTCTimerInfoLength      = 0;
    VOS_RtcOmInfo *rTCInfo                = VOS_NULL_PTR;

    /* 获取g_rtcSocTimerDebugInfo的大小 */
    rTCTimerDebugInfoLength = sizeof(RTC_SocTimerDebugInfo) * RTC_MAX_TIMER_NUMBER;

    /* 获取g_stRtcSocTimerInfo的大小 */
    rTCTimerInfoLength = sizeof(VOS_TimerSocTimerInfo);

    rTCLength = rTCTimerDebugInfoLength + rTCTimerInfoLength;

    rTCInfo = (VOS_RtcOmInfo *)VOS_MemAlloc(DOPRA_PID_TIMER, DYNAMIC_MEM_PT,
                                                      (VOS_UINT32)(sizeof(VOS_RtcOmInfo) -
                                                                   (VOS_UINT32)(sizeof(VOS_UINT8) * 4) + rTCLength));
    if (VOS_NULL_PTR != rTCInfo) {
        rTCInfo->ulSenderPid = DOPRA_PID_TIMER;

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
        rTCInfo->ulReceiverPid = ACPU_PID_PAM_OM;
#elif (OSA_CPU_NRCPU == VOS_OSA_CPU)
        rTCInfo->ulReceiverPid = NRCPU_PID_PAM_OM;
#else
        rTCInfo->ulReceiverPid = CCPU_PID_PAM_OM;
#endif

        rTCInfo->ulLength = rTCLength;

        /*lint -e426 */
        if (memcpy_s((VOS_VOID *)rTCInfo->value, rTCLength, (VOS_VOID *)g_rtcSocTimerDebugInfo,
                     rTCTimerDebugInfoLength) != EOK) {
            (VOS_VOID)VOS_MemFree(DOPRA_PID_TIMER, rTCInfo);

            return;
        }
        /*lint +e426 */

        /*lint -e416 -e426 */
        if (memcpy_s((VOS_VOID *)(rTCInfo->value + rTCTimerDebugInfoLength),
                     (rTCLength - rTCTimerDebugInfoLength), (VOS_VOID *)&g_rtcSocTimerInfo,
                     rTCTimerInfoLength) != EOK) {
            (VOS_VOID)VOS_MemFree(DOPRA_PID_TIMER, rTCInfo);

            return;
        }
        /*lint +e416 +e426 */

        mdrv_diag_trace_report((VOS_VOID *)(rTCInfo));

        (VOS_VOID)VOS_MemFree(DOPRA_PID_TIMER, rTCInfo);
    }

    return;
}

