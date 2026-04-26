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
#include "at_custom_nrmm_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_set_para_cmd.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_NRMM_SET_CMD_PROC_C

#define AT_C5GOPTION_PARA_NUM 3
#define AT_C5GOPTION_SA_SUPPORT_FLAG 0
#define AT_C5GOPTION_DC_MODE 1
#define AT_C5GOPTION_5GCACCESSMODE 2

#define AT_NRBANDBLOCKLIST_PARA_MIN_NUM 1
#define AT_NRBANDBLOCKLIST_PARA_MAX_NUM 3
#define AT_NRBANDBLOCKLIST_PARA_OPTION 0    /* NRBANDBLOCKLIST的第一个参数OPTION */
#define AT_NRBANDBLOCKLIST_PARA_BAND_NUM 1  /* NRBANDBLOCKLIST的第二个参数BAND_NUM */
#define AT_NRBANDBLOCKLIST_PARA_BAND_LIST 2 /* NRBANDBLOCKLIST的第三个参数BAND_LIST */

#define AT_NRBANDBLOCKLIST_PARA_BAND_VALID_VALUE1 1
#define AT_NRBANDBLOCKLIST_PARA_BAND_VALID_VALUE2 2
#define AT_C5GNSSAA_PARA_NUM 2
#define AT_C5GNSSAA_SNSSAI 0
#define AT_C5GNSSAA_EAPMSG 1

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 At_Set5gOptionPara(TAF_UINT8 indexNum)
{
    AT_MTA_Set5GOptionReq option5G;
    VOS_UINT32            rst;

    (VOS_VOID)memset_s(&option5G, sizeof(option5G), 0x00, sizeof(option5G));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_C5GOPTION_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_C5GOPTION_SA_SUPPORT_FLAG].paraLen == 0) ||
        (g_atParaList[AT_C5GOPTION_DC_MODE].paraLen == 0) ||
        (g_atParaList[AT_C5GOPTION_5GCACCESSMODE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    option5G.nrSaSupportFlag = (VOS_UINT8)g_atParaList[AT_C5GOPTION_SA_SUPPORT_FLAG].paraValue;
    option5G.nrDcMode        = (AT_MTA_NrDcModeUint8 )g_atParaList[AT_C5GOPTION_DC_MODE].paraValue;
    option5G.en5gcAccessMode = (AT_MTA_5GcAccessModeUint8 )g_atParaList[AT_C5GOPTION_5GCACCESSMODE].paraValue;

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_5G_OPTION_CFG_REQ,
                                 (VOS_UINT8 *)&option5G, sizeof(AT_MTA_Set5GOptionReq), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_5G_OPTION_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

LOCAL VOS_UINT32 AT_IsNrBandValid(VOS_UINT16 nrBand)
{
    if ((nrBand >= 1) && (nrBand <= TAF_MMA_MAX_NR_BAND_INDEX)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

LOCAL VOS_UINT32 AT_IsNrBandParaCharValid(VOS_UINT8 charactor)
{
    if (isdigit(charactor) || (charactor == ',')) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

LOCAL VOS_UINT32 AT_GetNrBandBlockList(VOS_UINT16 *nrBandBlockList, VOS_UINT32 bandMaxNum)
{
    VOS_UINT8 *tempPtr = VOS_NULL_PTR;
    VOS_UINT32 bandNum = 0;
    VOS_UINT32 nrBand  = 0;
    VOS_UINT16 bandLen = 0;
    VOS_UINT32 i;

    tempPtr = g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_LIST].para;

    for (i = 0; i < (VOS_UINT32)g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_LIST].paraLen + 1U; i++) {
        if (i < g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_LIST].paraLen) {
            if (AT_IsNrBandParaCharValid(g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_LIST].para[i]) != VOS_TRUE) {
                return 0;
            }

            if (g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_LIST].para[i] != ',') {
                bandLen++;
                if (bandLen > 3) {
                    /* 单个Band值不允许超过3位数 */
                    return 0;
                }
                continue;
            }
        }

        if (bandLen != 0) {
            if (bandNum >= bandMaxNum) {
                /* 超过最大Band个数按照失败返回 */
                return 0;
            }

            if (atAuc2ul(tempPtr, bandLen, &nrBand) != AT_SUCCESS) {
                return 0;
            }

            if (AT_IsNrBandValid((VOS_UINT16)nrBand) != VOS_TRUE) {
                return 0;
            }

            nrBandBlockList[bandNum] = (VOS_UINT16)nrBand;
            bandNum++;
            tempPtr += bandLen + 1;
            bandLen = 0;
        }
    }

    return bandNum;
}

LOCAL VOS_UINT32 AT_HasIdenticalItems(VOS_UINT16 *list, VOS_UINT32 num)
{
    VOS_UINT32 i, j;

    for (i = 0; i < num; i++) {
        for (j = i + 1; j < num; j++) {
            if (list[i] == list[j]) {
                return VOS_TRUE;
            }
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_SetNrBandBlockListWith3Para(TAF_MMA_NrBandBlockListSetPara *nrBandBlockListSetStru)
{
    VOS_UINT32 maxStrlen;

    /* 字符串最长的长度是10个3位数和9个逗号，共39 */
    maxStrlen = 3 * TAF_MMA_MAX_NR_BAND_BLOCKLIST_NUM + 9;

    if ((g_atParaList[AT_NRBANDBLOCKLIST_PARA_OPTION].paraValue !=
        TAF_MMA_NR_BAND_SET_OPTION_DEL_BAND_FROM_BLOCK_LIST) &&
        (g_atParaList[AT_NRBANDBLOCKLIST_PARA_OPTION].paraValue !=
        TAF_MMA_NR_BAND_SET_OPTION_ADD_BAND_TO_BLOCK_LIST)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_NUM].paraLen != AT_NRBANDBLOCKLIST_PARA_BAND_VALID_VALUE1) &&
        (g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_NUM].paraLen != AT_NRBANDBLOCKLIST_PARA_BAND_VALID_VALUE2)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_LIST].paraLen == 0) ||
        (g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_LIST].paraLen > maxStrlen)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    nrBandBlockListSetStru->option  = g_atParaList[AT_NRBANDBLOCKLIST_PARA_OPTION].paraValue;
    nrBandBlockListSetStru->bandNum = AT_GetNrBandBlockList(nrBandBlockListSetStru->bandList,
                                                            TAF_MMA_MAX_NR_BAND_BLOCKLIST_NUM);

    if (nrBandBlockListSetStru->bandNum != g_atParaList[AT_NRBANDBLOCKLIST_PARA_BAND_NUM].paraValue) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_HasIdenticalItems(nrBandBlockListSetStru->bandList, nrBandBlockListSetStru->bandNum) == VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetNrBandBlockListPara(VOS_UINT8 atIndex)
{
    TAF_MMA_NrBandBlockListSetPara nrBandBlockListSetStru;
    ModemIdUint16                  modemId;
    VOS_UINT32                     rslt;

    rslt = AT_GetModemIdFromClient(atIndex, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG1("AT_SetNrBandBlockListPara:Get ModemID From ClientID fail,ClientID:", atIndex);
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果参数长度不符合命令 */
    if (g_atParaList[AT_NRBANDBLOCKLIST_PARA_OPTION].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果9203不支持NR，直接返回ERR */
    if (AT_IsModemSupportRat(modemId, TAF_MMA_RAT_NR) != VOS_TRUE) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(&nrBandBlockListSetStru, sizeof(nrBandBlockListSetStru), 0x00, sizeof(nrBandBlockListSetStru));

    if (g_atParaIndex == AT_NRBANDBLOCKLIST_PARA_MIN_NUM) {
        /* 高频开关 */
        if ((g_atParaList[AT_NRBANDBLOCKLIST_PARA_OPTION].paraValue != TAF_MMA_NR_BAND_SET_OPTION_TURN_FR2_ON) &&
            (g_atParaList[AT_NRBANDBLOCKLIST_PARA_OPTION].paraValue != TAF_MMA_NR_BAND_SET_OPTION_TURN_FR2_OFF)) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            nrBandBlockListSetStru.option = g_atParaList[AT_NRBANDBLOCKLIST_PARA_OPTION].paraValue;
        }
    } else if (g_atParaIndex == AT_NRBANDBLOCKLIST_PARA_MAX_NUM) {
        /* BAND Block名单设置 */
        if (AT_SetNrBandBlockListWith3Para(&nrBandBlockListSetStru) != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (TAF_MMA_SetNrBandBlockListReq(WUEPS_PID_AT, g_atClientTab[atIndex].clientId, 0, &nrBandBlockListSetStru) ==
        VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[atIndex].cmdCurrentOpt = AT_CMD_NR_BAND_BLOCKLIST_SET;
        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_NSSAI_AUTH == FEATURE_ON)
VOS_UINT32 AT_SetC5gNssaaPara(VOS_UINT8 indexNum)
{
    AT_MTA_5GNwSliceAuthCmplReq *cmplReq = VOS_NULL_PTR;
    VOS_UINT32                   rslt;

    if (g_atParaIndex != AT_C5GNSSAA_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_C5GNSSAA_SNSSAI].paraLen == 0 || g_atParaList[AT_C5GNSSAA_EAPMSG].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    cmplReq = (AT_MTA_5GNwSliceAuthCmplReq *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(AT_MTA_5GNwSliceAuthCmplReq));
    if (cmplReq == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_SetC5gNssaaPara: PS_MEM_ALLOC fail.");
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(cmplReq, sizeof(*cmplReq), 0x00, sizeof(*cmplReq));

    cmplReq->eapMsg.eapMsgSize = AT_MIN((g_atParaList[AT_C5GNSSAA_EAPMSG].paraLen) / 2, AT_PARA_MAX_LEN + 1);
    cmplReq->eapMsg.eapMsgSize = AT_MIN(cmplReq->eapMsg.eapMsgSize, AT_MTA_EAP_MSG_MAX_NUM);

    if (AT_DecodeSNssai(g_atParaList[AT_C5GNSSAA_SNSSAI].para,
        g_atParaList[AT_C5GNSSAA_SNSSAI].paraLen, &cmplReq->sNssai) != VOS_OK) {
        PS_MEM_FREE(WUEPS_PID_AT, cmplReq);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiString2HexSimple(cmplReq->eapMsg.eapMsg, g_atParaList[AT_C5GNSSAA_EAPMSG].para,
                                 g_atParaList[AT_C5GNSSAA_EAPMSG].paraLen) == AT_FAILURE) {
        PS_MEM_FREE(WUEPS_PID_AT, cmplReq);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NW_SLICE_AUTH_CMPL_REQ, (VOS_UINT8 *)cmplReq, sizeof(*cmplReq), I0_UEPS_PID_MTA);
    if (rslt != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetC5gNssaaPara: AT_FillAndSndAppReqMsg fail.");
        PS_MEM_FREE(WUEPS_PID_AT, cmplReq);
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GNSSAA_SET;

    PS_MEM_FREE(WUEPS_PID_AT, cmplReq);

    return AT_OK;
}
#endif
#endif

