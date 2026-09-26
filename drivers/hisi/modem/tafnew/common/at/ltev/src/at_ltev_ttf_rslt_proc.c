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
#include "at_ltev_ttf_rslt_proc.h"
#include "securec.h"

#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_TTF_RSLT_PROC_C

#if (FEATURE_LTEV == FEATURE_ON)
/* AT 模块处理来自Vpdcp消息函数对应表 */
const AT_VPDCP_ProcMsgTbl g_atProcMsgFromVpdcpTab[] = {
    /* 消息ID */ /* 消息处理函数 */
    { VPDCP_MSG_RSU_PTRRPT_QRY_CNF, 0, AT_RcvPtrRptQryCnf },
    { VPDCP_MSG_RSU_PTRRPT_SET_CNF, 0, AT_RcvPtrRptSetCnf },
};

VOS_VOID AT_ProcMsgFromVpdcp(struct MsgCB *msg)
{
    MN_AT_IndEvt *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32    i;
    VOS_UINT32    msgCnt;
    VOS_UINT16    msgName;
    VOS_UINT32    rst;

    msgCnt  = AT_ARRAY_SIZE(g_atProcMsgFromVpdcpTab);
    rcvMsg  = (MN_AT_IndEvt *)msg;
    msgName = rcvMsg->msgName;

    /* g_astAtProcMsgFromVpdcpTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcMsgFromVpdcpTab[i].msgName == msgName) {
            rst = g_atProcMsgFromVpdcpTab[i].procMsgFunc(rcvMsg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcMsgFromVpdcp: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcMsgFromVpdcp: Msg Id is invalid!");
    }
}

VOS_UINT32 AT_RcvPtrRptSetCnf(MN_AT_IndEvt *msg)
{
    VPDCP_AT_PtrrptQryCnf *ptrRptCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT16             length;
    VOS_UINT8              indexNum = 0;

    /* 初始化 */
    ptrRptCnf = (VPDCP_AT_PtrrptQryCnf *)msg->content;
    length    = 0;
    result    = AT_OK;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvPtrRptSetCnf: WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvPtrRptSetCnf: AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_PTRRPT_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PTRRPT_SET) {
        AT_WARN_LOG("AT_RcvPtrRptSetCnf : AT_CMD_PTRRPT_SET.\n");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s:%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        ptrRptCnf->pc5TxBytes, ptrRptCnf->pc5RxBytes, ptrRptCnf->pc5TxPkts, ptrRptCnf->pc5RxPkts);

    g_atSendDataBuff.bufLen = length;

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvPtrRptQryCnf(MN_AT_IndEvt *msg)
{
    VPDCP_AT_PtrrptQryCnf *ptrRptCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT16             length;
    VOS_UINT8              indexNum = 0;

    /* 初始化 */
    ptrRptCnf = (VPDCP_AT_PtrrptQryCnf *)msg->content;
    length    = 0;
    result    = AT_OK;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvPtrRptQryCnf: WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvPtrRptQryCnf: AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_PTRRPT_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PTRRPT_QRY) {
        AT_WARN_LOG("AT_RcvPtrRptQryCnf : AT_CMD_PTRRPT_QRY.\n");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s:%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        ptrRptCnf->pc5TxBytes, ptrRptCnf->pc5RxBytes, ptrRptCnf->pc5TxPkts, ptrRptCnf->pc5RxPkts);

    g_atSendDataBuff.bufLen = length;

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvVrssiQryCnf(MN_AT_IndEvt *msg)
{
    VMAC_AT_VrssiQryCnf *vrssiCnf = VOS_NULL_PTR;
    VOS_UINT32           result;
    VOS_UINT16           length;
    VOS_UINT8            indexNum = 0;

    /* 初始化 */
    vrssiCnf = (VMAC_AT_VrssiQryCnf *)msg->content;
    length   = 0;
    result   = AT_OK;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvVrssiQryCnf:WARNING:AT INDEX NOT FOUND!\n");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvVrssiQryCnf : AT_BROADCAST_INDEX.\n");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_VRSSI_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VRSSI_QRY) {
        AT_WARN_LOG("AT_RcvVrssiQryCnf : AT_CMD_VRSSI_QRY.\n");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^VRSSI查询命令返回 */
    if (vrssiCnf->qryRslt != VOS_OK) {
        AT_WARN_LOG("AT_RcvVrssiQryCnf : AT_ERROR.\n");
        result = AT_ERROR;
    } else {
        result = AT_OK;
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, vrssiCnf->vrssi);

        g_atSendDataBuff.bufLen = length;
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvPc5PktSndCnf(VTC_AT_MsgCnf *msg)
{
    VOS_UINT8               indexNum    = 0;
    VOS_UINT32              result      = AT_OK;
    VTC_AT_SendVMacDataCnf *sndDataStat = VOS_NULL_PTR;

    /* 初始化 */
    sndDataStat = (VTC_AT_SendVMacDataCnf *)msg->content; /*lint !e838 */

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvPc5PktSndCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvPc5PktSndCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen = 0;

    if (sndDataStat->result == VOS_OK) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#endif

