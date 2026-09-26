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
#ifndef __ATSMSTAFSETCMDPROC_H__
#define __ATSMSTAFSETCMDPROC_H__
#include "vos.h"
#include "taf_type_def.h"
#include "AtParse.h"
#include "mn_msg_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

TAF_UINT32 At_SetCsdhPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCnmiPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmmsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmgfPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCgsmsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCscaPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCsmpPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmgdPara(TAF_UINT8 indexNum);
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
TAF_UINT32 At_SetCscbPara(TAF_UINT8 indexNum);
#endif
TAF_UINT32 At_SetCmgsPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmgwPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmglPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmgcPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmssPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmgrPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCnmaPara(TAF_UINT8 indexNum);
VOS_UINT32 At_ParseCsmpVp(VOS_UINT8 indexNum, MN_MSG_ValidPeriod *vp);
VOS_UINT32 At_ParseCsmpFo(VOS_UINT8 *fo);
TAF_UINT32 At_SetCmstPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmsrPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmgiPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetCmmtPara(TAF_UINT8 indexNum);
extern TAF_UINT32 At_GetAsciiOrBcdAddr(TAF_UINT8 *addr, TAF_UINT16 addrLen, TAF_UINT8 addrType, TAF_UINT16 numTypeLen,
                                       MN_MSG_AsciiAddr *asciiAddr, MN_MSG_BcdAddr *bcdAddr);
extern TAF_VOID AT_StubTriggerAutoReply(VOS_UINT8 indexNum, TAF_UINT8 cfgValue);
VOS_UINT32 AT_SetMemStatusPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetCsmsPara(TAF_UINT8 indexNum);

TAF_UINT32 At_SetCpmsPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetCmStubPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetSmsDomainPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetSmsTrustNumPara(VOS_UINT8 index);
#if (FEATURE_MBB_CUST == FEATURE_ON)
/* HSMF设置命令处理函数 */
VOS_UINT32 AT_SetHsmfPara(VOS_UINT8 index);
#endif

TAF_UINT32 At_AsciiNum2BcdNum(TAF_UINT8 *dst, TAF_UINT8 *src, TAF_UINT16 srcLen);
extern TAF_UINT32 AT_StubSendAutoReplyMsg(VOS_UINT8 indexNum, MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo);


#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
