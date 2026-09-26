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
#include "at_sat_pam_qry_cmd_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_sim_comm.h"
#include "si_app_stk.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAT_PAM_QRY_CMD_PROC_C

TAF_UINT32 At_QryStsfPara(TAF_UINT8 indexNum)
{
    USIMM_StkCfg  profileContent = {0};
    TAF_UINT32    result;
    TAF_UINT16    nVLen  = sizeof(USIMM_StkCfg);
    TAF_UINT16    length = 0;
    ModemIdUint16 modemId;

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TERMINALPROFILE_SET, &profileContent, nVLen);

    if (result != NV_OK) {
        return AT_ERROR;
    }

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "^STSF:%d,", profileContent.funcEnable);

    if (AT_GetUsimInfoCtxFromModemId(modemId)->cardType == TAF_MMA_USIMM_CARD_USIM) {
        length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                      profileContent.usimProfile.profile,
                                                      TAF_MIN(profileContent.usimProfile.profileLen,
                                                              sizeof(profileContent.usimProfile.profile)));
    } else {
        length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                      profileContent.simProfile.profile,
                                                      TAF_MIN(profileContent.simProfile.profileLen,
                                                              sizeof(profileContent.simProfile.profile)));
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", profileContent.timer);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) || (defined(DMT)) || (defined(__PC_UT__)))
/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_QryStgiPara(TAF_UINT8 indexNum)
{
    if (g_stkFunctionFlag == TAF_FALSE) {
        return AT_ERROR;
    }

    if (SI_STK_QuerySTKCommand(g_atClientTab[indexNum].clientId, USAT_CMDQRYSTGI) == AT_SUCCESS) {
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 At_QrySIMSlotPara(VOS_UINT8 indexNum)
{
    TAF_NvSciCfg sCICfg;
    VOS_UINT16   length;

    /* 从NV中读取当前SIM卡的SCI配置 */
    (VOS_VOID)memset_s(&sCICfg, sizeof(sCICfg), 0x00, sizeof(sCICfg));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SCI_DSDA_CFG, &sCICfg, sizeof(sCICfg)) != NV_OK) {
        AT_ERR_LOG("At_QrySIMSlotPara: NV_ITEM_SCI_DSDA_CFG read fail!");
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, sCICfg.bitCard0,
        sCICfg.bitCard1);

#if (MULTI_MODEM_NUMBER == 3)
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", sCICfg.bitCard2);
#endif

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_TestStsfPara(VOS_UINT8 indexNum)
{
    VOS_INT bufLen;

    bufLen = snprintf_s((TAF_CHAR *)g_atSndCodeAddress,
                        AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3,
                        (AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3) - 1,
                        "%s:(0-2),(160060c0-5ffffff7),(1-255)",
                        g_parseContext[indexNum].cmdElement->cmdName);

    if (bufLen < 0) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)bufLen;

    return AT_OK;
}

VOS_UINT32 AT_TestStgiPara(VOS_UINT8 indexNum)
{
    if (g_stkFunctionFlag == TAF_FALSE) {
        return AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: (0-12)", g_parseContext[indexNum].cmdElement->cmdName);
        return AT_OK;
    }
}

VOS_UINT32 AT_TestStgrPara(VOS_UINT8 indexNum)
{
    if (g_stkFunctionFlag == TAF_FALSE) {
        return AT_ERROR;
    } else {
        return AT_OK;
    }
}
