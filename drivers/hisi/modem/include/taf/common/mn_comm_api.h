/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __MN_COMM_API_H__
#define __MN_COMM_API_H__

#include "vos.h"
#include "taf_type_def.h"

#include "at_mn_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAF_GET_HOST_UINT32(data) \
    (TAF_GetHostUint32(data, sizeof(VOS_UINT32)))

#define TAF_GET_HOST_UINT16(data) \
    (TAF_GetHostUint16(data, sizeof(VOS_UINT16)))

#define TAF_PUT_HOST_UINT32(data, value) \
    (TAF_PutHostUint32(data, sizeof(VOS_UINT32), value))

#define TAF_PUT_HOST_UINT16(data, value) \
    (TAF_PutHostUint16(data, sizeof(VOS_UINT16), value))

typedef struct {
    MN_CLIENT_ID_T    clientId;
    MN_OPERATION_ID_T opId;
    VOS_UINT8         reserve;
} MN_CLIENT_OperationId;

/* 获取APP REQ消息头长度 */
VOS_VOID MN_GetAppReqMsgLen(VOS_UINT32 paraLen, VOS_UINT32 *msgLen);

/* 填充APP REQ消息头 */
VOS_VOID MN_FillAppReqMsgHeader(MN_APP_ReqMsg *msg, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, VOS_UINT16 msgType,
                                VOS_UINT32 receiverPid);

/* 填充APP REQ消息参数部分 */
VOS_VOID MN_FillAppReqMsgPara(VOS_VOID *sndMsgPara, const VOS_VOID *para, VOS_UINT32 len);

/* 填充APP REQ消息并发送 */
VOS_UINT32 MN_FillAndSndAppReqMsg(const MN_CLIENT_OperationId *clientOperationId, VOS_UINT16 msgType,
                                  const VOS_VOID *para, VOS_UINT32 paraLen, VOS_UINT32 receiverPid);

VOS_UINT32 TAF_GetHostUint32(const VOS_VOID *data, VOS_UINT32 length);

VOS_UINT16 TAF_GetHostUint16(const VOS_VOID *data, VOS_UINT32 length);

VOS_VOID TAF_PutHostUint32(VOS_VOID *data, VOS_UINT32 length, VOS_UINT32 value);

VOS_VOID TAF_PutHostUint16(VOS_VOID *data, VOS_UINT32 length, VOS_UINT16 value);

VOS_VOID TAF_CfgMsgHdr(MsgBlock *msg, VOS_UINT32 sndPid, VOS_UINT32 receiverPid, VOS_UINT32 length);

VOS_VOID* TAF_AllocPrivacyMsg(VOS_UINT32 pid, const MsgBlock *msg, VOS_UINT32 *length);

VOS_UINT32 TAF_TraceAndSendMsg(VOS_UINT32 sndPid, VOS_VOID *msg);
VOS_VOID *TAF_AllocMsgWithHeaderLen(VOS_UINT32 sndPid, VOS_UINT32 ulMsgLen);
VOS_VOID *TAF_AllocMsgWithoutHeaderLen(VOS_UINT32 sndPid, VOS_UINT32 ulMsgLen);

VOS_UINT32 TAF_GetDestPid(MN_CLIENT_ID_T clientId, VOS_UINT32 pid);
VOS_UINT32 TAF_GetSendPid(MN_CLIENT_ID_T clientId, VOS_UINT32 pid);
#ifdef MODEM_FUSION_VERSION
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
VOS_VOID TAF_AcoreTraceMsg(VOS_UINT32 sndPid, VOS_VOID *msg);
#endif
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
VOS_VOID TAF_CcoreTraceMsg(VOS_UINT32 sndPid, VOS_VOID *msg);
#endif
#endif

extern VOS_UINT32 TAF_ChkPsPidValidity(VOS_UINT32 pid);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mn_comm_api.h */
