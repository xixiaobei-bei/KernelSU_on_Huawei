/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "taf_mta_api.h"
#include "vos.h"
#include "mn_comm_api.h"
#include "securec.h"
#include "mta_comm_interface.h"
#include "ps_common_def.h"
#include "taf_type_def.h"

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "nas_multi_instance_api.h"
#include "nas_comm.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_TAF_MTA_API_C

#if (FEATURE_LTEV == FEATURE_ON)

MODULE_EXPORTED VOS_UINT32 TAF_MTA_VModeSetReq(TAF_MTA_Ctrl *ctrl, TAF_MTA_VModeRatTypeUint8 rat,
                                               TAF_MTA_VModePwModeTypeUint8 mode)
{
    TAF_MTA_VMODE_SetReq *reqMsg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;
    errno_t               memResult;

    if (ctrl == VOS_NULL_PTR) {
        return TAF_MTA_ERR_INPUT_NULL_PTR;
    }

    receiverPid = TAF_GetDestPid(ctrl->clientId, I0_UEPS_PID_MTA);
    senderPid   = TAF_GetSendPid(ctrl->clientId, WUEPS_PID_TAF);

    reqMsg      = (TAF_MTA_VMODE_SetReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MTA_VMODE_SetReq));
    if (reqMsg == VOS_NULL_PTR) {
        return TAF_MTA_ERR_ALLOC_FAIL;
    }
    TAF_CfgMsgHdr((MsgBlock *)reqMsg, senderPid, receiverPid, sizeof(TAF_MTA_VMODE_SetReq) - VOS_MSG_HEAD_LENGTH);
    reqMsg->msgName = ID_TAF_MTA_VMODE_SET_REQ;
    memResult       = memcpy_s(&reqMsg->ctrl, sizeof(TAF_MTA_Ctrl), ctrl, sizeof(TAF_MTA_Ctrl));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(TAF_MTA_Ctrl), sizeof(TAF_MTA_Ctrl));
    reqMsg->rat  = rat;
    reqMsg->mode = mode;
    if (TAF_TraceAndSendMsg(senderPid, reqMsg) != VOS_OK) {
        return TAF_MTA_ERR_SEND_FAIL;
    }
    return TAF_MTA_OK;
}

MODULE_EXPORTED VOS_UINT32 TAF_MTA_VModeQryReq(TAF_MTA_Ctrl *ctrl)
{
    TAF_MTA_VMODE_QryReq *reqMsg = VOS_NULL_PTR;
    VOS_UINT32            receiverPid;
    VOS_UINT32            senderPid;
    errno_t               memResult;

    if (ctrl == VOS_NULL_PTR) {
        return TAF_MTA_ERR_INPUT_NULL_PTR;
    }

    receiverPid = TAF_GetDestPid(ctrl->clientId, I0_UEPS_PID_MTA);
    senderPid   = TAF_GetSendPid(ctrl->clientId, WUEPS_PID_TAF);

    reqMsg      = (TAF_MTA_VMODE_QryReq *)TAF_AllocMsgWithHeaderLen(senderPid, sizeof(TAF_MTA_VMODE_QryReq));
    if (reqMsg == VOS_NULL_PTR) {
        return TAF_MTA_ERR_ALLOC_FAIL;
    }
    TAF_CfgMsgHdr((MsgBlock *)reqMsg, senderPid, receiverPid, sizeof(TAF_MTA_VMODE_QryReq) - VOS_MSG_HEAD_LENGTH);
    reqMsg->msgName = ID_TAF_MTA_VMODE_QRY_REQ;
    memResult       = memcpy_s(&reqMsg->ctrl, sizeof(TAF_MTA_Ctrl), ctrl, sizeof(TAF_MTA_Ctrl));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(TAF_MTA_Ctrl), sizeof(TAF_MTA_Ctrl));
    if (TAF_TraceAndSendMsg(senderPid, reqMsg) != VOS_OK) {
        return TAF_MTA_ERR_SEND_FAIL;
    }
    return TAF_MTA_OK;
}
#endif

