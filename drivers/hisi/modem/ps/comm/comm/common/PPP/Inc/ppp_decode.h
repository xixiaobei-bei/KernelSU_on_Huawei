/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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

#ifndef __PPP_DECODE_H__
#define __PPP_DECODE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "ppp_comm_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum PppDecState {
    PPP_DEC_NULL,
    PPP_DEC_SYN,
    PPP_DEC_FRM,
    PPP_DEC_BUTT
};
typedef VOS_UINT32 PPP_DEC_STATE_ENUM_UINT32;

/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/
/* 上报给上层有效帧的解码结果 */
typedef struct {
    PppUplayerMem                   buf;        /* 通过PppDecBufAlloc申请的信息完整带回 */
    VOS_UINT32                      used;       /* 已存储的数据长度，不包括FCS */
    VOS_UINT16                      proto;      /* 协议字段取值 */
    VOS_UINT8                       hdrAcLen;   /* 地址域+控制域长度，取值范围0~2 */
    VOS_UINT8                       hdrLen;     /* 地址域+控制域+协议域长度，取值范围1~4 */
    VOS_UINT32                      pppId;      /* 保存上层需要带回的信息，当前用于存储PppId */
} PppDecFrame;

/* 上报给上层有效帧的解码结果钩子函数，上层应该自己负责释放PppUplayerMem对应的内存 */
typedef VOS_VOID (*PppDecFrameDeliver)(PppDecFrame *frm);

/* 解码控制配置结构 */
typedef struct {
    PppUplayerMemAlloc              alloc;
    PppUplayerMemFree               free;
    PppDecFrameDeliver              deliver;
    VOS_UINT32                      memMaxLen;
} PppDecCfg;


/* 解码控制实体 */
typedef struct {
    PPP_DEC_STATE_ENUM_UINT32       state;
    VOS_BOOL                        escMode;
    PppDecCfg                       ops;
    PppUplayerMem                   buf;        /* 已存储的数据长度 */
    VOS_UINT16                      used;       /* 已存储的数据长度 */
    VOS_UINT16                      fcs;        /* 保存的过程中计算的FCS */
    VOS_UINT32                      transData;  /* 透传给上层的数据，本层不使用 */
} PppDecCtrl;

/*****************************************************************************
  5 全局变量声明
*****************************************************************************/

/*****************************************************************************
  6 消息头定义
*****************************************************************************/

/*****************************************************************************
  7 消息定义
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_UINT32 PPP_DEC_Proc(PppDecCtrl *ctrl, const VOS_UINT8 *data, VOS_UINT16 dataLen, VOS_UINT32 pppId);
VOS_VOID PPP_DEC_CtrlCreate(PppDecCtrl *ctrl, const PppDecCfg *ops);
VOS_VOID PPP_DEC_CtrlRelease(PppDecCtrl *ctrl);

#pragma pack()

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif  /*end of __PPP_DECODE_H__*/

