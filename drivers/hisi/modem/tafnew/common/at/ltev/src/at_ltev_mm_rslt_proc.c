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
#include "at_ltev_mm_rslt_proc.h"
#include "securec.h"

#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_MM_RSLT_PROC_C

#if ((FEATURE_LTEV == FEATURE_ON) && defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
AT_VNAS_ProcMsgTbl g_atProcMsgFromVnasTbl[] = {
    { ID_VNAS_AT_LTEV_SOURCE_ID_SET_CNF, AT_RcvVnasSrcidSetCnf },
    { ID_VNAS_AT_LTEV_SOURCE_ID_QRY_CNF, AT_RcvVnasSrcidQryCnf },
};
#endif

#if (FEATURE_LTEV == FEATURE_ON)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_VOID AT_ProcMsgFromVnas(struct MsgCB *msg)
{
    VNAS_ReqMsg *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32   i;
    VOS_UINT32   msgCnt;
    VOS_UINT32   msgName;
    VOS_UINT32   rst;

    /* 从g_atProcMsgFromVnasTbl中获取消息个数 */
    msgCnt = sizeof(g_atProcMsgFromVnasTbl) / sizeof(AT_MSG_Proc);
    rcvMsg = (VNAS_ReqMsg *)msg;

    /* 从消息包中获取MSG Name */
    msgName = rcvMsg->msgName;

    /* g_astAtProcMsgFromVnaspTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcMsgFromVnasTbl[i].msgType == msgName) {
            rst = g_atProcMsgFromVnasTbl[i].procMsgFunc((struct MsgCB *)rcvMsg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcMsgFromVnas: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcMsgFromVnas: Msg Id is invalid!");
    }
}

VOS_UINT32 AT_RcvVnasSrcidSetCnf(struct MsgCB *msg)
{
    VNAS_ReqMsg             *rcvMsg         = VOS_NULL_PTR;
    VNAS_LtevSourceIdSetCnf *setSourceIdCnf = VOS_NULL_PTR;
    VOS_UINT32               result;
    VOS_UINT8                tblIndex = 0;

    /* 初始化 */
    rcvMsg         = (VNAS_ReqMsg *)msg;
    setSourceIdCnf = (VNAS_LtevSourceIdSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(rcvMsg->clientId, AT_CMD_SRCID_SET, &tblIndex) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(tblIndex);

    /* 格式化AT^SRCID设置命令返回 */
    if (setSourceIdCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    /* 输出结果 */
    At_FormatResultData(tblIndex, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvVnasSrcidQryCnf(struct MsgCB *msg)
{
    VNAS_ReqMsg             *rcvMsg         = VOS_NULL_PTR;
    VNAS_LtevSourceIdQryCnf *qrySourceIdCnf = VOS_NULL_PTR;
    VOS_UINT8                tblIndex       = 0;

    /* 初始化 */
    rcvMsg         = (VNAS_ReqMsg *)msg;
    qrySourceIdCnf = (VNAS_LtevSourceIdQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(rcvMsg->clientId, AT_CMD_SRCID_QRY, &tblIndex) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(tblIndex);

    /* 格式化AT^SRCID查询命令返回 */
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "^SRCID: %d,%d", qrySourceIdCnf->sourceid, qrySourceIdCnf->sourceIdOrign);

    /* 输出结果 */
    At_FormatResultData(tblIndex, AT_OK);

    return VOS_OK;
}
#endif
#endif

