/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#include "at_query_para_cmd.h"
#include "securec.h"

#include "at_mdrv_interface.h"
#include "ppp_interface.h"
#include "taf_drv_agent.h"
#include "taf_tafm_remote.h"
#include "ttf_mem_at_interface.h"
#include "at_cmd_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "at_phy_interface.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_snd_msg.h"
#include "at_rnic_interface.h"
#include "at_input_proc.h"
#include "at_event_report.h"
#include "at_device_cmd.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "taf_oam_interface.h"
#include "taf_iface_api.h"
#include "at_lte_eventreport.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_common.h"
#include "at_lte_common.h"
#endif

#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"

#include "css_at_interface.h"

#include "at_msg_print.h"

#include "ps_common_def.h"
#include "ps_lib.h"
#include "product_config.h"
#include "taf_ccm_api.h"
#include "mn_comm_api.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#endif
#include "throt_at_interface.h"

#include "at_device_comm.h"

/*lint -esym(516,free,malloc)*/


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_QUERYPARACMD_C

/*
 * 3 函数、变量声明
 */
VOS_UINT32 AT_GetPhynumMac(VOS_UINT8 aucMac[], VOS_UINT32 macLen)
{
    VOS_UINT32 loop;
    VOS_UINT32 ret;
    VOS_UINT8  e5GwMacAddr[AT_MAC_ADDR_LEN + 1]; /* MAC地址 */
    VOS_UINT32 e5GwMacAddrOffset;
    VOS_UINT32 macOffset;
    errno_t    memResult;

    (VOS_VOID)memset_s(e5GwMacAddr, sizeof(e5GwMacAddr), 0x00, sizeof(e5GwMacAddr));

    /* 获取MAC地址字符串 */
    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WIFI_MAC_ADDR, e5GwMacAddr, AT_MAC_ADDR_LEN);
    if (ret != NV_OK) {
        AT_WARN_LOG("AT_GetPhynumMac: Fail to read en_Nv_Item_Imei_Svn.");
        return AT_ERROR;
    }

    /* MAC地址格式匹配: 7A:FE:E2:21:11:E4=>7AFEE22111E4 */
    macOffset         = 0;
    e5GwMacAddrOffset = 0;
    for (loop = 0; loop < (1 + AT_PHYNUM_MAC_COLON_NUM); loop++) {
        memResult = memcpy_s(&aucMac[macOffset], macLen - macOffset, &e5GwMacAddr[e5GwMacAddrOffset],
                             AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS);
        TAF_MEM_CHK_RTN_VAL(memResult, macLen - macOffset, AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS);
        macOffset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        e5GwMacAddrOffset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS + VOS_StrLen(":");
    }

    aucMac[macLen - 1] = '\0';

    return AT_OK;
}

VOS_VOID At_CovertMsInternalRxDivParaToUserSet(VOS_UINT16 curBandSwitch, VOS_UINT32 *userDivBandsLow,
                                               VOS_UINT32 *userDivBandsHigh)
{
    *userDivBandsLow  = 0;
    *userDivBandsHigh = 0;

    /*
     * 用户设置的接收分集格式如下:
     *   0x80000            GSM850
     *   0x300              GSM900
     *   0x80               DCS1800
     *   0x200000           PCS1900
     *   0x400000           WCDMA2100
     *   0x800000           WCDMA1900
     *   0x1000000          WCDMA1800
     *   0x2000000          WCDMA_AWS
     *   0x4000000          WCDMA850
     *   0x2000000000000    WCDMA900
     *   0x4000000000000    WCDMA1700
     * 而MS支持的接收分集格式如下:
     *   2100M/ bit1  1900M/bit2  1800M/bit3  1700M/bit4  1600M/bit5
     *   1500M/bit6   900M/bit7   850M/bit8   800M/bit9   450M/bit10
     * 需要把用户设置的接收分集转换成MS支持的格式
     */
    if ((curBandSwitch & AT_MS_SUPPORT_RX_DIV_900) != 0) {
        *userDivBandsHigh |= TAF_PH_BAND_WCDMA_VIII_900;
    }
    if ((curBandSwitch & AT_MS_SUPPORT_RX_DIV_IX_1700) != 0) {
        *userDivBandsHigh |= TAF_PH_BAND_WCDMA_IX_1700;
    }
    if ((curBandSwitch & AT_MS_SUPPORT_RX_DIV_2100) != 0) {
        *userDivBandsLow |= TAF_PH_BAND_WCDMA_I_IMT_2100;
    }
    if ((curBandSwitch & AT_MS_SUPPORT_RX_DIV_1900) != 0) {
        *userDivBandsLow |= TAF_PH_BAND_WCDMA_II_PCS_1900;
    }
    if ((curBandSwitch & AT_MS_SUPPORT_RX_DIV_1800) != 0) {
        *userDivBandsLow |= TAF_PH_BAND_WCDMA_III_1800;
    }
    if ((curBandSwitch & AT_MS_SUPPORT_RX_DIV_850) != 0) {
        *userDivBandsLow |= TAF_PH_BAND_WCDMA_V_850;
    }
}

#if (FEATURE_LTE == FEATURE_ON)
TAF_VOID AT_CalculateLTESignalValue(VOS_INT16 *psRssi, VOS_UINT8 *level, VOS_INT16 *psRsrp, VOS_INT16 *psRsrq

)
{
    VOS_UINT8 curAntennaLevel;
    VOS_INT16 rsrp;
    VOS_INT16 rsrq;

    /* 与 atSetAnlevelCnfSameProc处理相同 */
    /*
     * 上报数据转换:将 Rscp、Ecio显示为非负值，若Rscp、Ecio为-145，-32，或者rssi为99，
     * 则转换为0
     */
    if ((*psRsrp == TAF_PH_RSSIUNKNOW) || (*psRssi == TAF_PH_RSSIUNKNOW)) {
        /* 丢网返回0, 对应应用的圈外 */
        curAntennaLevel = AT_CMD_ANTENNA_LEVEL_0;
    } else {
        /* 取绝对值 */
        rsrp = (-(*psRsrp));

        /* 调用函数AT_CalculateAntennaLevel来根据D25算法计算出信号格数 */
        curAntennaLevel = AT_CalculateLTEAntennaLevel((VOS_INT16)(rsrp));
    }

    /* 信号磁滞处理 */
    *level = AT_GetSmoothLTEAntennaLevel(curAntennaLevel);

    if (*psRssi == AT_RSSI_UNKNOWN) {
        *psRssi = AT_RSSI_UNKNOWN;
    } else if (*psRssi >= AT_RSSI_HIGH) {
        *psRssi = AT_CSQ_RSSI_HIGH;
    } else if (*psRssi <= AT_RSSI_LOW) {
        *psRssi = AT_CSQ_RSSI_LOW;
    } else {
        *psRssi = (VOS_INT16)((*psRssi - AT_RSSI_LOW) / 2);
    }

    rsrp = (*psRsrp == TAF_PH_RSSIUNKNOW) ? TAF_PH_RSSIUNKNOW : (-(*psRsrp));
    rsrq = (*psRsrq == TAF_PH_RSSIUNKNOW) ? TAF_PH_RSSIUNKNOW : (-(*psRsrq));

    *psRsrp = rsrp;
    *psRsrq = rsrq;
}
#endif


VOS_UINT32 At_ReadDpaCatFromNV(VOS_UINT8 *dpaRate)
{
    AT_NvimUeCapa uECapa;
    AT_DpacatPara dhpaCategory[AT_DPACAT_CATEGORY_TYPE_BUTT] = {
        /* 支持速率等级3.6M  */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_6, PS_FALSE, 0, PS_FALSE },
        /* 支持速率等级7.2M  */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_8, PS_FALSE, 0, PS_FALSE },
        /* 支持速率等级1.8M  */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_11, PS_FALSE, 0, PS_FALSE },
        /*  支持速率等级14.4M */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_10, PS_FALSE, 0, PS_FALSE },
        /*  支持速率等级21M */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_10, PS_TRUE, AT_HSDSCH_PHY_CATEGORY_14, PS_FALSE }
    };
    VOS_UINT8 loop;

    (VOS_VOID)memset_s(&uECapa, sizeof(uECapa), 0x00, sizeof(uECapa));

    /* 输入参数空指针检查 */
    if (dpaRate == VOS_NULL_PTR) {
        AT_WARN_LOG("At_ReadDpaCatFromNV: null PTR.");
        return VOS_ERR;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, &uECapa, sizeof(AT_NvimUeCapa)) != NV_OK) {
        AT_WARN_LOG("At_ReadDpaCatFromNV():en_NV_Item_WAS_RadioAccess_Capa NV Read Fail!");
        return VOS_ERR;
    }

    for (loop = 0; loop < AT_DPACAT_CATEGORY_TYPE_BUTT; loop++) {
        if ((uECapa.hsdschSupport == dhpaCategory[loop].hsdschSupport) &&
            (uECapa.hsdschPhyCategory == dhpaCategory[loop].hsdschPhyCategory) &&
            (uECapa.macEhsSupport == dhpaCategory[loop].macEhsSupport) &&
            (uECapa.hsdschPhyCategoryExt == dhpaCategory[loop].hsdschPhyCategoryExt) &&
            (uECapa.multiCellSupport == dhpaCategory[loop].multiCellSupport)) {
            *dpaRate = loop;
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

VOS_UINT32 AT_ReadRrcVerFromNV(VOS_UINT8 *rrcVer)
{
    AT_NvimUeCapa uECapa;

    (VOS_VOID)memset_s(&uECapa, sizeof(uECapa), 0x00, sizeof(uECapa));

    /* 输入参数非空性检查 */
    if (rrcVer == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_ReadRrcVerFromNV: null PTR.");
        return VOS_ERR;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, &uECapa, sizeof(AT_NvimUeCapa)) != NV_OK) {
        AT_WARN_LOG("AT_ReadRrcVerFromNV: TAF_ACORE_NV_READ NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW fail!");
        return VOS_ERR;
    }

    /* NV未使能 */
    if (uECapa.hspaStatus == VOS_FALSE) {
        *rrcVer = AT_RRC_VERSION_DPA_AND_UPA;
        return VOS_OK;
    }

    /* NV使能 */
    /* HSPA+ */
    if ((uECapa.asRelIndicator >= AT_PTL_VER_ENUM_R7) && (uECapa.edchSupport == VOS_TRUE) &&
        (uECapa.hsdschSupport == VOS_TRUE)) {
        *rrcVer = AT_RRC_VERSION_HSPA_PLUNS;
    }
    /* DPA/UPA */
    else if ((uECapa.edchSupport == VOS_TRUE) && (uECapa.hsdschSupport == VOS_TRUE)) {
        *rrcVer = AT_RRC_VERSION_DPA_AND_UPA;
    }
    /* DPA */
    else if (uECapa.hsdschSupport == VOS_TRUE) {
        *rrcVer = AT_RRC_VERSION_DPA;
    }
    /* WCDMA */
    else {
        *rrcVer = AT_RRC_VERSION_WCDMA;
    }

    return VOS_OK;
}

VOS_UINT32 AT_JudgeU8ArrayZero(VOS_UINT8 aucValue[], VOS_UINT32 length)
{
    VOS_UINT32 loop;

    for (loop = 0; loop < length; loop++) {
        if (aucValue[loop] != 0) {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_GetWifiNvValue(VOS_UINT16 *csdValue)
{
    TAF_AT_MultiWifiSec  wifiKey;
    TAF_AT_MultiWifiSsid wifiSsid;
    VOS_UINT32           ret;
    VOS_UINT32           loop;

    /* 不支持WIFI情况下WIFI的NV项不关注，直接返回未修改 */
    if (AT_IsMiscSupport(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_UNSUPPORT) {
        *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_UNCHANGE;
        return VOS_OK;
    }

    (VOS_VOID)memset_s(&wifiKey, sizeof(wifiKey), 0x00, sizeof(wifiKey));
    (VOS_VOID)memset_s(&wifiSsid, sizeof(wifiSsid), 0x00, sizeof(wifiSsid));

    /*
     * 判断en_NV_Item_WIFI_KEY是否为默认值，关注下述字段是否全0:
     * aucWifiWpapsk aucWifiWepKey1 aucWifiWepKey2 aucWifiWepKey3 aucWifiWepKey4
     */
    if (TAF_ACORE_NV_READ_IN_CCORE(MODEM_ID_0, NV_ITEM_MULTI_WIFI_KEY, (VOS_UINT8 *)&wifiKey,
                                   sizeof(TAF_AT_MultiWifiSec)) != NV_OK) {
        AT_WARN_LOG("AT_GetWifiNvValue:READ NV ERROR");
        return VOS_ERR;
    }

    /*
     * 判断en_NV_Item_WIFI_STATUS_SSID是否为默认值，关注下述字段是否全0:
     * aucWifiSsid
     */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_MULTI_WIFI_STATUS_SSID, &wifiSsid, sizeof(TAF_AT_MultiWifiSsid)) !=
        VOS_OK) {
        AT_WARN_LOG("AT_GetWifiNvValue:READ NV ERROR");
        return VOS_ERR;
    }

    for (loop = 0; loop < AT_WIFI_MAX_SSID_NUM; loop++) {
        ret = AT_JudgeU8ArrayZero(wifiKey.wifiWpapsk[loop], sizeof(wifiKey.wifiWpapsk[loop]));
        if (ret != VOS_OK) {
            *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_CHANGED;
            return VOS_OK;
        }

        ret = AT_JudgeU8ArrayZero(wifiKey.wifiWepKey1[loop], sizeof(wifiKey.wifiWepKey1[loop]));
        if (ret != VOS_OK) {
            *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_CHANGED;
            return VOS_OK;
        }

        ret = AT_JudgeU8ArrayZero(wifiKey.wifiWepKey2[loop], sizeof(wifiKey.wifiWepKey2[loop]));
        if (ret != VOS_OK) {
            *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_CHANGED;
            return VOS_OK;
        }

        ret = AT_JudgeU8ArrayZero(wifiKey.wifiWepKey3[loop], sizeof(wifiKey.wifiWepKey3[loop]));
        if (ret != VOS_OK) {
            *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_CHANGED;
            return VOS_OK;
        }

        ret = AT_JudgeU8ArrayZero(wifiKey.wifiWepKey4[loop], sizeof(wifiKey.wifiWepKey4[loop]));
        if (ret != VOS_OK) {
            *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_CHANGED;
            return VOS_OK;
        }

        ret = AT_JudgeU8ArrayZero(wifiSsid.wifiSsid[loop], sizeof(wifiSsid.wifiSsid[loop]));
        if (ret != VOS_OK) {
            *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_CHANGED;
            return VOS_OK;
        }
    }

    *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_UNCHANGE;
    return VOS_OK;
}

VOS_UINT32 AT_GetCsdValue(VOS_BOOL bSimlockEnableFlg, VOS_UINT16 *csdValue)
{
    VOS_UINT32 ret;

    /* 判断SIM LOCK Status定制项的内容是否为默认值DISABLE */
    /* 获取SIM Lock Status定制项的值并检查是否为默认值 */
    if (bSimlockEnableFlg != VOS_FALSE) {
        *csdValue = AT_CUSTOMIZE_ITEM_DEFAULT_VAL_CHANGED;
        return VOS_OK;
    }

    /* 判断WIFI相关的NV项是否为默认值 */
    ret = AT_GetWifiNvValue(csdValue);

    return ret;
}

VOS_UINT32 AT_OutputCsdfltDefault(VOS_UINT8 indexNum, VOS_BOOL bSimlockEnableFlg)
{
    VOS_UINT16 length;
    VOS_UINT32 csdfltFlg = VOS_TRUE;
    VOS_UINT16 csdfltValue;
    VOS_UINT32 rslt;

    rslt = AT_GetCsdValue(bSimlockEnableFlg, &csdfltValue);

    if (rslt != VOS_OK) {
        return AT_ERROR;
    }

    /*
     * 显示定制值信息，0:表示定制项中的值为单板自定义的默认值;
     * 1:表示定制项中的值为非单板自定义的默认值
     */
    if (csdfltValue == AT_CUSTOMIZE_ITEM_DEFAULT_VAL_UNCHANGE) {
        csdfltFlg = VOS_FALSE;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, csdfltFlg);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

