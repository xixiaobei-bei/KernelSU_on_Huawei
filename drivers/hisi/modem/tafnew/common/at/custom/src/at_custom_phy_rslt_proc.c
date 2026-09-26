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
#include "at_custom_phy_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_PHY_RSLT_PROC_C

#define AT_AFCCLKINFO_PARA_A0_MANTISSA 0
#define AT_AFCCLKINFO_PARA_A0_EXPONENT 1
#define AT_AFCCLKINFO_PARA_A1_MANTISSA 2
#define AT_AFCCLKINFO_PARA_A1_EXPONENT 3
#define AT_AFCCLKINFO_PARA_A2_MANTISSA 0
#define AT_AFCCLKINFO_PARA_A2_EXPONENT 1
#define AT_AFCCLKINFO_PARA_A3_MANTISSA 2
#define AT_AFCCLKINFO_PARA_A3_EXPONENT 3

#define MCS_RAT_LTE 0
#define MCS_RAT_NR 1

VOS_UINT32 AT_RcvMtaAfcClkInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                    *mtaMsg = VOS_NULL_PTR;
    MTA_AT_QryAfcClkFreqXocoefCnf *afcCnf = VOS_NULL_PTR;
    VOS_UINT32                     ret;

    mtaMsg = (AT_MTA_Msg *)msg;
    afcCnf = (MTA_AT_QryAfcClkFreqXocoefCnf *)mtaMsg->content;

    /* 初始化消息变量 */
    ret      = AT_OK;

    g_atSendDataBuff.bufLen = 0;

    if (afcCnf->result != MTA_AT_RESULT_NO_ERROR) {
        ret = AT_ERROR;
    } else {
        /*
         * ^AFCCLKINFO: <status>,<deviation>,<sTemp>,<eTemp>,<a0_m>,<a0_e>,
         * <a1_m>,<a1_e>,<a2_m>,<a2_e>,<a3_m>,<a3_e>,
         * <rat>,<ModemId>
         */
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %u,%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, afcCnf->status, afcCnf->deviation,
                afcCnf->coeffStartTemp, afcCnf->coeffEndTemp, afcCnf->coeffMantissa[AT_AFCCLKINFO_PARA_A0_MANTISSA],
                afcCnf->coeffMantissa[AT_AFCCLKINFO_PARA_A0_EXPONENT],
                afcCnf->coeffMantissa[AT_AFCCLKINFO_PARA_A1_MANTISSA],
                afcCnf->coeffMantissa[AT_AFCCLKINFO_PARA_A1_EXPONENT],
                afcCnf->coeffExponent[AT_AFCCLKINFO_PARA_A2_MANTISSA],
                afcCnf->coeffExponent[AT_AFCCLKINFO_PARA_A2_EXPONENT],
                afcCnf->coeffExponent[AT_AFCCLKINFO_PARA_A3_MANTISSA],
                afcCnf->coeffExponent[AT_AFCCLKINFO_PARA_A3_EXPONENT], afcCnf->ratMode, afcCnf->modemId);
    }

    return ret;
}

VOS_UINT32 AT_RcvMtaBodySarSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg       *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_ResultCnf *bodySarSetCnf = (MTA_AT_ResultCnf *)mtaMsg->content;
    VOS_UINT32        result;

    /* 判断回复消息中的错误码 */
    if (bodySarSetCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result = AT_OK;

        /* 更新BODYSAR 状态 */
        if (g_atBodySarState == AT_MTA_BODY_SAR_OFF) {
            g_atBodySarState = AT_MTA_BODY_SAR_ON;
        } else {
            g_atBodySarState = AT_MTA_BODY_SAR_OFF;
        }
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;

    return result;
}

VOS_UINT32 AT_RcvMtaRefclkfreqQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg              *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_RefclkfreqQryCnf *refclkfreqCnf = (MTA_AT_RefclkfreqQryCnf *)mtaMsg->content;
    VOS_UINT32               result;

    /* 判断查询操作是否成功 */
    if (refclkfreqCnf->result == MTA_AT_RESULT_NO_ERROR) {
        /* 输出GPS参考时钟信息，命令版本号默认为0 */
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: 0,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                refclkfreqCnf->freq, refclkfreqCnf->precision, refclkfreqCnf->status);
        result = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        result = AT_ERROR;
    }

    return result;
}

VOS_UINT32 AT_RcvMtaHandleDectQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_HandledectQryCnf *qryCnf = (MTA_AT_HandledectQryCnf *)mtaMsg->content;
    VOS_UINT32 result;

    /* 格式化AT^HANDLEDECT?查询命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (qryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                (VOS_UINT16)qryCnf->handle);
    }

    return result;
}

LOCAL VOS_VOID AT_ProcUlMcsRsp(MTA_AT_McsQryCnf *mtaAtQryMcsCnf, VOS_UINT8 indexNum, VOS_UINT16 *length)
{
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32 loop;
#endif

    if (mtaAtQryMcsCnf->resultType != MTA_AT_RESULT_TYPE_NR) {
        /* LTE */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, AT_MCS_DIRECTION_UL, MCS_RAT_LTE, MTA_AT_INDEX_TABLE_ONE,
            mtaAtQryMcsCnf->lteMcsInfo.ulMcs[0], mtaAtQryMcsCnf->lteMcsInfo.ulMcs[1]);
    }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (mtaAtQryMcsCnf->resultType == MTA_AT_RESULT_TYPE_DC) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
    }

    if (mtaAtQryMcsCnf->resultType != MTA_AT_RESULT_TYPE_LTE) {
        /* NR */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            AT_MCS_DIRECTION_UL, MCS_RAT_NR);
        for (loop = 0; loop < (VOS_UINT32)AT_MIN(mtaAtQryMcsCnf->nrCellNum, MTA_AT_MAX_CC_NUMBER); loop++) {
            /* NR上行暂不支持并发 */
            if (mtaAtQryMcsCnf->nrMcsInfo[loop].ulMode == MTA_AT_UL_MODE_UL_ONLY) {
                *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d,%d,%d", mtaAtQryMcsCnf->nrMcsInfo[loop].ulMcsTable,
                    mtaAtQryMcsCnf->nrMcsInfo[loop].ulMcs[0], mtaAtQryMcsCnf->nrMcsInfo[loop].ulMcs[1]);
            } else if (mtaAtQryMcsCnf->nrMcsInfo[loop].ulMode == MTA_AT_UL_MODE_SUL_ONLY) {
                *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d,%d,%d", mtaAtQryMcsCnf->nrMcsInfo[loop].sulMcsTable,
                    mtaAtQryMcsCnf->nrMcsInfo[loop].sulMcs[0], mtaAtQryMcsCnf->nrMcsInfo[loop].sulMcs[1]);
            } else {
                *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d,%d,%d", MTA_AT_INDEX_TABLE_BUTT,
                    MTA_AT_INVALID_MCS_VALUE, MTA_AT_INVALID_MCS_VALUE);
            }
        }
    }
#endif
}

LOCAL VOS_VOID AT_ProcDlMcsRsp(MTA_AT_McsQryCnf *mtaAtQryMcsCnf, VOS_UINT8 indexNum, VOS_UINT16 *length)
{
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    VOS_UINT32 loop;
#endif

    if (mtaAtQryMcsCnf->resultType != MTA_AT_RESULT_TYPE_NR) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, AT_MCS_DIRECTION_DL, MCS_RAT_LTE, MTA_AT_INDEX_TABLE_ONE,
            mtaAtQryMcsCnf->lteMcsInfo.dlMcs[0], mtaAtQryMcsCnf->lteMcsInfo.dlMcs[1]);
    }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (mtaAtQryMcsCnf->resultType == MTA_AT_RESULT_TYPE_DC) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
    }

    if (mtaAtQryMcsCnf->resultType != MTA_AT_RESULT_TYPE_LTE) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            AT_MCS_DIRECTION_DL, MCS_RAT_NR);
        for (loop = 0; loop < (VOS_UINT32)AT_MIN(mtaAtQryMcsCnf->nrCellNum, MTA_AT_MAX_CC_NUMBER); loop++) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d,%d,%d", mtaAtQryMcsCnf->nrMcsInfo[loop].dlMcsTable,
                mtaAtQryMcsCnf->nrMcsInfo[loop].dlMcs[0], mtaAtQryMcsCnf->nrMcsInfo[loop].dlMcs[1]);
        }
    }
#endif
}

VOS_UINT32 AT_RcvMtaMcsSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg       *rcvMsg         = VOS_NULL_PTR;
    MTA_AT_McsQryCnf *mtaAtQryMcsCnf = VOS_NULL_PTR;
    AT_ModemNetCtx   *netCtx         = VOS_NULL_PTR;
    VOS_UINT32        result;
    VOS_UINT16        length;

    rcvMsg         = (AT_MTA_Msg *)msg;
    mtaAtQryMcsCnf = (MTA_AT_McsQryCnf *)rcvMsg->content;
    length = 0;
    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);
    result = AT_ConvertMtaResult(mtaAtQryMcsCnf->result);

    if (mtaAtQryMcsCnf->result == MTA_AT_RESULT_NO_ERROR) {
        if (netCtx->mcsDirection == AT_MCS_DIRECTION_UL) {
            AT_ProcUlMcsRsp(mtaAtQryMcsCnf, indexNum, &length);
        } else {
            AT_ProcDlMcsRsp(mtaAtQryMcsCnf, indexNum, &length);
        }
    }

    g_atSendDataBuff.bufLen = length;

    return result;
}

VOS_UINT32 AT_RcvMtaTxPowerQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *rcvMsg      = VOS_NULL_PTR;
    MTA_AT_TxpowerQryCnf *txpwrQryCnf = VOS_NULL_PTR;
    VOS_UINT32            result;
    VOS_UINT16            length;

    rcvMsg      = (AT_MTA_Msg *)msg;
    txpwrQryCnf = (MTA_AT_TxpowerQryCnf *)rcvMsg->content;
    length      = 0;
    result      = AT_ConvertMtaResult(txpwrQryCnf->result);

    if (txpwrQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, txpwrQryCnf->txpwrInfo.guTxPwr,
            txpwrQryCnf->txpwrInfo.puschPwr, txpwrQryCnf->txpwrInfo.pucchPwr, txpwrQryCnf->txpwrInfo.srsPwr,
            txpwrQryCnf->txpwrInfo.prachPwr);
    }

    g_atSendDataBuff.bufLen = length;

    return result;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNtxPowerQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg      = VOS_NULL_PTR;
    MTA_AT_NtxpowerQryCnf *txpwrQryCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT32             loop;
    VOS_UINT16             length;

    rcvMsg      = (AT_MTA_Msg *)msg;
    txpwrQryCnf = (MTA_AT_NtxpowerQryCnf *)rcvMsg->content;
    length      = 0;
    result      = AT_ConvertMtaResult(txpwrQryCnf->result);

    if (txpwrQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        for (loop = 0; loop < (VOS_UINT32)TAF_MIN(txpwrQryCnf->nrCellNum, MTA_AT_MAX_CC_NUMBER); loop++) {
            if (txpwrQryCnf->nrTxPwrInfo[loop].ulMode == MTA_AT_UL_MODE_SUL_ONLY) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,%d,%d,%d,",
                    txpwrQryCnf->nrTxPwrInfo[loop].sulPuschPwr, txpwrQryCnf->nrTxPwrInfo[loop].sulPucchPwr,
                    txpwrQryCnf->nrTxPwrInfo[loop].sulSrsPwr, txpwrQryCnf->nrTxPwrInfo[loop].sulPrachPwr,
                    txpwrQryCnf->nrTxPwrInfo[loop].sulFreq);
            } else {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,%d,%d,%d,",
                    txpwrQryCnf->nrTxPwrInfo[loop].ulPuschPwr, txpwrQryCnf->nrTxPwrInfo[loop].ulPucchPwr,
                    txpwrQryCnf->nrTxPwrInfo[loop].ulSrsPwr, txpwrQryCnf->nrTxPwrInfo[loop].ulPrachPwr,
                    txpwrQryCnf->nrTxPwrInfo[loop].ulFreq);
            }
        }
        /* 删除循环最后的,字符 */
        g_atSendDataBuff.bufLen = length - 1;
    } else {
        g_atSendDataBuff.bufLen = length;
    }

    return result;
}

#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_UINT32 At_PdmCtrlCnfProc(HPA_AT_PdmCtrlCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         rslt;
    VOS_UINT8          indexNum;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PDM_CTRL) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (msg->result == AT_HPA_RSLT_FAIL) {
        AT_INFO_LOG("At_PdmCtrlCnfProc: read PdmCtrl err");
        rslt = AT_ERROR;
    } else {
        rslt = AT_OK;
    }

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvMtaQryDpdtValueCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_QryDpdtValueCnf *qryDpdtValueCnf = (MTA_AT_QryDpdtValueCnf *)mtaMsg->content;
    VOS_UINT32              result;

    /* 判断查询操作是否成功 */
    if (qryDpdtValueCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result = AT_OK;
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                qryDpdtValueCnf->dpdtValue);
    } else {
        result = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}

VOS_UINT32 atLwclashCnfProc(struct MsgCB *msgBlock)
{
    L4A_READ_LwclashCnf *lwclash = NULL;
    VOS_UINT16           length;

    lwclash = (L4A_READ_LwclashCnf *)msgBlock;

    length = (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr,
        "^LWCLASH: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
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
        lwclash->scellInfo[AT_COMMAND_PARA_INDEX_3].dlMimo, lwclash->dl256QamFlag);

    CmdErrProc((VOS_UINT8)(lwclash->clientId), lwclash->errorCode, length, g_atSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

