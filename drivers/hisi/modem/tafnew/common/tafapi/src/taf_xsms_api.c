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

#include "taf_app_xsms_interface.h"
#include "mn_client.h"
#include "mdrv_nvim.h"
#include "nv_stru_gucnas.h"
#include "securec.h"


#define THIS_FILE_ID PS_FILE_ID_TAF_XSMS_API_C

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_UINT32 TAF_XSMS_GetReceiverPid(MN_CLIENT_ID_T clientId, VOS_UINT32 *receiverPid)
{
#if (MULTI_MODEM_NUMBER > 1)
    ModemIdUint16 modemID;

    /* 调用接口获取Modem ID */
    if (AT_GetModemIdFromClient(clientId, &modemID) != VOS_OK) {
        return VOS_ERR;
    }

    if (modemID == MODEM_ID_1) {
        *receiverPid = I1_UEPS_PID_XSMS;
    }
#if (MULTI_MODEM_NUMBER > 2)
    else if (modemID == MODEM_ID_2) {
        *receiverPid = I2_UEPS_PID_XSMS;
    }
#endif
    else {
        *receiverPid = I0_UEPS_PID_XSMS;
    }
#else
    *receiverPid = UEPS_PID_XSMS;
#endif

    return VOS_OK;
}

VOS_UINT32 TAF_XSMS_SendSmsReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_XSMS_SendOptionUint8 sndOption,
                               VOS_UINT8 *data)
{
    TAF_XSMS_SendMsgReq *msg = VOS_NULL_PTR;
    VOS_UINT32           receiverPid;
    errno_t              memResult;

    if (TAF_XSMS_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        return VOS_ERR;
    }

    if (data == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    msg = (TAF_XSMS_SendMsgReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(TAF_XSMS_SendMsgReq) - VOS_MSG_HEAD_LENGTH);

    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    msg->msgName = TAF_XSMS_APP_MSG_TYPE_SEND_REQ;
    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->clientId  = clientId;
    msg->opId      = opId;
    msg->sndOption = sndOption;

    memResult = memcpy_s(&msg->sms, sizeof(msg->sms), data, sizeof(TAF_XSMS_Message));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(msg->sms), sizeof(TAF_XSMS_Message));

    if (VOS_SendMsg(WUEPS_PID_AT, msg) == VOS_OK) {
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}

VOS_UINT32 TAF_XSMS_SetXsmsApMemFullReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT8 apMemFullFlag)
{
    TAF_XSMS_AppMsgSetApMemFullReq *msg = VOS_NULL_PTR;
    VOS_UINT32                      receiverPid;

    if (TAF_XSMS_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        return VOS_ERR;
    }

    if (apMemFullFlag >= TAF_XSMS_AP_MEM_BUTT) {
        return VOS_ERR;
    }

    msg = (TAF_XSMS_AppMsgSetApMemFullReq *)VOS_AllocMsg(WUEPS_PID_AT,
                                                         sizeof(TAF_XSMS_AppMsgSetApMemFullReq) - VOS_MSG_HEAD_LENGTH);

    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    msg->msgName = TAF_XSMS_APP_MSG_TYPE_UIM_MEM_SET_REQ;
    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->apMemFullFlag = apMemFullFlag;

    if (VOS_SendMsg(WUEPS_PID_AT, msg) == VOS_OK) {
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}

VOS_UINT32 TAF_XSMS_WriteSmsReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_XSMS_StatusUint8 status,
                                VOS_UINT8 *data)
{
    TAF_XSMS_WriteMsgReq *msg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    errno_t               memResult;

    if (TAF_XSMS_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        return VOS_ERR;
    }

    if (data == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    msg = (TAF_XSMS_WriteMsgReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(TAF_XSMS_WriteMsgReq) - VOS_MSG_HEAD_LENGTH);

    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    msg->msgName = TAF_XSMS_APP_MSG_TYPE_WRITE_REQ;
    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->clientId  = clientId;
    msg->opId      = opId;
    msg->smsStatus = status;

    memResult = memcpy_s(&msg->sms, sizeof(msg->sms), data, sizeof(TAF_XSMS_Message));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(msg->sms), sizeof(TAF_XSMS_Message));

    if (VOS_SendMsg(WUEPS_PID_AT, msg) == VOS_OK) {
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}

VOS_UINT32 TAF_XSMS_DeleteSmsReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT8 indexNum)
{
    TAF_XSMS_DeleteMsgReq *msg = VOS_NULL_PTR;
    VOS_UINT32             receiverPid;

    if (TAF_XSMS_GetReceiverPid(clientId, &receiverPid) != VOS_OK) {
        return VOS_ERR;
    }

    msg = (TAF_XSMS_DeleteMsgReq *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(TAF_XSMS_DeleteMsgReq) - VOS_MSG_HEAD_LENGTH);

    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    msg->msgName = TAF_XSMS_APP_MSG_TYPE_DELETE_REQ;
    VOS_SET_RECEIVER_ID(msg, receiverPid);
    msg->clientId = clientId;
    msg->opId     = opId;
    msg->indexNum = indexNum;

    if (VOS_SendMsg(WUEPS_PID_AT, msg) == VOS_OK) {
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}
#endif

