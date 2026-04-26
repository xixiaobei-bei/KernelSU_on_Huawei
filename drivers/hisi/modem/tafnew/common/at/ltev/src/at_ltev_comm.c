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
#include "at_ltev_comm.h"
#include "securec.h"
#include "taf_type_def.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "taf_v2x_api.h"
#endif
#include "mn_comm_api.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_COMM_C

#if (FEATURE_LTEV == FEATURE_ON)
VOS_UINT32 AT_SndVtcMsg(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, AT_VTC_MsgTypeUint32 msgName,
                        const VOS_UINT8 *data, VOS_UINT32 dataSize)
{
    AT_VTC_MsgReq *msg = VOS_NULL_PTR;
    errno_t        memResult;

    msg = (AT_VTC_MsgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VTC_MsgReq) - VOS_MSG_HEAD_LENGTH + dataSize);
    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, I0_PS_PID_VTC, sizeof(AT_VTC_MsgReq) - VOS_MSG_HEAD_LENGTH + dataSize);
    msg->msgName  = msgName;
    msg->clientId = clientId;
    msg->opId     = opId;

    /* 入参data不为空，且dataSize不为0，说明除消息ID外，还有其他数据需要拷贝 */
    if (dataSize != 0 && data != VOS_NULL_PTR) {
        memResult = memcpy_s(msg->content, dataSize, data, dataSize);
        TAF_MEM_CHK_RTN_VAL(memResult, dataSize, dataSize);
    }
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
#endif

