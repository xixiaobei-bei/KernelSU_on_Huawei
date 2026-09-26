/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "at_safety_mm_rslt_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_event_report.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "taf_drv_agent.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAFETY_MM_RSLT_PROC_C

VOS_UINT32 At_ProcPihFndBndCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CLCK_PIN_HANDLE) {
        AT_WARN_LOG("At_ProcPihFndBndCnf : CmdCurrentOpt is not AT_CMD_CLCK_PIN_HANDLE!");
        return AT_ERROR;
    }

    /* 如果是状态查询命令 */
    if (event->pihEvent.fdnCnf.fdnCmd == SI_PIH_FDN_BDN_QUERY) {
        (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s", g_atCrLf);
        (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + (*length), "%d", event->pihEvent.fdnCnf.fdnState);
    }

    return AT_OK;
}

VOS_UINT32 AT_ProcMnCallBackCmdCnfCnapQry(TAF_UINT8 indexNum, TAF_UINT32 errorCode)
{
    TAF_UINT32      result = AT_FAILURE;
    result = AT_ConvertCallError(errorCode);
    AT_StopTimerCmdReady(indexNum);
    return result;
}

