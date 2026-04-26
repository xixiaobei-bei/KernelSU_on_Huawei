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

#include "at_voice_hifi_cmd_tbl.h"
#include "at_voice_hifi_set_cmd_proc.h"
#include "at_voice_hifi_qry_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_HIFI_CMD_TBL_C

static const AT_ParCmdElement g_atVoiceHifiCmdTbl[] = {
#if (FEATURE_ECALL == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: eCall IVS触发MSD传输请求命令
     * [说明]: 本命令用于设置在eCall会话期间，此命令使IVS主动发出MSD传输请求，但是在现网情况下存在PSAP不响应状况。
     * [语法]:
     *     [命令]: ^ECLPUSH
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     * [示例]:
     *     · 在eCall会话期间，IVS触发MSD传输请求设置
     *       AT^ECLPUSH
     *       OK
     */
    { AT_CMD_ECLPUSH,
      AT_SetEclpushPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLPUSH", VOS_NULL_PTR },
#endif

    /* 支持5中形态的语音类型 */
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置语音模式
     * [说明]: 设置、查询语音模式。
     *         PC语音模式与NV项8471设置相关，不同版本的默认设置不同。
     * [语法]:
     *     [命令]: ^VMSET=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^VMSET?
     *     [结果]: <CR><LF>^VMSET: <mode>CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^VMSET=?
     *     [结果]: <CR><LF>^VMSET: (list of supported <mode>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，语音模式，取值范围（0-9）。
     *             0：手持模式
     *             1：手持免提模式
     *             2：车载免提模式（暂不支持）
     *             3：耳机模式
     *             4：蓝牙模式
     *             5：PC语音模式
     *             6：不带麦克的耳机模式
     *             7：超级免提模式
     *             8：智音通话模式
     *             9：USB设备模式
     *             10：USB音频模式
     * [示例]:
     *     · 设置蓝牙模式
     *       AT^VMSET=4
     *       OK
     *     · 查询命令
     *       AT^VMSET?
     *       ^VMSET: 4
     *       OK
     *     · 测试命令
     *       AT^VMSET=?
     *       ^VMSET: (0-10)
     *       OK
     */
    { AT_CMD_VMSET,
      At_SetVMSETPara, AT_SET_VC_PARA_TIME, At_QryVMSETPara, AT_QRY_VC_PARA_TIME, AT_TestVmsetPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^VMSET", (VOS_UINT8 *)"(0-10)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置语音环回模式命令
     * [说明]: 设置语音环回模式。
     * [语法]:
     *     [命令]: ^MODEMLOOP=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^MODEMLOOP=?
     *     [结果]: <CR><LF>^ MODEMLOOP: (list of supported <mode>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，环回模式，取值范围为0~1。
     *             0：退出环回模式
     *             1：进入环回模式
     * [示例]:
     *     · 设置进入环回模式
     *       AT^MODEMLOOP=1
     *       OK
     *     · 测试命令
     *       AT^MODEMLOOP=?
     *       ^MODEMLOOP: (0,1)
     *       OK
     */
    { AT_CMD_MODEMLOOP,
      At_SetModemLoopPara, AT_NOT_SET_TIME,
#if (FEATURE_MBB_CUST == FEATURE_ON)
      AT_QryModemLoopPara,
#else
      VOS_NULL_PTR,
#endif
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MODEMLOOP", (VOS_UINT8 *)"(0,1)" },

#if (FEATURE_MBB_CUST == FEATURE_ON)
    { AT_CMD_SMUT,
      AT_SetSpeakerMutePara, AT_SET_PARA_TIME, AT_QrySpeakerMutePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^SMUT", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_CMIC,
      At_SetCmicPara, AT_SET_PARA_TIME, At_QryCmicPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8*)"+CMIC", (VOS_UINT8*)"(1-12)" },

    { AT_CMD_ECHOEX,
      AT_SetEchoSuppressionPara, AT_SET_PARA_TIME, AT_QryEchoSuppressionPara, AT_QRY_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECHOEX", (VOS_UINT8 *)"(0,1),(0-24),(0-24),(0,1),(0-24)" },

    { AT_CMD_PCMFR,
      AT_SetPcmFrPara, AT_SET_PARA_TIME, AT_QryPcmFrPara, AT_QRY_PARA_TIME, AT_TestPcmFrPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^PCMFR", (VOS_UINT8 *)"(PCMFR)" },

    { AT_CMD_ECHOSWITCH,
      AT_SetEchoSwitchPara, AT_SET_PARA_TIME, AT_QryEchoSwitchPara, AT_QRY_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECHOSWITCH", (VOS_UINT8 *)"(0-1)" },

    { AT_CMD_CPCM,
      AT_SetCpcmPara, AT_NOT_SET_TIME, AT_QryCpcmPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CPCM", ( VOS_UINT8 *)"(0,2),(0),(0-4),(0),(0)" },
#endif

};

/* 注册taf呼叫AT命令表 */
VOS_UINT32 AT_RegisterVoiceHifiCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atVoiceHifiCmdTbl, sizeof(g_atVoiceHifiCmdTbl) / sizeof(g_atVoiceHifiCmdTbl[0]));
}

