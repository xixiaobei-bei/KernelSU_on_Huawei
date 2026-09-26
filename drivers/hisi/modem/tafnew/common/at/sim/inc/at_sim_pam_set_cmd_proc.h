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
#ifndef __ATSIMPAMSETCMDPROC_H__
#define __ATSIMPAMSETCMDPROC_H__

#include "vos.h"
#include "taf_type_def.h"
#include "AtParse.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

TAF_UINT32 At_SetCpbsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCpbrPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCpbwPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetScpbwPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetScpbrPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCpbr2Para(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCpbwPara2(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCnumPara(TAF_UINT8 indexNum);
TAF_UINT32 At_OneUnicodeToGsm(TAF_UINT16 src, TAF_UINT8 *dst, TAF_UINT32 dstBufLen, TAF_UINT16 *dstLen);
TAF_UINT32 At_SetCpbfPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCrsmPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetSimstPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetParaRspSimsqPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetPNNPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCPNNPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetOPLPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetSPNPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCardModePara(TAF_UINT8 indexNum);
extern VOS_UINT32 At_SetHvsstPara(VOS_UINT8 indexNum);
#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetUrsmPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 At_SetEflociInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetPsEflociInfoPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetCchoPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCchpPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetPrivateCchoPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetPrivateCchpPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCchcPara(TAF_UINT8 indexNum);
extern VOS_UINT32 At_SetBwtPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetCardHistoryATRPara(TAF_UINT8 indexNum);
TAF_UINT32 At_UnicodeTransferTo81CalcLength(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 *baseCode,
                                            TAF_UINT16 *dstLen);
TAF_UINT32 At_UnicodeTransferTo82CalcLength(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 *baseCode,
                                            TAF_UINT16 *dstLen);
TAF_UINT32 At_UnicodeTransferTo81(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 codeBase, TAF_UINT8 *dest);
TAF_UINT32 At_UnicodeTransferTo82(const TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT16 codeBase, TAF_UINT8 *dest);
extern VOS_UINT32 At_SetCrlaPara(VOS_UINT8 indexNum);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
