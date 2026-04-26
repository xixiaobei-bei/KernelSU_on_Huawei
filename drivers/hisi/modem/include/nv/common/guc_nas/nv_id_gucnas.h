/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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



#ifndef __NV_ID_GUCNAS_H__
#define __NV_ID_GUCNAS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/****************************************************************************
  1 其他头文件包含
*****************************************************************************/
/*****************************************************************************
 枚举名    : GUCNAS_NV_ID_ENUM
 枚举说明  : GUCNAS组件公共的NV项ID枚举定义
*****************************************************************************/
enum GUCNAS_NV_ID_ENUM
{
/*     6 */     NV_ITEM_SERIAL_NUM                       = 6,        /* TAF_NVIM_SerialNum */
/*    21 */     NV_ITEM_USB_ENUM_STATUS                  = 21,       /* USB_ENUM_Status */
/*    36 */     NV_ITEM_SW_VERSION_FLAG                  = 36,       /* OM_SW_VerFlag */
/*    60 */     NV_ITEM_SVLTE_FLAG                       = 60,       /* SVLTE_SupportFlag */
/*    70 */     NV_ITEM_LC_CTRL_PARA                     = 70,       /* TAF_NV_LC_CTRL_PARA_STRU */
/*    90 */     NV_ITEM_BATTERY_ADC                      = 90,       /* VBAT_CALIBART_Type */
/*   121 */     NV_ITEM_SYSTEM_APP_CONFIG                = 121,      /* NAS_NVIM_SystemAppConfig */

/*  2003 */     NV_ITEM_CFREQ_LOCK_CFG                    = 2003,     /* TAF_NVIM_CfreqLockCfg */
/*  2095 */     NV_ITEM_LTE_SMS_CFG                      = 2095,     /* TAF_NVIM_LteSmsCfg */
/*  2102 */     NV_ITEM_CDMAMODEMSWITCH_NOT_RESET_CFG    = 2102,     /* TAF_NVIM_CdmamodemswitchNotResetCfg  用来配置CDMAMODEMSWITCH不重启特性是否开启，若不开启，设置后需RESET MODEM */

/*  2120 */     NV_ITEM_AT_CMD_TRUST_LIST_CFG            = 2120,     /* TAF_NVIM_AtCmdTrustListCfg */
/*  2123 */     NV_ITEM_WAKEUP_CFG                       = 2123,     /* TAF_NVIM_MbbWakeupCfg */
/*  2124 */     NV_ITEM_CMIC_VOLUME_CFG                  = 2124,     /* TAF_NVIM_MbbCmicVolumeCfg */

/*  2300 */     NV_ITEM_DSDS_ACTIVE_MODEM_MODE           = 2300,     /* TAF_NV_DsdsActiveModemMode */
/*  2316 */     NV_ITEM_ENHANCED_OPERATOR_NAME_SRV_CFG   = 2316,     /* TAF_NVIM_EnhancedOperNameServiceCfg */
/*  2321 */     NV_ITEM_NVWR_SEC_CTRL                    = 2321,     /* TAF_NV_NvwrSecCtrl    NVWR命令安全控制 */
/*  2331 */     NV_ITEM_CCPU_RESET_RECORD                = 2331,     /* TAF_NVIM_CcpuResetRecord  C核单独复位记录 */
/*  2336 */     NV_ITEM_IPV6_ADDRESS_TEST_MODE_CFG       = 2336,     /* TAF_NVIM_Ipv6AddrTestModeCfg        IPV6地址测试模式配置，0x55AA55AA为测试模式，其他值为正常模式 */

/*  2340 */     NV_ITEM_CCWA_CTRL_MODE                   = 2340,     /* TAF_CALL_NvimCcwaCtrlMode */
/*  2352 */     NV_ITEM_PRIVACY_LOG_FILTER_CFG           = 2352,     /* NAS_NV_PrivacyFilterCfg */
/*  2361 */     NV_ITEM_PRINT_MODEM_LOG_TYPE             = 2361,     /* TAF_NV_PrintModemLogType             控制是否输出modem log的类型 */
/*  2371 */     NV_ITEM_XCPOSRRPT_CFG                    = 2371,     /* TAF_NVIM_XcposrrptCfg 是否上报清除GPS缓存的辅助定位信息 */
/*  2387 */     NV_ITEM_USER_SYS_CFG_RAT_INFO            = 2387,     /* TAF_NVIM_MultimodeRatCfg 用户SYS CFG RAT配置 */
/*  2391 */     NV_ITEM_USER_REBOOT_SUPPORT_FLG          = 2391,     /* TAF_NVIM_UserRebootSupport            用户是否可以发起整机复位 */
/*  2396 */     NV_ITEM_GPS_CUST_CFG                     = 2396,     /* TAF_NVIM_GpsCustCfg 查询GPS芯片类型 */

/*  2437 */     NV_ITEM_AT_CLIENT_CFG                    = 2437,     /* TAF_AT_NvimAtClientCfg               双卡双通控制AT通道归属哪个Modem信息 */
/*  2478 */     NV_ITEM_IMS_SWITCH_RAT_RELATION_CFG      = 2478,     /* TAF_NV_ImsSwitchRatRelationCfg      ^IMSSWITCH命令2/3G开关与4G开关关联配置 */

/* 2512 */      NV_ITEM_RAT_PRIO_LIST_EX                 = 2512,     /* TAF_NVIM_RatOrderEx */

/*  2525 */     NV_ITEM_CUSTOM_CALL_CFG                  = 2525,     /* NAS_NVIM_CustomCallCfg */

/*  2526 */     NV_ITEM_CUSTOM_USIMM_CFG                 = 2526,     /* NAS_NVIM_CustomUsimmCfg */

/*  2553 */     NV_ITEM_CUSTOM_ECALL_CFG                 = 2553,     /* TAF_NVIM_CustomEcallCfg */

/*  2557 */     NV_ITEM_APN_CUSTOM_FORMAT_CFG            = 2557,     /* TAF_NVIM_ApnCustomFormatCfg */
/*  2585 */     NV_ITEM_USIM_DEPENDENT_RESULT_INFO       = 2585,     /* TAF_NVIM_UsimDependentResultInfo */
/*  2589 */     NV_ITEM_MBB_FEATURE_CFG                  = 2589,     /* TAF_NVIM_MbbFeatureCfg */

/*  2600 */     NV_ITEM_COMMDEGBUG_CFG                   = 2600,     /* TAF_AT_NvimCommdegbugCfg */
/*  2601 */     NV_ITEM_PID_ENABLE_TYPE                  = 2601,     /* NV_PID_EnableType */
/*  2602 */     NV_ITEM_AT_DISSD_PWD                     = 2602,     /* TAF_AT_NvimDissdPwd */
/*  2603 */     NV_ITEM_AT_DISLOG_PWD_NEW                = 2603,     /* TAF_AT_NvimDislogPwdNew */
/*  2604 */     NV_ITEM_DEFAULT_LINK_OF_UART             = 2604,     /* TAF_AT_NvimDefaultLinkOfUart */
/*  2605 */     NV_ITEM_AT_FACTORY_INFO                  = 2605,     /* TAF_AT_NvimFactoryInfo */
/*  2606 */     NV_ITEM_AT_MANUFACTURE_DATE              = 2606,     /* TAF_AT_NvimManufactureDate */
/*  2607 */     NV_ITEM_UART_CFG                         = 2607,     /* TAF_NV_UartCfg */
/*  2608 */     NV_ITEM_PORT_BUFF_CFG                    = 2608,     /* TAF_NV_PortBuffCfg */

/*  2611 */     NV_ITEM_PRODUCT_NAME                      = 2611,     /* TAF_PH_ProductName */
/*  2615 */     NV_ITEM_MMA_ACCESS_MODE                   = 2615,     /* NAS_MMA_NvimAccessMode */
/*  2626 */     NV_ITEM_WINS_CONFIG                      = 2626,     /* WINS_Config */
/*  2627 */     NV_ITEM_RXDIV_CONFIG                     = 2627,     /* TAF_AT_NvimRxdivConfig */

/*  2635 */     NV_ITEM_NDIS_DHCP_DEF_LEASE_TIME         = 2635,     /* NDIS_NV_DhcpLeaseHour */
/*  2636 */     NV_ITEM_CREG_CGREG_CI_FOUR_BYTE_RPT      = 2636,     /* NAS_NV_CregCgregCiFourByteRpt */
/*  2640 */     NV_ITEM_ATSETZ_RET_VALUE                 = 2640,     /* TAF_AT_Tz */
/*  2641 */     NV_ITEM_NOT_SUPPORT_RET_VALUE            = 2641,     /* TAF_AT_NotSupportCmdErrorText */
/*  2642 */     NV_ITEM_AT_RIGHT_PASSWORD                = 2642,     /* TAF_AT_NvimRightOpenFlag */

/*  2644 */     NV_ITEM_PREVENT_TEST_IMSI_REG            = 2644,     /* NAS_PREVENT_TestImsiReg */
/*  2649 */     NV_ITEM_IPV6_ROUTER_MTU                  = 2649,     /* TAF_NDIS_NvIpv6RouterMtu */
/*  2650 */     NV_ITEM_DIAL_CONNECT_DISPLAY_RATE        = 2650,     /* AT_NVIM_DialConnectDisplayRate */
/*  2656 */     NV_ITEM_WIFI_INFO                        = 2656,     /* AT_WIFI_Info， XML文件中的结构体名为nv_wifi_Info */
/*  2675 */     NV_ITEM_MUX_SUPPORT_FLG                  = 2675,     /* TAF_AT_NvimMuxSupportFlg */
/*  2677 */     NV_ITEM_MUX_REPORT_CFG                   = 2677,     /* TAF_AT_NvimMuxReportCfg */
/*  2679 */     NV_ITEM_SS_CUSTOMIZE_PARA                = 2679,     /* AT_SS_CustomizePara */
/*  2680 */     NV_ITEM_SHARE_PDP_INFO                   = 2680,     /* TAF_NVIM_SharePdpInfo Share PDP特性控制NV */

/*  2695 */     NV_ITEM_JAM_DETECT_CFG                   = 2695,     /* NV_NAS_JamDetectCfg */
/*  2698 */     NV_ITEM_FREQ_LOCK_CFG                    = 2698,     /* TAF_NVIM_FreqLockCfg */
/*  2705 */     NV_ITEM_PRODUCT_ID                       = 2705,     /* TAF_AT_ProductId */
/*  2707 */     NV_ITEM_CUST_USSD_MODE                   = 2707,     /* TAF_UssdNvimTransMode */
/*  2713 */     NV_ITEM_AP_RPT_SRV_URL                   = 2713,     /* TAF_AT_NvimApRptSrvUrl */
/*  2714 */     NV_ITEM_AP_XML_INFO_TYPE                 = 2714,     /* TAF_AT_NvimApXmlInfoType */
/*  2715 */     NV_ITEM_AP_XML_RPT_FLAG                  = 2715,     /* TAF_AT_NvimApXmlRptFlg */

/*  6656 */     NV_ITEM_FOLLOW_ON_OPEN_SPEED_FLAG          = 6656,     /* NAS_NVIM_FollowonOpenspeedFlag */

/*  8200 */     NV_ITEM_GMM_INFO                          = 8200,     /* TAF_AT_NvGmmInfo */
/*  8203 */     NV_ITEM_FMRID                            = 8203,     /* TAF_PH_FmrId */
/*  8230 */     NV_ITEM_CCA_TELE_PARA                     = 8230,     /* TAF_CCA_TelePara */
/*  8233 */     NV_ITEM_MMA_MS_CLASS                      = 8233,     /* NAS_NVIM_MsClass */
/*  8237 */     NV_ITEM_SMS_SERVICE_PARA                 = 8237,     /* TAF_NVIM_SmsServicePara */
/*  8266 */     NV_ITEM_ROAM_CAPA                        = 8266,     /* NAS_NVIM_RoamCfgInfo */
/*  8267 */     NV_ITEM_CUSTOMIZE_SIM_LOCKPLMN_INFO         = 8267,     /* TAF_CUSTOM_SimLockPlmnInfo */
/*  8268 */     NV_ITEM_CARDLOCK_STATUS                   = 8268,     /* TAF_CUSTOM_CARDLOCK_STATUS_STRU */
/*  8269 */     NV_ITEM_CUSTOMIZE_SIM_LOCK_MAX_TIMES         = 8269,     /* TAF_CUSTOM_SimLockMaxTimes */
/*  8271 */     NV_ITEM_CUSTOMIZE_SERVICE                 = 8271,     /* NAS_NVIM_CustomizeService */
/*  8337 */     NV_ITEM_IMEI_SVN                         = 8337,     /* TAF_SVN_Data */
/*  8340 */     NV_ITEM_REPORT_REG_ACT_FLG               = 8340,     /* TAF_AT_NvimReportRegActFlg */

/*  8471 */     NV_ITEM_PCVOICE_SUPPORT_FLG              = 8471,     /* APP_VC_NvimPcVoiceSupportFalg */
/*  8501 */     NV_ITEM_SMS_ME_STORAGE_INFO              = 8501,     /* MN_MSG_MeStorageParm */
/*  8514 */     NV_ITEM_IPV6_CAPABILITY                  = 8514,     /* AT_NV_Ipv6Capability */

/*  9007 */     NV_ITEM_RAT_PRIO_LIST                    = 9007,     /* TAF_PH_NvimRatOrder */
/*  9041 */     NV_ITEM_SUB_PLATFORM_INFO                  = 9041,     /* NAS_NVIM_Platform */

/*  9052 */     NV_ITEM_LTE_INTERNATION_ROAM_CONFIG      = 9052,     /* NAS_MMC_NvimLteInternationalRoamCfg */
/*  9070 */     NV_ITEM_EQVER                            = 9070,     /* TAF_AT_EqVer */
/*  9071 */     NV_ITEM_CSVER                            = 9071,     /* TAF_NVIM_CsVer */

/*  9110 */     NV_ITEM_MULTI_WIFI_KEY                   = 9110,     /* TAF_AT_MultiWifiSec 支持多组SSID扩展新增WIKEY NV项 */
/*  9111 */     NV_ITEM_MULTI_WIFI_STATUS_SSID           = 9111,     /* TAF_AT_MultiWifiSsid 支持多组SSID扩展新增SSID NV项 */
/*  9113 */     NV_ITEM_AT_ABORT_CMD_PARA                = 9113,     /* AT_NVIM_AbortCmdPara */

/*  9118 */     NV_ITEM_SMS_MT_CUSTOMIZE_INFO            = 9118,     /* MN_MSG_MtCustomizeInfo */
/*  9123 */     NV_ITEM_CELL_SIGN_REPORT_CFG             = 9123,     /* NAS_NVIM_CellSignReportCfg */
/*  9128 */     NV_ITEM_CIMI_PORT_CFG                    = 9128,     /* TAF_AT_NvimCimiPortCfg */

/*  9130 */     NV_ITEM_IPV6_BACKPROC_EXT_CAUSE          = 9130,     /* TAF_NV_Ipv6FallbackExtCause */
/*  9136 */     NV_ITEM_SMS_CLOSE_PATH                   = 9136,     /* TAF_NVIM_SmsClosePathCfg */
/*  9203 */     NV_ITEM_PLATFORM_RAT_CAP                 = 9203,     /* PLATAFORM_RatCapability */
/*  9206 */     NV_ITEM_BODY_SAR_PARA                    = 9208,     /* MTA_BodySarPara */
/*  9212 */     NV_ITEM_TEMP_PROTECT_CONFIG              = 9212,     /* TAF_TempProtectConfig */
/*  9237 */     NV_ITEM_SMS_DOMAIN                       = 9237,     /* TAF_NVIM_SmsDomain */
/*  9269 */     NV_ITEM_DMS_DEBUG_CFG                    = 9269,     /* TAF_NV_PortDebugCfg */
/* 32768 */     NV_ITEM_MANUFACTURE_INFO                  = 32768,    /* OM_ManufactureInfo */

/* 50009 */     NV_ITEM_TRAFFIC_CLASS_TYPE               = 50009,    /* AT_TrafficClassCustomize */
/* 50014 */     NV_ITEM_WIFI_MAC_ADDR                    = 50014,    /* OM_MAC_Addr */
/* 50056 */     NV_ITEM_SMS_CLASS0_TAILOR                = 50056,    /* MN_MSG_NvimClass0Tailor */

/* 50061 */     NV_ITEM_PPP_DIAL_ERR_CODE                = 50061,    /* NAS_NV_PppDialErrCode */
/* 50091 */     NV_ITEM_HUAWEI_DYNAMIC_PID_TYPE          = 50091,    /* AT_DynamicPidType */
/* 50198 */     NV_ITEM_SYSCFGEX_RAT_GROUP_CTRL          = 50198,    /* MBB定制NV ^SYSCFGEX命令返回值格式定制 */
/* 50360 */     NV_ITEM_MT_SMS_NUM_TRUST_LIST            = 50360,    /* TAF_NVIM_MtSmsNumTrustList */
/* 50466 */     NV_ITEM_APP_SMS_FULL_FLG                 = 50466,    /* TAF_NVIM_NvAppSmsFullFlag */
/* 50550 */     NV_ITEM_MT_CALL_NUM_TRUST_LIST           = 50550,    /* TAF_NVIM_MtCallNumTrustList */
/* 50559 */     NV_ITEM_MT_LIST_AND_EID_CTRL             = 50559,    /* TAF_NVIM_MtListAndEidCtrl */
/* 50578 */     NV_ITEM_LTE_ATTACH_PROFILE_0             = 50578,    /* TAF_NVIM_LteAttachProfileInfo */
/* 50579 */     NV_ITEM_LTE_ATTACH_PROFILE_1             = 50579,    /* TAF_NVIM_LteAttachProfileInfo */
/* 50580 */     NV_ITEM_LTE_ATTACH_PROFILE_2             = 50580,    /* TAF_NVIM_LteAttachProfileInfo */
/* 50581 */     NV_ITEM_LTE_ATTACH_PROFILE_3             = 50581,    /* TAF_NVIM_LteAttachProfileInfo */
/* 50582 */     NV_ITEM_LTE_ATTACH_PROFILE_4             = 50582,    /* TAF_NVIM_LteAttachProfileInfo */
/* 50583 */     NV_ITEM_LTE_ATTACH_PROFILE_5             = 50583,    /* TAF_NVIM_LteAttachProfileInfo */
/* 50653 */     NV_ITEM_PROFILE_MATCH_CFG                = 50653,    /* TAF_NVIM_NvProfileMatchCfg */
/* 50663 */     NV_ITEM_VOIP_APN_KEY                     = 50663,    /* TAF_NVIM_ApnKeyList */
/* 50667 */     NV_ITEM_ICCID_TRANS_MODE                 = 50667,    /* ICCID查询结果的传输模式 */
/* 50670 */     NV_ITEM_AUTO_MODE_RAT_LIST               = 50670,    /* SYSCFGEX AUTO模式对应的制式列表 */
/* 52001 */     NV_ITEM_WEB_ADMIN_PASSWORD               = 52001,    /* TAF_AT_NvimWebAdminPassword */

/* 53252 */     NV_ITEM_TAF_LTE_RSRP_CFG                 = 53252,    /* TAF_NVIM_LteRsrpCfg */
/* 53253 */     NV_ITEM_TAF_LTE_ECIO_CFG                 = 53253,    /* TAF_NVIM_LteEcioCfg */
/* 53254 */     NV_ITEM_TAF_LTE_RSCP_CFG                 = 53254,    /* TAF_NVIM_LteRscpCfg */
/* 53257 */     NV_ITEM_AUTHORITY_VER                    = 53257,    /* TAF_NVIM_AuthVer */

/*  BUTT */     NV_ITEM_GUCNAS_NV_BUTT,
};


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __NV_ID_GUCNAS_H__ */

