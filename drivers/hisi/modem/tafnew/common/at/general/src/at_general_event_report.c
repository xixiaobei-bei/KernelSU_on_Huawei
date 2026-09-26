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
#include "at_general_event_report.h"
#include "taf_type_def.h"
#include "securec.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"
#include "taf_app_mma.h"
#include "at_lte_common.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_EVENT_REPORT_C

VOS_UINT32 AT_RcvMtaCommBoosterInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg           = VOS_NULL_PTR;
    MTA_AT_CommBoosterInd *mtaAtCommBooster = VOS_NULL_PTR;
    VOS_UINT32             i                = 0;
    VOS_UINT16             length;

    /* 初始化消息变量 */
    rcvMsg           = (AT_MTA_Msg *)msg;
    mtaAtCommBooster = (MTA_AT_CommBoosterInd *)rcvMsg->content;
    length           = 0;

    if (mtaAtCommBooster->len > MTA_AT_BOOSTER_IND_MAX_NUM) {
        mtaAtCommBooster->len = MTA_AT_BOOSTER_IND_MAX_NUM;
    }

    /* \r\n^BOOSTERNTF */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s", g_atCrLf, g_atStringTab[AT_STRING_BOOSTERNTF].text);

    /* module indexNum, len */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d, %d,\"", mtaAtCommBooster->pidIndex, mtaAtCommBooster->len);

    for (i = 0; i < mtaAtCommBooster->len; i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%02X", mtaAtCommBooster->boosterIndData[i]);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s", g_atCrLf);

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaInitLocInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_InitLocInfoInd *initLocInfoInd = (TAF_MMA_InitLocInfoInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^INITLOCINFO: %x%x%x,%d%s", g_atCrLf, (initLocInfoInd->mcc & 0x0f00) >> 8,
        (initLocInfoInd->mcc & 0x00f0) >> 4, (initLocInfoInd->mcc & 0x000f), initLocInfoInd->sid, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 atCerssiInfoIndProc(struct MsgCB *msgBlock)
{
    L4A_CSQ_INFO_IND_STRU *cerssi = NULL;
    VOS_UINT16             length = 0;

    cerssi = (L4A_CSQ_INFO_IND_STRU *)msgBlock;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "%s%s0,0,255,%d,%d,%d,%d,%d,%d%s", g_atCrLf, "^CERSSI:", cerssi->rsrp,
        cerssi->rsrq, cerssi->sinr, cerssi->cqi.ri, cerssi->cqi.cqi[0], cerssi->cqi.cqi[1], g_atCrLf);

    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, g_atSndCodeAddr, length);

    return AT_FW_CLIENT_STATUS_READY;
}

