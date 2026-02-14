/******************************************************************************

    Copyright(C)2008,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       : RrcNvInterface.h
  Description     : RrcNvInterface.h header file
  History           :
     1.Name+ID      yyyy-mm-dd  Draft Enact
******************************************************************************/

#ifndef __RRCNVINTERFACE_H__
#define __RRCNVINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include  "PsTypeDef.h"

#pragma pack(4)

/*****************************************************************************
  2 macro
*****************************************************************************/
#define RRC_MAX_NUM_OF_BANDS                    (64)
#define RRC_MAX_NUM_OF_FEATURE_GROUP            (32)
#define RRC_MAX_NUM_OF_CDMA_BAND_CLASS          (32)

#define LTE_COMM_NONSTANDARD_BAND_BEGIN         (101)

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/
/*****************************************************************************
 枚举名    : LRRC_LTE_MODE_TYPE_ENUM_UINT16
 协议表格  :
 ASN.1描述 :
 枚举说明  : LTE模式类型
*****************************************************************************/
enum LTE_COMM_BAND_MODE_TYPE_ENUM
{
    LTE_COMM_BAND_MODE_TYPE_FDD         = 0,            /* LTE FDD 制式 */
    LTE_COMM_BAND_MODE_TYPE_TDD         = 1,            /* LTE TDD 制式 */
    LTE_COMM_BAND_MODE_TYPE_BUTT
};
typedef VOS_UINT8 LTE_COMM_BAND_MODE_TYPE_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CATEGORY_ENUM_UINT8
 协议表格  :
 ASN.1描述 :
 枚举说明  : UE Category
*****************************************************************************/
enum RRC_UE_CATEGORY_ENUM
{
    RRC_UE_CATEGORY_1                   = 0,
    RRC_UE_CATEGORY_2                   = 1,
    RRC_UE_CATEGORY_3                   = 2,
    RRC_UE_CATEGORY_4                   = 3,
    RRC_UE_CATEGORY_5                   = 4,
    RRC_UE_CATEGORY_BUTT
};
typedef VOS_UINT8 RRC_UE_CATEGORY_ENUM_UINT8;

typedef VOS_UINT8 RRC_UE_CAP_CDMA2000_BANDCLASS_ENUM_UINT8;
/*****************************************************************************
 枚举名    : RRC_UE_CAP_ROH_CONTEXT_ENUM_UINT8
 协议表格  :
 ASN.1描述 :
 枚举说明  : maxNumberROHC-ContextSessions
*****************************************************************************/
enum RRC_UE_CAP_ROH_CONTEXT_ENUM
{
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS2    = 0,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS4    ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS8    ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS12   ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS16   ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS24   ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS32   ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS48   ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS64   ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS128  ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS256  ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS512  ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS1024 ,
    RRC_MAX_NUM_ROHC_CTX_SESSION_CS16384,
    RRC_MAX_NUM_ROHC_CTX_SESSION_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_ROH_CONTEXT_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_UTRA_FDD_BAND_ENUM_UINT8
 协议表格  :
 ASN.1描述 :
 枚举说明  : utra-FDD-Band
*****************************************************************************/
enum RRC_UE_CAP_UTRA_FDD_BAND_ENUM
{
    RRC_UE_CAP_UTRA_FDD_BAND_I          = 0,
    RRC_UE_CAP_UTRA_FDD_BAND_II         = 1,
    RRC_UE_CAP_UTRA_FDD_BAND_III        = 2,
    RRC_UE_CAP_UTRA_FDD_BAND_IV         = 3,
    RRC_UE_CAP_UTRA_FDD_BAND_V          = 4,
    RRC_UE_CAP_UTRA_FDD_BAND_VI         = 5,
    RRC_UE_CAP_UTRA_FDD_BAND_VII        = 6,
    RRC_UE_CAP_UTRA_FDD_BAND_VIII       = 7,
    RRC_UE_CAP_UTRA_FDD_BAND_IX         = 8,
    RRC_UE_CAP_UTRA_FDD_BAND_X          = 9,
    RRC_UE_CAP_UTRA_FDD_BAND_XI         = 10,
    RRC_UE_CAP_UTRA_FDD_BAND_XII        = 11,
    RRC_UE_CAP_UTRA_FDD_BAND_XIII       = 12,
    RRC_UE_CAP_UTRA_FDD_BAND_XIV        = 13,
    RRC_UE_CAP_UTRA_FDD_BAND_XV         = 14,
    RRC_UE_CAP_UTRA_FDD_BAND_XVI        = 15,
    RRC_UE_CAP_UTRA_FDD_BAND_XVII       = 16,
    RRC_UE_CAP_UTRA_FDD_BAND_XVIII      = 17,
    RRC_UE_CAP_UTRA_FDD_BAND_XIX        = 18,
    RRC_UE_CAP_UTRA_FDD_BAND_XX         = 19,
    RRC_UE_CAP_UTRA_FDD_BAND_XXI        = 20,
    RRC_UE_CAP_UTRA_FDD_BAND_XXII       = 21,
    RRC_UE_CAP_UTRA_FDD_BAND_XXIII      = 22,
    RRC_UE_CAP_UTRA_FDD_BAND_XXIV       = 23,
    RRC_UE_CAP_UTRA_FDD_BAND_XXV        = 24,
    RRC_UE_CAP_UTRA_FDD_BAND_XXVI       = 25,
    RRC_UE_CAP_UTRA_FDD_BAND_XXVII      = 26,
    RRC_UE_CAP_UTRA_FDD_BAND_XXVIII     = 27,
    RRC_UE_CAP_UTRA_FDD_BAND_XXIX       = 28,
    RRC_UE_CAP_UTRA_FDD_BAND_XXX        = 29,
    RRC_UE_CAP_UTRA_FDD_BAND_XXXI       = 30,
    RRC_UE_CAP_UTRA_FDD_BAND_XXXII      = 31,
    RRC_UE_CAP_UTRA_FDD_BAND_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_UTRA_FDD_BAND_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_UTRA_TDD128_BAND_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : utra-TDD128Band
*****************************************************************************/
enum RRC_UE_CAP_UTRA_TDD128_BAND_ENUM
{
    RRC_UE_CAP_UTRA_TDD128_BAND_A       = 0,
    RRC_UE_CAP_UTRA_TDD128_BAND_B       = 1,
    RRC_UE_CAP_UTRA_TDD128_BAND_C       = 2,
    RRC_UE_CAP_UTRA_TDD128_BAND_D       = 3,
    RRC_UE_CAP_UTRA_TDD128_BAND_E       = 4,
    RRC_UE_CAP_UTRA_TDD128_BAND_F       = 5,
    RRC_UE_CAP_UTRA_TDD128_BAND_G       = 6,
    RRC_UE_CAP_UTRA_TDD128_BAND_H       = 7,
    RRC_UE_CAP_UTRA_TDD128_BAND_I       = 8,
    RRC_UE_CAP_UTRA_TDD128_BAND_J       = 9,
    RRC_UE_CAP_UTRA_TDD128_BAND_K       = 10,
    RRC_UE_CAP_UTRA_TDD128_BAND_L       = 11,
    RRC_UE_CAP_UTRA_TDD128_BAND_M       = 12,
    RRC_UE_CAP_UTRA_TDD128_BAND_N       = 13,
    RRC_UE_CAP_UTRA_TDD128_BAND_O       = 14,
    RRC_UE_CAP_UTRA_TDD128_BAND_P       = 15,
    RRC_UE_CAP_UTRA_TDD128_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_UTRA_TDD128_BAND_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_UTRA_TDD384_BAND_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : utra-TDD384Band
*****************************************************************************/
enum RRC_UE_CAP_UTRA_TDD384_BAND_ENUM
{
    RRC_UE_CAP_UTRA_TDD384_BAND_A       = 0,
    RRC_UE_CAP_UTRA_TDD384_BAND_B       = 1,
    RRC_UE_CAP_UTRA_TDD384_BAND_C       = 2,
    RRC_UE_CAP_UTRA_TDD384_BAND_D       = 3,
    RRC_UE_CAP_UTRA_TDD384_BAND_E       = 4,
    RRC_UE_CAP_UTRA_TDD384_BAND_F       = 5,
    RRC_UE_CAP_UTRA_TDD384_BAND_G       = 6,
    RRC_UE_CAP_UTRA_TDD384_BAND_H       = 7,
    RRC_UE_CAP_UTRA_TDD384_BAND_I       = 8,
    RRC_UE_CAP_UTRA_TDD384_BAND_J       = 9,
    RRC_UE_CAP_UTRA_TDD384_BAND_K       = 10,
    RRC_UE_CAP_UTRA_TDD384_BAND_L       = 11,
    RRC_UE_CAP_UTRA_TDD384_BAND_M       = 12,
    RRC_UE_CAP_UTRA_TDD384_BAND_N       = 13,
    RRC_UE_CAP_UTRA_TDD384_BAND_O       = 14,
    RRC_UE_CAP_UTRA_TDD384_BAND_P       = 15,
    RRC_UE_CAP_UTRA_TDD384_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_UTRA_TDD384_BAND_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_UTRA_TDD768_BAND_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : utra-TDD768Band
*****************************************************************************/
enum RRC_UE_CAP_UTRA_TDD768_BAND_ENUM
{
    RRC_UE_CAP_UTRA_TDD768_BAND_A       = 0,
    RRC_UE_CAP_UTRA_TDD768_BAND_B       = 1,
    RRC_UE_CAP_UTRA_TDD768_BAND_C       = 2,
    RRC_UE_CAP_UTRA_TDD768_BAND_D       = 3,
    RRC_UE_CAP_UTRA_TDD768_BAND_E       = 4,
    RRC_UE_CAP_UTRA_TDD768_BAND_F       = 5,
    RRC_UE_CAP_UTRA_TDD768_BAND_G       = 6,
    RRC_UE_CAP_UTRA_TDD768_BAND_H       = 7,
    RRC_UE_CAP_UTRA_TDD768_BAND_I       = 8,
    RRC_UE_CAP_UTRA_TDD768_BAND_J       = 9,
    RRC_UE_CAP_UTRA_TDD768_BAND_K       = 10,
    RRC_UE_CAP_UTRA_TDD768_BAND_L       = 11,
    RRC_UE_CAP_UTRA_TDD768_BAND_M       = 12,
    RRC_UE_CAP_UTRA_TDD768_BAND_N       = 13,
    RRC_UE_CAP_UTRA_TDD768_BAND_O       = 14,
    RRC_UE_CAP_UTRA_TDD768_BAND_P       = 15,
    RRC_UE_CAP_UTRA_TDD768_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_UTRA_TDD768_BAND_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_GERAN_BAND_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : geran-Band
*****************************************************************************/
enum RRC_UE_CAP_GERAN_BAND_ENUM
{
    RRC_UE_CAP_GSM450_GERAN_BAND = 0,
    RRC_UE_CAP_GSM480_GERAN_BAND,
    RRC_UE_CAP_GSM710_GERAN_BAND,
    RRC_UE_CAP_GSM750_GERAN_BAND,
    RRC_UE_CAP_GSM810_GERAN_BAND,
    RRC_UE_CAP_GSM850_GERAN_BAND,
    RRC_UE_CAP_GSM900P_GERAN_BAND,
    RRC_UE_CAP_GSM900E_GERAN_BAND,
    RRC_UE_CAP_GSM900R_GERAN_BAND,
    RRC_UE_CAP_GSM1800_GERAN_BAND,
    RRC_UE_CAP_GSM1900_GERAN_BAND,
    RRC_UE_CAP_GERAN_BAND_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_GERAN_BAND_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_CDMA2000_HRPD_TX_CONFIG_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : cdma2000-HRPD-TxConfig
*****************************************************************************/
enum RRC_UE_CAP_CDMA2000_HRPD_TX_CONFIG_ENUM
{
    RRC_UE_CAP_CDMA2000_HRPD_TX_SINGLE  = 0,
    RRC_UE_CAP_CDMA2000_HRPD_TX_DUAL    = 1,
    RRC_UE_CAP_CDMA2000_HRPD_TX_BUTT
} ;
typedef VOS_UINT8 RRC_UE_CAP_CDMA2000_HRPD_TX_CONFIG_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_CDMA2000_HRPD_RX_CONFIG_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : cdma2000-HRPD-RxConfig
*****************************************************************************/
enum RRC_UE_CAP_CDMA2000_HRPD_RX_CONFIG_ENUM
{
    RRC_UE_CAP_CDMA2000_HRPD_RX_SINGLE  = 0,
    RRC_UE_CAP_CDMA2000_HRPD_RX_DUAL    = 1,
    RRC_UE_CAP_CDMA2000_HRPD_RX_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_CDMA2000_HRPD_RX_CONFIG_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_CDMA2000_1X_RTT_TX_CONFIG_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : cdma2000-1xRTT-TxConfig
*****************************************************************************/
enum RRC_UE_CAP_CDMA2000_1X_RTT_TX_CONFIG_ENUM
{
    RRC_UE_CAP_CDMA2000_1X_RTT_TX_SINGLE = 0,
    RRC_UE_CAP_CDMA2000_1X_RTT_TX_DUAL = 1,
    RRC_UE_CAP_CDMA2000_1X_RTT_TX_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_CDMA2000_1X_RTT_TX_CONFIG_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_CDMA2000_1X_RTT_RX_CONFIG_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : cdma2000-1xRTT-RxConfig
*****************************************************************************/
enum RRC_UE_CAP_CDMA2000_1X_RTT_RX_CONFIG_ENUM
{
    RRC_UE_CAP_CDMA2000_1X_RTT_RX_SINGLE = 0,
    RRC_UE_CAP_CDMA2000_1X_RTT_RX_DUAL   = 1,
    RRC_UE_CAP_CDMA2000_1X_RTT_RX_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_CDMA2000_1X_RTT_RX_CONFIG_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : 用来指示特性是否支持
*****************************************************************************/
enum RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM
{
    RRC_UE_CAP_FEATURE_SUPPORTED     = 0,
    RRC_UE_CAP_FEATURE_NOT_SUPPORTED = 1,
    RRC_UE_CAP_FEATURE_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8;

/*****************************************************************************
 枚举名    : RRC_UE_CAP_DEVICE_TYPE_R9_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : 用来指示是否获取NW-based省电优化
*****************************************************************************/
enum RRC_UE_CAP_DEVICE_TYPE_R9_ENUM
{
    RRC_NO_BEN_FROM_BAT_CONSUMP_OPT   = 0,
    RRC_BEN_FROM_BAT_CONSUMP_OPT      = 1,
    RRC_BAT_CONSUMP_OPT_BUTT
};
typedef VOS_UINT8 RRC_UE_CAP_DEVICE_TYPE_R9_ENUM_UINT8;

/*****************************************************************************
  5 STRU
*****************************************************************************/
/* add by lishangfeng for NorStandard begin */
/*****************************************************************************
 结构名    : LTE_NON_STANDARD_BAND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 非标频段的频点信息相关的数据结构,与36101 talbe 5.7.3-1对应

            RealFreq = usFLow + (RealArfcn - usRangOfNLow),
            其中 RealFreq 真实的频点信息；usRangOfNLow、usFLow从NV项中获取，具体含义
            见数据结构中的说明; RealArfcn实际的arfcn。
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                usFLow;                           /* 起始频点，单位是100khz */
    VOS_UINT16                                usFHigh;                          /* 结束频点，单位是100khz */
    VOS_UINT32                                ulRangOfNLow;                     /* Nlow,与Noffset一致 */
    VOS_UINT32                                ulRangOfNHigh;                    /* Nhigh*/
}LTE_COMM_NON_STANDARD_FREQ_INFO_STRU;

/*****************************************************************************
 结构名    : LTE_NON_STANDARD_POWER_CLASS_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 非标频段power class相关的数据结构
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                 usUEPowerClass;
    VOS_INT16                                  sPowerToleranceHigh;    /* 单位 0.1dBm */
    VOS_INT16                                  sPowerToleranceLow;     /* 单位 0.1dBm */
    VOS_UINT8                                  aucReservered[2];
}LTE_COMM_NON_STANDARD_POWER_CLASS_INFO_STRU;

/*****************************************************************************
 结构名    : LTE_NON_STANDARD_BAND_TEST_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 非标频段的测试信息相关的数据结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                   ucBandWidth;                       /* 带宽，单位0.1MHz，可选值14;30;50;150;200 */
    VOS_UINT8                                   aucReservered[3];
    VOS_UINT16                                  usTestFreqs[3];                    /* 测试低、中、高频点，单位0.1MHz */
    VOS_INT16                                   sRxRefSensPower;                   /* 下行测试灵敏度功率，单位0.1dBm */
    VOS_UINT32                                  aulReservered[16];
}LTE_NON_STANDARD_BAND_TEST_INFO_STRU;

/*****************************************************************************
 结构名    : LTE_NON_STANDARD_BAND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 非标频段的频点信息相关的数据结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                   ucBandID;                       /* band iD */
    LTE_COMM_BAND_MODE_TYPE_ENUM_UINT8          enBandMode;
    VOS_INT16                                   sTxRxFreqSepara;
    LTE_COMM_NON_STANDARD_FREQ_INFO_STRU        stUlFreqInfo;
    LTE_COMM_NON_STANDARD_FREQ_INFO_STRU        stDlFreqInfo;
    LTE_COMM_NON_STANDARD_POWER_CLASS_INFO_STRU stUePowerClassInfo;
    LTE_NON_STANDARD_BAND_TEST_INFO_STRU        stTestInfo[6];                 /*1.4MHz；3MHz；5MHz,10MHz,15MHz；20MHz带宽的测试参数 */
    VOS_UINT32                                  aulReservered[16];
}LTE_COMM_NON_STANDARD_BAND_COMM_STRU;

/* add by lishangfeng for NorStandard end */

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_ROH_CPROFILES_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中PDCP->ROHC参数
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                  enProfile0x0001;
    PS_BOOL_ENUM_UINT8                  enProfile0x0002;
    PS_BOOL_ENUM_UINT8                  enProfile0x0003;
    PS_BOOL_ENUM_UINT8                  enProfile0x0004;
    PS_BOOL_ENUM_UINT8                  enProfile0x0006;
    PS_BOOL_ENUM_UINT8                  enProfile0x0101;
    PS_BOOL_ENUM_UINT8                  enProfile0x0102;
    PS_BOOL_ENUM_UINT8                  enProfile0x0103;
    PS_BOOL_ENUM_UINT8                  enProfile0x0104;
    VOS_UINT8                           aucReserved[3];
}RRC_UE_CAP_SUPP_ROH_CPROFILES_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_PDCP_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中PDCP参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucReserved[3];
    RRC_UE_CAP_ROH_CONTEXT_ENUM_UINT8   enMaxNumRohcContextSessions;
    RRC_UE_CAP_SUPP_ROH_CPROFILES_STRU  stPdcpSuppRohCprofiles;
} RRC_UE_CAP_PDCP_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_PDCP_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中PHY参数
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                  enUeTxAntennaSelectionSupp;
    PS_BOOL_ENUM_UINT8                  enUeSpecRefSigsSupp;
    VOS_UINT8                           aucReserved[2];
} RRC_UE_CAP_PHY_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_EUTRA_BAND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中RF->Band参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucEutraBand;
    PS_BOOL_ENUM_UINT8                  enHalfDuplex;
    VOS_UINT8                           aucReserved[2];
} RRC_UE_CAP_SUPP_EUTRA_BAND_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_RF_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中RF参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucReserved[2];
    VOS_UINT16                          usCnt;
    RRC_UE_CAP_SUPP_EUTRA_BAND_STRU     astSuppEutraBandList[RRC_MAX_NUM_OF_BANDS];
} RRC_UE_CAP_RF_PARA_STRU;
/*****************************************************************************
 结构名    : RRC_UE_CAP_MEAS_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中MEAS参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucReserved[2];
    VOS_UINT16                          usCnt;
    VOS_UINT32                          aulUeCapMeasIdx[RRC_MAX_NUM_OF_BANDS];
} RRC_UE_CAP_MEAS_PARA_STRU;
/*****************************************************************************
 结构名    : RRC_UE_CAP_FEATURE_GROUP_IND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中特性组指示，aucFeatureInd[]: 1－支持，0－不支持，
             数组元素0指示Group1,元素1指示Group2,依次类推

*****************************************************************************/
typedef struct
{
    VOS_UINT8                     aucFeatureInd[RRC_MAX_NUM_OF_FEATURE_GROUP];
} RRC_UE_CAP_FEATURE_GROUP_IND_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_UTRA_FDD_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedUTRA-FDD-BandListItem
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_UTRA_FDD_BAND_ENUM_UINT8 enUtraFddBand;
    VOS_UINT8                           aucReserved[3];
} RRC_UE_CAP_SUPP_UTRA_FDD_BAND_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_UTRA_FDD_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中INTER RAT->UTRA FDD参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucReserved[2];
    VOS_UINT16                          usCnt;
    RRC_UE_CAP_SUPP_UTRA_FDD_BAND_STRU  astSuppUtraFddBandList[RRC_MAX_NUM_OF_BANDS];
} RRC_IRAT_UTRA_FDD_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_UTRA_TDD128_BAND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedUTRA-TDD128BandListItem
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_UTRA_TDD128_BAND_ENUM_UINT8        enUtraTdd128Band;
    VOS_UINT8                                     aucReserved[3];
} RRC_UE_CAP_SUPP_UTRA_TDD128_BAND_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_UTRA_TDD128_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedUTRA-TDD128BandList
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                     aucReserved[2];
    VOS_UINT16                                    usCnt;
    RRC_UE_CAP_SUPP_UTRA_TDD128_BAND_STRU         astSuppUtraTdd128BandList[RRC_MAX_NUM_OF_BANDS];
} RRC_IRAT_UTRA_TDD128_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_UTRA_TDD384_BAND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedUTRA-TDD384BandListItem
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_UTRA_TDD384_BAND_ENUM_UINT8        enUtraTdd384Band;
    VOS_UINT8                                     aucReserved[3];
} RRC_UE_CAP_SUPP_UTRA_TDD384_BAND_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_UTRA_TDD384_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedUTRA-TDD384BandList
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                     aucReserved[2];
    VOS_UINT16                                    usCnt;
    RRC_UE_CAP_SUPP_UTRA_TDD384_BAND_STRU         astSuppUtraTdd384BandList[RRC_MAX_NUM_OF_BANDS];
} RRC_IRAT_UTRA_TDD384_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_UTRA_TDD768_BAND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedUTRA-TDD768BandListItem
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_UTRA_TDD768_BAND_ENUM_UINT8        enUtraTdd768Band;
    VOS_UINT8                                     aucReserved[3];
} RRC_UE_CAP_SUPP_UTRA_TDD768_BAND_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_UTRA_TDD768_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedUTRA-TDD768BandList
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                     aucReserved[2];
    VOS_UINT16                                    usCnt;
    RRC_UE_CAP_SUPP_UTRA_TDD768_BAND_STRU         astSuppUtraTdd768BandList[RRC_MAX_NUM_OF_BANDS];
} RRC_IRAT_UTRA_TDD768_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_GERAN_BAND_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedGERAN-BandListItem
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_GERAN_BAND_ENUM_UINT8    enGeranBand;
    VOS_UINT8                           aucReserved[3];
} RRC_UE_CAP_SUPP_GERAN_BAND_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_GERAN_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : IRAT-GERAN-Parameters
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucReserved;
    PS_BOOL_ENUM_UINT8                  enInterRatPsHoToGeran;
    VOS_UINT16                          usCnt;
    RRC_UE_CAP_SUPP_GERAN_BAND_STRU     astSuppGeranBandList[RRC_MAX_NUM_OF_BANDS];
} RRC_IRAT_GERAN_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_HRPD_BAND_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SupportedHRPD-BandList
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucReserved[2];
    VOS_UINT16                          usCnt;
    RRC_UE_CAP_CDMA2000_BANDCLASS_ENUM_UINT8      aenCdma2000HrpdBand[RRC_MAX_NUM_OF_CDMA_BAND_CLASS];
} RRC_UE_CAP_SUPP_HRPD_BAND_INFO_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_CDMA2000_HRPD_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : IRAT-CDMA2000-HRPD-Parameters
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                     aucReserved[2];
    RRC_UE_CAP_CDMA2000_HRPD_TX_CONFIG_ENUM_UINT8 enCdma2000HrpdTxCfg;
    RRC_UE_CAP_CDMA2000_HRPD_RX_CONFIG_ENUM_UINT8 enCdma2000HrpdRxCfg;
    RRC_UE_CAP_SUPP_HRPD_BAND_INFO_STRU           stSuppHrpdBandList;
} RRC_IRAT_CDMA2000_HRPD_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_SUPP_1X_RTT_BAND_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : Supported1xRTT-BandList
*****************************************************************************/
typedef struct
{
    VOS_UINT8                             aucReserved[2];
    VOS_UINT16                            usCnt;
    RRC_UE_CAP_CDMA2000_BANDCLASS_ENUM_UINT8      aenCdma20001xRttBand[RRC_MAX_NUM_OF_CDMA_BAND_CLASS];
} RRC_UE_CAP_SUPP_1X_RTT_BAND_INFO_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_CDMA2000_1X_RTT_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : IRAT-CDMA2000-1xRTT-Parameters
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                           aucReserved[2];
    RRC_UE_CAP_CDMA2000_1X_RTT_TX_CONFIG_ENUM_UINT8     enCdma20001xRttTxCfg;
    RRC_UE_CAP_CDMA2000_1X_RTT_RX_CONFIG_ENUM_UINT8     enCdma20001xRttRxCfg;
    RRC_UE_CAP_SUPP_1X_RTT_BAND_INFO_STRU               stSupp1xRttBandList;
} RRC_IRAT_CDMA2000_1X_RTT_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_UE_CAP_INTER_RAT_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE EUTRA能力中INTER RAT参数
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitCdma2000HrpdPresent     : 1;
    VOS_UINT32                          bitCdma20001xRttPresent    : 1;
    VOS_UINT32                          bitSpare                   : 30;

    RRC_IRAT_UTRA_FDD_PARA_STRU         stUtraFdd;
    RRC_IRAT_UTRA_TDD128_PARA_STRU      stUtraTdd128;
    RRC_IRAT_UTRA_TDD384_PARA_STRU      stUtraTdd384;
    RRC_IRAT_UTRA_TDD768_PARA_STRU      stUtraTdd768;
    RRC_IRAT_GERAN_PARA_STRU            stGeran;
    RRC_IRAT_CDMA2000_HRPD_PARA_STRU    stCdma2000Hrpd;
    RRC_IRAT_CDMA2000_1X_RTT_PARA_STRU  stCdma20001xRtt;
} RRC_UE_CAP_INTER_RAT_PARA_STRU;

/*****************************************************************************
 结构名    : RRC_PHY_LAYER_PARAM_V920_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : PhyLayerParameters-v920
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enEnhncDualLayerFddR9;
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enEnhncDualLayerTddR9;
    VOS_UINT8                                     aucReserved[2];
}RRC_PHY_LAYER_PARAM_V920_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_PARAM_GERAN_V920_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : IRAT-ParametersGERAN-v920
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enDtmR9;
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enEReDirGeranR9;
    VOS_UINT8                                     aucReserved[2];
}RRC_IRAT_PARAM_GERAN_V920_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_PARAM_UTRA_V920_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : IRAT-ParametersUTRA-v920
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enEReDirUtraR9;
    VOS_UINT8                                     aucReserved[3];
}RRC_IRAT_PARAM_UTRA_V920_STRU;

/*****************************************************************************
 结构名    : RRC_IRAT_PARAM_CDMA2000_1XRTT_V920_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : IRAT-ParametersCDMA2000-1XRTT-v920
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enECsfb1xrttR9;
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enECsfbConcPsMob1XrttR9;
    VOS_UINT8                                     aucReserved[2];
}RRC_IRAT_PARAM_CDMA2000_1XRTT_V920_STRU;

/*****************************************************************************
 结构名    : RRC_CSG_PROXIMITY_IND_PARAM_R9_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : CSG-ProximityIndicationParameters-r9
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enIntraFreqProximityIndR9;
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enInterFreqProximityIndR9;
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enUtranProximityIndR9;
    VOS_UINT8                                     ucReserved;
}RRC_CSG_PROXIMITY_IND_PARAM_R9_STRU;

/*****************************************************************************
 结构名    : RRC_NEIGH_CELL_SI_ACQ_PARAM_R9_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : NeighCellSI-AcquisitionParameters-r9
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enIntraFreqSiAcqForHoR9;
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enInterFreqSiAcqForHoR9;
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enUtranSiAcqForHoR9;
    VOS_UINT8                                     ucReserved;
}RRC_NEIGH_CELL_SI_ACQ_PARAM_R9_STRU;

/*****************************************************************************
 结构名    : RRC_SON_PARAM_R9_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : SON-Parameters-r9
*****************************************************************************/
typedef struct
{
    RRC_UE_CAP_FEATURE_SUPPORT_FLAG_ENUM_UINT8    enRachRprtR9;
    VOS_UINT8                                     aucReserved[3];
}RRC_SON_PARAM_R9_STRU;

/*****************************************************************************
 结构名    : RRC_UE_EUTRA_CAP_V940_IES_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE-EUTRA-Capability-v940-IEs
*****************************************************************************/
typedef struct
{
    VOS_UINT32    bitUeEutraCapV9a0Present                    : 1;
    VOS_UINT32    bitUeEutraCapV1020Present                   : 1;
    VOS_UINT32    bitSpare                                    : 30;

    /*R10 NV begin*/
    //RRC_UE_EUTRA_CAP_V9A0_IES_STRU              stUeEutraCapV9a0;               /* optional */
    //RRC_UE_EUTRA_CAP_V1020_IES_STRU             stUeEutraCapV1020;              /* optional */
    /*R10 NV end*/
}RRC_UE_EUTRA_CAP_V940_IES_STRU;
/* v7r2 LLD end */

/*****************************************************************************
 结构名    : RRC_UE_EUTRA_CAP_V920_IES_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : UE-EUTRA-Capability-v920-IEs
*****************************************************************************/
typedef struct
{
    VOS_UINT32    bitInterRatParamUtraV920Present             : 1;
    VOS_UINT32    bitInterRatParamCdma2000V920Present         : 1;
    /* v7r2 LLD begin */
    VOS_UINT32    bitUeEutraCapV940Present                    : 1;
    VOS_UINT32    bitSpare                                    : 29;
    /* v7r2 LLD end */

    RRC_PHY_LAYER_PARAM_V920_STRU               stPhyLayerParamV920;
    RRC_IRAT_PARAM_GERAN_V920_STRU              stInterRatParamGeranV920;
    RRC_IRAT_PARAM_UTRA_V920_STRU               stInterRatParamUtraV920;        /* optional */
    RRC_IRAT_PARAM_CDMA2000_1XRTT_V920_STRU     stInterRatParamCdma2000V920;    /* optional */
    RRC_UE_CAP_DEVICE_TYPE_R9_ENUM_UINT8        enDeviceTypeR9;
    VOS_UINT8                                   aucReserved[3];
    RRC_CSG_PROXIMITY_IND_PARAM_R9_STRU         stCsgProximityIndParamR9;
    RRC_NEIGH_CELL_SI_ACQ_PARAM_R9_STRU         stNeighCellSiAcqParamR9;
    RRC_SON_PARAM_R9_STRU                       stSonParamR9;
    /* v7r2 LLD begin */
    RRC_UE_EUTRA_CAP_V940_IES_STRU              stUeEutraCapV940;               /* optional */
    /* v7r2 LLD end */
}RRC_UE_EUTRA_CAP_V920_IES_STRU;

/*****************************************************************************
  6 UNION
*****************************************************************************/


/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/


/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/


/*****************************************************************************
  9 OTHERS
*****************************************************************************/










#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of RrcNvInterface.h */
