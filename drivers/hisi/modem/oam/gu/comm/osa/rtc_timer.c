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

#include "vos.h"
#include "v_IO.h"
#include "v_private.h"
#include "msp_diag_comm.h"
#include "mdrv.h"
#include "pam_tag.h"

/* LINUX不支持 */
#if (VOS_VXWORKS == VOS_OS_VER)
#include "stdlib.h"
#endif


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_V_RTC_TIMER_C
#define    THIS_MODU           mod_pam_osa


/* the state of Timer */
#define RTC_TIMER_CTRL_BLK_RUNNIG                           0
#define RTC_TIMER_CTRL_BLK_PAUSE                            1
#define RTC_TIMER_CTRL_BLK_STOP                             2

/* the tag of  RTC_TimerCtrlBlkFree */
#define THE_FIRST_RTC_TIMER_TAG                             1
#define THE_SECOND_RTC_TIMER_TAG                            2

/* can't make a mistake,add threshold */
#define RTC_TIMER_CHECK_PRECISION                           10

#define RTC_SOC_TIMER_CHECK_PRECISION                       100
#define RTC_TIMER_CHECK_LONG_PRECISION                      32768

/* the tag of  RTC_Add_Timer_To_List */
enum
{
    VOS_ADD_TIMER_TAG_1,
    VOS_ADD_TIMER_TAG_2,
    VOS_ADD_TIMER_TAG_3,
    VOS_ADD_TIMER_TAG_4,
    VOS_ADD_TIMER_TAG_5,
    VOS_ADD_TIMER_TAG_BUTT
};

/* timer task's stack size */
#define RTC_TIMER_TASK_STACK_SIZE                           3072

/* 1s->1000ms */
#define RTC_ONE_SECOND                                      1000

/* 32.768K */
#define RTC_PRECISION_CYCLE_LENGTH                          0.32768


#define RTC_TIMER_NORMAL_COUNT                              5

#ifndef BIT
#define BIT(x)                  ((unsigned)0x1 << (x))
#endif

typedef struct RTC_TIMER_CONTROL_STRU
{
    VOS_UINT32      TimerId;/* timer ID */
    VOS_UINT32      ulUsedFlag;/* whether be used or not */
    VOS_PID         Pid;/* who allocate the timer */
    VOS_UINT32      Name;/* timer's name */
    VOS_UINT32      Para;/* timer's paremate */
    VOS_UINT8       aucRsv[4];
    REL_TIMER_FUNC  CallBackFunc;/* timer's callback function */
    HTIMER          *phTm;/* user's pointer which point the real timer room */
    VOS_UINT32      TimeOutValueInMilliSeconds;
    VOS_UINT32      TimeOutValueInCycle;
    struct RTC_TIMER_CONTROL_STRU *next;
    struct RTC_TIMER_CONTROL_STRU *previous;
    VOS_UINT8       Mode;/* timer's mode */
    VOS_UINT8       State;/* timer's state */
    VOS_UINT8       Reserved[2];/* for 4 byte aligned */
    VOS_UINT32      ulPrecision;/* record only */
    VOS_UINT32      ulPrecisionInCycle;/* unit is 32K cycle */

#if VOS_YES == VOS_TIMER_CHECK
    VOS_UINT32      ulAllocTick;/* CPU tick of block allocation */
    VOS_UINT32      ulFreeTick;/* CPU tick of block free */
    VOS_UINT32      ulFileID;/* alloc file ID */
    VOS_UINT32      ulLineNo;/* alloc line no. */
    VOS_UINT32      ulBackUpTimeOutValueInCycle;
#endif
    VOS_UINT32      ulBackUpTimerId;/* timer ID */
} RTC_TIMER_CONTROL_BLOCK;


/* Added by g47350 for DRX timer Project, 2012/11/5, begin */
typedef struct DRX_TIMER_CONTROL_STRU
{
    VOS_UINT32      ulUsedFlag;                     /* whether be used or not */
    VOS_PID         ulPid;                          /* who allocate the timer */
    VOS_UINT32      ulName;                         /* timer's name */
    VOS_UINT32      ulPara;                         /* timer's paremate */
    HTIMER         *phTm;                           /* user's pointer which point the real timer room */
    VOS_UINT32      ulTimeOutValueInMilliSeconds;   /* timer's length(ms) */
    VOS_UINT32      ulTimeOutValueSlice;            /* timer's length(32k) */
    VOS_UINT32      ulTimeEndSlice;                 /* the end slice time of timer */

#if VOS_YES == VOS_TIMER_CHECK
    VOS_UINT32      ulAllocTick;                    /* CPU tick of block allocation */
    VOS_UINT32      ulFileID;                       /* alloc file ID */
    VOS_UINT32      ulLineNo;                       /* alloc line no. */
#endif
} DRX_TIMER_CONTROL_BLOCK;
/* Added by g47350 for DRX timer Project, 2012/11/5, end */

typedef struct BIT64_TIMER_CONTROL_STRU
{
    VOS_UINT32      ulUsedFlag;                     /* whether be used or not */
    VOS_PID         ulPid;                          /* who allocate the timer */
    VOS_UINT32      ulName;                         /* timer's name */
    VOS_UINT32      ulPara;                         /* timer's paremate */
    HTIMER         *phTm;                           /* user's pointer which point the real timer room */
    VOS_UINT32      ulTimeOutValueInMilliSeconds;   /* timer's length(ms) */
    VOS_UINT64      ullTimeOutValueSlice;           /* timer's length(64k) */
    VOS_UINT64      ullTimeEndSlice;                /* the end slice time of timer */

#if VOS_YES == VOS_TIMER_CHECK
    VOS_UINT64      ullAllocTick;                   /* CPU tick of block allocation */
    VOS_UINT32      ulFileID;                       /* alloc file ID */
    VOS_UINT32      ulLineNo;                       /* alloc line no. */
#endif
}BIT64_TIMER_CONTROL_BLOCK;

/* the number of task's control block */
VOS_UINT32                RTC_TimerCtrlBlkNumber;

/* the number of free task's control block */
VOS_UINT32                RTC_TimerIdleCtrlBlkNumber;

/* the start address of task's control block */
RTC_TIMER_CONTROL_BLOCK   *RTC_TimerCtrlBlk;

/* the start address of free task's control block list */
RTC_TIMER_CONTROL_BLOCK   *RTC_TimerIdleCtrlBlk;

/* the begin address of timer control block */
VOS_VOID                  *RTC_TimerCtrlBlkBegin;

/* the end address of timer control block */
VOS_VOID                  *RTC_TimerCtrlBlkEnd;

/* the head of the running timer list */
RTC_TIMER_CONTROL_BLOCK   *RTC_Timer_head_Ptr = VOS_NULL_PTR;

/* the task ID of timer's task */
VOS_UINT32                RTC_TimerTaskId;

/* the Min usage of timer */
VOS_UINT32                RTC_TimerMinTimerIdUsed;

/* the queue will be given when RTC's interrupt occures */
VOS_UINT32                g_ulRTCTaskQueueId;

/*record start value */
VOS_UINT32                RTC_Start_Value = ELAPESD_TIME_INVAILD;

#define RTC_TIMER_CTRL_BUF_SIZE (sizeof(RTC_TIMER_CONTROL_BLOCK) * RTC_MAX_TIMER_NUMBER )

VOS_CHAR g_acRtcTimerCtrlBuf[RTC_TIMER_CTRL_BUF_SIZE];

/* 循环记录SOC Timer的启停记录 */
enum
{
    RTC_SOC_TIMER_SEND_ERR = 0xfffffffd,
    RTC_SOC_TIMER_EXPIRED = 0xfffffffe,
    RTC_SOC_TIMER_STOPED = 0xffffffff
};

VOS_UINT32  g_ulRtcSocTimerDebugInfoSuffix = 0;

RTC_SOC_TIMER_DEBUG_INFO_STRU g_astRtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER];

/* 记录 RTC timer 可维可测信息 */
VOS_TIMER_SOC_TIMER_INFO_STRU g_stRtcSocTimerInfo;

/* Added by g47350 for DRX timer Project, 2012/11/5, begin */

/* the array of DRX timer's control block */
DRX_TIMER_CONTROL_BLOCK   g_astDRXTimerCtrlBlk[DRX_TIMER_MAX_NUMBER];

/* the semaphore will be given when DRX's interrupt occures */
VOS_SEM                   g_ulDRXSem;

/* the task ID of DRX timer's task */
VOS_UINT32                g_ulDRXTimerTaskId;

/* Added by g47350 for DRX timer Project, 2012/11/5, end */


#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)

VOS_UINT32                  g_ulBit64TimerTaskId;

BIT64_TIMER_CONTROL_BLOCK   g_astBit64TimerCtrlBlk[BIT64_TIMER_MAX_NUMBER];

VOS_SEM                     g_ulBit64TimerSem;

VOS_UINT32                  g_ulBit64NextHardTimerSlice = 0;

#endif


#if (OSA_CPU_CCPU == VOS_OSA_CPU)

/* 自旋锁，用来作DRX Timer的临界资源保护 */
VOS_SPINLOCK                    g_stDrxTimerSpinLock;

/* flight mode max mode number */
#define DRX_TIMER_WAKE_SRC_MODE_NUM     (8)

VOS_UINT32                      g_ulFlightModeVoteMap = 0;  /*DRX TIMER在飞行模式投票*/

VOS_SPINLOCK                    g_ulFlightModeVoteMapSpinLock;/* 用于投票的自旋锁 */

enum DRX_TIMER_WAKE_SRC_VOTE_TYPE_ENUM
{
    DRX_TIMER_WAKE_SRC_VOTE_SET_ALL     = 0,
    DRX_TIMER_WAKE_SRC_VOTE_SET         = 1,
    DRX_TIMER_WAKE_SRC_VOTE_CLEAR       = 2,
    DRX_TIMER_WAKE_SRC_VOTE_BUTT
};
typedef VOS_UINT32 DRX_TIMER_WAKE_SRC_VOTE_ENUM_UINT32;

typedef struct
{
    DRX_TIMER_WAKE_SRC_VOTE_ENUM_UINT32 enVoteType;
    MODEM_ID_ENUM_UINT16                enVoteModem;
    VOS_UINT8                           ucReserved[2];
    VOS_RATMODE_ENUM_UINT32             enVoteMode;
    VOS_UINT32                          ulVoteValue;
    VOS_UINT32                          ulSlice;
}DRX_TIMER_WAKE_SRC_VOTE_STRU;

#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#define VOS_RTC_TIMER_ID  (TIMER_ACPU_OSA_ID)
#endif

#if ( (OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU) )
#define VOS_RTC_TIMER_ID  (TIMER_CCPU_OSA_ID)
#endif

#if VOS_YES == VOS_TIMER_CHECK

VOS_VOID VOS_ShowUsed32KTimerInfo( VOS_VOID );

#endif

extern VOS_VOID OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_ENUM_UINT32 enNumber);
extern VOS_VOID OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_ENUM_UINT32 enNumber, VOS_UINT32 ulSendPid, VOS_UINT32 ulRcvPid, VOS_UINT32 ulMsgName);
extern VOS_VOID PAMOM_DrxTimer_Event(VOS_VOID *pData, VOS_UINT32 ulLength);

/* Just for Sparse checking. */
VOS_VOID StartHardTimer( VOS_UINT32 value );
VOS_VOID StopHardTimer(VOS_VOID);
VOS_INT32 RTC_DualTimerIsr(VOS_INT lPara);
RTC_TIMER_CONTROL_BLOCK *RTC_TimerCtrlBlkGet(VOS_UINT32 ulFileID, VOS_INT32 usLineNo);
VOS_UINT32 RTC_TimerCtrlBlkFree(RTC_TIMER_CONTROL_BLOCK *Ptr, VOS_UINT8 ucTag );
VOS_UINT32 GetHardTimerElapsedTime(VOS_VOID);
VOS_VOID RTC_TimerTaskFunc( VOS_UINT32 Para0, VOS_UINT32 Para1, VOS_UINT32 Para2, VOS_UINT32 Para3 );
VOS_VOID RTC_Add_Timer_To_List( RTC_TIMER_CONTROL_BLOCK  *Timer);
VOS_VOID RTC_Del_Timer_From_List( RTC_TIMER_CONTROL_BLOCK  *Timer);
VOS_VOID ShowRtcTimerLog( VOS_VOID );

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
VOS_UINT64 BIT64_MUL_32_DOT_768(VOS_UINT32 ulValue,VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo);
VOS_UINT64 BIT64_DIV_32_DOT_768(VOS_UINT64 ulValue,VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo);
VOS_VOID VOS_Bit64TimerIsr(VOS_VOID);
VOS_VOID VOS_StartBit64HardTimer(VOS_UINT32 ulValue);
VOS_UINT32 VOS_GetNextBit64Timer(VOS_UINT64 ullCurSlice, VOS_UINT32* pulNeedStartTimer);
VOS_VOID VOS_Bit64TimerTaskFunc( VOS_UINT32 Para0, VOS_UINT32 Para1,
                            VOS_UINT32 Para2, VOS_UINT32 Para3 );
#endif

typedef struct RTC_TIMER_PMLOG_STRU
{
    PM_LOG_COSA_PAM_ENUM_UINT32     stPamType;
    VOS_UINT32                      ulPid;
    VOS_UINT32                      ulTimerId;
} RTC_TIMER_PMLOG;

VOS_UINT32 g_ulTimerlpm = VOS_FALSE;

VOS_INT VOS_TimerLpmCb(VOS_INT x)
{
    g_ulTimerlpm = VOS_TRUE;

    return VOS_OK;
}


/*****************************************************************************
 Function   : RTC_MUL_32_DOT_768
 Description: 乘以32.768
 Input      : ulValue -- timer's value.uint is 32K cycle.
              ulFileID -- 文件ID
              usLineNo -- 行号
 Return     : 与32.768做乘法的结果
 Other      :
 *****************************************************************************/
VOS_UINT32 RTC_MUL_32_DOT_768(VOS_UINT32 ulValue,VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo)
{
    VOS_UINT32 ulProductHigh;
    VOS_UINT32 ulProductLow;
    VOS_UINT32 ulQuotientHigh;
    VOS_UINT32 ulQuotientLow;
    VOS_UINT32 ulRemainder;
    VOS_UINT32 ulReturn;

    ulReturn = VOS_64Multi32(0, ulValue, RTC_TIMER_CHECK_LONG_PRECISION, &ulProductHigh, &ulProductLow);
    if ( VOS_OK != ulReturn )
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    ulReturn = VOS_64Div32(ulProductHigh, ulProductLow, 1000, &ulQuotientHigh, &ulQuotientLow, &ulRemainder);
    if ( VOS_OK != ulReturn )
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    if ( VOS_NULL != ulQuotientHigh )
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    return ulQuotientLow;
}

/*****************************************************************************
 Function   : RTC_DIV_32_DOT_768
 Description: 除以32.768
 Input      : ulValue -- timer's value.uint is 32K cycle.
              ulFileID -- 文件ID
              usLineNo -- 行号
 Return     : 与32.768做除法的结果
 Other      :
 *****************************************************************************/
VOS_UINT32 RTC_DIV_32_DOT_768(VOS_UINT32 ulValue,VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo)
{

    VOS_UINT32 ulProductHigh;
    VOS_UINT32 ulProductLow;
    VOS_UINT32 ulQuotientHigh;
    VOS_UINT32 ulQuotientLow;
    VOS_UINT32 ulRemainder;
    VOS_UINT32 ulReturn;

    ulReturn = VOS_64Multi32(0, ulValue, 1000, &ulProductHigh, &ulProductLow);
    if ( VOS_OK != ulReturn )
    {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    ulReturn = VOS_64Div32(ulProductHigh, ulProductLow, RTC_TIMER_CHECK_LONG_PRECISION, &ulQuotientHigh, &ulQuotientLow, &ulRemainder);
    if ( VOS_OK != ulReturn )
    {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    if ( VOS_NULL != ulQuotientHigh )
    {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    return ulQuotientLow;

}

/*****************************************************************************
 Function   : RTC_MUL_DOT_32768
 Description: 乘以0.32768
 Input      : ulValue -- timer's value.uint is 32K cycle.
              ulFileID -- 文件ID
              usLineNo -- 行号
 Return     : 与0.32768做乘法的结果
 Other      :
 *****************************************************************************/
VOS_UINT32 RTC_MUL_DOT_32768(VOS_UINT32 ulValue,VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo)
{
    VOS_UINT32 ulProductHigh;
    VOS_UINT32 ulProductLow;
    VOS_UINT32 ulQuotientHigh;
    VOS_UINT32 ulQuotientLow;
    VOS_UINT32 ulRemainder;
    VOS_UINT32 ulReturn;

    ulReturn = VOS_64Multi32(0, ulValue, RTC_TIMER_CHECK_LONG_PRECISION, &ulProductHigh, &ulProductLow);
    if ( VOS_OK != ulReturn )
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_DOT_32768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    ulReturn = VOS_64Div32(ulProductHigh, ulProductLow, 100000, &ulQuotientHigh, &ulQuotientLow, &ulRemainder);
    if ( VOS_OK != ulReturn )
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_DOT_32768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    if ( VOS_NULL != ulQuotientHigh )
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_DOT_32768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    return ulQuotientLow;
}


/*****************************************************************************
 Function   : RTC_DebugSocInfo
 Description: record the track of Soc timer
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
MODULE_EXPORTED VOS_VOID RTC_DebugSocInfo(VOS_UINT32 ulAction, VOS_UINT32 ulSlice)
{
    g_astRtcSocTimerDebugInfo[g_ulRtcSocTimerDebugInfoSuffix].ulAction = ulAction;
    g_astRtcSocTimerDebugInfo[g_ulRtcSocTimerDebugInfoSuffix].ulSlice  = ulSlice;

    g_astRtcSocTimerDebugInfo[g_ulRtcSocTimerDebugInfoSuffix].ulCoreId = 0;

    g_ulRtcSocTimerDebugInfoSuffix++;

    if ( RTC_MAX_TIMER_NUMBER <= g_ulRtcSocTimerDebugInfoSuffix )
    {
        g_ulRtcSocTimerDebugInfoSuffix = 0;
    }
}

/*****************************************************************************
 Function   : RTC_GetDebugSocInfo
 Description: get the track of Soc timer
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID RTC_GetDebugSocInfo(VOS_UINT32 *pulAction, VOS_UINT32 *pulSlice, VOS_UINT32 *pulCoreId)
{
    VOS_UINT32 ulCurrentInfoSuffix = g_ulRtcSocTimerDebugInfoSuffix;

    if ( 0 == ulCurrentInfoSuffix )
    {
        *pulAction = g_astRtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER - 1].ulAction;
        *pulSlice  = g_astRtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER - 1].ulSlice;
        *pulCoreId = g_astRtcSocTimerDebugInfo[RTC_MAX_TIMER_NUMBER - 1].ulCoreId;
    }
    else
    {
        *pulAction = g_astRtcSocTimerDebugInfo[ulCurrentInfoSuffix - 1].ulAction;
        *pulSlice  = g_astRtcSocTimerDebugInfo[ulCurrentInfoSuffix - 1].ulSlice;
        *pulCoreId = g_astRtcSocTimerDebugInfo[ulCurrentInfoSuffix - 1].ulCoreId;
    }
}

#if ( (OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU) )
/********************************************************************
 Function   : RTC_SocTimerMemDump
 Description: Record the SOC timer's info.
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID RTC_SocTimerMemDump(VOS_VOID)
{
    /*lint -e438 屏蔽pucDumpBuffer没有使用的错误*/
    VOS_UINT8       *pucDumpBuffer;
    VOS_UINT32       ulBufferSize;

    (VOS_VOID)VOS_TaskLock();

    pucDumpBuffer = (VOS_UINT8 *)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == pucDumpBuffer)
    {
        (VOS_VOID)VOS_TaskUnlock();
        return;
    }

    ulBufferSize = VOS_DUMP_MEM_TOTAL_SIZE;

    if ( VOS_NULL_PTR == VOS_MemSet_s((VOS_VOID *)pucDumpBuffer, ulBufferSize, 0, VOS_DUMP_MEM_TOTAL_SIZE) )
    {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, VOS_DUMP_MEM_TOTAL_SIZE, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    if ( VOS_NULL_PTR == VOS_MemCpy_s((VOS_VOID *)pucDumpBuffer, ulBufferSize, (VOS_VOID *)g_acRtcTimerCtrlBuf, RTC_TIMER_CTRL_BUF_SIZE) )
    {
        mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, RTC_TIMER_CTRL_BUF_SIZE, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    pucDumpBuffer += RTC_TIMER_CTRL_BUF_SIZE;
    ulBufferSize  -= RTC_TIMER_CTRL_BUF_SIZE;

    if ( VOS_NULL_PTR == VOS_MemCpy_s((VOS_VOID *)pucDumpBuffer, ulBufferSize, (VOS_VOID *)g_astRtcSocTimerDebugInfo, sizeof(g_astRtcSocTimerDebugInfo)) )
    {
        mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, (VOS_INT)sizeof(g_astRtcSocTimerDebugInfo), (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    pucDumpBuffer += sizeof(g_astRtcSocTimerDebugInfo);
    ulBufferSize  -= sizeof(g_astRtcSocTimerDebugInfo);

    if ( VOS_NULL_PTR == VOS_MemCpy_s((VOS_VOID *)pucDumpBuffer, ulBufferSize, (VOS_VOID *)(&g_stRtcSocTimerInfo), sizeof(g_stRtcSocTimerInfo)) )
    {
        mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, (VOS_INT)sizeof(g_stRtcSocTimerInfo), (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    (VOS_VOID)VOS_TaskUnlock();
    return;
    /*lint +e438 */
}
#endif

/*****************************************************************************
 Function   : RTC_DualTimerIsrEntry
 Description: handle timer's list
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID RTC_DualTimerIsrEntry(VOS_UINT32 ulElapsedCycles)
{
    RTC_TIMER_CONTROL_BLOCK     *head_Ptr;
    /* the timer control which expire */
    RTC_TIMER_CONTROL_BLOCK     *RTC_TimerCtrlBlkCurrent;
    /* the timer head control which expire */
    RTC_TIMER_CONTROL_BLOCK     *RTC_TimerCtrlBlkexpired = VOS_NULL_PTR;
    RTC_TIMER_CONTROL_BLOCK     *RTC_TimerCtrlBlkexpiredTail = VOS_NULL_PTR;
    VOS_UINT32                  ulTempCount;

    if (VOS_NULL_PTR == RTC_Timer_head_Ptr)
    {
        return;
    }

    head_Ptr = RTC_Timer_head_Ptr;

    /* sub timer value */
    head_Ptr->TimeOutValueInCycle -= ulElapsedCycles;

    ulTempCount = 0;

    /* check the left timer */
    while ( ( VOS_NULL_PTR != head_Ptr )
        && ( 0 == head_Ptr->TimeOutValueInCycle ) )
    {
        ulTempCount++;
        if ( RTC_TimerCtrlBlkNumber < ulTempCount )
        {
            return;
        }

        RTC_TimerCtrlBlkCurrent = head_Ptr;

        *(head_Ptr->phTm) = VOS_NULL_PTR;

        head_Ptr = RTC_TimerCtrlBlkCurrent->next;

        RTC_TimerCtrlBlkCurrent->next = VOS_NULL_PTR;
        if ( VOS_NULL_PTR == RTC_TimerCtrlBlkexpired )
        {
            RTC_TimerCtrlBlkexpired = RTC_TimerCtrlBlkCurrent;
            RTC_TimerCtrlBlkexpiredTail = RTC_TimerCtrlBlkCurrent;
        }
        else
        {
            /*lint -e613 */
            RTC_TimerCtrlBlkexpiredTail->next = RTC_TimerCtrlBlkCurrent;/* [false alarm]: 屏蔽Fortify 错误 */
            /*lint +e613 */
            RTC_TimerCtrlBlkexpiredTail = RTC_TimerCtrlBlkCurrent;
        }

        RTC_Timer_head_Ptr = head_Ptr;
    }

    if ( VOS_NULL_PTR != RTC_Timer_head_Ptr )
    {
        RTC_Timer_head_Ptr->previous = VOS_NULL_PTR;

        /* 上面已经把为0的都过滤了，这里不会再有为0的 */
        if (0 == RTC_Timer_head_Ptr->TimeOutValueInCycle)
        {
            RTC_Timer_head_Ptr->TimeOutValueInCycle += 1;
        }

        StartHardTimer(RTC_Timer_head_Ptr->TimeOutValueInCycle);
    }

    if ( VOS_NULL_PTR != RTC_TimerCtrlBlkexpired )
    {
        if (VOS_OK != VOS_FixedQueueWriteDirect(g_ulRTCTaskQueueId, (VOS_VOID *)RTC_TimerCtrlBlkexpired, VOS_NORMAL_PRIORITY_MSG))
        {
            g_stRtcSocTimerInfo.ulExpiredSendErrCount++;
            g_stRtcSocTimerInfo.ulExpiredSendErrSlice   = VOS_GetSlice();

            RTC_DebugSocInfo((VOS_UINT32)RTC_SOC_TIMER_SEND_ERR, VOS_GetSlice());
        }
    }

    return;
}

/*****************************************************************************
 Function   : RTC_DualTimerIsr
 Description: ISR of DualTimer
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_INT32 RTC_DualTimerIsr(VOS_INT lPara)
{
    VOS_UINT32      ulCurrentSlice;
    VOS_ULONG       ulLockLevel;
    VOS_UINT32      ulElapsedCycles;

    VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

    g_stRtcSocTimerInfo.ulExpireCount++;

    ulCurrentSlice = VOS_GetSlice();

    if ( (ulCurrentSlice - g_stRtcSocTimerInfo.ulStartSlice) < RTC_Start_Value )
    {
        g_stRtcSocTimerInfo.ulExpiredShortErrCount++;
        g_stRtcSocTimerInfo.ulExpiredShortErrSlice  = VOS_GetSlice();
    }

    if ( (ulCurrentSlice - g_stRtcSocTimerInfo.ulStartSlice) > (RTC_Start_Value + RTC_SOC_TIMER_CHECK_PRECISION) )
    {
        g_stRtcSocTimerInfo.ulExpiredLongErrCount++;
        g_stRtcSocTimerInfo.ulExpiredLongErrSlice  = VOS_GetSlice();
    }

    ulElapsedCycles = GetHardTimerElapsedTime();

    if ( VOS_OK != mdrv_timer_stop((unsigned int)lPara) )
    {
        g_stRtcSocTimerInfo.ulStopErrCount++;
    }

    RTC_DebugSocInfo((VOS_UINT32)RTC_SOC_TIMER_EXPIRED, ulCurrentSlice);

    RTC_DualTimerIsrEntry(ulElapsedCycles);

    VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

    return 0;
}

/*****************************************************************************
 Function   : StopHardTimer
 Description: stop hard timer
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID StopHardTimer(VOS_VOID)
{
    g_stRtcSocTimerInfo.ulStopCount++;

    if ( VOS_OK != mdrv_timer_stop(VOS_RTC_TIMER_ID) )
    {
        g_stRtcSocTimerInfo.ulStopErrCount++;
    }

    RTC_Start_Value = ELAPESD_TIME_INVAILD;

    RTC_DebugSocInfo((VOS_UINT32)RTC_SOC_TIMER_STOPED, VOS_GetSlice());

    return;
}

/*****************************************************************************
 Function   : StartHardTimer
 Description: start hard timer
 Input      : value -- timer's value.uint is 32K cycle.
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID StartHardTimer( VOS_UINT32 value )
{
    g_stRtcSocTimerInfo.ulStartCount++;

    StopHardTimer();

    g_stRtcSocTimerInfo.ulStartSlice = VOS_GetSlice();

    RTC_Start_Value = value;

    if ( VOS_OK != mdrv_timer_start(VOS_RTC_TIMER_ID, (FUNCPTR_1)RTC_DualTimerIsr, VOS_RTC_TIMER_ID, value, TIMER_ONCE_COUNT,TIMER_UNIT_NONE) )
    {
        g_stRtcSocTimerInfo.ulStartErrCount++;
    }

    RTC_DebugSocInfo(value, g_stRtcSocTimerInfo.ulStartSlice);

    return;
}

/*****************************************************************************
 Function   : GetHardTimerElapsedTime
 Description: get the elapsed time from hard timer
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_UINT32 GetHardTimerElapsedTime(VOS_VOID)
{
    VOS_UINT32 ulTempValue = 0;

    if ( ELAPESD_TIME_INVAILD == RTC_Start_Value )
    {
        return 0;
    }

    if ( VOS_OK != mdrv_timer_get_rest_time(VOS_RTC_TIMER_ID, TIMER_UNIT_NONE, (VOS_UINT*)&ulTempValue) )
    {
        g_stRtcSocTimerInfo.ulElapsedErrCount++;
    }

    if ( RTC_Start_Value < ulTempValue )
    {
        g_stRtcSocTimerInfo.ulElapsedContentErrCount++;
        g_stRtcSocTimerInfo.ulElapsedContentErrSlice    = VOS_GetSlice();

        return RTC_Start_Value;
    }

    return RTC_Start_Value - ulTempValue;
}

/*****************************************************************************
 Function   : RTC_TimerCtrlBlkInit
 Description: Init timer's control block
 Input      : ulTimerCtrlBlkNumber -- number
 Return     : VOS_OK on success or errno on failure.
 Other      :
 *****************************************************************************/
VOS_UINT32 RTC_TimerCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32                i;

    RTC_TimerCtrlBlkNumber  = RTC_MAX_TIMER_NUMBER;
    RTC_TimerIdleCtrlBlkNumber = RTC_MAX_TIMER_NUMBER;

    RTC_TimerCtrlBlk = (RTC_TIMER_CONTROL_BLOCK*)g_acRtcTimerCtrlBuf;

    RTC_TimerIdleCtrlBlk = RTC_TimerCtrlBlk;
    RTC_TimerCtrlBlkBegin = (VOS_VOID *)RTC_TimerCtrlBlk;
    RTC_TimerCtrlBlkEnd  = (VOS_VOID*)( (VOS_UINT_PTR)(RTC_TimerCtrlBlk) +
        RTC_TimerCtrlBlkNumber * sizeof(RTC_TIMER_CONTROL_BLOCK) );

    for(i=0; i<RTC_TimerCtrlBlkNumber-1; i++)
    {
        RTC_TimerCtrlBlk[i].State        = RTC_TIMER_CTRL_BLK_STOP;
        RTC_TimerCtrlBlk[i].ulUsedFlag   = VOS_NOT_USED;
        RTC_TimerCtrlBlk[i].TimerId      = i;
        RTC_TimerCtrlBlk[i].ulBackUpTimerId = i;
        RTC_TimerCtrlBlk[i].phTm         = VOS_NULL_PTR;
        RTC_TimerCtrlBlk[i].CallBackFunc = VOS_NULL_PTR;
        RTC_TimerCtrlBlk[i].previous     = VOS_NULL_PTR;
        /*lint -e679*/
        RTC_TimerCtrlBlk[i].next         = &(RTC_TimerCtrlBlk[i+1]);
        /*lint +e679*/
    }

    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].State        = RTC_TIMER_CTRL_BLK_STOP;
    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].ulUsedFlag   = VOS_NOT_USED;
    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].TimerId      = RTC_TimerCtrlBlkNumber-1;
    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].ulBackUpTimerId = RTC_TimerCtrlBlkNumber-1;
    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].phTm         = VOS_NULL_PTR;
    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].CallBackFunc = VOS_NULL_PTR;
    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].previous     = VOS_NULL_PTR;
    RTC_TimerCtrlBlk[RTC_TimerCtrlBlkNumber-1].next         = VOS_NULL_PTR;

    RTC_TimerMinTimerIdUsed             = RTC_TimerCtrlBlkNumber;

    if ( VOS_NULL_PTR == VOS_MemSet_s(&g_stRtcSocTimerInfo, sizeof(VOS_TIMER_SOC_TIMER_INFO_STRU), 0x0, sizeof(VOS_TIMER_SOC_TIMER_INFO_STRU)) )
    {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    if ( VOS_NULL_PTR == VOS_MemSet_s((VOS_VOID *)g_astRtcSocTimerDebugInfo, sizeof(g_astRtcSocTimerDebugInfo), 0x0, sizeof(g_astRtcSocTimerDebugInfo)) )
    {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    /* 1 -> only one queue */
    if ( VOS_OK != VOS_FixedQueueCreate( VOS_FID_QUEUE_LENGTH, &g_ulRTCTaskQueueId, VOS_MSG_Q_FIFO, VOS_FID_MAX_MSG_LENGTH, 1 ) )
    {
        VOS_ProtectionReboot(VOS_ERRNO_RELTM_CTRLBLK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }

    mdrv_timer_debug_register(VOS_RTC_TIMER_ID, (FUNCPTR_1)VOS_TimerLpmCb, 0);

    /* Added by g47350 for DRX timer Project, 2012/11/5, begin */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if ( VOS_OK != VOS_DrxTimerCtrlBlkInit())
    {
        VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_RESOURCE_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif
    /* Added by g47350 for DRX timer Project, 2012/11/5, end */

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if ( VOS_OK != VOS_Bit64RtcTimerCtrlBlkInit() )
    {
        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_RESOURCE_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif

    return(VOS_OK);
}

/*****************************************************************************
 Function   : RTC_TimerCtrlBlkGet
 Description: allocte a block
 Input      : void
 Return     : address
 Other      :
 *****************************************************************************/
RTC_TIMER_CONTROL_BLOCK *RTC_TimerCtrlBlkGet(VOS_UINT32 ulFileID, VOS_INT32 usLineNo)
{
    RTC_TIMER_CONTROL_BLOCK  *temp_Timer_Ctrl_Ptr;

    VOS_VOID                 *pDumpBuffer;


    if( 0 == RTC_TimerIdleCtrlBlkNumber )
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_SYSTIMER_FULL);

        pDumpBuffer = (VOS_VOID*)VOS_EXCH_MEM_MALLOC;

        if (VOS_NULL_PTR == pDumpBuffer)
        {
            return((RTC_TIMER_CONTROL_BLOCK*)VOS_NULL_PTR);
        }

        if ( VOS_NULL_PTR == VOS_MemSet_s(pDumpBuffer, VOS_DUMP_MEM_TOTAL_SIZE, 0, VOS_DUMP_MEM_TOTAL_SIZE) )
        {
            mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
        }

        /* 防止拷贝内存越界，取最小值 */
        /*lint -e506 */
        if ( VOS_NULL_PTR == VOS_MemCpy_s(pDumpBuffer, VOS_DUMP_MEM_TOTAL_SIZE, (VOS_VOID *)g_acRtcTimerCtrlBuf,
                   ((VOS_DUMP_MEM_TOTAL_SIZE < RTC_TIMER_CTRL_BUF_SIZE) ? VOS_DUMP_MEM_TOTAL_SIZE : RTC_TIMER_CTRL_BUF_SIZE )) )
        {
            mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
        }
        /*lint +e506 */

        return((RTC_TIMER_CONTROL_BLOCK*)VOS_NULL_PTR);
    }
    else
    {
        RTC_TimerIdleCtrlBlkNumber--;

        temp_Timer_Ctrl_Ptr = RTC_TimerIdleCtrlBlk;
        temp_Timer_Ctrl_Ptr->ulUsedFlag = VOS_USED;
        RTC_TimerIdleCtrlBlk = RTC_TimerIdleCtrlBlk->next;
    }

    /* record the usage of timer control block */
    if ( RTC_TimerIdleCtrlBlkNumber < RTC_TimerMinTimerIdUsed )
    {
        RTC_TimerMinTimerIdUsed = RTC_TimerIdleCtrlBlkNumber;
    }

    temp_Timer_Ctrl_Ptr->next = VOS_NULL_PTR;
    temp_Timer_Ctrl_Ptr->previous = VOS_NULL_PTR;

#if VOS_YES == VOS_TIMER_CHECK
        temp_Timer_Ctrl_Ptr->ulFileID = ulFileID;
        temp_Timer_Ctrl_Ptr->ulLineNo = (VOS_UINT32)usLineNo;
        temp_Timer_Ctrl_Ptr->ulAllocTick = VOS_GetSlice();
#endif

    return temp_Timer_Ctrl_Ptr;
}

/*****************************************************************************
 Function   : RTC_TimerCtrlBlkFree
 Description: free a block
 Input      : Ptr -- address
              ucTag -- where call this function.this should be deleted when release
 Return     : void
 Other      :
 *****************************************************************************/
VOS_UINT32 RTC_TimerCtrlBlkFree(RTC_TIMER_CONTROL_BLOCK *Ptr, VOS_UINT8 ucTag )
{
    if ( (VOS_UINT_PTR)Ptr < (VOS_UINT_PTR)RTC_TimerCtrlBlkBegin
        || (VOS_UINT_PTR)Ptr > (VOS_UINT_PTR)RTC_TimerCtrlBlkEnd )
    {
        return VOS_ERR;
    }

    if ( VOS_NOT_USED == Ptr->ulUsedFlag )
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_FREE_RECEPTION);

        return VOS_RTC_ERRNO_RELTM_FREE_RECEPTION;
    }

    Ptr->ulUsedFlag = VOS_NOT_USED;
    Ptr->Reserved[0] = ucTag;
    Ptr->next = RTC_TimerIdleCtrlBlk;
    RTC_TimerIdleCtrlBlk = Ptr;

    RTC_TimerIdleCtrlBlkNumber++;

#if VOS_YES == VOS_TIMER_CHECK
    Ptr->ulFreeTick = VOS_GetSlice();
#endif

    return VOS_OK;
}

/*****************************************************************************
 Function   : RTC_TimerTaskFunc
 Description: RTC timer task entry
 Input      : void
 Return     : void
 Other      :
 *****************************************************************************/
VOS_VOID RTC_TimerTaskFunc( VOS_UINT32 Para0, VOS_UINT32 Para1,
                            VOS_UINT32 Para2, VOS_UINT32 Para3 )
{
    RTC_TIMER_CONTROL_BLOCK     *head_Ptr;
    RTC_TIMER_CONTROL_BLOCK     *RTC_TimerCtrlBlkexpired = VOS_NULL_PTR;
    VOS_ULONG                   ulLockLevel;
    REL_TIMER_MSG               *pstExpireMsg;
    VOS_UINT32                  ulCurrentSlice;
    VOS_UINT32                  ulIntervalSlice;
    VOS_UINT_PTR                ulCtrlBlkAddress;
    VOS_UINT_PTR                TempValue;
#if ( (OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU) )
    RTC_TIMER_PMLOG             stPmLog;
#endif

#if (VOS_WIN32 == VOS_OS_VER)
    VOS_UINT32                  i;

    for(i = 0; i < 1; i++)
#else
    for(;;)
#endif
    {
        if (VOS_ERR == VOS_FixedQueueRead(g_ulRTCTaskQueueId, 0, &ulCtrlBlkAddress, VOS_FID_MAX_MSG_LENGTH))
        {
            continue;
        }

        ulCurrentSlice = VOS_GetSlice();

        RTC_TimerCtrlBlkexpired = (RTC_TIMER_CONTROL_BLOCK *)ulCtrlBlkAddress;

        while ( VOS_NULL_PTR != RTC_TimerCtrlBlkexpired )
        {
            head_Ptr = RTC_TimerCtrlBlkexpired;

            if( VOS_RELTIMER_LOOP == head_Ptr->Mode )
            {
                if ( VOS_NULL_PTR == head_Ptr->CallBackFunc )
                {
                   (VOS_VOID)V_Start32KRelTimer( head_Ptr->phTm,
                               head_Ptr->Pid,
                               head_Ptr->TimeOutValueInMilliSeconds,
                               head_Ptr->Name,
                               head_Ptr->Para,
                               VOS_RELTIMER_LOOP,
                               head_Ptr->ulPrecision,
#if VOS_YES == VOS_TIMER_CHECK
                               head_Ptr->ulFileID,
                               (VOS_INT32)head_Ptr->ulLineNo);
#else
                               VOS_FILE_ID,
                               (VOS_INT32)__LINE__);
#endif
                }
                else
                {
                    (VOS_VOID)V_Start32KCallBackRelTimer( head_Ptr->phTm,
                               head_Ptr->Pid,
                               head_Ptr->TimeOutValueInMilliSeconds,
                               head_Ptr->Name,
                               head_Ptr->Para,
                               VOS_RELTIMER_LOOP,
                               head_Ptr->CallBackFunc,
                               head_Ptr->ulPrecision,
#if VOS_YES == VOS_TIMER_CHECK
                               head_Ptr->ulFileID,
                               (VOS_INT32)head_Ptr->ulLineNo);
#else
                               VOS_FILE_ID,
                               (VOS_INT32)__LINE__);
#endif
                }
            }

            TempValue = (VOS_UINT_PTR)(RTC_TimerCtrlBlkexpired->CallBackFunc);

            /* CallBackFunc需要用32位传入，所以和name互换位置保证数据不丢失 */
            OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_3, (VOS_UINT32)(RTC_TimerCtrlBlkexpired->Pid), RTC_TimerCtrlBlkexpired->Name, (VOS_UINT32)TempValue);

            if ( VOS_NULL_PTR == RTC_TimerCtrlBlkexpired->CallBackFunc )
            {
                /* Alloc expires's Msg */
                pstExpireMsg
                    = VOS_TimerPreAllocMsg(RTC_TimerCtrlBlkexpired->Pid);

                if ( VOS_NULL_PTR != pstExpireMsg )
                {
                    pstExpireMsg->ulName = RTC_TimerCtrlBlkexpired->Name;
                    pstExpireMsg->ulPara = RTC_TimerCtrlBlkexpired->Para;
                    TempValue            = (VOS_UINT_PTR)(RTC_TimerCtrlBlkexpired->TimeOutValueInMilliSeconds);
                    pstExpireMsg->hTm    = (HTIMER)TempValue;

#if (VOS_YES == VOS_TIMER_CHECK)
                    TempValue            = (VOS_UINT_PTR)RTC_TimerCtrlBlkexpired->ulAllocTick;

                    pstExpireMsg->pNext
                        = (struct REL_TIMER_MSG_STRU *)TempValue;

                    TempValue            = (VOS_UINT_PTR)ulCurrentSlice;

                    pstExpireMsg->pPrev
                        = (struct REL_TIMER_MSG_STRU *)TempValue;

                    ulIntervalSlice = (RTC_TimerCtrlBlkexpired->ulBackUpTimeOutValueInCycle - RTC_TimerCtrlBlkexpired->ulPrecisionInCycle);

                    ulIntervalSlice = ( (ulIntervalSlice > RTC_TIMER_CHECK_PRECISION) ? (ulIntervalSlice - RTC_TIMER_CHECK_PRECISION) : ulIntervalSlice );

                    if ( (ulCurrentSlice - RTC_TimerCtrlBlkexpired->ulAllocTick) < ulIntervalSlice )
                    {
#if ( (OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU) )
    #if (FEATURE_ON == FEATURE_RTC_TIMER_DBG)
                        RTC_SocTimerMemDump();

                        VOS_ProtectionReboot(RTC_TIMER_EXPIRED_TOO_SHORT, 0, (VOS_INT)ulCurrentSlice, (VOS_CHAR *)RTC_TimerCtrlBlkexpired, sizeof(RTC_TIMER_CONTROL_BLOCK));
    #endif
#endif
                    }

                    ulIntervalSlice = (RTC_TimerCtrlBlkexpired->ulBackUpTimeOutValueInCycle + RTC_TimerCtrlBlkexpired->ulPrecisionInCycle);

                    if ( (ulCurrentSlice - RTC_TimerCtrlBlkexpired->ulAllocTick) > (ulIntervalSlice + RTC_TIMER_CHECK_LONG_PRECISION) )
                    {
#if ( (OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU) )
    #if (FEATURE_ON == FEATURE_RTC_TIMER_DBG)

    #endif
#endif
                    }
#endif
                    (VOS_VOID)VOS_SendMsg(DOPRA_PID_TIMER, pstExpireMsg);

                }
            }
            else
            {
                RTC_TimerCtrlBlkexpired->CallBackFunc(
                    RTC_TimerCtrlBlkexpired->Para,
                    RTC_TimerCtrlBlkexpired->Name);
            }

            OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_3);

#if ( (OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU) )
            if(VOS_TRUE == g_ulTimerlpm)
            {
                g_ulTimerlpm = VOS_FALSE;

                stPmLog.stPamType   = PM_LOG_COSA_PAM_TIMER;
                stPmLog.ulPid       = RTC_TimerCtrlBlkexpired->Pid;
                stPmLog.ulTimerId   = RTC_TimerCtrlBlkexpired->Name;

                (VOS_VOID)mdrv_pm_log(PM_MOD_CP_OSA, sizeof(RTC_TIMER_PMLOG), &stPmLog);
            }
#else
            if(VOS_TRUE == g_ulTimerlpm)
            {
                g_ulTimerlpm = VOS_FALSE;

                mdrv_err("<RTC_TimerTaskFunc> rtc_timer: allocpid=%d, timername = 0x%x.\n",
                    RTC_TimerCtrlBlkexpired->Pid, RTC_TimerCtrlBlkexpired->Name);
            }

#endif

            RTC_TimerCtrlBlkexpired = RTC_TimerCtrlBlkexpired->next;

            VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

            (VOS_VOID)RTC_TimerCtrlBlkFree(head_Ptr, THE_FIRST_RTC_TIMER_TAG);

            VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);
        }
    }
}

/*****************************************************************************
 Function   : RTC_TimerTaskCreat
 Description: create RTC timer task
 Input      : void
 Return     : VOS_OK on success or errno on failure.
 Other      :
 *****************************************************************************/
VOS_UINT32 RTC_TimerTaskCreat(VOS_VOID)
{
    VOS_UINT32 TimerArguments[4] = {0,0,0,0};

    if ( VOS_OK != VOS_CreateTask( "RTC_TIMER",
                            &RTC_TimerTaskId,
                            RTC_TimerTaskFunc,
                            COMM_RTC_TIMER_TASK_PRIO,
                            RTC_TIMER_TASK_STACK_SIZE,
                            TimerArguments) )
    {
        VOS_ProtectionReboot(VOS_ERRNO_RELTM_TASK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }

    /* Added by g47350 for DRX timer Project, 2012/11/5, begin */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if ( VOS_OK != VOS_DrxTimerTaskCreat() )
    {
        VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_TASK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif
    /* Added by g47350 for DRX timer Project, 2012/11/5, end */

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
    if ( VOS_OK != VOS_Bit64TimerTaskCreat() )
    {
        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_TASK_INITFAIL, 0, 0, 0, 0);

        return VOS_ERR;
    }
#endif

    return VOS_OK;
}

/*****************************************************************************
 Function   : RTC_Add_Timer_To_List
 Description: add a timer to list
 Input      : Timer -- the tiemr's adddress
 Return     : void
 Other      :
 *****************************************************************************/
VOS_VOID RTC_Add_Timer_To_List( RTC_TIMER_CONTROL_BLOCK  *Timer)
{
    RTC_TIMER_CONTROL_BLOCK  *temp_Ptr;
    RTC_TIMER_CONTROL_BLOCK  *pre_temp_Ptr;
    VOS_UINT32               ElapsedCycles = 0;

    if ( VOS_NULL_PTR == RTC_Timer_head_Ptr )
    {
        RTC_Timer_head_Ptr = Timer;

        Timer->Reserved[1] = VOS_ADD_TIMER_TAG_1;
    }
    else
    {
        ElapsedCycles = GetHardTimerElapsedTime();

        Timer->TimeOutValueInCycle += ElapsedCycles;

        /*  find the location to insert */
        temp_Ptr = pre_temp_Ptr = RTC_Timer_head_Ptr;

        while ( temp_Ptr != VOS_NULL_PTR )
        {
            if (Timer->TimeOutValueInCycle >= temp_Ptr->TimeOutValueInCycle)
            {
                Timer->TimeOutValueInCycle -= temp_Ptr->TimeOutValueInCycle;

                if ( Timer->TimeOutValueInCycle <= Timer->ulPrecisionInCycle )
                {
                    /* forward adjust; do nothindg when TimeOutValueInCycle == 0 */
                    Timer->TimeOutValueInCycle = 0;
                    Timer->Reserved[1] = VOS_ADD_TIMER_TAG_2;

                    pre_temp_Ptr = temp_Ptr;
                    temp_Ptr = temp_Ptr->next;

                    while ( (temp_Ptr != VOS_NULL_PTR)
                        && (Timer->TimeOutValueInCycle == temp_Ptr->TimeOutValueInCycle) )
                    {
                        pre_temp_Ptr = temp_Ptr;
                        temp_Ptr = temp_Ptr->next;
                    }/* make sure the order of expiry */

                    break;
                }

                pre_temp_Ptr = temp_Ptr;
                temp_Ptr = temp_Ptr->next;

                Timer->Reserved[1] = VOS_ADD_TIMER_TAG_3;
            }
            else
            {
                if ( temp_Ptr->TimeOutValueInCycle - Timer->TimeOutValueInCycle <= Timer->ulPrecisionInCycle )
                {
                    /* backward adjust */
                    Timer->TimeOutValueInCycle = 0;
                    Timer->Reserved[1] = VOS_ADD_TIMER_TAG_4;

                    pre_temp_Ptr = temp_Ptr;
                    temp_Ptr = temp_Ptr->next;

                    while ( (temp_Ptr != VOS_NULL_PTR)
                        && (Timer->TimeOutValueInCycle == temp_Ptr->TimeOutValueInCycle) )
                    {
                        pre_temp_Ptr = temp_Ptr;
                        temp_Ptr = temp_Ptr->next;
                    }/* make sure the order of expiry */

                    break;
                }

                Timer->Reserved[1] = VOS_ADD_TIMER_TAG_5;

                /* can't adjust */
                break;
            }
        }

        /* insert timer < head timer*/
        if ( temp_Ptr == RTC_Timer_head_Ptr )
        {
            Timer->next = RTC_Timer_head_Ptr;
            RTC_Timer_head_Ptr = Timer;
        }
        else
        {
            Timer->next = temp_Ptr;
            pre_temp_Ptr->next = Timer;
            Timer->previous = pre_temp_Ptr;
        }

        if ( temp_Ptr != VOS_NULL_PTR )
        {
            temp_Ptr->TimeOutValueInCycle
                = temp_Ptr->TimeOutValueInCycle - Timer->TimeOutValueInCycle;
            temp_Ptr->previous = Timer;
        }
    }

    /* restart RTC timer */
    if ( RTC_Timer_head_Ptr == Timer)
    {
        /* judge timer value when the new timer at head */
        Timer->TimeOutValueInCycle -= ElapsedCycles;

        if (0 == Timer->TimeOutValueInCycle)
        {
            Timer->TimeOutValueInCycle += 1;
        }

        StartHardTimer(Timer->TimeOutValueInCycle);
    }

}

/*****************************************************************************
 Function   : RTC_Del_Timer_From_List
 Description: del a timer from list
 Input      : Timer -- the timer's address
 Return     : void
 Other      :
 *****************************************************************************/
VOS_VOID RTC_Del_Timer_From_List( RTC_TIMER_CONTROL_BLOCK  *Timer)
{
    VOS_BOOL                 bIsHead = VOS_FALSE;

    /* deletet this timer from list */
    if ( Timer == RTC_Timer_head_Ptr )
    {
        bIsHead = VOS_TRUE;

        RTC_Timer_head_Ptr = Timer->next;
        if ( VOS_NULL_PTR != RTC_Timer_head_Ptr )
        {
            RTC_Timer_head_Ptr->previous = VOS_NULL_PTR;
        }
    }
    else
    {
        (Timer->previous)->next = Timer->next;
        if ( VOS_NULL_PTR != Timer->next )
        {
            (Timer->next)->previous = Timer->previous;
        }
    }

    /* adjust the time_val after this timer */
    if ( Timer->next != NULL )
    {
        Timer->next->TimeOutValueInCycle += Timer->TimeOutValueInCycle;

        if (VOS_TRUE == bIsHead)
        {
            Timer->next->TimeOutValueInCycle -= GetHardTimerElapsedTime();

            if (0 == Timer->next->TimeOutValueInCycle)
            {
                Timer->next->TimeOutValueInCycle += 1;
            }

            StartHardTimer(Timer->next->TimeOutValueInCycle);
        }
    }

    /* Stop timer3 if no timer */
    if ( VOS_NULL_PTR == RTC_Timer_head_Ptr )
    {
        StopHardTimer();
    }

}

/*****************************************************************************
 Function   : R_Stop32KTimer
 Description: stop a 32K relative timer which was previously started.
 Input      : phTm -- where store the timer to be stopped
 Return     :  VOS_OK on success or errno on failure
 *****************************************************************************/
VOS_UINT32 R_Stop32KTimer( HTIMER *phTm, VOS_UINT32 ulFileID, VOS_INT32 usLineNo, VOS_TIMER_OM_EVENT_STRU *pstEvent )
{
    VOS_UINT32               TimerId = 0;
    RTC_TIMER_CONTROL_BLOCK  *Timer;

    if( VOS_NULL_PTR == *phTm )
    {
        return VOS_OK;
    }

    if ( VOS_OK != RTC_CheckTimer(phTm, &TimerId, ulFileID, usLineNo ) )
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_STOP_TIMERINVALID);
        return(VOS_RTC_ERRNO_RELTM_STOP_TIMERINVALID);
    }

    Timer = &RTC_TimerCtrlBlk[TimerId];

    /* del the timer from the running list */
    RTC_Del_Timer_From_List( Timer );

    *(Timer->phTm) = VOS_NULL_PTR;

    /* OM */
    if ( VOS_NULL_PTR != pstEvent )
    {
        pstEvent->ucMode      = Timer->Mode;
        pstEvent->Pid         = Timer->Pid;
        pstEvent->ulLength    = Timer->TimeOutValueInMilliSeconds;
        pstEvent->ulName      = Timer->Name;
        pstEvent->ulParam     = Timer->Para;
        pstEvent->enPrecision = (VOS_TIMER_PRECISION_ENUM_UINT32)Timer->ulPrecision;
    }

    return RTC_TimerCtrlBlkFree(Timer, THE_SECOND_RTC_TIMER_TAG);
}

/*****************************************************************************
 Function     : R_Get32KRelTmRemainTime
 Description  : get left time
 Input Param  : phTm
 Output       : pulTime
 Return Value : VOS_OK on success or errno on failure
*****************************************************************************/
VOS_UINT32 R_Get32KRelTmRemainTime( HTIMER * phTm, VOS_UINT32 * pulTime,
                                 VOS_UINT32 ulFileID, VOS_INT32 usLineNo )
{
    VOS_UINT32                  TimerId      = 0;
    VOS_UINT32                  remain_value = 0;
    RTC_TIMER_CONTROL_BLOCK     *head_Ptr;
    RTC_TIMER_CONTROL_BLOCK     *temp_Ptr;
    VOS_UINT32                  ulTempValue;

    if( VOS_NULL_PTR == *phTm )
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_STOP_INPUTISNULL);
        return(VOS_RTC_ERRNO_RELTM_STOP_INPUTISNULL);
    }

    if ( VOS_OK != RTC_CheckTimer(phTm, &TimerId, ulFileID, usLineNo ) )
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_STOP_TIMERINVALID);
        return(VOS_RTC_ERRNO_RELTM_STOP_TIMERINVALID);
    }

    head_Ptr = RTC_Timer_head_Ptr;

    while ( (VOS_NULL_PTR != head_Ptr) && (head_Ptr->TimerId != TimerId) )
    {
        remain_value += (head_Ptr->TimeOutValueInCycle);

        temp_Ptr = head_Ptr;
        head_Ptr = temp_Ptr->next;
    }

    if ( (VOS_NULL_PTR == head_Ptr) || ( head_Ptr->TimerId != TimerId) )
    {
        return VOS_ERR;
    }
    else
    {
        remain_value += (head_Ptr->TimeOutValueInCycle);

        ulTempValue = GetHardTimerElapsedTime();

        *pulTime
            = (VOS_UINT32)RTC_DIV_32_DOT_768((remain_value - ulTempValue), ulFileID, usLineNo);

        return(VOS_OK);
    }
}


VOS_UINT32 RTC_CheckTimer( HTIMER  *phTm, VOS_UINT32 *ulTimerID,
                           VOS_UINT32 ulFileID, VOS_INT32 usLineNo )
{
    RTC_TIMER_CONTROL_BLOCK  *Timer;

    if ( ((VOS_UINT_PTR)*phTm >= (VOS_UINT_PTR)RTC_TimerCtrlBlkBegin)
        && ((VOS_UINT_PTR)*phTm < (VOS_UINT_PTR)RTC_TimerCtrlBlkEnd) )
    {
        Timer = (RTC_TIMER_CONTROL_BLOCK *)(*phTm);

        if ( phTm == Timer->phTm )
        {
            *ulTimerID = Timer->ulBackUpTimerId;

            if ( Timer->ulBackUpTimerId != Timer->TimerId)
            {
                Timer->TimerId = Timer->ulBackUpTimerId;
            }

            return VOS_OK;
        }

        VOS_SIMPLE_FATAL_ERROR(RTC_CHECK_TIMER_ID);
    }
    else
    {
        VOS_ProtectionReboot(RTC_CHECK_TIMER_RANG, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)phTm, sizeof(VOS_CHAR *));
    }

    return VOS_ERR;
}


/*****************************************************************************
 Function   : V_Start32KCallBackRelTimer
 Description: allocate and start a relative timer using callback function.
 Input      : Pid           -- process ID of application
              ulLength       -- expire time. unit is millsecond
              ulName         -- timer name to be pass to app as a parameter
              ulParam        -- additional parameter to be pass to app
              ucMode         -- timer work mode
                                VOS_RELTIMER_LOOP  -- start periodically
                                VOS_RELTIMER_NOLOO -- start once time
              TimeOutRoutine -- Callback function when time out
              ulPrecision    -- precision,unit is 0 - 100->0%- 100%
 Output     : phTm           -- timer pointer which system retuns to app
 Return     : VOS_OK on success and errno on failure
 *****************************************************************************/
VOS_UINT32 V_Start32KCallBackRelTimer( HTIMER *phTm, VOS_PID Pid, VOS_UINT32 ulLength,
    VOS_UINT32 ulName, VOS_UINT32 ulParam, VOS_UINT8 ucMode, REL_TIMER_FUNC TimeOutRoutine,
    VOS_UINT32 ulPrecision, VOS_UINT32 ulFileID, VOS_INT32 usLineNo )
{
    RTC_TIMER_CONTROL_BLOCK  *Timer;
    VOS_UINT32               TimerId = 0;
    VOS_ULONG                ulLockLevel;

    /* stop the timer if exists */
    VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

    if( VOS_NULL_PTR != *phTm )
    {
        if ( VOS_OK == RTC_CheckTimer(phTm, &TimerId, ulFileID, usLineNo) )
        {
            if ( VOS_OK != R_Stop32KTimer( phTm, ulFileID, usLineNo, VOS_NULL_PTR ) )
            {
                VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

                VOS_SIMPLE_FATAL_ERROR(START_32K_CALLBACK_RELTIMER_FAIL_TO_STOP);

                return VOS_ERR;
            }
        }
    }

    Timer = RTC_TimerCtrlBlkGet(ulFileID, usLineNo);
    if(Timer == VOS_NULL_PTR)
    {
        VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(START_32K_CALLBACK_RELTIMER_FAIL_TO_ALLOCATE, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)(&g_stRtcSocTimerInfo), sizeof(VOS_TIMER_SOC_TIMER_INFO_STRU));

        return(VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);
    }

    Timer->Pid                          = Pid;
    Timer->Name                         = ulName;
    Timer->Para                         = ulParam;
    Timer->Mode                         = ucMode;
    Timer->phTm                         = phTm;
    Timer->TimeOutValueInMilliSeconds   = ulLength;
    Timer->ulPrecision                  = ulPrecision;
    Timer->TimeOutValueInCycle = (VOS_UINT32)RTC_MUL_32_DOT_768(ulLength, ulFileID, usLineNo);
    Timer->ulPrecisionInCycle = (VOS_UINT32)RTC_MUL_DOT_32768((ulLength * ulPrecision), ulFileID, usLineNo);
    Timer->ulBackUpTimeOutValueInCycle = Timer->TimeOutValueInCycle;
    Timer->CallBackFunc                 = TimeOutRoutine;
    *phTm = (HTIMER)(&(Timer->TimerId));

    RTC_Add_Timer_To_List( Timer );

    VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

    return(VOS_OK);
}

/*****************************************************************************
 Function   : V_Start32KRelTimer
 Description: allocate and start a relative timer using callback function.
 Input      : Pid           -- process ID of application
              ulLength       -- expire time. unit is millsecond
              ulName         -- timer name to be pass to app as a parameter
              ulParam        -- additional parameter to be pass to app
              ucMode         -- timer work mode
                                VOS_RELTIMER_LOOP  -- start periodically
                                VOS_RELTIMER_NOLOO -- start once time
              ulPrecision    -- precision,unit is 0 - 100->0%- 100%
 Output     : phTm           -- timer pointer which system retuns to app
 Return     : VOS_OK on success and errno on failure
 *****************************************************************************/
VOS_UINT32 V_Start32KRelTimer( HTIMER *phTm, VOS_PID Pid, VOS_UINT32 ulLength,
    VOS_UINT32 ulName, VOS_UINT32 ulParam, VOS_UINT8 ucMode, VOS_UINT32 ulPrecision,
    VOS_UINT32 ulFileID, VOS_INT32 usLineNo)
{
    RTC_TIMER_CONTROL_BLOCK  *Timer;
    VOS_UINT32               TimerId = 0;
    VOS_ULONG                ulLockLevel;

    /* stop the timer if exists */
    VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

    if( VOS_NULL_PTR != *phTm )
    {
        if ( VOS_OK == RTC_CheckTimer( phTm, &TimerId, ulFileID, usLineNo ) )
        {
            if ( VOS_OK != R_Stop32KTimer( phTm, ulFileID, usLineNo, VOS_NULL_PTR ) )
            {
                VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

                VOS_SIMPLE_FATAL_ERROR(START_32K_RELTIMER_FAIL_TO_STOP);

                return VOS_ERR;
            }
        }
    }

    Timer = RTC_TimerCtrlBlkGet(ulFileID, usLineNo);

    if( VOS_NULL_PTR == Timer )
    {
        VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(START_32K_CALLBACK_RELTIMER_FAIL_TO_ALLOCATE, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)(&g_stRtcSocTimerInfo), sizeof(VOS_TIMER_SOC_TIMER_INFO_STRU));

        return(VOS_RTC_ERRNO_RELTM_START_MSGNOTINSTALL);
    }

    Timer->Pid                          = Pid;
    Timer->Name                         = ulName;
    Timer->Para                         = ulParam;
    Timer->Mode                         = ucMode;
    Timer->phTm                         = phTm;
    Timer->TimeOutValueInMilliSeconds   = ulLength;
    Timer->ulPrecision                  = ulPrecision;
    Timer->TimeOutValueInCycle = (VOS_UINT32)RTC_MUL_32_DOT_768(ulLength, ulFileID, usLineNo);
    Timer->ulPrecisionInCycle = (VOS_UINT32)RTC_MUL_DOT_32768((ulLength * ulPrecision), ulFileID, usLineNo);
    Timer->ulBackUpTimeOutValueInCycle = Timer->TimeOutValueInCycle;
    Timer->CallBackFunc                 = VOS_NULL_PTR;

    *phTm = (HTIMER)(&(Timer->TimerId));

    /* add the timer to the running list */
    RTC_Add_Timer_To_List( Timer );

    VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

    return(VOS_OK);
}

/*****************************************************************************
 Function   : R_Restart32KRelTimer
 Description: Restart a relative timer which was previously started
 Input      : phTm -- where store timer ID to be restarted
 Return     : VOS_OK on success or errno on failure.
 Other      : the properties of timer should not be changed,
              but timer ID could have been changed
 *****************************************************************************/
VOS_UINT32 R_Restart32KRelTimer( HTIMER *phTm, VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo )
{
    VOS_UINT32               TimerId = 0;
    RTC_TIMER_CONTROL_BLOCK  *Timer;

    if( VOS_NULL_PTR == *phTm )
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);

        VOS_SIMPLE_FATAL_ERROR(RESTART_32K_RELTIMER_NULL);

        return(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);
    }

    if ( VOS_OK != RTC_CheckTimer( phTm, &TimerId, ulFileID, usLineNo ) )
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);

        VOS_SIMPLE_FATAL_ERROR(RESTART_32K_RELTIMER_FAIL_TO_CHECK);

        return(VOS_ERRNO_RELTM_RESTART_TIMERINVALID);
    }

    Timer   = &RTC_TimerCtrlBlk[TimerId];

    /* Del the old timer but not free timer control block */
    RTC_Del_Timer_From_List( Timer );

    /* reset timer value */
    Timer->TimeOutValueInCycle = (VOS_UINT32)RTC_MUL_32_DOT_768(Timer->TimeOutValueInMilliSeconds, ulFileID, usLineNo);
    Timer->ulBackUpTimeOutValueInCycle = Timer->TimeOutValueInCycle;

    Timer->next = VOS_NULL_PTR;
    Timer->previous = VOS_NULL_PTR;

    /* add the new timer to list */
    RTC_Add_Timer_To_List( Timer );

    return VOS_OK;
}

/*****************************************************************************
 Function   : RTC_timer_running
 Description: a APP RTC timer is running or not
 Input      : void
 Return     : true or false
 Other      :
 *****************************************************************************/
VOS_UINT32 RTC_timer_running( VOS_VOID )
{
    RTC_TIMER_CONTROL_BLOCK     *head_Ptr;
    VOS_ULONG                    ulLockLevel;

    VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

    head_Ptr = RTC_Timer_head_Ptr;

    if ( head_Ptr != VOS_NULL_PTR)
    {
        while ( VOS_NULL_PTR != head_Ptr )
        {
            if ( APP_PID != head_Ptr->Pid )
            {
                head_Ptr = head_Ptr->next;
            }
            else
            {
                VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

                return VOS_TRUE;
            }
        }

        VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

        return VOS_FALSE;
    }
    else
    {
        VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

        return VOS_FALSE;
    }
}

/*****************************************************************************
 Function   : VOS_GetSlice
 Description: get left time of the first timer.Unit is 30us
 Input      : void
 Return     :
 Other      :
 *****************************************************************************/
MODULE_EXPORTED VOS_UINT32 VOS_GetSlice(VOS_VOID)
{
#if (VOS_WIN32 != VOS_OS_VER)
    return mdrv_timer_get_normal_timestamp();
#else
    return VOS_GetTick();
#endif
}


MODULE_EXPORTED VOS_UINT64 VOS_Get64BitSlice(VOS_VOID)
{
#if (VOS_WIN32 != VOS_OS_VER)
    VOS_UINT32  ulHighBitValue = 0;
    VOS_UINT32  ulLowBitValue = 0;
    VOS_UINT64  ullSlice;
    VOS_INT     lResult;

    lResult = mdrv_timer_get_accuracy_timestamp(&ulHighBitValue, &ulLowBitValue);

    if (VOS_OK != lResult)
    {
        VOS_ProtectionReboot(VOS_GET_64BIT_SLICE_ERROR, (VOS_INT)ulHighBitValue, (VOS_INT)ulLowBitValue, (VOS_CHAR *)&lResult, sizeof(VOS_INT));
    }

    ullSlice = (((VOS_UINT64)ulHighBitValue<<32) & 0xffffffff00000000) | ((VOS_UINT64)ulLowBitValue & 0x00000000ffffffff);

    return ullSlice;
#else
    return VOS_GetTick();
#endif
}

/*****************************************************************************
 Function   : VOS_GetSliceUnit
 Description:
 Input      : void
 Return     :
 Other      :
 *****************************************************************************/
MODULE_EXPORTED VOS_UINT32 VOS_GetSliceUnit(VOS_VOID)
{

    return RTC_TIMER_CHECK_LONG_PRECISION;
}

/*****************************************************************************
 Function   : RTC_CalcTimerInfo
 Description: print the usage info of timer
 Input      : void
 Return     : void
 Other      :
 *****************************************************************************/
VOS_BOOL RTC_CalcTimerInfo(VOS_VOID)
{
    if ( RTC_UPPER_TIMER_NUMBER > RTC_TimerMinTimerIdUsed )
    {
        RTC_TimerMinTimerIdUsed = RTC_TimerCtrlBlkNumber;
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
#if VOS_YES == VOS_TIMER_CHECK

/*****************************************************************************
 Function   : VOS_ShowUsed32KTimerInfo
 Description: print the usage info of 32K timer's control block
 Input      : void
 Return     : void
 Other      :
 *****************************************************************************/
MODULE_EXPORTED VOS_VOID VOS_ShowUsed32KTimerInfo( VOS_VOID )
{
    VOS_ULONG                    ulLockLevel;
    RTC_TIMER_CONTROL_BLOCK     *pstTimer;

    mdrv_debug("<VOS_ShowUsed32KTimerInfo:");

    VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

    pstTimer = RTC_Timer_head_Ptr;
    while( VOS_NULL_PTR != pstTimer )
    {
        mdrv_debug("F=%d L=%d P=%d N=%d R=%d T=%d.\n",
               (VOS_INT)pstTimer->ulFileID,
               (VOS_INT)pstTimer->ulLineNo,
               (VOS_INT)pstTimer->Pid,
               (VOS_INT)pstTimer->Name,
               (VOS_INT)pstTimer->Para,
               (VOS_INT)pstTimer->ulAllocTick);

        pstTimer = pstTimer->next;
    }

    VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

    return;
}

#endif

/* Added by g47350 for DRX timer Project, 2012/11/5, begin */

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

VOS_UINT32 VOS_DrxCheckTimer( HTIMER *phTm, VOS_UINT32 ulFileID, VOS_INT32 lLineNo )
{
    VOS_UINT32                i;

    for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++)
    {
        if (*phTm == (HTIMER)(&g_astDRXTimerCtrlBlk[i]))
        {
            return VOS_OK;
        }
    }

    VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_ERROR_TIMERHANDLE, (VOS_INT)ulFileID, (VOS_INT)lLineNo, VOS_NULL_PTR, VOS_NULL);

    return VOS_ERR;
}

/*****************************************************************************
 Function   : VOS_DrxTimerIsr
 Description: ISR of DRX Timer.
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID VOS_DrxTimerIsr(VOS_VOID)
{
    g_stRtcSocTimerInfo.ulStopCount++;

    if ( VOS_OK != mdrv_timer_stop(TIMER_CCPU_DRX_TIMER_ID) )
    {
        g_stRtcSocTimerInfo.ulStopErrCount++;
    }

    (VOS_VOID)VOS_SmV(g_ulDRXSem);

    return;
}

/*****************************************************************************
 Function   : VOS_StartDrxHardTimer
 Description: Start SOC hard timer by DRV interface.
 Input      : ulValue - 32k timer length.
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID VOS_StartDrxHardTimer(VOS_UINT32 ulValue)
{
    g_stRtcSocTimerInfo.ulStopCount++;
    if ( VOS_OK != mdrv_timer_stop(TIMER_CCPU_DRX_TIMER_ID) )
    {
        g_stRtcSocTimerInfo.ulStopErrCount++;
    }

    g_stRtcSocTimerInfo.ulStartCount++;
    if ( VOS_OK != mdrv_timer_start(TIMER_CCPU_DRX_TIMER_ID, (FUNCPTR_1)VOS_DrxTimerIsr, TIMER_CCPU_DRX_TIMER_ID, ulValue, TIMER_ONCE_COUNT,TIMER_UNIT_NONE) )
    {
        g_stRtcSocTimerInfo.ulStartErrCount++;
    }

    return;
}

/*****************************************************************************
 Function   : VOS_DrxTimerCtrlBlkInit
 Description: Init timer's control block
 Input      : VOS_VOID
 Return     : VOS_OK on success or errno on failure.
 Other      :
 *****************************************************************************/
VOS_UINT32 VOS_DrxTimerCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32                i;

    for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++)
    {
        g_astDRXTimerCtrlBlk[i].ulUsedFlag = DRX_TIMER_NOT_USED_FLAG;
    }

    if( VOS_OK != VOS_SmCreate("DRX", 0, VOS_SEMA4_FIFO, &g_ulDRXSem))
    {
        mdrv_err("<VOS_DrxTimerCtrlBlkInit> create semaphore error.\n");

        return VOS_ERR;
    }


    return VOS_OK;
}

/*****************************************************************************
 Function   : VOS_GetNextDrxTimer
 Description: Get the shortest timer near now.
 Input      : ulCurSlice - the current time.
 Return     : the length of the shortest timer, or VOS_NULL_DWORD indicates no timer.
 Other      :
 *****************************************************************************/
VOS_UINT32 VOS_GetNextDrxTimer(VOS_UINT32 ulCurSlice)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulInterval;
    VOS_UINT32                          ulMinValue;

    ulMinValue = VOS_NULL_DWORD;

    for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++)
    {
        if (DRX_TIMER_USED_FLAG == g_astDRXTimerCtrlBlk[i].ulUsedFlag)
        {
            ulInterval = g_astDRXTimerCtrlBlk[i].ulTimeEndSlice - ulCurSlice;

            /* The interval must be smaller than timer's length. */
            if (g_astDRXTimerCtrlBlk[i].ulTimeOutValueSlice >= ulInterval)
            {
                if (ulMinValue > ulInterval)
                {
                    ulMinValue = ulInterval;
                }
            }
        }
    }

    if (0 == ulMinValue)
    {
        ulMinValue += 1;
    }

    return ulMinValue;
}

/*****************************************************************************
 Function   : VOS_DrxTimerTaskFunc
 Description: DRX timer task entry
 Input      : Parameters are unuseful.
 Return     : void
 Other      :
 *****************************************************************************/
VOS_VOID VOS_DrxTimerTaskFunc( VOS_UINT32 Para0, VOS_UINT32 Para1,
                            VOS_UINT32 Para2, VOS_UINT32 Para3 )
{
    VOS_UINT32                          ulLockLevel;
    VOS_UINT32                          ulCurSlice;
    VOS_UINT32                          i;
    VOS_UINT32                          ulNextTimer;
    REL_TIMER_MSG                      *pstExpireMsg;

#if (VOS_WIN32 == VOS_OS_VER)
    VOS_UINT32                          j;

    for(j = 0; j < 1; j++)
#else
    for(;;)
#endif
    {
        /* SemTake SEM when release */
        (VOS_VOID)VOS_SmP(g_ulDRXSem, 0);

        VOS_SpinLockIntLock(&g_stDrxTimerSpinLock, ulLockLevel);

        ulCurSlice = VOS_GetSlice();

        for (i = 0; i < DRX_TIMER_MAX_NUMBER; i++)
        {
            if (DRX_TIMER_USED_FLAG == g_astDRXTimerCtrlBlk[i].ulUsedFlag)
            {
                /* Check timer is timeout.*/
                if (DRX_TIMER_TIMEOUT_INTERVAL >= (ulCurSlice - g_astDRXTimerCtrlBlk[i].ulTimeEndSlice))
                {
                    /* timer is timeout then notify user by sending msg */
                    pstExpireMsg = VOS_TimerPreAllocMsg(g_astDRXTimerCtrlBlk[i].ulPid);

                    if ( VOS_NULL_PTR != pstExpireMsg )
                    {
                        pstExpireMsg->ulName = g_astDRXTimerCtrlBlk[i].ulName;
                        pstExpireMsg->ulPara = g_astDRXTimerCtrlBlk[i].ulPara;

#if (VOS_YES == VOS_TIMER_CHECK)
                        pstExpireMsg->pNext
                            = (struct REL_TIMER_MSG_STRU *)(g_astDRXTimerCtrlBlk[i].ulAllocTick);
                        pstExpireMsg->pPrev
                            = (struct REL_TIMER_MSG_STRU *)ulCurSlice;
#endif
                        (VOS_VOID)VOS_SendMsg(DOPRA_PID_TIMER, pstExpireMsg);
                    }

                    *(g_astDRXTimerCtrlBlk[i].phTm)    = VOS_NULL_PTR;
                    g_astDRXTimerCtrlBlk[i].ulUsedFlag = DRX_TIMER_NOT_USED_FLAG;
                }
            }
        }

        ulNextTimer = VOS_GetNextDrxTimer(ulCurSlice);

        if (VOS_NULL_DWORD != ulNextTimer)
        {
            VOS_StartDrxHardTimer(ulNextTimer);
        }

        VOS_SpinUnlockIntUnlock(&g_stDrxTimerSpinLock, ulLockLevel);
    }

    /*lint -e527 */
    return;
    /*lint +e527 */
}


/*****************************************************************************
 Function   : VOS_DrxTimerTaskCreat
 Description: create DRX timer task
 Input      : void
 Return     : VOS_OK on success or errno on failure.
 Other      :
 *****************************************************************************/
VOS_UINT32 VOS_DrxTimerTaskCreat(VOS_VOID)
{
    VOS_UINT32 TimerArguments[4] = {0,0,0,0};

    /* DrxTimer自旋锁的初始化 */
    VOS_SpinLockInit(&g_stDrxTimerSpinLock);

    /* Wake Src投票中使用的自旋锁的初始化 */
    VOS_SpinLockInit(&g_ulFlightModeVoteMapSpinLock);

    return( VOS_CreateTask( "DRX_TIMER",
                            &g_ulDRXTimerTaskId,
                            VOS_DrxTimerTaskFunc,
                            COMM_RTC_TIMER_TASK_PRIO,
                            RTC_TIMER_TASK_STACK_SIZE,
                            TimerArguments) );
}

/*****************************************************************************
 Function   : V_StopDrxTimerFunc
 Description: stop a DRX timer which was previously started.
 Input      : phTm -- where store the timer to be stopped
 Return     :  VOS_OK on success or errno on failure
 *****************************************************************************/
VOS_UINT32 V_StopDrxTimerFunc( HTIMER *phTm, VOS_UINT32 ulFileID, VOS_INT32 lLineNo)
{
    DRX_TIMER_CONTROL_BLOCK            *pstTimerCtrl;
    VOS_UINT32                          ulNextTime;

    if (VOS_NULL_PTR == *phTm)
    {
        return VOS_OK;
    }

    if (VOS_ERR == VOS_DrxCheckTimer(phTm, ulFileID, lLineNo))
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_DRXTIME_STOP_INPUTISNULL);

        return VOS_ERRNO_DRXTIME_ERROR_TIMERHANDLE;
    }

    pstTimerCtrl = (DRX_TIMER_CONTROL_BLOCK*)(*phTm);
    *phTm        = VOS_NULL_PTR;

    if (DRX_TIMER_USED_FLAG == pstTimerCtrl->ulUsedFlag)
    {
        pstTimerCtrl->ulUsedFlag = DRX_TIMER_NOT_USED_FLAG;

        ulNextTime = VOS_GetNextDrxTimer(VOS_GetSlice());

        if (VOS_NULL_DWORD == ulNextTime)
        {
            g_stRtcSocTimerInfo.ulStopCount++;
            if ( VOS_OK != mdrv_timer_stop(TIMER_CCPU_DRX_TIMER_ID) )
            {
                g_stRtcSocTimerInfo.ulStopErrCount++;
            }
        }
        else
        {
            VOS_StartDrxHardTimer(ulNextTime);
        }
    }

    return VOS_OK;
}

/*****************************************************************************
 Function   : V_StopDrxTimer
 Description: stop a DRX timer which was previously started.
 Input      : phTm -- where store the timer to be stopped
 Return     :  VOS_OK on success or errno on failure
 *****************************************************************************/
MODULE_EXPORTED VOS_UINT32 V_StopDrxTimer( HTIMER *phTm, VOS_UINT32 ulFileID, VOS_INT32 lLineNo)
{
    VOS_UINT32                          ulLockLevel;
    VOS_UINT32                          ulReturn;

    if (VOS_NULL_PTR == phTm)
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_DRXTIME_STOP_INPUTISNULL);

        return(VOS_ERRNO_DRXTIME_STOP_INPUTISNULL);
    }

    VOS_SpinLockIntLock(&g_stDrxTimerSpinLock, ulLockLevel);

    ulReturn = V_StopDrxTimerFunc(phTm, ulFileID, lLineNo);

    VOS_SpinUnlockIntUnlock(&g_stDrxTimerSpinLock, ulLockLevel);

    return ulReturn;
}

/*****************************************************************************
 Function   : V_StartDrxTimer
 Description: allocate and start a DRX timer.
 Input      : Pid           -- process ID of application
              ulLength       -- expire time. unit is millsecond
              ulName         -- timer name to be pass to app as a parameter
              ulParam        -- additional parameter to be pass to app
 Output     : phTm           -- timer pointer which system retuns to app
 Return     : VOS_OK on success and errno on failure
 *****************************************************************************/
MODULE_EXPORTED VOS_UINT32 V_StartDrxTimer( HTIMER *phTm, VOS_PID Pid, VOS_UINT32 ulLength,
    VOS_UINT32 ulName, VOS_UINT32 ulParam, VOS_UINT32 ulFileID, VOS_INT32 lLineNo)

{
    VOS_UINT32                          i;
    VOS_UINT32                          ulLockLevel;
    VOS_UINT32                          ulCurSlice;
    VOS_UINT32                          ulNextTime;

    if (VOS_NULL_PTR == phTm)
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_DRXTIME_START_INPUTISNULL);
        return(VOS_ERRNO_DRXTIME_START_INPUTISNULL);
    }

    VOS_SpinLockIntLock(&g_stDrxTimerSpinLock, ulLockLevel);

    /* stop the timer if exists */
    if (VOS_NULL_PTR != *phTm)
    {
        if (VOS_OK != V_StopDrxTimerFunc(phTm, ulFileID, lLineNo))
        {
            VOS_SpinUnlockIntUnlock(&g_stDrxTimerSpinLock, ulLockLevel);

            return VOS_ERRNO_DRXTIME_START_STOP_FAIL;
        }
    }

    for(i = 0; i < DRX_TIMER_MAX_NUMBER; i++)
    {
        if (DRX_TIMER_NOT_USED_FLAG == g_astDRXTimerCtrlBlk[i].ulUsedFlag)
        {
            break;
        }
    }

    /* All DRX timer are used. */
    if (DRX_TIMER_MAX_NUMBER == i)
    {
        VOS_SpinUnlockIntUnlock(&g_stDrxTimerSpinLock, ulLockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_DRXTIME_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(VOS_ERRNO_DRXTIME_START_MSGNOTINSTALL, (VOS_INT)Pid, (VOS_INT)ulName, (VOS_CHAR*)g_astDRXTimerCtrlBlk, sizeof(g_astDRXTimerCtrlBlk));

        return(VOS_ERRNO_DRXTIME_START_MSGNOTINSTALL);
    }

    g_astDRXTimerCtrlBlk[i].ulPid   = Pid;
    g_astDRXTimerCtrlBlk[i].ulName  = ulName;
    g_astDRXTimerCtrlBlk[i].ulPara  = ulParam;
    g_astDRXTimerCtrlBlk[i].phTm    = phTm;
    g_astDRXTimerCtrlBlk[i].ulTimeOutValueInMilliSeconds = ulLength;

    g_astDRXTimerCtrlBlk[i].ulTimeOutValueSlice = (VOS_UINT32)RTC_MUL_32_DOT_768(ulLength, ulFileID, lLineNo);

    ulCurSlice = VOS_GetSlice();

    g_astDRXTimerCtrlBlk[i].ulTimeEndSlice = g_astDRXTimerCtrlBlk[i].ulTimeOutValueSlice + ulCurSlice;

#if VOS_YES == VOS_TIMER_CHECK
    g_astDRXTimerCtrlBlk[i].ulFileID    = ulFileID;
    g_astDRXTimerCtrlBlk[i].ulLineNo    = (VOS_UINT32)lLineNo;
    g_astDRXTimerCtrlBlk[i].ulAllocTick = VOS_GetSlice();
#endif

    g_astDRXTimerCtrlBlk[i].ulUsedFlag  = DRX_TIMER_USED_FLAG;

    *phTm = (HTIMER)(&g_astDRXTimerCtrlBlk[i]);

    ulNextTime = VOS_GetNextDrxTimer(ulCurSlice);

    if (VOS_NULL_DWORD != ulNextTime)
    {
        VOS_StartDrxHardTimer(ulNextTime);
    }

    VOS_SpinUnlockIntUnlock(&g_stDrxTimerSpinLock, ulLockLevel);

    return VOS_OK;
}

/*****************************************************************************
 Function   : OM_SetDrxTimerWakeSrcAllVote
 Description: 清除当前modem上所有的票
 Input      : MODEM_ID_ENUM_UINT16 enModem
 Return     : VOS_VOID
 Other      :
*****************************************************************************/
MODULE_EXPORTED VOS_VOID OM_SetDrxTimerWakeSrcAllVote(MODEM_ID_ENUM_UINT16 enModem)
{
    VOS_ULONG                           ulLockLevel;
    VOS_UINT32                          ulVoteBit;
    VOS_UINT32                          ulBitPos;
    DRX_TIMER_WAKE_SRC_VOTE_STRU        stDrxTimerWakeSrcVoteInfo;
    VOS_UINT32                          ulVoteMap;

    /* 参数检查 */
    if (enModem >= MODEM_ID_BUTT)
    {
        return;
    }

    VOS_SpinLockIntLock(&g_ulFlightModeVoteMapSpinLock, ulLockLevel);

    /* 当前modem不在drx，进飞行模式，销整个modem上的票 */
    for (ulBitPos = 0; ulBitPos < DRX_TIMER_WAKE_SRC_MODE_NUM; ulBitPos++)
    {
        ulVoteBit               = enModem * DRX_TIMER_WAKE_SRC_MODE_NUM + ulBitPos;
        g_ulFlightModeVoteMap  &= (~ BIT(ulVoteBit));
    }
    ulVoteMap   = g_ulFlightModeVoteMap;

    VOS_SpinUnlockIntUnlock(&g_ulFlightModeVoteMapSpinLock, ulLockLevel);

    /* 所有mode都退出了drx，就设为唤醒源 */
    if ( 0 == ulVoteMap )
    {
        mdrv_pm_set_wakesrc(PM_WAKE_SRC_DRX_TIMER);
    }

    /* Mntn Event */
    stDrxTimerWakeSrcVoteInfo.enVoteType    = DRX_TIMER_WAKE_SRC_VOTE_SET_ALL;
    stDrxTimerWakeSrcVoteInfo.enVoteModem   = enModem;
    stDrxTimerWakeSrcVoteInfo.enVoteMode    = VOS_RATMODE_GSM;
    stDrxTimerWakeSrcVoteInfo.ulVoteValue   = ulVoteMap;
    stDrxTimerWakeSrcVoteInfo.ulSlice       = VOS_GetSlice();
    PAMOM_DrxTimer_Event((VOS_VOID*)&stDrxTimerWakeSrcVoteInfo, sizeof(DRX_TIMER_WAKE_SRC_VOTE_STRU));

    return;
}

/*****************************************************************************
 Function   : OM_SetDrxTimerWakeSrc
 Description: 设置DRX timer作为唤醒源
 Input      : MODEM_ID_ENUM_UINT16 enModem
 Return     : VOS_VOID
 Other      :
*****************************************************************************/
MODULE_EXPORTED VOS_VOID OM_SetDrxTimerWakeSrc(MODEM_ID_ENUM_UINT16 enModem, VOS_RATMODE_ENUM_UINT32 enMode)
{
    VOS_ULONG                           ulLockLevel;
    VOS_UINT32                          ulVoteBit;
    DRX_TIMER_WAKE_SRC_VOTE_STRU        stDrxTimerWakeSrcVoteInfo;
    VOS_UINT32                          ulVoteMap;

    /* 参数检查 */
    if ((enModem >= MODEM_ID_BUTT) || (enMode >= VOS_RATMODE_BUTT))
    {
        return;
    }

    VOS_SpinLockIntLock(&g_ulFlightModeVoteMapSpinLock, ulLockLevel);

    /* 当前mode退出drx，要销票 */
    ulVoteBit               = enModem * DRX_TIMER_WAKE_SRC_MODE_NUM + enMode;
    g_ulFlightModeVoteMap  &= (~ BIT(ulVoteBit));
    ulVoteMap               = g_ulFlightModeVoteMap;

    VOS_SpinUnlockIntUnlock(&g_ulFlightModeVoteMapSpinLock, ulLockLevel);

    /* 所有mode都退出了drx，就设为唤醒源 */
    if ( 0 == ulVoteMap )
    {
        mdrv_pm_set_wakesrc(PM_WAKE_SRC_DRX_TIMER);
    }

    /* Mntn Event */
    stDrxTimerWakeSrcVoteInfo.enVoteType    = DRX_TIMER_WAKE_SRC_VOTE_SET;
    stDrxTimerWakeSrcVoteInfo.enVoteModem   = enModem;
    stDrxTimerWakeSrcVoteInfo.enVoteMode    = enMode;
    stDrxTimerWakeSrcVoteInfo.ulVoteValue   = ulVoteMap;
    stDrxTimerWakeSrcVoteInfo.ulSlice       = VOS_GetSlice();
    PAMOM_DrxTimer_Event((VOS_VOID*)&stDrxTimerWakeSrcVoteInfo, sizeof(DRX_TIMER_WAKE_SRC_VOTE_STRU));

    return;
}

/*****************************************************************************
 Function   : OM_DelDrxTimerWakeSrc
 Description: 设置DRX timer不作为唤醒源
 Input      : MODEM_ID_ENUM_UINT16 enModem
 Return     : VOS_VOID
 Other      :
*****************************************************************************/
MODULE_EXPORTED VOS_VOID OM_DelDrxTimerWakeSrc(MODEM_ID_ENUM_UINT16 enModem, VOS_RATMODE_ENUM_UINT32 enMode)
{
    VOS_ULONG                           ulLockLevel;
    VOS_UINT32                          ulVoteBit;
    DRX_TIMER_WAKE_SRC_VOTE_STRU        stDrxTimerWakeSrcVoteInfo;
    VOS_UINT32                          ulVoteMap;

    /* 参数检查 */
    if ((enModem >= MODEM_ID_BUTT) || (enMode >= VOS_RATMODE_BUTT))
    {
        return;
    }

    VOS_SpinLockIntLock(&g_ulFlightModeVoteMapSpinLock, ulLockLevel);

    /* 当前mode进drx，要投票 */
    ulVoteBit               = enModem * DRX_TIMER_WAKE_SRC_MODE_NUM + enMode;
    g_ulFlightModeVoteMap  |= BIT(ulVoteBit);
    ulVoteMap               = g_ulFlightModeVoteMap;

    VOS_SpinUnlockIntUnlock(&g_ulFlightModeVoteMapSpinLock, ulLockLevel);

    /* 只要有mode进drx，就设成不作为唤醒源 */
    if ( 0 != ulVoteMap )
    {
        mdrv_pm_clear_wakesrc(PM_WAKE_SRC_DRX_TIMER);
    }

    /* Mntn Event */
    stDrxTimerWakeSrcVoteInfo.enVoteType    = DRX_TIMER_WAKE_SRC_VOTE_CLEAR;
    stDrxTimerWakeSrcVoteInfo.enVoteModem   = enModem;
    stDrxTimerWakeSrcVoteInfo.enVoteMode    = enMode;
    stDrxTimerWakeSrcVoteInfo.ulVoteValue   = ulVoteMap;
    stDrxTimerWakeSrcVoteInfo.ulSlice       = VOS_GetSlice();
    PAMOM_DrxTimer_Event((VOS_VOID*)&stDrxTimerWakeSrcVoteInfo, sizeof(DRX_TIMER_WAKE_SRC_VOTE_STRU));

    return;
}

#endif
/* Added by g47350 for DRX timer Project, 2012/11/5, end */

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU)) && (FEATURE_ON == FEATURE_VOS_18H_TIMER)
/*****************************************************************************
 Function   : BIT64_MUL_32_DOT_768
 Description: 乘以32.768
 Input      : ulValue -- timer's value.uint is 32K cycle.
              ulFileID -- 文件ID
              usLineNo -- 行号
 Return     : 与32.768做乘法的结果
 Other      :
 *****************************************************************************/
VOS_UINT64 BIT64_MUL_32_DOT_768(VOS_UINT32 ulValue,VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo)
{
    VOS_UINT32                          ulProductHigh;
    VOS_UINT32                          ulProductLow;
    VOS_UINT32                          ulQuotientHigh;
    VOS_UINT32                          ulQuotientLow;
    VOS_UINT32                          ulRemainder;
    VOS_UINT32                          ulReturn;
    VOS_UINT64                          ull64Value;

    ulReturn = VOS_64Multi32(0, ulValue, RTC_TIMER_CHECK_LONG_PRECISION, &ulProductHigh, &ulProductLow);
    if (VOS_OK != ulReturn)
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    ulReturn = VOS_64Div32(ulProductHigh, ulProductLow, 1000, &ulQuotientHigh, &ulQuotientLow, &ulRemainder);
    if (VOS_OK != ulReturn)
    {
        VOS_ProtectionReboot(RTC_FLOAT_MUL_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    ull64Value = (((VOS_UINT64)ulQuotientHigh<<32) & 0xffffffff00000000) | ((VOS_UINT64)ulQuotientLow & 0x00000000ffffffff);

    return ull64Value;
}

/*****************************************************************************
 Function   : BIT64_DIV_32_DOT_768
 Description: 除以32.768
 Input      : ulValue -- timer's value.uint is 32K cycle.
              ulFileID -- 文件ID
              usLineNo -- 行号
 Return     : 与32.768做除法的结果
 Other      :
 *****************************************************************************/
VOS_UINT64 BIT64_DIV_32_DOT_768(VOS_UINT64 ulValue,VOS_UINT32 ulFileID,
                                 VOS_INT32 usLineNo)
{

    VOS_UINT32                          ulProductHigh;
    VOS_UINT32                          ulProductLow;
    VOS_UINT32                          ulQuotientHigh;
    VOS_UINT32                          ulQuotientLow;
    VOS_UINT32                          ulRemainder;
    VOS_UINT32                          ulReturn;
    VOS_UINT64                          ull64Value;

    ulReturn = VOS_64Multi32((VOS_UINT32)(ulValue>>32), (VOS_UINT32)(ulValue&0xffffffff), 1000, &ulProductHigh, &ulProductLow);
    if (VOS_OK != ulReturn)
    {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    ulReturn = VOS_64Div32(ulProductHigh, ulProductLow, RTC_TIMER_CHECK_LONG_PRECISION, &ulQuotientHigh, &ulQuotientLow, &ulRemainder);
    if (VOS_OK != ulReturn)
    {
        VOS_ProtectionReboot(RTC_FLOAT_DIV_32_DOT_768, (VOS_INT)ulFileID, (VOS_INT)usLineNo, (VOS_CHAR *)&ulValue, sizeof(ulValue));
        return VOS_ERR;
    }

    ull64Value = (((VOS_UINT64)ulQuotientHigh<<32) & 0xffffffff00000000) | ((VOS_UINT64)ulQuotientLow & 0x00000000ffffffff);

    return ull64Value;

}


VOS_UINT32 VOS_Bit64TimerCheckTimer( HTIMER *phTm )
{
    VOS_UINT32                i;

    for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++)
    {
        if (*phTm == (HTIMER)(&g_astBit64TimerCtrlBlk[i]))
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_VOID VOS_Bit64TimerIsr(VOS_VOID)
{
    g_stRtcSocTimerInfo.ulBit64TimerStopCount++;
    if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_OSA_TIMER2_ID))
    {
        g_stRtcSocTimerInfo.ulBit64TimerStopErrCount++;
    }

    (VOS_VOID)VOS_SmV(g_ulBit64TimerSem);

    return;
}


VOS_VOID VOS_StartBit64HardTimer(VOS_UINT32 ulValue)
{
    g_stRtcSocTimerInfo.ulBit64TimerStopCount++;
    if ( VOS_OK != mdrv_timer_stop(TIMER_CCPU_OSA_TIMER2_ID) )
    {
        g_stRtcSocTimerInfo.ulBit64TimerStopErrCount++;
    }

    g_stRtcSocTimerInfo.ulBit64TimerStartCount++;
    if ( VOS_OK != mdrv_timer_start(TIMER_CCPU_OSA_TIMER2_ID, (FUNCPTR_1)VOS_Bit64TimerIsr, TIMER_CCPU_OSA_TIMER2_ID, ulValue, TIMER_ONCE_COUNT,TIMER_UNIT_NONE) )
    {
        g_stRtcSocTimerInfo.ulBit64TimerStartErrCount++;
    }

    return;
}


VOS_UINT32 VOS_Bit64RtcTimerCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32                i;

    for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++)
    {
        g_astBit64TimerCtrlBlk[i].ulUsedFlag = BIT64_TIMER_NOT_USED_FLAG;
    }

#if (OSA_CPU_NRCPU == VOS_OSA_CPU) && (VOS_WIN32 == VOS_OS_VER)
#else
    if( VOS_OK != VOS_SmCreate("B64", 0, VOS_SEMA4_FIFO, &g_ulBit64TimerSem))
    {
        mdrv_err("<VOS_64BitRtcTimerCtrlBlkInit> create semaphore error.\n");

        return VOS_ERR;
    }
#endif

    return VOS_OK;
}


VOS_UINT32 VOS_GetNextBit64Timer(VOS_UINT64 ullCurSlice, VOS_UINT32* pulNeedStartTimer)
{
    VOS_UINT32                          i;
    VOS_UINT64                          ullInterval;
    VOS_UINT32                          ulMinValue;

    /* 64BIT定时器资源池列表中所有定时器距离当前系统时间超时时长超过18小时，
       则需要启动18小时最大的定时计数。换算成32K硬件定时器时长为0x7E900000 */
    ulMinValue         = 0x7E900000;
    *pulNeedStartTimer  = VOS_FALSE;

    for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++)
    {
        if (BIT64_TIMER_USED_FLAG == g_astBit64TimerCtrlBlk[i].ulUsedFlag)
        {
            *pulNeedStartTimer = VOS_TRUE;

            if (g_astBit64TimerCtrlBlk[i].ullTimeEndSlice >= ullCurSlice)
            {
                ullInterval = g_astBit64TimerCtrlBlk[i].ullTimeEndSlice - ullCurSlice;
            }
            else
            {
                ullInterval = 0;
            }

            if ((VOS_UINT64)ulMinValue > ullInterval)
            {
                ulMinValue = (VOS_UINT32)ullInterval;
            }
        }
    }

    /* 规避芯片timerbug，不能启动0步长定时器 */
    if (0 == ulMinValue)
    {
        ulMinValue += 1;
    }

    g_ulBit64NextHardTimerSlice    = ulMinValue;

    return ulMinValue;
}


VOS_VOID VOS_Bit64TimerTaskFunc( VOS_UINT32 Para0, VOS_UINT32 Para1,
                            VOS_UINT32 Para2, VOS_UINT32 Para3 )
{
    VOS_UINT32                          ulLockLevel;
    VOS_UINT64                          ullCurSlice;
    VOS_UINT32                          i;
    VOS_UINT32                          ulNextTimer;
    REL_TIMER_MSG                      *pstExpireMsg;
    VOS_UINT32                          ulNeedStartTimer;

#if (VOS_WIN32 == VOS_OS_VER)
    VOS_UINT32                          j;

    for(j = 0; j < 1; j++)
#else
    for(;;)
#endif
    {
        /* SemTake SEM when release */
        (VOS_VOID)VOS_SmP(g_ulBit64TimerSem, 0);

        VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

        ullCurSlice = VOS_Get64BitSlice();

        for (i = 0; i < BIT64_TIMER_MAX_NUMBER; i++)
        {
            if (BIT64_TIMER_USED_FLAG == g_astBit64TimerCtrlBlk[i].ulUsedFlag)
            {
                /* 64位Slice时间是个非常大的时间，按照正常使用该时间是几乎不会
                  发生反转的，所以此处判断可以不用考虑反转场景 */
                if (ullCurSlice >= g_astBit64TimerCtrlBlk[i].ullTimeEndSlice)
                {
                    /* 定时器超时后，给调用组件发送消息通知 */
                    pstExpireMsg = VOS_TimerPreAllocMsg(g_astBit64TimerCtrlBlk[i].ulPid);

                    if ( VOS_NULL_PTR != pstExpireMsg )
                    {
                        pstExpireMsg->ulName = g_astBit64TimerCtrlBlk[i].ulName;
                        pstExpireMsg->ulPara = g_astBit64TimerCtrlBlk[i].ulPara;

                        (VOS_VOID)VOS_SendMsg(DOPRA_PID_TIMER, pstExpireMsg);
                    }

                    *(g_astBit64TimerCtrlBlk[i].phTm)    = VOS_NULL_PTR;
                    g_astBit64TimerCtrlBlk[i].ulUsedFlag = BIT64_TIMER_NOT_USED_FLAG;
                }
            }
        }

        ulNextTimer = VOS_GetNextBit64Timer(ullCurSlice, &ulNeedStartTimer);

        if (VOS_TRUE == ulNeedStartTimer)
        {
            VOS_StartBit64HardTimer(ulNextTimer);
        }

        VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);
    }

    /*lint -e527 */
    return;
    /*lint +e527 */
}


VOS_UINT32 VOS_Bit64TimerTaskCreat(VOS_VOID)
{
    VOS_UINT32 TimerArguments[4] = {0,0,0,0};

    return( VOS_CreateTask( "B64_TIMER",
                            &g_ulBit64TimerTaskId,
                            VOS_Bit64TimerTaskFunc,
                            COMM_RTC_TIMER_TASK_PRIO,
                            RTC_TIMER_TASK_STACK_SIZE,
                            TimerArguments) );
}


VOS_UINT32 VOS_StopBit64Timer( HTIMER *phTm,
                                     VOS_UINT32 ulFileID,
                                     VOS_INT32 lLineNo,
                                     VOS_TIMER_OM_EVENT_STRU *pstEvent)
{
    BIT64_TIMER_CONTROL_BLOCK          *pstTimerCtrl;
    VOS_UINT64                          ullCurSlice;
    VOS_UINT32                          ulNextTime;
    VOS_UINT32                          ulNeedStartTimer;

    pstTimerCtrl = (BIT64_TIMER_CONTROL_BLOCK*)(*phTm);
    *phTm        = VOS_NULL_PTR;

    if (BIT64_TIMER_USED_FLAG == pstTimerCtrl->ulUsedFlag)
    {
        pstTimerCtrl->ulUsedFlag = BIT64_TIMER_NOT_USED_FLAG;

        ullCurSlice = VOS_Get64BitSlice();

        ulNextTime = VOS_GetNextBit64Timer(ullCurSlice, &ulNeedStartTimer);

        if (VOS_FALSE == ulNeedStartTimer)
        {
            g_stRtcSocTimerInfo.ulBit64TimerStopCount++;
            if (VOS_OK != mdrv_timer_stop(TIMER_CCPU_OSA_TIMER2_ID))
            {
                g_stRtcSocTimerInfo.ulBit64TimerStopErrCount++;
            }
        }
        else
        {
            VOS_StartBit64HardTimer(ulNextTime);
        }
    }
    else
    {
        if ( VOS_NULL_PTR != pstEvent )
        {
            pstEvent->ucMode      = 0;
            pstEvent->Pid         = pstTimerCtrl->ulPid;
            pstEvent->ulLength    = pstTimerCtrl->ulTimeOutValueInMilliSeconds;
            pstEvent->ulName      = pstTimerCtrl->ulName;
            pstEvent->ulParam     = pstTimerCtrl->ulPara;
            pstEvent->enPrecision = 0;
        }

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED);

        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED, (VOS_INT)ulFileID, (VOS_INT)lLineNo, (VOS_CHAR*)phTm, sizeof(VOS_CHAR *));

        /* 补充异常处理流程 */
        return VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED;
    }

    /* OM */
    if ( VOS_NULL_PTR != pstEvent )
    {
        pstEvent->ucMode      = 0;
        pstEvent->Pid         = pstTimerCtrl->ulPid;
        pstEvent->ulLength    = pstTimerCtrl->ulTimeOutValueInMilliSeconds;
        pstEvent->ulName      = pstTimerCtrl->ulName;
        pstEvent->ulParam     = pstTimerCtrl->ulPara;
        pstEvent->enPrecision = 0;
    }

    return VOS_OK;
}



MODULE_EXPORTED VOS_UINT32 VOS_StartBit64Timer( HTIMER *phTm,
                                                     VOS_PID Pid,
                                                     VOS_UINT32 ulLength,
                                                     VOS_UINT32 ulName,
                                                     VOS_UINT32 ulParam,
                                                     VOS_UINT32 ulFileID,
                                                     VOS_INT32 lLineNo)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulLockLevel;
    VOS_UINT64                          ullCurSlice;
    VOS_UINT32                          ulNextTime;
    VOS_UINT32                          ulNeedStartTimer;

    VOS_SpinLockIntLock(&g_stVosTimerSpinLock, ulLockLevel);

    /* 如果定时器已经启动，则先停止该定时器 */
    if (VOS_NULL_PTR != *phTm)
    {
        if (VOS_OK != VOS_StopBit64Timer(phTm, ulFileID, lLineNo, VOS_NULL_PTR))
        {
            VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

            (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_BIT64TIME_START_STOP_FAIL);

            VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_START_STOP_FAIL, (VOS_INT)Pid, (VOS_INT)ulName, (VOS_CHAR*)g_astBit64TimerCtrlBlk, sizeof(g_astBit64TimerCtrlBlk));

            return VOS_ERRNO_BIT64TIME_START_STOP_FAIL;
        }
    }

    /* 查找资源池中空闲的定时器资源 */
    for(i = 0; i < BIT64_TIMER_MAX_NUMBER; i++)
    {
        if (BIT64_TIMER_NOT_USED_FLAG == g_astBit64TimerCtrlBlk[i].ulUsedFlag)
        {
            break;
        }
    }

    /* 所有定时器资源都被使用，OSA发起保护性复位保留现场确认资源使用情况 */
    if (BIT64_TIMER_MAX_NUMBER == i)
    {
        VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_BIT64TIME_START_MSGNOTINSTALL);

        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_START_MSGNOTINSTALL, (VOS_INT)Pid, (VOS_INT)ulName, (VOS_CHAR*)g_astBit64TimerCtrlBlk, sizeof(g_astBit64TimerCtrlBlk));

        return(VOS_ERRNO_BIT64TIME_START_MSGNOTINSTALL);
    }

    g_astBit64TimerCtrlBlk[i].ulPid     = Pid;
    g_astBit64TimerCtrlBlk[i].ulName    = ulName;
    g_astBit64TimerCtrlBlk[i].ulPara    = ulParam;
    g_astBit64TimerCtrlBlk[i].phTm      = phTm;
    g_astBit64TimerCtrlBlk[i].ulTimeOutValueInMilliSeconds  = ulLength;

    g_astBit64TimerCtrlBlk[i].ullTimeOutValueSlice   = BIT64_MUL_32_DOT_768(ulLength, ulFileID, lLineNo);

    ullCurSlice = VOS_Get64BitSlice();

    g_astBit64TimerCtrlBlk[i].ullTimeEndSlice   = g_astBit64TimerCtrlBlk[i].ullTimeOutValueSlice + ullCurSlice;

#if VOS_YES == VOS_TIMER_CHECK
    g_astBit64TimerCtrlBlk[i].ulFileID      = ulFileID;
    g_astBit64TimerCtrlBlk[i].ulLineNo      = (VOS_UINT32)lLineNo;
    g_astBit64TimerCtrlBlk[i].ullAllocTick  = VOS_Get64BitSlice();
#endif

    g_astBit64TimerCtrlBlk[i].ulUsedFlag    = BIT64_TIMER_USED_FLAG;

    *phTm = (HTIMER)(&g_astBit64TimerCtrlBlk[i]);

    /* 遍历资源池，确认是否需要启动硬件定时器 */
    ulNextTime = VOS_GetNextBit64Timer(ullCurSlice, &ulNeedStartTimer);

    if (VOS_TRUE == ulNeedStartTimer)
    {
        VOS_StartBit64HardTimer(ulNextTime);
    }

    VOS_SpinUnlockIntUnlock(&g_stVosTimerSpinLock, ulLockLevel);

    return VOS_OK;
}


VOS_UINT32 VOS_RestartBit64Timer( HTIMER *phTm,
                                        VOS_UINT32 ulFileID,
                                        VOS_INT32 lLineNo )
{
    BIT64_TIMER_CONTROL_BLOCK          *pstTimerCtrl;
    VOS_UINT64                          ullCurSlice;
    VOS_UINT32                          ulNextTime;
    VOS_UINT32                          ulNeedStartTimer;

    pstTimerCtrl = (BIT64_TIMER_CONTROL_BLOCK*)(*phTm);

    if (BIT64_TIMER_NOT_USED_FLAG == pstTimerCtrl->ulUsedFlag)
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED);

        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED, (VOS_INT)ulFileID, (VOS_INT)lLineNo, (VOS_CHAR*)phTm, sizeof(VOS_CHAR *));

        return VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED;
    }

    ullCurSlice = VOS_Get64BitSlice();

    /* 重置定时器预期超时物理时间 */
    pstTimerCtrl->ullTimeEndSlice    = pstTimerCtrl->ullTimeOutValueSlice + ullCurSlice;

    /* 遍历资源池，确认是否需要启动硬件定时器 */
    ulNextTime = VOS_GetNextBit64Timer(ullCurSlice, &ulNeedStartTimer);

    if (VOS_TRUE == ulNeedStartTimer)
    {
        VOS_StartBit64HardTimer(ulNextTime);
    }

    return VOS_OK;
}


VOS_UINT32 VOS_GetBit64RemainTime( HTIMER *phTm,
                                           VOS_UINT32 *pulTime,
                                           VOS_UINT32 ulFileID,
                                           VOS_INT32 lLineNo )
{
    BIT64_TIMER_CONTROL_BLOCK          *pstTimerCtrl;
    VOS_UINT64                          ullCurSlice;
    VOS_UINT64                          ullTempTime;

    pstTimerCtrl    = (BIT64_TIMER_CONTROL_BLOCK*)(*phTm);

    if (BIT64_TIMER_NOT_USED_FLAG == pstTimerCtrl->ulUsedFlag)
    {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED);

        VOS_ProtectionReboot(VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED, (VOS_INT)ulFileID, (VOS_INT)lLineNo, (VOS_CHAR*)phTm, sizeof(VOS_CHAR *));

        return VOS_ERRNO_BIT64TIME_ERROR_TIMERNOUSED;
    }

    ullCurSlice     = VOS_Get64BitSlice();

    if (pstTimerCtrl->ullTimeEndSlice >= ullCurSlice)
    {
        ullTempTime = pstTimerCtrl->ullTimeEndSlice - ullCurSlice;
    }
    else
    {
        ullTempTime = 0;
    }

    *pulTime    = (VOS_UINT32)BIT64_DIV_32_DOT_768(ullTempTime, ulFileID, usLineNo);

    return VOS_OK;
}

#endif

/*****************************************************************************
 Function   : ShowRtcTimerLog
 Description:
 Input      : VOID
 Return     : VOID
 Other      :
 *****************************************************************************/
MODULE_EXPORTED VOS_VOID ShowRtcTimerLog( VOS_VOID )
{
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulStartCount=%d. (call DRV Start timer num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulStartCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulStopCount=%d. (call DRV Stop timer num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulStopCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulExpireCount=%d. (receive DRV ISR of DualTimer num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulExpireCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulStartErrCount=%d. (call DRV Stop timer err num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulStartErrCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulStopErrCount=%d. (call DRV Start timer err num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulStopErrCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulElapsedErrCount=%d. (call DRV get rest timer num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulElapsedErrCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulElapsedContentErrCount=%d. (call DRV get rest timer err num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulElapsedContentErrCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulBit64TimerStartCount=%d. (call DRV get start timer num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulBit64TimerStartCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulBit64TimerStopCount=%d. (call DRV get stop timer num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulBit64TimerStopCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulBit64TimerStartErrCount=%d. (call DRV get start timer err num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulBit64TimerStartErrCount);
    mdrv_debug("<%s> g_stRtcSocTimerInfo.ulBit64TimerStopErrCount=%d. (call DRV get stop timer err num)\n", __FUNCTION__,
                g_stRtcSocTimerInfo.ulBit64TimerStopErrCount);
}

/*****************************************************************************
 Function   : RTC_ReportOmInfo
 Description: report RTC OM info
 Input      :
 Return     :
 Other      :
 *****************************************************************************/
VOS_VOID RTC_ReportOmInfo(VOS_VOID)
{
    VOS_UINT32                   ulRTCLength               = 0;
    VOS_UINT32                   ulRTCTimerDebugInfoLength = 0;
    VOS_UINT32                   ulRTCTimerInfoLength      = 0;
    VOS_RTC_OM_INFO_STRU        *pstRTCInfo                = VOS_NULL_PTR;

    /* 获取g_astRtcSocTimerDebugInfo的大小 */
    ulRTCTimerDebugInfoLength = sizeof(RTC_SOC_TIMER_DEBUG_INFO_STRU) * RTC_MAX_TIMER_NUMBER;

    /* 获取g_stRtcSocTimerInfo的大小 */
    ulRTCTimerInfoLength = sizeof(VOS_TIMER_SOC_TIMER_INFO_STRU);

    ulRTCLength = ulRTCTimerDebugInfoLength + ulRTCTimerInfoLength;

    pstRTCInfo = (VOS_RTC_OM_INFO_STRU*)VOS_MemAlloc(DOPRA_PID_TIMER, DYNAMIC_MEM_PT,
                    (VOS_UINT32)(sizeof(VOS_RTC_OM_INFO_STRU) - (VOS_UINT32)(sizeof(VOS_UINT8) * 4) + ulRTCLength));

    if (VOS_NULL_PTR != pstRTCInfo)
    {
        pstRTCInfo->ulSenderPid   = DOPRA_PID_TIMER;

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
        pstRTCInfo->ulReceiverPid = ACPU_PID_PAM_OM;
#elif (OSA_CPU_NRCPU == VOS_OSA_CPU)
        pstRTCInfo->ulReceiverPid = NRCPU_PID_PAM_OM;
#else
        pstRTCInfo->ulReceiverPid = CCPU_PID_PAM_OM;
#endif

        pstRTCInfo->ulLength      = ulRTCLength;

        /*lint -e426 */
        if (VOS_NULL_PTR == VOS_MemCpy_s((VOS_VOID *)pstRTCInfo->aucValue, ulRTCLength,
            (VOS_VOID *)g_astRtcSocTimerDebugInfo, ulRTCTimerDebugInfoLength))
        {
            (VOS_VOID)VOS_MemFree(DOPRA_PID_TIMER, pstRTCInfo);

            return;
        }
        /*lint +e426 */

        /*lint -e416 -e426 */
        if (VOS_NULL_PTR == VOS_MemCpy_s((VOS_VOID *)(pstRTCInfo->aucValue + ulRTCTimerDebugInfoLength),
            (ulRTCLength - ulRTCTimerDebugInfoLength), (VOS_VOID *)&g_stRtcSocTimerInfo, ulRTCTimerInfoLength))
        {
            (VOS_VOID)VOS_MemFree(DOPRA_PID_TIMER, pstRTCInfo);

            return;
        }
        /*lint +e416 +e426 */

        DIAG_TraceReport((VOS_VOID *)(pstRTCInfo));

        (VOS_VOID)VOS_MemFree(DOPRA_PID_TIMER, pstRTCInfo);
    }

    return;
}



