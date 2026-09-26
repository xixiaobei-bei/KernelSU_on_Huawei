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
#ifndef __ATVOICEEVENTREPORT_H__
#define __ATVOICEEVENTREPORT_H__

#include "vos.h"
#include "app_vc_api.h"
#include "si_app_emat.h"
#include "taf_ps_api.h"
#include "si_app_stk.h"
#include "mn_call_api.h"
#include "AtParse.h"
#include "at_ctx.h"
#include "si_app_pb.h"

#include "at_cmd_proc.h"

#include "taf_ccm_api.h"
#include "taf_call_comm.h"
#include "at_set_para_cmd.h"
#include "taf_msg_chk_api.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_EVT_IS_VIDEO_CALL(enCallType)                                                   \
    (((enCallType) == MN_CALL_TYPE_VIDEO_TX) || ((enCallType) == MN_CALL_TYPE_VIDEO_RX) || \
     ((enCallType) == MN_CALL_TYPE_VIDEO))

#define AT_EVT_IS_PS_VIDEO_CALL(enCallType, enVoiceDomain)                               \
    ((((enCallType) == MN_CALL_TYPE_VIDEO) || ((enCallType) == MN_CALL_TYPE_VIDEO_TX) || \
      ((enCallType) == MN_CALL_TYPE_VIDEO_RX)) &&                                        \
     ((enVoiceDomain) == TAF_CALL_VOICE_DOMAIN_IMS))

#define AT_EVT_REL_IS_NEED_CLR_TIMER_STATUS_CMD(CmdCurrentOpt)                                            \
    (((CmdCurrentOpt) == AT_CMD_D_CS_VOICE_CALL_SET) || ((CmdCurrentOpt) == AT_CMD_D_CS_DATA_CALL_SET) || \
     ((CmdCurrentOpt) == AT_CMD_APDS_SET) || ((CmdCurrentOpt) == AT_CMD_CHLD_EX_SET) ||                   \
     ((CmdCurrentOpt) == AT_CMD_A_SET) || ((CmdCurrentOpt) == AT_CMD_END_SET))

enum AT_CS_CallState {
    AT_CS_CALL_STATE_ORIG = 0,  /* originate a MO Call */
    AT_CS_CALL_STATE_CALL_PROC, /* Call is Proceeding */
    AT_CS_CALL_STATE_ALERTING,  /* Alerting,MO Call */
    AT_CS_CALL_STATE_CONNECT,   /* Call Connect */
    AT_CS_CALL_STATE_RELEASED,  /* Call Released */
    AT_CS_CALL_STATE_INCOMMING, /* Incoming Call */
    AT_CS_CALL_STATE_WAITING,   /* Waiting Call */
    AT_CS_CALL_STATE_HOLD,      /* Hold Call */
    AT_CS_CALL_STATE_RETRIEVE,  /* Call Retrieved */

    AT_CS_CALL_STATE_BUTT
};
typedef VOS_UINT8 AT_CS_CallStateUint8;

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEconfNotifyInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvImsaMtStateInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvImsaRttEventInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvImsaRttErrorInd(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

#if (FEATURE_ECALL == FEATURE_ON)
VOS_VOID   AT_RcvTafEcallStatusErrorInd(VOS_VOID);
VOS_UINT32 At_ProcVcReportEcallStateEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo);
#endif

VOS_VOID AT_ReportCCallstateResult(ModemIdUint16 modemId, VOS_UINT8 callId, VOS_UINT8 *rptCfg,
                                   AT_CS_CallStateUint8 callState, TAF_CALL_VoiceDomainUint8 voiceDomain);
VOS_VOID At_ChangeEcallTypeToCallType(MN_CALL_TypeUint8 ecallType, MN_CALL_TypeUint8 *callType);
VOS_VOID AT_ReportCendResult(VOS_UINT8 indexNum, TAF_CCM_CallReleasedInd *callReleaseInd);
PS_BOOL_ENUM_UINT8 At_CheckReportCendCallType(MN_CALL_TypeUint8 callType);
PS_BOOL_ENUM_UINT8 At_CheckReportOrigCallType(MN_CALL_TypeUint8 callType);

PS_BOOL_ENUM_UINT8 At_CheckReportConfCallType(MN_CALL_TypeUint8 callType);

PS_BOOL_ENUM_UINT8 At_CheckUartRingTeCallType(MN_CALL_TypeUint8 callType);
VOS_UINT32 AT_RcvTafCcmCallConnectInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 At_RcvTafCcmCallIncomingInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallReleaseInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallProcInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallAlertingInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallRetrieveInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallHoldInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmChannelInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallOrigInd(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafCcmCallAllReleasedInd(VOS_VOID *msg, VOS_UINT8 indexNum);
TAF_CALL_ChannelTypeUint8 AT_ConvertCodecTypeToChannelType(VOS_UINT8              isLocalAlertingFlag,
                                                           MN_CALL_CodecTypeUint8 codecType);
VOS_UINT32 AT_RcvMmaEmcCallBackNtf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID At_RcvVcMsgDtmfDecoderIndProc(MN_AT_IndEvt *data);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
