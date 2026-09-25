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

#ifndef __PPP_ENCODE_H__
#define __PPP_ENCODE_H__

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
#define PPP_ENC_MAX_SEGS           2


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/
/* 同步转换异步输入信息，上层需要预先申请内存 */
typedef struct {
    VOS_UINT8                      *data;
    VOS_UINT16                      dataLen;
    VOS_UINT16                      proto;
} PppEncInput;

/* 上报的转换后的分段信息 */
typedef struct tagPppEncOutputSeg{
    PppUplayerMem                   seg;
    VOS_UINT32                      used;       /* 使用的内存大小 */
} PppEncOutputSeg;

/* 同步转换异步结果 */
typedef struct {
    VOS_UINT32                      segCnt;                         /* 由于内存尺寸大小限制，可能有多个分段 */
    PppEncOutputSeg                 outSegs[PPP_ENC_MAX_SEGS];      /* 转换结果 */
} PppEncOutput;

/* 编码实体 */
typedef struct {
    PppUplayerMemAlloc              alloc;
    PppUplayerMemFree               free;
    VOS_UINT32                      memMaxLen;
    VOS_UINT32                      accMap;
    VOS_BOOL                        acf;
    VOS_BOOL                        pcf;
} PppEncCfg;


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
VOS_UINT32 PPP_ENC_Proc(PppEncCfg *cfg, const PppEncInput *input, PppEncOutput *output);


#pragma pack()

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif  /*end of __PPP_ENCODE_H__*/

