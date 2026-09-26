/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
 * 1 Include HeadFile
 */

#include "ndis_msg_proc.h"
#include "securec.h"
#include "ps_tag.h"
#include "ps_common_def.h"
#include "ndis_interface.h"
#include "ndis_entity.h"
#include "ndis_arp_proc.h"
#include "ndis_at_proc.h"
#include "ndis_ads_proc.h"
#include "ndis_dl_proc.h"


#define THIS_MODU ps_ndis
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDISMSGPROC_C
/*lint +e767*/

/* 统计信息 */

/*
 * 2 Declare the Global Variable
 */

/*
 * 功能描述: 处理TmerMsg
 * 修改历史:
 *  1.日    期: 2012年4月28日
 *    修改内容: 新生成函数
 */
STATIC VOS_VOID Ndis_ProcTmrMsg(const REL_TimerMsgBlock *rcvMsg)
{
    if (sizeof(REL_TimerMsgBlock) - VOS_MSG_HEAD_LENGTH > rcvMsg->ulLength) {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_ProcTmrMsg, input msg length less than struc", rcvMsg->ulLength);
        return;
    }

    switch (rcvMsg->name) {
        case NDIS_PERIOD_ARP_TMRNAME:
            Ndis_ProcARPTimerExp();
            break;
        default:
            NDIS_INFO_LOG1(NDIS_TASK_PID, "Ndis_ProcTmrMsg, Recv other Timer", rcvMsg->name);
            break;
    }

    return;
}

VOS_UINT32 NDIS_RecvMsgParaCheck(VOS_UINT32 length, const NDIS_MsgTypeStruLen *msgStruLen,
                                 VOS_UINT32 msgIdNum, VOS_UINT32 msgId)
{
    VOS_UINT32  msgNumFloop;
    for (msgNumFloop = 0; msgNumFloop < msgIdNum; msgNumFloop++) {
        if (msgId == msgStruLen[msgNumFloop].msgId) {
            if (length < (msgStruLen[msgNumFloop].msTypeLenth - VOS_MSG_HEAD_LENGTH)) {
                return PS_FAIL;
            }
            return PS_SUCC;
        }
    }
    return PS_SUCC;
}

/*
 * 功能描述: NDIS接收各模块消息处理函数
 * 修改历史:
 *  1.日    期: 2011年2月15日
 *    修改内容: 新生成函数
 */
VOS_VOID APP_Ndis_PidMsgProc(MsgBlock *msgBlock)
{
    if (msgBlock == VOS_NULL_PTR) {
        PS_PRINTF_INFO("Error:APP_Ndis_DLPidMsgProc Parameter pRcvMsg is NULL!");
        return;
    }

    switch (msgBlock->ulSenderPid) {
        case DOPRA_PID_TIMER:
            /*lint -e826*/
            Ndis_ProcTmrMsg((REL_TimerMsgBlock *)msgBlock);
            /*lint +e826*/
            break;

        case APP_AT_PID:
            Ndis_AtMsgProc(msgBlock);
            break;

        case ACPU_PID_ADS_UL: /* ADS通过OSA消息发送DHCP和ND SERVER包给NDIS模块 */
            Ndis_AdsMsgDispatch(msgBlock);
            break;
        default:
            NDIS_WARNING_LOG(NDIS_TASK_PID, "Warning:APP_Ndis_PidMsgProc Recv not expected msg!");
            break;
    }

    return;
}

/*
 * 功能描述: 发送Cnf消息到AT
 * 修改历史:
 *  1.日    期: 2011年3月16日
 *    修改内容: 新生成函数
 */
VOS_UINT32 Ndis_SndMsgToAt(const VOS_UINT8 *buf, VOS_UINT16 msgLen, VOS_UINT32 msgId)
{
    MsgBlock        *msgBlock  = VOS_NULL_PTR;
    MSG_Header      *msgHeader = VOS_NULL_PTR;
    errno_t          rlt;

    /*lint -e516 -esym(516,*)*/
    msgBlock = (MsgBlock *)PS_ALLOC_MSG(NDIS_TASK_PID, msgLen - VOS_MSG_HEAD_LENGTH);
    /*lint -e516 +esym(516,*)*/
    if (msgBlock == VOS_NULL_PTR) {
        return PS_FAIL;
    }

    msgHeader = (MSG_Header *)(VOS_VOID *)msgBlock;

    rlt = memcpy_s(msgBlock->value, msgLen - VOS_MSG_HEAD_LENGTH, (buf + VOS_MSG_HEAD_LENGTH),
                   msgLen - VOS_MSG_HEAD_LENGTH);
    if (rlt != EOK) {
        PS_FREE_MSG(NDIS_TASK_PID, msgBlock);
        return PS_FAIL;
    }

    msgHeader->ulSenderPid   = NDIS_TASK_PID;
    msgHeader->ulReceiverPid = APP_AT_PID;
    msgHeader->msgName     = msgId;

    if (PS_SEND_MSG(NDIS_TASK_PID, msgBlock) != VOS_OK) {
        /* 异常打印 */
        return PS_FAIL;
    }

    return PS_SUCC;
}



