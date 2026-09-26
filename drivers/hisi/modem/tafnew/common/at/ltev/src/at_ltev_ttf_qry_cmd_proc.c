/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#include "at_ltev_ttf_qry_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_snd_msg.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_TTF_QRY_CMD_PROC_C

#if (FEATURE_LTEV == FEATURE_ON)
VOS_UINT32 VPDCP_QryPtrRpt(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VMAC模块 */
    VOS_UINT32       rslt;
    AT_VPDCP_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VPDCP_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VPDCP_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VPDCP_QryPtrRpt: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VPDCP_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VPDCP_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = VPDCP_MSG_RSU_PTRRPT_QRY_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VPDCP_UL);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("VPDCP_QryPtrRpt: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_QryPtrRpt(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (VPDCP_QryPtrRpt(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PTRRPT_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    AT_WARN_LOG("AT_QryPtrRpt AT_ERROR\n");
    return AT_ERROR;
}

VOS_UINT32 VMAC_QryVRssi(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VMAC模块 */
    VOS_UINT32      rslt;
    AT_VMAC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VMAC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VMAC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VMAC_QryVRssi: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VMAC_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VMAC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = VMAC_MSG_RSU_VRSSI_QRY_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VMAC_UL);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("VMAC_QryVRssi: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_QryVRssi(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (VMAC_QryVRssi(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VRSSI_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    AT_WARN_LOG("AT_QryVRssi AT_ERROR\n");
    return AT_ERROR;
}

#endif

