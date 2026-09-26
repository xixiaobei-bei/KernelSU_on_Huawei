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
#include "at_custom_event_report.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "taf_type_def.h"
#include "at_lte_common.h"
#include "at_mdrv_interface.h"
#include "dms_file_node_i.h"
#include "at_custom_comm.h"
#include "css_at_interface.h"
#include "at_event_report.h"
#include "at_init.h"
#include "osm.h"
#include "at_custom_mm_rslt_proc.h"
#include "throt_at_interface.h"
#include "at_custom_lnas_rslt_proc.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "at_custom_lnas_qry_cmd_proc.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_EVENT_REPORT_C

#define AT_FINETIME_FAIL_NUM 2

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaSibFineTimeNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_SibFineTimeInd *sibFineTime = (MTA_AT_SibFineTimeInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen = 0;

    if (sibFineTime->result != VOS_OK) {
        /* "^FINETIMEFAIL: 1 */
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_FINETIMEFAIL].text, 1, g_atCrLf);
        /* 输出结果 */
        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    } else {
        /* "^FINETIMEINFO: */
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s%d,%d,%d,%d,%s,%s", g_atCrLf,
            g_atStringTab[AT_STRING_FINETIMEINFO].text, sibFineTime->rat, sibFineTime->ta, sibFineTime->sinr,
            sibFineTime->state, sibFineTime->utcTime, sibFineTime->utcTimeOffset);

        if (sibFineTime->leapSecondValid == VOS_TRUE) {
            g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, ",%d%s",
                sibFineTime->leapSecond, g_atCrLf);
        } else {
            g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s",
                g_atCrLf);
        }

        /* 输出结果 */
        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaLrrcUeCapNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                 *rcvMsg = VOS_NULL_PTR;
    MTA_AT_LrrcUeCapInfoNotify *ueCap  = VOS_NULL_PTR;
    VOS_UINT32                  loop;
    VOS_UINT32                  i;

    /* 初始化消息变量 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    ueCap    = (MTA_AT_LrrcUeCapInfoNotify *)rcvMsg->content;
    loop     = AT_MIN(ueCap->msgLen, MTA_AT_UE_CAP_INFO_IND_MAX_LEN);

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s%d,\"", g_atCrLf,
            g_atStringTab[AT_STRING_LRRCUECAPINFONTF].text, (ueCap->msgLen * AT_DOUBLE_LENGTH));

    for (i = 0; i < loop; i++) {
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%02X", ueCap->msg[i]);
    }

    g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "\"%s", g_atCrLf);

    /* 输出结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaLppFineTimeNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg      = (AT_MTA_Msg *)msg;
    MTA_AT_LppFineTimeInd *lppFineTime = (MTA_AT_LppFineTimeInd *)rcvMsg->content;

    g_atSendDataBuff.bufLen = 0;

    if (lppFineTime->result != VOS_OK) {
        /* "^FINETIMEFAIL: 2 */
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_FINETIMEFAIL].text, AT_FINETIME_FAIL_NUM, g_atCrLf);
        /* 输出结果 */
        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    } else {
        /* "^SFN: */
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_SFN].text, lppFineTime->sysFn, g_atCrLf);
        /* 输出结果 */
        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    }

    return VOS_OK;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNrrcUeCapNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                 *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NrrcUeCapInfoNotify *ueCap  = VOS_NULL_PTR;
    VOS_UINT32                  i;
    VOS_UINT32                  loop;

    /* 初始化消息变量 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    ueCap    = (MTA_AT_NrrcUeCapInfoNotify *)rcvMsg->content;
    loop     = AT_MIN(ueCap->msgLen, MTA_AT_UE_CAP_INFO_IND_MAX_LEN);

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s%d,\"", g_atCrLf,
            g_atStringTab[AT_STRING_NRRCUECAPINFONTF].text, (ueCap->msgLen * AT_DOUBLE_LENGTH));

    for (i = 0; i < loop; i++) {
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%02X", ueCap->msg[i]);
    }

    g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "\"%s", g_atCrLf);

    /* 输出结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_LADN == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLadnInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg         *rcvMsg           = VOS_NULL_PTR;
    MTA_AT_LadnInfoInd *ladnInfoInd      = VOS_NULL_PTR;
    VOS_UINT16          length           = 0;
    VOS_UINT8           allowedDnnNum    = 0;
    VOS_UINT8           nonAllowedDnnNum = 0;
    VOS_UINT8           allowedDnnList[MTA_AT_MAX_LADN_DNN_NUM][PS_MAX_APN_LEN];
    VOS_UINT8           nonAllowedDnnList[MTA_AT_MAX_LADN_DNN_NUM][PS_MAX_APN_LEN];
    VOS_UINT32          i;
    errno_t             memResult;
    VOS_UINT8           strLen;
    VOS_UINT8           srcStr[PS_MAX_APN_LEN + 1]; /* 网络格式DNN */
    VOS_UINT8           destStr[PS_MAX_APN_LEN];    /* 字符串格式DNN */

    rcvMsg      = (AT_MTA_Msg *)msg;
    ladnInfoInd = (MTA_AT_LadnInfoInd *)rcvMsg->content;

    (VOS_VOID)memset_s(srcStr, sizeof(srcStr), 0x00, sizeof(srcStr));
    (VOS_VOID)memset_s(destStr, sizeof(destStr), 0x00, sizeof(destStr));

    for (i = 0; i < MTA_AT_MAX_LADN_DNN_NUM; i++) {
        (VOS_VOID)memset_s(allowedDnnList[i], sizeof(allowedDnnList[i]), 0x00, sizeof(allowedDnnList[i]));
        (VOS_VOID)memset_s(nonAllowedDnnList[i], sizeof(nonAllowedDnnList[i]), 0x00, sizeof(nonAllowedDnnList[i]));
    }

    for (i = 0; i < AT_MIN(ladnInfoInd->ladnDnnNum, MTA_AT_MAX_LADN_DNN_NUM); i++) {
        /* 转换网络格式DNN为字符串 */
        srcStr[0] = ladnInfoInd->ladnList[i].dnn.length;
        strLen    = AT_MIN(PS_MAX_APN_LEN, srcStr[0]);
        if (strLen == 0) {
            continue;
        }
        memResult = memcpy_s(&srcStr[1], sizeof(srcStr) - 1, ladnInfoInd->ladnList[i].dnn.value, strLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(srcStr) - 1, strLen);
        AT_ConvertNwDnnToString(srcStr, PS_MAX_APN_LEN, destStr, PS_MAX_APN_LEN);

        /* 根据DNN是否可用，将DNN分别拷贝至可用和不可用DNN的数组，并统计数量 */
        if (ladnInfoInd->ladnList[i].availFlg == VOS_TRUE) {
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
    /* ^CLADNU : */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^CLADNU: ", g_atCrLf);

    /* <allowed_ladn_dnn_num>,<allowed_dnn_list> */
    length = AT_PrintLadnDnn(allowedDnnNum, allowedDnnList, length);

    /* , */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",");

    /* <nonallowed_ladn_dnn_num>,<noallowed_dnn_list> */
    length = AT_PrintLadnDnn(nonAllowedDnnNum, nonAllowedDnnList, length);

    /* /r/n */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif
#endif

VOS_UINT32 AT_RcvMtaAfcClkUnlockCauseInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg               = VOS_NULL_PTR;
    MTA_AT_AfcClkUnlockInd *mtaAtAfcClkUnlockInd = VOS_NULL_PTR;
    VOS_UINT16              length;

    /* 初始化 */
    rcvMsg               = (AT_MTA_Msg *)msg;
    mtaAtAfcClkUnlockInd = (MTA_AT_AfcClkUnlockInd *)rcvMsg->content;
    length               = 0;

    /* 打印^AFCCLKUNLOCK: */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_AFCCLKUNLOCK].text,
        mtaAtAfcClkUnlockInd->cause, g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaBestFreqInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *mtaMsg          = VOS_NULL_PTR;
    MTA_AT_BestFreqCaseInd *bestFreqCaseInd = VOS_NULL_PTR;
    VOS_UINT32              loop;
    VOS_UINT16              length = 0;

    mtaMsg          = (AT_MTA_Msg *)msg;
    bestFreqCaseInd = (MTA_AT_BestFreqCaseInd *)mtaMsg->content;

    bestFreqCaseInd->rptDeviceNum = AT_MIN(bestFreqCaseInd->rptDeviceNum, MTA_AT_MAX_BESTFREQ_GROUPNUM);
    for (loop = 0; loop < bestFreqCaseInd->rptDeviceNum; loop++) {
        length = 0;

        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^BESTFREQ: %d,%d,%d%s", g_atCrLf,
            bestFreqCaseInd->deviceFreqList[loop].groupId, bestFreqCaseInd->deviceFreqList[loop].deviceId,
            bestFreqCaseInd->deviceFreqList[loop].caseId, g_atCrLf);

        g_atSendDataBuff.bufLen = length;

        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    }

    return VOS_OK;
}

VOS_VOID AT_ReportCposrInd(VOS_UINT8 indexNum, VOS_CHAR *pcXmlText)
{
    VOS_UINT16 len;
    VOS_UINT8  finalUsbIndex;
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    VOS_UINT8     finalVcomIndex;
    ModemIdUint16 modemId;
#endif

    finalUsbIndex = AT_CLIENT_ID_PCUI;
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    finalVcomIndex = AT_CLIENT_ID_APP9;
    modemId        = MODEM_ID_0;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_ReportCposrInd: Get modem id fail.");
        return;
    }

    switch (modemId) {
        case MODEM_ID_0:
            finalUsbIndex  = AT_CLIENT_ID_PCUI;
            finalVcomIndex = AT_CLIENT_ID_APP9;
            break;
        case MODEM_ID_1:
            finalUsbIndex  = AT_CLIENT_ID_CTRL;
            finalVcomIndex = AT_CLIENT_ID_APP12;
            break;
        case MODEM_ID_2:
            finalUsbIndex  = AT_CLIENT_ID_PCUI2;
            finalVcomIndex = AT_CLIENT_ID_APP24;
            break;
        default:
            AT_ERR_LOG("AT_ReportCposrInd: Invalid modem id!");
            return;
    }
#endif

    len = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s+CPOSR: %s%s", g_atCrLf, pcXmlText, g_atCrLf);

    At_SendResultData(finalUsbIndex, g_atSndCodeAddress, len);
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    At_SendResultData(finalVcomIndex, g_atSndCodeAddress, len);
#endif
}

VOS_UINT32 AT_RcvMtaCposrInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    /* 定义局部变量 */
    MTA_AT_CposrInd *event    = (MTA_AT_CposrInd *)mtaMsg->content;
    AT_ModemAgpsCtx *agpsCtx  = VOS_NULL_PTR;

    agpsCtx = AT_GetModemAgpsCtxAddrFromClientId(indexNum);

    /* 根据当前的AT_GetModemAgpsCtxAddrFromModemId(MODEM_ID_0)->enCposrReport的值判断是否允许主动上报辅助数据和指示 */
    if (agpsCtx->cposrReport == AT_CPOSR_ENABLE) {
        AT_ReportCposrInd(indexNum, event->xmlText);
    } else {
        AT_WARN_LOG1("AT_RcvMtaCposrInd: +CPOSR: ", agpsCtx->cposrReport);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTempprtStatusInd(struct MsgCB *msg)
{
    TEMP_PROTECT_EventAtInd *tempPrt = VOS_NULL_PTR;
    TAF_TempProtectConfig    tempProtectPara;
    VOS_UINT32               result;

    (VOS_VOID)memset_s(&tempProtectPara, sizeof(tempProtectPara), 0x00, sizeof(tempProtectPara));

    /* 读取温度保护状态主动上报NV项 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TEMP_PROTECT_CONFIG, &tempProtectPara,
                               sizeof(TAF_TempProtectConfig));

    if (result != NV_OK) {
        AT_ERR_LOG("AT_RcvTempprtStatusInd: Read NV fail");
        return VOS_ERR;
    }

    if (tempProtectPara.spyStatusIndSupport == AT_TEMPPRT_STATUS_IND_ENABLE) {
        tempPrt = (TEMP_PROTECT_EventAtInd *)msg;

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^THERM: %d%s", g_atCrLf, tempPrt->tempProtectEvent, g_atCrLf);

        At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

        return VOS_OK;
    }

    return VOS_OK;
}

/*
 * 功能描述: 处理消息ID_TEMPPRT_AT_EVENT_IND
 * 修改历史:
 */
VOS_UINT32 AT_ProcTempprtEventInd(TEMP_PROTECT_EventAtInd *msg)
{
    VOS_UINT16               length;
    TEMP_PROTECT_EventAtInd *tempPrt = VOS_NULL;

    tempPrt = (TEMP_PROTECT_EventAtInd *)msg;

    HAL_SDMLOG("AT_ProcTempprtEventInd: Event %d Param %d\n", (VOS_INT)tempPrt->tempProtectEvent,
               (VOS_INT)tempPrt->tempProtectParam);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr, "%s%s%d,%d%s", g_atCrLf, "^TEMPPRT:", tempPrt->tempProtectEvent,
        tempPrt->tempProtectParam, g_atCrLf);

    At_SendResultData(AT_CLIENT_ID_APP, g_atSndCodeAddr, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaRefclkfreqInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg           *mtaMsg        = VOS_NULL_PTR;
    MTA_AT_RefclkfreqInd *refclkfreqInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg        = (AT_MTA_Msg *)msg;
    refclkfreqInd = (MTA_AT_RefclkfreqInd *)mtaMsg->content;

    /* 输出GPS参考时钟信息，命令版本号默认为0 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: 0,%d,%d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_REFCLKFREQ].text,
        refclkfreqInd->freq, refclkfreqInd->precision, refclkfreqInd->status, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_VOID At_RcvMmaPsInitResultIndProc(TAF_UINT8 indexNum, TAF_PHONE_EventInfo *event)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;


    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_RcvMmaPsInitResultIndProc: Get modem id fail.");
        return;
    }

    if (event->opPsInitRslt == VOS_FALSE) {
        AT_ERR_LOG("At_RcvMmaPsInitResultIndProc: invalid msg.");
        return;
    }

    /* 只有modem初始化成功才调用底软接口操作 */
    if (event->psInitRslt == TAF_MMA_PS_INIT_SUCC) {
        /* 拉GPIO管脚通知AP侧MODEM已经OK */
        DRV_OS_STATUS_SWITCH(VOS_TRUE);

        /* 收到PS INIT上报后写设备节点，启动成功 */
        AT_SetModemState(modemId, VOS_TRUE);

        dms_set_modem_status(modemId);
    }
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^PSINIT: %d%s", g_atCrLf, event->psInitRslt, g_atCrLf);

    At_SendResultData((VOS_UINT8)indexNum, g_atSndCodeAddress, length);
}

VOS_UINT32 AT_RcvMtaEpduDataInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    errno_t             memResult;
    VOS_UINT8          *name             = VOS_NULL_PTR;
    AT_MTA_Msg         *rcvMsg           = VOS_NULL_PTR;
    MTA_AT_EpduDataInd *mtaAtEpduDataInd = VOS_NULL_PTR;
    VOS_UINT32          i;
    VOS_UINT16          length;
    VOS_UINT32          dataSmallLen = 0;

    /* 初始化 */
    rcvMsg           = (AT_MTA_Msg *)msg;
    mtaAtEpduDataInd = (MTA_AT_EpduDataInd *)rcvMsg->content;
    length           = 0;

    /* 打印^EPDUR: */
    /* transaction_id, msg_type, common_info_valid_flg, end_flag, id */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d,%d,", g_atCrLf, g_atStringTab[AT_STRING_EPDUR].text,
        mtaAtEpduDataInd->transactionId, mtaAtEpduDataInd->msgBodyType, mtaAtEpduDataInd->commonIeValidFlg,
        mtaAtEpduDataInd->endFlg, mtaAtEpduDataInd->id);

    /* name */
    if (mtaAtEpduDataInd->nameLength > 0) {
        mtaAtEpduDataInd->nameLength = AT_MIN(mtaAtEpduDataInd->nameLength, MTA_MAX_EPDU_NAME_LEN);
        name = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, mtaAtEpduDataInd->nameLength + 1);

        /* 分配内存失败，直接返回 */
        if (name == VOS_NULL_PTR) {
            AT_ERR_LOG("AT_RcvMtaEpduDataInd: name, Alloc mem fail");

            return VOS_ERR;
        }
        (VOS_VOID)memset_s(name, mtaAtEpduDataInd->nameLength + 1, 0x00, mtaAtEpduDataInd->nameLength + 1);

        memResult = memcpy_s(name, mtaAtEpduDataInd->nameLength, mtaAtEpduDataInd->name,
                             mtaAtEpduDataInd->nameLength);
        TAF_MEM_CHK_RTN_VAL(memResult, mtaAtEpduDataInd->nameLength, mtaAtEpduDataInd->nameLength);

        name[mtaAtEpduDataInd->nameLength] = '\0';

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", name);

        PS_MEM_FREE(WUEPS_PID_AT, name);
    }

    /* total, index */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,", mtaAtEpduDataInd->total, mtaAtEpduDataInd->index);
    dataSmallLen = (VOS_UINT32)TAF_MIN(mtaAtEpduDataInd->dataLength, MTA_MAX_EPDU_BODY_LEN);
    for (i = 0; i < dataSmallLen; i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%02X", mtaAtEpduDataInd->data[i]);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
/*lint -save -e838 -specific(-e838)*/
/*lint -save -e826 -specific(-e826)*/
VOS_UINT32 AT_RcvMtaEccStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *rcvMsg    = VOS_NULL_PTR;
    MTA_AT_EccStatusInd *eccStatus = VOS_NULL_PTR;
    VOS_UINT16           length;

    /* 初始化消息变量 */
    rcvMsg    = (AT_MTA_Msg *)msg;
    eccStatus = (MTA_AT_EccStatusInd *)rcvMsg->content;
    length    = 0;

    /* "^ECCSTATUS: */
    length +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d%s",
            g_atCrLf, g_atStringTab[AT_STRING_ECC_STATUS].text, eccStatus->validflag, eccStatus->rxAntNum,
            eccStatus->tmMode, eccStatus->corrQR1Data0011, eccStatus->corrQR1Data01Iq, eccStatus->corrQR2Data0011,
            eccStatus->corrQR2Data01Iq, eccStatus->corrQR3Data0011, eccStatus->corrQR3Data01Iq, eccStatus->rsrpR0,
            eccStatus->rsrpR1, eccStatus->rsrpR2, eccStatus->rsrpR3, eccStatus->rssiR0, eccStatus->rssiR1,
            eccStatus->rssiR2, eccStatus->rssiR3, g_atCrLf);

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
/*lint -restore*/
/*lint -restore*/
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvImsaImpuInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImpuTypeInd *impuInd = (IMSA_AT_ImpuTypeInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_IMPU].text, impuInd->impuType,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaCallAltSrvInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%s", g_atCrLf, g_atStringTab[AT_STRING_CALL_ALT_SRV].text, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaRatHandoverInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImsRatHandoverInd *handoverInd = (IMSA_AT_ImsRatHandoverInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^IMSRATHO: %d,%d,%d,%d%s", g_atCrLf, handoverInd->hoStatus,
        handoverInd->srcRat, handoverInd->dstRat, handoverInd->cause, g_atCrLf);
    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvSrvStatusUpdateInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImsSrvStatusUpdateInd *srvUpdateInd = (IMSA_AT_ImsSrvStatusUpdateInd *)msg;

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^IMSSRVSTATUS: %d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
            srvUpdateInd->imsSrvStatInfo.smsSrvStatus, srvUpdateInd->imsSrvStatInfo.smsSrvRat,
            srvUpdateInd->imsSrvStatInfo.voIpSrvStatus, srvUpdateInd->imsSrvStatInfo.voIpSrvRat,
            srvUpdateInd->imsSrvStatInfo.vtSrvStatus, srvUpdateInd->imsSrvStatInfo.vtSrvRat,
            srvUpdateInd->imsSrvStatInfo.vsSrvStatus, srvUpdateInd->imsSrvStatInfo.vsSrvRat, g_atCrLf);
    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_IsImsRegErrRptParaValid(struct MsgCB *msg)
{
    IMSA_AT_RegerrReportInd *regErrInd = VOS_NULL_PTR;
    VOS_UINT32               strLen;

    /* 初始化消息变量 */
    regErrInd = (IMSA_AT_RegerrReportInd *)msg;
    strLen    = 0;

    /* 如果注册域为非wifi和非lte，认为上报异常 */
    if (regErrInd->imsaRegDomain >= IMSA_AT_IMS_REG_DOMAIN_TYPE_UNKNOWN) {
        return VOS_FALSE;
    }

    /* 如果注册失败类型为非pdn和非reg，认为上报异常 */
    if (regErrInd->imsaRegErrType >= IMSA_AT_REG_ERR_TYPE_BUTT) {
        return VOS_FALSE;
    }

    /* PDN类型失败，但失败原因值大于pdn失败原因值的最大值，则认为上报异常 */
    if ((regErrInd->imsaRegErrType == IMSA_AT_REG_ERR_TYPE_PDN_FAIL) &&
        (regErrInd->imsaPdnFailCause >= IMSA_AT_PDN_FAIL_CAUSE_BUTT)) {
        return VOS_FALSE;
    }

    /* reg类型失败，但失败原因值大于reg失败原因值的最大值，则认为上报异常 */
    if ((regErrInd->imsaRegErrType == IMSA_AT_REG_ERR_TYPE_IMS_REG_FAIL) &&
        (regErrInd->imsaRegFailCause >= IMSA_AT_REG_FAIL_CAUSE_BUTT)) {
        return VOS_FALSE;
    }

    strLen = VOS_StrNLen(regErrInd->imsRegFailReasonCtx, IMSA_AT_REG_FAIL_CAUSE_STR_MAX_LEN);

    /* 失败字符串约定最大长度为255，大于255，认为上报异常 */
    if (strLen == IMSA_AT_REG_FAIL_CAUSE_STR_MAX_LEN) {
        AT_ERR_LOG1("AT_IsImsRegErrRptParaValid: str len beyond IMSA_AT_REG_FAIL_CAUSE_STR_MAX_LEN!", strLen);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_RcvImsaRegErrRptInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_RegerrReportInd *regErrInd = (IMSA_AT_RegerrReportInd *)msg;
    VOS_UINT32               failStage = 0;
    VOS_UINT32               failCause = 0;

    /* 添加异常原因值检查 */
    if (AT_IsImsRegErrRptParaValid(msg) != VOS_TRUE) {
        return VOS_ERR;
    }

    if (regErrInd->imsaRegErrType == IMSA_AT_REG_ERR_TYPE_PDN_FAIL) {
        failStage = regErrInd->imsaRegErrType;
        failCause = regErrInd->imsaPdnFailCause;
    }

    if (regErrInd->imsaRegErrType == IMSA_AT_REG_ERR_TYPE_IMS_REG_FAIL) {
        failStage = regErrInd->imsaRegErrType;
        failCause = regErrInd->imsaRegFailCause;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s %d,%u,%u,\"%s\"%s", g_atCrLf, g_atStringTab[AT_STRING_IMS_REG_ERR].text,
        regErrInd->imsaRegDomain, failStage, failCause, regErrInd->imsRegFailReasonCtx, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvMtaTempProtectInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *mtaMsg   = VOS_NULL_PTR;
    MTA_AT_TempProtectInd *tempInd  = VOS_NULL_PTR;

    mtaMsg  = (AT_MTA_Msg *)msg;
    tempInd = (MTA_AT_TempProtectInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_TEMPPROTECT].text,
            tempInd->tempResult, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaJamDetectInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    VOS_UINT16           length       = 0;
    AT_MTA_Msg          *mtaMsg       = VOS_NULL_PTR;
    MTA_AT_JamDetectInd *jamDetectInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg       = (AT_MTA_Msg *)msg;
    jamDetectInd = (MTA_AT_JamDetectInd *)mtaMsg->content;

    /* 上报干扰检测结果 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^JDINFO: %d,%d%s", g_atCrLf, jamDetectInd->jamResult, jamDetectInd->rat,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaPhyCommAckInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *rcvMsg          = VOS_NULL_PTR;
    MTA_AT_PhyCommAckInd *mtaAtCommAckInd = VOS_NULL_PTR;
    VOS_UINT16            length;

    /* 初始化 */
    rcvMsg          = (AT_MTA_Msg *)msg;                       /*lint !e838 */
    mtaAtCommAckInd = (MTA_AT_PhyCommAckInd *)rcvMsg->content; /*lint !e838 */
    length          = 0;

    /* 打印^PHYCOMACK: */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d%s", g_atCrLf,
        g_atStringTab[AT_STRING_PHYCOMACK].text, mtaAtCommAckInd->ackType, mtaAtCommAckInd->ackValue1,
        mtaAtCommAckInd->ackValue2, mtaAtCommAckInd->ackValue3, g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaPlmnSelectInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_PlmnSelectionInfoInd *plmnSelecInd = (TAF_MMA_PlmnSelectionInfoInd *)msg;
    VOS_UINT16                    length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s", g_atCrLf, g_atStringTab[AT_STRING_PLMNSELEINFO].text);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d%s", plmnSelecInd->plmnSelectInfo.plmnSelectFlag,
        plmnSelecInd->plmnSelectInfo.plmnSelectRlst, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvCssMccNotify(struct MsgCB *msg)
{
    /* 定义局部变量 */
    VOS_UINT32                 i;
    VOS_UINT8                  indexNum = 0;
    VOS_UINT8                  mccStr[AT_CSS_MAX_MCC_ID_NUM * AT_MCC_PLUS_COMMA_LENGTH] = {0};
    VOS_UINT8                  versionId[MCC_INFO_VERSION_LEN + 1]                      = {0};
    CSS_AT_QueryMccInfoNotify *cssMccNty                                                = VOS_NULL_PTR;
    errno_t                    memResult;

    /* 初始化消息变量 */
    cssMccNty = (CSS_AT_QueryMccInfoNotify *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(cssMccNty->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssMccNotify: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* MCC个数不对 */
    if ((cssMccNty->mccNum == 0) || (cssMccNty->mccNum > AT_CSS_MAX_MCC_ID_NUM)) {
        AT_WARN_LOG("AT_RcvCssMccNotify: WARNING:INVALID MCC NUM!");
        return VOS_ERR;
    }

    /* 构造上报给Ril的MCC字符串 */
    for (i = 0; i < cssMccNty->mccNum; i++) {
        mccStr[AT_MCC_PLUS_COMMA_LENGTH * i]     = (cssMccNty->mccId[i].mcc[0] & 0x0f) + '0';
        mccStr[AT_MCC_PLUS_COMMA_LENGTH * i + 1] = ((cssMccNty->mccId[i].mcc[0] & 0xf0) >> 4) + '0';
        mccStr[AT_MCC_PLUS_COMMA_LENGTH * i + 2] = (cssMccNty->mccId[i].mcc[1] & 0x0f) + '0';
        mccStr[AT_MCC_PLUS_COMMA_LENGTH * i + 3] = ',';
    }

    /* 字符串结束符 */
    mccStr[(VOS_UINT8)(cssMccNty->mccNum) * AT_MCC_PLUS_COMMA_LENGTH - 1] = 0;

    /* 构造Version String */
    memResult = memcpy_s(versionId, sizeof(versionId), cssMccNty->versionId, MCC_INFO_VERSION_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(versionId), MCC_INFO_VERSION_LEN);

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s %s,%s%s", g_atCrLf, g_atStringTab[AT_STRING_MCC].text,
        (VOS_CHAR *)versionId, (VOS_CHAR *)mccStr, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaCrrconnStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg              *mtaMsg           = VOS_NULL_PTR;
    MTA_AT_CrrconnStatusInd *crrconnStatusInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg           = (AT_MTA_Msg *)msg;
    crrconnStatusInd = (MTA_AT_CrrconnStatusInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CRRCONN: %d,%d,%d%s", g_atCrLf, crrconnStatusInd->status0,
        crrconnStatusInd->status1, crrconnStatusInd->status2, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaRlQualityInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg              *mtaMsg           = VOS_NULL_PTR;
    MTA_AT_RlQualityInfoInd *rlQualityInfoInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg           = (AT_MTA_Msg *)msg;
    rlQualityInfoInd = (MTA_AT_RlQualityInfoInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^LTERLQUALINFO: %d,%d,%d,%d%s", g_atCrLf, rlQualityInfoInd->rsrp,
        rlQualityInfoInd->rsrq, rlQualityInfoInd->rssi, rlQualityInfoInd->bler, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaVideoDiagInfoRpt(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg              *mtaMsg           = VOS_NULL_PTR;
    MTA_AT_VideoDiagInfoRpt *videoDiagInfoRpt = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg           = (AT_MTA_Msg *)msg;
    videoDiagInfoRpt = (MTA_AT_VideoDiagInfoRpt *)mtaMsg->content;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^LPDCPINFORPT: %u,%u,%u,%u%s", g_atCrLf, videoDiagInfoRpt->currBuffTime,
        videoDiagInfoRpt->currBuffPktNum, videoDiagInfoRpt->macUlThrput, videoDiagInfoRpt->maxBuffTime, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvCssBlockCellMccNotify(struct MsgCB *msg)
{
    errno_t memResult;
    /* 定义局部变量 */
    CSS_AT_BlockCellMccNotify *cssNty = VOS_NULL_PTR;
    VOS_UINT8                  versionId[AT_CSS_BLOCK_CELL_LIST_VERSION_LEN + 1];
    VOS_UINT32                 mcc;
    VOS_UINT32                 loop;
    VOS_UINT16                 length;
    VOS_UINT8                  indexNum;

    /* 初始化消息变量 */
    cssNty = (CSS_AT_BlockCellMccNotify *)msg;

    indexNum = 0;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(cssNty->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssBlockCellMccNotify: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    /* MCC个数不对 */
    if ((cssNty->mccInfo.mccNum == 0) || (cssNty->mccInfo.mccNum > AT_CSS_MAX_MCC_ID_NUM)) {
        AT_WARN_LOG("AT_RcvCssBlockCellMccNotify: WARNING:INVALID MCC NUM!");

        return VOS_ERR;
    }

    /* 构造上报给Ril的Version */
    (VOS_VOID)memset_s(versionId, sizeof(versionId), 0, sizeof(versionId));
    memResult = memcpy_s(versionId, sizeof(versionId), cssNty->versionId, sizeof(cssNty->versionId));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(versionId), sizeof(cssNty->versionId));

    length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s\"%s\"", g_atCrLf, g_atStringTab[AT_STRING_BLOCK_CELL_MCC].text,
        versionId);

    /* 构造上报给Ril的MCC字符串 */
    for (loop = 0; loop < cssNty->mccInfo.mccNum; loop++) {
        mcc = 0;

        AT_ConvertNasMccToBcdType(cssNty->mccInfo.mcc[loop], &mcc);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",%x%x%x", (mcc & 0x0f00) >> 8, (mcc & 0xf0) >> 4, (mcc & 0x0f));
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaElevatorStateInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ElevatorStateInd *elevatorState = (TAF_MMA_ElevatorStateInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d,%d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_ELEVATOR].text,
        elevatorState->sensorPara.sensorState, elevatorState->sensorPara.serviceState,
        elevatorState->sensorPara.sensorScene, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaPseudBtsIdentInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg              *mtaMsg           = VOS_NULL_PTR;
    MTA_AT_PseudBtsIdentInd *pseudBtsIdentInd = VOS_NULL_PTR;

    mtaMsg           = (AT_MTA_Msg *)msg;
    pseudBtsIdentInd = (MTA_AT_PseudBtsIdentInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d%s", g_atCrLf, g_atStringTab[AT_STRING_PSEUDBTS].text,
        pseudBtsIdentInd->pseudBtsType, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaPseudBtsIdentInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_PseudBtsIdentInd *pseudBtsIdentInfo = (TAF_MMA_PseudBtsIdentInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d%s", g_atCrLf, g_atStringTab[AT_STRING_PSEUDBTS].text,
        pseudBtsIdentInfo->currentRat, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaUlFreqChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_UlFreqInd *ulFreqInfoInd = (TAF_MMA_UlFreqInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d,%d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_ULFREQRPT].text,
        ulFreqInfoInd->rat, ulFreqInfoInd->freq, ulFreqInfoInd->bandWidth, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_DSDS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaDsdsStateNotify(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_DsdsStateNotify *dsdsStateNotify = (TAF_MMA_DsdsStateNotify *)msg;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_DSDSSTATE].text,
        dsdsStateNotify->supportDsds3, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}
#endif

#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
/* AT处理THROT主动上报的消息ID_THROT_AT_SINGLE_APN_BACKOFF_TIMER_PARA_NTF */
VOS_UINT32 AT_RcvThortSingleApnBackoffTimeParaNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_SingleApnBackoffParaNtf *singleApnBackoffParaNtf = (THROT_AT_SingleApnBackoffParaNtf *)msg;
    VOS_UINT16                        length = 0;

    /* apn长度错误或eventType取值错误，直接返回错误 */
    if ((singleApnBackoffParaNtf->apnInfo.apnLen > NAS_THROT_MAX_APN_LEN) ||
        (singleApnBackoffParaNtf->apnInfo.apnLen == 0) || (singleApnBackoffParaNtf->eventType >=
            THROT_AT_BACKOFF_EVENT_TYPE_BUTT)) {
        AT_WARN_LOG("AT_RcvThortSingleApnBackoffTimeParaNtf: apn number or eventType is invalid");
        return VOS_ERR;
    }

    /* backOff参数不正确，直接返回错误 */
    if (AT_CheckbackOffPara(&(singleApnBackoffParaNtf->backOffPara)) != VOS_OK) {
        AT_ERR_LOG("AT_RcvThortSingleApnBackoffTimeParaNtf: AT_CheckbackOffPara Check backOff para error!.");
        return VOS_ERR;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s", g_atCrLf, g_atStringTab[AT_STRING_CABTSRI].text);

    /* apn信息 */
    length = AT_FormatApnPara(singleApnBackoffParaNtf->apnInfo, length);

    /* event_type信息 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", singleApnBackoffParaNtf->eventType);

    if (singleApnBackoffParaNtf->eventType == THROT_AT_BACKOFF_EVENT_TYPE_DEACTIVATED) {
       length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
           (VOS_CHAR *)g_atSndCodeAddress + length, ",,");

       length = AT_ParseBackoffTimerOtherPara(&(singleApnBackoffParaNtf->backOffPara), length);
    }

    if (singleApnBackoffParaNtf->eventType == THROT_AT_BACKOFF_EVENT_TYPE_STARTED) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,", singleApnBackoffParaNtf->backOffPara.backOffTimerLen);

        length = AT_ParseBackoffTimerOtherPara(&(singleApnBackoffParaNtf->backOffPara), length);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    /* 调用At_FormatResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
/* AT处理THROT主动上报的消息ID_THROT_AT_SINGLE_SNSSAI_BACKOFF_TIMER_PARA_NTF */
VOS_UINT32 AT_RcvThortSingleSnssaiBackoffTimerParaNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_SingleSnssaiBackoffParaNtf *singleSnssaiBackoffParaNtf = (THROT_AT_SingleSnssaiBackoffParaNtf *)msg;
    THROT_AT_SnssaiBasedBackOffTimerPara backOffPara = singleSnssaiBackoffParaNtf->backOffPara.backOffPara;
    THROT_AT_BackOffEventTypeUint32      eventType = singleSnssaiBackoffParaNtf->eventType;
    VOS_UINT16                           length = 0;

    /* eventType取值错误，直接返回错误 */
    if (eventType >= THROT_AT_BACKOFF_EVENT_TYPE_BUTT) {
        AT_ERR_LOG("T3585 backoffTimer: eventType is invalid");
        return VOS_ERR;
    }

    /* backOff参数不正确，直接返回错误 */
    if (AT_CheckSnssaiDnnbackOffPara(&backOffPara) != VOS_OK) {
        AT_ERR_LOG("T3585 backoffTimer: AT_CheckSnssaiDnnbackOffPara error");
        return VOS_ERR;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s", g_atCrLf, g_atStringTab[AT_STRING_CSBTSRI].text);

    /* S-NSSAI信息 */
    AT_ParseSnssaiPara(&(singleSnssaiBackoffParaNtf->backOffPara.snssaiInfo), &length);

    /* event_type信息 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", eventType);

    if (eventType == THROT_AT_BACKOFF_EVENT_TYPE_DEACTIVATED) {
       length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
           (VOS_CHAR *)g_atSndCodeAddress + length, ",,");

       length = AT_ParseSnssaiDnnBackoffTimerOtherPara(&backOffPara, length);
    }

    if (eventType == THROT_AT_BACKOFF_EVENT_TYPE_STARTED) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,", backOffPara.backOffTimerLen);

        length = AT_ParseSnssaiDnnBackoffTimerOtherPara(&backOffPara, length);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    /* 调用At_FormatResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

/* AT处理THROT主动上报的消息ID_THROT_AT_SINGLE_SNSSAI_DNN_BACKOFF_TIMER_PARA_NTF */
VOS_UINT32 AT_RcvThortSingleSnssaiDnnBackoffTimerParaNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_SingleSnssaiDnnBackoffParaNtf *backoffParaNtf = (THROT_AT_SingleSnssaiDnnBackoffParaNtf *)msg;
    THROT_AT_SnssaiBasedBackOffTimerPara    backOffPara = backoffParaNtf->backOffPara.backOffPara;
    THROT_AT_BackOffEventTypeUint32         eventType = backoffParaNtf->eventType;
    VOS_UINT16                              length = 0;
    VOS_UINT8                               dnnLen = backoffParaNtf->backOffPara.apnInfo.apnLen;

    /* <DNN>长度错误、eventType取值错误，直接返回错误 */
    if ((dnnLen > NAS_THROT_MAX_APN_LEN) || (dnnLen == 0) || (eventType >= THROT_AT_BACKOFF_EVENT_TYPE_BUTT)) {
        AT_ERR_LOG("T3584 backoffTimer: dnn or eventType error");
        return VOS_ERR;
    }

    /* backOff参数不正确，直接返回错误 */
    if (AT_CheckSnssaiDnnbackOffPara(&backOffPara) != VOS_OK) {
        AT_ERR_LOG("T3584 backoffTimer:AT_CheckSnssaiDnnbackOffPara error!");
        return VOS_ERR;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s", g_atCrLf, g_atStringTab[AT_STRING_CSDBTSRI].text);

    /* S-NSSAI信息 */
    AT_ParseSnssaiPara(&(backoffParaNtf->backOffPara.snssaiInfo), &length);

    /* dnn信息 */
    length = AT_FormatApnPara(backoffParaNtf->backOffPara.apnInfo, length);

    /* event_type信息 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", eventType);

    if (eventType == THROT_AT_BACKOFF_EVENT_TYPE_DEACTIVATED) {
       length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
           (VOS_CHAR *)g_atSndCodeAddress + length, ",,");

       length = AT_ParseSnssaiDnnBackoffTimerOtherPara(&backOffPara, length);
    }

    if (eventType == THROT_AT_BACKOFF_EVENT_TYPE_STARTED) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,", backOffPara.backOffTimerLen);

        length = AT_ParseSnssaiDnnBackoffTimerOtherPara(&backOffPara, length);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    /* 调用At_FormatResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}
#endif

VOS_UINT32 atLwclashInd(struct MsgCB *msgBlock)
{
    L4A_READ_LwclashInd *lwclash = NULL;
    VOS_UINT16           length;
    VOS_UINT8            indexNum = 0;

    lwclash = (L4A_READ_LwclashInd *)msgBlock;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserBroadCastId(lwclash->clientId, &indexNum) != AT_SUCCESS) {
        AT_WARN_LOG("atLwclashInd: At_ClientIdToUserBroadCastId is err!");
        return VOS_ERR;
    }

    length = (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr,
        "%s^LWURC: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
        lwclash->lwclashInfo.state, lwclash->lwclashInfo.ulFreq, lwclash->lwclashInfo.ulBandwidth,
        lwclash->lwclashInfo.dlFreq, lwclash->lwclashInfo.dlBandwidth, lwclash->lwclashInfo.band, lwclash->scellNum,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_0].ulFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_0].ulBandwidth,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_0].dlFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_0].dlBandwidth,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_1].ulFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_1].ulBandwidth,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_1].dlFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_1].dlBandwidth,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_2].ulFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_2].ulBandwidth,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_2].dlFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_2].dlBandwidth,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_3].ulFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_3].ulBandwidth,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_3].dlFreq,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_3].dlBandwidth, lwclash->lwclashInfo.dlMimo,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_0].dlMimo,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_1].dlMimo,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_2].dlMimo,
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_3].dlMimo, lwclash->dl256QamFlag, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddr, length);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 At_PlmnDetectIndProc(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_PlmnDetectInd     *plmnDetectCnf = (TAF_MMA_PlmnDetectInd *)msg;
    TAF_MMA_PlmnDetectIndPara *plmnDetect    = &plmnDetectCnf->plmnDetectIndPara;
    VOS_UINT16                 length = 0;

    length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:", g_atCrLf, g_atStringTab[AT_STRING_DETECTPLMN].text);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%X%X%X", (0x0000000f & plmnDetect->plmnId.mcc),
        (0x00000f00 & plmnDetect->plmnId.mcc) >> 8, (0x000f0000 & plmnDetect->plmnId.mcc) >> 16);

    if (((0x000f0000 & (plmnDetect->plmnId.mnc)) >> 16) != 0x0F) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%X", (0x000f0000 & plmnDetect->plmnId.mnc) >> 16);
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%X%X", (0x0000000f & plmnDetect->plmnId.mnc),
        (0x00000f00 & plmnDetect->plmnId.mnc) >> 8);

    /* <rat> */
    if (plmnDetect->ratMode == TAF_MMA_RAT_LTE) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",7");
    }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    else if (plmnDetect->ratMode == TAF_MMA_RAT_NR) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",12");
    }
#endif
    else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",");
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", plmnDetect->rsrp);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaNvRefreshNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 更新Phone随卡NV */
    AT_ReadIpv6AddrTestModeCfgNV();
    AT_ReadPrivacyFilterCfgNv();
    AT_InitStk();
    AT_ReadWasCapabilityNV();
    AT_ReadAgpsNv();

    /* 更新MBB随卡NV */
    AT_ReadSmsNV();
    AT_ReadRoamCapaNV();
    AT_ReadIpv6CapabilityNV();
    AT_ReadGasCapabilityNV();
    AT_ReadApnCustomFormatCfgNV();

    return VOS_OK;
}

#if ((FEATURE_EDA_SUPPORT == FEATURE_ON) && (FEATURE_MBB_HSRCELLINFO == FEATURE_ON))
VOS_UINT32 AT_RcvMtaHsrcellInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *mtaMsg         = VOS_NULL_PTR;
    MTA_AT_HsrcellInfoInd *hsrcellInfoInd = VOS_NULL_PTR;
    VOS_UINT16             length = 0;

    mtaMsg         = (AT_MTA_Msg *)msg;
    hsrcellInfoInd = (MTA_AT_HsrcellInfoInd *)mtaMsg->content;

    if (hsrcellInfoInd->rat == MTA_AT_HSRCELLINFO_RAT_NR) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^HSRCELLINFO: %d,%d,%d,%d,%d%s", g_atCrLf, hsrcellInfoInd->rat,
            hsrcellInfoInd->highSpeedFlg, hsrcellInfoInd->rsrp, hsrcellInfoInd->cellIdLowBit,
            hsrcellInfoInd->cellIdHighBit, g_atCrLf);
    } else {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^HSRCELLINFO: %d,%d,%d,%d%s", g_atCrLf, hsrcellInfoInd->rat,
            hsrcellInfoInd->highSpeedFlg, hsrcellInfoInd->rsrp, hsrcellInfoInd->cellIdLowBit, g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return VOS_OK;
}
#endif /* (FEATURE_EDA_SUPPORT == FEATURE_ON) && (FEATURE_MBB_HSRCELLINFO == FEATURE_ON) */

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaPendingNssaiInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg = VOS_NULL_PTR;
    MTA_AT_PendingNssaiInd *pendingNssaiInd = VOS_NULL_PTR;
    VOS_UINT32              strLen = 0;
    VOS_CHAR                strNssai[AT_EVT_MULTI_S_NSSAI_LEN] = {0};
    VOS_UINT16              length = 0;

    rcvMsg = (AT_MTA_Msg *)msg;
    pendingNssaiInd = (MTA_AT_PendingNssaiInd *)rcvMsg->content;

    AT_ConvertMultiSNssaiToString(AT_MIN((VOS_UINT8)pendingNssaiInd->pendingNssaiNum, MTA_AT_MAX_PENDING_NSSAI_NUM),
                                  &pendingNssaiInd->sNssaiList[0], strNssai, sizeof(strNssai), &strLen);

    /* ^CPENDINGNSSAI : */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s: %d", g_atCrLf, g_atStringTab[AT_STRING_CPENDINGNSSAI].text,
        strLen);

    if (strLen != 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", strNssai);
    }

    /* /r/n */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvMmaMtReattachInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s%s%s", g_atCrLf, g_atStringTab[AT_STRING_MTREATTACH].text,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLteCategoryInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                *mtaMsg             = VOS_NULL_PTR;
    MTA_AT_LteCategoryInfoInd *lteCategoryInfoInd = VOS_NULL_PTR;

    mtaMsg             = (AT_MTA_Msg *)msg;
    lteCategoryInfoInd = (MTA_AT_LteCategoryInfoInd *)mtaMsg->content;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    AT_SetLteCategoryInfo(lteCategoryInfoInd->dlCategery, lteCategoryInfoInd->ulCategery);
#endif
    if ((lteCategoryInfoInd->dlCategery >= AT_UE_LTE_CATEGORY_NUM_MAX) ||
        (lteCategoryInfoInd->ulCategery >= AT_UE_LTE_CATEGORY_NUM_MAX)) {
        AT_WARN_LOG2("AT_RcvMtaLteCategoryInfoInd: WARNING: illegal Category value <DlCategery, UlCategery>!",
                     lteCategoryInfoInd->dlCategery, lteCategoryInfoInd->ulCategery);
        return VOS_ERR;
    } else {
        g_atDlRateCategory.lteUeDlCategory = lteCategoryInfoInd->dlCategery;
        g_atDlRateCategory.lteUeUlCategory = lteCategoryInfoInd->ulCategery;
    }

    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvMmaSrchedPlmnInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_SrchedPlmnInfoIndAt  *srchedPlmnInfo = (TAF_MMA_SrchedPlmnInfoIndAt *)msg;
    VOS_UINT16                    length;

    length   = 0;
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                 (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s ", g_atCrLf,
                                 g_atStringTab[AT_STRING_SRCHEDPLMNLIST].text);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                                     (TAF_CHAR *)g_atSndCodeAddress + length, "%s",
                                     srchedPlmnInfo->plmnList);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLendcInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *mtaMsg       = VOS_NULL_PTR;
    MTA_AT_LendcInfoInd *lendcInfoInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg       = (AT_MTA_Msg *)msg;
    lendcInfoInd = (MTA_AT_LendcInfoInd *)mtaMsg->content;

    /* ^LEND: <endc_available>,<endc_plmn_avail>,<endc_restricted>,<nr_pscell> */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^LENDC: %d,%d,%d,%d%s", g_atCrLf, lendcInfoInd->lendcInfo.endcAvaliableFlag,
        lendcInfoInd->lendcInfo.endcPlmnAvaliableFlag, lendcInfoInd->lendcInfo.endcEpcRestrictedFlag,
        lendcInfoInd->lendcInfo.nrPscellFlag, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaSib16TimeUpdateInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg          = VOS_NULL_PTR;
    MTA_AT_Sib16TimeUpdate *sib16TimeUpdate = VOS_NULL_PTR;
    VOS_UINT16              length;

    /* 初始化消息变量 */
    rcvMsg          = (AT_MTA_Msg *)msg;
    sib16TimeUpdate = (MTA_AT_Sib16TimeUpdate *)rcvMsg->content;
    length          = 0;

    switch (sib16TimeUpdate->rptOptType) {
        case MTA_AT_TIME_INFO_RPT_OPT_MMINFO:
            length += (VOS_UINT16)At_PrintMmTimeInfo(indexNum, &(sib16TimeUpdate->commTimeInfo),
                                                     (g_atSndCodeAddress + length));
            break;

        case MTA_AT_TIME_INFO_RPT_OPT_SIB16TIME:
            AT_WARN_LOG("AT_RcvMtaSib16TimeUpdateInd: Not support SIB16Time Report!");
            return VOS_ERR;

        default:
            break;
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

/*lint -save -e838 -specific(-e838)*/
/*lint -save -e826 -specific(-e826)*/
VOS_UINT32 AT_RcvMtaAccessStratumRelInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                 *rcvMsg              = VOS_NULL_PTR;
    MTA_AT_AccessStratumRelInd *accessStratumRelInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    rcvMsg              = (AT_MTA_Msg *)msg;
    accessStratumRelInd = (MTA_AT_AccessStratumRelInd *)rcvMsg->content;

    switch (accessStratumRelInd->accessStratumRel) {
        case MTA_AT_ACCESS_STRATUM_REL8:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL8;
            break;

        case MTA_AT_ACCESS_STRATUM_REL9:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL9;
            break;

        case MTA_AT_ACCESS_STRATUM_REL10:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL10;
            break;

        case MTA_AT_ACCESS_STRATUM_REL11:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL11;
            break;

        case MTA_AT_ACCESS_STRATUM_REL12:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL12;
            break;

        case MTA_AT_ACCESS_STRATUM_REL13:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL13;
            break;
        case MTA_AT_ACCESS_STRATUM_REL14:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL14;
            break;
        case MTA_AT_ACCESS_STRATUM_REL_SPARE1:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL_SPARE1;
            break;

        default:
            g_releaseInfo.accessStratumRel = AT_ACCESS_STRATUM_REL8;
            AT_NORM_LOG("AT_RcvMtaAccessStratumRelInd: Wrong Release Number!");
            break;
    }

    return VOS_OK;
}
/*lint -restore*/
/*lint -restore*/
#endif

VOS_UINT32 AT_RcvMtaXpassInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *mtaMsg       = VOS_NULL_PTR;
    MTA_AT_XpassInfoInd *xpassInfoInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg       = (AT_MTA_Msg *)msg;
    xpassInfoInd = (MTA_AT_XpassInfoInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^XPASSINFO: %d,%d%s", g_atCrLf, xpassInfoInd->gphyXpassMode,
        xpassInfoInd->wphyXpassMode, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaPsTransferInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *mtaMsg        = VOS_NULL_PTR;
    MTA_AT_PsTransferInd *psTransferInd = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg        = (AT_MTA_Msg *)msg;
    psTransferInd = (MTA_AT_PsTransferInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^PSTRANSFER: %d%s", g_atCrLf, psTransferInd->cause, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaMipiInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *mtaMsg     = VOS_NULL_PTR;
    MTA_AT_RfLcdMipiclkInd *mipiClkCnf = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg     = (AT_MTA_Msg *)msg;
    mipiClkCnf = (MTA_AT_RfLcdMipiclkInd *)mtaMsg->content;

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^MIPICLK: %d%s", g_atCrLf, mipiClkCnf->mipiClk, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
#if (FEATURE_NSSAI_AUTH == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNwSliceAuthCmdInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg               *mtaMsg = VOS_NULL_PTR;
    MTA_AT_NwSliceAuthCmdInd *cmdInd = VOS_NULL_PTR;
    VOS_UINT8                *eapMsg = VOS_NULL_PTR;
    VOS_UINT32                nssaiLen;
    VOS_UINT32                eapMsgLen;
    VOS_UINT32                rslt;
    VOS_CHAR                  nssai[AT_EVT_MULTI_S_NSSAI_LEN] = {0};

    /* 初始化消息变量 */
    mtaMsg = (AT_MTA_Msg *)msg;
    cmdInd = (MTA_AT_NwSliceAuthCmdInd *)mtaMsg->content;
    if (cmdInd->eapMsg.eapMsgSize > MTA_AT_EAP_MSG_MAX_NUM) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthCmdInd: WARNING: eapMsgSize is too large!");
        return VOS_ERR;
    }
    /* 取2是因为原始EAP消息符号数是上报AP的字符数的2倍；3为两个引号加一个\0的字符数 */
    eapMsgLen = cmdInd->eapMsg.eapMsgSize * sizeof(VOS_UINT8) * 2 + 3;
    eapMsg = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, eapMsgLen);
    if (eapMsg == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthCmdInd: WARNING: Alloc Mem Fail!");
        return VOS_ERR;
    }
    (VOS_VOID)memset_s(eapMsg, sizeof(*eapMsg), 0x00, sizeof(*eapMsg));
    eapMsg[eapMsgLen - 1] = '\0';

    rslt = AT_HexToAsciiString(&eapMsg[1], (eapMsgLen - 3), cmdInd->eapMsg.eapMsg, cmdInd->eapMsg.eapMsgSize);
    if (rslt != AT_OK) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthCmdInd: WARNING: Hex to Ascii trans fail!");
        PS_MEM_FREE(WUEPS_PID_AT, eapMsg);
        return VOS_ERR;
    }

    eapMsg[0] = '"';
    eapMsg[eapMsgLen - AT_DOUBLE_QUO_MARKS_LENGTH] = '"';

    /* sNssai只有1个，因此sNssaiNum为1 */
    AT_ConvertMultiSNssaiToString(1, &cmdInd->sNssai, nssai, sizeof(nssai), &nssaiLen);
    if (nssaiLen == 0) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthCmdInd: WARNING: SNssai's string length is 0!");
        PS_MEM_FREE(WUEPS_PID_AT, eapMsg);
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s:\"%s\",%s%s", g_atCrLf, g_atStringTab[AT_STRING_C5GNSSAA].text, nssai,
        eapMsg, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    PS_MEM_FREE(WUEPS_PID_AT, eapMsg);
    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaNwSliceAuthRsltInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg                *mtaMsg = VOS_NULL_PTR;
    MTA_AT_NwSliceAuthRsltInd *rsltInd = VOS_NULL_PTR;
    VOS_UINT8                 *eapMsg = VOS_NULL_PTR;
    VOS_UINT32                 nssaiLen;
    VOS_UINT32                 eapMsgLen;
    VOS_UINT32                 rslt;
    VOS_CHAR                   nssai[AT_EVT_MULTI_S_NSSAI_LEN] = {0};

    /* 初始化消息变量 */
    mtaMsg = (AT_MTA_Msg *)msg;
    rsltInd = (MTA_AT_NwSliceAuthRsltInd *)mtaMsg->content;
    if (rsltInd->eapMsg.eapMsgSize > MTA_AT_EAP_MSG_MAX_NUM) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthRsltInd: WARNING: eapMsgSize is too large!");
        return VOS_ERR;
    }
    eapMsgLen = rsltInd->eapMsg.eapMsgSize * sizeof(VOS_UINT8) * 2 + 3;
    eapMsg = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, eapMsgLen);
    if (eapMsg == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthRsltInd: WARNING: Alloc Mem Fail!");
        return VOS_ERR;
    }
    (VOS_VOID)memset_s(eapMsg, sizeof(*eapMsg), 0x00, sizeof(*eapMsg));
    eapMsg[eapMsgLen - 1] = '\0';

    rslt = AT_HexToAsciiString(&eapMsg[1], (eapMsgLen - 3), rsltInd->eapMsg.eapMsg, rsltInd->eapMsg.eapMsgSize);
    if (rslt != AT_OK) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthRsltInd: WARNING: Hex to Ascii trans fail!");
        PS_MEM_FREE(WUEPS_PID_AT, eapMsg);
        return VOS_ERR;
    }

    eapMsg[0] = '"';
    eapMsg[eapMsgLen - AT_DOUBLE_QUO_MARKS_LENGTH] = '"';

    /* sNssai只有1个，因此sNssaiNum为1 */
    AT_ConvertMultiSNssaiToString(1, &rsltInd->sNssai, nssai, sizeof(nssai), &nssaiLen);
    if (nssaiLen == 0) {
        AT_WARN_LOG("AT_RcvMtaNwSliceAuthRsltInd: WARNING: SNssai's string length is 0!");
        PS_MEM_FREE(WUEPS_PID_AT, eapMsg);
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s:\"%s\",%s%s", g_atCrLf, g_atStringTab[AT_STRING_C5GNSSAA].text, nssai,
        eapMsg, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    PS_MEM_FREE(WUEPS_PID_AT, eapMsg);
    return VOS_OK;
}
#endif
#endif

VOS_UINT32 AT_RcvMtaNCellMonitorInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg   = VOS_NULL_PTR;
    MTA_AT_NcellMonitorInd *mtaAtInd = VOS_NULL_PTR;
    VOS_UINT16              length;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    mtaAtInd = (MTA_AT_NcellMonitorInd *)rcvMsg->content;
    length   = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^NCELLMONITOR: %d%s", g_atCrLf, mtaAtInd->ncellState, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvAtMmaUsimStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MMA_UsimStatusInd *atMmaUsimStatusIndMsg = (AT_MMA_UsimStatusInd *)msg;
    AT_USIM_InfoCtx      *usimInfoCtx           = VOS_NULL_PTR;
    errno_t               memResult;
    ModemIdUint16         modemId;

    modemId = AT_GetModemIDFromPid(VOS_GET_SENDER_ID(atMmaUsimStatusIndMsg));

    if (modemId >= MODEM_ID_BUTT) {
        AT_PR_LOGE("enModemId :%d , ulSenderPid :%d", modemId, VOS_GET_SENDER_ID(atMmaUsimStatusIndMsg));

        return VOS_ERR;
    }

    usimInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);

    /* 刷新卡状态全局变量 */
    usimInfoCtx->cardType       = atMmaUsimStatusIndMsg->cardType;
    usimInfoCtx->cardMediumType = atMmaUsimStatusIndMsg->cardMediumType;
    usimInfoCtx->cardStatus     = atMmaUsimStatusIndMsg->cardStatus;
    usimInfoCtx->imsiLen        = atMmaUsimStatusIndMsg->imsiLen;
    memResult                   = memcpy_s(usimInfoCtx->imsi, sizeof(usimInfoCtx->imsi), atMmaUsimStatusIndMsg->imsi,
                                           NAS_MAX_IMSI_LENGTH);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(usimInfoCtx->imsi), NAS_MAX_IMSI_LENGTH);

    AT_PR_LOGI("CardType: %d , CardStatus: %d , ulSenderPid: %d", atMmaUsimStatusIndMsg->cardType,
               atMmaUsimStatusIndMsg->cardStatus, VOS_GET_SENDER_ID(atMmaUsimStatusIndMsg));

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaXcposrRptInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_XcposrrptInd *event = (MTA_AT_XcposrrptInd *)mtaMsg->content;

    /* 根据当前的AT_GetModemAgpsCtxAddrFromModemId(MODEM_ID_0)->enXcposrReport的值判断是否允许上报主动清除申请 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^XCPOSRRPT: %d%s", g_atCrLf, event->clearFlg, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaRrcStatInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RrcstatInd *rrcStat = (TAF_MMA_RrcstatInd *)msg;

    if (rrcStat->rrcCampStat.netRat == TAF_MMA_RAT_LTE) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^RRCSTAT: %d,%d%s",
            g_atCrLf, rrcStat->rrcCampStat.rrcStat, rrcStat->rrcCampStat.tempCampStat, g_atCrLf);
    }

#if (FEATURE_ON == FEATURE_UE_MODE_NR)
    if (rrcStat->rrcCampStat.netRat == TAF_MMA_RAT_NR) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^RRCSTAT: %d%s", g_atCrLf, rrcStat->rrcCampStat.rrcStat, g_atCrLf);
    }
#endif

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLteCaCellExInfoNtf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *mtaMsg        = VOS_NULL_PTR;
    MTA_AT_CaCellInfoNtf *caCellInfoNtf = VOS_NULL_PTR;
    VOS_UINT32            i;

    /* 初始化消息变量 */
    mtaMsg        = (AT_MTA_Msg *)msg;
    caCellInfoNtf = (MTA_AT_CaCellInfoNtf *)mtaMsg->content;

    g_atSendDataBuff.bufLen = 0;

    caCellInfoNtf->totalCellNum = TAF_MIN(caCellInfoNtf->totalCellNum, MTA_AT_CA_MAX_CELL_NUM);
    for (i = 0; i < caCellInfoNtf->totalCellNum; i++) {
        g_atSendDataBuff.bufLen +=
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d", g_atCrLf,
                g_atStringTab[AT_STRING_LCACELLEX].text, caCellInfoNtf->totalCellNum,
                caCellInfoNtf->cellInfo[i].cellIndex, caCellInfoNtf->cellInfo[i].ulConfigured,
                caCellInfoNtf->cellInfo[i].dlConfigured, caCellInfoNtf->cellInfo[i].actived,
                caCellInfoNtf->cellInfo[i].laaScellFlg, caCellInfoNtf->cellInfo[i].bandInd,
                caCellInfoNtf->cellInfo[i].bandWidth, caCellInfoNtf->cellInfo[i].earfcn);

        if (i == caCellInfoNtf->totalCellNum - 1) {
            g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s",
                g_atCrLf);
        }
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNrCaCellInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NrCaCellInfoInd *cnf = VOS_NULL_PTR;
    VOS_UINT32 i;
    VOS_UINT32 length = 0;

    rcvMsg = (AT_MTA_Msg *)msg;
    cnf = (MTA_AT_NrCaCellInfoInd *)rcvMsg->content;

    g_atSendDataBuff.bufLen = 0;
    cnf->cellNum = TAF_MIN(cnf->cellNum, MTA_AT_CA_MAX_CELL_NUM);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s: %d",
        g_atCrLf, g_atStringTab[AT_STRING_NRCACELLRPT].text, cnf->cellNum);
    for (i = 0; i < cnf->cellNum; i++) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d,%d",
            cnf->cellInfo[i].cellIdx, cnf->cellInfo[i].dlConfigured, cnf->cellInfo[i].nulConfigured,
            cnf->cellInfo[i].sulConfigured);
    }
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
    g_atSendDataBuff.bufLen = (VOS_UINT16)length;
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaNrNwCapInfoReportInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NrNwCapInfoReportInd *rptContent = VOS_NULL_PTR;
    VOS_UINT32 length = 0;

    rcvMsg = (AT_MTA_Msg *)msg;
    rptContent = (MTA_AT_NrNwCapInfoReportInd *)rcvMsg->content;

    AT_NORM_LOG1("AT_RcvMtaNwCaCapInfoReportInd : type is", rptContent->type);
    /* 添加类型判断，避免底层异常上报触发AT上报后唤醒AP */
    if (rptContent->type == AT_MTA_NR_NW_CAP_INFO_SA) {
        g_atSendDataBuff.bufLen = 0;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_NRNWCAP].text, rptContent->type,
            rptContent->capInfo.commPara.para1, rptContent->capInfo.commPara.para2,
            rptContent->capInfo.commPara.para3, rptContent->capInfo.commPara.para4,
            rptContent->capInfo.commPara.para5, rptContent->capInfo.commPara.para6,
            rptContent->capInfo.commPara.para7, rptContent->capInfo.commPara.para8,
            rptContent->capInfo.commPara.para9, rptContent->capInfo.commPara.para10,
            g_atCrLf);

        g_atSendDataBuff.bufLen = (VOS_UINT16)length;
        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    }
    return VOS_OK;
}

#endif
#endif

VOS_UINT32 AT_RcvMtaHfreqinfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *rcvMsg        = VOS_NULL_PTR;
    MTA_AT_HfreqinfoInd *mtaAtHfreqInd = VOS_NULL_PTR;
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32 loop;
#endif
    VOS_UINT16 length;

    rcvMsg        = (AT_MTA_Msg *)msg;
    mtaAtHfreqInd = (MTA_AT_HfreqinfoInd *)rcvMsg->content;
    length   = 0;

    if (mtaAtHfreqInd->result == MTA_AT_RESULT_NO_ERROR) {
        if (mtaAtHfreqInd->resultType == MTA_AT_RESULT_TYPE_LTE) {
            /* LTE */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s^HFREQINFO: %d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                HFREQ_INFO_RAT_LTE, mtaAtHfreqInd->lteHfreqInfo.band, mtaAtHfreqInd->lteHfreqInfo.dlEarfcn,
                mtaAtHfreqInd->lteHfreqInfo.dlFreq, mtaAtHfreqInd->lteHfreqInfo.dlBandWidth,
                mtaAtHfreqInd->lteHfreqInfo.ulEarfcn, mtaAtHfreqInd->lteHfreqInfo.ulFreq,
                mtaAtHfreqInd->lteHfreqInfo.ulBandWidth, g_atCrLf);
        }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (mtaAtHfreqInd->resultType == MTA_AT_RESULT_TYPE_NR) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s^HFREQINFO: %d", g_atCrLf, HFREQ_INFO_RAT_NR);
            for (loop = 0; loop < (VOS_UINT32)AT_MIN(mtaAtHfreqInd->nrCellNum, MTA_AT_MAX_CC_NUMBER); loop++) {
                if (mtaAtHfreqInd->nrHfreqInfo[loop].ulMode == MTA_AT_UL_MODE_SUL_ONLY) {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", mtaAtHfreqInd->nrHfreqInfo[loop].sulBand);
                } else {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", mtaAtHfreqInd->nrHfreqInfo[loop].band);
                }
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d", mtaAtHfreqInd->nrHfreqInfo[loop].dlNarfcn,
                    mtaAtHfreqInd->nrHfreqInfo[loop].dlFreq, mtaAtHfreqInd->nrHfreqInfo[loop].dlBandWidth);

                if (mtaAtHfreqInd->nrHfreqInfo[loop].ulMode == MTA_AT_UL_MODE_SUL_ONLY) {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d",
                        mtaAtHfreqInd->nrHfreqInfo[loop].sulNarfcn, mtaAtHfreqInd->nrHfreqInfo[loop].sulFreq,
                        mtaAtHfreqInd->nrHfreqInfo[loop].sulBandWidth);
                } else {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d",
                        mtaAtHfreqInd->nrHfreqInfo[loop].ulNarfcn, mtaAtHfreqInd->nrHfreqInfo[loop].ulFreq,
                        mtaAtHfreqInd->nrHfreqInfo[loop].ulBandWidth);
                }
            }
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
#endif
    }

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaFastReturn5gEndcInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "^CONNECTRECOVERY");
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    return VOS_OK;
}

#endif

VOS_UINT32 AT_RcvCssCloudDataReport(struct MsgCB *msg)
{
    /* 定义局部变量 */
    CSS_AT_CloudDataReport *cssReport = VOS_NULL_PTR;
    VOS_UINT16              length    = 0;
    VOS_UINT8               indexNum  = 0;

    /* 初始化消息变量 */
    cssReport = (CSS_AT_CloudDataReport *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(cssReport->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssCloudDataReport: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if((cssReport->dataLen == 0) || (cssReport->dataLen > AT_CSS_MSG_DATALEN_REPORT_MAX_SIZE)){
        AT_WARN_LOG("AT_RcvCssCloudDataReport: WARNING:data length is invalid!");
        return VOS_ERR;
    }

    /* 构造上报给Ril的length信息 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,", g_atCrLf, g_atStringTab[AT_STRING_CLOUD_DATA].text,
        cssReport->dataLen);

    length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                (g_atSndCodeAddress + length), cssReport->data,
                (TAF_UINT16)cssReport->dataLen);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能说明: 收到网络下发的运营商名称更新后AT处理函数
 * 输入参数: msg: 运营商名称变更消息
 * 返回结果: VOS_ERR: 处理失败
 *           VOS_OK: 处理成功
 */
VOS_UINT32 AT_RcvMmaNwNameChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TafMmaNwNameChangeInd *nwNameChangeInd = (TafMmaNwNameChangeInd *)msg;
    VOS_UINT16 length = 0;

    /* 如果不是运营商名称变化的话返回错误 */
    if (nwNameChangeInd->changeFlg == VOS_FALSE) {
        return AT_ERROR;
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\r\n^EONS: 0\r\n");
    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return AT_OK;
}

/*
 * 功能说明: 收到^NETSELOPT主动上报结果
 * 输入参数: msg: 收到的设置命令结果
 * 返回结果: VOS_ERR: 执行失败
 *           VOS_OK: 执行成功
 */
VOS_UINT32 AT_RcvMmaNetSelOptInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TafMmaNetSelOptSetInd *netSelOptInfoInd = (TafMmaNetSelOptSetInd *)msg;
    VOS_UINT16 length = 0;

    /* 当前不支持智能选网外的主动上报 */
    if (netSelOptInfoInd->scene != SCENE_TYPE_INTELLIGENT_NET_SEL) {
        return AT_ERROR;
    }

    /* 填充结束符 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^NETSELOPT: %d,%d",
        netSelOptInfoInd->scene, netSelOptInfoInd->operate);

    /* 填充结束符 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return AT_OK;
}

/*
 * 功能说明: 处理CERSSI的HCSQ上报
 * 输入参数: indexNum: AT上报通道
 *           rssiInfo: 上报的信号信息
 */
VOS_VOID AT_RptHcsqInfo(VOS_UINT8 indexNum, TAF_MMA_RssiInfoInd *rssiInfo)
{
    VOS_UINT32 result;
    VOS_UINT16 length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;

    /* 获取MODEM ID */
    result = AT_GetModemIdFromClient(indexNum, &modemId);
    if (result != VOS_OK) {
        return;
    }

    /* 当前制式无效，返回 */
    if (rssiInfo->rssiInfo.ratType >= TAF_MMA_RAT_BUTT) {
        return;
    }

    /* 主动上报之前打印新行 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    /* 根据信号打印各制式下的HCSQ信息 */
    AT_FormatHcsqResult(&length, &(rssiInfo->rssiInfo), VOS_TRUE);

    /* 主动上报之前打印新行 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

#endif

