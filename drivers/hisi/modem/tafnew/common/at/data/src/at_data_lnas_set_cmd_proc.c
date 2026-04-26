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

#include "at_data_lnas_set_cmd_proc.h"
#include "securec.h"
#include "taf_ps_api.h"
#include "taf_api.h"
#include "at_event_report.h"
#include "at_cmd_proc.h"
#include "at_data_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_LNAS_SET_CMD_PROC_C

#define AT_CGEQOS_CID 0
#define AT_CGEQOS_5QI 1
#define AT_CGEQOS_DL_GBR 2
#define AT_CGEQOS_UL_GBR 3
#define AT_CGEQOS_DL_MBR 4
#define AT_CGEQOS_UL_MBR 5

#define AT_CGEQOS_PARA_MAX_NUM 6

VOS_UINT32 atSetCgeqosPara(VOS_UINT8 indexNum)
{
    TAF_EPS_QosExt epsQosInfo;

    (VOS_VOID)memset_s(&epsQosInfo, sizeof(epsQosInfo), 0x00, sizeof(TAF_EPS_QosExt));

    if (g_atParaList[AT_CGEQOS_CID].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_CGEQOS_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    epsQosInfo.cid = (VOS_UINT8)g_atParaList[AT_CGEQOS_CID].paraValue;

    if (g_atParaList[AT_CGEQOS_5QI].paraLen != 0) {
        epsQosInfo.opQci = 1;
        epsQosInfo.qci   = (VOS_UINT8)g_atParaList[AT_CGEQOS_5QI].paraValue;
    }

    if (g_atParaList[AT_CGEQOS_DL_GBR].paraLen != 0) {
        epsQosInfo.opDlgbr = 1;
        epsQosInfo.dlgbr   = g_atParaList[AT_CGEQOS_DL_GBR].paraValue;
    }

    if (g_atParaList[AT_CGEQOS_UL_GBR].paraLen != 0) {
        epsQosInfo.opUlgbr = 1;
        epsQosInfo.ulgbr   = g_atParaList[AT_CGEQOS_UL_GBR].paraValue;
    }

    /* ULGBR和DLGBR需要同时设置才有效;不同时设置，认为无效。参考3gpp 27.007 */
    if (epsQosInfo.opDlgbr != epsQosInfo.opUlgbr) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_CGEQOS_DL_MBR].paraLen != 0) {
        epsQosInfo.opDlmbr = 1;
        epsQosInfo.dlmbr   = g_atParaList[AT_CGEQOS_DL_MBR].paraValue;
    }

    if (g_atParaList[AT_CGEQOS_UL_MBR].paraLen != 0) {
        epsQosInfo.opUlmbr = 1;
        epsQosInfo.ulmbr   = g_atParaList[AT_CGEQOS_UL_MBR].paraValue;
    }

    /* ULMBR和DLMBR需要同时设置才有效;不同时设置，认为无效。参考3gpp 27.007 */
    if (epsQosInfo.opDlmbr != epsQosInfo.opUlmbr) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_PS_SetEpsQosInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0, &epsQosInfo) !=
        VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGEQOS_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 atSetCgeqosrdpPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 cid = 0;

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen != 0) {
        cid = (VOS_UINT8)g_atParaList[0].paraValue;
    } else {
        cid = 0xff;
    }

    if (TAF_PS_GetDynamicEpsQosInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0, cid) !=
        VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGEQOSRDP_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

