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
#include "at_custom_taf_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_event_report.h"

#include "at_custom_comm.h"
#include "at_lte_common.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_TAF_RSLT_PROC_C

VOS_UINT32 AT_RcvMmaExchangeModemInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ExchangeModemInfoCnf *exchangeNvimCnf = (TAF_MMA_ExchangeModemInfoCnf *)msg;

    if (exchangeNvimCnf->rslt == VOS_OK) {
        return AT_OK;
    } else {
        return AT_CME_UNKNOWN;
    }
}

VOS_UINT32 AT_RcvMtaQryXcposrRptCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg             *mtaMsg          = VOS_NULL_PTR;
    MTA_AT_QryXcposrrptCnf *qryXcposrRptCnf = VOS_NULL_PTR;
    VOS_UINT32              result;

    /* 初始化消息变量 */
    mtaMsg          = (AT_MTA_Msg *)msg;
    qryXcposrRptCnf = (MTA_AT_QryXcposrrptCnf *)mtaMsg->content;

    /* 判断查询操作是否成功 */
    if (qryXcposrRptCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            qryXcposrRptCnf->xcposrRptFlg);
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}

VOS_UINT32 AT_RcvMtaQryXcposrCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg          *mtaMsg       = VOS_NULL_PTR;
    MTA_AT_QryXcposrCnf *qryXcposrCnf = VOS_NULL_PTR;
    VOS_UINT32           result;

    /* 初始化消息变量 */
    mtaMsg       = (AT_MTA_Msg *)msg;
    qryXcposrCnf = (MTA_AT_QryXcposrCnf *)mtaMsg->content;

    /* 判断查询操作是否成功 */
    if (qryXcposrCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            qryXcposrCnf->xcposrEnableCfg);
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}

VOS_UINT32 AT_RcvMtaMipiInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_RfLcdMipiclkCnf *mipiClkCnf = (MTA_AT_RfLcdMipiclkCnf *)mtaMsg->content;
    VOS_UINT32              result;

    g_atSendDataBuff.bufLen = 0;
    if (mipiClkCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        result = AT_OK;

        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                mipiClkCnf->mipiClk);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaQryBestFreqCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *mtaMsg      = VOS_NULL_PTR;
    MTA_AT_BestfreqQryCnf *bestFreqCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT32             loop;
    VOS_UINT16             length = 0;

    /* 初始化消息变量 */
    mtaMsg      = (AT_MTA_Msg *)msg;
    bestFreqCnf = (MTA_AT_BestfreqQryCnf *)mtaMsg->content;

    if (bestFreqCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ERROR;
    } else {
        result = AT_OK;

        bestFreqCnf->activeDeviceNum = AT_MIN(bestFreqCnf->activeDeviceNum, MTA_AT_MAX_BESTFREQ_GROUPNUM);
        for (loop = 0; loop < bestFreqCnf->activeDeviceNum; loop++) {
            if (loop != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, bestFreqCnf->deviceInfoList[loop].groupId,
                bestFreqCnf->deviceInfoList[loop].deviceId, bestFreqCnf->deviceInfoList[loop].caseId,
                bestFreqCnf->deviceInfoList[loop].mode);
        }
    }

    g_atSendDataBuff.bufLen = length;

    return result;
}

VOS_UINT32 AT_RcvMtaQryCrrconnCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg           *mtaMsg        = VOS_NULL_PTR;
    MTA_AT_QryCrrconnCnf *qryCrrconnCnf = VOS_NULL_PTR;
    VOS_UINT32            result;

    /* 初始化消息变量 */
    mtaMsg        = (AT_MTA_Msg *)msg;
    qryCrrconnCnf = (MTA_AT_QryCrrconnCnf *)mtaMsg->content;

    /* 判断查询操作是否成功 */
    if (qryCrrconnCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            qryCrrconnCnf->enable, qryCrrconnCnf->status0, qryCrrconnCnf->status1, qryCrrconnCnf->status2);
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}
VOS_UINT32 AT_RcvTafPsEvtConfigVTFlowRptCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_ConfigVtflowRptCnf *evtMsg = VOS_NULL_PTR;

    evtMsg = (TAF_PS_ConfigVtflowRptCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VTFLOWRPT_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, evtMsg->cause);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetDataSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SET_DATA_SWITCH_CNF_STRU *setDataSwitchCnf = VOS_NULL_PTR;
    setDataSwitchCnf                                  = (TAF_PS_SET_DATA_SWITCH_CNF_STRU *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DATASWITCH_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setDataSwitchCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetDataSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetDataSwitchCnf *getDataSwitchCnf = VOS_NULL_PTR;
    getDataSwitchCnf                          = (TAF_PS_GetDataSwitchCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DATASWITCH_QRY) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    /* 上报查询结果 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            getDataSwitchCnf->dataSwitch);

    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetDataRoamSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SET_DATA_ROAM_SWITCH_CNF_STRU *setDataRoamSwitchCnf = VOS_NULL_PTR;
    setDataRoamSwitchCnf                                       = (TAF_PS_SET_DATA_ROAM_SWITCH_CNF_STRU *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DATAROAMSWITCH_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setDataRoamSwitchCnf->cause);
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafPsEvtGetDataRoamSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetDataRoamSwitchCnf *getDataSwitchCnf = VOS_NULL_PTR;
    getDataSwitchCnf                              = (TAF_PS_GetDataRoamSwitchCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DATAROAMSWITCH_QRY) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    /* 上报查询结果 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            getDataSwitchCnf->dataRoamSwitch);

    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
/*
 * 功能描述: ID_MSG_TAF_PS_GET_LTE_ATTACH_INFO_CNF事件处理函数
 */
VOS_UINT32 AT_RcvTafGetLteAttachInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                     memResult;
    TAF_PS_GetLteAttachInfoCnf *getLteAttachInfoCnf      = VOS_NULL_PTR;
    VOS_UINT16                  length                   = 0;
    VOS_UINT32                  result                   = AT_ERROR;
    VOS_UINT8                   str[TAF_MAX_APN_LEN + 1] = {0};

    getLteAttachInfoCnf = (TAF_PS_GetLteAttachInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_LTEATTACHINFO_QRY) {
        AT_WARN_LOG("AT_RcvTafGetLteAttachInfoCnf : Current Option is not AT_CMD_LTEATTACHINFO_QRY.");
        return VOS_ERR;
    }

    /* 如果获取成功则填充信息 */
    if (getLteAttachInfoCnf->cause == TAF_PS_CAUSE_SUCCESS) {
        /* ^LTEATTACHINFO:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <PDP_type> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", getLteAttachInfoCnf->lteAttQueryInfo.pdpType);

        /* <APN> */
        getLteAttachInfoCnf->lteAttQueryInfo.apn.length =
            TAF_MIN(getLteAttachInfoCnf->lteAttQueryInfo.apn.length,
                    sizeof(getLteAttachInfoCnf->lteAttQueryInfo.apn.value));

        if (getLteAttachInfoCnf->lteAttQueryInfo.apn.length > 0) {
            memResult = memcpy_s(str, sizeof(str), getLteAttachInfoCnf->lteAttQueryInfo.apn.value,
                                 getLteAttachInfoCnf->lteAttQueryInfo.apn.length);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(str), getLteAttachInfoCnf->lteAttQueryInfo.apn.length);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", str);

        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLendcQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg         *rcvMsg      = VOS_NULL_PTR;
    MTA_AT_LendcQryCnf *lendcQryCnf = VOS_NULL_PTR;
    VOS_UINT32          result;

    /* 初始化 */
    rcvMsg      = (AT_MTA_Msg *)msg;
    lendcQryCnf = (MTA_AT_LendcQryCnf *)rcvMsg->content;
    result      = AT_ERROR;
    g_atSendDataBuff.bufLen = 0;

    if (lendcQryCnf->rslt == MTA_AT_RESULT_NO_ERROR) {
        /* ^LEND: <enable>,<endc_available>,<endc_plmn_avail>,<endc_restricted>,<nr_pscell> */
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                lendcQryCnf->reportFlag, lendcQryCnf->lendcInfo.endcAvaliableFlag,
                lendcQryCnf->lendcInfo.endcPlmnAvaliableFlag, lendcQryCnf->lendcInfo.endcEpcRestrictedFlag,
                lendcQryCnf->lendcInfo.nrPscellFlag);

        result = AT_OK;
    } else {
        result = AT_ConvertMtaResult(lendcQryCnf->rslt);
    }

    return result;
}

VOS_UINT32 AT_RcvTafPsEvtSetUePolicyRptCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetUePolicyRptCnf *setUePolicyRptCnf = VOS_NULL_PTR;

    setUePolicyRptCnf = (TAF_PS_SetUePolicyRptCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CPOLICYRPT_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setUePolicyRptCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_ChkTafPsEvtGetUePolicyCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetUePolicyCnf     *uePolicyCnf = VOS_NULL_PTR;
    const TAF_PS_Evt                *event = VOS_NULL_PTR;
    VOS_UINT32                       length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetUePolicyCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetUePolicyCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event       = (const TAF_PS_Evt *)msg;
    uePolicyCnf = (const TAF_PS_GetUePolicyCnf *)(event->content);
    length     += uePolicyCnf->length;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetUePolicyCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_RcvTafPsEvtGetUePolicyCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetUePolicyCnf *getUePolicytCnf = VOS_NULL_PTR;
    VOS_UINT16             length;

    getUePolicytCnf = (TAF_PS_GetUePolicyCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CPOLICYCODE_QRY) {
        return VOS_ERR;
    }

    if (getUePolicytCnf->cause != TAF_PS_CAUSE_SUCCESS) {
        /* 处理错误码 */
        AT_PrcoPsEvtErrCode(indexNum, getUePolicytCnf->cause);

        return VOS_OK;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d,", g_atCrLf, g_parseContext[indexNum].cmdElement->cmdName,
        getUePolicytCnf->index);

    /* 将16进制数转换为ASCII码 */
    length += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                       (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                       getUePolicytCnf->length, getUePolicytCnf->content);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

#if (FEATURE_LADN == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLadnInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg           = (AT_MTA_Msg *)msg;
    MTA_AT_QryLadnInfoCnf *qryCnf           = VOS_NULL_PTR;
    VOS_UINT32             rslt             = AT_ERROR;
    VOS_UINT16             length           = 0;
    VOS_UINT8              allowedDnnNum    = 0;
    VOS_UINT8              nonAllowedDnnNum = 0;
    VOS_UINT8              allowedDnnList[MTA_AT_MAX_LADN_DNN_NUM][PS_MAX_APN_LEN];
    VOS_UINT8              nonAllowedDnnList[MTA_AT_MAX_LADN_DNN_NUM][PS_MAX_APN_LEN];
    VOS_UINT32             i;
    errno_t                memResult;
    VOS_UINT8              strLen;
    VOS_UINT8              srcStr[PS_MAX_APN_LEN + 1] = {0}; /* 网络格式DNN */
    VOS_UINT8              destStr[PS_MAX_APN_LEN] = {0};    /* 字符串格式DNN */

    qryCnf = (MTA_AT_QryLadnInfoCnf *)rcvMsg->content;

    /* 如果查询结果失败，直接返回ERROR */
    if (qryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        g_atSendDataBuff.bufLen = 0;
    } else {
        for (i = 0; i < MTA_AT_MAX_LADN_DNN_NUM; i++) {
            (VOS_VOID)memset_s(allowedDnnList[i], sizeof(allowedDnnList[i]), 0x00, sizeof(allowedDnnList[i]));
            (VOS_VOID)memset_s(nonAllowedDnnList[i], sizeof(nonAllowedDnnList[i]), 0x00, sizeof(nonAllowedDnnList[i]));
        }

        for (i = 0; i < AT_MIN(qryCnf->ladnDnnNum, MTA_AT_MAX_LADN_DNN_NUM); i++) {
            /* 转换网络格式DNN为字符串 */
            srcStr[0] = qryCnf->ladnList[i].dnn.length;
            strLen    = AT_MIN(PS_MAX_APN_LEN, srcStr[0]);
            if (strLen == 0) {
                continue;
            }
            memResult = memcpy_s(&srcStr[1], sizeof(srcStr) - 1, qryCnf->ladnList[i].dnn.value, strLen);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(srcStr) - 1, strLen);
            AT_ConvertNwDnnToString(srcStr, PS_MAX_APN_LEN, destStr, PS_MAX_APN_LEN);

            /* 根据DNN是否可用，将DNN分别拷贝至可用和不可用DNN的数组，并统计数量 */
            if (qryCnf->ladnList[i].availFlg == VOS_TRUE) {
                memResult = memcpy_s(allowedDnnList[allowedDnnNum], sizeof(allowedDnnList[allowedDnnNum]), &destStr[1],
                                     PS_MAX_APN_LEN - 1);
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(allowedDnnList[allowedDnnNum]), PS_MAX_APN_LEN - 1);
                allowedDnnNum++;
            } else {
                memResult = memcpy_s(nonAllowedDnnList[nonAllowedDnnNum], sizeof(nonAllowedDnnList[nonAllowedDnnNum]),
                                     &destStr[1], PS_MAX_APN_LEN - 1);
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(nonAllowedDnnList[nonAllowedDnnNum]), PS_MAX_APN_LEN - 1);
                nonAllowedDnnNum++;
            }
            (VOS_VOID)memset_s(destStr, sizeof(destStr), 0x00, sizeof(destStr));
        }
        /* ^CLADN : */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <n>, */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", qryCnf->rptFlg);
        /* <allowed_ladn_dnn_num>,<allowed_dnn_list> */
        length = AT_PrintLadnDnn(allowedDnnNum, allowedDnnList, length);

        /* , */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");

        /* <nonallowed_ladn_dnn_num>,<noallowed_dnn_list> */
        length = AT_PrintLadnDnn(nonAllowedDnnNum, nonAllowedDnnList, length);

        g_atSendDataBuff.bufLen = length;
        rslt = AT_OK;
    }

    return rslt;
}
#endif

VOS_UINT32 AT_RcvTafPsEvtSetCgContExCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetCgContExCnf *setCnf =  (TAF_PS_SetCgContExCnf *)evtInfo;

    AT_PrcoPsEvtErrCode(indexNum, setCnf->cause);

    return VOS_OK;
}

#endif

VOS_UINT32 AT_RcvMtaCsqlvlQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    errno_t              memResult;
    AT_MTA_Msg          *rcvMsg           = VOS_NULL_PTR;
    MTA_AT_CsqlvlQryCnf *pstrCsqlvlQryCnf = VOS_NULL_PTR;
    VOS_UINT32           result = AT_ERROR;
    AT_MTA_CsqlvlPara    csqlvl;
    AT_MTA_CsqlvlextPara csqlvlext;

    /* 初始化 */
    rcvMsg           = (AT_MTA_Msg *)msg;
    pstrCsqlvlQryCnf = (MTA_AT_CsqlvlQryCnf *)rcvMsg->content;

    memResult = memcpy_s(&csqlvlext, sizeof(csqlvlext), &pstrCsqlvlQryCnf->csqLvlExtPara, sizeof(AT_MTA_CsqlvlextPara));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(csqlvlext), sizeof(AT_MTA_CsqlvlextPara));
    csqlvl = pstrCsqlvlQryCnf->csqLvlPara;

    if (pstrCsqlvlQryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        return result;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CSQLVL_SET) {
        result = AT_OK;
        /* 根据MMA上报的查询结果显示Csqlvl到串口； */
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            csqlvl.level, csqlvl.rscp);
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CSQLVLEXT_SET) {
        result = AT_OK;
        /* 根据MMA上报的查询结果显示Csqlvl到串口； */
        /* 根据MMA上报的查询结果显示CsqlvlExt到串口； */
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            csqlvlext.rssilv, csqlvlext.ber);
    }

    return result;
}

VOS_UINT32 AT_RcvDrvAgentVertimeQryRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg         *rcvMsg              = VOS_NULL_PTR;
    DRV_AGENT_VersionTime *drvAgentVersionTime = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT8              indexNum = 0;

    /* 初始化 */
    rcvMsg              = (DRV_AGENT_Msg *)msg;
    drvAgentVersionTime = (DRV_AGENT_VersionTime *)(rcvMsg->content);
    result              = AT_OK;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(drvAgentVersionTime->atAppCtrl.clientId, AT_CMD_VERSIONTIME_READ, &indexNum)
        != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^VERTIME命令返回 */
    g_atSendDataBuff.bufLen =
        (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:%s", g_parseContext[indexNum].cmdElement->cmdName,
            drvAgentVersionTime->data);

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaRrcStatQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RrcstatQryCnf *rrcStatQryCnf = (TAF_MMA_RrcstatQryCnf *)msg;
    VOS_UINT16             length = 0;

    if (rrcStatQryCnf->rslt == TAF_ERR_ERROR) {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }

    if (rrcStatQryCnf->rrcCampStat.netRat == TAF_MMA_RAT_LTE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            rrcStatQryCnf->reportFlg, rrcStatQryCnf->rrcCampStat.rrcStat,
            rrcStatQryCnf->rrcCampStat.tempCampStat);

        g_atSendDataBuff.bufLen = length;
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            rrcStatQryCnf->reportFlg, rrcStatQryCnf->rrcCampStat.rrcStat);

        g_atSendDataBuff.bufLen = length;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaHfreqinfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg           = VOS_NULL_PTR;
    MTA_AT_HfreqinfoQryCnf *mtaAtQryHfreqCnf = VOS_NULL_PTR;
    VOS_UINT32              result;
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32 loop;
#endif
    VOS_UINT16 length;

    rcvMsg           = (AT_MTA_Msg *)msg;
    mtaAtQryHfreqCnf = (MTA_AT_HfreqinfoQryCnf *)rcvMsg->content;
    length = 0;
    result = AT_ConvertMtaResult(mtaAtQryHfreqCnf->result);

    if (mtaAtQryHfreqCnf->result == MTA_AT_RESULT_NO_ERROR) {
        if (mtaAtQryHfreqCnf->resultType != MTA_AT_RESULT_TYPE_NR) {
            /* LTE */
            length +=
                (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d,%d,%d,%d",
                    g_parseContext[indexNum].cmdElement->cmdName, mtaAtQryHfreqCnf->isreportFlg, HFREQ_INFO_RAT_LTE,
                    mtaAtQryHfreqCnf->lteHfreqInfo.band, mtaAtQryHfreqCnf->lteHfreqInfo.dlEarfcn,
                    mtaAtQryHfreqCnf->lteHfreqInfo.dlFreq, mtaAtQryHfreqCnf->lteHfreqInfo.dlBandWidth,
                    mtaAtQryHfreqCnf->lteHfreqInfo.ulEarfcn, mtaAtQryHfreqCnf->lteHfreqInfo.ulFreq,
                    mtaAtQryHfreqCnf->lteHfreqInfo.ulBandWidth);
        }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (mtaAtQryHfreqCnf->resultType == MTA_AT_RESULT_TYPE_DC) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        if (mtaAtQryHfreqCnf->resultType != MTA_AT_RESULT_TYPE_LTE) {
            /* NR上行暂不支持并发 */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                mtaAtQryHfreqCnf->isreportFlg, HFREQ_INFO_RAT_NR);

            for (loop = 0; loop < (VOS_UINT32)AT_MIN(mtaAtQryHfreqCnf->nrCellNum, MTA_AT_MAX_CC_NUMBER); loop++) {
                if (mtaAtQryHfreqCnf->nrHfreqInfo[loop].ulMode == MTA_AT_UL_MODE_SUL_ONLY) {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", mtaAtQryHfreqCnf->nrHfreqInfo[loop].sulBand);
                } else {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", mtaAtQryHfreqCnf->nrHfreqInfo[loop].band);
                }

                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d", mtaAtQryHfreqCnf->nrHfreqInfo[loop].dlNarfcn,
                    mtaAtQryHfreqCnf->nrHfreqInfo[loop].dlFreq, mtaAtQryHfreqCnf->nrHfreqInfo[loop].dlBandWidth);

                if (mtaAtQryHfreqCnf->nrHfreqInfo[loop].ulMode == MTA_AT_UL_MODE_SUL_ONLY) {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d",
                        mtaAtQryHfreqCnf->nrHfreqInfo[loop].sulNarfcn, mtaAtQryHfreqCnf->nrHfreqInfo[loop].sulFreq,
                        mtaAtQryHfreqCnf->nrHfreqInfo[loop].sulBandWidth);
                } else {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d",
                        mtaAtQryHfreqCnf->nrHfreqInfo[loop].ulNarfcn, mtaAtQryHfreqCnf->nrHfreqInfo[loop].ulFreq,
                        mtaAtQryHfreqCnf->nrHfreqInfo[loop].ulBandWidth);
                }
            }
        }
#endif
    }

    g_atSendDataBuff.bufLen = length;

    return result;
}

VOS_UINT32 AT_RcvTafPsEvtSetImsPdpCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetImsPdpCfgCnf *setImsPdpCfgCnf = VOS_NULL_PTR;

    setImsPdpCfgCnf = (TAF_PS_SetImsPdpCfgCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_IMSPDPCFG_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setImsPdpCfgCnf->cause);

    return VOS_OK;
}

VOS_VOID AT_ShowCccRst(NAS_CC_StateInfo *ccState, VOS_UINT16 *length)
{
    VOS_UINT16 lengthTemp;

    lengthTemp = *length;

    lengthTemp += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + lengthTemp, "%d,", ccState->callId);

    lengthTemp += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + lengthTemp, "%d,", ccState->ti);

    lengthTemp += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + lengthTemp, "%d,", ccState->callState);

    lengthTemp += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + lengthTemp, "%d,", ccState->holdAuxState);

    lengthTemp += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + lengthTemp, "%d", ccState->mptyAuxState);

    lengthTemp += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + lengthTemp, "%s", g_atCrLf);

    *length = lengthTemp;
}

VOS_UINT32 At_RcvAtCcMsgStateQryCnfProc(struct MsgCB *msg)
{
    AT_CC_StateQryCnfMsg *atCcStateQryCnfMsg = VOS_NULL_PTR;
    VOS_UINT32            result;
    VOS_UINT8             indexNum = 0;
    VOS_UINT32            i;
    VOS_UINT16            length;

    /* 初始化 */
    atCcStateQryCnfMsg = (AT_CC_StateQryCnfMsg *)msg;
    result             = AT_OK;
    length             = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(atCcStateQryCnfMsg->atAppCtrl.clientId, AT_CMD_CC_STATE_QUERY, &indexNum)
        != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^CCC查询命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (atCcStateQryCnfMsg->callNum == 0) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
        atCcStateQryCnfMsg->callNum = AT_MIN(atCcStateQryCnfMsg->callNum, MN_CALL_MAX_NUM);
        for (i = 0; i < atCcStateQryCnfMsg->callNum; i++) {
            AT_ShowCccRst(&atCcStateQryCnfMsg->ccStateInfoList[i], &length);
        }

        if (length == 0) {
            result = AT_ERROR;
        }
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_LTE_MBMS == FEATURE_ON)
VOS_UINT32 AT_RcvMtaMBMSSib16NetworkTimeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_MbmsSib16NetworkTimeQryCnf *mtaCnf = (MTA_AT_MbmsSib16NetworkTimeQryCnf *)mtaMsg->content;
    VOS_UINT64 *utc = VOS_NULL_PTR;
    VOS_UINT8   utcInfo[AT_MBMS_UTC_MAX_LENGTH + 1];
    VOS_UINT32  result = AT_OK;
    VOS_INT32   bufLen;

    (VOS_VOID)memset_s(utcInfo, sizeof(utcInfo), 0x00, sizeof(utcInfo));

    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        utc    = (VOS_UINT64 *)mtaCnf->utc;
        bufLen = snprintf_s((VOS_CHAR *)utcInfo, AT_MBMS_UTC_MAX_LENGTH + 1, AT_MBMS_UTC_MAX_LENGTH, "%llu", *utc);
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, AT_MBMS_UTC_MAX_LENGTH + 1, AT_MBMS_UTC_MAX_LENGTH);
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName, utcInfo);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaMBMSBssiSignalLevelQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_MbmsBssiSignalLevelQryCnf *mtaCnf = (MTA_AT_MbmsBssiSignalLevelQryCnf *)mtaMsg->content;
    VOS_UINT32 result = AT_OK;

    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            (VOS_INT32)mtaCnf->bssiLevel);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaMBMSNetworkInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_MbmsNetworkInfoQryCnf *mtaCnf = (MTA_AT_MbmsNetworkInfoQryCnf *)mtaMsg->content;
    VOS_UINT32  result = AT_OK;

    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                (VOS_INT32)mtaCnf->cellId);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaMBMSModemStatusQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_EmbmsStatusQryCnf *mtaCnf = (MTA_AT_EmbmsStatusQryCnf *)mtaMsg->content;
    VOS_UINT32                result = AT_OK;

    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                (VOS_INT32)mtaCnf->status);
    }

    return result;
}

VOS_VOID AT_ReportMBMSCmdQryCnf(MTA_AT_MbmsAvlServiceListQryCnf *mtaCnf, VOS_UINT8 indexNum)
{
    VOS_UINT32 listNum;
    VOS_UINT16 length = 0;
    VOS_UINT8  serviceID[AT_MBMS_SERVICE_ID_LENGTH + 1];

    for (listNum = 0; listNum < TAF_MIN(mtaCnf->avlServiceNum, AT_MTA_MBMS_AVL_SERVICE_MAX_NUM); listNum++) {
        /* 获得MBMS Service ID字符串形式 */
        (VOS_VOID)memset_s(serviceID, sizeof(serviceID), 0x00, sizeof(serviceID));
        At_ul2Auc(mtaCnf->avlServices[listNum].tmgi.mbmsSerId, AT_MBMS_SERVICE_ID_LENGTH, serviceID);

        /* ^MBMSCMD: <AreaID>,<TMGI>:MBMS Service ID */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%s", g_parseContext[indexNum].cmdElement->cmdName,
            (VOS_INT32)mtaCnf->avlServices[listNum].areaId, serviceID);

        /* <TMGI>:Mcc */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%x%x%x",
            (mtaCnf->avlServices[listNum].tmgi.plmnId.mcc & 0x0f00) >> 8,
            (mtaCnf->avlServices[listNum].tmgi.plmnId.mcc & 0xf0) >> 4,
            (mtaCnf->avlServices[listNum].tmgi.plmnId.mcc & 0x0f));
        /* <TMGI>:Mnc */
        if ((mtaCnf->avlServices[listNum].tmgi.plmnId.mnc & 0x0f00) == 0x0f00) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%x%x",
                (mtaCnf->avlServices[listNum].tmgi.plmnId.mnc & 0xf0) >> 4,
                (mtaCnf->avlServices[listNum].tmgi.plmnId.mnc & 0x0f));
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%x%x%x",
                (mtaCnf->avlServices[listNum].tmgi.plmnId.mnc & 0x0f00) >> 8,
                (mtaCnf->avlServices[listNum].tmgi.plmnId.mnc & 0xf0) >> 4,
                (mtaCnf->avlServices[listNum].tmgi.plmnId.mnc & 0x0f));
        }

        if (mtaCnf->avlServices[listNum].opSessionId == VOS_TRUE) {
            /* <SessionID> */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", (VOS_INT32)mtaCnf->avlServices[listNum].sessionId);
        }

        if (listNum != (mtaCnf->avlServiceNum - 1)) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_RcvMtaMBMSCmdQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_MbmsAvlServiceListQryCnf *mtaCnf = (MTA_AT_MbmsAvlServiceListQryCnf *)mtaMsg->content;
    VOS_UINT32  result = AT_OK;

    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        AT_ReportMBMSCmdQryCnf(mtaCnf, indexNum);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaMBMSServiceEventInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                 *rcvMsg   = VOS_NULL_PTR;
    MTA_AT_MbmsServiceEventInd *mtaAtInd = VOS_NULL_PTR;
    VOS_UINT16                  length;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    mtaAtInd = (MTA_AT_MbmsServiceEventInd *)rcvMsg->content;
    length   = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s %d%s", g_atCrLf, g_atStringTab[AT_STRING_MBMSEV].text,
        mtaAtInd->event, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif
#endif

VOS_UINT32 AT_RcvDrvAgentAuthVerQryRsp(struct MsgCB *msg)
{
    VOS_UINT32               ret;
    VOS_UINT8                indexNum = 0;
    DRV_AGENT_AuthverQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_AuthverQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentAuthVerQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentAuthVerQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待AUTHVER查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_AUTHVER_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_AUTHVER_QRY_NO_ERROR) {
        /*
         * 设置错误码为AT_OK           构造结构为<CR><LF>^ AUTHVER: <value> <CR><LF>
         * <CR><LF>OK<CR><LF>格式
         */
        ret = AT_OK;
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName,
                event->simLockVersion);

    } else {
        /* 查询失败返回ERROR字符串 */
        ret                     = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 atQryCellInfoCnfProc(struct MsgCB *msgBlock)
{
    L4A_READ_CellInfoCnf *pstcnf = VOS_NULL_PTR;
    VOS_UINT32            i = 0;
    VOS_UINT32            cellNum = 0;
    VOS_UINT16            length = 0;

    pstcnf = (L4A_READ_CellInfoCnf *)msgBlock;

    if (pstcnf->errorCode != 0) {
        CmdErrProc((VOS_UINT8)(pstcnf->clientId), pstcnf->errorCode, 0, NULL);

        return AT_FW_CLIENT_STATUS_READY;
    }

    if (pstcnf->ncellListInfo.cellFlag == 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "%d:%d,%d,%d%s", 1, EN_SERVICE_CELL_ID,
            pstcnf->ncellListInfo.sevCellInfo.cellMeasInfo[0].measRslt.phyCellId,
            pstcnf->ncellListInfo.sevCellInfo.cellMeasInfo[0].measRslt.rssi, g_atCrLf);
    } else {
        /* 同频 */
        pstcnf->ncellListInfo.intraFreqNcellList.ncellNumber = AT_MIN(
            pstcnf->ncellListInfo.intraFreqNcellList.ncellNumber,
            LMAX_NEIGHBOR_CELL_NUM);
        for (i = 0; i < pstcnf->ncellListInfo.intraFreqNcellList.ncellNumber; i++) {
            cellNum++;
            length +=
                (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                    (VOS_CHAR *)g_atSndCodeAddr + length, "%d:%d,%d,%d%s", cellNum, EN_SYN_FREQ_CELL_ID,
                    pstcnf->ncellListInfo.intraFreqNcellList.cellMeasInfo[i].measRslt.phyCellId,
                    pstcnf->ncellListInfo.intraFreqNcellList.cellMeasInfo[i].measRslt.rssi, g_atCrLf);
        }
        /* 异频 */
        pstcnf->ncellListInfo.interFreqNcellList.ncellNumber = AT_MIN(
            pstcnf->ncellListInfo.interFreqNcellList.ncellNumber,
            LMAX_NEIGHBOR_CELL_NUM);
        for (i = 0; i < pstcnf->ncellListInfo.interFreqNcellList.ncellNumber; i++) {
            cellNum++;
            length +=
                (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                    (VOS_CHAR *)g_atSndCodeAddr + length, "%d:%d,%d,%d%s", cellNum, EN_ASYN_FREQ_CELL_ID,
                    pstcnf->ncellListInfo.interFreqNcellList.cellMeasInfo[i].measRslt.phyCellId,
                    pstcnf->ncellListInfo.interFreqNcellList.cellMeasInfo[i].measRslt.rssi, g_atCrLf);
        }
        /* W异频 */
        pstcnf->ncellListInfo.interRATUMTSNcellList.ncellNumber = AT_MIN(
            pstcnf->ncellListInfo.interRATUMTSNcellList.ncellNumber,
            LMAX_NEIGHBOR_CELL_NUM);
        for (i = 0; i < pstcnf->ncellListInfo.interRATUMTSNcellList.ncellNumber; i++) {
            cellNum++;
            length +=
                (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                    (VOS_CHAR *)g_atSndCodeAddr + length, "%d:%d,%d,%d%s", cellNum, EN_ASYN_UMTS_CELL_ID,
                    pstcnf->ncellListInfo.interRATUMTSNcellList.umtsNcellList[i].primaryScramCode,
                    pstcnf->ncellListInfo.interRATUMTSNcellList.umtsNcellList[i].cpichRscp, g_atCrLf);
        }
        /* G异频 */
        pstcnf->ncellListInfo.interRATGeranNcellList.ncellNumber = AT_MIN(
            pstcnf->ncellListInfo.interRATGeranNcellList.ncellNumber,
            LMAX_NEIGHBOR_CELL_NUM);
        for (i = 0; i < pstcnf->ncellListInfo.interRATGeranNcellList.ncellNumber; i++) {
            cellNum++;
            length += (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                (VOS_CHAR *)g_atSndCodeAddr + length, "%d:%d,%d,%d%s", cellNum, EN_ASYN_GSM_CELL_ID,
                (pstcnf->ncellListInfo.interRATGeranNcellList.geranNcellList[i].bsic.ncc) * AT_HEX_BASE_NUM *
                AT_HEX_BASE_NUM + pstcnf->ncellListInfo.interRATGeranNcellList.geranNcellList[i].bsic.bcc,
                pstcnf->ncellListInfo.interRATGeranNcellList.geranNcellList[i].rssi, g_atCrLf);
        }

        if (cellNum == 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                (VOS_CHAR *)g_atSndCodeAddr + length, "%d%s", cellNum, g_atCrLf);
        }
    }
    CmdErrProc((VOS_UINT8)(pstcnf->clientId), pstcnf->errorCode, length, g_atSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
LOCAL VOS_VOID AT_NcellListInfoProc(L4A_READ_CellInfoCnf *pstcnf, VOS_UINT32 *cellNum, VOS_UINT16 *length)
{
    VOS_UINT32            i = 0;

    /* 同频 */
    pstcnf->ncellListInfo.intraFreqNcellList.ncellNumber = AT_MIN(
        pstcnf->ncellListInfo.intraFreqNcellList.ncellNumber, LMAX_NEIGHBOR_CELL_NUM);
    for (i = 0; i < pstcnf->ncellListInfo.intraFreqNcellList.ncellNumber; i++) {
        (*cellNum)++;
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                  (VOS_CHAR *)g_atSndCodeAddr + (*length), "^LCELLINFO: %d,%d,%d,%d,%d%s", *cellNum, EN_SYN_FREQ_CELL_ID,
                  pstcnf->ncellListInfo.intraFreqNcellList.cellMeasInfo[i].measRslt.phyCellId,
                  pstcnf->ncellListInfo.intraFreqNcellList.cellMeasInfo[i].measRslt.rsrp,
                  pstcnf->ncellListInfo.intraFreqNcellList.cellMeasInfo[i].freqInfo, g_atCrLf);
    }
    /* 异频 */
    pstcnf->ncellListInfo.interFreqNcellList.ncellNumber = AT_MIN(
        pstcnf->ncellListInfo.interFreqNcellList.ncellNumber, LMAX_NEIGHBOR_CELL_NUM);
    for (i = 0; i < pstcnf->ncellListInfo.interFreqNcellList.ncellNumber; i++) {
        (*cellNum)++;
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                  (VOS_CHAR *)g_atSndCodeAddr + (*length), "^LCELLINFO: %d,%d,%d,%d,%d%s", *cellNum, EN_ASYN_FREQ_CELL_ID,
                  pstcnf->ncellListInfo.interFreqNcellList.cellMeasInfo[i].measRslt.phyCellId,
                  pstcnf->ncellListInfo.interFreqNcellList.cellMeasInfo[i].measRslt.rsrp,
                  pstcnf->ncellListInfo.interFreqNcellList.cellMeasInfo[i].freqInfo, g_atCrLf);
    }
    /* W异频 */
    pstcnf->ncellListInfo.interRATUMTSNcellList.ncellNumber = AT_MIN(
        pstcnf->ncellListInfo.interRATUMTSNcellList.ncellNumber, LMAX_NEIGHBOR_CELL_NUM);
    for (i = 0; i < pstcnf->ncellListInfo.interRATUMTSNcellList.ncellNumber; i++) {
        (*cellNum)++;
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                  (VOS_CHAR *)g_atSndCodeAddr + (*length), "^LCELLINFO: %d,%d,%d,%d,%d%s", *cellNum, EN_ASYN_UMTS_CELL_ID,
                  pstcnf->ncellListInfo.interRATUMTSNcellList.umtsNcellList[i].primaryScramCode,
                  pstcnf->ncellListInfo.interRATUMTSNcellList.umtsNcellList[i].cpichRscp,
                  pstcnf->ncellListInfo.interRATUMTSNcellList.umtsNcellList[i].arfcn, g_atCrLf);
    }
    /* G异频 */
    pstcnf->ncellListInfo.interRATGeranNcellList.ncellNumber = AT_MIN(
        pstcnf->ncellListInfo.interRATGeranNcellList.ncellNumber, LMAX_NEIGHBOR_CELL_NUM);
    for (i = 0; i < pstcnf->ncellListInfo.interRATGeranNcellList.ncellNumber; i++) {
        (*cellNum)++;
        (*length) += (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                  (VOS_CHAR *)g_atSndCodeAddr + (*length), "^LCELLINFO: %d,%d,%d,%d,%d%s", *cellNum, EN_ASYN_GSM_CELL_ID,
                  (pstcnf->ncellListInfo.interRATGeranNcellList.geranNcellList[i].bsic.ncc) * AT_HEX_BASE_NUM *
                  AT_HEX_BASE_NUM + pstcnf->ncellListInfo.interRATGeranNcellList.geranNcellList[i].bsic.bcc,
                  pstcnf->ncellListInfo.interRATGeranNcellList.geranNcellList[i].rssi,
                  pstcnf->ncellListInfo.interRATGeranNcellList.geranNcellList[i].arfcn, g_atCrLf);
    }
}
VOS_UINT32 AT_MbbQryCellInfoCnfProc(struct MsgCB *msgBlock)
{
    L4A_READ_CellInfoCnf *pstcnf = VOS_NULL_PTR;
    VOS_UINT32            cellNum = 0;
    VOS_UINT16            length = 0;

    pstcnf = (L4A_READ_CellInfoCnf *)msgBlock;

    if (pstcnf->errorCode != 0) {
        CmdErrProc((VOS_UINT8)(pstcnf->clientId), pstcnf->errorCode, 0, NULL);
        return AT_FW_CLIENT_STATUS_READY;
    }

    if (pstcnf->ncellListInfo.cellFlag == 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                  (VOS_CHAR *)g_atSndCodeAddr + length, "^LCELLINFO: %d,%d,%d,%d,%d%s", 1, EN_SERVICE_CELL_ID,
                  pstcnf->ncellListInfo.sevCellInfo.cellMeasInfo[0].measRslt.phyCellId,
                  pstcnf->ncellListInfo.sevCellInfo.cellMeasInfo[0].measRslt.rssi,
                  pstcnf->ncellListInfo.sevCellInfo.cellMeasInfo[0].freqInfo, g_atCrLf);
    } else {
        AT_NcellListInfoProc(pstcnf, &cellNum, &length);
        if (cellNum == 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                      (VOS_CHAR *)g_atSndCodeAddr + length, "^LCELLINFO: %d%s", cellNum, g_atCrLf);
        }
    }
    CmdErrProc((VOS_UINT8)(pstcnf->clientId), pstcnf->errorCode, length, g_atSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}
#endif

VOS_UINT32 AT_RcvMtaNvRefreshSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NvRefreshSetCnf *setCnf = VOS_NULL_PTR;
    VOS_UINT32              i;
    VOS_UINT16              length = 0;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    setCnf   = (MTA_AT_NvRefreshSetCnf *)rcvMsg->content;

    if (setCnf->result == VOS_OK) {
        return AT_ConvertMtaResult(setCnf->result);
    }

    setCnf->failedPidInfo.pidNum = AT_MIN(setCnf->failedPidInfo.pidNum, MTA_NV_REFRESH_FAIL_PID_MAX_NUM);
    switch (setCnf->result) {
        case MTA_AT_RESULT_OPTION_TIMEOUT:
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %s, %d", g_parseContext[indexNum].cmdElement->cmdName,
                "timeout", setCnf->failedPidInfo.pidNum);
            break;
        case MTA_AT_RESULT_ERROR:
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %s, %d", g_parseContext[indexNum].cmdElement->cmdName,
                "pid failed", setCnf->failedPidInfo.pidNum);
            break;
        default:
            break;
    }

    for (i = 0; i < setCnf->failedPidInfo.pidNum; i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ", %d", setCnf->failedPidInfo.pid[i]);
    }
    g_atSendDataBuff.bufLen = length;
    return AT_ConvertMtaResult(setCnf->result);
}

