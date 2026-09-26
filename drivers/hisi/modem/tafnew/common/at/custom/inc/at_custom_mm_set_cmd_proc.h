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
#ifndef _AT_CUSTOM_MM_SET_CMD_PROC_H_
#define _AT_CUSTOM_MM_SET_CMD_PROC_H_

#include "vos.h"
#include "taf_type_def.h"
#include "taf_app_mma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_NETSEL_PARA_NUM 2
#define AT_NETSEL_PARA_SCENE 0
#define AT_NETSEL_PARA_OPERATE 1

enum AT_NwscanRatType {
    AT_NWSCAN_WCDMA     = 0,    /* 0: WCDMA */
    AT_NWSCAN_CDMA      = 1,    /* 1: CDMA */
    AT_NWSCAN_GSM       = 3,    /* 3: GSM */
    AT_NWSCAN_EDGE      = 4,    /* 4: EDGE */
    AT_NWSCAN_LTE_FDD   = 6,    /* 6: FDD LTE */
    AT_NWSCAN_LTE_TDD   = 7,    /* 7: TDD LTE */
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
    AT_NWSCAN_NR        = 8,    /* 8: NR */
#endif
    AT_NWSCAN_BUTT
};
typedef VOS_UINT8 AT_NwscanRatTypeUint8;

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32        AT_SetRcsSwitch(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetDplmnListPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetFratIgnitionPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetBorderInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEmRssiRptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetPlmnSrchPara(TAF_UINT8 indexNum);
VOS_UINT32 At_SetDetectPlmnRsrp(TAF_UINT8 indexNum);
#if (FEATURE_DSDS == FEATURE_ON)
VOS_UINT32 AT_SetDsdsStatePara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 At_SetPsScenePara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetSmsAntiAttackState(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetSmsAntiAttackCapQrypara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetScidPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetCacdcPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetOosSrchStgyPara(VOS_UINT8 indexNum);
TAF_UINT32 At_SetAutoAttach(TAF_UINT8 indexNum);

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetImsSwitchPara(VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_SetQuickCardSwitchPara(VOS_UINT8 indexNum);
#if (FEATURE_CSG == FEATURE_ON)
VOS_UINT32 AT_SetCsgIdSearchPara(VOS_UINT8 indexNum);
#endif
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_SetUsimStub(VOS_UINT8 indexNum);
VOS_UINT32 At_SetRefreshStub(VOS_UINT8 indexNum);
VOS_UINT32 At_SetAutoReselStub(VOS_UINT8 indexNum);
TAF_UINT32 At_SetCmmPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetCLteRoamAllowPara(VOS_UINT8 indexNum);
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetNrImsSwitchPara(VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_SetUlFreqRptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_ParseDplmnLengthIllegal(VOS_UINT16 subStrLen);
VOS_UINT32 AT_ParseDplmnStringList(VOS_UINT8 indexNum, VOS_UINT8 *dplmnString, TAF_MMA_DplmnInfoSet *dplmnInfoSet);
VOS_UINT32 AT_ParseEhplmnStringList(VOS_UINT32 paraLen, VOS_UINT8 *para, VOS_UINT8 *ehPlmnNum, TAF_PLMN_Id *ehplmnList);
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_SetEonsPara(VOS_UINT8 clientIdx);
VOS_UINT32 AT_SetNwScanPara(VOS_UINT8 index);
VOS_UINT32 AT_SetNetSelOptPara(VOS_UINT8 index);
#endif
VOS_UINT32 AT_SetCarOosGearsPara(VOS_UINT8 indexNum);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
