/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef __AT_PPP_INTERFACE_H__
#define __AT_PPP_INTERFACE_H__

#include "vos.h"
#include "taf_type_def.h"
#include "ppp_interface.h"
#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#include "imm_interface.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

typedef PPPA_AuthPapContent AT_PPP_AuthPapContent;

typedef PPPA_AuthChapContent AT_PPP_AuthChapContent;

typedef PPPA_ReqAuthConfigInfo AT_PPP_ReqAuthConfigInfo;

typedef PPPA_ReqIpcpConfigInfo AT_PPP_ReqIpcpConfigInfo;

typedef PPPA_PdpActiveConfig AT_PPP_ReqConfigInfo;

/* PCO中携带的IPCP信息 */
typedef PPPA_PcoIpv4Item AT_PPP_PcoIpv4Item;

typedef PPPA_PdpActiveResult AT_PPP_IndConfigInfo;

extern VOS_UINT32 At_RcvTeConfigInfoReq(VOS_UINT16 pppId, AT_PPP_ReqConfigInfo *pppReqConfigInfo);
extern VOS_UINT32 At_RcvPppReleaseInd(VOS_UINT16 pppId);
extern VOS_UINT32 At_PsRab2PppId(TAF_UINT8 exRabId, TAF_UINT16 *pppId);
extern VOS_UINT32 At_PppId2PsRab(TAF_UINT16 pppId, TAF_UINT8 *exRabId);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
extern VOS_UINT32 At_PppId2IfaceId(VOS_UINT16 pppId, VOS_UINT8 *ifaceId);
extern VOS_UINT32 At_IfaceId2PppId(VOS_UINT8 ifaceId, VOS_UINT16 *pppId);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
