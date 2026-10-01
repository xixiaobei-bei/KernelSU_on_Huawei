

#ifndef __NV_STRU_WAS_H__
#define __NV_STRU_WAS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

#include "PsTypeDef.h"
#include "nv_id_guas.h"

/*****************************************************************************
  2 Macro
*****************************************************************************/
#define WAS_NV_PTL_VER_R3                       (0)                             /* WAS_PTL_VER_R3 */
#define WAS_NV_PTL_VER_R4                       (1)                             /* WAS_PTL_VER_R4 */
#define WAS_NV_PTL_VER_R5                       (2)                             /* WAS_PTL_VER_R5 */
#define WAS_NV_PTL_VER_R6                       (3)                             /* WAS_PTL_VER_R6 */
#define WAS_NV_PTL_VER_R7                       (4)                             /* WAS_PTL_VER_R7 */
#define WAS_NV_PTL_VER_R8                       (5)                             /* WAS_PTL_VER_R8 */
#define WAS_NV_PTL_VER_R9                       (6)                             /* WAS_PTL_VER_R9 */


/*****************************************************************************
  3 Massage Declare
*****************************************************************************/

/*****************************************************************************
  4 Enum
*****************************************************************************/
/*****************************************************************************
 枚举名    : WAS_TX_RX_FREQ_SEPARAT_ENUM_UINT8
 协议表格  :
 ASN.1描述 :
 枚举说明  :
*****************************************************************************/
enum WAS_TX_RX_FREQ_SEPARAT_ENUM
{
    WAS_TX_RX_FREQ_SEPARAT_DEFAULT_TX_RX_SEPARATION = 0,
    WAS_TX_RX_FREQ_SEPARAT_SPARE2,
    WAS_TX_RX_FREQ_SEPARAT_SPARE1,
    WAS_TX_RX_FREQ_SEPARAT_BUTT
} ;
typedef VOS_UINT8 WAS_TX_RX_FREQ_SEPARAT_ENUM_UINT8;

/*****************************************************************************
 枚举名    : WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8
 协议表格  :
 ASN.1描述 :
 枚举说明  :
*****************************************************************************/
enum  WAS_DL_SIMUL_HS_DSCH_CFG_ENUM
{
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS32 = 0,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS64,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS128,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS384,
    WAS_DL_SIMUL_HS_DSCH_CFG_BUTT
};
typedef VOS_UINT8 WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8;

/*****************************************************************************
 枚举名    : WAS_PTL_VER_TYPE_ENUM_UINT8
 协议表格  : 10.2.39    RRC CONNECTION REQUEST
 ASN.1描述 : AccessStratumReleaseIndicator
 枚举说明  : 协议版本
             Absence of the IE implies R3.
             The IE also indicates the release of the RRC transfer syntax
             supported by the UE 13 spare values are needed
*****************************************************************************/
enum WAS_PTL_VER_TYPE_ENUM
{
    WAS_PTL_VER_ENUM_R3                 = WAS_NV_PTL_VER_R3,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R3 = 0 */
    WAS_PTL_VER_ENUM_R4                 = WAS_NV_PTL_VER_R4,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R4 = 1 */
    WAS_PTL_VER_ENUM_R5                 = WAS_NV_PTL_VER_R5,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R5 = 2 */
    WAS_PTL_VER_ENUM_R6                 = WAS_NV_PTL_VER_R6,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R6 = 3 */
    WAS_PTL_VER_ENUM_R7                 = WAS_NV_PTL_VER_R7,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R7 = 4 */
    WAS_PTL_VER_ENUM_R8                 = WAS_NV_PTL_VER_R8,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R8 = 5 */
    WAS_PTL_VER_ENUM_R9                 = WAS_NV_PTL_VER_R9,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R9 = 6 */
    WAS_PTL_VER_ENUM_BUTT
};
typedef VOS_UINT8 WAS_PTL_VER_TYPE_ENUM_UINT8;

/*****************************************************************************
  5 STRUCT
*****************************************************************************/

/*****************************************************************************
 结构名    : WAS_RF_CAPA_STRU
 协议表格  : 10.3.3.33 RF capability FDD
 ASN.1描述 :
 结构说明  : RF能力信息
 DESCRIPTION: RF能力信息
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucPowerClass;                       /* UE功率级别                               */
    WAS_TX_RX_FREQ_SEPARAT_ENUM_UINT8       enTxRxFreqSeparate;                 /* Tx/Rx 频率区间                           */
    VOS_UINT8                               aucReserve1[2];                     /* 4字节对齐，保留 */
}WAS_RF_CAPA_STRU;

/*****************************************************************************
 结构名    : WAS_NVIM_UE_CAPA_CUSTOMED_STRU
 结构说明  : 解耦能力后存储在NVIM中的UE能力信息,当前供BOSTON分支解析使用   ID:9008 en_NV_Item_WAS_RadioAccess_Capa_New
 DESCRIPTION: 解耦能力后存储在NVIM中的UE能力信息,当前供BOSTON分支解析使用
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulHspaStatus;                       /* 0表示未激活,那么DPA和UPA都支持;1表示激活 */

    WAS_RF_CAPA_STRU                        stRfCapa;                           /* RF 能力信息                                  */

    PS_BOOL_ENUM_UINT8                      enSupportPwrBoosting;               /* 16QAM特性相关，是否支持E-DPCCH Power Boosting*/
    PS_BOOL_ENUM_UINT8                      enReserve1;                         /* 解耦enSf4Support，原位置置为reserved */

    WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8     enDlSimulHsDschCfg;                 /* ENUMERATED  OPTIONAL                         */
    WAS_PTL_VER_TYPE_ENUM_UINT8             enAsRelIndicator;                   /* Access Stratum Release Indicator             */

    PS_BOOL_ENUM_UINT8                      enHSDSCHSupport;                    /* 是否支持enHSDSCHSupport的标志                */
    VOS_UINT8                               ucHSDSCHPhyCategory;                /* 支持HS-DSCH物理层的类型标志                  */

    PS_BOOL_ENUM_UINT8                      enMacEhsSupport;
    VOS_UINT8                               ucHSDSCHPhyCategoryExt;
    PS_BOOL_ENUM_UINT8                      enMultiCellSupport;                 /* 是否支持 Multi cell support,如果支持MultiCell,Ex2存在 */
    VOS_UINT8                               ucHSDSCHPhyCategoryExt2;            /* HS-DSCH physical layer category extension 2 */

    PS_BOOL_ENUM_UINT8                      enCellSpecTxDiversityForDc;         /*  This IE is optionally present if Dual-Cell HSDPA is supported. Otherwise it is not needed.
                                                                                    The IE is not needed in the INTER RAT HANDOVER INFO message. Otherwise, it is optional*/
    PS_BOOL_ENUM_UINT8                      enReserve2;                         /* 解耦enEFDPCHSupport，原位置置为reserved */
    PS_BOOL_ENUM_UINT8                      enEDCHSupport;                      /* 是否支持EDCH的标志                           */
    VOS_UINT8                               ucEDCHPhyCategory;                  /* 支持UPA的等级                                */
    PS_BOOL_ENUM_UINT8                      enSuppUl16QAM;                      /* 是否支持上行16QAM，当支持时ucEDCHPhyCategoryExt才有效 */
    VOS_UINT8                               ucEDCHPhyCategoryExt;               /* 上行单载波支持16QAM时，填写7 */
    PS_BOOL_ENUM_UINT8                      enSuppEDpdchInterpolationFormula;   /* 16QAM特性相关，是否支持E-DPDCH power interpolation formula */
    PS_BOOL_ENUM_UINT8                      enReserve3;                         /* 解耦enSuppHsdpaInFach，原位置置为reserved */
    PS_BOOL_ENUM_UINT8                      enReserve4;                         /* 解耦enSuppHsdpaInPch，原位置置为reserved */

    PS_BOOL_ENUM_UINT8                      enMacIsSupport;                     /* 是否支持MAC_I/MAC_Is */

    PS_BOOL_ENUM_UINT8                      enReserve5;                         /* 解耦enFDPCHSupport，原位置置为reserved */

    PS_BOOL_ENUM_UINT8                      enReserve6;                         /* 解耦enHsscchLessSupport，原位置置为reserved */
    PS_BOOL_ENUM_UINT8                      enReserve7;                         /* 解耦enUlDpcchDtxSupport，原位置置为reserved */

    PS_BOOL_ENUM_UINT8                      enAdjFreqMeasWithoutCmprMode;       /* 是否支持 Adjacent Frequency measurements without compressed mode */

    PS_BOOL_ENUM_UINT8                      enMimoSingleStreamStrict;           /* 是否限制只能使用单流MIMO */
    PS_BOOL_ENUM_UINT8                      enMimoWithDlTxDiversity;            /* R9特性，在MIMO激活时，下行控制信道是否允许使用分集 */

    /* V7r1 双模增加NV项 LTE能力 */
    PS_BOOL_ENUM_UINT8                      enSptAbsPriBasedReselInUtra;        /* 支持UTRA中的优先级重选，默认为0，1为支持，0为不支持 */

    VOS_UINT8                               ucHSDSCHPhyCategoryExt3;            /* HS-DSCH physical layer category extension 3 */
    PS_BOOL_ENUM_UINT8                      enDcMimoSupport;                    /* 是否支持DC+MIMO */
    PS_BOOL_ENUM_UINT8                      enSuppCommEDCH;                     /* E-RACH新增特性 */

    PS_BOOL_ENUM_UINT8                      enDcUpaSupport;                     /* 是否支持DC UPA的标志 */
    VOS_UINT8                               ucEDCHPhyCategoryExt2;              /* EDCH  category extension 2*/
    PS_BOOL_ENUM_UINT8                      enEdpdchGainFactorFlg;              /* E-DPDCH功率回退因子使能标志位 */
    PS_BOOL_ENUM_UINT8                      enReserve8;                         /* 解耦enHo2EutranUnSupportFlg，原位置置为reserved */
    PS_BOOL_ENUM_UINT8                      enReserve9;                         /* 解耦enEutranMeasUnSupportFlg，原位置置为reserved */
    VOS_UINT8                               aucReserve2[5];
}WAS_NVIM_UE_CAPA_CUSTOMED_STRU;


/*****************************************************************************
 结构名    : WAS_NVIM_UE_CAPA_STRU
 结构说明  : 存储在NVIM中的UE能力信息               ID:9008 en_NV_Item_WAS_RadioAccess_Capa_New
 DESCRIPTION: 存储在NVIM中的UE能力信息
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulHspaStatus;                       /* 0表示未激活,那么DPA和UPA都支持;1表示激活 */

    WAS_RF_CAPA_STRU                        stRfCapa;                           /* RF 能力信息                                  */

    PS_BOOL_ENUM_UINT8                      enSupportPwrBoosting;               /* 16QAM特性相关，是否支持E-DPCCH Power Boosting*/
    PS_BOOL_ENUM_UINT8                      enSf4Support;                       /* 是否支持ul dpcch 使用 slotFormat4 */

    WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8     enDlSimulHsDschCfg;                 /* ENUMERATED  OPTIONAL                         */
    WAS_PTL_VER_TYPE_ENUM_UINT8             enAsRelIndicator;                   /* Access Stratum Release Indicator             */

    PS_BOOL_ENUM_UINT8                      enHSDSCHSupport;                    /* 是否支持enHSDSCHSupport的标志                */
    VOS_UINT8                               ucHSDSCHPhyCategory;                /* 支持HS-DSCH物理层的类型标志                  */

    PS_BOOL_ENUM_UINT8                      enMacEhsSupport;
    VOS_UINT8                               ucHSDSCHPhyCategoryExt;
    PS_BOOL_ENUM_UINT8                      enMultiCellSupport;                 /* 是否支持 Multi cell support,如果支持MultiCell,Ex2存在 */
    VOS_UINT8                               ucHSDSCHPhyCategoryExt2;            /* HS-DSCH physical layer category extension 2 */

    PS_BOOL_ENUM_UINT8                      enCellSpecTxDiversityForDc;         /*  This IE is optionally present if Dual-Cell HSDPA is supported. Otherwise it is not needed.
                                                                                    The IE is not needed in the INTER RAT HANDOVER INFO message. Otherwise, it is optional*/
    PS_BOOL_ENUM_UINT8                      enEFDPCHSupport;                    /* 是否支持E-FDPCH的标志,FDPCH支持时此NV才生效 */
    PS_BOOL_ENUM_UINT8                      enEDCHSupport;                      /* 是否支持EDCH的标志                           */
    VOS_UINT8                               ucEDCHPhyCategory;                  /* 支持UPA的等级                                */
    PS_BOOL_ENUM_UINT8                      enSuppUl16QAM;                      /* 是否支持上行16QAM，当支持时ucEDCHPhyCategoryExt才有效 */
    VOS_UINT8                               ucEDCHPhyCategoryExt;               /* 上行单载波支持16QAM时，填写7 */
    PS_BOOL_ENUM_UINT8                      enSuppEDpdchInterpolationFormula;   /* 16QAM特性相关，是否支持E-DPDCH power interpolation formula */
    PS_BOOL_ENUM_UINT8                      enSuppHsdpaInFach;                  /* 支持CELL_FACH下HS-DSCH的接收 */
    PS_BOOL_ENUM_UINT8                      enSuppHsdpaInPch;                   /* 支持CELL_PCH或URA_PCH下HS-DSCH的接收 */

    PS_BOOL_ENUM_UINT8                      enMacIsSupport;                     /* 是否支持MAC_I/MAC_Is */

    PS_BOOL_ENUM_UINT8                      enFDPCHSupport;                     /* 是否支持FDPCH的标志                          */

    PS_BOOL_ENUM_UINT8                      enHsscchLessSupport;                /* 是否支持 hsscchlessHsdschOperation           */
    PS_BOOL_ENUM_UINT8                      enUlDpcchDtxSupport;                /* 是否支持 discontinuousDpcchTransmission      */

    PS_BOOL_ENUM_UINT8                      enAdjFreqMeasWithoutCmprMode;       /* 是否支持 Adjacent Frequency measurements without compressed mode */

    PS_BOOL_ENUM_UINT8                      enMimoSingleStreamStrict;           /* 是否限制只能使用单流MIMO */
    PS_BOOL_ENUM_UINT8                      enMimoWithDlTxDiversity;            /* R9特性，在MIMO激活时，下行控制信道是否允许使用分集 */

    /* V7r1 双模增加NV项 LTE能力 */
    PS_BOOL_ENUM_UINT8                      enSptAbsPriBasedReselInUtra;        /* 支持UTRA中的优先级重选，默认为0，1为支持，0为不支持 */

    VOS_UINT8                               ucHSDSCHPhyCategoryExt3;            /* HS-DSCH physical layer category extension 3 */
    PS_BOOL_ENUM_UINT8                      enDcMimoSupport;                    /* 是否支持DC+MIMO */
    PS_BOOL_ENUM_UINT8                      enSuppCommEDCH;                     /* E-RACH新增特性 */

    PS_BOOL_ENUM_UINT8                      enDcUpaSupport;                     /* 是否支持DC UPA的标志 */
    VOS_UINT8                               ucEDCHPhyCategoryExt2;              /* EDCH  category extension 2*/
    PS_BOOL_ENUM_UINT8                      enEdpdchGainFactorFlg;              /* E-DPDCH功率回退因子使能标志位 */
    PS_BOOL_ENUM_UINT8                      enHo2EutranUnSupportFlg;            /* 是否不支持到L的HO */
    PS_BOOL_ENUM_UINT8                      enEutranMeasUnSupportFlg;           /* 是否不支持到连接态L的测量 */
    VOS_UINT8                               aucReserve2[5];
}WAS_NVIM_UE_CAPA_STRU;


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

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif

/* end of nv_stru_was.h */

