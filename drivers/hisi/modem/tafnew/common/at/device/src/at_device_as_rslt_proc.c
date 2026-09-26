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
#include "at_device_as_rslt_proc.h"
#include "securec.h"

#include "nv_stru_was.h"
#include "nv_stru_gas.h"
#include "nv_stru_lps.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"
#include "at_external_module_msg_proc.h"
#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"

#include "at_mta_interface.h"
#include "at_mdrv_interface.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_AS_RSLT_PROC_C

/* 主线现在支持140个频段大于128扩展为256个最大频段支持 */
#define AT_FEATURE_LTE_MAX_SUPPORT_BAND 256

#define AT_HSPA_STATUS_ACTIVATE 1 /* 0表示未激活,那么DPA和UPA都支持;1表示激活 */

#define AT_SFEATURE_TMP_STRING_LEN 50

VOS_UINT32 AT_GetWcdmaBandStr(VOS_UINT8 *gsmBandstr, VOS_UINT32 gsmBandLen, AT_UE_BandCapaSt *bandCapa)
{
    VOS_UINT32 len = 0;

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaI2100 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC1,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaIi1900 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC2,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaIii1800 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC3,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaIv1700 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC4,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaV850 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC5,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaVi800 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC6,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaVii2600 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC7,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaViii900 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC8,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaIxJ1700 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC9,");
    }

    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaXi1500 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "BC11,");
    }
    if (bandCapa->unWRFSptBand.bitBand.bandWcdmaXix850 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "BC19,");
    }

    if (len != 0) {
        gsmBandstr[len - 1] = '\0';
    }

    return len;
}

VOS_VOID AT_SupportHsdpa(AT_NvimUeCapa *uECapa, VOS_BOOL *pbSupportHsdpa)
{
    /* 初始化为不支持DPA */
    *pbSupportHsdpa = VOS_FALSE;

    if (uECapa->asRelIndicator >= AT_PTL_VER_ENUM_R5) {
        if (uECapa->hspaStatus == AT_HSPA_STATUS_ACTIVATE) {
            if (uECapa->hsdschSupport == PS_TRUE) {
                *pbSupportHsdpa = VOS_TRUE;
            }
        } else {
            *pbSupportHsdpa = VOS_TRUE;
        }
    }
}

VOS_VOID AT_SupportHsupa(AT_NvimUeCapa *uECapa, VOS_BOOL *pbSupportHsupa)
{
    /* 初始化为不支持UPA */
    *pbSupportHsupa = VOS_FALSE;

    if (uECapa->asRelIndicator >= AT_PTL_VER_ENUM_R6) {
        if (uECapa->hspaStatus == AT_HSPA_STATUS_ACTIVATE) {
            if (uECapa->edchSupport == PS_TRUE) {
                *pbSupportHsupa = VOS_TRUE;
            }
        } else {
            *pbSupportHsupa = VOS_TRUE;
        }
    }
}

VOS_UINT32 AT_GetWcdmaDivBandStr(VOS_UINT8 *gsmBandstr, VOS_UINT32 gsmBandLen)
{
    VOS_UINT32 len;
    VOS_UINT32 band;

    AT_WCDMA_PrefBand *bitBand = VOS_NULL_PTR;

    /*
     * 获取UMTS支持的分集频带集合
     * V3R2版本是en_NV_Item_W_RF_DIV_BAND，V3R1是en_NV_Item_ANTENNA_CONFIG
     */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_W_RF_DIV_BAND, &band, sizeof(band)) != NV_OK) {
        AT_WARN_LOG("AT_GetWcdmaDivBandStr: Read NVIM Smss Error");
        return 0;
    }

    len     = 0;
    bitBand = (AT_WCDMA_PrefBand *)&band;

    /*
     * 单板分集支持的BAND 通路，数据为十进制数，转化为二进制后
     * 从右往左依次为Band1、2、3……。
     */

    if (bitBand->bandWcdmaI2100 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U1,");
    }

    if (bitBand->bandWcdmaIi1900 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U2,");
    }

    if (bitBand->bandWcdmaIii1800 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U3,");
    }

    if (bitBand->bandWcdmaIv1700 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U4,");
    }

    if (bitBand->bandWcdmaV850 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U5,");
    }

    if (bitBand->bandWcdmaVi800 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U6,");
    }

    if (bitBand->bandWcdmaVii2600 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U7,");
    }

    if (bitBand->bandWcdmaViii900 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U8,");
    }

    if (bitBand->bandWcdmaIxJ1700 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U9,");
    }

    if (bitBand->bandWcdmaXi1500 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U11,");
    }

    if (bitBand->bandWcdmaXix850 == 1) {
        len += (VOS_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (VOS_CHAR *)gsmBandstr,
            (VOS_CHAR *)gsmBandstr + len, "U19,");
    }

    if (len != 0) {
        gsmBandstr[len - 1] = '\0';
    }

    return len;
}

VOS_UINT32 AT_GetWFeatureInfo(AT_FEATURE_SUPPORT_ST *feATure, MTA_AT_SfeatureQryCnf *atAgentSfeatureQryCnf)
{
    AT_NvimUeCapa uECapa;
    VOS_UINT32    len;
    VOS_BOOL      bSupportHsdpa = VOS_FALSE;
    VOS_BOOL      bSupportHsupa = VOS_FALSE;
    VOS_UINT8     strTmp[AT_SFEATURE_TMP_STRING_LEN] = {0};
    VOS_UINT8     strDiv[AT_SFEATURE_DIV_STRING_LEN];
    VOS_UINT32    divLen;
    errno_t       memResult;

    (VOS_VOID)memset_s(&uECapa, sizeof(uECapa), 0x00, sizeof(uECapa));

    /* 读取HSPA,HSPA+支持能力 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, &uECapa, sizeof(AT_NvimUeCapa)) != NV_OK) {
        AT_WARN_LOG("AT_GetWFeatureInfo: en_NV_Item_WAS_RadioAccess_Capa NV Read  Fail!");
        return VOS_ERR;
    }

    /* 获取Wcdma频段信息 */
    /* !!!如果增加频段，aucStrTmp 长度需要增加!!! */
    len = (VOS_UINT32)AT_GetWcdmaBandStr(strTmp, sizeof(strTmp), &(atAgentSfeatureQryCnf->bandFeature));

    /* HSPA+是否支持 */
    if ((uECapa.asRelIndicator >= PS_PTL_VER_R7) && (uECapa.macEhsSupport == PS_TRUE) &&
        (uECapa.hsdschPhyCategory >= AT_HSDSCH_PHY_CATEGORY_10) && (uECapa.hsdschPhyCategoryExt != 0) &&
        (uECapa.hsdschSupport == PS_TRUE)) {
        feATure[AT_FEATURE_HSPAPLUS].featureFlag = AT_FEATURE_EXIST;
        if (len > 0) {
            memResult = memcpy_s(feATure[AT_FEATURE_HSPAPLUS].content, sizeof(feATure[AT_FEATURE_HSPAPLUS].content),
                                 strTmp, len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_HSPAPLUS].content), len);
        }
    }

    /* DPA是否支持 */
    AT_SupportHsdpa(&uECapa, &bSupportHsdpa);
    if (bSupportHsdpa == VOS_TRUE) {
        feATure[AT_FEATURE_HSDPA].featureFlag = AT_FEATURE_EXIST;
        if (len > 0) {
            memResult = memcpy_s(feATure[AT_FEATURE_HSDPA].content, sizeof(feATure[AT_FEATURE_HSDPA].content), strTmp,
                                 len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_HSDPA].content), len);
        }
    }

    /* UPA是否支持 */
    AT_SupportHsupa(&uECapa, &bSupportHsupa);
    if (bSupportHsupa == VOS_TRUE) {
        feATure[AT_FEATURE_HSUPA].featureFlag = AT_FEATURE_EXIST;
        if (len > 0) {
            memResult = memcpy_s(feATure[AT_FEATURE_HSUPA].content, sizeof(feATure[AT_FEATURE_HSUPA].content), strTmp,
                                 len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_HSUPA].content), len);
        }
    }

    /* 分集信息 */
    feATure[AT_FEATURE_DIVERSITY].featureFlag = AT_FEATURE_EXIST;
    /* UMTS的分集信息输出 */
    (VOS_VOID)memset_s(strDiv, sizeof(strDiv), 0x00, sizeof(strDiv));

    /* !!!如果增加频段，aucStrDiv 长度需要增加!!! */
    divLen = (VOS_UINT32)AT_GetWcdmaDivBandStr(strDiv, sizeof(strDiv));

    if (divLen > 0) {
        memResult = memcpy_s(feATure[AT_FEATURE_DIVERSITY].content, sizeof(feATure[AT_FEATURE_DIVERSITY].content),
                             strDiv, divLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_DIVERSITY].content), divLen);
    }

    /* UMTS */
    feATure[AT_FEATURE_UMTS].featureFlag = AT_FEATURE_EXIST;
    if (len > 0) {
        memResult = memcpy_s(feATure[AT_FEATURE_UMTS].content, sizeof(feATure[AT_FEATURE_UMTS].content), strTmp, len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_UMTS].content), len);
    }

    return VOS_OK;
}

VOS_UINT32 AT_GetLteFeatureInfo(AT_FEATURE_SUPPORT_ST *feATure)
{
    VOS_UINT32 rst;

    VOS_UINT8 bandStr[AT_FEATURE_BAND_STR_LEN_MAX] = {0};
    VOS_UINT8 bandFlag = 0;

    VOS_UINT32 strlen1 = 0;
    VOS_UINT32 i       = 0;

    VOS_UINT32 bandNv                                       = 0;
    VOS_UINT8 bandNum[AT_FEATURE_LTE_MAX_SUPPORT_BAND + 1] = {0};
    LRRC_NV_UE_EUTRA_CAP_STRU *eutraCap = NULL;

    eutraCap = VOS_MemAlloc(WUEPS_PID_AT, (DYNAMIC_MEM_PT), sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));

    if (eutraCap == NULL) {
        return ERR_MSP_MALLOC_FAILUE;
    }

    memset_s(eutraCap, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU), 0x00, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));

    rst = TAF_ACORE_NV_READ(MODEM_ID_0, EN_NV_ID_UE_CAPABILITY, eutraCap, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));
    if (rst != ERR_MSP_SUCCESS) {
        VOS_MemFree(WUEPS_PID_AT, eutraCap);
        return ERR_MSP_FAILURE;
    }

    /* RRC_MAX_NUM_OF_BANDS */

    eutraCap->stRfPara.usCnt = AT_MIN(eutraCap->stRfPara.usCnt, RRC_MAX_NUM_OF_BANDS);
    for (i = 0; i < eutraCap->stRfPara.usCnt; i++) {
        if (eutraCap->stRfPara.astSuppEutraBandList[i].ucEutraBand > 0) {
            bandNv = eutraCap->stRfPara.astSuppEutraBandList[i].ucEutraBand;
            if (bandNv < (AT_FEATURE_LTE_MAX_SUPPORT_BAND + 1)) {
                bandNum[bandNv] = 1;
                bandFlag++;
            }
        }
    }

    /* 解析获取LTE band信息 */

    if (bandFlag > 0) {
        feATure[AT_FEATURE_LTE].featureFlag = AT_FEATURE_EXIST;
    } else {
        VOS_MemFree(WUEPS_PID_AT, eutraCap);
        return ERR_MSP_FAILURE;
    }

    for (i = 1; i < AT_FEATURE_LTE_MAX_SUPPORT_BAND + 1; i++) {
        if (bandNum[i] == 1) {
            strlen1 += (VOS_UINT32)AT_FormatReportString(AT_FEATURE_BAND_STR_LEN_MAX, (VOS_CHAR *)bandStr,
                (VOS_CHAR *)(bandStr + strlen1), "B%d,", i);
        }
    }

    strlen1 = AT_MIN(strlen1, AT_FEATURE_CONTENT_LEN_MAX);
    for (i = 1; i < strlen1; i++) {
        feATure[AT_FEATURE_LTE].content[i - 1] = bandStr[i - 1];
    }

    for (i = 0; i < strlen1; i++) {
        bandStr[i] = 0;
    }

    VOS_MemFree(WUEPS_PID_AT, eutraCap);
    return ERR_MSP_SUCCESS;
}

#if (FEATURE_LTEV == FEATURE_ON)
VOS_VOID AT_GetLtevFeatureInfo(AT_FEATURE_SUPPORT_ST *featureSupport, MTA_AT_SfeatureQryCnf *featureQryCnf)
{
    VOS_UINT32 strLen = 0;
    VOS_UINT32 bandNum;
    VOS_UINT32 i;

    bandNum = AT_MIN(featureQryCnf->bandFeature.ltevBand.bandNum, MTA_AT_LTEV_BAND_MAX_NUM);
    if (bandNum != 0) {
        featureSupport[AT_FEATURE_LTEV].featureFlag = AT_FEATURE_EXIST;
    } else {
        return;
    }
    for (i = 0; i < bandNum; i++) {
        if (featureQryCnf->bandFeature.ltevBand.opBand[i] != 0) {
            strLen += (VOS_UINT32)AT_FormatReportString(AT_FEATURE_CONTENT_LEN_MAX,
                (VOS_CHAR *)(featureSupport[AT_FEATURE_LTEV].content),
                (VOS_CHAR *)(&featureSupport[AT_FEATURE_LTEV].content[strLen]), "B%d,",
                featureQryCnf->bandFeature.ltevBand.opBand[i]);
        }
    }
    /* 去掉最后一个逗号 */
    if (strLen > 0) {
        featureSupport[AT_FEATURE_LTEV].content[--strLen] = '\0';
    }
}
#endif

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
VOS_UINT32 AT_GetTdsFeatureInfo(AT_FEATURE_SUPPORT_ST *feATure)
{
    VOS_UINT32                    rst;
    NVIM_UeTdsSupportFreqBandList tdsFeature;
    VOS_UINT32                    i = 0;
    VOS_UINT32                    len;
    VOS_INT32                     bufLen;

    (VOS_VOID)memset_s(&tdsFeature, sizeof(tdsFeature), 0x00, sizeof(tdsFeature));

    rst = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_UTRAN_TDD_FREQ_BAND, &tdsFeature,
                            sizeof(NVIM_UeTdsSupportFreqBandList));
    if (rst != ERR_MSP_SUCCESS) {
        PS_PRINTF_WARNING("Read EN_NV_ID_TDS_SUPPORT_FREQ_BAND failed:0x%x.\n", NV_ITEM_UTRAN_TDD_FREQ_BAND);
        return ERR_MSP_FAILURE;
    }
    if (tdsFeature.bandCnt > NVIM_TDS_MAX_SUPPORT_BANDS_NUM) {
        PS_PRINTF_WARNING("Read NV_ITEM_UTRAN_TDD_FREQ_BAND stTdsFeature.ucBandCnt:%d.\n", tdsFeature.bandCnt);
        return ERR_MSP_FAILURE;
    }

    /* 读取NV成功，支持TDS */
    feATure[AT_FEATURE_TDSCDMA].featureFlag = AT_FEATURE_EXIST;

    len = 0;

    /* BandA:2000Hz, BanE:2300Hz, BandF:1900Hz */
    for (i = 0; i < tdsFeature.bandCnt; i++) {
        if (tdsFeature.bandNo[i] == ID_NVIM_TDS_FREQ_BAND_A) {
            bufLen = snprintf_s((VOS_CHAR *)(feATure[AT_FEATURE_TDSCDMA].content + len),
                                AT_FEATURE_CONTENT_LEN_MAX - len, (AT_FEATURE_CONTENT_LEN_MAX - len) - 1, "2000,");
            TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, AT_FEATURE_CONTENT_LEN_MAX - len,
                                             (AT_FEATURE_CONTENT_LEN_MAX - len) - 1);
            len = VOS_StrLen((VOS_CHAR *)feATure[AT_FEATURE_TDSCDMA].content);
            continue;
        }

        if (tdsFeature.bandNo[i] == ID_NVIM_TDS_FREQ_BAND_E) {
            bufLen = snprintf_s((VOS_CHAR *)(feATure[AT_FEATURE_TDSCDMA].content + len),
                                AT_FEATURE_CONTENT_LEN_MAX - len, (AT_FEATURE_CONTENT_LEN_MAX - len) - 1, "2300,");
            TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, AT_FEATURE_CONTENT_LEN_MAX - len,
                                             (AT_FEATURE_CONTENT_LEN_MAX - len) - 1);
            len = VOS_StrLen((VOS_CHAR *)feATure[AT_FEATURE_TDSCDMA].content);
            continue;
        }

        if (tdsFeature.bandNo[i] == ID_NVIM_TDS_FREQ_BAND_F) {
            bufLen = snprintf_s((VOS_CHAR *)(feATure[AT_FEATURE_TDSCDMA].content + len),
                                AT_FEATURE_CONTENT_LEN_MAX - len, (AT_FEATURE_CONTENT_LEN_MAX - len) - 1, "1900,");
            TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, AT_FEATURE_CONTENT_LEN_MAX - len,
                                             (AT_FEATURE_CONTENT_LEN_MAX - len) - 1);
            len = VOS_StrLen((VOS_CHAR *)feATure[AT_FEATURE_TDSCDMA].content);
            continue;
        }
    }

    if (len > 0) {
        /* 屏蔽掉最后一个逗号 */
        feATure[AT_FEATURE_TDSCDMA].content[len - 1] = '\0';
    }

    return ERR_MSP_SUCCESS;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_VOID AT_GetNrFeatureInfo(MTA_NRRC_BandInfo nrBandInfo, AT_FEATURE_SUPPORT_ST *feATure)
{
    VOS_UINT32 i;
    VOS_UINT32 strLen = 0;
    VOS_UINT32 bandCnt;
    errno_t    memResult;
    VOS_UINT8  tempStr[AT_FEATURE_BAND_STR_LEN_MAX];

    /* 初始化 */
    (VOS_VOID)memset_s(tempStr, sizeof(tempStr), 0x0, sizeof(tempStr));

    /* 设置Flag标志 */
    feATure[AT_FEATURE_NR].featureFlag = AT_FEATURE_EXIST;

    bandCnt = (nrBandInfo.bandCnt > AT_MTA_UECAP_MAX_NR_BAND_NUM) ? AT_MTA_UECAP_MAX_NR_BAND_NUM : nrBandInfo.bandCnt;

    for (i = 0; i < bandCnt; i++) {
        strLen += (VOS_UINT32)AT_FormatReportString(AT_FEATURE_BAND_STR_LEN_MAX, (VOS_CHAR *)tempStr,
            (VOS_CHAR *)(tempStr + strLen), "B%d,", nrBandInfo.bandInfo[i]);
    }

    if (strLen > 0) {
        /* 屏蔽掉最后一个逗号 */
        tempStr[strLen - 1] = '\0';

        memResult = memcpy_s(feATure[AT_FEATURE_NR].content, sizeof(feATure[AT_FEATURE_NR].content), tempStr, strLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_NR].content), strLen);
    }
}

VOS_UINT32 AT_RcvMtaQryTrxTasCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_QryTrxTasCnf *qryCnf = (MTA_AT_QryTrxTasCnf *)mtaMsg->content;
    VOS_UINT32           result;

    if (qryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, qryCnf->ratMode,
            qryCnf->trxTasValue);
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}
#endif

VOS_UINT32 AT_GetGsmBandStr(VOS_UINT8 *gsmBandstr, VOS_UINT32 gsmBandLen, AT_UE_BandCapaSt *bandCapa)
{
    VOS_UINT32 len = 0;

    if (bandCapa->unGRFSptBand.bitBand.bandGsm450 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "450,");
    }

    if (bandCapa->unGRFSptBand.bitBand.bandGsm480 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "480,");
    }

    if (bandCapa->unGRFSptBand.bitBand.bandGsm850 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "850,");
    }

    if (bandCapa->unGRFSptBand.bitBand.bandGsmP900 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "P900,");
    }
    if (bandCapa->unGRFSptBand.bitBand.bandGsmR900 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "R900,");
    }
    if (bandCapa->unGRFSptBand.bitBand.bandGsmE900 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "E900,");
    }
    if (bandCapa->unGRFSptBand.bitBand.bandGsm1800 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "1800,");
    }
    if (bandCapa->unGRFSptBand.bitBand.bandGsm1900 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "1900,");
    }
    if (bandCapa->unGRFSptBand.bitBand.bandGsm700 == 1) {
        len += (TAF_UINT32)AT_FormatReportString((VOS_INT32)gsmBandLen, (TAF_CHAR *)gsmBandstr,
            (TAF_CHAR *)gsmBandstr + len, "700,");
    }

    if (len != 0) {
        gsmBandstr[len - 1] = '\0';
    }

    return len;
}

VOS_VOID AT_GetGFeatureInfo(AT_FEATURE_SUPPORT_ST *feATure, MTA_AT_SfeatureQryCnf *aTAgentSfeatureQryCnf)
{
    VOS_UINT32 len;
    errno_t    memResult;

    VOS_UINT16 egprsFlag = 0;
    VOS_UINT8  strTmp[AT_SFEATURE_TMP_STRING_LEN] = {0};

    /* 获取GSM频段信息 */
    /* !!!如果增加频段，aucStrDiv 长度需要增加!!! */
    len = (VOS_UINT32)AT_GetGsmBandStr(strTmp, sizeof(strTmp), &(aTAgentSfeatureQryCnf->bandFeature));

    if (len == 0) {
        PS_PRINTF_WARNING("<AT_GetGFeatureInfo> Not Support G.\n");
        return;
    }

    /* 读取EDGE支持能力 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_EGPRS_FLAG, &egprsFlag, sizeof(VOS_UINT16)) != NV_OK) {
        AT_WARN_LOG("AT_GetGFeatureInfo: NV_ITEM_EGPRS_FLAG NV Read  Fail!");
        feATure[AT_FEATURE_EDGE].featureFlag = AT_FEATURE_EXIST;
        memResult = memcpy_s(feATure[AT_FEATURE_EDGE].content, sizeof(feATure[AT_FEATURE_EDGE].content), strTmp, len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_EDGE].content), len);
    } else {
        if (egprsFlag != 0) {
            feATure[AT_FEATURE_EDGE].featureFlag = AT_FEATURE_EXIST;
            memResult = memcpy_s(feATure[AT_FEATURE_EDGE].content, sizeof(feATure[AT_FEATURE_EDGE].content), strTmp,
                                 len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_EDGE].content), len);
        }
    }

    /* GPRS */
    feATure[AT_FEATURE_GPRS].featureFlag = AT_FEATURE_EXIST;
    memResult = memcpy_s(feATure[AT_FEATURE_GPRS].content, sizeof(feATure[AT_FEATURE_GPRS].content), strTmp, len);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_GPRS].content), len);

    /* GSM */
    feATure[AT_FEATURE_GSM].featureFlag = AT_FEATURE_EXIST;
    memResult = memcpy_s(feATure[AT_FEATURE_GSM].content, sizeof(feATure[AT_FEATURE_GSM].content), strTmp, len);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feATure[AT_FEATURE_GSM].content), len);
}

VOS_UINT32 AT_GetFeatureInfoByMtaQrySfeatureRsp(AT_FEATURE_SUPPORT_ST *feature, VOS_UINT32 length,
    MTA_AT_SfeatureQryCnf *atAgentSfeatureQryCnf)
{
    VOS_UINT32             reult;

    reult = AT_GetWFeatureInfo(feature, atAgentSfeatureQryCnf);
    if (reult != VOS_OK) {
        return VOS_ERR;
    }

#if (FEATURE_LTE == FEATURE_ON)
    reult = AT_GetLteFeatureInfo(feature);
    if (reult != VOS_OK) {
        return VOS_ERR;
    }
#endif
#if (FEATURE_LTEV == FEATURE_ON)
    AT_GetLtevFeatureInfo(feature, atAgentSfeatureQryCnf);
#endif
#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    reult = AT_GetTdsFeatureInfo(feature);
    if (reult != VOS_OK) {
        return VOS_ERR;
    }
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    AT_GetNrFeatureInfo(atAgentSfeatureQryCnf->bandFeature.nrBand, feature);
#endif

    AT_GetGFeatureInfo(feature, atAgentSfeatureQryCnf);
    return  VOS_OK;
}

VOS_UINT32 AT_RcvMtaQrySfeatureRsp(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    errno_t memResult;
    /* 初始化 */
    AT_MTA_Msg            *rcvMsg                = (AT_MTA_Msg *)msg;
    MTA_AT_SfeatureQryCnf *atAgentSfeatureQryCnf = VOS_NULL_PTR;
    AT_FEATURE_SUPPORT_ST *feature               = VOS_NULL_PTR;
    VOS_UINT8              featureNum = 0;
    VOS_UINT32             len = 0;
    VOS_UINT32             result;
    VOS_UINT8              indexTmp;
    VOS_INT                bufLen;
    const VOS_UINT8        featureName[][AT_FEATURE_NAME_LEN_MAX] = {
        "LTE",  "HSPA+", "HSDPA",  "HSUPA", "DIVERSITY", "UMTS", "EVDO", "EDGE",
        "GPRS", "GSM",   "CDMA1X", "WIMAX", "WIFI",      "GPS",  "TD",

        "NR", "LTE-V"
    };

    atAgentSfeatureQryCnf = (MTA_AT_SfeatureQryCnf *)rcvMsg->content;

    if (atAgentSfeatureQryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ConvertMtaResult(atAgentSfeatureQryCnf->result);
        return result;
    }

    /*lint -save -e516 */
    feature = (AT_FEATURE_SUPPORT_ST *)PS_MEM_ALLOC(WUEPS_PID_AT, AT_FEATURE_MAX * sizeof(AT_FEATURE_SUPPORT_ST));
    /*lint -restore */
    if (feature == VOS_NULL) {
        AT_WARN_LOG("AT_RcvMtaQrySfeatureRsp: GET MEM Fail!");
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(feature, AT_FEATURE_MAX * sizeof(AT_FEATURE_SUPPORT_ST), 0x00,
                       AT_FEATURE_MAX * sizeof(AT_FEATURE_SUPPORT_ST));

    if (AT_GetFeatureInfoByMtaQrySfeatureRsp(feature, AT_FEATURE_MAX * sizeof(AT_FEATURE_SUPPORT_ST),
        atAgentSfeatureQryCnf) != VOS_OK) {
        /* 复位AT状态 */
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, feature);
        /*lint -restore */
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (AT_IsMiscSupport(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT) {
        /* WIFI */
        feature[AT_FEATURE_WIFI].featureFlag = AT_FEATURE_EXIST;
        bufLen = snprintf_s((VOS_CHAR *)feature[AT_FEATURE_WIFI].content, sizeof(feature[AT_FEATURE_WIFI].content),
                            sizeof(feature[AT_FEATURE_WIFI].content) - 1, "B,G,N");
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, sizeof(feature[AT_FEATURE_WIFI].content),
                                         sizeof(feature[AT_FEATURE_WIFI].content) - 1);
    }

    /* 计算支持特性的个数 */
    for (indexTmp = 0; indexTmp < AT_FEATURE_MAX; indexTmp++) {
        memResult = memcpy_s(feature[indexTmp].featureName, sizeof(feature[indexTmp].featureName),
                             featureName[indexTmp], sizeof(featureName[indexTmp]));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(feature[indexTmp].featureName), sizeof(featureName[indexTmp]));
        if (feature[indexTmp].featureFlag == AT_FEATURE_EXIST) {
            featureNum++;
        }
    }

    /* 打印输出支持的特性数 */
    len = (TAF_UINT32)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + len, "%s:%d%s", g_parseContext[indexNum].cmdElement->cmdName, featureNum,
        g_atCrLf);

    /* 分行打印输出支持的特性 */
    for (indexTmp = 0; indexTmp < AT_FEATURE_MAX; indexTmp++) {
        if (feature[indexTmp].featureFlag == AT_FEATURE_EXIST) {
            len += (TAF_UINT32)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + len, "%s:%s,%s%s", g_parseContext[indexNum].cmdElement->cmdName,
                feature[indexTmp].featureName, feature[indexTmp].content, g_atCrLf);
        }
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)(len - (VOS_UINT32)VOS_StrLen((VOS_CHAR *)g_atCrLf));

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, feature);
    /*lint -restore */
    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_UINT32 AT_RcvMtaQryNmrCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg       *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_QryNmrCnf *qryNmrCnf = (MTA_AT_QryNmrCnf *)(mtaMsg->content);
    VOS_UINT32        nmrDataIndex;
    VOS_UINT16        length;

    if (qryNmrCnf->result == MTA_AT_RESULT_ERROR) {
        /* 查询失败直接上报error */
        g_atSendDataBuff.bufLen = 0;

        /* 输出结果 */
        return AT_ERROR;
    }

    length = 0;

    /* 按AT^CNMR查询命令返回将接入层返回的数据码流显示上报 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    if (qryNmrCnf->totalIndex == 0) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d", 0, 0);

        g_atSendDataBuff.bufLen = length;

        /* 输出结果 */
        return AT_OK;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,", qryNmrCnf->totalIndex, qryNmrCnf->currIndex);

    for (nmrDataIndex = 0; nmrDataIndex < qryNmrCnf->nmrLen; nmrDataIndex++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%02x", qryNmrCnf->nmrData[nmrDataIndex]);
    }

    if (qryNmrCnf->totalIndex != qryNmrCnf->currIndex) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        At_BufferorSendResultData(indexNum, g_atSndCodeAddress, length);
    }

    if (qryNmrCnf->totalIndex == qryNmrCnf->currIndex) {
        At_BufferorSendResultData(indexNum, g_atSndCodeAddress, length);

        g_atSendDataBuff.bufLen = 0;

        /* 输出结果 */
        return AT_OK;
    }

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)

#else

VOS_UINT32 AT_RcvBbicCalSetTrxTasCnf(struct MsgCB *msg)
{
    BBIC_CAL_SetTrxTasCnfStru *trxTasCnf   = VOS_NULL_PTR;
    AT_MT_Info                *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                 rslt = AT_ERROR;
    VOS_UINT8                  indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 获取本地保存的用户索引 */
    indexNum  = atMtInfoCtx->atInfo.indexNum;
    trxTasCnf = (BBIC_CAL_SetTrxTasCnfStru *)msg;

    AT_PR_LOGH("AT_RcvBbicCalSetTrxTasCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_RcvBbicCalSetTrxTasCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_TRX_TAS_SET) {
        AT_ERR_LOG("AT_RcvBbicCalSetTrxTasCnf: CmdCurrentOpt is not AT_CMD_TRX_TAS_SET!");
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (trxTasCnf->data.errorCode != MT_OK) {
        AT_ERR_LOG1("AT_RcvBbicCalSetTrxTasCnf: set trxtas error, ErrorCode is ", trxTasCnf->data.errorCode);
        rslt = AT_ERROR;
    } else {
        rslt                    = AT_OK;
        g_atSendDataBuff.bufLen = 0;
    }

    At_FormatResultData(indexNum, rslt);
    return VOS_OK;
}
#endif

