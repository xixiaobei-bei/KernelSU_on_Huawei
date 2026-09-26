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

#ifndef __RNIC_MSGPROC_H__
#define __RNIC_MSGPROC_H__

#include "vos.h"
#include "ps_common_def.h"
#include "NetMgrCtrlInterface.h"
#include "rnic_ctx.h"
#include "rnic_iface_cfg.h"
#include "rnic_intra_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* 封装OSA申请消息接口 */
#define RNIC_ALLOC_MSG_WITH_HDR(msg_len) PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_RNIC, (msg_len))

/* 封装OSA消息头 */
#if (VOS_OS_VER == VOS_WIN32)
#define RNIC_CFG_MSG_HDR(msg, rcv_pid, msg_id)               \
    ((MSG_Header *)(msg))->ulSenderCpuId   = VOS_LOCAL_CPUID; \
    ((MSG_Header *)(msg))->ulSenderPid     = ACPU_PID_RNIC;   \
    ((MSG_Header *)(msg))->ulReceiverCpuId = VOS_LOCAL_CPUID; \
    ((MSG_Header *)(msg))->ulReceiverPid   = (rcv_pid);       \
    ((MSG_Header *)(msg))->msgName         = (msg_id)
#else
#define RNIC_CFG_MSG_HDR(msg, rcv_pid, msg_id) \
    VOS_SET_SENDER_ID(msg, ACPU_PID_RNIC);     \
    VOS_SET_RECEIVER_ID(msg, (rcv_pid));       \
    ((MSG_Header *)(msg))->msgName = (msg_id)
#endif
#define RNIC_CFG_INTRA_MSG_HDR(msg, msg_id) RNIC_CFG_MSG_HDR(msg, ACPU_PID_RNIC, msg_id)

#define RNIC_CFG_AT_MSG_HDR(msg, msg_id) RNIC_CFG_MSG_HDR(msg, WUEPS_PID_AT, msg_id)

#define RNIC_CFG_CDS_MSG_HDR(msg, msg_id) RNIC_CFG_MSG_HDR(msg, UEPS_PID_CDS, msg_id)

/* 获取OSA消息内容 */
#define RNIC_GET_MSG_ENTITY(msg) ((VOS_VOID *)&(((MSG_Header *)(msg))->msgName))

/* 获取OSA消息长度 */
#define RNIC_GET_MSG_LENGTH(msg) (VOS_GET_MSG_LEN(msg))

/* 消息处理处理函数指针 */
typedef VOS_UINT32 (*RNIC_PROC_MSG_FUNC)(MsgBlock *pstMsg);


typedef struct {
    VOS_MSG_HEADER                  /* 消息头 */ /* _H2ASN_Skip */
    RNIC_IntraMsgIdUint32 msgId;    /* 消息类型 */ /* _H2ASN_Skip */
    RNIC_DialModePara     dialInfo; /* 按需拨号配置内容 */
} RNIC_NotifyMsg;


typedef struct {
    VOS_MSG_HEADER               /* 消息头 */ /* _H2ASN_Skip */
    RNIC_IntraMsgIdUint32 msgId; /* 消息ID */ /* _H2ASN_Skip */
} RNIC_CommMsg;


typedef RNIC_CommMsg RNIC_DEV_READY_STRU;


typedef struct {
    VOS_UINT32         senderPid;
    VOS_UINT32         msgId;
    RNIC_PROC_MSG_FUNC rnicProcMsgFunc;
} RNIC_PROC_Msg;


typedef RNIC_CommMsg RNIC_CCPU_RESET_IND_STRU;

VOS_UINT32 RNIC_RcvTiDsflowStatsExpired(MsgBlock *msg);
VOS_UINT32 RNIC_RcvTiDemandDialDisconnectExpired(MsgBlock *msg);

VOS_UINT32 RNIC_RcvDsmPdnInfoCfgInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvDsmPdnInfoRelInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvDsmNapiCfgInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvDsmNapiLbCfgInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvDsmRhcCfgInd(MsgBlock *msg);
VOS_UINT32 RNIC_SendMsg(VOS_VOID *msg);

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
VOS_UINT32 RNIC_RcvImsaPdnActInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvImsaPdnDeactInd(MsgBlock *msg);

VOS_UINT32 RNIC_RcvCdsImsDataInd(MsgBlock *msg);
VOS_UINT32 RNIC_ProcImsaPdnActInd_Wifi(IMSA_RNIC_PdnInfoConfig *pdnInfo);
VOS_UINT32 RNIC_ProcImsaPdnActIndCellular(IMSA_RNIC_PdnInfoConfig *pdnInfo);
VOS_UINT32 RNIC_ProcImsaPdnDeactInd_Wifi(ModemIdUint16 modemId, IMSA_RNIC_PdnEmcIndUint8 emcInd);
VOS_UINT32 RNIC_ProcImsaPdnDeactIndCellular(ModemIdUint16 modemId, IMSA_RNIC_PdnEmcIndUint8 emcInd);

VOS_UINT32 RNIC_RcvImsaReservedPortsCfgInd(MsgBlock *msg);

VOS_UINT32 RNIC_RcvImsaSocketExceptionInd(MsgBlock *msg);

VOS_UINT32 RNIC_RcvImsaSipPortRangeInd(MsgBlock *msg);

VOS_VOID RNIC_FillNetManagerMsgReservedPortCfgInfo(NM_RESERVED_PORTS_CONFIG_STRU *destReservedProtInfo,
                                                   IMSA_RNIC_ImsPortInfo         *srcReservedProtInfo);
VOS_VOID RNIC_FillNetManagerMsgPdnCfgInfo(NM_PDN_INFO_CONFIG_STRU *destPdnInfo, IMSA_RNIC_PdnInfoConfig *srcPdnInfo);
VOS_VOID RNIC_FillNetManagerMsgSipPortRangeInfo(NM_SIP_PORTS_RANGE_STRU   *destSipProtRange,
                                                IMSA_RNIC_SipPortRangeInd *srcSipProtRange);
VOS_VOID RNIC_SndNetManagerPdpActInd(MsgBlock *msg);
VOS_VOID RNIC_SndNetManagerPdpDeactInd(MsgBlock *msg);
VOS_VOID RNIC_SndNetManagerPdpModifyInd(MsgBlock *msg);
VOS_VOID RNIC_SndNetManagerModemResetInd(VOS_VOID);
VOS_VOID RNIC_SndNetManagerReservedPortCfgInd(MsgBlock *msg);
VOS_VOID RNIC_SndNetManagerSocketExceptInd(MsgBlock *msg);
VOS_VOID RNIC_SndNetManagerSipPortRangeInd(MsgBlock *msg);
#endif

VOS_UINT32 RNIC_RcvAtUsbTetherInfoInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvAtRmnetCfgReq(MsgBlock *msg);
VOS_UINT32 RNIC_RcvAtDialModeReq(MsgBlock *msg);
VOS_UINT32 RNIC_RcvCcpuResetStartInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvCcpuResetEndInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvNetdevReadyInd(MsgBlock *msg);
VOS_UINT32 RNIC_RcvImsaPdnModifyInd(MsgBlock *msg);
VOS_UINT32 RNIC_ProcImsData(MsgBlock *msg);
VOS_UINT32 RNIC_SendDialInfoMsg(RNIC_IntraMsgIdUint32 msgId);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of rnic_msg_proc.h */
