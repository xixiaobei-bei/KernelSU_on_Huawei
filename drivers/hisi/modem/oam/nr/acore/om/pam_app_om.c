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
 * 功能描述: 该C文件给出了---ACPU's OM模块实现
 * 生成日期: 2018-2-1
 */
#include "pam_app_om.h"
#include "om_private.h"
#include "at_oam_interface.h"
#include "ps_log_filter_interface.h"
#include "si_pih.h"
#include "pam_tag.h"
#include "mdrv_nvim.h"

#include "msp_diag_comm.h"

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#pragma pack(push, 4)

#define THIS_FILE_ID PS_FILE_ID_PAM_APP_OM_C
#define THIS_MODU mod_pam_om

#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))

#define OM_NORMAL_LOG1(string, para1) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, PS_LOG_LEVEL_NORMAL),\
        ACPU_PID_PAM_OM, VOS_NULL_PTR, __LINE__, string "%d \r\n", (VOS_INT32)(para1))

typedef struct {
    VOS_PID pid;
    VOS_PID i0Pid;
} OM_PID_Map;

/* 记录收到消息信息的buffer及当前长度 */
OM_RecordBuf g_acpuRecordInfo[VOS_EXC_DUMP_MEM_NUM_BUTT];

VOS_UINT32 g_acpuOmFilterFlag;

PAM_STATIC const  OM_PID_Map g_omUsimPidList[] = {
    {I0_MAPS_PIH_PID,   I0_MAPS_PIH_PID},
    {I1_MAPS_PIH_PID,   I0_MAPS_PIH_PID},
    {I2_MAPS_PIH_PID,   I0_MAPS_PIH_PID},
    {I0_MAPS_STK_PID,   I0_MAPS_STK_PID},
    {I1_MAPS_STK_PID,   I0_MAPS_STK_PID},
    {I2_MAPS_STK_PID,   I0_MAPS_STK_PID},
    {I0_MAPS_PB_PID,    I0_MAPS_PB_PID},
    {I1_MAPS_PB_PID,    I0_MAPS_PB_PID},
    {I2_MAPS_PB_PID,    I0_MAPS_PB_PID},
    {I0_MAPS_EMAT_PID,  I0_MAPS_EMAT_PID},
    {I1_MAPS_EMAT_PID,  I0_MAPS_EMAT_PID},
    {I2_MAPS_EMAT_PID,  I0_MAPS_EMAT_PID},
};

/*
 * 功能描述: 当安全函数返回失败时，打印其返回值
 * 修改历史:
 * 1. 日    期: 2019年05月31日
 *    修改内容: 新生成函数
 */
VOS_VOID PAM_OM_PrintSecResult(errno_t ret, VOS_UINT32 fileId, VOS_UINT32 line)
{
    if (ret != EOK) {
        mdrv_err("<PAM_OM_PrintSecResult> secfun errno %d,file %d,line %d.\n", ret, fileId, line);
    }

    return;
}

/*
 * 功能描述: 当非安全函数返回失败时，打印其返回值
 * 修改历史:
 * 1. 日    期: 2019年05月31日
 *    修改内容: 新生成函数
 */
VOS_VOID PAM_OM_PrintNormalResult(VOS_UINT32 ret, VOS_UINT32 fileId, VOS_UINT32 line)
{
    if (ret != EOK) {
        mdrv_err("<PAM_OM_PrintNormalResult> result %d,file %d,line %d.\n", ret, fileId, line);
    }

    return;
}

/*
 * 功能描述: A核保留桩函数
 * 修改历史:
 * 1. 日    期: 2012-07-09
 *    修改内容: 新生成函数
 */
VOS_VOID OM_RecordInfoEnd(VOS_ExcDumpMemNumUint32 number)
{
    VOS_UINT32 *buf = VOS_NULL_PTR;

    if (number >= VOS_EXC_DUMP_MEM_NUM_BUTT) {
        return;
    }

    if (g_acpuRecordInfo[number].buf == VOS_NULL_PTR) {
        return;
    }

    if (g_acpuRecordInfo[number].len == 0) {
        return;
    }

    /* 在start中已经变更了记录endslice的长度，因此此处回退四个字节填写endslice的值 */
    buf = (VOS_UINT32 *)(g_acpuRecordInfo[number].buf + g_acpuRecordInfo[number].len - sizeof(VOS_UINT32));

    *buf = VOS_GetSlice();

    return;
}

/*
 * 功能描述: A核保留桩函数
 * 修改历史:
 * 1. 日    期: 2012-07-09
 *    修改内容: 新生成函数
 */
VOS_VOID OM_RecordInfoStart(VOS_ExcDumpMemNumUint32 number, VOS_UINT32 sendPid, VOS_UINT32 rcvPid, VOS_UINT32 msgName)
{
    OM_RecordInfo recordInfo;

    if (number >= VOS_EXC_DUMP_MEM_NUM_BUTT) {
        return;
    }

    if (g_acpuRecordInfo[number].buf == VOS_NULL_PTR) {
        return;
    }

    g_acpuRecordInfo[number].len %= VOS_TASK_DUMP_INFO_SIZE;

    recordInfo.sendPid    = (VOS_UINT16)sendPid;
    recordInfo.rcvPid     = (VOS_UINT16)rcvPid;
    recordInfo.msgName    = msgName;
    recordInfo.sliceStart = VOS_GetSlice();
    recordInfo.sliceEnd   = 0;

    if (memcpy_s((g_acpuRecordInfo[number].buf + g_acpuRecordInfo[number].len),
                 (VOS_TASK_DUMP_INFO_SIZE - g_acpuRecordInfo[number].len), &recordInfo, sizeof(OM_RecordInfo)) != EOK) {
        mdrv_err("<OM_RecordInfoStart> memcpy_s Fail!\n");
    }

    g_acpuRecordInfo[number].len += (VOS_UINT32)sizeof(OM_RecordInfo);

    return;
}

/*
 * 功能描述: 可维可测空间分配
 * 修改历史:
 * 1. 日    期: 2012-06-28
 *    修改内容: 新生成函数
 */
VOS_VOID OM_RecordMemInit(VOS_VOID)
{
    VOS_UINT32 i;

    (VOS_VOID)memset_s(g_acpuRecordInfo, sizeof(g_acpuRecordInfo), 0, sizeof(g_acpuRecordInfo));

    /* 分配每个模块记录可谓可测信息的空间 */
    for (i = 0; i < VOS_EXC_DUMP_MEM_NUM_BUTT; i++) {
        g_acpuRecordInfo[i].buf = (VOS_UINT8 *)VOS_ExcDumpMemAlloc(i);

        if (g_acpuRecordInfo[i].buf == VOS_NULL_PTR) {
            return;
        }
    }

    return;
}

/*
 * 功能描述: Handle all timer messages sent to OM.
 * 修改历史:
 * 1. 日    期: 2012-04-12
 *    修改内容: 新生成函数
 */
VOS_VOID PAMOM_AcpuTimerMsgProc(MsgBlock *msg)
{
    return;
}
/*
 * 功能描述: querey pid info.
 * 修改历史:
 * 1. 日    期: 2015-06-10
 *    修改内容: 新生成函数
 */
VOS_VOID PAMOM_QuereyPidInfo(VOS_UINT32 pid)
{
    PAM_VOS_QuereyPidInfoReq *msg = VOS_NULL_PTR;

    msg = (PAM_VOS_QuereyPidInfoReq *)VOS_AllocMsg(ACPU_PID_PAM_OM,
                                                   sizeof(PAM_VOS_QuereyPidInfoReq) - VOS_MSG_HEAD_LENGTH);
    /* 分配消息失败 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    msg->ulReceiverPid = pid;
    msg->primId      = PAM_VOS_QUEREY_PID_INFO_REQ;

    if (VOS_SendMsg(ACPU_PID_PAM_OM, msg) != VOS_OK) {
        mdrv_err("<PAMOM_QuereyPidInfo> VOS_SendMsg Fail!\n");
    }

    return;
}

/*
 * 功能描述: querey pid info.
 * 修改历史:
 * 1. 日    期: 2015-06-10
 *    修改内容: 新生成函数
 */
VOS_VOID PAMOM_QuereyPidInfoMsgProc(const MsgBlock *msg, VOS_UINT32 cpuId)
{
    PAM_VOS_QuereyPidInfoReq *reqMsg = VOS_NULL_PTR;
    PAM_VOS_QuereyPidInfoCnf *cnfMsg   = VOS_NULL_PTR;

    reqMsg = (PAM_VOS_QuereyPidInfoReq *)msg;

    if (reqMsg->primId == PAM_VOS_QUEREY_PID_INFO_CNF) {
        cnfMsg = (PAM_VOS_QuereyPidInfoCnf *)msg;

        if (cnfMsg->ulLength <
            ((sizeof(PAM_VOS_QuereyPidInfoCnf) - VOS_MSG_HEAD_LENGTH - sizeof(cnfMsg->value)) + cnfMsg->len)) {
            mdrv_err("<PAMOM_QuereyPidInfoMsgProc> the cnf msg len err: %d.\n", cnfMsg->ulLength);
            return;
        }

        VOS_SetCpuPidInfo(cpuId, (VOS_VOID *)(cnfMsg->value), cnfMsg->len);
    }

    return;
}

/*
 * 功能描述: 获取I 0的PID
 * 修改历史:
 * 1. 日    期: 2018年05月07日
 *    修改内容: 新生成函数
 */
VOS_PID PAM_OM_GetI0UsimPid(VOS_PID pid)
{
    VOS_UINT32 i;
    VOS_UINT32 pidListNum;

    pidListNum = ARRAYSIZE(g_omUsimPidList);

    for (i = 0; i < pidListNum; i++) {
        if (g_omUsimPidList[i].pid == pid) {
            return g_omUsimPidList[i].i0Pid;
        }
    }

    return pid;
}

/*
 * 功能描述: 过滤pih到at 消息的函数 CGLA CRSM CRLA
 * 修改历史:
 * 1. 日    期: 2016年08月29日
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAM_OM_AcpuPihToAtMsgFilter(const MsgBlock *msg)
{
    MN_APP_PihAtCnf *eventCnf = VOS_NULL_PTR;

    eventCnf = (MN_APP_PihAtCnf *)msg;

    if (eventCnf->msgId == PIH_AT_EVENT_CNF) {
        OM_NORMAL_LOG1("PAM_OM_AcpuAtToPihMsgFilter: The filter eventType is :", eventCnf->pihAtEvent.eventType);

        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * 功能描述: 过滤pih到at 消息的函数 CGLA CRSM CRLA
 * 修改历史:
 * 1. 日    期: 2016年08月29日
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAM_OM_AcpuPihToPCSCMsgFilter(const MsgBlock *msg)
{
    MSG_Header *msgHead = VOS_NULL_PTR;

    msgHead = (MSG_Header *)msg;

    if (msgHead->msgName == USIMM_CARDSTATUS_IND) {
        OM_NORMAL_LOG1("PAM_OM_AcpuPihToPCSCMsgFilter: The filter name is :", msgHead->msgName);

        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * 功能描述: 过滤pih到A核的消息
 * 修改历史:
 * 1. 日    期: 2018年08月29日
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAM_OM_AcpuPihMsgFilter(const MsgBlock *msg)
{
    MSG_Header *msgHead = VOS_NULL_PTR;
    VOS_UINT32       ret;

    msgHead = (MSG_Header *)msg;

    switch (msgHead->ulReceiverPid) {
        case WUEPS_PID_AT:
            ret = PAM_OM_AcpuPihToAtMsgFilter(msg);
            break;

        case ACPU_PID_PCSC:
            ret = PAM_OM_AcpuPihToPCSCMsgFilter(msg);
            break;

        default:
            ret = VOS_FALSE;
            break;
    }

    return ret;
}

/*
 * 功能描述: 过滤stk到at 消息的函数
 * 修改历史:
 * 1. 日    期: 2018年05月07日
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAM_OM_AcpuStkToAtMsgFilter(const MsgBlock *msg)
{
    MN_APP_StkAtCnf *eventCnf = VOS_NULL_PTR;

    eventCnf = (MN_APP_StkAtCnf *)msg;

    switch (eventCnf->msgId) {
        case STK_AT_EVENT_CNF:
        case STK_AT_DATAPRINT_CNF:
            OM_NORMAL_LOG1("PAM_OM_AcpuStkToAtMsgFilter: The filter msgId is :", eventCnf->msgId);

            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}

/*
 * 功能描述: 过滤Emat到AT消息的函数
 * VOS_FALSE:表示这个消息需要上报OM
 */
VOS_UINT32 PAM_OM_AcpuEmatToAtMsgFilter(const MsgBlock *msg)
{
    MN_APP_EmatAtCnf *eventCnf = VOS_NULL_PTR;

    eventCnf = (MN_APP_EmatAtCnf *)msg;

    switch (eventCnf->msgId) {
        case EMAT_AT_EVENT_CNF:
            OM_NORMAL_LOG1("PAM_OM_AcpuEmatToAtMsgFilter: The filter msgId is :", eventCnf->msgId);
            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}

/*
 * 功能描述: 过滤at到pih 消息的函数 CGLA CRSM CRLA
 * 修改历史:
 * 1. 日    期: 2016年08月29日
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAM_OM_AcpuAtToPihMsgFilter(const MsgBlock *msg)
{
    MSG_Header *msgHead = VOS_NULL_PTR;

    msgHead = (MSG_Header *)msg;

    switch (msgHead->msgName) {
        case SI_PIH_CRSM_SET_REQ:
        case SI_PIH_CRLA_SET_REQ:
        case SI_PIH_CGLA_SET_REQ:
#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))
        case SI_PIH_SILENT_PININFO_SET_REQ:
#endif
        case SI_PIH_GACCESS_REQ:
        case SI_PIH_ISDB_ACCESS_REQ:
        case SI_PIH_PRIVATECGLA_SET_REQ:
        case SI_PIH_URSM_REQ:
        case SI_PIH_UICCAUTH_REQ:
        case SI_PIH_FDN_ENABLE_REQ:
        case SI_PIH_FDN_DISALBE_REQ:
            OM_NORMAL_LOG1("PAM_OM_AcpuAtToPihMsgFilter: The filter msgName is :", msgHead->msgName);

            return VOS_TRUE;

        default:
            break;
    }

    return VOS_FALSE;
}

/*
 * 功能描述: 过滤at到stk 消息的函数
 * 修改历史:
 * 1. 日    期: 2018年05月07日
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAM_OM_AcpuAtToStkMsgFilter(const MsgBlock *msg)
{
    MSG_Header *msgHead = VOS_NULL_PTR;

    msgHead = (MSG_Header *)msg;

    OM_NORMAL_LOG1("PAM_OM_CcpuAtToStkMsgFilter: The Filter At To Stk Msg and Type Are: ", msgHead->msgName);

    return VOS_TRUE;
}

/*
 * 功能描述: 过滤PAM中的敏感信息
 * 修改历史:
 * 1. 日    期: 2015年10月28日
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAM_OM_AcpuLayerMsgFilterOptProc(const MsgBlock *msg)
{
    OM_FilterMsgHead *msgHead = VOS_NULL_PTR;
    VOS_PID           sendI0Pid;
    VOS_PID           recvI0Pid;

    if (g_acpuOmFilterFlag == VOS_FALSE) {
        return VOS_FALSE;
    }

    msgHead = (OM_FilterMsgHead *)msg;

    sendI0Pid = PAM_OM_GetI0UsimPid(msgHead->ulSenderPid);
    recvI0Pid = PAM_OM_GetI0UsimPid(msgHead->ulReceiverPid);

    /* PB相关的消息全部过滤 */
    if ((sendI0Pid == I0_MAPS_PB_PID) || (recvI0Pid == I0_MAPS_PB_PID) || (msgHead->ulReceiverPid == ACPU_PID_PB)) {
        return VOS_TRUE;
    }

    /* PIH 消息过滤 */
    if (sendI0Pid == I0_MAPS_PIH_PID) {
        return PAM_OM_AcpuPihMsgFilter(msg);
    }

    if ((sendI0Pid == WUEPS_PID_AT) && (recvI0Pid == I0_MAPS_PIH_PID)) {
        return PAM_OM_AcpuAtToPihMsgFilter(msg);
    }

    if ((sendI0Pid == WUEPS_PID_AT) && (recvI0Pid == I0_MAPS_STK_PID)) {
        return PAM_OM_AcpuAtToStkMsgFilter(msg);
    }

    if (sendI0Pid == I0_MAPS_STK_PID) {
        return PAM_OM_AcpuStkToAtMsgFilter(msg);
    }

    if (sendI0Pid == I0_MAPS_EMAT_PID) {
        return PAM_OM_AcpuEmatToAtMsgFilter(msg);
    }

    return VOS_FALSE;
}

/*
 * 功能描述: PAM OM消息过滤函数
 * 修改历史:
 * 1. 日    期: 2017年04月06日
 *    修改内容: 新生成函数
 */
VOS_VOID *PAM_OM_LayerMsgFilter(MsgBlock *msg)
{
    if (msg == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    if (PAM_OM_AcpuLayerMsgFilterOptProc(msg) == VOS_TRUE) {
        return VOS_NULL_PTR;
    }

    return msg;
}

/*
 * 功能描述: PAM OM为各pid注册过滤函数
 * 修改历史:
 * 1. 日    期: 2017年04月06日
 *    修改内容: 新生成函数
 */
VOS_VOID PAM_OM_LayerMsgReplaceCBReg(VOS_VOID)
{
    PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_MAPS_PIH_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_MAPS_PB_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_MAPS_STK_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I0_MAPS_EMAT_PID, PAM_OM_LayerMsgFilter);

#if (FEATURE_MULTI_MODEM == FEATURE_ON)
    PS_OM_LayerMsgReplaceCBReg(I1_MAPS_PIH_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_MAPS_PB_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_MAPS_STK_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I1_MAPS_EMAT_PID, PAM_OM_LayerMsgFilter);

#if (MULTI_MODEM_NUMBER == 3)
    PS_OM_LayerMsgReplaceCBReg(I2_MAPS_PIH_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_MAPS_PB_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_MAPS_STK_PID, PAM_OM_LayerMsgFilter);
    PS_OM_LayerMsgReplaceCBReg(I2_MAPS_EMAT_PID, PAM_OM_LayerMsgFilter);
#endif
#endif
}


/*
 * 功能描述: LOG打印消息处理
 */
VOS_VOID  PAMOM_KmsgProc(PAM_KmsgInd *msg)
{
#if (VOS_OS_VER == VOS_LINUX)
    if (VOS_GET_MSG_LEN(msg) != sizeof(PAM_KmsgInd) - VOS_MSG_HEAD_LENGTH) {
        return;
    }

    msg->kmsgData.str[PAM_KMSG_PRINT_STR_LEN] = '\0';

    pr_info("[PAMOM]: [INFO] %s %d %d %d %d\n", msg->kmsgData.str, msg->kmsgData.data1,
        msg->kmsgData.data2, msg->kmsgData.data3, msg->kmsgData.data4);
#endif
    return;
}

/*
 * 功能描述: process CCPU pam's msg
 * 修改历史:
 * 1. 日    期: 2015-06-10
 *    修改内容: 新生成函数
 */
VOS_VOID PAMOM_AcpuCcpuPamMsgProc(const MsgBlock *msg)
{
    VOS_UINT16 primId;

    if (msg->ulLength < sizeof(primId)) {
        mdrv_err("<PAMOM_AcpuCcpuPamMsgProc> the msg len err: %d.\n", msg->ulLength);
        return;
    }

    primId = *(VOS_UINT16 *)(msg->value);

    if (primId == PAM_VOS_QUEREY_PID_INFO_CNF) {
        PAMOM_QuereyPidInfoMsgProc(msg, VOS_CPU_ID_CCPU);
    } else if (primId == PAM_DBG_PRINT_IND) {
        PAMOM_KmsgProc((PAM_KmsgInd*)msg);
    } else {
    }

    return;
}

/*
 * 功能描述: process CCPU pam's msg
 * 修改历史:
 * 1. 日    期: 2018-01-20
 *    修改内容: 新生成函数
 */
VOS_VOID PAMOM_AcpuNrcpuPamMsgProc(MsgBlock *msg)
{
    VOS_UINT16 primId;

    if (msg->ulLength < sizeof(primId)) {
        mdrv_err("<PAMOM_AcpuCcpuPamMsgProc> the msg len err: %d.\n", msg->ulLength);
        return;
    }

    primId = *(VOS_UINT16 *)(msg->value);

    if (primId == PAM_VOS_QUEREY_PID_INFO_CNF) {
        PAMOM_QuereyPidInfoMsgProc(msg, VOS_CPU_ID_NRCPU);
    } else {
    }

    return;
}

/*
 * 功能描述: Handle all messages sent to OM.
 * 修改历史:
 * 1. 日    期: 2008-03-20
 *    修改内容: 新生成函数
 */
VOS_VOID PAMOM_AppMsgProc(MsgBlock *msg)
{
    if (msg == VOS_NULL_PTR) {
        return;
    }

    if (msg->ulSenderPid == VOS_PID_TIMER) {
        PAMOM_AcpuTimerMsgProc(msg);
    } else if (msg->ulSenderPid == CCPU_PID_PAM_OM) {
        PAMOM_AcpuCcpuPamMsgProc(msg);
    } else if (msg->ulSenderPid == NRCPU_PID_PAM_OM) {
        PAMOM_AcpuNrcpuPamMsgProc(msg);
    } else {
        /* blank */
    }

    return;
}

/*
 * 功能描述: OM' initializtion function
 * 修改历史:
 * 1. 日    期: 2011-07-01
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAMOM_AcpuInit(VOS_VOID)
{

    PAM_OM_LayerMsgReplaceCBReg();
    PAMOM_QuereyPidInfo(CCPU_PID_PAM_OM);
    PAMOM_QuereyPidInfo(NRCPU_PID_PAM_OM);

    g_acpuOmFilterFlag = VOS_TRUE;

    return VOS_OK;
}

/*
 * 功能描述: ACPU OM PID' initializtion function
 * 修改历史:
 * 1. 日    期: 2011-07-01
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAMOM_AppPidInit(enum VOS_InitPhaseDefine ip)
{
    switch (ip) {
        case VOS_IP_LOAD_CONFIG:
            return PAMOM_AcpuInit();

        default:
            break;
    }

    return VOS_OK;
}

/*
 * 功能描述: ACPU OM PID' initializtion function
 * 修改历史:
 * 1. 日    期: 2011-07-01
 *    修改内容: 新生成函数
 */
VOS_UINT32 PAMOM_APP_FID_Init(enum VOS_InitPhaseDefine ip)
{
    VOS_UINT32 rslt;

    switch (ip) {
        case VOS_IP_LOAD_CONFIG: {
            rslt = VOS_RegisterPIDInfo(ACPU_PID_PAM_OM, (InitFunType)PAMOM_AppPidInit, (MsgFunType)PAMOM_AppMsgProc);
            if (rslt != VOS_OK) {
                return VOS_ERR;
            }

            rslt = VOS_RegisterMsgTaskPrio(ACPU_FID_PAM_OM, VOS_PRIORITY_M2);
            if (rslt != VOS_OK) {
                return VOS_ERR;
            }

            break;
        }

        default:
            break;
    }
    return VOS_OK;
}

#if (VOS_OS_VER != VOS_WIN32)
/*
 * 功能描述: 上报OSA的可维可测消息
 * 修改历史:
 * 1. 日    期: 2013年7月27日
 *    修改内容: 新生成函数
 */
VOS_VOID OM_OSAEvent(VOS_TimerOmEvent *data, VOS_UINT32 length)
{
    mdrv_diag_event_ind_s eventInd;
    VOS_UINT32            diagResult;

    eventInd.ulModule  = MDRV_DIAG_GEN_MODULE(DIAG_MODEM_0, DIAG_MODE_COMM);
    eventInd.ulPid     = ACPU_PID_PAM_OM;
    eventInd.ulEventId = OAM_EVENT_TIMER;
    eventInd.ulLength  = length;
    eventInd.pData     = data;

    data->hTimerAddress = 0;

    diagResult = mdrv_diag_event_report(&eventInd);
    if (diagResult != ERR_MSP_SUCCESS) {
        return;
    }

    return;
}
#endif

/*
 * 功能描述: 对发送方为本项目组的层间消息进行脱敏并上报
 * 修改历史:
 * 1. 日    期: 2020年02月06日
 *    修改内容: 新生成函数
 */
VOS_VOID PAM_OM_ReportLayerMsg(VOS_VOID *oriMsg)
{
    return;
}

#pragma pack(pop)

