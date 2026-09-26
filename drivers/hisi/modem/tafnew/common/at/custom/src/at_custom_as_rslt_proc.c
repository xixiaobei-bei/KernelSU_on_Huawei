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
#include "at_custom_as_rslt_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_mta_interface.h"
#include "at_custom_comm.h"
#include "at_lte_common.h"
#include "at_init.h"
#include "osm.h"
#include "at_external_module_msg_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_AS_RSLT_PROC_C

VOS_UINT32 g_ate5Order = 0;

#define AT_CASCELL_RSSI_VALUE_MIN (-120) /* RSSI 最小门限 */
#define AT_CASCELL_RSSI_VALUE_MAX (-25) /* RSSI 最大门限 */
#define AT_CASCELL_RSRP_VALUE_MIN (-140) /* RSRP 最小门限 */
#define AT_CASCELL_RSRP_VALUE_MAX (-44) /* RSRP 最大门限 */
#define AT_CASCELL_RSRQ_VALUE_MIN (-19) /* RSRQ 最小门限 */
#define AT_CASCELL_RSRQ_VALUE_MAX (-3) /* RSRQ 最大门限 */
#define AT_NVIM_ECIO_VALUE_INDEX_1 1
#define AT_NVIM_ECIO_VALUE_INDEX_2 2
#define AT_NVIM_ECIO_VALUE_INDEX_3 3
#define AT_NVIM_RSCP_VALUE_INDEX_1 1
#define AT_NVIM_RSCP_VALUE_INDEX_2 2
#define AT_NVIM_RSCP_VALUE_INDEX_3 3
#define AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_1 1
#define AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_2 2
#define AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_3 3
#define AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_1 1
#define AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_2 2
#define AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_3 3

enum AT_NrPowerSavingParaIndex {
    AT_NRPOWERSAVING_PARA0 = 0,
    AT_NRPOWERSAVING_PARA1,
    AT_NRPOWERSAVING_PARA2,
    AT_NRPOWERSAVING_PARA3,
    AT_NRPOWERSAVING_PARA4,
    AT_NRPOWERSAVING_PARA5,
    AT_NRPOWERSAVING_PARA6,
    AT_NRPOWERSAVING_PARA7,
    AT_NRPOWERSAVING_PARA8,
    AT_NRPOWERSAVING_PARA9,
    AT_NRPOWERSAVING_PARA10,
    AT_NRPOWERSAVING_PARA11,
    AT_NRPOWERSAVING_PARA12,
    AT_NRPOWERSAVING_PARA13,
    AT_NRPOWERSAVING_PARA14,
    AT_NRPOWERSAVING_PARA15,
    AT_NRPOWERSAVING_PARA16,
    AT_NRPOWERSAVING_PARA17,
    AT_NRPOWERSAVING_PARA18,
    AT_NRPOWERSAVING_PARA19,
    AT_NRPOWERSAVING_PARA20,
    AT_NRPOWERSAVING_PARA21,
    AT_NRPOWERSAVING_PARA22,
    AT_NRPOWERSAVING_PARA_BUTT
};
typedef VOS_UINT32 AT_NrPowerSavingParaIndex;

VOS_UINT32 AT_RcvMtaEcidSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg        *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_EcidSetCnf *ecidSetCnf = (MTA_AT_EcidSetCnf *)mtaMsg->content;
    VOS_UINT32         result;

    /* 判断查询操作是否成功 */
    if (ecidSetCnf->result == MTA_AT_RESULT_NO_ERROR) {
        /* 输出查询的增强型小区信息 */
        /* +ECID=<version>,<rat>,[<cell_description>] */
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s=%s", g_parseContext[indexNum].cmdElement->cmdName,
                ecidSetCnf->cellInfoStr);
        result = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s=%s", g_parseContext[indexNum].cmdElement->cmdName, "0,NONE:,");

        result = AT_OK;
    }

    return result;
}

VOS_VOID AT_NetMonFmtPlmnId(VOS_UINT32 mcc, VOS_UINT32 mnc, VOS_CHAR *pstrPlmn, VOS_UINT16 *length)
{
    VOS_UINT32 maxLength = AT_NETMON_PLMN_STRING_MAX_LENGTH;
    VOS_UINT32 lengthRlt = 0;
    VOS_INT32  lengthTemp = 0;

    /* 格式输出MCC MNC */
    if ((mnc & 0x0f0000) == 0x0f0000) {
        lengthTemp = snprintf_s((VOS_CHAR *)pstrPlmn, (VOS_UINT32)maxLength, (VOS_UINT32)(maxLength - 1),
                                            "%X%X%X,%X%X", (mcc & 0x0f), (mcc & 0x0f00) >> 8, (mcc & 0x0f0000) >> 16,
                                            (mnc & 0x0f), (mnc & 0x0f00) >> 8);
    } else {
        lengthTemp = snprintf_s((VOS_CHAR *)pstrPlmn, (VOS_UINT32)maxLength, (VOS_UINT32)(maxLength - 1),
                                            "%X%X%X,%X%X%X", (mcc & 0x0f), (mcc & 0x0f00) >> 8, (mcc & 0x0f0000) >> 16,
                                            (mnc & 0x0f), (mnc & 0xf00) >> 8, (mnc & 0x0f0000) >> 16);
    }

    if (lengthTemp < 0) {
        AT_ERR_LOG("AT_NetMonFmtPlmnId: MCC ulLength is error!");
        *(pstrPlmn + AT_NETMON_PLMN_STRING_MAX_LENGTH - 1) = 0;
        *length                                            = AT_NETMON_PLMN_STRING_MAX_LENGTH;
        return;
    }

    lengthRlt = (VOS_UINT32)lengthTemp;
    /* 长度翻转保护 */
    if (lengthRlt >= AT_NETMON_PLMN_STRING_MAX_LENGTH) {
        AT_ERR_LOG("AT_NetMonFmtPlmnId: MCC ulLength is error!");
        *(pstrPlmn + AT_NETMON_PLMN_STRING_MAX_LENGTH - 1) = 0;
        *length                                            = AT_NETMON_PLMN_STRING_MAX_LENGTH;
        return;
    }

    *length = (VOS_UINT16)lengthRlt;
}

VOS_VOID AT_NetMonFmtGsmSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length)
{
    VOS_UINT8  plmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32 mcc;
    VOS_UINT32 mnc;
    VOS_UINT16 plmnStrLen = 0;
    VOS_UINT16 lengthTemp = 0;

    (VOS_VOID)memset_s(plmnstr, sizeof(plmnstr), 0x00, sizeof(plmnstr));

    mcc = sCellInfo->u.unSCellInfo.gsmSCellInfo.mcc;
    mnc = sCellInfo->u.unSCellInfo.gsmSCellInfo.mnc;

    AT_NetMonFmtPlmnId(mcc, mnc, (VOS_CHAR *)plmnstr, &plmnStrLen);

    /* 格式输出PLMN */
    lengthTemp +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "^MONSC: GSM,%s,%d,%u,%d,%X,%X", plmnstr,
            sCellInfo->u.unSCellInfo.gsmSCellInfo.band, sCellInfo->u.unSCellInfo.gsmSCellInfo.arfcn,
            sCellInfo->u.unSCellInfo.gsmSCellInfo.bsic, sCellInfo->u.unSCellInfo.gsmSCellInfo.cellId,
            sCellInfo->u.unSCellInfo.gsmSCellInfo.lac);

    /* RSSI无效值，不显示 */
    if (sCellInfo->u.unSCellInfo.gsmSCellInfo.rssi == AT_NETMON_GSM_RSSI_INVALID_VALUE) {
        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",");
    } else {
        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", sCellInfo->u.unSCellInfo.gsmSCellInfo.rssi);
    }

    /* 无效值，不显示 */
    if (sCellInfo->u.unSCellInfo.gsmSCellInfo.rxQuality == AT_NETMON_GSM_RX_QUALITY_INVALID_VALUE) {
        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",");
    } else {
        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", sCellInfo->u.unSCellInfo.gsmSCellInfo.rxQuality);
    }

    /* 输出TA */
    if (sCellInfo->u.unSCellInfo.gsmSCellInfo.opTa == PS_IE_PRESENT) {
        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", sCellInfo->u.unSCellInfo.gsmSCellInfo.ta);
    } else {
        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",");
    }

    *length = lengthTemp;
}
/*
 * 功能描述: WCDMA 服务小区输出 PSC
 */
LOCAL VOS_VOID AT_PrintUtranSCellPscInfo(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if ((sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.rscp == AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE) &&
        (sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.ecn0 == AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE)) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%d",
            sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.psc);
    }
}
/*
 * 功能描述: WCDMA 服务小区输出 CellId
 */
LOCAL VOS_VOID AT_PrintUtranSCellCellIdInfo(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if (sCellInfo->u.unSCellInfo.utranSCellInfo.opCellId == PS_IE_PRESENT) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%X", sCellInfo->u.unSCellInfo.utranSCellInfo.cellId);
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    }
}
/*
 * 功能描述: WCDMA 服务小区输出 Lac
 */
LOCAL VOS_VOID AT_PrintUtranSCellLacInfo(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if (sCellInfo->u.unSCellInfo.utranSCellInfo.opLac == PS_IE_PRESENT) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%X", sCellInfo->u.unSCellInfo.utranSCellInfo.lac);
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    }
}
/*
 * 功能描述: WCDMA 服务小区输出 rscp
 */
LOCAL VOS_VOID AT_PrintUtranSCellRscpInfo(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if (sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.rscp == AT_NETMON_UTRAN_FDD_RSCP_INVALID_VALUE) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%d",
            sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.rscp);
    }
}
/*
 * 功能描述: WCDMA 服务小区输出 rssi
 */
LOCAL VOS_VOID AT_PrintUtranSCellRssiInfo(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if (sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.rssi == AT_NETMON_UTRAN_FDD_RSSI_INVALID_VALUE) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%d",
            sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.rssi);
    }
}
/*
 * 功能描述: WCDMA 服务小区输出 ecn0
 */
LOCAL VOS_VOID AT_PrintUtranSCellEcn0Info(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if (sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.ecn0 == AT_NETMON_UTRAN_FDD_ECN0_INVALID_VALUE) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%d",
            sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.ecn0);
    }
}
/*
 * 功能描述: WCDMA 服务小区输出 drx
 */
LOCAL VOS_VOID AT_PrintUtranSCellDrxInfo(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if (sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.opDrx == PS_IE_PRESENT) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%d",
            sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.drx);
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    }
}
/*
 * 功能描述: WCDMA 服务小区输出 ura id
 */
LOCAL VOS_VOID AT_PrintUtranSCellUraIdInfo(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *lengthTemp)
{
    if (sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.opUra == PS_IE_PRESENT) {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",%d",
            sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsFdd.ura);
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",");
    }
}

VOS_VOID AT_NetMonFmtUtranFddSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length)
{
    VOS_UINT8  plmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32 mcc;
    VOS_UINT32 mnc;
    VOS_UINT16 plmnStrLen = 0;
    VOS_UINT16 lengthTemp = 0;

    (VOS_VOID)memset_s(plmnstr, sizeof(plmnstr), 0x00, sizeof(plmnstr));

    mcc = sCellInfo->u.unSCellInfo.utranSCellInfo.mcc;
    mnc = sCellInfo->u.unSCellInfo.utranSCellInfo.mnc;

    AT_NetMonFmtPlmnId(mcc, mnc, (VOS_CHAR *)plmnstr, &plmnStrLen);

    /* 格式输出PLMN */
    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "^MONSC: WCDMA,%s", plmnstr);

    /* 输出频点 */
    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%u", sCellInfo->u.unSCellInfo.utranSCellInfo.arfcn);

    /* PSC无效值，不显示 */
    AT_PrintUtranSCellPscInfo(sCellInfo, &lengthTemp);

    /* 输出Cell ID */
    AT_PrintUtranSCellCellIdInfo(sCellInfo, &lengthTemp);

    /* 输出LAC */
    AT_PrintUtranSCellLacInfo(sCellInfo, &lengthTemp);

    /* RSCP无效值，不显示 */
    AT_PrintUtranSCellRscpInfo(sCellInfo, &lengthTemp);

    /* RSSI无效值，不显示 */
    AT_PrintUtranSCellRssiInfo(sCellInfo, &lengthTemp);

    /* ECN0无效值，不显示 */
    AT_PrintUtranSCellEcn0Info(sCellInfo, &lengthTemp);

    /* 输出DRX */
    AT_PrintUtranSCellDrxInfo(sCellInfo, &lengthTemp);

    /* 输出URA Id */
    AT_PrintUtranSCellUraIdInfo(sCellInfo, &lengthTemp);

    *length = lengthTemp;
}

VOS_VOID AT_NetMonFmtGsmNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen)
{
    VOS_UINT32 loop;
    VOS_UINT16 length = inLen;

    if (nCellInfo->u.nCellInfo.gsmNCellCnt > NETMON_MAX_GSM_NCELL_NUM) {
        nCellInfo->u.nCellInfo.gsmNCellCnt = NETMON_MAX_GSM_NCELL_NUM;
    }

    /* GSM邻区显示 */
    for (loop = 0; loop < nCellInfo->u.nCellInfo.gsmNCellCnt; loop++) {
        /* 如果输出的不是第一个邻区，先打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONNC: GSM,%d,%u",
            nCellInfo->u.nCellInfo.gsmNCellInfo[loop].band, nCellInfo->u.nCellInfo.gsmNCellInfo[loop].afrcn);

        /* 输出Bsic */
        if (nCellInfo->u.nCellInfo.gsmNCellInfo[loop].opBsic == PS_IE_PRESENT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", nCellInfo->u.nCellInfo.gsmNCellInfo[loop].bsic);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }

        /* 输出Cell ID */
        if (nCellInfo->u.nCellInfo.gsmNCellInfo[loop].opCellId == PS_IE_PRESENT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%X", nCellInfo->u.nCellInfo.gsmNCellInfo[loop].cellId);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }

        /* 输出LAC */
        if (nCellInfo->u.nCellInfo.gsmNCellInfo[loop].opLac == PS_IE_PRESENT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%X", nCellInfo->u.nCellInfo.gsmNCellInfo[loop].lac);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }

        /* 输出RSSI */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", nCellInfo->u.nCellInfo.gsmNCellInfo[loop].rssi);
    }

    *outLen = length;
}

VOS_VOID AT_NetMonFmtUtranFddNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen)
{
    VOS_UINT32 loop;
    VOS_UINT16 length = inLen;

    if (nCellInfo->u.nCellInfo.utranNCellCnt > NETMON_MAX_UTRAN_NCELL_NUM) {
        nCellInfo->u.nCellInfo.utranNCellCnt = NETMON_MAX_UTRAN_NCELL_NUM;
    }

    /* WCDMA 临区显示 */
    for (loop = 0; loop < nCellInfo->u.nCellInfo.utranNCellCnt; loop++) {
        /* 如果不是第一次打印邻区，打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONNC: WCDMA,%u,%d,%d,%d",
            nCellInfo->u.nCellInfo.u.fddNCellInfo[loop].arfcn, nCellInfo->u.nCellInfo.u.fddNCellInfo[loop].psc,
            nCellInfo->u.nCellInfo.u.fddNCellInfo[loop].rscp, nCellInfo->u.nCellInfo.u.fddNCellInfo[loop].ecN0);
    }

    *outLen = length;
}

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)

VOS_VOID AT_NetMonFmtUtranTddSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length)
{
    VOS_UINT8  plmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32 mcc;
    VOS_UINT32 mnc;
    VOS_UINT16 plmnStrLen = 0;
    VOS_UINT16 lengthTemp = 0;

    (VOS_VOID)memset_s(plmnstr, sizeof(plmnstr), 0x00, sizeof(plmnstr));

    mcc = sCellInfo->u.unSCellInfo.utranSCellInfo.mcc;
    mnc = sCellInfo->u.unSCellInfo.utranSCellInfo.mnc;

    AT_NetMonFmtPlmnId(mcc, mnc, (VOS_CHAR *)plmnstr, &plmnStrLen);

    /* 格式输出PLMN */
    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "^MONSC: TD_SCDMA,%s", plmnstr);

    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%u,%d,%d,%X,%X,%d,%d,%d",
        sCellInfo->u.unSCellInfo.utranSCellInfo.arfcn,
        sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsTdd.syncID,
        sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsTdd.sc,
        sCellInfo->u.unSCellInfo.utranSCellInfo.cellId, sCellInfo->u.unSCellInfo.utranSCellInfo.lac,
        sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsTdd.rscp,
        sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsTdd.drx,
        sCellInfo->u.unSCellInfo.utranSCellInfo.u.cellMeasResultsTdd.rac);

    *length = lengthTemp;
}

VOS_VOID AT_NetMonFmtUtranTddNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen)
{
    VOS_UINT32 loop;
    VOS_UINT16 length = inLen;

    if (nCellInfo->u.nCellInfo.utranNCellCnt > NETMON_MAX_UTRAN_NCELL_NUM) {
        nCellInfo->u.nCellInfo.utranNCellCnt = NETMON_MAX_UTRAN_NCELL_NUM;
    }

    /* WCDMA 临区显示 */
    for (loop = 0; loop < nCellInfo->u.nCellInfo.utranNCellCnt; loop++) {
        /* 如果不是第一次打印邻区，打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONNC: TD_SCDMA,%u,%d,%d,%d",
            nCellInfo->u.nCellInfo.u.tddNCellInfo[loop].arfcn, nCellInfo->u.nCellInfo.u.tddNCellInfo[loop].syncId,
            nCellInfo->u.nCellInfo.u.tddNCellInfo[loop].sc, nCellInfo->u.nCellInfo.u.tddNCellInfo[loop].rscp);
    }

    *outLen = length;
}
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_VOID AT_NetMonFmtEutranSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length)
{
    VOS_UINT8  plmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32 mcc;
    VOS_UINT32 mnc;
    VOS_UINT16 plmnStrLen = 0;
    VOS_UINT16 lengthTemp = 0;

    (VOS_VOID)memset_s(plmnstr, sizeof(plmnstr), 0x00, sizeof(plmnstr));

    mcc = sCellInfo->u.unSCellInfo.lteSCellInfo.mcc;
    mnc = sCellInfo->u.unSCellInfo.lteSCellInfo.mnc;

    AT_NetMonFmtPlmnId(mcc, mnc, (VOS_CHAR *)plmnstr, &plmnStrLen);

    /* 格式输出PLMN */
    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "^MONSC: LTE,%s", plmnstr);

    lengthTemp +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%u,%X,%X,%X,%d,%d,%d",
            sCellInfo->u.unSCellInfo.lteSCellInfo.arfcn, sCellInfo->u.unSCellInfo.lteSCellInfo.cellID,
            sCellInfo->u.unSCellInfo.lteSCellInfo.pid, sCellInfo->u.unSCellInfo.lteSCellInfo.tac,
            sCellInfo->u.unSCellInfo.lteSCellInfo.rsrp, sCellInfo->u.unSCellInfo.lteSCellInfo.rsrq,
            sCellInfo->u.unSCellInfo.lteSCellInfo.rssi);
    *length = lengthTemp;
}

VOS_VOID AT_NetMonFmtEutranNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen)
{
    VOS_UINT32 loop;
    VOS_UINT16 length = inLen;

    if (nCellInfo->u.nCellInfo.lteNCellCnt > NETMON_MAX_LTE_NCELL_NUM) {
        nCellInfo->u.nCellInfo.lteNCellCnt = NETMON_MAX_LTE_NCELL_NUM;
    }

    /* LTE邻区显示 */
    for (loop = 0; loop < nCellInfo->u.nCellInfo.lteNCellCnt; loop++) {
        /* 如果不是第一次打印邻区，打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONNC: LTE,%u,%X,%d,%d,%d",
            nCellInfo->u.nCellInfo.lteNCellInfo[loop].arfcn, nCellInfo->u.nCellInfo.lteNCellInfo[loop].pid,
            nCellInfo->u.nCellInfo.lteNCellInfo[loop].rsrp, nCellInfo->u.nCellInfo.lteNCellInfo[loop].rsrq,
            nCellInfo->u.nCellInfo.lteNCellInfo[loop].rssi);
    }

    *outLen = length;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID AT_NrCellBandFmtDataSa(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length)
{
    VOS_UINT16 lengthTemp = 0;

    /* 格式输出PLMN */
    lengthTemp +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "^NRCELLBAND: SA,%u,%d",
            sCellInfo->u.unSCellInfo.nrSCellInfo.arfcn, sCellInfo->u.unSCellInfo.nrSCellInfo.bandInd);

    *length = lengthTemp;
}

VOS_VOID AT_NrCellBandFmtDataNsa(MTA_AT_NetmonCellInfo *cellInfo, VOS_UINT16 *outLen)
{
    VOS_UINT32 loop;
    VOS_UINT32 nrCcCnt;
    VOS_UINT16 length = 0;

    nrCcCnt = cellInfo->u.secSrvCellInfo.componentCarrierNum;

    if (nrCcCnt > NETMON_MAX_NR_CC_NUM) {
        nrCcCnt = NETMON_MAX_NR_CC_NUM;
    }

    /* NR CC打印 */
    for (loop = 0; loop < nrCcCnt; loop++) {
        /* 如果不是第一次打印CC，打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^NRCELLBAND: NSA,%u,%d",
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].arfcn,
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].bandInd);
    }

    *outLen = length;
}

VOS_VOID AT_NetMonFmtNrSCellData(MTA_AT_NetmonCellInfo *sCellInfo, VOS_UINT16 *length)
{
    VOS_UINT8  plmnstr[AT_NETMON_PLMN_STRING_MAX_LENGTH];
    VOS_UINT32 mcc;
    VOS_UINT32 mnc;
    VOS_UINT16 plmnStrLen = 0;
    VOS_UINT16 lengthTemp = 0;
    VOS_UINT8  cellIdStr[AT_CELLID_STRING_MAX_LEN];

    (VOS_VOID)memset_s(plmnstr, sizeof(plmnstr), 0x00, sizeof(plmnstr));
    (VOS_VOID)memset_s(cellIdStr, sizeof(cellIdStr), 0x00, sizeof(cellIdStr));

    mcc = sCellInfo->u.unSCellInfo.nrSCellInfo.mcc;
    mnc = sCellInfo->u.unSCellInfo.nrSCellInfo.mnc;

    AT_NetMonFmtPlmnId(mcc, mnc, (VOS_CHAR *)plmnstr, &plmnStrLen);

    AT_ConvertCellIdToHexStrFormat(sCellInfo->u.unSCellInfo.nrSCellInfo.cellIdentityLowBit,
                                   sCellInfo->u.unSCellInfo.nrSCellInfo.cellIdentityHighBit, (VOS_CHAR *)cellIdStr);

    /* 格式输出PLMN */
    lengthTemp +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "^MONSC: NR,%s,%u,%d,%s,%X,%X,%d,%d,%d", plmnstr,
            sCellInfo->u.unSCellInfo.nrSCellInfo.arfcn, sCellInfo->u.unSCellInfo.nrSCellInfo.nrScsType, cellIdStr,
            sCellInfo->u.unSCellInfo.nrSCellInfo.phyCellId, sCellInfo->u.unSCellInfo.nrSCellInfo.tac,
            sCellInfo->u.unSCellInfo.nrSCellInfo.rsrp, sCellInfo->u.unSCellInfo.nrSCellInfo.rsrq,
            sCellInfo->u.unSCellInfo.nrSCellInfo.sinr);

    *length = lengthTemp;
}

VOS_VOID AT_NetMonFmtNrSSCellData(MTA_AT_NetmonCellInfo *cellInfo, VOS_UINT16 *outLen)
{
    VOS_UINT32 loop;
    VOS_UINT16 length = 0;
    VOS_UINT32 nrCcCnt;

    nrCcCnt = cellInfo->u.secSrvCellInfo.componentCarrierNum;

    if (nrCcCnt > NETMON_MAX_NR_CC_NUM) {
        nrCcCnt = NETMON_MAX_NR_CC_NUM;
    }

    /* NR CC打印 */
    for (loop = 0; loop < nrCcCnt; loop++) {
        /* 如果不是第一次打印CC，打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONSSC: NR,%u,%X,%d,%d,%d,%d",
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].arfcn,
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].phyCellId,
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].rsrp,
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].rsrq,
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].sinr,
            cellInfo->u.secSrvCellInfo.componentCarrierInfo[loop].measRsType);
    }

    *outLen = length;
}

VOS_VOID AT_NetMonFmtNrNCellData(MTA_AT_NetmonCellInfo *nCellInfo, VOS_UINT16 inLen, VOS_UINT16 *outLen)
{
    VOS_UINT32 loop;
    VOS_UINT16 length = inLen;
    VOS_UINT8  nrNCellCnt;

    nrNCellCnt = nCellInfo->u.nCellInfo.nrNCellCnt;

    if (nrNCellCnt > NETMON_MAX_NR_NCELL_NUM) {
        nrNCellCnt = NETMON_MAX_NR_NCELL_NUM;
    }

    /* NR邻区显示 */
    for (loop = 0; loop < nrNCellCnt; loop++) {
        /* 如果不是第一次打印邻区，打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONNC: NR,%u,%X,%d,%d,%d",
            nCellInfo->u.nCellInfo.nrNCellInfo[loop].arfcn, nCellInfo->u.nCellInfo.nrNCellInfo[loop].phyCellId,
            nCellInfo->u.nCellInfo.nrNCellInfo[loop].rsrp, nCellInfo->u.nCellInfo.nrNCellInfo[loop].rsrq,
            nCellInfo->u.nCellInfo.nrNCellInfo[loop].sinr);
    }

    *outLen = length;
}
#endif

VOS_UINT32 AT_RcvMtaSetNetMonSCellCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NetmonCellInfo *setCnf = VOS_NULL_PTR;
    VOS_UINT16             length;

    /* 初始化 */
    rcvMsg = (AT_MTA_Msg *)msg;
    setCnf = (MTA_AT_NetmonCellInfo *)rcvMsg->content;

    length   = 0;

    if ((setCnf->result == MTA_AT_RESULT_ERROR) || (setCnf->cellType != MTA_NETMON_SCELL_TYPE)) {
        g_atSendDataBuff.bufLen = length;

        return AT_ERROR;
    }

    switch (setCnf->ratType) {
        case MTA_AT_NETMON_CELL_INFO_GSM: {
            AT_NetMonFmtGsmSCellData(setCnf, &length);
            break;
        }
        case MTA_AT_NETMON_CELL_INFO_UTRAN_FDD: {
            AT_NetMonFmtUtranFddSCellData(setCnf, &length);
            break;
        }
#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
        case MTA_AT_NETMON_CELL_INFO_UTRAN_TDD: {
            AT_NetMonFmtUtranTddSCellData(setCnf, &length);
            break;
        }
#endif
#if (FEATURE_LTE == FEATURE_ON)
        case MTA_AT_NETMON_CELL_INFO_LTE: {
            AT_NetMonFmtEutranSCellData(setCnf, &length);
            break;
        }
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case MTA_AT_NETMON_CELL_INFO_NR: {
            AT_NetMonFmtNrSCellData(setCnf, &length);
            break;
        }
#endif

        default:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "^MONSC: NONE");
            break;
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaSetNetMonNCellCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NetmonCellInfo *setCnf = VOS_NULL_PTR;
    VOS_UINT16             length;
    VOS_UINT16             lengthTemp;

    /* 初始化 */
    rcvMsg = (AT_MTA_Msg *)msg;
    setCnf = (MTA_AT_NetmonCellInfo *)rcvMsg->content;

    length   = 0;

    if ((setCnf->result == MTA_AT_RESULT_ERROR) || (setCnf->cellType != MTA_NETMON_NCELL_TYPE)) {
        g_atSendDataBuff.bufLen = length;

        return AT_ERROR;
    }

    lengthTemp = 0;
    length     = 0;

    /* GSM邻区显示 */
    AT_NetMonFmtGsmNCellData(setCnf, lengthTemp, &length);

    /* UTRAN邻区显示 */
    lengthTemp = length;

    if (setCnf->u.nCellInfo.cellMeasTypeChoice == MTA_NETMON_UTRAN_FDD_TYPE) {
        AT_NetMonFmtUtranFddNCellData(setCnf, lengthTemp, &length);
    }
#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    else if (setCnf->u.nCellInfo.cellMeasTypeChoice == MTA_NETMON_UTRAN_TDD_TYPE) {
        AT_NetMonFmtUtranTddNCellData(setCnf, lengthTemp, &length);
    }
#endif
    else {
        /* 类型不对，不进行任何处理 */
    }

#if (FEATURE_LTE == FEATURE_ON)
    /* LTE邻区显示 */
    lengthTemp = length;

    AT_NetMonFmtEutranNCellData(setCnf, lengthTemp, &length);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /* NR邻区显示 */
    lengthTemp = length;

    AT_NetMonFmtNrNCellData(setCnf, lengthTemp, &length);
#endif

    /* 无邻区，返回NONE */
    if ((setCnf->u.nCellInfo.gsmNCellCnt + setCnf->u.nCellInfo.utranNCellCnt + setCnf->u.nCellInfo.lteNCellCnt +
         setCnf->u.nCellInfo.nrNCellCnt) == 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONNC: NONE");
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_RcvMtaSetNrCellBandCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NetmonCellInfo *setCnf = VOS_NULL_PTR;
    VOS_UINT16             length;

    /* 初始化 */
    rcvMsg = (AT_MTA_Msg *)msg;
    setCnf = (MTA_AT_NetmonCellInfo *)rcvMsg->content;

    length   = 0;

    if (setCnf->result == MTA_AT_RESULT_ERROR) {
        g_atSendDataBuff.bufLen = length;
        return AT_ERROR;
    }

    /* 如果收到特殊回复码，那么说明此时不在L下，或者不在ENDC状态，回复NONE */
    if ((setCnf->result == MTA_AT_RESULT_OPERATION_NOT_ALLOWED) ||
        (setCnf->ratType == MTA_AT_NETMON_CELL_INFO_RAT_BUTT)) {
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^NRCELLBAND: NONE");
        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }

    switch (setCnf->cellType) {
        case MTA_NETMON_SCELL_TYPE: {
            AT_NrCellBandFmtDataSa(setCnf, &length);
            break;
        }
        case MTA_NETMON_SSCELL_TYPE: {
            AT_NrCellBandFmtDataNsa(setCnf, &length);
            break;
        }
        default:
            g_atSendDataBuff.bufLen = length;
            return AT_ERROR;
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaSetNetMonSSCellCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg   = VOS_NULL_PTR;
    MTA_AT_NetmonCellInfo *cellInfo = VOS_NULL_PTR;
    VOS_UINT16             length;
    VOS_UINT32             msgLength;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    cellInfo = (MTA_AT_NetmonCellInfo *)rcvMsg->content;

    length    = 0;
    msgLength = sizeof(AT_MTA_Msg) - AT_DATA_DEFAULT_LENGTH + sizeof(MTA_AT_NetmonCellInfo) -
                VOS_MSG_HEAD_LENGTH;

    if (msgLength != VOS_GET_MSG_LEN(rcvMsg)) {
        AT_WARN_LOG("AT_RcvMtaSetNetMonSSCellCnf: WARNING: AT Length Is Wrong");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if ((cellInfo->result == MTA_AT_RESULT_ERROR) || (cellInfo->cellType != MTA_NETMON_SSCELL_TYPE)) {
        g_atSendDataBuff.bufLen = length;
        return AT_ERROR;
    }

    /* 如果收到特殊回复码，那么说明此时不在L下，回复NONE */
    if (cellInfo->result == MTA_AT_RESULT_OPERATION_NOT_ALLOWED) {
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^MONSSC: NONE");
        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }

    switch (cellInfo->ratType) {
        case MTA_AT_NETMON_CELL_INFO_NR: {
            AT_NetMonFmtNrSSCellData(cellInfo, &length);
            break;
        }

        default: {
            length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "^MONSSC: NONE");
            break;
        }
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}
#endif

#if (FEATURE_LTE == FEATURE_ON)
/*lint -save -e838 -specific(-e838)*/
/*lint -save -e826 -specific(-e826)*/

VOS_UINT32 AT_RcvMtaErrcCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *rcvMsg        = VOS_NULL_PTR;
    MTA_AT_ErrccapQryCnf *errcCapQryCnf = VOS_NULL_PTR;
    VOS_UINT32            result;

    /* 初始化 */
    rcvMsg        = (AT_MTA_Msg *)msg;
    errcCapQryCnf = (MTA_AT_ErrccapQryCnf *)rcvMsg->content;
    result        = AT_ERROR;

    g_atSendDataBuff.bufLen = 0;

    if (errcCapQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        /* ^ERRCCAPQRY:  */
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "^ERRCCAPQRY: %u,%u,%u,%u",
            errcCapQryCnf->errcCapType, errcCapQryCnf->para1, errcCapQryCnf->para2, errcCapQryCnf->para3);
        result = AT_OK;
    }

    return result;
}
/*lint -restore*/
/*lint -restore*/

VOS_UINT32 AT_RcvMtaEccCfgCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                  *rcvMsg       = VOS_NULL_PTR;
    MTA_AT_SET_ECC_CFG_CNF_STRU *eccCfgSetCnf = VOS_NULL_PTR;
    VOS_UINT32                   result;

    /* 初始化 */
    rcvMsg       = (AT_MTA_Msg *)msg;
    eccCfgSetCnf = (MTA_AT_SET_ECC_CFG_CNF_STRU *)rcvMsg->content;
    result       = AT_ERROR;

    /* 格式化命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (eccCfgSetCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result = AT_OK;
    }

    return result;
}
/*lint -save -e845 -specific(-e845)*/

VOS_VOID AT_FormatRsrp(VOS_UINT8 indexNum, MTA_AT_RsInfoQryCnf *rsInfoQryCnf)
{
    VOS_UINT32 i;
    VOS_UINT16 length = 0;

    /* 格式化AT+RSRP?查询命令返回 */
    g_atSendDataBuff.bufLen = 0;

    /* +RSRP: */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 判断查询操作是否成功 */
    if ((rsInfoQryCnf->result != MTA_AT_RESULT_NO_ERROR) || (rsInfoQryCnf->rsInfoRslt.rsInfoNum == 0)) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "NONE");

        g_atSendDataBuff.bufLen = length;
        return;
    }

    for (i = 0; i < TAF_MIN(rsInfoQryCnf->rsInfoRslt.rsInfoNum, AT_MAX_RS_INFO_NUM); i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", rsInfoQryCnf->rsInfoRslt.u.rsrpInfo[i].cellId);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rsInfoQryCnf->rsInfoRslt.u.rsrpInfo[i].earfcn);

        if (rsInfoQryCnf->rsInfoRslt.u.rsrpInfo[i].rsrp >= 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d.%02d",
                rsInfoQryCnf->rsInfoRslt.u.rsrpInfo[i].rsrp / AT_RS_INFO_MULTI,
                rsInfoQryCnf->rsInfoRslt.u.rsrpInfo[i].rsrp % AT_RS_INFO_MULTI);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",-%d.%02d",
                (-rsInfoQryCnf->rsInfoRslt.u.rsrpInfo[i].rsrp) / AT_RS_INFO_MULTI,
                (-rsInfoQryCnf->rsInfoRslt.u.rsrpInfo[i].rsrp) % AT_RS_INFO_MULTI);
        }

        if (i == rsInfoQryCnf->rsInfoRslt.rsInfoNum - 1) {
            break;
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_VOID AT_FormatRsrq(VOS_UINT8 indexNum, MTA_AT_RsInfoQryCnf *rsInfoQryCnf)
{
    VOS_UINT32 i;
    VOS_UINT16 length = 0;

    /* 格式化AT+RSRQ?查询命令返回 */
    g_atSendDataBuff.bufLen = 0;

    /* +RSRQ: */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 判断查询操作是否成功 */
    if ((rsInfoQryCnf->result != MTA_AT_RESULT_NO_ERROR) || (rsInfoQryCnf->rsInfoRslt.rsInfoNum == 0)) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "NONE");

        g_atSendDataBuff.bufLen = length;
        return;
    }

    for (i = 0; i < TAF_MIN(rsInfoQryCnf->rsInfoRslt.rsInfoNum, AT_MAX_RS_INFO_NUM); i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", rsInfoQryCnf->rsInfoRslt.u.rsrqInfo[i].cellId);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", rsInfoQryCnf->rsInfoRslt.u.rsrqInfo[i].earfcn);

        if (rsInfoQryCnf->rsInfoRslt.u.rsrqInfo[i].rsrq >= 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d.%02d",
                rsInfoQryCnf->rsInfoRslt.u.rsrqInfo[i].rsrq / AT_RS_INFO_MULTI,
                rsInfoQryCnf->rsInfoRslt.u.rsrqInfo[i].rsrq % AT_RS_INFO_MULTI);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",-%d.%02d",
                (-rsInfoQryCnf->rsInfoRslt.u.rsrqInfo[i].rsrq) / AT_RS_INFO_MULTI,
                (-rsInfoQryCnf->rsInfoRslt.u.rsrqInfo[i].rsrq) % AT_RS_INFO_MULTI);
        }

        if (i == rsInfoQryCnf->rsInfoRslt.rsInfoNum - 1) {
            break;
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    g_atSendDataBuff.bufLen = length;
}
/*lint -restore*/

/*lint -save -e838 -specific(-e838)*/
/*lint -save -e826 -specific(-e826)*/

VOS_UINT32 AT_RcvMtaRsInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *rcvMsg       = VOS_NULL_PTR;
    MTA_AT_RsInfoQryCnf *rsInfoQryCnf = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg       = (AT_MTA_Msg *)msg;
    rsInfoQryCnf = (MTA_AT_RsInfoQryCnf *)rcvMsg->content;

    if (rsInfoQryCnf->rsInfoRslt.rsInfoType == AT_MTA_RSRP_TYPE) {
        AT_FormatRsrp(indexNum, rsInfoQryCnf);
    } else {
        AT_FormatRsrq(indexNum, rsInfoQryCnf);
    }

    return AT_OK;
}
/*lint -restore*/
/*lint -restore*/
#endif

VOS_UINT32 AT_RcvMtaPseudBtsSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg         = VOS_NULL_PTR;
    MTA_AT_PseudBtsSetCnf *setPseudBtsCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT16             length = 0;

    rcvMsg         = (AT_MTA_Msg *)msg;
    setPseudBtsCnf = (MTA_AT_PseudBtsSetCnf *)(rcvMsg->content);

    if (setPseudBtsCnf->result == MTA_AT_RESULT_NO_ERROR) {
        if (setPseudBtsCnf->pseudBtsIdentType == AT_PSEUD_BTS_PARAM_TIMES) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                setPseudBtsCnf->u.pseudBtsIdentTimes);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                setPseudBtsCnf->u.pseudBtsIdentCap);
        }

        g_atSendDataBuff.bufLen = length;
        result                  = AT_OK;
    } else if (setPseudBtsCnf->result == MTA_AT_RESULT_INCORRECT_PARAMETERS) {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_CME_INCORRECT_PARAMETERS;
    } else if (setPseudBtsCnf->result == MTA_AT_RESULT_FUNC_DISABLE) {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_CME_FUNC_DISABLE;
    } else {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_ERROR;
    }

    return result;
}

VOS_UINT32 AT_RcvMtaSubClfSparamQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    MTA_AT_SubclfsparamQryCnf *subClfSparamQryCnf = VOS_NULL_PTR;
    AT_MTA_Msg                *rcvMsg             = VOS_NULL_PTR;
    VOS_UINT32                 result;
    VOS_UINT16                 length = 0;

    rcvMsg             = (AT_MTA_Msg *)msg;
    subClfSparamQryCnf = (MTA_AT_SubclfsparamQryCnf *)(rcvMsg->content);

    if (subClfSparamQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName,
            subClfSparamQryCnf->versionId);

        g_atSendDataBuff.bufLen = length;
        result                  = AT_OK;
    } else if (subClfSparamQryCnf->result == MTA_AT_RESULT_INCORRECT_PARAMETERS) {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_CME_INCORRECT_PARAMETERS;
    } else if (subClfSparamQryCnf->result == MTA_AT_RESULT_FUNC_DISABLE) {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_CME_FUNC_DISABLE;
    } else {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_ERROR;
    }

    return result;
}

VOS_UINT32 AT_RcvMtaOverHeatingQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *rcvMsg = VOS_NULL_PTR;
    MTA_AT_OverHeatingQryCnf *mtaCnf = VOS_NULL_PTR;
    VOS_UINT32                result;

    /* 初始化 */
    rcvMsg = (AT_MTA_Msg *)msg;
    mtaCnf = (MTA_AT_OverHeatingQryCnf *)rcvMsg->content;
    result = AT_OK;
    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ERROR;
        return result;
    }
#if (FEATURE_LTE == FEATURE_ON)

    if (mtaCnf->ratMode == MTA_AT_RAT_MODE_LTE) {
        g_atSendDataBuff.bufLen +=
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen,
                "%s:%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", g_parseContext[indexNum].cmdElement->cmdName,
                mtaCnf->lteOverHeatingSupportFlag, mtaCnf->lteLastReportParamFlag,
                mtaCnf->param1, mtaCnf->param2, mtaCnf->param3, mtaCnf->param4,
                mtaCnf->param5, mtaCnf->param6, mtaCnf->param7, mtaCnf->param8, mtaCnf->param9, mtaCnf->param10);
    }
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (mtaCnf->ratMode == MTA_AT_RAT_MODE_NR) {
        g_atSendDataBuff.bufLen +=
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen,
                "%s:%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", g_parseContext[indexNum].cmdElement->cmdName,
                mtaCnf->nrOverHeatingSupportFlag, mtaCnf->nrLastReportParamFlag,
                mtaCnf->param5, mtaCnf->param6, mtaCnf->param7, mtaCnf->param8, mtaCnf->param9, mtaCnf->param10,
                mtaCnf->param11, mtaCnf->param12, mtaCnf->param13, mtaCnf->param14);
    }
#endif

    return result;
}

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNrNwCapInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NrNwCapInfoQryCnf *mtaCnfContent = VOS_NULL_PTR;
    AT_RreturnCodeUint32 result;

    /* 初始化 */
    rcvMsg = (AT_MTA_Msg *)msg;
    mtaCnfContent = (MTA_AT_NrNwCapInfoQryCnf *)rcvMsg->content;

    g_atSendDataBuff.bufLen = 0;
    if (mtaCnfContent->result == MTA_AT_RESULT_NO_ERROR) {
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, mtaCnfContent->type,
                mtaCnfContent->capInfo.commPara.para1, mtaCnfContent->capInfo.commPara.para2,
                mtaCnfContent->capInfo.commPara.para3, mtaCnfContent->capInfo.commPara.para4,
                mtaCnfContent->capInfo.commPara.para5, mtaCnfContent->capInfo.commPara.para6,
                mtaCnfContent->capInfo.commPara.para7, mtaCnfContent->capInfo.commPara.para8,
                mtaCnfContent->capInfo.commPara.para9, mtaCnfContent->capInfo.commPara.para10);
        result = AT_OK;
    } else {
        result = AT_ConvertMtaResult(mtaCnfContent->result);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaNrNwCapInfoRptCfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NrNwCapInfoRptCfgQryCnf *mtaCnf = VOS_NULL_PTR;
    AT_RreturnCodeUint32 result;

    /* 初始化 */
    rcvMsg = (AT_MTA_Msg *)msg;
    mtaCnf = (MTA_AT_NrNwCapInfoRptCfgQryCnf *)rcvMsg->content;

    g_atSendDataBuff.bufLen = 0;
    if (mtaCnf->result == MTA_AT_RESULT_NO_ERROR) {
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s: %d,%d",
                g_parseContext[indexNum].cmdElement->cmdName, mtaCnf->type, mtaCnf->rptFlg);
        result = AT_OK;
    } else {
        result = AT_ConvertMtaResult(mtaCnf->result);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaNrCaCellInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NrCaCellInfoCnf *cnf = VOS_NULL_PTR;
    VOS_UINT32 i;
    VOS_UINT32 length = 0;
    AT_RreturnCodeUint32 result;

    rcvMsg = (AT_MTA_Msg *)msg;
    cnf = (MTA_AT_NrCaCellInfoCnf *)rcvMsg->content;

    /* 如果消息携带结果失败，返回ERROR */
    if (cnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ERROR;
    } else {
        result = AT_OK;

        cnf->cellNum = TAF_MIN(cnf->cellNum, MTA_AT_CA_MAX_CELL_NUM);
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
            (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + length,
            "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, cnf->cellNum);

        for (i = 0; i < cnf->cellNum; i++) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + length,
                ",%d,%d,%d,%d", cnf->cellInfo[i].cellIdx, cnf->cellInfo[i].dlConfigured,
                cnf->cellInfo[i].nulConfigured, cnf->cellInfo[i].sulConfigured);
        }
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)length;
    return result;
}

VOS_UINT32 AT_RcvMtaNrCaCellInfoRptCfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NrCaCellInfoRptCfgQryCnf *mtaCnf = VOS_NULL_PTR;
    VOS_UINT32 result;

    rcvMsg = (AT_MTA_Msg *)msg;
    mtaCnf = (MTA_AT_NrCaCellInfoRptCfgQryCnf *)rcvMsg->content;
    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            mtaCnf->caCellRptFlg);
        result = AT_OK;
    }

    return result;
}

#endif
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvMtaNrrcCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *rcvMsg        = VOS_NULL_PTR;
    MTA_AT_NrrccapQryCnf *nrrcCapQryCnf = VOS_NULL_PTR;
    VOS_UINT32            result;

    /* 初始化 */
    rcvMsg        = (AT_MTA_Msg *)msg;
    nrrcCapQryCnf = (MTA_AT_NrrccapQryCnf *)rcvMsg->content;
    g_atSendDataBuff.bufLen = 0;

    if (nrrcCapQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        /* ^NRRCCAPQRY:  */
        g_atSendDataBuff.bufLen +=
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen,
                "^NRRCCAPQRY: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", nrrcCapQryCnf->nrrcCfgNetMode,
                nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_0], nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_1],
                nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_2], nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_3],
                nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_4], nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_5],
                nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_6], nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_7],
                nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_8], nrrcCapQryCnf->para[AT_COMMAND_PARA_INDEX_9]);

        result = AT_OK;
    } else {
        result = AT_ConvertMtaResult(nrrcCapQryCnf->result);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaNrPowerSavingQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    MTA_AT_NrPowerSavingQryCnf *nrPowerSavingQryCnf = VOS_NULL_PTR;
    AT_MTA_Msg *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32  result;

    rcvMsg = (AT_MTA_Msg *)msg;
    nrPowerSavingQryCnf = (MTA_AT_NrPowerSavingQryCnf *)rcvMsg->content;
    g_atSendDataBuff.bufLen = 0;
    if (nrPowerSavingQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen,
            "^NRPOWERSAVINGQRY: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
            nrPowerSavingQryCnf->type, nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA0],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA1], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA2],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA3], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA4],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA5], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA6],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA7], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA8],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA9], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA10],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA11], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA12],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA13], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA14],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA15], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA16],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA17], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA18],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA19], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA20],
            nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA21], nrPowerSavingQryCnf->para[AT_NRPOWERSAVING_PARA22]);
        result = AT_OK;
    } else {
        result = AT_ConvertMtaResult(nrPowerSavingQryCnf->result);
    }

    return result;
}

VOS_UINT32 AT_PreCheckSsbIdData(MTA_AT_NrSsbIdQryCnf *ssbIdInfo)
{
    /* 如果连服务小区都没有，那么说明接入层上报逻辑错误，直接返回 */
    if (ssbIdInfo->spcellPresent == VOS_FALSE) {
        AT_WARN_LOG("AT_PreCheckSsbIdData : Rrc Error spcell not present.");
        return VOS_FALSE;
    }

    /* 如果邻区上报存在，但是个数为0，那么说明接入层上报逻辑错误，直接返回 */
    if ((ssbIdInfo->ncellPresent == VOS_TRUE) && (ssbIdInfo->ncellNum == 0)) {
        AT_WARN_LOG("AT_PreCheckSsbIdData : Rrc Error ncell present but cellnum is 0.");
        return VOS_FALSE;
    }

    /* 如果邻区上报不存在，但是个数不为0，那么说明接入层上报逻辑错误，直接返回 */
    if ((ssbIdInfo->ncellPresent == VOS_FALSE) && (ssbIdInfo->ncellNum != 0)) {
        AT_WARN_LOG("AT_PreCheckSsbIdData : Rrc Error ncell not present but cellnum is not 0.");
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_VOID AT_FmtNrSsbIdData(MTA_AT_NrSsbIdQryCnf *ssbIdInfo, VOS_UINT16 *outLen)
{
    VOS_UINT32 loopI;
    VOS_UINT32 loopJ;
    VOS_UINT16 length;
    VOS_UINT32 cgiHighBit;
    VOS_UINT32 cgiLowBit;

    if (AT_PreCheckSsbIdData(ssbIdInfo) != VOS_TRUE) {
        *outLen = 0;
        return;
    }

    length = 0;

    /* Cgi */
    cgiLowBit  = ssbIdInfo->spcellSsbInfo.cgiLow;
    cgiHighBit = ssbIdInfo->spcellSsbInfo.cgiHigh;
    /* 接入层逻辑保证标志位可以代表数据的正确性，所以不判断数据是否为无效值 */

    /* NR SSB info打印 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^NRSSBID:%u,%08x%08x,%u,%d,%d,%d", ssbIdInfo->spcellSsbInfo.arFcn,
        cgiHighBit, cgiLowBit, ssbIdInfo->spcellSsbInfo.phyCellId, ssbIdInfo->spcellSsbInfo.rsrp,
        ssbIdInfo->spcellSsbInfo.sinr, ssbIdInfo->spcellSsbInfo.ta);

    for (loopI = 0; loopI < MTA_AT_MAX_NR_SSB_BEAM_NUM; loopI++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d", ssbIdInfo->spcellSsbInfo.pscellMeasRslt[loopI].ssbId,
            ssbIdInfo->spcellSsbInfo.pscellMeasRslt[loopI].rsrp);
    }

    /* 邻区数目 */
    ssbIdInfo->ncellNum = AT_MIN(ssbIdInfo->ncellNum, MTA_AT_MAX_NR_NCELL_RPT_NUM);
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", ssbIdInfo->ncellNum);

    if (ssbIdInfo->ncellPresent == VOS_TRUE) {
        /* 邻区信息打印 */
        for (loopI = 0; loopI < ssbIdInfo->ncellNum; loopI++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%u,%u,%d,%d", ssbIdInfo->ncellSsbInfo[loopI].phyCellId,
                ssbIdInfo->ncellSsbInfo[loopI].arFcn, ssbIdInfo->ncellSsbInfo[loopI].rsrp,
                ssbIdInfo->ncellSsbInfo[loopI].sinr);

            for (loopJ = 0; loopJ < MTA_AT_MAX_NR_NCELL_BEAM_RPT_NUM; loopJ++) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d",
                    ssbIdInfo->ncellSsbInfo[loopI].ncellMeasRslt[loopJ].ssbId,
                    ssbIdInfo->ncellSsbInfo[loopI].ncellMeasRslt[loopJ].rsrp);
            }
        }
    }

    *outLen = length;
}

VOS_UINT32 AT_RcvMtaNrSsbIdQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *rcvMsg    = VOS_NULL_PTR;
    MTA_AT_NrSsbIdQryCnf *ssbIdInfo = VOS_NULL_PTR;
    VOS_UINT16            length;
    VOS_UINT32            msgLength;
    VOS_UINT32            rslt;

    /* 初始化 */
    rcvMsg    = (AT_MTA_Msg *)msg;
    ssbIdInfo = (MTA_AT_NrSsbIdQryCnf *)rcvMsg->content;

    length    = 0;
    msgLength = sizeof(AT_MTA_Msg) - AT_DATA_DEFAULT_LENGTH + sizeof(MTA_AT_NrSsbIdQryCnf) -
                VOS_MSG_HEAD_LENGTH;

    if (VOS_GET_MSG_LEN(rcvMsg) != msgLength) {
        AT_WARN_LOG("AT_RcvMtaNrSsbIdQryCnf: WARNING: AT Length Is Wrong");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if ((ssbIdInfo->result == MTA_AT_RESULT_ERROR) || (ssbIdInfo->result == MTA_AT_RESULT_OPTION_TIMEOUT)) {
        g_atSendDataBuff.bufLen = length;
        return AT_ERROR;
    }

    /* 如果收到特殊回复码MTA_AT_RESULT_INCORRECT_PARAMETERS，那么说明此时在LTE但是不在NSA下，回复AT_CME_OPERATION_NOT_SUPPORTED */
    if (ssbIdInfo->result == MTA_AT_RESULT_INCORRECT_PARAMETERS) {
        g_atSendDataBuff.bufLen = length;
        return AT_CME_OPERATION_NOT_SUPPORTED;
    }

    /* 如果收到特殊回复码MTA_AT_RESULT_OPERATION_NOT_ALLOWED，那么说明接入层回复失败，回复AT_CME_OPERATION_NOT_ALLOWED */
    if (ssbIdInfo->result == MTA_AT_RESULT_OPERATION_NOT_ALLOWED) {
        g_atSendDataBuff.bufLen = length;
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    AT_FmtNrSsbIdData(ssbIdInfo, &length);

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    if (length != 0) {
        rslt = AT_OK;
    } else {
        rslt = AT_ERROR;
    }

    return rslt;
}
#endif

VOS_UINT32 AT_RcvMtaQryNCellMonitorCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_NcellMonitorQryCnf *mtaAtQryCnf = (MTA_AT_NcellMonitorQryCnf *)mtaMsg->content;
    VOS_UINT32                 result;
    VOS_UINT16                 length = 0;

    if (mtaAtQryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        /* 查询时，还需要上报邻区状态 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            mtaAtQryCnf->switchFlag, mtaAtQryCnf->ncellState);
    }

    g_atSendDataBuff.bufLen = length;

    return result;
}

LOCAL VOS_UINT32 AT_IsSystemWebUiAntennaLevel4Range(VOS_UINT16 rscpTemp, VOS_UINT16 ecioTemp)
{
    if ((ecioTemp <= g_ecioCfg.value[AT_NVIM_ECIO_VALUE_INDEX_3]) &&
        (rscpTemp <= g_rscpCfg.value[AT_NVIM_RSCP_VALUE_INDEX_3])) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

LOCAL VOS_UINT32 AT_IsSystemWebUiAntennaLevel3Range(VOS_UINT16 rscpTemp, VOS_UINT16 ecioTemp)
{
    if (((ecioTemp <= g_ecioCfg.value[AT_NVIM_ECIO_VALUE_INDEX_2]) &&
         (ecioTemp > g_ecioCfg.value[AT_NVIM_ECIO_VALUE_INDEX_3]) &&
         (rscpTemp <= g_rscpCfg.value[AT_NVIM_RSCP_VALUE_INDEX_2])) ||
        ((rscpTemp <= g_rscpCfg.value[AT_NVIM_RSCP_VALUE_INDEX_2]) &&
         (rscpTemp > g_rscpCfg.value[AT_NVIM_RSCP_VALUE_INDEX_3]) &&
         (ecioTemp < g_ecioCfg.value[AT_NVIM_ECIO_VALUE_INDEX_2]))) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

LOCAL VOS_UINT32 AT_IsSystemWebUiAntennaLevel2Range(VOS_UINT16 rscpTemp, VOS_UINT16 ecioTemp)
{
    if (((ecioTemp <= g_ecioCfg.value[AT_NVIM_ECIO_VALUE_INDEX_1]) &&
         (ecioTemp > g_ecioCfg.value[AT_NVIM_ECIO_VALUE_INDEX_2]) &&
         (rscpTemp <= g_rscpCfg.value[AT_NVIM_RSCP_VALUE_INDEX_1])) ||
        ((rscpTemp <= g_rscpCfg.value[AT_NVIM_RSCP_VALUE_INDEX_1]) &&
         (rscpTemp > g_rscpCfg.value[AT_NVIM_RSCP_VALUE_INDEX_2]) &&
         (ecioTemp < g_ecioCfg.value[AT_NVIM_ECIO_VALUE_INDEX_1]))) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
LOCAL AT_CmdAntennaLevelUint8 AT_CalculateSystemWebUiAntennaLevel(VOS_INT16 rscp, VOS_INT16 ecio)
{
    VOS_UINT16              rscpTemp;
    VOS_UINT16              ecioTemp;
    AT_CmdAntennaLevelUint8 rlstAntennaLevel;

    /* 取绝对值 */
    rscpTemp = (VOS_UINT16)-rscp;
    ecioTemp = (VOS_UINT16)-ecio;

    if (AT_IsSystemWebUiAntennaLevel4Range(rscpTemp, ecioTemp) == VOS_TRUE) {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_4;
    } else if (AT_IsSystemWebUiAntennaLevel3Range(rscpTemp, ecioTemp) == VOS_TRUE) {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_3;
    } else if (AT_IsSystemWebUiAntennaLevel2Range(rscpTemp, ecioTemp) == VOS_TRUE) {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_2;
    } else {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_1;
    }
    return rlstAntennaLevel;
}

LOCAL VOS_UINT32 AT_IsDefaultSystemAntennaLevel4Range(VOS_UINT16 rscpTemp, VOS_UINT16 ecioTemp)
{
    /* RSCP and ECIO different Thresholds variables */
    VOS_INT32 alRscpThreshold[AT_ANTEN_LEV_MAX_NUM] = { 125, 108, 102, 96 };
    VOS_INT32 alEcioThreshold[AT_ANTEN_LEV_MAX_NUM] = { 30, 16, 13, 10 };

    if ((ecioTemp <= alEcioThreshold[AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_3]) &&
        (rscpTemp <= alRscpThreshold[AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_3])) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

LOCAL VOS_UINT32 AT_IsDefaultSystemAntennaLevel3Range(VOS_UINT16 rscpTemp, VOS_UINT16 ecioTemp)
{
    /* RSCP and ECIO different Thresholds variables */
    VOS_INT32 alRscpThreshold[AT_ANTEN_LEV_MAX_NUM] = { 125, 108, 102, 96 };
    VOS_INT32 alEcioThreshold[AT_ANTEN_LEV_MAX_NUM] = { 30, 16, 13, 10 };

    if (((ecioTemp <= alEcioThreshold[AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_2]) &&
         (ecioTemp > alEcioThreshold[AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_3]) &&
         (rscpTemp <= alRscpThreshold[AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_2])) ||
        ((rscpTemp <= alRscpThreshold[AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_2]) &&
         (rscpTemp > alRscpThreshold[AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_3]) &&
         (ecioTemp < alEcioThreshold[AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_2]))) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

LOCAL VOS_UINT32 AT_IsDefaultSystemAntennaLevel2Range(VOS_UINT16 rscpTemp, VOS_UINT16 ecioTemp)
{
    /* RSCP and ECIO different Thresholds variables */
    VOS_INT32 alRscpThreshold[AT_ANTEN_LEV_MAX_NUM] = { 125, 108, 102, 96 };
    VOS_INT32 alEcioThreshold[AT_ANTEN_LEV_MAX_NUM] = { 30, 16, 13, 10 };

    if (((ecioTemp <= alEcioThreshold[AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_1]) &&
         (ecioTemp > alEcioThreshold[AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_2]) &&
         (rscpTemp <= alRscpThreshold[AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_1])) ||
        ((rscpTemp <= alRscpThreshold[AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_1]) &&
         (rscpTemp > alRscpThreshold[AT_ANTEN_LEV_RSCP_THRESHOLD_INDEX_2]) &&
         (ecioTemp < alEcioThreshold[AT_ANTEN_LEV_ECIO_THRESHOLD_INDEX_1]))) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
LOCAL AT_CmdAntennaLevelUint8 AT_CalculateDefaultSystemAntennaLevel(VOS_INT16 rscp, VOS_INT16 ecio)
{
    VOS_UINT16              rscpTemp;
    VOS_UINT16              ecioTemp;
    AT_CmdAntennaLevelUint8 rlstAntennaLevel;

    /* 取绝对值 */
    rscpTemp = (VOS_UINT16)-rscp;
    ecioTemp = (VOS_UINT16)-ecio;

    if (AT_IsDefaultSystemAntennaLevel4Range(rscpTemp, ecioTemp) == VOS_TRUE) {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_4;
    } else if (AT_IsDefaultSystemAntennaLevel3Range(rscpTemp, ecioTemp) == VOS_TRUE) {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_3;
    } else if (AT_IsDefaultSystemAntennaLevel2Range(rscpTemp, ecioTemp) == VOS_TRUE) {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_2;
    } else {
        rlstAntennaLevel = AT_CMD_ANTENNA_LEVEL_1;
    }

    return rlstAntennaLevel;
}

AT_CmdAntennaLevelUint8 AT_CalculateAntennaLevel(VOS_INT16 rscp, VOS_INT16 ecio)
{
#if (FEATURE_LTE == FEATURE_ON)

    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
    systemAppConfig            = AT_GetSystemAppConfigAddr();

    if (*systemAppConfig == SYSTEM_APP_WEBUI) {
        return AT_CalculateSystemWebUiAntennaLevel(rscp, ecio);
    }
#endif

    return AT_CalculateDefaultSystemAntennaLevel(rscp, ecio);
}

VOS_VOID AT_GetSmoothAntennaLevel(VOS_UINT8 indexNum, AT_CmdAntennaLevelUint8 level)
{
    VOS_UINT8       i;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    /* 丢网时 立即更新  */
    if (level == AT_CMD_ANTENNA_LEVEL_0) {
        netCtx->calculateAntennaLevel = level;

        (VOS_VOID)memset_s(netCtx->antennaLevel, sizeof(netCtx->antennaLevel), 0x00, sizeof(netCtx->antennaLevel));
        return;
    }

    /* 丢网到有服务状态  立即更新  */
    if (netCtx->calculateAntennaLevel == AT_CMD_ANTENNA_LEVEL_0) {
        netCtx->calculateAntennaLevel = level;

        (VOS_VOID)memset_s(netCtx->antennaLevel, sizeof(netCtx->antennaLevel), 0x00, sizeof(netCtx->antennaLevel));
        netCtx->antennaLevel[AT_ANTENNA_LEVEL_MAX_NUM - 1] = level;
        return;
    }

    /* 与上次的信号格数比较, 变化比较大(超过1格)就立即更新 */
    if (level > (netCtx->antennaLevel[AT_ANTENNA_LEVEL_MAX_NUM - 1] + 1)) {
        netCtx->calculateAntennaLevel = level;

        (VOS_VOID)memset_s(netCtx->antennaLevel, sizeof(netCtx->antennaLevel), 0x00, sizeof(netCtx->antennaLevel));
        netCtx->antennaLevel[AT_ANTENNA_LEVEL_MAX_NUM - 1] = level;
        return;
    } else if ((level + 1) < netCtx->antennaLevel[AT_ANTENNA_LEVEL_MAX_NUM - 1]) {
        netCtx->calculateAntennaLevel = level;

        (VOS_VOID)memset_s(netCtx->antennaLevel, sizeof(netCtx->antennaLevel), 0x00, sizeof(netCtx->antennaLevel));
        netCtx->antennaLevel[AT_ANTENNA_LEVEL_MAX_NUM - 1] = level;
        return;
    } else {
        /* Do nothing... */
    }

    /* 先进先出存最近3次的查询结果 */
    for (i = 0; i < AT_ANTENNA_LEVEL_MAX_NUM - 1; i++) {
        netCtx->antennaLevel[i] = netCtx->antennaLevel[i + 1];
    }
    netCtx->antennaLevel[i] = level;

    /* 格数波动则不更新，以达到平滑的效果 */
    for (i = 0; i < AT_ANTENNA_LEVEL_MAX_NUM; i++) {
        if (netCtx->calculateAntennaLevel == netCtx->antennaLevel[i]) {
            return;
        }
    }

    /* 信号格数稳定了 AT_ANTENNA_LEVEL_MAX_NUM 次时才做更新 */
    netCtx->calculateAntennaLevel = level;
}

VOS_VOID AT_QryParaAnQueryProc(VOS_UINT8 indexNum, VOS_UINT8 opId, const VOS_UINT8 *para)
{
    errno_t                 memResult;
    VOS_UINT32              result;
    MN_MMA_AnqueryPara      anqueryPara;
    AT_CmdAntennaLevelUint8 curAntennaLevel;
    AT_ModemNetCtx         *netCtx = VOS_NULL_PTR;
#if (FEATURE_LTE == FEATURE_ON)
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
    VOS_INT16  rsrp;
    VOS_INT16  rsrq;
    VOS_UINT8  level;
    VOS_INT16  rssi;
#endif

    /* 初始化 */
    result = AT_OK;

    (VOS_VOID)memset_s(&anqueryPara, sizeof(anqueryPara), 0x00, sizeof(MN_MMA_AnqueryPara));
    memResult = memcpy_s(&anqueryPara, sizeof(anqueryPara), para, sizeof(MN_MMA_AnqueryPara));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(anqueryPara), sizeof(MN_MMA_AnqueryPara));

    if ((anqueryPara.serviceSysMode == TAF_MMA_RAT_GSM) || (anqueryPara.serviceSysMode == TAF_MMA_RAT_WCDMA)) {
        netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

        /*
         * 上报数据转换:将 Rscp、Ecio显示为非负值，若Rscp、Ecio为-145，-32，或者rssi为99，
         * 则转换为0
         */
        if (((anqueryPara.u.st2G3GCellSignInfo.cpichRscp == 0) && (anqueryPara.u.st2G3GCellSignInfo.cpichEcNo == 0)) ||
            (anqueryPara.u.st2G3GCellSignInfo.rssi == TAF_PH_RSSIUNKNOW)) {
            /* 丢网返回0, 对应应用的圈外 */
            curAntennaLevel = AT_CMD_ANTENNA_LEVEL_0;
        } else {
            /* 调用函数AT_CalculateAntennaLevel来根据D25算法计算出信号格数 */
            curAntennaLevel = AT_CalculateAntennaLevel(anqueryPara.u.st2G3GCellSignInfo.cpichRscp,
                                                       anqueryPara.u.st2G3GCellSignInfo.cpichEcNo);
        }

        /* 信号磁滞处理 */
        AT_GetSmoothAntennaLevel(indexNum, curAntennaLevel);

        anqueryPara.u.st2G3GCellSignInfo.cpichRscp = -anqueryPara.u.st2G3GCellSignInfo.cpichRscp;
        anqueryPara.u.st2G3GCellSignInfo.cpichEcNo = -anqueryPara.u.st2G3GCellSignInfo.cpichEcNo;

#if (FEATURE_LTE == FEATURE_ON)
        systemAppConfig = AT_GetSystemAppConfigAddr();

        if (*systemAppConfig == SYSTEM_APP_WEBUI) {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s:%d,%d,%d,%d,0,0", g_parseContext[indexNum].cmdElement->cmdName,
                (VOS_INT32)anqueryPara.u.st2G3GCellSignInfo.cpichRscp,
                (VOS_INT32)anqueryPara.u.st2G3GCellSignInfo.cpichEcNo,
                (VOS_INT32)anqueryPara.u.st2G3GCellSignInfo.rssi, (VOS_INT32)netCtx->calculateAntennaLevel);


            /* 回复用户命令结果 */
            At_FormatResultData(indexNum, result);

            return;
        }
#endif
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d,%d,%d,%d,0x%X", g_parseContext[indexNum].cmdElement->cmdName,
            (VOS_INT32)anqueryPara.u.st2G3GCellSignInfo.cpichRscp,
            (VOS_INT32)anqueryPara.u.st2G3GCellSignInfo.cpichEcNo, (VOS_INT32)anqueryPara.u.st2G3GCellSignInfo.rssi,
            (VOS_INT32)netCtx->calculateAntennaLevel, (VOS_INT32)anqueryPara.u.st2G3GCellSignInfo.cellId);

        /* 回复用户命令结果 */
        At_FormatResultData(indexNum, result);

        return;
    } else if (anqueryPara.serviceSysMode == TAF_MMA_RAT_LTE) {
#if (FEATURE_LTE == FEATURE_ON)
        rsrp = anqueryPara.u.st4GCellSignInfo.rsrp;
        rsrq = anqueryPara.u.st4GCellSignInfo.rsrq;
        rssi = anqueryPara.u.st4GCellSignInfo.rssi;
        level = 0;

        AT_CalculateLTESignalValue(&rssi, &level, &rsrp, &rsrq);

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:0,99,%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            (VOS_INT32)rssi, (VOS_INT32)level, (VOS_INT32)rsrp, (VOS_INT32)rsrq);


        /* 回复用户命令结果 */
        At_FormatResultData(indexNum, result);

        return;
#endif
    } else {
        AT_WARN_LOG("AT_QryParaAnQueryProc:WARNING: THE RAT IS INVALID!");
        return;
    }
}

VOS_UINT32 AT_RcvMtaAnqueryQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *rcvMsg            = VOS_NULL_PTR;
    MTA_AT_AntennaInfoQryCnf *antennaInfoQryCnf = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg            = (AT_MTA_Msg *)msg;
    antennaInfoQryCnf = (MTA_AT_AntennaInfoQryCnf *)rcvMsg->content;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (antennaInfoQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        AT_QryParaAnQueryProc(indexNum, rcvMsg->appCtrl.opId, (VOS_UINT8 *)&(antennaInfoQryCnf->antennaInfo));
    } else {
        At_FormatResultData(indexNum, AT_CME_UNKNOWN);
    }

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_INT16 AT_GetValidValue(VOS_INT16 curValue, VOS_INT16 minValue, VOS_INT16 maxValue)
{
    VOS_INT16 validValue;

    if (curValue > maxValue) {
        validValue = maxValue;
    } else if (curValue < minValue) {
        validValue = minValue;
    } else {
        validValue = curValue;
    }
    return validValue;
}

VOS_UINT32 atScellInfoQryCnfProc(struct MsgCB *msgBlock)
{
    L4A_READ_ScellInfoCnf *scellInfoCnf = VOS_NULL_PTR;
    VOS_UINT16             length = 0;
    VOS_UINT32             i;
    VOS_INT16              rssi;
    VOS_INT16              rsrp;
    VOS_INT16              rsrq;

    scellInfoCnf = (L4A_READ_ScellInfoCnf *)msgBlock;

    if (scellInfoCnf->errorCode != AT_SUCCESS) {
        CmdErrProc((VOS_UINT8)(scellInfoCnf->clientId), scellInfoCnf->errorCode, 0, NULL);
        return AT_FW_CLIENT_STATUS_READY;
    }

    scellInfoCnf->sCellCnt = AT_MIN(scellInfoCnf->sCellCnt, LRRC_SCELL_MAX_NUM);
    for (i = 0; i < scellInfoCnf->sCellCnt; i++) {
        /* 不是首次进入，需要打印回车换行 */
        if (length != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "%s", g_atCrLf);
        }

        /* 如果 RSSI\RSRP\RSRQ都为0直接上传不处理 */
        if ((scellInfoCnf->sCellInfo[i].rssi == 0) &&
            (scellInfoCnf->sCellInfo[i].rsrp == 0) &&
            (scellInfoCnf->sCellInfo[i].rsrq == 0)) {
            rssi = scellInfoCnf->sCellInfo[i].rssi;
            rsrp = scellInfoCnf->sCellInfo[i].rsrp;
            rsrq = scellInfoCnf->sCellInfo[i].rsrq;
        } else {
            rssi = AT_GetValidValue(scellInfoCnf->sCellInfo[i].rssi,
                                    AT_CASCELL_RSSI_VALUE_MIN,
                                    AT_CASCELL_RSSI_VALUE_MAX);
            rsrp = AT_GetValidValue(scellInfoCnf->sCellInfo[i].rsrp,
                                    AT_CASCELL_RSRP_VALUE_MIN,
                                    AT_CASCELL_RSRP_VALUE_MAX);
            rsrq = AT_GetValidValue(scellInfoCnf->sCellInfo[i].rsrq,
                                    AT_CASCELL_RSRQ_VALUE_MIN,
                                    AT_CASCELL_RSRQ_VALUE_MAX);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "^CASCELLINFO: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            scellInfoCnf->sCellInfo[i].scellIndex, scellInfoCnf->sCellInfo[i].phyCellId,
            rssi, rsrp, rsrq, scellInfoCnf->sCellInfo[i].freqBandIndicator,scellInfoCnf->sCellInfo[i].scellUlFreqPoint,
            scellInfoCnf->sCellInfo[i].scellDlFreqPoint,scellInfoCnf->sCellInfo[i].scellUlFreq,
            scellInfoCnf->sCellInfo[i].scellDlFreq,scellInfoCnf->sCellInfo[i].scellUlBandWidth,
            scellInfoCnf->sCellInfo[i].scellDlBandWidth);
    }

    CmdErrProc((VOS_UINT8)(scellInfoCnf->clientId), scellInfoCnf->errorCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 AT_RcvMtaWrrRrcVersionSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_WrrRrcVersionSetCnf *wrrVersionSetCnf = (MTA_AT_WrrRrcVersionSetCnf *)mtaMsg->content;
    VOS_UINT32 result;

    /* 格式化AT^FREQLOCK命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (wrrVersionSetCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        /* 设置成功后重新读写一下NV */
        AT_ReadWasCapabilityNV();

        result = AT_OK;
    }

    return result;
}

VOS_UINT32 AT_RcvMtaWrrRrcVersionQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_WrrRrcVersionQryCnf *wrrVersionQryCnf = (MTA_AT_WrrRrcVersionQryCnf *)mtaMsg->content;
    VOS_UINT32 result = AT_OK;

    /* 格式化AT^FREQLOCK查询命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (wrrVersionQryCnf->result != VOS_OK) {
        result = AT_CME_UNKNOWN;
    } else {
        result = AT_OK;

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            (TAF_INT32)wrrVersionQryCnf->rrcVersion);
    }

    return result;
}

VOS_UINT32 atSetAnlevelCnfSameProc(struct MsgCB *msgBlock)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 result;
    VOS_UINT16 rsrp = 0;
    VOS_UINT16 rsrq = 0;

    AT_AnlevelInfoCnf anlevelAnqueryInfo = {0};
    g_ate5Order                          = 1;

    result = at_CsqInfoProc(msgBlock, &anlevelAnqueryInfo);

    if (result == ERR_MSP_SUCCESS) {
        rsrp = (VOS_UINT16)((anlevelAnqueryInfo.rsrp == AT_ANLEVEL_INFO_RSRP_UNVALID) ?
                AT_ANLEVEL_INFO_RSRP_UNVALID : (-(anlevelAnqueryInfo.rsrp)));
        rsrq = (VOS_UINT16)((anlevelAnqueryInfo.rsrq == AT_ANLEVEL_INFO_RSRQ_UNVALID) ?
                AT_ANLEVEL_INFO_RSRQ_UNVALID : (-(anlevelAnqueryInfo.rsrq)));

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "%s%s0,99,%d,%d,%d,%d%s", g_atCrLf, "^ANQUERY:",
            anlevelAnqueryInfo.rssi, anlevelAnqueryInfo.level, rsrp, rsrq, g_atCrLf);
    } else {
        HAL_SDMLOG(" atSetAnlevelCnfSameProc ulResult = %d,\n", (VOS_INT)result);
    }

    CmdErrProc((VOS_UINT8)(anlevelAnqueryInfo.clientId), anlevelAnqueryInfo.errorCode, length, g_atSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

