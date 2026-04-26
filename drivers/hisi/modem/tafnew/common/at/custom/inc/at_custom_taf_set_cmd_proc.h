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
#ifndef _AT_CUSTOM_TAF_SET_CMD_PROC_H_
#define _AT_CUSTOM_TAF_SET_CMD_PROC_H_

#include "vos.h"
#include "taf_type_def.h"
#include "at_cmd_proc.h"
#include "taf_drv_agent.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#define AT_CMD_EPDU_PARA_NUM 11

#if (FEATURE_MULTI_MODEM == FEATURE_ON)
VOS_UINT32 AT_ExchangeModemInfo(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetApRptPortSelectPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCposPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCposrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetXcposrPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetXcposrRptPara(VOS_UINT8 indexNum);

VOS_UINT32 AT_SetGpsInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetLogSavePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetBestFreqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMtReattachPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetSensorPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNvRefreshPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetScreenPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetLogEnablePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetActPdpStubPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCtzuPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetModemTimePara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetCrrconnPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetEpduPara(VOS_UINT8 index);
TAF_UINT32 AT_SetVTFlowRptPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetDataSwitchStatus(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetDataRoamSwitchStatus(VOS_UINT8 indexNum);
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetLendcPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCpolicyRptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_GetCpolicyCodePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetLadnRptPara(VOS_UINT8 idx);
VOS_UINT32 AT_SetC5gRsdPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetCsqlvlPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCsqlvlExtPara(VOS_UINT8 indexNum);
VOS_UINT32 atSetNVRstSTTS(VOS_UINT8 clientId);
VOS_UINT32 AT_SetApRptSrvUrlPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetApXmlInfoTypePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetApXmlRptFlagPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetVertime(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetRrcStatPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetHfreqInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 At_SetHifiReset(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetUeModeStatus(VOS_UINT8 indexNum);
#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetImsPdpCfg(VOS_UINT8 indexNum);
#endif
TAF_UINT32 At_SetpidPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetRATCombinePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetTrustNumCtrlPara(VOS_UINT8 index);
VOS_UINT32 AT_SetHistoryFreqPara(VOS_UINT8 indexNum);
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SetCccPara(TAF_UINT8 indexNum);
TAF_UINT32 At_SetImeiPara(TAF_UINT8 indexNum);
#endif

VOS_UINT32 AT_SetLogNvePara(VOS_UINT8 indexNum);

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_LTE_MBMS == FEATURE_ON)
VOS_UINT32 AT_SetMBMSServiceOptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMBMSServiceStatePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMBMSPreferencePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryMBMSSib16NetworkTimePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryMBMSBssiSignalLevelPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryMBMSNetworkInfoPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryMBMSModemStatusPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMBMSCMDPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMBMSEVPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetMBMSInterestListPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SetLteLowPowerPara(VOS_UINT8 indexNum);
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_SetRsrpCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetRscpCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEcioCfgPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetFrStatus(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_SaveSimlockDataIntoCtx(AT_SimlockWriteExPara *simlockWriteExPara, VOS_UINT8 indexNum,
                                     VOS_UINT8 netWorkFlg);
VOS_UINT32 AT_SetSimlockDataWriteExPara(AT_SimlockWriteExPara *simlockWriteExPara, VOS_UINT8 indexNum,
                                        AT_SimlockTypeUint8 netWorkFlg);
VOS_UINT32 AT_ProcSimlockWriteExData(VOS_UINT8 *simLockData, VOS_UINT16 paraLen);

VOS_UINT32 AT_ProcPhonePhynumCmdCtx(AT_PhonePhynumPara *phonePhynumSetPara);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
