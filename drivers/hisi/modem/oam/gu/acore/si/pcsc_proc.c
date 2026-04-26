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

/*
 * 版权所有（c）华为技术有限公司 2012-2019
 * 功能描述: 该文件定义了ACPU上PCSC流程实现
 * 生成日期: 2011年9月19日
 */
#include "vos.h"
#include "pam_tag.h"
#include "si_typedef.h"
#include "usimm_ps_interface.h"


#define THIS_FILE_ID PS_FILE_ID_PCSC_APP_PROC_C
#define THIS_MODU mod_pam_pcsc

/* ACPU上维护卡状态的全局变量 */
USIMM_CardAppServicUint32 g_acpuCardStatus = USIMM_CARDAPP_SERVIC_BUTT;

extern VOS_VOID OM_RecordInfoStart(VOS_ExcDumpMemNumUint32 number, VOS_UINT32 sendPid, VOS_UINT32 rcvPid,
    VOS_UINT32 msgName);
extern VOS_VOID OM_RecordInfoEnd(VOS_ExcDumpMemNumUint32 number);

/*
 * 功能描述: 收到CCPU发送来的卡状态更新消息处理函数
 * 1. 日    期: 2011年10月10日
 *    修改内容: Creat
 */
VOS_VOID PCSC_UpdateCardStatus(USIMM_CardStatusInd *msg)
{
    if (msg->ulLength != (sizeof(USIMM_CardStatusInd) - VOS_MSG_HEAD_LENGTH)) {
        mdrv_err("<PCSC_UpdateCardStatus> length error.\n");
        return;
    }

    if (g_acpuCardStatus == USIMM_CARDAPP_SERVIC_BUTT) {
        mdrv_debug("<PCSC_UpdateCardStatus> Reg PCSC Func.\n");
    }

    /* 更新本地卡状态的全局变量 */
    g_acpuCardStatus = msg->usimSimInfo.cardAppService;

    mdrv_debug("<PCSC_UpdateCardStatus> g_acpuCardStatus=%d .\n", g_acpuCardStatus);

    return;
}

/*
 * 功能描述: PCSC消息处理函数
 * 1. 日    期: 2010年10月10日
 *    修改内容: Creat
 */
VOS_VOID PCSC_AcpuMsgProc(MsgBlock *msg)
{
    PS_SI_Msg *pcscMsg = VOS_NULL_PTR;

    if (msg == VOS_NULL_PTR) {
        mdrv_err("<PCSC_AcpuMsgProc> msg is null");
        return;
    }

    pcscMsg = (PS_SI_Msg *)msg;

    if (pcscMsg->ulLength < sizeof(pcscMsg->msgName)) {
        mdrv_err("<PCSC_AcpuMsgProc> msg len err, %d.", pcscMsg->ulLength);
        return;
    }

    OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, msg->ulSenderPid, ACPU_PID_PCSC, *((VOS_UINT32 *)msg->value));

    switch (pcscMsg->msgName) {
        case USIMM_CARDSTATUS_IND:
            PCSC_UpdateCardStatus((USIMM_CardStatusInd *)msg);
            break;
        default:
            mdrv_err("<PCSC_AcpuMsgProc> msg err, %d.", pcscMsg->msgName);
            break;
    }

    OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

    return;
}

