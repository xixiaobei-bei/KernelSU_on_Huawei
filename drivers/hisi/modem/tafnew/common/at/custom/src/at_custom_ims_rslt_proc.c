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
#include "at_custom_ims_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_external_module_msg_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_IMS_RSLT_PROC_C

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaImsSmsCfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ImsSmsCfgQryCnf *imsSmsCfgQryCnf = (TAF_MMA_ImsSmsCfgQryCnf *)msg;
    VOS_UINT32               result;

    g_atSendDataBuff.bufLen = 0;

    if (imsSmsCfgQryCnf->result == TAF_ERR_NO_ERROR) {
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                imsSmsCfgQryCnf->imsSmsCfg.wifiEnable, imsSmsCfgQryCnf->imsSmsCfg.lteEnable,
                imsSmsCfgQryCnf->imsSmsCfg.utranEnable, imsSmsCfgQryCnf->imsSmsCfg.gsmEnable);

        result = AT_OK;
    } else {
        result = At_ChgTafErrorCode(indexNum, imsSmsCfgQryCnf->result);
    }

    return result;
}

VOS_UINT32 AT_RcvImsaRegErrRptQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    IMSA_AT_RegerrReportQryCnf *errRptQryCnf = (IMSA_AT_RegerrReportQryCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (errRptQryCnf->result == VOS_OK) {
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                errRptQryCnf->reportFlag);

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaImsSrvStatRptQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImsSrvStatReportQryCnf *imsSrvStatRptQryCnf = (IMSA_AT_ImsSrvStatReportQryCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (imsSrvStatRptQryCnf->result == VOS_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            (VOS_UINT32)(imsSrvStatRptQryCnf->imsSrvStatRpt));

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaImsSrvStatusQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImsServiceStatusQryCnf *imsSrvStatusQryCnf = (IMSA_AT_ImsServiceStatusQryCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (imsSrvStatusQryCnf->result == VOS_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, imsSrvStatusQryCnf->imsSrvStatInfo.smsSrvStatus,
            imsSrvStatusQryCnf->imsSrvStatInfo.smsSrvRat, imsSrvStatusQryCnf->imsSrvStatInfo.voIpSrvStatus,
            imsSrvStatusQryCnf->imsSrvStatInfo.voIpSrvRat, imsSrvStatusQryCnf->imsSrvStatInfo.vtSrvStatus,
            imsSrvStatusQryCnf->imsSrvStatInfo.vtSrvRat, imsSrvStatusQryCnf->imsSrvStatInfo.vsSrvStatus,
            imsSrvStatusQryCnf->imsSrvStatInfo.vsSrvRat);

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaSipPortQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_TransportTypeQryCnf *sipTransPort = (IMSA_AT_TransportTypeQryCnf *)msg;

    /* 判断查询操作是否成功 */
    if (sipTransPort->result == VOS_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            sipTransPort->tcpThreshold);

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_VOID AT_PS_ReportImsaFusioncCallCtrlMsgu(VOS_UINT8 indexNum, VOS_UINT8 srcId, VOS_UINT32 msgLen, VOS_UINT8 *msgData)
{
    /* 定义局部变量 */
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^FUSIONCALLRAWU: %d,%d,\"", g_atCrLf, srcId, msgLen);

    length += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (VOS_INT8 *)g_atSndCodeAddress,
                                                  (VOS_UINT8 *)g_atSndCodeAddress + length, msgData, (VOS_UINT16)msgLen);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_UINT32 AT_RcvImsaFusionCallCtrlMsg(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    IMSA_AT_FusionCallCtrlMsg *imsFusionCall = (IMSA_AT_FusionCallCtrlMsg *)msg;
    imsFusionCall->msgLen = AT_MIN(imsFusionCall->msgLen, AT_IMSA_FUSIONCALL_DATA_MAX_LEN);

    AT_PS_ReportImsaFusioncCallCtrlMsgu(indexNum, imsFusionCall->srcId, imsFusionCall->msgLen, imsFusionCall->msgData);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaVolteImpiQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_VolteimpiQryCnf *impiCnf = (IMSA_AT_VolteimpiQryCnf *)msg;
    errno_t                  memResult;
    VOS_UINT16               length = 0;
    VOS_CHAR                 acString[AT_IMSA_IMPI_MAX_LENGTH + 1];

    (VOS_VOID)memset_s(acString, sizeof(acString), 0x00, sizeof(acString));
    impiCnf->impiLen = AT_MIN(impiCnf->impiLen, AT_IMSA_IMPI_MAX_LENGTH);

    if (impiCnf->impiLen > 0) {
        memResult = memcpy_s(acString, sizeof(acString), impiCnf->impi, impiCnf->impiLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acString), impiCnf->impiLen);
    }

    /* 判断查询操作是否成功 */
    if (impiCnf->result == VOS_OK) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName, acString);

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaVolteDomainQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_VoltedomainQryCnf *domainCnf = (IMSA_AT_VoltedomainQryCnf *)msg;
    errno_t                    memResult;
    VOS_CHAR                   acString[AT_IMSA_DOMAIN_MAX_LENGTH + 1];

    (VOS_VOID)memset_s(acString, sizeof(acString), 0x00, sizeof(acString));
    domainCnf->domainLen = AT_MIN(domainCnf->domainLen, AT_IMSA_DOMAIN_MAX_LENGTH);

    if (domainCnf->domainLen > 0) {
        memResult = memcpy_s(acString, sizeof(acString), domainCnf->domain, domainCnf->domainLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acString), domainCnf->domainLen);
    }

    /* 判断查询操作是否成功 */
    if (domainCnf->result == VOS_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName, acString);
        return AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvImsaImsUrspSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImsUrspSetCnf *setCnf = (IMSA_AT_ImsUrspSetCnf *)msg;
    VOS_UINT32             result;
    VOS_UINT8              broadCastIdx;

    /* 判断设置操作是否成功 */
    if (setCnf->result == VOS_OK) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;

     /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);

    if (setCnf->result != VOS_OK) {
        if (At_ClientIdToUserBroadCastId(setCnf->appCtrl.clientId, &broadCastIdx) != AT_SUCCESS) {
            AT_WARN_LOG("AT_RcvImsaImsUrspSetCnf: At_ClientIdToUserBroadCastId is err!");
            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^IMSURSPVERSION: %d%s", g_atCrLf, setCnf->version, g_atCrLf);

        At_SendResultData(broadCastIdx, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    }

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}
#endif

#if (FEATURE_IMS == FEATURE_ON)
#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_RcvImsaEcallContentTypeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_EcallContentTypeQryCnf      *contentTypeQryCnf = (IMSA_AT_EcallContentTypeQryCnf *)msg;

    /* 判断查询操作是否成功 */
    if (contentTypeQryCnf->result == VOS_ERR) {
        return AT_ERROR;
    } else {
        /* 如果CONTENT TYPE为ECALL 客户定制，则返回CONTENT TYOE设置模式及内容，否则只返回设置模式 */
        if (contentTypeQryCnf->contentTypeMode == AT_IMSA_ECALL_CONTENT_TYPE_MODE_PS_PREFER_CUSTOM) {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,\"%s\"", g_parseContext[indexNum].cmdElement->cmdName,
                contentTypeQryCnf->contentTypeMode, contentTypeQryCnf->contentTypeContext);
        } else {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                contentTypeQryCnf->contentTypeMode);
        }

        return AT_OK;
    }
}
#endif
#endif

