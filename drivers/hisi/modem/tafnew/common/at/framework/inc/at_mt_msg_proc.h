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
#ifndef _AT_MT_MSG_PROC_H_
#define _AT_MT_MSG_PROC_H_

#include "at_mt_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
/* 填写消息头 */
#if (VOS_OS_VER == VOS_WIN32)
#define AT_CFG_MT_MSG_HDR(msg, ulRecvPid, usSndMsgId) do { \
    ((AT_MT_MsgHeader *)(msg))->ulSenderCpuId   = VOS_LOCAL_CPUID; \
    ((AT_MT_MsgHeader *)(msg))->ulSenderPid     = WUEPS_PID_AT;    \
    ((AT_MT_MsgHeader *)(msg))->ulReceiverCpuId = VOS_LOCAL_CPUID; \
    ((AT_MT_MsgHeader *)(msg))->ulReceiverPid   = (ulRecvPid);     \
    ((AT_MT_MsgHeader *)(msg))->usMsgId         = (usSndMsgId);    \
} while (0)

#else
#define AT_CFG_MT_MSG_HDR(pstMsg, ulRecvPid, usSndMsgId) do { \
    VOS_SET_SENDER_ID(pstMsg, WUEPS_PID_AT);              \
    VOS_SET_RECEIVER_ID(pstMsg, (ulRecvPid));             \
    ((AT_MT_MsgHeader *)(pstMsg))->usMsgId = (usSndMsgId); \
} while (0)
#endif
/* 获取消息内容开始地址 */
#define AT_MT_GET_MSG_ENTITY(msg) ((VOS_VOID *)&(((AT_MT_MsgHeader *)(msg))->usMsgId))

/* 获取消息长度 */
#define AT_MT_GET_MSG_LENGTH(msg) VOS_GET_MSG_LEN(msg)

/* 封装消息初始化消息内容接口 */
VOS_VOID AT_MT_ClearMsgEntity(AT_MT_MsgHeader *msg);
#define AT_MT_CLR_MSG_ENTITY(msg) AT_MT_ClearMsgEntity((AT_MT_MsgHeader *)msg)
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_VOID At_WTxCltIndProc(WPHY_AT_TxCltInd *msg);
VOS_VOID At_RfRssiIndProc(HPA_AT_RfRxRssiInd *msg);

#else
VOS_VOID   AT_ProcCbtMsg(struct MsgCB *msg);
VOS_VOID   AT_ProcUeCbtMsg(struct MsgCB *msg);
VOS_VOID   AT_ProcDspIdleMsg(struct MsgCB *msg);
VOS_VOID   AT_ProcBbicMsg(struct MsgCB *msg);

#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
