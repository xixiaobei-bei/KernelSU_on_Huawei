/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef _TAF_MSG_CHK_API_H_
#define _TAF_MSG_CHK_API_H_

#include "ps_type_def.h"
#include "taf_type_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAF_DEFAULT_CONTENT_LEN 4

typedef VOS_UINT32 (*TAF_ChkMsgLenFunc)(const MSG_Header *msgHeader);
typedef VOS_INT (*TAF_COMP_FUNC)(const VOS_VOID *, const VOS_VOID *);

typedef struct {
    VOS_UINT32        eventType;
    VOS_UINT32        fixdedPartLen;
    TAF_ChkMsgLenFunc chkFunc;
} AT_PbMsgLenInfo;

/*
 * 功能说明: 按消息名称获取待检查消息的长度并检查消息长度合法性
 * msgName 消息ID，注意: 需要唯一的发送接收PID上下文的消息名
 * fixdedPartLen 通过消息关联的结构可以直接计算出来消息长度的，直接标记消息有效长度
 * chkFunc 消息结构中存在长度变量指示字段的，需要定义函数计算有效长度
 */
typedef struct {
    VOS_UINT32        msgName;
    VOS_UINT32        fixdedPartLen;
    TAF_ChkMsgLenFunc chkFunc;
} TAF_ChkMsgLenNameMapTbl;

typedef struct {
    VOS_UINT32 senderPid;
    VOS_UINT32 receiverPid;
} TAF_PidMap;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgName;
    VOS_UINT16 reserve;
} TAF_MsgHeaderWithTwoByteMsgName;

typedef TAF_ChkMsgLenNameMapTbl* (*TAF_GetChkMsgLenTblFunc)(VOS_VOID);
typedef VOS_UINT32 (*TAF_GetChkMsgLenTblSizeFunc)(VOS_VOID);
typedef struct {
    VOS_UINT32                  sndPid;
    TAF_GetChkMsgLenTblFunc     getSndPidTblAddrFunc;        /* 动作表的基地址                      */
    TAF_GetChkMsgLenTblSizeFunc getSndPidTblAddrSize;        /* 动作表的大小字节数除以ACT结构的大小 */
} TAF_ChkMsgLenSndPidMapTbl;

typedef struct {
    VOS_UINT32                 receiverPid;
    VOS_UINT32                 tabSize;
    TAF_ChkMsgLenSndPidMapTbl *tabAddr;
} TAF_ChkMsgLenRcvPidMapTbl;

#define TAF_MSG_CHECK_TBL_ITEM(receivePid, actTbl)\
{\
    (receivePid), (TAF_GET_ARRAY_NUM(actTbl)), (actTbl)\
}

VOS_UINT32 TAF_ChkTimerMsgLen(const MsgBlock *msgHeader);
VOS_UINT32 TAF_ChkMsgLenWithExpectedLen(const MSG_Header *msgHeader, VOS_UINT32 expectedLen);
VOS_UINT32 TAF_ChkMsgLen(const MsgBlock *msg, const TAF_ChkMsgLenRcvPidMapTbl *rcvPidMapTbl, VOS_UINT32 tabSize);

VOS_VOID TAF_SortChkMsgLenTblByMsgName(TAF_ChkMsgLenNameMapTbl *tabAddr, VOS_UINT32 tabSize);

VOS_UINT32 TAF_RunChkMsgLenFunc(const MSG_Header *msgHeader, VOS_UINT32 structLen, TAF_ChkMsgLenFunc checkFun);

TAF_ChkMsgLenNameMapTbl* TAF_GetChkChrMsgLenTblAddr(VOS_VOID);
VOS_UINT32 TAF_GetChkChrMsgLenTblSize(VOS_VOID);
#if ((FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF) || (VOS_OSA_CPU == OSA_CPU_CCPU))
VOS_UINT32 TAF_ChkGunasLogPrivacyMsgLen(const MsgBlock *msg);
VOS_UINT32 TAF_ChkAtSndLogPrivacyMsgLen(const MsgBlock *msg);
VOS_UINT32 TAF_ChkTafSndLogPrivacyMsgLen(const MsgBlock *msg);
#endif
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of _TAF_MSG_CHK_API_H_ */

