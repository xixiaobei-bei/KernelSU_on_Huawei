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

#ifndef __ADS_CTX_H__
#define __ADS_CTX_H__

#include "vos.h"
#include "ps_iface_global_def.h"
#include "imm_interface.h"
#include "ads_timer.h"
#include "ads_public.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

/* ADS任务优先级 */
#define ADS_TASK_PRIORITY (82)
#define ADS_EVENT_BASE (0x00000000)
#define ADS_EVENT_SPECIAL_DATA_IND (ADS_EVENT_BASE | 0x0001)

#define ADS_RA_MAX_NUM 5
#define ADS_RA_PROTECT_DUR msecs_to_jiffies(100)

/*
 * 4 STRUCT&UNION定义
 */

/*
 * 结构说明: ADS每个网卡的上下文
 */
typedef struct {
    VOS_UINT8  ifaceId;
    VOS_UINT8  stats;
    VOS_UINT8  addrFamilyMask; /* 网卡激活类型 bit0:ipv4 1:ipv6 2:NON ip */
    VOS_UINT8  transMode;
    VOS_UINT16 modemId;
    VOS_UINT8  pduSessionTbl[ADS_ADDR_BUTT];
    VOS_UINT8  rsv[6];
} ADS_IFACE_Info;

/*
 * 结构说明: WAKELOCK配置
 */
typedef struct {
    VOS_UINT32 timeOut; /* msec */
} ADS_WakeLockCfg;

/*
 * 结构说明: RA控制相关信息
 */
typedef struct {
    VOS_ULONG  startTime; /* 保护期内首个RA报文的时间 */
    VOS_UINT64 raNum;     /* 保护期内收到的RA报文个数 */
} ADS_RaThrot_Info;

/*
 * 结构说明: ADS上下文
 */
typedef struct {
    ADS_IFACE_Info   ifaceInfo[PS_IFACE_ID_BUTT]; /* 每个网卡专有的上下文 */
    ADS_TIMER_Ctx    timerCtx[ADS_MAX_TIMER_NUM]; /* 定时器上下文 */
    IMM_ZcHeader     dataQue;                     /* 特殊包队列 */
    ADS_WakeLockCfg  wakeLockCfg;                 /* wakelock配置 */
    VOS_UINT64       ifaceStatsBitMask;           /* 网卡状态bit掩码 */
    VOS_SEM          resetSem;                    /* 二进制信号量，用于复位处理  */
    ADS_RaThrot_Info raThrot[PS_IFACE_ID_BUTT];   /* RA控制相关信息 */
} ADS_CONTEXT;

ADS_CONTEXT*    ADS_GetCtx(VOS_VOID);
ADS_IFACE_Info* ADS_GetIfaceInfo(VOS_UINT8 ifaceNum);

VOS_UINT32 ADS_GetTaskReadyFlag(VOS_VOID);
VOS_VOID   ADS_SetTaskReadyFlag(VOS_UINT32 taskReadyFlag);
VOS_UINT32 ADS_GetTaskId(VOS_VOID);
VOS_VOID   ADS_SetTaskId(VOS_UINT32 taskId);

IMM_ZcHeader* ADS_GetDataQue(VOS_VOID);
VOS_SEM       ADS_GetResetSem(VOS_VOID);
VOS_VOID      ADS_AllIfaceInfoInit(VOS_VOID);
VOS_VOID      ADS_IfaceInfoInit(VOS_UINT8 ifaceId);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_CTX_H__ */
