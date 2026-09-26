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
#ifndef __ATVOICETAFSETCMDPROC_H__
#define __ATVOICETAFSETCMDPROC_H__
#include "vos.h"
#include "taf_type_def.h"
#include "taf_call_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

TAF_UINT32 At_SetS0Para(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmodPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCbstPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetClccPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetChupPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetCstaPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetVtsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetAPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_ModemHangupCall(VOS_UINT8 indexNum);
VOS_UINT32 At_SetHPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetDtmfPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetDPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetDialNumPara(TAF_UINT8 indexNum, MN_CALL_TypeUint8 callType);
VOS_UINT32 AT_SetCdurPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetApdsPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetClprPara(VOS_UINT8 indexNum);
#if (FEATURE_IMS == FEATURE_ON)
TAF_UINT32 AT_SetCacmimsPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetEconfDialPara(VOS_UINT8 indexNum);
TAF_UINT32 AT_SetCcwaiPara(TAF_UINT8 indexNum);

#endif

extern VOS_UINT32 AT_SetRejCallPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCvhuPara(VOS_UINT8 indexNum);

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_SetEclstartPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEclstopPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEclcfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEclmsdPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEclAbortPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEclModePara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetTrustCallNumPara(VOS_UINT8 index);
VOS_UINT32 At_SetCpasPara(VOS_UINT8 indexNum);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
