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
 * 功能描述: OSA FID/PID 功能实现
 * 生成日期: 2006年10月3日
 */

#include "vos_id.h"
#include "v_fid_entry.h"
#include "v_fid_pid_config.h"
#include "v_timer.h"
#include "v_int.h"
#include "mdrv.h"
#include "pam_tag.h"

#if (VOS_OS_VER != VOS_WIN32)
#include <securec.h>
#endif


#define THIS_FILE_ID PS_FILE_ID_VOS_ID_C
#define THIS_MODU mod_pam_osa

/* recoed PID info VOS_PID_BUTT comes from v_fid_pid_config.h */
VOS_INT        g_vosPidRecordsNumber;
VOS_PidRecord g_vosPidRecords[VOS_PID_BUTT - VOS_PID_DOPRAEND];

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
VOS_QueryPidModemId g_vosQueryCcpuPidInfo[VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND];
VOS_QueryPidModemId g_vosQueryNrcpuPidInfo[VOS_CPU_ID_2_PID_BUTT - VOS_PID_CPU_ID_2_DOPRAEND];
#endif

/* recoed FID info VOS_FID_BUTT comes from v_fid_pid_config.h */
VOS_INT               g_vosFidCtrlBlkNumber;
VOS_FidControlBlock g_vosFidCtrlBlk[VOS_FID_BUTT];

#if (VOS_WIN32 == VOS_OS_VER)
/* the map of PRI between FID and Win32 */
VOS_UINT32 g_vosFidTaskRealPriority[VOS_PRIORITY_NUM] = {
    32, 33, 64, 65, 96, 97, 128, 129, 160, 161, 192, 193, 224, 225
};
#endif

#if (VOS_LINUX == VOS_OS_VER)
/* the map of PRI between FID and Linux */
VOS_UINT32 g_vosFidTaskRealPriority[VOS_PRIORITY_NUM] = {
    30, 32, 35, 38, 41, 44, 47, 51, 55, 59, 63, 67, 71, 75
};
#endif

#if (VOS_RTOSCK == VOS_OS_VER)
/* the map of PRI between FID and RTOSCK */
VOS_UINT32 g_vosFidTaskRealPriority[VOS_PRIORITY_NUM] = {
    48, 47, 46, 43, 42, 41, 40, 38, 36, 34, 33, 32, 30, 28
};
#endif

/* default value of task stack size */
#define TASK_STACK_SIZE 32768

/* which should be del when only one FID exists */
#define DEFAULT_FID_QUEUE_NUM 1

/* the base value of Fid's Init */
#define VOS_FID_INIT_ERR_BASE 0x80000000
#define VOS_PID_INIT_ERR_BASE 0x80000000

VOS_UINT32 g_omFidInit = VOS_NULL;
VOS_UINT32 g_omPidInit = VOS_NULL;

VOS_UINT16 g_fidInitStep;
VOS_UINT16 g_fidInitId;
VOS_UINT16 g_pidInitStep;
VOS_UINT16 g_pidInitId;

extern VOS_VOID V_LogRecord(VOS_UINT32 vosIndex, VOS_UINT32 value);

#if VOS_YES == VOS_ANALYZE_PID_MEM

/* 自旋锁，用来作PID 内存消耗统计临界资源保护 */
VOS_SPINLOCK g_vosPidMemAnaSpinLock;

#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
/* 自旋锁，用来作querey pid info 的临界资源保护 */
VOS_SPINLOCK g_vosQuereyPidInfoSpinLock;
#endif

/*
 * Description: map the PID and FID
 */
VOS_UINT32 MOD_RegFidPidRSP(VOS_UINT32 pid, VOS_UINT32 fid, VOS_INT priority, ModemIdUint16 modemId)
{
    if (pid <= VOS_PID_DOPRAEND - 1) {
        return VOS_OK;
    }

    if (pid >= VOS_PID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_REG_FID_PID_INVALID_PID);
        return (VOS_ERRNO_REG_FID_PID_INVALID_FID);
    }

    if (VOS_ID_PID_NULL != g_vosPidRecords[pid - VOS_PID_DOPRAEND].pid) {
        mdrv_err("<VOS_RegisterPIDInfo> multiple register.\n");
    }

    if (priority > VOS_PRIORITY_P6) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT2INVALID);
        return (VOS_ERRNO_FIDPID_REGMPRI_INPUT2INVALID);
    }

    g_vosPidRecords[pid - VOS_PID_DOPRAEND].pid      = (int)pid;
    g_vosPidRecords[pid - VOS_PID_DOPRAEND].fid      = (int)fid;
    g_vosPidRecords[pid - VOS_PID_DOPRAEND].priority = priority;

    if (modemId > MODEM_ID_BUTT) {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].modemId = MODEM_ID_BUTT;
    } else {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].modemId = modemId;
    }

    return (VOS_OK);
}

/*
 * Description: record PID info
 */

VOS_UINT32 VOS_RegisterPIDInfo(VOS_PID pid, InitFunType pfnInitFun, MsgFunType pfnMsgFun)
{
    if ((pid >= VOS_PID_BUTT) || (pid < VOS_PID_DOPRAEND)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGPIDI_INPUTPIDINVALID);
        mdrv_err("<VOS_RegisterPIDInfo> Register Wrong PID=%d", (VOS_INT)pid);
        return (VOS_ERRNO_FIDPID_REGPIDI_INPUTPIDINVALID);
    }

    if (pfnInitFun != VOS_NULL_PTR) {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].initFunc = pfnInitFun;
    }

    if (pfnMsgFun == VOS_NULL_PTR) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGPIDI_INPUTFUNCARENULL);
        return (VOS_ERRNO_FIDPID_REGPIDI_INPUTFUNCARENULL);
    } else {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].msgFunction = pfnMsgFun;
    }

    return (VOS_OK);
}

/*
 * Description: the default Init function of pid
 */
VOS_UINT32 PidInitFuncDefault(enum VOS_InitPhaseDefine initPhrase)
{
    return (VOS_OK);
}

/*
 * Description: the default process function of pid
 */
VOS_VOID MsgProcFuncDefault(MsgBlock *msg)
{
    return;
}

/*
 * Description: Init the control block of PID
 */
VOS_UINT32 VOS_PidCtrlBlkInit(VOS_VOID)
{
    int i;

    for (i = VOS_PID_DOPRAEND; i < VOS_PID_BUTT; i++) {
        g_vosPidRecords[i - VOS_PID_DOPRAEND].pid = VOS_ID_PID_NULL;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].fid = VOS_ID_FID_NULL;

        g_vosPidRecords[i - VOS_PID_DOPRAEND].initFunc    = PidInitFuncDefault;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].msgFunction = MsgProcFuncDefault;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].priority    = VOS_PRIORITY_NULL;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].modemId   = MODEM_ID_BUTT;
#if VOS_YES == VOS_ANALYZE_PID_MEM
        g_vosPidRecords[i - VOS_PID_DOPRAEND].msgSize     = VOS_NULL;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].memSize     = VOS_NULL;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].msgPeakSize = VOS_NULL;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].memPeakSize = VOS_NULL;
#endif
        g_vosPidRecords[i - VOS_PID_DOPRAEND].isError     = VOS_FALSE;
        g_vosPidRecords[i - VOS_PID_DOPRAEND].initPhase   = VOS_IP_BUTT;
    }

    /* which comes from v_fid_pid_config.h */
    REG_FID_PID_RSP();

#if VOS_YES == VOS_ANALYZE_PID_MEM
    VOS_SpinLockInit(&g_vosPidMemAnaSpinLock);
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    VOS_SpinLockInit(&g_vosQuereyPidInfoSpinLock);

    for (i = 0; i < (VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND); i++) {
        g_vosQueryCcpuPidInfo[i].modemId    = MODEM_ID_BUTT;
        g_vosQueryCcpuPidInfo[i].pidInvaild = VOS_PID_UNSURE;
    }

    for (i = 0; i < (VOS_CPU_ID_2_PID_BUTT - VOS_PID_CPU_ID_2_DOPRAEND); i++) {
        g_vosQueryNrcpuPidInfo[i].modemId    = MODEM_ID_BUTT;
        g_vosQueryNrcpuPidInfo[i].pidInvaild = VOS_PID_UNSURE;
    }
#endif

    return (VOS_OK);
}

/*
 * Description: one step of PID initialization
 */
VOS_UINT32 VOS_PidsInitOnePhase(enum VOS_InitPhaseDefine initPhrase)
{
    VOS_UINT32 i;
    VOS_UINT32 returnValue;
    VOS_UINT32 errorCounter;

    errorCounter = 0;

    for (i = VOS_PID_DOPRAEND; i < VOS_PID_BUTT; i++) {
        g_pidInitId = (VOS_UINT16)i;
        V_LogRecord(8, (VOS_UINT32)g_pidInitId);

        if (g_vosPidRecords[i - VOS_PID_DOPRAEND].initFunc == VOS_NULL_PTR) {
            continue;
        }

        returnValue = g_vosPidRecords[i - VOS_PID_DOPRAEND].initFunc(initPhrase);
        if (returnValue != VOS_OK) {
            g_vosPidRecords[i - VOS_PID_DOPRAEND].isError   = VOS_TRUE;
            g_vosPidRecords[i - VOS_PID_DOPRAEND].initPhase = (VOS_UINT8)initPhrase;
            mdrv_err("<VOS_PidsInitOnePhase> InitFunc Fail. Pid=0x%x\n", i);
            /* 由于PID扩展后,PID已经扩展超过了0xffff，需要扩展 */
            g_omPidInit |= (i & 0xFFFFFF);
            V_LogRecord(9, g_omPidInit);
            errorCounter++;
        }
    }

    return (errorCounter);
}

/*
 * Description: Init PID
 */
VOS_UINT32 VOS_PidsInit(VOS_VOID)
{
    VOS_UINT32 startUpFailStage = VOS_PID_INIT_ERR_BASE;

    VOS_UINT32 initPhase;

    for (initPhase = VOS_IP_LOAD_CONFIG; initPhase <= VOS_IP_BUTT; initPhase++) {
        g_pidInitStep = (VOS_UINT16)initPhase;
        V_LogRecord(6, g_pidInitStep);

        if (VOS_OK != VOS_PidsInitOnePhase((enum VOS_InitPhaseDefine)initPhase)) {
            mdrv_err("<VOS_PidsInitOnePhase> VOS_IP_LOAD_CONFIG Error. InitPhase=0x%x\n", initPhase);

            g_omPidInit |= (initPhase << 24);
            V_LogRecord(7, g_omPidInit);

            startUpFailStage |= initPhase;

            return (startUpFailStage);
        }
    }

    return VOS_OK;
}

/*
 * Description: default init function of FID
 */
VOS_UINT32 FidInitFuncDefault(enum VOS_InitPhaseDefine initPhrase)
{
    return (VOS_OK);
}

/*
 * Description: Init the control block of FID
 */
VOS_UINT32 VOS_FidCtrlBlkInit(VOS_VOID)
{
    VOS_UINT32 i;
    int        j;

    VOS_FidControlBlock *fidCtrlBlk     = g_vosFidCtrlBlk;
    const FID_StaticTableType *fidStaticTable = VOS_NULL_PTR;

    for (i = 0; i < (VOS_UINT32)VOS_FID_BUTT; i++) {
        fidCtrlBlk->fid = (VOS_INT)i;

        fidCtrlBlk->initFunc     = FidInitFuncDefault;
        fidCtrlBlk->taskFunction = vos_FidTask;
        fidCtrlBlk->priority     = g_vosFidTaskRealPriority[VOS_PRIORITY_BASE];
        fidCtrlBlk->stackSize    = TASK_STACK_SIZE;


        fidCtrlBlk->isError      = VOS_FALSE;
        fidCtrlBlk->initPhase    = VOS_IP_BUTT;

        for (j = 0; j < VOS_MAX_SELF_TASK_OF_FID; j++) {
            fidCtrlBlk->selfProcTaskFunc[j] = VOS_NULL_PTR;
            fidCtrlBlk->selfProcTaskTid[j]  = VOS_NULL_DWORD;

        }
        /* which should be del when only one FID exists */
        fidCtrlBlk->maxQidSize = DEFAULT_FID_QUEUE_NUM;
        fidCtrlBlk++;
    }

    for (i = (VOS_UINT32)VOS_FID_DOPRAEND; i < (VOS_UINT32)VOS_FID_BUTT; i++) {
        fidCtrlBlk     = &g_vosFidCtrlBlk[i];
        fidStaticTable = &g_aFidStaticTable[i];

        if (fidStaticTable->fid != (unsigned long)i) {
            mdrv_err("<VOS_FidCtrlBlkInit> Fid Init Fid invalid ulFID=%d I=%d", (int)fidStaticTable->fid, i);

            return (VOS_ERR);
        }

        for (j = 0; j < VOS_ID_MAX_FID_NAME_LENGTH; j++) {
            fidCtrlBlk->name[j] = (char)(fidStaticTable->achFIDName[j]);
        }
        fidCtrlBlk->name[j - 1] = '\0';

        fidCtrlBlk->initFunc   = fidStaticTable->pfnInitFun;
        fidCtrlBlk->startOrder = fidStaticTable->startOrder;

        if (0 != fidStaticTable->stackSize) {
            fidCtrlBlk->stackSize = fidStaticTable->stackSize;
        } else {
            fidCtrlBlk->stackSize = TASK_STACK_SIZE;
        }

        /* which should be del when only one FID exists */
        if (0 != fidStaticTable->queueSize) {
            fidCtrlBlk->maxQidSize = fidStaticTable->queueSize;
        }
    }

    return (VOS_OK);
}

/*
 * Description: one step of FID initialization
 */
VOS_UINT32 VOS_FidsInitOnePhase(enum VOS_InitPhaseDefine initPhrase)
{
    VOS_UINT32 i;
    VOS_UINT32 returnValue;
    VOS_UINT32 errorCounter;

    errorCounter = 0;

    for (i = (VOS_UINT32)VOS_FID_DOPRAEND; i < (VOS_UINT32)VOS_FID_BUTT; i++) {
        g_fidInitId = (VOS_UINT16)i;
        V_LogRecord(4, g_fidInitId);

        if (g_vosFidCtrlBlk[i].initFunc == VOS_NULL_PTR) {
            continue;
        }

        returnValue = g_vosFidCtrlBlk[i].initFunc(initPhrase);
        if (returnValue != VOS_OK) {
            g_vosFidCtrlBlk[i].isError   = VOS_TRUE;
            g_vosFidCtrlBlk[i].initPhase = (VOS_UINT8)initPhrase;
            mdrv_err("<VOS_FidsInitOnePhase> Name=%s InitPhrase=%d i=%d returnValueValue=%x", g_vosFidCtrlBlk[i].name,
                     initPhrase, i, returnValue);

            g_omFidInit |= (i & VOS_NULL_WORD);
            V_LogRecord(5, g_omFidInit);

            errorCounter++;
        }
    }

    return (errorCounter);
}

/*
 * Description: Init PID
 */
VOS_UINT32 VOS_FidsInit(VOS_VOID)
{
    VOS_UINT32 startUpFailStage = VOS_FID_INIT_ERR_BASE;

    VOS_UINT32 initPhase;

    for (initPhase = VOS_IP_LOAD_CONFIG; initPhase <= VOS_IP_BUTT; initPhase++) {
        g_fidInitStep = (VOS_UINT16)initPhase;
        V_LogRecord(2, g_fidInitStep);

        if (VOS_OK != VOS_FidsInitOnePhase((enum VOS_InitPhaseDefine)initPhase)) {
            mdrv_err("<VOS_FidsInitOnePhase> VOS_IP_LOAD_CONFIG Error.\n");

            g_omFidInit |= (initPhase << 16);
            V_LogRecord(3, g_omFidInit);

            startUpFailStage |= initPhase;

            return (startUpFailStage);
        }
    }

    return VOS_OK;
}

/*
 * Description: Register priority of FID message handling task.
 */
VOS_UINT32 VOS_RegisterMsgTaskPrio(VOS_FID fid, enum VOS_PriorityDefine taskPrio)
{
    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);
        return (VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);
    }

    if (taskPrio > VOS_PRIORITY_P6) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT2INVALID);
        return (VOS_ERRNO_FIDPID_REGMPRI_INPUT2INVALID);
    }

    g_vosFidCtrlBlk[fid].priority = g_vosFidTaskRealPriority[taskPrio];

    return (VOS_OK);
}

/*
 * Description: Register real priority of FID message handling task.
 */
VOS_UINT32 VOS_RegisterTaskPrio(VOS_FID fid, VOS_UINT32 taskRealPri)
{
    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);
        return (VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);
    }

    g_vosFidCtrlBlk[fid].priority = taskRealPri;

    return (VOS_OK);
}

/*
 * Description: create queue of one FID
 */
VOS_UINT32 CreateFidsQueque(VOS_VOID)
{
    VOS_UINT32 i;
    VOS_UINT32 returnValue;

    for (i = (VOS_UINT32)VOS_FID_DOPRAEND; i < (VOS_UINT32)VOS_FID_BUTT; i++) {
        returnValue = VOS_FixedQueueCreate(VOS_FID_QUEUE_LENGTH, &(g_vosFidCtrlBlk[i].qid), VOS_MSG_Q_FIFO,
                                             VOS_FID_MAX_MSG_LENGTH, g_vosFidCtrlBlk[i].maxQidSize);
        if (returnValue != VOS_OK) {
            mdrv_err("<CreateFidsQueque> create=%d FID queue error.\n", i);

            return (returnValue);
        }
    }

    return (VOS_OK);
}

VOS_MsgHookFunc vos_MsgHook = VOS_NULL_PTR;

/*
 * Description: Register the hook for VOS_GetMsg
 */
VOS_MsgHookFunc VOS_RegisterMsgGetHook(VOS_MsgHookFunc pfnMsgHook)
{
    VOS_MsgHookFunc oldMsgHook;

    oldMsgHook  = vos_MsgHook;
    vos_MsgHook = pfnMsgHook;

    return (oldMsgHook);
}

/*
 * Description: the process entry of every FID
 */
VOS_VOID vos_FidTask(VOS_UINT32 queueID, VOS_UINT32 fidValue, VOS_UINT32 para1, VOS_UINT32 para2)
{
    MSG_CB      *msg       = VOS_NULL_PTR;
    VOS_UINT32  *msgAddr = VOS_NULL_PTR;
    VOS_UINT32   iThePid;
    VOS_UINT_PTR msgAddress;

    for (;;) {
        if (VOS_ERR == VOS_FixedQueueRead(queueID, 0, &msgAddress, VOS_FID_MAX_MSG_LENGTH)) {
            mdrv_err("<vos_FidTask> FID fetch message error. the Q ID is %d.\n", (int)queueID);

            continue;
        }

        msgAddress += VOS_MSG_BLK_HEAD_LEN;

        msg = (MSG_CB *)msgAddress;

        iThePid = msg->ulReceiverPid;

        if (iThePid >= VOS_PID_BUTT) {
            mdrv_err("<vos_FidTask> Pid too big.\n");

            msgAddr = (VOS_UINT32 *)msg;
            mdrv_err("<vos_FidTask> Msg :S pid=%08X R Pid=%08X Length=%08X.\n", (VOS_INT)(*(msgAddr + 1)),
                     (VOS_INT)(*(msgAddr + 3)), (VOS_INT)(*(msgAddr + 4)));

            (VOS_VOID)VOS_FreeMsg(iThePid, msg);

            continue;
        }

        if (fidValue == (VOS_UINT32)(g_vosPidRecords[iThePid - VOS_PID_DOPRAEND].fid)) {
            if (VOS_NULL_PTR != vos_MsgHook) {
                (VOS_VOID)(vos_MsgHook)(msg);
            }
            (g_vosPidRecords[iThePid - VOS_PID_DOPRAEND].msgFunction)(msg);
        } else {
            mdrv_err("<vos_FidTask> Pid not belong the Fid.\n");

            msgAddr = (VOS_UINT32 *)msg;
            mdrv_err("<vos_FidTask> Msg :S pid=%08X R Pid=%08X Length=%08X.\n", (VOS_INT)(*(msgAddr + 1)),
                     (VOS_INT)(*(msgAddr + 3)), (VOS_INT)(*(msgAddr + 4)));
        }

        (VOS_VOID)VOS_FreeMsg(iThePid, msg);
    }
}


/*
 * Description: Check CoreMask Validity
 */
VOS_UINT32 VOS_IsCoreMaskValid(VOS_UINT32 coreMask)
{
    if (VOS_CORE_MASK_NULL == coreMask) {
        return VOS_ERRNO_CORE_MASK_INVALID;
    }

    return VOS_OK;
}

/*
 * Description: get all cores mask
 */
VOS_UINT32 VOS_GetAllCoresMask(VOS_VOID)
{
    return VOS_CORE_MASK_CORE0;
}

/*
 * Description: Register core bind of FID message handling task.
 */
VOS_UINT32 VOS_RegisterFidTaskCoreBind(VOS_FID fid, VOS_UINT32 coreMask)
{
    if (VOS_OK != VOS_IsCoreMaskValid(coreMask)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_CORE_MASK_INVALID);

        return VOS_ERRNO_CORE_MASK_INVALID;
    }

    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);

        return VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID;
    }

    return VOS_OK;
}

/*
 * Description: Register a self-handling task
 */
VOS_UINT32 VOS_RegisterSelfTaskCoreBind(VOS_FID fid, VOS_UINT8 vosIndex, VOS_UINT32 coreMask)
{
    if (VOS_OK != VOS_IsCoreMaskValid(coreMask)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_CORE_MASK_INVALID);

        return VOS_ERRNO_CORE_MASK_INVALID;
    }

    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);

        return VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID;
    }

    if (vosIndex >= VOS_MAX_SELF_TASK_OF_FID) {
        return VOS_NULL_BYTE;
    }

    return VOS_OK;
}

/*
 * Description: CoreBind fid task
 */
VOS_UINT32 VOS_CoreBindFidTask(VOS_FID fid, VOS_UINT32 coreMask)
{
    if (VOS_OK != VOS_IsCoreMaskValid(coreMask)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_CORE_MASK_INVALID);

        return VOS_ERRNO_CORE_MASK_INVALID;
    }

    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);

        return VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID;
    }

    return VOS_OK;
}

/*
 * Description: CoreBind self task
 */
VOS_UINT32 VOS_CoreBindSelfTask(VOS_FID fid, VOS_UINT8 vosIndex, VOS_UINT32 coreMask)
{
    if (VOS_OK != VOS_IsCoreMaskValid(coreMask)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_CORE_MASK_INVALID);

        return VOS_ERRNO_CORE_MASK_INVALID;
    }

    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);

        return VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID;
    }

    if (vosIndex >= VOS_MAX_SELF_TASK_OF_FID) {
        return VOS_NULL_BYTE;
    }

    return VOS_OK;
}


/*
 * Description: create tasks of all FIDs
 */
VOS_UINT32 CreateFidsTask(VOS_VOID)
{
    VOS_UINT32 i;
    int        ubyIndex;
    VOS_UINT32 returnValue;
    VOS_UINT32 argument[VOS_TARG_NUM] = {0};
    VOS_CHAR   acSelfTaskName[VOS_MAX_LENGTH_TASK_NAME];

    argument[3] = VOS_NULL_DWORD;

    for (i = VOS_FID_DOPRAEND; i < VOS_FID_BUTT; i++) {
        g_vosFidCtrlBlk[i].taskArgument[0] = g_vosFidCtrlBlk[i].qid;
        g_vosFidCtrlBlk[i].taskArgument[1] = i;
        g_vosFidCtrlBlk[i].taskArgument[3] = VOS_NULL_DWORD;

        returnValue = VOS_CreateTaskOnly(g_vosFidCtrlBlk[i].name, &(g_vosFidCtrlBlk[i].tid),
                                           g_vosFidCtrlBlk[i].taskFunction, g_vosFidCtrlBlk[i].priority,
                                           g_vosFidCtrlBlk[i].stackSize, g_vosFidCtrlBlk[i].taskArgument);
        if (returnValue != VOS_OK) {
            mdrv_err("<CreateFidsTask> Fail in CreateFidsTask, FID=%d\n", i);
            return (returnValue);
        }


        (VOS_VOID)VOS_RecordFIDTIDInfo(i, g_vosFidCtrlBlk[i].tid);

        for (ubyIndex = 0; ubyIndex < VOS_MAX_SELF_TASK_OF_FID; ubyIndex++) {
            if (g_vosFidCtrlBlk[i].selfProcTaskFunc[ubyIndex] != VOS_NULL_PTR) {
                if (snprintf_s(acSelfTaskName, VOS_MAX_LENGTH_TASK_NAME, VOS_MAX_LENGTH_TASK_NAME - 1, "F%d_I%d", i, ubyIndex)
                    <= VOS_NULL) {
                    acSelfTaskName[0] = 'N';
                    acSelfTaskName[1] = 'A';
                    acSelfTaskName[2] = 0;
                    mdrv_err("<CreateFidsTask> Fail in name, FID=%d.\n", i);
                }
                returnValue = VOS_CreateTaskOnly(acSelfTaskName, &(g_vosFidCtrlBlk[i].selfProcTaskTid[ubyIndex]),
                                                   g_vosFidCtrlBlk[i].selfProcTaskFunc[ubyIndex],
                                                   g_vosFidCtrlBlk[i].selfProcTaskPriority[ubyIndex],
                                                   g_vosFidCtrlBlk[i].selfProcTaskStack[ubyIndex], argument);
                if (returnValue != VOS_OK) {
                    mdrv_err("<CreateFidsTask> Fail in CreateFidsTask, FID=%d.\n", i);
                    return (returnValue);
                }

            }
        }
    }

    return (VOS_OK);
}

/*
 * Description: Register a self-handling task
 */
VOS_UINT8 VOS_RegisterSelfTask(VOS_FID fid, VOS_TaskEntryType pfnSelfTask, enum VOS_PriorityDefine taskPrio,
                               VOS_UINT32 stackSize)
{
    VOS_UINT8 ubyIndex;

    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);
        return VOS_NULL_BYTE;
    }

    if (taskPrio > VOS_PRIORITY_P6) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT2INVALID);
        return VOS_NULL_BYTE;
    }

    for (ubyIndex = 0; ubyIndex < VOS_MAX_SELF_TASK_OF_FID; ubyIndex++) {
        if ((VOS_NULL_PTR == g_vosFidCtrlBlk[fid].selfProcTaskFunc[ubyIndex]) &&
            (VOS_NULL_DWORD == g_vosFidCtrlBlk[fid].selfProcTaskTid[ubyIndex])) {
            break;
        }
    }

    if (ubyIndex >= VOS_MAX_SELF_TASK_OF_FID) {
        mdrv_err("<VOS_RegisterSelfTask> vosIndex exceed VOS_MAX_SELF_TASK_OF_FID.\n");

        return VOS_NULL_BYTE;
    }

    g_vosFidCtrlBlk[fid].selfProcTaskFunc[ubyIndex]     = pfnSelfTask;
    g_vosFidCtrlBlk[fid].selfProcTaskPriority[ubyIndex] = g_vosFidTaskRealPriority[taskPrio];
    g_vosFidCtrlBlk[fid].selfProcTaskStack[ubyIndex]    = stackSize;

    return ubyIndex;
}

/*
 * Description: Register a self-handling task
 */
VOS_UINT8 VOS_RegisterSelfTaskPrio(VOS_FID fid, VOS_TaskEntryType pfnSelfTask, VOS_UINT32 taskRealPri,
                                   VOS_UINT32 stackSize)
{
    VOS_UINT8 ubyIndex;

    if (fid >= VOS_FID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_FIDPID_REGMPRI_INPUT1INVALID);
        return VOS_NULL_BYTE;
    }

    for (ubyIndex = 0; ubyIndex < VOS_MAX_SELF_TASK_OF_FID; ubyIndex++) {
        if ((VOS_NULL_PTR == g_vosFidCtrlBlk[fid].selfProcTaskFunc[ubyIndex]) &&
            (VOS_NULL_DWORD == g_vosFidCtrlBlk[fid].selfProcTaskTid[ubyIndex])) {
            break;
        }
    }

    if (ubyIndex >= VOS_MAX_SELF_TASK_OF_FID) {
        mdrv_err("<VOS_RegisterSelfTask> vosIndex exceed VOS_MAX_SELF_TASK_OF_FID.\n");

        return VOS_NULL_BYTE;
    }

    g_vosFidCtrlBlk[fid].selfProcTaskFunc[ubyIndex]     = pfnSelfTask;
    g_vosFidCtrlBlk[fid].selfProcTaskPriority[ubyIndex] = taskRealPri;
    g_vosFidCtrlBlk[fid].selfProcTaskStack[ubyIndex]    = stackSize;

    return ubyIndex;
}

#if (VOS_LINUX != VOS_OS_VER)
/*
 * Description: Suspend task of FID
 */
VOS_UINT32 VOS_SuspendFidTask(VOS_FID fid)
{
    if ((VOS_FID_DOPRAEND > fid) || (VOS_FID_BUTT <= fid)) {
        return VOS_ERR;
    }

    return VOS_SuspendTask(g_vosFidCtrlBlk[fid].tid);
}
#endif

/*
 * Description: Resume task of FID
 */
VOS_UINT32 VOS_ResumeFidTask(VOS_FID fid)
{
    if ((VOS_FID_DOPRAEND > fid) || (VOS_FID_BUTT <= fid)) {
        return VOS_ERR;
    }

    return VOS_ResumeTask(g_vosFidCtrlBlk[fid].tid);
}

#if (VOS_LINUX != VOS_OS_VER)
/*
 * Description: Suspend tasks of all FIDs
 */
VOS_UINT32 VOS_SuspendFidsTask(VOS_VOID)
{
    VOS_UINT32 i;
    int        ubyIndex;
    VOS_UINT32 returnValue;

    for (i = VOS_FID_DOPRAEND; i < VOS_FID_BUTT; i++) {
        returnValue = VOS_SuspendTask(g_vosFidCtrlBlk[i].tid);
        if (returnValue != VOS_OK) {
            mdrv_err("<SuspendFidsTask> Fail 1 in SuspendFidsTask.\n");
            return (returnValue);
        }

        for (ubyIndex = 0; ubyIndex < VOS_MAX_SELF_TASK_OF_FID; ubyIndex++) {
            if (g_vosFidCtrlBlk[i].selfProcTaskFunc[ubyIndex] != VOS_NULL_PTR) {
                returnValue = VOS_SuspendTask(g_vosFidCtrlBlk[i].selfProcTaskTid[ubyIndex]);
                if (returnValue != VOS_OK) {
                    mdrv_err("<SuspendFidsTask> Fail 2 in SuspendFidsTask.\n");

                    return (returnValue);
                }
            }
        }
    }

    return (VOS_OK);
}
#endif

/*
 * Description: Resume tasks of all FIDs
 */
VOS_UINT32 VOS_ResumeFidsTask(VOS_VOID)
{
    VOS_UINT32 i;
    int        ubyIndex;
    VOS_UINT32 returnValue;

    for (i = VOS_FID_DOPRAEND; i < VOS_FID_BUTT; i++) {
        returnValue = VOS_ResumeTask(g_vosFidCtrlBlk[i].tid);
        if (returnValue != VOS_OK) {
            mdrv_err("<ResumeFidsTask> Fail 1 in ResumeFidsTask.\n");
            return (returnValue);
        }

        for (ubyIndex = 0; ubyIndex < VOS_MAX_SELF_TASK_OF_FID; ubyIndex++) {
            if (g_vosFidCtrlBlk[i].selfProcTaskFunc[ubyIndex] != VOS_NULL_PTR) {
                returnValue = VOS_ResumeTask(g_vosFidCtrlBlk[i].selfProcTaskTid[ubyIndex]);
                if (returnValue != VOS_OK) {
                    mdrv_err("<ResumeFidsTask> Fail 2 in ResumeFidsTask.\n");

                    return (returnValue);
                }
            }
        }
    }

    return (VOS_OK);
}

#if (VOS_LINUX != VOS_OS_VER)
/*
 * Description: suspend all FID & selftask
 */
VOS_VOID VOS_SuspendAllTask(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3)
{
    if (VOS_OK != VOS_SuspendFidsTask()) {
        mdrv_err("<VOS_SuspendAllTask> SUSPED FID error.\n");
    }

    if (VOS_OK != VOS_SuspendTask(g_vosTimerTaskId)) {
        mdrv_err("<VOS_SuspendAllTask> SUSPED VOS timer task error.\n");
    }

    if (VOS_OK != VOS_SuspendTask(g_rtcTimerTaskId)) {
        mdrv_err("<VOS_SuspendAllTask> SUSPED RTC timer task error.\n");
    }
}
#endif

/*
 * Description: Register message handling task's entry of FID
 */
VOS_UINT32 VOS_RegisterMsgTaskEntry(VOS_FID fid, VOS_TaskEntryType pfnMsgTask)
{
    if (fid >= VOS_FID_BUTT) {
        mdrv_err("<VOS_RegisterMsgTaskEntry> FID invaild.\n");
        return VOS_ERR;
    }

    if (VOS_NULL_PTR == pfnMsgTask) {
        mdrv_err("<VOS_RegisterMsgTaskEntry> task null.\n");
        return VOS_ERR;
    }

    g_vosFidCtrlBlk[fid].taskFunction = pfnMsgTask;

    return VOS_OK;
}

/*
 * Description: get the queue ID of a FId
 */
VOS_UINT32 VOS_GetQueueIDFromFid(VOS_UINT32 fid)
{
    if (fid >= VOS_FID_BUTT) {
        return 0xffffffff;
    }

    return g_vosFidCtrlBlk[fid].qid;
}

#if VOS_YES == VOS_ANALYZE_PID_MEM
/*
 * Description: Increase Pid Memory
 */
VOS_VOID VOS_IncreasePidMemory(VOS_PID pid, VOS_UINT32 size, VOS_UINT32 type)
{
    VOS_ULONG lockLevel;

    VOS_SpinLockIntLock(&g_vosPidMemAnaSpinLock, lockLevel);

    if (VOS_LOCATION_MEM == type) {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].memSize += size;

        if (g_vosPidRecords[pid - VOS_PID_DOPRAEND].memSize >
            g_vosPidRecords[pid - VOS_PID_DOPRAEND].memPeakSize) {
            g_vosPidRecords[pid - VOS_PID_DOPRAEND].memPeakSize = g_vosPidRecords[pid - VOS_PID_DOPRAEND].memSize;
        }
    } else {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].msgSize += size;

        if (g_vosPidRecords[pid - VOS_PID_DOPRAEND].msgSize >
            g_vosPidRecords[pid - VOS_PID_DOPRAEND].msgPeakSize) {
            g_vosPidRecords[pid - VOS_PID_DOPRAEND].msgPeakSize = g_vosPidRecords[pid - VOS_PID_DOPRAEND].msgSize;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosPidMemAnaSpinLock, lockLevel);

    return;
}

/*
 * Description: Decrease Pid Memory
 */
VOS_VOID VOS_DecreasePidMemory(VOS_PID pid, VOS_UINT32 size, VOS_UINT32 type)
{
    VOS_ULONG lockLevel;

    VOS_SpinLockIntLock(&g_vosPidMemAnaSpinLock, lockLevel);

    if (VOS_LOCATION_MEM == type) {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].memSize -= size;
    } else {
        g_vosPidRecords[pid - VOS_PID_DOPRAEND].msgSize -= size;
    }

    VOS_SpinUnlockIntUnlock(&g_vosPidMemAnaSpinLock, lockLevel);

    return;
}
#endif

/*
 * Description: print used msg's useage info
 */
VOS_UINT32 VOS_AnalyzePidMemory(VOS_VOID *bUffer, VOS_UINT32 size, VOS_UINT32 *num)
{
#if VOS_YES == VOS_ANALYZE_PID_MEM
    VOS_UINT32             tempSize;
    VOS_PidPeakMem *tempPtr = VOS_NULL_PTR;
    VOS_INT                i;

    if ((VOS_NULL_PTR == bUffer) || (VOS_NULL_PTR == num)) {
        return VOS_ERR;
    }

    tempSize = (VOS_PID_BUTT - VOS_PID_DOPRAEND);

    *num = tempSize;

    tempSize *= sizeof(VOS_PidPeakMem); /* 4byte msg, 4byte mem */

    if (tempSize > size) {
        return VOS_ERR;
    }

    tempPtr = (VOS_PidPeakMem *)bUffer;

    for (i = VOS_PID_DOPRAEND; i < VOS_PID_BUTT; i++) {
        tempPtr->pid         = g_vosPidRecords[i - VOS_PID_DOPRAEND].pid;
        tempPtr->msgPeakSize = g_vosPidRecords[i - VOS_PID_DOPRAEND].msgPeakSize;
        tempPtr->memPeakSize = g_vosPidRecords[i - VOS_PID_DOPRAEND].memPeakSize;

        tempPtr++;
    }

    return VOS_OK;
#else
    return VOS_ERR;
#endif
}

/*
 * Description: Get Tid From Pid
 */
VOS_UINT32 VOS_GetTCBFromPid(VOS_UINT32 pid)
{
    VOS_UINT32 fid;

    if ((pid < VOS_PID_DOPRAEND) || (pid >= VOS_PID_BUTT)) {
        mdrv_err("<VOS_GetTCBFromPid> Wrong Pid=%d\n", pid);

        return VOS_NULL;
    }

    fid = (VOS_UINT32)(g_vosPidRecords[pid - VOS_PID_DOPRAEND].fid);
    if ((fid < (VOS_UINT32)VOS_FID_DOPRAEND) || (fid >= (VOS_UINT32)VOS_FID_BUTT)) {
        mdrv_err("<VOS_GetTCBFromPid> Wrong PID=%d. FID=%d\n", (VOS_INT)pid, (VOS_INT)fid);

        return VOS_NULL;
    }

    return VOS_GetRealTID(g_vosFidCtrlBlk[fid].tid);
}

#if ((OSA_CPU_NRCPU == VOS_OSA_CPU) || (OSA_CPU_CCPU == VOS_OSA_CPU))
/*
 * Description: Get Modem ID From Pid
 */
ModemIdUint16 VOS_GetModemIDFromPid(VOS_UINT32 pid)
{
    VOS_UINT32 cpuID;

    cpuID = VOS_GetCpuId(pid);
#if (OSA_CPU_NRCPU == VOS_OSA_CPU)
    if (VOS_CPU_ID_NRCPU != cpuID) {
        mdrv_err("<VOS_GetModemIDFromPid> CPU=%d\n", pid);

        return MODEM_ID_BUTT;
    }
#else
    if (VOS_CPU_ID_CCPU != cpuID) {
        mdrv_err("<VOS_GetModemIDFromPid> CPU=%d\n", pid);

        return MODEM_ID_BUTT;
    }
#endif

    if ((pid < VOS_PID_DOPRAEND) || (pid >= VOS_PID_BUTT)) {
        mdrv_err("<VOS_GetModemIDFromPid> Pid=%d\n", pid);

        return MODEM_ID_BUTT;
    }

    return (g_vosPidRecords[pid - VOS_PID_DOPRAEND].modemId);
}

#endif

/*
 * Description: check Protocol Stack's Pid validity
 */
VOS_BOOL VOS_CheckPSPidValidity(VOS_UINT32 pid)
{
    if (((VOS_CPU_ID_0_PID_RESERVED < pid) && (pid < VOS_CPU_ID_0_PID_BUTT)) ||
        ((VOS_CPU_ID_1_PID_RESERVED < pid) && (pid < VOS_CPU_ID_1_PID_BUTT)) ||
        ((VOS_CPU_ID_2_PID_RESERVED < pid) && (pid < VOS_CPU_ID_2_PID_BUTT))) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * Description: global variable show
 */
VOS_VOID VOS_OsaGlobalShow(VOS_VOID)
{
    mdrv_debug("<VOS_OsaGlobalShow> g_omFidInit=%d\n", g_omFidInit);
    mdrv_debug("<VOS_OsaGlobalShow> g_omPidInit=%d\n", g_omPidInit);
    mdrv_debug("<VOS_OsaGlobalShow> g_fidInitStep=%d\n", g_fidInitStep);
    mdrv_debug("<VOS_OsaGlobalShow> g_fidInitId=%d\n", g_fidInitId);
    mdrv_debug("<VOS_OsaGlobalShow> g_pidInitStep=%d\n", g_pidInitStep);
    mdrv_debug("<VOS_OsaGlobalShow> g_pidInitId=%d\n", g_pidInitId);

    return;
}

/*
 * Description: get the real priority of a task
 */
VOS_UINT32 VOS_GetFIdRelPri(enum VOS_PriorityDefine fidPri, VOS_UINT32 *taskRealPri00001)
{
    if (fidPri > VOS_PRIORITY_P6) {
        return VOS_ERR;
    }

    if (VOS_NULL_PTR == taskRealPri00001) {
        return VOS_ERR;
    }

    *taskRealPri00001 = g_vosFidTaskRealPriority[fidPri];

    return VOS_OK;
}

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
/*
 * Description: check the Validity of a pid
 */
VOS_UINT32 VOS_CheckPidValidity(VOS_UINT32 pid)
{
    VOS_UINT32 cpuID;
    VOS_UINT32 fid;

    cpuID = VOS_GetCpuId(pid);
    if (VOS_CPU_ID_CCPU == cpuID) {
        if ((pid < VOS_PID_CPU_ID_0_DOPRAEND) || (pid >= VOS_CPU_ID_0_PID_BUTT)) {
            mdrv_err("<VOS_CheckPidValidity> Wrong Pid=%d\n", pid);

            return VOS_PID_UNSURE;
        }

        return g_vosQueryCcpuPidInfo[pid - VOS_PID_CPU_ID_0_DOPRAEND].pidInvaild;
    }

    if (VOS_CPU_ID_NRCPU == cpuID) {
        if ((pid < VOS_PID_CPU_ID_2_DOPRAEND) || (pid >= VOS_CPU_ID_2_PID_BUTT)) {
            mdrv_err("<VOS_CheckPidValidity> Wrong Pid=%d\n", pid);

            return VOS_PID_UNSURE;
        }

        return g_vosQueryNrcpuPidInfo[pid - VOS_PID_CPU_ID_2_DOPRAEND].pidInvaild;
    }

    if (VOS_CPU_ID_ACPU == cpuID) {
        if ((pid < VOS_PID_DOPRAEND) || (pid >= VOS_PID_BUTT)) {
            mdrv_err("<VOS_CheckPidValidity> Wrong Pid=%d\n", pid);

            return VOS_PID_UNSURE;
        }

        fid = (VOS_UINT32)(g_vosPidRecords[pid - VOS_PID_DOPRAEND].fid);
        if ((fid < (VOS_UINT32)VOS_FID_DOPRAEND) || (fid >= (VOS_UINT32)VOS_FID_BUTT)) {
            return VOS_PID_INVAILD;
        }

        return VOS_PID_AVAILABLE;
    }

    mdrv_err("<VOS_CheckPidValidity> Wrong CPU=%d\n", pid);

    return VOS_PID_UNSURE;
}

/*
 * Description: set the pid's info which comes from another core
 */
VOS_VOID VOS_SetCpuPidInfo(VOS_UINT32 cpuID, VOS_VOID *buf, VOS_UINT16 len)
{
    VOS_ULONG                lockLevel;
    VOS_UINT32               i;
    VOS_UINT16               localCount = 0;
    VOS_QueryPidModemId *tempBuf       = (VOS_QueryPidModemId *)buf;

    if (VOS_CPU_ID_CCPU == cpuID) {
        localCount = VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND;
    } else if (VOS_CPU_ID_NRCPU == cpuID) {
        localCount = VOS_CPU_ID_2_PID_BUTT - VOS_PID_CPU_ID_2_DOPRAEND;
    } else {
        return;
    }

    if ((len / sizeof(VOS_QueryPidModemId)) != localCount) {
        return;
    }

    VOS_SpinLockIntLock(&g_vosQuereyPidInfoSpinLock, lockLevel);

    if (VOS_CPU_ID_CCPU == cpuID) {
        for (i = 0; i < localCount; i++) {
            g_vosQueryCcpuPidInfo[i].modemId    = tempBuf->modemId;
            g_vosQueryCcpuPidInfo[i].pidInvaild = tempBuf->pidInvaild;

            tempBuf++;
        }
    } else {
        for (i = 0; i < localCount; i++) {
            g_vosQueryNrcpuPidInfo[i].modemId    = tempBuf->modemId;
            g_vosQueryNrcpuPidInfo[i].pidInvaild = tempBuf->pidInvaild;

            tempBuf++;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosQuereyPidInfoSpinLock, lockLevel);

    return;
}

#endif

#if ((OSA_CPU_NRCPU == VOS_OSA_CPU) || (OSA_CPU_CCPU == VOS_OSA_CPU))
/*
 * Description: get the size of pid's info
 */
VOS_UINT32 VOS_QueryPidInfoBufSize(VOS_VOID)
{
    return (VOS_PID_BUTT - VOS_PID_DOPRAEND) * sizeof(VOS_QueryPidModemId);
}

/*
 * Description: copy pid's info.
 */
VOS_VOID VOS_QueryPidInfo(VOS_VOID *buf, VOS_UINT32 len)
{
    VOS_UINT32               i;
    VOS_UINT32               fid;
    VOS_QueryPidModemId *tempBuf = (VOS_QueryPidModemId *)buf;

    if (buf == VOS_NULL_PTR) {
        return;
    }

    if (len < VOS_QueryPidInfoBufSize()) {
        return;
    }

    for (i = VOS_PID_DOPRAEND; i < VOS_PID_BUTT; i++) {
        tempBuf->modemId = g_vosPidRecords[i - VOS_PID_DOPRAEND].modemId;

        fid = (VOS_UINT32)(g_vosPidRecords[i - VOS_PID_DOPRAEND].fid);
        if ((fid < (VOS_UINT32)VOS_FID_DOPRAEND) || (fid >= (VOS_UINT32)VOS_FID_BUTT)) {
            tempBuf->pidInvaild = VOS_PID_INVAILD;
        } else {
            tempBuf->pidInvaild = VOS_PID_AVAILABLE;
        }

        tempBuf++;
    }
}

/*
 * Description: check the Validity of a pid.
 */
VOS_UINT32 VOS_CheckPidValidity(VOS_UINT32 pid)
{
    VOS_UINT32 cpuID;
    VOS_UINT32 fid;

    cpuID = VOS_GetCpuId(pid);
#if (OSA_CPU_NRCPU == VOS_OSA_CPU)
    if (VOS_CPU_ID_NRCPU != cpuID) {
        mdrv_err("<VOS_CheckPidValidity> Wrong CPU=%d\n", pid);

        return VOS_PID_UNSURE;
    }
#else
    if (VOS_CPU_ID_CCPU != cpuID) {
        mdrv_err("<VOS_CheckPidValidity> Wrong CPU=%d\n", pid);

        return VOS_PID_UNSURE;
    }
#endif

    if ((pid < VOS_PID_DOPRAEND) || (pid >= VOS_PID_BUTT)) {
        mdrv_err("<VOS_CheckPidValidity> Wrong Pid=%d\n", pid);

        return VOS_PID_UNSURE;
    }

    fid = (VOS_UINT32)(g_vosPidRecords[pid - VOS_PID_DOPRAEND].fid);
    if ((fid < (VOS_UINT32)VOS_FID_DOPRAEND) || (fid >= (VOS_UINT32)VOS_FID_BUTT)) {
        return VOS_PID_INVAILD;
    }

    return VOS_PID_AVAILABLE;
}
#endif

