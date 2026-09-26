/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include "pppa_task.h"
#include "ttf_comm.h"
#include "ppp_public.h"
#include "ppp_init.h"
#include "ppp_input.h"
#include "gucttf_tag.h"
#include "linux_interface.h"

#define THIS_FILE_ID PS_FILE_ID_PPPA_TASK_C
#define THIS_MODU mod_ppp

__STATIC_ VOS_UINT32 g_pppTaskId; /* PPP任务ID，通知PPP任务有数处理时使用 */

__STATIC_ VOS_VOID PPP_BindToCpu(VOS_VOID)
{
    struct cpumask mask;

    /* 获取当前线程的affinity */
    if (sched_getaffinity(current->pid, &mask) < 0) {
        PPP_MNTN_LOG(PS_PRINT_WARNING, "warning: unable to get cpu affinity\n");
        return;
    }

    /* 如果CPU核数超过1则去绑定核0 */
    if (cpumask_weight(&mask) > 1) {
        cpumask_clear_cpu(0, &mask);
        if (sched_setaffinity(current->pid, &mask) < 0) {
            PPP_MNTN_LOG(PS_PRINT_WARNING, "warning: unable to set cpu affinity\n");
        }
    }
}

VOS_UINT32 PPP_GetTaskId(VOS_VOID)
{
    return g_pppTaskId;
}

__STATIC_ VOS_VOID PPP_FidTask(VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3, VOS_UINT32 para4)
{
    MsgBlock  *msg = VOS_NULL_PTR;
    VOS_UINT32 event = 0;
    VOS_UINT32 taskId, rtn, eventMask, expectEvent;

    taskId = VOS_GetCurrentTaskID();
    if (taskId == PS_NULL_UINT32) {
        PPP_MNTN_LOG(PS_PRINT_ERROR, "PPP_FidTask: ERROR, TaskID is invalid.");
        return;
    }

    if (VOS_CreateEvent(taskId) != VOS_OK) {
        PPP_MNTN_LOG(PS_PRINT_ERROR, "PPP_FidTask: ERROR, create event fail.");
        return;
    }

    PPP_BindToCpu();

    expectEvent = PPP_RCV_DATA_EVENT | VOS_MSG_SYNC_EVENT;
    eventMask   = VOS_EVENT_ANY | VOS_EVENT_WAIT;

    g_pppTaskId  = taskId;

    for (;;) {
        rtn = VOS_EventRead(expectEvent, eventMask, 0, &event);
        if (rtn != VOS_OK) {
            PPP_MNTN_LOG(PS_PRINT_ERROR, "PPP_FidTask: ERROR, read event error.");
            continue;
        }

        /* 事件处理 */
        if ((event & PPP_RCV_DATA_EVENT) == PPP_RCV_DATA_EVENT) {
            PPPA_DataEventProc();
            continue;
        }

        msg = (MsgBlock*)VOS_GetMsg(taskId);
        if (msg != VOS_NULL_PTR) {
            if (TTF_GET_MSG_RECEIVER_ID(msg) == PS_PID_APP_PPP) {
                PPP_MsgProc(msg);
            }

            (VOS_VOID)VOS_FreeMsg(PS_PID_APP_PPP, msg);
        }
    }
}

VOS_UINT32 PPP_FidInit(enum VOS_InitPhaseDefine ip)
{
    VOS_UINT32 rslt;

    if (ip == VOS_IP_LOAD_CONFIG) {
        rslt = VOS_RegisterPIDInfo(PS_PID_APP_PPP, (InitFunType)PPP_PidInit, (MsgFunType)PPP_MsgProc);
        if (rslt != VOS_OK) {
            TTF_PRINT_ERR("[init]<PPP_FidInit>, register PPP PID fail!\n");
            return VOS_ERR;
        }

        rslt = VOS_RegisterMsgTaskEntry(ACPU_FID_PPP, PPP_FidTask);
        if (rslt != VOS_OK) {
            TTF_PRINT_ERR("[init]<PPP_FidInit>, VOS_RegisterMsgTaskEntry fail!\n");
            return VOS_ERR;
        }

        /* 任务优先级 */
        rslt = VOS_RegisterTaskPrio(ACPU_FID_PPP, VOS_PRIORITY_P4);
        if (rslt != VOS_OK) {
            TTF_PRINT_ERR("[init]<PPP_FidInit>, register priority fail!\n");
            return VOS_ERR;
        }
    }
    return VOS_OK;
}

