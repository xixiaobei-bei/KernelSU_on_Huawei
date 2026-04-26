/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef __ADS_PUBLIC_H__
#define __ADS_PUBLIC_H__

#include "vos.h"
#include "ps_iface_global_def.h"
#include "ads_iface_i.h"
#include "mn_comm_api.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#define ADS_BIT64_MASK(bit) ((VOS_UINT64)(1ULL << (bit)))
#define ADS_BIT64_SET(val, bit) ((val) | ADS_BIT64_MASK(bit))
#define ADS_BIT64_CLR(val, bit) ((val) & ~ADS_BIT64_MASK(bit))
#define ADS_BIT64_IS_SET(val, bit) ((val) & ADS_BIT64_MASK(bit))

#define ADS_BIT8_MASK(bit) ((VOS_UINT8)(1 << (bit)))
#define ADS_BIT8_SET(val, bit) ((val) | ADS_BIT8_MASK(bit))
#define ADS_BIT8_CLR(val, bit) ((val) & ~ADS_BIT8_MASK(bit))
#define ADS_BIT8_IS_SET(val, bit) ((val) & ADS_BIT8_MASK(bit))

/* 网卡接口操作 */
#define ADS_IFACE_OP_TYPE_DOWN (0x00)
#define ADS_IFACE_OP_TYPE_UP (0x01)

#define ADS_INVAILD_IFACE_ID (0xFF)
#define ADS_INVAILD_PDU_SESSION_ID (0xFF)

#if defined(MODEM_FUSION_VERSION)
#define ADS_BUILD_EX_PDU_SESSION(modemid, sessionid) (sessionid)
#else
#define ADS_BUILD_EX_PDU_SESSION(modemid, sessionid) ((VOS_UINT8)((((modemid) << 6) & 0xC0) | ((sessionid) & 0x3F)))
#endif

#define ADS_ALLOC_MSG_WITH_HDR(ulMsgLen) PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_ADS_UL, (ulMsgLen))

/* 封装OSA消息头 */
#if (VOS_OS_VER == VOS_WIN32)
#define ADS_CFG_MSG_HDR(pstMsg, ulRecvPid, ulMsgId)             \
    ((MSG_Header *)(pstMsg))->ulSenderCpuId   = VOS_LOCAL_CPUID; \
    ((MSG_Header *)(pstMsg))->ulSenderPid     = ACPU_PID_ADS_UL; \
    ((MSG_Header *)(pstMsg))->ulReceiverCpuId = VOS_LOCAL_CPUID; \
    ((MSG_Header *)(pstMsg))->ulReceiverPid   = (ulRecvPid);     \
    ((MSG_Header *)(pstMsg))->msgName         = (ulMsgId)
#else
#define ADS_CFG_MSG_HDR(pstMsg, ulRecvPid, ulMsgId) \
    VOS_SET_SENDER_ID(pstMsg, ACPU_PID_ADS_UL);     \
    VOS_SET_RECEIVER_ID(pstMsg, (ulRecvPid));       \
    ((MSG_Header *)(pstMsg))->msgName = (ulMsgId)
#endif
/* 封装OSA消息头(ADS内部消息) */
#define ADS_CFG_INTRA_MSG_HDR(msg, ulMsgId) ADS_CFG_MSG_HDR(msg, ACPU_PID_ADS_UL, ulMsgId)

/* 封装OSA消息头(CDS消息) */
#define ADS_CFG_CDS_MSG_HDR(msg, ulMsgId) ADS_CFG_MSG_HDR(msg, UEPS_PID_CDS, ulMsgId)

/* 封装OSA消息头(NDIS消息) */
#define ADS_CFG_NDIS_MSG_HDR(pstMsg, ulMsgId) ADS_CFG_MSG_HDR(pstMsg, PS_PID_APP_NDIS, ulMsgId)

/* 获取OSA消息内容 */
#define ADS_GET_MSG_ENTITY(msg) ((VOS_VOID *)&(((MSG_Header *)(msg))->msgName))

/* 获取OSA消息长度 */
#define ADS_GET_MSG_LENGTH(msg) (VOS_GET_MSG_LEN(msg))

VOS_UINT32 ADS_SendMsg(VOS_VOID *msg);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_PUBLIC_H__ */
