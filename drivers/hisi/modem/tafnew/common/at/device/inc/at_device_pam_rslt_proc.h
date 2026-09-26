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
#ifndef _AT_DEVICE_PAM_RSLT_PROC_H_
#define _AT_DEVICE_PAM_RSLT_PROC_H_

#include "vos.h"
#include "si_app_emat.h"
#include "taf_msg_chk_api.h"
#include "si_app_pih.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

typedef VOS_UINT32 (*AT_EMAT_RSP_PROC_FUNC)(TAF_UINT8 ucIndex, SI_EMAT_EventInfo *pstEvent, VOS_UINT16 *pusLength);

typedef struct {
    SI_EMAT_EventUint32   eventType;
    VOS_UINT32            fixdedPartLen; /* 通过消息关联的结构可以直接计算出来消息长度的，直接标记消息有效长度 */
    TAF_ChkMsgLenFunc     chkFunc;       /* 消息结构中存在长度变量指示字段的，需要定义函数计算有效长度 */
    AT_EMAT_RSP_PROC_FUNC atEMATRspProcFunc;  //lint !e958
} AT_EmatRspProcFunc;

VOS_UINT32 AT_RcvDrvAgentIdentifyStartSetCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvDrvAgentHukSetCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvDrvAgentSimlockDataReadExReadCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvDrvAgentPhoneSimlockInfoQryCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvDrvAgentSimlockDataReadQryCnf(struct MsgCB *msg);
VOS_UINT32 AT_RcvDrvAgentGetSimlockEncryptIdQryCnf(struct MsgCB *msg);
TAF_VOID   At_EMATIndProc(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event);
TAF_UINT32 At_EMATNotBroadIndProc(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event);
TAF_VOID   At_EMATRspProc(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event);

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)

#else
VOS_UINT32 At_RcvUeCbtRfIcIdExQryCnf(struct MsgCB *msg);
#endif
VOS_UINT32 At_PrintSetEsimSwitchInfo(TAF_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 At_PrintQryEsimSwitchInfo(TAF_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 At_PrintEsimCleanProfileInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 At_PrintEsimCheckProfileInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 At_PrintGetEsimEidInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 At_PrintGetEsimPKIDInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length);

#if ( defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON)) && (defined(MODEM_FUSION_VERSION))
VOS_UINT32 At_RcvFdcmCheckCnf(struct MsgCB *msg);
#endif
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
