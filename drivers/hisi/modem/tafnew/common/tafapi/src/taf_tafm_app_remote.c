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

#include "mn_comm_api.h"

#include "taf_api.h"

#include "mn_client.h"
#include "taf_app_mma.h"
#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_TAF_TAFM_APPREMOTE_C

/* ************************************************** */
/*    远端API使用的全局变量全部在定义时进行初始化   */
/* ************************************************** */
/* 记录APP回调函数信息结果 */

/* 记录TAF远端是否注册过MUX回调函数 */
VOS_UINT8 g_tafRetRegMuxCallbackFlag = TAF_REG_MUX_CALLBACK_NO;

/* ******************************************************** */
/*                  通信参数设置查询API                   */
/* ******************************************************** */

VOS_UINT32 Taf_ParaQuery(VOS_UINT16 clientId, VOS_UINT8 opId, TAF_PARA_TYPE paraType, VOS_VOID *para)
{
    VOS_UINT8            *paraAddr = VOS_NULL_PTR;
    VOS_UINT16            msgName;
    VOS_UINT32            paraLen;
    VOS_UINT32            rst;
    VOS_UINT32            receiverPid;
    errno_t               memResult;
    MN_CLIENT_OperationId clientOperationId = {0};

    if ((paraType >= TAF_PH_MS_CLASS_PARA) && (paraType < TAF_TELE_PARA_BUTT)) {
        paraLen     = sizeof(TAF_PARA_TYPE);
        receiverPid = I0_WUEPS_PID_MMA;
        msgName     = TAF_MSG_PARA_READ;
    } else {
        return TAF_FAILURE;
    }

    paraAddr = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, paraLen);

    if (paraAddr == VOS_NULL_PTR) {
        return TAF_FAILURE;
    }

    (VOS_VOID)memset_s(paraAddr, paraLen, 0x00, paraLen);

    memResult = memcpy_s(paraAddr, paraLen, &paraType, sizeof(TAF_PARA_TYPE));
    TAF_MEM_CHK_RTN_VAL(memResult, paraLen, sizeof(TAF_PARA_TYPE));

    clientOperationId.clientId = clientId;
    clientOperationId.opId     = opId;
    rst                        = MN_FillAndSndAppReqMsg(&clientOperationId, msgName, paraAddr, paraLen, receiverPid);
    PS_MEM_FREE(WUEPS_PID_AT, paraAddr);

    return rst;
}

