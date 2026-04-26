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

#include "at_test_as_rslt_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_external_module_msg_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_AS_RSLT_PROC_C

#define AT_CGAS_PARA_GAS_STATUS 0 /* CGAS的第一个参数GAS_STATUS */
#define AT_AUTOTEST_VALID_RSULT_NUM 3
#define AT_RSLT_BASE_NUM 3

VOS_UINT32 AT_RcvMtaWrrAutotestQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                 *mtaMsg = VOS_NULL_PTR;
    MTA_AT_WrrAutotestQryCnf   *wrrAutotestCnf = VOS_NULL_PTR;
    VOS_UINT32 result = AT_OK;
    VOS_UINT32 rsltNum = 0;
    VOS_UINT32 i;

    mtaMsg = (AT_MTA_Msg *)msg;
    wrrAutotestCnf = (MTA_AT_WrrAutotestQryCnf *)mtaMsg->content;

    /* 格式化AT^CWAS命令返回 */
    if (wrrAutotestCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        if (wrrAutotestCnf->wrrAutoTestRslt.rsltNum > 0) {
            g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%d",
                wrrAutotestCnf->wrrAutoTestRslt.rslt[0]);

            rsltNum = AT_MIN(wrrAutotestCnf->wrrAutoTestRslt.rsltNum, (MTA_AT_WRR_AUTOTEST_MAX_RSULT_NUM - 1));
            for (i = 1; i < rsltNum; i++) {
                g_atSendDataBuff.bufLen +=
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, ",%d",
                        wrrAutotestCnf->wrrAutoTestRslt.rslt[i]);
            }
        }
    }

    return result;
}

VOS_VOID AT_FormatGasAtCmdRslt(MTA_AT_GasAutotestQryRslt *atCmdRslt)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 j = 0;
    VOS_UINT16 length = 0;

    for (i = 0; i < (atCmdRslt->rsltNum / AT_RSLT_BASE_NUM); i++) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d:%d,", (i + 1), atCmdRslt->rslt[j]);
        j++;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "0x%X,", atCmdRslt->rslt[j] & 0xff);
        j++;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d\r\n", atCmdRslt->rslt[j]);
        j++;
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_VOID AT_RcvMtaGrrAutotestQryProc(MTA_AT_GasAutotestQryCnf *mtaAutotestCnf, VOS_UINT32 *ulRslt)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 rsltTemp = AT_OK;
    VOS_UINT32 i = 0;

    /* 邻区状态 */
    if (g_atParaList[AT_CGAS_PARA_GAS_STATUS].paraValue == GAS_AT_CMD_NCELL) {
        /* 邻区状态下，获取的测试结果个数应该是3的整数倍 */
        if ((mtaAutotestCnf->grrAutoTestRslt.rsltNum > 0) && ((mtaAutotestCnf->grrAutoTestRslt.rsltNum % 3) == 0) &&
            (mtaAutotestCnf->grrAutoTestRslt.rsltNum <= MTA_AT_GAS_AUTOTEST_MAX_RSULT_NUM)) {
            AT_FormatGasAtCmdRslt(&(mtaAutotestCnf->grrAutoTestRslt));
        } else if (mtaAutotestCnf->grrAutoTestRslt.rsltNum == 0) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "0");
            g_atSendDataBuff.bufLen = length;
        } else {
            rsltTemp = AT_ERROR;
        }
    } else if (g_atParaList[AT_CGAS_PARA_GAS_STATUS].paraValue == GAS_AT_CMD_SCELL) {
        /* 服务小区状态 */
        if (mtaAutotestCnf->grrAutoTestRslt.rsltNum != AT_AUTOTEST_VALID_RSULT_NUM) {
            rsltTemp = AT_ERROR;
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d,", mtaAutotestCnf->grrAutoTestRslt.rslt[0]);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "0x%X,", mtaAutotestCnf->grrAutoTestRslt.rslt[1] & 0xff);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d", mtaAutotestCnf->grrAutoTestRslt.rslt[2]);
            g_atSendDataBuff.bufLen = length;
            rsltTemp                = AT_OK;
        }
    } else if (mtaAutotestCnf->grrAutoTestRslt.rsltNum > 0) {
        /* 其它命令 */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d", mtaAutotestCnf->grrAutoTestRslt.rslt[0]);

        mtaAutotestCnf->grrAutoTestRslt.rsltNum = AT_MIN(mtaAutotestCnf->grrAutoTestRslt.rsltNum,
                                                         MTA_AT_GAS_AUTOTEST_MAX_RSULT_NUM);
        for (i = 1; i < mtaAutotestCnf->grrAutoTestRslt.rsltNum; i++) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", mtaAutotestCnf->grrAutoTestRslt.rslt[i]);
        }
        g_atSendDataBuff.bufLen = length;
    } else {
    }

    *ulRslt = rsltTemp;
}

VOS_UINT32 AT_RcvMtaGrrAutotestQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_GasAutotestQryCnf *grrAutotestCnf = (MTA_AT_GasAutotestQryCnf *)mtaMsg->content;
    VOS_UINT32                result = AT_OK;

    /* 格式化AT^CGAS命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (grrAutotestCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;
        AT_RcvMtaGrrAutotestQryProc(grrAutotestCnf, &result);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaWrrMeanrptQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_WrrMeanrptQryCnf *wrrMeanrptQryCnf = (MTA_AT_WrrMeanrptQryCnf *)mtaMsg->content;
    VOS_UINT32 result = AT_OK;
    VOS_UINT16 length = 0;
    VOS_UINT32 i;
    VOS_UINT32 cellNumLoop;
    VOS_UINT32 rptNum = 0;
    VOS_UINT16 cellNum = 0;

    /* 格式化AT^MEANRPT命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (wrrMeanrptQryCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d", wrrMeanrptQryCnf->meanRptRslt.rptNum);

        rptNum = AT_MIN(wrrMeanrptQryCnf->meanRptRslt.rptNum, MTA_AT_WRR_MAX_MEANRPT_NUM);

        for (i = 0; i < rptNum; i++) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\r\n0x%X,%d",
                wrrMeanrptQryCnf->meanRptRslt.meanRptInfo[i].eventId,
                wrrMeanrptQryCnf->meanRptRslt.meanRptInfo[i].cellNum);

            cellNum = AT_MIN(wrrMeanrptQryCnf->meanRptRslt.meanRptInfo[i].cellNum, MTA_AT_WRR_ONE_MEANRPT_MAX_CELL_NUM);

            for (cellNumLoop = 0; cellNumLoop < cellNum; cellNumLoop++) {
                length +=
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d",
                        wrrMeanrptQryCnf->meanRptRslt.meanRptInfo[i].primaryScramCode[cellNumLoop]);
            }
        }
        g_atSendDataBuff.bufLen = length;
    }

    return result;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_RcvMtaNrFreqLockQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                *rcvMsg          = VOS_NULL_PTR;
    MTA_AT_NrrcFreqlockQryCnf *nrrcFreqlockCnf = VOS_NULL_PTR;
    VOS_UINT32                 result;

    /* 初始化 */
    rcvMsg          = (AT_MTA_Msg *)msg;
    nrrcFreqlockCnf = (MTA_AT_NrrcFreqlockQryCnf *)rcvMsg->content;
    result          = AT_OK;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^NRFREQLOCK查询命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (nrrcFreqlockCnf->result != VOS_OK) {
        result = AT_ConvertMtaResult(nrrcFreqlockCnf->result);
    } else {
        result = AT_OK;

        switch (nrrcFreqlockCnf->freqType) {
            case MTA_AT_FREQLOCK_TYPE_LOCK_NONE:
                g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                    (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress, "%s: %d",
                    g_parseContext[indexNum].cmdElement->cmdName, nrrcFreqlockCnf->freqType);
                break;

            case MTA_AT_FREQLOCK_TYPE_LOCK_FREQ:
                g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                    (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%u",
                    g_parseContext[indexNum].cmdElement->cmdName, nrrcFreqlockCnf->freqType, nrrcFreqlockCnf->scsType,
                    nrrcFreqlockCnf->band, nrrcFreqlockCnf->nrArfcn);
                break;

            case MTA_AT_FREQLOCK_TYPE_LOCK_CELL:
                g_atSendDataBuff.bufLen =
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%u,%d",
                        g_parseContext[indexNum].cmdElement->cmdName, nrrcFreqlockCnf->freqType,
                        nrrcFreqlockCnf->scsType, nrrcFreqlockCnf->band, nrrcFreqlockCnf->nrArfcn,
                        nrrcFreqlockCnf->phyCellId);
                break;

            case MTA_AT_FREQLOCK_TYPE_LOCK_BAND:
                g_atSendDataBuff.bufLen =
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                        nrrcFreqlockCnf->freqType, nrrcFreqlockCnf->scsType, nrrcFreqlockCnf->band);
                break;

            default:
                AT_WARN_LOG("AT_RcvMtaNrFreqLockQryCnf : Current req type is out of bounds.");
                return AT_CMD_NO_NEED_FORMAT_RSLT;
        }
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);
    return AT_CMD_NO_NEED_FORMAT_RSLT;
}
#endif
#endif

