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

#ifndef _ATCDMARSLTPROC_H_
#define _ATCDMARSLTPROC_H_

#include "vos.h"
#include "v_msg.h"
#include "si_app_pih.h"
#include "taf_api.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
typedef struct {
    AT_XPDS_MsgTypeUint32 msgType;
    VOS_UINT32            reserved;
    VOS_CHAR             *pcATCmd;
} AT_CagpsCmdNameTlb;


typedef struct {
    AT_XPDS_MsgTypeUint32 msgType;
    AT_CmdCurOptUint32    cmdOpt;
} AT_CagpsCmdOptTlb;
#endif

extern VOS_UINT32 At_ProcPihCcimiQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);

VOS_UINT32 AT_RcvTafPsEvtSet1xDormTimerCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtSetPktCdataInactivityTimeLenCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);
VOS_UINT32 AT_RcvTafPsEvtGetPktCdataInactivityTimeLenCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);


#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmSndFlashRslt(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmSndBurstDTMFCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaQryCurrSidNidCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCLocInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_UINT32 AT_RcvMma1xChanQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaHdrCsqSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_UINT32 AT_RcvMmaHdrCsqQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaRatCombinedModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCtRoamInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmPrivacyModeSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaPrlIdQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmPrivacyModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEncryptCallCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmEccCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmRemoteCtrlAnsCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmEccCapSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmSetEccTestModeCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmQryEccTestModeCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmQryEccRandomCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmQryEccKmcCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmSetEccKmcCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
#endif

VOS_UINT32 AT_RcvMmaCVerQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaStateQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCHverQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaNoCardModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvCdmaModemSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
AT_CmdCurOptUint32 AT_SearchCagpsATCmdOpt(AT_XPDS_MsgTypeUint32 msgType);
extern VOS_UINT32 AT_RcvXpdsCagpsRlstCnf(struct MsgCB *msg);
extern VOS_UINT32 AT_RcvXpdsCagpsCnf(struct MsgCB *msg);
#endif

VOS_UINT32 AT_RcvMmaCdmaCsqSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCdmaCsqQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCFreqLockSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCFreqLockQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaMeidSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaMeidQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
