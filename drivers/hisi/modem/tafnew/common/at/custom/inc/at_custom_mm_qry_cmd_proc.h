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
#ifndef _AT_CUSTOM_MM_QRY_CMD_PROC_H_
#define _AT_CUSTOM_MM_QRY_CMD_PROC_H_

#include "vos.h"
#include "taf_type_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryRcsSwitch(VOS_UINT8 indexNum);
#endif
VOS_UINT32 At_QryHplmnPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryDplmnListPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryBorderInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryFratIgnitionInfo(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEmRssiRptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryPsScenePara(TAF_UINT8 indexNum);
TAF_UINT32 At_QryAutoAttach(TAF_UINT8 indexNum);

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_QryImsSwitchPara(VOS_UINT8 indexNum);
#endif

VOS_UINT32 At_QryAppWronReg(VOS_UINT8 indexNum);
VOS_UINT32 At_QryApHplmn(VOS_UINT8 indexNum);
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_QryCLteRoamAllowPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_QryUlFreqPara(VOS_UINT8 indexNum);
#if (FEATURE_CSG == FEATURE_ON)
VOS_UINT32 At_QryCampCsgIdInfo(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_TestCsgIdSearchPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_AbortCsgIdSearchPara(VOS_UINT8 index);
#endif


#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryNrImsSwitchPara(VOS_UINT8 indexNum);
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_TestEonsPara(VOS_UINT8 indexId);
VOS_UINT32 AT_QryCellRoamPara(VOS_UINT8 indexId);
VOS_UINT32 AT_QryHcsqPara(VOS_UINT8 indexId);
VOS_UINT32 AT_TestHcsqPara(VOS_UINT8 indexId);
#endif
VOS_UINT32 AT_QryCarOosGearsPara(TAF_UINT8 indexNum);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
