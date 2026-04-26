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

#ifndef __PPP_CONVERT_H__
#define __PPP_CONVERT_H__

#include "vos.h"
#include "ppp_comm_def.h"
#include "ppp_decode.h"
#include "ppp_encode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

enum PPP_ConvState {
    PPP_CONV_NULL,
    PPP_CONV_BUSY,
    PPP_CONV_BUTT
};
typedef VOS_UINT32 PPP_ConvStateUint32;

/* 创建同步异步转换操作实体所需的回调接口 */
typedef struct {
    PPP_DecCfg decCfg; /* 解码配置 */
    PPP_EncCfg encCfg; /* 编码配置 */
} PPP_ConvOps;

/* 解码控制实体 */
typedef struct {
    PPP_ConvStateUint32 state;
    PPP_DecCtrl         decCtrl;
    PPP_EncCfg          encCfg;
    VOS_UINT32          userData;
} PPP_ConvCtrl;

/* 上下文相关同异步帧转换实体建立, 上层需要保证输入参数的正确性和有效性，尚未协商的参数使用默认值 */
VOS_VOID PPP_ConvCreate(PPP_ConvCtrl* self, const PPP_ConvOps *ops);

/* 获取当前pppId实体的配置信息 */
VOS_VOID PPP_ConvGetCfg(const PPP_ConvCtrl* self, PPP_ConvOps *ops);

/* 重配pppId实体，主要用于LCP协商完成后，相关链路参数变更后刷新实体 */
VOS_VOID PPP_ConvReCfg(PPP_ConvCtrl* self, const PPP_ConvOps *ops);

/* 上下文相关同异步帧转换实体释放, 上层需要保证输入参数的正确性和有效性 */
VOS_UINT32 PPP_ConvRelease(PPP_ConvCtrl* self);

/*
 * 上下文相关异步帧转同步帧接口, 上层需要保证输入参数的正确性和有效性
 * 注意：1.data中的内存需要上层模块申请和释放
 *       2.该接口处理过程中，可能多次回调PppDecFrmDeliver，回调参数涉及的动态内存也需要上层释放
 */
VOS_UINT32 PPP_ConvDecode(PPP_ConvCtrl* self, const VOS_UINT8 *data, VOS_UINT16 dataLen, VOS_UINT32 userData);

/*
 * 上下文相关同步帧转异步帧接口, 上层需要保证输入参数的正确性和有效性
 * 注意：1.input中的内存如果是动态申请内存需要上层模块申请和释放
 *       2.当返回VOS_OK时，output中包括动态申请内存，需要上层进行释放
 */
VOS_UINT32 PPP_ConvEncode(PPP_ConvCtrl* self, const PPP_EncInput *input, PPP_EncOutput *output);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
