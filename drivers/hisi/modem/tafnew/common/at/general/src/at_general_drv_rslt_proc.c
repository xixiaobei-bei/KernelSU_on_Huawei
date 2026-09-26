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

#include "at_general_drv_rslt_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "taf_drv_agent.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_DRV_RSLT_PROC_C

VOS_UINT32 AT_FormatAtiCmdQryString(ModemIdUint16 modemId, DRV_AGENT_MsidQryCnf *drvAgentMsidQryCnf)
{
    TAF_PH_Meinfo meInfo;
    VOS_UINT8     tmp[AT_MAX_VERSION_LEN];
    VOS_UINT32    i = 0;
    errno_t       memResult;
    VOS_UINT16    dataLen = 0;
    VOS_UINT8     checkData = 0;

    /* 初始化 */
    (VOS_VOID)memset_s(&meInfo, sizeof(meInfo), 0x00, sizeof(TAF_PH_Meinfo));

    (VOS_VOID)memset_s(tmp, sizeof(tmp), 0x00, sizeof(tmp));

    /* 读取Model信息 */
    if ((drvAgentMsidQryCnf == VOS_NULL_PTR) || (drvAgentMsidQryCnf->result != DRV_AGENT_MSID_QRY_NO_ERROR)) {
        return AT_ERROR;
    }

    /* 读取Manufacturer信息 */
    dataLen = TAF_MAX_MFR_ID_LEN + 1;
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_FMRID, tmp, dataLen) != NV_OK) {
        AT_WARN_LOG("AT_FormatAtiCmdQryString:WARNING:NVIM Read NV_ITEM_FMRID falied!");
        return AT_ERROR;
    } else {
        memResult = memcpy_s(meInfo.fmrId.mfrId, sizeof(meInfo.fmrId.mfrId), tmp, dataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(meInfo.fmrId.mfrId), dataLen);
    }

    /* 读取IMEI信息 */
    for (i = 0; i < TAF_PH_IMEI_LEN; i++) {
        meInfo.imeisV.imei[i] = drvAgentMsidQryCnf->imei[i] + AT_HEX_CONVERT_ASCII_NUM_BASE_VALUE;
    }
    /* checkdata和结尾符单独赋值，所以减2，一次循环赋i和i+1的值，所以循环加2 */
    for (i = 0; i < (TAF_PH_IMEI_LEN - 2); i += 2) {
        checkData += (TAF_UINT8)(((drvAgentMsidQryCnf->imei[i]) +
                                   ((drvAgentMsidQryCnf->imei[i + 1UL] * AT_DOUBLE_LENGTH) / AT_DECIMAL_BASE_NUM)) +
                                  ((drvAgentMsidQryCnf->imei[i + 1UL] * AT_DOUBLE_LENGTH) % AT_DECIMAL_BASE_NUM));
    }
    checkData = (AT_DECIMAL_BASE_NUM - (checkData % AT_DECIMAL_BASE_NUM)) % AT_DECIMAL_BASE_NUM;

    meInfo.imeisV.imei[TAF_PH_IMEI_LEN - 2] = checkData + AT_HEX_CONVERT_ASCII_NUM_BASE_VALUE;
    meInfo.imeisV.imei[TAF_PH_IMEI_LEN - 1] = 0;

    /* 输出ATI命令返回结果 */
    dataLen = 0;
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s: %s\r\n", "Manufacturer", meInfo.fmrId.mfrId);
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s: %s\r\n", "Model", drvAgentMsidQryCnf->modelId);
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s: %s\r\n", "Revision", drvAgentMsidQryCnf->softwareVerId);
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s: %s\r\n", "IMEI", meInfo.imeisV.imei);
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s", "+GCAP: +CGSM,+DS,+ES");

    g_atSendDataBuff.bufLen = dataLen;

    return AT_OK;
}

VOS_UINT32 AT_RcvDrvAgentMsidQryCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg        *rcvMsg             = VOS_NULL_PTR;
    DRV_AGENT_MsidQryCnf *drvAgentMsidQryCnf = VOS_NULL_PTR;
    VOS_UINT32            result;
    VOS_UINT8             indexNum = 0;
    ModemIdUint16         modemId = MODEM_ID_0;
    VOS_UINT32            ret;

    /* 初始化 */
    rcvMsg             = (DRV_AGENT_Msg *)msg;
    drvAgentMsidQryCnf = (DRV_AGENT_MsidQryCnf *)(rcvMsg->content);

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(drvAgentMsidQryCnf->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentMsidQryCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentMsidQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }
    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_MSID_READ) {
        AT_StopTimerCmdReady(indexNum);
        g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_MSID;
        result            = AT_FormatAtiCmdQryString(modemId, drvAgentMsidQryCnf);
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGMM_READ) {
        AT_StopTimerCmdReady(indexNum);
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s", drvAgentMsidQryCnf->modelId);
        result                  = AT_OK;
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGMR_READ) {
        AT_StopTimerCmdReady(indexNum);
        drvAgentMsidQryCnf->softwareVerId[TAF_MAX_REVISION_ID_LEN] = '\0';
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s", drvAgentMsidQryCnf->softwareVerId);
        result                  = AT_OK;
    } else {
        return VOS_ERR;
    }

    ret = AT_GetModemIdFromClient(indexNum, &modemId);

    if (ret != VOS_OK) {
        g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_BUTT;

        AT_ERR_LOG("At_SetImeiPara: Get modem id fail.");
        return VOS_ERR;
    }

    if (result == AT_ERROR) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
    } else {
        At_FormatResultData(indexNum, AT_OK);
    }

    g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_BUTT;

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentHardwareQryRsp(struct MsgCB *msg)
{
    VOS_UINT32                ret;
    TAF_UINT16                length;
    VOS_UINT8                 indexNum    = 0;
    DRV_AGENT_Msg            *rcvMsg      = VOS_NULL_PTR;
    DRV_AGENT_HardwareQryCnf *hardWareCnf = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg      = (DRV_AGENT_Msg *)msg;
    hardWareCnf = (DRV_AGENT_HardwareQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(hardWareCnf->atAppCtrl.clientId, AT_CMD_DRV_AGENT_HARDWARE_QRY, &indexNum)
        != VOS_OK) {
        return VOS_ERR;
    }

    length                  = 0;
    g_atSendDataBuff.bufLen = 0;

    AT_StopTimerCmdReady(indexNum);

    if (hardWareCnf->result == DRV_AGENT_NO_ERROR) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\"", hardWareCnf->hwVer);

        g_atSendDataBuff.bufLen = length;
        ret                     = AT_OK;
    } else {
        ret = AT_ERROR;
    }

    At_FormatResultData(indexNum, ret);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentFullHardwareQryRsp(struct MsgCB *msg)
{
    TAF_UINT16                    length;
    VOS_UINT8                     indexNum = 0;
    DRV_AGENT_Msg                *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_FullHardwareQryCnf *event    = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_FullHardwareQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_DRV_AGENT_FULL_HARDWARE_QRY, &indexNum)
        != VOS_OK) {
        return VOS_ERR;
    }

    length                  = 0;
    g_atSendDataBuff.bufLen = 0;

    AT_StopTimerCmdReady(indexNum);

    if (event->result == DRV_AGENT_NO_ERROR) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s ", event->modelId);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s,", event->revisionId);

        event->hwVer[DRV_AGENT_MAX_HARDWARE_LEN] = '\0';
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s\"", event->hwVer);

        g_atSendDataBuff.bufLen = length;

        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentPfverQryRsp(struct MsgCB *msg)
{
    VOS_UINT32             ret;
    VOS_UINT8              indexNum = 0;
    DRV_AGENT_PfverQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg         *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_PfverQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentPfverQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentPfverQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待PFVER查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PFVER_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_PFVER_QRY_NO_ERROR) {
        /*
         * 设置错误码为AT_OK           构造结构为<CR><LF>^PFVER: <PfVer>,<VerTime> <CR><LF>
         * <CR><LF>OK<CR><LF>格式
         */
        ret                     = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:\"%s %s\"", g_parseContext[indexNum].cmdElement->cmdName,
            event->pfverInfo.pfVer, event->pfverInfo.verTime);

    } else {
        /* 查询失败返回ERROR字符串 */
        ret                     = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

