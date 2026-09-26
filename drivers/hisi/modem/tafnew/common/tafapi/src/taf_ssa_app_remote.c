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

#include "vos.h"
#include "taf_tafm_remote.h"
#include "taf_app_ssa.h"
#include "taf_api.h"

#include "mn_comm_api.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_SSA_APP_REMOTE_C

TAF_UINT32 TAF_RegisterSSReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_RegisterssReq *para)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_REGISTERSS_MSG, para, sizeof(TAF_SS_RegisterssReq),
                                  I0_WUEPS_PID_TAF);
}

TAF_UINT32 TAF_EraseSSReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_ErasessReq *para)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_ERASESS_MSG, para, sizeof(TAF_SS_ErasessReq),
                                  I0_WUEPS_PID_TAF);
}

TAF_UINT32 TAF_ActivateSSReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_ActivatessReq *para

)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_ACTIVATESS_MSG, para, sizeof(TAF_SS_ActivatessReq),
                                  I0_WUEPS_PID_TAF);
}

TAF_UINT32 TAF_DeactivateSSReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_DeactivatessReq *para)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_DEACTIVATESS_MSG, para, sizeof(TAF_SS_DeactivatessReq),
                                  I0_WUEPS_PID_TAF);
}

TAF_UINT32 TAF_InterrogateSSReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_InterrogatessReq *para)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_INTERROGATESS_MSG, para, sizeof(TAF_SS_InterrogatessReq),
                                  I0_WUEPS_PID_TAF);
}

TAF_UINT32 TAF_RegisterPasswordReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_RegpwdReq *para)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_REGPWD_MSG, para, sizeof(TAF_SS_RegpwdReq),
                                  I0_WUEPS_PID_TAF);
}

TAF_UINT32 TAF_ProcessUnstructuredSSReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_ProcessUssReq *para)
{
    TAF_SS_ProcessUssReq *ssReq             = VOS_NULL_PTR;
    MN_CLIENT_OperationId clientOperationId = {0};

    ssReq = para;
    if ((TAF_SS_7bit_LANGUAGE_UNSPECIFIED == ssReq->datacodingScheme) || (ssReq->datacodingScheme == TAF_SS_8bit) ||
        (ssReq->datacodingScheme == TAF_SS_UCS2) || (ssReq->datacodingScheme == TAF_SS_7BIT_DATA_CODING) ||
        (ssReq->ussdTransMode == AT_USSD_TRAN_MODE)) {
        clientOperationId.clientId = clientId;
        clientOperationId.opId     = opId;

        return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_PROCESS_USS_MSG, ssReq, sizeof(TAF_SS_ProcessUssReq),
                                      I0_WUEPS_PID_TAF);
    } else {
        return TAF_FAILURE;
    }
}

VOS_UINT32 TAF_EraseCCEntryReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, TAF_SS_EraseccEntryReq *para)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_ERASECCENTRY_MSG, para, sizeof(TAF_SS_EraseccEntryReq),
                                  I0_WUEPS_PID_TAF);
}

TAF_UINT32 TAF_SsReleaseComplete(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    MN_CLIENT_OperationId clientOperationId = {0};

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;

    return MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_RLEASE_MSG, VOS_NULL_PTR, 0, I0_WUEPS_PID_TAF);
}

