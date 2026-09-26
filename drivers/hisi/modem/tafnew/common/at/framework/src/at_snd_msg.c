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

#include "at_snd_msg.h"
#include "securec.h"
#include "at_csimagent.h"
#include "at_imsa_interface.h"
#include "taf_ccm_api.h"
#include "mn_comm_api.h"


#define THIS_FILE_ID PS_FILE_ID_AT_SND_MSG_C

VOS_UINT32 AT_FillAppReqMsgHeader(MN_APP_ReqMsg *msg, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                  VOS_UINT16 msgType, VOS_UINT32 rcvPid)
{
    MN_APP_ReqMsg *appMsgHeader = VOS_NULL_PTR;

    appMsgHeader           = msg;
    appMsgHeader->clientId = clientId;
    appMsgHeader->opId     = opId;
    appMsgHeader->msgName  = msgType;
    VOS_SET_SENDER_ID(appMsgHeader, WUEPS_PID_AT);
    VOS_SET_RECEIVER_ID(appMsgHeader, AT_GetDestPid(clientId, rcvPid));

#if (VOS_OS_VER == VOS_WIN32)
    appMsgHeader->ulSenderCpuId   = VOS_LOCAL_CPUID;
    appMsgHeader->ulReceiverCpuId = VOS_LOCAL_CPUID;
#endif

    return VOS_OK;
}

VOS_UINT32 AT_FillAppReqMsgPara(VOS_UINT8 *sndMsgPara, VOS_UINT32 msgLen, const VOS_UINT8 *para, VOS_UINT32 len)
{
    errno_t memResult;
    if (sndMsgPara == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    if (len > 0) {
        memResult = memcpy_s(sndMsgPara, msgLen, para, len);
        TAF_MEM_CHK_RTN_VAL(memResult, msgLen, len);
    }

    return VOS_OK;
}

VOS_UINT32 AT_GetAppReqMsgLen(VOS_UINT32 paraLen, VOS_UINT32 *msgLen)
{
    VOS_UINT32 msgLenTemp;

    if (msgLen == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    if (paraLen <= MN_APP_REQ_DEFAULT_CONTENT_LEN) {
        msgLenTemp = sizeof(MN_APP_ReqMsg);
    } else {
        msgLenTemp = (sizeof(MN_APP_ReqMsg) + paraLen) - MN_APP_REQ_DEFAULT_CONTENT_LEN;
    }

    *msgLen = msgLenTemp;

    return VOS_OK;
}

VOS_UINT32 AT_FillAndSndAppReqMsg(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT16 msgType, VOS_UINT8 *para,
                                  VOS_UINT32 paraLen, VOS_UINT32 rcvPid)
{
    VOS_UINT32    ret;
    VOS_UINT32    msgLen;
    VOS_UINT8    *msg     = VOS_NULL_PTR;
    VOS_UINT8    *msgPara = VOS_NULL_PTR;
    ModemIdUint16 modemId = MODEM_ID_0;

    /* 获取client id对应的Modem Id */
    ret = AT_GetModemIdFromClient(clientId, &modemId);

    if (ret == VOS_ERR) {
        return TAF_FAILURE;
    }

    if ((rcvPid == PS_PID_IMSA)
#if (FEATURE_MODEM1_SUPPORT_LTE == FEATURE_ON)
        && (modemId == MODEM_ID_2))
#else
        && ((modemId == MODEM_ID_1) || (modemId == MODEM_ID_2)))
#endif
    {
        return TAF_FAILURE;
    }

    if ((para == VOS_NULL_PTR) && (paraLen > 0)) {
        return TAF_FAILURE;
    }

    msgPara = (VOS_UINT8 *)para;

    /* 获取消息长度 */
    AT_GetAppReqMsgLen(paraLen, &msgLen);

    /* 申请消息 */
    /*lint -save -e516 */
    msg = (VOS_UINT8 *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, msgLen - VOS_MSG_HEAD_LENGTH);
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        return TAF_FAILURE;
    }

    (VOS_VOID)memset_s((msg + VOS_MSG_HEAD_LENGTH), (msgLen - VOS_MSG_HEAD_LENGTH), 0x00,
                       (msgLen - VOS_MSG_HEAD_LENGTH));

    /* 填充消息头 */
    AT_FillAppReqMsgHeader((MN_APP_ReqMsg *)msg, clientId, opId, msgType, rcvPid);

    /* 填充消息参数 */
    AT_FillAppReqMsgPara(&msg[sizeof(MN_APP_ReqMsg) - AT_DATA_DEFAULT_LENGTH],
                         msgLen - sizeof(MN_APP_ReqMsg) + AT_DATA_DEFAULT_LENGTH, msgPara, paraLen);

    ret = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);

    if (ret != VOS_OK) {
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

