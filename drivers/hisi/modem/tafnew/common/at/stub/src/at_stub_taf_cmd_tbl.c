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

#include "at_stub_taf_cmd_tbl.h"
#include "AtParse.h"

#include "at_stub_taf_set_cmd_proc.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_STUB_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atStubTafCmdTbl[] = {
#if (VOS_WIN32 == VOS_OS_VER)
    { AT_CMD_NVIM,
      AT_SetPsBearIsrFlgStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^PDPISRSTUB", (VOS_UINT8 *)"(5,15),(0,1),(0,1)" },

    { AT_CMD_IMSRATSTUB,
      AT_SetImsRatStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^IMSRATSTUB", (VOS_UINT8 *)"(0,1),(0,1)" },

    { AT_CMD_IMSCAPSTUB,
      AT_SetImsCapabilityStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^IMSCAPSTUB", (VOS_UINT8 *)"(0,1),(0,1),(0,1)" },

    { AT_CMD_DOMAINSTUB,
      AT_SetDomainStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^DOMAINSTUB", (VOS_UINT8 *)"(0,1,2,3),(0,1),(0,1)" },

    { AT_CMD_CIMSROAMINGSTUB,
      AT_SetCmdImsRoamingStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CIMSROAMINGSTUB", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_CREDIALSTUB,
      AT_SetCmdRedailStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CREDIALSTUB", (VOS_UINT8 *)"(0,1),(0,1),(0,1),(0,1),(0,1),(0,1)" },

    { AT_CMD_CIMSUSSDSTUB,
      AT_SetCmdImsUssdStub, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CIMSUSSDSTUB", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_NVIM,
      AT_SetNvimPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NVSTUB", (VOS_UINT8 *)"(0,1),(0-65535),(nv)" },

    { AT_CMD_USIM,
      AT_SetUsimPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^USIMWRITESTUB", (VOS_UINT8 *)"(0-65535),(content),(0-65535),(0-22)" },

    { AT_CMD_SIM,
      AT_SetSimPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^SIMWRITESTUB", (VOS_UINT8 *)"(0-65535),(content)" },

    { AT_CMD_PIDREINIT,
      AT_SetPidReinitPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^PIDREINIT", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_REFRESH_STUB,
      At_SetReadUsimStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^READUSIMSTUB", (VOS_UINT8 *)"(0,1)" },


#endif
};

/* 注册桩命令AT命令表 */
VOS_UINT32 AT_RegisterStubTafCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atStubTafCmdTbl, sizeof(g_atStubTafCmdTbl) / sizeof(g_atStubTafCmdTbl[0]));
}

