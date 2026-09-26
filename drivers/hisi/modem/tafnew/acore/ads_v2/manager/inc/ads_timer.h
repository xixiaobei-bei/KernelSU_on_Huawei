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

#ifndef __ADS_TIMER_H__
#define __ADS_TIMER_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

/* ADS中同时运行的定时器的最大数目,目前只有一个 */
#define ADS_MAX_TIMER_NUM (TI_ADS_TIMER_BUTT)

/* 定时器TI_ADS_DSFLOW_STATS的时长，1秒统计一次速率 */
#define TI_ADS_DSFLOW_STATS_LEN (1000)

/*
 * 枚举说明: ADS定时器的ID
 */
enum ADS_TIMER_Id {
    TI_ADS_TIMER_RESERVED = 0x00,
    TI_ADS_DSFLOW_STATS   = 0x01, /* 流量统计定时器 */

    TI_ADS_TIMER_BUTT
};
typedef VOS_UINT32 ADS_TIMER_IdUint32;

/*
 * 枚举说明: ADS_TIMER_OperationType
 * 枚举说明: ADS定时器操作的取值，用于SDT中显示
 */
enum ADS_TIMER_OperationType {
    ADS_TIMER_OPERATION_START, /* 启动定时器 */
    ADS_TIMER_OPERATION_STOP,  /* 停止定时器 */
    ADS_TIMER_OPERATION_TYPE_ENUM_BUTT
};
typedef VOS_UINT8 ADS_TIMER_OperationTypeUint8;

/*
 * 枚举说明: ADS_TIMER_StopCause
 * 枚举说明: ADS定时器停止的原因，用于SDT中显示
 */
enum ADS_TIMER_StopCause {
    ADS_TIMER_STOP_CAUSE_USER,    /* 用户主动停止的 */
    ADS_TIMER_STOP_CAUSE_TIMEOUT, /* 定时器超时显示停止的 */
    ADS_TIMER_STOP_CAUSE_ENUM_BUTT
};
typedef VOS_UINT8 ADS_TIMER_StopCauseUint8;

/*
 * 4 STRUCT&UNION定义
 */

/*
 * 结构说明: ADS定时器运行时的上下文
 */
typedef struct {
    HTIMER    timer;  /* 定时器的运行指针 */
    VOS_UINT8 rsv[8]; /* 保留 */
} ADS_TIMER_Ctx;

/*
 * 结构说明: ADS定时器信息结构，用于SDT中显示
 */
typedef struct {
    VOS_MSG_HEADER                               /* _H2ASN_Skip */
    ADS_TIMER_IdUint32           timerId;        /* _H2ASN_Skip */
    VOS_UINT32                   timerLen;       /* 定时器长度 */
    ADS_TIMER_OperationTypeUint8 timerAction;    /* 定时器操作类型 */
    ADS_TIMER_StopCauseUint8     timerStopCause; /* 定时器停止的原因 */
    VOS_UINT8                    reserved[2];
} ADS_TIMER_Info;

VOS_VOID ADS_StartTimer(ADS_TIMER_IdUint32 timerId, VOS_UINT32 len);

VOS_VOID ADS_StopTimer(ADS_TIMER_IdUint32 timerId, ADS_TIMER_StopCauseUint8 stopCause);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_TIMER_H__ */
