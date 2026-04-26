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
#include "at_custom_rrm_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_RRM_SET_CMD_PROC_C

VOS_UINT32 AT_SetActiveModem(VOS_UINT8 indexNum)
{
    TAF_NV_DsdsActiveModemMode mode;
    AT_MTA_ModemCapUpdateReq   atMtaModemCapUpdate;

    (VOS_VOID)memset_s(&atMtaModemCapUpdate, (VOS_UINT32)sizeof(AT_MTA_ModemCapUpdateReq), 0x00,
                       (VOS_UINT32)sizeof(AT_MTA_ModemCapUpdateReq));

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&mode, sizeof(mode), 0x00, sizeof(mode));
    mode.activeModem = (TAF_NV_ActiveModemModeUint8)g_atParaList[0].paraValue;

    /* 写NV, 返回AT_OK */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_DSDS_ACTIVE_MODEM_MODE, (VOS_UINT8 *)&mode, sizeof(mode)) != NV_OK) {
        AT_ERR_LOG("AT_SetActiveModem(): NV_ITEM_DSDS_ACTIVE_MODEM_MODE NV Write Fail!");
        return AT_ERROR;
    }

    /* 更新了NV，通知AT->MTA->RRM，进行底层平台能力更新 */
    /* AT发送至MTA的消息结构赋值 */
    atMtaModemCapUpdate.modemCapUpdateType = AT_MTA_MODEM_CAP_UPDATE_TYPE_ACTIVE_MODEM;

    /* 发送消息给C核处理 */
    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_MODEM_CAP_UPDATE_REQ,
                               (VOS_UINT8 *)&atMtaModemCapUpdate, (VOS_UINT32)sizeof(AT_MTA_ModemCapUpdateReq),
                               I0_UEPS_PID_MTA) != AT_SUCCESS) {
        AT_WARN_LOG("AT_SetActiveModem(): Snd MTA Req Failed!");

        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MODEM_CAP_UPDATE_SET;

    return AT_WAIT_ASYNC_RETURN;
}

