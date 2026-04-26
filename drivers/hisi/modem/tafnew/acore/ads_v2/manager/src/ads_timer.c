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

#include "ads_timer.h"
#include "ads_ctx.h"
#include "ads_log.h"
#include "mn_comm_api.h"
#include "msp_diag_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_ADS_TIMER_C

VOS_VOID ADS_MntnTraceTimerOperation(VOS_UINT32 pid, ADS_TIMER_IdUint32 timerId, VOS_UINT32 timerLen,
                                     ADS_TIMER_OperationTypeUint8 timerAction, ADS_TIMER_StopCauseUint8 stopCause)
{
    ADS_TIMER_Info msg = {0};

    VOS_SET_MSG_LEN(&msg, sizeof(ADS_TIMER_Info) - VOS_MSG_HEAD_LENGTH);
    ADS_CFG_MSG_HDR(&msg, VOS_PID_TIMER, timerId);

    msg.timerLen       = timerLen;
    msg.timerAction    = timerAction;
    msg.timerStopCause = stopCause;
    msg.reserved[0]    = 0;
    msg.reserved[1]    = 0;

    mdrv_diag_trace_report(&msg);
}

VOS_VOID ADS_StartTimer(ADS_TIMER_IdUint32 timerId, VOS_UINT32 len)
{
    ADS_TIMER_Ctx           *tiCtx  = VOS_NULL_PTR;
    ADS_CONTEXT             *adsCtx = VOS_NULL_PTR;
    VOS_UINT32               ret;
    VOS_TimerPrecisionUint32 timerPrecision;

    /* 不在使用的定时器范围内 */
    if (timerId >= ADS_MAX_TIMER_NUM) {
        ADS_ERROR_LOG1("ADS_StartTimer: timer id is invalid. <enTimerId>", timerId);
        return;
    }

    /* 获取定时器上下文 */
    adsCtx = ADS_GetCtx();
    tiCtx  = &(adsCtx->timerCtx[timerId]);

    /* 目前只有流量统计定时器 */
    timerPrecision = VOS_TIMER_NO_PRECISION;

    /* 定时器长度检查 */
    if (len == 0) {
        ADS_ERROR_LOG("ADS_StartTimer: timer len is 0,");
        return;
    }

    /* 定时器已运行 */
    if (tiCtx->timer != VOS_NULL_PTR) {
        return;
    }

    /* 启动定时器 */
    ret = VOS_StartRelTimer(&(tiCtx->timer), ACPU_PID_ADS_UL, len, timerId, 0, VOS_RELTIMER_NOLOOP, timerPrecision);

    if (ret != VOS_OK) {
        ADS_ERROR_LOG1("ADS_StartTimer: timer start failed! <ret>", ret);
        return;
    }

    /* 勾包ADS_TIMER_Info */
    ADS_MntnTraceTimerOperation(ACPU_PID_ADS_UL, timerId, len, ADS_TIMER_OPERATION_START,
                                ADS_TIMER_STOP_CAUSE_ENUM_BUTT);
}

VOS_VOID ADS_StopTimer(ADS_TIMER_IdUint32 timerId, ADS_TIMER_StopCauseUint8 stopCause)
{
    ADS_CONTEXT   *adsCtx = VOS_NULL_PTR;
    ADS_TIMER_Ctx *tiCtx  = VOS_NULL_PTR;

    /* 不在使用的定时器范围内 */
    if (timerId >= ADS_MAX_TIMER_NUM) {
        ADS_ERROR_LOG1("ADS_StopTimer: timer id is invalid. <enTimerId>", timerId);
        return;
    }

    /* 获取定时器上下文 */
    adsCtx = ADS_GetCtx();
    tiCtx  = &(adsCtx->timerCtx[timerId]);

    /* 停止定时器 */
    if (tiCtx->timer != VOS_NULL_PTR) {
        (VOS_VOID)VOS_StopRelTimer(&(tiCtx->timer));
    }

    /* 勾包ADS_TIMER_Info */
    ADS_MntnTraceTimerOperation(ACPU_PID_ADS_UL, timerId, 0, ADS_TIMER_OPERATION_STOP, stopCause);
}

