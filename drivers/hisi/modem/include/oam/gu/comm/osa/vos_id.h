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
 * 功能描述: OSA FID/PID 模块头文件
 * 生成日期: 2006年10月3日
 */

#ifndef _VOS_ID_H
#define _VOS_ID_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "dopra_def.h"
#include "v_typdef.h"
#include "vos_config.h"
#include "v_queue.h"
#include "v_msg.h"
#include "v_id.h"
#include "v_io.h"
#include "v_task.h"

/* errno definiens */
#define VOS_ERRNO_REG_FID_PID_INVALID_PID 0x20000001
#define VOS_ERRNO_REG_FID_PID_INVALID_FID 0x20000002

#define VOS_ERRNO_CORE_MASK_INVALID 0x20000003

#define VOS_ERRNO_FIDPID_REGPIDI_INPUTPIDINVALID 0x20140006
#define VOS_ERRNO_FIDPID_REGPIDI_INPUTFUNCARENULL 0x20140007
#define VOS_ERRNO_FIDPID_REGPIDI_PIDINACTIVE 0x20140008
#define VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID 0x2014000F
#define VOS_ERRNO_FIDPID_REGMPRI_INPUT2INVALID 0x20140010

/* the type of buffer.msg or mem */
enum {
    VOS_LOCATION_MSG,
    VOS_LOCATION_MEM
};

/* the Max messages stored in queue of one FID */
#define VOS_FID_QUEUE_LENGTH 120
/* the Max length of message stored in queue of one FID */
/* the pointer of address be sent not the message self */
#define VOS_FID_MAX_MSG_LENGTH sizeof(VOS_VOID *)

#define VOS_ID_PID_NULL -1
#define VOS_ID_FID_NULL -1

typedef struct {
    int           pid;
    int           fid;
    InitFunType   initFunc;
    MsgFunType    msgFunction;
    int           priority;
#if VOS_YES == VOS_ANALYZE_PID_MEM
    VOS_UINT32    msgSize;
    VOS_UINT32    memSize;
    VOS_UINT32    msgPeakSize;
    VOS_UINT32    memPeakSize;
#endif
    ModemIdUint16 modemId;
    VOS_UINT16    reserved;
    VOS_UINT8     isError; /* 0:NO ERROR 1:ERROR */
    VOS_UINT8     initPhase;
    VOS_UINT8     rsv[2];
} VOS_PidRecord;

enum {
    VOS_PID_INVAILD,
    VOS_PID_AVAILABLE,
    VOS_PID_UNSURE
};

typedef struct {
    ModemIdUint16 modemId;
    VOS_UINT16    pidInvaild;
} VOS_QueryPidModemId;

extern VOS_PidRecord g_vosPidRecords[];

#define VOS_ID_MAX_FID_NAME_LENGTH 16

/* the max selftask belong to one FID */
#define VOS_MAX_SELF_TASK_OF_FID 6

typedef struct {
    int               fid;
    VOS_UINT8         isError; /* 0:NO ERROR 1:ERROR */
    VOS_UINT8         initPhase;
    VOS_UINT8         rsv[2];
    InitFunType       initFunc; /* VOS_ULFUNCPTR       InitFunc; */
    VOS_UINT32        startOrder;

    VOS_UINT32        tid;
    VOS_UINT32        priority;
    VOS_UINT32        stackSize;
    VOS_TaskEntryType taskFunction;
    VOS_UINT32        taskArgument[VOS_TARG_NUM];
#if ((defined __OS_RTOSCK_SMP__) || (defined __OS_RTOSCK_TSP__))
    VOS_UINT32        coreMask;
#endif

    VOS_UINT32        qid;
    VOS_UINT32        maxMsgNumber;
    VOS_UINT32        maxMsgLength;
    VOS_UINT32        queueOptions;
    /* which should be del when only one FID exists */
    VOS_UINT32        maxQidSize;
    VOS_UINT8         rsv1[4];

    VOS_UINT32        selfProcTaskTid[VOS_MAX_SELF_TASK_OF_FID];
    VOS_UINT32        selfProcTaskPriority[VOS_MAX_SELF_TASK_OF_FID];
    VOS_UINT32        selfProcTaskStack[VOS_MAX_SELF_TASK_OF_FID];
    VOS_TaskEntryType selfProcTaskFunc[VOS_MAX_SELF_TASK_OF_FID];
#if ((defined __OS_RTOSCK_SMP__) || (defined __OS_RTOSCK_TSP__))
    VOS_UINT32        selfProcTaskCoreMask[VOS_MAX_SELF_TASK_OF_FID];
#endif
    char              name[VOS_ID_MAX_FID_NAME_LENGTH];
} VOS_FidControlBlock;

typedef struct {
    VOS_PID    pid;
    VOS_UINT32 msgPeakSize;
    VOS_UINT32 memPeakSize;
} VOS_PidPeakMem;

extern VOS_FidControlBlock g_vosFidCtrlBlk[];

VOS_UINT32 VOS_RegisterPIDInfo(VOS_PID pid, InitFunType pfnInitFun, MsgFunType pfnMsgFun);

VOS_UINT32 VOS_RegisterMsgTaskPrio(VOS_FID fid, enum VOS_PriorityDefine taskPrio);

VOS_UINT32 VOS_RegisterTaskPrio(VOS_FID fid, VOS_UINT32 taskRealPri);

VOS_UINT8 VOS_RegisterSelfTask(VOS_FID fid, VOS_TaskEntryType pfnSelfTask, enum VOS_PriorityDefine taskPrio,
                               VOS_UINT32 stackSize);

VOS_UINT8 VOS_RegisterSelfTaskPrio(VOS_FID fid, VOS_TaskEntryType pfnSelfTask, VOS_UINT32 taskRealPri,
                                   VOS_UINT32 stackSize);

/* for rtosck smp */
VOS_UINT32 VOS_GetAllCoresMask(VOS_VOID);

VOS_UINT32 VOS_RegisterFidTaskCoreBind(VOS_FID fid, VOS_UINT32 coreMask);

VOS_UINT32 VOS_RegisterSelfTaskCoreBind(VOS_FID fid, VOS_UINT8 vosIndex, VOS_UINT32 coreMask);

VOS_UINT32 VOS_CoreBindFidTask(VOS_FID fid, VOS_UINT32 coreMask);

VOS_UINT32 VOS_CoreBindSelfTask(VOS_FID fid, VOS_UINT8 vosIndex, VOS_UINT32 coreMask);

VOS_UINT32 VOS_PidCtrlBlkInit(VOS_VOID);

VOS_UINT32 VOS_PidsInit(VOS_VOID);

VOS_UINT32 VOS_FidCtrlBlkInit(VOS_VOID);

VOS_UINT32 VOS_FidsInit(VOS_VOID);

VOS_UINT32 CreateFidsQueque(VOS_VOID);

VOS_UINT32 CreateFidsTask(VOS_VOID);

VOS_VOID vos_FidTask(VOS_UINT32 queueID, VOS_UINT32 fidValue, VOS_UINT32 para1, VOS_UINT32 para2);

typedef VOS_UINT32 (*VOS_MsgHookFunc)(VOS_VOID *pMsg);

VOS_MsgHookFunc VOS_RegisterMsgGetHook(VOS_MsgHookFunc pfnMsgHook);

VOS_UINT32 VOS_RegisterMsgTaskEntry(VOS_FID fid, VOS_TaskEntryType pfnMsgTask);

VOS_UINT32 VOS_GetQueueIDFromFid(VOS_UINT32 fid);

#if (VOS_LINUX != VOS_OS_VER)
VOS_UINT32 VOS_SuspendFidTask(VOS_FID fid);
#endif

VOS_UINT32 VOS_ResumeFidTask(VOS_FID fid);

#if (VOS_LINUX != VOS_OS_VER)
VOS_UINT32 VOS_SuspendFidsTask(VOS_VOID);
#endif

VOS_UINT32 VOS_ResumeFidsTask(VOS_VOID);

#if VOS_YES == VOS_ANALYZE_PID_MEM
VOS_VOID VOS_IncreasePidMemory(VOS_PID pid, VOS_UINT32 size, VOS_UINT32 type);

VOS_VOID VOS_DecreasePidMemory(VOS_PID pid, VOS_UINT32 size, VOS_UINT32 type);
#endif

VOS_UINT32 VOS_AnalyzePidMemory(VOS_VOID *bUffer, VOS_UINT32 size, VOS_UINT32 *num);

VOS_UINT32 VOS_GetTCBFromPid(VOS_UINT32 pid);

/* 该API不能查询卡模块PID(usim/pih/stk/pb),建议外部组建使用PS_GetModemIdFromPid */
ModemIdUint16 VOS_GetModemIDFromPid(VOS_UINT32 pid);

VOS_BOOL VOS_CheckPSPidValidity(VOS_UINT32 pid);

VOS_VOID VOS_OsaGlobalShow(VOS_VOID);

VOS_UINT32 VOS_GetFIdRelPri(enum VOS_PriorityDefine fidPri, VOS_UINT32 *taskRealPri);

VOS_UINT32 VOS_CheckPidValidity(VOS_UINT32 pid);

VOS_UINT32 VOS_QueryPidInfoBufSize(VOS_VOID);

VOS_VOID VOS_QueryPidInfo(VOS_VOID *buf, VOS_UINT32 len);

VOS_VOID VOS_SetPidInfo(VOS_VOID *buf, VOS_UINT16 len);

VOS_VOID VOS_SetCpuPidInfo(VOS_UINT32 cpuID, VOS_VOID *buf, VOS_UINT16 len);

VOS_BOOL VOS_CheckLocalPidValidity(VOS_UINT32 pid);

VOS_UINT32 VOS_GetOsIdFromFid(VOS_FID fid);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _VOS_ID_H */
