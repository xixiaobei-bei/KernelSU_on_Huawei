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
#ifndef _AT_CUSTOM_AS_SET_CMD_PROC_H_
#define _AT_CUSTOM_AS_SET_CMD_PROC_H_

#include "vos.h"
#include "at_mta_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_SetFineTimeReqPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetEcidPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetNetMonSCellPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetNetMonNCellPara(VOS_UINT8 indexNum);
VOS_VOID AT_ConvertMncToNasType(VOS_UINT32 mnc, VOS_UINT32 mncLen, VOS_UINT32 *nasMnc);

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 At_SetNetMonSSCellPara(VOS_UINT8 atIndex);
VOS_UINT32 At_SetNrCellBandPara(VOS_UINT8 atIndex);
#endif

VOS_UINT32 AT_SetErrcCapCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetErrcCapQryPara(VOS_UINT8 indexNum);

VOS_UINT32 AT_CheckPseucellInfoParaLen(VOS_VOID);
VOS_UINT32 AT_ParsePseucellInfoMccMnc(VOS_UINT8 *para, VOS_UINT16 paraLen, VOS_UINT32 *mcc, VOS_UINT32 *mnc);
VOS_UINT32 AT_ParsePseucellInfoLac(VOS_UINT8 *para, VOS_UINT16 paraLen, VOS_UINT32 *lac);
VOS_UINT32 AT_ParsePseucellInfoCellId(VOS_UINT8 *para, VOS_UINT16 paraLen, VOS_UINT32 *cellId);
VOS_UINT32 AT_SetPseucellInfoPara(VOS_UINT8 indexNum);

VOS_UINT32 AT_SetEccCfgPara(VOS_UINT8 indexNum);

VOS_UINT32 AT_SetPseudBtsPara(VOS_UINT8 indexNum);

VOS_UINT32 AT_SetSubClfsParamPara(VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_SetConnectRecovery(VOS_UINT8 indexNum);
#endif
VOS_UINT32 At_SetTfDpdtPara(VOS_UINT8 indexNum);

VOS_UINT32 AT_CheckUeCapPara(VOS_VOID);
VOS_UINT32 AT_FillUeCapPara(AT_MTA_UeCapParamSet *setUeCap, VOS_UINT32 bsLen);
VOS_BOOL   AT_CheckModem0(VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_SetLrrcUeCapPara(VOS_UINT8 indexNum);
VOS_VOID   AT_FillOverHeatingReqParam(AT_MTA_OverHeatingSetReq *overHeatingReq);
VOS_UINT32 AT_CheckOverHeatingPara(VOS_VOID);
VOS_UINT32 AT_SetOverHeatingCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetOverHeatingQryParam(VOS_UINT8 indexNum);
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetNrrcCapCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrrcCapQryPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrrcUeCapPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrCaCellRptCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrNwCapRptCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrNwCapRptQryPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrNwCapQryPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrPowerSavingCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNrPowerSavingQryPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetAutoN2LOptCfgPara(VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_SetNCellMonitorPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetLFromConnToIdlePara(VOS_UINT8 indexNum);
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
TAF_UINT32 At_SetRRCVersion(TAF_UINT8 indexNum);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
