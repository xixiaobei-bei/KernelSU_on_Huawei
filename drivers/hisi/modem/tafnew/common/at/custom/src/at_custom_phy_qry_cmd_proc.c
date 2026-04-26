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
#include "at_custom_phy_qry_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"

#include "at_custom_comm.h"
#include "nv_stru_msp_interface.h"
#include "at_input_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_PHY_QRY_CMD_PROC_C

VOS_UINT32 AT_QryAfcClkInfo(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_QRY_AFC_CLK_FREQ_XOCOEF_REQ,
                                 VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);
    if (ret != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_AFCCLKINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryBodySarOnPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, g_atBodySarState);
    return AT_OK;
}

VOS_UINT32 AT_FillBodySarWcdmaQryPara(MTA_BodySarPara *bodySarPara, AT_BodysarwcdmaSetPara *bodySarWcdmaPara)
{
    VOS_UINT32 loop1;
    VOS_UINT8         loop2;
    AT_NvWgRfMainBand wGBand;
    VOS_UINT32        tmpBand;

    (VOS_VOID)memset_s(&wGBand, sizeof(wGBand), 0x00, sizeof(wGBand));
    tmpBand = 0;

    /* 获取WCDMA Band能力值 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_WG_RF_MAIN_BAND, &wGBand, sizeof(wGBand)) != NV_OK) {
        AT_ERR_LOG("AT_FillBodySarWcdmaQryPara: Read NV fail!");
        return VOS_ERR;
    }

    for (loop1 = 0; loop1 < (VOS_UINT32)TAF_MIN(bodySarPara->wBandNum, MTA_BODY_SAR_WBAND_MAX_NUM); loop1++) {
        for (loop2 = 0; loop2 < TAF_MIN(bodySarWcdmaPara->paraNum, AT_BODYSARWCDMA_MAX_PARA_GROUP_NUM); loop2++) {
            if (bodySarPara->wBandPara[loop1].power == bodySarWcdmaPara->asPower[loop2]) {
                break;
            }
        }

        if ((loop2 == bodySarWcdmaPara->paraNum) && (loop2 < AT_BODYSARWCDMA_MAX_PARA_GROUP_NUM)) {
            bodySarWcdmaPara->paraNum++;
            bodySarWcdmaPara->asPower[loop2] = bodySarPara->wBandPara[loop1].power;
        }

        if (loop2 < AT_BODYSARWCDMA_MAX_PARA_GROUP_NUM) {
            bodySarWcdmaPara->band[loop2] |= 0x00000001U << (bodySarPara->wBandPara[loop1].band - 1);
            tmpBand |= bodySarWcdmaPara->band[loop2];
        }
    }

    /* 剩余未设置的频段返回默认值 */
    tmpBand = wGBand.unWcdmaBand.band & (~tmpBand);
    if ((tmpBand != 0) && (bodySarWcdmaPara->paraNum < AT_BODYSARWCDMA_MAX_PARA_GROUP_NUM)) {
        bodySarWcdmaPara->band[bodySarWcdmaPara->paraNum]    = tmpBand;
        bodySarWcdmaPara->asPower[bodySarWcdmaPara->paraNum] = AT_WCDMA_BAND_DEFAULT_POWER;
        bodySarWcdmaPara->paraNum++;
    }

    return VOS_OK;
}

VOS_UINT32 AT_QryBodySarWcdmaPara(VOS_UINT8 indexNum)
{
    MTA_BodySarPara        bodySarPara;
    AT_BodysarwcdmaSetPara bodySarWcdmaPara;
    VOS_UINT8              loop;
    VOS_UINT16             length;

    (VOS_VOID)memset_s(&bodySarPara, sizeof(bodySarPara), 0x00, sizeof(bodySarPara));
    (VOS_VOID)memset_s(&bodySarWcdmaPara, sizeof(bodySarWcdmaPara), 0x00, sizeof(bodySarWcdmaPara));
    length = 0;

    /* 从NV项中读取Body SAR功率门限值 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_BODY_SAR_PARA, &bodySarPara, sizeof(bodySarPara)) != NV_OK) {
        AT_ERR_LOG("AT_QryBodySarWcdmaPara: Read NV fail!");
        return AT_ERROR;
    }

    /* 从Body SAR参数结构体填充^BODYSARWCDMA命令参数结构体 */
    if (AT_FillBodySarWcdmaQryPara(&bodySarPara, &bodySarWcdmaPara) != VOS_OK) {
        AT_ERR_LOG("AT_QryBodySarWcdmaPara: AT_FillBodySarWcdmaQryPara fail!");
        return AT_ERROR;
    }

    /* 打印命令名 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 打印WCDMA频段Body SAR参数 */
    if (bodySarWcdmaPara.paraNum == 1) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(g_atSndCodeAddress + length), "(%d,%08X)", bodySarWcdmaPara.asPower[0], AT_RF_BAND_ANY);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }

    for (loop = 0; loop < bodySarWcdmaPara.paraNum; loop++) {
        if (loop != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)(g_atSndCodeAddress + length), ",");
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(g_atSndCodeAddress + length), "(%d,%08X)", bodySarWcdmaPara.asPower[loop],
            bodySarWcdmaPara.band[loop]);
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_FillBodySarGsmDefaultPower(MTA_BodySarPara *bodySarPara)
{
    VOS_UINT32 gBandCapa = 0;
    VOS_UINT32 tmpBand = 0;

    /* 获取GSM Band能力值 */
    if (AT_GetGsmBandCapa(&gBandCapa) != VOS_OK) {
        AT_ERR_LOG("AT_FillBodySarGsmDefaultPower: AT_GetGsmBandCapa fail!");
        return VOS_ERR;
    }

    tmpBand = gBandCapa & (~bodySarPara->gBandMask);

    if ((AT_BODY_SAR_GBAND_GPRS_850_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_GPRS_850_MASK;
        bodySarPara->gBandPara[AT_GSM_850].gprsPower = AT_GSM_GPRS_850_DEFAULT_POWER;
    }

    if ((AT_BODY_SAR_GBAND_EDGE_850_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_EDGE_850_MASK;
        bodySarPara->gBandPara[AT_GSM_850].edgePower = AT_GSM_EDGE_850_DEFAULT_POWER;
    }

    if ((AT_BODY_SAR_GBAND_GPRS_900_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_GPRS_900_MASK;
        bodySarPara->gBandPara[AT_GSM_900].gprsPower = AT_GSM_GPRS_900_DEFAULT_POWER;
    }

    if ((AT_BODY_SAR_GBAND_EDGE_900_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_EDGE_900_MASK;
        bodySarPara->gBandPara[AT_GSM_900].edgePower = AT_GSM_EDGE_900_DEFAULT_POWER;
    }

    if ((AT_BODY_SAR_GBAND_GPRS_1800_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_GPRS_1800_MASK;
        bodySarPara->gBandPara[AT_GSM_1800].gprsPower = AT_GSM_GPRS_1800_DEFAULT_POWER;
    }

    if ((AT_BODY_SAR_GBAND_EDGE_1800_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_EDGE_1800_MASK;
        bodySarPara->gBandPara[AT_GSM_1800].edgePower = AT_GSM_EDGE_1800_DEFAULT_POWER;
    }

    if ((AT_BODY_SAR_GBAND_GPRS_1900_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_GPRS_1900_MASK;
        bodySarPara->gBandPara[AT_GSM_1900].gprsPower = AT_GSM_GPRS_1900_DEFAULT_POWER;
    }

    if ((AT_BODY_SAR_GBAND_EDGE_1900_MASK & tmpBand) != 0) {
        bodySarPara->gBandMask |= AT_BODY_SAR_GBAND_EDGE_1900_MASK;
        bodySarPara->gBandPara[AT_GSM_1900].edgePower = AT_GSM_EDGE_1900_DEFAULT_POWER;
    }

    return VOS_OK;
}

VOS_VOID AT_FillBodySarGsmByModulationMode(MTA_BodySarPara *bodySarPara, VOS_UINT8 modulationOffset,
                                           AT_BodysargsmSetPara *bodySarGsmPara)
{
    VOS_UINT8 loop1;
    VOS_UINT8 loop2;

    for (loop1 = AT_GSM_850; loop1 < AT_GSM_BAND_BUTT; loop1++) {
        for (loop2 = 0; loop2 < AT_MIN(bodySarGsmPara->paraNum, AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM); loop2++) {
            if (modulationOffset == AT_GSM_GPRS_BAND_OFFSET) {
                if (bodySarPara->gBandPara[loop1].gprsPower == bodySarGsmPara->asPower[loop2]) {
                    break;
                }
            } else {
                if (bodySarPara->gBandPara[loop1].edgePower == bodySarGsmPara->asPower[loop2]) {
                    break;
                }
            }
        }

        if ((loop2 == bodySarGsmPara->paraNum) && (loop2 < AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM)) {
            bodySarGsmPara->paraNum++;
            if (modulationOffset == AT_GSM_GPRS_BAND_OFFSET) {
                bodySarGsmPara->asPower[loop2] = bodySarPara->gBandPara[loop1].gprsPower;
            } else {
                bodySarGsmPara->asPower[loop2] = bodySarPara->gBandPara[loop1].edgePower;
            }
        }
        if (loop2 < AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM) {
            bodySarGsmPara->band[loop2] |= 0x00000001U << (loop1 + modulationOffset);
        }
    }
}

VOS_UINT32 AT_QryBodySarGsmPara(VOS_UINT8 indexNum)
{
    MTA_BodySarPara      bodySarPara;
    AT_BodysargsmSetPara bodySarGsmPara;
    VOS_UINT8            loop;
    VOS_UINT16           length;

    (VOS_VOID)memset_s(&bodySarPara, sizeof(bodySarPara), 0x00, sizeof(bodySarPara));
    (VOS_VOID)memset_s(&bodySarGsmPara, sizeof(bodySarGsmPara), 0x00, sizeof(bodySarGsmPara));
    length = 0;

    /* 从NV项中读取Body SAR功率门限值 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_BODY_SAR_PARA, &bodySarPara, sizeof(bodySarPara)) != NV_OK) {
        AT_ERR_LOG("AT_QryBodySarGsmPara: Read NV fail!");
        return AT_ERROR;
    }

    /* 填充GSM频段Body SAR参数默认值 */
    if (AT_FillBodySarGsmDefaultPower(&bodySarPara) != VOS_OK) {
        AT_ERR_LOG("AT_QryBodySarGsmPara: AT_FillBodySarGsmDefaultPower fail!");
        return AT_ERROR;
    }

    /* 从Body SAR参数结构体填充^BODYSARGSM命令参数结构体 */
    /* GPRS调制方式 */
    AT_FillBodySarGsmByModulationMode(&bodySarPara, AT_GSM_GPRS_BAND_OFFSET, &bodySarGsmPara);

    /* EDGE调制方式 */
    AT_FillBodySarGsmByModulationMode(&bodySarPara, AT_GSM_EDGE_BAND_OFFSET, &bodySarGsmPara);

    /* 打印命令名 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* 打印GSM频段Body SAR参数 */
    if (bodySarGsmPara.paraNum == 1) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(g_atSndCodeAddress + length), "(%d,%08X)", bodySarGsmPara.asPower[0], AT_RF_BAND_ANY);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }

    for (loop = 0; loop < AT_MIN(bodySarGsmPara.paraNum, AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM); loop++) {
        if (loop != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)(g_atSndCodeAddress + length), ",");
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(g_atSndCodeAddress + length), "(%d,%08X)", bodySarGsmPara.asPower[loop],
            bodySarGsmPara.band[loop]);
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_QryRefclkfreqPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 发送跨核消息到C核, 查询GPS参考时钟状态 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_REFCLKFREQ_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryRefclkfreqPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_REFCLKFREQ_READ;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_QryHandleDect(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    /* 读取NV项中当前产品形态 */
    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* 非ANDROID系统不支持 */
    if (*systemAppConfig != SYSTEM_APP_ANDROID) {
        return AT_CMD_NOT_SUPPORT;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 ID_AT_MTA_HANDLEDECT_QRY_REQ 给MTA处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, At_GetOpId(), ID_AT_MTA_HANDLEDECT_QRY_REQ,
                                 VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HANDLEDECT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryRatRfSwitch(VOS_UINT8 indexNum)
{
    NAS_NvTriModeEnable       triModeEnableStru;
    NAS_NvTriModeFemProfileId triModeFemProfileIdStru;

    (VOS_VOID)memset_s(&triModeEnableStru, sizeof(triModeEnableStru), 0x00, sizeof(triModeEnableStru));
    (VOS_VOID)memset_s(&triModeFemProfileIdStru, sizeof(triModeFemProfileIdStru), 0x00,
                       sizeof(triModeFemProfileIdStru));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 读取NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_TRI_MODE_ENABLE, &triModeEnableStru, sizeof(triModeEnableStru)) !=
        NV_OK) {
        AT_WARN_LOG("At_QryRatRfSwitch:read en_NV_Item_TRI_MODE_ENABLE failed");
        return AT_ERROR;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_TRI_MODE_FEM_PROFILE_ID, &triModeFemProfileIdStru,
                          sizeof(triModeFemProfileIdStru)) != NV_OK) {
        AT_WARN_LOG("At_QryRatRfSwitch:read en_NV_Item_TRI_MODE_FEM_PROFILE_ID failed");
        return AT_ERROR;
    }

    /* 输出 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%u", g_parseContext[indexNum].cmdElement->cmdName,
        triModeEnableStru.enable, triModeFemProfileIdStru.profileId);
    return AT_OK;
}

VOS_UINT32 AT_QryTxPowerPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_TXPOWER_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryTxPowerPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型，返回命令处理挂起状态 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TXPOWER_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

VOS_UINT32 AT_QryNtxPowerPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NTXPOWER_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryNtxPowerPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型，返回命令处理挂起状态 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NTXPOWER_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestNrPwrCtrlPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: (0-2),(0-1)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}
#endif

VOS_UINT32 AT_TestBodySarWcdmaPara(VOS_UINT8 indexNum)
{
    AT_NvWgRfMainBand wGBand;

    (VOS_VOID)memset_s(&wGBand, sizeof(wGBand), 0x00, sizeof(wGBand));

    /* 获取WCDMA Band能力值 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_WG_RF_MAIN_BAND, &wGBand, sizeof(wGBand)) != NV_OK) {
        AT_ERR_LOG("AT_TestBodySarWcdmaPara: Write NV fail!");
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (17,24),%08X", g_parseContext[indexNum].cmdElement->cmdName,
        wGBand.unWcdmaBand.band);

    return AT_OK;
}

VOS_UINT32 AT_TestBodySarGsmPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 gsmBandCapa = 0;

    if (AT_GetGsmBandCapa(&gsmBandCapa) != VOS_OK) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (15,33),%08X", g_parseContext[indexNum].cmdElement->cmdName, gsmBandCapa);

    return AT_OK;
}

VOS_UINT32 AT_TestRefclkfreqPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 打印输出^REFCLKFREQ支持的参数设置范围 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_TestLtePwrDissPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: (0-2),(-8-63)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}

VOS_UINT32 AT_QryLwclashPara(VOS_UINT8 indexNum)
{
    VOS_UINT32                ret;
    L4A_READ_LWCLASH_REQ_STRU lwclash = {0};

    lwclash.ctrl.clientId = g_atClientTab[indexNum].clientId;

    lwclash.ctrl.opId = 0;
    lwclash.ctrl.pid  = WUEPS_PID_AT;

    ret = atSendL4aDataMsg(g_atClientTab[indexNum].clientId, I0_MSP_L4_L4A_PID, ID_MSG_L4A_LWCLASHQRY_REQ,
                           (VOS_UINT8 *)(&lwclash), sizeof(L4A_READ_LWCLASH_REQ_STRU));

    if (ret == ERR_MSP_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LWCLASH_READ;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

