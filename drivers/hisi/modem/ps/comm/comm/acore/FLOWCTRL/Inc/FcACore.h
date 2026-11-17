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

#ifndef __FC_ACORE_H__
#define __FC_ACORE_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "Fc.h"
#include "acore_nv_stru_gucttf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define FC_ACORE_GetEnableMask(Val, Mask) ((Val & (0x01 << Mask)) >> Mask)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum FC_ACORE_ENABLE_MASK_ENUM
{
    FC_ACORE_MEM_ENABLE_MASK                    = 0,
    FC_ACORE_CPUA_ENABLE_MASK,
    FC_ACORE_CDS_ENABLE_MASK,
    FC_ACORE_CST_ENABLE_MASK,
    FC_ACORE_ENABLE_MASK_BUTT
};
typedef VOS_UINT8 FC_ACORE_ENABLE_MASK_ENUM_UINT8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)

/*****************************************************************************
 结构名    : FC_CPU_DRV_ASSEM_PARA_NV_STRU
 结构说明  : FC_CPU_DRV_ASSEM配置维测结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    FC_MNTN_EVENT_TYPE_ENUM_UINT16      enMsgName;                              /* _H2ASN_Skip */
    VOS_UINT8                           aucRsv[2];
    FC_DRV_ASSEM_PARA_STRU              stDrvAssemPara;
}FC_MNTN_DRV_ASSEM_PARA_STRU;

/*****************************************************************************
 结构名    : FC_CPU_DRV_ASSEM_PARA_NV_STRU
 结构说明  : FC_CPU_DRV_ASSEM配置LEV参数
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulLev;
    VOS_UINT8                           aucRsv[4];
    FC_CPU_DRV_ASSEM_PARA_STRU         *pstCpuDrvAssemPara;
}FC_CPU_DRV_ASSEM_PARA_LEV_STRU;

/*****************************************************************************
 结构名    : FC_CPU_DRV_ASSEM_PARA_NV_STRU
 结构说明  : FC_CPU_DRV_ASSEM配置实体
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucSmoothCntUp;
    VOS_UINT8                               ucSmoothCntDown;
    VOS_UINT8                               ucSetDrvFailCnt;
    VOS_UINT8                               ucRsv;
    VOS_UINT32                              ulCurLev;
    VOS_UINT8                               aucRsv[4];
    FC_CPU_DRV_ASSEM_PARA_STRU              stCurAssemPara;
    FC_ACORE_DRV_ASSEMBLE_PARA_FUNC         pDrvSetAssemParaFuncUe;             /* _H2ASN_Replace VOS_UINT32  pDrvSetAssemParaFuncUe; */
    FC_ACORE_DRV_ASSEMBLE_PARA_FUNC         pDrvSetAssemParaFuncPc;             /* _H2ASN_Replace VOS_UINT32  pDrvSetAssemParaFuncPc; */
}FC_CPU_DRV_ASSEM_PARA_ENTITY_STRU;

/*****************************************************************************
 结构名    : FC_CPU_DRV_ASSEM_PARA_NV_STRU
 结构说明  : FC_CPU_DRV_ASSEM配置参数
*****************************************************************************/
typedef struct
{
    VOS_UINT8                              ucEnableMask;
    VOS_UINT8                              ucSmoothCntUpLev;
    VOS_UINT8                              ucSmoothCntDownLev;
    VOS_UINT8                              ucRsv;
    FC_CPU_DRV_ASSEM_PARA_STRU             stCpuDrvAssemPara[FC_ACPU_DRV_ASSEM_LEV_BUTT];
}FC_CPU_DRV_ASSEM_CONFIG_PARA_STRU;

#endif

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/

#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of FcAcore.h */

