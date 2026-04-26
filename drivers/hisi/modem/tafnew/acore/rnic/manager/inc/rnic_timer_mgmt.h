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

#ifndef __RNIC_TIMERMGMT_H__
#define __RNIC_TIMERMGMT_H__

#include "vos.h"
#include "product_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* RNIC中同时运行的定时器的最大数目 */
#define RNIC_MAX_TIMER_NUM (TI_RNIC_TIMER_BUTT)

#define TI_RNIC_UNIT (1000) /* 1s 等于 1000ms */

#define TI_RNIC_DSFLOW_STATS_LEN (1000)            /* 1秒统计一次速率 */
#define TI_RNIC_DEMAND_DIAL_DISCONNECT_LEN (10000) /* 10秒拨号断开定时器 */
#define TI_RNIC_DEMAND_DIAL_PROTECT_LEN (1000)     /* 1秒拨号保护定时器 */


enum RNIC_TIMER_IdType {

    TI_RNIC_DEMAND_DIAL_DISCONNECT, /* 按需拨号断开定时器 */
    TI_RNIC_DEMAND_DIAL_PROTECT,    /* 按需拨号保护定时器 */
    TI_RNIC_DSFLOW_STATS,           /* 网卡的流量统计定时器 */

    TI_RNIC_TIMER_BUTT
};
typedef VOS_UINT16 RNIC_TIMER_IdUint16;


enum RNIC_TIMER_Status {
    RNIC_TIMER_STATUS_STOP,   /* 定时器停止状态 */
    RNIC_TIMER_STATUS_RUNING, /* 定时器运行状态 */
    RNIC_TIMER_STATUS_BUTT
};
typedef VOS_UINT8 RNIC_TIMER_StatusUint8;


enum RNIC_TIMER_OperationType {
    RNIC_TIMER_OPERATION_START, /* 启动定时器 */
    RNIC_TIMER_OPERATION_STOP,  /* 停止定时器 */
    RNIC_TIMER_OPERATION_TYPE_ENUM_BUTT
};
typedef VOS_UINT8 RNIC_TIMER_OperationTypeUint8;


typedef struct {
    HTIMER                 timer;       /* 定时器的运行指针 */
    RNIC_TIMER_StatusUint8 timerStatus; /* 定时器的运行状态,启动或停止 */
    VOS_UINT8              rsv[7];      /* 保留 */
} RNIC_TIMER_Ctx;


typedef struct {
    VOS_MSG_HEADER
    RNIC_TIMER_IdUint16           timerId;    /* 定时器ID */
    RNIC_TIMER_OperationTypeUint8 timeAction; /* 定时器操作类型 */
    VOS_UINT8                     rsv[1];     /* 保留 */
    VOS_UINT32                    timeLen;    /* 定时器长度 */
} RNIC_TIMER_Info;

VOS_VOID RNIC_InitAllTimers(VOS_VOID);

VOS_VOID RNIC_StartTimer(RNIC_TIMER_IdUint16 timerId, VOS_UINT32 len);

VOS_VOID RNIC_StopTimer(RNIC_TIMER_IdUint16 timerId);

VOS_VOID RNIC_StopAllTimer(VOS_VOID);

RNIC_TIMER_StatusUint8 RNIC_GetTimerStatus(RNIC_TIMER_IdUint16 timerId);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of rnic_timer_mgmt.h */
