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

#ifndef _ATCDMASETCMDPROC_H_
#define _ATCDMASETCMDPROC_H_

#include "vos.h"
#include "taf_type_def.h"
#include "at_mta_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

VOS_UINT32 AT_Hex2Ascii_Revers(VOS_UINT8 aucHex[], VOS_UINT32 length, VOS_UINT8 aucAscii[]);
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_CheckCFreqLockEnablePara(VOS_VOID);
extern VOS_UINT32 AT_SetCBurstDTMFPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetCfshPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_CheckCfshNumber(VOS_UINT8 *atPara, VOS_UINT16 len);
extern VOS_UINT32 AT_SetCContinuousDTMFPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_CheckCContDtmfKeyPara(VOS_VOID);
VOS_UINT32 AT_SetCSidEnable(VOS_UINT8 indexNum);
VOS_UINT32 AT_ConvertCSidListMcc(VOS_UINT8 *para, VOS_UINT16 len, VOS_UINT32 *mcc);

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_Set1xChanPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCSidList(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCFreqLockPara(VOS_UINT8 indexNum);

#endif

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
VOS_UINT32 AT_SetEncryptCall(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEncryptCallRemoteCtrl(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEncryptCallCap(VOS_UINT8 indexNum);
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_UINT32 AT_SetEncryptCallKmc(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetEccTestMode(VOS_UINT8 indexNum);
#endif
#endif

VOS_UINT32 AT_SetClocinfoEnable(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetPrivacyModePreferred(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCtOosCount(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCtRoamInfo(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetNoCardMode(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCdmaCsqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetHdrCsqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCclprPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCcmgdPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCdmaMemStatusPara(VOS_UINT8 indexNum);
extern TAF_UINT32 At_SetCCimiPara(TAF_UINT8 indexNum);
VOS_UINT32 AT_SetCrmPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCtaPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCdmaDormantTimer(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCdmaAttDiversitySwitch(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetMeidPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_QryMeidPara(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetEvdoSysEvent(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetDoSigMask(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCdmaCapResume(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCustomDial(VOS_UINT8 indexNum);
VOS_UINT32 At_SetCdmaModemSwitch(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetClDbDomainStatusPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SetCdmaModemCapPara(PLATAFORM_RatCapability *sourceModemPlatform,
                                  PLATAFORM_RatCapability *destinationModemPlatform, ModemIdUint16 sourceModemId,
                                  ModemIdUint16 destinationModemId, VOS_UINT8 indexNum);
VOS_UINT32 AT_SwitchCdmaMode(VOS_UINT8 indexNum, VOS_UINT32 modemId, PLATAFORM_RatCapability *modem0Platform,
                             PLATAFORM_RatCapability *modem1Platform, PLATAFORM_RatCapability *modem2Platform);

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
extern VOS_UINT32 At_SetAgpsDataCallStatus(VOS_UINT8 indexNum);

extern VOS_UINT32 At_SetAgpsUpBindStatus(VOS_UINT8 indexNum);
extern VOS_UINT32 At_SetCgpsControlStart(VOS_UINT8 indexNum);
extern VOS_UINT32 AT_SetCagpsCfgPosMode(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsStart(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsStop(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsCfgMpcAddr(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsCfgPdeAddr(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsQryRefloc(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsQryTime(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsPrmInfo(VOS_UINT8 indexNum);

extern VOS_UINT32 AT_SetCagpsReplyNiReq(VOS_UINT8 indexNum);

extern VOS_UINT32 At_SetCagpsPosInfo(VOS_UINT8 indexNum);
extern VOS_UINT32 At_SetCgpsControlStop(VOS_UINT8 indexNum);
extern VOS_UINT32 At_SetAgpsForwardData(VOS_UINT8 indexNum);
#endif


#endif
VOS_VOID AT_ReadPlatFormPrint(ModemIdUint16 modemId, PLATAFORM_RatCapability *platRat);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
