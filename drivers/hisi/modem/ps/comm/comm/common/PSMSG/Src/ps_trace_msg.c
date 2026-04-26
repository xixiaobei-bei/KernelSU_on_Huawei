/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#include "ps_trace_msg.h"
#include "gucttf_tag.h"
#include "ttf_comm.h"

#define THIS_FILE_ID    PS_FILE_ID_PS_TRACE_MSG_C
#define THIS_MODU mod_pstracemsg


VOS_VOID PS_RecordFailLog(const PS_MSG_Mntn *mntn, const char *fmt, VOS_UINT32 ret)
{
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_COMM, PS_PRINT_ERROR), 
        mntn->pid, NULL, (VOS_UINT32)mntn->line, (char *)fmt, ret);
}

VOS_VOID PS_FilterAndTraceMsg(const VOS_VOID *msg, VOS_PID pid, PsMsgFilter filter)
{
}

MODULE_EXPORTED VOS_UINT32 PS_FilterAndSendMsg(const VOS_VOID *msg, const PS_MSG_Mntn *mntn, PsMsgFilter filter)
{
    VOS_UINT32   ret;
    PS_FilterAndTraceMsg(msg, mntn->pid, filter);

    ret = V_SendMsg(mntn->pid, (VOS_VOID**)(&(msg)), mntn->fileId, mntn->line);
    if (ret != VOS_OK) {
        PS_RecordFailLog(mntn, mntn->logFmt, ret);
        return VOS_ERR;
    }
    return VOS_OK;
}

MODULE_EXPORTED VOS_UINT32 PS_FilterAndSendUrgentMsg(const VOS_VOID *msg, const PS_MSG_Mntn *mntn, PsMsgFilter filter)
{
    VOS_UINT32   ret;
    PS_FilterAndTraceMsg(msg, mntn->pid, filter);

    ret = V_SendUrgentMsg(mntn->pid, (VOS_VOID**)(&(msg)), mntn->fileId, mntn->line);
    if (ret != VOS_OK) {
        PS_RecordFailLog(mntn, mntn->logFmt, ret);
        return VOS_ERR;
    }
    return VOS_OK;
}
