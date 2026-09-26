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
#ifndef __ATCDMAEVENTREPORT_H__
#define __ATCDMAEVENTREPORT_H__

#include "vos.h"
#include "si_app_emat.h"
#include "taf_ps_api.h"
#include "si_app_stk.h"
#include "AtParse.h"
#include "at_ctx.h"

#include "at_cmd_proc.h"
#include "at_set_para_cmd.h"
#include "taf_msg_chk_api.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
/*
 * 结构说明: AT与XPDS消息与对应处理函数的结构
 */
/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    AT_XPDS_MsgTypeUint32 msgType;
    AT_MMA_MSG_PROC_FUNC  procMsgFunc;
} AT_PROC_MsgFromXpds;
/*lint +e958 +e959 ;cause:64bit*/
#endif

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON))

typedef struct {
    AT_EncryptVoiceErrorUint32      atEncErr;
    TAF_CALL_EncryptVoiceTypeUint32 tafEncErr;
} AT_EncryptVoiceErrCodeMap;
#endif


#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON))
#define AT_GET_ENC_VOICE_ERR_CODE_MAP_TBL_PTR() (g_atEncVoiceErrCodeMapTbl)
#define AT_GET_ENC_VOICE_ERR_CODE_MAP_TBL_SIZE() (sizeof(g_atEncVoiceErrCodeMapTbl) / sizeof(AT_EncryptVoiceErrCodeMap))
#endif


#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
extern VOS_UINT32 AT_RcvTafCcmCalledNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmCallingNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmDispInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmExtDispInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmConnNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmRedirNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmSignalInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmLineCtrlInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmCCWACInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmRcvContinuousDtmfInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvTafCcmRcvBurstDtmfInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvMtaEvdoRevARLinkInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvMtaEvdoSigExEventInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCSidInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCombinedModeSwitchInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaClocinfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCdmaCsqInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCLModInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaClDbdomainStatusReportInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvMmaCTimeInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaHdrCsqInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaRoamingModeSwitchInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaIccAppTypeSwitchInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCtRoamingInfoChgInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmPrivacyModeInd(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_VOID At_XsmsIndProc(VOS_UINT8 indexNum, TAF_XSMS_AppMsgTypeUint32 eventType, TAF_XSMS_AppAtEventInfo *event);
extern TAF_UINT32 At_ChgXsmsErrorCodeToAt(TAF_UINT32 xsmsError);
extern VOS_VOID At_XsmsCnfProc(VOS_UINT8 indexNum, TAF_XSMS_AppMsgTypeUint32 eventType, TAF_XSMS_AppAtEventInfo *event);
extern VOS_VOID AT_ProcXsmsMsg(struct MsgCB *msg);
VOS_UINT32 AT_RcvTafPsEvtCgmtuValueChgInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo);

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
AT_EncryptVoiceErrorUint32 AT_MapEncVoiceErr(TAF_CALL_EncryptVoiceStatusUint32 tafEncVoiceErr);
VOS_UINT32 AT_RcvTafCcmEncryptCallInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmEccRemoteCtrlInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEncryptedVoiceDataInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
#endif

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaCLImsCfgInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
extern VOS_UINT32 AT_RcvXpdsAgpsDataCallReq(struct MsgCB *msg);
extern VOS_UINT32 AT_RcvXpdsAgpsServerBindReq(struct MsgCB *msg);
extern VOS_UINT32 AT_RcvXpdsAgpsReverseDataInd(struct MsgCB *msg);
extern VOS_UINT32 AT_CagpsSndXpdsReq(VOS_UINT8 indexNum, AT_XPDS_MsgTypeUint32 msgType, VOS_UINT32 msgStructSize);
VOS_UINT32 AT_RcvXpdsEphInfoInd(struct MsgCB *msg);
VOS_UINT32 AT_RcvXpdsAlmInfoInd(struct MsgCB *msg);
extern VOS_VOID AT_ProcXpdsMsg(struct MsgCB *msg);
#endif

#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
