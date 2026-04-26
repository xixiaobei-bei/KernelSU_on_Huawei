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

#ifndef _ATSTUBMMSETCMDPROC_H_
#define _ATSTUBMMSETCMDPROC_H_

#include "vos.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (VOS_OS_VER == VOS_WIN32)
VOS_UINT32 AT_SetRplmnStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetTinTypeStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCsUnAvailPlmnStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetForbRoamTaStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDisableRatPlmnStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCsgStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDamParaStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEonsParaStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetWildCardParaStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetImsVoiceInterSysLauEnableStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetImsVoiceMMEnableStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_ResetNplmn(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNplmn(VOS_UINT8 indexNum);
VOS_VOID NAS_STUB_AT_ResetNplmn(VOS_VOID);
VOS_VOID NAS_STUB_AT_SetNplmn(VOS_UINT8 nplmnNum, TAF_MMA_PlmnWithSimRat *nplmnList);




#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
