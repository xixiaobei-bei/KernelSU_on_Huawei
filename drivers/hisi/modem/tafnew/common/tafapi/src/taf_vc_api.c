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

#include "app_vc_api.h"
#include "vos.h"
#include "ps_common_def.h"
#include "at_cmd_proc.h"
#include "mn_comm_api.h"
#include "securec.h"


#define THIS_FILE_ID PS_FILE_ID_VC_API_C

VOS_UINT32 APP_VC_SetVoiceVolume(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT8 voiceVolume)
{
    /* 构造消息发送消息给VC模块 */
    VOS_UINT32     rslt;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetVoiceVolume: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 初始化消息 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId = clientId;
    msg->opId     = opId;
    msg->msgName  = APP_VC_MSG_REQ_SET_VOLUME;

    msg->content[0] = voiceVolume;
    msg->content[1] = 0;
    msg->content[2] = 0;
    msg->content[3] = 0;

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetVoiceVolume: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_SetVoiceMode(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT8 voiceMode, VOS_UINT32 rcvPid)
{
    /* 构造消息发送消息给VC模块 */
    VOS_UINT32     rslt;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetVoiceMode: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 初始化消息 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, rcvPid, sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId = clientId;
    msg->opId     = opId;
    msg->msgName  = APP_VC_MSG_REQ_SET_MODE;

    msg->content[0] = voiceMode;
    msg->content[1] = 0;
    msg->content[2] = 0;
    msg->content[3] = 0;

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetVoiceMode: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_GetVoiceMode(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 构造消息发送消息给VC模块 */
    VOS_UINT32     rslt;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_GetVoiceMode: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 初始化消息 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId = clientId;
    msg->opId     = opId;
    msg->msgName  = APP_VC_MSG_REQ_QRY_MODE;

    msg->content[0] = 0;
    msg->content[1] = 0;
    msg->content[2] = 0;
    msg->content[3] = 0;

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("APP_VC_GetVoiceMode: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_GetVoiceVolume(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 构造消息发送消息给VC模块 */
    VOS_UINT32     rslt;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_GetVoiceVolume: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 初始化消息 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId = clientId;
    msg->opId     = opId;
    msg->msgName  = APP_VC_MSG_REQ_QRY_VOLUME;

    msg->content[0] = 0;
    msg->content[1] = 0;
    msg->content[2] = 0;
    msg->content[3] = 0;

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("APP_VC_GetVoiceVolume: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_SetMuteStatus(VOS_UINT16 clientId, VOS_UINT8 opId, APP_VC_MuteStatusUint8 muteStatus)
{
    VOS_UINT32     rslt;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    /* 构造消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetMuteStatus: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_SET_MUTE_STATUS_REQ;

    /* 填写消息内容 */
    msg->clientId   = clientId;
    msg->opId       = opId;
    msg->content[0] = muteStatus;

    /* 发送消息 */
    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetMuteStatus: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_GetMuteStatus(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    VOS_UINT32     rslt;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    /* 构造消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetMuteStatus: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_GET_MUTE_STATUS_REQ;

    /* 填写消息内容 */
    msg->clientId = clientId;
    msg->opId     = opId;

    /* 发送消息 */
    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetMuteStatus: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_SetModemLoop(VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT8 modemLoop)
{
    VOS_UINT32     rslt;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    /* 构造消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetModemLoop: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_SET_MODEMLOOP_REQ;

    /* 填写消息内容 */
    msg->clientId   = clientId;
    msg->opId       = opId;
    msg->content[0] = modemLoop;

    /* 发送消息 */
    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetModemLoop: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_SendHifiResetNtf(VOS_UINT32 rcvPid, APP_VC_HifiResetTypeUint8 hifiResetType,
    APP_VC_MsgUint16 msgName)
{
    APP_VC_ReqMsg              *msg = VOS_NULL_PTR;
    struct APP_VC_HifiResetInfo hifiResetMsg;
    errno_t                     memResult;

    (VOS_VOID)memset_s(&hifiResetMsg, sizeof(hifiResetMsg), 0x00, sizeof(hifiResetMsg));

    /* 构造消息 */
    msg = (APP_VC_ReqMsg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SendHifiResetNtf: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    /* 填写消息头 */
    (VOS_VOID)memset_s(msg, sizeof(APP_VC_ReqMsg), 0x00, sizeof(APP_VC_ReqMsg));
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, rcvPid, sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = msgName;

    /* 填写消息内容 */
    msg->clientId = 0;
    msg->opId     = 0;
    hifiResetMsg.hifiResetType = hifiResetType;

    memResult = memcpy_s(msg->content, sizeof(msg->content), &hifiResetMsg, sizeof(hifiResetMsg));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(msg->content), sizeof(hifiResetMsg));

    /* 发送消息 */
    TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);

    return VOS_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 APP_VC_SetSpeakerMuteStatus(VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT8 muteStatus)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetSpeakerMuteStatus: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
        sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_SET_SMUT_STATUS_REQ;
    msg->clientId = clientId;
    msg->opId = opId;
    msg->content[0] = muteStatus;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetSpeakerMuteStatus: SEND MSG FAIL.");
        return VOS_ERR;
    }
    return VOS_OK;
}

VOS_UINT32 APP_VC_GetSpeakerMuteStatus(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_GetSpeakerMuteStatus: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
        sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_GET_SMUT_STATUS_REQ;
    msg->clientId = clientId;
    msg->opId = opId;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_GetSpeakerMuteStatus: SEND MSG FAIL.");
        return VOS_ERR;
    }
    return VOS_OK;
}

VOS_UINT32 APP_VC_UpdateSpeakerMuteStatus(VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT8 mute)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_UpdateSpeakerMuteStatus: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
        sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_UPDATE_SMUT_STATUS_REQ;
    msg->clientId = clientId;
    msg->opId = opId;
    msg->content[0] = mute;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_UpdateSpeakerMuteStatus: SEND MSG FAIL.");
        return VOS_ERR;
    }
    return VOS_OK;
}

VOS_UINT32 APP_VC_SetUpVoiceVolume(TAF_UINT16 clientId, TAF_UINT8 opId, VOS_UINT8 voiceVolume)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetUpVoiceVolume: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->clientId = clientId;
    msg->opId = opId;
    msg->msgName = APP_VC_MSG_SET_CMIC_STATUS_REQ;

    msg->content[0] = voiceVolume;
    msg->content[1] = 0;
    msg->content[2] = 0;
    msg->content[3] = 0;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
         AT_ERR_LOG("APP_VC_SetUpVoiceVolume: SEND MSG FAIL.");
         return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_GetUpVoiceVolume(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_GetUpVoiceVolume: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_GET_CMIC_STATUS_REQ;
    msg->clientId = clientId;
    msg->opId = opId;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_GetUpVoiceVolume: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_SetEchoSuppression(VOS_UINT16 clientId, VOS_UINT8 opId, AppVcEchoexReq echoEx)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetEchoSuppression: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_SET_ECHOEX_REQ;
    msg->clientId = clientId;
    msg->opId = opId;
    msg->echoEnable = echoEx.echoEnable;
    msg->echoInputGain = echoEx.echoInputGain;
    msg->echoLevelControl = echoEx.echoLevelControl;
    msg->echoClearFilterButton = echoEx.echoClearFilterButton;
    msg->echoVadInputLevel = echoEx.echoVadInputLevel;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetEchoSuppression: SEND MSG FAIL.");
        return VOS_ERR;
    }
    AT_INFO_LOG("APP_VC_SetEchoSuppression: SEND MSG SUCCESS.");
    return VOS_OK;
}

VOS_UINT32 APP_VC_GetEchoSuppression(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_GetEchoSuppression: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_GET_ECHOEX_REQ;
    msg->clientId = clientId;
    msg->opId = opId;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_GetEchoSuppression: SEND MSG FAIL.");
        return VOS_ERR;
    }
    return VOS_OK;
}

VOS_UINT32 APP_VC_SetPcmFr(VOS_UINT16 clientId, VOS_UINT8 opId, APP_VC_PcmFrReq pcmFrPara)
{
    VOS_UINT32 result;
    errno_t memResult;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetPcmFr: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_SET_PCMFR_STATUS_REQ;
    msg->clientId = clientId;
    msg->opId = opId;

    /* 将入参前半部传递给SND，后半部传递给RCV */
    memResult = memcpy_s(msg->anfuAudioSndEq, sizeof(msg->anfuAudioSndEq), pcmFrPara.pcmFr,
                         sizeof(pcmFrPara.pcmFr) / 2);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(msg->anfuAudioSndEq), sizeof(pcmFrPara.pcmFr) / 2);
    memResult = memcpy_s(msg->anfuAudioRcvEq, sizeof(msg->anfuAudioSndEq), &pcmFrPara.pcmFr[MN_CALL_PCM_FR_PARA_NUM],
                         sizeof(pcmFrPara.pcmFr) / 2);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(msg->anfuAudioSndEq), sizeof(pcmFrPara.pcmFr) / 2);

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetPcmFr: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 APP_VC_GetPcmFr(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    VOS_UINT32 result;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_GetPcmFr: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
                  sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_GET_PCMFR_STATUS_REQ;
    msg->clientId = clientId;
    msg->opId = opId;

    result = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (result != VOS_OK) {
        AT_ERR_LOG("APP_VC_GetPcmFr: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

/* 设置回声消除开关 */
VOS_UINT32 APP_VC_SetEchoSwitch(VOS_UINT16 clientId, VOS_UINT8 opId, VOS_UINT16 echoState)
{
    VOS_UINT32 ret;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_SetEchoSwitch: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
        sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_SET_ECHOSWITCH_REQ;
    msg->clientId = clientId;
    msg->opId = opId;
    msg->echoSwitchState = echoState;

    ret = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (ret != VOS_OK) {
        AT_ERR_LOG("APP_VC_SetEchoSwitch: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

/* 获取回声消除开关状态 */
VOS_UINT32 APP_VC_GetEchoSwitch(VOS_UINT16 clientId, VOS_UINT8 opId)
{
    VOS_UINT32 ret;
    APP_VC_ReqMsg *msg = VOS_NULL_PTR;

    msg = (APP_VC_ReqMsg *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT, sizeof(APP_VC_ReqMsg));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("APP_VC_GetEchoSwitch: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(clientId, I0_WUEPS_PID_VC),
        sizeof(APP_VC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    msg->msgName = APP_VC_MSG_GET_ECHOSWITCH_REQ;
    msg->clientId = clientId;
    msg->opId = opId;

    ret = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (ret != VOS_OK) {
        AT_ERR_LOG("APP_VC_GetEchoSwitch: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}
#endif

