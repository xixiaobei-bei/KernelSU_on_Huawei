/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#ifndef __ATVOICETAFQRYCMDPROC_H__
#define __ATVOICETAFQRYCMDPROC_H__

#include "vos.h"
#include "taf_type_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
TAF_UINT32 At_QryS0Para(TAF_UINT8 indexNum);
TAF_UINT32 At_QryCmodPara(TAF_UINT8 indexNum);
TAF_UINT32 At_QryCbstPara(TAF_UINT8 indexNum);
extern VOS_UINT32 At_TestVtsPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_AbortVtsPara(VOS_UINT8 index);
VOS_UINT32 AT_AbortDtmfPara(VOS_UINT8 index);
VOS_UINT32 AT_QryCCsErrPara(VOS_UINT8 indexNum);
TAF_UINT32 At_QryCstaPara(TAF_UINT8 indexNum);
#if (FEATURE_LTE == FEATURE_ON)
TAF_UINT32 At_QryLtecsInfo(TAF_UINT8 indexNum);
#endif
VOS_UINT32 AT_QryClccPara(VOS_UINT8 indexNum);
#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryClccEconfInfo(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEconfErrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCcwaiPara(VOS_UINT8 indexNum);
#endif
extern VOS_UINT32 AT_TestRejCallPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_QryCsChannelInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEmcCallBack(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCvhuPara(VOS_UINT8 indexNum);

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_TestEclstartPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEclcfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEclmsdPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestEclmsdPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEclModePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestEclModePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEclListPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_QryTrustCallNumPara(VOS_UINT8 index);
VOS_UINT32 AT_TestCallTrustNumPara(VOS_UINT8 index);
extern VOS_UINT32 AT_TestCpasPara(VOS_UINT8 indexNum);
/*
 * 功能描述: 设置命令处理函数
 */
VOS_UINT32 AT_TestClprPara(VOS_UINT8 indexNum);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
