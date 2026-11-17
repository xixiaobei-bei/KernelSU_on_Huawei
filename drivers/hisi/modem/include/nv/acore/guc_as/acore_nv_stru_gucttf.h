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


#ifndef __ACORE_NV_STRU_GUCTTF_H__
#define __ACORE_NV_STRU_GUCTTF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include "vos.h"
#include "acore_nv_id_gucttf.h"

#define FC_ACPU_DRV_ASSEM_NV_LEV            (4)


enum FC_ACPU_DRV_ASSEM_LEV_ENUM
{
    FC_ACPU_DRV_ASSEM_LEV_1             = 0,
    FC_ACPU_DRV_ASSEM_LEV_2             = 1,
    FC_ACPU_DRV_ASSEM_LEV_3             = 2,
    FC_ACPU_DRV_ASSEM_LEV_4             = 3,
    FC_ACPU_DRV_ASSEM_LEV_5             = 4,
    FC_ACPU_DRV_ASSEM_LEV_BUTT          = 5
};
typedef VOS_UINT32  FC_ACPU_DRV_ASSEM_LEV_ENUM_UINT32;


enum FC_MEM_THRESHOLD_LEV_ENUM
{
    FC_MEM_THRESHOLD_LEV_1              = 0,
    FC_MEM_THRESHOLD_LEV_2              = 1,
    FC_MEM_THRESHOLD_LEV_3              = 2,
    FC_MEM_THRESHOLD_LEV_4              = 3,
    FC_MEM_THRESHOLD_LEV_5              = 4,
    FC_MEM_THRESHOLD_LEV_6              = 5,
    FC_MEM_THRESHOLD_LEV_7              = 6,
    FC_MEM_THRESHOLD_LEV_8              = 7,
    FC_MEM_THRESHOLD_LEV_BUTT           = 8
};
typedef VOS_UINT32  FC_MEM_THRESHOLD_LEV_ENUM_UINT32;

/*****************************************************************************
 枚举名    : TTF_BOOL_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : TTF统一布尔类型枚举定义
*****************************************************************************/
enum TTF_ACORE_BOOL_ENUM
{
    TTF_ACORE_FALSE                     = 0,
    TTF_ACORE_TRUE                      = 1,

    TTF_ACORE_BOOL_BUTT
};
typedef VOS_UINT8   TTF_ACORE_BOOL_ENUM_UINT8;

/*****************************************************************************
 结构名    : FC_CPU_DRV_ASSEM_PARA_STRU
 DESCRIPTION: 根据CPU LOAD动态高速驱动组包参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                          ucHostOutTimeout;    /* PC驱动组包时延 */
    VOS_UINT8                          ucEthTxMinNum;       /* UE驱动下行组包个数 */
    VOS_UINT8                          ucEthTxTimeout;      /* UE驱动下行组包时延 */
    VOS_UINT8                          ucEthRxMinNum;       /* UE驱动上行组包个数 */
    VOS_UINT8                          ucEthRxTimeout;      /* UE驱动上行组包时延 */
    VOS_UINT8                          ucCdsGuDlThres;      /* 已废弃 */
    VOS_UINT8                          aucRsv[2];
}FC_DRV_ASSEM_PARA_STRU;

/*****************************************************************************
 结构名    : FC_CPU_DRV_ASSEM_PARA_STRU
 DESCRIPTION: 根据CPU LOAD动态高速驱动组包参数
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulCpuLoad;                              /* CPU负载,Range:[0,100] */
    FC_DRV_ASSEM_PARA_STRU              stDrvAssemPara;
}FC_CPU_DRV_ASSEM_PARA_STRU;

/*****************************************************************************
 结构名    : FC_ACORE_CFG_CPU_STRU
 DESCRIPTION: FC_ACORE_CFG_CPU结构,CPU流控的门限和配置值
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulCpuOverLoadVal;                       /* CPU启流控水线，取值范围[1,100]，表示CPU占用率大小。*/
    VOS_UINT32                          ulCpuUnderLoadVal;                      /* CPU停流控水线，取值范围[1,ulCpuOverLoadVal]，表示CPU占用率大小。*/
    VOS_UINT32                          ulSmoothTimerLen;                       /* CPU流控启动平滑次数，取值范围[2,1000]，单位：CPU监控周期。*/
    VOS_UINT32                          ulStopAttemptTimerLen;                  /* CPU流控达到最高级后，启动定时器尝试解流控，该定时器的时长，单位毫秒，取值范围大于等于0，0表示不使用。*/
    VOS_UINT32                          ulUmUlRateThreshold;                    /* 上行速率水线，上行速率超此水线才可以启CPU流控，单位bps，取值范围大于等于0。*/
    VOS_UINT32                          ulUmDlRateThreshold;                    /* 下行速率水线，下行速率超此水线才可以启CPU流控，单位bps，取值范围大于等于0。*/
    VOS_UINT32                          ulRmRateThreshold;                      /* 网桥速率水线，网桥速率超此水线才可以启CPU流控，单位bps，取值范围大于等于0。*/
} FC_ACORE_CFG_CPU_STRU;

/*****************************************************************************
 结构名    : FC_ACORE_CFG_MEM_THRESHOLD_STRU
 DESCRIPTION: FC_ACORE_CFG_MEM_THRESHOLD结构,MEM流控的门限和配置值
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulSetThreshold;                         /* 启动流控门限 单位字节 */
    VOS_UINT32                          ulStopThreshold;                        /* 停止流控门限 单位字节 */
} FC_ACORE_CFG_MEM_THRESHOLD_STRU;

/*****************************************************************************
 结构名    : FC_CFG_MEM_THRESHOLD_CST_STRU
 DESCRIPTION: FC_CFG_MEM_THRESHOLD_CST结构,MEM流控的门限和配置值
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulSetThreshold;                         /*Range:[0,4096]*//* 启动流控门限 单位字节 */
    VOS_UINT32                          ulStopThreshold;                        /*Range:[0,4096]*//* 停止流控门限 单位字节 */
} FC_CFG_MEM_THRESHOLD_CST_STRU;

/*****************************************************************************
 结构名    : NV_TTF_PPP_CONFIG_STRU
 DESCRIPTION: 对应en_NV_Item_PPP_CONFIG项的结构
*****************************************************************************/
typedef struct
{
    TTF_ACORE_BOOL_ENUM_UINT8           enChapEnable;           /* 是否使能Chap鉴权 */
    TTF_ACORE_BOOL_ENUM_UINT8           enPapEnable;            /* 是否使能Pap鉴权 */
    VOS_UINT16                          usLcpEchoMaxLostCnt;    /* 发送LcpEchoRequest允许丢弃的最大个数 */

    VOS_UINT16                          usQueneMaxCnt;          /* 队列最大允许个数 */
    VOS_UINT8                           aucRsv[2];
}NV_TTF_PPP_CONFIG_STRU;

/*****************************************************************************
 结构名    : CPULOAD_CFG_STRU
 DESCRIPTION: 对应en_NV_Item_Linux_CPU_Moniter_Timer_Len项的结构,A核CPU占用率监测任务定时检测时长
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulMonitorTimerLen;
} CPULOAD_CFG_STRU;

/*****************************************************************************
 结构名    : FC_CPU_DRV_ASSEM_PARA_NV_STRU
 DESCRIPTION: 对应en_NV_Item_FC_ACPU_DRV_ASSEMBLE_PARA项的结构，根据CPU LOAD动态高速驱动组包参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                              ucEnableMask;                        /* 使能控制，0x0:不使能，0x1:使能 */
    VOS_UINT8                              ucSmoothCntUpLev;                    /* 向上调整平滑系数 */
    VOS_UINT8                              ucSmoothCntDownLev;                  /* 向下调整平滑系数 */
    VOS_UINT8                              ucRsv;
    FC_CPU_DRV_ASSEM_PARA_STRU             stCpuDrvAssemPara[FC_ACPU_DRV_ASSEM_NV_LEV]; /* CPU配置参数 */
}FC_CPU_DRV_ASSEM_PARA_NV_STRU;

/*****************************************************************************
 结构名    : NF_EXT_NV_STRU
 DESCRIPTION: 对应en_NV_Item_NETFILTER_HOOK_MASK项的结构,设置勾包点的NV项，预留5种掩码组合:
             ulNvValue1~ulNvValue5, 每个掩码取值范围为0-FFFFFFFF,
             其中掩码位为1则代表该掩码位对应的钩子函数可能会被注册到内核中
*****************************************************************************/
typedef struct
{
    VOS_UINT32          ulNetfilterPara1;                                       /* 钩子函数掩码参数1 */
    VOS_UINT32          ulNetfilterPara2;                                       /* 钩子函数掩码参数2 */
    VOS_UINT32          ulNetfilterPara3;                                       /* 钩子函数掩码参数3 */
    VOS_UINT32          ulNetfilterPara4;                                       /* 钩子函数掩码参数4 */
    VOS_UINT32          ulNetfilterPara5;                                       /* 钩子函数掩码参数5 */
}NF_EXT_NV_STRU;

/*****************************************************************************
结构名    : PPP_CONFIG_MRU_TYPE_NV_STRU
DESCRIPTION: 对应en_NV_Item_PPP_CONFIG_MRU_Type项的结构,默认协商MTU参数，用于PPP LCP协商
*****************************************************************************/
typedef struct
{
    VOS_UINT16                           usPppConfigType;   /* 默认MRU大小,Range:[296,1500]*/
    VOS_UINT8                            aucReserve[2];
}PPP_CONFIG_MRU_TYPE_NV_STRU;

/*****************************************************************************
 结构名    : FC_CFG_CST_STRU
 协议表格  : 无
 ASN.1描述 : 无
 结构说明  : CST流控门限
*****************************************************************************/
typedef struct
{
    FC_ACORE_CFG_MEM_THRESHOLD_STRU     stThreshold;                            /* CST环形缓存队列流控水线，单位字节，总长4096。*/
} FC_CFG_CST_STRU;

/*****************************************************************************
 结构名    : FC_CFG_MEM_STRU
 协议表格  : 无
 ASN.1描述 : 无
 结构说明  : MEM流控的门限和配置值
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulThresholdCnt;                         /* A核内存流控档位个数，取值范围[0,8]。*/
    FC_ACORE_CFG_MEM_THRESHOLD_STRU     astThreshold[FC_MEM_THRESHOLD_LEV_BUTT];/* A核内存流控水线配置，取A核内存池剩余空闲块个数。*/
} FC_CFG_MEM_STRU;

/*****************************************************************************
 结构名    : ACORE_FC_CFG_NV_STRU
 DESCRIPTION: 对应en_NV_Item_Acore_Flow_Ctrl_Config项的结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulFcEnableMask;                         /* A核流控使能开关。每个比特位置0表示该项流控去使能，1表示使能。Bit0：MEM流控；Bit1：A核CPU流控；Bit2：CDS流控；Bit3：CST流控；*/
    FC_ACORE_CFG_CPU_STRU               stFcCfgCpuA;                            /* A核CPU流控水线配置。*/
    FC_CFG_MEM_STRU                     stFcCfgMem;                             /* A核内存流控水线配置。*/
    FC_CFG_CST_STRU                     stFcCfgCst;                             /* CST业务流控水线配置。*/
} ACORE_FC_CFG_NV_STRU;

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __ACORE_NV_ID_GUCTTF_H__ */

