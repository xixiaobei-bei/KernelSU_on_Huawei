/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 * * This program is free software; you can redistribute it andor modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 * Otherwise, the following license terms apply:
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation andor other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
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
 */

#ifndef __NV_STRU_WAS_H__
#define __NV_STRU_WAS_H__

#include "ps_type_def.h"
#include "nv_id_guas.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 Include Headfile */
#pragma pack(push, 4)

/* 2 Macro */
#define WAS_NV_PTL_VER_R3                       (0)                             /* WAS_PTL_VER_R3 */
#define WAS_NV_PTL_VER_R4                       (1)                             /* WAS_PTL_VER_R4 */
#define WAS_NV_PTL_VER_R5                       (2)                             /* WAS_PTL_VER_R5 */
#define WAS_NV_PTL_VER_R6                       (3)                             /* WAS_PTL_VER_R6 */
#define WAS_NV_PTL_VER_R7                       (4)                             /* WAS_PTL_VER_R7 */
#define WAS_NV_PTL_VER_R8                       (5)                             /* WAS_PTL_VER_R8 */
#define WAS_NV_PTL_VER_R9                       (6)                             /* WAS_PTL_VER_R9 */

/*
 * 枚举名    : WAS_TX_RX_FREQ_SEPARAT_ENUM_UINT8
 * 协议表格  :
 * ASN.1描述 :
 * 枚举说明  :
 */
enum WAS_TxRxFreqSeparat
{
    WAS_TX_RX_FREQ_SEPARAT_DEFAULT_TX_RX_SEPARATION = 0,
    WAS_TX_RX_FREQ_SEPARAT_SPARE2,
    WAS_TX_RX_FREQ_SEPARAT_SPARE1,
    WAS_TX_RX_FREQ_SEPARAT_BUTT
} ;
typedef VOS_UINT8 WAS_TxRxFreqSeparatUint8;

/*
 * 枚举名    : WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8
 * 协议表格  :
 * ASN.1描述 :
 * 枚举说明  :
 */
enum  WAS_DlSimulHsDschCfg
{
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS32 = 0,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS64,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS128,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS384,
    WAS_DL_SIMUL_HS_DSCH_CFG_BUTT
};
typedef VOS_UINT8 WAS_DlSimulHsDschCfgUint8;

/*
 * 枚举名    : WAS_PtlVerTypeUint8
 * 协议表格  : 10.2.39    RRC CONNECTION REQUEST
 * ASN.1描述 : AccessStratumReleaseIndicator
 * 枚举说明  : 协议版本
 *            Absence of the IE implies R3.
 *            The IE also indicates the release of the RRC transfer syntax
 *            supported by the UE 13 spare values are needed
 */
enum WAS_PtlVerType
{
    WAS_PTL_VER_ENUM_R3                 = WAS_NV_PTL_VER_R3, /* _H2ASN_Replace  WAS_NV_PTL_VER_R3 = 0 */
    WAS_PTL_VER_ENUM_R4                 = WAS_NV_PTL_VER_R4, /* _H2ASN_Replace  WAS_NV_PTL_VER_R4 = 1 */
    WAS_PTL_VER_ENUM_R5                 = WAS_NV_PTL_VER_R5, /* _H2ASN_Replace  WAS_NV_PTL_VER_R5 = 2 */
    WAS_PTL_VER_ENUM_R6                 = WAS_NV_PTL_VER_R6, /* _H2ASN_Replace  WAS_NV_PTL_VER_R6 = 3 */
    WAS_PTL_VER_ENUM_R7                 = WAS_NV_PTL_VER_R7, /* _H2ASN_Replace  WAS_NV_PTL_VER_R7 = 4 */
    WAS_PTL_VER_ENUM_R8                 = WAS_NV_PTL_VER_R8, /* _H2ASN_Replace  WAS_NV_PTL_VER_R8 = 5 */
    WAS_PTL_VER_ENUM_R9                 = WAS_NV_PTL_VER_R9, /* _H2ASN_Replace  WAS_NV_PTL_VER_R9 = 6 */
    WAS_PTL_VER_ENUM_BUTT
};
typedef VOS_UINT8 WAS_PtlVerTypeUint8;

/*
 * 结构名    : WAS_RF_CAPA_STRU
 * 协议表格  : 10.3.3.33 RF capability FDD
 * ASN.1描述 :
 * 结构说明  : RF能力信息
 */
typedef struct
{
    VOS_UINT8                      powerClass;                       /* UE功率级别 */
    WAS_TxRxFreqSeparatUint8       txRxFreqSeparate;                 /* Tx/Rx 频率区间 */
    VOS_UINT8                      rsv[2];                     /* 4字节对齐，保留 */
}WAS_RfCapa;

/*
 * 结构名    : WAS_NVIM_UE_CAPA_CUSTOMED_STRU
 * 结构说明  : 标志WCDMA各种能力。
 */
typedef struct
{
    /*
     * 激活项。
     * 0：未激活，则DPA，UPA都支持；
     * 1：激活，是否支持DPA，UPA由下面NV项控制。
     * 注意：enEDCHSupport、ucHSDSCHPhyCategory、enHSDSCHSupport这3项NV受此激活NV项控制。
     * 当ulHspaStatus=1时，上述3项NV配置才生效，按照用户的配置上报能力等级，否则按产品默认值上报。
     */
    VOS_UINT32                              hspaStatus;

    WAS_RfCapa                              rfCapa; /* RF能力信息，请参见WAS_RF_CAPA_STRU。 */

    /*
     * 16QAM特性相关，是否支持E-DPCCH Power Boosting。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            supportPwrBoosting;
    PS_BoolUint8                            rsv1; /* 保留位 */

    /*
     * 和HSDSCH同时并行的DCH速率。
     * 0：32kbit/s；
     * 1：64kbit/s；
     * 2：128kbit/s；
     * 3：384kbit/s。
     */
    WAS_DlSimulHsDschCfgUint8               dlSimulHsDschCfg;
    /*
     * UE W接入层支持的协议版本。
     * 0：R3；
     * 1：R4；
     * 2：R5；
     * 3：R6；
     * 4：R7；
     * 5：R8；
     * 6：R9。
     */
    WAS_PtlVerTypeUint8                     asRelIndicator;

    /*
     * 0：不支持HS-DSCH；
     * 1：支持HS-DSCH。
     * 当ulHspaStatus = 1时，这项才生效。
     */
    PS_BoolUint8                            hsDschSupport;
    /*
     * 下行HS-DSCH category能力等级。
     * 6：支持速率为3.6 Mbits；
     * 8：支持速率为7.2 Mbits；
     * 10：支持速率为14.4 Mbits。
     * 注意：协议版本为R7或以上时，网络配置速率以ucHSDSCHPhyCategoryext为准。若测试HSDPA且category<13，需将
     * ucHSDSCHPhyCategoryext设为0。
     * 此值应该大于等于1，且小于等于64，如果超出此范围，使用值6。
     */
    VOS_UINT8                               hsDschPhyCategory;

    /*
     * 0：不支持MAC-ehs；
     * 1：支持MAC-ehs。
     */
    PS_BoolUint8                            macEhsSupport;
    /*
     * 下行HS-DSCH category能力等级扩展。
     * 取值范围12~20。
     * 当该值≤14，向物理层配置MIMO参数时，需要根据NV项enMimoSingleStreamStrict配置是否限制只能使用单流MIMO；
     * 当该值=17或者18，且小区支持64QAM，向物理层配置MIMO参数时，需要根据NV项enMimoSingleStreamStrict配置是否限制只能使
     * 用单流MIMO。
     */
    VOS_UINT8                               hsDschPhyCategoryExt;
    /*
     * R8特性，是否支持DC（可以根据产品定制）。
     * 0：不支持Multi Cell；
     * 1：支持Multi Cell。
     */
    PS_BoolUint8                            multiCellSupport;
    /*
     * 下行HS-DSCH category能力等级扩展2。enMultiCellSupport为1时，这项才生效。
     * 取值范围21~24，否则MutiCell不支持。
     */
    VOS_UINT8                               hsDschPhyCategoryExt2;

    /*
     * DC特性相关，是否支持DC小区指定发射分集。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            cellSpecTxDiversityForDc;
    PS_BoolUint8                            rsv2; /* 保留位 */
    /*
     * 0：不支持HSUPA功能；
     * 1：支持HSUPA功能。
     */
    PS_BoolUint8                            edchSupport;
    /*
     * 支持UPA的等级。
     * 1：支持速率为0.7296Mbit/s；
     * 2：支持速率为1.4592Mbit/s；
     * 3：支持速率为1.4592Mbit/s；
     * 4：支持速率为2.9185Mbit/s；
     * 5：支持速率为2Mbit/s；
     * 6：支持速率为5.76Mbit/s。
     * 支持DC UPA特性时ucEDCHPhyCategoryExt2值为9。
     */
    VOS_UINT8                               edchPhyCategory;
    /*
     * R7特性，是否支持上行16QAM，当enSuppUl16QAM=1时ucEDCHPhyCategoryExt才有效。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            suppUl16QAM;
    /*
     * 支持UPA的等级的扩展项。
     * 上行单载波支持16QAM时，填写7，填写其他值无效。
     */
    VOS_UINT8                               edchPhyCategoryExt;
    /*
     * 16QAM特性相关，是否支持E-DPDCH power interpolation formula（功率内插方式）。
     * 0：不支持；
     * 1；支持。
     */
    PS_BoolUint8                            suppEDpdchInterpolationFormula;
    PS_BoolUint8                            rsv3; /* 保留位 */
    PS_BoolUint8                            rsv4; /* 保留位 */

    /*
     * R8上行增强L2特性，是否支持MAC_I/MAC_Is。
     * 0：不支持；
     * 1：支持。
     * 只有当UE支持UPA(enEDCHSupport = 1)和下行MAC-ehs(enMacEhsSupport = 1)时，才支持MAC-i。
     */
    PS_BoolUint8                            macIsSupport;

    PS_BoolUint8                            rsv5; /* 保留位 */

    PS_BoolUint8                            rsv6; /* 保留位 */
    PS_BoolUint8                            rsv7; /* 保留位 */

    /*
     * R8 DC特性，是否支持在无压模的情况下进行邻频测量。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            adjFreqMeasWithoutCmprMode;

    /*
     * MIMO特性相关，是否支持限制只能使用单流MIMO。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            mimoSingleStreamStrict;
    /*
     * MIMO特性相关，MIMO激活时，下行控制信道是否支持允许使用分集。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            mimoWithDlTxDiversity;

    /*
     * 是否支持UTRA中的优先级重选。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            sptAbsPriBasedReselInUtra;

    /*
     * R9特性，HS-DSCH能力等级扩展项3。
     * 取值范围：25~28，超出范围时不支持DC+MIMO。
     */
    VOS_UINT8                               hsDschPhyCategoryExt3;
    /*
     * R9特性，是否支持DC+MIMO。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            dcMimoSupport;
    /*
     * R8特性，是否支持上行使用公共E-DCH信道(E-RACH新增特性)。
     * 只有当支持上行增强FACH、下行增强FACH和MAC-I时，该项才生效，即enMacEhsSupport=1、enEDCHSupport=1、
     * enSuppHsdpaInFach=1时，该项NV才生效。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            suppCommEDCH;

    /*
     * R9特性，是否支持DC UPA。
     * 0：不支持；
     * 1：支持。
     * 如果ucEDCHPhyCategoryExt2小于8或者大于9，或者ucEDCHPhyCategory不等于6，则该NV项无效，即不支持DC UPA特性。
     */
    PS_BoolUint8                            dcUpaSupport;
    /*
     * EDCH category extension 2。
     * 如果不支持DC UPA，该NV项不生效。
     * 如果支持DC UPA且enSuppUl16QAM=0，该项值为8。
     */
    VOS_UINT8                               edchPhyCategoryExt2;
    /*
     * E-DPDCH功率回退因子使能标志，德电定制。
     * 0：不支持；
     * 1：支持。
     */
    PS_BoolUint8                            edpdchGainFactorFlg;
    PS_BoolUint8                            rsv8; /* 保留位 */
    PS_BoolUint8                            rsv9; /* 保留位 */
    VOS_UINT8                               rsv10[5];
}WAS_NVIM_UeCapaCustomed;

/*
 * 结构名    : WAS_NVIM_UE_CAPA_STRU
 * 结构说明  : 存储在NVIM中的UE能力信息
 */
typedef struct
{
    VOS_UINT32                              hspaStatus; /* 0表示未激活,那么DPA和UPA都支持;1表示激活 */

    WAS_RfCapa                              rfCapa;                           /* RF 能力信息 */

    PS_BoolUint8                            supportPwrBoosting; /* 16QAM特性相关，是否支持E-DPCCH Power Boosting */
    PS_BoolUint8                            sf4Support;                       /* 是否支持ul dpcch 使用 slotFormat4 */

    WAS_DlSimulHsDschCfgUint8               dlSimulHsDschCfg;                 /* ENUMERATED  OPTIONAL */
    WAS_PtlVerTypeUint8                     asRelIndicator;                   /* Access Stratum Release Indicator */

    PS_BoolUint8                            hsDschSupport;                    /* 是否支持enHSDSCHSupport的标志 */
    VOS_UINT8                               hsDschPhyCategory;                /* 支持HS-DSCH物理层的类型标志 */

    PS_BoolUint8                            macEhsSupport;
    VOS_UINT8                               hsDschPhyCategoryExt;
    /* 是否支持 Multi cell support,如果支持MultiCell,Ex2存在 */
    PS_BoolUint8                            multiCellSupport;
    VOS_UINT8                               hsDschPhyCategoryExt2; /* HS-DSCH physical layer category extension 2 */
    /*
     * This IE is optionally present if Dual-Cell HSDPA is supported. Otherwise it is not needed.
     * The IE is not needed in the INTER RAT HANDOVER INFO message. Otherwise, it is optional
     */
    PS_BoolUint8                            cellSpecTxDiversityForDc;
    PS_BoolUint8                            efdpchSupport; /* 是否支持E-FDPCH的标志,FDPCH支持时此NV才生效 */
    PS_BoolUint8                            edchSupport;                      /* 是否支持EDCH的标志 */
    VOS_UINT8                               edchPhyCategory;                  /* 支持UPA的等级 */
    PS_BoolUint8                            suppUl16QAM; /* 是否支持上行16QAM，当支持时ucEDCHPhyCategoryExt才有效 */
    VOS_UINT8                               edchPhyCategoryExt;               /* 上行单载波支持16QAM时，填写7 */
    /* 16QAM特性相关，是否支持E-DPDCH power interpolation formula */
    PS_BoolUint8                            suppEDpdchInterpolationFormula;
    PS_BoolUint8                            suppHsdpaInFach;                  /* 支持CELL_FACH下HS-DSCH的接收 */
    PS_BoolUint8                            suppHsdpaInPch; /* 支持CELL_PCH或URA_PCH下HS-DSCH的接收 */

    PS_BoolUint8                            macIsSupport;                     /* 是否支持MAC_I/MAC_Is */

    PS_BoolUint8                            fdpchSupport;                     /* 是否支持FDPCH的标志 */

    PS_BoolUint8                            hsscchLessSupport;                /* 是否支持 hsscchlessHsdschOperation */
    PS_BoolUint8                            ulDpcchDtxSupport; /* 是否支持 discontinuousDpcchTransmission */

    /* 是否支持 Adjacent Frequency measurements without compressed mode */
    PS_BoolUint8                            adjFreqMeasWithoutCmprMode;

    PS_BoolUint8                            mimoSingleStreamStrict;           /* 是否限制只能使用单流MIMO */
    /* R9特性，在MIMO激活时，下行控制信道是否允许使用分集 */
    PS_BoolUint8                            mimoWithDlTxDiversity;

    /* 支持UTRA中的优先级重选，默认为0，1为支持，0为不支持 */
    PS_BoolUint8                            sptAbsPriBasedReselInUtra;

    VOS_UINT8                               hsDschPhyCategoryExt3; /* HS-DSCH physical layer category extension 3 */
    PS_BoolUint8                            dcMimoSupport;                    /* 是否支持DC+MIMO */
    PS_BoolUint8                            suppCommEDCH;                     /* E-RACH新增特性 */

    PS_BoolUint8                            dcUpaSupport;                     /* 是否支持DC UPA的标志 */
    VOS_UINT8                               edchPhyCategoryExt2;              /* EDCH  category extension 2 */
    PS_BoolUint8                            edpdchGainFactorFlg;              /* E-DPDCH功率回退因子使能标志位 */
    PS_BoolUint8                            ho2EutranUnSupportFlg;            /* 是否不支持到L的HO */
    PS_BoolUint8                            eutranMeasUnSupportFlg;           /* 是否不支持到连接态L的测量 */
    VOS_UINT8                               rsv[5];
}WAS_NVIM_UeCapa;

#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif

/* end of nv_stru_was.h */

