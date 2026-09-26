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
#ifndef __ATSIMPAMRSLTPROC_H__
#define __ATSIMPAMRSLTPROC_H__

#include "at_ctx.h"
#include "AtParse.h"
#include "si_app_pih.h"
#include "si_app_pb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

extern TAF_VOID   At_PbMsgProc(struct MsgCB *msg);
extern TAF_VOID   At_TAFPbMsgProc(TAF_UINT8 *data, TAF_UINT16 len);
VOS_UINT32 AT_RcvDrvAgentCpnnQryRsp(struct MsgCB *msg);
VOS_UINT32 AT_RcvDrvAgentCpnnTestRsp(struct MsgCB *msg);
VOS_UINT32 AT_RcvMmaSpnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 At_ProcPihHvsstSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_HvsstQueryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
#if (FEATURE_IMS == FEATURE_ON)
extern VOS_UINT32 AT_UiccAccessFileCnf(TAF_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
#endif

extern VOS_UINT32 At_ProcPihCardTypeQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihCardTypeExQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 AT_RcvMmaPsEflociInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaPsEflociInfoSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaEflociInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaEflociInfoSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 At_ProcPihCchoSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihCchpSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 At_ProcPihPrivateCchoSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
VOS_UINT32 At_ProcPihPrivateCchpSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihCchcSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihBwtSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihCardHistoryAtrQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihCrsmSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihSessionQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihIsdbAccessCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern VOS_UINT32 At_ProcPihCrlaSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length);
extern TAF_UINT16 At_PrintPrivateCglaResult(TAF_UINT8 indexNum, SI_PIH_EventInfo *event);
VOS_UINT32 AT_RcvDrvAgentSimlockWriteExSetCnf(struct MsgCB *msg);
VOS_UINT32 At_DialNumByIndexFromPb(VOS_UINT8 indexNum, SI_PB_EventInfo *event);
TAF_VOID At_PbUnicode80FormatPrint(TAF_UINT8 *src, TAF_UINT16 srcLen, TAF_UINT8 *dst, VOS_UINT32 decMaxBufLen, TAF_UINT16 *dstLen);
TAF_VOID At_PbUnicode81FormatPrint(TAF_UINT8 *puc81Code, VOS_UINT32 srcCodeLen,
                                   TAF_UINT8 *dst, VOS_UINT32 decBufLen, TAF_UINT16 *dstLen);
TAF_VOID At_PbUnicode82FormatPrint(TAF_UINT8 *puc82Code, VOS_UINT32 srcCodeLen,
                                   TAF_UINT8 *dst, VOS_UINT32 decBufLen, TAF_UINT16 *dstLen);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
