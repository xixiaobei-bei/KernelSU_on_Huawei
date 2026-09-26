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

#ifndef _ATCDMAQRYCMDPROC_H_
#define _ATCDMAQRYCMDPROC_H_

#include "vos.h"
#include "taf_type_def.h"
#include "taf_ccm_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_QryCFreqLockInfo(VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_QryCtaPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCasState(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryHighVer(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryHdrCsqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCurrSidNid(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryProRevInUse(VOS_UINT8 indexNum);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_TestCclprPara(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestCContinuousDTMFPara(VOS_UINT8 indexNum);
extern VOS_UINT32 At_TestCBurstDTMFPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCLocInfo(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryPrivacyModePreferred(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCdmaCsqPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCtRoamInfo(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryPRLID(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryNoCardMode(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryRatCombinedMode(VOS_UINT8 indexNum);
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_Qry1xChanPara(VOS_UINT8 indexNum);
#endif

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
VOS_UINT32 AT_QryEncryptCallCap(VOS_UINT8 indexNum);
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_UINT32 AT_QryEncryptCallRandom(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEncryptCallKmc(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryEccTestMode(VOS_UINT8 indexNum);
#endif
#endif

VOS_UINT32 AT_QryProGetEsn(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryProGetMeid(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryCdmaDormTimerVal(VOS_UINT8 indexNum);
VOS_UINT32 At_QryFactoryCdmaCap(VOS_UINT8 indexNum);
VOS_UINT32 At_QryCdmaModemSwitch(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestClDbDomainStatusPara(VOS_UINT8 indexNum);

#if (MULTI_MODEM_NUMBER >= 2)
LOCAL VOS_UINT32 At_QryCdmaModemSwitchCdmaCapCheck(VOS_UINT32 modem0SupportCMode, VOS_UINT32 modem1SupportCMode,
#if (MULTI_MODEM_NUMBER == 3)
                                                   VOS_UINT32 ulModem2SupportCMode,
#endif
                                                   VOS_UINT32 *pulAllModemNotSupportCMode);
#endif



#endif


#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
