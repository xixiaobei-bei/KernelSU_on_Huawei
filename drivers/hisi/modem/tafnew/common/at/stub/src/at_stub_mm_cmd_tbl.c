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

#include "at_stub_mm_cmd_tbl.h"
#include "AtParse.h"

#include "at_stub_mm_set_cmd_proc.h"
#include "at_stub_mm_qry_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_STUB_MM_CMD_TBL_C

static const AT_ParCmdElement g_atStubMmCmdTbl[] = {
#if (VOS_WIN32 == VOS_OS_VER)
    { AT_CMD_NVIM,
      AT_SetRplmnStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^RPLMNSTUB", TAF_NULL_PTR },

    { AT_CMD_NVIM,
      AT_SetTinTypeStub, AT_NOT_SET_TIME, At_QryTinTypeStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^TINTYPESTUB", (VOS_UINT8 *)"(0,1,2,3)" },

    { AT_CMD_NVIM,
      AT_SetCsUnAvailPlmnStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CSUNAVAILPLMNSTUB", (VOS_UINT8 *)"(0,1),(@oper)" },

    { AT_CMD_NVIM,
      AT_SetForbRoamTaStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^FORBROAMTASTUB", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_NVIM,
      AT_SetDisableRatPlmnStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^DISABLERATPLMNSTUB", (VOS_UINT8 *)"(0)" },

    { AT_CMD_NVIM,
      AT_SetCsgStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CSGSTUB", TAF_NULL_PTR },

    { AT_CMD_NVIM,
      AT_SetDamParaStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^DAMCFGSTUB", TAF_NULL_PTR },

    { AT_CMD_ENOS_PARASTUB,
      AT_SetEonsParaStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^EONSSTUB", (VOS_UINT8 *)"(0,1,2,3,4),(0,1,2,3,4),(0,1,2,3,4),(0,1,2,3,4),(0,1,2,3,4),(0,1)" },

    { AT_CMD_COMPARE_PLMN_SUPPORT_WILDCARD,
      AT_SetWildCardParaStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^WILDCARDSTUB", TAF_NULL_PTR },

    { AT_CMD_IMSVOICEINTERSYSLAUSTUB,
      AT_SetImsVoiceInterSysLauEnableStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CIMSVOICEINTERSYSLAUSTUB", (VOS_UINT8 *)"(0,1),(0,1),(0,1),(0,1),(0,1),(0,1)" },

    { AT_CMD_IMSVOICEMMENABLESTUB,
      AT_SetImsVoiceMMEnableStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CIMSVOICEMMENABLESTUB", (VOS_UINT8 *)"(0,1),(0,1),(0,1),(0,1),(0,1),(0,1)" },

    { AT_CMD_NVIM,
      AT_ResetNplmn, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RESETNPLMN", VOS_NULL_PTR },

    { AT_CMD_NVIM,
      AT_SetNplmn, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NPLMN", (VOS_UINT8 *)"(1-256),(str),(0-65535)" },

    { AT_CMD_DELAYBG_STUB,
      At_SetDelayBgStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^DELAYBGSTUB", (TAF_UINT8 *)"(0,1),(0-255)" },


#endif
};

/* 注册桩命令AT命令表 */
VOS_UINT32 AT_RegisterStubMmCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atStubMmCmdTbl, sizeof(g_atStubMmCmdTbl) / sizeof(g_atStubMmCmdTbl[0]));
}

