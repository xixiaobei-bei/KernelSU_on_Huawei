/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#include "taf_tafm_remote.h"
#include "ps_common_def.h"
#include "mdrv_nvim.h"
#include "taf_api.h"

#include "mn_comm_api.h"
#include "taf_app_mma.h"

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "nas_multi_instance_api.h"
#include "nas_comm.h"
#endif
#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_TAF_MMA_API_C

#define TAF_GET_DEST_PID(clientId, ulPid) TAF_GetDestPid(clientId, ulPid)
#define TAF_GET_SEND_PID(clientId, ulPid) TAF_GetSendPid(clientId, ulPid)

TAF_UINT32 Taf_PhonePinHandle(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_PH_PinData *pinData)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_OP_PIN_REQ, pinData, sizeof(TAF_PH_PinData),
                                  I0_WUEPS_PID_MMA);
}

VOS_UINT32 Taf_MePersonalisationHandle(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                       TAF_ME_PersonalisationData *mePersonalData)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_ME_PERSONAL_REQ, mePersonalData,
                                  sizeof(TAF_ME_PersonalisationData), I0_WUEPS_PID_MMA);
}

/* Taf_PhoneAttach */

TAF_UINT32 Taf_PhonePlmnList(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                             TAF_MMA_PlmnListPara *plmnListPara)
{
    errno_t              memResult;
    TAF_MMA_PlmnListReq *msg = VOS_NULL_PTR;
    VOS_UINT32           receiverPid;
    VOS_UINT32           senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (plmnListPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_DetachReq */
    msg = (TAF_MMA_PlmnListReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PlmnListReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_PLMN_LIST_REQ_STRU */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PlmnListReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PLMN_LIST_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult = memcpy_s(&(msg->plmnListPara), sizeof(msg->plmnListPara), plmnListPara, sizeof(TAF_MMA_PlmnListPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->plmnListPara), sizeof(TAF_MMA_PlmnListPara));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_AbortPlmnListReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_PlmnListAbortReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_PlmnListAbortReq */
    msg = (TAF_MMA_PlmnListAbortReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PlmnListAbortReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PlmnListAbortReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PLMN_LIST_ABORT_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryLocInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_LocationInfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MMA_LOCINFO_QRY_REQ */
    msg = (TAF_MMA_LocationInfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_LocationInfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_LocationInfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_LOCATION_INFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCipherReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CipherQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MMA_LOCINFO_QRY_REQ */
    msg = (TAF_MMA_CipherQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CipherQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CipherQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CIPHER_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetPrefPlmnTypeReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                      const MN_PH_PrefPlmnTypeUint8 *prefPlmnType)
{
    TAF_MMA_PrefPlmnTypeSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_PrefPlmnTypeSetReq */
    msg = (TAF_MMA_PrefPlmnTypeSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PrefPlmnTypeSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PrefPlmnTypeSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PREF_PLMN_TYPE_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->prefPlmnType  = *prefPlmnType;
    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 TAF_MMA_MtPowerDownReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_MtPowerDownReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_MtPowerDownReq */
    msg = (TAF_MMA_MtPowerDownReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_MtPowerDownReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_MtPowerDownReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_MT_POWER_DOWN_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#endif

VOS_UINT32 TAF_MMA_SetQuickStartReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT32 setValue)
{
    TAF_MMA_QuickstartSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_QuickstartSetReq */
    msg = (TAF_MMA_QuickstartSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_QuickstartSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_QuickstartSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName        = ID_TAF_MMA_QUICKSTART_SET_REQ;
    msg->ctrl.moduleId  = moduleId;
    msg->ctrl.clientId  = clientId;
    msg->ctrl.opId      = opId;
    msg->quickStartMode = setValue;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryQuickStartReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_QuickstartQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_QuickstartQryReq */
    msg = (TAF_MMA_QuickstartQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_QuickstartQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_QuickstartQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_QUICKSTART_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

/* 函数Taf_UsimRestrictedAccessCommand无调用点，清理 */

VOS_UINT32 TAF_MMA_QrySyscfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_SyscfgQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_SyscfgQryReq */
    msg = (TAF_MMA_SyscfgQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SyscfgQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SyscfgQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_SYSCFG_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 Taf_SetCopsFormatTypeReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                    TAF_MMA_CopsFormatTypeSetReq *copsFormatType)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_COPS_FORMAT_TYPE_SET_REQ, copsFormatType,
                                  sizeof(TAF_MMA_CopsFormatTypeSetReq), I0_WUEPS_PID_MMA);
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 TAF_SetUsimStub(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_MMA_UsimStubSetReq *usimStub)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_USIM_STUB_SET_REQ, usimStub,
                                  sizeof(TAF_MMA_UsimStubSetReq), I0_WUEPS_PID_MMA);
}

VOS_UINT32 TAF_SetRefreshStub(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_MMA_RefreshStubSetReq *refreshStub)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_REFRESH_STUB_SET_REQ, refreshStub,
                                  sizeof(TAF_MMA_RefreshStubSetReq), I0_WUEPS_PID_MMA);
}
#endif

VOS_UINT32 TAF_SetAutoReselStub(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                TAF_MMA_AutoReselStubSetReq *autoReselStub)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_AUTO_RESEL_STUB_SET_REQ, autoReselStub,
                                  sizeof(TAF_MMA_AutoReselStubSetReq), I0_WUEPS_PID_MMA);
}

TAF_UINT32 TAF_QryUsimInfo(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_PH_QryUsimInfo *info)
{
    MN_CLIENT_OperationId clientOperationId = {0};
    if ((info->icctype == TAF_PH_ICC_UNKNOW) || (info->icctype > TAF_PH_ICC_USIM)) {
        return TAF_FAILURE;
    }

    if (info->efId > TAF_PH_OPL_FILE) {
        return TAF_FAILURE;
    }

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_USIM_INFO, info, sizeof(TAF_PH_QryUsimInfo),
                                  I0_WUEPS_PID_MMA);
}

TAF_UINT32 TAF_QryCpnnInfo(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_PH_IccType iccType)
{
    MN_CLIENT_OperationId clientOperationId = {0};
    if ((iccType == TAF_PH_ICC_UNKNOW) || (iccType > TAF_PH_ICC_USIM)) {
        return TAF_FAILURE;
    }

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_CPNN_INFO, &iccType, sizeof(iccType),
                                  I0_WUEPS_PID_MMA);
}

VOS_UINT32 TAF_MMA_PhoneModeSetReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                   TAF_MMA_PhoneModePara *phoneModePara)
{
    errno_t                     memResult;
    TAF_MMA_PhoneModeSetReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (phoneModePara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_PhoneModeSetReq */
    msg = (TAF_MMA_PhoneModeSetReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PhoneModeSetReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PhoneModeSetReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PHONE_MODE_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult = memcpy_s(&(msg->phoneModePara), sizeof(msg->phoneModePara), phoneModePara,
                         sizeof(TAF_MMA_PhoneModePara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->phoneModePara), sizeof(TAF_MMA_PhoneModePara));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryPhoneModeReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_PhoneModeQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_PhoneModeSetReq */
    msg = (TAF_MMA_PhoneModeQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PhoneModeQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PhoneModeQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PHONE_MODE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#if (FEATURE_CSG == FEATURE_ON)

VOS_UINT32 TAF_MMA_CsgListSearchReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                    TAF_MMA_PlmnListPara *plmnListPara)
{
    errno_t                          memResult;
    TAF_MMA_CsgPlmnListSearchReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                       receiverPid;
    VOS_UINT32                       senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (plmnListPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包 */
    msg = (TAF_MMA_CsgPlmnListSearchReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                          sizeof(TAF_MMA_CsgPlmnListSearchReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_CsgPlmnListSearchReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CSG_LIST_SEARCH_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    memResult = memcpy_s(&msg->plmnListPara, sizeof(msg->plmnListPara), plmnListPara, sizeof(TAF_MMA_PlmnListPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->plmnListPara), sizeof(TAF_MMA_PlmnListPara));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_AbortCsgListSearchReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CsgListAbortReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_CsgListAbortReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CsgListAbortReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CsgListAbortReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CSG_LIST_ABORT_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetCsgIdSearch(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                  TAF_MMA_CsgSpecSearchInfo *userSelCsgId)
{
    TAF_MMA_CsgSpecSearchReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;
    errno_t                   memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (userSelCsgId == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_CsgSpecSearchReq */
    msg = (TAF_MMA_CsgSpecSearchReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CsgSpecSearchReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_CsgSpecSearchReq, 发送PID统一填写为WUEPS_PID_TAF */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CsgSpecSearchReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CSG_SPEC_SEARCH_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    memResult = memcpy_s(&msg->csgSpecSearchInfo, sizeof(msg->csgSpecSearchInfo), userSelCsgId,
                         sizeof(msg->csgSpecSearchInfo));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->csgSpecSearchInfo), sizeof(msg->csgSpecSearchInfo));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCampCsgIdInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_QryCampCsgIdInfoReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                      receiverPid;
    VOS_UINT32                      senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_QryCampCsgIdInfoReq */
    msg = (TAF_MMA_QryCampCsgIdInfoReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                         sizeof(TAF_MMA_QryCampCsgIdInfoReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_QryCampCsgIdInfoReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_QRY_CAMP_CSG_ID_INFO_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#endif

VOS_UINT32 TAF_MMA_SetSysCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                TAF_MMA_SysCfgPara *sysCfgPara)
{
    errno_t            memResult;
    TAF_MMA_SysCfgReq *msg = VOS_NULL_PTR;
    VOS_UINT32         receiverPid;
    VOS_UINT32         senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (sysCfgPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_SysCfgReq */
    msg = (TAF_MMA_SysCfgReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SysCfgReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_SysCfgReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SysCfgReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_SYS_CFG_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    memResult = memcpy_s(&(msg->sysCfgPara), sizeof(msg->sysCfgPara), sysCfgPara, sizeof(TAF_MMA_SysCfgPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->sysCfgPara), sizeof(TAF_MMA_SysCfgPara));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryEonsUcs2Req(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_EonsUcs2Req *msg = VOS_NULL_PTR;
    VOS_UINT32           receiverPid;
    VOS_UINT32           senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_EonsUcs2Req */
    msg = (TAF_MMA_EonsUcs2Req *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EonsUcs2Req));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_EonsUcs2Req */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EonsUcs2Req) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MSG_MMA_EONS_UCS2_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_DetachReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, TAF_MMA_DetachPara *detachPara)
{
    errno_t               memResult;
    TAF_MMA_DetachReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (detachPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_DetachReq */
    msg = (TAF_MMA_DetachReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_DetachReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_DetachReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_DetachReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_DETACH_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult          = memcpy_s(&(msg->detachPara), sizeof(msg->detachPara), detachPara, sizeof(TAF_MMA_DetachPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->detachPara), sizeof(TAF_MMA_DetachPara));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QrySmsAntiAttackCapReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                          VOS_UINT8 antiSmsType)
{
    TAF_MMA_SmsAntiAttackCapQryReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                      receiverPid;
    VOS_UINT32                      senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);
    sndMsg = (TAF_MMA_SmsAntiAttackCapQryReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                            sizeof(TAF_MMA_SmsAntiAttackCapQryReq));
    /* 内存申请失败，返回 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)sndMsg, senderPid, receiverPid,
                  sizeof(TAF_MMA_SmsAntiAttackCapQryReq) - VOS_MSG_HEAD_LENGTH);

    sndMsg->msgName       = ID_TAF_MMA_SMS_ANTI_ATTACK_CAP_QRY_REQ;
    sndMsg->ctrl.moduleId = moduleId;
    sndMsg->ctrl.clientId = clientId;
    sndMsg->ctrl.opId     = opId;
    sndMsg->smstype       = antiSmsType;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, sndMsg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetSmsAntiAttackState(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                         TAF_MMA_SmsAntiAttackStateUint8 state)
{
    TAF_MMA_SmsAntiAttackSetReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32                   receiverPid;
    VOS_UINT32                   senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息内存 */
    sndMsg = (TAF_MMA_SmsAntiAttackSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SmsAntiAttackSetReq));
    /* 内存申请失败，返回 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_DETACH_REQ_STRU */
    TAF_CfgMsgHdr((MsgBlock *)sndMsg, senderPid, receiverPid,
                  sizeof(TAF_MMA_SmsAntiAttackSetReq) - VOS_MSG_HEAD_LENGTH);

    sndMsg->msgName       = ID_TAF_MMA_SMS_ANTI_ATTACK_SET_REQ;
    sndMsg->ctrl.moduleId = moduleId;
    sndMsg->ctrl.clientId = clientId;
    sndMsg->ctrl.opId     = opId;
    sndMsg->state         = state;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, sndMsg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QrySmsNasCapReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_SmsNasCapQryReq *sndMsg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);
    sndMsg      = (TAF_MMA_SmsNasCapQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SmsNasCapQryReq));
    /* 内存申请失败，返回 */
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)sndMsg, senderPid, receiverPid, sizeof(TAF_MMA_SmsNasCapQryReq) - VOS_MSG_HEAD_LENGTH);

    sndMsg->msgName       = ID_TAF_MMA_NAS_SMS_CAP_QRY_REQ;
    sndMsg->ctrl.moduleId = moduleId;
    sndMsg->ctrl.clientId = clientId;
    sndMsg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, sndMsg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

#if (FEATURE_IMS == FEATURE_ON)

VOS_UINT32 TAF_MMA_SetImsSwitchReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                   TAF_MMA_ImsSwitchInfo *imsSwitchInfo)
{
    TAF_MMA_ImsSwitchSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (imsSwitchInfo == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_ImsSwitchSetReq */
    msg = (TAF_MMA_ImsSwitchSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ImsSwitchSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_ImsSwitchSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ImsSwitchSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_IMS_SWITCH_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->imsSwitchInfo = *imsSwitchInfo;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryImsSwitchReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
    TAF_MMA_ImsSwitchTypeUint8 imsSwitchType)
{
    TAF_MMA_ImsSwitchQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_ImsSwitchQryReq */
    msg = (TAF_MMA_ImsSwitchQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ImsSwitchQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_ImsSwitchQryReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ImsSwitchQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_IMS_SWITCH_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->imsSwitchType = imsSwitchType;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryRcsSwitchReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_RcsSwitchQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_ImsSwitchQryReq */
    msg = (TAF_MMA_RcsSwitchQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_RcsSwitchQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_RcsSwitchQryReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_RcsSwitchQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_RCS_SWITCH_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetVoiceDomainReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                     TAF_MMA_VoiceDomainUint32 voiceDomain)
{
    TAF_MMA_VoiceDomainSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (voiceDomain >= TAF_MMA_VOICE_DOMAIN_BUTT) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_VoiceDomainSetReq */
    msg = (TAF_MMA_VoiceDomainSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_VoiceDomainSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_VoiceDomainSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_VoiceDomainSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_VOICE_DOMAIN_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->voiceDomain   = voiceDomain;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryVoiceDomainReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_VoiceDomainQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_VoiceDomainQryReq */
    msg = (TAF_MMA_VoiceDomainQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_VoiceDomainQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_VoiceDomainQryReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_VoiceDomainQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_VOICE_DOMAIN_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetRoamImsSupportReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                        TAF_MMA_RoamImsSupportUint32 roamImsSupport)
{
    errno_t                          memResult;
    TAF_MMA_RoamImsSupportSetReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                       receiverPid;
    VOS_UINT32                       senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (roamImsSupport >= TAF_MMA_ROAM_IMS_BUTT) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_RoamImsSupportSetReq */
    msg = (TAF_MMA_RoamImsSupportSetReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                          sizeof(TAF_MMA_RoamImsSupportSetReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_RoamImsSupportSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_RoamImsSupportSetReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_ROAM_IMS_SUPPORT_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult = memcpy_s(&(msg->roamingImsSupportFlag), sizeof(msg->roamingImsSupportFlag), &roamImsSupport,
                         sizeof(TAF_MMA_RoamImsSupportUint32));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->roamingImsSupportFlag), sizeof(TAF_MMA_RoamImsSupportUint32));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetImsDomainCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                      TAF_MMA_ImsDomainCfgTypeUint32 imsDomainCfg)
{
    TAF_MMA_ImsDomainCfgSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    if (imsDomainCfg >= TAF_MMA_IMS_DOMAIN_CFG_TYPE_BUTT) {
        return VOS_FALSE;
    }

    /* 内存申请 */
    msg = (TAF_MMA_ImsDomainCfgSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ImsDomainCfgSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_ImsDomainCfgSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ImsDomainCfgSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_IMS_DOMAIN_CFG_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->imsDoaminCfg  = imsDomainCfg;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryImsDomainCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_ImsDomainCfgQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 内存申请 */
    msg = (TAF_MMA_ImsDomainCfgQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ImsDomainCfgQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_ImsDomainCfgQryReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ImsDomainCfgQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_IMS_DOMAIN_CFG_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetImsVtCapCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                     TAF_MMA_ImsVideoCallCap *imsVtCap)
{
    errno_t                     memResult;
    TAF_MMA_ImsVideoCallCapReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 内存申请 */
    msg = (TAF_MMA_ImsVideoCallCapReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ImsVideoCallCapReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_ImsSmsCfgSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ImsVideoCallCapReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_IMS_VIDEO_CALL_CAP_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult          = memcpy_s(&msg->imsVtCap, sizeof(msg->imsVtCap), imsVtCap, sizeof(TAF_MMA_ImsVideoCallCap));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->imsVtCap), sizeof(TAF_MMA_ImsVideoCallCap));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetImsSmsCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                   TAF_MMA_ImsSmsCfg *imsSmsCfg)
{
    TAF_MMA_ImsSmsCfgSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 内存申请 */
    msg = (TAF_MMA_ImsSmsCfgSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ImsSmsCfgSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_ImsSmsCfgSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ImsSmsCfgSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_IMS_SMS_CFG_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->imsSmsCfg     = *imsSmsCfg;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryImsSmsCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_ImsSmsCfgQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 内存申请 */
    msg = (TAF_MMA_ImsSmsCfgQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ImsSmsCfgQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_ImsDomainCfgQryReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ImsSmsCfgQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_IMS_SMS_CFG_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#endif

VOS_UINT32 TAF_MMA_AttachReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                             TAF_MMA_AttachTypeUint8 attachType)
{
    TAF_MMA_AttachReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请内存消息并初始化 */
    msg = (TAF_MMA_AttachReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_AttachReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_AttachReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_AttachReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_ATTACH_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->attachType    = attachType;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_AttachStatusQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                      TAF_MMA_ServiceDomainUint8 domainType)
{
    TAF_MMA_AttachStatusQryReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                     receiverPid;
    VOS_UINT32                     senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请内存消息并初始化 */
    msg = (TAF_MMA_AttachStatusQryReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                        sizeof(TAF_MMA_AttachStatusQryReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_AttachReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_AttachStatusQryReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_ATTACH_STATUS_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->domainType    = domainType;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

MODULE_EXPORTED VOS_VOID TAF_MMA_SrvAcqReq(TAF_MMA_SrvTypeUint8 srvType, TAF_MMA_SrvAcqRatList *ratList,
                                           VOS_UINT32 moduleId)
{
    ModemIdUint16         modemId;
    TAF_MMA_SrvAcqReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32            sndPid;
    VOS_UINT32            rcvPid;

    modemId = NAS_MULTIINSTANCE_GetModemIdFromPid(moduleId);
    sndPid  = NAS_MULTIINSTANCE_GetSpecModemPid(modemId, WUEPS_PID_TAF);
    rcvPid  = NAS_MULTIINSTANCE_GetSpecModemPid(modemId, WUEPS_PID_MMA);

    /* 申请消息包TAF_MMA_SrvAcqReq */
    msg = (TAF_MMA_SrvAcqReqMsg *)TAF_AllocMsgWithHeaderLen(sndPid, sizeof(TAF_MMA_SrvAcqReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, sndPid, rcvPid, sizeof(TAF_MMA_SrvAcqReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = ID_TAF_MMA_SRV_ACQ_REQ;

    msg->ctrl.moduleId = moduleId;

    msg->srvType = srvType;
    msg->ratList = *ratList;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(sndPid, msg) != VOS_OK) {
        return;
    }
}

MODULE_EXPORTED VOS_VOID TAF_MMA_SrvAcqStop(VOS_UINT32 moduleId)
{
    TAF_MMA_StopSrvacqNotify *msg = VOS_NULL_PTR;
    ModemIdUint16             modemId;
    VOS_UINT32                sndPid;
    VOS_UINT32                rcvPid;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(moduleId);
    sndPid  = NAS_MULTIINSTANCE_GetSpecModemPid(modemId, WUEPS_PID_TAF);
    rcvPid  = NAS_MULTIINSTANCE_GetSpecModemPid(modemId, WUEPS_PID_MMA);

    /* 申请消息包TAF_MMA_StopSrvacqNotify */
    msg = (TAF_MMA_StopSrvacqNotify *)TAF_AllocMsgWithHeaderLen(sndPid, sizeof(TAF_MMA_StopSrvacqNotify));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, sndPid, rcvPid, sizeof(TAF_MMA_StopSrvacqNotify) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = ID_TAF_MMA_STOP_SRVACQ_NOTIFY;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(sndPid, msg);
}

MODULE_EXPORTED VOS_VOID TAF_MMA_SndStopSrvAcqReq(VOS_UINT32 moduleId)
{
    TAF_MMA_StopSrvAcqReq *msg = VOS_NULL_PTR;
    ModemIdUint16          modemId;
    VOS_UINT32             sndPid;
    VOS_UINT32             rcvPid;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(moduleId);
    sndPid  = NAS_MULTIINSTANCE_GetSpecModemPid(modemId, WUEPS_PID_TAF);
    rcvPid  = NAS_MULTIINSTANCE_GetSpecModemPid(modemId, WUEPS_PID_MMA);

    /* 申请消息包TAF_MMA_StopSrvacqNotify */
    msg = (TAF_MMA_StopSrvAcqReq *)TAF_AllocMsgWithHeaderLen(sndPid, sizeof(TAF_MMA_StopSrvAcqReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    NAS_CfgMsgHdr((MsgBlock *)msg, sndPid, rcvPid, (sizeof(TAF_MMA_StopSrvAcqReq) - VOS_MSG_HEAD_LENGTH));

    msg->msgName = ID_TAF_MMA_STOP_SRV_ACQ_REQ;
    msg->ctrl.moduleId = moduleId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(sndPid, msg);
}

#endif

VOS_UINT32 TAF_MMA_SetQuitCallBack(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_QuitCallbackSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MMA_QUIT_CALLBACK_SET_REQ */
    msg = (TAF_MMA_QuitCallbackSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_QuitCallbackSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_QuitCallbackSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_QUIT_CALLBACK_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 TAF_MMA_ProcCFreqLockSetReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                       TAF_MMA_CfreqLockSetPara *cFreqLockPara)
{
    errno_t                  memResult;
    TAF_MMA_CfreqLockSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (cFreqLockPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_CfreqLockSetReq */
    msg = (TAF_MMA_CfreqLockSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CfreqLockSetReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CfreqLockSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName  = ID_TAF_MMA_CDMA_FREQ_LOCK_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult     = memcpy_s(&msg->cFreqLockPara, sizeof(msg->cFreqLockPara), cFreqLockPara,
                             sizeof(TAF_MMA_CfreqLockSetPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->cFreqLockPara), sizeof(TAF_MMA_CfreqLockSetPara));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcCFreqLockQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CfreqLockQueryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CFREQ_LOCK_QRY_REQ_STRU */
    msg = (TAF_MMA_CfreqLockQueryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CfreqLockQueryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CfreqLockQueryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CDMA_FREQ_LOCK_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcCdmaCsqSetReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                     TAF_MMA_CdmacsqPara *cdmaCsqPara)
{
    errno_t                memResult;
    TAF_MMA_CdmacsqSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (cdmaCsqPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_CDMA_CSQ_SET_REQ_STRU */
    msg = (TAF_MMA_CdmacsqSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CdmacsqSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CdmacsqSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CDMACSQ_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult = memcpy_s(&(msg->cdmaCsqPara), sizeof(msg->cdmaCsqPara), cdmaCsqPara, sizeof(TAF_MMA_CdmacsqPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->cdmaCsqPara), sizeof(TAF_MMA_CdmacsqPara));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcCdmaCsqQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CdmacsqQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CdmacsqQryReq */
    msg = (TAF_MMA_CdmacsqQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CdmacsqQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CdmacsqQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CDMACSQ_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_Proc1xChanSetReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                    TAF_MMA_CfreqLockSetPara *cFreqLockPara)
{
    errno_t                  memResult;
    TAF_MMA_CfreqLockSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (cFreqLockPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_CfreqLockSetReq */
    msg = (TAF_MMA_CfreqLockSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CfreqLockSetReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CfreqLockSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName  = ID_TAF_MMA_1XCHAN_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult     = memcpy_s(&msg->cFreqLockPara, sizeof(msg->cFreqLockPara), cFreqLockPara,
                             sizeof(TAF_MMA_CfreqLockSetPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->cFreqLockPara), sizeof(TAF_MMA_CfreqLockSetPara));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_Proc1xChanQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_1XchanQueryReq *msg = VOS_NULL_PTR;
    VOS_UINT32              receiverPid;
    VOS_UINT32              senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MSG_MMA_CVER_QUERY_REQ */
    msg = (TAF_MMA_1XchanQueryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_1XchanQueryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_1XchanQueryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_1XCHAN_QUERY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcProRevInUseQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CverQueryReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MSG_MMA_CVER_QUERY_REQ */
    msg = (TAF_MMA_CverQueryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CverQueryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CverQueryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CVER_QUERY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcStateQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_StateQueryReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MSG_MMA_CVER_QUERY_REQ */
    msg = (TAF_MMA_StateQueryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_StateQueryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_StateQueryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_GETSTA_QUERY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcCHVerQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_ChighverQueryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MSG_MMA_CVER_QUERY_REQ */
    msg = (TAF_MMA_ChighverQueryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ChighverQueryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ChighverQueryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CHIGHVER_QUERY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/* 移出CDMA编译开关 */
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 TAF_MMA_SetCSidList(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                               TAF_MMA_OperLockTrustSid *trustSidList)
{
    errno_t                 memResult;
    TAF_MMA_CsidlistSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32              receiverPid;
    VOS_UINT32              senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MSG_MMA_CVER_QUERY_REQ */
    msg = (TAF_MMA_CsidlistSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CsidlistSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CsidlistSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CSIDLIST_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult = memcpy_s(&msg->trustSidInfo, sizeof(msg->trustSidInfo), trustSidList, sizeof(TAF_MMA_OperLockTrustSid));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->trustSidInfo), sizeof(TAF_MMA_OperLockTrustSid));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#endif

/* 移出CDMA编译开关 */

VOS_UINT32 TAF_MMA_ProcHdrCsqSetReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                    TAF_MMA_HdrCsqPara *hdrCsqPara)
{
    errno_t               memResult;
    TAF_MMA_HdrCsqSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CDMA_CSQ_SET_REQ_STRU */
    msg = (TAF_MMA_HdrCsqSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_HdrCsqSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_HdrCsqSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_HDR_CSQ_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult = memcpy_s(&(msg->hdrCsqSetting), sizeof(msg->hdrCsqSetting), hdrCsqPara, sizeof(TAF_MMA_HdrCsqPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->hdrCsqSetting), sizeof(TAF_MMA_HdrCsqPara));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcHdrCsqQryReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_HdrCsqQrySettingReq *msg = VOS_NULL_PTR;
    VOS_UINT32                   receiverPid;
    VOS_UINT32                   senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CdmacsqQryReq */
    msg = (TAF_MMA_HdrCsqQrySettingReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_HdrCsqQrySettingReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_HdrCsqQrySettingReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_HDR_CSQ_QRY_SETTING_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCurrSidNid(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CurrSidNidQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_CurrSidNidQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CurrSidNidQryReq));

    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CurrSidNidQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CURR_SID_NID_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCtRoamInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CtccRoamingNwInfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                       receiverPid;
    VOS_UINT32                       senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_CtccRoamingNwInfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                          sizeof(TAF_MMA_CtccRoamingNwInfoQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_CtccRoamingNwInfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CTCC_ROAMING_NW_INFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetCtOosCount(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT32 clOosCount,
                                 VOS_UINT32 gulOosCount)
{
    TAF_MMA_CtccOosCountSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MMA_QUIT_CALLBACK_SET_REQ */
    msg = (TAF_MMA_CtccOosCountSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CtccOosCountSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CtccOosCountSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CTCC_OOS_COUNT_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->clOosCount    = (VOS_UINT16)clOosCount;
    msg->gulOosCount   = (VOS_UINT16)gulOosCount;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetCtRoamInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT8 ctRoamRtpFlag)
{
    TAF_MMA_CtccRoamingNwInfoRtpCfgSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                             receiverPid;
    VOS_UINT32                             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包ID_TAF_MMA_CTCC_ROAMING_NW_INFO_RTP_CFG_SET_REQ */
    msg = (TAF_MMA_CtccRoamingNwInfoRtpCfgSetReq *)TAF_AllocMsgWithHeaderLen(
        senderPid, sizeof(TAF_MMA_CtccRoamingNwInfoRtpCfgSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_CtccRoamingNwInfoRtpCfgSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CTCC_ROAMING_NW_INFO_RTP_CFG_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->ctRoamRtpFlag = ctRoamRtpFlag;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryPrlIdInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_PrlidQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32           receiverPid;
    VOS_UINT32           senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_PrlidQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PrlidQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PrlidQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PRLID_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryRatCombinedMode(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_RatCombinedModeQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                     receiverPid;
    VOS_UINT32                     senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_RatCombinedModeQryReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                        sizeof(TAF_MMA_RatCombinedModeQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_RatCombinedModeQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_RAT_COMBINED_MODE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_ProcResetNtf(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT8 resetStep)
{
    TAF_MMA_ResetNtf *msg = VOS_NULL_PTR;
    VOS_UINT32        receiverPid;
    VOS_UINT32        senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CfreqLockSetReq */
    msg = (TAF_MMA_ResetNtf *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_ResetNtf));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ResetNtf) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_RESET_NTF;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->resetStep     = resetStep;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCLocInfoReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_CdmaLocinfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_CdmaLocinfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CdmaLocinfoQryReq));

    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CdmaLocinfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CDMA_LOCINFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#endif

VOS_UINT32 TAF_MMA_QryCurrEmcCallBackMode(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_EmcCallBackQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_EmcCallBackQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EmcCallBackQryReq));

    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EmcCallBackQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EMC_CALL_BACK_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetFPlmnInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                TAF_PH_FplmnOperate *cFPlmnPara)
{
    errno_t               memResult;
    TAF_MMA_CfplmnSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (cFPlmnPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_CDMA_CSQ_SET_REQ_STRU */
    msg = (TAF_MMA_CfplmnSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CfplmnSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CfplmnSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CFPLMN_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult          = memcpy_s(&(msg->cFPlmnPara), sizeof(msg->cFPlmnPara), cFPlmnPara, sizeof(TAF_PH_FplmnOperate));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->cFPlmnPara), sizeof(TAF_PH_FplmnOperate));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryFPlmnInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CfplmnQueryReq *msg = VOS_NULL_PTR;
    VOS_UINT32              receiverPid;
    VOS_UINT32              senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CdmacsqQryReq */
    msg = (TAF_MMA_CfplmnQueryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CfplmnQueryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CfplmnQueryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CFPLMN_QUERY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);
    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetCpolReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, TAF_PH_SetPrefplmn *prefPlmn)
{
    errno_t                 memResult;
    TAF_MMA_PrefPlmnSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32              receiverPid;
    VOS_UINT32              senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (prefPlmn == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_PrefPlmnSetReq */
    msg = (TAF_MMA_PrefPlmnSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PrefPlmnSetReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PrefPlmnSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PREF_PLMN_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult          = memcpy_s(&msg->prefPlmn, sizeof(msg->prefPlmn), prefPlmn, sizeof(TAF_PH_SetPrefplmn));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->prefPlmn), sizeof(TAF_PH_SetPrefplmn));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QueryCpolReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                TAF_MMA_CpolInfoQueryReq *cpolInfo)
{
    TAF_MMA_PrefPlmnQueryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;
    errno_t                   memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (cpolInfo == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_PrefPlmnQueryReq */
    msg = (TAF_MMA_PrefPlmnQueryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PrefPlmnQueryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PrefPlmnQueryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PREF_PLMN_QUERY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    memResult = memcpy_s(&msg->cpolInfo, sizeof(msg->cpolInfo), cpolInfo, sizeof(TAF_MMA_CpolInfoQueryReq));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->cpolInfo), sizeof(TAF_MMA_CpolInfoQueryReq));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_TestCpolReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                               MN_PH_PrefPlmnTypeUint8 prefPlmnType)
{
    TAF_MMA_PrefPlmnTestReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (prefPlmnType > MN_PH_PREF_PLMN_HPLMN) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_PrefPlmnTestReq */
    msg = (TAF_MMA_PrefPlmnTestReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PrefPlmnTestReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PrefPlmnTestReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PREF_PLMN_TEST_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->prefPlmnType  = prefPlmnType;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetCerssiReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                TAF_START_InfoInd *startInfoInd)
{
    TAF_MMA_CerssiSetReq *cerssiCfg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (startInfoInd == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_CDMA_CSQ_SET_REQ_STRU */
    cerssiCfg = (TAF_MMA_CerssiSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CerssiSetReq));

    /* 内存申请失败，返回 */
    if (cerssiCfg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)cerssiCfg, senderPid, receiverPid, sizeof(TAF_MMA_CerssiSetReq) - VOS_MSG_HEAD_LENGTH);
    cerssiCfg->msgName = ID_TAF_MMA_CERSSI_SET_REQ;

    cerssiCfg->ctrl.moduleId       = moduleId;
    cerssiCfg->ctrl.clientId       = clientId;
    cerssiCfg->ctrl.opId           = opId;
    cerssiCfg->actionType          = startInfoInd->actionType;
    cerssiCfg->rrcMsgType          = startInfoInd->rrcMsgType;
    cerssiCfg->minRptTimerInterval = startInfoInd->minRptTimerInterval;
    cerssiCfg->signThreshold       = startInfoInd->signThreshold;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, cerssiCfg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCerssiReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CerssiInfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_PhoneModeSetReq */
    msg = (TAF_MMA_CerssiInfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CerssiInfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CerssiInfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CERSSI_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    msg->nsaQryFlag = VOS_FALSE;
#endif

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 TAF_MMA_QryCserssiReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CerssiInfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_PhoneModeSetReq */
    msg = (TAF_MMA_CerssiInfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CerssiInfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CerssiInfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CERSSI_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->nsaQryFlag    = VOS_TRUE;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#endif

VOS_UINT32 TAF_MMA_SetCindReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, TAF_START_InfoInd *startInfoInd)
{
    TAF_MMA_CIND_SET_REQ_STRU *cindCfg = VOS_NULL_PTR;
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (startInfoInd == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 申请消息 */
    cindCfg = (TAF_MMA_CIND_SET_REQ_STRU *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CIND_SET_REQ_STRU));

    /* 内存申请失败，返回 */
    if (cindCfg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)cindCfg, senderPid, receiverPid, sizeof(TAF_MMA_CIND_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    cindCfg->msgName = ID_TAF_MMA_CIND_SET_REQ;

    cindCfg->ctrl.moduleId       = moduleId;
    cindCfg->ctrl.clientId       = clientId;
    cindCfg->ctrl.opId           = opId;
    cindCfg->actionType          = startInfoInd->actionType;
    cindCfg->rrcMsgType          = startInfoInd->rrcMsgType;
    cindCfg->minRptTimerInterval = startInfoInd->minRptTimerInterval;
    cindCfg->signThreshold       = startInfoInd->signThreshold;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, cindCfg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCrpnReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, TAF_MMA_CrpnQryPara *crpnQryReq)
{
    TAF_MMA_CrpnQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32          receiverPid;
    VOS_UINT32          senderPid;
    errno_t             memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CrpnQryReq */
    msg = (TAF_MMA_CrpnQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CrpnQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CrpnQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CRPN_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    if (crpnQryReq != VOS_NULL_PTR) {
        memResult = memcpy_s(&msg->crpnQryPara, sizeof(msg->crpnQryPara), crpnQryReq, sizeof(TAF_MMA_CrpnQryPara));
        TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->crpnQryPara), sizeof(TAF_MMA_CrpnQryPara));
    }

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetCmmReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, MM_TEST_AtCmdPara *testAtCmd)
{
    TAF_MMA_CmmSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32         receiverPid;
    VOS_UINT32         senderPid;
    errno_t            memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CmmSetReq */
    msg = (TAF_MMA_CmmSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CmmSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CmmSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CMM_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    if (testAtCmd != VOS_NULL_PTR) {
        memResult = memcpy_s(&msg->cmmSetReq, sizeof(msg->cmmSetReq), testAtCmd, sizeof(MM_TEST_AtCmdPara));
        TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->cmmSetReq), sizeof(MM_TEST_AtCmdPara));
    }

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryAcInfoReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_AcInfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_AcInfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_AcInfoQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_AcInfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_AC_INFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCopnInfoReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, VOS_UINT16 fromIndex,
                                  MN_OPERATION_ID_T opId)
{
    TAF_MMA_CopnInfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32              receiverPid;
    VOS_UINT32              senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /*
     * AT向MMA请求运营商信息:
     * 因为核间消息限制，不能一次获取所有运营商信息，这里定义为一次获取50条运营商信息
     * 第一条请求消息，从索引0开始要求连续的50条运营商信息
     */

    /* Allocating memory for message */
    msg = (TAF_MMA_CopnInfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CopnInfoQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CopnInfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_COPN_INFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->fromIndex     = fromIndex;
    msg->plmnNum       = TAF_MMA_COPN_PLMN_MAX_NUM;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 TAF_MMA_SimInsertReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                TAF_SIM_InsertStateUint32 simInsertState)
{
    TAF_MMA_SimInsertReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_SimInsertReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SimInsertReqMsg));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SimInsertReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName        = ID_TAF_MMA_SIM_INSERT_REQ;
    msg->ctrl.moduleId  = moduleId;
    msg->ctrl.clientId  = clientId;
    msg->ctrl.opId      = opId;
    msg->simInsertState = simInsertState;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#endif

VOS_UINT32 TAF_MMA_SetEOPlmnReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                TAF_MMA_SetEoplmnList *eOPlmnCfg)
{
    TAF_MMA_EoplmnSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;
    errno_t               memResult;

    if (eOPlmnCfg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_EoplmnSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EoplmnSetReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EoplmnSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EOPLMN_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    memResult = memcpy_s(&(msg->eOPlmnSetPara), sizeof(msg->eOPlmnSetPara), eOPlmnCfg, sizeof(TAF_MMA_SetEoplmnList));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->eOPlmnSetPara), sizeof(TAF_MMA_SetEoplmnList));

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryEOPlmnReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_EoplmnQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_EoplmnQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EoplmnQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EoplmnQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EOPLMN_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_NetScanReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                              TAF_MMA_NetScanSetPara *netScanSetPara)
{
    TAF_MMA_NetScanReqMsg *netScanReq = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    if (netScanSetPara == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    netScanReq = (TAF_MMA_NetScanReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_NetScanReqMsg));
    if (netScanReq == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)netScanReq, senderPid, receiverPid, sizeof(TAF_MMA_NetScanReqMsg) - VOS_MSG_HEAD_LENGTH);
    netScanReq->msgName       = ID_TAF_MMA_NET_SCAN_REQ;
    netScanReq->ctrl.moduleId = moduleId;
    netScanReq->ctrl.clientId = clientId;
    netScanReq->ctrl.opId     = opId;
    netScanReq->para          = *netScanSetPara;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, netScanReq);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_NetScanAbortReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_NetScanAbortReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_NetScanAbortReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_NetScanAbortReqMsg));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_NetScanAbortReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_NET_SCAN_ABORT_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QrySpnReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_SpnQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32         receiverPid;
    VOS_UINT32         senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_SpnQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SpnQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SpnQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_SPN_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryMMPlmnInfoReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_MmplmninfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_MmplmninfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_MmplmninfoQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_MmplmninfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_MMPLMNINFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryPlmnReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_LastCampPlmnQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_LastCampPlmnQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_LastCampPlmnQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_LastCampPlmnQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_LAST_CAMP_PLMN_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryUserSrvStateReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_UserSrvStateQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_UserSrvStateQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_UserSrvStateQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_UserSrvStateQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_USER_SRV_STATE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryApPwrOnAndRegTimeReq(VOS_UINT32 moduleId, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    TAF_MMA_PowerOnAndRegTimeQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                       receiverPid;
    VOS_UINT32                       senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* Allocating memory for message */
    msg = (TAF_MMA_PowerOnAndRegTimeQryReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                          sizeof(TAF_MMA_PowerOnAndRegTimeQryReq));
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_PowerOnAndRegTimeQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_POWER_ON_AND_REG_TIME_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetAutoAttachReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT32 setValue)
{
    TAF_MMA_AutoAttachSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_AutoAttachSetReq */
    msg = (TAF_MMA_AutoAttachSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_AutoAttachSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_AutoAttachSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName          = ID_TAF_MMA_AUTO_ATTACH_SET_REQ;
    msg->ctrl.moduleId    = moduleId;
    msg->ctrl.clientId    = clientId;
    msg->ctrl.opId        = opId;
    msg->autoAttachEnable = setValue;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_TestSysCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_SyscfgTestReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_SyscfgTestReq */
    msg = (TAF_MMA_SyscfgTestReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SyscfgTestReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SyscfgTestReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_SYSCFG_TEST_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);
    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryAccessModeReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_AccessModeQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_AccessModeQryReq */
    msg = (TAF_MMA_AccessModeQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_AccessModeQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_AccessModeQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_ACCESS_MODE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCopsInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CopsQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32          receiverPid;
    VOS_UINT32          senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CopsQryReq */
    msg = (TAF_MMA_CopsQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CopsQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CopsQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_COPS_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetEflociInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                 TAF_MMA_Eflociinfo *efLociInfo)
{
    TAF_MMA_EflociinfoSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;
    errno_t                   memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_EflociinfoSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EflociinfoSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EflociinfoSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EFLOCIINFO_SET_REQ;
    msg->ctrl.opId     = opId;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    memResult = memcpy_s(&(msg->eflociInfo), sizeof(TAF_MMA_Eflociinfo), efLociInfo, sizeof(TAF_MMA_Eflociinfo));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(TAF_MMA_Eflociinfo), sizeof(TAF_MMA_Eflociinfo));

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryEflociInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_EflociinfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_EflociinfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EflociinfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EflociinfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EFLOCIINFO_QRY_REQ;
    msg->ctrl.opId     = opId;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetPsEflociInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                   TAF_MMA_Efpslociinfo *psefLociInfo)
{
    TAF_MMA_EfpslociinfoSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;
    errno_t                     memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_EfpslociinfoSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EfpslociinfoSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EfpslociinfoSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EFPSLOCIINFO_SET_REQ;
    msg->ctrl.opId     = opId;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    memResult = memcpy_s(&(msg->psEflociInfo), sizeof(TAF_MMA_Efpslociinfo), psefLociInfo,
                         sizeof(TAF_MMA_Efpslociinfo));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(TAF_MMA_Efpslociinfo), sizeof(TAF_MMA_Efpslociinfo));

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryPsEflociInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_EfpslociinfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                  receiverPid;
    VOS_UINT32                  senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_EfpslociinfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EfpslociinfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EfpslociinfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EFPSLOCIINFO_QRY_REQ;
    msg->ctrl.opId     = opId;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryDplmnListReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_DplmnQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32           receiverPid;
    VOS_UINT32           senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_DplmnQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_DplmnQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_DplmnQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_DPLMN_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetDplmnListReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 seq, const VOS_UINT8 *version,
                                   TAF_MMA_DplmnInfoSet *dplmnInfo)
{
    TAF_MMA_DplmnSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32           receiverPid;
    VOS_UINT32           senderPid;
    errno_t              memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_DplmnSetReq *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_TAF, sizeof(TAF_MMA_DplmnSetReq));

    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 填充消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_DplmnSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_DPLMN_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = 0;

    /* 填充流水号、版本号 */
    msg->seq  = seq;
    memResult = memcpy_s(msg->versionId, sizeof(msg->versionId), version, TAF_MMA_VERSION_INFO_LEN);
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->versionId), TAF_MMA_VERSION_INFO_LEN);

    if (dplmnInfo->ehPlmnNum > TAF_MMA_MAX_EHPLMN_NUM) {
        dplmnInfo->ehPlmnNum = TAF_MMA_MAX_EHPLMN_NUM;
    }

    if (dplmnInfo->dplmnNum > TAF_MMA_MAX_DPLMN_NUM) {
        dplmnInfo->dplmnNum = TAF_MMA_MAX_DPLMN_NUM;
    }

    /* 填充HPLMN个数、HPLMN列表、预制DPLMN个数、DPLMN列表 */
    msg->dplmnInfo.ehPlmnNum = dplmnInfo->ehPlmnNum;
    msg->dplmnInfo.dplmnNum  = dplmnInfo->dplmnNum;
    if (dplmnInfo->ehPlmnNum > 0) {
        memResult = memcpy_s(msg->dplmnInfo.ehPlmnInfo, sizeof(msg->dplmnInfo.ehPlmnInfo), dplmnInfo->ehPlmnInfo,
                             sizeof(TAF_PLMN_Id) * dplmnInfo->ehPlmnNum);
        TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->dplmnInfo.ehPlmnInfo),
                                     sizeof(TAF_PLMN_Id) * dplmnInfo->ehPlmnNum);
    }
    if (dplmnInfo->dplmnNum > 0) {
        memResult = memcpy_s(msg->dplmnInfo.dplmnList, sizeof(msg->dplmnInfo.dplmnList), dplmnInfo->dplmnList,
                             sizeof(TAF_MMA_PlmnWithSimRat) * dplmnInfo->dplmnNum);
        TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->dplmnInfo.dplmnList),
                                     sizeof(TAF_MMA_PlmnWithSimRat) * dplmnInfo->dplmnNum);
    }
    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetBorderInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId,
                                    TAF_MMA_SetBorderInfoOperateTypeUint8 operateType, TAF_MMA_BorderInfo *borderInfo)
{
    TAF_MMA_BorderInfoSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;
    errno_t                   memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_BorderInfoSetReq *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_TAF, sizeof(TAF_MMA_BorderInfoSetReq) +
                                                                   borderInfo->borderInfoLen - 4);

    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 填充消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_BorderInfoSetReq) + borderInfo->borderInfoLen - 4 - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_BORDER_INFO_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = 0;

    /* 填充消息内容 */
    msg->operateType = operateType;

    memResult = memcpy_s(&(msg->borderInfo), sizeof(TAF_MMA_BorderInfo) + borderInfo->borderInfoLen - 4, borderInfo,
                         sizeof(TAF_MMA_BorderInfo) + borderInfo->borderInfoLen - 4);
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(TAF_MMA_BorderInfo) + borderInfo->borderInfoLen - 4,
                                 sizeof(TAF_MMA_BorderInfo) + borderInfo->borderInfoLen - 4);

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryBorderInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_BorderInfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_BorderInfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_BorderInfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_BorderInfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_BORDER_INFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#if (FEATURE_DSDS == FEATURE_ON)

VOS_UINT32 TAF_MMA_SetDsdsStateReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT32 enable)
{
    TAF_MMA_DsdsStateSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_DsdsStateSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_DsdsStateSetReq));

    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 填充消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_DsdsStateSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_DSDS_STATE_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = 0;

    /* 填充消息内容 */
    msg->enable = (VOS_UINT8)enable;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#endif

VOS_UINT32 TAF_MMA_QryRegStateReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                  TAF_MMA_QryRegStatusTypeUint32 regStaType)
{
    TAF_MMA_RegStateQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32              receiverPid;
    VOS_UINT32              senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_RegStateQryReq */
    msg = (TAF_MMA_RegStateQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_RegStateQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_RegStateQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_REG_STATE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    msg->qryRegStaType = regStaType;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryAutoAttachInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_AutoAttachQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_AUTOATTACH_QRY_REQ_STRU */
    msg = (TAF_MMA_AutoAttachQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_AutoAttachQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_AutoAttachQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_AUTO_ATTACH_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QrySystemInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT32 sysInfoExFlag)
{
    TAF_MMA_SysinfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_SysinfoQryReq */
    msg = (TAF_MMA_SysinfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SysinfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SysinfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_SYSINFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->sysInfoExFlag = sysInfoExFlag;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
/* QryAntennaInfoReq 移至MTA处理 */

VOS_UINT32 TAF_MMA_QryApHplmnInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_HomePlmnQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32              receiverPid;
    VOS_UINT32              senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_APHPLMN_QRY_REQ_STRU */
    msg = (TAF_MMA_HomePlmnQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_HomePlmnQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_HomePlmnQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_HOME_PLMN_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

/* QryCsnrReq移至MTA处理 */

VOS_UINT32 TAF_MMA_QryCsqReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CsqQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32         receiverPid;
    VOS_UINT32         senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CsqQryReq */
    msg = (TAF_MMA_CsqQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CsqQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CsqQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CSQ_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

/* QryCsqlvlReq移至MTA处理 */

VOS_UINT32 TAF_MMA_QryBatteryCapacityReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_BatteryCapacityQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                     receiverPid;
    VOS_UINT32                     senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_CBC_QRY_REQ_STRU */
    msg = (TAF_MMA_BatteryCapacityQryReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                        sizeof(TAF_MMA_BatteryCapacityQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_BatteryCapacityQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_BATTERY_CAPACITY_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryHandShakeReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_HandShakeQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_HS_QRY_REQ_STRU */
    msg = (TAF_MMA_HandShakeQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_HandShakeQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_HandShakeQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_HAND_SHAKE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryPacspReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_PacspQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32           receiverPid;
    VOS_UINT32           senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_PacspQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PacspQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PacspQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PACSP_QRY_REQ;
    msg->ctrl.opId     = opId;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_VOID TAF_MMA_CardQuickSwitchNotify(VOS_UINT32 moduleId, VOS_UINT16 clientId,
    VOS_UINT8 opId, TAF_MMA_IsSamePlmnFlagUint8 isSamePlmnInfo)
{
    TAF_MMA_QuickCardSwithIndMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                    receiverPid;
    VOS_UINT32                    senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_QuickCardSwithIndMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_QuickCardSwithIndMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_QuickCardSwithIndMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_QUICK_CARD_SWITCH_IND;
    msg->ctrl.opId     = opId;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->isSamePlmn    = isSamePlmnInfo;

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);
}

#if (FEATURE_LTE == FEATURE_ON)

/*lint -save -e838 -specific(-e838)*/
VOS_UINT32 TAF_MMA_AcdcAppNotify(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                 TAF_MMA_AcdcAppInfo *acdcAppInfo)
{
    TAF_MMA_AcdcAppNotifyMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;
    errno_t                   memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_AcdcAppNotifyMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_AcdcAppNotifyMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_AcdcAppNotifyMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_ACDC_APP_NOTIFY;
    msg->ctrl.opId     = opId;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    memResult = memcpy_s(&(msg->acdcAppInfo), sizeof(TAF_MMA_AcdcAppInfo), acdcAppInfo, sizeof(TAF_MMA_AcdcAppInfo));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(TAF_MMA_AcdcAppInfo), sizeof(TAF_MMA_AcdcAppInfo));

    /* 消息发送 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
/*lint -restore*/
#endif

MODULE_EXPORTED VOS_VOID TAF_MMA_SndRestartReq(VOS_UINT32 moduleId, ModemIdUint16 modemId)
{
    TAF_MMA_RestartReq *msg = VOS_NULL_PTR;
    VOS_UINT32          senderPid;
    VOS_UINT32          receiverPid;

    if (modemId == MODEM_ID_0) {
        senderPid   = I0_WUEPS_PID_TAF;
        receiverPid = I0_WUEPS_PID_MMA;
    } else if (modemId == MODEM_ID_1) {
        senderPid   = I1_WUEPS_PID_TAF;
        receiverPid = I1_WUEPS_PID_MMA;
    } else if (modemId == MODEM_ID_2) {
        senderPid   = I2_WUEPS_PID_TAF;
        receiverPid = I2_WUEPS_PID_MMA;
    } else {
        return;
    }

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    senderPid = ACPU_PID_TAF;
#endif

    msg = (TAF_MMA_RestartReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_RestartReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_RestartReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName  = ID_TAF_MMA_RESTART_REQ;
    msg->moduleId = moduleId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);
}

#if (FEATURE_MULTI_MODEM == FEATURE_ON)

VOS_UINT32 TAF_MMA_ExchangeModemInfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, ModemIdUint16 firstModemId,
                                        ModemIdUint16 secondModemId)
{
    TAF_MMA_ExchangeModemInfoReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                       receiverPid;
    VOS_UINT32                       senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包 */
    msg = (TAF_MMA_ExchangeModemInfoReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                          sizeof(TAF_MMA_ExchangeModemInfoReqMsg));

    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 填充消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid,
                  sizeof(TAF_MMA_ExchangeModemInfoReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EXCHANGE_MODEM_INFO_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = 0;
    msg->firstModemId  = firstModemId;
    msg->secondModemId = secondModemId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#endif

VOS_UINT32 TAF_MMA_PlmnSearchReq(TAF_MMA_Ctrl *ctrl, TAF_MMA_SearchTypeUint32 searchType, TAF_PLMN_UserSel *plmnUserSel)
{
    TAF_MMA_PlmnSearchReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;
    errno_t                   memResult;

    receiverPid = TAF_GET_DEST_PID(ctrl->clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(ctrl->clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_PLMN_AUTO_RESEL_REQ_STRU */
    msg = (TAF_MMA_PlmnSearchReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PlmnSearchReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PlmnSearchReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName    = ID_TAF_MMA_PLMN_SEARCH_REQ;
    msg->searchType = searchType;

    /* 控制信息 */
    memResult = memcpy_s(&msg->ctrl, sizeof(msg->ctrl), ctrl, sizeof(TAF_MMA_Ctrl));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->ctrl), sizeof(TAF_MMA_Ctrl));

    memResult = memcpy_s(&msg->plmnUserSel, sizeof(msg->plmnUserSel), plmnUserSel, sizeof(TAF_PLMN_UserSel));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->plmnUserSel), sizeof(TAF_PLMN_UserSel));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_PlmnDetectReq(TAF_MMA_Ctrl *ctrl, TAF_DETECT_Plmn *plmnDetect)
{
    TAF_MMA_PlmnDetectReqMsg *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;
    errno_t                   memResult;

    receiverPid = TAF_GET_DEST_PID(ctrl->clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(ctrl->clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_PLMN_AUTO_RESEL_REQ_STRU */
    msg = (TAF_MMA_PlmnDetectReqMsg *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PlmnDetectReqMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PlmnDetectReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = ID_TAF_MMA_PLMN_DETECT_NTF;

    /* 控制信息 */
    memResult = memcpy_s(&msg->ctrl, sizeof(msg->ctrl), ctrl, sizeof(TAF_MMA_Ctrl));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->ctrl), sizeof(TAF_MMA_Ctrl));

    memResult = memcpy_s(&msg->plmnDetect, sizeof(msg->plmnDetect), plmnDetect, sizeof(TAF_DETECT_Plmn));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->plmnDetect), sizeof(TAF_DETECT_Plmn));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

VOS_VOID TAF_MMA_SendSmcNoEntityNtf(VOS_VOID)
{
    TAF_MMA_SmcNoEntityNotify *msg = VOS_NULL_PTR;

    ModemIdUint16 modemId;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(WUEPS_PID_TAF);

    msg = (TAF_MMA_SmcNoEntityNotify *)NAS_MULTIINSTANCE_AllocMsgWithHdr(modemId, WUEPS_PID_TAF,
                                                                         sizeof(TAF_MMA_SmcNoEntityNotify));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_TAF, WUEPS_PID_MMA,
                  sizeof(TAF_MMA_SmcNoEntityNotify) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = ID_TAF_MMA_SMC_NO_ENTITY_NOTIFY;

    /* 发送消息 */
    (VOS_VOID)NAS_MULTIINSTANCE_SendMsg(modemId, msg);
}

#if (FEATURE_IMS == FEATURE_ON)

VOS_VOID TAF_MMA_ImsRegDomainNotify(TAF_MMA_ImsRegDomainUint8 imsRegDomain, TAF_MMA_ImsRegStatusUint8 imsRegStatus)
{
    TAF_MMA_ImsRegDomainNotifyMsg *msg = VOS_NULL_PTR;
    ModemIdUint16                  modemId;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_SrvAcqReq */
    msg = (TAF_MMA_ImsRegDomainNotifyMsg *)NAS_MULTIINSTANCE_AllocMsgWithHdr(modemId, WUEPS_PID_TAF,
                                                                             sizeof(TAF_MMA_ImsRegDomainNotifyMsg));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_TAF, WUEPS_PID_MMA,
                  sizeof(TAF_MMA_ImsRegDomainNotifyMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName      = ID_TAF_MMA_IMS_REG_DOMAIN_NOTIFY;
    msg->imsRegDomain = imsRegDomain;

    msg->imsRegStatus = imsRegStatus;

    /* 发送消息 */
    if (NAS_MULTIINSTANCE_SendMsg(modemId, msg) != VOS_OK) {
        return;
    }
}

VOS_VOID TAF_MMA_VoimsEmc380FailNotify(VOS_VOID)
{
    TAF_MMA_VoimsEmc380FailNtf *msg = VOS_NULL_PTR;
    ModemIdUint16               modemId;

    modemId = NAS_MULTIINSTANCE_GetCurrInstanceModemId(WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_VoimsEmc380FailNtf */
    msg = (TAF_MMA_VoimsEmc380FailNtf *)NAS_MULTIINSTANCE_AllocMsgWithHdr(modemId, WUEPS_PID_TAF,
                                                                          sizeof(TAF_MMA_VoimsEmc380FailNtf));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_TAF, WUEPS_PID_MMA,
                  sizeof(TAF_MMA_VoimsEmc380FailNtf) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = ID_TAF_MMA_VOIMS_EMC_380_FAIL_NTF;

    /* 发送消息 */
    (VOS_VOID)NAS_MULTIINSTANCE_SendMsg(modemId, msg);
}

#endif

#endif

VOS_UINT32 TAF_MMA_SetCemodeReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT32 setValue)
{
    TAF_MMA_CemodeSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_SysCfgReq */
    msg = (TAF_MMA_CemodeSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CemodeSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充 TAF_MMA_CemodeSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CemodeSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CEMODE_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->ceMode        = (VOS_UINT8)setValue;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryCemodeReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CemodeQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_SysCfgReq */
    msg = (TAF_MMA_CemodeQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CemodeQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充 TAF_MMA_CemodeQryReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CemodeQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CEMODE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryRejinfoReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_RejinfoQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_RejinfoQryReq */
    msg = (TAF_MMA_RejinfoQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_RejinfoQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_RejinfoQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_REJINFO_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetEmRssiCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                   TAF_MMA_EmrssicfgReq *psEmRssiCfgPara)
{
    TAF_MMA_EmrssicfgSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;
    errno_t                  memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_EmrssicfgSetReq */
    msg = (TAF_MMA_EmrssicfgSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EmrssicfgSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充TAF_MMA_EmrssicfgSetReq */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EmrssicfgSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EMRSSICFG_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    memResult = memcpy_s(&(msg->emRssiCfg), sizeof(msg->emRssiCfg), psEmRssiCfgPara, sizeof(TAF_MMA_EmrssicfgReq));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->emRssiCfg), sizeof(TAF_MMA_EmrssicfgReq));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryEmRssiCfgReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_EmrssicfgQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_EmrssicfgQryReq */
    msg = (TAF_MMA_EmrssicfgQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EmrssicfgQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EmrssicfgQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EMRSSICFG_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetEmRssiRptReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
    const VOS_UINT8 *emRssiRptSwitch)
{
    TAF_MMA_EmrssirptSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_EMRSSIPRT_SET_REQ_STRU */
    msg = (TAF_MMA_EmrssirptSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EmrssirptSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充ID_TAF_MMA_EMRSSIRPT_SET_REQ */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EmrssirptSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName         = ID_TAF_MMA_EMRSSIRPT_SET_REQ;
    msg->ctrl.moduleId   = moduleId;
    msg->ctrl.clientId   = clientId;
    msg->ctrl.opId       = opId;
    msg->emRssiRptSwitch = *emRssiRptSwitch;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryEmRssiRptReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_EmrssirptQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_EmrssicfgQryReq */
    msg = (TAF_MMA_EmrssirptQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_EmrssirptQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_EmrssirptQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_EMRSSIRPT_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SetUlFreqRptReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT8 mode)
{
    TAF_MMA_UlfreqrptSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_UlfreqrptSetReq */
    msg = (TAF_MMA_UlfreqrptSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_UlfreqrptSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 根据输入参数填充ID_TAF_MMA_ULFREQRPT_SET_REQ */
    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_UlfreqrptSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_ULFREQRPT_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->mode          = mode;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryUlFreqRptReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_UlfreqrptQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32               receiverPid;
    VOS_UINT32               senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_UlfreqrptQryReq */
    msg = (TAF_MMA_UlfreqrptQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_UlfreqrptQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_UlfreqrptQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_ULFREQRPT_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 TAF_MMA_SetClDbDomainStatus(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT8 mode)
{
    TAF_MMA_ClDbdomainStatusSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                      receiverPid;
    VOS_UINT32                      senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 参数检查 */
    if (mode > 1) {
        return VOS_FALSE;
    }

    /* 申请消息包TAF_MMA_ClDbdomainStatusSetReq */
    msg = (TAF_MMA_ClDbdomainStatusSetReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                         sizeof(TAF_MMA_ClDbdomainStatusSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_ClDbdomainStatusSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CL_DBDOMAIN_STATUSE_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    msg->enableFlag    = mode;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}
#endif

#if (FEATURE_MULTI_MODEM == FEATURE_ON)

VOS_UINT32 TAF_MMA_SetPsSceneReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                 TAF_MMA_PsScenePara *psSrvStatePara)
{
    TAF_MMA_PsSceneSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;
    errno_t                memResult;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);
    msg         = (TAF_MMA_PsSceneSetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PsSceneSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PsSceneSetReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PS_SCENE_SET_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;
    memResult = memcpy_s(&(msg->psStatePara), sizeof(msg->psStatePara), psSrvStatePara, sizeof(TAF_MMA_PsScenePara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->psStatePara), sizeof(TAF_MMA_PsScenePara));

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryPsSceneReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_PsSceneQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);
    msg         = (TAF_MMA_PsSceneQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_PsSceneQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_PsSceneQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_PS_SCENE_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

VOS_UINT32 TAF_MMA_QryOosGearsReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_CarOosGearsQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                receiverPid;
    VOS_UINT32                senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);
    msg         = (TAF_MMA_CarOosGearsQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_CarOosGearsQryReq));
    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_CarOosGearsQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_CAR_OOS_GEARS_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_SndCarOosGearsSetNtf(TAF_MMA_Ctrl *ctrl, TAF_MMA_SetCarOosGearsNtf *carGearsNtf)
{
    TAF_MMA_SetCarOosGearsNtf            *msg = VOS_NULL_PTR;
    VOS_UINT32                            receiverPid;
    VOS_UINT32                            senderPid;
    errno_t                               memResult;

    receiverPid = TAF_GET_DEST_PID(ctrl->clientId, WUEPS_PID_MMA);
    senderPid    = TAF_GET_SEND_PID(ctrl->clientId, WUEPS_PID_TAF);

    msg = (TAF_MMA_SetCarOosGearsNtf *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SetCarOosGearsNtf));
    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SetCarOosGearsNtf) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = ID_TAF_MMA_SET_CAR_OOS_GEARS_NTF;

    memResult = memcpy_s(&msg->ctrl, sizeof(msg->ctrl), ctrl, sizeof(TAF_MMA_Ctrl));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->ctrl), sizeof(TAF_MMA_Ctrl));

    msg->carGears = carGearsNtf->carGears;

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryRrcStatReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_RrcstatQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;
    VOS_UINT32             senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    /* 申请消息包TAF_MMA_RrcstatQryReq */
    msg = (TAF_MMA_RrcstatQryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_RrcstatQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_RrcstatQryReq) - VOS_MSG_HEAD_LENGTH);
    msg->msgName       = ID_TAF_MMA_RRCSTAT_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

#if (FEATURE_ON == FEATURE_UE_MODE_NR)

VOS_UINT32 TAF_MMA_SetNrBandBlockListReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                                         TAF_MMA_NrBandBlockListSetPara *nrBandBlockListSetStru)
{
    TAF_MMA_NrBandBlockListSetReq *msg = VOS_NULL_PTR;
    VOS_UINT32                     receiverPid;
    VOS_UINT32                     senderPid;

    /* 参数检查 */
    if (nrBandBlockListSetStru == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);
    msg         = (TAF_MMA_NrBandBlockListSetReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                                sizeof(TAF_MMA_NrBandBlockListSetReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_NrBandBlockListSetReq) - VOS_MSG_HEAD_LENGTH);

    msg->msgName             = ID_TAF_MMA_NR_BAND_BLOCK_LIST_SET_REQ;
    msg->ctrl.moduleId       = moduleId;
    msg->ctrl.clientId       = clientId;
    msg->ctrl.opId           = opId;
    msg->nrBandBlocklistPara = *nrBandBlockListSetStru;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 TAF_MMA_QryNrBandBlockListReq(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_MMA_NrBandBlockListQryReq *msg = VOS_NULL_PTR;
    VOS_UINT32                     receiverPid;
    VOS_UINT32                     senderPid;

    receiverPid = TAF_GET_DEST_PID(clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(clientId, WUEPS_PID_TAF);

    msg = (TAF_MMA_NrBandBlockListQryReq *)TAF_AllocMsgWithHeaderLen(senderPid,
                                                                        sizeof(TAF_MMA_NrBandBlockListQryReq));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_NrBandBlockListQryReq) - VOS_MSG_HEAD_LENGTH);

    msg->msgName       = ID_TAF_MMA_NR_BAND_BLOCK_LIST_QRY_REQ;
    msg->ctrl.moduleId = moduleId;
    msg->ctrl.clientId = clientId;
    msg->ctrl.opId     = opId;

    /* 发送消息 */
    if (TAF_TraceAndSendMsg(senderPid, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
#endif

VOS_UINT32 TAF_MMA_SndOosSrchStrategySetInd(TAF_MMA_Ctrl *ctrl,
    TAF_MMA_SetOosSrchStrategyPara *srchStrategy)
{
    TAF_MMA_SetOosSrchStrategyInd      *msg = VOS_NULL_PTR;
    VOS_UINT32                          receiverPid;
    VOS_UINT32                          senderPid;
    errno_t                             memResult;

    receiverPid = TAF_GET_DEST_PID(ctrl->clientId, WUEPS_PID_MMA);
    senderPid   = TAF_GET_SEND_PID(ctrl->clientId, WUEPS_PID_TAF);

    msg = (TAF_MMA_SetOosSrchStrategyInd *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MMA_SetOosSrchStrategyInd));

    /* 内存申请失败，返回 */
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, senderPid, receiverPid, sizeof(TAF_MMA_SetOosSrchStrategyInd) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = ID_TAF_MMA_SET_OOS_SRCH_STRATEGY_IND;

    memResult = memcpy_s(&msg->ctrl, sizeof(msg->ctrl), ctrl, sizeof(TAF_MMA_Ctrl));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->ctrl), sizeof(TAF_MMA_Ctrl));

    memResult = memcpy_s(&msg->setPara, sizeof(msg->setPara),
        srchStrategy, sizeof(TAF_MMA_SetOosSrchStrategyPara));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->setPara), sizeof(TAF_MMA_SetOosSrchStrategyPara));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(senderPid, msg);

    return VOS_TRUE;
}

