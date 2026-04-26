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

#include "taf_ssa_api.h"
#include "ps_common_def.h"
#include "mn_client.h"

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "nas_multi_instance_api.h"
#endif
#include "securec.h"
#include "mn_comm_api.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_TAF_SSA_API_C

VOS_VOID TAF_SSA_SndTafMsg(TAF_SSA_MsgIdUint32 msgId, const VOS_VOID *data, VOS_UINT32 length)
{
    TAF_SSA_Msg *msg = VOS_NULL_PTR;
    VOS_UINT32   realPid;
    VOS_UINT32   senderPid;
    errno_t      memResult;

    realPid   = TAF_GetDestPid(((TAF_Ctrl *)data)->clientId, WUEPS_PID_TAF);
    senderPid = TAF_GetSendPid(((TAF_Ctrl *)data)->clientId, WUEPS_PID_TAF);

    /* 构造消息 */
    msg = (TAF_SSA_Msg *)TAF_AllocMsgWithHeaderLen(realPid, sizeof(MSG_Header) + length);
    if (msg == VOS_NULL_PTR) {
        return;
    }
    VOS_SET_SENDER_ID(msg, senderPid);
    VOS_SET_RECEIVER_ID(msg, realPid);
    msg->header.msgName = msgId;

    /* 填写消息内容 */
    if (length > 0) {
        memResult = memcpy_s(msg->content, length, data, length);
        TAF_MEM_CHK_RTN_VAL(memResult, length, length);
    }

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(realPid, msg);
}

VOS_VOID TAF_SSA_SetCmolrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                              TAF_SSA_LcsMolrParaSet *molrPara)
{
    TAF_SSA_SetLcsMolrReq setCmolrReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&setCmolrReq, sizeof(setCmolrReq), 0x00, sizeof(setCmolrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&setCmolrReq, moduleId, clientId, opId);

    setCmolrReq.molrPara = *molrPara;

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_SET_LCS_MOLR_REQ, &setCmolrReq, sizeof(setCmolrReq));
}

VOS_VOID TAF_SSA_GetCmolrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_SSA_GetLcsMolrReq getCmolrReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&getCmolrReq, sizeof(getCmolrReq), 0x00, sizeof(getCmolrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&getCmolrReq, moduleId, clientId, opId);

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_GET_LCS_MOLR_REQ, &getCmolrReq, sizeof(getCmolrReq));
}

VOS_VOID TAF_SSA_SetCmtlrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                              TAF_SSA_LcsMtlrSubscribeUint8 subscribe)
{
    TAF_SSA_SetLcsMtlrReq setCmtlrReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&setCmtlrReq, sizeof(setCmtlrReq), 0x00, sizeof(setCmtlrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&setCmtlrReq, moduleId, clientId, opId);

    setCmtlrReq.subscribe = subscribe;

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_SET_LCS_MTLR_REQ, &setCmtlrReq, sizeof(setCmtlrReq));
}

VOS_VOID TAF_SSA_GetCmtlrInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_SSA_GetLcsMtlrReq getCmtlrReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&getCmtlrReq, sizeof(getCmtlrReq), 0x00, sizeof(getCmtlrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&getCmtlrReq, moduleId, clientId, opId);

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_GET_LCS_MTLR_REQ, &getCmtlrReq, sizeof(getCmtlrReq));
}

VOS_VOID TAF_SSA_SetCmtlraInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId,
                               TAF_SSA_LcsMtlraParaSet *cmtlraPara)
{
    TAF_SSA_SetLcsMtlraReq setCmtlraReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&setCmtlraReq, sizeof(setCmtlraReq), 0x00, sizeof(setCmtlraReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&setCmtlraReq, moduleId, clientId, opId);

    setCmtlraReq.cmtlraPara = *cmtlraPara;

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_SET_LCS_MTLRA_REQ, &setCmtlraReq, sizeof(setCmtlraReq));
}

VOS_VOID TAF_SSA_GetCmtlraInfo(VOS_UINT32 moduleId, VOS_UINT16 clientId, VOS_UINT8 opId)
{
    TAF_SSA_GetLcsMtlraReq getCmtlraReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&getCmtlraReq, sizeof(getCmtlraReq), 0x00, sizeof(getCmtlraReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&getCmtlraReq, moduleId, clientId, opId);

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_GET_LCS_MTLRA_REQ, &getCmtlraReq, sizeof(getCmtlraReq));
}

VOS_VOID TAF_SSA_ClearMsgEntity(MSG_Header *msg)
{

    (VOS_VOID)memset_s(TAF_SSA_GET_MSG_ENTITY(msg), TAF_SSA_GET_MSG_LENGTH(msg), 0x00, TAF_SSA_GET_MSG_LENGTH(msg));
}

