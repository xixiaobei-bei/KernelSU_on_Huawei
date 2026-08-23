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

#include "vos.h"
#include "nv_id_gucnas.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#pragma pack(push, 4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define TAF_NVIM_SN_LEN                                     (20)
#define TAF_NVIM_MAX_OPER_NAME_SERVICE_PRIO_NUM             (4)
#define TAF_NV_BLACK_LIST_MAX_NUM                           (51)
#define TAF_NVIM_MAX_USER_SYS_CFG_RAT_NUM                   (5)
#define TAF_MAX_MFR_ID_LEN                                  (31)
#define TAF_MAX_MFR_ID_STR_LEN                              (TAF_MAX_MFR_ID_LEN + 1)
#define TAF_PH_PRODUCT_NAME_LEN                             (15)
#define TAF_PH_PRODUCT_NAME_STR_LEN                         (TAF_PH_PRODUCT_NAME_LEN + 1)
#define MN_MSG_SRV_PARAM_LEN                                (8)                 /* 短信业务参数结构与NV项中存储的长度 */
#define TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN             (160)
#define TAF_SVN_DATA_LENGTH                                 (2)                 /* SVN有效数据长度 */
#define TAF_PH_NVIM_MAX_GUL_RAT_NUM                         (3)                 /*AT^syscfgex中acqorder代表的接入技术个数 */
#define TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM              (20)                /* 9130扩展IPv6回退处理扩展原因值最大个数 */
#define PLATFORM_MAX_RAT_NUM                                (7)                 /* 接入技术最大值 */
#define MTA_BODY_SAR_WBAND_MAX_NUM                          (5)
#define MTA_BODY_SAR_GBAND_MAX_NUM                          (4)
#define TAF_PH_SIMLOCK_PLMN_STR_LEN                         (8)                 /* Plmn 号段长度 */
#define TAF_MAX_SIM_LOCK_RANGE_NUM                          (20)

/* WINS可配置NV项的结构体 */
#define WINS_CONFIG_DISABLE                                 (0)                 /* WINS不使能 */
#define WINS_CONFIG_ENABLE                                  (1)                 /* WINS使能 */

typedef VOS_UINT32  MMA_QUICK_START_STA_UINT32;
/* 对NVID枚举的转定义(PS_NV_ID_ENUM, SYS_NV_ID_ENUM, RF_NV_ID_ENUM) */
typedef VOS_UINT16  NV_ID_ENUM_U16;

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
 枚举名    : SYSTEM_APP_CONFIG_TYPE_ENUM
 结构说明  : 上层对接应用类型枚举
*****************************************************************************/
enum SYSTEM_APP_CONFIG_TYPE_ENUM
{
    SYSTEM_APP_MP                       = 0,                /*上层应用是Mobile Partner*/
    SYSTEM_APP_WEBUI,                                       /*上层应用是Web UI*/
    SYSTEM_APP_ANDROID,                                     /*上层应用是Android*/
    SYSTEM_APP_BUTT
};
typedef VOS_UINT16 SYSTEM_APP_CONFIG_TYPE_ENUM_UINT16;


enum TAF_NVIM_LC_RAT_COMBINED_ENUM
{
    TAF_NVIM_LC_RAT_COMBINED_GUL  = 0x55,
    TAF_NVIM_LC_RAT_COMBINED_CL   = 0xAA,
    TAF_NVIM_LC_RAT_COMBINED_BUTT
};
typedef VOS_UINT8 TAF_NVIM_LC_RAT_COMBINED_ENUM_UINT8;


enum TAF_NVIM_LC_WORK_CFG_ENUM
{
    TAF_NVIM_LC_INDEPENT_WORK = 0,
    TAF_NVIM_LC_INTER_WORK    = 1,
    TAF_NVIM_LC_WORK_CFG_BUTT
};
typedef VOS_UINT8 TAF_NVIM_LC_WORK_CFG_ENUM_UINT8;

/*****************************************************************************
 枚举名    : TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM
 枚举说明  :
*****************************************************************************/
enum TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM
{
    TAF_MMA_CFREQ_LOCK_MODE_OFF         = 0x00,     /* 禁止锁频功能 */
    TAF_MMA_CFREQ_LOCK_MODE_ON          = 0x01,     /* 启动锁频功能 */
    TAF_MMA_CFREQ_LOCK_MODE_BUTT        = 0x02
};
typedef VOS_UINT8 TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM_UINT8;


enum TAF_LSMS_RESEND_FLAG_ENUM
{
    TAF_LSMS_RESEND_FLAG_DISABLE          = 0,  /* 禁止重发 */
    TAF_LSMS_RESEND_FLAG_ENABLE           = 1,  /* 使能重发 */

    TAF_LSMS_RESEND_FLAG_BUTT
};
typedef VOS_UINT8 TAF_LSMS_RESEND_FLAG_ENUM_UINT8;


enum TAF_NV_ACTIVE_MODEM_MODE_ENUM
{
    TAF_NV_ACTIVE_SINGLE_MODEM              = 0x00,
    TAF_NV_ACTIVE_MULTI_MODEM               = 0x01,

    TAF_NV_ACTIVE_MODEM_MODE_BUTT
};
typedef VOS_UINT8 TAF_NV_ACTIVE_MODEM_MODE_ENUM_UINT8;


enum TAF_NV_GPS_CHIP_TYPE_ENUM
{
    TAF_NV_GPS_CHIP_BROADCOM          = 0,
    TAF_NV_GPS_CHIP_HISI1102          = 1,

    TAF_NV_GPS_CHIP_BUTT
};
typedef VOS_UINT8 TAF_NV_GPS_CHIP_TYPE_ENUM_UINT8;

/* ME Storage Function On or Off*/
enum MN_MSG_ME_STORAGE_STATUS_ENUM
{
    MN_MSG_ME_STORAGE_DISABLE           = 0x00,
    MN_MSG_ME_STORAGE_ENABLE            = 0x01,
    MN_MSG_ME_STORAGE_BUTT
};
typedef VOS_UINT8 MN_MSG_ME_STORAGE_STATUS_ENUM_UINT8;


enum PLATFORM_RAT_TYPE_ENUM
{
    PLATFORM_RAT_GSM,                                                           /*GSM接入技术 */
    PLATFORM_RAT_WCDMA,                                                         /* WCDMA接入技术 */
    PLATFORM_RAT_LTE,                                                           /* LTE接入技术 */
    PLATFORM_RAT_TDS,                                                           /* TDS接入技术 */
    PLATFORM_RAT_1X,                                                            /* CDMA-1X接入技术 */
    PLATFORM_RAT_HRPD,                                                          /* CDMA-EV_DO接入技术 */
    PLATFORM_RAT_BUTT
};
typedef VOS_UINT16 PLATFORM_RAT_TYPE_ENUM_UINT16;


enum MTA_WCDMA_BAND_ENUM
{
    MTA_WCDMA_I_2100                    = 0x0001,
    MTA_WCDMA_II_1900,
    MTA_WCDMA_III_1800,
    MTA_WCDMA_IV_1700,
    MTA_WCDMA_V_850,
    MTA_WCDMA_VI_800,
    MTA_WCDMA_VII_2600,
    MTA_WCDMA_VIII_900,
    MTA_WCDMA_IX_J1700,
    /* 频段暂不支持
    MTA_WCDMA_X,
    */
    MTA_WCDMA_XI_1500                   = 0x000B,
    /* 以下频段暂不支持
    MTA_WCDMA_XII,
    MTA_WCDMA_XIII,
    MTA_WCDMA_XIV,
    MTA_WCDMA_XV,
    MTA_WCDMA_XVI,
    MTA_WCDMA_XVII,
    MTA_WCDMA_XVIII,
    */
    MTA_WCDMA_XIX_850                   = 0x0013,

    MTA_WCDMA_BAND_BUTT
};
typedef VOS_UINT16 MTA_WCDMA_BAND_ENUM_UINT16;


enum TAF_NVIM_RAT_MODE_ENUM
{
    TAF_NVIM_RAT_MODE_GSM               = 0x01,
    TAF_NVIM_RAT_MODE_WCDMA,
    TAF_NVIM_RAT_MODE_LTE,
    TAF_NVIM_RAT_MODE_CDMA1X,
    TAF_NVIM_RAT_MODE_TDSCDMA,
    TAF_NVIM_RAT_MODE_WIMAX,
    TAF_NVIM_RAT_MODE_EVDO,

    TAF_NVIM_RAT_MODE_BUTT
};
typedef VOS_UINT8 TAF_NVIM_RAT_MODE_ENUM_UINT8;


enum TAF_NVIM_GSM_BAND_ENUM
{
    TAF_NVIM_GSM_BAND_850               = 0,
    TAF_NVIM_GSM_BAND_900,
    TAF_NVIM_GSM_BAND_1800,
    TAF_NVIM_GSM_BAND_1900,

    TAF_NVIM_GSM_BAND_BUTT
};
typedef VOS_UINT16 TAF_NVIM_GSM_BAND_ENUM_UINT16;


enum NV_MS_MODE_ENUM
{
    NV_MS_MODE_CS_ONLY,                                                         /* 仅支持CS域 */
    NV_MS_MODE_PS_ONLY,                                                         /* 仅支持PS域 */
    NV_MS_MODE_CS_PS,                                                           /* CS和PS都支持 */

    NV_MS_MODE_ANY,                                                             /* ANY,相当于仅支持CS域 */

    NV_MS_MODE_BUTT
};
typedef VOS_UINT8 NV_MS_MODE_ENUM_UINT8;


/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           aucSerialNumber[TAF_NVIM_SN_LEN];
}TAF_NVIM_SERIAL_NUM_STRU;


typedef struct
{
    VOS_UINT8                           ucSvlteSupportFlag;                     /* SVLTE功能是否支持:0-不支持；1-支持 */
    VOS_UINT8                           aucReserved[3];                         /* 保留 */
}SVLTE_SUPPORT_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           ucLCEnableFlg;
    TAF_NVIM_LC_RAT_COMBINED_ENUM_UINT8 enRatCombined;
    TAF_NVIM_LC_WORK_CFG_ENUM_UINT8     enLCWorkCfg;
    VOS_UINT8                           aucReserved[1];
}TAF_NV_LC_CTRL_PARA_STRU;


typedef struct
{
    SYSTEM_APP_CONFIG_TYPE_ENUM_UINT16  usSysAppConfigType;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_NVIM_SYSTEM_APP_CONFIG_STRU;


typedef struct
{
    TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM_UINT8                 enFreqLockMode;
    VOS_UINT8                                               aucReserve[3];
    VOS_UINT16                                              usSid;
    VOS_UINT16                                              usNid;
    VOS_UINT16                                              usCdmaBandClass;
    VOS_UINT16                                              usCdmaFreq;
    VOS_UINT16                                              usCdmaPn;
    VOS_UINT16                                              usEvdoBandClass;
    VOS_UINT16                                              usEvdoFreq;
    VOS_UINT16                                              usEvdoPn;
}TAF_NVIM_CFREQ_LOCK_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucInternationalRoamEmcNotSelIms;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT8                           ucReserved4;
    VOS_UINT8                           ucReserved5;
    VOS_UINT8                           ucReserved6;
    VOS_UINT8                           ucReserved7;
}TAF_NVIM_SPM_EMC_CUSTOM_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucLteSmsEnable;                         /* LTE发送3GPP2短信特性控制 VOS_TURE:使能,VOS_FALSE:禁止 */
    TAF_LSMS_RESEND_FLAG_ENUM_UINT8     enResendFlag;                           /* 重发使能标志 */
    VOS_UINT8                           ucResendMax;                            /* 重试最大次数，默认最大1次 */
    VOS_UINT8                           ucResendInterval;                       /* 重新发送等待时间，单位: 秒，默认30秒 */
}TAF_NVIM_LTE_SMS_CFG_STRU;


typedef struct
{
    TAF_NV_ACTIVE_MODEM_MODE_ENUM_UINT8 enActiveModem;
    VOS_UINT8                           aucReserve[3];
}TAF_NV_DSDS_ACTIVE_MODEM_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucOperNameServicePrioNum;

    /* 1代表支持PNN，cosp或^eonsucs2查询时检查PNN文件查找匹配的运营商名称;
       2代表支持CPHS,cops或^eonsucs2查询时检查CPHS文件查找匹配的运营商名称;
       3代表支持MM INFO,cops或^eonsucs2查询时从mm/gmm/emm information中查找匹配的运营商名称;
       4代表支持SPN,cops或^eonsucs2查询时检查SPN文件查找匹配的运营商名称*/
    VOS_UINT8                           aucOperNameSerivcePrio[TAF_NVIM_MAX_OPER_NAME_SERVICE_PRIO_NUM];
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT8                           ucPlmnCompareSupportWildCardFlag;       /* plmn比较时是否支持通配符 */
    VOS_UINT8                           ucWildCard;                             /* 通配符,取值a-f,可代表0-9任意数字 */
    VOS_UINT8                           ucReserved4;
    VOS_UINT8                           ucReserved5;                            /* 控制内部测试命令^usimstub和^refreshstub命令是否可用*/
}TAF_NVIM_ENHANCED_OPER_NAME_SERVICE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucSecType;
    VOS_UINT8                           ucBlackListNum;
    VOS_UINT16                          ausBlackList[TAF_NV_BLACK_LIST_MAX_NUM];
} TAF_NV_NVWR_SEC_CTRL_STRU;


typedef struct
{
    VOS_UINT8                           ucCcpuResetFlag;                        /* 0:不是C核单独复位，而是上电 */
                                                                                /* 1:是C核单独复位 */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_CCPU_RESET_RECORD_STRU;


typedef struct
{
    VOS_UINT8                           ucFilterEnableFlg;
    VOS_UINT8                           aucReserved[3];
} NAS_NV_PRIVACY_FILTER_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucXcposrRptNvCfg;                       /* 是否上报清除GPS缓存的辅助定位信息 */
    VOS_UINT8                           ucCposrDefault;
    VOS_UINT8                           ucXcposrDefault;
    VOS_UINT8                           ucXcposrRptDefault;
}TAF_NVIM_XCPOSRRPT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucRatOrderNum;                                      /* syscfgex中设置的acqoder中的指示个数 */
    VOS_UINT8                           aenRatOrder[TAF_NVIM_MAX_USER_SYS_CFG_RAT_NUM];     /* syscfgex中设置的acqoder类型 */
    VOS_UINT8                           aucReserved[2];
}TAF_NVIM_MULTIMODE_RAT_CFG_STRU;


typedef struct
{
    TAF_NV_GPS_CHIP_TYPE_ENUM_UINT8     enGpsChipType;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
}TAF_NVIM_GPS_CUST_CFG_STRU;


typedef struct
{
    MMA_QUICK_START_STA_UINT32          ulQuickStartSta;
}NAS_NVIM_FOLLOWON_OPENSPEED_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           aucMfrId[TAF_MAX_MFR_ID_STR_LEN];
}TAF_PH_FMR_ID_STRU;


/*为了与V1R1NV配匹，前13个字节保留*/
typedef struct
{
    VOS_UINT8                           aucRsv[15];                             /* NV项的结构中，4字节对齐方式下有空洞要手动补齐 */
    VOS_UINT8                           ucS0TimerLen;
} TAF_CCA_TELE_PARA_STRU;


typedef struct
{
    VOS_UINT8                           aucProductName[TAF_PH_PRODUCT_NAME_STR_LEN];
}TAF_PH_PRODUCT_NAME_STRU;


typedef struct
{
    VOS_UINT8                           aucAccessMode[2];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMA_NVIM_ACCESS_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucMsClass;
    VOS_UINT8                           ucReserved;
}NAS_NVIM_MS_CLASS_STRU;


typedef struct
{
    VOS_UINT8                           aucSmsServicePara[MN_MSG_SRV_PARAM_LEN];
}TAF_NVIM_SMS_SERVICE_PARA_STRU;


typedef struct
{
    VOS_UINT8                           ucRoamFeatureFlg;                       /*记录漫游特性是否激活,VOS_FALSE:不激活,VOS_TRUE:激活*/
    VOS_UINT8                           ucRoamCapability;                       /*记录用户设置的漫游属性*/
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_NVIM_ROAM_CFG_INFO_STRU;

/*en_NV_Item_CustomizeSimLockPlmnInfo 8267*/

typedef struct
{
    VOS_UINT8                           ucMncNum;
    VOS_UINT8                           aucRangeBegin[TAF_PH_SIMLOCK_PLMN_STR_LEN];
    VOS_UINT8                           aucRangeEnd[TAF_PH_SIMLOCK_PLMN_STR_LEN];
}TAF_CUSTOM_SIM_LOCK_PLMN_RANGE_STRU;


typedef struct
{
    VOS_UINT32                          ulStatus;                               /*是否激活，0不激活，1激活 */
    TAF_CUSTOM_SIM_LOCK_PLMN_RANGE_STRU astSimLockPlmnRange[TAF_MAX_SIM_LOCK_RANGE_NUM];
}TAF_CUSTOM_SIM_LOCK_PLMN_INFO_STRU;

/* en_NV_Item_CardlockStatus 8268 */

typedef struct
{
    VOS_UINT32                          ulStatus;                               /*是否激活，0不激活，1激活 */
    VOS_UINT32                          ulCardlockStatus;
    VOS_UINT32                          ulRemainUnlockTimes;                    /*解锁剩余次数*/
}TAF_NVIM_CUSTOM_CARDLOCK_STATUS_STRU;

/*en_NV_Item_CustomizeSimLockMaxTimes 8269*/

typedef struct
{
    VOS_UINT32                          ulStatus;                               /*是否激活，0不激活，1激活 */
    VOS_UINT32                          ulLockMaxTimes;
}TAF_CUSTOM_SIM_LOCK_MAX_TIMES_STRU;


typedef struct
{
    VOS_UINT32                          ulStatus;                               /*是否激活，0不激活，1激活 */
    VOS_UINT32                          ulCustomizeService;                     /*终端说明书是1个byte，为了没有空洞，扩充成4byte，高3byte保留*/
}NAS_NVIM_CUSTOMIZE_SERVICE_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /* 1: NV有效标志位，0：无效 */
    VOS_UINT8                           ucWins;                                 /* WINS使能标记: 0：Disable,  1：Enable */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}WINS_CONFIG_STRU;


typedef struct
{
    VOS_INT8                            cStatus;
    VOS_UINT8                           ucEncodeType;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT32                          ulLength;
    VOS_UINT8                           ucData[TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN];
}TAF_AT_NVIM_SMS_ACTIVE_MESSAGE_STRU;


typedef struct
{
    VOS_UINT8                           ucActiveFlag;
    VOS_UINT8                           aucSvn[TAF_SVN_DATA_LENGTH];
    VOS_UINT8                           aucReserve[1];
}TAF_SVN_DATA_STRU;


typedef struct
{
    VOS_UINT16                          usPcVoiceSupportFlag;                   /* Range:[0,1] */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}APP_VC_NVIM_PC_VOICE_SUPPORT_FLAG_STRU;


typedef struct
{
    MN_MSG_ME_STORAGE_STATUS_ENUM_UINT8 enMeStorageStatus;                      /* ME存储短信功能是否启用标志*/
    VOS_UINT8                           aucReserve[1];                          /* NV项相关的结构体，在4字节方式下，需手动补齐空洞 */
    VOS_UINT16                          usMeStorageNum;                         /* ME存储短信容量 */
}MN_MSG_ME_STORAGE_PARM_STRU;


typedef struct
{
    VOS_UINT8                           ucRatOrderNum;                          /* syscfgex中设置的acqoder中的指示个数 */
    VOS_UINT8                           aenRatOrder[TAF_PH_NVIM_MAX_GUL_RAT_NUM];    /* at^syscfgex中设置的acqoder的顺序 */
}TAF_PH_NVIM_RAT_ORDER_STRU;


typedef struct
{
    VOS_UINT8                           ucLteRoamAllowedFlg;
    VOS_UINT8                           aucReserve[1];
    VOS_UINT8                           aucRoamEnabledMccList[20];              /* 允许漫游的国家码列表 */
}NAS_MMC_NVIM_LTE_INTERNATIONAL_ROAM_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /*是否激活，0不激活，1激活 */
    VOS_UINT8                           ucActFlg;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_PREVENT_TEST_IMSI_REG_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlag;
    VOS_UINT8                           enMtCustomize;
    VOS_UINT8                           aucMtCustomize[2];                      /* MT定制流程配置. [0]: download mt流程中是否延时发送stk短信 */
}MN_MSG_MT_CUSTOMIZE_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucSignThreshold;                        /* 信号变化门限,当RSSI变化超过该值，接入层需要主动上报信号质量，取值0表示接入层按默认值处理 */
    VOS_UINT8                           ucMinRptTimerInterval;                  /* 间隔上报的时间 */
    VOS_UINT8                           ucRserved1;
    VOS_UINT8                           ucRserved2;
} NAS_NVIM_CELL_SIGN_REPORT_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulActiveFlag;
    VOS_UINT8                           aucSmCause[TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM];
} TAF_NV_IPV6_FALLBACK_EXT_CAUSE_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimValid;
    VOS_UINT8                           ucSmsClosePathFlg;
} TAF_NVIM_SMS_CLOSE_PATH_CFG_STRU;


typedef struct
{
    VOS_UINT16                           usRatNum;                              /* 接入技术的数目*/
    PLATFORM_RAT_TYPE_ENUM_UINT16        aenRatList[PLATFORM_MAX_RAT_NUM];      /* 接入技术 */
}PLATAFORM_RAT_CAPABILITY_STRU;


typedef struct
{
    VOS_INT16                           sGPRSPower;                             /* GPRS功率门限值 */
    VOS_INT16                           sEDGEPower;                             /* EDGE功率门限值 */
}MTA_BODY_SAR_G_PARA_STRU;


typedef struct
{
    MTA_WCDMA_BAND_ENUM_UINT16          enBand;                                 /* WCDMA频段 */
    VOS_INT16                           sPower;                                 /* 功率门限值 */
}MTA_BODY_SAR_W_PARA_STRU;


typedef struct
{
    VOS_UINT32                          ulGBandMask;                                /* GSM Band的Mask位 */
    VOS_UINT16                          usWBandNum;                                 /* WCDMA的Band个数 */
    VOS_UINT16                          ausReserved1[1];                            /* 保留位 */
    MTA_BODY_SAR_G_PARA_STRU            astGBandPara[MTA_BODY_SAR_GBAND_MAX_NUM];   /* GSM功率门限值 */
    MTA_BODY_SAR_W_PARA_STRU            astWBandPara[MTA_BODY_SAR_WBAND_MAX_NUM];   /* WCDMA功率门限值 */
}MTA_BODY_SAR_PARA_STRU;


typedef struct
{
    VOS_UINT8                           ucECallNotifySupport;                   /* 紧急呼叫与温度保护交互NV项 */
    VOS_UINT8                           ucSpyStatusIndSupport;                  /* 温保状态上报使能NV项 */
    VOS_UINT8                           aucReserved[2];                         /* 保留项 */
}TAF_TEMP_PROTECT_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                          ucSmsDomain;
    VOS_UINT8                          aucReserved[3];
} TAF_NVIM_SMS_DOMAIN_STRU;


typedef struct
{
    VOS_UINT8                           ucMode;                                 /* JAM设置的模式，0:关闭, 1:打开 */
    VOS_UINT8                           ucMethod;                               /* JAM检测使用的方案，1:方案1；2:方案2,目前只支持2 */
    VOS_UINT8                           ucFreqNum;                              /* 检测需要达到的频点个数，取值范围:[0,255] */
    VOS_UINT8                           ucThreshold;                            /* 检测需要达到的频点测量值，取值范围:[0,70] */
    VOS_UINT8                           ucJamDetectingTmrLen;                   /* 强信号频点个数满足门限后，检查同步结果的定时器时长(s) */
    VOS_UINT8                           ucJamDetectedTmrLen;                    /* 检测到干扰后，等待搜网结果的最长时间(s) */
    VOS_UINT8                           ucFastReportFlag;                       /* 搜网过程中是否需要快速上报 */
    VOS_UINT8                           ucRxlevThresholdOffset;                 /* G模噪音门限配置偏移值 */
}NV_NAS_JAM_DETECT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* 1:锁频功能打开，0:功能关闭 */
    TAF_NVIM_RAT_MODE_ENUM_UINT8        enRatMode;
    TAF_NVIM_GSM_BAND_ENUM_UINT16       enBand;
    VOS_UINT32                          ulLockedFreq;
} TAF_NVIM_FREQ_LOCK_CFG_STRU;

/*****************************************************************************
 结构名    : AT_TRAFFIC_CLASS_CUSTOMIZE_STRU
 结构说明  : en_NV_Item_TRAFFIC_CLASS_Type(50009)  用于定制PDP激活请求中QoS中 Traffic Class的值的结构体
*****************************************************************************/
typedef struct
{
    VOS_UINT8                          ucStatus;                                /* 1: NV有效标志位，0：无效 */
    VOS_UINT8                          ucTrafficClass;                          /* Traffic Class的值 */
    VOS_UINT8                          ucReserve1;
    VOS_UINT8                          ucReserve2;
}AT_TRAFFIC_CLASS_CUSTOMIZE_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /* 1: NV有效标志位，0：无效 */
    VOS_UINT8                           ucUssdTransMode;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_USSD_NVIM_TRANS_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlg;                               /* NVIM中该项是否激活 */
    VOS_UINT8                           enClass0Tailor;
}MN_MSG_NVIM_CLASS0_TAILOR_STRU;


typedef struct
{
    /* 此NV 用来设置呼叫等待的控制模式，ulCcwaCtrlMode为0，则呼叫等待由3gpp网络控制；
       ulCcwaCtrlMode为1，则呼叫等待由UE控制，用于VOLTE的网络。
      （在VoLTE的网络上，AP配置CCWA支持时，IMS并没有和网络交互，VoLTE的电话的CCWA由UE控制）。
       支持VOLTE的终端，不管VOLTE开关有没有打开，都是下发CCWAI来设置呼叫等待。
    */
    VOS_UINT8                           enCcwaCtrlMode;                         /* TAF_CALL_CCWA_CTRL_MODE_ENUM_U8 */
    VOS_UINT8                           enCcwaiMode;                            /* TAF_CALL_CCWAI_MODE_ENUM_UINT8 */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
} TAF_CALL_NVIM_CCWA_CTRL_MODE_STRU;


#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __NV_STRU_GUCNAS_H__ */


