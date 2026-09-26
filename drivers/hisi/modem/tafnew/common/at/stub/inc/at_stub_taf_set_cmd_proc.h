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

#ifndef _ATSTUBTAFSETCMDPROC_H_
#define _ATSTUBTAFSETCMDPROC_H_

#include "vos.h"
#include "taf_ps_api.h"
#include "pppa_ppps_at_type.h"
#include "taf_ps_type_def.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_parse_core.h"

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (VOS_OS_VER != VOS_LINUX)
#include "Linuxstub.h"
#endif
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
#include "ads_dev_i.h"
#else
#include "ads_device_interface.h"
#endif
#else
#include "cds_interface.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#include "UsimmApi.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (VOS_OS_VER == VOS_WIN32)
VOS_UINT32 AT_SetPsBearIsrFlgStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetImsRatStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetImsCapabilityStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDomainStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCmdImsRoamingStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCmdRedailStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCmdImsUssdStub(VOS_UINT8 indexNum);
#ifdef DMT
VOS_VOID NAS_STUB_ReadUsimPara(VOS_UINT8 value);
VOS_VOID NAS_STUB_PidReinit(VOS_UINT8 indexNum);
VOS_VOID NAS_MULTIINSTANCE_DMT_SetCurrInstanceModemId(VOS_UINT16 modemId);
VOS_VOID NAS_STUB_UpdateGlobalVar(NV_IdU16 eNvimTempId);
#endif
VOS_UINT32 At_SetReadUsimStub(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNvimPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetPidReinitPara(VOS_UINT8 indexNum);
TAF_UINT32 AT_SetUsimPara(TAF_UINT8 indexNum);
TAF_UINT32 AT_SetSimPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_RefreshUsimPara(VOS_UINT8 indexNum);



#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
