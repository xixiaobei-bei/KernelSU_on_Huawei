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

#include "taf_mtc_api.h"
#include "ps_common_def.h"
#include "mn_comm_api.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_TAF_MTC_API_C

#if (OSA_CPU_CCPU == VOS_OSA_CPU)

VOS_UINT32 MTC_SndMsg(VOS_UINT32 taskId, VOS_UINT32 msgId, const VOS_VOID *data, VOS_UINT32 length)
{
    TAF_PS_Msg *msg = VOS_NULL_PTR;
    VOS_UINT32  result = VOS_OK;
    errno_t     memResult;

    /* 构造消息 */
    msg = (TAF_PS_Msg *)TAF_AllocMsgWithHeaderLen(UEPS_PID_MTC, sizeof(MSG_Header) + length);
    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    VOS_SET_RECEIVER_ID(msg, UEPS_PID_MTC);
    msg->header.msgName = msgId;

    /* 填写消息内容 */
    if (length > 0) {
        memResult = memcpy_s(msg->content, length, data, length);
        TAF_MEM_CHK_RTN_VAL(memResult, length, length);
    }

    /* 发送消息 */
    result = TAF_TraceAndSendMsg(UEPS_PID_MTC, msg);
    if (result != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

MODULE_EXPORTED VOS_UINT32 MTC_SetModemServiceConnState(TAF_Ctrl *ctrl, TAF_MTC_SrvConnStateInfo *modemConnSt)
{
    errno_t                    memResult;
    TAF_MTC_ModemConnStatusInd modemConnStInfo;
    VOS_UINT32                 result = VOS_OK;

    /* 初始化 */
    (VOS_VOID)memset_s(&modemConnStInfo, sizeof(modemConnStInfo), 0x00, sizeof(TAF_MTC_ModemConnStatusInd));

    /* 构造ID_MSG_MTC_CDMA_CONN_STATUS消息 */
    TAF_API_CTRL_HEADER(&(modemConnStInfo.ctrl), ctrl->moduleId, ctrl->clientId, ctrl->opId);

    memResult = memcpy_s(&modemConnStInfo.modemConnStateInfo, sizeof(modemConnStInfo.modemConnStateInfo), modemConnSt,
                         sizeof(TAF_MTC_SrvConnStateInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(modemConnStInfo.modemConnStateInfo), sizeof(TAF_MTC_SrvConnStateInfo));

    /* 发送消息 */
    result = MTC_SndMsg(UEPS_PID_MTC, ID_MSG_MTC_MODEM_SERVICE_CONN_STATE_IND, &modemConnStInfo,
                        sizeof(TAF_MTC_ModemConnStatusInd));

    return result;
}

VOS_UINT32 MTC_SetModemUsimmState(TAF_Ctrl *ctrl, TAF_MTC_UsimmCardServicUint16 usimState,
                                  TAF_MTC_UsimmCardServicUint16 csimState)
{
    TAF_MTC_UsimmStatusInd usimmState;
    VOS_UINT32             result = VOS_OK;

    /* 初始化 */
    (VOS_VOID)memset_s(&usimmState, sizeof(usimmState), 0x00, sizeof(TAF_MTC_UsimmStatusInd));

    /* 构造ID_MSG_MTC_CDMA_CONN_STATUS消息 */
    TAF_API_CTRL_HEADER(&(usimmState.ctrl), ctrl->moduleId, ctrl->clientId, ctrl->opId);

    usimmState.usimState = usimState;
    usimmState.csimState = csimState;

    /* 发送消息 */
    result = MTC_SndMsg(UEPS_PID_MTC, ID_MSG_MTC_USIMM_STATE_IND, &usimmState, sizeof(TAF_MTC_UsimmStatusInd));

    return result;
}

MODULE_EXPORTED VOS_VOID MTC_SetRatModeInfo(TAF_Ctrl *ctrl, MTC_RATMODE_Uint8 ratMode)
{
    MTC_RAT_ModeInd ratModeInfo;

    /* 初始化 */
    (VOS_VOID)memset_s(&ratModeInfo, sizeof(ratModeInfo), 0x00, sizeof(MTC_RAT_ModeInd));

    /* 构造ID_MSG_MTC_RAT_MODE_IND消息 */
    TAF_API_CTRL_HEADER(&(ratModeInfo.ctrl), ctrl->moduleId, ctrl->clientId, ctrl->opId);

    ratModeInfo.ratMode = ratMode;

    /* 发送消息 */
    MTC_SndMsg(UEPS_PID_MTC, ID_MSG_MTC_RAT_MODE_IND, &ratModeInfo, (VOS_UINT32)sizeof(MTC_RAT_ModeInd));
}

MODULE_EXPORTED VOS_VOID MTC_SetBeginSessionInfo(TAF_Ctrl *ctrl, MTC_SESSION_TypeUint8 sessionType)
{
    MTC_BEGIN_SessionInd sessionInfo;

    /* 初始化 */
    (VOS_VOID)memset_s(&sessionInfo, sizeof(sessionInfo), 0x00, sizeof(MTC_BEGIN_SessionInd));

    /* 构造ID_MSG_MTC_BEGIN_SESSION_IND消息 */
    TAF_API_CTRL_HEADER(&(sessionInfo.ctrl), ctrl->moduleId, ctrl->clientId, ctrl->opId);

    sessionInfo.sessionType = sessionType;

    /* 发送消息 */
    MTC_SndMsg(UEPS_PID_MTC, ID_MSG_MTC_BEGIN_SESSION_IND, &sessionInfo, sizeof(MTC_BEGIN_SessionInd));
}

MODULE_EXPORTED VOS_VOID MTC_SetEndSessionInfo(TAF_Ctrl *ctrl, MTC_SESSION_TypeUint8 sessionType)
{
    MTC_END_SessionInd endSessionInfo;

    /* 初始化 */
    (VOS_VOID)memset_s(&endSessionInfo, sizeof(endSessionInfo), 0x00, sizeof(MTC_END_SessionInd));

    /* 构造ID_MSG_MTC_END_SESSION_IND消息 */
    TAF_API_CTRL_HEADER(&(endSessionInfo.ctrl), ctrl->moduleId, ctrl->clientId, ctrl->opId);

    endSessionInfo.sessionType = sessionType;
    endSessionInfo.csRelAll    = VOS_FALSE;
    endSessionInfo.psRelAll    = VOS_FALSE;

    /* 发送消息 */
    MTC_SndMsg(UEPS_PID_MTC, ID_MSG_MTC_END_SESSION_IND, &endSessionInfo, sizeof(MTC_END_SessionInd));
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

MODULE_EXPORTED VOS_VOID MTC_Set1xActiveFlg(VOS_UINT8 uc1xActiveFlg, VOS_UINT8 isSwitchOnScene)
{
    MTC_1X_ActiveFlgInd st1xActiveInfo;

    /* 初始化 */
    (VOS_VOID)memset_s(&st1xActiveInfo, sizeof(st1xActiveInfo), 0x00, sizeof(MTC_1X_ActiveFlgInd));

    /* 构造ID_MSG_MTC_RAT_MODE_IND消息 */
    st1xActiveInfo.activeFlg       = uc1xActiveFlg;
    st1xActiveInfo.isSwitchOnScene = isSwitchOnScene;

    /* 发送消息 */
    MTC_SndMsg(UEPS_PID_MTC, ID_MSG_MTC_1X_ACTIVE_IND, &st1xActiveInfo, (VOS_UINT32)sizeof(MTC_1X_ActiveFlgInd));
}
#endif
#endif
