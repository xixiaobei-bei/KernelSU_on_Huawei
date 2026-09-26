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
#include "at_init.h"


#include "securec.h"
#include "taf_type_def.h"

/* 保护此头文件是因为包含了PS域相关的定义，但是本项目未修改PS域相关的，后续需要调整 */
#include "at_data_proc.h"
#include "acpu_reset.h"
#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_lte_common.h"
#endif
#include "nv_stru_was.h"
#include "nv_stru_gas.h"
#include "nv_stru_msp.h"
#include "product_config.h"
#include "at_mdrv_interface.h"
#include "msp_diag_comm.h"
#include "gu_nas_log_filter.h"
#include "ps_log_filter_interface.h"
#include "taf_acore_log_privacy.h"

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
#include "ads_dev_i.h"
#else
#include "ads_device_interface.h"
#endif
#endif

#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_acore_only_cmd.h"
#if (FEATURE_AT_PROXY == FEATURE_ON)
#include "at_atp_proc.h"
#endif

#include "at_mbb_cmd.h"

#include "at_device_as_cmd_tbl.h"
#include "at_device_drv_cmd_tbl.h"
#include "at_device_mm_cmd_tbl.h"
#include "at_device_pam_cmd_tbl.h"
#include "at_device_phy_cmd_tbl.h"
#include "at_device_taf_cmd_tbl.h"

#include "at_custom_as_cmd_tbl.h"
#include "at_custom_drv_cmd_tbl.h"
#include "at_custom_hifi_cmd_tbl.h"
#include "at_custom_ims_cmd_tbl.h"
#include "at_custom_l2_cmd_tbl.h"
#include "at_custom_lnas_cmd_tbl.h"
#include "at_custom_mm_cmd_tbl.h"
#include "at_custom_pam_cmd_tbl.h"
#include "at_custom_phy_cmd_tbl.h"
#include "at_custom_taf_cmd_tbl.h"
#include "at_custom_comm_cmd_tbl.h"
#include "at_custom_lrrc_cmd_tbl.h"
#include "at_custom_nrmm_cmd_tbl.h"
#include "at_custom_rcm_cmd_tbl.h"
#include "at_custom_rrm_cmd_tbl.h"

#include "at_cdma_cmd_tbl.h"
#include "at_general_drv_cmd_tbl.h"
#include "at_general_mm_cmd_tbl.h"
#include "at_general_taf_cmd_tbl.h"
#include "at_general_pam_cmd_tbl.h"

#include "at_stub_mm_cmd_tbl.h"
#include "at_stub_taf_cmd_tbl.h"

#include "at_data_taf_cmd_tbl.h"
#include "at_data_mm_cmd_tbl.h"
#include "at_data_basic_cmd_tbl.h"
#include "at_data_drv_cmd_tbl.h"

#include "at_voice_taf_cmd_tbl.h"
#include "at_voice_hifi_cmd_tbl.h"
#include "at_voice_ims_cmd_tbl.h"
#include "at_voice_mm_cmd_tbl.h"

#include "at_test_as_cmd_tbl.h"
#include "at_test_phy_cmd_tbl.h"
#include "at_test_taf_cmd_tbl.h"
#include "at_test_ttf_cmd_tbl.h"

#include "at_sim_pam_cmd_tbl.h"
#include "at_sms_taf_cmd_tbl.h"
#include "at_ss_taf_cmd_tbl.h"
#include "at_sat_pam_cmd_tbl.h"

#include "at_phone_taf_cmd_tbl.h"
#include "at_phone_phy_cmd_tbl.h"
#include "at_phone_mm_cmd_tbl.h"
#include "at_phone_ims_cmd_tbl.h"
#include "at_phone_as_cmd_tbl.h"

#include "at_ltev_as_cmd_tbl.h"
#include "at_ltev_mm_cmd_tbl.h"
#include "at_ltev_taf_cmd_tbl.h"
#include "at_ltev_ttf_cmd_tbl.h"

#include "at_safety_mm_cmd_tbl.h"
#include "at_safety_pam_cmd_tbl.h"

#include "at_device_comm.h"
#include "at_proc_api.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_INIT_C
#define AT_UE_CATEGORY_MIN_VALID_VALUE 1
#define AT_UE_CATEGORY_MAX_VALID_VALUE 12
#define AT_UE_CATEGORYEXT_MIN_VALID_VALUE 13
#define AT_UE_CATEGORYEXT_MAX_VALID_VALUE 20
#define AT_UE_CATEGORYEXT2_MIN_VALID_VALUE 21
#define AT_UE_CATEGORYEXT2_MAX_VALID_VALUE 24
#define AT_UE_CATEGORYEXT3_MIN_VALID_VALUE 25
#define AT_UE_CATEGORYEXT3_MAX_VALID_VALUE 28

/* M2M形态没有GPS，CMUX里面暂时不设GPS口，此全局变量保持默认值0 */
#if (FEATURE_IOT_CMUX == FEATURE_ON)
VOS_UINT32 g_gpsPortEnableFlag = 0;

VOS_UINT32 AT_GetMuxGpsPortEnableFlag(VOS_VOID)
{
    return g_gpsPortEnableFlag;
}
#endif

VOS_VOID AT_ReadClientConfigNV(VOS_VOID)
{
    TAF_AT_NvimAtClientCfg        atClientCfg;
    AT_ClinetConfigDesc          *cfgDesc   = VOS_NULL_PTR;
    AT_ClientConfiguration       *clientCfg = VOS_NULL_PTR;
    const AT_ClientCfgMapTab     *cfgMapTbl = VOS_NULL_PTR;
    VOS_UINT8                     i;

    (VOS_VOID)memset_s(&atClientCfg, sizeof(atClientCfg), 0x00, sizeof(TAF_AT_NvimAtClientCfg));

    /*
     * NV项en_NV_Item_AT_CLIENT_CONFIG的
     * aucAtClientConfig[Index]
     * Index = 0 -- AT_CLIENT_TAB_PCUI_INDEX
     * Index = 1 -- AT_CLIENT_TAB_CTRL_INDEX
     * ......
     * 按index顺序递增
     * aucAtClientConfig[Index]
     * BIT0-BIT1对应一个client归属于哪个ModemId:00:表示modem0 01:表示modem1
     * BIT2对应一个client是否允许广播:0:表示不允许 1:表示允许
     */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AT_CLIENT_CFG, &atClientCfg, sizeof(TAF_AT_NvimAtClientCfg)) != VOS_OK) {
        return;
    }

    cfgDesc = (AT_ClinetConfigDesc *)&(atClientCfg.atClientConfig[0]);

    for (i = 0; i < AT_GET_CLIENT_CFG_TAB_LEN(); i++) {
        cfgMapTbl = AT_GetClientCfgMapTbl(i);
        clientCfg = AT_GetClientConfig(cfgMapTbl->clientId);

#if (MULTI_MODEM_NUMBER >= 2)
        clientCfg->modemId = cfgDesc[cfgMapTbl->nvIndex].modemId;
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
        if ((cfgMapTbl->clientId == AT_CLIENT_ID_CMUXGPS) && (AT_GetMuxGpsPortEnableFlag() == VOS_TRUE)) {
            clientCfg->reportFlg = VOS_FALSE;
            continue;
        }
#endif
        clientCfg->reportFlg = cfgDesc[cfgMapTbl->nvIndex].reportFlg;
    }
}

MN_MSG_MemStoreUint8 AT_GetCpmsMtMem(ModemIdUint16 modemId)
{
    VOS_UINT32              ret;
    MN_MSG_MemStoreUint8    smMemStore;
    TAF_NVIM_SmsServicePara smsServicePara;
    /*
     * 短信接收存储介质保存在NV配置文件的使能标志
     * VOS_TRUE        短信接收存储介质保存在NV配置文件
     * VOS_FALSE       短信接收存储介质不保存在NV配置文件，每次上电后恢复为SM存储
     */
    VOS_BOOL        bSmMemEnable;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&smsServicePara, sizeof(smsServicePara), 0x00, sizeof(TAF_NVIM_SmsServicePara));

    smMemStore = MN_MSG_MEM_STORE_SIM;

    ret = TAF_ACORE_NV_READ(modemId, NV_ITEM_SMS_SERVICE_PARA, &smsServicePara, MN_MSG_SRV_PARAM_LEN);
    if (ret != NV_OK) {
        AT_ERR_LOG("AT_GetCpmsMtMem: Read Service Parm From Nvim Failed");
        return smMemStore;
    }

    bSmMemEnable = (smsServicePara.smsServicePara)[AT_MSG_SRV_SM_MEM_ENABLE_OFFSET];

    smsCtx = AT_GetModemSmsCtxAddrFromModemId(modemId);

    if (smsCtx->msgMeStorageStatus == MN_MSG_ME_STORAGE_ENABLE) {
        if (bSmMemEnable == VOS_TRUE) {
            smMemStore = (smsServicePara.smsServicePara)[AT_MSG_SRV_RCV_SM_MEM_STORE_OFFSET];
        }
    }

    return smMemStore;
}

VOS_VOID AT_ReadSmsMeStorageInfoNV(VOS_VOID)
{
    VOS_UINT32           ret;
    MN_MSG_MeStorageParm meStorageParm;
    ModemIdUint16        modemId;
    AT_ModemSmsCtx      *smsCtx = VOS_NULL_PTR;

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        smsCtx = AT_GetModemSmsCtxAddrFromModemId(modemId);

        meStorageParm.meStorageStatus = MN_MSG_ME_STORAGE_DISABLE;
        meStorageParm.meStorageNum    = 0;

        /* 获取ME短信存储介质的支持状态 */
        ret = TAF_ACORE_NV_READ(modemId, NV_ITEM_SMS_ME_STORAGE_INFO, &meStorageParm, sizeof(meStorageParm));
        if ((ret == NV_OK) && (meStorageParm.meStorageStatus == MN_MSG_ME_STORAGE_ENABLE)) {
            smsCtx->msgMeStorageStatus = meStorageParm.meStorageStatus;
        }

        /* 获取短信及状态报告接收存储介质 */
        smsCtx->cpmsInfo.rcvPath.smMemStore = AT_GetCpmsMtMem(modemId);
        smsCtx->cpmsInfo.rcvPath.staRptMemStore = smsCtx->cpmsInfo.rcvPath.smMemStore;
    }
}

VOS_VOID AT_ReadSmsClass0TailorNV(VOS_VOID)
{
    VOS_UINT32              ret;
    MN_MSG_NvimClass0Tailor class0Tailor;

    class0Tailor.actFlg       = MN_MSG_NVIM_ITEM_INACTIVE;
    class0Tailor.class0Tailor = MN_MSG_CLASS0_DEF;

    /* 获取CLASS0类短信接收上报方式 */
    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SMS_CLASS0_TAILOR, &class0Tailor, sizeof(MN_MSG_NvimClass0Tailor));
    if ((ret == NV_OK) && (class0Tailor.actFlg == MN_MSG_NVIM_ITEM_ACTIVE)) {
        g_class0Tailor = class0Tailor.class0Tailor;
    } else {
        g_class0Tailor = MN_MSG_CLASS0_DEF;
    }
}

VOS_VOID AT_ReadSmsClosePathNV(VOS_VOID)
{
    TAF_NVIM_SmsClosePathCfg closePath;
    AT_ModemSmsCtx          *smsCtx = VOS_NULL_PTR;
    ModemIdUint16            modemId;
    VOS_UINT32               ret;

    (VOS_VOID)memset_s(&closePath, sizeof(closePath), 0x00, sizeof(closePath));

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        smsCtx = AT_GetModemSmsCtxAddrFromModemId(modemId);

        /* 获取ME短信存储介质的支持状态 */
        ret = TAF_ACORE_NV_READ(modemId, NV_ITEM_SMS_CLOSE_PATH, &closePath, sizeof(closePath));
        if ((ret == NV_OK) && (closePath.nvimValid == VOS_TRUE)) {
            smsCtx->localStoreFlg = closePath.smsClosePathFlg;
        }
    }
}

VOS_VOID AT_ReadMtCustomizeInfo(VOS_VOID)
{
    VOS_UINT32             ret;
    MN_MSG_MtCustomizeInfo mtCustomize;
    ModemIdUint16          modemId;
    AT_ModemSmsCtx        *smsCtx = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&mtCustomize, sizeof(mtCustomize), 0x00, sizeof(MN_MSG_MtCustomizeInfo));

    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SMS_MT_CUSTOMIZE_INFO, &mtCustomize, sizeof(mtCustomize));
    if (ret != NV_OK) {
        AT_INFO_LOG("AT_ReadMtCustomizeInfo: Fail to read ");
        return;
    }

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        smsCtx = AT_GetModemSmsCtxAddrFromModemId(modemId);

        if (mtCustomize.actFlag == MN_MSG_NVIM_ITEM_ACTIVE) {
            smsCtx->smMeFullCustomize.actFlg      = VOS_TRUE;
            smsCtx->smMeFullCustomize.mtCustomize = mtCustomize.enMtCustomize;
        }
    }
}


VOS_VOID AT_ReadPortBuffCfgNV(VOS_VOID)
{
    AT_CommCtx        *commCtx = VOS_NULL_PTR;
    VOS_UINT32         ret;
    TAF_NV_PortBuffCfg smsBuffCfg;

    commCtx = AT_GetCommCtxAddr();

    (VOS_VOID)memset_s(&smsBuffCfg, sizeof(smsBuffCfg), 0x00, sizeof(smsBuffCfg));

    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PORT_BUFF_CFG, &smsBuffCfg, sizeof(smsBuffCfg));

    if (ret == NV_OK) {
        if (smsBuffCfg.enableFlg < AT_PORT_BUFF_BUTT) {
            commCtx->portBuffCfg.smsBuffCfg = smsBuffCfg.enableFlg;
        } else {
            commCtx->portBuffCfg.smsBuffCfg = AT_PORT_BUFF_DISABLE;
        }
    } else {
        commCtx->portBuffCfg.smsBuffCfg = AT_PORT_BUFF_DISABLE;
    }
}

VOS_VOID AT_ReadSmsNV(VOS_VOID)
{
    AT_ReadSmsMeStorageInfoNV();

    AT_ReadSmsClass0TailorNV();

    AT_ReadSmsClosePathNV();

    AT_ReadMtCustomizeInfo();
}

VOS_VOID AT_ReadNvloadCarrierNV(VOS_VOID)
{
    AT_ModemNvloadCtx                *nvloadCarrierCtx = VOS_NULL_PTR;
    TAF_NVIM_UsimDependentResultInfo  activeNvloadInfo;
    errno_t                           memResult;
    ModemIdUint16                     modemId;

    /* 初始化 */
    (VOS_VOID)memset_s(&activeNvloadInfo, sizeof(activeNvloadInfo), 0, sizeof(activeNvloadInfo));

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        /* 读取NV项 */
        if (TAF_ACORE_NV_READ(modemId, NV_ITEM_USIM_DEPENDENT_RESULT_INFO, &activeNvloadInfo,
                sizeof(TAF_NVIM_UsimDependentResultInfo)) != NV_OK) {
            AT_ERR_LOG("AT_ReadNvloadCarrierNV: NV_ITEM_USIM_DEPENDENT_RESULT_INFO NV Read Error.");
            continue;
        }

        nvloadCarrierCtx = AT_GetModemNvloadCtxAddrFromModemId(modemId);

        memResult = memcpy_s(nvloadCarrierCtx->carrierName, sizeof(nvloadCarrierCtx->carrierName),
            activeNvloadInfo.operName, sizeof(activeNvloadInfo.operName));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(nvloadCarrierCtx->carrierName), sizeof(activeNvloadInfo.operName));
        nvloadCarrierCtx->specialEffectiveFlg = activeNvloadInfo.effectiveWay;
    }
}

VOS_VOID AT_ReadRoamCapaNV(VOS_VOID)
{
    AT_ModemNetCtx      *netCtx = VOS_NULL_PTR;
    NAS_NVIM_RoamCfgInfo roamCfgInfo;
    ModemIdUint16        modemId;

    (VOS_VOID)memset_s(&roamCfgInfo, sizeof(roamCfgInfo), 0x00, sizeof(roamCfgInfo));

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

        roamCfgInfo.roamFeatureFlg = NV_ITEM_ACTIVE;
        roamCfgInfo.roamCapability = 0;
#if (FEATURE_MBB_CUST == FEATURE_ON)
        roamCfgInfo.roamRule = NVIM_ROAM_RULE_CMP_PLMN;
#endif

        if (TAF_ACORE_NV_READ(modemId, NV_ITEM_ROAM_CAPA, &roamCfgInfo, sizeof(NAS_NVIM_RoamCfgInfo)) == NV_OK) {
            if ((roamCfgInfo.roamFeatureFlg != NV_ITEM_ACTIVE) && (roamCfgInfo.roamFeatureFlg != NV_ITEM_DEACTIVE)) {
                roamCfgInfo.roamFeatureFlg = AT_ROAM_FEATURE_OFF;
            }

            if (roamCfgInfo.roamCapability >= TAF_MMA_ROAM_BUTT) {
                roamCfgInfo.roamCapability = TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF;
            }

            netCtx->roamFeature = roamCfgInfo.roamFeatureFlg;
            netCtx->roamCapa = roamCfgInfo.roamCapability;
#if (FEATURE_MBB_CUST == FEATURE_ON)
            if (roamCfgInfo.roamRule >= NVIM_ROAM_RULE_BUTT) {
                roamCfgInfo.roamRule = NVIM_ROAM_RULE_CMP_PLMN;
            }
            netCtx->roamRule = roamCfgInfo.roamRule;
#endif
        } else {
            netCtx->roamFeature = AT_ROAM_FEATURE_OFF;
            netCtx->roamCapa    = TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF;
#if (FEATURE_MBB_CUST == FEATURE_ON)
            netCtx->roamRule    = NVIM_ROAM_RULE_CMP_PLMN;
#endif
        }
    }
}

VOS_VOID AT_ReadPrivacyFilterCfgNv(VOS_VOID)
{
    AT_ModemPrivacyFilterCtx *filterCtx = VOS_NULL_PTR;
    NAS_NV_PrivacyFilterCfg   privacyFilterCfg;
    ModemIdUint16             modemId;

    /* 初始化 */
    (VOS_VOID)memset_s(&privacyFilterCfg, sizeof(privacyFilterCfg), 0x00, sizeof(NAS_NV_PrivacyFilterCfg));

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        /* 读取NV项 */
        if (TAF_ACORE_NV_READ(modemId, NV_ITEM_PRIVACY_LOG_FILTER_CFG, &privacyFilterCfg,
                              sizeof(NAS_NV_PrivacyFilterCfg)) != NV_OK) {
            privacyFilterCfg.filterEnableFlg = VOS_FALSE;
        }

        /* NV读取值保护 */
        if (privacyFilterCfg.filterEnableFlg == VOS_TRUE) {
            privacyFilterCfg.filterEnableFlg = VOS_TRUE;
        }

        filterCtx = AT_GetModemPrivacyFilterCtxAddrFromModemId(modemId);

        filterCtx->filterEnableFlg = VOS_TRUE;
    }
}

VOS_VOID AT_ReadSystemAppConfigNV(VOS_VOID)
{
    VOS_UINT8               *systemAppConfig = VOS_NULL_PTR;
    NAS_NVIM_SystemAppConfig sysAppConfig;

    sysAppConfig.sysAppConfigType = SYSTEM_APP_BUTT;

    /* 获取控制特性的NV上下文地址 */
    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* 读取失败按默认值处理 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SYSTEM_APP_CONFIG, &sysAppConfig, sizeof(NAS_NVIM_SystemAppConfig)) !=
        VOS_OK) {
        *systemAppConfig = SYSTEM_APP_MP;
        return;
    }

    if (sysAppConfig.sysAppConfigType > SYSTEM_APP_ANDROID) {
        *systemAppConfig = SYSTEM_APP_MP;
    } else {
        *systemAppConfig = (VOS_UINT8)sysAppConfig.sysAppConfigType;
    }
}

VOS_VOID AT_ReadAtDislogPwdNV(VOS_VOID)
{
    VOS_UINT8              *systemAppConfig = VOS_NULL_PTR;
    TAF_AT_NvimDislogPwdNew dislogPwdNew;
    errno_t                 memResult;

    /* 按D25的做法  不保存权限标志, 则不用从NV中读取权限, 默认无权限 */
    g_ate5RightFlag = AT_E5_RIGHT_FLAG_NO;
    (VOS_VOID)memset_s(&dislogPwdNew, sizeof(dislogPwdNew), 0x00, sizeof(dislogPwdNew));

    /* 读取NV项中当前产品形态 */
    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* 新加NV项保存DISLOG密码(OPWORD使用) */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AT_DISLOG_PWD_NEW, &dislogPwdNew, AT_OPWORD_PWD_LEN) == NV_OK) {
        memResult = memcpy_s((VOS_INT8 *)g_atOpwordPwd, AT_OPWORD_PWD_LEN + 1, (VOS_INT8 *)(&dislogPwdNew),
                             AT_OPWORD_PWD_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_OPWORD_PWD_LEN + 1, AT_OPWORD_PWD_LEN);

        /* 跟上字符串结尾符 */
        g_atOpwordPwd[AT_OPWORD_PWD_LEN] = 0;

        /* 如果密码为空，且非AP-Modem形态，则取消密码保护 */
        if ((VOS_StrLen((VOS_CHAR *)g_atOpwordPwd) == 0) && (*systemAppConfig != SYSTEM_APP_ANDROID)) {
            /* NV中密码为空则输入任何密码都能获取权限 */
            g_ate5RightFlag = AT_E5_RIGHT_FLAG_YES;
        }
    } else {
        AT_WARN_LOG("AT_ReadAtDislogPwdNV:read NV_ITEM_AT_DISLOG_PWD_NEW failed");
    }
    (VOS_VOID)memset_s(&dislogPwdNew, sizeof(dislogPwdNew), 0x00, sizeof(dislogPwdNew));
}

VOS_VOID AT_ReadAtRightPasswordNV(VOS_VOID)
{
    TAF_AT_NvimRightOpenFlag nvimRightOpenFlg;
    errno_t                  memResult;

    (VOS_VOID)memset_s(&nvimRightOpenFlg, sizeof(nvimRightOpenFlg), 0x00, sizeof(nvimRightOpenFlg));

    /* 从NV中获取当前操作AT命令的权限 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AT_RIGHT_PASSWORD, &nvimRightOpenFlg, sizeof(nvimRightOpenFlg)) !=
        NV_OK) {
        /* 读取NV失败,采用默认密码 */
        (VOS_VOID)memset_s(&g_atRightOpenFlg, sizeof(g_atRightOpenFlg), 0x00, sizeof(g_atRightOpenFlg));
        AT_WARN_LOG("AT_ReadAtRightPasswordNV:read NV_ITEM_AT_RIGHT_PASSWORD failed");
    }

    memResult = memcpy_s(&g_atRightOpenFlg, sizeof(AT_RightOpenFlagInfo), &nvimRightOpenFlg,
                         sizeof(AT_RightOpenFlagInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(AT_RightOpenFlagInfo), sizeof(AT_RightOpenFlagInfo));
    (VOS_VOID)memset_s(&nvimRightOpenFlg, sizeof(nvimRightOpenFlg), 0x00, sizeof(nvimRightOpenFlg));
}

VOS_VOID AT_ReadAtDissdPwdNV(VOS_VOID)
{
    TAF_AT_NvimDissdPwd dissdPwd;
    errno_t             memResult;

    (VOS_VOID)memset_s(&dissdPwd, sizeof(dissdPwd), 0x00, sizeof(dissdPwd));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AT_DISSD_PWD, &dissdPwd, AT_DISSD_PWD_LEN) == NV_OK) {
        memResult = memcpy_s((VOS_INT8 *)g_ate5DissdPwd, AT_DISSD_PWD_LEN + 1, (VOS_INT8 *)(&dissdPwd),
                             AT_DISSD_PWD_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_DISSD_PWD_LEN + 1, AT_DISSD_PWD_LEN);
        /* 跟上字符串结尾符 */
        g_ate5DissdPwd[AT_DISSD_PWD_LEN] = 0;
    } else {
        AT_WARN_LOG("AT_ReadAtDissdPwdNV:read NV_ITEM_AT_DISSD_PWD failed");
    }
    (VOS_VOID)memset_s(&dissdPwd, sizeof(dissdPwd), 0x00, sizeof(dissdPwd));
}

VOS_VOID AT_ReadNotSupportRetValueNV(VOS_VOID)
{
    errno_t                       memResult;
    VOS_UINT32                    retLen;
    VOS_CHAR                      acRetVal[AT_NOTSUPPORT_STR_LEN + 1];
    TAF_AT_NotSupportCmdErrorText errorText;

    (VOS_VOID)memset_s(&errorText, sizeof(errorText), 0x00, sizeof(errorText));

    /*
     * 从NV中获取不支持命令的返回值，当为E5、LCARD、DONGLE时，
     * 该NV设置为"ERROR",其他平台设置为"COMMAND NOT SUPPORT"
     */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_NOT_SUPPORT_RET_VALUE, errorText.errorText, AT_NOTSUPPORT_STR_LEN) ==
        NV_OK) {
        memResult = memcpy_s(acRetVal, sizeof(acRetVal), errorText.errorText, AT_NOTSUPPORT_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acRetVal), AT_NOTSUPPORT_STR_LEN);

        /* 跟上字符串结尾符 */
        acRetVal[AT_NOTSUPPORT_STR_LEN] = 0;
        retLen                          = VOS_StrLen(acRetVal);
        if (retLen > 0) {
            /* AT_NOTSUPPORT_STR_LEN + 4是g_atCmdNotSupportStr数组定义时的长度 */
            memResult = strcpy_s((VOS_CHAR *)g_atCmdNotSupportStr, AT_NOTSUPPORT_STR_LEN + 4, acRetVal);
            TAF_STRCPY_CHK_RTN_VAL_CONTINUE(memResult, AT_NOTSUPPORT_STR_LEN + 4, AT_NOTSUPPORT_STR_LEN + 4);
        }
    } else {
        AT_WARN_LOG("AT_ReadNotSupportRetValueNV:read NV_ITEM_NOT_SUPPORT_RET_VALUE failed");
    }
}

VOS_VOID AT_ReadE5NV(VOS_VOID)
{
    AT_ReadAtDislogPwdNV();

    AT_ReadAtRightPasswordNV();

    AT_ReadAtDissdPwdNV();

    AT_ReadNotSupportRetValueNV();
}

VOS_UINT32 AT_IsAbortCmdCharValid(VOS_UINT8 *abortCmdChar, VOS_UINT32 len)
{
    VOS_UINT32 i;

    /*
     * 由于在AT命令的处理的流程中, 会对输入的字符进行格式化处理, 大致会过滤掉不可见字符
     * (<0x20的ASCII字符,空格, S3, S5)等.
     * 如果NV中设置的ABORT命令字符串中包含这些字符会导致打断命令匹配失败, 故做容错处理,
     * 如果NV中设置的ABORT命令字符串中包含这些字符则认为设置无效, 使用默认值
     */

    /* 如果打断命令或打断回复为空指针，无效 */
    if (abortCmdChar == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 如果打断命令或打断回复长度为0，无效 */
    if (len == 0) {
        return VOS_FALSE;
    }

    /* 如果打断命令或打断回复有不可见字符(<0x20的ASCII字符,空格, S3, S5)等，无效 */
    for (i = 0; i < len; i++) {
        if ((abortCmdChar[i] == g_atS3) || (abortCmdChar[i] == g_atS5) || (abortCmdChar[i] <= 0x20)) {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}

VOS_VOID AT_ReadAbortCmdParaNV(VOS_VOID)
{
    AT_ABORT_CmdPara    *abortCmdPara   = VOS_NULL_PTR;
    const VOS_UINT8      dfltAbortCmd[] = "AT";
    /* 命令比较时不区分大小写, 此处默认值定义为大小写均可 */
    const VOS_UINT8      dfltAbortRsp[] = "OK";
    AT_NVIM_AbortCmdPara nvAbortCmdPara;
    VOS_UINT32           isAbortCmdValid;
    VOS_UINT32           isAbortRspValid;
    VOS_UINT32           rlst;
    const VOS_UINT8      anyCharAbortCmd[] = "ANY";
    VOS_UINT32           len;
    errno_t              stringRet;

    /* 初始化 避免TQE告警 */
    (VOS_VOID)memset_s(&nvAbortCmdPara, sizeof(nvAbortCmdPara), 0x00, sizeof(nvAbortCmdPara));

    /* 打断命令初始化 */
    abortCmdPara = AT_GetAbortCmdPara();
    (VOS_VOID)memset_s(abortCmdPara, sizeof(AT_ABORT_CmdPara), 0x00, sizeof(AT_ABORT_CmdPara));

    /* 读取NV项中配置的打断命令和打断回复 */
    rlst = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AT_ABORT_CMD_PARA, &nvAbortCmdPara, sizeof(AT_NVIM_AbortCmdPara));

    /* NV读取失败则使用默认值 */
    if (rlst != NV_OK) {
        /* 打断使能标志默认值为TRUE */
        abortCmdPara->abortEnableFlg = VOS_TRUE;

        /* 打断命令默认值 */
        stringRet = strncpy_s((VOS_CHAR *)(abortCmdPara->abortAtCmdStr), sizeof(abortCmdPara->abortAtCmdStr),
                              (VOS_CHAR *)dfltAbortCmd, sizeof(dfltAbortCmd));
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, sizeof(abortCmdPara->abortAtCmdStr), AT_MAX_ABORT_CMD_STR_LEN);
        abortCmdPara->abortAtCmdStr[AT_MAX_ABORT_CMD_STR_LEN] = '\0';

        /* 打断回复默认值 */
        stringRet = strncpy_s((VOS_CHAR *)(abortCmdPara->abortAtRspStr), sizeof(abortCmdPara->abortAtRspStr),
                              (VOS_CHAR *)dfltAbortRsp, sizeof(dfltAbortRsp));
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, sizeof(abortCmdPara->abortAtRspStr), AT_MAX_ABORT_RSP_STR_LEN);
        abortCmdPara->abortAtRspStr[AT_MAX_ABORT_RSP_STR_LEN] = '\0';

        AT_WARN_LOG("AT_ReadAbortCmdParaNV:read NV_ITEM_AT_ABORT_CMD_PARA failed");

        return;
    }

    /* 如果打断功能未使能, 则可以不用关注其它内容 */
    abortCmdPara->abortEnableFlg = nvAbortCmdPara.abortEnableFlg;
    if (abortCmdPara->abortEnableFlg != VOS_TRUE) {
        return;
    }

    /*
     * 将NV中打断命令的内容赋值到全局变量，赋值长度为AT_MAX_ABORT_CMD_STR_LEN，
     * 然后将数组第AT_MAX_ABORT_CMD_STR_LEN+1个字符赋值为结束符，
     * 对应数组下标为AT_MAX_ABORT_CMD_STR_LEN
     */
    stringRet = strncpy_s((VOS_CHAR *)(abortCmdPara->abortAtCmdStr), sizeof(abortCmdPara->abortAtCmdStr),
                          (VOS_CHAR *)(nvAbortCmdPara.abortAtCmdStr), AT_MAX_ABORT_CMD_STR_LEN);
    TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, sizeof(abortCmdPara->abortAtCmdStr), AT_MAX_ABORT_CMD_STR_LEN);

    abortCmdPara->abortAtCmdStr[AT_MAX_ABORT_CMD_STR_LEN] = '\0';

    /*
     * 将NV中打断回复的内容赋值到全局变量，赋值长度为AT_MAX_ABORT_CMD_STR_LEN，
     * 然后将数组第AT_MAX_ABORT_RSP_STR_LEN+1个字符赋值为结束符，
     * 对应数组下标为AT_MAX_ABORT_RSP_STR_LEN
     */
    stringRet = strncpy_s((VOS_CHAR *)(abortCmdPara->abortAtRspStr), sizeof(abortCmdPara->abortAtRspStr),
                          (VOS_CHAR *)(nvAbortCmdPara.abortAtRspStr), AT_MAX_ABORT_RSP_STR_LEN);
    TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, sizeof(abortCmdPara->abortAtRspStr), AT_MAX_ABORT_RSP_STR_LEN);

    abortCmdPara->abortAtRspStr[AT_MAX_ABORT_RSP_STR_LEN] = '\0';

    /* 判断打断命令的NV设置中是否有无效字符 */
    isAbortCmdValid = AT_IsAbortCmdCharValid(abortCmdPara->abortAtCmdStr,
                                             VOS_StrLen((VOS_CHAR *)(abortCmdPara->abortAtCmdStr)));

    isAbortRspValid = AT_IsAbortCmdCharValid(abortCmdPara->abortAtRspStr,
                                             VOS_StrLen((VOS_CHAR *)(abortCmdPara->abortAtRspStr)));

    /* 如果NV设置中含有不可见字符(空格, S3, S5, 小于x020的字符), 则使用默认值 */
    if (isAbortCmdValid != VOS_TRUE) {
        stringRet = strncpy_s((VOS_CHAR *)(abortCmdPara->abortAtCmdStr), sizeof(abortCmdPara->abortAtCmdStr),
                              (VOS_CHAR *)dfltAbortCmd, AT_MAX_ABORT_CMD_STR_LEN);
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, sizeof(abortCmdPara->abortAtCmdStr), AT_MAX_ABORT_CMD_STR_LEN);
        abortCmdPara->abortAtCmdStr[AT_MAX_ABORT_CMD_STR_LEN] = '\0';
    }

    if (isAbortRspValid != VOS_TRUE) {
        stringRet = strncpy_s((VOS_CHAR *)(abortCmdPara->abortAtRspStr), sizeof(abortCmdPara->abortAtRspStr),
                              (VOS_CHAR *)dfltAbortRsp, AT_MAX_ABORT_RSP_STR_LEN);
        TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, sizeof(abortCmdPara->abortAtRspStr), AT_MAX_ABORT_RSP_STR_LEN);
        abortCmdPara->abortAtRspStr[AT_MAX_ABORT_RSP_STR_LEN] = '\0';
    }

    len = VOS_StrLen((VOS_CHAR *)abortCmdPara->abortAtCmdStr);

    if (VOS_StrLen((VOS_CHAR *)anyCharAbortCmd) != len) {
        return;
    }

    if ((VOS_StrNiCmp((VOS_CHAR *)anyCharAbortCmd, (VOS_CHAR *)abortCmdPara->abortAtCmdStr, len) == 0)) {
        /* PS_MEM_SET的时候已经将其置成0，即为默认不支持任意字符打断 */
        abortCmdPara->anyAbortFlg = VOS_TRUE;
    }
}

VOS_VOID AT_ReadSysNV(VOS_VOID)
{
    AT_ReadRoamCapaNV();

    AT_ReadSystemAppConfigNV();

    AT_ReadE5NV();

    AT_ReadAbortCmdParaNV();
}

VOS_VOID AT_ReadCellSignReportCfgNV(VOS_VOID)
{
    AT_ModemNetCtx            *netCtx = VOS_NULL_PTR;
    NAS_NVIM_CellSignReportCfg cellSignReportCfg;
    ModemIdUint16              modemId;

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

        (VOS_VOID)memset_s(&cellSignReportCfg, sizeof(cellSignReportCfg), 0x00, sizeof(cellSignReportCfg));

        if (TAF_ACORE_NV_READ(modemId, NV_ITEM_CELL_SIGN_REPORT_CFG, &cellSignReportCfg,
                              sizeof(NAS_NVIM_CellSignReportCfg)) != NV_OK) {
            AT_WARN_LOG("AT_ReadCellSignReportCfgNV:read NV_ITEM_CELL_SIGN_REPORT_CFG failed");
            return;
        }

        netCtx->cerssiReportType = cellSignReportCfg.signThreshold;

        if (cellSignReportCfg.signThreshold >= AT_CERSSI_REPORT_TYPE_BUTT) {
            netCtx->cerssiReportType = AT_CERSSI_REPORT_TYPE_NOT_REPORT;
        }

        netCtx->cerssiMinTimerInterval = cellSignReportCfg.minRptTimerInterval;

        if (cellSignReportCfg.minRptTimerInterval > AT_CERSSI_MAX_TIMER_INTERVAL) {
            netCtx->cerssiMinTimerInterval = 0;
        }
    }
}

VOS_UINT32 AT_ValidateWasCategory(AT_NvUeCapability *ueCapability)
{
    if (ueCapability->hSDSCHSupport != VOS_TRUE) {
        return VOS_ERR;
    }

    if ((ueCapability->hSDSCHPhyCategory < AT_UE_CATEGORY_MIN_VALID_VALUE) ||
        (ueCapability->hSDSCHPhyCategory > AT_UE_CATEGORY_MAX_VALID_VALUE)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_ValidateWasCategoryExt(AT_NvUeCapability *ueCapability)
{
    if ((ueCapability->hSDSCHSupport != VOS_TRUE) || (ueCapability->macEhsSupport != VOS_TRUE)) {
        return VOS_ERR;
    }

    if ((ueCapability->hSDSCHPhyCategoryExt < AT_UE_CATEGORYEXT_MIN_VALID_VALUE) ||
        (ueCapability->hSDSCHPhyCategoryExt > AT_UE_CATEGORYEXT_MAX_VALID_VALUE)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_ValidateWasCategoryExt2(AT_NvUeCapability *ueCapability)
{
    if ((ueCapability->hSDSCHSupport != VOS_TRUE) || (ueCapability->macEhsSupport != VOS_TRUE) ||
        (ueCapability->multiCellSupport != VOS_TRUE)) {
        return VOS_ERR;
    }

    if ((ueCapability->hSDSCHPhyCategoryExt2 < AT_UE_CATEGORYEXT2_MIN_VALID_VALUE) ||
        (ueCapability->hSDSCHPhyCategoryExt2 > AT_UE_CATEGORYEXT2_MAX_VALID_VALUE)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_ValidateWasCategoryExt3(AT_NvUeCapability *ueCapability)
{
    if ((ueCapability->hSDSCHSupport != VOS_TRUE) || (ueCapability->macEhsSupport != VOS_TRUE) ||
        (ueCapability->multiCellSupport != VOS_TRUE) || (ueCapability->dcMimoSupport != VOS_TRUE)) {
        return VOS_ERR;
    }

    if ((ueCapability->hSDSCHPhyCategoryExt3 < AT_UE_CATEGORYEXT3_MIN_VALID_VALUE) ||
        (ueCapability->hSDSCHPhyCategoryExt3 > AT_UE_CATEGORYEXT3_MAX_VALID_VALUE)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT8 AT_GetWasDefaultCategory(AT_NvUeCapability *ueCapability)
{
    AT_RateWcdmaCategoryUint8 rateCategory;

    switch (ueCapability->asRelIndicator) {
        case PS_PTL_VER_R5:
        case PS_PTL_VER_R6:
            rateCategory = AT_RATE_WCDMA_CATEGORY_10;
            break;

        case PS_PTL_VER_R7:
            rateCategory = AT_RATE_WCDMA_CATEGORY_18;
            break;

        case PS_PTL_VER_R8:
            rateCategory = AT_RATE_WCDMA_CATEGORY_28;
            break;

        default:
            rateCategory = AT_RATE_WCDMA_CATEGORY_10;
            break;
    }

    return rateCategory;
}

VOS_UINT8 AT_CalcWasCategory(AT_NvUeCapability *ueCapability)
{
    AT_RateWcdmaCategoryUint8 rateCategory;

    if (AT_ValidateWasCategoryExt3(ueCapability) == VOS_OK) {
        return ueCapability->hSDSCHPhyCategoryExt3;
    }

    if (AT_ValidateWasCategoryExt2(ueCapability) == VOS_OK) {
        return ueCapability->hSDSCHPhyCategoryExt2;
    }

    if (AT_ValidateWasCategoryExt(ueCapability) == VOS_OK) {
        return ueCapability->hSDSCHPhyCategoryExt;
    }

    if (AT_ValidateWasCategory(ueCapability) == VOS_OK) {
        return ueCapability->hSDSCHPhyCategory;
    }

    rateCategory = AT_GetWasDefaultCategory(ueCapability);

    return rateCategory;
}

VOS_VOID AT_ReadWasCapabilityNV(VOS_VOID)
{
    AT_NvUeCapability nvUeCapability;
    VOS_UINT32        result;

    (VOS_VOID)memset_s(&nvUeCapability, sizeof(nvUeCapability), 0x00, sizeof(nvUeCapability));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, &nvUeCapability,
                               sizeof(AT_NvUeCapability));
    if (result != VOS_OK) {
        /* NV读取失败, 协议版本默认为R99 */
        g_atDlRateCategory.wasRelIndicator = PS_PTL_VER_R99;
        g_atDlRateCategory.wasCategory     = AT_RATE_WCDMA_R99;
    } else {
        g_atDlRateCategory.wasRelIndicator = (VOS_INT8)nvUeCapability.asRelIndicator;
        g_atDlRateCategory.wasCategory     = AT_CalcWasCategory(&nvUeCapability);
    }
}

VOS_VOID AT_CalcGasCategory(NVIM_GAS_HighMultiSlotClass *highMultislotclass, VOS_UINT16 gprsMultiSlotClass,
                            VOS_UINT16 egprsMultiSlotClass, VOS_UINT16 egprsFlag)
{
    /* 默认为VOS_FALSE */
    g_atDlRateCategory.gasMultislotClass33Flg = VOS_FALSE;

    if ((highMultislotclass->highMultislotClassFlg == VOS_TRUE) &&
        (highMultislotclass->highMultislotClass == AT_GAS_HIGH_MULTISLOT_CLASS_0)) {
        if ((egprsFlag == VOS_FALSE) && (gprsMultiSlotClass == AT_GAS_GRR_MULTISLOT_CLASS_MAX)) {
            g_atDlRateCategory.gasMultislotClass33Flg = VOS_TRUE;
        }

        if ((egprsFlag == VOS_TRUE) && (egprsMultiSlotClass == AT_GAS_GRR_MULTISLOT_CLASS_MAX)) {
            g_atDlRateCategory.gasMultislotClass33Flg = VOS_TRUE;
        }
    }
}

VOS_VOID AT_ReadGasCapabilityNV(VOS_VOID)
{
    VOS_UINT32                  result;
    NVIM_GAS_HighMultiSlotClass highMultislotclass;
    VOS_UINT16                  gprsMultiSlotClass;
    VOS_UINT16                  egprsMultiSlotClass;
    VOS_UINT16                  egprsFlag;

    highMultislotclass.highMultislotClassFlg = VOS_FALSE;
    highMultislotclass.highMultislotClass    = AT_GAS_HIGH_MULTISLOT_CLASS_0;

    gprsMultiSlotClass  = 0;
    egprsMultiSlotClass = 0;
    egprsFlag           = VOS_FALSE;

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_EGPRS_FLAG, &egprsFlag, sizeof(VOS_UINT16));
    if (result != NV_OK || egprsFlag != VOS_FALSE) {
        egprsFlag = VOS_TRUE;
    }

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_GPRS_MULTI_SLOT_CLASS, &gprsMultiSlotClass, sizeof(VOS_UINT16));

    /* 读取失败，默认多时隙能力等级为12 */
    if (result != NV_OK) {
        gprsMultiSlotClass = AT_GAS_GRR_MULTISLOT_CLASS_MAX;
    }

    /* 多时隙能力等级取值为1-12 */
    if ((gprsMultiSlotClass == 0) || (gprsMultiSlotClass > AT_GAS_GRR_MULTISLOT_CLASS_MAX)) {
        gprsMultiSlotClass = AT_GAS_GRR_MULTISLOT_CLASS_MAX;
    }

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_EGPRS_MULTI_SLOT_CLASS, &egprsMultiSlotClass, sizeof(VOS_UINT16));
    if (result != NV_OK) {
        egprsMultiSlotClass = AT_GAS_GRR_MULTISLOT_CLASS_MAX;
    }

    /* 多时隙能力等级取值为1-12 */
    if ((egprsMultiSlotClass == 0) || (egprsMultiSlotClass > AT_GAS_GRR_MULTISLOT_CLASS_MAX)) {
        egprsMultiSlotClass = AT_GAS_GRR_MULTISLOT_CLASS_MAX;
    }

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_GAS_HIGH_MULTI_SLOT_CLASS, &highMultislotclass,
                               sizeof(NVIM_GAS_HighMultiSlotClass));

    /* 读取失败，默认High Multislot Class 无效  */
    if (result != NV_OK) {
        return;
    }

    if ((highMultislotclass.highMultislotClassFlg == VOS_TRUE) &&
        (highMultislotclass.highMultislotClass != AT_GAS_HIGH_MULTISLOT_CLASS_0)) {
        highMultislotclass.highMultislotClass = AT_GAS_HIGH_MULTISLOT_CLASS_0;
    }

    AT_CalcGasCategory(&highMultislotclass, gprsMultiSlotClass, egprsMultiSlotClass, egprsFlag);
}

VOS_VOID AT_ReadPppDialErrCodeNV(VOS_VOID)
{
    NAS_NV_PppDialErrCode pppErrRpt;
    VOS_UINT32            result;

    pppErrRpt.status     = VOS_FALSE;
    pppErrRpt.errCodeRpt = PPP_DIAL_ERR_CODE_BUTT;

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PPP_DIAL_ERR_CODE, &pppErrRpt, sizeof(NAS_NV_PppDialErrCode));

    if ((result == NV_OK) && (pppErrRpt.status == VOS_TRUE) && (pppErrRpt.errCodeRpt == PPP_DIAL_ERR_CODE_ENABLE)) {
        g_pppDialErrCodeRpt = PPP_DIAL_ERR_CODE_ENABLE;
    } else {
        g_pppDialErrCodeRpt = PPP_DIAL_ERR_CODE_DISABLE;
    }
}

VOS_VOID AT_ReadReportRegActFlgNV(VOS_VOID)
{
    VOS_UINT32                 result;
    TAF_AT_NvimReportRegActFlg reportRegFlg;

    (VOS_VOID)memset_s(&reportRegFlg, sizeof(reportRegFlg), 0x00, sizeof(TAF_AT_NvimReportRegActFlg));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_REPORT_REG_ACT_FLG, &(reportRegFlg.reportRegActFlg),
                               sizeof(reportRegFlg.reportRegActFlg));

    if ((result == NV_OK) && (reportRegFlg.reportRegActFlg == VOS_TRUE)) {
        g_reportCregActParaFlg = reportRegFlg.reportRegActFlg;
    } else {
        g_reportCregActParaFlg = VOS_FALSE;
    }
}

VOS_VOID AT_ReadCregAndCgregCiFourByteRptNV(VOS_VOID)
{
    NAS_NV_CregCgregCiFourByteRpt ciFourByteRpt;
    VOS_UINT32                    result;

    ciFourByteRpt.status     = VOS_FALSE;
    ciFourByteRpt.ciBytesRpt = 0;

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CREG_CGREG_CI_FOUR_BYTE_RPT, &ciFourByteRpt,
                               sizeof(NAS_NV_CregCgregCiFourByteRpt));

    if ((result == NV_OK) && (ciFourByteRpt.status == VOS_TRUE) &&
        (ciFourByteRpt.ciBytesRpt == CREG_CGREG_CI_RPT_FOUR_BYTE)) {
        g_ciRptByte = CREG_CGREG_CI_RPT_FOUR_BYTE;
    } else {
        g_ciRptByte = CREG_CGREG_CI_RPT_TWO_BYTE;
    }
}

VOS_VOID AT_ReadIpv6CapabilityNV(VOS_VOID)
{
    AT_NV_Ipv6Capability nvIpv6Capability;
    VOS_UINT32           rslt;
    AT_CommPsCtx        *commPsCtx = VOS_NULL_PTR;
    VOS_UINT32           ipv6CapabilityValid;

    (VOS_VOID)memset_s(&nvIpv6Capability, sizeof(nvIpv6Capability), 0x00, sizeof(AT_NV_Ipv6Capability));

    commPsCtx = AT_GetCommPsCtxAddr();

    rslt = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_IPV6_CAPABILITY, &nvIpv6Capability, sizeof(AT_NV_Ipv6Capability));

    ipv6CapabilityValid = AT_PS_IsIpv6CapabilityValid(nvIpv6Capability.ipv6Capablity);

    if ((rslt == NV_OK) && (nvIpv6Capability.status == VOS_TRUE) && (ipv6CapabilityValid == VOS_TRUE)) {
        commPsCtx->ipv6Capability = nvIpv6Capability.ipv6Capablity;
    } else {
        commPsCtx->ipv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY;
    }
}

VOS_VOID AT_ReadIpv6AddrTestModeCfgNV(VOS_VOID)
{
    TAF_NVIM_Ipv6AddrTestModeCfg ipv6AddrTestModeCfg;
    AT_CommPsCtx                *commPsCtx = VOS_NULL_PTR;
    VOS_UINT32                   rslt;

    (VOS_VOID)memset_s(&ipv6AddrTestModeCfg, sizeof(ipv6AddrTestModeCfg), 0x00, sizeof(TAF_NVIM_Ipv6AddrTestModeCfg));

    commPsCtx = AT_GetCommPsCtxAddr();

    rslt = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_IPV6_ADDRESS_TEST_MODE_CFG, &ipv6AddrTestModeCfg,
                             sizeof(ipv6AddrTestModeCfg));

    if (rslt == NV_OK) {
        commPsCtx->ipv6AddrTestModeCfg = ipv6AddrTestModeCfg.ipv6AddrTestModeCfg;
    } else {
        commPsCtx->ipv6AddrTestModeCfg = 0;
    }
}

VOS_VOID AT_ReadSharePdpInfoNV(VOS_VOID)
{
    TAF_NVIM_SharePdpInfo sharePdpInfo;
    AT_CommPsCtx         *commPsCtx = VOS_NULL_PTR;
    VOS_UINT32            rslt;

    (VOS_VOID)memset_s(&sharePdpInfo, sizeof(sharePdpInfo), 0x00, sizeof(TAF_NVIM_SharePdpInfo));

    commPsCtx = AT_GetCommPsCtxAddr();

    rslt = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SHARE_PDP_INFO, &sharePdpInfo, sizeof(sharePdpInfo));

    if ((rslt == NV_OK) && (sharePdpInfo.enableFlag == VOS_TRUE)) {
        commPsCtx->sharePdpFlag = sharePdpInfo.enableFlag;
    } else {
        commPsCtx->sharePdpFlag = VOS_FALSE;
    }
}

VOS_VOID AT_ReadDialConnectDisplayRateNV(VOS_VOID)
{
    VOS_UINT32                     result;
    AT_NVIM_DialConnectDisplayRate dialConnectDisplayRate;

    (VOS_VOID)memset_s(&dialConnectDisplayRate, sizeof(dialConnectDisplayRate), 0x00, sizeof(dialConnectDisplayRate));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_DIAL_CONNECT_DISPLAY_RATE, &dialConnectDisplayRate,
                               sizeof(AT_NVIM_DialConnectDisplayRate));
    if (result != NV_OK) {
        (VOS_VOID)memset_s(&g_dialConnectDisplayRate, sizeof(g_dialConnectDisplayRate), 0x00,
                           sizeof(g_dialConnectDisplayRate));
        return;
    }

    g_dialConnectDisplayRate.dpaConnectRate   = dialConnectDisplayRate.dpaConnectRate;
    g_dialConnectDisplayRate.edgeConnectRate  = dialConnectDisplayRate.edgeConnectRate;
    g_dialConnectDisplayRate.gprsConnectRate  = dialConnectDisplayRate.gprsConnectRate;
    g_dialConnectDisplayRate.gsmConnectRate   = dialConnectDisplayRate.gsmConnectRate;
    g_dialConnectDisplayRate.reserve1         = dialConnectDisplayRate.reserve1;
    g_dialConnectDisplayRate.reserve2         = dialConnectDisplayRate.reserve2;
    g_dialConnectDisplayRate.reserve3         = dialConnectDisplayRate.reserve3;
    g_dialConnectDisplayRate.wcdmaConnectRate = dialConnectDisplayRate.wcdmaConnectRate;
}

LOCAL VOS_VOID AT_SetApnCustFormatCfg(TAF_NVIM_ApnCustomFormatCfg *apnNVCfg, AT_PS_ApnCustomFormatCfg *apnAtCfg)
{
    VOS_UINT32 charNum;
    VOS_UINT32 charNvNum;
    VOS_UINT32 i;

    apnAtCfg->protocolStringCheckFlag = (apnNVCfg->ucProtocolStringCheckFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;

    charNum   = 0;
    charNvNum = TAF_MIN(apnNVCfg->ucCustomCharNum, AT_PS_APN_CUSTOM_CHAR_MAX_NUM);
    for (i = 0; i < charNvNum; i++) {
        /* 控制字符,正反斜杠不容许设置为特殊字符 */
        if ((apnNVCfg->aucCustomChar[i] > AT_PS_ASCII_UINT_SEPARATOR) &&
            (apnNVCfg->aucCustomChar[i] != AT_PS_ASCII_DELETE) && (apnNVCfg->aucCustomChar[i] != AT_PS_ASCII_SLASH) &&
            (apnNVCfg->aucCustomChar[i] != AT_PS_ASCII_BACKSLASH)) {
            apnAtCfg->customChar[charNum] = apnNVCfg->aucCustomChar[i];
            charNum++;
        }
    }
    apnAtCfg->customCharNum = (VOS_UINT8)charNum;
}

VOS_VOID AT_ReadApnCustomFormatCfgNV(VOS_VOID)
{
    TAF_NVIM_ApnCustomFormatCfg apnCustomFormatCfg;
    AT_CommPsCtx               *commPsCtx = VOS_NULL_PTR;
    VOS_UINT32                  rslt;
    VOS_UINT32                  i;

    (VOS_VOID)memset_s(&apnCustomFormatCfg, sizeof(apnCustomFormatCfg), 0x00, sizeof(TAF_NVIM_ApnCustomFormatCfg));

    commPsCtx = AT_GetCommPsCtxAddr();

    /* 多Modem分别处理 */
    for (i = 0; i < MODEM_ID_BUTT; i++) {
        rslt = TAF_ACORE_NV_READ(i, NV_ITEM_APN_CUSTOM_FORMAT_CFG, &apnCustomFormatCfg, sizeof(apnCustomFormatCfg));

        if (rslt != NV_OK) {
            commPsCtx->apnCustomFormatCfg[i].customCharNum           = 0;
            commPsCtx->apnCustomFormatCfg[i].protocolStringCheckFlag = VOS_FALSE;
        } else {
            AT_SetApnCustFormatCfg(&apnCustomFormatCfg, &(commPsCtx->apnCustomFormatCfg[i]));
        }
    }
}

VOS_VOID AT_ReadPsNV(VOS_VOID)
{
    /* 读取拨号错误码上报NV */
    AT_ReadPppDialErrCodeNV();

    /* 读取是否上报CREG/CGREG的ACT参数控制NV */
    AT_ReadReportRegActFlgNV();

    /* 读取CREG/CGREG的CI参数是否以4字节上报的控制NV(Vodafone需求) */
    AT_ReadCregAndCgregCiFourByteRptNV();

    /* 读取IPV6能力的NV, 目前IPV6能力只能支持到IPV4V6_OVER_ONE_PDP */
    AT_ReadIpv6CapabilityNV();

    AT_ReadIpv6AddrTestModeCfgNV();

    /* 读取拨号系统托盘显示速率定制NV */
    AT_ReadDialConnectDisplayRateNV();

    /* 读取WCDMA接入等级NV, 用于PS域拨号速率上报 */
    AT_ReadWasCapabilityNV();

    /* 读取GAS是否支持CLASS33 NV项目 */
    AT_ReadGasCapabilityNV();

    AT_ReadCellSignReportCfgNV();

    AT_ReadSharePdpInfoNV();

    AT_ReadApnCustomFormatCfgNV();
}

VOS_VOID AT_ReadCsNV(VOS_VOID)
{
    TAF_UssdNvimTransMode ussdTranMode;
    TAF_CCA_TelePara      timeInfo;
    ModemIdUint16         modemId;
    AT_ModemCcCtx        *ccCtx = VOS_NULL_PTR;
    AT_ModemSsCtx        *ssCtx = VOS_NULL_PTR;

    NAS_NVIM_CustomCallCfg callCfgInfo;

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        ccCtx = AT_GetModemCcCtxAddrFromModemId(modemId);
        ssCtx = AT_GetModemSsCtxAddrFromModemId(modemId);

        (VOS_VOID)memset_s(&timeInfo, sizeof(timeInfo), 0x00, sizeof(timeInfo));

        /* 读取语音自动应答 */
        if (TAF_ACORE_NV_READ(modemId, NV_ITEM_CCA_TELE_PARA, &timeInfo, sizeof(TAF_CCA_TelePara)) == NV_OK) {
            ccCtx->s0TimeInfo.s0TimerLen = timeInfo.s0TimerLen;
        }

        (VOS_VOID)memset_s(&ussdTranMode, sizeof(ussdTranMode), 0x00, sizeof(ussdTranMode));

        /* 读取USSD是否透传设置 */
        if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CUST_USSD_MODE, &ussdTranMode, sizeof(TAF_UssdNvimTransMode)) ==
            NV_OK) {
            if (ussdTranMode.status == VOS_TRUE) {
                ssCtx->ussdTransMode = ussdTranMode.ussdTransMode;
            }
        }

        /* 读取语音功能定制 */
        (VOS_VOID)memset_s(&callCfgInfo, sizeof(callCfgInfo), 0x00, sizeof(callCfgInfo));

        if (TAF_ACORE_NV_READ(modemId, NV_ITEM_CUSTOM_CALL_CFG, &callCfgInfo, sizeof(NAS_NVIM_CustomCallCfg)) ==
            NV_OK) {
            /* 如果NV取值超过范围，使用默认值CVHU_MODE_0 */
            if (callCfgInfo.cvhuMode >= CVHU_MODE_BUTT) {
                ccCtx->cvhuMode = CVHU_MODE_0;
            } else {
                ccCtx->cvhuMode = callCfgInfo.cvhuMode;
            }
        }
    }

    AT_ReadSsNV();
}

#if (FEATURE_LTE == FEATURE_ON)

VOS_VOID AT_ReadLTENV(VOS_VOID)
{
    VOS_UINT32 result;

    (VOS_VOID)memset_s(&g_rsrpCfg, sizeof(g_rsrpCfg), 0x00, sizeof(g_rsrpCfg));
    (VOS_VOID)memset_s(&g_rscpCfg, sizeof(g_rscpCfg), 0x00, sizeof(g_rscpCfg));
    (VOS_VOID)memset_s(&g_ecioCfg, sizeof(g_ecioCfg), 0x00, sizeof(g_ecioCfg));

    /* 从NV中获取门限值 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TAF_LTE_RSRP_CFG, &g_rsrpCfg, sizeof(TAF_NVIM_LteRsrpCfg));

    if (result != NV_OK) {
        PS_PRINTF_WARNING("read RSRP NV fail!\n");
        return;
    }

    /* 从NV中获取门限值 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TAF_LTE_RSCP_CFG, &g_rscpCfg, sizeof(TAF_NVIM_LteRscpCfg));

    if (result != NV_OK) {
        PS_PRINTF_WARNING("read RSCP NV fail!\n");
        return;
    }

    /* 从NV中获取门限值 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TAF_LTE_ECIO_CFG, &g_ecioCfg, sizeof(TAF_NVIM_LteEcioCfg));

    if (result != NV_OK) {
        PS_PRINTF_WARNING("read ECIO NV fail!\n");
        return;
    }
}

#endif

VOS_VOID AT_ReadAgpsNv(VOS_VOID)
{
    AT_ModemAgpsCtx      *agpsCtx = VOS_NULL_PTR;
    TAF_NVIM_XcposrrptCfg xcposrRptCfg;
    ModemIdUint16         modemId;

    (VOS_VOID)memset_s(&xcposrRptCfg, sizeof(xcposrRptCfg), 0x00, sizeof(xcposrRptCfg));

    /* 读取NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_XCPOSRRPT_CFG, &xcposrRptCfg, sizeof(xcposrRptCfg)) == NV_OK) {
        for (modemId = MODEM_ID_0; modemId < MODEM_ID_BUTT; modemId++) {
            agpsCtx               = AT_GetModemAgpsCtxAddrFromModemId(modemId);
            agpsCtx->cposrReport  = xcposrRptCfg.cposrDefault;
            agpsCtx->xcposrReport = xcposrRptCfg.xcposrRptDefault;
        }
    } else {
        AT_ERR_LOG("AT_ReadAgpsNv: NV_Read Failed!");
    }
}

LOCAL VOS_VOID AT_ReadCustomUsimmCfg(VOS_VOID)
{
    VOS_UINT32              result;
    AT_CommCtx             *commCtx = VOS_NULL_PTR;
    NAS_NVIM_CustomUsimmCfg customUsimmCfg;

    commCtx = AT_GetCommCtxAddr();

    (VOS_VOID)memset_s(&customUsimmCfg, sizeof(NAS_NVIM_CustomUsimmCfg), 0, sizeof(NAS_NVIM_CustomUsimmCfg));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CUSTOM_USIMM_CFG, &customUsimmCfg, sizeof(NAS_NVIM_CustomUsimmCfg));

    if ((result != NV_OK) || ((customUsimmCfg.simsqEnable != 0) && (customUsimmCfg.simsqEnable != 1))) {
        (VOS_VOID)memset_s(&commCtx->customUsimmCfg, sizeof(NAS_NVIM_CustomUsimmCfg), 0,
                           sizeof(NAS_NVIM_CustomUsimmCfg));
        AT_WARN_LOG("AT_ReadCustomUsimmCfg: read nv failed!");
    } else {
        commCtx->customUsimmCfg.simsqEnable = customUsimmCfg.simsqEnable;
    }
}

VOS_VOID AT_ReadAtCmdTrustListCfgNv(VOS_VOID)
{
    AT_CommCtx                *commCtx = VOS_NULL_PTR;
    TAF_NVIM_AtCmdTrustListCfg nvimAtCmdTrustListCfg;
    VOS_UINT32                 result;

    commCtx = AT_GetCommCtxAddr();

    (VOS_VOID)memset_s(&nvimAtCmdTrustListCfg, sizeof(TAF_NVIM_AtCmdTrustListCfg), 0x00,
        sizeof(TAF_NVIM_AtCmdTrustListCfg));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AT_CMD_TRUST_LIST_CFG, &nvimAtCmdTrustListCfg,
        sizeof(TAF_NVIM_AtCmdTrustListCfg));

    if (result != NV_OK) {
        AT_ERR_LOG("AT_ReadAtCmdTrustListCfgNv: read NV_ITEM_AT_CMD_TRUST_LIST_CFG failed!");
        commCtx->cmdTrustListCtrl.atCmdTrustListEnableFlg = VOS_TRUE;

        return;
    }

    commCtx->cmdTrustListCtrl.atCmdTrustListEnableFlg =
        (nvimAtCmdTrustListCfg.atCmdTrustListEnableFlg == VOS_FALSE) ? VOS_FALSE : VOS_TRUE;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)

/*
 * 功能说明: 从NV中读取Syscfgex的制式列表
 */
LOCAL VOS_VOID AT_ReadSysCfgExRatGroupCtrlInfo(VOS_VOID)
{
    TAF_NVIM_SysCfgExRatGroupCtrl groupCtrl;
    VOS_UINT32 result;
    errno_t memResult;
    AT_ModemMbbSysCfgExRatGroupCtrl *sysCfgExRatGroup = AT_GetModemMbbSysCfgExRatGroupCtrl(MODEM_ID_0);

    (VOS_VOID)memset_s(&groupCtrl, sizeof(TAF_NVIM_SysCfgExRatGroupCtrl), 0, sizeof(TAF_NVIM_SysCfgExRatGroupCtrl));
    (VOS_VOID)memset_s(sysCfgExRatGroup, sizeof(AT_ModemMbbSysCfgExRatGroupCtrl),
        0, sizeof(AT_ModemMbbSysCfgExRatGroupCtrl));
    /* 读NV */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SYSCFGEX_RAT_GROUP_CTRL, &groupCtrl, sizeof(TAF_NVIM_SysCfgExRatGroupCtrl));
    if (result != NV_OK) {
        AT_ERR_LOG("AT_ReadSysCfgExRatGroupCtrlInfo: Read NV50198 Failed!");
        return;
    }
    if (groupCtrl.itemNum > TAF_NVIM_SYSCFGEX_RAT_GROUP_MAX) {
        AT_ERR_LOG("AT_ReadSysCfgExRatGroupCtrlInfo: itemNum error!");
        return;
    }
    sysCfgExRatGroup->itemNum = TAF_MIN(groupCtrl.itemNum, AT_SYSCFGEX_GROUP_MAX);
    sysCfgExRatGroup->restrictFlg = groupCtrl.restrictFlg;
    memResult = memcpy_s(sysCfgExRatGroup->ratGroupList, (AT_SYSCFGEX_GROUP_MAX * AT_SYSCFGEX_RAT_MAX),
        groupCtrl.ratGroupList, (TAF_NVIM_SYSCFGEX_RAT_GROUP_MAX * TAF_NVIM_SYSCFGEX_RAT_NUM_MAX));
    TAF_MEM_CHK_RTN_VAL(memResult, (AT_SYSCFGEX_GROUP_MAX * AT_SYSCFGEX_RAT_MAX),
        (TAF_NVIM_SYSCFGEX_RAT_GROUP_MAX * TAF_NVIM_SYSCFGEX_RAT_NUM_MAX));
}

/*
 * 功能说明: 读取AUTO制式转换功能NV
 */
LOCAL VOS_VOID AT_ReadAutoModeRatListInfoFromNv(VOS_VOID)
{
    TAF_NVIM_AutoModeRatInfo ratInfo;
    VOS_UINT32 result;
    AT_ModemMbbAutoModeRatInfo *autoModeRatInfo = AT_GetModemMbbModeRatInfo(MODEM_ID_0);
    autoModeRatInfo->ratNum = 0;
    (VOS_VOID)memset_s(&ratInfo, sizeof(TAF_NVIM_AutoModeRatInfo), 0, sizeof(TAF_NVIM_AutoModeRatInfo));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_AUTO_MODE_RAT_LIST, &ratInfo, sizeof(TAF_NVIM_AutoModeRatInfo));
    if (result != NV_OK) {
        AT_ERR_LOG("AT_ReadAutoModeRatListInfoFromNv: read nv fail!");
        return;
    }
    if (ratInfo.ratNum > TAF_NVIM_AUTO_MODE_RAT_MAX) {
        AT_ERR_LOG("AT_ReadAutoModeRatListInfoFromNv: ratNum error!");
        return;
    }
    autoModeRatInfo->ratNum = TAF_MIN(ratInfo.ratNum, AT_AUTO_MODE_RAT_LIST_MAX_NUM);
    for (VOS_UINT32 loop = 0; loop < autoModeRatInfo->ratNum; loop++) {
        autoModeRatInfo->ratList[loop] = ratInfo.ratList[loop];
    }
}

/*
 * 功能说明: 读取AUTO制式转换功能NV
 */
VOS_VOID AT_ReadIccidTransModeFromNv(VOS_VOID)
{
    TAF_NVIM_IccidTransMode iccidTransMode = {0};
    VOS_UINT32 result;
    AT_ModemIccidTransMode *transMode = AT_GetModemMbbIccidTransMode(MODEM_ID_0);

    (VOS_VOID)memset_s(transMode, sizeof(AT_ModemIccidTransMode), 0, sizeof(AT_ModemIccidTransMode));
    transMode->transMode = AT_ICCID_TRANS_TRANSMISSION; /* 初始化为透传模式 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_ICCID_TRANS_MODE, &iccidTransMode, sizeof(iccidTransMode));
    if (result != NV_OK) {
        AT_ERR_LOG("AT_ReadIccidTransModeFromNv: read nv fail!");
        return;
    }
    transMode->transMode = iccidTransMode.transMode;
}

#endif

VOS_VOID AT_ReadMbbFeatureNv(VOS_VOID)
{
    TAF_NVIM_MbbFeatureCfg    readMbbFeatureCfg;
    AT_ModemMbbCustmizeCtx   *featureCfgInfo = VOS_NULL_PTR;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    VOS_UINT32                result;
#endif
    ModemIdUint16             modemId;

    memset_s(&readMbbFeatureCfg, sizeof(TAF_NVIM_MbbFeatureCfg), 0x00, sizeof(TAF_NVIM_MbbFeatureCfg));

#if (FEATURE_MBB_CUST == FEATURE_ON)
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_MBB_FEATURE_CFG, &readMbbFeatureCfg, sizeof(TAF_NVIM_MbbFeatureCfg));

    if (result != NV_OK) {
        AT_ERR_LOG("AT_ReadMbbFeatureNv: read NV_ITEM_MBB_FEATURE_CFG failed!");

        return;
    }
#endif

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        featureCfgInfo = AT_GetModemMbbCustCtxAddrFromModemId(modemId);

        featureCfgInfo->voipApnKey = (readMbbFeatureCfg.voipApnKey == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->dialStickFlg = (readMbbFeatureCfg.dialStickFlg == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->dtmfCustFlg = (readMbbFeatureCfg.dtmfCustFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->ssAbortEnable= (readMbbFeatureCfg.ssAbortEnable == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->smsAbortEnable= (readMbbFeatureCfg.smsAbortEnable == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->nwTimeQryFlg = (readMbbFeatureCfg.nwTimeQryFlg == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->blockListFlg = (readMbbFeatureCfg.simLockBlockListFlg == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->vcomEchoFlag = (readMbbFeatureCfg.vcomEchoFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->powerManagerSwitch = (readMbbFeatureCfg.powerManagerSwitch == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->telematicAudioFlag= (readMbbFeatureCfg.telematicAudioFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->pcmAudioFlag = (readMbbFeatureCfg.wpgPcmFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->confDelayReportFlag = (readMbbFeatureCfg.confFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->ccwaWithInternationNumType = (readMbbFeatureCfg.ccwaFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->cendWithEndSorceFlag = (readMbbFeatureCfg.cendFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
        featureCfgInfo->clipWithEmptyParaElideFlag = (readMbbFeatureCfg.clipFlag == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;

    }
}

VOS_VOID AT_ReadMbbCustomizeNv(VOS_VOID)
{
    AT_ReadMbbFeatureNv();
#if (FEATURE_MBB_CUST == FEATURE_ON)
    AT_ReadSysCfgExRatGroupCtrlInfo();
    AT_ReadAutoModeRatListInfoFromNv();
    AT_ReadIccidTransModeFromNv();
#endif
}

VOS_VOID AT_ReadNV(VOS_VOID)
{
    /* 平台接入技术NV读取 */
    AT_ReadPlatformNV();

    /* client NV读取 */
    AT_ReadClientConfigNV();

    /* 读取系统相关的NV项 */
    AT_ReadSysNV();

    /* 读取PS域相关的NV项 */
    AT_ReadPsNV();

    /* 读取cs域（CC和SS相关的NV) */
    AT_ReadCsNV();

    /* 读取SMS相关的NV */
    AT_ReadSmsNV();

#if (FEATURE_LTE == FEATURE_ON)
    /* 读取LTE NV项 */
    AT_ReadLTENV();

#endif

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /* 读取uart相关相关NV项 */
    AT_ReadUartCfgNV();
#endif

    AT_ReadNvloadCarrierNV();

    AT_ReadPortBuffCfgNV();

    AT_ReadPrivacyFilterCfgNv();

    AT_ReadAgpsNv();

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    AT_ReadCdmaModemSwitchNotResetCfgNv();
#endif

    AT_ReadCustomUsimmCfg();

    /* MBB AT模块定制NV读取接口 */
    AT_ReadMbbCustomizeNv();

    AT_ReadAtCmdTrustListCfgNv();
}

VOS_UINT32 AT_ReadPhyNV(VOS_VOID)
{
    VOS_BOOL      bImeiIsNull;
    ModemIdUint16 modemId;

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        if (AT_PhyNumIsNull(modemId, AT_PHYNUM_TYPE_IMEI, &bImeiIsNull) != AT_OK) {
            return VOS_ERR;
        }

        if (bImeiIsNull != VOS_TRUE) {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)

VOS_VOID AT_InitDevCmdCtrl(VOS_VOID)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    (VOS_VOID)memset_s(atDevCmdCtrl, sizeof(AT_DEVICE_CmdCtrl), 0x00, sizeof(AT_DEVICE_CmdCtrl));
}
#else

VOS_VOID AT_InitMTInfo(VOS_VOID)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();
    (VOS_VOID)memset_s(atMtInfoCtx, sizeof(AT_MT_Info), 0x00, sizeof(AT_MT_Info));
    atMtInfoCtx->atInfo.rficTestResult = AT_RFIC_MEM_TEST_NOT_START;
    atMtInfoCtx->rserTestResult = (VOS_INT32)AT_SERDES_TEST_NOT_START;
    atMtInfoCtx->serdesTestAsyncInfo.result = (VOS_INT16)AT_SERDES_TEST_RST_NOT_START;
    atMtInfoCtx->serdesTestAsyncInfo.cmdType = 0;
    atMtInfoCtx->serdesTestAsyncInfo.rptFlg = 0;
    AT_SetUartTestState(AT_UART_TEST_NOT_START);
    AT_SetI2sTestState(AT_I2S_TEST_NOT_START);
}
#endif

VOS_VOID AT_InitDeviceCmd(VOS_VOID)
{
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    AT_InitDevCmdCtrl();
#else
    AT_InitMTInfo();
#endif

    if (AT_ReadPhyNV() != VOS_OK) {
        return;
    }

    /* IMEI为全零时，认为DATALOCK解锁状态 */
    g_atDataLocked = VOS_FALSE;
}

VOS_VOID AT_InitStk(VOS_VOID)
{
    USIMM_StkCfg profile;

    (VOS_VOID)memset_s(&profile, sizeof(profile), 0x00, sizeof(profile));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TERMINALPROFILE_SET, &profile, sizeof(USIMM_StkCfg)) != NV_OK) {
        AT_ERR_LOG("AT_StkInit: read NV_ITEM_TERMINALPROFILE_SET fail.");

        g_stkFunctionFlag = VOS_FALSE;
    } else {
        g_stkFunctionFlag = profile.funcEnable;
    }
}

VOS_VOID AT_InitPara(VOS_VOID)
{
    VOS_UINT8            *systemAppConfig       = VOS_NULL_PTR;
    const VOS_UINT8       atCmdNotSupportStr1[] = "ERROR";
    const VOS_UINT8       atCmdNotSupportStr2[] = "COMMAND NOT SUPPORT";
    errno_t               memResult;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* 根据产品类型分别进行处理 */
    switch (*systemAppConfig) {
        /* E5 */
        case SYSTEM_APP_WEBUI:
            /* E5版本对不支持命令返回ERROR */
            /* AT_NOTSUPPORT_STR_LEN + 4是g_atCmdNotSupportStr数组定义时的长度 */
            memResult = memcpy_s((VOS_CHAR *)g_atCmdNotSupportStr, AT_NOTSUPPORT_STR_LEN + 4,
                                 (VOS_CHAR *)atCmdNotSupportStr1, sizeof(atCmdNotSupportStr1));
            TAF_MEM_CHK_RTN_VAL(memResult, AT_NOTSUPPORT_STR_LEN + 4, sizeof(atCmdNotSupportStr1));

            /* E5错误码默认错误编号 */
            g_atCmeeType = AT_CMEE_ERROR_CODE;

            break;

            /* STICK */
        case SYSTEM_APP_MP:
            /* 命令不支持提示字串 */
            /* AT_NOTSUPPORT_STR_LEN + 4是g_atCmdNotSupportStr数组定义时的长度 */
            memResult = memcpy_s((VOS_CHAR *)g_atCmdNotSupportStr, AT_NOTSUPPORT_STR_LEN + 4,
                                 (VOS_CHAR *)atCmdNotSupportStr2, sizeof(atCmdNotSupportStr2));
            TAF_MEM_CHK_RTN_VAL(memResult, AT_NOTSUPPORT_STR_LEN + 4, sizeof(atCmdNotSupportStr2));

            /* 指示错误命令返回码类型 */
            g_atCmeeType = AT_CMEE_ERROR_CONTENT;

            break;

        /* PAD,目前没有，为保持与原代码一致性，暂按stick处理 */
        case SYSTEM_APP_ANDROID:

            /* 命令不支持提示字串 */
            /* AT_NOTSUPPORT_STR_LEN + 4是g_atCmdNotSupportStr数组定义时的长度 */
            memResult = memcpy_s((VOS_CHAR *)g_atCmdNotSupportStr, AT_NOTSUPPORT_STR_LEN + 4,
                                 (VOS_CHAR *)atCmdNotSupportStr2, sizeof(atCmdNotSupportStr2));
            TAF_MEM_CHK_RTN_VAL(memResult, AT_NOTSUPPORT_STR_LEN + 4, sizeof(atCmdNotSupportStr2));

            /* 指示错误命令返回码类型 */
            g_atCmeeType = AT_CMEE_ERROR_CODE;

            break;

        default:
            break;
    }
}

STATIC VOS_VOID AT_SubscripDmsEvent(VOS_VOID)
{
    DMS_PORT_SubscripEvent(ID_DMS_EVENT_LOW_WATER_MARK,  WUEPS_PID_AT);
    DMS_PORT_SubscripEvent(ID_DMS_EVENT_NCM_CONN_BREAK,  WUEPS_PID_AT);
    DMS_PORT_SubscripEvent(ID_DMS_EVENT_USB_DISCONNECT,  WUEPS_PID_AT);
    DMS_PORT_SubscripEvent(ID_DMS_EVENT_SWITCH_GW_MODE,  WUEPS_PID_AT);
    DMS_PORT_SubscripEvent(ID_DMS_EVENT_ESCAPE_SEQUENCE, WUEPS_PID_AT);
    DMS_PORT_SubscripEvent(ID_DMS_EVENT_DTR_DEASSERT,    WUEPS_PID_AT);
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    DMS_PORT_SubscripEvent(ID_DMS_EVENT_UART_INIT,    WUEPS_PID_AT);
#endif
}

#if (VOS_WIN32 == VOS_OS_VER)

STATIC VOS_VOID AT_DesubscripDmsEvent(VOS_VOID)
{
    DMS_PORT_DesubscripEvent(ID_DMS_EVENT_LOW_WATER_MARK,  WUEPS_PID_AT);
    DMS_PORT_DesubscripEvent(ID_DMS_EVENT_NCM_CONN_BREAK,  WUEPS_PID_AT);
    DMS_PORT_DesubscripEvent(ID_DMS_EVENT_USB_DISCONNECT,  WUEPS_PID_AT);
    DMS_PORT_DesubscripEvent(ID_DMS_EVENT_SWITCH_GW_MODE,  WUEPS_PID_AT);
    DMS_PORT_DesubscripEvent(ID_DMS_EVENT_ESCAPE_SEQUENCE, WUEPS_PID_AT);
    DMS_PORT_DesubscripEvent(ID_DMS_EVENT_DTR_DEASSERT,    WUEPS_PID_AT);
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    DMS_PORT_DesubscripEvent(ID_DMS_EVENT_UART_INIT,    WUEPS_PID_AT);
#endif
}
#endif

VOS_VOID AT_InitPort(VOS_VOID)
{
    /*
     * 为了保证PC回放工程和 NAS GTR PC工程中SDT USBCOM AT端口的注册client ID
     * 相同，必须让USB COM口第一个注册
     */
    /* USB PCUI口链路的建立 */
    At_UsbPcuiEst();

    /* USB Control口链路的建立 */
    At_UsbCtrEst();

    /* USB PCUI2口链路的建立 */
    At_UsbPcui2Est();

    /* UART口链路的建立 */
    AT_UartInitPort();

    /* NDIS MODEM口链路的建立 */
    AT_UsbNdisEst();

    /* APP 建立通道 */
    AT_AppComEst();

    AT_UsbNdisDataEst();

#if (FEATURE_AT_HSUART == FEATURE_ON)
    AT_HSUART_InitPort();
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    AT_CMUX_InitPort();
#endif

#if (FEATURE_AT_PROXY == FEATURE_ON)
    AT_InitAtpPort();
#endif
    /* NDIS MODEM口链路的建立 */
    AT_ModemEst();

    /* APP 建立通道 */
    AT_SockEst();

    DMS_PORT_Init();

    /* AT_UartInitLink 和 AT_HSUART_ConfigDefaultPara 必须放在DMS_PORT_Init函数之后，
     * 因为这两个函数内部有调用dms的接口，调用dms port相关接口需要在dms port init之后调用
     */
#if (FEATURE_AT_HSUART == FEATURE_ON)
#if (FEATURE_UART_BAUDRATE_AUTO_ADAPTION == FEATURE_ON)
    AT_UART_ConfigDefaultPara();
#endif
#endif

#if (FEATURE_AT_HSUART == FEATURE_ON)
    AT_HSUART_ConfigDefaultPara();
#endif

    AT_SubscripDmsEvent();

#if (VOS_WIN32 == VOS_OS_VER)
    AT_DesubscripDmsEvent();
#endif

}


LOCAL VOS_VOID AT_InitCommonWakeLock(VOS_VOID)
{
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    AT_InitWakeLock(&g_atWakeLock, "appds_wakelock");
#endif

#if (FEATURE_MT_CALL_SMS_WAKELOCK == FEATURE_ON)
    AT_InitWakeLock(&g_callWakeLock, "call wake lock");
    AT_InitWakeLock(&g_smsWakeLock, "sms wake lock");
#endif
#endif
}

LOCAL VOS_VOID AT_RegisterDeviceCmdTbl(VOS_VOID)
{
    AT_RegisterDeviceAsCmdTable();
    AT_RegisterDeviceDrvCmdTable();
    AT_RegisterDeviceMmCmdTable();
    AT_RegisterDevicePamCmdTable();
    AT_RegisterDevicePhyCmdTable();
    AT_RegisterDeviceTafCmdTable();
}

LOCAL VOS_VOID AT_RegisterCustomCmdTbl(VOS_VOID)
{
    AT_RegisterCustomAsCmdTable();
    AT_RegisterCustomCommCmdTable();
    AT_RegisterCustomDrvCmdTable();
    AT_RegisterCustomHifiCmdTable();
    AT_RegisterCustomImsCmdTable();
    AT_RegisterCustomL2CmdTable();
    AT_RegisterCustomLnasCmdTable();
    AT_RegisterCustomLrrcCmdTable();
    AT_RegisterCustomMmCmdTable();
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    AT_RegisterCustomNrmmCmdTable();
#endif
    AT_RegisterCustomPamCmdTable();
    AT_RegisterCustomPhyCmdTable();
#if (FEATURE_DCXO_HI1102_SAMPLE_SHARE == FEATURE_ON)
    AT_RegisterCustomRcmCmdTable();
#endif
    AT_RegisterCustomRrmCmdTable();
    AT_RegisterCustomTafCmdTable();
}

#if (FEATURE_LTEV == FEATURE_ON)
LOCAL VOS_VOID AT_RegisterLtevCmdTbl(VOS_VOID)
{
    AT_RegisterLtevAsCmdTable();
    AT_RegisterLtevMmCmdTable();
    AT_RegisterLtevTafCmdTable();
    AT_RegisterLtevTtfCmdTable();
}
#endif

LOCAL VOS_VOID AT_RegisterVoiceCmdTbl(VOS_VOID)
{
    AT_RegisterVoiceTafCmdTbl();
    AT_RegisterVoiceHifiCmdTbl();
    AT_RegisterVoiceImsCmdTbl();
    AT_RegisterVoiceMmCmdTbl();
}

LOCAL VOS_VOID AT_RegisterSsCmdTbl(VOS_VOID)
{
    AT_RegisterSsTafCmdTbl();
}

LOCAL VOS_VOID AT_RegisterSmsCmdTbl(VOS_VOID)
{
    AT_RegisterSmsTafCmdTbl();
}

LOCAL VOS_VOID AT_RegisterPhoneCmdTbl(VOS_VOID)
{
    AT_RegisterPhoneTafCmdTbl();
    AT_RegisterPhoneMmCmdTbl();
    AT_RegisterPhoneAsCmdTbl();
    AT_RegisterPhonePhyCmdTbl();
#if (FEATURE_IMS == FEATURE_ON)
    AT_RegisterPhoneImsCmdTbl();
#endif
}
LOCAL VOS_VOID AT_RegisterStubCmdTbl(VOS_VOID)
{
    AT_RegisterStubMmCmdTable();
    AT_RegisterStubTafCmdTable();
}

LOCAL VOS_VOID AT_RegisterDataCmdTbl(VOS_VOID)
{
    AT_RegisterDataDrvCmdTable();
#if (FEATURE_AT_HSUART == FEATURE_ON)
    AT_RegisterDataBasicCmdTable();
#endif
    AT_RegisterDataMmCmdTable();
    AT_RegisterDataTafCmdTable();
}

LOCAL VOS_VOID AT_RegisterGeneralCmdTbl(VOS_VOID)
{
    AT_RegisterGeneralDrvCmdTable();
    AT_RegisterGeneralMmCmdTable();
    AT_RegisterGeneralPamCmdTable();
    AT_RegisterGeneralTafCmdTable();
}

LOCAL VOS_VOID AT_RegisterSafetyCmdTbl(VOS_VOID)
{
    AT_RegisterSafetyMmCmdTable();
    AT_RegisterSafetyPamCmdTable();
}

LOCAL VOS_VOID AT_RegisterTestCmdTbl(VOS_VOID)
{
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    AT_RegisterTestTafCmdTbl();
    AT_RegisterTestAsCmdTbl();
    AT_RegisterTestTtfCmdTbl();
    AT_RegisterTestPhyCmdTbl();
#endif
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
LOCAL VOS_VOID AT_RegisterCdmaCmdTbl(VOS_VOID)
{
    AT_RegisterCdmaCmdTable();
}
#endif

/* 初始化消息处理表, 表内元素进行排序 */
STATIC VOS_VOID AT_InitMsgProcTbl(VOS_VOID)
{
    AT_InitMtaMsgProcTbl();
    AT_InitMmaMsgProcTbl();

#if (FEATURE_IMS == FEATURE_ON)
    AT_InitImsaMsgProcTbl();
#endif

    AT_InitCcmMsgProcTbl();

#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
    AT_InitThrotMsgProcTbl();
#endif

}

STATIC VOS_VOID At_RegisterAllKindOfCmdTables(VOS_VOID)
{
    /* 注册AT命令表 */
    At_RegisterBasicCmdTable();
    At_RegisterExCmdTable();

    /* 后续添加宏“FEATURE_ACORE_MODULE_TO_CCORE”，该宏的意义是：AT模块是否部署在C核；
       下面这个at命令表用这个宏控制，AT部署在C核时该AT命令表失效 */
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    At_RegisterAcoreOnlyCmdTable();
#endif

    AT_RegisterDeviceCmdTbl();
    AT_RegisterCustomCmdTbl();
#if (FEATURE_LTEV == FEATURE_ON)
    AT_RegisterLtevCmdTbl();
#endif
    AT_RegisterVoiceCmdTbl();
    AT_RegisterSsCmdTbl();
    AT_RegisterSmsCmdTbl();
    AT_RegisterPhoneCmdTbl();
    AT_RegisterSimPamCmdTbl();
    AT_RegisterSatPamCmdTbl();
    AT_RegisterStubCmdTbl();
    AT_RegisterDataCmdTbl();
    AT_RegisterGeneralCmdTbl();
    AT_RegisterTestCmdTbl();
    AT_RegisterSafetyCmdTbl();
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    AT_RegisterCdmaCmdTbl();
#endif
}

VOS_UINT32 At_PidInit(enum VOS_InitPhaseDefine phase)
{
    switch (phase) {
        case VOS_IP_INITIAL:

            /* 初始化AT的上下文 */
            AT_InitCtx();

            AT_InitCommonWakeLock();

#if (FEATURE_AT_HSUART == FEATURE_ON)
            /* 初始化UART相关的上下文 */
            AT_InitUartCtx();
#endif

            AT_InitPortBuffCfg();

            AT_InitTraceMsgTab();

            /* 初始化复位相关的上下文 */
            AT_InitResetCtx();

            /* 读取NV项 */
            AT_ReadNV();

            /* AT 解析器初始化 */
            At_ParseInit();

            /* 注册AT命令表 */
            At_RegisterAllKindOfCmdTables();

            AT_InitMsgProcTbl();

            /* 装备初始化 */
            AT_InitDeviceCmd();

            /* STK在AT模块的初始化 */
            AT_InitStk();
#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
            AT_ReadStkReportAppConfig();
#endif
#endif
            /* AT模块参数的初始化 */
            AT_InitPara();

            /* AT消息数量控制上下文初始化 */
            AT_InitMsgNumCtrlCtx();

            /* 端口初始化 */
            AT_InitPort();

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
            AT_RegResetCallbackFunc();
#endif
            AT_InitMntnCtx();
            AT_RegisterDumpCallBack();

            /* 下移C核后，此处的脱敏函数不注册 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
            /* 注册层间消息过滤函数 */
            GUNAS_OM_LayerMsgReplaceCBRegACore();
            TAF_OM_LayerMsgLogPrivacyMatchRegAcore();
#endif
            break;

        default:
            break;
    }

    return VOS_OK;
}

VOS_VOID AT_ReadSsNV(VOS_VOID)
{
    (VOS_VOID)memset_s(&g_atSsCustomizePara, sizeof(g_atSsCustomizePara), 0x00, sizeof(g_atSsCustomizePara));

    /* 读取SS定制的NV项到全局变量 */
    if ((TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SS_CUSTOMIZE_PARA, &g_atSsCustomizePara, sizeof(g_atSsCustomizePara)) !=
         NV_OK) ||
        (g_atSsCustomizePara.status != VOS_TRUE)) {
        g_atSsCustomizePara.status = VOS_FALSE;
    }
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_VOID AT_ReadUartCfgNV(VOS_VOID)
{
    AT_UartCtx    *uartCtx = VOS_NULL_PTR;
    TAF_NV_UartCfg uartNVCfg;
    VOS_UINT32     ret;

    (VOS_VOID)memset_s(&uartNVCfg, sizeof(uartNVCfg), 0x00, sizeof(TAF_NV_UartCfg));

    uartCtx = AT_GetUartCtxAddr();

    /* 读取NV项 */
    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_UART_CFG, &uartNVCfg, sizeof(TAF_NV_UartCfg));

    if (ret == VOS_OK) {
        /* 检查NV中设置的波特率是否在支持的范围内 */
        ret = AT_HSUART_IsBaudRateValid(uartNVCfg.baudRate);
        if (ret == VOS_TRUE) {
            /* 将NV中的值赋给上下文全局变量 */
            uartCtx->phyConfig.baudRate = uartNVCfg.baudRate;
        }

        /* 检查NV中设置的帧格式是否在支持的范围内 */
        ret = AT_HSUART_ValidateCharFrameParam(uartNVCfg.frame.format, uartNVCfg.frame.parity);
        if (ret == VOS_TRUE) {
            uartCtx->phyConfig.frame.format = uartNVCfg.frame.format;
            uartCtx->phyConfig.frame.parity = uartNVCfg.frame.parity;
        }

        uartCtx->riConfig.smsRiOnInterval    = uartNVCfg.riConfig.smsRiOnInterval;
        uartCtx->riConfig.smsRiOffInterval   = uartNVCfg.riConfig.smsRiOffInterval;
        uartCtx->riConfig.voiceRiOnInterval  = uartNVCfg.riConfig.voiceRiOnInterval;
        uartCtx->riConfig.voiceRiOffInterval = uartNVCfg.riConfig.voiceRiOffInterval;
        uartCtx->riConfig.voiceRiCycleTimes  = uartNVCfg.riConfig.voiceRiCycleTimes;
    }
}
#endif

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID AT_ReadCdmaModemSwitchNotResetCfgNv(VOS_VOID)
{
    AT_ModemCdmamodemswitchCtx         *cdmaModemSwitchCtx = VOS_NULL_PTR;
    TAF_NVIM_CdmamodemswitchNotResetCfg cdmaModemSwitchNvCfg;
    ModemIdUint16                       modemId;

    /* 初始化 */
    (VOS_VOID)memset_s(&cdmaModemSwitchNvCfg, (VOS_UINT32)sizeof(TAF_NVIM_CdmamodemswitchNotResetCfg), 0x00,
                       (VOS_UINT32)sizeof(TAF_NVIM_CdmamodemswitchNotResetCfg));

    /* 读取NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CDMAMODEMSWITCH_NOT_RESET_CFG, &cdmaModemSwitchNvCfg,
                          (VOS_UINT32)sizeof(TAF_NVIM_CdmamodemswitchNotResetCfg)) != NV_OK) {
        cdmaModemSwitchNvCfg.enableFlg = VOS_FALSE;
    }

    if (cdmaModemSwitchNvCfg.enableFlg != VOS_TRUE) {
        cdmaModemSwitchNvCfg.enableFlg = VOS_FALSE;
    }

    for (modemId = 0; modemId < MODEM_ID_BUTT; modemId++) {
        cdmaModemSwitchCtx = AT_GetModemCdmaModemSwitchCtxAddrFromModemId(modemId);

        cdmaModemSwitchCtx->enableFlg = cdmaModemSwitchNvCfg.enableFlg;
    }
}
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
VOS_VOID AT_ReadStkReportAppConfig(VOS_VOID)
{
    NV_STK_CmdReportAppCfg config;
    VOS_UINT32 i;

    for (i = MODEM_ID_0; i < MODEM_ID_BUTT; i++) {
        (VOS_VOID)memset_s(&config, sizeof(config), 0x00, sizeof(config));

        if (TAF_ACORE_NV_READ(i, NV_ITEM_STK_CMD_REPORT_APP_CFG, &config, sizeof(NV_STK_CmdReportAppCfg)) != NV_OK) {
            AT_ERR_LOG("AT_StkInit: read NV_ITEM_STK_CMD_REPORT_APP_CFG fail.");
            AT_SetStkReportAppConfigFlag((ModemIdUint16)i, VOS_FALSE);
            continue;
        }

        if (config.value != 0) {
            AT_SetStkReportAppConfigFlag((ModemIdUint16)i, VOS_TRUE);
        } else {
            AT_SetStkReportAppConfigFlag((ModemIdUint16)i, VOS_FALSE);
        }
    }
}
#endif
#endif

