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
 * 功能描述: OSA消息功能实现
 * 生成日期: 2006年10月3日
 */

#include "v_msg.h"
#include "v_blk_mem.h"
#include "v_queue.h"
#include "vos_id.h"
#include "v_timer.h"
#include "v_id_def.h"
#include "v_private.h"
#include "mdrv.h"
#include "pam_tag.h"

#if (VOS_OS_VER != VOS_WIN32)
#include <securec.h>
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_V_MSG_C
#define THIS_MODU mod_pam_osa

/* 支持互相发送消息的多核的最大数目 */
#define VOS_SUPPORT_CPU_NUM_MAX (5)

extern VOS_MsgHookFunc vos_MsgHook;

#define MSG_SEND_SIZE (sizeof(VOS_UINT_PTR))

#define VOS_ICC_CHANNEL_PRIORITY (0)

#define VOS_ICC_HANDSHAKE_TIME_MAX (200000)

#if (FEATURE_MULTI_MODEM == FEATURE_ON)
#define OSA_ICC_BUFFER_SIZE (128 * 1024)
#else
#define OSA_ICC_BUFFER_SIZE (64 * 1024)
#endif



#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#define VOS_AP_TO_NR_EICC_ID (EICC_CHANNEL_ID_MAKEUP(EICC_AP_SEND_NRCCPU_OSA_0, 0))
#define VOS_NR_TO_AP_EICC_ID (EICC_CHANNEL_ID_MAKEUP(EICC_AP_RECV_NRCCPU_OSA_0, 0))
#define VOS_AP_TO_NR_SRAM_SIZE (OSA_ICC_BUFFER_SIZE) /* unit is byte */
#define VOS_NR_TO_AP_SRAM_SIZE (OSA_ICC_BUFFER_SIZE) /* unit is byte */
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#define VOS_CCPU_TO_NR_EICC_ID (EICC_CHANNEL_ID_MAKEUP(EICC_LRCCPU_SEND_NRCCPU_OSA_0, 0))
#define VOS_NR_TO_CCPU_EICC_ID (EICC_CHANNEL_ID_MAKEUP(EICC_LRCCPU_RECV_NRCCPU_OSA_0, 0))
#define VOS_CCPU_TO_NR_SRAM_SIZE (OSA_ICC_BUFFER_SIZE) /* unit is byte */
#define VOS_NR_TO_CCPU_SRAM_SIZE (OSA_ICC_BUFFER_SIZE) /* unit is byte */
#endif


#if (VOS_DEBUG == VOS_DOPRA_VER)

MEMORY_HOOK_FUNC g_funcAllocMsgHook = VOS_NULL_PTR;
MEMORY_HOOK_FUNC g_funcFreeMsgHook  = VOS_NULL_PTR;

#endif

typedef struct {
    VOS_UINT32 qid;
    VOS_UINT32 sendPid;
    VOS_UINT32 rcvPid;
    VOS_UINT32 msgName;
} VOS_DumpMsg;

/* dump msg info */
#define DUMP_MSG_INFO_LEN 1024
VOS_CHAR g_dumpMsgInfo[DUMP_MSG_INFO_LEN];

typedef struct {
    VOS_UINT32 sendNum;
    VOS_UINT32 sendSlice;
    VOS_UINT32 sendLen;
    VOS_UINT32 rcvNum;
    VOS_UINT32 rcvSlice;
    VOS_UINT32 rcvLen;
} VOS_IccDebugInfo;

/* 用于ICC通道UDI控制数据片结构 */
typedef struct {
    VOS_UINT32      iccId;   /* 底软定义的当前通道号 */
    icc_chan_attr_s iccAttr; /* 当前通道属性 */
} VOS_IccUdiCtrl;

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
VOS_UINT32 V_NRCcpuSendMsgByICC(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo);
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
VOS_UINT32 V_NRAcpuSendMsgByICC(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo);
#endif



#if (FEATURE_ON == FEATURE_HIFI_USE_ICC) /* ICC */

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
VOS_STATIC VOS_CONST VOS_SendmsgFunclist g_vosSendMsgProcTable[VOS_SUPPORT_CPU_NUM_MAX] = {
    { VOS_CPU_ID_0_PID_BUTT, 0, V_SendLocalMsg, V_SendLocalUrgentMsg },       /* Send Local Msg */
    { VOS_CPU_ID_1_PID_BUTT, 0, V_SendMsgByICC, V_SendMsgByICC },             /* Send Msg to AP */
    { VOS_CPU_ID_2_PID_BUTT, 0, V_NRCcpuSendMsgByICC, V_NRCcpuSendMsgByICC }, /* Send Msg to NR */
    { VOS_CPU_ID_3_PID_BUTT, 0, V_SendHifiMsgByICC, V_SendHifiMsgByICC },     /* Send Msg to HIFI */
    { VOS_CPU_ID_4_PID_BUTT, 0, VOS_SendMCUMsg, VOS_SendMCUUrgentMsg }
}; /* Send Msg to MCU */
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
VOS_STATIC VOS_CONST VOS_SendmsgFunclist g_vosSendMsgProcTable[VOS_SUPPORT_CPU_NUM_MAX] = {
    { VOS_CPU_ID_0_PID_BUTT, 0, V_SendMsgByICC, V_SendMsgByICC },             /* Send Msg to Other CCPU */
    { VOS_CPU_ID_1_PID_BUTT, 0, V_SendLocalMsg, V_SendLocalUrgentMsg },       /* Send Local Msg */
    { VOS_CPU_ID_2_PID_BUTT, 0, V_NRAcpuSendMsgByICC, V_NRAcpuSendMsgByICC }, /* Send Msg to NR */
    { VOS_CPU_ID_3_PID_BUTT, 0, VOS_NULL_PTR, VOS_NULL_PTR },                 /* Send Msg to HIFI */
    { VOS_CPU_ID_4_PID_BUTT, 0, VOS_NULL_PTR, VOS_NULL_PTR }
}; /* Send Msg to MCU */
#endif

#else /* mailbox */

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
VOS_STATIC VOS_CONST VOS_SendmsgFunclist g_vosSendMsgProcTable[VOS_SUPPORT_CPU_NUM_MAX] = {
    { VOS_CPU_ID_0_PID_BUTT, 0, V_SendLocalMsg, V_SendLocalUrgentMsg },       /* Send Local Msg */
    { VOS_CPU_ID_1_PID_BUTT, 0, V_SendMsgByICC, V_SendMsgByICC },             /* Send Msg to AP */
    { VOS_CPU_ID_2_PID_BUTT, 0, V_NRCcpuSendMsgByICC, V_NRCcpuSendMsgByICC }, /* Send Msg to NR */
    { VOS_CPU_ID_3_PID_BUTT, 0, VOS_SendHIFIMsg, VOS_SendHIFIUrgentMsg },     /* Send Msg to HIFI */
    { VOS_CPU_ID_4_PID_BUTT, 0, VOS_SendMCUMsg, VOS_SendMCUUrgentMsg }
}; /* Send Msg to MCU */
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
VOS_STATIC VOS_CONST VOS_SendmsgFunclist g_vosSendMsgProcTable[VOS_SUPPORT_CPU_NUM_MAX] = {
    { VOS_CPU_ID_0_PID_BUTT, 0, V_SendMsgByICC, V_SendMsgByICC },             /* Send Msg to CCPU */
    { VOS_CPU_ID_1_PID_BUTT, 0, V_SendLocalMsg, V_SendLocalUrgentMsg },       /* Send Local Msg */
    { VOS_CPU_ID_2_PID_BUTT, 0, V_NRAcpuSendMsgByICC, V_NRAcpuSendMsgByICC }, /* Send Msg to NR */
    { VOS_CPU_ID_3_PID_BUTT, 0, VOS_NULL_PTR, VOS_NULL_PTR },                 /* Send Msg to HIFI */
    { VOS_CPU_ID_4_PID_BUTT, 0, VOS_NULL_PTR, VOS_NULL_PTR }
}; /* Send Msg to MCU */
#endif

#endif


enum {
    VOS_HIFI_TO_ACPU_VOS_MSG_NORMAL,
    VOS_MCU_TO_ACPU_VOS_MSG_NORMAL,
    VOS_HIFI_TO_CCPU_VOS_MSG_NORMAL,
    VOS_MAIL_BOX_MSG_NORMAL_BUTT,
    VOS_HIFI_TO_ACPU_VOS_MSG_URGENT,
    VOS_MCU_TO_ACPU_VOS_MSG_URGENT,
    VOS_HIFI_TO_CCPU_VOS_MSG_URGENT
};

/* DSP上移后提供强制唤醒功能 */
MsgFunType g_funcVosAwakeFunHook[MODEM_ID_BUTT] = {VOS_NULL_PTR};

/* 自旋锁，用来作Dump Msg的临界资源保护 */
VOS_SPINLOCK g_vosDumpMsgSpinLock;
VOS_UINT32   g_vosDumpMsgFlag;

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
VOS_UINT32 g_msglpm = VOS_FALSE;

VOS_INT VOS_MsgLpmCb(VOS_INT x)
{
    g_msglpm = VOS_TRUE;

    return 0;
}
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
/* 记录低功耗场景下ICC通道中的消息 */
typedef struct {
    VOS_UINT32 sendPid;
    VOS_UINT32 rcvPid;
    VOS_UINT32 msgName;
    VOS_UINT32 slice;
} VOS_IccDumpMsg;

#define NR_TO_LR_ICC_DUMP_MSG_MAX 5

VOS_IccDumpMsg g_nrToLrIccDumpMsg[NR_TO_LR_ICC_DUMP_MSG_MAX];

VOS_UINT32 g_nrToLrIccDumpMsgCount = 0;

VOS_UINT32 g_nrToLrIccDpm = VOS_FALSE;

VOS_VOID VOS_NrToLrIccDpmCb(VOS_VOID)
{
    g_nrToLrIccDpm = VOS_TRUE;

    return;
}
#endif

enum PM_LOG_AosaPam {
    PM_LOG_AOSA_PAM_MSG = 0x10000001,
    PM_LOG_AOSA_PAM_BUTT,
};
typedef VOS_UINT32 PM_LOG_AosaPamUint32;

typedef struct {
    PM_LOG_AosaPamUint32        type;
    VOS_UINT32                  senderPid;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  msgId;
} VOS_AMsgLog;

/* the debugging info of ICC which is between A&C core */
VOS_IccDebugInfo g_vosAcoreCcoreIccDebugInfo;

/* the debugging info of ICC which is between C&hifi core */
VOS_IccDebugInfo g_vosCcoreHifiIccDebugInfo;

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
/* the debugging info of ICC which is between NR&C core */
VOS_IccDebugInfo g_vosNRCcoreIccDebugInfo;
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
/* the debugging info of ICC which is between A&NR core */
VOS_IccDebugInfo g_vosNRAcoreIccDebugInfo;
#endif


#define VOS_OSA_ICC_INFO_NUM 2                                /* A核和C核需要使用共享内存 */
#define VOS_OSA_ICC_INFO_SIZE sizeof(VOS_IccDebugInfo) /* 每个核需要共享内存的大小 */
#define VOS_OSA_ICC_INFO_ALL_SIZE (VOS_OSA_ICC_INFO_SIZE * VOS_OSA_ICC_INFO_NUM)

VOS_IccDebugInfo *g_iccShareBuffer = VOS_NULL_PTR;

/*
 * Description: the debug info init of Icc
 */
VOS_VOID VOS_IccDebugInfoInit(VOS_VOID)
{
    if (memset_s(&g_vosAcoreCcoreIccDebugInfo, sizeof(VOS_IccDebugInfo), 0, sizeof(VOS_IccDebugInfo)) !=
        EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    if (memset_s(&g_vosCcoreHifiIccDebugInfo, sizeof(VOS_IccDebugInfo), 0, sizeof(VOS_IccDebugInfo)) !=
        EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if (memset_s(&g_vosNRCcoreIccDebugInfo, sizeof(VOS_IccDebugInfo), 0, sizeof(VOS_IccDebugInfo)) !=
        EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    if (memset_s(&g_vosNRAcoreIccDebugInfo, sizeof(VOS_IccDebugInfo), 0, sizeof(VOS_IccDebugInfo)) !=
        EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }
#endif


    return;
}

/*
 * Description: Check if other core is in msg dump process
 */
VOS_UINT32 VOS_MsgDumpCheck(VOS_VOID)
{
    VOS_ULONG  lockLevel = 0;
    VOS_UINT32 dumpFlag;

    VOS_SpinLockIntLock(&g_vosDumpMsgSpinLock, lockLevel);

    dumpFlag = g_vosDumpMsgFlag;

    if (VOS_TRUE == g_vosDumpMsgFlag) {
        g_vosDumpMsgFlag = VOS_FALSE;
    }

    VOS_SpinUnlockIntUnlock(&g_vosDumpMsgSpinLock, lockLevel);

    return dumpFlag;
}

/*
 * Description: register awake fun to OSA
 */
VOS_UINT32 VOS_RegisterAwakeFun(ModemIdUint16 modem, MsgFunType funcHook)
{
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    g_funcVosAwakeFunHook[modem] = funcHook;

    return VOS_OK;
#else
    return VOS_ERR;
#endif
}

/*
 * Description: Call Awake fun
 */
VOS_VOID VOS_ExecuteAwakeFun(MsgBlock *msgCtrlBlk)
{
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    ModemIdUint16 modem = MODEM_ID_BUTT;

    modem = VOS_GetModemIDFromPid(msgCtrlBlk->ulReceiverPid);
    if (modem >= MODEM_ID_BUTT) {
        return;
    }

    if (VOS_NULL_PTR == g_funcVosAwakeFunHook[modem]) {
        return;
    }

    (g_funcVosAwakeFunHook[modem])(msgCtrlBlk);
#endif

    return;
}

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
/*
 * Description: DRV MB dallback func, read Msg data from MB, and then send it to receiver.
 */
VOS_VOID VOS_DRVMB_OSAMsg_CB(VOS_VOID *userPara, VOS_VOID *mailHandle, VOS_UINT32 len)
{
    VOS_UINT8   *msgData    = VOS_NULL_PTR;
    MsgBlock    *msgCtrlBlk = VOS_NULL_PTR;
    VOS_UINT32   mailboxLen  = len;
    VOS_UINT32   mailCode;
    VOS_UINT_PTR tempValue;

    if ((userPara == VOS_NULL_PTR) || (mailHandle == VOS_NULL_PTR)) {
        return;
    }

    tempValue = (VOS_UINT_PTR)userPara;

    if (len <= VOS_MSG_HEAD_LENGTH) {
        mdrv_err("<VOS_DRVMB_OSAMsg_CB> The Data Len is small.\n");

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_MB_DATALENISNULL);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_MB_DATALENISNULL, (VOS_INT)len, (VOS_INT)tempValue, VOS_NULL_PTR, 0);

        return;
    }

    /* Alloc message space with static PID number */
    msgData = (VOS_UINT8 *)VOS_AllocMsg(VOS_PID_DOPRAEND, (VOS_UINT32)(len - VOS_MSG_HEAD_LENGTH));
    if (VOS_NULL_PTR == msgData) {
        mdrv_err("<VOS_DRVMB_OSAMsg_CB> Alloc Msg memory failed.\n");

        return;
    }

    if (MAILBOX_OK != DRV_MAILBOX_READMAILDATA(mailHandle, msgData, &mailboxLen)) {
        (VOS_VOID)VOS_FreeMsg(VOS_PID_DOPRAEND, msgData);

        mdrv_err("<VOS_DRVMB_OSAMsg_CB> DRV_MAILBOX_READMAILDATA failed.\n");

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_DRV_READ_MAIL_FAIL);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_DRV_READ_MAIL_FAIL, (VOS_INT)len, (VOS_INT)tempValue, VOS_NULL_PTR, 0);
        return;
    }

    /* get the Message ctrl data from the MB channel */
    msgCtrlBlk = (MsgBlock *)(msgData);

    VOS_ModifyMsgInfo((VOS_VOID *)msgCtrlBlk, msgCtrlBlk->ulSenderPid);

    mailCode = (VOS_UINT32)tempValue;

    if (VOS_MAIL_BOX_MSG_NORMAL_BUTT < mailCode) { /* urgend msg */
        (VOS_VOID)V_SendUrgentMsg(VOS_PID_DOPRAEND, (VOS_VOID **)(&(msgData)), VOS_FILE_ID,
                                  (VOS_INT32)(VOS_MSG_HEAD_FLAG + VOS_HIFI_TO_CCPU_VOS_MSG_URGENT));
    } else {
        (VOS_VOID)V_SendMsg(VOS_PID_DOPRAEND, (VOS_VOID **)(&(msgData)), VOS_FILE_ID,
                            (VOS_INT32)(VOS_MSG_HEAD_FLAG + VOS_HIFI_TO_CCPU_VOS_MSG_NORMAL));
    }

    return;
}

/*
 * Description: Init the DRV's mailebox channle.
 */
VOS_UINT32 VOS_DRVMB_Init(VOS_VOID)
{
#if (FEATURE_ON != FEATURE_HIFI_USE_ICC)

    /* Register HIFI->CCPU OSA Msg CallBack */
    if (VOS_OK != DRV_MAILBOX_REGISTERRECVFUNC(MAILBOX_MAILCODE_HIFI_TO_CCPU_VOS_MSG_NORMAL, VOS_DRVMB_OSAMsg_CB,
                                               (VOS_VOID *)VOS_HIFI_TO_CCPU_VOS_MSG_NORMAL)) {
        mdrv_err("<VOS_DRVMB_Init> Register HIFI->CCPU Normal CB failed.\n");
        return VOS_ERR;
    }

    /* Register HIFI->CCPU OSA Msg CallBack */
    if (VOS_OK != DRV_MAILBOX_REGISTERRECVFUNC(MAILBOX_MAILCODE_HIFI_TO_CCPU_VOS_MSG_URGENT, VOS_DRVMB_OSAMsg_CB,
                                               (VOS_VOID *)VOS_HIFI_TO_CCPU_VOS_MSG_URGENT)) {
        mdrv_err("<VOS_DRVMB_Init> Register HIFI->CCPU Urgent CB failed.\n");
        return VOS_ERR;
    }
#endif

    return VOS_OK;
}

/*
 * Description: Send the msg to another ARM's OSA by the DRV's mailebox channle.
 */
VOS_UINT32 VOS_SendMsgByDrvMB(VOS_PID pid, VOS_VOID **msg, VOS_DrvmbMsgPara *msgPara)
{
    VOS_UINT32 dataLen;
    VOS_UINT32 result;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;

    msgCtrlBlk = (MsgBlock *)(*msg);

    if (DOPRA_PID_TIMER == msgCtrlBlk->ulSenderPid) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_MB_PIDTERROR);

        mdrv_err("<VOS_SendMsgByDrvMB> Timer Sender MB Msg, Rec PID=%d\n", (VOS_INT)msgCtrlBlk->ulReceiverPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_MB_PIDTERROR, (VOS_INT)msgCtrlBlk->ulReceiverPid, 0, 0, 0);

        return (VOS_ERRNO_MSG_MB_PIDTERROR);
    }

    /* Get the Msg Length */
    dataLen = msgCtrlBlk->ulLength + VOS_MSG_HEAD_LENGTH;

    result = DRV_MAILBOX_SENDMAIL(msgPara->mailBoxCode, *msg, dataLen);

    (VOS_VOID)VOS_FreeMsg(pid, *msg); /* need free the Msg memory */

    /* 由于HIFI复位，写 mailbox通道失败会返回一个特殊值，不能复位单板 */
    if (MAILBOX_TARGET_NOT_READY == result) {
        mdrv_err("<VOS_SendMsgByDrvMB> HIFI Reset Fail, File=%d. line=%d. Size=%d.\n", (VOS_INT)msgPara->fileId,
                 msgPara->lineNo, (VOS_INT)dataLen);

        return VOS_ERRNO_MSG_MAILBOX_RESET;
    }

    if (MAILBOX_OK != result) { /* Send Data to mail box channle error */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_MB_SENDERROR);

        mdrv_err("<VOS_SendMsgByDrvMB> Write DRV MB Fail, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)msgPara->fileId, msgPara->lineNo, (VOS_INT)dataLen, (VOS_INT)result);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_MB_SENDERROR, (VOS_INT)result, 0, (VOS_CHAR *)msgPara,
                             sizeof(VOS_DrvmbMsgPara));

        return (VOS_ERRNO_MSG_MB_SENDERROR);
    }

    return VOS_OK;
}

/*
 * Description: Send the msg to HIFI's OSA by the DRV's mailebox channle.
 */
VOS_UINT32 VOS_SendHIFIMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_DrvmbMsgPara msgPara;

    msgPara.lineNo       = lineNo;
    msgPara.fileId      = fileId;
    msgPara.mailBoxCode = MAILBOX_MAILCODE_CCPU_TO_HIFI_VOS_MSG_NORMAL;

    return VOS_SendMsgByDrvMB(pid, msg, &msgPara);
}

/*
 * Description: Send the urgent msg to HIFI's OSA by the DRV's mailebox channle.
 */
VOS_UINT32 VOS_SendHIFIUrgentMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_DrvmbMsgPara msgPara;

    msgPara.lineNo       = lineNo;
    msgPara.fileId      = fileId;
    msgPara.mailBoxCode = MAILBOX_MAILCODE_CCPU_TO_HIFI_VOS_MSG_URGENT;

    return VOS_SendMsgByDrvMB(pid, msg, &msgPara);
}

/*
 * Description: Send the msg to MCU's OSA by the DRV's mailebox channle.
 */
VOS_UINT32 VOS_SendMCUMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_DrvmbMsgPara msgPara;

    msgPara.lineNo  = lineNo;
    msgPara.fileId = fileId;

    msgPara.mailBoxCode = MAILBOX_MAILCODE_CCPU_TO_MCU_VOS_MSG_NORMAL;

    return VOS_SendMsgByDrvMB(pid, msg, &msgPara);
}

/*
 * Description: Send the urgent msg to MCU's OSA by the DRV's mailebox channle.
 */
VOS_UINT32 VOS_SendMCUUrgentMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_DrvmbMsgPara msgPara;

    msgPara.lineNo  = lineNo;
    msgPara.fileId = fileId;
    msgPara.mailBoxCode = MAILBOX_MAILCODE_CCPU_TO_MCU_VOS_MSG_URGENT;

    return VOS_SendMsgByDrvMB(pid, msg, &msgPara);
}
#endif

/*
 * Description: allocates messages block
 */
MsgBlock *V_AllocMsg(VOS_PID pid, VOS_UINT32 length, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32      totalLength;
    MSG_BlockHead *msgBlkHead  = VOS_NULL_PTR;
    MsgBlock       *msgBlockPtr = VOS_NULL_PTR;

#if (VOS_YES == VOS_CHECK_PARA)
    if (pid >= VOS_PID_BUTT) {
        return ((MsgBlock *)VOS_NULL_PTR);
    }

    if (0 == length) {
        return ((MsgBlock *)VOS_NULL_PTR);
    }

    /* 如果申请的空间大小超过0x7FFFFFFF个Byte，直接返回空指针 */
    if (0x7FFFFFFF < (length + VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH)) {
        mdrv_err("<V_AllocMsg> size over 0x7FFFFFFF,FileID=%d LineNo=%d.\n", fileId, lineNo);

        return VOS_NULL_PTR;
    }
#endif

    totalLength = (VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH + length);

    msgBlkHead = (MSG_BlockHead *)VOS_MemBlkMalloc(pid, totalLength, fileId, lineNo);
    if (VOS_NULL_PTR == msgBlkHead) {
        return ((MsgBlock *)VOS_NULL_PTR);
    }

    msgBlkHead->flag = VOS_MSG_HEAD_FLAG;

    msgBlockPtr                  = (MsgBlock *)((VOS_UINT_PTR)msgBlkHead + VOS_MSG_BLK_HEAD_LEN);
    msgBlockPtr->ulSenderCpuId   = VOS_LOCAL_CPUID;
    msgBlockPtr->ulSenderPid     = pid;
    msgBlockPtr->ulReceiverCpuId = VOS_LOCAL_CPUID;
    msgBlockPtr->ulLength        = (VOS_UINT32)length;

#if (VOS_DEBUG == VOS_DOPRA_VER)

    if (VOS_NULL_PTR != g_funcAllocMsgHook) {
        g_funcAllocMsgHook((VOS_UINT32)totalLength);
    }

#endif

    return msgBlockPtr;
}

/*
 * Description: allocates timer messages block
 */
MsgBlock *VOS_AllocTimerMsg(VOS_PID pid, VOS_UINT32 length)
{
    VOS_UINT32      totalLength;
    MSG_BlockHead *msgBlkHead  = VOS_NULL_PTR;
    MsgBlock       *msgBlockPtr = VOS_NULL_PTR;

    if (pid >= VOS_PID_BUTT) {
        mdrv_err("<VOS_AllocTimerMsg> Alloc Timer msg Pid too big.\n");
        return ((MsgBlock *)VOS_NULL_PTR);
    }

    if (0 == length) {
        return ((MsgBlock *)VOS_NULL_PTR);
    }

    totalLength = VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH + length;

    msgBlkHead = (MSG_BlockHead *)VOS_TIMER_MemCtrlBlkMalloc(pid, totalLength);
    if (VOS_NULL_PTR == msgBlkHead) {
        return ((MsgBlock *)VOS_NULL_PTR);
    }

    msgBlkHead->flag = VOS_MSG_HEAD_FLAG;

    msgBlockPtr = (MsgBlock *)((VOS_UINT_PTR)msgBlkHead + VOS_MSG_BLK_HEAD_LEN);

    msgBlockPtr->ulSenderCpuId   = VOS_LOCAL_CPUID;
    msgBlockPtr->ulSenderPid     = DOPRA_PID_TIMER;
    msgBlockPtr->ulReceiverCpuId = VOS_LOCAL_CPUID;
    msgBlockPtr->ulReceiverPid   = pid;
    msgBlockPtr->ulLength        = length;

    return msgBlockPtr;
}

/*
 * Description: Free a message which status must be ALLOCATED
 */
VOS_UINT32 V_FreeMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_VOID       *msgBlkHead = VOS_NULL_PTR;
    MSG_BlockHead  *tempMsg      = VOS_NULL_PTR;
    VOS_UINT_PTR    blockAdd;
    VOS_UINT_PTR    ctrlkAdd;

#if (VOS_YES == VOS_CHECK_PARA)
    if (pid >= VOS_PID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTMSGISNULL);
        return (VOS_ERRNO_MSG_FREE_INPUTMSGISNULL);
    }

    if (VOS_NULL_PTR == msg) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }

    if (VOS_NULL_PTR == *msg) {
        mdrv_err("<V_FreeMsg> free msg again.F=%d L=%d T=%d.\n", (VOS_INT)fileId, lineNo, (VOS_INT)VOS_GetTick());

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }
#endif

    msgBlkHead = (VOS_VOID *)((VOS_UINT_PTR)(*msg) - VOS_MSG_BLK_HEAD_LEN);
    if (VOS_OK != VOS_MemCheck(msgBlkHead, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        return VOS_ERR;
    }

    tempMsg = (MSG_BlockHead *)msgBlkHead;
    if (VOS_MSG_RESERVED_HEAD_FLAG == tempMsg->flag) {
        return VOS_OK;
    } else {
        /* Clear user's pointer */
        *msg = VOS_NULL_PTR;

#if (VOS_DEBUG == VOS_DOPRA_VER)

        if (VOS_NULL_PTR != g_funcFreeMsgHook) {
            g_funcFreeMsgHook(0);
        }
#endif

        return VOS_MemCtrlBlkFree((VOS_MemCtrlBlock *)ctrlkAdd, (VOS_MemHeadBlock *)blockAdd, fileId, pid);
    }
}

/*
 * Description: In msg proceed function, reserve a message
 */
VOS_UINT32 V_ReserveMsg(VOS_PID pid, MsgBlock *msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    MSG_BlockHead *msgBlockPtr = VOS_NULL_PTR;
    VOS_UINT_PTR    blockAdd;
    VOS_UINT_PTR    ctrlkAdd;

    if (pid >= VOS_PID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_RESERVE_INVALIDMSG);
        return (VOS_ERRNO_MSG_RESERVE_INVALIDMSG);
    }

    if (msg == VOS_NULL_PTR) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }

    msgBlockPtr = (MSG_BlockHead *)((VOS_UINT_PTR)msg - VOS_MSG_BLK_HEAD_LEN);

    if (VOS_OK != VOS_MemCheck((VOS_VOID *)msgBlockPtr, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        return VOS_ERR;
    }

    if (VOS_MSG_HEAD_FLAG == msgBlockPtr->flag) {
        msgBlockPtr->flag = VOS_MSG_RESERVED_HEAD_FLAG;
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}

/*
 * Description: In msg proceed function, unreserve a message
 */
VOS_UINT32 V_UnreserveMsg(VOS_PID pid, MsgBlock *msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    MSG_BlockHead *msgBlockPtr = VOS_NULL_PTR;
    VOS_UINT_PTR    blockAdd;
    VOS_UINT_PTR    ctrlkAdd;

    if (pid >= VOS_PID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_RESERVE_INVALIDMSG);
        return (VOS_ERRNO_MSG_RESERVE_INVALIDMSG);
    }

    if (msg == VOS_NULL_PTR) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }

    msgBlockPtr = (MSG_BlockHead *)((VOS_UINT_PTR)msg - VOS_MSG_BLK_HEAD_LEN);

    if (VOS_OK != VOS_MemCheck((VOS_VOID *)msgBlockPtr, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        return VOS_ERR;
    }

    msgBlockPtr->flag = VOS_MSG_HEAD_FLAG;

    return VOS_OK;
}

/*
 * 功能描述: 检查发送的消息是否为核间消息
 */
VOS_BOOL VOS_CheckMsgCPUId(VOS_UINT32 cpuId)
{
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    /* CCPU的id为0，如果消息接收的cpuid不为ccpu id，则为跨核消息 */
    if (OSA_CPU_CCPU != cpuId) {
        return VOS_TRUE;
    }
#elif (OSA_CPU_ACPU == VOS_OSA_CPU)
    /* ACPU的id为1，如果消息接收的cpuid不为acpu id，则为跨核消息 */
    if (OSA_CPU_ACPU != cpuId) {
        return VOS_TRUE;
    }
#endif
    return VOS_FALSE;
}

/*
 * 功能描述: 检测是否为中断上下文
 */
VOS_UINT32 VOS_CheckInterrupt(VOS_VOID)
{
#if (VOS_OS_VER == VOS_RTOSCK)
    return mdrv_int_is_inside_context();
#elif (VOS_OS_VER == VOS_LINUX)
    return (VOS_UINT32)in_interrupt();
#else
    return VOS_FALSE;
#endif
}

/*
 * Description: Free a reserved message.
 */
VOS_UINT32 V_FreeReservedMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_VOID       *msgBlkHead = VOS_NULL_PTR;
    MSG_BlockHead   *tempMsg      = VOS_NULL_PTR;
    VOS_UINT_PTR    blockAdd;
    VOS_UINT_PTR    ctrlkAdd;

    if (pid >= VOS_PID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTMSGISNULL);
        return (VOS_ERRNO_MSG_FREE_INPUTMSGISNULL);
    }

    if (VOS_NULL_PTR == msg) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }

    if (VOS_NULL_PTR == *msg) {
        mdrv_err("<V_FreeReservedMsg> free reserved msg again.F=%d L=%d T=%d.\n", (VOS_INT)fileId, lineNo,
                 (VOS_INT)VOS_GetTick());

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }

    msgBlkHead = (VOS_VOID *)((VOS_UINT_PTR)(*msg) - VOS_MSG_BLK_HEAD_LEN);
    if (VOS_OK != VOS_MemCheck(msgBlkHead, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        return VOS_ERR;
    }

    tempMsg = (MSG_BlockHead *)msgBlkHead;
    if (VOS_MSG_RESERVED_HEAD_FLAG == tempMsg->flag) {
        /* Clear user's pointer */
        *msg = VOS_NULL_PTR;

#if (VOS_DEBUG == VOS_DOPRA_VER)

        if (VOS_NULL_PTR != g_funcFreeMsgHook) {
            g_funcFreeMsgHook(0);
        }

#endif

        return VOS_MemCtrlBlkFree((VOS_MemCtrlBlock *)ctrlkAdd, (VOS_MemHeadBlock *)blockAdd, fileId, pid);
    } else {
        return VOS_ERRNO_MSG_FREERES_STATUSERROR;
    }
}

/*
 * Description: Check if the receiver pid is correct.
 */
VOS_UINT32 VOS_CheckRcvPid(VOS_UINT32 receiverPid)
{
    VOS_UINT32 cpuID;

    cpuID = VOS_GET_CPU_ID(receiverPid);
    if (cpuID >= VOS_SUPPORT_CPU_NUM_MAX) { /* The CPU ID is too large */
        return (VOS_ERRNO_MSG_ICC_CPUIDISBIG);
    }

    if (receiverPid >= g_vosSendMsgProcTable[cpuID].endPID) { /* The Receive PID is too large */
        return (VOS_ERRNO_MSG_POST_INPUTPIDINVALID);
    }

    return VOS_OK;
}

/*
 * Description: Check the msg parameter.
 */
VOS_UINT32 V_CheckMsgPara(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 rcvPid;
    VOS_UINT32 cpuID;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;

#if (VOS_YES == VOS_CHECK_PARA)
    if (VOS_NULL_PTR == msg) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);

        VOS_ProtectionReboot(OSA_SEND_MSG_NULL, (VOS_INT)fileId, lineNo, VOS_NULL_PTR, 0);

        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }

    if (VOS_NULL_PTR == *msg) {
        mdrv_err("<V_SendMsg> send msg again.F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);

        VOS_ProtectionReboot(OSA_SEND_MSG_PP_NULL, (VOS_INT)fileId, lineNo, VOS_NULL_PTR, 0);

        return (VOS_ERRNO_MSG_FREE_INPUTPIDINVALID);
    }
#endif

    msgCtrlBlk = (MsgBlock *)(*msg);
    rcvPid       = msgCtrlBlk->ulReceiverPid;
    cpuID     = VOS_GET_CPU_ID(rcvPid);
    if (cpuID >= VOS_SUPPORT_CPU_NUM_MAX) { /* The CPU ID is too large */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_CPUIDISBIG);

        mdrv_err("<V_SendMsg> invalid CPU id, PID=%d.\n", (VOS_INT)rcvPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_CPUIDISBIG, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return (VOS_ERRNO_MSG_ICC_CPUIDISBIG);
    }

    if (rcvPid < VOS_PID_CPU_ID_0_DOPRAEND) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTOOSMALL);

        mdrv_err("<V_SendMsg> pid too small, PID=%d.\n", (VOS_INT)rcvPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTOOSMALL, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return (VOS_ERRNO_MSG_ICC_PIDTOOSMALL);
    }

    if (rcvPid >= g_vosSendMsgProcTable[cpuID].endPID) { /* The Receive PID is too large */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_POST_INPUTPIDINVALID);

        mdrv_err("<V_SendMsg> invalid PID=%d.\n", (VOS_INT)rcvPid);

        VOS_ProtectionReboot(OSA_SEND_MSG_PID_BIG, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return (VOS_ERRNO_MSG_POST_INPUTPIDINVALID);
    }

    return VOS_OK;
}

/*
 * Description: Send the msg to a task, which is in this CPU..
 */
VOS_UINT32 V_SendLocalMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)

{
    MsgBlock          *msgCtrlBlk = VOS_NULL_PTR;
    VOS_UINT32         rcvPid;
    VOS_UINT32         fid;
    VOS_UINT32         qid;
    VOS_VOID          *actualMsg = VOS_NULL_PTR;
    VOS_UINT_PTR       blockAdd;
    VOS_UINT_PTR       ctrlkAdd;
    VOS_DumpMsg *dumpMsgInfo = VOS_NULL_PTR;
    VOS_UINT32         realTaskId;

    msgCtrlBlk = (MsgBlock *)(*msg);
    rcvPid       = msgCtrlBlk->ulReceiverPid;

    if (rcvPid < VOS_PID_DOPRAEND) {
        mdrv_err("<VOS_SendLocalMsg> Rec PID=%d Check error.\n", (VOS_INT)rcvPid);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTOOSMALL);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTOOSMALL, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }

    if (rcvPid >= VOS_PID_BUTT) { /* The Receive PID is too large */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_POST_INPUTPIDINVALID);

        mdrv_err("<VOS_SendLocalMsg> invalid PID=%d.\n", (VOS_INT)rcvPid);

        VOS_ProtectionReboot(OSA_SEND_MSG_PID_BIG, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }

    fid = (VOS_UINT32)(g_vosPidRecords[rcvPid - VOS_PID_DOPRAEND].fid);
    if ((fid < (VOS_UINT32)VOS_FID_DOPRAEND) || (fid >= (VOS_UINT32)VOS_FID_BUTT)) {
        mdrv_err("<VOS_SendLocalMsg>Send PID=%d, Rec PID=%d.\n", msgCtrlBlk->ulSenderPid, (VOS_INT)rcvPid);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_PID_NO_FID);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }

    qid = g_vosFidCtrlBlk[fid].qid;

    actualMsg = (VOS_VOID *)((VOS_UINT_PTR)(*msg) - VOS_MSG_BLK_HEAD_LEN);
    if (VOS_OK != VOS_MemCheck(actualMsg, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        mdrv_err("<VOS_SendLocalMsg> Mem Check error.\n");

        VOS_ProtectionReboot(OSA_SEND_MSG_FAIL_TO_CHECK, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        return VOS_ERR;
    }

    if (VOS_OK != VOS_FixedQueueWrite(qid, actualMsg, (VOS_UINT32)MSG_SEND_SIZE, VOS_NORMAL_PRIORITY_MSG, rcvPid)) {
        mdrv_err("<V_SendLocalMsg> queue full,Tx Pid=%d Rx Pid=%d Name 0x%x.\n", (int)(msgCtrlBlk->ulSenderPid),
                 (int)rcvPid, (int)(*(VOS_UINT32 *)(msgCtrlBlk->value)));

        if (VOS_FALSE == VOS_MsgDumpCheck()) {
            mdrv_debug("<V_SendLocalMsg> not need dump\n");

            (VOS_VOID)VOS_FreeMsg(pid, *msg);

            return VOS_ERR;
        }

        dumpMsgInfo = (VOS_DumpMsg *)g_dumpMsgInfo;

        (VOS_VOID)VOS_TaskLock();

        dumpMsgInfo->qid     = qid;
        dumpMsgInfo->sendPid = msgCtrlBlk->ulSenderPid;
        dumpMsgInfo->rcvPid  = rcvPid;
        dumpMsgInfo->msgName = *(VOS_UINT32 *)(msgCtrlBlk->value);

        dumpMsgInfo++;

        VOS_QueuePrintFull(qid, (VOS_CHAR *)dumpMsgInfo, DUMP_MSG_INFO_LEN - sizeof(VOS_DumpMsg));

        realTaskId = VOS_GetRealTID(g_vosFidCtrlBlk[fid].tid);

        VOS_ProtectionReboot(VOS_SAVE_STACK(OSA_SEND_MSG_FAIL_TO_WRITE), (VOS_INT)realTaskId, lineNo, g_dumpMsgInfo,
                             DUMP_MSG_INFO_LEN);

        (VOS_VOID)VOS_TaskUnlock();

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }

    if (VOS_MSG_RESERVED_HEAD_FLAG != ((MSG_BlockHead *)actualMsg)->flag) {
        *msg = VOS_NULL_PTR;
    }

    return VOS_OK;
}

/*
 * Description: modify the information. alloc msg. who?
 */
VOS_VOID VOS_ModifyMsgInfo(VOS_VOID *msg, VOS_PID pid)
{
    VOS_VOID    *actualMsg = VOS_NULL_PTR;
    VOS_UINT_PTR blockAdd;
    VOS_UINT_PTR ctrlkAdd;

    actualMsg = (VOS_VOID *)((VOS_UINT_PTR)(msg) - VOS_MSG_BLK_HEAD_LEN);
    if (VOS_OK != VOS_MemCheck(actualMsg, &blockAdd, &ctrlkAdd, 0, 0)) {
        return;
    }

    VOS_ModifyMemBlkInfo(blockAdd, pid);

    return;
}

/*
 * Description: Send the msg to another ARM's OSA by the ICC channle.
 */
VOS_UINT32 V_SendMsgByICC(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 dataLen;
    VOS_INT    result;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;

    msgCtrlBlk = (MsgBlock *)(*msg);

    if (DOPRA_PID_TIMER == msgCtrlBlk->ulSenderPid) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTERROR);

        mdrv_err("<V_SendMsgByICC> Timer Sender Icc Msg, Rec PID=%d\n", (VOS_INT)msgCtrlBlk->ulReceiverPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTERROR, (VOS_INT)msgCtrlBlk->ulReceiverPid, lineNo, 0, 0);

        return (VOS_ERRNO_MSG_ICC_PIDTERROR);
    }

    dataLen = msgCtrlBlk->ulLength + VOS_MSG_HEAD_LENGTH; /* Get the Msg Length */

    result = DRV_ICC_WRITE(UDI_ICC_GUOM4, *msg, (VOS_INT32)dataLen);

    (VOS_VOID)VOS_FreeMsg(pid, *msg); /* need free the Msg memory */

    /* 由于C核复位，写ICC通道失败会返回一个特殊值，不能复位单板 */
    if (BSP_ERR_ICC_CCORE_RESETTING == result) {
        mdrv_err("<V_SendMsgByICC> Ccore Reset, File=%d. line=%d. Size=%d .\n", (VOS_INT)fileId, lineNo,
                 (VOS_INT)dataLen);

        return VOS_ERRNO_MSG_CCORE_RESET;
    }

    /* 写ICC邮箱满，OSA发起主动复位 */
    if (ICC_INVALID_NO_FIFO_SPACE == result) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGFULL);

        mdrv_err("<V_SendMsgByICC Error> Write ICC Channel Full, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_WRITEMSGFULL, (VOS_INT)fileId, lineNo, (VOS_CHAR *)&result,
                             sizeof(result));

        return (VOS_ERRNO_MSG_ICC_WRITEMSGFULL);
    }

    if (dataLen != (VOS_UINT32)result) { /* Write Data to ICC channle Success */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGERROR);

        mdrv_err("<V_SendMsgByICC Error> Write ICC Channel Error, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_WRITEMSGERROR, (VOS_INT)fileId, lineNo, (VOS_CHAR *)&result,
                             sizeof(result)); /* Save the UDI result */

        return (VOS_ERRNO_MSG_ICC_WRITEMSGERROR);
    }

    g_vosAcoreCcoreIccDebugInfo.sendNum++;
    g_vosAcoreCcoreIccDebugInfo.sendSlice = VOS_GetSlice();
    g_vosAcoreCcoreIccDebugInfo.sendLen += dataLen;

    if (g_iccShareBuffer != VOS_NULL_PTR) {
        g_iccShareBuffer->sendNum   = g_vosAcoreCcoreIccDebugInfo.sendNum;
        g_iccShareBuffer->sendSlice = g_vosAcoreCcoreIccDebugInfo.sendSlice;
        g_iccShareBuffer->sendLen   = g_vosAcoreCcoreIccDebugInfo.sendLen;
    }

    return VOS_OK;
}


#if (OSA_CPU_CCPU == VOS_OSA_CPU)

/*
 * Description: Send the msg to another ARM's OSA by the ICC channle.
 */
VOS_UINT32 V_NRCcpuSendMsgByICC(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 dataLen;
    VOS_INT    result;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;

    msgCtrlBlk = (MsgBlock *)(*msg);

    if (DOPRA_PID_TIMER == msgCtrlBlk->ulSenderPid) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTERROR);

        mdrv_err("<V_NRCcpuSendMsgByICC> Error Timer Sender Icc Msg, Rec PID=%d\n",
                 (VOS_INT)msgCtrlBlk->ulReceiverPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTERROR, (VOS_INT)msgCtrlBlk->ulReceiverPid, lineNo, 0, 0);

        return (VOS_ERRNO_MSG_ICC_PIDTERROR);
    }

    dataLen = msgCtrlBlk->ulLength + VOS_MSG_HEAD_LENGTH; /* Get the Msg Length */

    result = DRV_ICC_WRITE(UDI_ICC_NRCCPU_LRCCPU_OSA, *msg, (VOS_INT32)dataLen);

    (VOS_VOID)VOS_FreeMsg(pid, *msg); /* need free the Msg memory */

    /* 由于C核复位，写ICC通道失败会返回一个特殊值，不能复位单板 */
    if (BSP_ERR_ICC_CCORE_RESETTING == result) {
        mdrv_err("<V_NRCcpuSendMsgByICC> Error,Ccore Reset, File=%d. line=%d. Size=%d .\n", (VOS_INT)fileId, lineNo,
                 (VOS_INT)dataLen);

        return VOS_ERRNO_MSG_CCORE_RESET;
    }

    /* 写ICC邮箱满，OSA发起主动复位 */
    if (ICC_INVALID_NO_FIFO_SPACE == result) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGFULL);

        mdrv_err("<V_NRCcpuSendMsgByICC> Error,Write ICC Channel Full, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_WRITEMSGFULL, (VOS_INT)fileId, lineNo, (VOS_CHAR *)&result,
                             sizeof(result));

        return (VOS_ERRNO_MSG_ICC_WRITEMSGFULL);
    }

    if (dataLen != (VOS_UINT32)result) { /* Write Data to ICC channle Success */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGERROR);

        mdrv_err("<V_NRCcpuSendMsgByICC> Error,Write ICC Channel Error, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_WRITEMSGERROR, (VOS_INT)fileId, lineNo, (VOS_CHAR *)&result,
                             sizeof(result)); /* Save the UDI result */

        return (VOS_ERRNO_MSG_ICC_WRITEMSGERROR);
    }

    g_vosNRCcoreIccDebugInfo.sendNum++;
    g_vosNRCcoreIccDebugInfo.sendSlice = VOS_GetSlice();
    g_vosNRCcoreIccDebugInfo.sendLen += dataLen;

    return VOS_OK;
}
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)

/*
 * Description: Send the msg to another ARM's OSA by the ICC channle.
 */
VOS_UINT32 V_NRAcpuSendMsgByICC(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 dataLen;
    VOS_INT    result;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;

    msgCtrlBlk = (MsgBlock *)(*msg);

    if (DOPRA_PID_TIMER == msgCtrlBlk->ulSenderPid) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTERROR);

        mdrv_err("<V_NRAcpuSendMsgByICC> Error Timer Sender Icc Msg, Rec PID=%d\n",
                 (VOS_INT)msgCtrlBlk->ulReceiverPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTERROR, (VOS_INT)msgCtrlBlk->ulReceiverPid, lineNo, 0, 0);

        return (VOS_ERRNO_MSG_ICC_PIDTERROR);
    }

    dataLen = msgCtrlBlk->ulLength + VOS_MSG_HEAD_LENGTH; /* Get the Msg Length */

    result = DRV_ICC_WRITE(UDI_ICC_NRCCPU_APCPU_OSA, *msg, (VOS_INT32)dataLen);

    (VOS_VOID)VOS_FreeMsg(pid, *msg); /* need free the Msg memory */

    /* 由于C核复位，写ICC通道失败会返回一个特殊值，不能复位单板 */
    if (BSP_ERR_ICC_CCORE_RESETTING == result) {
        mdrv_err("<V_NRAcpuSendMsgByICC> Error,Ccore Reset, File=%d. line=%d. Size=%d .\n", (VOS_INT)fileId, lineNo,
                 (VOS_INT)dataLen);

        return VOS_ERRNO_MSG_CCORE_RESET;
    }

    /* 写ICC邮箱满，OSA发起主动复位 */
    if (ICC_INVALID_NO_FIFO_SPACE == result) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGFULL);

        mdrv_err("<V_NRAcpuSendMsgByICC> Error,Write ICC Channel Full, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_WRITEMSGFULL, (VOS_INT)fileId, lineNo, (VOS_CHAR *)&result,
                             sizeof(result));

        return (VOS_ERRNO_MSG_ICC_WRITEMSGFULL);
    }

    if (dataLen != (VOS_UINT32)result) { /* Write Data to ICC channle Success */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGERROR);

        mdrv_err("<V_NRAcpuSendMsgByICC> Error,Write ICC Channel Error, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_WRITEMSGERROR, (VOS_INT)fileId, lineNo, (VOS_CHAR *)&result,
                             sizeof(result)); /* Save the UDI result */

        return (VOS_ERRNO_MSG_ICC_WRITEMSGERROR);
    }

    g_vosNRAcoreIccDebugInfo.sendNum++;
    g_vosNRAcoreIccDebugInfo.sendSlice = VOS_GetSlice();
    g_vosNRAcoreIccDebugInfo.sendLen += dataLen;

    return VOS_OK;
}
#endif


#if (FEATURE_ON == FEATURE_HIFI_USE_ICC)

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

/*
 * Description: Send the msg to another ARM's OSA by the ICC channle.
 */
VOS_UINT32 V_SendHifiMsgByICC(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 dataLen;
    VOS_INT    result;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;

    msgCtrlBlk = (MsgBlock *)(*msg);

    if (DOPRA_PID_TIMER == msgCtrlBlk->ulSenderPid) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTERROR);

        mdrv_err("<V_SendHifiMsgByICC> Error Timer Sender Icc Msg, Rec PID=%d\n", (VOS_INT)msgCtrlBlk->ulReceiverPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTERROR, (VOS_INT)msgCtrlBlk->ulReceiverPid, lineNo, 0, 0);

        return (VOS_ERRNO_MSG_ICC_PIDTERROR);
    }

    dataLen = msgCtrlBlk->ulLength + VOS_MSG_HEAD_LENGTH; /* Get the Msg Length */

    result = DRV_ICC_WRITE(UDI_ICC_CCPU_HIFI_VOS_NORMAL_MSG, *msg, (VOS_INT32)dataLen);

    (VOS_VOID)VOS_FreeMsg(pid, *msg); /* need free the Msg memory */

    /* 写ICC邮箱满，OSA发起主动复位 */
    if (ICC_INVALID_NO_FIFO_SPACE == result) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEHIFIMSGFULL);

        mdrv_err("<V_SendHifiMsgByICC> Error,Write ICC Channel Full, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        return (VOS_ERRNO_MSG_ICC_WRITEHIFIMSGFULL);
    }

    if (dataLen != (VOS_UINT32)result) { /* Write Data to ICC channle Success */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGERROR);

        mdrv_err("<V_SendHifiMsgByICC> Error,Write ICC Channel Error, File=%d. line=%d. Sizer%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        return (VOS_ERRNO_MSG_ICC_WRITEMSGERROR);
    }

    g_vosCcoreHifiIccDebugInfo.sendNum++;
    g_vosCcoreHifiIccDebugInfo.sendSlice = VOS_GetSlice();
    g_vosCcoreHifiIccDebugInfo.sendLen += dataLen;

    return VOS_OK;
}

/*
 * Description: Send the msg to another ARM's OSA by the ICC channle.
 */
VOS_UINT32 V_SendHifiMsgByICCNormally(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 dataLen;
    VOS_INT    result;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;

    msgCtrlBlk = (MsgBlock *)(*msg);

    if (DOPRA_PID_TIMER == msgCtrlBlk->ulSenderPid) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTERROR);

        mdrv_err("<V_SendHifiMsgByICCNormally> Error Timer Sender Icc Msg, Rec PID=%d\n",
                 (VOS_INT)msgCtrlBlk->ulReceiverPid);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTERROR, (VOS_INT)msgCtrlBlk->ulReceiverPid, lineNo, 0, 0);

        return (VOS_ERRNO_MSG_ICC_PIDTERROR);
    }

    dataLen = msgCtrlBlk->ulLength + VOS_MSG_HEAD_LENGTH; /* Get the Msg Length */

    result = DRV_ICC_WRITE(UDI_ICC_CCPU_HIFI_VOS_NORMAL_MSG, *msg, (VOS_INT32)dataLen);

    (VOS_VOID)VOS_FreeMsg(pid, *msg); /* need free the Msg memory */

    /* 写ICC邮箱满，OSA发起主动复位 */
    if (ICC_INVALID_NO_FIFO_SPACE == result) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEHIFIMSGFULL);

        mdrv_err("<V_SendHifiMsgByICCNormally> Error,Write ICC Channel Full, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        return (VOS_ERRNO_MSG_ICC_WRITEHIFIMSGFULL);
    }

    g_vosCcoreHifiIccDebugInfo.sendNum++;
    g_vosCcoreHifiIccDebugInfo.sendSlice = VOS_GetSlice();
    g_vosCcoreHifiIccDebugInfo.sendLen += dataLen;

    if (dataLen != (VOS_UINT32)result) { /* Write Data to ICC channle Success */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_WRITEMSGERROR);

        mdrv_err("<V_SendHifiMsgByICCNormally> Error,Write ICC Channel Error, File=%d. line=%d. Size=%d result=%d.\n",
                 (VOS_INT)fileId, lineNo, (VOS_INT)dataLen, result);

        return (VOS_ERRNO_MSG_ICC_WRITEMSGERROR);
    }

    return VOS_OK;
}

/*
 * Description: Receive the msg from the ICC channel, and send it to the Receive PID.
 */
VOS_UINT V_ICC_OSAHifiMsg_CB(VOS_UINT channelId, VOS_INT len)
{
    VOS_INT32  result;
    VOS_UINT8 *msgData  = VOS_NULL_PTR;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;
    MsgBlock   tempDebug;

    g_vosCcoreHifiIccDebugInfo.rcvNum++;
    g_vosCcoreHifiIccDebugInfo.rcvSlice = VOS_GetSlice();
    g_vosCcoreHifiIccDebugInfo.rcvLen += (VOS_UINT32)len;

    if (len <= VOS_MSG_HEAD_LENGTH) {
        mdrv_err("<V_ICC_OSAHifiMsg_CB> The Data Len is small.\n");

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_DATALENISNULL);

        result = DRV_ICC_READ(UDI_ICC_CCPU_HIFI_VOS_NORMAL_MSG, (VOS_UINT8 *)&tempDebug, len);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_DATALENISNULL, (VOS_INT)len, (VOS_INT)result, (VOS_CHAR *)&tempDebug,
                             sizeof(MsgBlock));

        return VOS_ERR;
    }

    /* Alloc message space with static PID number */
    msgData = (VOS_UINT8 *)VOS_AllocMsg(VOS_PID_DOPRAEND, (VOS_UINT32)(len - VOS_MSG_HEAD_LENGTH));
    if (VOS_NULL_PTR == msgData) {
        mdrv_err("<V_ICC_OSAHifiMsg_CB> Alloc Msg memory failed.\n");

        return VOS_ERR;
    }

    /* get the Message data from the ICC channel */
    result = DRV_ICC_READ(UDI_ICC_CCPU_HIFI_VOS_NORMAL_MSG, msgData, len);
    if (len != result) {
        /* Record Debug info */
        if (memcpy_s((VOS_CHAR *)&tempDebug, sizeof(MsgBlock), (VOS_CHAR *)msgData, sizeof(MsgBlock)) != EOK) {
            mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
        }

        (VOS_VOID)VOS_FreeMsg(VOS_PID_DOPRAEND, msgData);

        mdrv_err("<V_ICC_OSAHifiMsg_CB> DRV_ICC_READ is Failed.\n");

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_READDATAFAIL);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_READDATAFAIL, (VOS_INT)len, (VOS_INT)result, (VOS_CHAR *)&tempDebug,
                             sizeof(MsgBlock));

        return VOS_ERRNO_MSG_ICC_READDATAFAIL;
    }

    msgCtrlBlk = (MsgBlock *)(msgData);

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    if (VOS_TRUE == g_msglpm) {
        g_msglpm = VOS_FALSE;

        mdrv_err("<V_ICC_OSAHifiMsg_CB> v_msg senderpid=%d, receivepid=%d, msgid=0x%x.\n", msgCtrlBlk->ulSenderPid,
                 msgCtrlBlk->ulReceiverPid, *((VOS_UINT32 *)(msgCtrlBlk->value)));
    }
#endif

    VOS_ModifyMsgInfo((VOS_VOID *)msgCtrlBlk, msgCtrlBlk->ulSenderPid);

    return V_SendMsg(VOS_PID_DOPRAEND, (VOS_VOID **)(&(msgData)), VOS_FILE_ID, __LINE__);
}

/*
 * Description: the init of the ICC channel of Hifi
 */
VOS_UINT32 VOS_Hifi_ICC_Init(VOS_VOID)
{
    VOS_IccUdiCtrl iCCCtrlTable = {0};

    iCCCtrlTable.iccId                  = UDI_ICC_CCPU_HIFI_VOS_NORMAL_MSG;
    iCCCtrlTable.iccAttr.read_cb        = V_ICC_OSAHifiMsg_CB;
    iCCCtrlTable.iccAttr.u32Priority    = VOS_ICC_CHANNEL_PRIORITY; /* 统一使用最高优先级 */
    iCCCtrlTable.iccAttr.u32TimeOut     = VOS_ICC_HANDSHAKE_TIME_MAX;
    iCCCtrlTable.iccAttr.u32FIFOInSize  = OSA_ICC_BUFFER_SIZE;
    iCCCtrlTable.iccAttr.u32FIFOOutSize = OSA_ICC_BUFFER_SIZE;
    iCCCtrlTable.iccAttr.enChanMode     = ICC_CHAN_MODE_PACKET;
    iCCCtrlTable.iccAttr.event_cb       = VOS_NULL_PTR;
    iCCCtrlTable.iccAttr.write_cb       = VOS_NULL_PTR;

    if (VOS_ERROR == DRV_ICC_OPEN(iCCCtrlTable.iccId, &iCCCtrlTable.iccAttr)) {
        /* 打开失败时记录当前ICC通道信息 */
        VOS_ProtectionReboot(OM_APP_ICC_INIT_ERROR, THIS_FILE_ID, __LINE__, VOS_NULL_PTR, 0);
        return VOS_ERR;
    }

    return VOS_OK;
}

#endif

#endif

/*
 * Description: Send the msg to a task, the task may be not in the same CPU.
 */
VOS_UINT32 V_SendMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 result;
    VOS_UINT32 rcvPid;
    VOS_UINT32 cpuID;
    MsgBlock   *msgCtrlBlk = VOS_NULL_PTR;
    VOS_UINT32 spanMsg;

    result = V_CheckMsgPara(pid, msg, fileId, lineNo);
    if (VOS_OK != result) {
        return result;
    }

    msgCtrlBlk = (MsgBlock *)(*msg);
    rcvPid       = msgCtrlBlk->ulReceiverPid;
    cpuID     = VOS_GET_CPU_ID(rcvPid);
    if (VOS_NULL_PTR == g_vosSendMsgProcTable[cpuID].sendMsg) {
        mdrv_err("<V_SendMsg> send api is null.F=%d L=%d SendPid=%d RecvPid=%d.\n", (VOS_INT)fileId, lineNo,
                 msgCtrlBlk->ulSenderPid, (VOS_INT)rcvPid);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_SEND_FUNCEMPTY);

        return (VOS_ERRNO_MSG_SEND_FUNCEMPTY);
    }

    spanMsg = VOS_CheckMsgCPUId(cpuID);
    /* 中断中发送跨核消息，返回错误 */
    if ((VOS_TRUE == spanMsg) && (VOS_FALSE != VOS_CheckInterrupt())) {
        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return (VOS_ERRNO_MSG_INT_MSGERROR);
    }

    /* 跨核消息 勾子函数不为空 */
    if ((VOS_NULL_PTR != vos_MsgHook) && (VOS_TRUE == spanMsg)) {
        (VOS_VOID)(vos_MsgHook)(*msg);
    }

    return g_vosSendMsgProcTable[cpuID].sendMsg(pid, msg, fileId, lineNo);
}

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

/*
 * Description: Send the msg to a task, the task may be not in the same CPU.
 * Not rest when a error occures.
 */
VOS_UINT32 V_SendMsgNormally(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 result;
    VOS_UINT32 rcvPid;
    VOS_UINT32 cpuID;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;
    VOS_UINT32 spanMsg;

    result = V_CheckMsgPara(pid, msg, fileId, lineNo);
    if (VOS_OK != result) {
        return result;
    }

    msgCtrlBlk = (MsgBlock *)(*msg);
    rcvPid       = msgCtrlBlk->ulReceiverPid;
    cpuID     = VOS_GET_CPU_ID(rcvPid);
    if (VOS_CPU_ID_MEDDSP != cpuID) {
        mdrv_err("<V_SendMsgNormally> send api is null.F=%d L=%d SendPid=%d RecvPid=%d.\n", (VOS_INT)fileId, lineNo,
                 msgCtrlBlk->ulSenderPid, (VOS_INT)rcvPid);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_SEND_FUNCEMPTY);

        return (VOS_ERRNO_MSG_SEND_FUNCEMPTY);
    }

    spanMsg = VOS_CheckMsgCPUId(cpuID);
    /* 中断中发送跨核消息，返回错误 */
    if ((VOS_TRUE == spanMsg) && (VOS_FALSE != VOS_CheckInterrupt())) {
        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return (VOS_ERRNO_MSG_INT_MSGERROR);
    }

    /* 跨核消息 勾子函数不为空 */
    if ((VOS_NULL_PTR != vos_MsgHook) && (VOS_TRUE == spanMsg)) {
        (VOS_VOID)(vos_MsgHook)(*msg);
    }

#if (FEATURE_ON == FEATURE_HIFI_USE_ICC)
    return V_SendHifiMsgByICCNormally(pid, msg, fileId, lineNo);
#else
    (VOS_VOID)VOS_FreeMsg(pid, *msg);

    return VOS_ERRNO_MSG_SEND_FUNCEMPTY;
#endif
}
#endif

/*
 * Description: Receive the msg from the ICC channel, and send it to the Receive PID.
 */
VOS_UINT V_ICC_OSAMsg_CB(VOS_UINT channelId, VOS_INT len)
{
    VOS_INT32  result;
    VOS_UINT8 *msgData  = VOS_NULL_PTR;
    MsgBlock *msgCtrlBlk = VOS_NULL_PTR;
    MsgBlock   tempDebug;

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    VOS_AMsgLog osaMsgLog;
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    VOS_UINT32 iccDumpIndex;
#endif

    if (len <= VOS_MSG_HEAD_LENGTH) {
        mdrv_err("<V_ICC_OSAMsg_CB> The Data Len is small.\n");

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_DATALENISNULL);

        result = DRV_ICC_READ(channelId, (VOS_UINT8 *)&tempDebug, len);

        /* ulSenderCpuId is reserved. import debugging info by the reserved domain */
        tempDebug.ulSenderCpuId = (VOS_UINT32)channelId;

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_DATALENISNULL, (VOS_INT)len, (VOS_INT)result, (VOS_CHAR *)&tempDebug,
                             sizeof(MsgBlock));

        return VOS_ERR;
    }

    /* Alloc message space with static PID number */
    msgData = (VOS_UINT8 *)VOS_AllocMsg(VOS_PID_DOPRAEND, (VOS_UINT32)(len - VOS_MSG_HEAD_LENGTH));
    if (VOS_NULL_PTR == msgData) {
        mdrv_err("<V_ICC_OSAMsg_CB> Alloc Msg memory failed.\n");

        return VOS_ERR;
    }

    /* get the Message data from the ICC channel */
    result = DRV_ICC_READ(channelId, msgData, len);
    if (len != result) {
        /* Record Debug info */
        if (memcpy_s((VOS_CHAR *)&tempDebug, sizeof(MsgBlock), (VOS_CHAR *)msgData, sizeof(MsgBlock)) != EOK) {
            /* ulSenderCpuId is reserved. import debugging info by the reserved domain */
            tempDebug.ulSenderCpuId = (VOS_UINT32)channelId;

            mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
        }

        (VOS_VOID)VOS_FreeMsg(VOS_PID_DOPRAEND, msgData);

        mdrv_err("<V_ICC_OSAMsg_CB> DRV_ICC_READ is Failed.\n");

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_READDATAFAIL);

        /* ulSenderCpuId is reserved. import debugging info by the reserved domain */
        tempDebug.ulSenderCpuId = (VOS_UINT32)channelId;

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_READDATAFAIL, (VOS_INT)len, (VOS_INT)result, (VOS_CHAR *)&tempDebug,
                             sizeof(MsgBlock));

        return VOS_ERRNO_MSG_ICC_READDATAFAIL;
    }

    msgCtrlBlk = (MsgBlock *)(msgData);

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    if (VOS_TRUE == g_msglpm) {
        g_msglpm = VOS_FALSE;

        if (VOS_CPU_ID_0_PID_BUTT > msgCtrlBlk->ulSenderPid) {
            osaMsgLog.type        = PM_LOG_AOSA_PAM_MSG;
            osaMsgLog.senderPid   = msgCtrlBlk->ulSenderPid;
            osaMsgLog.receiverPid = msgCtrlBlk->ulReceiverPid;
            osaMsgLog.msgId       = *((VOS_UINT32 *)(msgCtrlBlk->value));

            (VOS_VOID)mdrv_pm_log(PM_MOD_AP_OSA, sizeof(VOS_AMsgLog), &osaMsgLog);
        }

        mdrv_err("<V_ICC_OSAMsg_CB> v_msg senderpid=%d, receivepid=%d, msgid=0x%x.\n", msgCtrlBlk->ulSenderPid,
                 msgCtrlBlk->ulReceiverPid, *((VOS_UINT32 *)(msgCtrlBlk->value)));
    }
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if (g_nrToLrIccDpm == VOS_TRUE) {
        g_nrToLrIccDpm = VOS_FALSE;

        iccDumpIndex = g_nrToLrIccDumpMsgCount % NR_TO_LR_ICC_DUMP_MSG_MAX;

        g_nrToLrIccDumpMsg[iccDumpIndex].sendPid = msgCtrlBlk->ulSenderPid;
        g_nrToLrIccDumpMsg[iccDumpIndex].rcvPid  = msgCtrlBlk->ulReceiverPid;
        g_nrToLrIccDumpMsg[iccDumpIndex].msgName = *((VOS_UINT32 *)(msgCtrlBlk->value));
        g_nrToLrIccDumpMsg[iccDumpIndex].slice   = VOS_GetSlice();

        g_nrToLrIccDumpMsgCount++;
    }
#endif

    VOS_ModifyMsgInfo((VOS_VOID *)msgCtrlBlk, msgCtrlBlk->ulSenderPid);

    return V_SendMsg(VOS_PID_DOPRAEND, (VOS_VOID **)(&(msgData)), VOS_FILE_ID, __LINE__);
}


#if (OSA_CPU_CCPU == VOS_OSA_CPU)

/*
 * Description: Receive the msg from the ICC channel, and send it to the Receive PID.
 */
VOS_UINT V_NrCcpuIccOsaMsgCb(VOS_UINT channelId, VOS_INT len)
{
    g_vosNRCcoreIccDebugInfo.rcvNum++;
    g_vosNRCcoreIccDebugInfo.rcvSlice = VOS_GetSlice();
    g_vosNRCcoreIccDebugInfo.rcvLen += (VOS_UINT32)len;

    return V_ICC_OSAMsg_CB(UDI_ICC_NRCCPU_LRCCPU_OSA, len);
}
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
/*
 * Description: Receive the msg from the ICC channel, and send it to the Receive PID.
 */
VOS_UINT V_NrAcpuIccOsaMsgCb(VOS_UINT channelId, VOS_INT len)
{
    g_vosNRAcoreIccDebugInfo.rcvNum++;
    g_vosNRAcoreIccDebugInfo.rcvSlice = VOS_GetSlice();
    g_vosNRAcoreIccDebugInfo.rcvLen += (VOS_UINT32)len;

    return V_ICC_OSAMsg_CB(UDI_ICC_NRCCPU_APCPU_OSA, len);
}
#endif


/*
 * Description: Receive the msg from the ICC channel, and send it to the Receive PID.
 */
VOS_UINT V_CcpuAcpuIccOsaMsgCb(VOS_UINT channelId, VOS_INT len)
{
    g_vosAcoreCcoreIccDebugInfo.rcvNum++;
    g_vosAcoreCcoreIccDebugInfo.rcvSlice = VOS_GetSlice();
    g_vosAcoreCcoreIccDebugInfo.rcvLen += (VOS_UINT32)len;

    if (g_iccShareBuffer != VOS_NULL_PTR) {
        g_iccShareBuffer->rcvNum   = g_vosAcoreCcoreIccDebugInfo.rcvNum;
        g_iccShareBuffer->rcvSlice = g_vosAcoreCcoreIccDebugInfo.rcvSlice;
        g_iccShareBuffer->rcvLen   = g_vosAcoreCcoreIccDebugInfo.rcvLen;
    }

    return V_ICC_OSAMsg_CB(UDI_ICC_GUOM4, len);
}

/*
 * 功能描述: 初始化ICC通道
 */
VOS_UINT32 VOS_ICC_Init(VOS_VOID)
{
    VOS_IccUdiCtrl iCCCtrlTable = {0};

    /* open the channel of ACPU&CCPU */
    iCCCtrlTable.iccId                  = UDI_ICC_GUOM4;
    iCCCtrlTable.iccAttr.read_cb        = V_CcpuAcpuIccOsaMsgCb;
    iCCCtrlTable.iccAttr.u32Priority    = VOS_ICC_CHANNEL_PRIORITY; /* 统一使用最高优先级 */
    iCCCtrlTable.iccAttr.u32TimeOut     = VOS_ICC_HANDSHAKE_TIME_MAX;
    iCCCtrlTable.iccAttr.u32FIFOInSize  = OSA_ICC_BUFFER_SIZE;
    iCCCtrlTable.iccAttr.u32FIFOOutSize = OSA_ICC_BUFFER_SIZE;
    iCCCtrlTable.iccAttr.enChanMode     = ICC_CHAN_MODE_PACKET;
    iCCCtrlTable.iccAttr.event_cb       = VOS_NULL_PTR;
    iCCCtrlTable.iccAttr.write_cb       = VOS_NULL_PTR;

    if (VOS_ERROR == DRV_ICC_OPEN(iCCCtrlTable.iccId, &iCCCtrlTable.iccAttr)) {
        /* 打开失败时记录当前ICC通道信息 */
        VOS_ProtectionReboot(OM_APP_ICC_INIT_ERROR, THIS_FILE_ID, __LINE__, VOS_NULL_PTR, 0);
        return VOS_ERR;
    }


#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    /* open the channel of NR&ACPU */
    iCCCtrlTable.iccId           = UDI_ICC_NRCCPU_APCPU_OSA;
    iCCCtrlTable.iccAttr.read_cb = V_NrAcpuIccOsaMsgCb;
#else
    /* open the channel of NR&CCPU */
    iCCCtrlTable.iccId           = UDI_ICC_NRCCPU_LRCCPU_OSA;
    iCCCtrlTable.iccAttr.read_cb = V_NrCcpuIccOsaMsgCb;
#endif

    if (VOS_ERROR == DRV_ICC_OPEN(iCCCtrlTable.iccId, &iCCCtrlTable.iccAttr)) {
        /* 打开失败时记录当前ICC通道信息 */
        VOS_ProtectionReboot(OM_APP_ICC_INIT_ERROR, THIS_FILE_ID, __LINE__, VOS_NULL_PTR, 0);
        return VOS_ERR;
    }


#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    /* 注册icc 唤醒A核时的钩子函数，AP在睡眠状态下被CP唤醒 */
    if (VOS_OK != mdrv_icc_register_resume_cb(UDI_ICC_GUOM4, VOS_MsgLpmCb, 0)) {
        return VOS_ERR;
    }
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    /* 注册C核icc唤醒钩子函数，LR在睡眠流程会收到NR发送过来的消息 */
    if (mdrv_icc_dpm_register(UDI_ICC_NRCCPU_LRCCPU_OSA, VOS_NrToLrIccDpmCb) != VOS_OK) {
        return VOS_ERR;
    }
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    /* 注册OSA ICC通道维测信息的共享内存，共享内存只在A核注册 */
    if (mdrv_om_register_field(DUMP_SAVE_OSA_ICC_INFO, "OSAICC", VOS_OSA_ICC_INFO_ALL_SIZE,
                               0) == VOS_NULL_PTR) {
        mdrv_err("<VOS_ICC_Init> mdrv_om_register_field OSAICC Error.\n");
    }
#endif

    g_iccShareBuffer = (VOS_IccDebugInfo *)VOS_ICC_INFO_MEM_ALLOC;

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    if (g_iccShareBuffer != VOS_NULL_PTR) {
        g_iccShareBuffer++; /* A核使用前面的BUFFER，C核使用后面的BUFFER */
    }
#endif

    VOS_IccDebugInfoInit();

    return VOS_OK;
}

/*
 * Description: the init of the msg
 */
VOS_UINT32 VOS_MsgInit(VOS_VOID)
{
    VOS_UINT32 result = 0;

    /* ICC channel Init */
    if (VOS_OK != VOS_ICC_Init()) {
        result |= 0x00100000;
    }

#if (FEATURE_ON == FEATURE_HIFI_USE_ICC)
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    /* HIFI ICC channel Init */
    if (VOS_OK != VOS_Hifi_ICC_Init()) {
        result |= 0x00200000;
    }
#endif
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    /* DRV MB Init */
    if (VOS_OK != VOS_DRVMB_Init()) {
        result |= 0x00400000;
    }
#endif


    VOS_SpinLockInit(&g_vosDumpMsgSpinLock);
    g_vosDumpMsgFlag = VOS_TRUE;

    return result;
}

/*
 * Description: send urgent message, that in this CPU
 */
VOS_UINT32 V_SendLocalUrgentMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    MsgBlock          *msgCtrlBlk = VOS_NULL_PTR;
    VOS_UINT32         rcvPid;
    VOS_UINT32         fid;
    VOS_UINT32         qid;
    VOS_VOID          *actualMsg = VOS_NULL_PTR;
    VOS_UINT_PTR       blockAdd;
    VOS_UINT_PTR       ctrlkAdd;
    VOS_UINT32         realTaskId;
    VOS_DumpMsg *dumpMsgInfo = VOS_NULL_PTR;

    msgCtrlBlk = (MsgBlock *)(*msg);
    rcvPid       = msgCtrlBlk->ulReceiverPid;

    if (rcvPid < VOS_PID_DOPRAEND) {
        mdrv_err("<V_SendLocalUrgentMsg> Rec PID=%d Check error.\n", (VOS_INT)rcvPid);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_ICC_PIDTOOSMALL);

        VOS_ProtectionReboot(VOS_ERRNO_MSG_ICC_PIDTOOSMALL, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }

    if (rcvPid >= VOS_PID_BUTT) { /* The Receive PID is too large */
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_POST_INPUTPIDINVALID);

        mdrv_err("<V_SendLocalUrgentMsg> invalid PID=%d.\n", (VOS_INT)rcvPid);

        VOS_ProtectionReboot(OSA_SEND_MSG_PID_BIG, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }

    fid = (VOS_UINT32)(g_vosPidRecords[rcvPid - VOS_PID_DOPRAEND].fid);

    /*lint -e574*/
    if ((fid < (VOS_UINT32)VOS_FID_DOPRAEND) || (fid >= (VOS_UINT32)VOS_FID_BUTT)) {
        mdrv_err("<V_SendLocalUrgentMsg> Send PID=%d,Rec PID=%d.\n", msgCtrlBlk->ulSenderPid, (VOS_INT)rcvPid);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_PID_NO_FID);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }
    /*lint +e574*/

    qid = g_vosFidCtrlBlk[fid].qid;

    actualMsg = (VOS_VOID *)((VOS_UINT_PTR)(*msg) - VOS_MSG_BLK_HEAD_LEN);
    if (VOS_OK != VOS_MemCheck(actualMsg, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        mdrv_err("<V_SendLocalUrgentMsg> mem check error.\n");

        VOS_ProtectionReboot(OSA_SEND_URG_MSG_FAIL_TO_CHECK, (VOS_INT)fileId, lineNo, (VOS_CHAR *)msgCtrlBlk,
                             VOS_MSG_BLOCK_HEAD_AND_RESERVED_LENGTH);

        return VOS_ERR;
    }

    if (VOS_OK != VOS_FixedQueueWrite(qid, actualMsg, (VOS_UINT32)MSG_SEND_SIZE, VOS_EMERGENT_PRIORITY_MSG, rcvPid)) {
        mdrv_err("<V_SendLocalUrgentMsg> queue full,Tx Pid=%d Rx Pid=%d Name=0x%x.\n", (int)(msgCtrlBlk->ulSenderPid),
                 (int)rcvPid, (int)(*(VOS_UINT32 *)(msgCtrlBlk->value)));

        if (VOS_FALSE == VOS_MsgDumpCheck()) {
            mdrv_err("V_SendLocalUrgentMsg not need dump\n");

            (VOS_VOID)VOS_FreeMsg(pid, *msg);

            return VOS_ERR;
        }

        dumpMsgInfo = (VOS_DumpMsg *)g_dumpMsgInfo;

        (VOS_VOID)VOS_TaskLock();

        dumpMsgInfo->qid     = qid;
        dumpMsgInfo->sendPid = msgCtrlBlk->ulSenderPid;
        dumpMsgInfo->rcvPid  = rcvPid;
        dumpMsgInfo->msgName = *(VOS_UINT32 *)(msgCtrlBlk->value);

        dumpMsgInfo++;

        VOS_QueuePrintFull(qid, (VOS_CHAR *)dumpMsgInfo, DUMP_MSG_INFO_LEN - sizeof(VOS_DumpMsg));

        realTaskId = VOS_GetRealTID(g_vosFidCtrlBlk[fid].tid);

        VOS_ProtectionReboot(VOS_SAVE_STACK(OSA_SEND_URG_MSG_FAIL_TO_WRITE), (VOS_INT)realTaskId, lineNo,
                             g_dumpMsgInfo, DUMP_MSG_INFO_LEN);

        (VOS_VOID)VOS_TaskUnlock();

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return VOS_ERR;
    }

    if (VOS_MSG_RESERVED_HEAD_FLAG != ((MSG_BlockHead *)actualMsg)->flag) {
        *msg = VOS_NULL_PTR;
    }

    return VOS_OK;
}

/*
 * Description: send urgent message
 */
VOS_UINT32 V_SendUrgentMsg(VOS_PID pid, VOS_VOID **msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32 result;
    VOS_UINT32 rcvPid;
    VOS_UINT32 cpuID;
    MsgBlock  *msgCtrlBlk = VOS_NULL_PTR;
    VOS_UINT32 spanMsg;

    result = V_CheckMsgPara(pid, msg, fileId, lineNo);
    if (VOS_OK != result) {
        return result;
    }

    msgCtrlBlk = (MsgBlock *)(*msg);
    rcvPid       = msgCtrlBlk->ulReceiverPid;
    cpuID     = VOS_GET_CPU_ID(rcvPid);
    if (VOS_NULL_PTR == g_vosSendMsgProcTable[cpuID].sendUrgentMsg) {
        mdrv_err("<V_SendUrgentMsg> send api is null.F=%d L=%d RecvPid=%d.\n", (VOS_INT)fileId, lineNo,
                 (VOS_INT)rcvPid);

        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_SEND_FUNCEMPTY);

        return (VOS_ERRNO_MSG_SEND_FUNCEMPTY);
    }

    spanMsg = VOS_CheckMsgCPUId(cpuID);
    /* 中断中发送跨核消息，返回错误 */
    if ((VOS_TRUE == spanMsg) && (VOS_FALSE != VOS_CheckInterrupt())) {
        (VOS_VOID)VOS_FreeMsg(pid, *msg);

        return (VOS_ERRNO_MSG_INT_MSGERROR);
    }

    if ((VOS_NULL_PTR != vos_MsgHook) && (VOS_PID_BUTT != g_vosSendMsgProcTable[cpuID].endPID)) {
        (VOS_VOID)(vos_MsgHook)(*msg);
    }

    return g_vosSendMsgProcTable[cpuID].sendUrgentMsg(pid, msg, fileId, lineNo);
}

/*
 * Description: This function is synchronization msg send.
 * In fact, it calls receiver Pid's Msg Process func to handle
 * the message directly.
 */
VOS_UINT32 V_PostMsg(VOS_PID pid, VOS_VOID *msg, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT32   receiverPid;
    VOS_UINT32   receiverCpuId;
    VOS_UINT_PTR blockAdd;
    VOS_UINT_PTR ctrlkAdd;
    VOS_VOID    *actualMsg = VOS_NULL_PTR;

    if (VOS_NULL_PTR == msg) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_POST_INPUTMSGISNULL);
        return VOS_ERRNO_MSG_POST_INPUTMSGISNULL;
    }

    /* The sender Pid and Receiver Pid must in one Board */
    receiverCpuId = ((MsgBlock *)msg)->ulReceiverCpuId;
    if (VOS_LOCAL_CPUID != receiverCpuId) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_POST_RECVCPUNOTLOCAL);
        return VOS_ERRNO_MSG_POST_RECVCPUNOTLOCAL;
    }

    receiverPid = ((MsgBlock *)msg)->ulReceiverPid;
    if (receiverPid < VOS_PID_DOPRAEND) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_POST_RECVPIDINVALID);
        return VOS_ERRNO_MSG_POST_RECVPIDINVALID;
    }

    if (receiverPid >= VOS_PID_BUTT) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_POST_RECVPIDINVALID);
        return VOS_ERRNO_MSG_POST_RECVPIDINVALID;
    }

    actualMsg = (VOS_VOID *)((VOS_UINT_PTR)msg - VOS_MSG_BLK_HEAD_LEN);

    if (VOS_OK != VOS_MemCheck(actualMsg, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        mdrv_err("<VOS_PostMsg> Mem error.\n");
        return VOS_ERR;
    }

    /* call receive Pid's Msg Process func */
    (g_vosPidRecords[receiverPid - VOS_PID_DOPRAEND].msgFunction)((MsgBlock *)msg);

    return VOS_OK;
}

/*
 * Description: get message name
 */
VOS_UINT32 VOS_GetMsgName(VOS_UINT_PTR addrress)
{
    MsgBlock *msgBlock = (MsgBlock *)(addrress + VOS_MSG_BLK_HEAD_LEN);

    return *((VOS_UINT32 *)(msgBlock->value));
}

#if (VOS_DEBUG == VOS_DOPRA_VER)

/*
 * Description: register a hook to msg alloc
 */
VOS_UINT32 VOS_RegisterMsgAllocHook(VOS_UINT32 mode, MEMORY_HOOK_FUNC funcHook)
{
    if (VOS_ALLOC_MODE == mode) {
        g_funcAllocMsgHook = funcHook;

        return VOS_OK;
    } else if (VOS_FREE_MODE == mode) {
        g_funcFreeMsgHook = funcHook;

        return VOS_OK;
    } else {
        mdrv_err("<VOS_RegisterMsgAllocHook> mode error.\n");

        return VOS_ERR;
    }
}

#endif


#if (OSA_CPU_CCPU == VOS_OSA_CPU)
/*
 * Description: print Icc's debug info
 */
MODULE_EXPORTED VOS_VOID VOS_ShowIccDumpMsg(VOS_VOID)
{
    VOS_UINT32 iccDumpIndex;

    for (iccDumpIndex = 0; iccDumpIndex < NR_TO_LR_ICC_DUMP_MSG_MAX; iccDumpIndex++) {
        mdrv_err("<VOS_ShowIccDumpMsg> Icc Msg senderpid=%d, receiverpid=%d, msgid=0x%x, slice=%d.\n",
                 g_nrToLrIccDumpMsg[iccDumpIndex].sendPid, g_nrToLrIccDumpMsg[iccDumpIndex].rcvPid,
                 g_nrToLrIccDumpMsg[iccDumpIndex].msgName, g_nrToLrIccDumpMsg[iccDumpIndex].slice);
    }
}
#endif

