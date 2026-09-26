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

#include "rnic_timer_mgmt.h"
#include "rnic_ctx.h"
#include "v_timer.h"
#include "rnic_log.h"
#include "mdrv.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_RNIC_TIMERMGMT_C

VOS_VOID RNIC_InitAllTimers(VOS_VOID)
{
    RNIC_TIMER_Ctx *rnicTimerCtx;
    VOS_UINT32      i;

    rnicTimerCtx = RNIC_GET_RNIC_TIMER_ADR();

    for (i = 0; i < RNIC_MAX_TIMER_NUM; i++) {
        rnicTimerCtx[i].timer       = VOS_NULL_PTR;
        rnicTimerCtx[i].timerStatus = RNIC_TIMER_STATUS_STOP;
    }
}

VOS_VOID RNIC_StartTimer(RNIC_TIMER_IdUint16 timerId, VOS_UINT32 len)
{
    RNIC_TIMER_Ctx          *rnicTimerCtx = VOS_NULL_PTR;
    VOS_TimerPrecisionUint32 timerPrecision;
    VOS_UINT32               ret;

    rnicTimerCtx   = RNIC_GET_RNIC_TIMER_ADR();
    timerPrecision = VOS_TIMER_PRECISION_5;

    RNIC_NORMAL_LOG1(ACPU_PID_ADS_UL, "RNIC_StartTimer: enTimerId is ", timerId);

    /* 不在使用的定时器范围内 */
    if (timerId >= RNIC_MAX_TIMER_NUM) {
        RNIC_ERROR_LOG1(ACPU_PID_ADS_UL, "RNIC_StartTimer: invalid enTimerId ", timerId);
        return;
    }

    /* 如果缓存队列中该定时器已经启动则直接返回 */
    if (rnicTimerCtx[timerId].timerStatus == RNIC_TIMER_STATUS_RUNING) {
        RNIC_WARNING_LOG1(ACPU_PID_ADS_UL, "RNIC_StartTimer: The timer is running ", timerId);
        return;
    }

    /* 输入参数检查 */
    if (len == 0) {
        RNIC_ERROR_LOG(ACPU_PID_ADS_UL, "RNIC_StartTimer:ulLen is 0 ");
        return;
    }

    if (len >= VOS_TIMER_MAX_LENGTH) {
        len = VOS_TIMER_MAX_LENGTH - 1;
    }

    /* 如果是流量上报定时器，则挂在26M时钟上，以降低功耗 */
    if (timerId == TI_RNIC_DSFLOW_STATS) {
        timerPrecision = VOS_TIMER_NO_PRECISION;
    }

    /* VOS_StartRelTimer 启动定时器 */
    ret = VOS_StartRelTimer(&(rnicTimerCtx[timerId].timer), ACPU_PID_RNIC, len, timerId, 0, VOS_RELTIMER_NOLOOP,
                            timerPrecision);
    if (ret != VOS_OK) {
        RNIC_WARNING_LOG(ACPU_PID_RNIC, "NAS_MMC_StartTimer:NAS_StartRelTimer failed");
        return;
    }

    if (timerId == TI_RNIC_DEMAND_DIAL_DISCONNECT) {
        /* 按需拨号统计清零 */
        RNIC_SET_IFACE_PERIOD_SEND_PKT(RNIC_DEV_ID_RMNET0, 0);
    }

    rnicTimerCtx[timerId].timerStatus = RNIC_TIMER_STATUS_RUNING;

    /* 定时器状态勾包出来 */
}

VOS_VOID RNIC_StopTimer(RNIC_TIMER_IdUint16 timerId)
{
    RNIC_TIMER_Ctx *rnicTimerCtx;

    rnicTimerCtx = RNIC_GET_RNIC_TIMER_ADR();

    /* 不在使用的定时器范围内 */
    if (timerId >= RNIC_MAX_TIMER_NUM) {
        return;
    }

    /* 没有启动则不需要停止 */
    if (rnicTimerCtx[timerId].timerStatus != RNIC_TIMER_STATUS_RUNING) {
        return;
    }

    /* 停止VOS定时器: 当定时器的指针已经为空的时候, 说明其已经停止或者超时 */
    if (rnicTimerCtx[timerId].timer != VOS_NULL_PTR) {
        VOS_StopRelTimer(&(rnicTimerCtx[timerId].timer));
    }

    rnicTimerCtx[timerId].timer       = VOS_NULL_PTR;
    rnicTimerCtx[timerId].timerStatus = RNIC_TIMER_STATUS_STOP;
}

VOS_VOID RNIC_StopAllTimer(VOS_VOID)
{
    RNIC_TIMER_Ctx *rnicTimerCtx;
    VOS_UINT32      i;

    rnicTimerCtx = RNIC_GET_RNIC_TIMER_ADR();

    for (i = 0; i < RNIC_MAX_TIMER_NUM; i++) {
        if (rnicTimerCtx[i].timerStatus == RNIC_TIMER_STATUS_RUNING) {
            /* 停止VOS定时器 */
            VOS_StopRelTimer(&(rnicTimerCtx[i].timer));

            rnicTimerCtx[i].timer       = VOS_NULL_PTR;
            rnicTimerCtx[i].timerStatus = RNIC_TIMER_STATUS_STOP;
        }
    }
}

RNIC_TIMER_StatusUint8 RNIC_GetTimerStatus(RNIC_TIMER_IdUint16 timerId)
{
    RNIC_TIMER_Ctx *rnicTimerCtx;
    VOS_UINT32      leftLen;

    rnicTimerCtx = RNIC_GET_RNIC_TIMER_ADR();
    leftLen      = 0;

    if (timerId >= RNIC_MAX_TIMER_NUM) {
        return RNIC_TIMER_STATUS_STOP;
    }

    if (rnicTimerCtx[timerId].timerStatus == RNIC_TIMER_STATUS_RUNING) {
        if ((VOS_GetRelTmRemainTime(&(rnicTimerCtx[timerId].timer), &leftLen)) != VOS_OK) {
            RNIC_StopTimer(timerId);
            return RNIC_TIMER_STATUS_STOP;
        } else {
            return RNIC_TIMER_STATUS_RUNING;
        }
    }

    return RNIC_TIMER_STATUS_STOP;
}

