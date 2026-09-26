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
#include "at_device_as_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"

#include "nv_stru_gas.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_AS_QRY_CMD_PROC_C

#define AT_TRXTAS_QRY_MODE 0
#define AT_TRXTAS_QRY_RAT_MODE 1
#define AT_TRXTASQRY_PARA_NUM 2

VOS_UINT32 AT_QryFeaturePara(VOS_UINT8 indexNum)
{
    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                               ID_AT_MTA_SFEATURE_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA) == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SFEATURE_QRY; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                 /* 等待异步事件返回 */
    } else {
        return AT_ERROR;
    }
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_QryDpaCat(VOS_UINT8 indexNum)
{
    VOS_UINT8  dpaRate;
    VOS_UINT32 wasResult;
    VOS_UINT16 length;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 调用读NV接口函数: At_ReadDpaCatFromNV,返回操作结果 */
    wasResult = At_ReadDpaCatFromNV(&dpaRate);
    if (wasResult == VOS_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", (VOS_UINT32)dpaRate);
        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    } else {
        AT_WARN_LOG("At_QryDpaCat:WARNING:WAS_MNTN_QueryDpaCat failed!");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryHsspt(VOS_UINT8 indexNum)
{
    VOS_UINT8  rRCVer;
    VOS_UINT32 result;
    VOS_UINT16 length;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_DPAUPA_ERROR;
    }

    result = AT_ReadRrcVerFromNV(&rRCVer);

    if (result == VOS_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", (VOS_UINT32)rRCVer);
        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    } else {
        AT_WARN_LOG("AT_QryHspaSpt:WARNING:WAS_MNTN_QueryHspaSpt failed!");
        return AT_DPAUPA_ERROR;
    }
}
#endif

VOS_UINT32 AT_QryGTimerPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;
    VOS_UINT32 gTimerLength = 0;

    /* 读取NV项NV_ITEM_GPRS_ACTIVE_TIMER_LEN获取GPRS定时器时长 */
    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_GPRS_ACTIVE_TIMER_LEN, &gTimerLength, sizeof(gTimerLength));
    if (ret != NV_OK) {
        AT_WARN_LOG("AT_QryGTimerPara: Fail to read NV_ITEM_GPRS_ACTIVE_TIMER_LEN");
        return AT_DEVICE_OTHER_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, gTimerLength);

    return AT_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 At_QryTrxTasPara(VOS_UINT8 indexNum)
{
    AT_MTA_QryTrxTasReq qryTrxTas;
    VOS_UINT32          rst;

    AT_PR_LOGH("At_SetQryTrxTasPara Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (g_atParaIndex != AT_TRXTASQRY_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 信令模式 */
    if (g_atParaList[AT_TRXTAS_QRY_MODE].paraValue == AT_MTA_CMD_SIGNALING_MODE) {
        /* AT发送至MTA的消息结构赋值 */
        (VOS_VOID)memset_s(&qryTrxTas, sizeof(qryTrxTas), 0x00, sizeof(qryTrxTas));
        qryTrxTas.mode    = (AT_MTA_CmdSignalingUint8)g_atParaList[AT_TRXTAS_QRY_MODE].paraValue;
        qryTrxTas.ratMode = (AT_MTA_CmdRatmodeUint8)g_atParaList[AT_TRXTAS_QRY_RAT_MODE].paraValue;

        /* 发送消息给C核处理 */
        rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_QRY_TRX_TAS_REQ,
                                     (VOS_UINT8 *)&qryTrxTas, sizeof(qryTrxTas), I0_UEPS_PID_MTA);
    }
    /* 非信令模式 */
    else {
        return AT_CME_OPERATION_NOT_SUPPORTED;
    }

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TRX_TAS_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

