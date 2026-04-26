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
#include "at_data_taf_rslt_proc.h"
#include "securec.h"
#include "at_data_proc.h"
#include "at_data_comm.h"
#include "at_event_report.h"
#include "at_mdrv_interface.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_TAF_RSLT_PROC_C

#define AT_SDU_ERR_RATIO_STATUS_0 0
#define AT_SDU_ERR_RATIO_STATUS_1 1
#define AT_SDU_ERR_RATIO_STATUS_2 2
#define AT_SDU_ERR_RATIO_STATUS_3 3
#define AT_SDU_ERR_RATIO_STATUS_4 4
#define AT_SDU_ERR_RATIO_STATUS_5 5
#define AT_SDU_ERR_RATIO_STATUS_6 6
#define AT_SDU_ERR_RATIO_STATUS_7 7
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_0 0
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_1 1
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_2 2
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_3 3
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_4 4
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_5 5
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_6 6
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_7 7
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_8 8
#define AT_RESIDUAL_BIT_ERR_RATIO_STATUS_9 9
#define TAF_MAX_LTE_APN_IMSI_PREFIX_SUPPORT 10
#define AT_BYTE_TO_BITS_LENS 8
#define AT_EVENT_INFO_ARRAY_MAX_LEN 4


#define AT_PACK_CREAT_PPP_EVENT(eventInfo, pppId, indexNum) do { \
        eventInfo[AT_EVENT_INFO_ARRAY_INDEX_0] = (TAF_UINT8)(pppId >> 8); \
        eventInfo[AT_EVENT_INFO_ARRAY_INDEX_1] = (TAF_UINT8)pppId;        \
        eventInfo[AT_EVENT_INFO_ARRAY_INDEX_2] = indexNum;                \
    } while (0)

VOS_UINT32 AT_RcvTafPsEvtSetUmtsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetUmtsQosInfoCnf *setUmtsQosInfoCnf = VOS_NULL_PTR;

    setUmtsQosInfoCnf = (TAF_PS_SetUmtsQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQREQ_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setUmtsQosInfoCnf->cause);

    return VOS_OK;
}

/* 格式化输出SDU error ratio */
VOS_VOID AT_FormatTafPsEvtGetUmtsQosInfoSduErrRatio(VOS_UINT8  sduErrRatio, VOS_UINT16 *length)
{
    switch (sduErrRatio) {
        case AT_SDU_ERR_RATIO_STATUS_0:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_0E0].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_1:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E2].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_2:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_7E3].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_3:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E3].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_4:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E4].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_5:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E5].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_6:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E6].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_7:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E1].text);
            break;

        default:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
            break;
    }
}

/* 格式化输出Residual bit error ratio */
VOS_VOID AT_FormatTafPsEvtGetUmtsQosInfoResidualBer(VOS_UINT8 residualBer, VOS_UINT16 *length)
{
    switch (residualBer) {
        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_0:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_0E0].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_1:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_5E2].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_2:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E2].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_3:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_5E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_4:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_4E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_5:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_6:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E4].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_7:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E5].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_8:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E6].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_9:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_6E8].text);
            break;

        default:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
            break;
    }
}

VOS_UINT32 AT_RcvTafPsEvtGetUmtsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                   memResult;
    TAF_UINT32                result;
    VOS_UINT16                length      = 0;
    TAF_UINT32                tmp         = 0;
    TAF_PS_GetUmtsQosInfoCnf *umtsQosInfo = VOS_NULL_PTR;
    TAF_UMTS_QosQueryInfo     cgeq;

    (VOS_VOID)memset_s(&cgeq, sizeof(cgeq), 0x00, sizeof(TAF_UMTS_QosQueryInfo));

    umtsQosInfo = (TAF_PS_GetUmtsQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQREQ_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < umtsQosInfo->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cgeq, sizeof(cgeq), &umtsQosInfo->umtsQosQueryInfo[tmp], sizeof(TAF_UMTS_QosQueryInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgeq), sizeof(TAF_UMTS_QosQueryInfo));
        /* +CGEQREQ:+CGEQMIN   */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", cgeq.cid);
        /* <Traffic class> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.trafficClass);
        /* <Maximum bitrate UL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxBitUl);
        /* <Maximum bitrate DL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxBitDl);
        /* <Guaranteed bitrate UL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.guarantBitUl);
        /* <Guaranteed bitrate DL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.guarantBitDl);
        /* <Delivery order> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.deliverOrder);
        /* <Maximum SDU size> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxSduSize);
        /* <SDU error ratio> */
        AT_FormatTafPsEvtGetUmtsQosInfoSduErrRatio(cgeq.qosInfo.sduErrRatio, &length);

        /* <Residual bit error ratio> */
        AT_FormatTafPsEvtGetUmtsQosInfoResidualBer(cgeq.qosInfo.residualBer, &length);

        /* <Delivery of erroneous SDUs> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.deliverErrSdu);
        /* <Transfer delay> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.transDelay);
        /* <Traffic handling priority> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.traffHandlePrior);

        /* <Source Statistics Descriptor> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.srcStatisticsDescriptor);
        /* <Signalling Indication> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.signallingIndication);
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetUmtsQosMinInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetUmtsQosMinInfoCnf *setUmtsQosMinInfoCnf = VOS_NULL_PTR;

    setUmtsQosMinInfoCnf = (TAF_PS_SetUmtsQosMinInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQMIN_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setUmtsQosMinInfoCnf->cause);

    return VOS_OK;
}

/* 格式化输出SDU error ratio */
VOS_VOID AT_FormatTafPsEvtGetUmtsQosMinInfoSduErrRatio(VOS_UINT8 sduErrRatio, VOS_UINT16 *length)
{
    switch (sduErrRatio) {
        case AT_SDU_ERR_RATIO_STATUS_0:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_0E0].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_1:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E2].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_2:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_7E3].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_3:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E3].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_4:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E4].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_5:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E5].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_6:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E6].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_7:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E1].text);
            break;

        default:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
            break;
    }
}

/* 格式化输出Residual bit error ratio */
VOS_VOID AT_FormatTafPsEvtGetUmtsQosMinInfoResidualBer(VOS_UINT8 residualBer, VOS_UINT16 *length)
{
    switch (residualBer) {
        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_0:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_0E0].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_1:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_5E2].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_2:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E2].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_3:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_5E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_4:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_4E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_5:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_6:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E4].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_7:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E5].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_8:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E6].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_9:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_6E8].text);
            break;

        default:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
            break;
    }
}

VOS_UINT32 AT_RcvTafPsEvtGetUmtsQosMinInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                      memResult;
    TAF_UINT32                   result;
    TAF_UINT16                   length         = 0;
    TAF_UINT32                   tmp            = 0;
    TAF_PS_GetUmtsQosMinInfoCnf *umtsQosMinInfo = VOS_NULL_PTR;
    TAF_UMTS_QosQueryInfo        cgeq;

    (VOS_VOID)memset_s(&cgeq, sizeof(cgeq), 0x00, sizeof(TAF_UMTS_QosQueryInfo));

    umtsQosMinInfo = (TAF_PS_GetUmtsQosMinInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQMIN_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < umtsQosMinInfo->cidNum; tmp++) {
        if (tmp != 0) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cgeq, sizeof(cgeq), &umtsQosMinInfo->umtsQosQueryInfo[tmp],
                             sizeof(TAF_UMTS_QosQueryInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgeq), sizeof(TAF_UMTS_QosQueryInfo));
        /* +CGEQREQ:+CGEQMIN   */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d", cgeq.cid);
        /* <Traffic class> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.trafficClass);
        /* <Maximum bitrate UL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxBitUl);
        /* <Maximum bitrate DL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxBitDl);
        /* <Guaranteed bitrate UL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.guarantBitUl);
        /* <Guaranteed bitrate DL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.guarantBitDl);
        /* <Delivery order> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.deliverOrder);
        /* <Maximum SDU size> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxSduSize);
        /* <SDU error ratio> */
        AT_FormatTafPsEvtGetUmtsQosMinInfoSduErrRatio(cgeq.qosInfo.sduErrRatio, &length);

        /* <Residual bit error ratio> */
        AT_FormatTafPsEvtGetUmtsQosMinInfoResidualBer(cgeq.qosInfo.residualBer, &length);

        /* <Delivery of erroneous SDUs> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.deliverErrSdu);
        /* <Transfer delay> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.transDelay);
        /* <Traffic handling priority> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.traffHandlePrior);

        /* <Source Statistics Descriptor> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.srcStatisticsDescriptor);
        /* <Signalling Indication> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.signallingIndication);
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetTftInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetTftInfoCnf *setTftInfoCnf = VOS_NULL_PTR;

    setTftInfoCnf = (TAF_PS_SetTftInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGTFT_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setTftInfoCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_Ipv6Addr2DecString(VOS_CHAR *pcIpv6FormatStr, VOS_UINT8 aucIpv6Addr[])
{
    VOS_UINT32 length;
    VOS_UINT32 loop;
    VOS_INT    iRslt;

    length = 0;

    /* 循环打印10进制点分IPv6地址 */
    for (loop = 0; loop < AT_IPV6_ADDR_DEC_TOKEN_NUM; loop++) {
        /* 打印点分隔符 */
        if (loop != 0) {
            *(pcIpv6FormatStr + length) = AT_IP_STR_DOT_DELIMITER;
            length++;
        }

        iRslt = snprintf_s(pcIpv6FormatStr + length, AT_IPV6_ADDR_DEC_FORMAT_STR_LEN - length,
                           (AT_IPV6_ADDR_DEC_FORMAT_STR_LEN - length) - 1, "%d", aucIpv6Addr[loop]);

        if (iRslt <= 0) {
            AT_WARN_LOG("AT_Ipv6Addr2DecString: Print IPv6 Addr Failed!");
            return 0;
        }

        length += iRslt;
    }

    return length;
}

VOS_VOID AT_ConvertIpv6AddrToHexAddrAndGetMaxZeroCnt(VOS_UINT8 aucIpv6Addr[], VOS_UINT16 ausIpv6HexAddr[],
                                                     VOS_UINT32 *zeroStartIndex, VOS_UINT32 *zeroMaxCnt)
{
    VOS_UINT32 loop;
    VOS_UINT32 zeroTmpIndex;
    VOS_UINT32 zeroTmpCnt;

    zeroTmpIndex = 0;
    zeroTmpCnt   = 0;

    /* 循环转换IPv6格式地址，并统计最长连续零段 */
    for (loop = 0; loop < AT_IPV6_ADDR_HEX_TOKEN_NUM; loop++) {
        /* 合并字节 */
        ausIpv6HexAddr[loop] = *(aucIpv6Addr + loop + loop);
        ausIpv6HexAddr[loop] <<= 8;
        ausIpv6HexAddr[loop] |= *(aucIpv6Addr + loop + loop + 1);

        if (ausIpv6HexAddr[loop] == 0) {
            /* 如果16进制字段为0，则记录临时cnt和Index */
            if (zeroTmpCnt == 0) {
                zeroTmpIndex = loop;
            }

            zeroTmpCnt++;
        } else {
            /* 如果16进制字段不为0，则判断是否更新最长连续零记录 */
            if (zeroTmpCnt > *zeroMaxCnt) {
                *zeroStartIndex = zeroTmpIndex;
                *zeroMaxCnt     = zeroTmpCnt;
            }

            zeroTmpCnt   = 0;
            zeroTmpIndex = 0;
        }
    }

    /* 判断是否更新最长连续零记录 */
    if (zeroTmpCnt > *zeroMaxCnt) {
        *zeroStartIndex = zeroTmpIndex;
        *zeroMaxCnt     = zeroTmpCnt;
    }
}

VOS_UINT32 AT_Ipv6Addr2HexString(VOS_CHAR *pcIpv6FormatStr, VOS_UINT8 aucIpv6Addr[])
{
    VOS_UINT32    length;
    AT_CommPsCtx *commPsCtx   = VOS_NULL_PTR;
    VOS_CHAR     *pcFormatStr = VOS_NULL_PTR;
    VOS_UINT32    loop;
    VOS_UINT16    ipv6HexAddr[AT_IPV6_ADDR_HEX_TOKEN_NUM];
    VOS_UINT32    zeroStartIndex;
    VOS_UINT32    zeroMaxCnt;
    VOS_INT       iRslt;

    /* 局部变量初始化 */
    length         = 0;
    commPsCtx      = AT_GetCommPsCtxAddr();
    zeroStartIndex = 0;
    zeroMaxCnt     = 0;
    (VOS_VOID)memset_s(ipv6HexAddr, sizeof(ipv6HexAddr), 0, sizeof(ipv6HexAddr));

    /* 根据是否保留前导零确认打印格式 */
    pcFormatStr = (commPsCtx->opIpv6LeadingZeros == VOS_FALSE) ? "%04X" : "%X";

    /* 转换IPv6地址为16Bit HEX类型，并统计最大连续零个数 */
    AT_ConvertIpv6AddrToHexAddrAndGetMaxZeroCnt(aucIpv6Addr, ipv6HexAddr, &zeroStartIndex, &zeroMaxCnt);

    /* 循环打印16进制点分IPv6地址 */
    for (loop = 0; loop < AT_IPV6_ADDR_HEX_TOKEN_NUM; loop++) {
        /* 开启零压缩功能，且存在最大连续零，则压缩打印 */
        if ((commPsCtx->opIpv6CompressZeros != VOS_FALSE) && (zeroMaxCnt > 0)) {
            /* 第一个0，打印冒号 */
            if (zeroStartIndex == loop) {
                *(pcIpv6FormatStr + length) = AT_IP_STR_COLON_DELIMITER;
                length++;
                continue;
            }

            /* 其余0不打印 */
            if ((loop > zeroStartIndex) && (loop < (zeroStartIndex + zeroMaxCnt))) {
                /* 最后一位为0，需要多打印一个冒号 */
                if (loop == (AT_IPV6_ADDR_HEX_TOKEN_NUM - 1)) {
                    *(pcIpv6FormatStr + length) = AT_IP_STR_COLON_DELIMITER;
                    length++;
                }

                continue;
            }
        }

        /* 打印冒号分隔符 */
        if (loop != 0) {
            *(pcIpv6FormatStr + length) = AT_IP_STR_COLON_DELIMITER;
            length++;
        }

        iRslt = snprintf_s(pcIpv6FormatStr + length, AT_IPV6_ADDR_COLON_FORMAT_STR_LEN - length,
                           (AT_IPV6_ADDR_COLON_FORMAT_STR_LEN - length) - 1, pcFormatStr, ipv6HexAddr[loop]);

        if (iRslt <= 0) {
            AT_WARN_LOG("AT_Ipv6Addr2HexString: Print IPv6 Addr Failed!");
            return 0;
        }

        length += iRslt;
    }

    return length;
}

VOS_UINT32 AT_Ipv6AddrMask2FormatString(VOS_CHAR *pcIpv6FormatStr, VOS_UINT8 aucIpv6Addr[], VOS_UINT32 addrLen,
        VOS_UINT8 aucIpv6Mask[], VOS_UINT32 maskLen)
{
    VOS_UINT32    length;
    AT_CommPsCtx *commPsCtx = VOS_NULL_PTR;
    VOS_INT       iRslt;

    /* 局部变量初始化 */
    length    = 0;
    commPsCtx = AT_GetCommPsCtxAddr();
    iRslt     = 0;

    /* 必须要有IPv6地址，不单独打印IPv6子网掩码 */
    if (aucIpv6Addr == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_Ipv6AddrMask2FormatString: No IPv6 Address!");
        *pcIpv6FormatStr = '\0';
        return 0;
    }

    if (commPsCtx->opIpv6AddrFormat == VOS_FALSE) {
        /* 10进制点分格式打印IPv6地址 */
        length = AT_Ipv6Addr2DecString(pcIpv6FormatStr, aucIpv6Addr);

        /* 打印子网掩码 */
        if (aucIpv6Mask != VOS_NULL_PTR) {
            /* 使用点分隔IP地址与子网掩码 */
            *(pcIpv6FormatStr + length) = AT_IP_STR_DOT_DELIMITER;
            length++;

            /* 10进制点分格式打印IPv6子网掩码 */
            length += AT_Ipv6Addr2DecString(pcIpv6FormatStr + length, aucIpv6Mask);
        }
    } else {
        /* 16进制冒号分隔格式打印IPv6地址 */
        length = AT_Ipv6Addr2HexString(pcIpv6FormatStr, aucIpv6Addr);

        /* 打印子网掩码 */
        if (aucIpv6Mask != VOS_NULL_PTR) {
            /* 判断子网掩码格式 */
            if (commPsCtx->opIpv6SubnetNotation == VOS_FALSE) {
                /* 完整子网掩码地址与IPv6地址通过空格分隔 */
                /* 使用点分隔IP地址与子网掩码 */
                *(pcIpv6FormatStr + length) = ' ';
                length++;

                /* 10进制点分格式打印IPv6子网掩码 */
                length += AT_Ipv6Addr2HexString(pcIpv6FormatStr + length, aucIpv6Mask);
            } else {
                /* 斜线分隔子网前缀无类域与IPv6地址 */
                /* 使用斜线分隔IP地址与子网掩码 */
                *(pcIpv6FormatStr + length) = '/';
                length++;

                iRslt = snprintf_s(pcIpv6FormatStr + length, AT_IPV6_ADDR_MASK_FORMAT_STR_LEN - length,
                                   (AT_IPV6_ADDR_MASK_FORMAT_STR_LEN - length) - 1, "%d",
                                   AT_CalcIpv6PrefixLength(aucIpv6Mask, TAF_IPV6_ADDR_LEN));
                if (iRslt <= 0) {
                    AT_WARN_LOG("AT_Ipv6AddrMask2FormatString: Print IPv6 Subnet Failed!");
                    *pcIpv6FormatStr = '\0';
                    return 0;
                }

                length += iRslt;
            }
        }
    }

    /* 最后补充字符串结束符 */
    *(pcIpv6FormatStr + length) = '\0';

    return length;
}

VOS_VOID AT_ConvertIpAddrAndMaskParaToString(TAF_TFT_QureyInfo *cgtft, VOS_UINT8 tmp2, VOS_UINT16 *length)
{
    VOS_CHAR acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    (VOS_VOID)memset_s(acIpv4StrTmp, sizeof(acIpv4StrTmp), 0x00, sizeof(acIpv4StrTmp));
    (VOS_VOID)memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, sizeof(acIpv6StrTmp));

    if (cgtft->pfInfo[tmp2].bitOpRmtIpv4AddrAndMask == VOS_TRUE) {
        AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), cgtft->pfInfo[tmp2].rmtIpv4Address);
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s", acIpv4StrTmp);
        AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), cgtft->pfInfo[tmp2].rmtIpv4Mask);
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ".%s\"", acIpv4StrTmp);
    } else if (cgtft->pfInfo[tmp2].bitOpRmtIpv6AddrAndMask == VOS_TRUE) {
        (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp, cgtft->pfInfo[tmp2].rmtIpv6Address, TAF_IPV6_ADDR_LEN,
                                               cgtft->pfInfo[tmp2].rmtIpv6Mask, TAF_IPV6_ADDR_LEN);
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", acIpv6StrTmp);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

LOCAL VOS_VOID AT_PrintTtfSecuParaIndex(VOS_UINT32 opPara, VOS_UINT32 secuParaIndex, VOS_UINT16 *length)
{
    if (opPara == 1) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%X\"", secuParaIndex);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

LOCAL VOS_VOID AT_PrintTtfFlowLabelType(VOS_UINT32 opPara, VOS_UINT32 flowLabelType, VOS_UINT16 *length)
{
    if (opPara == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%X", flowLabelType);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_VOID AT_GetIpv6MaskByPrefixLength(VOS_UINT8 localIpv6Prefix, VOS_UINT8 *localIpv6Mask)
{
    VOS_UINT8  num;
    VOS_UINT32 i = 0;

    num = localIpv6Prefix / AT_BYTE_TO_BITS_LENS;

    if (localIpv6Mask == VOS_NULL_PTR) {
        return;
    }

    for (i = 0; i < num; i++) {
        *(localIpv6Mask + i) = 0xFF;
    }

    num = localIpv6Prefix % AT_BYTE_TO_BITS_LENS;

    if (num != 0) {
        *(localIpv6Mask + i) = 0xFF & ((VOS_UINT32)0xFF << (AT_BYTE_TO_BITS_LENS - num));
    }
}

VOS_VOID AT_ConvertRangeParaToString(VOS_UINT32 opPara1, VOS_UINT32 opPara2, VOS_UINT32 paraVal1, VOS_UINT32 paraVal2,
                                     VOS_UINT16 *length)
{
    if (opPara1 == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", paraVal1);
    } else if (opPara2 == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%d", paraVal1);
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ".%d\"", paraVal2);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_VOID AT_ConvertULParaToString(VOS_UINT32 opPara, VOS_UINT32 paraVal, VOS_UINT16 *length)
{
    if (opPara == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", paraVal);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

LOCAL VOS_VOID AT_PrintCgtftInfoPara(TAF_TFT_QureyInfo *cgtft, VOS_UINT8 tmp2, VOS_UINT16 *length)
{
    VOS_CHAR              acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN] = {0};
    VOS_CHAR              localIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN] = {0};
    VOS_UINT8             localIpv6Mask[TAF_IPV6_ADDR_LEN] = {0};

    /* <cid> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", cgtft->cid);
    /* <packet filter identifier> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgtft->pfInfo[tmp2].packetFilterId);
    /* <evaluation precedence index> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgtft->pfInfo[tmp2].precedence);
    /* <source address and subnet mask> */
    AT_ConvertIpAddrAndMaskParaToString(cgtft, tmp2, length);

    /* <protocol number (ipv4) / next header (ipv6)> */
    AT_ConvertULParaToString(cgtft->pfInfo[tmp2].bitOpProtocolId, cgtft->pfInfo[tmp2].protocolId, length);

    /* <destination port range> */
    AT_ConvertRangeParaToString(cgtft->pfInfo[tmp2].bitOpSingleLocalPort,
                                cgtft->pfInfo[tmp2].bitOpLocalPortRange, cgtft->pfInfo[tmp2].lcPortLowLimit,
                                cgtft->pfInfo[tmp2].lcPortHighLimit, length);

    /* <source port range> */
    AT_ConvertRangeParaToString(cgtft->pfInfo[tmp2].bitOpSingleRemotePort,
                                cgtft->pfInfo[tmp2].bitOpRemotePortRange, cgtft->pfInfo[tmp2].rmtPortLowLimit,
                                cgtft->pfInfo[tmp2].rmtPortHighLimit, length);

    /* <ipsec security parameter index (spi)> */
    AT_PrintTtfSecuParaIndex(cgtft->pfInfo[tmp2].bitOpSecuParaIndex, cgtft->pfInfo[tmp2].secuParaIndex,
                             length);

    /* <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask> */
    AT_ConvertRangeParaToString(VOS_FALSE, cgtft->pfInfo[tmp2].bitOpTypeOfService,
                                cgtft->pfInfo[tmp2].typeOfService, cgtft->pfInfo[tmp2].typeOfServiceMask,
                                length);

    /* <flow label (ipv6)> */
    AT_PrintTtfFlowLabelType(cgtft->pfInfo[tmp2].bitOpFlowLabelType, cgtft->pfInfo[tmp2].flowLabelType,
                             length);

    /* <direction> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgtft->pfInfo[tmp2].direction);

    if (AT_IsSupportReleaseRst(AT_ACCESS_STRATUM_REL11)) {
        /* <local address and subnet mask> */
        if (cgtft->pfInfo[tmp2].bitOpLocalIpv4AddrAndMask == VOS_TRUE) {
            AT_Ipv4AddrItoa(localIpv4StrTmp, sizeof(localIpv4StrTmp), cgtft->pfInfo[tmp2].localIpv4Addr);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s", localIpv4StrTmp);

            AT_Ipv4AddrItoa(localIpv4StrTmp, sizeof(localIpv4StrTmp), cgtft->pfInfo[tmp2].localIpv4Mask);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ".%s\"", localIpv4StrTmp);
        } else if (cgtft->pfInfo[tmp2].bitOpLocalIpv6AddrAndMask == VOS_TRUE) {
            AT_GetIpv6MaskByPrefixLength(cgtft->pfInfo[tmp2].localIpv6Prefix, localIpv6Mask);
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp, cgtft->pfInfo[tmp2].localIpv6Addr, TAF_IPV6_ADDR_LEN,
                                                   localIpv6Mask, TAF_IPV6_ADDR_LEN);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", acIpv6StrTmp);
        } else {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
        }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        AT_ConvertULParaToString(cgtft->pfInfo[tmp2].bitOpQri, cgtft->pfInfo[tmp2].qri, length);
#endif
    }
}

VOS_UINT32 AT_RcvTafPsEvtGetTftInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t               memResult;
    VOS_UINT32            result;
    VOS_UINT16            length = 0;
    VOS_UINT32            tmp1   = 0;
    VOS_UINT8             tmp2   = 0;
    TAF_TFT_QureyInfo    *cgtft         = VOS_NULL_PTR;
    TAF_PS_GetTftInfoCnf *getTftInfoCnf = VOS_NULL_PTR;

    getTftInfoCnf = (TAF_PS_GetTftInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGTFT_READ) {
        return VOS_ERR;
    }

    /* 动态申请内存 */
    cgtft = (TAF_TFT_QureyInfo *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_TFT_QureyInfo));
    if (cgtft == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    (VOS_VOID)memset_s(cgtft, sizeof(TAF_TFT_QureyInfo), 0x00, sizeof(TAF_TFT_QureyInfo));

    for (tmp1 = 0; tmp1 < getTftInfoCnf->cidNum; tmp1++) {
        memResult = memcpy_s(cgtft, sizeof(TAF_TFT_QureyInfo), &getTftInfoCnf->tftQueryInfo[tmp1],
                             sizeof(TAF_TFT_QureyInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TAF_TFT_QureyInfo), sizeof(TAF_TFT_QureyInfo));

        cgtft->pfNum = AT_MIN(cgtft->pfNum, TAF_MAX_SDF_PF_NUM);
        for (tmp2 = 0; tmp2 < cgtft->pfNum; tmp2++) {
            if (!(tmp1 == 0 && tmp2 == 0)) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }
            /* +CGTFT:  */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

            AT_PrintCgtftInfoPara(cgtft, tmp2, &length);
        }
    }

    /* 释放动态申请的内存 */
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, cgtft);
    /*lint -restore */
    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    /* 处理错误码 */
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetAnsModeInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetAnswerModeInfoCnf *setAnsModeInfoCnf = VOS_NULL_PTR;

    setAnsModeInfoCnf = (TAF_PS_SetAnswerModeInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGAUTO_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setAnsModeInfoCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetAnsModeInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT16                   length = 0;
    TAF_PS_GetAnswerModeInfoCnf *callAns = VOS_NULL_PTR;

    /* 初始化 */
    callAns = (TAF_PS_GetAnswerModeInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGAUTO_READ) {
        return VOS_ERR;
    }

    /* +CGAUTO */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", callAns->ansMode);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetPdpIpAddrInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                     memResult;
    VOS_UINT16                  length = 0;
    VOS_UINT32                  tmp    = 0;
    VOS_CHAR                    aStrTmp[TAF_MAX_IPV4_ADDR_STR_LEN] = { 0 };
    VOS_CHAR                    acIPv6Str[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN] = { 0 };
    TAF_PDP_AddrQueryInfo       pdpAddrQuery = { 0 };
    TAF_PS_GetPdpIpAddrInfoCnf *pdpIpAddr = VOS_NULL_PTR;

    /* 初始化 */
    pdpIpAddr = (TAF_PS_GetPdpIpAddrInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGPADDR_SET) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < pdpIpAddr->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&pdpAddrQuery, sizeof(pdpAddrQuery), &pdpIpAddr->pdpAddrQueryInfo[tmp],
                             sizeof(TAF_PDP_AddrQueryInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pdpAddrQuery), sizeof(TAF_PDP_AddrQueryInfo));

        /* +CGPADDR:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", pdpAddrQuery.cid);

        /* <PDP_addr> */
        if ((pdpAddrQuery.pdpAddr.pdpType == TAF_PDP_IPV4) || (pdpAddrQuery.pdpAddr.pdpType == TAF_PDP_PPP)) {
            AT_Ipv4AddrItoa(aStrTmp, sizeof(aStrTmp), pdpAddrQuery.pdpAddr.ipv4Addr);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", aStrTmp);
        } else if (pdpAddrQuery.pdpAddr.pdpType == TAF_PDP_IPV6) {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIPv6Str, pdpAddrQuery.pdpAddr.ipv6Addr, TAF_IPV6_ADDR_LEN,
                    VOS_NULL_PTR, 0);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acIPv6Str);
        } else if (pdpAddrQuery.pdpAddr.pdpType == TAF_PDP_IPV4V6) {
            AT_Ipv4AddrItoa(aStrTmp, sizeof(aStrTmp), pdpAddrQuery.pdpAddr.ipv4Addr);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", aStrTmp);

            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIPv6Str, pdpAddrQuery.pdpAddr.ipv6Addr, TAF_IPV6_ADDR_LEN,
                    VOS_NULL_PTR, 0);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acIPv6Str);
        } else {
            /* TAF_PDP_TYPE_BUTT */
            return VOS_ERR;
        }
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT32                   result = AT_FAILURE;
    VOS_UINT16                   length = 0;
    VOS_UINT32                   tmp = 0;
    TAF_PS_GetPdpContextInfoCnf *getPdpCtxInfoCnf = VOS_NULL_PTR;

    getPdpCtxInfoCnf = (TAF_PS_GetPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGPADDR_TEST) {
        return VOS_ERR;
    }

    /* +CGPADDR:  */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "(");

    for (tmp = 0; tmp < getPdpCtxInfoCnf->cidNum; tmp++) {
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", getPdpCtxInfoCnf->cid[tmp]);

        if ((tmp + 1) >= getPdpCtxInfoCnf->cidNum) {
            break;
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ")");

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetPdpStateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetPdpStateCnf *setPdpStateCnf = VOS_NULL_PTR;

    setPdpStateCnf = (TAF_PS_SetPdpStateCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGACT_ORG_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGACT_END_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGDATA_SET)) {
        return VOS_ERR;
    }

    /*
     * (1)协议栈异常错误, 未发起PDP激活, 直接上报ERROR
     * (2)协议栈正常, 发起PDP激活, 根据PDP激活事件返回结果
     */

    if (setPdpStateCnf->cause != TAF_PS_CAUSE_SUCCESS) {
        /* 记录PS域呼叫错误码 */
        AT_PS_SetPsCallErrCause(indexNum, setPdpStateCnf->cause);

        AT_StopTimerCmdReady(indexNum);

        if (setPdpStateCnf->cause == TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT) {
            At_FormatResultData(indexNum, AT_CME_PDP_ACT_LIMIT);
        } else {
            At_FormatResultData(indexNum, AT_ERROR);
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtCgactQryCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                memResult;
    VOS_UINT16             length = 0;
    VOS_UINT32             tmp    = 0;
    TAF_CID_State          cgact;
    TAF_PS_GetPdpStateCnf *pdpState = VOS_NULL_PTR;

    pdpState = (TAF_PS_GetPdpStateCnf *)evtInfo;

    (VOS_VOID)memset_s(&cgact, sizeof(cgact), 0x00, sizeof(TAF_CID_State));

    /* 检查当前命令的操作类型 */
    for (tmp = 0; tmp < pdpState->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cgact, sizeof(cgact), &pdpState->cidStateInfo[tmp], sizeof(TAF_CID_State));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgact), sizeof(TAF_CID_State));
        /* +CGACT:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", cgact.cid);
        /* <state> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgact.state);
    }

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtCgeqnegTestCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                memResult;
    VOS_UINT16             length = 0;
    VOS_UINT32             qosnegNum = 0;
    VOS_UINT32             tmp;
    TAF_CID_State          cgact;
    TAF_PS_GetPdpStateCnf *pdpState = VOS_NULL_PTR;

    pdpState  = (TAF_PS_GetPdpStateCnf *)evtInfo;

    (VOS_VOID)memset_s(&cgact, sizeof(cgact), 0x00, sizeof(TAF_CID_State));

    /* CGEQNEG的测试命令 */

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", "(");

    for (tmp = 0; tmp < pdpState->cidNum; tmp++) {
        memResult = memcpy_s(&cgact, sizeof(cgact), &pdpState->cidStateInfo[tmp], sizeof(TAF_CID_State));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgact), sizeof(TAF_CID_State));

        if (cgact.state == TAF_PDP_ACTIVE) { /* 如果该CID是激活态,则打印该CID和可能的一个逗号;否则跳过该CID */
            if (qosnegNum == 0) {            /* 如果是第一个CID，则CID前不打印逗号 */
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "%d", cgact.cid);
            } else { /* 如果不是第一个CID，则CID前打印逗号 */
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", cgact.cid);
            }

            qosnegNum++;
        }
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", ")");

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetPdpStateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGACT_READ) {
        return AT_RcvTafPsEvtCgactQryCnf(indexNum, evtInfo);
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGEQNEG_TEST) {
        return AT_RcvTafPsEvtCgeqnegTestCnf(indexNum, evtInfo);

    } else {
        return VOS_ERR;
    }
}

VOS_UINT32 AT_RcvTafPsEvtSetPrimPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetPrimPdpContextInfoCnf *setPdpCtxInfoCnf = VOS_NULL_PTR;

    setPdpCtxInfoCnf = (TAF_PS_SetPrimPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGDCONT_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setPdpCtxInfoCnf->cause);

    return VOS_OK;
}

LOCAL VOS_VOID AT_PrintCgdcontParaPdpType(TAF_PRI_PdpQueryInfo *cgdcont, VOS_UINT16 *length)
{
    if (cgdcont->priPdpInfo.pdpAddr.pdpType == TAF_PDP_IPV4) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%s", g_atStringTab[AT_STRING_IP].text);
    } else if (cgdcont->priPdpInfo.pdpAddr.pdpType == TAF_PDP_IPV6) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%s", g_atStringTab[AT_STRING_IPV6].text);
    } else if (cgdcont->priPdpInfo.pdpAddr.pdpType == TAF_PDP_IPV4V6) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%s", g_atStringTab[AT_STRING_IPV4V6].text);
    } else if (cgdcont->priPdpInfo.pdpAddr.pdpType == TAF_PDP_ETHERNET) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%s", g_atStringTab[AT_STRING_Ethernet].text);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%s", g_atStringTab[AT_STRING_PPP].text);
    }
}

LOCAL VOS_VOID AT_PrintCgdcontParaApn(TAF_PRI_PdpQueryInfo *cgdcont, VOS_UINT16 *length)
{
    VOS_UINT8                       str[TAF_MAX_APN_LEN + 1] = {0};
    errno_t                         memResult;

    if (cgdcont->priPdpInfo.apn.length > sizeof(cgdcont->priPdpInfo.apn.value)) {
        AT_WARN_LOG1("AT_PrintCgdcontParaApn: stCgdcont.stPriPdpInfo.stApn.ucLength: ",
                     cgdcont->priPdpInfo.apn.length);
        cgdcont->priPdpInfo.apn.length = sizeof(cgdcont->priPdpInfo.apn.value);
    }

    if (cgdcont->priPdpInfo.apn.length > 0) {
        memResult = memcpy_s(str, sizeof(str), cgdcont->priPdpInfo.apn.value, cgdcont->priPdpInfo.apn.length);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(str), cgdcont->priPdpInfo.apn.length);
    }
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", str);
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID AT_ConvertSNssaiToString(PS_S_NSSAI_STRU *sNssai, VOS_UINT16 *length)
{
    /*
     * 27007 rel15, 10.1.1章节
     * sst                                     only slice/service type (SST) is present
     * sst;mapped_sst                          SST and mapped configured SST are present
     * sst.sd                                  SST and slice differentiator (SD) are present
     * sst.sd;mapped_sst                       SST, SD and mapped configured SST are present
     * sst.sd;mapped_sst.mapped_sd             SST, SD, mapped configured SST and mapped configured SD are present
     */

    if ((sNssai->bitOpSd == VOS_TRUE) && (sNssai->bitOpMappedSst == VOS_TRUE) && (sNssai->bitOpMappedSd == VOS_TRUE)) {
        /* sst.sd;mapped_sst.mapped_sd             SST, SD, mapped configured SST and mapped configured SD are present */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%02x.%06x;%02x.%06x\"", sNssai->ucSst, sNssai->ulSd,
            sNssai->ucMappedSst, sNssai->ulMappedSd);
    } else if ((sNssai->bitOpSd == VOS_TRUE) && (sNssai->bitOpMappedSst == VOS_TRUE)) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%02x.%06x;%02x\"", sNssai->ucSst, sNssai->ulSd,
            sNssai->ucMappedSst);
    } else if (sNssai->bitOpSd == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%02x.%06x\"", sNssai->ucSst, sNssai->ulSd);
    } else if (sNssai->bitOpMappedSst == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%02x;%02x\"", sNssai->ucSst, sNssai->ucMappedSst);
    } else {
        if (sNssai->ucSst == 0) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
        } else {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%02x\"", sNssai->ucSst);
        }
    }
}

LOCAL VOS_VOID AT_PrintCgdcontParaNrPara(TAF_PRI_PdpQueryInfo *cgdcont, VOS_UINT16 *length)
{
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", ",,,,,");

    /* ssc mode */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.sscMode);
    /* SNssai */

    AT_ConvertSNssaiToString(&cgdcont->priPdpInfo.sNssai, length);

    /* Pref Access Type */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d",
                                     cgdcont->priPdpInfo.prefAccessType);
    /* Reflect Qos Ind */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.rQosInd);
    /* Ipv6 multi-homing */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.mh6Pdu);
    /* Always on ind */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d",
                                     cgdcont->priPdpInfo.alwaysOnInd);
}
#endif

/*
 * 功能描述: 格式化输出cgdcont的参数
 */
LOCAL VOS_VOID AT_PrintCgdcontPara(TAF_PRI_PdpQueryInfo *cgdcont, VOS_UINT16 *length, VOS_UINT8 indexNum)
{
    VOS_UINT8                       str[TAF_MAX_APN_LEN + 1];

    /* +CGDCONT:  */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    /* <cid> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", cgdcont->cid);

    /* <PDP_type> */
    AT_PrintCgdcontParaPdpType(cgdcont, length);

    /* <APN> */
    AT_PrintCgdcontParaApn(cgdcont, length);

    /* <PDP_addr> */
    (VOS_VOID)memset_s(str, sizeof(str), 0x00, sizeof(str));
    AT_Ipv4Addr2Str((VOS_CHAR *)str, sizeof(str), cgdcont->priPdpInfo.pdpAddr.ipv4Addr, TAF_IPV4_ADDR_LEN);
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", str);
    /* <d_comp> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.pdpDcomp);
    /* <h_comp> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.pdpHcomp);

    /* <IPv4AddrAlloc>  */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.ipv4AddrAlloc);
    /* <Emergency Indication> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.emergencyInd);
    /* <P-CSCF_discovery> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.pcscfDiscovery);
    /* <IM_CN_Signalling_Flag_Ind> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.imCnSignalFlg);
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->priPdpInfo.nasSigPrioInd);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /* the following five parameters omit: securePCO, Ipv4_mtu_discovery, Local_Addr_ind, Non-Ip_Mtu_discovery and
     * Reliable data service */
    AT_PrintCgdcontParaNrPara(cgdcont, length);
#endif
}

VOS_UINT32 AT_RcvTafPsEvtGetPrimPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                          memResult;
    VOS_UINT32                       result;
    VOS_UINT16                       length = 0;
    VOS_UINT32                       tmp    = 0;
    TAF_PRI_PdpQueryInfo             cgdcont;
    TAF_PS_GetPrimPdpContextInfoCnf *getPrimPdpCtxInfoCnf = VOS_NULL_PTR;

    memset_s(&cgdcont, sizeof(cgdcont), 0x00, sizeof(TAF_PRI_PdpQueryInfo));
    getPrimPdpCtxInfoCnf = (TAF_PS_GetPrimPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGDCONT_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < getPrimPdpCtxInfoCnf->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cgdcont, sizeof(cgdcont), &getPrimPdpCtxInfoCnf->pdpContextQueryInfo[tmp],
                             sizeof(TAF_PRI_PdpQueryInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgdcont), sizeof(TAF_PRI_PdpQueryInfo));

        AT_PrintCgdcontPara(&cgdcont, &length, indexNum);
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetSecPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetSecPdpContextInfoCnf *setPdpCtxInfoCnf = VOS_NULL_PTR;

    setPdpCtxInfoCnf = (TAF_PS_SetSecPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGDSCONT_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setPdpCtxInfoCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetSecPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                         memResult;
    VOS_UINT32                      result;
    VOS_UINT16                      length = 0;
    VOS_UINT32                      tmp    = 0;
    TAF_PDP_SecContext              secPdpInfo;
    TAF_PS_GetSecPdpContextInfoCnf *getSecPdpCtxInfoCnf = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&secPdpInfo, sizeof(secPdpInfo), 0x00, sizeof(TAF_PDP_SecContext));

    getSecPdpCtxInfoCnf = (TAF_PS_GetSecPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGDSCONT_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < getSecPdpCtxInfoCnf->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&secPdpInfo, sizeof(secPdpInfo), &getSecPdpCtxInfoCnf->pdpContextQueryInfo[tmp],
                             sizeof(TAF_PDP_SecContext));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(secPdpInfo), sizeof(TAF_PDP_SecContext));
        /* +CGDSCONT:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", secPdpInfo.cid);
        /* <p_cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", secPdpInfo.linkdCid);
        /* <d_comp> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", secPdpInfo.pdpDcomp);
        /* <h_comp> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", secPdpInfo.pdpHcomp);
        /* <IM_CN_Signalling_Flag_Ind> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", secPdpInfo.imCnSignalFlg);
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtCallModifyCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallModifyCnf *callModifyCnf = VOS_NULL_PTR;

    /* 初始化 */
    callModifyCnf = (TAF_PS_CallModifyCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGCMOD_SET) {
        return VOS_ERR;
    }

    /*
     * (1)协议栈异常错误, 未发起PDP修改, 直接上报ERROR
     * (2)协议栈正常, 发起PDP修改, 根据PDP修改事件返回结果
     */
    if (callModifyCnf->cause != TAF_PS_CAUSE_SUCCESS) {
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

LOCAL VOS_BOOL AT_IsMtPppActiveType(TAF_GPRS_ActiveTypeUint8 activeType)
{
    if ((activeType == TAF_IP_ACTIVE_TE_PPP_MT_PPP_TYPE) ||
        (activeType == TAF_IPV6_ACTIVE_TE_PPP_MT_PPP_TYPE) ||
        (activeType == TAF_IPV4V6_ACTIVE_TE_PPP_MT_PPP_TYPE)) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

LOCAL PPP_IpCapabilityUint32 AT_GetPppCapByActType(TAF_GPRS_ActiveTypeUint8 activeType)
{
    switch (activeType) {
        case TAF_IP_ACTIVE_TE_PPP_MT_PPP_TYPE:
            return PPP_CAPABILITY_IPV4;
        case TAF_IPV6_ACTIVE_TE_PPP_MT_PPP_TYPE:
            return PPP_CAPABILITY_IPV6;
        case TAF_IPV4V6_ACTIVE_TE_PPP_MT_PPP_TYPE:
            return PPP_CAPABILITY_IPV4V6;
        default:
            return PPP_CAPABILITY_BUTT;
    }
}

/*
 * 功能描述: 封装PPP请求参数
 * 修改历史:
 *  1.日    期: 2020-08-26
 *    修改内容: 新生成函数
 */
PPPA_CreatPppReq* AT_PackPppReqPara(PPP_SendDataToModem sender, VOS_ULONG userData, TAF_GPRS_ActiveTypeUint8 activeType,
    PPPA_CreatPppReq *creatPppReq)
{
    (VOS_VOID)memset_s(creatPppReq, sizeof(PPPA_CreatPppReq), 0x00, sizeof(PPPA_CreatPppReq));

    creatPppReq->capability = AT_GetPppCapByActType(activeType);
    creatPppReq->sender = DMS_PORT_SendPppPacket;
    creatPppReq->userData = userData;

    return creatPppReq;
}

TAF_UINT32 At_SetDialGprsPara(TAF_UINT8 indexNum, TAF_UINT8 cid, TAF_GPRS_ActiveTypeUint8 activeType)
{
    TAF_PPP_ReqConfigInfo pppReqConfigInfo;
    PPPA_CreatPppReq      creatPppReq;
    VOS_ULONG             userData;
    VOS_UINT16            pppId = 0;
    DMS_PortIdUint16      portId;
    VOS_UINT8             eventInfo[AT_EVENT_INFO_ARRAY_MAX_LEN];

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    VOS_INT32             dlc;
    dlc = DMS_PORT_GetCmuxActivePppPstaskDlc();
#endif

    (VOS_VOID)memset_s(&pppReqConfigInfo, sizeof(pppReqConfigInfo), 0x00, sizeof(TAF_PPP_ReqConfigInfo));
    portId   = g_atClientTab[indexNum].portNo;
    userData = (VOS_ULONG)portId;

    /* 进行激活操作 */
    if (AT_IsMtPppActiveType(activeType) == VOS_TRUE) { /* MT中启动PPP */
        g_atClientTab[indexNum].cid = cid;                /* 保存当前指定的cid */
        AT_PackPppReqPara(DMS_PORT_SendPppPacket, userData, activeType, &creatPppReq);

        /* TE中启动了PPP，MT中启动PPP */
        if (PPPA_CreatePppReq(&pppId, &creatPppReq) == AT_SUCCESS) {
            /* EVENT-At_SetDialGprsPara: usPppId/index */
            AT_PACK_CREAT_PPP_EVENT(eventInfo, pppId, indexNum);
            AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DCE_CREATE_PPP, eventInfo, sizeof(eventInfo));

            g_atPppIndexTab[pppId]        = indexNum; /* 记录PPP id和Index的对应关系 */
            g_atClientTab[indexNum].pppId = pppId;    /* 保存PPP id */

#if (FEATURE_IOT_CMUX == FEATURE_ON)
            if ((AT_CheckCmuxUser(indexNum) == VOS_TRUE) && (g_atClientTab[indexNum].dataId != indexNum)) {
                /* 保存PPP id */
                g_atClientTab[g_atClientTab[indexNum].dataId].pppId = pppId;
                /* 切换为PPP数传状态 */
                DMS_PORT_SwitchToPppDataMode(g_atClientTab[g_atClientTab[indexNum].dataId].portNo, DMS_PORT_DATA_PPP, PPP_PullPacketEvent, pppId);
                /*保存下发ATD的接口*/
                g_atClientTab[g_atClientTab[indexNum].dataId].ctrlId = indexNum;
            } else {
#endif
            DMS_PORT_SwitchToPppDataMode(portId, DMS_PORT_DATA_PPP, PPP_PullPacketEvent, pppId);
#if (FEATURE_IOT_CMUX == FEATURE_ON)
            }
            /*设置拨号成功的标志位*/
            dlc++;
            DMS_PORT_SetCmuxActivePppPstaskDlc(dlc);
#endif
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_PPP_CALL_SET;

            return AT_CONNECT;
        } else {
            return AT_ERROR;
        }
    } else {
        pppReqConfigInfo.auth.authType = TAF_PDP_AUTH_TYPE_NONE;
        pppReqConfigInfo.ipcp.ipcpLen  = 0;

        /* 创建透传的PPP实体 */
        if (PPPA_CreateRawDataPppReq(&pppId, userData, DMS_PORT_SendPppPacket) != AT_SUCCESS) {
            return AT_ERROR;
        }

        if (TAF_PS_PPP_DIAL_UP(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0, cid,
                               &pppReqConfigInfo) != VOS_OK) {
            AT_SendRelPppReq(pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
            return AT_ERROR;
        }

        g_atPppIndexTab[pppId]        = indexNum; /* 记录PPP id和Index的对应关系 */
        g_atClientTab[indexNum].pppId = pppId;    /* 保存PPP id */

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_IP_CALL_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }
}

VOS_UINT32 AT_RcvTafPsCallEvtCallAnswerCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallAnswerCnf *callAnswerCnf = VOS_NULL_PTR;
    VOS_UINT32            result;

    /* 初始化 */
    callAnswerCnf = (TAF_PS_CallAnswerCnf *)evtInfo;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafPsCallEvtCallAnswerCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 检查当前命令的操作类型 */
    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGANS_ANS_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGANS_ANS_EXT_SET)) {
        return VOS_ERR;
    }

    /*
     * (1)协议栈异常错误, 未发起PDP应答, 直接上报ERROR
     * (2)协议栈正常, 发起PDP应答, 根据PDP激活事件返回结果
     */

    /* IP类型的应答，需要先给上层回CONNECT */
    if (callAnswerCnf->cause == TAF_ERR_AT_CONNECT) {
        result = At_SetDialGprsPara(indexNum, callAnswerCnf->cid, TAF_IP_ACTIVE_TE_PPP_MT_PPP_TYPE);

        /* 如果是connect，CmdCurrentOpt不清，At_RcvTeConfigInfoReq中使用 */
        if (result == AT_ERROR) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
        }

        AT_StopRelTimer(indexNum, &g_atClientTab[indexNum].hTimer);
        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;
        g_atClientTab[indexNum].opId          = 0;
        At_FormatResultData(indexNum, result);

        return VOS_OK;
    }

    /* 其他错误，命令返回ERROR */
    if (callAnswerCnf->cause != TAF_ERR_NO_ERROR) {
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtCallHangupCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT32            result;
    TAF_PS_CallHangupCnf *callHangUpCnf = VOS_NULL_PTR;

    callHangUpCnf = (TAF_PS_CallHangupCnf *)evtInfo;

    if (callHangUpCnf->cause == TAF_ERR_NO_ERROR) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    /* 根据操作类型 */
    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_CGANS_ANS_SET:
            AT_StopTimerCmdReady(indexNum);
            At_FormatResultData(indexNum, result);
            break;

        default:
            break;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetPdpDnsInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetPdpDnsInfoCnf *setPdpDnsInfoCnf = VOS_NULL_PTR;

    setPdpDnsInfoCnf = (TAF_PS_SetPdpDnsInfoCnf *)evtInfo;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGDNS_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setPdpDnsInfoCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetPdpDnsInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t memResult;
    /* 移植At_QryParaRspCgdnsProc的实现逻辑 */
    VOS_UINT32               result;
    VOS_UINT16               length = 0;
    VOS_UINT32               tmp    = 0;
    TAF_DNS_QueryInfo        pdpDns;
    TAF_PS_GetPdpDnsInfoCnf *pdpDnsInfo = VOS_NULL_PTR;
    VOS_INT8                 acDnsAddr[TAF_MAX_IPV4_ADDR_STR_LEN];

    (VOS_VOID)memset_s(&pdpDns, sizeof(pdpDns), 0x00, sizeof(TAF_DNS_QueryInfo));
    (VOS_VOID)memset_s(acDnsAddr, sizeof(acDnsAddr), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    pdpDnsInfo = (TAF_PS_GetPdpDnsInfoCnf *)evtInfo;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGDNS_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < pdpDnsInfo->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&pdpDns, sizeof(pdpDns), &pdpDnsInfo->pdpDnsQueryInfo[tmp], sizeof(TAF_DNS_QueryInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pdpDns), sizeof(TAF_DNS_QueryInfo));
        /* +CGDNS:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", pdpDns.cid);
        /* <PriDns> */
        if (pdpDns.dnsInfo.bitOpPrimDnsAddr == 1) {
            AT_Ipv4Addr2Str((VOS_CHAR *)acDnsAddr, sizeof(acDnsAddr), pdpDns.dnsInfo.primDnsAddr, TAF_IPV4_ADDR_LEN);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acDnsAddr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <SecDns> */
        if (pdpDns.dnsInfo.bitOpSecDnsAddr == 1) {
            AT_Ipv4Addr2Str((VOS_CHAR *)acDnsAddr, sizeof(acDnsAddr), pdpDns.dnsInfo.secDnsAddr, TAF_IPV4_ADDR_LEN);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acDnsAddr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamicDnsInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT32                   result     = AT_ERROR;
    VOS_UINT16                   length     = 0;
    TAF_PS_GetNegotiationDnsCnf *negoDnsCnf = VOS_NULL_PTR;
    VOS_INT8                     acDnsAddr[TAF_MAX_IPV4_ADDR_STR_LEN];

    (VOS_VOID)memset_s(acDnsAddr, sizeof(acDnsAddr), 0x00, sizeof(acDnsAddr));

    negoDnsCnf = (TAF_PS_GetNegotiationDnsCnf *)evtInfo;

    /* 检查用户索引值 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafPsEvtGetDynamicDnsInfoCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DNSQUERY_SET) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (negoDnsCnf->cause != TAF_PARA_OK) {
        result = AT_ERROR;
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <PriDns> */
        if (negoDnsCnf->negotiationDns.dnsInfo.bitOpPrimDnsAddr == VOS_TRUE) {
            AT_Ipv4Addr2Str((VOS_CHAR *)acDnsAddr, sizeof(acDnsAddr), negoDnsCnf->negotiationDns.dnsInfo.primDnsAddr,
                            TAF_IPV4_ADDR_LEN);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acDnsAddr);
        }
        /* <SecDns> */
        if (negoDnsCnf->negotiationDns.dnsInfo.bitOpSecDnsAddr == VOS_TRUE) {
            AT_Ipv4Addr2Str((VOS_CHAR *)acDnsAddr, sizeof(acDnsAddr), negoDnsCnf->negotiationDns.dnsInfo.secDnsAddr,
                            TAF_IPV4_ADDR_LEN);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", acDnsAddr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_ProcRabmSetFastDormParaCnf(RABM_AT_SetFastdormParaCnf *msg)
{
    VOS_UINT8  indexNum = 0;
    VOS_UINT32 rslt;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_ProcRabmSetFastDormParaCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置结果 */
    g_atSendDataBuff.bufLen = 0;
    rslt                    = AT_ERROR;
    if (msg->rslt == AT_RABM_PARA_SET_RSLT_SUCC) {
        rslt = AT_OK;
    }

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_VOID AT_ConvertPdpContextIpAddrParaToString(TAF_PDP_DynamicPrimExt *cgdcont, VOS_UINT16 *length)
{
    VOS_CHAR acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    (VOS_VOID)memset_s(acIpv4StrTmp, sizeof(acIpv4StrTmp), 0x00, sizeof(acIpv4StrTmp));
    (VOS_VOID)memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, sizeof(acIpv6StrTmp));

    if ((cgdcont->opIpAddr == VOS_TRUE) && (cgdcont->opSubMask == VOS_TRUE)) {
        if (cgdcont->pdpAddr.pdpType == TAF_PDP_IPV4) {
            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), cgdcont->pdpAddr.ipv4Addr);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s", acIpv4StrTmp);

            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), cgdcont->subnetMask.ipv4Addr);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ".%s\"", acIpv4StrTmp);
        } else if (cgdcont->pdpAddr.pdpType == TAF_PDP_IPV6) {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp, cgdcont->pdpAddr.ipv6Addr, TAF_IPV6_ADDR_LEN,
                                                   cgdcont->subnetMask.ipv6Addr, TAF_IPV6_ADDR_LEN);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", acIpv6StrTmp);
        } else {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
        }
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

VOS_VOID AT_ConvertPdpContextAddrParaToString(VOS_UINT32 opAddr, TAF_PDP_TypeUint8 pdpType, VOS_UINT8 *number,
                                              VOS_UINT8 aucIpv6Addr[], VOS_UINT16 *stringLength)
{
    VOS_CHAR acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    (VOS_VOID)memset_s(acIpv4StrTmp, sizeof(acIpv4StrTmp), 0x00, TAF_MAX_IPV4_ADDR_STR_LEN);
    (VOS_VOID)memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, AT_IPV6_ADDR_MASK_FORMAT_STR_LEN);

    if (opAddr == VOS_TRUE) {
        if ((pdpType == TAF_PDP_IPV4) || (pdpType == TAF_PDP_PPP)) {
            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), number);
            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",\"%s\"", acIpv4StrTmp);
        } else if (pdpType == TAF_PDP_IPV6) {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp, aucIpv6Addr, TAF_IPV6_ADDR_LEN, VOS_NULL_PTR, 0);
            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",\"%s\"", acIpv6StrTmp);
        } else {
            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",");
        }
    } else {
        *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",");
    }
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
LOCAL VOS_VOID AT_PrintCgdContrdpParaNrPara(TAF_PDP_DynamicPrimExt *cgdcont, VOS_UINT16 *length)
{
    /* <LIPA indication omit> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",");

    /* <ipv4 mtu> */
    AT_ConvertULParaToString(cgdcont->opIpv4Mtu, cgdcont->ipv4Mtu, length);

    /* the following six parameters omit: <wlan offload>, <local addr ind>, <Non-Ip mtu>, <Serving plmn rate
     * control value>, <Reliable data service> and <Ps data off support> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",,,,,,");

    /* <pdu session id> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->pduSessionId);

    /* <qfi> */
    AT_ConvertULParaToString(cgdcont->opQfi, cgdcont->qfi, length);

    /* <ssc mode> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->sscMode);

    AT_ConvertSNssaiToString(&cgdcont->sNssai, length);

    /* <access type> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->accessType);

    /* <reflect qos timer value> */
    AT_ConvertULParaToString(cgdcont->opRqTimer, cgdcont->rqTimer, length);

    /* <AlwaysOnInd> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                     (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->alwaysOnInd);
}
#endif

LOCAL VOS_VOID AT_PrintCgdcontrdpPara(TAF_PDP_DynamicPrimExt *cgdcont, VOS_UINT16 *length, VOS_UINT8 indexNum)
{
    VOS_UINT8   str[TAF_MAX_APN_LEN + 1];
    errno_t     memResult;

    /* +CGCONTRDP:  */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* <p_cid> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", cgdcont->primayCid);

    /* <bearer_id> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", cgdcont->bearerId);

    /* <APN> */
    if (cgdcont->opApn == 1) {
        (VOS_VOID)memset_s(str, sizeof(str), 0x00, sizeof(str));
        memResult = memcpy_s(str, sizeof(str), cgdcont->apn, TAF_MAX_APN_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(str), TAF_MAX_APN_LEN);
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", cgdcont->apn);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }

    /* <ip_addr> */
    AT_ConvertPdpContextIpAddrParaToString(cgdcont, length);

    /* <gw_addr> */
    AT_ConvertPdpContextAddrParaToString(cgdcont->opGwAddr, cgdcont->gwAddr.pdpType, cgdcont->gwAddr.ipv4Addr,
                                         cgdcont->gwAddr.ipv6Addr, length);

    /* <DNS_prim_addr> */
    AT_ConvertPdpContextAddrParaToString(cgdcont->opDnsPrimAddr, cgdcont->dnsPrimAddr.pdpType,
                                         cgdcont->dnsPrimAddr.ipv4Addr, cgdcont->dnsPrimAddr.ipv6Addr, length);

    /* <DNS_sec_addr> */
    AT_ConvertPdpContextAddrParaToString(cgdcont->opDnsSecAddr, cgdcont->dnsSecAddr.pdpType,
                                         cgdcont->dnsSecAddr.ipv4Addr, cgdcont->dnsSecAddr.ipv6Addr, length);

    /* <P-CSCF_prim_addr> */
    AT_ConvertPdpContextAddrParaToString(cgdcont->opPcscfPrimAddr, cgdcont->pcscfPrimAddr.pdpType,
                                         cgdcont->pcscfPrimAddr.ipv4Addr, cgdcont->pcscfPrimAddr.ipv6Addr,
                                         length);

    /* <P-CSCF_sec_addr> */
    AT_ConvertPdpContextAddrParaToString(cgdcont->opPcscfSecAddr, cgdcont->pcscfSecAddr.pdpType,
                                         cgdcont->pcscfSecAddr.ipv4Addr, cgdcont->pcscfSecAddr.ipv6Addr, length);

    /* <im-cn-signal-flag> */
    AT_ConvertULParaToString(cgdcont->opImCnSignalFlg, cgdcont->imCnSignalFlg, length);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    AT_PrintCgdContrdpParaNrPara(cgdcont, length);
#endif
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamicPrimPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t    memResult;
    VOS_UINT32 result = AT_FAILURE;
    VOS_UINT32 tmp;
    VOS_UINT16 length = 0;

    TAF_PDP_DynamicPrimExt                  cgdcont = {0};
    TAF_PS_GetDynamicPrimPdpContextInfoCnf *getDynamicPdpCtxInfoCnf = VOS_NULL_PTR;

    getDynamicPdpCtxInfoCnf = (TAF_PS_GetDynamicPrimPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGCONTRDP_SET) {
        return VOS_ERR;
    }

    if (getDynamicPdpCtxInfoCnf->cause == VOS_OK) {
        for (tmp = 0; tmp < getDynamicPdpCtxInfoCnf->cidNum; tmp++) {
            if (tmp != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            memResult = memcpy_s(&cgdcont, sizeof(cgdcont), &getDynamicPdpCtxInfoCnf->pdpContxtInfo[tmp],
                                 sizeof(TAF_PDP_DynamicPrimExt));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgdcont), sizeof(TAF_PDP_DynamicPrimExt));

            AT_PrintCgdcontrdpPara(&cgdcont, &length, indexNum);
        }
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = length;

    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamicSecPdpContextInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t    memResult;
    VOS_UINT32 result = AT_FAILURE;
    VOS_UINT32 tmp    = 0;
    VOS_UINT16 length = 0;

    TAF_PDP_DynamicSecExt                  cgdscont;
    TAF_PS_GetDynamicSecPdpContextInfoCnf *getDynamicPdpCtxInfoCnf = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&cgdscont, sizeof(cgdscont), 0x00, sizeof(TAF_PDP_DynamicSecExt));
    getDynamicPdpCtxInfoCnf = (TAF_PS_GetDynamicSecPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGSCONTRDP_SET) {
        return VOS_ERR;
    }

    if (getDynamicPdpCtxInfoCnf->cause == VOS_OK) {
        for (tmp = 0; tmp < getDynamicPdpCtxInfoCnf->cidNum; tmp++) {
            if (tmp != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            memResult = memcpy_s(&cgdscont, sizeof(cgdscont), &getDynamicPdpCtxInfoCnf->pdpContxtInfo[tmp],
                                 sizeof(TAF_PDP_DynamicSecExt));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgdscont), sizeof(TAF_PDP_DynamicSecExt));

            /* +CGSCONTRDP:  */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
            /* <cid> */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d", cgdscont.cid);
            /* <p_cid> */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgdscont.primaryCid);
            /* <bearer_id> */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgdscont.bearerId);
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
            /* the following two parameters omit: im_cn_signalling_flag and wlan offload */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",,");
            /* pdu_session_id */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgdscont.pduSessionId);
            /* qfi */
            if (cgdscont.opQfi == VOS_TRUE) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgdscont.qfi);
            } else {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",");
            }
#endif
        }

        result                  = AT_OK;
        g_atSendDataBuff.bufLen = length;
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

LOCAL VOS_VOID AT_PrintCgtftRdpCrlf(VOS_UINT32 index1, VOS_UINT32 index2, VOS_UINT16 *length)
{
    if (!(index1 == 0 && index2 == 0)) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
    }
}

LOCAL VOS_VOID AT_PrintCgtftRdpInfoParaLocalAddressAndSubnet(TAF_PF_Tft *cgtft, VOS_UINT32 index2, VOS_UINT16 *length)
{
    VOS_CHAR   acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN] = {0};
    VOS_CHAR   localIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN] = {0};
    VOS_UINT8  localIpv6Mask[TAF_IPV6_ADDR_LEN] = {0};

    if (AT_IsSupportReleaseRst(AT_ACCESS_STRATUM_REL11)) {
        /* <local address and subnet> */
        if (cgtft->tftInfo[index2].opLocalIpv4AddrAndMask == 1) {
            AT_Ipv4AddrItoa(localIpv4StrTmp, sizeof(localIpv4StrTmp), cgtft->tftInfo[index2].localIpv4Addr);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s", localIpv4StrTmp);

            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ".");

            AT_Ipv4AddrItoa(localIpv4StrTmp, sizeof(localIpv4StrTmp), cgtft->tftInfo[index2].localIpv4Mask);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s\"", localIpv4StrTmp);
        } else if (cgtft->tftInfo[index2].opLocalIpv6AddrAndMask == 1) {
            AT_GetIpv6MaskByPrefixLength(cgtft->tftInfo[index2].localIpv6Prefix, localIpv6Mask);
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp, cgtft->tftInfo[index2].localIpv6Addr,
                                                 TAF_IPV6_ADDR_LEN, localIpv6Mask, TAF_IPV6_ADDR_LEN);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"", acIpv6StrTmp);
        } else {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
        }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        /* <qri> */
        AT_ConvertULParaToString(cgtft->tftInfo[index2].opQri, cgtft->tftInfo[index2].qri, length);
#endif
    }
}

VOS_VOID AT_ConvertTftSrcAddrParaToString(TAF_TFT_Ext *tftInfo, VOS_UINT16 *stringLength)
{
    VOS_CHAR acIpv4StrTmp[TAF_MAX_IPV4_ADDR_STR_LEN];
    VOS_CHAR acIpv6StrTmp[AT_IPV6_ADDR_MASK_FORMAT_STR_LEN];

    (VOS_VOID)memset_s(acIpv4StrTmp, sizeof(acIpv4StrTmp), 0x00, sizeof(acIpv4StrTmp));
    (VOS_VOID)memset_s(acIpv6StrTmp, sizeof(acIpv6StrTmp), 0x00, sizeof(acIpv6StrTmp));

    if (tftInfo->opSrcIp == VOS_TRUE) {
        if ((tftInfo->sourceIpaddr.pdpType == TAF_PDP_IPV4) || (tftInfo->sourceIpaddr.pdpType == TAF_PDP_PPP)) {
            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), tftInfo->sourceIpaddr.ipv4Addr);
            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",\"%s", acIpv4StrTmp);

            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ".");

            AT_Ipv4AddrItoa(acIpv4StrTmp, sizeof(acIpv4StrTmp), tftInfo->sourceIpMask.ipv4Addr);
            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, "%s\"", acIpv4StrTmp);
        } else if (tftInfo->sourceIpaddr.pdpType == TAF_PDP_IPV6) {
            (VOS_VOID)AT_Ipv6AddrMask2FormatString(acIpv6StrTmp, tftInfo->sourceIpaddr.ipv6Addr, TAF_IPV6_ADDR_LEN,
                                                   tftInfo->sourceIpMask.ipv6Addr, TAF_IPV6_ADDR_LEN);
            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",\"%s\"", acIpv6StrTmp);
        } else {
            *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",");
        }
    } else {
        *stringLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *stringLength, ",");
    }
}

VOS_VOID AT_ConvertULParaToXString(VOS_UINT32 opPara, VOS_UINT32 paraVal, VOS_UINT16 *length)
{
    if (opPara == VOS_TRUE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",%X", paraVal);
    } else {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

LOCAL VOS_VOID AT_PrintCgtftRdpInfoPara(TAF_PF_Tft *cgtft, VOS_UINT32 index2, VOS_UINT16 *length)
{
    /* <cid> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%d", cgtft->cid);
    /* <packet filter identifier> */
    AT_ConvertULParaToString(cgtft->tftInfo[index2].opPktFilterId, cgtft->tftInfo[index2].packetFilterId,
                             length);

    /* <evaluation precedence index> */
    AT_ConvertULParaToString(cgtft->tftInfo[index2].opPrecedence, cgtft->tftInfo[index2].precedence,
                             length);

    /* <source address and subnet> */
    AT_ConvertTftSrcAddrParaToString(&cgtft->tftInfo[index2], length);

    /* <protocal number(ipv4)/next header ipv6> */
    AT_ConvertULParaToString(cgtft->tftInfo[index2].opProtocolId, cgtft->tftInfo[index2].protocolId,
                             length);

    /* <destination port range> */
    AT_ConvertRangeParaToString(VOS_FALSE, cgtft->tftInfo[index2].opDestPortRange,
                                cgtft->tftInfo[index2].lowDestPort, cgtft->tftInfo[index2].highDestPort,
                                length);

    /* <source port range> */
    AT_ConvertRangeParaToString(VOS_FALSE, cgtft->tftInfo[index2].opSrcPortRange,
                                cgtft->tftInfo[index2].lowSourcePort, cgtft->tftInfo[index2].highSourcePort,
                                length);

    /* <ipsec security parameter index(spi)> */
    AT_ConvertULParaToXString(cgtft->tftInfo[index2].opSpi, cgtft->tftInfo[index2].secuParaIndex, length);

    /* <type os service(tos) (ipv4) and mask> */
    AT_ConvertRangeParaToString(VOS_FALSE, cgtft->tftInfo[index2].opTosMask,
                                cgtft->tftInfo[index2].typeOfService,
                                cgtft->tftInfo[index2].typeOfServiceMask, length);

    /* <traffic class (ipv6) and mask> */

    /* <flow lable (ipv6)> */
    AT_ConvertULParaToXString(cgtft->tftInfo[index2].opFlowLable, cgtft->tftInfo[index2].flowLable,
                              length);

    /* <direction> */
    AT_ConvertULParaToString(cgtft->tftInfo[index2].opDirection, cgtft->tftInfo[index2].direction, length);

    /* <NW packet filter Identifier> */
    AT_ConvertULParaToString(cgtft->tftInfo[index2].opNwPktFilterId, cgtft->tftInfo[index2].nwPktFilterId,
                             length);
    /* <local address and subnet> */
    AT_PrintCgtftRdpInfoParaLocalAddressAndSubnet(cgtft, index2, length);
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamicTftInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t    memResult;
    VOS_UINT32 result = AT_FAILURE;
    VOS_UINT32 index1 = 0;
    VOS_UINT32 index2 = 0;
    VOS_UINT16 length = 0;

    TAF_PF_Tft                  *cgtft                = VOS_NULL_PTR;
    TAF_PS_GetDynamicTftInfoCnf *getDynamicTftInfoCnf = VOS_NULL_PTR;

    getDynamicTftInfoCnf = (TAF_PS_GetDynamicTftInfoCnf *)evtInfo;

    cgtft = (TAF_PF_Tft *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_PF_Tft));
    if (cgtft == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    (VOS_VOID)memset_s(cgtft, sizeof(TAF_PF_Tft), 0x00, sizeof(TAF_PF_Tft));

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGTFTRDP_SET) {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, cgtft);
        /*lint -restore */
        return VOS_ERR;
    }

    if (getDynamicTftInfoCnf->cause == VOS_OK) {
        for (index1 = 0; index1 < getDynamicTftInfoCnf->cidNum; index1++) {
            for (index2 = 0; index2 < AT_MIN(getDynamicTftInfoCnf->pfTftInfo[index1].pfNum, TAF_MAX_SDF_PF_NUM); index2++) {

                AT_PrintCgtftRdpCrlf(index1, index2, &length);

                memResult = memcpy_s(cgtft, sizeof(TAF_PF_Tft), &getDynamicTftInfoCnf->pfTftInfo[index1],
                                     sizeof(TAF_PF_Tft));
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TAF_PF_Tft), sizeof(TAF_PF_Tft));

                /* +CGTFTRDP:  */
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

                AT_PrintCgtftRdpInfoPara(cgtft, index2, &length);
            }

            /* <3,0,0,"192.168.0.2.255.255.255.0">,0,"0.65535","0.65535",0,"0.0",0,0 */
        }

        result                  = AT_OK;
        g_atSendDataBuff.bufLen = length;
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, cgtft);
    /*lint -restore */
    return VOS_OK;
}

VOS_VOID AT_ProcPppDialCnf(TAF_PS_CauseUint32 cause, VOS_UINT8 indexNum)
{
    VOS_UINT32 result = AT_FAILURE;

    /* MODEM拨号错误处理 */
    if (AT_IsPppUser(indexNum) == VOS_FALSE) {
        return;
    }

    if (cause == TAF_PS_CAUSE_SUCCESS) {
        return;
    }

    /* 记录PS域呼叫错误码 */
    AT_PS_SetPsCallErrCause(indexNum, cause);

    if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_D_PPP_CALL_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_PPP_ORG_SET)) {
        result = AT_NO_CARRIER;

        AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_REQ);

        /* 返回命令模式 */
        DMS_PORT_ResumeCmdMode(g_atClientTab[indexNum].portNo);
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_D_IP_CALL_SET) {
        if (cause == TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT) {
            result = AT_CME_PDP_ACT_LIMIT;
        } else {
            result = AT_ERROR;
        }

        AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
    } else {

    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
}

VOS_UINT32 AT_RcvTafIfaceEvtIfaceUpCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_UpCnf    *ifaceUpCnf  = VOS_NULL_PTR;
    AT_PS_DataChanlCfg *dataChanCfg = VOS_NULL_PTR;
    VOS_UINT32          result = AT_OK;

    ifaceUpCnf = (TAF_IFACE_UpCnf *)evtInfo;

#if ((FEATURE_SHARE_APN == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
    if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_D_PPP_CALL_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_PPP_ORG_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_D_IP_CALL_SET)) {
        AT_ProcPppDialCnf(ifaceUpCnf->cause, indexNum);
        return VOS_OK;
    }
#endif

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_NDISDUP_SET) {
        AT_WARN_LOG("AT_RcvTafIfaceEvtIfaceUpCnf : Current Option is not AT_CMD_NDISDUP_SET.");
        return VOS_ERR;
    }

    if (ifaceUpCnf->cause != TAF_PS_CAUSE_SUCCESS) {
        AT_PS_SetPsCallErrCause(indexNum, ifaceUpCnf->cause);

        result = AT_ERROR;

        if (ifaceUpCnf->cause == TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT) {
            result = AT_CME_PDP_ACT_LIMIT;
        }

        if (ifaceUpCnf->cid <= TAF_MAX_CID) {
            dataChanCfg            = AT_PS_GetDataChanlCfg(indexNum, ifaceUpCnf->cid);
            dataChanCfg->portIndex = AT_CLIENT_ID_BUTT;
        }

        AT_WARN_LOG1("AT_RcvTafIfaceEvtIfaceUpCnf: <enCause> is ", ifaceUpCnf->cause);
    }

    /* 清除命令处理状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#if (FEATURE_IOT_CMUX == FEATURE_ON)
VOS_VOID AT_CmuxProcCallEndCnfEvent(VOS_UINT8 indexNum, VOS_UINT32 result)
{
    /*设置拨号成功的标志位*/
    VOS_INT32 dlc = DMS_PORT_GetCmuxActivePppPstaskDlc();
    if (dlc > 0) {
        dlc--;
        DMS_PORT_SetCmuxActivePppPstaskDlc(dlc);
    }
    if (!((AT_CheckCmuxUser(indexNum) == VOS_TRUE) && (g_atClientTab[indexNum].ctrlId != indexNum))) {
        At_FormatResultData(indexNum, result);
    } else {
        At_FormatResultData(indexNum, AT_NO_CARRIER);
    }
}
#endif

VOS_VOID AT_ModemProcCallEndCnfEvent(TAF_PS_CauseUint32 cause, VOS_UINT8 indexNum)
{
    VOS_UINT32              modemUsrFlg;
    VOS_UINT32              aTHCmdFlg;
    VOS_UINT32              result;
    VOS_UINT16              length;
    DMS_PortIdUint16        portId;
    DMS_PortDataModeUint8   dataMode;

    modemUsrFlg = AT_CheckModemUser(indexNum);
    result      = AT_FAILURE;
    length      = 0;
    aTHCmdFlg   = (AT_PS_GET_CURR_CMD_OPT(indexNum) == AT_CMD_H_PS_SET) ? VOS_TRUE : VOS_FALSE;
    portId      = AT_GetDmsPortIdByClientId(indexNum);
    dataMode    = DMS_PORT_GetDataMode(portId);

    /* 检查当前用户的操作类型 */
    if ((AT_PS_GET_CURR_CMD_OPT(indexNum) != AT_CMD_PS_DATA_CALL_END_SET) &&
        (AT_PS_GET_CURR_CMD_OPT(indexNum) != AT_CMD_H_PS_SET)) {
        return;
    }

    /* PS域呼叫正在处理断开请求, 直接返回 */
    if (cause == TAF_ERR_NO_ERROR) {
        return;
    }

    /* 释放PPP实体 & HDLC去使能 */
    if (dataMode == DMS_PORT_DATA_PPP) {
        AT_SendRelPppReq(AT_PS_GET_PPPID(indexNum), PPP_AT_CTRL_REL_PPP_REQ);
    } else if (dataMode == DMS_PORT_DATA_PPP_RAW) {
        AT_SendRelPppReq(AT_PS_GET_PPPID(indexNum), PPP_AT_CTRL_REL_PPP_RAW_REQ);
    } else {

    }

    /* 返回命令模式 */
    DMS_PORT_ResumeCmdMode(portId);

    /*
     * 为了规避Linux后台二次拨号失败问题，采用如下规避方案:
     * PDP正常断开流程结束后，接收到TAF_PS_EVT_PDP_DEACTIVE_CNF事件，判断操作
     * 系统是否为Linux，若否，则按照原先流程处理，若是，则不再发送"NO CARRIER"
     */
    if ((DRV_GET_LINUXSYSTYPE() == VOS_OK) && (modemUsrFlg == VOS_TRUE)) {
        /*
         * 原先流程中，在At_FormatResultData函数内部，发送了"NO CARRIER"之后，
         * 需要将DCD拉低，在此规避方案中，不发送"NO CARRIER"，但DCD信号的拉低
         * 操作仍旧需要保持
         */
        AT_StopTimerCmdReady(indexNum);
        DMS_PORT_DeassertDcd(portId);
        return;
    }

    if (aTHCmdFlg == VOS_TRUE) {
        result = AT_OK;
    } else {
        result = AT_NO_CARRIER;
    }

    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen = length;

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    AT_CmuxProcCallEndCnfEvent(indexNum, result);
#else
    At_FormatResultData(indexNum, result);
#endif

    /* ATH断开PPP连接时, 如果DCD信号模式为CONNECT ON, 需要拉低DCD信号 */
    if ((result == AT_OK) && (DMS_PORT_IsDcdModeConnectOn() == VOS_TRUE)) {
        DMS_PORT_DeassertDcd(portId);
    }
}

VOS_UINT32 AT_RcvTafIfaceEvtIfaceDownCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_IFACE_DownCnf *ifaceDownCnf = VOS_NULL_PTR;
    VOS_UINT32         result = AT_OK;

    ifaceDownCnf = (TAF_IFACE_DownCnf *)evtInfo;

#if ((FEATURE_SHARE_APN == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
    if ((AT_PS_GET_CURR_CMD_OPT(indexNum) == AT_CMD_PS_DATA_CALL_END_SET) ||
        (AT_PS_GET_CURR_CMD_OPT(indexNum) == AT_CMD_H_PS_SET)) {
        /* 共享场景使用原有callEnd处理 */
        AT_ModemProcCallEndCnfEvent(ifaceDownCnf->cause, indexNum);
        return VOS_OK;
    }
#endif

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_NDISDUP_SET) {
        AT_WARN_LOG("AT_RcvTafIfaceEvtIfaceDownCnf : Current Option is not AT_CMD_NDISDUP_SET.");
        return VOS_ERR;
    }

    if (ifaceDownCnf->cause != TAF_PS_CAUSE_SUCCESS) {
        AT_WARN_LOG1("AT_RcvTafIfaceEvtIfaceDownCnf: <enCause> is ", ifaceDownCnf->cause);
        result = AT_ERROR;
    }

    /* 清除命令处理状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtClearDsFlowInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_ClearDsflowCnf *evtMsg = VOS_NULL_PTR;

    evtMsg = (TAF_PS_ClearDsflowCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DSFLOWCLR_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, evtMsg->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetDsFlowInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetDsflowInfoCnf *evtMsg    = VOS_NULL_PTR;
    TAF_DSFLOW_QueryInfo    *queryInfo = VOS_NULL_PTR;
    VOS_UINT16               length    = 0;

    evtMsg    = (TAF_PS_GetDsflowInfoCnf *)evtInfo;
    queryInfo = &evtMsg->queryInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DSFLOWQRY_SET) {
        return VOS_ERR;
    }

    /* 上报流量查询结果 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%08X,%08X%08X,%08X%08X", queryInfo->currentFlowInfo.linkTime,
        queryInfo->currentFlowInfo.sendFluxHigh, queryInfo->currentFlowInfo.sendFluxLow,
        queryInfo->currentFlowInfo.recvFluxHigh, queryInfo->currentFlowInfo.recvFluxLow);

    length +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%08X,%08X%08X,%08X%08X", queryInfo->totalFlowInfo.linkTime,
            queryInfo->totalFlowInfo.sendFluxHigh, queryInfo->totalFlowInfo.sendFluxLow,
            queryInfo->totalFlowInfo.recvFluxHigh, queryInfo->totalFlowInfo.recvFluxLow);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtConfigDsFlowRptCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_ConfigDsflowRptCnf *evtMsg = VOS_NULL_PTR;

    evtMsg = (TAF_PS_ConfigDsflowRptCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DSFLOWRPT_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, evtMsg->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetAuthDataInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetAuthDataInfoCnf *setAuthDataInfoCnf = VOS_NULL_PTR;

    /* 初始化 */
    setAuthDataInfoCnf = (TAF_PS_SetAuthDataInfoCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_AUTHDATA_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setAuthDataInfoCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetAuthDataInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                    memResult;
    VOS_UINT32                 result;
    VOS_UINT32                 tmp    = 0;
    VOS_UINT16                 length = 0;
    TAF_AUTHDATA_QueryInfo     pdpAuthData;
    TAF_PS_GetAuthdataInfoCnf *pdpAuthDataInfo = VOS_NULL_PTR;

    memset_s(&pdpAuthData, sizeof(pdpAuthData), 0x00, sizeof(TAF_AUTHDATA_QueryInfo));

    pdpAuthDataInfo = (TAF_PS_GetAuthdataInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_AUTHDATA_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < pdpAuthDataInfo->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&pdpAuthData, sizeof(pdpAuthData), &pdpAuthDataInfo->authDataQueryInfo[tmp],
                             sizeof(TAF_AUTHDATA_QueryInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pdpAuthData), sizeof(TAF_AUTHDATA_QueryInfo));
        /* ^AUTHDATA:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", pdpAuthData.cid);

        /* <Auth_type> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", pdpAuthData.authDataInfo.authType);

        /* <passwd> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", pdpAuthData.authDataInfo.password);

        /* <username> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", pdpAuthData.authDataInfo.username);

        /* <PLMN> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", pdpAuthData.authDataInfo.plmn);
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    memset_s(&pdpAuthData, sizeof(pdpAuthData), 0x00, sizeof(TAF_AUTHDATA_QueryInfo));

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetRoamPdpTypeCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetRoamingPdpTypeCnf *setPdpTypeCnf = VOS_NULL_PTR;

    setPdpTypeCnf = (TAF_PS_SetRoamingPdpTypeCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ROAMPDPTYPE_SET) {
        AT_WARN_LOG("AT_RcvTafPsEvtSetRoamPdpTypeCnf: WARNING: CmdCurrentOpt != AT_CMD_ROAMPDPTYPE_SET!");
        return VOS_ERR;
    }

    /* 处理错误码 上报结果 */
    AT_PrcoPsEvtErrCode(indexNum, setPdpTypeCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetSinglePdnSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetSinglePdnSwitchCnf *setSinglePdnSwitchCnf = VOS_NULL_PTR;

    setSinglePdnSwitchCnf = (TAF_PS_SetSinglePdnSwitchCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SINGLEPDNSWITCH_SET) {
        AT_WARN_LOG("AT_RcvTafPsEvtSetSinglePdnSwitchCnf: WARNING: CmdCurrentOpt != AT_CMD_SINGLEPDNSWITCH_SET!");
        return VOS_ERR;
    }

    /* 处理错误码 上报结果 */
    AT_PrcoPsEvtErrCode(indexNum, setSinglePdnSwitchCnf->cause);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetSinglePdnSwitchCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetSinglePdnSwitchCnf *getSinglePdnSwitchCnf = VOS_NULL_PTR;

    getSinglePdnSwitchCnf = (TAF_PS_GetSinglePdnSwitchCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SINGLEPDNSWITCH_QRY) {
        AT_WARN_LOG("AT_RcvTafPsEvtGetSinglePdnSwitchCnf: WARNING: CmdCurrentOpt != AT_CMD_SINGLEPDNSWITCH_QRY!");
        return VOS_ERR;
    }

    /* 上报查询结果 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            getSinglePdnSwitchCnf->enable);

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_ProcRabmReleaseRrcCnf(RABM_AT_ReleaseRrcCnf *msg)
{
    VOS_UINT8  indexNum = 0;
    VOS_UINT32 rslt = AT_ERROR;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ProcRabmReleaseRrcCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 输出设置结果 */
    g_atSendDataBuff.bufLen = 0;

    if (msg->rslt == VOS_OK) {
        rslt = AT_OK;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_VOID AT_RcvRnicRmnetCfgCnf(MsgBlock *msg)
{
    RNIC_AT_RmnetCfgCnf *rmnetCfgCnf;
    VOS_UINT32 ret;
    VOS_UINT8 indexNum = 0;

    rmnetCfgCnf = (RNIC_AT_RmnetCfgCnf *)msg;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(rmnetCfgCnf->clientId, AT_CMD_RMNETCFG_SET, &indexNum) != VOS_OK) {
        return;
    }

    ret = (rmnetCfgCnf->result == VOS_OK) ? AT_OK : AT_ERROR;

    /* 输出设置结果 */
    g_atSendDataBuff.bufLen = 0;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_UINT32 AT_RcvRnicDialModeCnf(MsgBlock *msg)
{
    VOS_UINT16           length = 0;
    VOS_UINT8            indexNum = 0;
    RNIC_AT_DialModeCnf *rcvMsg   = VOS_NULL_PTR;

    rcvMsg = (RNIC_AT_DialModeCnf *)msg;

    if (At_ClientIdToUserId(rcvMsg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvRnicDialModeCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvRnicDialModeCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", rcvMsg->dialMode);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rcvMsg->eventReportFlag);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rcvMsg->idleTime);

    g_atSendDataBuff.bufLen = length;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetApDsFlowRptCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetApdsflowRptCfgCnf *evtMsg = VOS_NULL_PTR;

    evtMsg = (TAF_PS_SetApdsflowRptCfgCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_APDSFLOWRPTCFG_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, evtMsg->cause);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetApDsFlowRptCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetApdsflowRptCfgCnf *evtMsg = VOS_NULL_PTR;
    VOS_UINT16                   length;

    evtMsg = (TAF_PS_GetApdsflowRptCfgCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_APDSFLOWRPTCFG_QRY) {
        return VOS_ERR;
    }

    /* 检查错误码 */
    if (evtMsg->cause != TAF_PS_CAUSE_SUCCESS) {
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    /* 上报查询结果 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%u", g_parseContext[indexNum].cmdElement->cmdName,
        evtMsg->rptCfg.rptEnabled, evtMsg->rptCfg.fluxThreshold);

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetDsFlowNvWriteCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetDsflowNvWriteCfgCnf *setNvWriteCfgCnf = VOS_NULL_PTR;

    setNvWriteCfgCnf = (TAF_PS_SetDsflowNvWriteCfgCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DSFLOWNVWRCFG_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setNvWriteCfgCnf->cause);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetDsFlowNvWriteCfgCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetDsflowNvWriteCfgCnf *getNvWriteCfgCnf = VOS_NULL_PTR;
    VOS_UINT16                     length;

    getNvWriteCfgCnf = (TAF_PS_GetDsflowNvWriteCfgCnf *)evtInfo;
    length           = 0;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DSFLOWNVWRCFG_QRY) {
        return VOS_ERR;
    }

    /* 检查错误码 */
    if (getNvWriteCfgCnf->cause != TAF_PS_CAUSE_SUCCESS) {
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    /* 上报查询结果 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        getNvWriteCfgCnf->nvWriteCfg.enabled, getNvWriteCfgCnf->nvWriteCfg.interval);

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

#if (FEATURE_HUAWEI_VP == FEATURE_ON)
VOS_UINT32 AT_ProcRabmSetVoicePreferParaCnf(RABM_AT_SetVoicepreferParaCnf *msg)
{
    VOS_UINT8  indexNum;
    VOS_UINT32 rslt;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ProcRabmSetVoicePreferParaCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置结果 */
    g_atSendDataBuff.bufLen = 0;
    rslt                      = AT_ERROR;
    if (msg->rslt == AT_RABM_PARA_SET_RSLT_SUCC) {
        rslt = AT_OK;
    }

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 AT_ProcRabmQryVoicePreferEnableParaCnf(RABM_AT_QryVoicepreferParaCnf *msg)
{
    VOS_UINT8  indexNum;
    VOS_UINT32 result = AT_ERROR;
    VOS_UINT16 length;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ProcRabmQryVoicePreferEnableParaCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s:%d%s", g_atCrLf, g_parseContext[indexNum].cmdElement->cmdName,
        msg->rslt, g_atCrLf);
    result                    = AT_OK;
    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, result);

    return VOS_OK;
}
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
* 功能说明: 增加打印Profile信息
* 输出参数: curLen: 目前打印字符串的长度
*           eventCnf: 事件信息
* 返回结果: 增加Profile信息打印后的字符串长度
*/
LOCAL VOS_UINT16 AT_PrintAttachProfileInfo(VOS_UINT16 curLen, TAF_PS_GetCustAttachProfileInfoCnf *eventCnf)
{
    errno_t                             memResult;
    VOS_UINT16                          length = curLen;
    VOS_UINT8                           tempBuffer[TAF_NVIM_MAX_APNRETRY_PRONAME_LEN * 2 + 1] = { 0 };

    /* 填充APN */
    if (eventCnf->custAttachApn.nvAttachProfileInfo.apnLen > 0) {
        memResult = memcpy_s(tempBuffer, sizeof(tempBuffer), eventCnf->custAttachApn.nvAttachProfileInfo.apn,
            TAF_NVIM_MAX_APNRETRY_APN_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tempBuffer), TAF_NVIM_MAX_APNRETRY_APN_LEN);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",", tempBuffer);
    }
    else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"\",");
    }

    /* 填充User Name */
    if (eventCnf->custAttachApn.nvAttachProfileInfo.userNameLen > 0) {
        (VOS_VOID)memset_s(tempBuffer, sizeof(tempBuffer), 0x00, sizeof(tempBuffer));
        memResult = memcpy_s(tempBuffer, sizeof(tempBuffer), eventCnf->custAttachApn.nvAttachProfileInfo.userName,
            TAF_NVIM_MAX_APNRETRY_USERNAME_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tempBuffer), TAF_NVIM_MAX_APNRETRY_USERNAME_LEN);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",", tempBuffer);
    }
    else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"\",");
    }

    if (eventCnf->custAttachApn.nvAttachProfileInfo.pwdLen > 0) {
        (VOS_VOID)memset_s(tempBuffer, sizeof(tempBuffer), 0x00, sizeof(tempBuffer));
        memResult = memcpy_s(tempBuffer, sizeof(tempBuffer), eventCnf->custAttachApn.nvAttachProfileInfo.pwd,
            TAF_NVIM_MAX_APNRETRY_PASSWORD_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tempBuffer), TAF_NVIM_MAX_APNRETRY_PASSWORD_LEN);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",", tempBuffer);

        /* 敏感信息使用完毕后重置 */
        (VOS_VOID)memset_s(tempBuffer, sizeof(tempBuffer), 0x00, sizeof(tempBuffer));
        (VOS_VOID)memset_s(eventCnf->custAttachApn.nvAttachProfileInfo.pwd,
            sizeof(eventCnf->custAttachApn.nvAttachProfileInfo.pwd), 0x00,
            sizeof(eventCnf->custAttachApn.nvAttachProfileInfo.pwd));
    }
    else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"\",");
    }

    /* 填充Auth Type */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d,", eventCnf->custAttachApn.nvAttachProfileInfo.authType);

    /* 填充Profile Name */
    if (eventCnf->custAttachApn.nvAttachProfileInfo.profileNameLen > 0) {
        /* HEX转String */
        (VOS_VOID)memset_s(tempBuffer, sizeof(tempBuffer), 0x00, sizeof(tempBuffer));
        (VOS_VOID)AT_HexToAsciiString(tempBuffer, TAF_NVIM_MAX_APNRETRY_PRONAME_LEN,
            eventCnf->custAttachApn.nvAttachProfileInfo.profileName, eventCnf->custAttachApn.nvAttachProfileInfo.profileNameLen);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", tempBuffer);
    }
    return length;
}

/*
 * 功能说明: 处理^LTEPROFILE=0,x,...获取APN定制信息命令返回
 * 输出参数: index: AT通道号
 *           eventInfo: 事件信息
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_RcvTafPsEvtGetCustomAttachProfileCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetCustAttachProfileInfoCnf *eventCnf = VOS_NULL_PTR;
    VOS_UINT32                          result = AT_ERROR;
    VOS_UINT16                          length = 0;
    VOS_UINT8                           imsiPrefixBufLen = 0;
    VOS_UINT8                           tempBuffer[TAF_MAX_LTE_APN_IMSI_PREFIX_SUPPORT + 1] = {0};

    eventCnf = (TAF_PS_GetCustAttachProfileInfoCnf *)evtInfo;
    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_LTEPROFILE_SET) {
        return VOS_ERR;
    }

    if (eventCnf->result == VOS_OK) {
        /* 填充AT命令头 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d,", g_parseContext[indexNum].cmdElement->cmdName,
            eventCnf->custAttachApn.exInfo.index);

        /* 填充IP类型 */
        if (eventCnf->custAttachApn.nvAttachProfileInfo.pdpType == TAF_PDP_IPV4V6) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",", "IPV4V6");
        } else if (eventCnf->custAttachApn.nvAttachProfileInfo.pdpType == TAF_PDP_IPV6) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",", "IPV6");
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",", "IP");
        }

        /* 填充IMSI */
        if (eventCnf->custAttachApn.nvAttachProfileInfo.imsiPrefixLen > 0) {
            eventCnf->custAttachApn.nvAttachProfileInfo.imsiPrefixLen =
                AT_MIN(eventCnf->custAttachApn.nvAttachProfileInfo.imsiPrefixLen,
                TAF_MAX_LTE_APN_IMSI_PREFIX_SUPPORT);

            imsiPrefixBufLen = (eventCnf->custAttachApn.nvAttachProfileInfo.imsiPrefixLen + 1) >> 1;
            (VOS_VOID)AT_HexToAsciiString(tempBuffer, TAF_MAX_LTE_APN_IMSI_PREFIX_SUPPORT,
                eventCnf->custAttachApn.nvAttachProfileInfo.imsiPrefixBcd, imsiPrefixBufLen);
            tempBuffer[eventCnf->custAttachApn.nvAttachProfileInfo.imsiPrefixLen] = '\0';
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s\",", tempBuffer);
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"\",");
        }

        /* 填充Profile信息 */
        length = AT_PrintAttachProfileInfo(length, eventCnf);
        result = AT_OK;
    }

    /* 停止AT保护定时器 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出打印内容 */
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

/*
 * 功能说明: 处理^LTEPROFILE=?命令异步返回
 * 输出参数: index: AT通道号
 *           eventInfo: 事件信息
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_RcvTafPsEvtGetCustomAttachProfileCountCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetCustAttachProfileCountCnf *eventCnf = VOS_NULL_PTR;
    VOS_UINT32                           result = AT_ERROR;
    VOS_UINT16                           length = 0;

    eventCnf = (TAF_PS_GetCustAttachProfileCountCnf*)evtInfo;
    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_LTEPROFILE_SET) {
        return VOS_ERR;
    }

    if (eventCnf->result == VOS_OK) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: (0,1),(0-%d),10,32,32,32,(0-3)",
            g_parseContext[indexNum].cmdElement->cmdName, eventCnf->maxCount);
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

/*
 * 功能说明: 处理^LTEAPNATTACH=x,...设置轮训开关命令返回
 * 输入参数: index: AT通道号
 *           eventInfo: 事件信息
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_RcvTafPsEvtSetAttachProfileSwitchStatusCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetAttachProfileSwitchStatusCnf *eventCnf = VOS_NULL_PTR;
    VOS_UINT32                              result = AT_ERROR;

    eventCnf = (TAF_PS_SetAttachProfileSwitchStatusCnf*)evtInfo;
    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_LTEAPNATTACH_SWITCH) {
        return VOS_ERR;
    }

    if (eventCnf->result == VOS_OK) {
        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

/*
 * 功能说明: 处理^LTEAPNATTACH?...查询轮训开关和轮训状态命令返回
 * 输入参数: index: AT通道号
 *           eventInfo: 事件信息
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_RcvTafPsEvtGetAttachProfileSwitchStatusCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetAttachProfileSwitchStatusCnf *eventCnf = VOS_NULL_PTR;
    VOS_UINT32                              result = AT_ERROR;
    VOS_UINT16                              length = 0;

    eventCnf = (TAF_PS_GetAttachProfileSwitchStatusCnf*)evtInfo;
    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_LTEAPNATTACH_SWITCH) {
        return VOS_ERR;
    }

    if (eventCnf->result == VOS_OK) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            eventCnf->switchFlag, eventCnf->status);
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

/*
 * 功能说明: 处理^LTEPROFILE=1,x,...设置APN定制信息命令返回
 * 输出参数: index: AT通道号
 *           eventInfo: 事件信息
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_RcvTafPsEvtSetCustomAttachProfileCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetCustAttachProfileInfoCnf *eventCnf = VOS_NULL_PTR;
    VOS_UINT32                          result = AT_ERROR;
    eventCnf = (TAF_PS_SetCustAttachProfileInfoCnf*)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_LTEPROFILE_SET) {
        return VOS_ERR;
    }

    if (eventCnf->result == VOS_OK) {
        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

/*
 * 功能说明: 处理LTEATTACHAPN主动上报
 * 输入参数: index: AT通道号
 *           eventInfo: 事件信息
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_RcvTafPsEvtSuccAttachProfileIndexInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SuccAttachProfileIndexInd   *eventInd = VOS_NULL_PTR;
    ModemIdUint16                       modemId = MODEM_ID_0;
    VOS_UINT16                          length = 0;

    eventInd = (TAF_PS_SuccAttachProfileIndexInd*)evtInfo;
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafPsEvtSuccAttachProfileIndexInd: Get modem id fail.");
        return VOS_ERR;
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s^LTEPROFILE: %d%s", g_atCrLf, eventInd->succProIdx, g_atCrLf);
    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return VOS_OK;
}

/*
 * 功能说明: A核收到DSAMBR查询结果的处理函数
 * 输入参数: msg: 结果消息
 * 返回结果: VOS_ERR: 执行失败
 *           VOS_OK: 执行成功
 */
VOS_UINT32 AT_RcvTafDsambrInfoQryCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_DsambrQryResponse *dsambrResponse = VOS_NULL_PTR;
    TAF_PS_DsambrQryResponsePara *dsambrResponsePara = VOS_NULL_PTR;
    VOS_UINT32 result = AT_ERROR;
    VOS_UINT16 length = 0;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DSAMBR_QRY) {
        return VOS_ERR;
    }

    dsambrResponse = (TAF_PS_DsambrQryResponse*)evtInfo;
    dsambrResponsePara = &(dsambrResponse->dsambrResponsePara);
    if (dsambrResponse->cause == TAF_ERR_NO_ERROR) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%d,%d,%d", "^DSAMBR: ",
            dsambrResponsePara->cid,
            dsambrResponsePara->ambr.dlApnAmbr,
            dsambrResponsePara->ambr.ulApnAmbr);
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = length;

    /* 停止保护定时器 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

/*
 * 功能说明: A核收到EPS承载激活或修改主动上报AMBR消息的处理函数
 * 输入参数: evtInfo: 结果消息
 * 返回结果: VOS_ERR: 执行失败
 *           VOS_OK: 执行成功
 */
VOS_UINT32 AT_RcvTafPsEvtDsambrInfoReportInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_DsambrReportInd *dsambrChangeInd = VOS_NULL_PTR;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT16 length = 0;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        return VOS_ERR;
    }

    dsambrChangeInd = (TAF_PS_DsambrReportInd*)evtInfo;

    /* 打印DSAMBR查询结果 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d%s", g_atCrLf, "^DSAMBR: ",
        dsambrChangeInd->cid, dsambrChangeInd->dlApnAmbr, dsambrChangeInd->ulApnAmbr, g_atCrLf);

    /* 输出结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvTafPsEvtSet5gQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_Set5gQosInfoCnf *set5gQosInfoCnf = VOS_NULL_PTR;

    set5gQosInfoCnf = (TAF_PS_Set5gQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_C5GQOS_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, set5gQosInfoCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGet5gQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t    memResult;
    VOS_UINT32 result;
    VOS_UINT16 length = 0;
    VOS_UINT32 tmp    = 0;

    TAF_5G_QosExt           cg5qos;
    TAF_PS_Get5GQosInfoCnf *get5gQosInfoCnf = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&cg5qos, sizeof(cg5qos), 0x00, sizeof(TAF_5G_QosExt));

    get5gQosInfoCnf = (TAF_PS_Get5GQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_C5GQOS_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < get5gQosInfoCnf->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cg5qos, sizeof(cg5qos), &get5gQosInfoCnf->ast5gQosInfo[tmp], sizeof(TAF_5G_QosExt));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cg5qos), sizeof(TAF_5G_QosExt));

        /* +C5GQOS:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", cg5qos.cid);
        /* <5QI> */
        if (cg5qos.op5Qi == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cg5qos.uc5Qi);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <DL GFBR> */
        if (cg5qos.opDlgfbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cg5qos.dlgfbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <UL GFBR> */
        if (cg5qos.opUlgfbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cg5qos.ulgfbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <DL MFBR> */
        if (cg5qos.opDlmfbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cg5qos.dlmfbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <UL MFBR> */
        if (cg5qos.opUlmfbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cg5qos.ulmfbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_VOID AT_PrintDynamic5gQosInfo(TAF_PS_GetDynamic5GQosInfoCnf *getDynamic5gQosInfoCnf, VOS_UINT8 indexNum)
{
    TAF_PS_5GDynamicQosExt st5gDynamicQos;
    VOS_UINT16             length = 0;
    VOS_UINT32             i;
    errno_t                memResult;

    memset_s(&st5gDynamicQos, sizeof(st5gDynamicQos), 0x00, sizeof(TAF_PS_5GDynamicQosExt));

    /* AT命令格式: [+C5GQOSRDP:
     * <cid>,<5QI>[,<DL_GFBR>,<UL_GFBR>[,<DL_MFBR>,<UL_MFBR>[,<DL_SAMBR>,<UL_SAMBR>[,<Averaging_window>]]]]] */
    for (i = 0; i < getDynamic5gQosInfoCnf->cidNum; i++) {
        if (i != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&st5gDynamicQos, sizeof(st5gDynamicQos), &getDynamic5gQosInfoCnf->ast5gQosInfo[i],
                             sizeof(TAF_PS_5GDynamicQosExt));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(st5gDynamicQos), sizeof(TAF_PS_5GDynamicQosExt));

        /* +C5GQOSRDP:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                         (VOS_CHAR *)g_atSndCodeAddress + length, "%d", st5gDynamicQos.cid);

        /* <QCI> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                         (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", st5gDynamicQos.uc5Qi);

        /* 如果各个参数如果存在就打印出来，目前按照这种方式打印出来 */
        /* <DL GFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.opDlgfbr, st5gDynamicQos.dlgfbr, &length);

        /* <UL GFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.opUlgfbr, st5gDynamicQos.ulgfbr, &length);

        /* <DL MFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.opDlmfbr, st5gDynamicQos.dlmfbr, &length);

        /* <UL MFBR> */
        AT_ConvertULParaToString(st5gDynamicQos.opUlmfbr, st5gDynamicQos.ulmfbr, &length);

        /* <Session ambr> */
        if (st5gDynamicQos.opSambr == VOS_TRUE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                             (VOS_CHAR *)g_atSndCodeAddress + length, ",%d",
                                             st5gDynamicQos.ambr.dlSessionAmbr);

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                             (VOS_CHAR *)g_atSndCodeAddress + length, ",%d",
                                             st5gDynamicQos.ambr.ulSessionAmbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",,");
        }

        /* <Average Window> */
        if (st5gDynamicQos.opAveragWindow == VOS_TRUE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                             (VOS_CHAR *)g_atSndCodeAddress + length, ",%d",
                                             st5gDynamicQos.averagWindow);
        }
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamic5gQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetDynamic5GQosInfoCnf *getDynamic5gQosInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                     result                 = AT_FAILURE;

    getDynamic5gQosInfoCnf = (TAF_PS_GetDynamic5GQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_C5GQOSRDP_SET) {
        return VOS_ERR;
    }

    if (getDynamic5gQosInfoCnf->cause == TAF_PS_CAUSE_SUCCESS) {
        AT_PrintDynamic5gQosInfo(getDynamic5gQosInfoCnf, indexNum);
        result = AT_OK;
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    AT_StopTimerCmdReady(indexNum);

    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_ChkTafPsEvtGet5gQosInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_Get5GQosInfoCnf *qosInfo = VOS_NULL_PTR;
    const TAF_PS_Evt             *event   = VOS_NULL_PTR;
    VOS_UINT32                    length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_Get5GQosInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_WARN_LOG2("AT_ChkTafPsEvtGet5gQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    qosInfo = (const TAF_PS_Get5GQosInfoCnf *)(event->content);
    length += (qosInfo->cidNum * sizeof(TAF_5G_QosExt));
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_WARN_LOG2("AT_ChkTafPsEvtGet5gQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetDynamic5gQosInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetDynamic5GQosInfoCnf *epsQos = VOS_NULL_PTR;
    const TAF_PS_Evt                    *event = VOS_NULL_PTR;
    VOS_UINT32                           length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDynamic5GQosInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamic5gQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    epsQos  = (const TAF_PS_GetDynamic5GQosInfoCnf *)(event->content);
    length += sizeof(TAF_PS_5GDynamicQosExt) * epsQos->cidNum;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamic5gQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_RcvTafPsEvtUePolicyRptInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_UePolicyRptInd *uePolicyInd = VOS_NULL_PTR;
    VOS_UINT16             length;
    ModemIdUint16          modemId;

    uePolicyInd = (TAF_PS_UePolicyRptInd *)evtInfo;
    modemId     = MODEM_ID_0;

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafPsEvtUePolicyRptInd: Get modem id fail.");
        return VOS_ERR;
    }

    /* UE POLICY IND码流长度为0时，用于URSP更新上报，不携带任何参数 */
    if (uePolicyInd->totalLength == 0) {
        /* ^CPOLICYRPT */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CPOLICYRPT%s", g_atCrLf, g_atCrLf);
    } else {
        /* ^CPOLICYRPT: <total_len>,<section_num>,<protocal_ver> */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CPOLICYRPT: %d,%d,%04x%s", g_atCrLf, uePolicyInd->totalLength,
            uePolicyInd->sectionNum, uePolicyInd->protocalVer, g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtUePolicyRspCheckRsltInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_UePolicyRspCheckRsltInd *checkRsltInd = VOS_NULL_PTR;
    VOS_UINT32                      result;

    checkRsltInd = (TAF_PS_UePolicyRspCheckRsltInd *)evtInfo;

    /* AT模块在等待^CSUEPOLICY命令的操作结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CSUEPOLICY_SET) {
        AT_WARN_LOG("AT_RcvTafPsEvtUePolicyRspCheckRsltInd: WARNING:Not AT_CMD_CSUEPOLICY_SET!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断设置操作是否成功 */
    if (checkRsltInd->rslt == TAF_PS_UE_POLICY_RSP_CHECK_RSLT_SUCC) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEtherSessCapInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_EtherSessCapInd *etherSessCapInd = (TAF_PS_EtherSessCapInd *)evtInfo;

    AT_GetCommPsCtxAddr()->etherCap = etherSessCapInd->enable;
    return VOS_OK;
}

TAF_PS_PrefAccessTypeUint8 AT_TransPrefAccessType(PS_PREF_AccessTypeUint8 accType)
{
    TAF_PS_PrefAccessTypeUint8 prefAccType = TAF_PS_PREF_ACCESS_TYPE_3GPP;

    switch (accType) {
        case PS_PREF_ACCESS_TYPE_3GPP:
            prefAccType = TAF_PS_PREF_ACCESS_TYPE_3GPP;
            break;

        case PS_PREF_ACCESS_TYPE_NON_3GPP:
            prefAccType = TAF_PS_PREF_ACCESS_TYPE_NON_3GPP;
            break;

        default:
            break;
    }

    return prefAccType;
}

TAF_PS_SscModeUint8 AT_TransSscMode(PS_SSC_ModeUint8 sscMode)
{
    TAF_PS_SscModeUint8 tafSscMode = TAF_PS_SSC_MODE_1;

    switch (sscMode) {
        case PS_SSC_MODE_1:
            tafSscMode = TAF_PS_SSC_MODE_1;
            break;

        case PS_SSC_MODE_2:
            tafSscMode = TAF_PS_SSC_MODE_2;
            break;

        case PS_SSC_MODE_3:
            tafSscMode = TAF_PS_SSC_MODE_3;
            break;

        default:
            break;
    }

    return tafSscMode;
}

VOS_VOID AT_Print5gUrspRsdSscModeInfo(const PS_URSP_Rsd *rstInfo, VOS_UINT16 *length)
{
    if (rstInfo->opSscMode == VOS_TRUE) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%d", AT_TransSscMode(rstInfo->sscMode));
    } else {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
    }
}

VOS_VOID AT_Print5gUrspRsdNssaiInfo(const PS_URSP_Rsd *rstInfo, VOS_UINT16 *length)
{
    VOS_CHAR acStrNssai[AT_EVT_MULTI_S_NSSAI_LEN];
    VOS_UINT32 nssaiLen = 0;

    (VOS_VOID)memset_s(acStrNssai, sizeof(acStrNssai), 0, sizeof(acStrNssai));

    AT_ConvertMultiSNssaiToString(AT_MIN(rstInfo->sNssaiNum, PS_URSP_MAX_S_NSSAI_NUM),
        &rstInfo->sNssai[0], acStrNssai, sizeof(acStrNssai), &nssaiLen);
    if (nssaiLen != 0) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"%s\"", acStrNssai);
    } else {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
    }
}

VOS_VOID AT_Print5gUrspRsdDnnInfo(const PS_URSP_Rsd *rstInfo, VOS_UINT16 *length)
{
    VOS_UINT32 i;
    VOS_UINT8 tmpNum;

    if (rstInfo->dnnNum == 0) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");

        return;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"");

    tmpNum = AT_MIN(rstInfo->dnnNum, PS_URSP_MAX_DNN_NUM);
    for (i = 0; i < tmpNum; i++) {
        if (i == tmpNum - 1) {
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), "%s\"", rstInfo->dnn[i].value);
        } else {
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), "%s;", rstInfo->dnn[i].value);
        }
    }
}

VOS_VOID AT_Print5gUrspRsdPduSessionType(const PS_URSP_Rsd *rstInfo, VOS_UINT16 *length)
{
    if (rstInfo->opPduSessionType != VOS_TRUE) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
        return;
    }

    if (rstInfo->pduSessionType == PS_PDU_SESSION_TYPE_IPV4) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_IP].text);
    } else if (rstInfo->pduSessionType == PS_PDU_SESSION_TYPE_IPV6) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_IPV6].text);
    } else if (rstInfo->pduSessionType == PS_PDU_SESSION_TYPE_IPV4V6) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_IPV4V6].text);
    } else if (rstInfo->pduSessionType == PS_PDU_SESSION_TYPE_ETHERNET) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_Ethernet].text);
    } else {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_PPP].text);
    }
}

VOS_VOID AT_Print5gUrspRsdInfo(const TAF_PS_UrspRsdQryCnf *rsdQryCnf, VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 i;

    for (i = 0; i < (VOS_UINT32)AT_MIN(rsdQryCnf->rsdNum, PS_URSP_MAX_RSD_NUM); i++) {
        if (i != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        /* ^C5GRSDQRY:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <cid> */
        if (rsdQryCnf->cid[i] != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d", rsdQryCnf->cid[i]);
        }

        /* <route selection descriptor type> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rsdQryCnf->rsd[i].type);

        /* <ursp rule precedence> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rsdQryCnf->rsd[i].urspPrecedence);

        /* <route selection descriptor precedence> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rsdQryCnf->rsd[i].rsdPrecedence);

        /* <ssc mode> */
        AT_Print5gUrspRsdSscModeInfo(&rsdQryCnf->rsd[i], &length);

        /* <nssai> */
        AT_Print5gUrspRsdNssaiInfo(&rsdQryCnf->rsd[i], &length);

        /* <dnn> */
        AT_Print5gUrspRsdDnnInfo(&rsdQryCnf->rsd[i], &length);

        /* <pdu session type> */
        AT_Print5gUrspRsdPduSessionType(&rsdQryCnf->rsd[i], &length);

        /* <preferred access type> */
        if (rsdQryCnf->rsd[i].opPrefAccType == VOS_TRUE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", AT_TransPrefAccessType(rsdQryCnf->rsd[i].prefAccType));
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }

        /* <non-seamless non-3gpp offload indication> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rsdQryCnf->rsd[i].offloadFlg);
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_RcvTafPsGet5gUrspRsdInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_UrspRsdQryCnf *rsdQryCnf = VOS_NULL_PTR;
    VOS_UINT32 result = AT_FAILURE;

    rsdQryCnf = (TAF_PS_UrspRsdQryCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_C5GRSDQRY_SET) {
        return VOS_ERR;
    }

    if (rsdQryCnf->rsdNum != 0) {
        AT_Print5gUrspRsdInfo(rsdQryCnf, indexNum);
        result = AT_OK;
    } else {
        /* rsdNum为0，表示没有找到有效rsd信息，查询失败 */
        result = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    AT_StopTimerCmdReady(indexNum);

    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#endif

#if (FEATURE_IMS == FEATURE_ON)
LOCAL VOS_VOID AT_UpdateIpv4Ipv6PdnInfo(AT_IMS_EmcRdp *imsEmcRdp, const IMSA_AT_EmcPdnActivateCnf *pdnActivateCnf)
{
    errno_t memResult;

    /* 更新IPv4 PDN信息 */
    if ((pdnActivateCnf->pdpAddr.pdpType == TAF_PDP_IPV4) || (pdnActivateCnf->pdpAddr.pdpType == TAF_PDP_IPV4V6)) {
        imsEmcRdp->opIPv4PdnInfo = VOS_TRUE;

        memResult = memcpy_s(imsEmcRdp->iPv4PdnInfo.ipAddr, sizeof(imsEmcRdp->iPv4PdnInfo.ipAddr),
            pdnActivateCnf->pdpAddr.ipv4Addr, sizeof(pdnActivateCnf->pdpAddr.ipv4Addr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv4PdnInfo.ipAddr), sizeof(pdnActivateCnf->pdpAddr.ipv4Addr));

        if (pdnActivateCnf->ipv4Dns.bitOpPrimDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv4PdnInfo.dnsPrimAddr, sizeof(imsEmcRdp->iPv4PdnInfo.dnsPrimAddr),
                pdnActivateCnf->ipv4Dns.primDnsAddr, sizeof(pdnActivateCnf->ipv4Dns.primDnsAddr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv4PdnInfo.dnsPrimAddr),
            sizeof(pdnActivateCnf->ipv4Dns.primDnsAddr));
        }

        if (pdnActivateCnf->ipv4Dns.bitOpSecDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv4PdnInfo.dnsSecAddr, sizeof(imsEmcRdp->iPv4PdnInfo.dnsSecAddr),
                pdnActivateCnf->ipv4Dns.secDnsAddr, sizeof(pdnActivateCnf->ipv4Dns.secDnsAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv4PdnInfo.dnsSecAddr),
                sizeof(pdnActivateCnf->ipv4Dns.secDnsAddr));
        }

        imsEmcRdp->iPv4PdnInfo.mtu = pdnActivateCnf->mtu;
    }
    /* 更新IPv6 PDN信息 */
    if ((pdnActivateCnf->pdpAddr.pdpType == TAF_PDP_IPV6) || (pdnActivateCnf->pdpAddr.pdpType == TAF_PDP_IPV4V6)) {
        imsEmcRdp->opIPv6PdnInfo = VOS_TRUE;

        memResult = memcpy_s(imsEmcRdp->iPv6PdnInfo.ipAddr, sizeof(imsEmcRdp->iPv6PdnInfo.ipAddr),
            pdnActivateCnf->pdpAddr.ipv6Addr, sizeof(pdnActivateCnf->pdpAddr.ipv6Addr));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv6PdnInfo.ipAddr), sizeof(pdnActivateCnf->pdpAddr.ipv6Addr));

        if (pdnActivateCnf->ipv6Dns.bitOpPrimDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv6PdnInfo.dnsPrimAddr, sizeof(imsEmcRdp->iPv6PdnInfo.dnsPrimAddr),
                pdnActivateCnf->ipv6Dns.primDnsAddr, sizeof(pdnActivateCnf->ipv6Dns.primDnsAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv6PdnInfo.dnsPrimAddr),
            sizeof(pdnActivateCnf->ipv6Dns.primDnsAddr));
        }

        if (pdnActivateCnf->ipv6Dns.bitOpSecDnsAddr == VOS_TRUE) {
            memResult = memcpy_s(imsEmcRdp->iPv6PdnInfo.dnsSecAddr, sizeof(imsEmcRdp->iPv6PdnInfo.dnsSecAddr),
                pdnActivateCnf->ipv6Dns.secDnsAddr, sizeof(pdnActivateCnf->ipv6Dns.secDnsAddr));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(imsEmcRdp->iPv6PdnInfo.dnsSecAddr),
                sizeof(pdnActivateCnf->ipv6Dns.secDnsAddr));
        }

        imsEmcRdp->iPv6PdnInfo.mtu = pdnActivateCnf->mtu;
    }
}

VOS_UINT32 AT_RcvImsaEmcPdnActivateCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    IMSA_AT_EmcPdnActivateCnf  *pdnActivateCnf = (IMSA_AT_EmcPdnActivateCnf *)msg;
    AT_IMS_EmcRdp              *imsEmcRdp      = VOS_NULL_PTR;

    /* 建立失败，直接回复结果 */
    if (pdnActivateCnf->result == VOS_ERR) {
        AT_WARN_LOG("AT_RcvImsaEmcPdnActivateCnf: EmcPdn Active fail.");
        AT_ReportImsEmcStatResult(indexNum, AT_PDP_STATUS_DEACT);
        return VOS_OK;
    }

    /* 获取IMS EMC RDP */
    imsEmcRdp = AT_GetImsEmcRdpByClientId(indexNum);
    if (imsEmcRdp == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_RcvImsaEmcPdnActivateInd: ImsEmcRdp not found.");
        return VOS_ERR;
    }

    /* 清除IMS EMC信息 */
    (VOS_VOID)memset_s(imsEmcRdp, sizeof(AT_IMS_EmcRdp), 0x00, sizeof(AT_IMS_EmcRdp));

    AT_UpdateIpv4Ipv6PdnInfo(imsEmcRdp, pdnActivateCnf);

    /* 上报连接状态 */
    AT_ReportImsEmcStatResult(indexNum, AT_PDP_STATUS_ACT);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaEmcPdnDeactivateCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    IMSA_AT_EmcPdnDeactivateCnf *pstPdnDeactivateCnf = (IMSA_AT_EmcPdnDeactivateCnf *)msg;
    AT_IMS_EmcRdp               *imsEmcRdp           = VOS_NULL_PTR;

    if (pstPdnDeactivateCnf->result == VOS_OK) {
        /* 上报状态 */
        AT_ReportImsEmcStatResult(indexNum, AT_PDP_STATUS_DEACT);

        /* 获取IMS EMC上下文 */
        imsEmcRdp = AT_GetImsEmcRdpByClientId(indexNum);
        if (imsEmcRdp == VOS_NULL_PTR) {
            AT_WARN_LOG("AT_RcvImsaEmcPdnDeactivateCnf: ImsEmcRdp not found.");
            return VOS_ERR;
        }

        /* 检查IMS EMC状态 */
        if ((imsEmcRdp->opIPv4PdnInfo != VOS_TRUE) && (imsEmcRdp->opIPv6PdnInfo != VOS_TRUE)) {
            AT_WARN_LOG("AT_RcvImsaEmcPdnDeactivateCnf: IMS EMC PDN not active.");
            return VOS_ERR;
        }

        /* 清除IMS EMC信息 */
        (VOS_VOID)memset_s(imsEmcRdp, sizeof(AT_IMS_EmcRdp), 0x00, sizeof(AT_IMS_EmcRdp));
     } else {
        /* 没有释放成功，给上报hold */
        AT_ReportImsEmcStatResult(indexNum, AT_PDP_STATUS_HOLD);
     }

    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvTafPsEvtSetPsCallRptCmdCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetPsCallRptCmdCnf *setRptCmdCnf = (TAF_PS_SetPsCallRptCmdCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PSCALLRPTCMD_SET) {
        AT_WARN_LOG("AT_RcvTafPsEvtSetPsCallRptCmdCnf: WARNING: CmdCurrentOpt != AT_CMD_PSCALLRPTCMD_SET!");
        return VOS_ERR;
    }

    /* 失败场景恢复为默认值 */
    if (setRptCmdCnf->cause != TAF_PS_CAUSE_SUCCESS) {
        AT_GetCommPsCtxAddr()->stickCmd = TAF_PS_CALL_RPT_CMD_NDISSTAT;
    }

    /* 处理错误码 上报结果 */
    AT_PrcoPsEvtErrCode(indexNum, setRptCmdCnf->cause);
    return VOS_OK;
}

VOS_VOID AT_RcvNdisPdnInfoCfgCnf(AT_NDIS_PdnInfoCfgCnf *ndisPdnInfoCfgCnf)
{
    if (ndisPdnInfoCfgCnf->result == AT_NDIS_PDNCFG_CNF_FAIL) {
        AT_ERR_LOG("AT_RcvNdisPdnInfoRelCnf: Failed.");
    }
}
VOS_UINT32 AT_GetLanAddr32(VOS_UINT8 *addr)
{
    VOS_UINT32 addrPara;

    addrPara = *addr++;
    addrPara <<= 8;
    addrPara |= *addr++;
    addrPara <<= 8;
    addrPara |= *addr++;
    addrPara <<= 8;
    addrPara |= *addr;

    return addrPara;
}

VOS_VOID AT_PS_AddIpAddrMap(VOS_UINT16 clientId, TAF_PS_CallPdpActivateCnf *event)
{
    AT_ModemPsCtx *psModemCtx = VOS_NULL_PTR;
    VOS_UINT32     ipAddr;

    psModemCtx = AT_GetModemPsCtxAddrFromClientId(clientId);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    if (event->ifaceId >= PS_IFACE_ID_BUTT) {
        return;
    }
#else
    if (!AT_PS_IS_RABID_VALID(event->rabId)) {
        return;
    }
#endif

    if (event->opPdpAddr) {
        ipAddr = AT_GetLanAddr32(event->pdpAddr.ipv4Addr);
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
        psModemCtx->ipAddrIfaceIdMap[event->ifaceId] = ipAddr;
#else
        psModemCtx->ipAddrRabIdMap[event->rabId - AT_PS_RABID_OFFSET] = ipAddr;
#endif
    }
}

VOS_VOID AT_AnswerPdpActInd(VOS_UINT8 indexNum, TAF_PS_CallPdpActivateCnf *event)
{
    VOS_ULONG               userData;
    VOS_UINT32              rslt;
    VOS_UINT16              pppId;
    DMS_PortIdUint16        portId;

    userData = (VOS_ULONG)g_atClientTab[indexNum].portNo;
    portId   = g_atClientTab[indexNum].portNo;
    pppId = 0;

    if (event->pdpAddr.pdpType == TAF_PDP_PPP) {
        if (PPPA_CreateRawDataPppReq(&pppId, userData, DMS_PORT_SendPppPacket) != VOS_OK) {
            rslt = AT_ERROR;
        } else {
            /* 记录PPP id和Index的对应关系 */
            g_atPppIndexTab[pppId] = indexNum;

            /* 保存PPP id */
            g_atClientTab[indexNum].pppId = pppId;

            DMS_PORT_SwitchToPppDataMode(portId, DMS_PORT_DATA_PPP_RAW, PPP_PullRawDataEvent, pppId);

            /* 注册下行数据发送函数 */
            PPP_RegDlDataCallback(pppId);

            rslt = AT_CONNECT;
        }

        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, rslt);
    } else {
        /* 方案待定.... */
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_CONNECT);
    }
}

VOS_VOID AT_FillPppIndConfigIpv4Info(AT_PPP_IndConfigInfo *pppIndConfigInfo, TAF_PS_CallPdpActivateCnf *event)
{
    errno_t memResult;

    pppIndConfigInfo->ipv4Flag = VOS_TRUE;
    /* 填写IP地址 */
    memResult = memcpy_s(pppIndConfigInfo->ipAddr, sizeof(pppIndConfigInfo->ipAddr), event->pdpAddr.ipv4Addr,
                         TAF_IPV4_ADDR_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pppIndConfigInfo->ipAddr), TAF_IPV4_ADDR_LEN);

    /* 填写DNS地址 */
    if (event->dns.bitOpPrimDnsAddr == VOS_TRUE) {
        pppIndConfigInfo->pcoIpv4Item.opPriDns = VOS_TRUE;

        memResult = memcpy_s(pppIndConfigInfo->pcoIpv4Item.priDns, sizeof(pppIndConfigInfo->pcoIpv4Item.priDns),
                             event->dns.primDnsAddr, TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pppIndConfigInfo->pcoIpv4Item.priDns), TAF_IPV4_ADDR_LEN);
    }

    if (event->dns.bitOpSecDnsAddr == VOS_TRUE) {
        pppIndConfigInfo->pcoIpv4Item.opSecDns = VOS_TRUE;

        memResult = memcpy_s(pppIndConfigInfo->pcoIpv4Item.secDns, sizeof(pppIndConfigInfo->pcoIpv4Item.secDns),
                             event->dns.secDnsAddr, TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pppIndConfigInfo->pcoIpv4Item.secDns), TAF_IPV4_ADDR_LEN);
    }

    /* 填写NBNS地址 */
    if (event->nbns.bitOpPrimNbnsAddr == VOS_TRUE) {
        pppIndConfigInfo->pcoIpv4Item.opPriNbns = VOS_TRUE;

        memResult = memcpy_s(pppIndConfigInfo->pcoIpv4Item.priNbns, sizeof(pppIndConfigInfo->pcoIpv4Item.priNbns),
                             event->nbns.primNbnsAddr, TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pppIndConfigInfo->pcoIpv4Item.priNbns), TAF_IPV4_ADDR_LEN);
    }

    if (event->nbns.bitOpSecNbnsAddr == VOS_TRUE) {
        pppIndConfigInfo->pcoIpv4Item.opSecNbns = VOS_TRUE;

        memResult = memcpy_s(pppIndConfigInfo->pcoIpv4Item.secNbns, sizeof(pppIndConfigInfo->pcoIpv4Item.secNbns),
                             event->nbns.secNbnsAddr, TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pppIndConfigInfo->pcoIpv4Item.secNbns), TAF_IPV4_ADDR_LEN);
    }

    /* 填写GATE WAY地址 */
    if (event->gateWay.bitOpGateWayAddr == VOS_TRUE) {
        pppIndConfigInfo->pcoIpv4Item.opGateWay = VOS_TRUE;

        memResult = memcpy_s(pppIndConfigInfo->pcoIpv4Item.gateWay, sizeof(pppIndConfigInfo->pcoIpv4Item.gateWay),
                             event->gateWay.gateWayAddr, TAF_IPV4_ADDR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pppIndConfigInfo->pcoIpv4Item.gateWay), TAF_IPV4_ADDR_LEN);
    }
}

VOS_VOID AT_FillPppIndConfigIpv6Info(AT_PPP_IndConfigInfo *pppIndConfigInfo, TAF_PS_CallPdpActivateCnf *event)
{
    errno_t memResult;

    pppIndConfigInfo->ipv6Flag = VOS_TRUE;
    memResult = memcpy_s(pppIndConfigInfo->ipv6Addr, sizeof(pppIndConfigInfo->ipv6Addr),
                         event->pdpAddr.ipv6Addr + IPV6_INTERFACE_ID_LEN, IPV6_INTERFACE_ID_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pppIndConfigInfo->ipv6Addr), IPV6_INTERFACE_ID_LEN);
}

VOS_VOID AT_FillPppIndConfigInfoPara(AT_PPP_IndConfigInfo *pppIndConfigInfo, TAF_PS_CallPdpActivateCnf *event)
{
    if ((event->pdpAddr.pdpType & TAF_PDP_IPV4)== TAF_PDP_IPV4) {
        AT_FillPppIndConfigIpv4Info(pppIndConfigInfo, event);
    }

    if ((event->pdpAddr.pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        AT_FillPppIndConfigIpv6Info(pppIndConfigInfo, event);
    }
}

VOS_VOID AT_ModemPsRspPdpActEvtCnfProc(VOS_UINT8 indexNum, TAF_PS_CallPdpActivateCnf *event)
{
    AT_PPP_IndConfigInfo pppIndConfigInfo;
    DMS_PortIdUint16     portId;
    VOS_UINT16           pppId;

    /* 初始化 */
    (VOS_VOID)memset_s(&pppIndConfigInfo, sizeof(pppIndConfigInfo), 0x00, sizeof(pppIndConfigInfo));
    portId = g_atClientTab[indexNum].portNo;
    pppId  = AT_PS_GET_PPPID(indexNum);

    /* 清除命令操作类型 */
    AT_StopTimerCmdReady(indexNum);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    g_atClientTab[indexNum].ifaceId = PS_IFACE_ID_PPP0;
#endif

    if (event->pdpAddr.pdpType == TAF_PDP_PPP) {
        /* 注册ppp包接收函数 */
        DMS_PORT_SwitchToPppDataMode(portId, DMS_PORT_DATA_PPP_RAW, PPP_PullRawDataEvent, pppId);

        /* 注册下行数据发送函数 */
        PPP_RegDlDataCallback(g_atClientTab[indexNum].pppId);

        At_FormatResultData(indexNum, AT_CONNECT);
    } else if ((event->pdpAddr.pdpType == TAF_PDP_IPV4) ||
               (event->pdpAddr.pdpType == TAF_PDP_IPV6) ||
               (event->pdpAddr.pdpType == TAF_PDP_IPV4V6)) {
        /* 填写IP地址, DNS, NBNS */
        AT_FillPppIndConfigInfoPara(&pppIndConfigInfo, event);

        /* 将AUTH和IPCP帧交给PPP处理: */
        PPPA_RcvConfigInfoInd(g_atClientTab[indexNum].pppId, (PPPA_PdpActiveResult *)&pppIndConfigInfo);
    } else {
        /* 其他类型不做处理 */
    }
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallPdpActivateCnf *event = VOS_NULL_PTR;
    ModemIdUint16       modemId;
    DMS_PortIdUint16    portId;
    VOS_UINT16          pppId;

    /* 初始化 */
    event = (TAF_PS_CallPdpActivateCnf *)evtInfo;

    /* 记录<CID> */
    g_atClientTab[indexNum].cid = event->cid;

    /* 记录<RabId> */
    modemId = MODEM_ID_0;
    portId  = g_atClientTab[indexNum].portNo;
    pppId   = AT_PS_GET_PPPID(indexNum);

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_RcvTafPsCallEvtPdpActivateCnf: Get modem id fail.");
        return AT_ERROR;
    }

    /* 保存为扩展RABID 等于 modemId + rabId */
    g_atClientTab[indexNum].exPsRabId = AT_BUILD_EXRABID(modemId, event->rabId);
#if (FEATURE_MBB_CUST == FEATURE_ON)
    if (event->cause == VOS_TRUE) {
        /* 在双栈拨号时有错误码则记录错误码 */
        AT_PS_SetPsCallErrCause(indexNum, event->cause);
    } else {
        /* 没有错误码则清空错误码 */
        AT_PS_SetPsCallErrCause(indexNum, TAF_PS_CAUSE_SUCCESS);
    }
#else
    /* 清除PS域呼叫错误码 */
    AT_PS_SetPsCallErrCause(indexNum, TAF_PS_CAUSE_SUCCESS);
#endif

    AT_PS_AddIpAddrMap(indexNum, event);

    /* 根据操作类型 */
    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_CGACT_ORG_SET:
        case AT_CMD_CGANS_ANS_SET:
            AT_StopTimerCmdReady(indexNum);
            At_FormatResultData(indexNum, AT_OK);
            break;

        case AT_CMD_CGDATA_SET:
            AT_StopTimerCmdReady(indexNum);
            DMS_PORT_SwitchToPppDataMode(portId, DMS_PORT_DATA_PPP_RAW, PPP_PullRawDataEvent, pppId);
            At_FormatResultData(indexNum, AT_CONNECT);
            break;

        case AT_CMD_CGANS_ANS_EXT_SET:
            AT_AnswerPdpActInd(indexNum, event);
            break;

        case AT_CMD_D_IP_CALL_SET:
        case AT_CMD_PPP_ORG_SET:
            /*
             * Modem发起的PDP激活成功，
             * AT_CMD_D_IP_CALL_SET为PPP类型
             * AT_CMD_PPP_ORG_SET为IP类型
             */
            AT_ModemPsRspPdpActEvtCnfProc(indexNum, event);
            break;

        default:
            break;
    }

    return VOS_OK;
}

VOS_VOID AT_ModemPsRspPdpActEvtRejProc(VOS_UINT8 indexNum, TAF_PS_CallPdpActivateRej *event)
{
    VOS_UINT32 result = AT_FAILURE;
    VOS_UINT16 length = 0;

    if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_D_PPP_CALL_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_PPP_ORG_SET)) {
        result = AT_NO_CARRIER;

        AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_REQ);

        /* 返回命令模式 */
        DMS_PORT_ResumeCmdMode(g_atClientTab[indexNum].portNo);

    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_D_IP_CALL_SET) {
        result = AT_ERROR;
        AT_SendRelPppReq(g_atClientTab[indexNum].pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
    } else {
        result = AT_ERROR;
    }

    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpActivateRej(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallPdpActivateRej *event = VOS_NULL_PTR;

    event = (TAF_PS_CallPdpActivateRej *)evtInfo;

    /* 记录PS域呼叫错误码 */
    AT_PS_SetPsCallErrCause(indexNum, event->cause);

    /* 按用户类型分别处理 */
    switch (g_atClientTab[indexNum].userType) {
        /* MODEM拨号处理 */
        case AT_HSUART_USER:
        case AT_MODEM_USER:
        case AT_UART_USER:
#if (FEATURE_IOT_CMUX == FEATURE_ON)
        case AT_CMUXAT_USER:
        case AT_CMUXMDM_USER:
        case AT_CMUXEXT_USER:
        case AT_CMUXGPS_USER:
#endif
            AT_ModemPsRspPdpActEvtRejProc(indexNum, event);
            return VOS_OK;

        /* NDIS拨号处理 */
        case AT_NDIS_USER:
        /* E5和闪电卡使用同一个端口名 */
        case AT_APP_USER:
            if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CGACT_ORG_SET) {
                /* AT+CGACT拨号 */
                AT_StopTimerCmdReady(indexNum);
                At_FormatResultData(indexNum, AT_ERROR);
            }

            return VOS_OK;

        /* 其他端口全部返回ERROR */
        default:
            AT_StopTimerCmdReady(indexNum);
            At_FormatResultData(indexNum, AT_ERROR);
            break;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpModifyCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGCMOD_SET) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpModifyRej(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGCMOD_SET) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_ERROR);

    return VOS_OK;
}

VOS_VOID AT_ModemPsRspPdpDeactEvtCnfProc(VOS_UINT8 indexNum, TAF_PS_CallPdpDeactivateCnf *event)
{
    AT_ClientManage        *clientManage = VOS_NULL_PTR;
    VOS_UINT32              modemUsrFlg;
    VOS_UINT32              aTHCmdFlg;
    VOS_UINT32              result;
    VOS_UINT16              length;
    DMS_PortDataModeUint8   dataMode;

    modemUsrFlg = AT_CheckModemUser(indexNum);
    length      = 0;
    result      = AT_FAILURE;
    clientManage = AT_GetClientManage(indexNum);
    aTHCmdFlg   = (clientManage->cmdCurrentOpt == AT_CMD_H_PS_SET) ? VOS_TRUE : VOS_FALSE;
    dataMode    = DMS_PORT_GetDataMode(clientManage->portNo);

    if (AT_IsPppUser(indexNum) == VOS_FALSE) {
        return;
    }

    if (dataMode == DMS_PORT_DATA_PPP) {
        /* 释放PPP实体 & HDLC去使能 */
        AT_SendRelPppReq(clientManage->pppId, PPP_AT_CTRL_REL_PPP_REQ);
    } else if (dataMode == DMS_PORT_DATA_PPP_RAW) {
        /* 释放PPP实体 & HDLC去使能 */
        AT_SendRelPppReq(clientManage->pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);
    } else {
        /* 其它数传模式，不用处理 */
        AT_WARN_LOG("TAF_PS_EVT_PDP_DEACTIVE_CNF OTHER MODE");
    }

    /* 返回命令模式 */
    DMS_PORT_ResumeCmdMode(clientManage->portNo);

    if ((event->pdpType & TAF_PDP_IPV6) == TAF_PDP_IPV6) {
        AT_PS_SendNdserverPppIPv6RelInd();
    }
    /*
     * 为了规避Linux后台二次拨号失败问题，采用如下规避方案:
     * PDP正常断开流程结束后，接收到TAF_PS_EVT_PDP_DEACTIVE_CNF事件，判断操作
     * 系统是否为Linux，若否，则按照原先流程处理，若是，则不再发送"NO CARRIER"
     */
    if ((DRV_GET_LINUXSYSTYPE() == VOS_OK) && (modemUsrFlg == VOS_TRUE)) {
        /*
         * 原先流程中，在At_FormatResultData函数内部，发送了"NO CARRIER"之后，
         * 需要将DCD拉低，在此规避方案中，不发送"NO CARRIER"，但DCD信号的拉低
         * 操作仍旧需要保持
         */
        AT_StopTimerCmdReady(indexNum);
        DMS_PORT_DeassertDcd(clientManage->portNo);
        return;
    }

    if (aTHCmdFlg == VOS_TRUE) {
        result = AT_OK;
    } else {
        result = AT_NO_CARRIER;
    }

    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);

    /* ATH断开PPP连接时, 如果DCD信号模式为CONNECT ON, 需要拉低DCD信号 */
    if ((result == AT_OK) && (DMS_PORT_IsDcdModeConnectOn() == VOS_TRUE)) {
        DMS_PORT_DeassertDcd(clientManage->portNo);
    }
}

VOS_UINT32 AT_RcvTafPsCallEvtPdpDeactivateCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallPdpDeactivateCnf *event = VOS_NULL_PTR;

    event = (TAF_PS_CallPdpDeactivateCnf *)evtInfo;

    AT_PS_DeleteIpAddrMap(indexNum, event);

    /* 还是应该先判断是否是数传状态，再决定处理 */
    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_CGACT_END_SET:
            AT_StopTimerCmdReady(indexNum);
            DMS_PORT_ResumeCmdMode(g_atClientTab[indexNum].portNo);
            At_FormatResultData(indexNum, AT_OK);
            break;

        case AT_CMD_H_PS_SET:
        case AT_CMD_PS_DATA_CALL_END_SET:
            AT_ModemPsRspPdpDeactEvtCnfProc(indexNum, event);
            break;

        default:
            break;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsCallEvtCallEndCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CallEndCnf *callEndCnf = VOS_NULL_PTR;

    /* 初始化 */
    callEndCnf = (TAF_PS_CallEndCnf *)evtInfo;

    if (AT_IsPppUser(indexNum) == VOS_TRUE) {
        AT_ModemProcCallEndCnfEvent(callEndCnf->cause, indexNum);
        return VOS_OK;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetGprsActiveTypeCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT32                    result = AT_FAILURE;
    TAF_PS_GetDGprsActiveTypeCnf *getGprsActiveTypeCnf = VOS_NULL_PTR;

    /* 初始化 */
    getGprsActiveTypeCnf = (TAF_PS_GetDGprsActiveTypeCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_D_GPRS_SET) {
        return VOS_ERR;
    }

    /* 转换错误码格式 */
    if (getGprsActiveTypeCnf->cause == TAF_PARA_OK) {
        result = At_SetDialGprsPara(indexNum, getGprsActiveTypeCnf->cidGprsActiveType.cid,
                                    getGprsActiveTypeCnf->cidGprsActiveType.activeType);
    } else {
        result = AT_ERROR;
    }

    if (result != AT_WAIT_ASYNC_RETURN) {
        if (result == AT_ERROR) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
        }

        AT_StopRelTimer(indexNum, &g_atClientTab[indexNum].hTimer);
        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;
        g_atClientTab[indexNum].opId          = 0;

        At_FormatResultData(indexNum, result);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtPppDialOrigCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_PppDialOrigCnf *pppDialOrigCnf = VOS_NULL_PTR;

    /* 初始化 */
    pppDialOrigCnf = (TAF_PS_PppDialOrigCnf *)evtInfo;

    AT_ProcPppDialCnf(pppDialOrigCnf->cause, indexNum);

    return VOS_OK;
}

/* 格式化输出Residual bit error ratio */
VOS_VOID AT_FormatTafPsEvtGetDynamicUmtsQosInfoResidualBer(VOS_UINT8 residualBer, VOS_UINT16 *length)
{
    switch (residualBer) {
        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_0:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_0E0].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_1:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_5E2].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_2:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E2].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_3:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_5E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_4:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_4E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_5:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E3].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_6:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E4].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_7:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E5].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_8:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E6].text);
            break;

        case AT_RESIDUAL_BIT_ERR_RATIO_STATUS_9:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_6E8].text);
            break;

        default:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
            break;
    }
}

/* 格式化输出SDU error ratio */
VOS_VOID AT_FormatTafPsEvtGetDynamicUmtsQosInfoSduErrRatio(VOS_UINT8 sduErrRatio, VOS_UINT16 *length)
{
    switch (sduErrRatio) {
        case AT_SDU_ERR_RATIO_STATUS_0:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_0E0].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_1:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E2].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_2:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_7E3].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_3:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E3].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_4:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E4].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_5:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E5].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_6:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E6].text);
            break;

        case AT_SDU_ERR_RATIO_STATUS_7:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%s", g_atStringTab[AT_STRING_1E1].text);
            break;

        default:
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
            break;
    }
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamicUmtsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t                          memResult;
    VOS_UINT32                       result;
    VOS_UINT16                       length            = 0;
    VOS_UINT32                       tmp               = 0;
    TAF_PS_GetDynamicUmtsQosInfoCnf *dynUmtsQosMinInfo = VOS_NULL_PTR;
    TAF_UMTS_QosQueryInfo            cgeq;

    (VOS_VOID)memset_s(&cgeq, sizeof(cgeq), 0x00, sizeof(TAF_UMTS_QosQueryInfo));

    dynUmtsQosMinInfo = (TAF_PS_GetDynamicUmtsQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQNEG_SET) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < dynUmtsQosMinInfo->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cgeq, sizeof(cgeq), &dynUmtsQosMinInfo->umtsQosQueryInfo[tmp],
                             sizeof(TAF_UMTS_QosQueryInfo));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgeq), sizeof(TAF_UMTS_QosQueryInfo));
        /* +CGEQREQ:+CGEQMIN   */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", cgeq.cid);
        /* <Traffic class> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.trafficClass);
        /* <Maximum bitrate UL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxBitUl);
        /* <Maximum bitrate DL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxBitDl);
        /* <Guaranteed bitrate UL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.guarantBitUl);
        /* <Guaranteed bitrate DL> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.guarantBitDl);
        /* <Delivery order> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.deliverOrder);
        /* <Maximum SDU size> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.maxSduSize);
        /* <SDU error ratio> */
        AT_FormatTafPsEvtGetDynamicUmtsQosInfoSduErrRatio(cgeq.qosInfo.sduErrRatio, &length);

        /* <Residual bit error ratio> */
        AT_FormatTafPsEvtGetDynamicUmtsQosInfoResidualBer(cgeq.qosInfo.residualBer, &length);

        /* <Delivery of erroneous SDUs> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.deliverErrSdu);
        /* <Transfer delay> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.transDelay);
        /* <Traffic handling priority> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeq.qosInfo.traffHandlePrior);
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetCidSdfInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    return VOS_OK;
}

VOS_UINT32 At_RcvTafPsEvtSetDialModeCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT32               result = AT_OK;
    TAF_PS_CdataDialModeCnf *dialMode = VOS_NULL_PTR;

    /* 初始化 */
    dialMode = (TAF_PS_CdataDialModeCnf *)evtInfo;

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CRM_SET) {
        AT_WARN_LOG("At_RcvTafPsEvtSetDialModeCnf : Current Option is not AT_CMD_CRM_SET.");
        return VOS_OK;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (dialMode->cause != TAF_PS_CAUSE_SUCCESS) {
        result = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_VOID AT_RcvNdisPdnInfoRelCnf(AT_NDIS_PdnInfoRelCnf *ndisPdnInfoRelCnf)
{
    if (ndisPdnInfoRelCnf->result != AT_NDIS_SUCC) {
        AT_ERR_LOG("AT_RcvNdisPdnInfoRelCnf: Failed.");
    }
}

VOS_UINT32 AT_ProcRabmQryFastDormParaCnf(RABM_AT_QryFastdormParaCnf *msg)
{
    VOS_UINT8  indexNum = 0;
    VOS_UINT32 result = AT_ERROR;
    VOS_UINT16 length;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_RcvAtCcMsgStateQryCnfProc:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (msg->rslt == VOS_TRUE) {
        /* 输出查询结果 */
        length = 0;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "^FASTDORM:%d", msg->fastDormPara.fastDormOperationType);

        if (msg->fastDormPara.fastDormOperationType != AT_RABM_FASTDORM_STOP_FD_ASCR) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", msg->fastDormPara.timeLen);
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        result                  = AT_OK;
        g_atSendDataBuff.bufLen = length;
    }

    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#if (FEATURE_HUAWEI_VP == FEATURE_ON)
VOS_UINT32 AT_ProcRabmVoicePreferStatusReport(RABM_AT_VoicepreferStatusReport *msg)
{
    VOS_UINT8  indexNum = 0;
    VOS_UINT16 length = 0;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_ProcRabmVoicePreferStatusReport:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 输出上报结果 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^VOICEPREFERSTATUS:%d%s", g_atCrLf, msg->vpStatus, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif



