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

#include "vos.h"
#include "acore_nv_id_gucttf.h"

#pragma pack(push, 4)

enum TTF_AcoreBool {
    TTF_ACORE_FALSE = 0,
    TTF_ACORE_TRUE  = 1,

    TTF_ACORE_BOOL_BUTT
};
typedef VOS_UINT8 TTF_AcoreBoolUint8;


/*
 * 对应NV_ITEM_NETFILTER_HOOK_MASK项的结构,设置勾包点的NV项
 * 结构说明  : 设置勾包点的NV项，预留5种掩码组合：netfilterPara1-netfilterPara5，每个掩码取值范围为0-FFFFFFFF,
 * 其中掩码位为1则代表该掩码位对应的钩子函数可能会被注册到内核中
 */
typedef struct {
    VOS_UINT32 netfilterPara1; /* 钩子函数掩码参数1，对应的是网桥和ARP的钩子函数，当存在网桥时，掩码值设置为100，当没有网桥时，掩码值设置为96。 */
    VOS_UINT32 netfilterPara2; /* 钩子函数掩码参数2，对应的是IP协议栈入口处的钩子函数，取值为4224。 */
    VOS_UINT32 netfilterPara3; /* 钩子函数掩码参数3，对应的是IP协议栈出口处的钩子函数，取值为8448。 */

    VOS_UINT32 netfilterPara4; /* 钩子函数掩码参数4，对应的是网桥流控的钩子函数，当存在网桥流控时，掩码值设置为131072，当没有网桥流控时，掩码值设置为0。 */
    VOS_UINT32 netfilterPara5; /* 钩子函数掩码参数5，预留，掩码值设置为0。 */
} NfExtNv;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
