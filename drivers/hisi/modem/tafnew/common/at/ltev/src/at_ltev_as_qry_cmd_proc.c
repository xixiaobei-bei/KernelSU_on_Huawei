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
#include "at_ltev_as_qry_cmd_proc.h"
#include "at_ltev_comm.h"
#include "securec.h"

#include "AtParse.h"
#include "at_snd_msg.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_AS_QRY_CMD_PROC_C

#if (FEATURE_LTEV == FEATURE_ON)
VOS_UINT32 VRRC_QryGnssInfo(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VRRC模块 */
    VOS_UINT32      rslt;
    AT_VRRC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VRRC_QryGnssInfo: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_GNSS_INFO_QRY_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_QryGnssInfo: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 VRRC_QryRsuVphyStat(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VRRC模块 */
    VOS_UINT32      rslt;
    AT_VRRC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VRRC_QryRsuVphyStat: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_RSU_VPHYSTAT_QRY_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_QryRsuVphyStat: SEND MSG FAIL!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 VRRC_QryRsuVSnrRsrp(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VRRC模块 */
    VOS_UINT32      rslt;
    AT_VRRC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VRRC_QryRsuVSnrRsrp: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_RSU_VSNRRSRP_QRY_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_QryRsuVSnrRsrp: SEND MSG FAIL!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 VRRC_QryRsuV2xRssi(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VRRC模块 */
    VOS_UINT32      rslt;
    AT_VRRC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VRRC_QryRsuV2xRssi: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_RSU_RSSI_QRY_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_QryRsuV2xRssi: SEND MSG FAIL!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_QryCatm(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_TEST_MODE_ACTIVE_STATE_QRY_REQ, VOS_NULL_PTR, 0);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CATM_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCcutle(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_TEST_LOOP_MODE_E_CLOSE_STATE_QRY_REQ, VOS_NULL_PTR, 0);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCUTLE_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestCcutle(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1),(0,1),(1),(16)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}

VOS_UINT32 AT_QryCbr(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, AT_VMAC_CBR_QRY_REQ,
                                    VOS_NULL_PTR, 0, I0_PS_PID_VMAC_UL);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CBR_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCv2xdts(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_SENDING_DATA_ACTION_QRY_REQ, VOS_NULL_PTR, 0);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CV2XDTS_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestCv2xdts(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1),(8176),(10)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}

VOS_UINT32 AT_QryVsyncsrc(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    AT_VRRC_SYNC_SOURCE_QRY_REQ, VOS_NULL_PTR, 0, I0_PS_PID_VERRC);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VSYNCSRC_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryVsyncsrcrpt(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    AT_VRRC_SYNC_SOURCE_RPT_QRY_REQ, VOS_NULL_PTR, 0, I0_PS_PID_VERRC);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VSYNCSRCRPT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryVsyncmode(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    AT_VRRC_SYNC_MODE_QRY_REQ, VOS_NULL_PTR, 0, I0_PS_PID_VERRC);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VSYNCMODE_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCv2xL2Id(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_LAYER_TWO_ID_QRY_REQ, VOS_NULL_PTR, 0);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CV2XL2ID_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryData(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (VRRC_QryGnssInfo(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DATA_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    AT_WARN_LOG("AT_QryGnssInfo AT_ERROR\n");
    return AT_ERROR;
}

VOS_UINT32 AT_TestQryData(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_TEST_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1),(0,1),(0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_QryVPhyStat(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (VRRC_QryRsuVphyStat(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VPHYSTAT_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_WARN_LOG("AT_QryVPhyStat: AT_ERROR!\n");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryVSnrRsrp(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (VRRC_QryRsuVSnrRsrp(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VSNRRSRP_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryV2xRssi(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (VRRC_QryRsuV2xRssi(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_V2XRSSI_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_WARN_LOG("AT_QryV2xRssi AT_ERROR!\n");
        return AT_ERROR;
    }
}

#endif

