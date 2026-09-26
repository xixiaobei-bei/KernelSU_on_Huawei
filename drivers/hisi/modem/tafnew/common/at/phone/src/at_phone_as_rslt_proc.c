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
#include "at_phone_as_rslt_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_phone_comm.h"
#include "gen_msg.h"
#include "at_lte_common.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_AS_RSLT_PROC_C
#define AT_MNC_MIN_NUM 2

#if (FEATURE_PROBE_FREQLOCK == FEATURE_ON)
static const AT_FreqlockModeTbl g_freqLockModeTbl[] = {
    { AT_FREQLOCK_MODE_TYPE_GSM, 0, "01" },
    { AT_FREQLOCK_MODE_TYPE_WCDMA, 0, "02" },
    { AT_FREQLOCK_MODE_TYPE_TDSCDMA, 0, "03" },
    { AT_FREQLOCK_MODE_TYPE_LTE, 0, "04" },
};

static const AT_FreqlockGsmBandTbl g_freqLockGsmBandTbl[] = {
    { AT_FREQLOCK_BAND_TYPE_GSM_850, 0, "00" },
    { AT_FREQLOCK_BAND_TYPE_GSM_900, 0, "01" },
    { AT_FREQLOCK_BAND_TYPE_GSM_1800, 0, "02" },
    { AT_FREQLOCK_BAND_TYPE_GSM_1900, 0, "03" },
};
#endif /* FEATURE_PROBE_FREQLOCK == FEATURE_ON */

VOS_UINT32 AT_RcvMtaCsnrQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    MTA_AT_CsnrQryCnf *pstrCsnrQryCnf = VOS_NULL_PTR;
    AT_MTA_Msg        *rcvMsg         = VOS_NULL_PTR;
    VOS_UINT32         result;

    /* 初始化 */
    rcvMsg         = (AT_MTA_Msg *)msg;
    pstrCsnrQryCnf = (MTA_AT_CsnrQryCnf *)rcvMsg->content;
    result         = AT_OK;

    g_atSendDataBuff.bufLen = 0;

    if (pstrCsnrQryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ERROR;
    } else {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            (TAF_INT32)pstrCsnrQryCnf->csnrPara.cpichRscp, (TAF_INT32)pstrCsnrQryCnf->csnrPara.cpichEcNo);
    }

    /* 回复用户命令结果 */
    return result;
}

VOS_UINT32 AT_RcvMtaWrrFreqLockQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_WrrFreqlockQryCnf *wrrFreqlockCnf = (MTA_AT_WrrFreqlockQryCnf *)mtaMsg->content;
    VOS_UINT32 result = AT_OK;

    /* 格式化AT^FREQLOCK查询命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (wrrFreqlockCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        if (wrrFreqlockCnf->freqLockInfo.freqLockEnable == VOS_FALSE) {
            g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                (TAF_INT32)wrrFreqlockCnf->freqLockInfo.freqLockEnable);
        } else {
            g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                (TAF_INT32)wrrFreqlockCnf->freqLockInfo.freqLockEnable,
                (TAF_INT32)wrrFreqlockCnf->freqLockInfo.lockedFreq);
        }
    }

    return result;
}

VOS_UINT32 AT_RcvMtaGFreqLockQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_QryGsmFreqlockCnf *qryGFreqlockCnf = (MTA_AT_QryGsmFreqlockCnf *)mtaMsg->content;
    VOS_UINT32                result;

    /* 格式化AT^GFREQLOCK查询命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (qryGFreqlockCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        if (qryGFreqlockCnf->lockFlg == VOS_FALSE) {
            g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                (TAF_INT32)qryGFreqlockCnf->lockFlg);
        } else {
            g_atSendDataBuff.bufLen =
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                    (TAF_INT32)qryGFreqlockCnf->lockFlg, (TAF_INT32)qryGFreqlockCnf->freq,
                    (TAF_INT32)qryGFreqlockCnf->band);
        }
    }

    return result;
}

VOS_UINT32 AT_RcvMtaWrrCellinfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *mtaMsg = VOS_NULL_PTR;
    MTA_AT_WrrCellinfoQryCnf *wrrCellinfoQryCnf = VOS_NULL_PTR;
    VOS_UINT32 result;
    VOS_UINT32 cellNum = 0;
    VOS_UINT32 i;

    mtaMsg = (AT_MTA_Msg *)msg;
    wrrCellinfoQryCnf = (MTA_AT_WrrCellinfoQryCnf *)mtaMsg->content;

    /* 格式化AT^CELLINFO命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (wrrCellinfoQryCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        /* 没有获取的小区信息，打印0 */
        if (wrrCellinfoQryCnf->wrrCellInfo.cellNum == 0) {
            g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen,
                "no cellinfo rslt");
        }

        cellNum = AT_MIN(wrrCellinfoQryCnf->wrrCellInfo.cellNum, MTA_AT_WRR_MAX_NCELL_NUM);

        for (i = 0; i < cellNum; i++) {
            g_atSendDataBuff.bufLen +=
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%d,%d,%d,%d\r\n",
                    wrrCellinfoQryCnf->wrrCellInfo.wCellInfo[i].cellFreq,
                    wrrCellinfoQryCnf->wrrCellInfo.wCellInfo[i].primaryScramCode,
                    wrrCellinfoQryCnf->wrrCellInfo.wCellInfo[i].cpichRscp,
                    wrrCellinfoQryCnf->wrrCellInfo.wCellInfo[i].cpichEcN0);
        }
    }

    return result;
}

VOS_UINT32 AT_RcvMtaWrrCellSrhQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg              *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_WrrCellsrhQryCnf *wrrCellSrhQryCnf = (MTA_AT_WrrCellsrhQryCnf *)mtaMsg->content;
    VOS_UINT32               result = AT_OK;

    /* 格式化AT^FREQLOCK查询命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (wrrCellSrhQryCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            (TAF_INT32)wrrCellSrhQryCnf->cellSearchType);
    }

    return result;
}

#if (FEATURE_PROBE_FREQLOCK == FEATURE_ON)
VOS_VOID AT_PrintGsmFreqlockInfo(MTA_AT_QryM2MFreqlockCnf *msg, VOS_UINT8 indexNum, VOS_UINT16 *length)
{
    if (msg->freqState[0] == AT_MTA_M2M_FREQLOCK_FLAG_TYPE_OFF) {
        /* <enable>,<mode> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\"",
            msg->freqState[0],
            g_freqLockModeTbl[0].strMode);
    } else {
        /* <enable>,<freq>,<mode>,<band>,, */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\",%d,\"%s\",,",
            msg->freqState[0],
            g_freqLockModeTbl[0].strMode, msg->gFreqLockInfo.freq,
            g_freqLockGsmBandTbl[msg->gFreqLockInfo.band].strGsmBand);
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}

VOS_VOID AT_PrintWcdmaFreqlockInfo(MTA_AT_QryM2MFreqlockCnf *msg, VOS_UINT8 indexNum, VOS_UINT16 *length)
{
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    if (msg->freqState[AT_FREQLOCK_MODE_TYPE_WCDMA - 1] == AT_MTA_M2M_FREQLOCK_FLAG_TYPE_OFF) {
        /* <enable>,<mode> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\"",
            msg->freqState[AT_FREQLOCK_MODE_TYPE_WCDMA - 1],
            g_freqLockModeTbl[AT_FREQLOCK_MODE_TYPE_WCDMA - 1].strMode);
    } else {
        /* 判断WCDMA锁频的类型:区分锁频、锁频+扰码组合 */
        if (msg->wFreqLockInfo.freqType == AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_ONLY) {
            /* <enable>,<mode>,<freq>,,, */
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\",%d,,,",
                msg->freqState[AT_FREQLOCK_MODE_TYPE_WCDMA - 1],
                g_freqLockModeTbl[AT_FREQLOCK_MODE_TYPE_WCDMA - 1].strMode, msg->wFreqLockInfo.freq);
        } else {
            /* <enable>,<mode>,<freq>,,<psc>, */
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\",%d,,%d,",
                msg->freqState[AT_FREQLOCK_MODE_TYPE_WCDMA - 1],
                g_freqLockModeTbl[AT_FREQLOCK_MODE_TYPE_WCDMA - 1].strMode, msg->wFreqLockInfo.freq,
                msg->wFreqLockInfo.psc);
        }
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}
VOS_VOID AT_PrintTdsCdmaFreqlockInfo(MTA_AT_QryM2MFreqlockCnf *msg, VOS_UINT8 indexNum, VOS_UINT16 *length)
{
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    if (msg->freqState[AT_FREQLOCK_MODE_TYPE_TDSCDMA - 1] == AT_MTA_M2M_FREQLOCK_FLAG_TYPE_OFF) {
        /* <enable>,<mode> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\"",
            msg->freqState[AT_FREQLOCK_MODE_TYPE_TDSCDMA - 1],
            g_freqLockModeTbl[AT_FREQLOCK_MODE_TYPE_TDSCDMA - 1].strMode);
    } else {
        /* 判断WCDMA锁频的类型:区分锁频、锁频+扰码组合 */
        if (msg->tFreqLockInfo.freqType == AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_ONLY) {
            /* <enable>,<mode>,<freq>,,, */
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\",%d,,,",
                msg->freqState[AT_FREQLOCK_MODE_TYPE_TDSCDMA - 1],
                g_freqLockModeTbl[AT_FREQLOCK_MODE_TYPE_TDSCDMA - 1].strMode, msg->tFreqLockInfo.freq);
        } else {
            /* <enable>,<mode>,<freq>,,<psc>, */
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\",%d,,%d,",
                msg->freqState[AT_FREQLOCK_MODE_TYPE_TDSCDMA - 1],
                g_freqLockModeTbl[AT_FREQLOCK_MODE_TYPE_TDSCDMA - 1].strMode, msg->tFreqLockInfo.freq,
                msg->tFreqLockInfo.sc);
        }
    }

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
}

VOS_VOID AT_PrintLteFreqlockInfo(MTA_AT_QryM2MFreqlockCnf *msg, VOS_UINT8 indexNum, VOS_UINT16 *length)
{
    /* 输出LTE制式锁频状态信息 */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    if (msg->freqState[AT_MTA_M2M_FREQLOCK_MODE_LTE - 1] == AT_MTA_M2M_FREQLOCK_FLAG_TYPE_OFF) {
        /* <enable>,<mode> */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\"",
            msg->freqState[AT_MTA_M2M_FREQLOCK_MODE_LTE - 1],
            g_freqLockModeTbl[AT_MTA_M2M_FREQLOCK_MODE_LTE - 1].strMode);
    } else {
        /* 判断LTE锁频的类型:区分锁频、锁频+扰码组合 */
        if (msg->lFreqLockInfo.freqType == AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_ONLY) {
            /* <enable>,<mode>,<freq>,,, */
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\",%d,,,",
                msg->freqState[AT_MTA_M2M_FREQLOCK_MODE_LTE - 1],
                g_freqLockModeTbl[AT_MTA_M2M_FREQLOCK_MODE_LTE - 1].strMode, msg->lFreqLockInfo.freq);
        } else {
            /* <enable>,<mode>,<freq>,,,<pci> */
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,\"%s\",%d,,,%d",
                msg->freqState[AT_FREQLOCK_MODE_TYPE_LTE - 1],
                g_freqLockModeTbl[AT_FREQLOCK_MODE_TYPE_LTE - 1].strMode, msg->lFreqLockInfo.freq,
                msg->lFreqLockInfo.pci);
        }
    }
}

VOS_UINT32 AT_RcvMtaQryM2MFreqLockCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_QryM2MFreqlockCnf *qryFreqLockCnf = (MTA_AT_QryM2MFreqlockCnf *)mtaMsg->content;
    VOS_UINT32 result = AT_OK;
    VOS_UINT16 length = 0;

    /* 锁频查询结果按G-W-T-L顺序依次输出  */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 输出GSM制式锁频状态信息 */
    AT_PrintGsmFreqlockInfo(qryFreqLockCnf, indexNum, &length);

    /* 输出WCDMA制式锁频状态信息 */
    AT_PrintWcdmaFreqlockInfo(qryFreqLockCnf, indexNum, &length);

    /* 输出TDS-CDMA制式锁频状态信息 */
    AT_PrintTdsCdmaFreqlockInfo(qryFreqLockCnf, indexNum, &length);

    /* 输出LTE制式锁频状态信息 */
    AT_PrintLteFreqlockInfo(qryFreqLockCnf, indexNum, &length);

    g_atSendDataBuff.bufLen = length;

    return result;
}
#endif

VOS_UINT32 atQryCellIdCnfProc(struct MsgCB *msgBlock)
{
    L4A_READ_CellIdCnf *cnf = NULL;
    VOS_UINT16          length;

    cnf = (L4A_READ_CellIdCnf *)msgBlock;

    /* MNC：用于识别移动用户所归属的移动通信网，2~3位数字组成 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr,
        ((cnf->mncNum == AT_MNC_MIN_NUM) ? "%s^CECELLID: %03x%02x,%d,%d,%d%s" : "%s^CECELLID: %03x%03x,%d,%d,%d%s"),
        g_atCrLf, cnf->mcc, (cnf->mncNum == AT_MNC_MIN_NUM) ? (cnf->mnc & 0xff) : cnf->mnc, cnf->ci, cnf->pci,
        cnf->tac, g_atCrLf);

    CmdErrProc((VOS_UINT8)(cnf->clientId), ERR_MSP_SUCCESS, length, g_atSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 AT_RcvMtaTransModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg               = VOS_NULL_PTR;
    MTA_AT_TransmodeQryCnf *mtaAtQryTransModeCnf = VOS_NULL_PTR;
    VOS_UINT32              result;

    /* 初始化 */
    rcvMsg               = (AT_MTA_Msg *)msg;
    mtaAtQryTransModeCnf = (MTA_AT_TransmodeQryCnf *)rcvMsg->content;
    result               = AT_OK;

    g_atSendDataBuff.bufLen = 0;
    if (mtaAtQryTransModeCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            mtaAtQryTransModeCnf->transMode);
    }

    return result;
}

