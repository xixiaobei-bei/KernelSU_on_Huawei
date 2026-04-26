/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
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
#define __PSLOGFILTERCOMM_H__

#include "vos.h"
#include "ps_log_filter_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/*
 * 每个SendPid下存储的过滤函数, 大部分场景一个SendPid只允许
 * 注册一个过滤函数，跨核和消息透传转发等特殊场景允许注册多个
 */
typedef struct {
    VOS_UINT32             senderPid;  /* 注册的SendPid，存储的是低16位 */
    VOS_UINT16             useCnt;     /* 每个SendPid下注册的过滤函数的个数 */
    VOS_UINT16             maxCnt;     /* 允许注册的过滤函数个数，此值可动态调整，申请个数达到上限后可以扩容 */
    PsOmLayerMsgReplaceCb *filterFunc; /* 指向存储过滤函数的动态数组 */
} PsOmPerPidRegCtrl;

/* 层间消息过滤功能的控制结构，以SendPid为单元注册的过滤回调函数 */
typedef struct {
    VOS_UINT16 useCnt;               /* 已注册的SendPid的个数 */
    VOS_UINT16 maxCnt;               /* 允许注册的SendPid个数 */
    PsOmPerPidRegCtrl *pidMsgFilter; /* 指向以SendPid为单元注册的过滤回调函数数组，数组大小由usMaxCnt确定，使用数量由usUseCnt确定 */
} PsOmCpuLayerMsgFilterCtrl;

/* 层间消息过滤功能的控制结构 */
typedef struct {
    VOS_UINT16                localCpuMaxPidCnt; /* 取值与VOS_PID_BUTT低16位相同，表示本CPU上最大PID的数量 */
    VOS_UINT8                 localCpuId;        /* 取值与VOS_CPU_ID_ENUM相同，0代表C核，1代表A核 */
    VOS_UINT8                 reserve[1];
    PsOmCpuLayerMsgFilterCtrl localCpuFilter; /* 同核层间消息过滤控制结构 */
    PsOmCpuLayerMsgFilterCtrl diffCpuFilter;  /* 跨核层间消息过滤控制结构 */
} PsOmLayerMsgReplaceCtrl;

MsgBlock* PS_OM_CpuLayerMsgCommReplace(MsgBlock *msg, PsOmLayerMsgReplaceCtrl *layerMsgReplaceCtrl);

/*
 * A C核跨核层间消息匹配过滤函数注册接口
 *
 * 输入参数回调接口规则:
 * 1.如果注册回调内部没有对消息进行处理，则需要将入参指针返回，否则
 *  本模块不知道是否需要将此消息传递给下一个注册回调进行处理
 * 2.如果注册回调内部对消息进行了处理，则返回值能够实现两个功能:
 *  ①返回VOS_NULL，则将此消息进行完全过滤，不会再勾取出来
 *  ②返回与入参指针不同的另一个指针，则勾取的消息将会使用返回的指
 *  针内容替换原消息的内容。
 */
VOS_UINT32 PS_OM_LayerMsgReplaceCBCommReg(
    PsOmLayerMsgReplaceCtrl *layerMsgCtrl, VOS_UINT32 senderPid, PsOmLayerMsgReplaceCb func);

VOS_VOID PS_OM_LogFilterCommShow(PsOmLayerMsgReplaceCtrl *layerMsgCtrl);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
