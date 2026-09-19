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

#ifndef __PSLOGFILTERCOMM_H__
#define __PSLOGFILTERCOM_H__


/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "vos.h"
#include "PsLogFilterInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)


/******************************************************************************
  2 宏定义
******************************************************************************/


/******************************************************************************
  3 枚举定义
******************************************************************************/


/******************************************************************************
  4 全局变量声明
******************************************************************************/


/******************************************************************************
  5 消息头定义
******************************************************************************/


/******************************************************************************
  6 消息定义
******************************************************************************/


/******************************************************************************
  7 STRUCT定义
******************************************************************************/

/*****************************************************************************
 结构名    : PS_OM_PER_PID_REG_CTRL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 每个SendPid下存储的过滤函数, 大部分场景一个SendPid只允许
             注册一个过滤函数，跨核和消息透传转发等特殊场景允许注册多个
*****************************************************************************/
typedef struct
{
    VOS_UINT32                         ulSendPid; /*注册的SendPid，存储的是低16位*/
    VOS_UINT16                         usUseCnt;  /*每个SendPid下注册的过滤函数的个数*/
    VOS_UINT16                         usMaxCnt;  /*允许注册的过滤函数个数，此值可动态调整，申请个数达到上限后可以扩容*/
    PS_OM_LAYER_MSG_REPLACE_CB        *pafuncFilterEntry; /*指向存储过滤函数的动态数组*/
}PS_OM_PER_PID_REG_CTRL_STRU;


/*****************************************************************************
 结构名    : PS_OM_ONE_CPU_LAYER_MSG_FILTER_CTRL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 层间消息过滤功能的控制结构，以SendPid为单元注册的过滤回调函数
*****************************************************************************/
typedef struct
{
    VOS_UINT16                           usUseCnt; /* 已注册的SendPid的个数 */
    VOS_UINT16                           usMaxCnt; /* 允许注册的SendPid个数 */
    PS_OM_PER_PID_REG_CTRL_STRU         *pastPerPidMsgFilter;  /* 指向以SendPid为单元注册的过滤回调函数数组，数组大小由usMaxCnt确定，使用数量由usUseCnt确定 */
}PS_OM_CPU_LAYER_MSG_FILTER_CTRL_STRU;


/*****************************************************************************
 结构名    : PS_OM_LAYER_MSG_REPLACE_CTRL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 层间消息过滤功能的控制结构
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usLocalCpuMaxPidCnt;        /* 取值与VOS_PID_BUTT低16位相同，表示本CPU上最大PID的数量 */
    VOS_UINT8                                       ucLocalCpuId;               /* 取值与VOS_CPU_ID_ENUM相同，0代表C核，1代表A核 */
    VOS_UINT8                                       aucReserve[1];
    PS_OM_CPU_LAYER_MSG_FILTER_CTRL_STRU            stLocalCpuLayerMsgFilter;   /* 同核层间消息过滤控制结构 */
    PS_OM_CPU_LAYER_MSG_FILTER_CTRL_STRU            stDiffCpuLayerMsgFilter;    /* 跨核层间消息过滤控制结构 */
}PS_OM_LAYER_MSG_REPLACE_CTRL_STRU;


/*****************************************************************************
 结构名    : PS_OM_LAYER_MSG_REPLACE_REG_STATISTIC_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 层间消息过滤注册统计结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32                                      ulRegSuccCnt;
    VOS_UINT32                                      ulRegFailCnt;
    VOS_UINT32                                      ulLocalCpuFilterCnt;
    VOS_UINT32                                      ulDiffCpuFilterCnt;
}PS_OM_LAYER_MSG_REPLACE_STATISTIC_STRU;


/******************************************************************************
  8 UNION定义
******************************************************************************/


/******************************************************************************
  9 OTHERS定义
*****************************************************************************/

VOS_VOID* PS_OM_CpuLayerMsgCommReplace
(
    VOS_VOID                                    *pMsg,
    PS_OM_LAYER_MSG_REPLACE_CTRL_STRU           *pstLayerMsgReplaceCtrl
);

VOS_UINT32 PS_OM_LayerMsgReplaceCBCommReg
(
    PS_OM_LAYER_MSG_REPLACE_CTRL_STRU          *pstLayerMsgCtrl,
    VOS_UINT32                                  ulSendPid,
    PS_OM_LAYER_MSG_REPLACE_CB                  pFunc
);

VOS_VOID PS_OM_LogFilterCommShow
(
    PS_OM_LAYER_MSG_REPLACE_CTRL_STRU          *pstLayerMsgCtrl
);


#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* PsLogFilterComm.h */


