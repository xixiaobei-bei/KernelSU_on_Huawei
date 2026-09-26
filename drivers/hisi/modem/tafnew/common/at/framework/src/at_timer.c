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

#include "at_timer.h"
#include "at_cmd_proc.h"
#include "mn_comm_api.h"


#define THIS_FILE_ID PS_FILE_ID_AT_TIMER_C

VOS_UINT32 AT_StartRelTimer(HTIMER *phTm, VOS_UINT32 length, VOS_UINT32 name, VOS_UINT32 param, VOS_UINT8 mode)
{
    VOS_UINT32         ret;
    AT_TIMER_Operation timer;

    if (length == 0) {
        AT_ERR_LOG("AT_StartRelTimer: Start Rel Timer Length is 0!");
        return VOS_ERR;
    }

    if (length >= VOS_TIMER_MAX_LENGTH) {
        length = VOS_TIMER_MAX_LENGTH - 1;
    }

    ret = VOS_StartRelTimer(phTm, WUEPS_PID_AT, length, name, param, mode, VOS_TIMER_PRECISION_5);

    if (ret == VOS_ERR) {
        AT_ERR_LOG("AT_StartRelTimer: Start Rel Timer Fail!");
    }

    TAF_CfgMsgHdr((MsgBlock *)(&timer), WUEPS_PID_AT, VOS_PID_TIMER, sizeof(AT_TIMER_Operation) - VOS_MSG_HEAD_LENGTH);

    timer.msgName    = name;
    timer.timeAction = AT_TIMER_OPERATION_START;
    timer.timeLen    = length;

    mdrv_diag_trace_report(&timer);

    return ret;
}

VOS_UINT32 AT_StopRelTimer(VOS_UINT32 name, HTIMER *phTm)
{
    VOS_UINT32         ret;
    AT_TIMER_Operation timer;

    ret = VOS_StopRelTimer(phTm);

    if (ret == VOS_ERR) {
        AT_ERR_LOG("AT_StopRelTimer: Stop Rel Timer Fail!");
    }

    TAF_CfgMsgHdr((MsgBlock *)(&timer), WUEPS_PID_AT, VOS_PID_TIMER, sizeof(AT_TIMER_Operation) - VOS_MSG_HEAD_LENGTH);

    timer.msgName    = name;
    timer.timeAction = AT_TIMER_OPERATION_STOP;
    timer.timeLen    = 0x0;

    mdrv_diag_trace_report(&timer);

    return ret;
}

VOS_UINT32 At_StartTimer(VOS_UINT32 length, VOS_UINT8 indexNum)
{
    VOS_StopRelTimer(&g_atClientTab[indexNum].hTimer);
    if (AT_StartRelTimer(&g_atClientTab[indexNum].hTimer, length, indexNum, 0, VOS_RELTIMER_NOLOOP) != VOS_OK) {
        AT_ERR_LOG("At_StartTimer:ERROR:Start Timer");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

