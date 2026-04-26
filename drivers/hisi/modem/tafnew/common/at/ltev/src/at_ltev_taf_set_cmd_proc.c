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
#include "at_ltev_taf_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_snd_msg.h"

#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "mbb_log_service_api.h"
#endif


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_TAF_SET_CMD_PROC_C

#define AT_FILERD_PARA_NUM    1
#define AT_FILERD_PARA_AINDEX 0 /* FILERD的第一个参数AINDEX */
#define AT_FILEWR_PARA_NUM    2
#define AT_FILEWR_PARA_AINDEX 0 /* FILEWR的第一个参数AINDEX */
#define AT_FILEWR_PARA_INFO   1 /* FILEWR的第二个参数INFO */
#define AT_SFPTEST_CMD_TIMER_LEN 500
#define AT_SETBOOTMODE_PARA_NUM                1
#define AT_SETBOOTMODE_PARA_TRACE_ONSTART_FLAG 0 /* SETBOOTMODE的第一个参数TRACE_ONSTART_FLAG */

#if (FEATURE_LTEV == FEATURE_ON)
#define AT_VMODE_PARA_DEFAULT_NUM 2

TAF_UINT32 At_SetVModePara(VOS_UINT8 indexNum)
{
    TAF_MTA_VModePwModeTypeUint8 mode;
    TAF_MTA_VModeRatTypeUint8    rat;
    TAF_MTA_Ctrl                 ctrl = {0};
    VOS_UINT32                   rst;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数必须为2 */
    if (g_atParaIndex != AT_VMODE_PARA_DEFAULT_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    rat = (TAF_MTA_VModeRatTypeUint8)g_atParaList[0].paraValue;

    if (g_atParaList[1].paraValue == 0) {
        mode = TAF_MTA_VMODE_POWEROFF;
    } else if (g_atParaList[1].paraValue == 1) {
        mode = TAF_MTA_VMODE_POWERON;
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 端口控制信息 */
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.opId     = g_atClientTab[indexNum].opId;

#if (FEATURE_MBB_CUST == FEATURE_ON)
    LogServiceParamInfo logParamInfo;
    (VOS_VOID)memset_s(&logParamInfo, sizeof(logParamInfo), 0x00, sizeof(logParamInfo));
    logParamInfo.paramLtevInfo.ltevMode = mode;
    LOG_SERVICE_EvtCollect(LOG_SERVICE_EVT_MOBILITY_LTEV_MODE_INFO, &logParamInfo);
#endif

    rst = TAF_MTA_VModeSetReq(&ctrl, rat, mode);
    if (rst == TAF_MTA_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VMODE_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

