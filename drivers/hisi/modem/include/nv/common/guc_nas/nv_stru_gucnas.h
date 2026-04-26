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

#ifndef __NV_STRU_GUCNAS_H__
#define __NV_STRU_GUCNAS_H__

#if (defined(MODEM_FUSION_VERSION) && defined(LPS_RTT))
#include "v_basic_type_def.h"
#else
#include "vos.h"
#endif
#include "nv_id_gucnas.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAF_NVIM_SN_LEN 20
#define TAF_NVIM_MAX_OPER_NAME_SERVICE_PRIO_NUM 4
#define TAF_NV_BLOCK_LIST_MAX_NUM 51
#define TAF_NVIM_MAX_USER_SYS_CFG_RAT_NUM 7
#define TAF_MAX_MFR_ID_LEN 31
#define TAF_MAX_MFR_ID_STR_LEN (TAF_MAX_MFR_ID_LEN + 1)
#define TAF_PH_PRODUCT_NAME_LEN 15
#define TAF_PH_PRODUCT_NAME_STR_LEN (TAF_PH_PRODUCT_NAME_LEN + 1)
#define MN_MSG_SRV_PARAM_LEN 8 /* 短信业务参数结构与NV项中存储的长度 */
#define TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN 160
#define TAF_SVN_DATA_LENGTH 2                     /* SVN有效数据长度 */
#define TAF_PH_NVIM_MAX_GUL_RAT_NUM 3             /* AT^syscfgex中acqorder代表的接入技术个数 */
#define TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM 20 /* 9130扩展IPv6回退处理扩展原因值最大个数 */
#define PLATFORM_MAX_RAT_NUM (7U)                   /* 接入技术最大值 */
#define MTA_BODY_SAR_WBAND_MAX_NUM 5
#define MTA_BODY_SAR_GBAND_MAX_NUM 4
#define TAF_PH_SIMLOCK_PLMN_STR_LEN 8 /* Plmn 号段长度 */
#define TAF_MAX_SIM_LOCK_RANGE_NUM 20

#define TAF_NVIM_MAX_RAT_ORDER_NUM 8
/* WINS可配置NV项的结构体 */
#define WINS_CONFIG_DISABLE 0 /* WINS不使能 */
#define WINS_CONFIG_ENABLE  1  /* WINS使能 */

#define TAF_NVIM_APN_CUSTOM_CHAR_MAX_NUM 16

#define AT_MDATE_STRING_LENGTH 20
#define TAF_AT_NVIM_CLIENT_CFG_LEN 128
#define AT_DISSD_PWD_LEN 16
#define AT_OPWORD_PWD_LEN 16
#define AT_FACINFO_INFO1_LENGTH 128
#define AT_FACINFO_INFO2_LENGTH 128
#define AT_FACINFO_STRING_LENGTH ((AT_FACINFO_INFO1_LENGTH + 1) + (AT_FACINFO_INFO2_LENGTH + 1))
#define AT_FACINFO_INFO1_STR_LENGTH (AT_FACINFO_INFO1_LENGTH + 1)
#define AT_FACINFO_INFO2_STR_LENGTH (AT_FACINFO_INFO2_LENGTH + 1)
#define AT_NVIM_SETZ_LEN 16
#define AT_NOTSUPPORT_STR_LEN 16
#define AT_NVIM_RIGHT_PWD_LEN 16

#define AT_WIFI_SSID_LEN_MAX 33          /* WIFI SSID KEY最大长度 */
#define AT_WIFI_KEY_LEN_MAX 27
#define AT_WIFI_WLAUTHMODE_LEN 16     /* 鉴权模式字符串长度 */
#define AT_WIFI_ENCRYPTIONMODES_LEN 5 /* 加密模式字符串长度 */
#define AT_WIFI_WLWPAPSK_LEN 65       /* WPA的密码字符串长度 */
#define AT_WIFI_MAX_SSID_NUM 4        /* 最多支持4组SSID */
#define AT_WIFI_KEY_NUM (AT_WIFI_MAX_SSID_NUM)
#define AT_MAX_ABORT_CMD_STR_LEN 16
#define AT_MAX_ABORT_RSP_STR_LEN 16
#define AT_PRODUCT_NAME_MAX_NUM 29
#define AT_PRODUCT_NAME_LENGHT (AT_PRODUCT_NAME_MAX_NUM + 1)
#define AT_WEBUI_PWD_MAX 16 /* WEB UI 密码最大长度 */
#define AT_AP_XML_RPT_SRV_URL_LEN 127
#define AT_AP_XML_RPT_SRV_URL_STR_LEN (AT_AP_XML_RPT_SRV_URL_LEN + 1)
#define AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN 127
#define AT_AP_NVIM_XML_RPT_INFO_TYPE_STR_LEN (AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN + 1)
#define TAF_NVIM_USIM_DEPEND_OPER_NAME_MAX_LEN     48  /* 定制运营商名称最大长度 */

#define TAF_TRUSTLIST_NUM_MAX_LEN 26 /* 可信名单号码最大长度 */
#define TAF_TRUSTLIST_MAX_LIST_SIZE 20 /* 可信名单列表最大条数 */

#define MAX_ENCRPT_APN_KEY_LENGTH 112
#define AT_MAX_APN_KEY_LENGTH 96
#define IV_LEN_VOIP 16

#define RNIC_NVIM_NAPI_LB_MAX_LEVEL 16 /* 软中断负载均衡最大档位 */
#define RNIC_NVIM_NAPI_LB_MAX_CPUS 16  /* 软中断负载均衡最大CPU个数 */
#define RNIC_NVIM_MAX_CLUSTER 3        /* CPU最大CLUSTER个数 */
#define RNIC_NVIM_RHC_MAX_LEVEL 16     /* 高速控制最大档位 */

#define TAF_NVIM_MAX_APNRETRY_BCD_IMSI_LEN 5
#define TAF_NVIM_MAX_APNRETRY_APN_LEN 32
#define TAF_NVIM_MAX_APNRETRY_USERNAME_LEN 32
#define TAF_NVIM_MAX_APNRETRY_PASSWORD_LEN 32
#define TAF_NVIM_MAX_APNRETRY_PRONAME_LEN 19

#define TAF_NVIM_RSRP_CFG_LEVEL_LEN 4
#define TAF_NVIM_RSRP_CFG_VALUE_LEN 4
#define TAF_NVIM_ECIO_CFG_LEVEL_LEN 4
#define TAF_NVIM_ECIO_CFG_VALUE_LEN 4
#define TAF_NVIM_RSCP_CFG_LEVEL_LEN 4
#define TAF_NVIM_RSCP_CFG_VALUE_LEN 4

#define TAF_MAX_SKU_PROFILE_INFO_NUM 5

#if (FEATURE_MBB_CUST == FEATURE_ON)
#define TAF_NVIM_SYSCFGEX_RAT_GROUP_MAX 10
#define TAF_NVIM_SYSCFGEX_RAT_NUM_MAX 6
#define TAF_NVIM_AUTO_MODE_RAT_MAX 7   /* 自动模式下接入制式最大数量 */
#endif

typedef VOS_UINT32 MMA_QuickStartStaUint32;
/* 对NVID枚举的转定义(PS_NV_ID_ENUM, SYS_NV_ID_ENUM, RF_NV_ID_ENUM) */
typedef VOS_UINT16 NV_IdU16;

/* 结构说明: 上层对接应用类型枚举 */
enum SYSTEM_AppConfigType {
    SYSTEM_APP_MP = 0,  /* 上层应用是Mobile Partner */
    SYSTEM_APP_WEBUI,   /* 上层应用是Web UI */
    SYSTEM_APP_ANDROID, /* 上层应用是Android */
    SYSTEM_APP_BUTT     /* 无效值 */
};
typedef VOS_UINT16 SYSTEM_AppConfigTypeUint16;


enum TAF_NVIM_LcRatCombined {
    TAF_NVIM_LC_RAT_COMBINED_GUL = 0x55, /* 互操作方案GUL */
    TAF_NVIM_LC_RAT_COMBINED_CL  = 0xAA, /* 互操作方案CL */
    TAF_NVIM_LC_RAT_COMBINED_BUTT        /* 无效值 */
};
typedef VOS_UINT8 TAF_NVIM_LcRatCombinedUint8;


enum TAF_NVIM_LcWorkCfg {
    TAF_NVIM_LC_INDEPENT_WORK = 0, /* 独立方案 */
    TAF_NVIM_LC_INTER_WORK    = 1, /* 互通方案 */
    TAF_NVIM_LC_WORK_CFG_BUTT      /* 无效值 */
};
typedef VOS_UINT8 TAF_NVIM_LcWorkCfgUint8;

/* 枚举说明: */
enum TAF_MMA_CfreqLockModeType {
    TAF_MMA_CFREQ_LOCK_MODE_OFF  = 0x00, /* 禁止锁频功能 */
    TAF_MMA_CFREQ_LOCK_MODE_ON   = 0x01, /* 启动锁频功能 */
    TAF_MMA_CFREQ_LOCK_MODE_BUTT = 0x02  /* 无效值 */
};
typedef VOS_UINT8 TAF_MMA_CfreqLockModeTypeUint8;

/* 实际未使用 */

enum TAF_LSMS_ResendFlag {
    TAF_LSMS_RESEND_FLAG_DISABLE = 0, /* 禁止重发 */
    TAF_LSMS_RESEND_FLAG_ENABLE  = 1, /* 使能重发 */

    TAF_LSMS_RESEND_FLAG_BUTT         /* 无效值 */
};
typedef VOS_UINT8 TAF_LSMS_ResendFlagUint8;


enum TAF_NV_ActiveModemMode {
    TAF_NV_ACTIVE_SINGLE_MODEM = 0x00, /* 单Modem模式 */
    TAF_NV_ACTIVE_MULTI_MODEM  = 0x01, /* 多Modem模式 */

    TAF_NV_ACTIVE_MODEM_MODE_BUTT      /* 无效值 */
};
typedef VOS_UINT8 TAF_NV_ActiveModemModeUint8;


enum TAF_NV_GpsChipType {
    TAF_NV_GPS_CHIP_BROADCOM = 0, /* Broadcom类型 */
    TAF_NV_GPS_CHIP_HISI1102 = 1, /* Hisi1102类型 */

    TAF_NV_GPS_CHIP_BUTT          /* 无效值 */
};
typedef VOS_UINT8 TAF_NV_GpsChipTypeUint8;

/* ME Storage Function On or Off */
enum MN_MSG_MeStorageStatus {
    MN_MSG_ME_STORAGE_DISABLE = 0x00, /* ME存储短信功能关闭 */
    MN_MSG_ME_STORAGE_ENABLE  = 0x01, /* ME存储短信功能开启 */
    MN_MSG_ME_STORAGE_BUTT            /* 无效值 */
};
typedef VOS_UINT8 MN_MSG_MeStorageStatusUint8;


enum PLATFORM_RatType {
    PLATFORM_RAT_GSM,   /* GSM接入技术 */
    PLATFORM_RAT_WCDMA, /* WCDMA接入技术 */
    PLATFORM_RAT_LTE,   /* LTE接入技术 */
    PLATFORM_RAT_TDS,   /* TDS接入技术 */
    PLATFORM_RAT_1X,    /* CDMA-1X接入技术 */
    PLATFORM_RAT_HRPD,  /* CDMA-EV_DO接入技术 */

    PLATFORM_RAT_NR,    /* NR接入技术 */
    PLATFORM_RAT_BUTT   /* 无效值 */
};
typedef VOS_UINT16 PLATFORM_RatTypeUint16;


enum MTA_WcdmaBand {
    MTA_WCDMA_I_2100 = 0x0001,  /* 频段I-2100 */
    MTA_WCDMA_II_1900,          /* 频段II-1900 */
    MTA_WCDMA_III_1800,         /* 频段III-1800 */
    MTA_WCDMA_IV_1700,          /* 频段IV-1700 */
    MTA_WCDMA_V_850,            /* 频段V-850 */
    MTA_WCDMA_VI_800,           /* 频段VI-800 */
    MTA_WCDMA_VII_2600,         /* 频段VII-2600 */
    MTA_WCDMA_VIII_900,         /* 频段VIII-900 */
    MTA_WCDMA_IX_J1700,         /* 频段IX-J1700 */
    MTA_WCDMA_XI_1500 = 0x000B, /* 频段XI-1500 */
    MTA_WCDMA_XIX_850 = 0x0013, /* 频段XIX-850 */
    /*
     * 以下频段暂不支持
     * MTA_WCDMA_X,
     * MTA_WCDMA_XII,
     * MTA_WCDMA_XIII,
     * MTA_WCDMA_XIV,
     * MTA_WCDMA_XV,
     * MTA_WCDMA_XVI,
     * MTA_WCDMA_XVII,
     * MTA_WCDMA_XVIII,
     */
    MTA_WCDMA_BAND_BUTT
};
typedef VOS_UINT16 MTA_WcdmaBandUint16;


enum TAF_NVIM_RatMode {
    TAF_NVIM_RAT_MODE_GSM = 0x01, /* GSM接入模式 */
    TAF_NVIM_RAT_MODE_WCDMA,      /* WCDMA接入模式 */
    TAF_NVIM_RAT_MODE_LTE,        /* LTE接入模式 */
    TAF_NVIM_RAT_MODE_CDMA1X,     /* CDMA1X接入模式 */
    TAF_NVIM_RAT_MODE_TDSCDMA,    /* TDSCDMA接入模式 */
    TAF_NVIM_RAT_MODE_WIMAX,      /* WIMAX接入模式 */
    TAF_NVIM_RAT_MODE_EVDO,       /* EVDO接入模式 */

    TAF_NVIM_RAT_MODE_BUTT        /* 无效值 */
};
typedef VOS_UINT8 TAF_NVIM_RatModeUint8;


enum TAF_NVIM_GSM_BAND_ENUM {
    TAF_NVIM_GSM_BAND_850 = 0, /* 频段850 */
    TAF_NVIM_GSM_BAND_900,     /* 频段900 */
    TAF_NVIM_GSM_BAND_1800,    /* 频段1800 */
    TAF_NVIM_GSM_BAND_1900,    /* 频段1900 */

    TAF_NVIM_GSM_BAND_BUTT     /* 无效值 */
};
typedef VOS_UINT16 TAF_NVIM_GsmBandUint16;


enum NV_MsMode {
    NV_MS_MODE_CS_ONLY, /* 仅支持CS域 */
    NV_MS_MODE_PS_ONLY, /* 仅支持PS域 */
    NV_MS_MODE_CS_PS,   /* CS和PS都支持 */

    NV_MS_MODE_ANY, /* ANY,相当于仅支持CS域 */

    NV_MS_MODE_BUTT
};
typedef VOS_UINT8 NV_MsModeUint8;


enum AT_CvhuMode {
    CVHU_MODE_0,   /* 下发ATH可以挂断通话 */
    CVHU_MODE_1,   /* 下发ATH只是返回OK，不会影响通话 */
    CVHU_MODE_BUTT /* 其他值按照ATH可以挂断通话处理 */
};
typedef VOS_UINT8 AT_CvhuModeUint8;


enum CCWAI_Mode {
    CCWAI_MODE_DISABLE = 0,
    CCWAI_MODE_ENABLE  = 1,

    CCWAI_MODE_BUTT
};
typedef VOS_UINT8 CCWAI_ModeUint8;


enum CCWA_CtrlMode {
    CCWA_CTRL_BY_NW = 0,
    CCWA_CTRL_BY_UE = 1,

    CCWA_CTRL_MODE_BUTT
};
typedef VOS_UINT8 CCWA_CtrlModeUint8;

enum AT_UART_LinkType {
    AT_UART_LINK_TYPE_OM = 1, /* OM模式 */
    AT_UART_LINK_TYPE_AT = 2, /* AT模式 */
    AT_UART_LINK_TYPE_BUTT    /* 无效值 */
};
typedef VOS_UINT16 AT_UART_LinkTypeUint16;


typedef struct {
    VOS_UINT8 serialNumber[TAF_NVIM_SN_LEN]; /* SN码流 */
} TAF_NVIM_SerialNum;


typedef struct {
    /*
     * SVLTE特性的开启和关闭。
     * 1：支持SVLTE；
     * 0：不支持SVLTE。
     */
    VOS_UINT8 svlteSupportFlag;
    VOS_UINT8 reserved[3]; /* 保留 */
} SVLTE_SupportFlag;


typedef struct {
    /*
     * 取LC特性功能是否使能
     * 1：使能；
     * 0：不使能。
     * CDMA宏打开时lcEnableFlg默认填false
     */
    VOS_UINT8                           ucLCEnableFlg;
    TAF_NVIM_LcRatCombinedUint8 enRatCombined; /* L-C互操作方案接入模式 */
    TAF_NVIM_LcWorkCfgUint8     lcWorkCfg;   /* L-C互操作方案配置 */
    VOS_UINT8                           reserved[1];/* 保留 */
} TAF_NV_LC_CTRL_PARA_STRU;


typedef struct {
    /*
     * 0：上层应用为Mobile Partner（Stick形态默认值）；
     * 1：上层应用为Web UI（E5、Hilink形态默认值）；
     * 2：上层应用为Android（AP-Modem形态默认值）。
     */
    SYSTEM_AppConfigTypeUint16         sysAppConfigType;
    VOS_UINT8                          reserve1; /* 保留 */
    VOS_UINT8                          reserve2; /* 保留 */
} NAS_NVIM_SystemAppConfig;


typedef struct {
    /*
     * 锁频功能是否启用
     * 1：启用
     * 0：不启用
     */
    TAF_MMA_CfreqLockModeTypeUint8 freqLockMode;
    VOS_UINT8                               reserve[3];    /* 保留 */
    VOS_UINT16                              sid;           /* 系统号 */
    VOS_UINT16                              nid;           /* 网络号 */
    VOS_UINT16                              cdmaBandClass; /* cdma 1X频段 */
    VOS_UINT16                              cdmaFreq;      /* cdma 1X频点 */
    VOS_UINT16                              cdmaPn;        /* cdma 1X的PN值 */
    VOS_UINT16                              evdoBandClass; /* EVDO频段 */
    VOS_UINT16                              evdoFreq;      /* EVDO频点 */
    VOS_UINT16                              evdoPn;        /* EVDO的PN值 */
} TAF_NVIM_CfreqLockCfg;


typedef struct {
    VOS_UINT8 internationalRoamEmcNotSelIms; /* 国际漫游紧急呼不选择Ims标志 */
    /*
     * 紧急呼失败后是否换域标志，
     * 1:紧急呼失败后换域
     * 0:紧急呼失败后继续在当前域搜网尝试
     */
    VOS_UINT8 notReSelDomainAfterEmcFailFlg;
    VOS_UINT8 reserved2; /* 保留位 */
    VOS_UINT8 reserved3; /* 保留位 */
    VOS_UINT8 reserved4; /* 保留位 */
    VOS_UINT8 reserved5; /* 保留位 */
    VOS_UINT8 reserved6; /* 保留位 */
    VOS_UINT8 reserved7; /* 保留位 */
} TAF_NVIM_SpmEmcCustomCfg;


typedef struct {
    VOS_UINT8 multiModeEmcSupportFlag; /* 0：不支持；1：支持 */
    VOS_UINT8 reserved1;               /* 保留位 */
    VOS_UINT8 reserved2;               /* 保留位 */
    VOS_UINT8 reserved3;               /* 保留位 */
} TAF_NVIM_MultiModeEmcCfg;

/* 实际未使用 */

typedef struct {
    /*
     * IMS发送3GPP2短信功能使能标志：
     * 0：功能禁止
     * 1：功能开启
     */
    VOS_UINT8 lteSmsEnable;
    /*
     * IMS发送3GPP2短信重发功能使标志：
     * 0：禁止重发
     * 1：允许重发
     */
    TAF_LSMS_ResendFlagUint8 resendFlag;
    VOS_UINT8                       resendMax; /* IMS发送3GPP2短信重发最大次数 */
    /*
     * IMS发送3GPP2短信重发时间间隔
     * 单位：秒。
     */
    VOS_UINT8 resendInterval;
} TAF_NVIM_LteSmsCfg;


typedef struct
{
   /*
     * AT可信名单特性
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8       atCmdTrustListEnableFlg;
    VOS_UINT8       reserved1;
    VOS_UINT8       reserved2;
    VOS_UINT8       reserved3;
}TAF_NVIM_AtCmdTrustListCfg;


typedef struct {
    /*
     * 0：单活动Modem；
     * 1：多活动Modem。
     */
    TAF_NV_ActiveModemModeUint8         activeModem;
    VOS_UINT8                           reserve[3]; /* 保留位 */
} TAF_NV_DsdsActiveModemMode;


typedef struct {
    VOS_UINT8 operNameServicePrioNum; /* 用于配置查询运营商名称时支持的文件总数，取值范围[0-4] */

    /*
     * 1代表支持PNN：cosp或^eonsucs2查询时检查PNN文件查找匹配的运营商名称；
     * 2代表支持CPHS：cops或^ eonsucs2查询时检查CPHS文件查找匹配的运营商名；
     * 3代表支持SPN：cops或^eonsucs2查询时检查SPN文件查找匹配的运营商名称；
     * 4代表支持MM INFO：cops或^eonsucs2查询时从mm/gmm/emm information中查找匹配的运营商名称
     */
    VOS_UINT8 operNameSerivcePrio[TAF_NVIM_MAX_OPER_NAME_SERVICE_PRIO_NUM];
    VOS_UINT8 reserved1; /* 保留 */
    VOS_UINT8 reserved2; /* 保留 */
    VOS_UINT8 reserved3; /* 保留 */
    /*
     * 配置PLMN比较是否支持通配符
     * 0：不支持
     * 1：支持
     */
    VOS_UINT8 plmnCompareSupportWildCardFlag;
    VOS_UINT8 wildCard;                       /* 通配符,取值a-f,可代表0-9任意数字 */
    VOS_UINT8 reserved4;                      /* 保留 */
    VOS_UINT8 reserved5;                      /* 保留 */
} TAF_NVIM_EnhancedOperNameServiceCfg;


typedef struct {
    /*
     * NV安全控制模式：
     * 0：不受限；
     * 1：全部限制；
     * 2：受限名单。
     */
    VOS_UINT8 secType;
    /*
     * 整型值，受限名单中NV ID的个数，取值范围0~51。
     * 仅在secType值为2时生效。
     */
    VOS_UINT8 blockListNum;
    /*
     * 整型值数组，受限NV ID名单。
     * 仅在secType值为2时生效，有效个数由blockListNum指示。
     */
    VOS_UINT16 blockList[TAF_NV_BLOCK_LIST_MAX_NUM];
} TAF_NV_NvwrSecCtrl;


typedef struct {
    /*
     * 是否为C核单独复位
     * 0：不是C核单独复位,而是上电
     * 1：是C核单独复位
     */
    VOS_UINT8 ccpuResetFlag;
    VOS_UINT8 reserved1; /* 保留字段 */
    VOS_UINT8 reserved2; /* 保留字段 */
    VOS_UINT8 reserved3; /* 保留字段 */
} TAF_NVIM_CcpuResetRecord;


typedef struct {
    /*
     * 隐私过滤功能是否使能标志：
     * 0：不使能
     * 1：使能
     */
    VOS_UINT8 filterEnableFlg;
    VOS_UINT8 reserved[3]; /* 保留字段; */
} NAS_NV_PrivacyFilterCfg;


typedef struct {
    /*
     * 控制^XCPOSRRPT命令主动上报功能是否打开：
     * 0：关闭；
     * 1：开启。
     * 注：除此NV项外^XCPOSRRPT命令同时还受到^XCPOSR命令和^XCPOSRRPT设置命令控制
     */
    VOS_UINT8 xcposrRptNvCfg;
    VOS_UINT8 cposrDefault;     /* 单板上电后+CPOSR主动上报命令的默认值 */
    VOS_UINT8 xcposrDefault;    /* 单板上电后^XCPOSR命令的默认值 */
    VOS_UINT8 xcposrRptDefault; /* 单板上电后^XCPOSRRPT命令的默认值 */
} TAF_NVIM_XcposrrptCfg;


typedef struct {
    VOS_UINT8 ratOrderNum; /* SYS CFG配置时ratOrder中有效个数 */
    /*
     * Rat类型
     * 0: GSM
     * 1:WCDMA
     * 2:LTE
     * 3:1X
     * 4:HRPD
     * 5:NR
     */
    VOS_UINT8 ratOrder[TAF_NVIM_MAX_USER_SYS_CFG_RAT_NUM];
} TAF_NVIM_MultimodeRatCfg;


typedef struct {
    /*
     * GPS芯片类型：
     * 0：Broadcom
     * 1：Hisi1102
     */
    TAF_NV_GpsChipTypeUint8 gpsChipType;
    /*
     * SCPLL调整规避方案类型：
     * 0：关闭，参考时钟锁定按照实际情况上报；
     * 1：时钟锁定在Modem1时，替换模式为CDMA。
     */
    VOS_UINT8 pllStubType;
    VOS_UINT8 reserve2; /* 保留 */
    VOS_UINT8 reserve3; /* 保留 */
} TAF_NVIM_GpsCustCfg;


typedef struct {
    /*
     * 读取快速开机标志。
     * 0x00000000：不激活；
     * 0x01000101：激活。
     */
    MMA_QuickStartStaUint32 quickStartSta;
} NAS_NVIM_FollowonOpenspeedFlag;


typedef struct {
    VOS_UINT8 mfrId[TAF_MAX_MFR_ID_STR_LEN]; /* UE制造商名称，最后一个字节为结束符‘\0’。 */
} TAF_PH_FmrId;


typedef struct {
    VOS_UINT8 rsv[15];   /* 保留。 */
    VOS_UINT8 s0TimerLen; /* 自动应答等待时间（单位：s）。 */
} TAF_CCA_TelePara;


typedef struct {
    VOS_UINT8 productName[TAF_PH_PRODUCT_NAME_STR_LEN]; /* 保存产品名称，最后一个字节为结束符’\0’。 */
} TAF_PH_ProductName;


typedef struct {
    /*
     * 1、accessMode[0]
     * WCDMA单模，值为0x00。
     * WCDMA/GSM双模，值为0x01。
     * GSM单模，值为0x02。
     * 2、accessMode[1]
     * 自动选择网络，值为0x00。
     * 双模下优先选择GSM模式，值为0x01。
     * 双模下优先选择WCDMA模式，值为0x02。
     */
    VOS_UINT8 accessMode[2];
    VOS_UINT8 reserve1; /* 预留位 */
    VOS_UINT8 reserve2; /* 预留位 */
} NAS_MMA_NvimAccessMode;


typedef struct {
    /*
     * 0：只支持CS域手机；
     * 1：只支持PS域手机；
     * 2：支持CS/PS域手机；
     * 3：NV_MS_MODE_ANY，等同NV_MS_MODE_CS
     */
    VOS_UINT8 msClass;
    VOS_UINT8 reserved; /* 保留，用于字节对齐。 */
} NAS_NVIM_MsClass;


typedef struct {
    /*
     * 1、smsServicePara[0]
     *    短信接收存储上报方式：0：丢弃；1：modem存储短信并上报给AP；
     *                          2：modem不存储直接上报给AP；3：modem将短信上报给AP并进行确认。
     * 2、smsServicePara[1]
     *    仅当smsServicePara[6]为1时有效。
     *    0：无存储介质；
     *    1：SM（SIM卡）存储；
     *    2：ME（Flash）存储。
     * 3、smsServicePara[2]：短信状态报告存储上报方式：取值同index 0。
     * 4、smsServicePara[3]：短信状态报告接收存储介质：取值同index 1。
     * 5、smsServicePara[4]：广播短信接收存储介质：取值同index 1。
     * 6、smsServicePara[5]
     *    应用的短信存储状态，根据产品形态定制。
     *    0：应用的短信存储空间满不可用；
     *    1：应用的短信存储空间满可用。
     * 7、smsServicePara[6]
     *    smsServicePara[1]中存储的接收短信存储介质类型是否有效。
     *    0：无效，上电开机时短信接收存储介质类型是SM；
     *    1：有效，上电开机时短信接收存储介质类型从smsServicePara[1]获取。
     * 8、smsServicePara[7]
     *    发送域定制类别。
     *    0：自动调整短信发送域；
     *    1：只在+CGSMS命令指定的域中发送短信。
     */
    VOS_UINT8 smsServicePara[MN_MSG_SRV_PARAM_LEN];
} TAF_NVIM_SmsServicePara;


typedef struct {
    /*
     * 0：NV项关闭；
     * 1：NV项打开。
     */
    VOS_UINT8 roamFeatureFlg;
    /*
     * 0：国内国际漫游打开；
     * 1：国内漫游打开国际漫游关闭；
     * 2：国内漫游关闭国际漫游打开；
     * 3：国内漫游关闭国际漫游关闭；
     * 4：漫游不变。
     */
    VOS_UINT8 roamCapability;
    /*
     *  0: 驻留plmn和ehplmn比较，plmn相同认为非漫游，不同认为是漫游
     *  1: 驻留plmn的mcc和ehplmn的mcc比较，mcc相同认为非漫游，不同认为是漫游
     */
    VOS_UINT8 roamRule;
    VOS_UINT8 reserve2; /* 保留 */
} NAS_NVIM_RoamCfgInfo;

/* NV_ITEM_CUSTOMIZE_SIM_LOCKPLMN_INFO 8267 */

typedef struct {
    VOS_UINT8 mncNum; /* 字符型：默认0xaa */
    /*
     * 字符“0”～“9”，对号码段中未填入号码段的位置全部置为0x0A，而对填入了号码段但未达到15位的位置全部置为0x0F，
     * NV中的实际值对应15位IMSI号段或“0x0A”或“0x0F”
     */
    VOS_UINT8 rangeBegin[TAF_PH_SIMLOCK_PLMN_STR_LEN];
    /*
     * 字符“0”～“9”，对号码段中未填入号码段的位置全部置为0x0A，而对填入了号码段但未达到15位的位置全部置为0x0F，
     * NV中的实际值对应15位IMSI号段或“0x0A”或“0x0F”
     */
    VOS_UINT8 rangeEnd[TAF_PH_SIMLOCK_PLMN_STR_LEN];
} TAF_CUSTOM_SimLockPlmnRange;


typedef struct {
    VOS_UINT32                          status; /* NV项是否激活标志，0不激活，1激活 */
    /* 记录SIM LOCK要求的PLMN信息 */
    TAF_CUSTOM_SimLockPlmnRange simLockPlmnRange[TAF_MAX_SIM_LOCK_RANGE_NUM];
} TAF_CUSTOM_SimLockPlmnInfo;

/* NV_ITEM_CARDLOCK_STATUS 8268 */

typedef struct {
    /*
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT32 status;
    /*
     * 0：CARDLOCK_CARD_UNLOCKCODE_NEEDED，数据卡可以正常使用，目前暂时没有使用。
     * 1：CARDLOCK_CARD_UNLOCKED，数据卡没有被锁定，但是需要输入<unlock_code>。
     * 2：CARDLOCK_CARD_LOCKED，数据卡已经被解锁，可以正常使用，如果是合法的IMSI，上报给后台的也是这个值。
     * 3：CARDLOCK_MAX，数据卡已经被锁定，不能够使用非法的SIM卡。
     */
    VOS_UINT32 cardlockStatus;
    VOS_UINT32 remainUnlockTimes; /* 剩余解锁次数，必须为整数：0～4294967295（2的32次方减1） */
} TAF_NVIM_CustomCardLockStatus;

/* NV_ITEM_CUSTOMIZE_SIM_LOCK_MAX_TIMES 8269 */

typedef struct {
    /*
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT32 status;
    VOS_UINT32 lockMaxTimes; /* 最大解锁次数，必须为整数：0～10 */
} TAF_CUSTOM_SimLockMaxTimes;


typedef struct {
    /*
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT32 status;
    /*
     * 是否使能普通GSM和CDMA语音业务。
     * 0：禁止；
     * 1：使能。
     */
    VOS_UINT32 customizeService;
} NAS_NVIM_CustomizeService;


typedef struct {
    /*
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 ucStatus;
    /*
     * WINS使能标记
     * 0：禁止；
     * 1：使能。
     */
    VOS_UINT8 ucWins;
    VOS_UINT8 reserve1; /* 保留 */
    VOS_UINT8 reserve2; /* 保留 */
} WINS_Config;


typedef struct {
    /*
     * 激活状态：
     * 0：未激活；
     * 1：激活；
     * -1：不支持。
     */
    VOS_INT8 status;
    /*
     * URL字符串的编码方式：
     * 1：8bit编码；
     * 2：UCS2编码；
     */
    VOS_UINT8  encodeType;
    VOS_UINT8  reserved1;                                     /* 保留 */
    VOS_UINT8  reserved2;                                     /* 保留 */
    VOS_UINT32 length;                                        /* URL字符串的长度。 */
    VOS_UINT8  data[TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN]; /* URL字符串码流，目前支持的最大长度为160字节。 */
} TAF_AT_NvimSmsActiveMessage;


typedef struct {
    /*
     * 此NV是否有效。
     * 0：无效；
     * 1：有效。
     */
    VOS_UINT8 activeFlag;
    VOS_UINT8 svn[TAF_SVN_DATA_LENGTH]; /* IMEI SVN第二个数字，取值范围0~9。 */
    VOS_UINT8 reserve[1];               /* 保留字节。 */
} TAF_SVN_Data;


typedef struct {
    /*
     * 是否支持PC Voice功能。
     * 0：不支持（其他形态默认值）；
     * 1：支持（STICK，HILINK形态默认值）。
     */
    VOS_UINT16 pcVoiceSupportFlag;
    VOS_UINT8  reserve1; /* 保留 */
    VOS_UINT8  reserve2; /* 保留 */
} APP_VC_NvimPcVoiceSupportFalg;


typedef struct {
    /*
     * ME存储短信功能使能标志。
     * 0：不使能；
     * 1：使能。
     */
    MN_MSG_MeStorageStatusUint8 meStorageStatus;
    VOS_UINT8                           reserve[1]; /* 保留。 */
    /*
     * ME存储短信容量，在MeStorageStatus使能时有效，取值范围0~500条。
     * 取值=0，ME存储介质无可用空间；
     * 取值>500，协议栈强制将ME存储容量修改为500。
     */
    VOS_UINT16 meStorageNum;
} MN_MSG_MeStorageParm;


typedef struct {
    VOS_UINT8 ratOrderNum; /* 当前设定的接入技术个数，取值范围1~6。*/
    /*
     * 当前系统支持的接入技术及其优先级列表。
     * 0：GSM；
     * 1：WCDMA；
     * 2：LTE；
     * 3：1X；
     * 4：HRPD；
     * 5: NR。
     */
    VOS_UINT8 ratOrder[TAF_PH_NVIM_MAX_GUL_RAT_NUM];
} TAF_PH_NvimRatOrder;


typedef struct {
    VOS_UINT8 ratOrderNum; /* 当前设定支持的接入制式个数，取值范围1~6; */
    /*
     * 当前支持的接入制式：
     * 0：GSM；
     * 1：WCDMA；
     * 2：LTE；
     * 3：1X；
     * 4：HRPD；
     * 5：NR;
     */
    VOS_UINT8 ratOrder[TAF_NVIM_MAX_RAT_ORDER_NUM];
    VOS_UINT8 reserved1; /* 保留 */
    VOS_UINT8 reserved2; /* 保留 */
    VOS_UINT8 reserved3; /* 保留 */

} TAF_NVIM_RatOrderEx;


typedef struct {
    /*
     * LTE允许漫游标记。
     * 0：LTE禁止漫游；
     * 1：LTE允许漫游。
     */
    VOS_UINT8 lteRoamAllowedFlg;
    VOS_UINT8 reserve[1];             /* 保留项 */
    /* 允许漫游的国家码列表。配置漫游MCC，每4个字节配置一个国家码。如：MCC为460时，配置的4个字节分别是 4 6 0 0。 */
    VOS_UINT8 roamEnabledMccList[20];
} NAS_MMC_NvimLteInternationalRoamCfg;


typedef struct {
    /*
     * NV项状态。
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 status;
    /*
     * 是否激活防卡贴功能。
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 actFlg;
    VOS_UINT8 reserve1; /* 保留 */
    VOS_UINT8 reserve2; /* 保留 */
} NAS_PREVENT_TestImsiReg;


typedef struct {
    /*
     * 短信接收流程定制激活标志。
     * 0：未激活；
     * 1：已激活。
     */
    VOS_UINT8 actFlag;
    /*
     * 短信接收流程定制类型。
     * 0：未定制；
     * 1：DCM定制；
     */
    VOS_UINT8 enMtCustomize;
    /*
     * 接收Download短信流程中，是否延时发送stk短信。
     * 0：不延时
     * 1：延时
     */
    VOS_UINT8 mtCustomize[2];
} MN_MSG_MtCustomizeInfo;


typedef struct {
    /*
     * 信号改变门限配置
     * 0：信号质量改变门限接入层使用默认值。GAS默认值是3dB，WAS默认值由NV9067配置；
     * 1：信号质量改变超过1dB，接入层需主动上报信号质量；
     * 2：信号质量改变超过2dB，接入层需主动上报信号质量；
     * 3：信号质量改变超过3dB，接入层需主动上报信号质量；
     * 4：信号质量改变超过4dB，接入层需主动上报信号质量；
     * 5：信号质量改变超过5dB，接入层需主动上报信号质量。
     */
    VOS_UINT8 signThreshold;
    /*
     * 0：上报扩展信号质量无时间限制；
     * 1-20：两次上报信号质量的最小间隔时间，单位：s。
     */
    VOS_UINT8 minRptTimerInterval;
    VOS_UINT8 rerved1; /* 保留，值为0 */
    VOS_UINT8 rerved2; /* 保留，值为0 */
} NAS_NVIM_CellSignReportCfg;


typedef struct {
    /*
     * 是否使能产品线定制的PDP回退原因值。
     * 1：使能；
     * 0：禁止；
     */
    VOS_UINT32 activeFlag;
    VOS_UINT8  smCause[TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM]; /* 产品线定制的PDP回退原因值。 */
} TAF_NV_Ipv6FallbackExtCause;


typedef struct {
    /*
     * 此NV是否生效。
     * 0：此NV不生效；
     * 1：此NV生效。
     */
    VOS_UINT8 nvimValid;
    /*
     * 当ucNvimValid指示此NV生效时，当收不到AP的RP ACK时，后续收到的短信是否需要本地存储。
     * 0：不需要；
     * 1：需要。
     * 当设置为1时，后续收到的MT短信进行本地存储，不会上报给AP，即关闭上报给AP的通道，把CNMI的参数<mode>和<ds>修改为0。
     */
    VOS_UINT8 smsClosePathFlg;
} TAF_NVIM_SmsClosePathCfg;


typedef struct {
    VOS_UINT16 ratNum; /* 接入技术的个数。 */
    /*
     * 接入技术，Rat类型。
     * 0：GSM；
     * 1：WCDMA；
     * 2：LTE；
     * 3：TDS；
     * 4：1X；
     * 5：HRPD；
     * 6：NR。
     */
    PLATFORM_RatTypeUint16 ratList[PLATFORM_MAX_RAT_NUM];
} PLATAFORM_RatCapability;


typedef struct {
    VOS_INT16 gprsPower; /* GSM频段的GPRS发射功率门限值，单位为dbm，取值范围15~33 */
    VOS_INT16 edgePower; /* GSM频段的EDGE发射功率门限值，单位为dbm，取值范围15~33 */
} MTA_BodySarGPara;


typedef struct {
    MTA_WcdmaBandUint16        band; /* WCDMA频段，Band值为1~9、11、19。 */
    VOS_INT16                  power; /* 对应WCDMA频段的最大发射功率门限值，单位为dbm，取值范围17~24。 */
} MTA_BodySarWPara;


typedef struct {
    /*
     * GSM 配置功率门限的BAND掩码，其参数为16进制，每个bit对应一个频段，Bit值为1表示已配置，为0表示未配置。
     * Bit0：GSM850(GPRS)；
     * Bit1：GSM900(GPRS)；
     * Bit2：GSM1800(GPRS)；
     * Bit3：GSM1900(GPRS)；
     * Bit16：GSM850(EDGE)；
     * Bit17：GSM900(EDGE)；
     * Bit18：GSM1800(EDGE)；
     * Bit19：GSM1900(EDGE)。
     */
    VOS_UINT32               gBandMask;
    VOS_UINT16               wBandNum;      /* WCDMA配置功率门限的BAND个数，最多支持5个频段。 */
    VOS_UINT16               reserved1[1]; /* 保留。 */
    MTA_BodySarGPara gBandPara[MTA_BODY_SAR_GBAND_MAX_NUM]; /* GSM频段对应的GPRS/EDGE功率门限值。 */
    MTA_BodySarWPara wBandPara[MTA_BODY_SAR_WBAND_MAX_NUM]; /* WCDMA对应的频段及功率门限值。 */
} MTA_BodySarPara;


typedef struct {
    /*
     * 紧急呼叫与温度保护交互标志位。
     * 1：使能；
     * 0：禁止。
     */
    VOS_UINT8 eCallNotifySupport;
    /*
     * 温保状态上报使能标志位。
     * 1：使能；
     * 0：禁止。
     */
    VOS_UINT8 spyStatusIndSupport;
    VOS_UINT8 reserved[2]; /* 保留项 */
} TAF_TempProtectConfig;


typedef struct {
    /*
     * 短信优选域。
     * 0：不使用IMS短信；
     * 1：优先使用IMS短信,CS/PS短信备选。
     */
    VOS_UINT8 smsDomain;
    VOS_UINT8 reserved[3]; /* 保留字段 */
} TAF_NVIM_SmsDomain;


typedef struct {
    /*
     * Jam Detect功能工作模式。
     * 0：关闭
     * 1：打开
     */
    VOS_UINT8 mode;
    /*
     * Jam Detect使用的方案。目前只支持方案2。
     * 1：方案1
     * 2：方案2
     */
    VOS_UINT8 method;
    VOS_UINT8 freqNum;            /* 检测需要达到的频点个数，取值范围[0-255]。 */
    VOS_UINT8 threshold;          /* 检测需要达到的频点测量值，取值范围[0-70]。 */
    VOS_UINT8 jamDetectingTmrLen; /* 强信号频点个数满足门限后，检查同步结果的定时器时长，单位：秒。 */
    VOS_UINT8 jamDetectedTmrLen;  /* 检测到干扰后，等待搜网结果的最长时间，单位：秒。 */
    /*
     * 搜网过程中是否需要快速上报。
     * 0：否
     * 1：是
     */
    VOS_UINT8 fastReportFlag;
    VOS_UINT8 rxlevThresholdOffset; /* G模噪音门限配置偏移值。 */
} NV_NAS_JamDetectCfg;


typedef struct {
    /*
     * 锁频功能开关。
     * 0：关闭；
     * 1：打开。
     */
    VOS_UINT8                     enableFlg;
    TAF_NVIM_RatModeUint8  ratMode;    /* 接入模式。 */
    TAF_NVIM_GsmBandUint16 band;       /* GSM频段。 */
    VOS_UINT32                    lockedFreq; /* 锁频频点。 */
} TAF_NVIM_FreqLockCfg;

/*
 * 标    识: NV_ITEM_TRAFFIC_CLASS_TYPE(50009)
 * 结构说明: 用于定制PDP激活请求中QoS中Traffic Class的值
 */
typedef struct {
    /*
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 status;
    /*
     * Traffic Class的值,整数。
     * 0：Subscribed value；
     * 1：Conversational class；
     * 2：Streaming Class；
     * 3：Interactive Class；
     * 4：Background Class。
     */
    VOS_UINT8 trafficClass;
    VOS_UINT8 reserve1; /* 保留位 */
    VOS_UINT8 reserve2; /* 保留位 */
} AT_TrafficClassCustomize;


typedef struct {
    /*
     * NV激活标志位
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 status;
    /*
     * USSD消息传输方案, 默认为透传模式。
     * 0：USSD非透传方案（即单板支持编解码）；
     * 1：USSD透传方案（即单板不编解码，只是透传，由后台来编解码）
     */
    VOS_UINT8 ussdTransMode;
    VOS_UINT8 reserve1; /* 保留位 */
    VOS_UINT8 reserve2; /* 保留位 */
} TAF_UssdNvimTransMode;


typedef struct {
    /*
     * 本NV项是否激活。
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 actFlg;
    /*
     * 0：Vodafone Class0短信定制；
     * 1：Italy TIM Class0短信定制（即H3G Class0 短信定制）；
     * 2：3GPP协议的处理方式；
     * 3：巴西VIVO定制（CLASS0类短信总是直接上报内容，根据短信协议类型（通过CSMS命令修改）确定是否由应用确认短信接收）。
     */
    VOS_UINT8 class0Tailor;
} MN_MSG_NvimClass0Tailor;


typedef struct {
    /*
     * 设置呼叫等待的控制模式；
     * ccwaCtrlMode为0，则呼叫等待由3gpp网络控制；
     * ccwaCtrlMode为1，则呼叫等待由UE控制，用于VOLTE的网络。
     * （在VoLTE的网络上，AP配置CCWA支持时，IMS并没有和网络交互，VoLTE的电话的CCWA由UE控制）。
     * 支持VOLTE的终端，不管VOLTE开关有没有打开，都是下发CCWAI来设置呼叫等待。
     */
    VOS_UINT8 ccwaCtrlMode;
    /*
     * 呼叫等待设置模式
     * 0：disable
     * 1：enable
     */
    VOS_UINT8 ccwaiMode;
    VOS_UINT8 reserved1; /* 预留 */
    VOS_UINT8 reserved2; /* 预留 */
} TAF_CALL_NvimCcwaCtrlMode;


typedef struct {
    /*
     * CVHU模式设置,控制是否支持ATH挂断通话。
     * 0：下发ATH可以挂断通话。
     * 1：下发ATH只是返回OK，不影响通话。
     */
    AT_CvhuModeUint8        cvhuMode;
    VOS_UINT8               reserved1; /* 预留位 */
    VOS_UINT8               reserved2; /* 预留位 */
    VOS_UINT8               reserved3; /* 预留位 */
} NAS_NVIM_CustomCallCfg;


typedef struct {
    /*
     * USIMM、SIMSQ命令主动上报功能开关。
     * 0：关闭。
     * 1：打开。
     */
    VOS_UINT8 simsqEnable;
    VOS_UINT8 reserved1; /* 预留位 */
    VOS_UINT8 reserved2; /* 预留位 */
    VOS_UINT8 reserved3; /* 预留位 */
} NAS_NVIM_CustomUsimmCfg;


typedef struct {
    VOS_UINT8 ucCustomCharNum; /* 定制字符个数，最多可以设置16个字符。 */
    /*
     * APNNI部分开头结尾字符串检查使能项。
     * 0：不检查
     * 1：检查
     */
    VOS_UINT8 ucProtocolStringCheckFlag;
    VOS_UINT8 ucReserved1; /* 保留位 */
    VOS_UINT8 ucReserved2; /* 保留位 */
    VOS_UINT8 ucReserved3; /* 保留位 */
    VOS_UINT8 ucReserved4; /* 保留位 */
    VOS_UINT8 ucReserved5; /* 保留位 */
    VOS_UINT8 ucReserved6; /* 保留位 */
    /* 定制字符数组，可以设置ASCII表中除控制字符（0-31），正反斜杠（47,92）以及删除符号（127）以外的字符。 */
    VOS_UINT8 aucCustomChar[TAF_NVIM_APN_CUSTOM_CHAR_MAX_NUM];
} TAF_NVIM_ApnCustomFormatCfg;


typedef struct {
    /*
     * eCall优选域
     * 0：eCall优先在PS域发起，PS域不支持时再CS域发起。
     * 1：针对MIEC、AIEC eCall考虑支持紧急呼换域重拨，优选域配置为1时优先在CS域发起。
     *    针对test、recfg eCall，普通呼不支持低制式向高制式换域，优先域配置为1时，eCall只允许在CS域发起。
     */
    VOS_UINT8 ecallDomainMode;
    /*
     * eCall强制模式
     * 0：Auto模式，UE根据SIM文件来决定运行模式，根据3GPP协议TS31.102，
     *    如果在EFUST中eCall data (Service n°89) 和FDN (Service n°2) 开启，并且EFEST中FDN（Service n°1）开启，
     *    则以eCall only模式运行，
     *    如果在EFUST中eCall data (Service n°89) 和SDN (Service n°4) 开启，则以eCall and normal模式运行，其他情况按
     *    照normal模式运行；
     * 1：Force eCall Only模式，无论是插入的是哪种SIM卡，UE都以eCall only模式来运行；eCall only模式下不主动注册网络，
     *    处于eCall inactive状态；
     * 2：Force eCall and normal模式，无论是插入的是哪种SIM卡，UE都以eCall and normal模式来运行；eCall and normal
     *    模式正常注册网络；
     * 3：normal模式，无论是插入的是哪种SIM卡，UE都以normal模式来运行；normal模式正常注册网络；
     */
    VOS_UINT8 ecallForceMode;
    /*
      * 插入USIM卡时，eCall Over IMS特性开关
      * 0：关闭，则不读取eCall Over IMS相关卡文件，认为不支持IMS域，只支持CS域
      * 1：支持IMS+CS域
      */
    VOS_UINT8 eCallOverImsCapUsimCard;
    /*
      * 无卡时eCall Over IMS特性开关
      * 0：关闭，认为不支持IMS域，只支持CS域
      * 1：支持IMS+CS域
      */
    VOS_UINT8 eCallOverImsCapNoCard;
} TAF_NVIM_CustomEcallCfg;

/*
 * 标    识: NV_ITEM_USB_ENUM_STATUS(21)
 * 结构说明: 单板的USB虚拟设备形态值，可以通过“at^u2diag=”来设置。
 */
typedef struct {
    /*
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT32 status;
    VOS_UINT32 value;    /* 产线配置。 */
    VOS_UINT32 reserve1; /* 保留 */
    VOS_UINT32 reserve2; /* 保留 */
} USB_ENUM_Status;


typedef struct {
    /*
     * Bit[0]：DRX Debug Flag；
     * Bit[1]：USIMM Debug Flag；
     * Bit[2] - Bit[31]：保留。
     */
    VOS_UINT32 commDebugFlag;
} TAF_AT_NvimCommdegbugCfg;

/*
 * 标    识: NV_ITEM_PID_ENABLE_TYPE(2601)
 * 结构说明: 不再使用。
 */
typedef struct {
    VOS_UINT32 pidEnabled; /* pid使能标记；0：不使能；1：使能 */
} NV_PID_EnableType;


typedef struct {
    /*
     * 0：烧片版本。
     * 1：正式版本。
     */
    VOS_UINT32 nvSwVerFlag;
} OM_SW_VerFlag;


typedef struct {
    VOS_INT8 ate5DissdPwd[AT_DISSD_PWD_LEN]; /* 字符串。 */
} TAF_AT_NvimDissdPwd;


typedef struct {
    VOS_INT8 atOpwordPwd[AT_OPWORD_PWD_LEN]; /* 字符串，默认用户已授权。 */
} TAF_AT_NvimDislogPwdNew;


typedef struct {
    /*
     * UART端口默认工作模式。
     * 1：OM；
     * 2：AT。
     */
    AT_UART_LinkTypeUint16  uartLinkType;
    VOS_UINT8                     reserve1; /* 保留 */
    VOS_UINT8                     reserve2; /* 保留 */
} TAF_AT_NvimDefaultLinkOfUart;

/*
 * 标    识: NV_ITEM_BATTERY_ADC(90)
 * 结构说明: 用于保存电池校准参数数据结构
 */
typedef struct {
    VOS_UINT16 minValue; /* 最小值 */
    VOS_UINT16 maxValue; /* 最大值 */
} VBAT_CALIBART_Type;


typedef struct {
    /* 字段0的定制内容，内容长度为128字节，每个字节用十进制表示，第129字节存储结束符。 */
    VOS_UINT8 factInfo1[AT_FACINFO_INFO1_STR_LENGTH];
    /* 字段1的定制内容，内容长度为128字节，每个字节用十进制表示，第258字节存储结束符。 */
    VOS_UINT8 factInfo2[AT_FACINFO_INFO2_STR_LENGTH];
    VOS_UINT8 reserve1; /* 保留 */
    VOS_UINT8 reserve2; /* 保留 */
} TAF_AT_NvimFactoryInfo;


typedef struct {
    /*
     * Yyyy-mm-dd hh-mm-ss格式存储的生产日期。
     * 年份固定为4位，月份，日期和小时，分钟，秒均为2位。不足2位时，左边补0。
     */
    VOS_UINT8 mDate[AT_MDATE_STRING_LENGTH];
} TAF_AT_NvimManufactureDate;


typedef struct {
    VOS_UINT32 smsRiOnInterval;    /* 短信RI高电平持续时间，单位为毫秒(ms)。 */
    VOS_UINT32 smsRiOffInterval;   /* 短信RI低电平持续时间，单位为毫秒(ms)。 */
    VOS_UINT32 voiceRiOnInterval;  /* 语音RI高电平持续时间，单位为毫秒(ms)。 */
    VOS_UINT32 voiceRiOffInterval; /* 语音RI低电平持续时间，单位为毫秒(ms)。 */
    VOS_UINT8  voiceRiCycleTimes;  /* 语音RI波形周期次数。 */
    VOS_UINT8  reserved[3];        /* 保留。 */
} TAF_NV_UartRi;


typedef struct {
    /*
     * 帧格式。取值范围：（1-6）
     * 1：8个数据位2个停止位
     * 2：8个数据位1个校验位1个停止位
     * 3：8个数据位1个停止位
     * 4：7个数据位2个停止位
     * 5：7个数据位1个校验位1个停止位
     * 6：7个数据为1个停止位
     */
    VOS_UINT8 format;
    /*
     * 校验算法。取值范围：（0-3）
     * 0：奇校验
     * 1：偶校验
     * 2：标记校验
     * 3：空校验
     */
    VOS_UINT8 parity;
    VOS_UINT8 reserved[2]; /* 保留 */

} TAF_NV_UartFrame;


typedef struct {
    /*
     * 波特率。取值范围：
     * 0, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800. 921600, 2764800, 4000000
     */
    VOS_UINT32             baudRate;
    TAF_NV_UartFrame frame;    /* UART帧格式。 */
    TAF_NV_UartRi    riConfig; /* UART Ring脚配置。 */
} TAF_NV_UartCfg;


typedef struct {
    /*
     * 短信读取缓存控制。
     * 0：无效；
     * 1：有效。
     */
    VOS_UINT8 enableFlg;
    VOS_UINT8 reserved[3]; /* 保留 */
} TAF_NV_PortBuffCfg;


typedef struct {
    /*
     * CDMAMODEMSWITCH不重启Modem特性是否启用。
     * 0：不启用，即切模后重启Modem；
     * 1：启用，即切模后不重启Modem。
     * 注意：
     * 1. 此特性由终端定制；
     * 2. 若启用特性，将不支持开机状态下发CdmaModemSwitch命令，也不支持在副卡下发CdmaModemSwitch命令。
     *    即在下发CdmaModemSwitch命令进行切模前，AP必须确保主副Modem均处于关机状态，且在主卡下发此命令。
     * 3. 若关闭此特性，切模后需重启Modem。
     */
    VOS_UINT8 enableFlg;
    VOS_UINT8 reversed[7]; /* 保留位，八字节对齐 */
} TAF_NVIM_CdmamodemswitchNotResetCfg;


typedef struct {
    VOS_UINT32 ipv6AddrTestModeCfg; /* 值为0x55AA55AA时，是测试模式，其他值为正常模式。 */
} TAF_NVIM_Ipv6AddrTestModeCfg;



typedef struct {
    VOS_UINT8 info[128]; /* 单板制造信息，前16bytes记录单板SN号，其他为保留位。 */
} OM_ManufactureInfo;


typedef struct {
    /*
     * 是否能够通过netlink socket输出modem log。
     * 0：不使能, default:0；
     * 1：使能。
     */
    VOS_UINT8 printModemLogType;
    VOS_UINT8 reserved0; /* 保留字节 */
    VOS_UINT8 reserved1; /* 保留字节 */
    VOS_UINT8 reserved2; /* 保留字节 */
} TAF_NV_PrintModemLogType;


typedef struct {
    /*
     * 用户是否可以通过AT+CFUN=1,1直接触发整机复位。
     * 0xEF：可以
     * 其他：不可以
     */
    VOS_UINT8 userRebootConfig;
    VOS_UINT8 reserved1; /* 保留字段 */
    VOS_UINT8 reserved2; /* 保留字段 */
    VOS_UINT8 reserved3; /* 保留字段 */
} TAF_NVIM_UserRebootSupport;


typedef struct {
    /*
     * 其中每个字节的BIT0-BIT1对应一client归属归属于哪个Modem Id。
     * 00:表示Modem0
     * 01:表示Modem1
     * 10:表示Modem2。
     * BIT2对应一个client是否允许广播:
     * 0:表示不允许
     * 1:表示允许。
     * 最多支持96个通道
     */
    VOS_UINT8 atClientConfig[TAF_AT_NVIM_CLIENT_CFG_LEN];
} TAF_AT_NvimAtClientCfg;


typedef struct {
    VOS_UINT8 gmmInfo[2]; /* Gmm信息 */
    VOS_UINT8 reserve1;    /* 保留 */
    VOS_UINT8 reserve2;    /* 保留 */
} TAF_AT_NvGmmInfo;


typedef struct {
    /*
     * 0：用户没有设置接收分集；
     * 1：用户设置过接收分集。
     */
    VOS_UINT8 vaild;
    VOS_UINT8 reserved1; /* 保留。 */
    VOS_UINT8 reserved2; /* 保留。 */
    VOS_UINT8 reserved3; /* 保留。 */
} TAF_AT_NvimRxdivConfig;


typedef struct {
    /*
     * 0：不上报（手机以外形态默认值）；
     * 1：上报（手机形态的默认值）。
     */
    VOS_UINT16 reportRegActFlg;
    VOS_UINT8  reserve1; /* 保留 */
    VOS_UINT8  reserve2; /* 保留 */
} TAF_AT_NvimReportRegActFlg;

/*
 * 标    识: NV_ITEM_NDIS_DHCP_DEF_LEASE_TIME(2635)
 * 结构说明: 此NV已废弃。
 */
typedef struct {
    VOS_UINT32 dhcpLeaseHour; /* Range:[0x1,0x2250] */
} NDIS_NV_DhcpLeaseHour;


typedef struct {
    /*
     * 此NV项是否激活。
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 status;
    /*
     * CREG/CGREG的<CI>域上报字节数
     * 0：<CI>域以2字节方式上报；
     * 1：<CI>域以4字节方式上报。
     */
    VOS_UINT8 ciBytesRpt;
    VOS_UINT8 reserve1;                  /* 保留 */
    VOS_UINT8 reserve2;                  /* 保留 */
} NAS_NV_CregCgregCiFourByteRpt; /* VDF需求: CREG/CGREG命令<CI>域是否以4字节上报的NV项控制结构体 */


typedef struct {
    VOS_INT8 tz[AT_NVIM_SETZ_LEN]; /* 字符串。 */
} TAF_AT_Tz;


typedef struct {
    VOS_INT8 errorText[AT_NOTSUPPORT_STR_LEN]; /* 字符串。 */
} TAF_AT_NotSupportCmdErrorText;


typedef struct {
    /*
     * 0：PCUI口打开；
     * 1：PCUI口关闭。
     */
    VOS_UINT32 rightOpenFlg;
    VOS_INT8   password[AT_NVIM_RIGHT_PWD_LEN]; /* 开启PCUI通道的密码字符串。 */
} TAF_AT_NvimRightOpenFlag;


typedef struct {
    /* IPV6 MTU 信息, 单位为字节  */
    VOS_UINT32 ipv6RouterMtu;
} TAF_NDIS_NvIpv6RouterMtu;

/*
 * 标    识: NV_ITEM_IPV6_CAPABILITY(8514)
 * 结构说明: 用于读取读取IPV6能力相关信息, 目前IPV6能力只能支持到IPV4V6_OVER_ONE_PDP
 */
typedef struct {
    /*
     * 此NV是否有效标志。
     * 0：无效；
     * 1：有效。
     */
    VOS_UINT8 status;
    /*
     * 1：IPv4 only；
     * 2：IPv6 only；
     * 4：IPV4V6 support enabled over one IPV4V6 context (fallbacking on 2 single address PDP contexts if necessary)；
     * 8：IPV4V6 support enabled over 2 single address PDP contexts（暂不支持）。
     */
    VOS_UINT8 ipv6Capablity;
    VOS_UINT8 reversed[2]; /* 保留。 */
} AT_NV_Ipv6Capability;


typedef struct {
    VOS_UINT8 gsmConnectRate;   /* GSM连接速率，单位：MB/s。 */
    VOS_UINT8 gprsConnectRate;  /* Gprs连接速率，单位：MB/s。 */
    VOS_UINT8 edgeConnectRate;  /* Edge连接速率，单位：MB/s。 */
    VOS_UINT8 wcdmaConnectRate; /* Wcdma连接速率，单位：MB/s。 */
    VOS_UINT8 dpaConnectRate;   /* DPA连接速率，单位：MB/s。 */
    VOS_UINT8 reserve1;         /* 保留 */
    VOS_UINT8 reserve2;         /* 保留 */
    VOS_UINT8 reserve3;         /* 保留 */
} AT_NVIM_DialConnectDisplayRate;


typedef struct {
    VOS_UINT32 opbSupport : 1;
    VOS_UINT32 opgSupport : 1;
    VOS_UINT32 opnSupport : 1;
    VOS_UINT32 opSpare : 29;
    VOS_UINT8  bHighChannel; /* 802.11b制式支持最大信道号，取值范围0~14 */
    VOS_UINT8  bLowChannel;  /* 802.11b制式支持最小信道号，取值范围0~14 */
    VOS_UINT8  rsv1[2];     /* 预留，暂未使用 */
    /*
     * 802.11b制式下的期望功率，取值范围0~65535。
     * bPower[0]：PA模式期望功率；
     * bPower[1]：NO PA模式期望功率。
     */
    VOS_UINT16 bPower[2];
    VOS_UINT8  gHighChannel; /* 802.11g制式支持最大信道号，取值范围0~14 */
    VOS_UINT8  gLowChannel;  /* 802.11g制式支持最小信道号，取值范围0~14 */
    VOS_UINT8  rsv2[2];     /* 预留，暂未使用 */
    /*
     * 802.11g制式下的期望功率，取值范围0~65535。
     * gPower[0]：PA模式期望功率；
     * gPower[1]：NO PA模式期望功率。
     */
    VOS_UINT16 gPower[2];   /* 预留，暂未使用 */
    VOS_UINT8  nHighChannel; /* 802.11n制式支持最大信道号，取值范围0~14 */
    VOS_UINT8  nLowChannel;  /* 802.11n制式支持最小信道号，取值范围0~14 */
    VOS_UINT8  rsv3[2];     /* 预留，暂未使用 */
    /*
     * 802.11n制式下的期望功率，取值范围0~65535。
     * nPower[0]：PA模式期望功率；
     * nPower[1]：NO PA模式期望功率。
     */
    VOS_UINT16 nPower[2];
} AT_WIFI_Info;

/*
 * 标    识: NV_ITEM_WIFI_INFO(2656)
 * 结构说明: 用于存储WIFI支持模式、信道号、期望功率等信息。
 */
typedef struct {
    /*
     * BIT 0单板是否支持802.11b制式。
     * 0：不支持；
     * 1：支持。
     * BIT 1单板是否支持802.11g制式。
     * 0：不支持；
     * 1：支持。
     * BIT 2单板是否支持802.11n制式。
     * 0：不支持；
     * 1：支持。
     * BIT 4~BIT 31预留，暂未使用。
     */
    VOS_UINT32 opSupport;
    VOS_UINT8  bHighChannel; /* 802.11b制式支持最大信道号，取值范围0~14。 */
    VOS_UINT8  bLowChannel;  /* 802.11b制式支持最小信道号，取值范围0~14。 */
    VOS_UINT8  rsv1[2];     /* 预留，暂未使用。 */
    /*
     * 802.11b制式下的期望功率，取值范围0~65535。
     * bPower[0]：PA模式期望功率；
     * bPower[1]：NO PA模式期望功率。
     */
    VOS_UINT16 bPower[2];
    VOS_UINT8  gHighsChannel; /* 802.11g制式支持最大信道号，取值范围0~14。 */
    VOS_UINT8  gLowChannel;   /* 802.11g制式支持最小信道号，取值范围0~14。 */
    VOS_UINT8  rsv2[2];      /* 预留，暂未使用。 */
    /*
     * 802.11g制式下的期望功率，取值范围0~65535。
     * gPower[0]：PA模式期望功率；
     * gPower[1]：NO PA模式期望功率。
     */
    VOS_UINT16 gPower[2];
    VOS_UINT8  nHighsChannel; /* 802.11n制式支持最大信道号，取值范围0~14。 */
    VOS_UINT8  nLowChannel;   /* 802.11n制式支持最小信道号，取值范围0~14。 */
    VOS_UINT8  rsv3[2];     /* 预留，暂未使用。 */
    /*
     * 802.11n制式下的期望功率，取值范围0~65535。
     * nPower[0]：PA模式期望功率；
     * nPower[1]：NO PA模式期望功率。
     */
    VOS_UINT16 nPower[2];
} NV_WIFI_Info;


typedef struct {
    VOS_UINT16 platform; /* 子平台信息，取值范围0~65535。 */
    VOS_UINT8  reserve1; /* 保留 */
    VOS_UINT8  reserve2; /* 保留 */
} NAS_NVIM_Platform;


typedef struct {
    VOS_UINT16 eqver;    /* 装备归一化AT命令的版本号，取值范围0~65535。 */
    VOS_UINT8  reserve1; /* 保留 */
    VOS_UINT8  reserve2; /* 保留 */
} TAF_AT_EqVer;


typedef struct {
    /*
     * 数据卡定制版本号，取值范围0~65535。
     * 默认值由产品线定制。
     */
    VOS_UINT16 csver;
    VOS_UINT8  reserve1; /* 保留位 */
    VOS_UINT8  reserve2; /* 保留位 */
} TAF_NVIM_CsVer;


typedef struct {
    /*
     * 是否支持MUX特性。
     * 0：不支持（HILINK默认值）；
     * 1：支持。
     */
    VOS_UINT8 muxSupportFlg;
    VOS_UINT8 reserved1; /* 保留位。 */
    VOS_UINT8 reserved2; /* 保留位。 */
    VOS_UINT8 reserved3; /* 保留位。 */
} TAF_AT_NvimMuxSupportFlg;


typedef struct {
    VOS_UINT8 wifiAuthmode[AT_WIFI_WLAUTHMODE_LEN];                   /* 鉴权模式。 */
    VOS_UINT8 wifiBasicencryptionmodes[AT_WIFI_ENCRYPTIONMODES_LEN];  /* 基本加密模式。 */
    VOS_UINT8 wifiWpaencryptionmodes[AT_WIFI_ENCRYPTIONMODES_LEN];    /* WPA加密模式。 */
    VOS_UINT8 wifiWepKey1[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX]; /* WIFI KEY1。 */
    VOS_UINT8 wifiWepKey2[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX]; /* WIFI KEY2。 */
    VOS_UINT8 wifiWepKey3[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX]; /* WIFI KEY3。 */
    VOS_UINT8 wifiWepKey4[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX]; /* WIFI KEY4。 */
    VOS_UINT8 wifiWepKeyIndex[AT_WIFI_MAX_SSID_NUM]; /* 使用的WIFI KEY INDEX。例如，1代表使用wlKeys1 */
    VOS_UINT8 wifiWpapsk[AT_WIFI_MAX_SSID_NUM][AT_WIFI_WLWPAPSK_LEN]; /* WPA的密码。 */
    VOS_UINT8 wifiWpsenbl; /* WPS是否使能。0：不使能；1：使能 */
    VOS_UINT8 wifiWpscfg;  /* 是否允许寄存器改变enrollee的参数。0: 不允许(默认); 1:允许 */
} TAF_AT_MultiWifiSec;


typedef struct {
    /* 该数组表示四组WIFI SSID，每组最大长度为33。 */
    VOS_UINT8 wifiSsid[AT_WIFI_MAX_SSID_NUM][AT_WIFI_SSID_LEN_MAX];
    VOS_UINT8 reserved[84]; /* 保留 */
} TAF_AT_MultiWifiSsid;


typedef struct {
    /*
     * 打断参数有效标志。
     * 1：有效
     * 0：无效
     */
    VOS_UINT8 abortEnableFlg;
    VOS_UINT8 reserve1; /* 保留，值为0。 */
    VOS_UINT8 reserve2; /* 保留，值为0。 */
    VOS_UINT8 reserve3; /* 保留，值为0。 */
    /* 打断命令的命令名，AT输入此命令表示请求打断当前正执行的命令。需有字符串结束符号。 */
    VOS_UINT8 abortAtCmdStr[AT_MAX_ABORT_CMD_STR_LEN];
    /* 打断命令的返回结果，AT返回此结果表示打断操作完成。需有字符串结束符。 */
    VOS_UINT8 abortAtRspStr[AT_MAX_ABORT_RSP_STR_LEN];
} AT_NVIM_AbortCmdPara;


typedef struct {
    VOS_UINT32 muxReportCfg; /* MUX 端口状态，值0表示所有MUX端口均能够主动上报AT命令。 */
} TAF_AT_NvimMuxReportCfg;


typedef struct {
    /*
     * 控制是否允许某个或某些AT端口使用AT+CIMI命令查询IMSI。
     * Bit位为0：允许。
     * Bit位为1：不允许。
     */
    VOS_UINT32 cimiPortCfg;
} TAF_AT_NvimCimiPortCfg;


typedef struct {
    /*
     * NV项激活标志
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT8 status;
    /*
     * SS标准命令定制参数
     * BIT0：控制+CCWA查询命令是否输出完整业务状态。
     * 0：不输出；
     * 1：输出；
     * BIT1：控制+CLCK查询命令输出完整业务状态。
     * 0：不输出；
     * 1：输出；
     * 其他BIT位保留。
     */
    VOS_UINT8 ssCmdCustomize;
    VOS_UINT8 reserved1[2]; /* 保留 */
} AT_SS_CustomizePara;


typedef struct {
    /*
     * 是否使能Share PDP特性。
     * 1：使能；
     * 0：禁止。
     */
    VOS_UINT8 enableFlag;
    VOS_UINT8 reserved; /* 保留。 */
    /*
     * 老化时间，单位为秒(s)。
     * 在Share PDP特性使能的情况下，设置为0代表不老化，设置为非0值则代表老化周期。
     */
    VOS_UINT16 agingTimeLen;
} TAF_NVIM_SharePdpInfo;


typedef struct {
    /*
     * VCOM log的打印级别，分为INFO、NORM、ERR和DEBUG,其中：
     * INFO: 1
     * NORMAL:2
     * ERR:4
     * DEBUG:7
     */
    VOS_UINT32 appVcomDebugLevel;
    /*
     * APP VCOM 端口掩码，每1bit的值代表一个端口是否打印
     * 0: 不打印
     * 1: 打印
     */
    VOS_UINT32 appVcomPortIdMask1; /* VCOM端口ID掩码 */
    VOS_UINT32 appVcomPortIdMask2; /* VCOM端口ID掩码 */

    /*
     * DMS log的打印级别，分为INFO、WARNING、ERROR和DEBUG,其中：
     * INFO:    1
     * WARNING: 2
     * ERROR:   4
     * DEBUG:   7
     */
    VOS_UINT32 dmsDebugLevel;
    /*
     * DMS 端口掩码，每1bit的值代表一个端口是否打印
     * 0: 不打印
     * 1: 打印
     */
    VOS_UINT32 dmsPortIdMask1; /* 第0到31个bit分别对应DMS_PortId中的第0到第31个端口 */
    VOS_UINT32 dmsPortIdMask2; /* 第0到31个bit分别对应DMS_PortId中的第32到第63个端口 */
} TAF_NV_PortDebugCfg;


typedef struct {
    VOS_UINT8 macAddr[32]; /* MAC地址。 */
} OM_MAC_Addr;


typedef struct {
    /*
     * NV是否激活标识
     * *0：NV项未激活；
     *  1：NV项已激活。
     */
    VOS_UINT32 nvStatus;
    VOS_UINT8  productId[AT_PRODUCT_NAME_LENGHT]; /* 产品名称，ASIC编码方式，“\0”结束。 */
    VOS_UINT8  reserve1;                           /* 保留 */
    VOS_UINT8  reserve2;                           /* 保留 */
} TAF_AT_ProductId;


typedef struct {
    /*
     * NV是否有效标识
     * 0：有效；
     * 1：无效。
     */
    VOS_UINT8 status;
    /*
     * 拨号失败错误码上报标志。
     * 0：不上报，使用系统默认的拨号错误提示，使用AT^GLASTERR=1查询返回ERROR。
     * 1：上报，使用AT^GLASTERR=1查询本次拨号失败的错误码。
     */
    VOS_UINT8 errCodeRpt;
} NAS_NV_PppDialErrCode; /* 巴西TIM拨号错误码NV项结构体 */

/*
 * 标    识: NV_ITEM_HUAWEI_DYNAMIC_PID_TYPE(50091)
 * 结构说明: 用于获取当前设备的端口状态。
 */
typedef struct {
    /*
     * 本NV项是否激活。
     * 0：未激活；
     * 1：激活。
     */
    VOS_UINT32 nvStatus;
    VOS_UINT8  firstPortStyle[17];  /* 设备切换前端口形态, 取值范围0x00~0xFF。 */
    VOS_UINT8  rewindPortStyle[17]; /* 设备切换后端口形态, 取值范围0x00~0xFF。 */
    VOS_UINT8  reserved[22];        /* 保留位 */
} AT_DynamicPidType;


typedef struct {
    VOS_UINT8 webPwd[AT_WEBUI_PWD_MAX]; /* 字符串类型，WEBUI登录密码。 */
} TAF_AT_NvimWebAdminPassword;


typedef struct {
    /* 保存XML reporting服务器的URL，最后一个字节为结束符’\0’。 */
    VOS_UINT8 apRptSrvUrl[AT_AP_XML_RPT_SRV_URL_STR_LEN];
} TAF_AT_NvimApRptSrvUrl;


typedef struct {
    /* 保存XML reporting的信息类型，最后一个字节为结束符’\0’。 */
    VOS_UINT8 apXmlInfoType[AT_AP_NVIM_XML_RPT_INFO_TYPE_STR_LEN];
} TAF_AT_NvimApXmlInfoType;


typedef struct {
    /*
     * 0：关闭XML reporting特性；
     * 1：打开XML reporting特性。
     */
    VOS_UINT8 apXmlRptFlg;
    VOS_UINT8 reserve[3]; /* 保留位，未使用 */
} TAF_AT_NvimApXmlRptFlg;


typedef struct {
    /*
     * 整型值，控制当^IMSSWITCH命令未携带<utran_enable>参数时，此参数值是否与<lte_enable>关联，取值范围0~1。
     * 0：否
     * 1：是
     */
    VOS_UINT8 utranRelationLteFlg;
    /*
     * 整型值，控制当^IMSSWITCH命令未携带<gsm_enable>参数时，此参数值是否与<lte_enable>关联，取值范围0~1。
     * 0：否
     * 1：是
     */
    VOS_UINT8 gsmRelationLteFlg;
    VOS_UINT8 reserved1; /* 预留位。 */
    VOS_UINT8 reserved2; /* 预留位。 */
} TAF_NV_ImsSwitchRatRelationCfg;

typedef struct {
    VOS_UINT32 mcc;
    VOS_UINT32 mnc;
} TAF_NV_PLMN_Id;


typedef struct {
    /* 保存随卡匹配的运营商名称(说明:字符使用ASCII存储) */
    VOS_UINT8               operName[TAF_NVIM_USIM_DEPEND_OPER_NAME_MAX_LEN];
    /* 保存随卡匹配的运营商PLMN */
    TAF_NV_PLMN_Id          matchedPlmn;
    /*
     * 运营商定制NV生效方式
     * 0: 无需重启Modem或卡上下电， 1: NV生效需要Modem重启，2: NV生效需要卡上下电，默认:0
     */
    VOS_UINT8               effectiveWay;
    VOS_UINT8               errorCode;  /* 保存随卡过程中的错误码 */
    VOS_UINT8               reserved2;  /* 保留位 */
    VOS_UINT8               reserved3;  /* 保留位 */
    /* 保存正在做随卡匹配的运营商PLMN，
     * 在收到卡状态上报的时候如果此PLMN为有效值，说明之前的随卡流程还没有结束，
     * 在随卡成功后清除， */
    TAF_NV_PLMN_Id          recordPlmn;
} TAF_NVIM_UsimDependentResultInfo;

typedef struct {
    /* 远程唤醒特性控制开关 0:关闭，1:打开 */
    VOS_UINT8 remoteWakeupCtrl;
    /* 唤醒通道设置Bit0:wake up pin Bit[1-7]保留 */
    VOS_UINT8 remoteWakeupChannel;
    /*
     * 唤醒源设置:
     * Bit0:通话相关AT
     * Bit1:短信相关AT
     * Bit2:TCP/IP DATA
     * Bit3:unsolicited report
     * Bit4:GPS
     * Bit5:网络注册相关
     * Bit[6-15]:reserve
     */
    VOS_UINT16 remoteWakeupSource;
} TAF_NVIM_MbbWakeupCfg;

typedef struct {
    /* 手持模式上行音量 */
    VOS_INT16 handSetUpVolValue;
    /* 免提模式上行音量 */
    VOS_INT16 handFreeUpVolValue;
    /* 车载模式上行音量 */
    VOS_INT16 carUpVolValue;
    /* 耳机模式上行音量 */
    VOS_INT16 headSetUpVolValue;
    /* 蓝牙模式上行音量 */
    VOS_INT16 blueRoothUpVolValue;
    /* PC语言模式上行音量 */
    VOS_INT16 pcVoiceUpVolValue;
    /* 不带耳机MIC的耳机通话模式上行音量 */
    VOS_INT16 headPhoneUpVolValue;
    VOS_INT16 reserved[5];
} TAF_NVIM_MbbCmicVolumeCfg;

enum TAF_DHCP_SUBNET_MaskMode {
    TAF_DHCP_IPV4_CLASS_SUBNET  = 0x00, /* a、b、c、类子网掩码模式 */
    TAF_DHCP_IPV4_MIN_SUBNET = 0x01, /* 最小子网掩码模式 */
    TAF_DHCP_IPV4_FIXED_SUBNET = 0x02, /* 固定子网掩码模式 */

    TAF_DHCP_IPV4_SUBNET_MASK_MODE_BUTT /* 无效值 */
};
typedef VOS_UINT8 TAF_DHCP_SubnetMaskModeUint8;


typedef struct {
    /*
     * 禁止STK短信临时TR
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 smsNoFastTrFlg;
    /*
     * 软银定制，仅接收PP-Download短信
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 smsPpDownloadOnlyFlg;
    /*
     * 车载产品定制，紧急呼叫释放普通呼叫
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 callEmcRelNormalCallFlg;
    /*
     * 飞行模式下允许发起紧急呼叫
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 callEmcDisableMiniFlg;
    /*
     * 车载定制，来电RING上报时长
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 callRingWaitingTimeCustFlg;
    /*
     * 海外禁止IPV6注册
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 disableIpv6Foreign;
    /*
     * profile自适配定制
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 profileMatchFlg;
    /*
     * stick拨号特性
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 dialStickFlg;
    /*
     * 秘鲁Claro VOIP APN KEY定制
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 voipApnKey;
    /*
     * dtmf定制
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 dtmfCustFlag;
    /*
     * 补充业务可中断
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 ssAbortEnable;
    /*
     * 短信业务可中断
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 smsAbortEnable;
    /*
     * A核休眠不唤醒定制
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 standbyNoReportFlag;
    /*
     * DHCP子网掩码模式定制
     * 0: a、b、c类子网掩码， 1:最小子网掩码，2:固定子网掩码
     */
    TAF_DHCP_SubnetMaskModeUint8 dhcpSubNetMaskMode;
    /*
     * AppVcom回显特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 vcomEchoFlag;
    /*
     * 电源管理特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 powerManagerSwitch;
    /*
     * 车载音频特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 telematicAudioFlag;
    /*
     * 车载PCM语音特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 wpgPcmFlag;
    /*
     * 延迟上报conf特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 confFlag;
    /*
     * 来电不区分CS IMS特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 ccwaFlag;
    /*
     * end携带endSource特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 cendFlag;
    /*
     * clip参数上报修改特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 clipFlag;
    /*
     * auto hold特性控制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 autoHoldFlag;
    VOS_UINT8 rsv1;
    /*
     * KDDI定制simlock受限名单机制
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 simLockBlockListFlg;
    /*
     * EONS主动上报特性开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 eonsRptFlg;
    /*
     *  HCSQ主动上报门限定制特性开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 hcsqDeltaFlg;
    /*
     * 由syscfgex 设置band 变化后，重新入网定制
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 syscfgReAttachFlg;
    /*
     * 指定搜恢复CSPS ATTACH ALLOW 标记
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 specSrchCsPsAllowFlg;
    /*
     * 协议多子网适配特性定制开关
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 subnetAdapterFlg;
    VOS_UINT8 phonePwrOffDelayTime; /* tmode=4关机延迟时间, 使用时将数值乘以10然后作为毫秒用 */
    VOS_UINT8 nwTimeQryFlg; /* 系统时间获取定制 */
    VOS_UINT8 cfunSetFlg; /* CFUN设置值保存定制 */
    VOS_UINT8 mmrsv8;
    VOS_UINT8 mmrsv9;
    VOS_UINT8 mmrsv10;
    VOS_UINT8 mmrsv11;
    VOS_UINT8 mmrsv12;
    VOS_UINT8 mmrsv13;
    VOS_UINT8 mmrsv14;
} TAF_NVIM_MbbFeatureCfg;


typedef struct {
    /*
     * MT短信可信名单
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 mtSmsTrustListEnable;
    /*
     * MT来电可信名单
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 mtCallTrustListEnable;
    /*
     * 来电不可信名单
     * 0: 特性关闭， 1:特性开启
     */
    VOS_UINT8 trustBlockNumEnable;
    VOS_UINT8 method;                /* 设置AT命令读取EID的方式,使用M2M或者消费者产品模式 */
    VOS_UINT8 reserve[124];          /* 124个保留位，4字节对齐 */
} TAF_NVIM_MtListAndEidCtrl;

/* 可信名单号码结构 */
typedef struct {
    VOS_UINT8 index;                             /* 号码index */
    VOS_UINT8 reserved;
    VOS_UINT8 number[TAF_TRUSTLIST_NUM_MAX_LEN]; /* 号码信息 */
} TAF_NVIM_NumType;


typedef struct {
    TAF_NVIM_NumType recordList[TAF_TRUSTLIST_MAX_LIST_SIZE];
} TAF_NVIM_MtCallNumTrustList;


typedef struct {
    TAF_NVIM_NumType recordList[TAF_TRUSTLIST_MAX_LIST_SIZE];
} TAF_NVIM_MtSmsNumTrustList;


typedef struct {
    /*
     * profile 生效状态
     * 0: 信息无效， 1:信息有效
     */
    VOS_UINT8 activeFlag;
    /*
     * PDP类型
     * 1:TAF_PDP_IPV4
     * 2:TAF_PDP_IPV6
     * 3:TAF_PDP_IPV4V6
     */
    VOS_UINT8 pdpType;
    /*
     * 鉴权类型
     * 0:TAF_PDP_AUTH_TYPE_NONE
     * 1:TAF_PDP_AUTH_TYPE_PAP
     * 2:TAF_PDP_AUTH_TYPE_CHAP
     */
    VOS_UINT8 authType;
    VOS_UINT8 imsiPrefixLen; /* 匹配IMSI位数 */
    /*
     * 匹配的IMSI前缀的BCD码
     * 如46050应预制为:[0x46, 0x05, 0x00,]
     */
    VOS_UINT8 imsiPrefixBcd[TAF_NVIM_MAX_APNRETRY_BCD_IMSI_LEN];
    VOS_UINT8 apnLen; /* APN长度 */
    VOS_UINT8 apn[TAF_NVIM_MAX_APNRETRY_APN_LEN]; /* 匹配的APN */
    VOS_UINT8 userNameLen; /* 用户名长度 */
    VOS_UINT8 userName[TAF_NVIM_MAX_APNRETRY_USERNAME_LEN]; /* 匹配的APN用户名 */
    VOS_UINT8 pwdLen; /* 密码长度 */
    VOS_UINT8 pwd[TAF_NVIM_MAX_APNRETRY_PASSWORD_LEN]; /* 匹配的APN密码 */
    VOS_UINT8 profileNameLen; /* Profile名称长度 */
    VOS_UINT8 profileName[TAF_NVIM_MAX_APNRETRY_PRONAME_LEN]; /* Profile名称 */
} TAF_NVIM_LteAttachProfileInfo;


typedef struct {
    VOS_UINT8 useFlag; /* 有效位 0:数据无效 1:数据有效 */
    VOS_UINT8 reserved[3]; /* 预留字节，用于四字节对齐 */
    VOS_UINT8 apnKey[MAX_ENCRPT_APN_KEY_LENGTH]; /* 加密后的密码,长度112 */
    VOS_UINT8 iv[IV_LEN_VOIP]; /* 加密使用的IV，长度16 */
} TAF_NVIM_ApnKeyList;


typedef struct {
    VOS_UINT8                   sku;                    /* SKU值 */
    VOS_UINT8                   profileId;              /* 对应的Profile ID */
    VOS_UINT8                   rsv[2];
} TAF_NVIM_NvSkuProfileInfo;


typedef struct {
    /*
     * 特性总开关
     * 0:特性未激活
     * 1:特性激活
     */
    VOS_UINT8                   activeFlag;
    /*
     * 匹配状态标记
     * 0:未匹配
     * 1:匹配
     */
    VOS_UINT8                   matchedFlag;
    VOS_UINT8                   skuNum;            /* 有效记录个数 */
    VOS_UINT8                   baseProfileId;     /* 基准Profile ID, 其存储版本中配置的默认Profile1的内容 */
    TAF_NVIM_NvSkuProfileInfo   skuProfileInfo[TAF_MAX_SKU_PROFILE_INFO_NUM]; /* SKU和Profile Id的对应关系 */
} TAF_NVIM_NvProfileMatchCfg;


typedef struct {
    /*
     * 应用层短信满的标记
     * 0:未满
     * 1:已满
     */
    VOS_UINT8 smsFullFlg;
    VOS_UINT8 sendSmmaFlg; /* 是否需要发送SMMA */
    VOS_UINT8 rsv[2]; /* 保留 */
} TAF_NVIM_NvAppSmsFullFlag;


enum NAPI_WEIGHT_AdjMode {
    NAPI_WEIGHT_ADJ_STATIC_MODE  = 0x00, /* 静态调整模式 */
    NAPI_WEIGHT_ADJ_DYNAMIC_MODE = 0x01, /* 动态调整模式 */

    NAPI_WEIGHT_ADJ_MODE_BUTT            /* 无效值 */
};
typedef VOS_UINT8 NAPI_WEIGHT_AdjModeUint8;


typedef struct {
    VOS_UINT32 dlPktNumPerSecLevel1; /* RNIC网卡每秒下行报文数档位1 */
    VOS_UINT32 dlPktNumPerSecLevel2; /* RNIC网卡每秒下行报文数档位2 */
    VOS_UINT32 dlPktNumPerSecLevel3; /* RNIC网卡每秒下行报文数档位3 */
    VOS_UINT32 dlPktNumPerSecLevel4; /* RNIC网卡每秒下行报文数档位4 */
    VOS_UINT32 dlPktNumPerSecLevel5; /* RNIC网卡每秒下行报文数档位5 */
    VOS_UINT32 reserved0;            /* 保留位 */
    VOS_UINT32 reserved1;            /* 保留位 */
    VOS_UINT32 reserved2;            /* 保留位 */

} RNIC_NAPI_DlPktNumPerSecLevel;


typedef struct {
    VOS_UINT8 napiWeightLevel1;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位1 */
    VOS_UINT8 napiWeightLevel2;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位2 */
    VOS_UINT8 napiWeightLevel3;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位3 */
    VOS_UINT8 napiWeightLevel4;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位4 */
    VOS_UINT8 napiWeightLevel5;  /* RNIC网卡对应每秒下行报文数NAPI Weight值档位5 */
    VOS_UINT8 reserved0;         /* 保留位 */
    VOS_UINT8 reserved1;         /* 保留位 */
    VOS_UINT8 reserved2;         /* 保留位 */

} RNIC_NAPI_WeightLevel;


typedef struct {
    RNIC_NAPI_DlPktNumPerSecLevel dlPktNumPerSecLevel; /* RNIC网卡每秒下行报文数档位 */
    RNIC_NAPI_WeightLevel         napiWeightLevel;     /* RNIC网卡对应每秒下行报文数NAPI Weight值档位 */

} RNIC_NAPI_WeightDynamicAdjCfg;


typedef struct {
    /*
     * RNIC网卡下行数据到Linux网络协议栈的接口模式：
     * 0：Net_rx接口
     * 1：NAPI接口
     */
    VOS_UINT8 napiEnable;
    /*
     * NAPI Weight调整模式：
     * 0：静态模式
     * 1：动态调整模式
     */
    NAPI_WEIGHT_AdjModeUint8      napiWeightAdjMode;
    /* RNIC网卡NAPI方式一次poll的最大报文数 */
    VOS_UINT8                     napiPollWeight;
    /*
     * GRO开启：
     * 0：不开启
     * 1：开启
     */
    VOS_UINT8                     groEnable;
    VOS_UINT16                    napiPollQueMaxLen;       /* RNIC Poll队列支持的最大长度 */
    VOS_UINT8                     reserved1;               /* 保留位 */
    VOS_UINT8                     reserved2;               /* 保留位 */
    RNIC_NAPI_WeightDynamicAdjCfg napiWeightDynamicAdjCfg; /* Napi Weight动态调整配置 */

} TAF_NV_RnicNapiCfg;


typedef struct {
    VOS_UINT32 pps;                                      /* RNIC网卡下行每秒收包个数档位值 */
    VOS_UINT8  cpusWeight[RNIC_NVIM_NAPI_LB_MAX_CPUS];   /* 参与NAPI负载均衡的CPU Weight值档位 */

} RNIC_NAPI_LbLevelCfg;


typedef struct {
    VOS_UINT8            napiLbEnable;                                 /* NAPI负载均衡功能开关 */
    VOS_UINT8            napiLbValidLevel;                             /* Napi负载均衡有效档位 */
    VOS_UINT16           napiLbCpumask;                                /* 参与Napi负载均衡的CPU掩码 */
    VOS_UINT8            reserved0;                                    /* 保留位 */
    VOS_UINT8            reserved1;                                    /* 保留位 */
    VOS_UINT8            reserved2;                                    /* 保留位 */
    VOS_UINT8            reserved3;                                    /* 保留位 */
    VOS_UINT32           reserved4;                                    /* 保留位 */
    VOS_UINT32           reserved5;                                    /* 保留位 */
    RNIC_NAPI_LbLevelCfg napiLbLevelCfg[RNIC_NVIM_NAPI_LB_MAX_LEVEL]; /* Napi负载均衡档位配置 */

} TAF_NV_RnicNapiLbCfg;


typedef struct {
    VOS_UINT32 totalPps;                       /* RNIC网卡下行每秒收包个数档位值 */
    VOS_UINT32 nonTcpPps;                      /* RNIC网卡下行每秒收非tcp包个数档位值 */
    VOS_UINT32 backlogQueLimitLen;             /* CPU backlog que限长 */
    VOS_UINT8  congestionCtrl;                 /* 拥塞控制开关 */
    VOS_UINT8  rpsBitMask;                     /* RPS CPU掩码 */
    VOS_UINT8  reserved1;                      /* 保留位 */
    VOS_UINT8  isolationDisable;               /* 关闭isolation标识 */
    VOS_INT32  ddrBd;                          /* DDR带宽档位 */
    VOS_UINT32 freqReq[RNIC_NVIM_MAX_CLUSTER]; /* 请求调整CLUSTER主频值 */

} RNIC_RhcLevel;


typedef struct {
    VOS_UINT8     rhcFeature;                        /* 接收高速控制特性开关 */
    VOS_UINT8     qccEnable;                         /* NAPI队列拥塞控制功能开关 */
    VOS_UINT8     napiWtExpEnable;                   /* NAPI队列拥塞控制Weight扩张功能开关 */
    VOS_UINT8     ddrReqEnable;                      /* 根据NAPI队列拥塞程度给DDR投票功能开关 */
    VOS_UINT8     qccTimeout;                        /* NAPI队列拥塞控制持续时长,单位:100ms */
    VOS_UINT8     napiPollMax;                       /* NAPI poll最大连续执行次数 */
    VOS_UINT8     reserved0;                         /* 保留位 */
    VOS_UINT8     reserved1;                         /* 保留位 */
    VOS_INT32     ddrMidBd;                          /* 中档位DDR带宽 */
    VOS_INT32     ddrHighBd;                         /* 高档位DDR带宽 */
    RNIC_RhcLevel levelCfg[RNIC_NVIM_RHC_MAX_LEVEL]; /* 接收高速控制档位配置 */

} TAF_NV_RnicRhcCfg;


typedef struct {
    VOS_INT16 level[TAF_NVIM_RSRP_CFG_LEVEL_LEN];   /* rsrp门限值级别 */
    VOS_INT16 value[TAF_NVIM_RSRP_CFG_VALUE_LEN];   /* 对应级别的门限值 */
} TAF_NVIM_LteRsrpCfg;


typedef struct {
    VOS_INT16 level[TAF_NVIM_ECIO_CFG_LEVEL_LEN];   /* ecio门限值级别 */
    VOS_INT16 value[TAF_NVIM_ECIO_CFG_VALUE_LEN];   /* 对应级别的门限值 */
} TAF_NVIM_LteEcioCfg;


typedef struct {
    VOS_INT16 level[TAF_NVIM_RSCP_CFG_LEVEL_LEN];   /* rscp门限值级别 */
    VOS_INT16 value[TAF_NVIM_RSCP_CFG_VALUE_LEN];   /* 对应级别的门限值 */
} TAF_NVIM_LteRscpCfg;


typedef struct
{
    VOS_UINT32  AuthVer;             /* 鉴权版本号，终端确认默认值。 */
}TAF_NVIM_AuthVer;

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*****************************************************************************
 结构名    : TAF_NVIM_SysCfgExRatGroupCtrl
 结构说明  : 控制SYSCFGEX设置/查询的接入技术组合
*****************************************************************************/
typedef struct {
    /*
     * NV配置的制式组合个数。
     * 取值范围: 0-10
     */
    VOS_UINT8  itemNum;
    /*
     * 是否限制SYSCFGEX命令下发的制式组合。
     * 取值范围:
     * 0: 不限制
     * 1: 限制
     */
    VOS_UINT8  restrictFlg;
    /*
     * 定制的制式组合列表，最大支持10组，每组最多配置6个制式，当每组配置不足6个时，第一个无效制式需要配置为无效值。
     * 取值范围：
     * 0x00: 自动模式
     * 0x01: GSM
     * 0x02: WCDMA
     * 0x03: LTE
     * 0x04: 1xRTT
     * 0x07: EVDO
     * 0x08: NR
     * 0x99: NO CHANGE
     * 0xFF: 无效值
     */
    VOS_UINT8  ratGroupList[TAF_NVIM_SYSCFGEX_RAT_GROUP_MAX][TAF_NVIM_SYSCFGEX_RAT_NUM_MAX];
    VOS_UINT8  rsv[2];                 /* 保留 */
}TAF_NVIM_SysCfgExRatGroupCtrl;

/*****************************************************************************
 结构名    : NvAutoRatInfo
 结构说明  : AUTO模式代表的制式信息
*****************************************************************************/
typedef struct {
    VOS_UINT8 ratNum;                               /* AUTO代表的制式个数 */
    VOS_UINT8 ratList[TAF_NVIM_AUTO_MODE_RAT_MAX];  /* AUTO代表的制式列表 */
}TAF_NVIM_AutoModeRatInfo;

/*****************************************************************************
 结构名    : NvIccidTransMode
 结构说明  : ICCID查询时的传输模式
*****************************************************************************/
typedef struct {
    VOS_UINT8                   transMode;                                  /* 0: ICCID截断模式; 1: 透传模式 */
    VOS_UINT8                   rsv1;
    VOS_UINT8                   rsv2;
    VOS_UINT8                   rsv3;
}TAF_NVIM_IccidTransMode;
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __NV_STRU_GUCNAS_H__ */

