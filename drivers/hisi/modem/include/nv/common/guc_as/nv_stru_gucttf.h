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


#ifndef __NV_STRU_GUCTTF_H__
#define __NV_STRU_GUCTTF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include "vos.h"
#include "nv_id_gucttf.h"

/*****************************************************************************
  2 Macro
*****************************************************************************/

#define FC_UL_RATE_MAX_LEV                  (11)
#define FC_ACPU_DRV_ASSEM_NV_LEV            (4)

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/

enum FC_ACPU_DRV_ASSEM_LEV_ENUM
{
    FC_ACPU_DRV_ASSEM_LEV_1             = 0,
    FC_ACPU_DRV_ASSEM_LEV_2,
    FC_ACPU_DRV_ASSEM_LEV_3,
    FC_ACPU_DRV_ASSEM_LEV_4,
    FC_ACPU_DRV_ASSEM_LEV_5             = 4,
    FC_ACPU_DRV_ASSEM_LEV_BUTT          = 5
};
typedef VOS_UINT32  FC_ACPU_DRV_ASSEM_LEV_ENUM_UINT32;


enum FC_MEM_THRESHOLD_LEV_ENUM
{
    FC_MEM_THRESHOLD_LEV_1              = 0,
    FC_MEM_THRESHOLD_LEV_2,
    FC_MEM_THRESHOLD_LEV_3,
    FC_MEM_THRESHOLD_LEV_4,
    FC_MEM_THRESHOLD_LEV_5,
    FC_MEM_THRESHOLD_LEV_6,
    FC_MEM_THRESHOLD_LEV_7,
    FC_MEM_THRESHOLD_LEV_8,
    FC_MEM_THRESHOLD_LEV_BUTT           = 8
};
typedef VOS_UINT32  FC_MEM_THRESHOLD_LEV_ENUM_UINT32;

/*****************************************************************************
 枚举名    : TTF_BOOL_ENUM
 协议表格  :
 ASN.1描述 :
 枚举说明  : TTF统一布尔类型枚举定义
*****************************************************************************/
enum TTF_BOOL_ENUM
{
    TTF_FALSE                            = 0,
    TTF_TRUE                             = 1,

    TTF_BOOL_BUTT
};
typedef VOS_UINT8   TTF_BOOL_ENUM_UINT8;

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
 结构名    : FC_CFG_CPU_STRU
 DESCRIPTION: FC_CFG_CPU结构,CPU流控的门限和配置值
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulCpuOverLoadVal;                       /*Range:[0,100]*//* CPU流控门限 */
    VOS_UINT32                          ulCpuUnderLoadVal;                      /*Range:[0,100]*//* CPU解除流控门限 */
    VOS_UINT32                          ulSmoothTimerLen;                       /*Range:[2,1000]*//* CPU流控平滑次数，单位:CPU监控周期 */
    VOS_UINT32                          ulStopAttemptTimerLen;                  /* CPU引发R接口流控后，数传中断时间较长，启动定时器，尝试提前解除，单位: 毫秒，0表示不使用 */
    VOS_UINT32                          ulUmUlRateThreshold;                    /* 空口上行速率门限，高于此门限，认为是数传引起的CPU负载高，需要流控 */
    VOS_UINT32                          ulUmDlRateThreshold;                    /* 空口下行速率门限，高于此门限，认为是数传引起的CPU负载高，需要流控 */
    VOS_UINT32                          ulRmRateThreshold;                      /* E5形态下， WIFI/USB入口处速率门限， 高于此门限，认为是数传引起的CPU负载高，需要流控，单位bps */
} FC_CFG_CPU_STRU;

/*****************************************************************************
 结构名    : FC_CFG_MEM_THRESHOLD_STRU
 DESCRIPTION: FC_CFG_MEM_THRESHOLD结构,MEM流控的门限和配置值
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulSetThreshold;                         /* 启动流控门限 单位字节 */
    VOS_UINT32                          ulStopThreshold;                        /* 停止流控门限 单位字节 */
} FC_CFG_MEM_THRESHOLD_STRU;

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
 结构名    : FC_CFG_UM_UL_RATE_STRU
 DESCRIPTION: FC_CFG_UM_UL_RATE结构,空口上行速率档位
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucRateCnt;                              /*Range:[0,11]*//* 上行速率档位个数，最多支持11个档位设置，但是优先级最高为FC_PRI_9，所以使用档位时只有前9档生效 */
    VOS_UINT8                           aucRsv[1];
    VOS_UINT16                          ausRate[FC_UL_RATE_MAX_LEV];            /* 上行速率限制，取值范围[0,65535]，单位bps */
} FC_CFG_UM_UL_RATE_STRU;

/*****************************************************************************
 结构名    : FC_CFG_NV_STRU
 DESCRIPTION: 对应en_NV_Item_Flow_Ctrl_Config项的结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulFcEnbaleMask;                         /* 流控使能标识 */
    FC_CFG_CPU_STRU                     stFcCfgCpuA;                            /* A核CPU流控门限 */
    VOS_UINT32                          ulFcCfgMemThresholdCnt;                 /* Range:[0,8]*/
    FC_CFG_MEM_THRESHOLD_STRU           stFcCfgMem[FC_MEM_THRESHOLD_LEV_BUTT];  /* A核内存流控门限 */
    FC_CFG_MEM_THRESHOLD_CST_STRU       stFcCfgCst;                             /* CSD业务流控门限 */
    FC_CFG_MEM_THRESHOLD_STRU           stFcCfgGprsMemSize;                     /* G模内存总量流控门限 */
    FC_CFG_MEM_THRESHOLD_STRU           stFcCfgGprsMemCnt;                      /* G模内存块数流控门限 */
    FC_CFG_CPU_STRU                     stFcCfgCpuC;                            /* C核CPU流控门限 */
    FC_CFG_UM_UL_RATE_STRU              stFcCfgUmUlRateForCpu;                  /* C核CPU流控上行速率档位配置 */
    FC_CFG_UM_UL_RATE_STRU              stFcCfgUmUlRateForTmp;                  /* C核温度流控上行行速率档位配置 */
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)    
    FC_CFG_MEM_THRESHOLD_STRU           stFcCfgCdmaMemSize;                     /* X模内存总量流控门限 */
    FC_CFG_MEM_THRESHOLD_STRU           stFcCfgCdmaMemCnt;                      /* X模内存块数流控门限 */
#else
    FC_CFG_MEM_THRESHOLD_STRU           stRsv1;                     
    FC_CFG_MEM_THRESHOLD_STRU           stRsv2;                      
#endif
} FC_CFG_NV_STRU;

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __NV_STRU_GUCTTF_H__ */

