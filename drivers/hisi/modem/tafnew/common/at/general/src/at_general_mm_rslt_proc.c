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

#include "at_general_mm_rslt_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_event_report.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "taf_drv_agent.h"
#include "at_external_module_msg_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_MM_RSLT_PROC_C

#define AT_BUF_ELEM_PROROCOL 4
#define AT_BUF_ELEM_UE_CUR_CFUN_MODE 5
#define AT_BUF_ELEM_PRODUCT_CLASS 6
#define AT_BUF_ELEM_PRODUCT_ID 7
#define AT_PLMN_PLMNID 0      /* PLMN ID  */
#define AT_PLMN_PLMNSTATUS 1  /* PLMN状态 */
#define AT_PLMN_ACCESS_MODE 2 /* 接入方式 */

/* CESQ命令Rscp参数相关宏定义 */
#define AT_CMD_CESQ_RSCP_MIN_VALUE 0
#define AT_CMD_CESQ_RSCP_MAX_VALUE 96
#define AT_CMD_CESQ_RSCP_INVALID_VALUE 255
#define AT_CMD_CESQ_RSCP_CONVERT_BASE_VALUE 121
#define AT_CMD_CESQ_RSCP_LOWER_BOUNDARY_VALUE (-120)
#define AT_CMD_CESQ_RSCP_UPPER_BOUNDARY_VALUE (-25)

/* CESQ命令Ber参数相关宏定义 */
#define AT_CMD_CESQ_BER_INVALID_VALUE 99

#define AT_RX_RSRPR0 0  /* 天线 0 rsrp， 单位 dB */
#define AT_RX_RSRPR1 1  /* 天线 1 rsrp， 单位 dB */
#define AT_RX_RSRPR2 2  /* 天线 2 rsrp， 单位 dB，仅 4收时有效 */
#define AT_RX_RSRPR3 3  /* 天线 3 rsrp， 单位 dB，仅 4收时有效 */

#define AT_RX_SINRR0 0  /* 天线 0 sinr */
#define AT_RX_SINRR1 1  /* 天线 1 sinr */
#define AT_RX_SINRR2 2  /* 天线 2 sinr */
#define AT_RX_SINRR3 3  /* 天线 3 sinr */

/* CESQ命令Rxlev参数相关宏定义 */
#define AT_CMD_CESQ_RXLEV_MIN_VALUE 0
#define AT_CMD_CESQ_RXLEV_MAX_VALUE 63
#define AT_CMD_CESQ_RXLEV_INVALID_VALUE 99
#define AT_CMD_CESQ_RXLEV_CONVERT_BASE_VALUE 111
#define AT_CMD_CESQ_RXLEV_LOWER_BOUNDARY_VALUE (-110)
#define AT_CMD_CESQ_RXLEV_UPPER_BOUNDARY_VALUE (-48)

/* CESQ命令Ecno参数相关宏定义 */
#define AT_CMD_CESQ_ECNO_MIN_VALUE 0
#define AT_CMD_CESQ_ECNO_MAX_VALUE 49
#define AT_CMD_CESQ_ECNO_INVALID_VALUE 255
#define AT_CMD_CESQ_ECNO_CONVERT_BASE_VALUE 49
#define AT_CMD_CESQ_ECNO_LOWER_BOUNDARY_VALUE (-48)
#define AT_CMD_CESQ_ECNO_UPPER_BOUNDARY_VALUE 0

/* CESQ命令Rsrq参数相关宏定义 */
#define AT_CMD_CESQ_RSRQ_MIN_VALUE 0
#define AT_CMD_CESQ_RSRQ_MAX_VALUE 34
#define AT_CMD_CESQ_RSRQ_INVALID_VALUE 255
#define AT_CMD_CESQ_RSRQ_CONVERT_BASE_VALUE 40
#define AT_CMD_CESQ_RSRQ_LOWER_BOUNDARY_VALUE (-39)
#define AT_CMD_CESQ_RSRQ_UPPER_BOUNDARY_VALUE (-6)

/* CESQ命令Rsrp参数相关宏定义 */
#define AT_CMD_CESQ_RSRP_MIN_VALUE 0
#define AT_CMD_CESQ_RSRP_MAX_VALUE 97
#define AT_CMD_CESQ_RSRP_INVALID_VALUE 255
#define AT_CMD_CESQ_RSRP_CONVERT_BASE_VALUE 141
#define AT_CMD_CESQ_RSRP_LOWER_BOUNDARY_VALUE (-140)
#define AT_CMD_CESQ_RSRP_UPPER_BOUNDARY_VALUE (-44)

VOS_UINT32 AT_RcvMmaHsQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_HandShakeQryCnf *hsCnf = (TAF_MMA_HandShakeQryCnf *)msg;
    VOS_UINT32               id;
    TAF_UINT16               length   = 0;

    id = TAF_GET_HOST_UINT32(hsCnf->buf);

    /* id */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, id);
    /* protocol */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", hsCnf->buf[AT_BUF_ELEM_PROROCOL]);
    /* is_offline */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", hsCnf->buf[AT_BUF_ELEM_UE_CUR_CFUN_MODE]);
    /* product_class */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", hsCnf->buf[AT_BUF_ELEM_PRODUCT_CLASS]);
    /* product_id */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", hsCnf->buf[AT_BUF_ELEM_PRODUCT_ID]);

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 AT_RcvMmaLocInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_LocationInfoQryCnf *locInfoCnf = (TAF_MMA_LocationInfoQryCnf *)msg;
    VOS_UINT32                  result = AT_OK;
    VOS_UINT16                  length = 0;
    VOS_UINT8                   cellIdStr[AT_CELLID_STRING_MAX_LEN];

    (VOS_VOID)memset_s(cellIdStr, sizeof(cellIdStr), 0x00, sizeof(cellIdStr));

    /* 格式化AT^CIPERQRY查询命令返回 */
    if (locInfoCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        result = AT_OK;

        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        /* 上报MCC和MNC */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%X%X%X", (locInfoCnf->mcc & 0x0f),
            (locInfoCnf->mcc & 0x0f00) >> 8, (locInfoCnf->mcc & 0x0f0000) >> 16);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%X%X", (locInfoCnf->mnc & 0x0f),
            (locInfoCnf->mnc & 0x0f00) >> 8);

        /* MNC最后一位不等于F，则MNC为三位，否则为两位 */
        if (((locInfoCnf->mnc & 0x0f0000) >> 16) != 0x0f) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%X", (locInfoCnf->mnc & 0x0f0000) >> 16);
        }

        /* Lac */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",0x%X", locInfoCnf->lac);

        /* Rac */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",0x%X", locInfoCnf->rac);

        /* Cell ID */
        AT_ConvertCellIdToHexStrFormat(locInfoCnf->cellId.cellIdLowBit, locInfoCnf->cellId.cellIdHighBit,
                                       (VOS_CHAR *)cellIdStr);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",0x%s", cellIdStr);

        g_atSendDataBuff.bufLen = length;
    }

    return result;
}

VOS_UINT32 AT_RcvMmaEOPlmnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_EoplmnQryCnf *eOPlmnQryCnf = (TAF_MMA_EoplmnQryCnf *)msg;
    VOS_UINT16            len = 0;
    VOS_UINT32            rslt;

    /* 输出查询结果 */
    if (eOPlmnQryCnf->result == TAF_ERR_NO_ERROR) {
        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "%s: \"%s\",%d,", g_parseContext[indexNum].cmdElement->cmdName,
            eOPlmnQryCnf->version, eOPlmnQryCnf->oPlmnNum * TAF_AT_PLMN_WITH_RAT_LEN);

        /* 将16进制数转换为ASCII码后输入主动命令内容 */
        len += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                        (TAF_UINT8 *)g_atSndCodeAddress + len,
                                                        eOPlmnQryCnf->oPlmnNum * TAF_SIM_PLMN_WITH_RAT_LEN,
                                                        eOPlmnQryCnf->oPlmnList);

        g_atSendDataBuff.bufLen = len;

        rslt = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        rslt = AT_ERROR;
    }

    return rslt;
}

VOS_UINT32 AT_RcvMmaSysCfgSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_SysCfgCnf *cnfMsg = (TAF_MMA_SysCfgCnf *)msg;
    VOS_UINT32         result;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvMmaSysCfgSetCnf: AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (cnfMsg->rslt == TAF_MMA_APP_OPER_RESULT_SUCCESS) {
        result = AT_OK;
    } else {
        result = At_ChgTafErrorCode(indexNum, cnfMsg->errorCause);
    }

    g_atSendDataBuff.bufLen = 0;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_IsSupport1XMode(TAF_MMA_MultimodeRatCfg *ratOrder)
{
    VOS_UINT32 i;
    for (i = 0; i < ratOrder->ratNum; i++) {
        if (ratOrder->ratOrder[i] == TAF_MMA_RAT_1X) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_IsSupportHrpdMode(TAF_MMA_MultimodeRatCfg *ratOrder)
{
    VOS_UINT32 i;

    for (i = 0; i < ratOrder->ratNum; i++) {
        if (ratOrder->ratOrder[i] == TAF_MMA_RAT_HRPD) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}
#endif

VOS_UINT32 AT_IsSupportGMode(TAF_MMA_MultimodeRatCfg *ratOrder)
{
    VOS_UINT32 i;

    for (i = 0; i < ratOrder->ratNum; i++) {
        if (ratOrder->ratOrder[i] == TAF_MMA_RAT_GSM) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_IsSupportWMode(TAF_MMA_MultimodeRatCfg *ratOrder)
{
    VOS_UINT32 i;

    for (i = 0; i < ratOrder->ratNum; i++) {
        if (ratOrder->ratOrder[i] == TAF_MMA_RAT_WCDMA) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

LOCAL VOS_VOID AT_SysCfgQryGetAccessMode(TAF_MMA_RatTypeUint8 ratType, TAF_MMA_MultimodeRatCfg *multiModeRatCfg,
    AT_SYSCFG_RatTypeUint8 *accessMode, AT_SYSCFG_RatPrioUint8 *acqorder)
{
    switch (ratType) {
        case TAF_MMA_RAT_GSM:
            if (AT_IsSupportWMode(multiModeRatCfg) == VOS_TRUE) {
                *accessMode = AT_SYSCFG_RAT_AUTO;
            } else {
                *accessMode = AT_SYSCFG_RAT_GSM;
            }
            break;
        case TAF_MMA_RAT_WCDMA:
            if (AT_IsSupportGMode(multiModeRatCfg) == VOS_TRUE) {
                *accessMode = AT_SYSCFG_RAT_AUTO;
            } else {
                *accessMode = AT_SYSCFG_RAT_WCDMA;
            }
            break;
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case TAF_MMA_RAT_1X:
            if (AT_IsSupportHrpdMode(multiModeRatCfg) == VOS_TRUE) {
                *accessMode = AT_SYSCFG_RAT_1X_AND_HRPD;
            } else {
                *accessMode = AT_SYSCFG_RAT_1X;
            }
            break;

        case TAF_MMA_RAT_HRPD:
            if (AT_IsSupport1XMode(multiModeRatCfg) == VOS_TRUE) {
                *accessMode = AT_SYSCFG_RAT_1X_AND_HRPD;
            } else {
                *accessMode = AT_SYSCFG_RAT_HRPD;
            }
            break;
#endif
        default:
            /* 只支持L的情况 */
            *accessMode = AT_SYSCFG_RAT_AUTO;
            *acqorder = AT_SYSCFG_RAT_PRIO_AUTO;
            break;
    }
}

VOS_VOID AT_GetOnlyGURatOrder(TAF_MMA_MultimodeRatCfg *ratOrder)
{
    TAF_MMA_MultimodeRatCfg ratOrderInfo;
    VOS_UINT32              i;
    errno_t                 memResult;
    VOS_UINT8               indexNum = 0;

    memset_s(&ratOrderInfo, (VOS_SIZE_T)sizeof(ratOrderInfo), 0x00, (VOS_SIZE_T)sizeof(ratOrderInfo));

    memResult = memcpy_s(&ratOrderInfo, (VOS_SIZE_T)sizeof(ratOrderInfo), ratOrder, (VOS_SIZE_T)sizeof(ratOrderInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)sizeof(ratOrderInfo), (VOS_SIZE_T)sizeof(ratOrderInfo));

    /* 获取GU模信息 */
    ratOrderInfo.ratNum = AT_MIN(ratOrderInfo.ratNum, TAF_MMA_RAT_BUTT);
    for (i = 0; i < ratOrderInfo.ratNum; i++) {
        if ((ratOrderInfo.ratOrder[i] == TAF_MMA_RAT_WCDMA) || (ratOrderInfo.ratOrder[i] == TAF_MMA_RAT_GSM)) {
            ratOrder->ratOrder[indexNum] = ratOrderInfo.ratOrder[i];
            indexNum++;
        }
    }

    ratOrder->ratNum             = indexNum;
    ratOrder->ratOrder[indexNum] = TAF_MMA_RAT_BUTT;
}

VOS_INT8 AT_GetValidLteBandIndex(TAF_USER_SetLtePrefBandInfo *lBand)
{
    VOS_INT8 validIndex;

    if (lBand == VOS_NULL_PTR) {
        return 0;
    }

    for (validIndex = TAF_MMA_LTE_BAND_MAX_LENGTH - 1; validIndex >= 0; validIndex--) {
        if (lBand->bandInfo[validIndex] != 0) {
            return validIndex;
        }
    }

    return 0;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)

VOS_VOID AT_SysCfgexQryRoamParaConvert(VOS_UINT8 clientId, VOS_UINT8 *outRoam)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    netCtx = AT_GetModemNetCtxAddrFromClientId(clientId);

    if (netCtx->roamFeature == VOS_FALSE) {
        *outRoam = (*outRoam == TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF) ? VOS_FALSE : VOS_TRUE;
        AT_WARN_LOG1("AT_SysCfgexQryRoamParaConvert():Convert RoamCapa, outRoam", *outRoam);
    }
}

#endif

VOS_VOID AT_ConvertSysCfgStrToAutoModeStr(VOS_UINT8 *acqOrderBegin, VOS_UINT8 *acqOrder,
                                                VOS_UINT32 acqOrderLength, VOS_UINT8 ratNum)
{
    errno_t returnValue;
#if ((FEATURE_UE_MODE_NR == FEATURE_ON) && (FEATURE_LTE == FEATURE_ON))
    if ((VOS_StrCmp((VOS_CHAR *)acqOrderBegin, "08030201") == 0) && (ratNum == TAF_PH_MAX_GULNR_RAT_NUM)) {
        /* 接入技术的个数为4且接入优先顺序为NR->L->W->G,acqorder上报00 */
        acqOrder    = acqOrderBegin;
        returnValue = strcpy_s((VOS_CHAR *)acqOrder, acqOrderLength, "00");
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, acqOrderLength, acqOrderLength);
        acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *acqOrder = '\0';
    }
#elif (FEATURE_LTE == FEATURE_ON)
    if ((VOS_StrCmp((VOS_CHAR *)acqOrderBegin, "030201") == 0) && (ratNum == TAF_PH_MAX_GUL_RAT_NUM)) {
        /* 接入技术的个数为3且接入优先顺序为L->W->G,acqorder上报00 */
        acqOrder = acqOrderBegin;
        returnValue = strcpy_s((VOS_CHAR *)acqOrder, acqOrderLength, "00");
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, acqOrderLength, acqOrderLength);
        acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *acqOrder = '\0';
    }
#elif (FEATURE_UE_MODE_NR == FEATURE_ON)
    if ((VOS_StrCmp((VOS_CHAR *)acqOrderBegin, "080201") == 0) && (ratNum == TAF_PH_MAX_GUNR_RAT_NUM)) {
        /* 接入技术的个数为3且接入优先顺序为NR->W->G,acqorder上报00 */
        acqOrder       = acqOrderBegin;
        returnValue    = strcpy_s((VOS_CHAR *)acqOrder, acqOrderLength, "00");
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, acqOrderLength, acqOrderLength);
        acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *acqOrder = '\0';
    }
#else
    if ((VOS_StrCmp((VOS_CHAR *)acqOrderBegin, "0201") == 0) && (ratNum == TAF_PH_MAX_GU_RAT_NUM)) {
        /* 接入技术的个数为2且接入优先顺序为W->G,acqorder上报00 */
        acqOrder    = acqOrderBegin;
        returnValue = strcpy_s((VOS_CHAR *)acqOrder, acqOrderLength, "00");
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, acqOrderLength, acqOrderLength);
        acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
        *acqOrder = '\0';
    }
#endif
}

VOS_VOID AT_ConvertSysCfgRatOrderToStr(TAF_MMA_MultimodeRatCfg *ratOrder, VOS_UINT8 *acqOrder, VOS_UINT32 acqOrderLength)
{
    VOS_UINT32 i;
    VOS_UINT8 *acqOrderBegin = VOS_NULL_PTR;
    VOS_UINT32 length = acqOrderLength;
    errno_t    returnValue;

    acqOrderBegin = acqOrder;

    ratOrder->ratNum = AT_MIN(ratOrder->ratNum, TAF_MMA_RAT_BUTT);
    for (i = 0; i < ratOrder->ratNum; i++) {
        if (ratOrder->ratOrder[i] == TAF_MMA_RAT_WCDMA) {
            returnValue = strcpy_s((VOS_CHAR *)acqOrder, length, "02");
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, length, length);
            acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
            length -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        } else if (ratOrder->ratOrder[i] == TAF_MMA_RAT_GSM) {
            returnValue = strcpy_s((VOS_CHAR *)acqOrder, length, "01");
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, length, length);
            acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
            length -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#if (FEATURE_LTE == FEATURE_ON)
        else if (ratOrder->ratOrder[i] == TAF_MMA_RAT_LTE) {
            returnValue = strcpy_s((VOS_CHAR *)acqOrder, length, "03");
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, length, length);
            acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
            length -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#endif
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        else if (ratOrder->ratOrder[i] == TAF_MMA_RAT_1X) {
            returnValue = strcpy_s((VOS_CHAR *)acqOrder, length, "04");
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, length, length);
            acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
            length -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        } else if (ratOrder->ratOrder[i] == TAF_MMA_RAT_HRPD) {
            returnValue = strcpy_s((VOS_CHAR *)acqOrder, length, "07");
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, length, length);
            acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
            length -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        else if (ratOrder->ratOrder[i] == TAF_MMA_RAT_NR) {
            returnValue = strcpy_s((VOS_CHAR *)acqOrder, length, "08");
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(returnValue, length, length);
            acqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
            length -= AT_SYSCFGEX_RAT_MODE_STR_LEN;
        }
#endif
        else {
        }
    }

    *acqOrder = '\0';

    AT_ConvertSysCfgStrToAutoModeStr(acqOrderBegin, acqOrder, acqOrderLength, ratOrder->ratNum);
}

VOS_VOID AT_ReportSysCfgExQryCmdResult(TAF_MMA_SysCfgPara *sysCfg, VOS_UINT8 indexNum)
{
    VOS_UINT8  acqorder[TAF_MMA_RAT_BUTT * 2 + 1] = {0};
    VOS_UINT8 *acqOrder = VOS_NULL_PTR;
    VOS_INT8   lteBandIndex;
    VOS_UINT16 length;

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* mbb下，漫游特性关闭时，漫游能力转化 */
    AT_SysCfgexQryRoamParaConvert(indexNum, &(sysCfg->roam));
#endif

    length   = 0;
    acqOrder = acqorder;

    /* 把上报的TAF_MMA_MultimodeRatCfg结构转换为acqorder字符串 */
    AT_ConvertSysCfgRatOrderToStr(&sysCfg->multiModeRatCfg, acqOrder, sizeof(acqorder));

    /* 按syscfgex查询格式上报^SYSCFGEX: <acqorder>,<band>,<roam>,<srvdomain>,<lteband> */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

    if (sysCfg->guBand.bandHigh == 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\",%X,%d,%d", acqOrder, sysCfg->guBand.bandLow, sysCfg->roam,
            sysCfg->srvDomain);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\",%X%08X,%d,%d", acqOrder, sysCfg->guBand.bandHigh,
            sysCfg->guBand.bandLow, sysCfg->roam, sysCfg->srvDomain);
    }
    lteBandIndex = AT_GetValidLteBandIndex(&(sysCfg->lBand));

    if (lteBandIndex == 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%X", sysCfg->lBand.bandInfo[0]);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%X", sysCfg->lBand.bandInfo[lteBandIndex]);

        lteBandIndex--;

        while (lteBandIndex >= 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%08X", sysCfg->lBand.bandInfo[lteBandIndex]);

            lteBandIndex--;
        }
    }
    g_atSendDataBuff.bufLen = length;
}

VOS_VOID AT_ReportSysCfgQryCmdResult(TAF_MMA_SysCfgPara *sysCfg, VOS_UINT8 indexNum)
{
    AT_SYSCFG_RatTypeUint8 accessMode;
    AT_SYSCFG_RatPrioUint8 acqorder;
    VOS_UINT16             length = 0;

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* mbb下，漫游特性关闭时，漫游能力转化 */
    AT_SysCfgexQryRoamParaConvert(indexNum, &(sysCfg->roam));
#endif

    /* 从当前接入优先级中提取GU模接入优先级的信息 */
    AT_GetOnlyGURatOrder(&sysCfg->multiModeRatCfg);
    acqorder = sysCfg->userPrio;

    /* 把上报的TAF_MMA_RatOrder结构转换为mode和acqorder */
    sysCfg->multiModeRatCfg.ratNum = AT_MIN(sysCfg->multiModeRatCfg.ratNum, TAF_MMA_RAT_BUTT);

    AT_SysCfgQryGetAccessMode(sysCfg->multiModeRatCfg.ratOrder[0], &sysCfg->multiModeRatCfg, &accessMode, &acqorder);

    /* 按syscfg查询格式上报^SYSCFG:<mode>,<acqorder>,<band>,<roam>,<srvdomain> */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
    if (sysCfg->guBand.bandHigh == 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,%X,%d,%d", accessMode, acqorder, sysCfg->guBand.bandLow,
            sysCfg->roam, sysCfg->srvDomain);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,%X%08X,%d,%d", accessMode, acqorder,
            sysCfg->guBand.bandHigh, sysCfg->guBand.bandLow, sysCfg->roam, sysCfg->srvDomain);
    }
    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_ConvertSysCfgRatOrderToWs46No(TAF_MMA_MultimodeRatCfg *ratOrder, VOS_UINT32 *proctolNo)
{
    VOS_UINT8                       gsmSupportFlag;
    VOS_UINT8                       wcdmaSupportFlag;
    VOS_UINT8                       lteSupportFlag;
    VOS_UINT32                      i;
    const AT_WS46_RatTransformTbl    ws46RatTransFormTab[] = {
        /*     G,         U,         L,     Ws46No */
        { VOS_TRUE, VOS_FALSE, VOS_FALSE, 12 }, { VOS_FALSE, VOS_TRUE, VOS_FALSE, 22 },
        { VOS_TRUE, VOS_TRUE, VOS_TRUE, 25 },   { VOS_FALSE, VOS_FALSE, VOS_TRUE, 28 },
        { VOS_TRUE, VOS_TRUE, VOS_FALSE, 29 },  { VOS_TRUE, VOS_FALSE, VOS_TRUE, 30 },
        { VOS_FALSE, VOS_TRUE, VOS_TRUE, 31 },
    };

    gsmSupportFlag   = VOS_FALSE;
    wcdmaSupportFlag = VOS_FALSE;
    lteSupportFlag   = VOS_FALSE;
    ratOrder->ratNum = AT_MIN(ratOrder->ratNum, TAF_MMA_RAT_BUTT);
    for (i = 0; i < (VOS_UINT32)ratOrder->ratNum; i++) {
        if (ratOrder->ratOrder[i] == TAF_MMA_RAT_GSM) {
            gsmSupportFlag = VOS_TRUE;
        } else if (ratOrder->ratOrder[i] == TAF_MMA_RAT_WCDMA) {
            wcdmaSupportFlag = VOS_TRUE;
        }
#if (FEATURE_LTE == FEATURE_ON)
        else if (ratOrder->ratOrder[i] == TAF_MMA_RAT_LTE) {
            lteSupportFlag = VOS_TRUE;
        }
#endif
        else {
        }
    }

    for (i = 0; i < sizeof(ws46RatTransFormTab) / sizeof(ws46RatTransFormTab[0]); i++) {
        if ((gsmSupportFlag == ws46RatTransFormTab[i].gsmSupportFlg) &&
            (wcdmaSupportFlag == ws46RatTransFormTab[i].wcdmaSupportFlg) &&
            (lteSupportFlag == ws46RatTransFormTab[i].lteSupportFlg)) {
            *proctolNo = ws46RatTransFormTab[i].ws46No;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_ReportWs46QryCmdResult(TAF_MMA_SysCfgPara *sysCfg, VOS_UINT8 indexNum)
{
    VOS_UINT32 ws46No = 0;
    VOS_UINT16 length = 0;

    /* 把上报的TAF_MMA_MultimodeRatCfg结构转换为<n> */
    if (AT_ConvertSysCfgRatOrderToWs46No(&sysCfg->multiModeRatCfg, &ws46No) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 按+WS46查询格式上报+WS46: <n> */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", ws46No);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_RcvTafMmaSyscfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_SyscfgQryCnf *syscfgQryCnf = (TAF_MMA_SyscfgQryCnf *)msg;
    AT_RreturnCodeUint32  result = AT_FAILURE;

    AT_PR_LOGI("Rcv Msg");

    if (syscfgQryCnf->errorCause != TAF_ERR_NO_ERROR) {                  /* MT本地错误 */
        result = At_ChgTafErrorCode(indexNum, syscfgQryCnf->errorCause); /* 发生错误 */
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_SYSCFG_READ) {
        AT_ReportSysCfgQryCmdResult(&(syscfgQryCnf->sysCfg), indexNum);
        result = AT_OK;
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_SYSCFGEX_READ) {
        AT_ReportSysCfgExQryCmdResult(&(syscfgQryCnf->sysCfg), indexNum);
        result = AT_OK;
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_WS46_QRY) {
        result = AT_ReportWs46QryCmdResult(&(syscfgQryCnf->sysCfg), indexNum);
    } else {
        result = AT_ERROR;
    }

    return result;
}

LOCAL VOS_VOID AT_ProcCerssiInfoQueryCampOnGsm(TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf, VOS_UINT8 indexNum,
                                               VOS_UINT8 systemAppConfig)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    VOS_UINT16      length = 0;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if (systemAppConfig != SYSTEM_APP_ANDROID) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.gCellSignInfo.rssiValue, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99,
            99, 99, 99, 99, 99, 99, 99, 0, 0, 0);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.gCellSignInfo.rssiValue, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 0, 0);
    }

    g_atSendDataBuff.bufLen = length;
}

LOCAL VOS_VOID AT_ProcCerssiInfoQueryCampOnWcdma(TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf, VOS_UINT8 indexNum,
                                                 VOS_UINT8 systemAppConfig)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    VOS_UINT16      length = 0;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if (cerssiInfoQueryCnf->cerssi.currentUtranMode == TAF_UTRANCTRL_UTRAN_MODE_FDD) {
        if (systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue,
                cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.ecioValue, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99,
                99, 99, 99, 99, 99, 99, 0, 0, 0);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue,
                cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.ecioValue, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 0, 0);
        }

        g_atSendDataBuff.bufLen = length;
    } else {
        /* 非fdd 3g 小区，ecio值为无效值255 */
        if (systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0,
                99, 99, 99, 99, 99, 99, 99, 99, 0, 0, 0);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0,
                0, 0);
        }

        g_atSendDataBuff.bufLen = length;
    }
}

#if (FEATURE_LTE == FEATURE_ON)
LOCAL VOS_VOID AT_ProcCerssiInfoQueryCampOnLte(TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf, VOS_UINT8 indexNum,
                                               VOS_UINT8 systemAppConfig)
{
    VOS_UINT16      length = 0;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    /* LTE下的cerssi查询处理由L4A移到NAS，和atCerssiInfoCnfProc的处理相同 */
    if (systemAppConfig != SYSTEM_APP_ANDROID) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval, 0,
            0, 255,
            (cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp == AT_SIGNAL_INVALID_VALUE ?
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp :
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp / AT_SIGNAL_DIVISOR_EIGHT),
            (cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq == AT_SIGNAL_INVALID_VALUE ?
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq :
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq / AT_SIGNAL_DIVISOR_EIGHT),
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.sinr,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.ri,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[0],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[1],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rxANTNum,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR0],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR1],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR2],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR3],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR0],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR1],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR2],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR3], 0, 0, 0);

    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval, 0,
            0, 255,
            (cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp == AT_SIGNAL_INVALID_VALUE ?
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp :
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp / AT_SIGNAL_DIVISOR_EIGHT),
            (cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq == AT_SIGNAL_INVALID_VALUE ?
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq :
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq / AT_SIGNAL_DIVISOR_EIGHT),
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.sinr,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.ri,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[0],
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[1], 0, 0, 0);
    }
    g_atSendDataBuff.bufLen = length;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
LOCAL VOS_VOID AT_ProcCerssiInfoQueryCampOnNr(TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf, VOS_UINT8 indexNum,
                                              VOS_UINT8 systemAppConfig)
{
    VOS_UINT16      length = 0;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if (systemAppConfig != SYSTEM_APP_ANDROID) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval, 0,
            0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99, 99, 99, 99, 99, 99,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrp,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrq,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.l5GSinr);

        g_atSendDataBuff.bufLen = length;
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval, 0,
            0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, cerssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrp,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrq,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.l5GSinr);

        g_atSendDataBuff.bufLen = length;
    }
}
#endif

LOCAL VOS_VOID AT_ProcCerssiInfoQueryNotCampOn(TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf, VOS_UINT8 indexNum,
                                               VOS_UINT8 systemAppConfig)
{
    VOS_UINT16      length = 0;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if (systemAppConfig != SYSTEM_APP_ANDROID) {
        /* 刚开机没有接入模式，参数都返回无效值 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.gCellSignInfo.rssiValue,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99,
            99, 99, 99, 99, 99, 99, 0, 0, 0);
    } else {
        /* 刚开机没有接入模式，参数都返回无效值 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.gCellSignInfo.rssiValue,
            cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 0, 0);
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_ProcCerssiInfoQuery(struct MsgCB *msg,  VOS_UINT8 indexNum)
{
    TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf = (TAF_MMA_CerssiInfoQryCnf *)msg;
    VOS_UINT8 *systemAppConfig =  AT_GetSystemAppConfigAddr();

    if (cerssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_GSM) {
        AT_ProcCerssiInfoQueryCampOnGsm(cerssiInfoQueryCnf, indexNum, *systemAppConfig);
    } else if (cerssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_WCDMA) {
        AT_ProcCerssiInfoQueryCampOnWcdma(cerssiInfoQueryCnf, indexNum, *systemAppConfig);
    }
#if (FEATURE_LTE == FEATURE_ON)
    else if (cerssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_LTE) {
        AT_ProcCerssiInfoQueryCampOnLte(cerssiInfoQueryCnf, indexNum, *systemAppConfig);
    }
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    else if (cerssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_NR) {
        AT_ProcCerssiInfoQueryCampOnNr(cerssiInfoQueryCnf, indexNum, *systemAppConfig);
    }
#endif

    /* 还没有收到小区信号 */
    else {
        AT_ProcCerssiInfoQueryNotCampOn(cerssiInfoQueryCnf, indexNum, *systemAppConfig);
    }

    return AT_OK;
}

VOS_INT16 AT_ConvertCerssiRssiToCesqRxlev(VOS_INT16 cerssiRssi)
{
    VOS_INT16 cesqRxlev;

    /*
     * 转换原则如下:
     * 0                        sCerssiRssi < -110dBm
     * 1             -110dBm <= sCerssiRssi < -109dBm
     * 2             -109dBm <= sCerssiRssi < -108dBm
     * 3             -108dBm <= sCerssiRssi < -107dBm
     * ...           ...
     * 62            -49dBm  <= sCerssiRssi < -48 dBm
     * 63            -48dBm  <= sCerssiRssi
     * 99            not known or not detectable
     */

    if (cerssiRssi < AT_CMD_CESQ_RXLEV_LOWER_BOUNDARY_VALUE) {
        cesqRxlev = AT_CMD_CESQ_RXLEV_MIN_VALUE;
    } else if (cerssiRssi < AT_CMD_CESQ_RXLEV_UPPER_BOUNDARY_VALUE) {
        cesqRxlev = cerssiRssi + AT_CMD_CESQ_RXLEV_CONVERT_BASE_VALUE;
    } else {
        cesqRxlev = AT_CMD_CESQ_RXLEV_MAX_VALUE;
    }

    return cesqRxlev;
}

VOS_INT16 AT_ConvertCerssiRscpToCesqRscp(VOS_INT16 cerssiRscp)
{
    VOS_INT16 cesqRscp;

    /*
     * 转换原则如下:
     * 0                        sCerssiRscp < -120dBm
     * 1             -120dBm <= sCerssiRscp < -119dBm
     * 2             -119dBm <= sCerssiRscp < -118dBm
     * 3             -118dBm <= sCerssiRscp < -117dBm
     * ...           ...
     * 95            -26dBm  <= sCerssiRscp < -25 dBm
     * 96            -25dBm  <= sCerssiRscp
     * 255            not known or not detectable
     */

    if (cerssiRscp >= AT_HCSQ_VALUE_INVALID) {
        /* 无效值 */
        cesqRscp = AT_HCSQ_VALUE_INVALID;
    } else if (cerssiRscp < AT_CMD_CESQ_RSCP_LOWER_BOUNDARY_VALUE) {
        cesqRscp = AT_CMD_CESQ_RSCP_MIN_VALUE;
    } else if (cerssiRscp < AT_CMD_CESQ_RSCP_UPPER_BOUNDARY_VALUE) {
        cesqRscp = cerssiRscp + AT_CMD_CESQ_RSCP_CONVERT_BASE_VALUE;
    } else {
        cesqRscp = AT_CMD_CESQ_RSCP_MAX_VALUE;
    }

    return cesqRscp;
}

VOS_INT8 AT_ConvertCerssiEcnoToCesqEcno(VOS_INT8 ecno)
{
    VOS_INT8 cesqEcno;

    /*
     * 转换原则如下:
     * 【原有sEcioValue转换方法】
     *        0                        sEcioValue < -24dB
     *        1             -24dB   <= sEcioValue < -23.5dB
     *        2             -23.5dB <= sEcioValue < -23dB
     *        ...           ...
     *        47            -1dB    <= sEcioValue < -0.5dB
     *        48            -0.5dB  <= sEcioValue < 0dB
     *        49             0dB    <= sEcioValue
     *        255            not known or not detectable
     * 【新增cEcno接口, 比原有的sEcioValue增加0.5精度，目前仅为AT命令CESQ使用。范围为: -49~0】
     *        0                        cEcno < -48dB
     *        1             -48dB   <= cEcno < -47dB
     *        2             -47dB   <= cEcno < -46dB
     *        3             -46dB   <= cEcno < -45dB
     *        4             -45dB   <= cEcno < -44dB
     *        ...           ...
     *        47            -2dB    <= cEcno < -1dB
     *        48            -1dB    <= cEcno < 0dB
     *        49            0dB     <= cEcno
     *        255           not known or not detectable
     */

    if (ecno < AT_CMD_CESQ_ECNO_LOWER_BOUNDARY_VALUE) {
        cesqEcno = AT_CMD_CESQ_ECNO_MIN_VALUE;
    } else if (ecno < AT_CMD_CESQ_ECNO_UPPER_BOUNDARY_VALUE) {
        cesqEcno = ecno + AT_CMD_CESQ_ECNO_CONVERT_BASE_VALUE;
    } else {
        cesqEcno = AT_CMD_CESQ_ECNO_MAX_VALUE;
    }

    return cesqEcno;
}

VOS_INT16 AT_ConvertCerssiRsrq(VOS_INT16 cerssiRsrq)
{
    /*
     * 转换原则如下:
     * 【Rsrq底层上报原值，这里除以4, 所以没有小数, 目前sCerssiRsrq按照-40~-6上报的】
     *        0                       sCerssiRsrq < -39dB
     *        1             -39dB  <= sCerssiRsrq < -38dB
     *        2             -38dB  <= sCerssiRsrq < -37dB
     *        3             -37dB  <= sCerssiRsrq < -36dB
     *        ...           ...
     *        33            -7dB   <= sCerssiRsrq < -6dB
     *        34            -6 dB  <= sCerssiRsrq
     *        255           not known or not detectable
     */
    if (cerssiRsrq >= AT_HCSQ_VALUE_INVALID) {
        /* 无效值 */
        cerssiRsrq = AT_HCSQ_VALUE_INVALID;
    } else if (cerssiRsrq < AT_CMD_CESQ_RSRQ_LOWER_BOUNDARY_VALUE) {
        cerssiRsrq = AT_CMD_CESQ_RSRQ_MIN_VALUE;
    } else if (cerssiRsrq < AT_CMD_CESQ_RSRQ_UPPER_BOUNDARY_VALUE) {
        cerssiRsrq = cerssiRsrq + AT_CMD_CESQ_RSRQ_CONVERT_BASE_VALUE;
    } else {
        cerssiRsrq = AT_CMD_CESQ_RSRQ_MAX_VALUE;
    }

    return cerssiRsrq;
}

VOS_INT16 AT_ConvertCerssiRsrqToCesqRsrq(VOS_INT16 cerssiRsrq)
{
    VOS_INT16 result;
    /* LRRC上报给空口的测量报告按照除法处理，此处需要保持一致 */
    result = cerssiRsrq / AT_SIGNAL_DIVISOR_FOUR;
    /* 左闭右开，因此结果需要向下取整 */
    result -= (cerssiRsrq % AT_SIGNAL_DIVISOR_FOUR == 0 ? 0 : 1);

    return AT_ConvertCerssiRsrq(result);
}

VOS_INT16 AT_ConvertCerssiRsrp(VOS_INT16 cerssiRsrp)
{
    VOS_INT16 cesqRsrp;

    /*
    * 转换原则如下:
    * 0                        sCerssiRsrp < -140dBm
    * 1             -140dBm <= sCerssiRsrp < -139dBm
    * 2             -139dBm <= sCerssiRsrp < -138dBm
    * 3             -138dBm <= sCerssiRsrp < -137dBm
    * ...           ...
    * 96            -45dBm  <= sCerssiRsrp < -44 dBm
    * 97            -44dBm  <= sCerssiRsrp
    * 255            not known or not detectable
    */
    if (cerssiRsrp >= AT_HCSQ_VALUE_INVALID) {
        /* 无效值 */
        cesqRsrp = AT_HCSQ_VALUE_INVALID;
    } else if (cerssiRsrp < AT_CMD_CESQ_RSRP_LOWER_BOUNDARY_VALUE) {
        cesqRsrp = AT_CMD_CESQ_RSRP_MIN_VALUE;
    } else if (cerssiRsrp < AT_CMD_CESQ_RSRP_UPPER_BOUNDARY_VALUE) {
        cesqRsrp = cerssiRsrp + AT_CMD_CESQ_RSRP_CONVERT_BASE_VALUE;
    } else {
        cesqRsrp = AT_CMD_CESQ_RSRP_MAX_VALUE;
    }

    return cesqRsrp;
}

VOS_INT16 AT_ConvertCerssiRsrpToCesqRsrp(VOS_INT16 cerssiRsrp)
{
    VOS_INT16 result;

    /* LRRC上报给空口的测量报告按照除法处理，此处需要保持一致 */
    result = cerssiRsrp / AT_SIGNAL_DIVISOR_EIGHT;
    /* 左闭右开，因此结果需要向下取整 */
    result -= (cerssiRsrp % AT_SIGNAL_DIVISOR_EIGHT == 0 ? 0 : 1);
    return AT_ConvertCerssiRsrp(result);
}

VOS_UINT32 AT_ProcCesqInfoSet(struct MsgCB *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf = (TAF_MMA_CerssiInfoQryCnf *)msg;
    VOS_UINT16                length = 0;

    /* GSM */
    if (cerssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_GSM) {
        length += (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName,
            AT_ConvertCerssiRssiToCesqRxlev(cerssiInfoQueryCnf->cerssi.aRssi[0].u.gCellSignInfo.rssiValue),
            cerssiInfoQueryCnf->cerssi.aRssi[0].channalQual, AT_CMD_CESQ_RSCP_INVALID_VALUE,
            AT_CMD_CESQ_ECNO_INVALID_VALUE, AT_CMD_CESQ_RSRQ_INVALID_VALUE, AT_CMD_CESQ_RSRP_INVALID_VALUE);

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }

    if (cerssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_WCDMA) {
        /* FDD 3G小区 */
        if (cerssiInfoQueryCnf->cerssi.currentUtranMode == TAF_UTRANCTRL_UTRAN_MODE_FDD) {
            length += (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, AT_CMD_CESQ_RXLEV_INVALID_VALUE,
                AT_CMD_CESQ_BER_INVALID_VALUE,
                AT_ConvertCerssiRscpToCesqRscp(cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue),
                AT_ConvertCerssiEcnoToCesqEcno(cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.ecno),
                AT_CMD_CESQ_RSRQ_INVALID_VALUE, AT_CMD_CESQ_RSRP_INVALID_VALUE);
            g_atSendDataBuff.bufLen = length;
            return AT_OK;
        } else {
            /* 非FDD 3G小区，ecio值为无效值255 */
            length += (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, AT_CMD_CESQ_RXLEV_INVALID_VALUE,
                AT_CMD_CESQ_BER_INVALID_VALUE,
                AT_ConvertCerssiRscpToCesqRscp(cerssiInfoQueryCnf->cerssi.aRssi[0].u.wCellSignInfo.rscpValue),
                AT_CMD_CESQ_ECNO_INVALID_VALUE, AT_CMD_CESQ_RSRQ_INVALID_VALUE, AT_CMD_CESQ_RSRP_INVALID_VALUE);

            g_atSendDataBuff.bufLen = length;
            return AT_OK;
        }
    }

#if (FEATURE_LTE == FEATURE_ON)
    /* LTE */
    if (cerssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_LTE) {
        length += (VOS_UINT16)AT_FormatReportString( AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, AT_CMD_CESQ_RXLEV_INVALID_VALUE,
            AT_CMD_CESQ_BER_INVALID_VALUE, AT_CMD_CESQ_RSCP_INVALID_VALUE, AT_CMD_CESQ_ECNO_INVALID_VALUE,
            AT_ConvertCerssiRsrqToCesqRsrq(cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq),
            AT_ConvertCerssiRsrpToCesqRsrp(cerssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp));

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }
#endif

    /* 刚开机没有接入模式，参数都返回无效值 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        AT_CMD_CESQ_RXLEV_INVALID_VALUE, AT_CMD_CESQ_BER_INVALID_VALUE, AT_CMD_CESQ_RSCP_INVALID_VALUE,
        AT_CMD_CESQ_ECNO_INVALID_VALUE, AT_CMD_CESQ_RSRQ_INVALID_VALUE, AT_CMD_CESQ_RSRP_INVALID_VALUE);

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 AT_RcvMmaCerssiInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CerssiInfoQryCnf *cerssiInfoQueryCnf = (TAF_MMA_CerssiInfoQryCnf *)msg;
    VOS_UINT32               result;

    /* 判断查询结果是否失败,如果失败则返回ERROR */
    if (cerssiInfoQueryCnf->errorCause != TAF_ERR_NO_ERROR) {
        return AT_ERROR;
    } else {
        switch (g_atClientTab[indexNum].cmdCurrentOpt) {
            case AT_CMD_CERSSI_READ:
                result = AT_ProcCerssiInfoQuery((struct MsgCB *)cerssiInfoQueryCnf, indexNum);
                break;

            case AT_CMD_CESQ_SET:
                result = AT_ProcCesqInfoSet((struct MsgCB *)cerssiInfoQueryCnf, indexNum);
                break;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
            case AT_CMD_CSERSSI_READ:
                result = AT_ProcCserssiInfoQuery((struct MsgCB *)cerssiInfoQueryCnf, indexNum);
                break;
#endif

            default:
                result = AT_ERROR;
        }
    }

    return result;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_ProcCserssiInfoQuery(struct MsgCB *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CerssiInfoQryCnf *cserssiInfoQueryCnf = (TAF_MMA_CerssiInfoQryCnf *)msg;
    AT_ModemNetCtx *netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);
    VOS_UINT8 *systemAppConfig = AT_GetSystemAppConfigAddr();
    VOS_UINT16 length = 0;
    VOS_INT16 rsrp;
    VOS_INT16 rsrq;

    rsrp = cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp == AT_SIGNAL_INVALID_VALUE ?
               cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp :
               cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrp / AT_SIGNAL_DIVISOR_EIGHT;
    rsrq = cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq == AT_SIGNAL_INVALID_VALUE ?
               cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq :
               cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rsrq / AT_SIGNAL_DIVISOR_EIGHT;

    if (cserssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_NR) {
        if (*systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99, 99, 99, 99, 99, 99,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrp,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrq,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.l5GSinr);

            g_atSendDataBuff.bufLen = length;

        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, cserssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrp,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.s5GRsrq,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.nrCellSignInfo.l5GSinr);

            g_atSendDataBuff.bufLen = length;
        }
    } else if (cserssiInfoQueryCnf->cerssi.ratType == TAF_MMA_RAT_LTE) {
        if (*systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, 0, 255, rsrp, rsrq, cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.sinr,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.ri,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[0],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[1],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rxANTNum,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR0],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR1],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR2],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR3],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR0],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR1],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR2],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR3], 0, 0, 0);

        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, 0, 255, rsrp, rsrq,  cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.sinr,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.ri,
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[0],
                cserssiInfoQueryCnf->cerssi.aRssi[0].u.lCellSignInfo.cqi.cqi[1], 0, 0, 0);
        }
        g_atSendDataBuff.bufLen = length;
    } else {
        if (*systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99, 99, 99, 99, 99, 99, 0, 0, 0);

            g_atSendDataBuff.bufLen = length;

        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval,
                0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 0, 0);

            g_atSendDataBuff.bufLen = length;
        }
    }

    return AT_OK;
}

VOS_VOID AT_Print5gPreferNssai(VOS_UINT8 indexNum, MTA_AT_5GNssaiQryCnf *pst5gNssaiQryCnf)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 nssaiLen;
    VOS_CHAR   acStrPreferNssai[AT_EVT_MULTI_S_NSSAI_LEN];

    /* +C5GPNSSAI:
     * <Preferred_NSSAI_3gpp_length>,<Preferred_NSSAI_3gpp>,<Preferred_NSSAI_non3gpp_length>,<Preferred_NSSAI_non3gpp>
     */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 组装切片消息格式 */
    nssaiLen = 0;
    (VOS_VOID)memset_s(acStrPreferNssai, sizeof(acStrPreferNssai), 0, sizeof(acStrPreferNssai));

    AT_ConvertMultiSNssaiToString(AT_MIN(pst5gNssaiQryCnf->st5gPreferNssai.snssaiNum, PS_MAX_CFG_S_NSSAI_NUM),
                                  &pst5gNssaiQryCnf->st5gPreferNssai.snssai[0], acStrPreferNssai,
                                  sizeof(acStrPreferNssai), &nssaiLen);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", nssaiLen);

    if (nssaiLen != 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acStrPreferNssai);
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_VOID AT_Print5gDefCfgNssai(VOS_UINT8 indexNum, MTA_AT_5GNssaiQryCnf *pst5gNssaiQryCnf)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 nssaiLen;
    VOS_CHAR   acStrDefCfgNssai[AT_EVT_MULTI_S_NSSAI_LEN];

    /* +C5GNSSAI: [<default_configured_nssai_length>,<default_configured_nssai>] */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 组装切片消息格式 */
    nssaiLen = 0;
    (VOS_VOID)memset_s(acStrDefCfgNssai, sizeof(acStrDefCfgNssai), 0, sizeof(acStrDefCfgNssai));

    AT_ConvertMultiSNssaiToString(AT_MIN(pst5gNssaiQryCnf->st5gDefCfgNssai.snssaiNum, PS_MAX_CFG_S_NSSAI_NUM),
                                  &pst5gNssaiQryCnf->st5gDefCfgNssai.snssai[0], acStrDefCfgNssai,
                                  sizeof(acStrDefCfgNssai), &nssaiLen);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", nssaiLen);

    if (nssaiLen != 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acStrDefCfgNssai);
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_VOID AT_ConvertMultiRejectSNssaiToString(VOS_UINT8 sNssaiNum, PS_REJECTED_SNssai *sNssai, VOS_CHAR *pcStrNssai,
                                             VOS_UINT32 srcNssaiLength, VOS_UINT32 *dsrLength)
{
    VOS_UINT32 length = 0;
    VOS_UINT32 loop;
    VOS_INT32  bufLen;

    /*
     * 27007 rel15, 10.1.1章节
     * sst                                     only slice/service type (SST) is present
     * sst.sd                                  SST and slice differentiator (SD) are present
     */

    *dsrLength = 0;

    for (loop = 0; loop < sNssaiNum; loop++) {
        if (length >= AT_EVT_MULTI_S_NSSAI_LEN) {
            AT_ERR_LOG1("AT_ConvertMultiRejectSNssaiToString :ERROR: ulLength abnormal:", length);
            *dsrLength = 0;

            return;
        }

        /* 如果有多个S-NSSAI，每个S-NSSAI通过":"分割 */
        if (loop != 0) {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1, ":");
            TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, srcNssaiLength - length, (srcNssaiLength - length) - 1);
            length = length + (VOS_UINT32)bufLen;
        }

        if (sNssai[loop].opSd == VOS_TRUE) {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1, "%02x.%06x#%x",
                                sNssai[loop].sst, sNssai[loop].sd, sNssai[loop].cause);
            TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, srcNssaiLength - length, (srcNssaiLength - length) - 1);
            length = length + (VOS_UINT32)bufLen;
            /* sst/sd共4个字节 */
            *dsrLength += 4;
        } else {
            bufLen = snprintf_s(pcStrNssai + length, srcNssaiLength - length, (srcNssaiLength - length) - 1, "%02x#%x",
                                sNssai[loop].sst, sNssai[loop].cause);
            TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, srcNssaiLength - length, (srcNssaiLength - length) - 1);
            length = length + (VOS_UINT32)bufLen;
            /* sst1个字节 */
            *dsrLength += 1;
        }
    }
}

VOS_VOID AT_Print5gNssaiRdpInfo(VOS_UINT8 indexNum, MTA_AT_5GNssaiQryCnf *pst5gNssaiQryCnf)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 nssaiLen;
    VOS_CHAR   acStrNssai[AT_EVT_MULTI_S_NSSAI_LEN];
    VOS_UINT32 i;

    /*
     * [+C5GNSSAIRDP:
     * [<default_configured_nssai_length>,<default_configured_nssai>[,<rejected_nssai_3gpp_length>,<rejected_nssai_3gpp>[,<rejected_nssai_non3gpp_length>,<rejected_nssai_non3gpp>]]]
     * [<CR><LF>+C5GNSSAIRDP:
     * <plmn_id>[,<configured_nssai_length>,<configured_nssai>[,<allowed_nssai_3gpp_length>,<allowed_nssai_3gpp>,<allowed_nssai_non3gpp_length>,<allowed_nssai_non3gpp>]]
     * [<CR><LF>+C5GNSSAIRDP:
     * <plmn_id>[,<configured_nssai_length>,<configured_nssai>[,<allowed_nssai_3gpp_length>,<allowed_nssai_3gpp>,<allowed_nssai_non3gpp_length>,<allowed_nssai_non3gpp>]]
     * [...]]]]
     */

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 组装Default切片消息格式 */
    nssaiLen = 0;
    (VOS_VOID)memset_s(acStrNssai, sizeof(acStrNssai), 0, sizeof(acStrNssai));

    AT_ConvertMultiSNssaiToString(AT_MIN(pst5gNssaiQryCnf->st5gDefCfgNssai.snssaiNum, PS_MAX_CFG_S_NSSAI_NUM),
        &pst5gNssaiQryCnf->st5gDefCfgNssai.snssai[0], acStrNssai, sizeof(acStrNssai), &nssaiLen);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", nssaiLen);

    if (nssaiLen != 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acStrNssai);
    }

    if (pst5gNssaiQryCnf->sNssaiQryType > AT_MTA_NSSAI_QRY_DEF_CFG_NSSAI_ONLY) {
        /* 组装reject 3gpp切片消息格式 */
        nssaiLen = 0;
        (VOS_VOID)memset_s(acStrNssai, sizeof(acStrNssai), 0, sizeof(acStrNssai));

        AT_ConvertMultiRejectSNssaiToString(AT_MIN(pst5gNssaiQryCnf->st5gRejNssai.snssaiNum, PS_MAX_REJECT_S_NSSAI_NUM),
            &pst5gNssaiQryCnf->st5gRejNssai.snssai[0], acStrNssai, sizeof(acStrNssai), &nssaiLen);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,", nssaiLen);

        if (nssaiLen != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acStrNssai);
        }
    }

    if (pst5gNssaiQryCnf->sNssaiQryType <= AT_MTA_NSSAI_QRY_DEF_CFG_AND_REJ_NSSAI) {
        /* 如果查询type是default config nssai only或者default config nssai and reject nssai，查询完毕，则直接提前return */
        g_atSendDataBuff.bufLen = length;
        return;
    }

    pst5gNssaiQryCnf->st5gAllowCfgNssai.plmnNum = AT_MIN(pst5gNssaiQryCnf->st5gAllowCfgNssai.plmnNum,
                                                         MTA_AT_MAX_ALLOWED_AND_CFG_NSSAI_PLMN_NUM);

    for (i = 0; i < pst5gNssaiQryCnf->st5gAllowCfgNssai.plmnNum; i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* 打印输出PLMN ID */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"%X%X%X",
            (0x0f & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_PLMNID]),
            (0xf0 & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_PLMNID]) >> 4,
            (0x0f & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_PLMNSTATUS]));

        /* MNC为两位 */
        if ((0xf0 & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_PLMNSTATUS]) == 0xf0) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%X%X\"",
                (0x0f & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_ACCESS_MODE]),
                (0xf0 & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_ACCESS_MODE]) >> 4);
        }
        /* MNC为三位 */
        else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "%X%X%X\"",
                    (0x0f & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_ACCESS_MODE]),
                    (0xf0 & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_ACCESS_MODE]) >> 4,
                    (0xf0 & pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].plmnId.plmnId[AT_PLMN_PLMNSTATUS]) >> 4);
        }

        /* 组装config切片消息格式 */
        nssaiLen = 0;
        (VOS_VOID)memset_s(acStrNssai, sizeof(acStrNssai), 0, sizeof(acStrNssai));

        AT_ConvertMultiSNssaiToString(AT_MIN(pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].cfgNssai.snssaiNum,
            PS_MAX_CFG_S_NSSAI_NUM), &pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].cfgNssai.snssai[0], acStrNssai,
            sizeof(acStrNssai), &nssaiLen);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,", nssaiLen);

        if (nssaiLen != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acStrNssai);
        }

        if (pst5gNssaiQryCnf->sNssaiQryType == AT_MTA_NSSAI_QRY_DEF_CFG_AND_REJ_AND_CFG_NSSAI) {
            continue;
        }

        /* 组装Allowed切片消息格式 */
        nssaiLen = 0;
        (VOS_VOID)memset_s(acStrNssai, sizeof(acStrNssai), 0, sizeof(acStrNssai));

        AT_ConvertMultiSNssaiToString(AT_MIN(pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].allowNssai.snssaiNum,
            PS_MAX_ALLOWED_S_NSSAI_NUM), &pst5gNssaiQryCnf->st5gAllowCfgNssai.nssai[i].allowNssai.snssai[0], acStrNssai,
            sizeof(acStrNssai), &nssaiLen);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,", nssaiLen);

        if (nssaiLen != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acStrNssai);
        }
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_RcvMta5gNssaiQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *rcvMsg = VOS_NULL_PTR;
    MTA_AT_5GNssaiQryCnf *qryCnf = VOS_NULL_PTR;
    VOS_UINT32            rslt = AT_ERROR;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    qryCnf   = (MTA_AT_5GNssaiQryCnf *)rcvMsg->content;

    /* 判断查询结果是否失败,如果失败则返回ERROR */
    if (qryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        g_atSendDataBuff.bufLen = 0;
    } else {
        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_C5GPNSSAI_QRY) {
            AT_Print5gPreferNssai(indexNum, qryCnf);
        }

        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_C5GNSSAI_QRY) {
            AT_Print5gDefCfgNssai(indexNum, qryCnf);
        }

        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_C5GNSSAIRDP_SET) {
            AT_Print5gNssaiRdpInfo(indexNum, qryCnf);
        }

        rslt = AT_OK;
    }

    return rslt;
}
#endif

VOS_UINT32 atCerssiInfoCnfProc(struct MsgCB *msgBlock)
{
    L4A_CSQ_InfoCnf *cerssi   = NULL;
    VOS_UINT16       length   = 0;
    AT_ModemNetCtx  *netCtx   = VOS_NULL_PTR;
    VOS_UINT8        indexNum = 0;

    cerssi = (L4A_CSQ_InfoCnf *)msgBlock;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(cerssi->clientId, &indexNum) == AT_FAILURE) {
        AT_PR_LOGI("WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if (cerssi->errorCode == 0) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr, "%s%s%d,%d,0,0,255,%d,%d,%d,%d,%d,%d%s", g_atCrLf, "^CERSSI:",
            netCtx->cerssiReportType, netCtx->cerssiMinTimerInterval, cerssi->rsrp, cerssi->rsrq, cerssi->sinr,
            cerssi->cqi.ri, cerssi->cqi.cqi[0], cerssi->cqi.cqi[1], g_atCrLf);
    }

    CmdErrProc((VOS_UINT8)(cerssi->clientId), cerssi->errorCode, length, g_atSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

