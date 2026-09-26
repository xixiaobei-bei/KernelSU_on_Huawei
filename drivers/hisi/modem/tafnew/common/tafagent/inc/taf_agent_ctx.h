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

#ifndef _TAFAGENT_CTX_H_
#define _TAFAGENT_CTX_H_

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define TAFAGENT_APS_MAX_MSG_LEN (1024)

#define TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(locked) (g_tafAgentCtx.acpuCnfSemLockedFlg = (locked))
#define TAF_AGENT_GET_ACPU_CNF_SEM_LOCK_FLG() (g_tafAgentCtx.acpuCnfSemLockedFlg)

#define TAF_AGENT_DBG_SAVE_MUTEX_SEM_ID(sem_id) (g_tafAgentStats.mutexSemId = (sem_id))
#define TAF_AGENT_DBG_SAVE_BINARY_SEM_ID(sem_id) (g_tafAgentStats.binarySemId = (sem_id))
#define TAF_AGENT_DBG_CREATE_MUTEX_SEM_FAIL_NUM(n) (g_tafAgentStats.createMutexSemFailNum += (n))
#define TAF_AGENT_DBG_CREATE_BINARY_SEM_FAIL_NUM(n) (g_tafAgentStats.createBinarySemFailNum += (n))
#define TAF_AGENT_DBG_LOCK_MUTEX_SEM_FAIL_NUM(n) (g_tafAgentStats.lockMutexSemFailNum += (n))
#define TAF_AGENT_DBG_LOCK_BINARY_SEM_FAIL_NUM(n) (g_tafAgentStats.lockBinarySemFailNum += (n))
#define TAF_AGENT_DBG_SAVE_LAST_MUTEX_SEM_ERR_RSLT(rslt) (g_tafAgentStats.lastMutexSemErrRslt = (rslt))
#define TAF_AGENT_DBG_SAVE_LAST_BIN_SEM_ERR_RSLT(rslt) (g_tafAgentStats.lastBinarySemErrRslt = (rslt))
#define TAF_AGENT_DBG_SYNC_MSG_IS_NULL_NUM(n) (g_tafAgentStats.syncMsgIsNullNum += (n))
#define TAF_AGENT_DBG_SYNC_MSG_NOT_MATCH_NUM(n) (g_tafAgentStats.syncMsgNotMatchNum += (n))

#define NV_READ_UNABLE 9
#define NV_WRITE_UNABLE (10)


typedef struct {
    /* 互斥信号量，用来同步PS域同步API的互斥操作,初始值为非锁状态(0xFFFFFFFF) */
    VOS_SEM tafAcpuSyncSem;

    /* 二进制信号量，用来等待返回结果， */
    VOS_SEM tafAcpuCnfSem;

    /* 初始化标识, VOS_TRUE: 成功; VOS_FALSE: 失败 */
    VOS_UINT32 initFlg;

    /* 二进制信号量状态 */
    VOS_UINT32 acpuCnfSemLockedFlg;

    /* 保存PS域同步API回复消息的指针 */
    VOS_UINT8 *tafAcpuCnfMsg;

    VOS_UINT8 msg[TAFAGENT_APS_MAX_MSG_LEN];
} TAFAGENT_CTX;


typedef struct {
    VOS_SEM    mutexSemId;             /* 互斥信号量ID */
    VOS_SEM    binarySemId;            /* 二进制信号量ID */
    VOS_UINT32 createMutexSemFailNum;  /* 创建互斥信号量失败次数 */
    VOS_UINT32 createBinarySemFailNum; /* 创建二进制信号量失败次数 */
    VOS_UINT32 lockMutexSemFailNum;    /* 锁互斥信号量失败次数 */
    VOS_UINT32 lockBinarySemFailNum;   /* 锁二进制信号量失败次数 */
    VOS_UINT32 lastMutexSemErrRslt;    /* 最后一次锁互斥信号量失败结果 */
    VOS_UINT32 lastBinarySemErrRslt;   /* 最后一次锁二进制信号量失败结果 */
    VOS_UINT32 syncMsgIsNullNum;       /* 同步消息为空次数 */
    VOS_UINT32 syncMsgNotMatchNum;     /* 同步消息不匹配次数 */

} TAF_AGENT_StatsInfo;

extern TAF_AGENT_StatsInfo g_tafAgentStats;

extern TAFAGENT_CTX g_tafAgentCtx;

extern VOS_UINT32 Taf_Agent_InitCtx(VOS_VOID);

extern VOS_SEM TAF_AGENT_GetTafAcpuSyncSem(VOS_VOID);

extern VOS_SEM TAF_AGENT_GetTafAcpuCnfSem(VOS_VOID);

extern VOS_UINT8* TAF_AGENT_GetTafAcpuCnfMsg(VOS_VOID);

extern VOS_VOID TAF_AGENT_SetTafAcpuCnfMsg(VOS_UINT8 *msg);

extern VOS_VOID TAF_AGENT_ClearMsg(VOS_VOID);

extern VOS_UINT8* TAF_AGENT_SaveMsg(VOS_UINT8 *msg, VOS_UINT32 len);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
