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
#include "at_phone_event_report.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_event_report.h"
#include "at_phone_comm.h"
#include "taf_std_lib.h"
#include "at_lte_common.h"
#include "osm.h"
#include "at_custom_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_EVENT_REPORT_C

#define AT_RX_RSRPR0 0  /* 天线 0 rsrp， 单位 dB */
#define AT_RX_RSRPR1 1  /* 天线 1 rsrp， 单位 dB */
#define AT_RX_RSRPR2 2  /* 天线 2 rsrp， 单位 dB，仅 4收时有效 */
#define AT_RX_RSRPR3 3  /* 天线 3 rsrp， 单位 dB，仅 4收时有效 */

#define AT_RX_SINRR0 0  /* 天线 0 sinr */
#define AT_RX_SINRR1 1  /* 天线 1 sinr */
#define AT_RX_SINRR2 2  /* 天线 2 sinr */
#define AT_RX_SINRR3 3  /* 天线 3 sinr */
#define AT_TMP_ARRAY_LEN 4
#define AT_TMP_ARRAY_INDEX_0 0
#define AT_TMP_ARRAY_INDEX_1 1
#define AT_TMP_ARRAY_INDEX_2 2
#define AT_CTZE_DEFAULT_YEAR 2000
#define AT_HCSQ_LEVEL_MIN 0
#define AT_HCSQ_VALUE_INVALID 255
#define AT_HCSQ_RSSI_VALUE_MIN (-120)
#define AT_HCSQ_RSSI_VALUE_MAX (-25)
#define AT_HCSQ_RSSI_LEVEL_MAX 96

VOS_VOID AT_RcvMmaGsmCERssiChangeInd(TAF_UINT8 indexNum, TAF_MMA_RssiInfoInd *rssiInfoInd,
                                           VOS_UINT8 systemAppConfig)
{
    VOS_UINT16 length = 0;

    if (systemAppConfig != SYSTEM_APP_ANDROID) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CERSSI].text, rssiInfoInd->rssiInfo.aRssi[0].u.gCellSignInfo.rssiValue, 0, 255, 0,
            0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99, 99, 99, 99, 99, 99, 0, 0, 0, g_atCrLf);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CERSSI].text, rssiInfoInd->rssiInfo.aRssi[0].u.gCellSignInfo.rssiValue, 0, 255, 0,
            0, 0, 0, 0x7f7f, 0x7f7f, 0, 0, 0, g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_VOID AT_RcvMmaWcdmaCERssiChangeInd(TAF_UINT8 indexNum, TAF_MMA_RssiInfoInd *rssiInfoInd,
                                             VOS_UINT8 systemAppConfig)
{
    VOS_UINT16 length = 0;

    if (rssiInfoInd->rssiInfo.currentUtranMode == TAF_UTRANCTRL_UTRAN_MODE_FDD) {
        if (systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CERSSI].text, 0, rssiInfoInd->rssiInfo.aRssi[0].u.wCellSignInfo.rscpValue,
                rssiInfoInd->rssiInfo.aRssi[0].u.wCellSignInfo.ecioValue, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99,
                99, 99, 99, 99, 99, 0, 0, 0, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CERSSI].text, 0, rssiInfoInd->rssiInfo.aRssi[0].u.wCellSignInfo.rscpValue,
                rssiInfoInd->rssiInfo.aRssi[0].u.wCellSignInfo.ecioValue, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 0, 0,
                g_atCrLf);
        }
    } else {
        if (systemAppConfig != SYSTEM_APP_ANDROID) {
            /* 非fdd 3g 小区，ecio值为无效值255 */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CERSSI].text, 0, rssiInfoInd->rssiInfo.aRssi[0].u.wCellSignInfo.rscpValue,
                255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99, 99, 99, 99, 99, 99, 0, 0, 0, g_atCrLf);
        } else {
            /* 非fdd 3g 小区，ecio值为无效值255 */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CERSSI].text, 0, rssiInfoInd->rssiInfo.aRssi[0].u.wCellSignInfo.rscpValue,
                255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 0, 0, g_atCrLf);
        }
    }
    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

#if (FEATURE_LTE == FEATURE_ON)

VOS_VOID AT_RcvMmaLteCERssiChangeInd(TAF_UINT8 indexNum, TAF_MMA_RssiInfoInd *rssiInfoInd,
                                           VOS_UINT8 systemAppConfig)
{
    VOS_UINT16 length = 0;

    if (systemAppConfig != SYSTEM_APP_ANDROID) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CERSSI].text, 0, 0, 255, rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rsrp,
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rsrq, rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.sinr,
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.cqi.ri,
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.cqi.cqi[0],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.cqi.cqi[1],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.rxANTNum,
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR0],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR1],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR2],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.rsrpRx[AT_RX_RSRPR3],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR0],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR1],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR2],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rxAntInfo.sinrRx[AT_RX_SINRR3], 0, 0, 0, g_atCrLf);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CERSSI].text, 0, 0, 255, rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rsrp,
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rsrq, rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.sinr,
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.cqi.ri,
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.cqi.cqi[0],
            rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.cqi.cqi[1], 0, 0, 0, g_atCrLf);
    }
    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_VOID AT_RcvMmaNrCERssiChangeInd(TAF_UINT8 indexNum, TAF_MMA_RssiInfoInd *rssiInfoInd,
                                          VOS_UINT8 systemAppConfig)
{
    VOS_UINT16 length = 0;

    /* SA, use ^CERSSI to report */
    if (rssiInfoInd->isNsaRptFlg == VOS_FALSE) {
        if (systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CERSSI].text, 0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99, 99, 99, 99,
                99, 99, rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrp,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrq,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.l5GSinr, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CERSSI].text, 0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrp,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrq,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.l5GSinr, g_atCrLf);
        }
        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    /* NSA, use ^CSERSSI to report */
    else {
        if (systemAppConfig != SYSTEM_APP_ANDROID) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length,
                "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CSERSSI].text, 0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f, 0, 99, 99, 99, 99, 99,
                99, 99, 99, rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrp,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrq,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.l5GSinr, g_atCrLf);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
                g_atStringTab[AT_STRING_CSERSSI].text, 0, 0, 255, 0, 0, 0, 0, 0x7f7f, 0x7f7f,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrp,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.s5GRsrq,
                rssiInfoInd->rssiInfo.aRssi[0].u.nrCellSignInfo.l5GSinr, g_atCrLf);
        }

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }
}
#endif

VOS_VOID AT_RcvMmaRssiChangeInd(TAF_UINT8 indexNum, TAF_MMA_RssiInfoInd *rssiInfoInd)
{
    VOS_UINT16 length;
    VOS_UINT32 rptCmdRssi;
    VOS_UINT32 rptCmdCerssi;
#if (FEATURE_LTE == FEATURE_ON)
    VOS_UINT32 rptCmdAnlevel;
    VOS_INT16  rsrp;
    VOS_INT16  rsrq;
    VOS_UINT8  level;
    VOS_INT16  rssi;
#endif
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    length = 0;

    rptCmdRssi    = AT_CheckRptCmdStatus(rssiInfoInd->curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_RSSI);
    rptCmdCerssi  = AT_CheckRptCmdStatus(rssiInfoInd->curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CERSSI);
    rptCmdAnlevel = AT_CheckRptCmdStatus(rssiInfoInd->curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_ANLEVEL);

    if ((AT_CheckRptCmdStatus(rssiInfoInd->unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_RSSI) ==
         VOS_TRUE) &&
        (rptCmdRssi == VOS_TRUE)) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_RSSI].text,
            rssiInfoInd->rssiInfo.aRssi[0].rssiValue, g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    if ((AT_CheckRptCmdStatus(rssiInfoInd->unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CERSSI) ==
         VOS_TRUE) &&
        (rptCmdCerssi == VOS_TRUE)) {
        systemAppConfig = AT_GetSystemAppConfigAddr();

        if (rssiInfoInd->rssiInfo.ratType == TAF_MMA_RAT_GSM) {
            AT_RcvMmaGsmCERssiChangeInd(indexNum, rssiInfoInd, *systemAppConfig);

            return;
        }

        if (rssiInfoInd->rssiInfo.ratType == TAF_MMA_RAT_WCDMA) {
            AT_RcvMmaWcdmaCERssiChangeInd(indexNum, rssiInfoInd, *systemAppConfig);

            return;
        }

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (rssiInfoInd->rssiInfo.ratType == TAF_MMA_RAT_NR) {
            AT_RcvMmaNrCERssiChangeInd(indexNum, rssiInfoInd, *systemAppConfig);

            return;
        }
#endif

        /* 上报LTE 的CERSSI */
#if (FEATURE_LTE == FEATURE_ON)
        if (rssiInfoInd->rssiInfo.ratType == TAF_MMA_RAT_LTE) {
            AT_RcvMmaLteCERssiChangeInd(indexNum, rssiInfoInd, *systemAppConfig);
        }
#endif
    }

    /* 上报ANLEVEL */
#if (FEATURE_LTE == FEATURE_ON)

    length = 0;
    if (rptCmdAnlevel == VOS_TRUE) {
        rsrp = rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rsrp;
        rsrq = rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rsrq;
        rssi = rssiInfoInd->rssiInfo.aRssi[0].u.lCellSignInfo.rssi;

        AT_CalculateLTESignalValue(&rssi, &level, &rsrp, &rsrq);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s0,99,%d,%d,%d,%d%s", g_atCrLf, "^ANLEVEL:", rssi, level,
            rsrp, rsrq, g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }
#endif
}

VOS_UINT32 atAnlevelInfoIndProc(struct MsgCB *msgBlock)
{
    VOS_UINT16        length = 0;
    VOS_UINT32        result;
    VOS_UINT16        rsrp               = 0;
    VOS_UINT16        rsrq               = 0;
    AT_AnlevelInfoCnf anlevelAnqueryInfo = {0};

    result = at_CsqInfoProc(msgBlock, &anlevelAnqueryInfo);

    if (result == ERR_MSP_SUCCESS) {
        rsrp = (VOS_UINT16)((anlevelAnqueryInfo.rsrp == AT_ANLEVEL_INFO_RSRP_UNVALID) ?
                AT_ANLEVEL_INFO_RSRP_UNVALID : (-(anlevelAnqueryInfo.rsrp)));
        rsrq = (VOS_UINT16)((anlevelAnqueryInfo.rsrq == AT_ANLEVEL_INFO_RSRQ_UNVALID) ?
                AT_ANLEVEL_INFO_RSRQ_UNVALID : (-(anlevelAnqueryInfo.rsrq)));

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "%s%s0,99,%d,%d,%d,%d%s", g_atCrLf, "^ANLEVEL:",
            anlevelAnqueryInfo.rssi, anlevelAnqueryInfo.level, rsrp, rsrq, g_atCrLf);
        At_SendResultData(AT_CLIENT_ID_APP, g_atSndCodeAddr, length);
    } else {
        HAL_SDMLOG("ulResult = %d,\n", (VOS_INT)result);
    }

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 AT_RcvMmaRssiInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RssiInfoInd *rssiInfoInd = (TAF_MMA_RssiInfoInd *)msg;

    AT_RcvMmaRssiChangeInd(indexNum, rssiInfoInd);

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* ^HCSQ主动上报 */
    AT_RptHcsqInfo(indexNum, rssiInfoInd);
#endif

    return VOS_OK;
}

VOS_VOID AT_RcvMmaNsmStatusInd(TAF_UINT8 indexNum, TAF_PHONE_EventInfo *event)
{
    VOS_UINT16 length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s+PACSP", g_atCrLf);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d", event->plmnMode);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

VOS_VOID AT_PhSendRoaming(VOS_UINT8 tmpRoamStatus)
{
    VOS_UINT32 i;
    VOS_UINT16 length;
    VOS_UINT8  roamStatus = tmpRoamStatus;

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        if (g_atClientTab[i].userType == AT_APP_USER) {
            break;
        }
    }

    /* 未找到E5 User,则不用上报 */
    if (i >= AT_MAX_CLIENT_NUM) {
        return;
    }

    length = 0;
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^APROAMRPT:%d%s", g_atCrLf, roamStatus, g_atCrLf);

    At_SendResultData((VOS_UINT8)i, g_atSndCodeAddress, length);
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_VOID AT_ReportCeregResult(VOS_UINT8 indexNum, TAF_MMA_RegStatusInd *regInd, VOS_UINT16 *length)
{
    VOS_UINT32      rst;
    ModemIdUint16   modemId = MODEM_ID_0;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    rst = AT_GetModemIdFromClient(indexNum, &modemId);
    if (rst != VOS_OK) {
        AT_ERR_LOG1("AT_ReportCeregResult:Get ModemID From ClientID fail,ClientID:", indexNum);
        return;
    }

    /* 当前平台是否支持LTE */
    if (AT_IsModemSupportRat(modemId, TAF_MMA_RAT_LTE) != VOS_TRUE) {
        return;
    }

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    if ((netCtx->ceregType == AT_CEREG_RESULT_CODE_BREVITE_TYPE) && (regInd->regStatus.OP_PsRegState == VOS_TRUE)) {
        /* +CEREG: <stat> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_CEREG].text,
            regInd->regStatus.psRegState, g_atCrLf);
    } else if ((netCtx->ceregType == AT_CEREG_RESULT_CODE_ENTIRE_TYPE) &&
               (regInd->regStatus.OP_PsRegState == VOS_TRUE)) {
        if ((regInd->regStatus.psRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK) ||
            (regInd->regStatus.psRegState == TAF_PH_REG_REGISTERED_ROAM)) {
            /* +CEREG: <stat>[,<lac>,<ci>,[rat]] */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d", g_atCrLf, g_atStringTab[AT_STRING_CEREG].text,
                regInd->regStatus.psRegState);

            /* GU下只上报+CGREG: <stat> */
            if (regInd->regStatus.ratType == TAF_PH_INFO_LTE_RAT) {
                *length += (VOS_UINT16)At_PhReadCreg(&(regInd->regStatus), g_atSndCodeAddress + *length);
            }

            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
        } else {
            /* +CEREG: <stat> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_CEREG].text,
                regInd->regStatus.psRegState, g_atCrLf);
        }
    } else {
    }
}
#endif

VOS_VOID AT_ReportCgregResult(VOS_UINT8 indexNum, TAF_MMA_RegStatusInd *regInd, VOS_UINT16 *length)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if ((netCtx->cgregType == AT_CGREG_RESULT_CODE_BREVITE_TYPE) && (regInd->regStatus.OP_PsRegState == VOS_TRUE)) {
        /* +CGREG: <stat> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_CGREG].text,
            regInd->regStatus.psRegState, g_atCrLf);
    } else if ((netCtx->cgregType == AT_CGREG_RESULT_CODE_ENTIRE_TYPE) &&
               (regInd->regStatus.OP_PsRegState == VOS_TRUE)) {
        if (((regInd->regStatus.psRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK) ||
             (regInd->regStatus.psRegState == TAF_PH_REG_REGISTERED_ROAM)) &&
            (regInd->regStatus.ratType != TAF_PH_INFO_NR_5GC_RAT)) {
            /* +CGREG: <stat>[,<lac>,<ci>,[rat]] */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d", g_atCrLf, g_atStringTab[AT_STRING_CGREG].text,
                regInd->regStatus.psRegState);

            *length += (VOS_UINT16)At_PhReadCreg(&(regInd->regStatus), g_atSndCodeAddress + *length);

            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
        } else {
            /* +CGREG: <stat> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_CGREG].text,
                regInd->regStatus.psRegState, g_atCrLf);
        }
    } else {
    }
}

VOS_VOID AT_ReportCregResult(VOS_UINT8 indexNum, TAF_MMA_RegStatusInd *regInd, VOS_UINT16 *length)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if ((netCtx->cregType == AT_CREG_RESULT_CODE_BREVITE_TYPE) && (regInd->regStatus.OP_CsRegState == VOS_TRUE)) {
        /* +CREG: <stat> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_CREG].text,
            regInd->regStatus.regState, g_atCrLf);
    } else if ((netCtx->cregType == AT_CREG_RESULT_CODE_ENTIRE_TYPE) && (regInd->regStatus.OP_CsRegState == VOS_TRUE)) {
        if ((regInd->regStatus.regState == TAF_PH_REG_REGISTERED_HOME_NETWORK) ||
            (regInd->regStatus.regState == TAF_PH_REG_REGISTERED_ROAM)) {
            /* +CREG: <stat>[,<lac>,<ci>,[rat]] */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d", g_atCrLf, g_atStringTab[AT_STRING_CREG].text,
                regInd->regStatus.regState);

            *length += (VOS_UINT16)At_PhReadCreg(&(regInd->regStatus), (g_atSndCodeAddress + *length));

            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
        } else {
            /* +CREG: <stat> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_CREG].text,
                regInd->regStatus.regState, g_atCrLf);
        }
    } else {
    }
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

LOCAL VOS_VOID AT_ReportC5gregResult(VOS_UINT8 indexNum, TAF_MMA_RegStatusInd *regInd, VOS_UINT16 *length)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    VOS_UINT32      rst;
    ModemIdUint16   modemId = MODEM_ID_0;
    VOS_UINT32      lengthTemp;
    VOS_CHAR        acStrAllowedNssai[AT_EVT_MULTI_S_NSSAI_LEN];

    rst = AT_GetModemIdFromClient(indexNum, &modemId);
    if (rst != VOS_OK) {
        AT_ERR_LOG1("AT_ReportC5gregResult: ERROR: Get ModemID From ClientID fail,ClientID:", indexNum);
        return;
    }

    /* 当前平台是否支持NR */
    if (AT_IsModemSupportRat(modemId, TAF_MMA_RAT_NR) != VOS_TRUE) {
        return;
    }

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    if ((netCtx->c5gregType == AT_C5GREG_RESULT_CODE_BREVITE_TYPE) && (regInd->regStatus.OP_PsRegState == VOS_TRUE)) {
        /* +C5GREG: <stat> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_C5GREG].text,
            regInd->regStatus.psRegState, g_atCrLf);
    } else if ((netCtx->c5gregType == AT_C5GREG_RESULT_CODE_ENTIRE_TYPE) &&
               (regInd->regStatus.OP_PsRegState == VOS_TRUE)) {
        if ((regInd->regStatus.psRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK) ||
            (regInd->regStatus.psRegState == TAF_PH_REG_REGISTERED_ROAM)) {
            /* +C5GREG:
             * <stat>[,<lac>,<ci>,[rat],[<Allowed_NSSAI_length>],<Allowed_NSSAI>][,<cause_type>,<reject_cause>]]   */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                              (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d", g_atCrLf,
                                              g_atStringTab[AT_STRING_C5GREG].text, regInd->regStatus.psRegState);

            if (regInd->regStatus.ratType == TAF_PH_INFO_NR_5GC_RAT) {
                *length += (VOS_UINT16)At_PhReadC5greg(&(regInd->regStatus), g_atSndCodeAddress + *length);

                *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                                  (VOS_CHAR *)g_atSndCodeAddress + *length, ",");

                /* 组装切片消息格式 */
                lengthTemp = 0;
                memset_s(acStrAllowedNssai, sizeof(acStrAllowedNssai), 0, sizeof(acStrAllowedNssai));

                AT_ConvertMultiSNssaiToString(AT_MIN(regInd->regStatus.allowedNssai.snssaiNum,
                                                     PS_MAX_ALLOWED_S_NSSAI_NUM),
                                              &regInd->regStatus.allowedNssai.snssai[0], acStrAllowedNssai,
                                              sizeof(acStrAllowedNssai), &lengthTemp);

                *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                                  (VOS_CHAR *)g_atSndCodeAddress + *length, "%d,", lengthTemp);

                if (lengthTemp != 0) {
                    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                                      (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%s\"",
                                                      acStrAllowedNssai);
                }
            }

            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
        } else {
            /* +C5GREG: <stat> */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                                              (VOS_CHAR *)g_atSndCodeAddress + *length, "%s%s%d%s", g_atCrLf,
                                              g_atStringTab[AT_STRING_C5GREG].text, regInd->regStatus.psRegState,
                                              g_atCrLf);
        }
    } else {
    }
}

#endif

VOS_UINT32 AT_RcvMmaRegStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RegStatusInd *regStatusInd = (TAF_MMA_RegStatusInd *)msg;
    VOS_UINT16 length = 0;

    AT_ReportCregResult(indexNum, regStatusInd, &length);

    AT_ReportCgregResult(indexNum, regStatusInd, &length);

#if (FEATURE_LTE == FEATURE_ON)
    /* 通过NV判断当前是否支持LTE */
    AT_ReportCeregResult(indexNum, regStatusInd, &length);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /* 通过NV判断当前是否支持NR */
    AT_ReportC5gregResult(indexNum, regStatusInd, &length);
#endif

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_VOID AT_ConvertNasMncToBcdType(VOS_UINT32 nasMnc, VOS_UINT32 *mnc)
{
    VOS_UINT32 i;
    VOS_UINT8  tmp[AT_TMP_ARRAY_LEN];

    *mnc = 0;

    for (i = 0; i < AT_TMP_ARRAY_LEN - 1; i++) {
        tmp[i] = nasMnc & 0x0f;
        nasMnc >>= 8;
    }

    if (tmp[AT_TMP_ARRAY_INDEX_2] == 0xf) {
        *mnc = ((VOS_UINT32)tmp[AT_TMP_ARRAY_INDEX_0] << 4) |
                ((VOS_UINT32)tmp[AT_TMP_ARRAY_INDEX_1]) | 0xf00;
    } else {
        *mnc = ((VOS_UINT32)tmp[AT_TMP_ARRAY_INDEX_0] << 8) |
                ((VOS_UINT32)tmp[AT_TMP_ARRAY_INDEX_1] << 4) | tmp[AT_TMP_ARRAY_INDEX_2];
    }
}

VOS_VOID AT_ConvertNasMccToBcdType(VOS_UINT32 nasMcc, VOS_UINT32 *mcc)
{
    VOS_UINT32 i;
    VOS_UINT8  tmp[AT_TMP_ARRAY_LEN];

    *mcc = 0;

    for (i = 0; i < AT_TMP_ARRAY_LEN - 1; i++) {
        tmp[i] = nasMcc & 0x0f;
        nasMcc >>= 8;
    }

    *mcc = ((VOS_UINT32)tmp[AT_TMP_ARRAY_INDEX_0] << 8) |
            ((VOS_UINT32)tmp[AT_TMP_ARRAY_INDEX_1] << 4) | tmp[AT_TMP_ARRAY_INDEX_2];
}

VOS_UINT32 AT_RcvTafCcmEccNumInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EccNumInd *eccNumInfo = (TAF_CCM_EccNumInd *)msg;
    VOS_UINT32         i;
    VOS_UINT16         length ;

    /* 向APP逐条上报紧急呼号码 */
    eccNumInfo->eccNumInd.eccNumCount = AT_MIN(eccNumInfo->eccNumInd.eccNumCount, MN_CALL_MAX_EMC_NUM);
    for (i = 0; i < eccNumInfo->eccNumInd.eccNumCount; i++) {
        length = AT_FormatOneXlemaPara(&(eccNumInfo->eccNumInd.customEccNumList[i]),
            eccNumInfo->eccNumInd.eccNumCount, i);

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaModeChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ModeChangeInd *rcvMsg   = (TAF_MMA_ModeChangeInd *)msg;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s%s%d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_MODE].text,
        rcvMsg->ratType, rcvMsg->sysSubMode, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaRegRejInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RegRejInfoInd *regRejInd = (TAF_MMA_RegRejInfoInd *)msg;
    VOS_UINT16             length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s", g_atCrLf, g_atStringTab[AT_STRING_REJINFO].text);

    /* 打印rejinfo AT命令参数 */
    length = AT_PrintRejinfo(length, &(regRejInd->regRejInfo));

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaPlmnChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_PlmnChangeInd *rcvMsg = (TAF_MMA_PlmnChangeInd *)msg;
    VOS_UINT16 length = 0;

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^PLMN:", g_atCrLf);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x%x,", (rcvMsg->curPlmn.mcc & 0x0f00) >> 8,
        (rcvMsg->curPlmn.mcc & 0xf0) >> 4, (rcvMsg->curPlmn.mcc & 0x0f));

    if ((rcvMsg->curPlmn.mnc & 0x0f00) == 0x0f00) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x", (rcvMsg->curPlmn.mnc & 0xf0) >> 4,
            (rcvMsg->curPlmn.mnc & 0x0f));
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x%x", (rcvMsg->curPlmn.mnc & 0x0f00) >> 8,
            (rcvMsg->curPlmn.mnc & 0xf0) >> 4, (rcvMsg->curPlmn.mnc & 0x0f));
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData((VOS_UINT8)indexNum, g_atSndCodeAddress, length);
    return VOS_TRUE;
}

VOS_UINT32 AT_RcvMmaSrvStatusInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_SrvStatusInd *srvStatusInd = (TAF_MMA_SrvStatusInd *)msg;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_SRVST].text,
        srvStatusInd->srvStatus, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaAcInfoChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_AcInfoChangeInd *acInfoChangeInd = (TAF_MMA_AcInfoChangeInd *)msg;
    TAF_MMA_CellAcInfo      *cellAcInfo = (TAF_MMA_CellAcInfo *)(&acInfoChangeInd->cellAcInfo);

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,%d,%d,%d%s", g_atCrLf,
        g_atStringTab[AT_STRING_ACINFO].text, cellAcInfo->srvDomain, /* 上报服务域 */
        cellAcInfo->cellAcType,        /* 上报小区禁止接入类型 */
        cellAcInfo->restrictRegister,  /* 上报是否注册受限 */
        cellAcInfo->restrictPagingRsp, /* 上报是否寻呼受限 */
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_VOID AT_ConvertRssiLevel(VOS_INT16 rssiValue, VOS_UINT8 *rssiLevel)
{
    if (rssiValue >= AT_HCSQ_VALUE_INVALID) {
        *rssiLevel = AT_HCSQ_VALUE_INVALID;
    } else if (rssiValue >= AT_HCSQ_RSSI_VALUE_MAX) {
        *rssiLevel = AT_HCSQ_RSSI_LEVEL_MAX;
    } else if (rssiValue < AT_HCSQ_RSSI_VALUE_MIN) {
        *rssiLevel = AT_HCSQ_LEVEL_MIN;
    } else {
        *rssiLevel = (VOS_UINT8)((rssiValue - AT_HCSQ_RSSI_VALUE_MIN) + 1);
    }
}

VOS_UINT32 AT_RcvMmaEmRssiRptInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RssiInfoInd *emRssiRptInd = (TAF_MMA_RssiInfoInd *)msg;
    VOS_UINT8            rssiValue    = 0;
    VOS_INT16            emRssi       = 0;

    switch (emRssiRptInd->rssiInfo.ratType) {
        case TAF_MMA_RAT_GSM: {
            emRssi = emRssiRptInd->rssiInfo.aRssi[0].u.gCellSignInfo.rssiValue;
            break;
        }
        case TAF_MMA_RAT_WCDMA: {
            emRssi = emRssiRptInd->rssiInfo.aRssi[0].u.wCellSignInfo.rscpValue -
                     emRssiRptInd->rssiInfo.aRssi[0].u.wCellSignInfo.ecioValue;
            break;
        }
        case TAF_MMA_RAT_LTE: {
            emRssi = emRssiRptInd->rssiInfo.aRssi[0].u.lCellSignInfo.rssi;
            break;
        }
        default: {
            return VOS_ERR;
        }
    }

    AT_ConvertRssiLevel(emRssi, &rssiValue);

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_EMRSSIRPT].text,
        emRssiRptInd->rssiInfo.ratType, rssiValue, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

TAF_UINT32 At_PrintTimeZoneInfo(NAS_MM_InfoInd *mmInfo, VOS_UINT8 *dst)
{
    VOS_INT8   timeZone = AT_INVALID_TZ_VALUE;
    VOS_UINT8  timeZoneValue;
    VOS_UINT16 length = 0;

    /* 获取网络上报的时区信息 */
    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        timeZone = mmInfo->universalTimeandLocalTimeZone.timeZone;
    }

    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ) {
        timeZone = mmInfo->localTimeZone;
    }

    if (timeZone < 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, "-");

        timeZoneValue = (VOS_UINT8)(timeZone * (-1));
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, "+");

        timeZoneValue = (VOS_UINT8)timeZone;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%02d", timeZoneValue);

    /* 结尾 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "\"%s", g_atCrLf);
    return length;
}

LOCAL VOS_VOID AT_TimeZoneDisplayFormatCtzv(TAF_UINT16 *length, TAF_AT_CommTime *mmTimeInfo, TAF_UINT8 *dst,
    AT_ModemNetCtx *netCtx)
{
    VOS_UINT32      chkCtzvFlg;
    TAF_INT8        timeZone;

    chkCtzvFlg = AT_CheckRptCmdStatus(mmTimeInfo->unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CTZV);

    if ((AT_CheckRptCmdStatus(mmTimeInfo->curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CTZV) == VOS_TRUE) &&
        (chkCtzvFlg == VOS_TRUE)) {
        if ((mmTimeInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
            timeZone = mmTimeInfo->universalTimeandLocalTimeZone.timeZone;
        } else {
            timeZone = mmTimeInfo->localTimeZone;
        }

        if (timeZone != netCtx->timeInfo.localTimeZone) {
            /* 保存网络下发的时区信息 */
            netCtx->timeInfo.ieFlg |= NAS_MM_INFO_IE_LTZ;
            netCtx->timeInfo.localTimeZone                          = timeZone;
            netCtx->timeInfo.universalTimeandLocalTimeZone.timeZone = timeZone;

            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)dst + *length, "%s%s\"", g_atCrLf, g_atStringTab[AT_STRING_CTZV].text);

            *length += (VOS_UINT16)At_PrintTimeZoneInfo(&(netCtx->timeInfo), dst + *length);
        }
    }
}

VOS_UINT16 AT_PrintLocalTimeWithCtzeType(TAF_AT_CommTime *mmInfo, VOS_UINT8 *dst)
{
    VOS_UINT16 length = 0;

    /* YY */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, ",\"%d/", mmInfo->universalTimeandLocalTimeZone.year);
    /* MM */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%d%d/", mmInfo->universalTimeandLocalTimeZone.month / AT_DECIMAL_BASE_NUM,
        mmInfo->universalTimeandLocalTimeZone.month % AT_DECIMAL_BASE_NUM);
    /* dd */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%d%d,", mmInfo->universalTimeandLocalTimeZone.day / AT_DECIMAL_BASE_NUM,
        mmInfo->universalTimeandLocalTimeZone.day % AT_DECIMAL_BASE_NUM);

    /* hh */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%d%d:", mmInfo->universalTimeandLocalTimeZone.hour / AT_DECIMAL_BASE_NUM,
        mmInfo->universalTimeandLocalTimeZone.hour % AT_DECIMAL_BASE_NUM);

    /* mm */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%d%d:", mmInfo->universalTimeandLocalTimeZone.minute / AT_DECIMAL_BASE_NUM,
        mmInfo->universalTimeandLocalTimeZone.minute % AT_DECIMAL_BASE_NUM);

    /* ss */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%d%d\"", mmInfo->universalTimeandLocalTimeZone.second / AT_DECIMAL_BASE_NUM,
        mmInfo->universalTimeandLocalTimeZone.second % AT_DECIMAL_BASE_NUM);

    return length;
}

VOS_UINT64 AT_TransUniversalTimeToUTC(TAF_AT_CommTime *mmInfo)
{
    TAF_STD_TimeZoneType universalTime = {0};

    /* 输入的year值为两位数，默认从2000年开始计算 */
    universalTime.year = (VOS_UINT16)(mmInfo->universalTimeandLocalTimeZone.year + AT_CTZE_DEFAULT_YEAR);
    universalTime.month = (VOS_UINT16)mmInfo->universalTimeandLocalTimeZone.month;
    universalTime.day = (VOS_UINT16)mmInfo->universalTimeandLocalTimeZone.day;
    universalTime.hour = (VOS_UINT16)mmInfo->universalTimeandLocalTimeZone.hour;
    universalTime.minute = (VOS_UINT16)mmInfo->universalTimeandLocalTimeZone.minute;
    universalTime.second = (VOS_UINT16)mmInfo->universalTimeandLocalTimeZone.second;

    return TAF_STD_DateTimeToSecondTime(&universalTime);
}

VOS_VOID AT_TransUTCToLocalTime(TAF_AT_CommTime *mmInfo, VOS_UINT64 secondTime)
{
    TAF_STD_TimeZoneType  dateTime = {0};

    TAF_STD_SecondTimeToDateTime(secondTime, &dateTime);

    mmInfo->universalTimeandLocalTimeZone.year = (VOS_UINT16)dateTime.year;
    mmInfo->universalTimeandLocalTimeZone.month = (VOS_UINT8)dateTime.month;
    mmInfo->universalTimeandLocalTimeZone.day = (VOS_UINT8)dateTime.day;
    mmInfo->universalTimeandLocalTimeZone.hour = (VOS_UINT8)dateTime.hour;
    mmInfo->universalTimeandLocalTimeZone.minute = (VOS_UINT8)dateTime.minute;
    mmInfo->universalTimeandLocalTimeZone.second = (VOS_UINT8)dateTime.second;
}

VOS_UINT32 AT_PrintTimeZoneInfoWithCtzeType(TAF_AT_CommTime *mmInfo, VOS_UINT8 *dst)
{
    VOS_UINT64 timep = 0;
    VOS_UINT16 length = 0;
    VOS_INT8   timeZone = AT_INVALID_TZ_VALUE;
    VOS_UINT8  timeZoneValue = 0;

    /* 获取网络上报的时区信息 */
    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        timeZone = mmInfo->universalTimeandLocalTimeZone.timeZone;
    }

    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ) {
        timeZone = mmInfo->localTimeZone;
    }

    /* 将网络时间(年/月/日/时/分/秒)转换为UTC(秒为单位) */
    timep = AT_TransUniversalTimeToUTC(mmInfo);

    if (timeZone < 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, "-");

        timeZoneValue = (VOS_UINT8)(timeZone * (-1));
        /* 减去时区时间(秒) */
        if (timeZoneValue <= AT_LOCAL_TIME_ZONE_DEL_MAX_VALUE) {
            timep -= timeZoneValue * AT_MINUTES_OF_ONE_QUARTER * AT_SECONDS_OF_ONE_MINUTE;
        }
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, "+");

        timeZoneValue = (VOS_UINT8)timeZone;
        /* 累加时区时间(秒) */
        if (timeZoneValue <= AT_LOCAL_TIME_ZONE_ADD_MAX_VALUE) {
            timep += timeZoneValue * AT_MINUTES_OF_ONE_QUARTER * AT_SECONDS_OF_ONE_MINUTE;
        }
    }

    /* 将UTC(秒为单位)转换为本地时间(年/月/日/时/分/秒) */
    AT_TransUTCToLocalTime(mmInfo, timep);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%02d\"", timeZoneValue);

    /* 显示夏时制或冬时制信息 */
    if (((mmInfo->ieFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST) && (mmInfo->dst > 0)) {
        /* 夏时制: DST字段存在, 且值大于0 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, ",%01d", mmInfo->dst);
    } else {
        /* 冬时制 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, ",0");
    }

    /* 显示时间信息 */
    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
       length += AT_PrintLocalTimeWithCtzeType(mmInfo, dst + length);
    }

    /* 结尾 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%s", g_atCrLf);

    return length;
}

LOCAL VOS_VOID AT_TimeZoneDisplayFormatCtze(TAF_UINT16 *length, TAF_AT_CommTime *mmTimeInfo, TAF_UINT8 *dst,
    AT_ModemNetCtx *netCtx)
{
    VOS_UINT32      chkCtzeFlg;
    TAF_INT8        timeZone;

    chkCtzeFlg = AT_CheckRptCmdStatus(mmTimeInfo->unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_CTZE);

    if ((AT_CheckRptCmdStatus(mmTimeInfo->curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_CTZE) == VOS_TRUE) &&
        (chkCtzeFlg == VOS_TRUE)) {
        if ((mmTimeInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
            timeZone = mmTimeInfo->universalTimeandLocalTimeZone.timeZone;
        } else {
            timeZone = mmTimeInfo->localTimeZone;
        }

        if (timeZone != netCtx->timeInfo.localTimeZone) {
            /* 保存网络下发的时区信息 */
            netCtx->timeInfo.ieFlg |= NAS_MM_INFO_IE_LTZ;
            netCtx->timeInfo.localTimeZone                          = timeZone;
            netCtx->timeInfo.universalTimeandLocalTimeZone.timeZone = timeZone;
        }

        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "%s%s\"", g_atCrLf, g_atStringTab[AT_STRING_CTZE].text);

        *length += (VOS_UINT16)AT_PrintTimeZoneInfoWithCtzeType(mmTimeInfo, dst + *length);
    }
}

VOS_UINT32 AT_PrintTimeZoneInfoNoAdjustment(TAF_AT_CommTime *mmInfo, VOS_UINT8 *dst)
{
    VOS_INT8   timeZone = AT_INVALID_TZ_VALUE;
    VOS_UINT8  timeZoneValue;
    VOS_UINT16 length = 0;

    /* 获得时区 */
    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        timeZone = mmInfo->universalTimeandLocalTimeZone.timeZone;
    }

    if (timeZone < 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, "-");

        timeZoneValue = (VOS_UINT8)(timeZone * (-1));
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, "+");

        timeZoneValue = (VOS_UINT8)timeZone;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)dst + length, "%d", timeZoneValue);

    /* 显示夏时制或冬时制信息 */
    if (((mmInfo->ieFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST) && (mmInfo->dst > 0)) {
        /* 夏时制: DST字段存在, 且值大于0， */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, ",%02d\"%s", mmInfo->dst, g_atCrLf);
    } else {
        /* 冬时制 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + length, ",00\"%s", g_atCrLf);
    }

    return length;
}
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT16 AT_PrintNwTimeInfoReprot(NAS_MM_InfoInd *mmInfo, const VOS_CHAR *command,
    VOS_UINT32 nwSecond, VOS_UINT16 length)
{
    NAS_MM_InfoInd localAtTimeInfo;
    TIME_ZONE_Time localTimeZone;
    VOS_UINT8 dst = 0;
    VOS_INT8 timeZone;

    (VOS_VOID)memset_s(&localAtTimeInfo, sizeof(localAtTimeInfo), 0x00, sizeof(localAtTimeInfo));

    /* 时间显示格式: ^TIME(NWTIME): "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    if ((mmInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        /* 获取当前的时间 */
        AT_GetLiveTime(mmInfo, &localAtTimeInfo, nwSecond);
        localTimeZone = localAtTimeInfo.universalTimeandLocalTimeZone;
        /* YY/MM/DD,HH:MM:SS */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d%d/%d%d/%d%d,%d%d:%d%d:%d%d", (VOS_CHAR *)command,
            localTimeZone.year / 10, localTimeZone.year % 10, localTimeZone.month / 10, localTimeZone.month % 10,
            localTimeZone.day / 10, localTimeZone.day % 10, localTimeZone.hour / 10, localTimeZone.hour % 10,
            localTimeZone.minute / 10, localTimeZone.minute % 10, localTimeZone.second / 10, localTimeZone.second % 10);

        /* 获得时区 */
        if ((localAtTimeInfo.ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ) {
            timeZone = localAtTimeInfo.localTimeZone;
        } else {
            timeZone = localAtTimeInfo.universalTimeandLocalTimeZone.timeZone;
        }

        /* 如果时区有效则打印时区 */
        if (timeZone != AT_INVALID_TZ_VALUE) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%02d", (timeZone < 0) ? "-" : "+",
                (timeZone < 0) ? (0 - timeZone) : timeZone);
        }

        /* 显示夏时制或冬时制信息 */
        if (((localAtTimeInfo.ieFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST) && (localAtTimeInfo.dst > 0)) {
            dst = localAtTimeInfo.dst;
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d%d", dst / 10, dst % 10);
    }

    return length;
}
#endif
LOCAL VOS_VOID AT_TimeZoneDisplayFormatTime(TAF_UINT16 *length, TAF_AT_CommTime *mmTimeInfo, TAF_UINT8 *dst,
    AT_ModemNetCtx *netCtx, VOS_UINT8 indexNum)
{
    VOS_UINT32      chkTimeFlg;

    chkTimeFlg = AT_CheckRptCmdStatus(mmTimeInfo->unsolicitedRptCfg, AT_CMD_RPT_CTRL_BY_UNSOLICITED, AT_RPT_CMD_TIME);

    if ((AT_CheckRptCmdStatus(mmTimeInfo->curcRptCfg, AT_CMD_RPT_CTRL_BY_CURC, AT_RPT_CMD_TIME) == VOS_TRUE) &&
        (chkTimeFlg == VOS_TRUE)
        && ((mmTimeInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)) {
#if (FEATURE_MBB_CUST == FEATURE_ON)
        if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->nwTimeQryFlg == (VOS_TRUE)) {
            /* 输出NWTIME结果 */
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)dst + *length, "%s", g_atCrLf);
            *length = AT_PrintNwTimeInfoReprot((NAS_MM_InfoInd *)&(netCtx->timeInfo), "^NWTIME", 0, *length);
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)dst + *length, "%s", g_atCrLf);

           return;
        }
#endif
        /* "^TIME: */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "%s%s", g_atCrLf, g_atStringTab[AT_STRING_TIME].text);

        /* YY */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "\"%d%d/",
            netCtx->timeInfo.universalTimeandLocalTimeZone.year / AT_DECIMAL_BASE_NUM,
            netCtx->timeInfo.universalTimeandLocalTimeZone.year % AT_DECIMAL_BASE_NUM);
        /* MM */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "%d%d/",
            netCtx->timeInfo.universalTimeandLocalTimeZone.month / AT_DECIMAL_BASE_NUM,
            netCtx->timeInfo.universalTimeandLocalTimeZone.month % AT_DECIMAL_BASE_NUM);
        /* dd */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "%d%d,",
            netCtx->timeInfo.universalTimeandLocalTimeZone.day / AT_DECIMAL_BASE_NUM,
            netCtx->timeInfo.universalTimeandLocalTimeZone.day % AT_DECIMAL_BASE_NUM);

        /* hh */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "%d%d:",
            netCtx->timeInfo.universalTimeandLocalTimeZone.hour / AT_DECIMAL_BASE_NUM,
            netCtx->timeInfo.universalTimeandLocalTimeZone.hour % AT_DECIMAL_BASE_NUM);

        /* mm */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "%d%d:",
            netCtx->timeInfo.universalTimeandLocalTimeZone.minute / AT_DECIMAL_BASE_NUM,
            netCtx->timeInfo.universalTimeandLocalTimeZone.minute % AT_DECIMAL_BASE_NUM);

        /* ss */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)dst + *length, "%d%d",
            netCtx->timeInfo.universalTimeandLocalTimeZone.second / AT_DECIMAL_BASE_NUM,
            netCtx->timeInfo.universalTimeandLocalTimeZone.second % AT_DECIMAL_BASE_NUM);

        /* GMT±tz, Summer(Winter) Time" */
        *length += (VOS_UINT16)AT_PrintTimeZoneInfoNoAdjustment(mmTimeInfo, dst + *length);
    }
}

TAF_UINT32 At_PrintMmTimeInfo(VOS_UINT8 indexNum, TAF_AT_CommTime *mmTimeInfo, TAF_UINT8 *dst)
{
    TAF_UINT16      length = 0;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;
    ModemIdUint16   modemId = MODEM_ID_0;
    VOS_UINT32      rslt;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_PrintMmTimeInfo: Get modem id fail.");
        return length;
    }

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    if ((mmTimeInfo->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        /* 保存网络下发的时间信息，无该字段，则使用原有值 */
        netCtx->timeInfo.ieFlg |= NAS_MM_INFO_IE_UTLTZ;

        /* 除以100是为了取年份的后两位 */
        netCtx->timeInfo.universalTimeandLocalTimeZone.year     = mmTimeInfo->universalTimeandLocalTimeZone.year % 100;
        netCtx->timeInfo.universalTimeandLocalTimeZone.month    = mmTimeInfo->universalTimeandLocalTimeZone.month;
        netCtx->timeInfo.universalTimeandLocalTimeZone.day      = mmTimeInfo->universalTimeandLocalTimeZone.day;
        netCtx->timeInfo.universalTimeandLocalTimeZone.hour     = mmTimeInfo->universalTimeandLocalTimeZone.hour;
        netCtx->timeInfo.universalTimeandLocalTimeZone.minute   = mmTimeInfo->universalTimeandLocalTimeZone.minute;
        netCtx->timeInfo.universalTimeandLocalTimeZone.second   = mmTimeInfo->universalTimeandLocalTimeZone.second;
        netCtx->timeInfo.universalTimeandLocalTimeZone.reserved = 0x00;
        netCtx->timeInfo.universalTimeandLocalTimeZone.timeZone = mmTimeInfo->universalTimeandLocalTimeZone.timeZone;
        netCtx->nwSecond                                        = AT_GetSeconds();
    }

    /* 更新DST信息 */
    if ((mmTimeInfo->ieFlg & NAS_MM_INFO_IE_DST) == NAS_MM_INFO_IE_DST) {
        /* 保存网络下发的时间信息 */
        netCtx->timeInfo.ieFlg |= NAS_MM_INFO_IE_DST;
        netCtx->timeInfo.dst = mmTimeInfo->dst;
    } else {
        netCtx->timeInfo.ieFlg &= ~NAS_MM_INFO_IE_DST;
    }

    /* 时区显示格式: +CTZV: "GMT±tz, Summer(Winter) Time" */
    AT_TimeZoneDisplayFormatCtzv(&length, mmTimeInfo, dst, netCtx);

    /* 时区显示格式:+CTZE: "(+/-)tz,dst,yyyy/mm/dd,hh:mm:ss" */
    AT_TimeZoneDisplayFormatCtze(&length, mmTimeInfo, dst, netCtx);

    /* 时间显示格式: ^TIME: "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    AT_TimeZoneDisplayFormatTime(&length, mmTimeInfo, dst, netCtx, indexNum);

    return length;
}

VOS_UINT32 AT_RcvMmaTimeChangeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_TimeChangeInd *timeChangeInd = (TAF_MMA_TimeChangeInd *)msg;
    TAF_AT_CommTime        mmTimeInfo;
    errno_t                memResult;
    TAF_UINT16             length;

    length        = 0;

    (VOS_VOID)memset_s(&mmTimeInfo, sizeof(mmTimeInfo), 0x00, sizeof(mmTimeInfo));

    mmTimeInfo.dst           = timeChangeInd->dST;
    mmTimeInfo.ieFlg         = timeChangeInd->ieFlg;
    mmTimeInfo.localTimeZone = timeChangeInd->localTimeZone;

    memResult = memcpy_s(mmTimeInfo.curcRptCfg, sizeof(mmTimeInfo.curcRptCfg), timeChangeInd->curcRptCfg,
                         sizeof(timeChangeInd->curcRptCfg));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mmTimeInfo.curcRptCfg), sizeof(timeChangeInd->curcRptCfg));

    memResult = memcpy_s(mmTimeInfo.unsolicitedRptCfg, sizeof(mmTimeInfo.unsolicitedRptCfg),
                         timeChangeInd->unsolicitedRptCfg, sizeof(timeChangeInd->unsolicitedRptCfg));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mmTimeInfo.unsolicitedRptCfg), sizeof(timeChangeInd->unsolicitedRptCfg));

    mmTimeInfo.universalTimeandLocalTimeZone.year     = timeChangeInd->universalTimeandLocalTimeZone.year;
    mmTimeInfo.universalTimeandLocalTimeZone.month    = timeChangeInd->universalTimeandLocalTimeZone.month;
    mmTimeInfo.universalTimeandLocalTimeZone.day      = timeChangeInd->universalTimeandLocalTimeZone.day;
    mmTimeInfo.universalTimeandLocalTimeZone.hour     = timeChangeInd->universalTimeandLocalTimeZone.hour;
    mmTimeInfo.universalTimeandLocalTimeZone.minute   = timeChangeInd->universalTimeandLocalTimeZone.minute;
    mmTimeInfo.universalTimeandLocalTimeZone.second   = timeChangeInd->universalTimeandLocalTimeZone.second;
    mmTimeInfo.universalTimeandLocalTimeZone.timeZone = timeChangeInd->universalTimeandLocalTimeZone.timeZone;

    if (((timeChangeInd->ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) ||
        ((timeChangeInd->ieFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ)) {
        /* 存在时间信息 */

        length += (TAF_UINT16)At_PrintMmTimeInfo(indexNum, &mmTimeInfo, (g_atSndCodeAddress + length));
#if (FEATURE_MBB_CUST == FEATURE_ON)
        AtUpdateCclkInfo(timeChangeInd);
#endif
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvImsaDmcnInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_INFO_LOG("AT_RcvImsaDmcnInd entered!");

    /* 构造AT主动上报^DMCN */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%s", g_atCrLf, g_atStringTab[AT_STRING_DMCN].text, g_atCrLf);

    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaCirephInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_CirephInd *cirephInd = (IMSA_AT_CirephInd *)msg;

    /* 判断查询操作是否成功 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d%s", g_atCrLf, g_atStringTab[AT_STRING_CIREPH].text,
        cirephInd->handover, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaCirepiInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_CirepiInd *cirepiInd = (IMSA_AT_CirepiInd *)msg;

    /* 判断查询操作是否成功 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d%s", g_atCrLf, g_atStringTab[AT_STRING_CIREPI].text,
        cirepiInd->imsvops, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaCireguInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_CireguInd *cireguInd = (IMSA_AT_CireguInd *)msg;
    VOS_UINT16         length = 0;

    /* 判断查询操作是否成功 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s: %d", g_atCrLf, g_atStringTab[AT_STRING_CIREGU].text,
        cireguInd->regInfo);

    /* 如果IMS未注册，<ext_info>参数无意义 */
    if ((cireguInd->regInfo != VOS_FALSE) && (cireguInd->opExtInfo != VOS_FALSE)) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cireguInd->extInfo);
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    g_atSendDataBuff.bufLen = length;
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaImsRegFailInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_RegFailInd *regFailInd = (IMSA_AT_RegFailInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d%s", g_atCrLf, g_atStringTab[AT_STRING_IMS_REG_FAIL].text,
        regFailInd->regFailInfo.failCode, g_atCrLf);
    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#endif

TAF_UINT32 At_ProcMePersonalizationOpRsltOk(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_FAILURE;
    TAF_UINT32 tmp = 0;

    switch (event->mePersonalisation.cmdType) {
        case TAF_ME_PERSONALISATION_ACTIVE:
        case TAF_ME_PERSONALISATION_DEACTIVE:
        case TAF_ME_PERSONALISATION_SET:
        case TAF_ME_PERSONALISATION_PWD_CHANGE:
        case TAF_ME_PERSONALISATION_VERIFY:
            result = AT_OK;
            break;

        case TAF_ME_PERSONALISATION_QUERY:
            if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CARD_LOCK_READ) {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,",
                    event->mePersonalisation.unReportContent.OperatorLockInfo.operatorLockStatus);
                *length +=
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,",
                        event->mePersonalisation.unReportContent.OperatorLockInfo.remainTimes);
                if ((event->mePersonalisation.unReportContent.OperatorLockInfo.operatorLen <
                     TAF_PH_ME_LOCK_OPER_LEN_MIN) ||
                    (event->mePersonalisation.unReportContent.OperatorLockInfo.operatorLen >
                     TAF_PH_ME_LOCK_OPER_LEN_MAX)) {
                    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + *length, "0");
                } else {
                    for (tmp = 0; tmp < event->mePersonalisation.unReportContent.OperatorLockInfo.operatorLen; tmp++) {
                        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d",
                            event->mePersonalisation.unReportContent.OperatorLockInfo.operatorArray[tmp]);
                    }
                }
                result = AT_OK;
            } else {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
                if (event->mePersonalisation.activeStatus == TAF_ME_PERSONALISATION_ACTIVE_STATUS) {
                    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + *length, "1");
                } else {
                    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                        (TAF_CHAR *)g_atSndCodeAddress + *length, "0");
                }
                result = AT_OK;
            }
            break;

        case TAF_ME_PERSONALISATION_RETRIEVE:
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
            event->mePersonalisation.unReportContent.SimPersionalisationStr.dataLen = AT_MIN(
                event->mePersonalisation.unReportContent.SimPersionalisationStr.dataLen,
                TAF_MAX_IMSI_LEN);
            for (tmp = 0; tmp < event->mePersonalisation.unReportContent.SimPersionalisationStr.dataLen; tmp++) {
                *(g_atSndCodeAddress + *length + tmp) =
                    event->mePersonalisation.unReportContent.SimPersionalisationStr.simPersonalisationStr[tmp] + 0x30;
            }
            *length += (VOS_UINT16)tmp;
            break;

        default:
            result = AT_ERROR;
            break;
    }

    return result;
}

TAF_UINT32 At_ProcPhoneEvtMePersonalizationCnf(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_FAILURE;

    if (event->mePersonalisation.opRslt != TAF_PH_ME_PERSONALISATION_OK) {
        if (event->mePersonalisation.opRslt == TAF_PH_ME_PERSONALISATION_NO_SIM) {
            result = At_ChgTafErrorCode(indexNum, TAF_ERR_CMD_TYPE_ERROR);
        } else if (event->mePersonalisation.opRslt == TAF_PH_ME_PERSONALISATION_OP_NOT_ALLOW) {
            result = AT_CME_OPERATION_NOT_ALLOWED;
        } else if (event->mePersonalisation.opRslt == TAF_PH_ME_PERSONALISATION_WRONG_PWD) {
            result = AT_CME_INCORRECT_PASSWORD;
        } else {
            result = AT_ERROR;
        }
    } else {
        result = At_ProcMePersonalizationOpRsltOk(event, length, indexNum);
    }

    AT_StopTimerCmdReady(indexNum);

    return result;
}

TAF_UINT32 At_ProcPhoneEvtPlmnListRej(TAF_PHONE_EventInfo *event, TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_FAILURE;

    if (event->phoneError == TAF_ERR_NO_RF) {
        result = AT_CME_NO_RF;
    } else {
        result = AT_CME_OPERATION_NOT_ALLOWED;
    }

    AT_StopTimerCmdReady(indexNum);

    return result;
}

TAF_UINT32 At_ProcPhoneEvtRestrictedAccessCnf(TAF_PHONE_EventInfo *event, TAF_UINT16 *length, TAF_UINT8 indexNum)
{
    TAF_UINT32 result = AT_OK;

    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    if (event->opUsimRestrictAccess == 1) {
        /* <sw1, sw2>, */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%d,%d", event->restrictedAccess.sW1,
            event->restrictedAccess.sW2);

        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",\"");

        if (event->restrictedAccess.len != 0) {
            /* <response> */
            *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                           (TAF_UINT8 *)g_atSndCodeAddress + *length,
                                                           event->restrictedAccess.content,
                                                           event->restrictedAccess.len);
        }

        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
    }

    AT_StopTimerCmdReady(indexNum);

    return result;
}

VOS_UINT32 atCsqInfoIndProc(struct MsgCB *msgBlock)
{
    L4A_CSQ_INFO_IND_STRU *csqInfo = NULL;
    VOS_UINT16             length  = 0;
    VOS_INT16              rssi    = 0;

    csqInfo = (L4A_CSQ_INFO_IND_STRU *)msgBlock;

    if (csqInfo->errorCode == ERR_MSP_SUCCESS) {
        if (csqInfo->rssi == AT_RSSI_UNKNOWN) {
            rssi = AT_RSSI_UNKNOWN;
        } else if (csqInfo->rssi >= AT_RSSI_HIGH) {
            rssi = AT_CSQ_RSSI_HIGH;
        } else if (csqInfo->rssi <= AT_RSSI_LOW) {
            rssi = AT_CSQ_RSSI_LOW;
        } else {
            rssi = (csqInfo->rssi - AT_RSSI_LOW) / 2;
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "%s%s%d%s", g_atCrLf, "^RSSI:", rssi, g_atCrLf);
        At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, g_atSndCodeAddr, length);
    }

    return AT_FW_CLIENT_STATUS_READY;
}

