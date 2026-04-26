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

#include "at_safety_pam_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAFETY_PAM_SET_CMD_PROC_C

#define AT_UICCAUTH_PARA_NUM 3
#define AT_UICCAUTH_AUTH_TYPE 0
#define AT_UICCAUTH_RAND 1
#define AT_UICCAUTH_AUTH 2
#define AT_KSNAFAUTH_AUTH_TYPE 0
#define AT_KSNAFAUTH_PARA_NUM 3
#define AT_KSNAFAUTH_NAF_ID 1
#define AT_KSNAFAUTH_IMPI 2

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetUiccAuthPara(VOS_UINT8 indexNum)
{
    SI_PIH_UiccAuth authData;
    errno_t         memResult;

    if ((g_atParaIndex != AT_UICCAUTH_PARA_NUM) || (g_atParaList[AT_UICCAUTH_RAND].paraLen == 0) ||
        (g_atParaList[AT_UICCAUTH_AUTH].paraLen == 0)) {
        AT_ERR_LOG("AT_SetUiccAuthPara: para check fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_UICCAUTH_RAND].para,
                              &g_atParaList[AT_UICCAUTH_RAND].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("AT_SetUiccAuthPara: At_AsciiNum2HexString para1 fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_UICCAUTH_AUTH].para,
                              &g_atParaList[AT_UICCAUTH_AUTH].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("AT_SetUiccAuthPara: At_AsciiNum2HexString para2 fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_UICCAUTH_RAND].paraLen > SI_AUTH_DATA_MAX) ||
        (g_atParaList[AT_UICCAUTH_AUTH].paraLen > SI_AUTH_DATA_MAX)) {
        AT_ERR_LOG("AT_SetUiccAuthPara: String Len fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_UICCAUTH_AUTH_TYPE].paraValue == SI_PIH_ATUICCAUTH_USIMAKA) {
        authData.authType = SI_PIH_UICCAUTH_AKA;
        authData.appType  = SI_PIH_UICCAPP_USIM;
    } else if (g_atParaList[AT_UICCAUTH_AUTH_TYPE].paraValue == SI_PIH_ATUICCAUTH_ISIMAKA) {
        authData.authType = SI_PIH_UICCAUTH_AKA;
        authData.appType  = SI_PIH_UICCAPP_ISIM;
    } else if (g_atParaList[AT_UICCAUTH_AUTH_TYPE].paraValue == SI_PIH_ATUICCAUTH_USIMGBA) {
        authData.authType = SI_PIH_UICCAUTH_GBA;
        authData.appType  = SI_PIH_UICCAPP_USIM;
    } else if (g_atParaList[AT_UICCAUTH_AUTH_TYPE].paraValue == SI_PIH_ATUICCAUTH_ISIMGBA) {
        authData.authType = SI_PIH_UICCAUTH_GBA;
        authData.appType  = SI_PIH_UICCAPP_ISIM;
    } else {
        AT_ERR_LOG("AT_SetUiccAuthPara: Para 1 value fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    authData.authData.aka.randLen = g_atParaList[AT_UICCAUTH_RAND].paraLen;

    authData.authData.aka.authLen = g_atParaList[AT_UICCAUTH_AUTH].paraLen;

    /* GBA和AKA鉴权数据结构是一样的 */
    memResult = memcpy_s(authData.authData.aka.rand, sizeof(authData.authData.aka.rand),
                         g_atParaList[AT_UICCAUTH_RAND].para, g_atParaList[AT_UICCAUTH_RAND].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(authData.authData.aka.rand), g_atParaList[AT_UICCAUTH_RAND].paraLen);

    memResult = memcpy_s(authData.authData.aka.auth, sizeof(authData.authData.aka.auth),
                         g_atParaList[AT_UICCAUTH_AUTH].para, g_atParaList[AT_UICCAUTH_AUTH].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(authData.authData.aka.auth), g_atParaList[AT_UICCAUTH_AUTH].paraLen);

    if (SI_PIH_UiccAuthReq(g_atClientTab[indexNum].clientId, 0, &authData) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UICCAUTH_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_ERR_LOG("AT_SetUiccAuthPara: SI_PIH_UiccAuthReq fail.");

        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetKsNafAuthPara(VOS_UINT8 indexNum)
{
    SI_PIH_UiccAuth authData;
    errno_t         memResult;

    if ((g_atParaIndex != AT_KSNAFAUTH_PARA_NUM) || (g_atParaList[AT_KSNAFAUTH_NAF_ID].paraLen == 0) ||
        (g_atParaList[AT_KSNAFAUTH_IMPI].paraLen == 0)) {
        AT_ERR_LOG("AT_SetKsNafAuthPara: check para fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_KSNAFAUTH_NAF_ID].para,
                              &g_atParaList[AT_KSNAFAUTH_NAF_ID].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("AT_SetKsNafAuthPara: At_AsciiNum2HexString para1 fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_KSNAFAUTH_IMPI].para,
                              &g_atParaList[AT_KSNAFAUTH_IMPI].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("AT_SetKsNafAuthPara: At_AsciiNum2HexString para2 fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_KSNAFAUTH_NAF_ID].paraLen > SI_AUTH_DATA_MAX) ||
        (g_atParaList[AT_KSNAFAUTH_IMPI].paraLen > SI_AUTH_DATA_MAX)) {
        AT_ERR_LOG("AT_SetKsNafAuthPara: Str Len check fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_KSNAFAUTH_AUTH_TYPE].paraValue == SI_PIH_ATKSNAFAUTH_USIM) {
        authData.authType = SI_PIH_UICCAUTH_NAF;
        authData.appType  = SI_PIH_UICCAPP_USIM;
    } else if (g_atParaList[AT_KSNAFAUTH_AUTH_TYPE].paraValue == SI_PIH_ATKSNAFAUTH_ISIM) {
        authData.authType = SI_PIH_UICCAUTH_NAF;
        authData.appType  = SI_PIH_UICCAPP_ISIM;
    } else {
        AT_ERR_LOG("AT_SetKsNafAuthPara: Auth type fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    authData.authData.ksNAF.nafIdLen = g_atParaList[AT_KSNAFAUTH_NAF_ID].paraLen;

    authData.authData.ksNAF.impiLen = g_atParaList[AT_KSNAFAUTH_IMPI].paraLen;

    memResult = memcpy_s(authData.authData.ksNAF.nafId, sizeof(authData.authData.ksNAF.nafId),
                         g_atParaList[AT_KSNAFAUTH_NAF_ID].para, g_atParaList[AT_KSNAFAUTH_NAF_ID].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(authData.authData.ksNAF.nafId), g_atParaList[AT_KSNAFAUTH_NAF_ID].paraLen);

    memResult = memcpy_s(authData.authData.ksNAF.impi, sizeof(authData.authData.ksNAF.impi),
                         g_atParaList[AT_KSNAFAUTH_IMPI].para, g_atParaList[AT_KSNAFAUTH_IMPI].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(authData.authData.ksNAF.impi), g_atParaList[AT_KSNAFAUTH_IMPI].paraLen);

    if (SI_PIH_UiccAuthReq(g_atClientTab[indexNum].clientId, 0, &authData) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_KSNAFAUTH_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_ERR_LOG("AT_SetKsNafAuthPara: SI_PIH_UiccAuthReq fail.");

        return AT_ERROR;
    }
}
#endif

