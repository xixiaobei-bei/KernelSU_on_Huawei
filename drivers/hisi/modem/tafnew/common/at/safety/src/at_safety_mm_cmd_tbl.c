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

#include "at_safety_mm_cmd_tbl.h"
#include "at_safety_mm_qry_cmd_proc.h"
#include "at_safety_mm_set_cmd_proc.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAFETY_MM_CMD_TBL_C

static const AT_ParCmdElement g_atSafetyMmCmdTbl[] = {
    /*
     * [类别]: 协议AT-安全管理
     * [含义]: PIN管理
     * [说明]: PIN码验证和解锁。
     *         验证时<pin>输入的是PIN码，<newpin>不用输入；
     *         解锁时<pin>输入的是PUK码，<newpin>是替换旧PIN码的新PIN码。
     *         查询命令返回PIN码的当前状态。
     *         此命令和“AT+CPIN”的区别在于此命令是对“AT+CPIN”的扩展。
     * [语法]:
     *     [命令]: ^CPIN=<pin>[,<newpin>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CPIN?
     *     [结果]: <CR><LF>^CPIN: <code>,[<times>],<puk_times>,<pin_times>,<puk2_times>,<pin2_times><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CPIN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <pin>,<newpin>: 字符串类型，长度4～8。
     *     <code>: 字符串参数（不带双引号），取值如下：
     *             READY：MT无密码输入请求；
     *             SIM PIN：(U)SIM PIN密码请求；
     *             SIM PUK：(U)SIM PUK密码请求；
     *             SIM PIN2：(U)SIM PIN2密码请求；
     *             SIM PUK2：(U)SIM PUK2密码请求。
     *     <times>: 剩余的输入次数，对于PIN和PIN2，最大输入次数是3次；对于PUK和PUK2最大输入次数是10次。如果存在密码输入请求，则<times>字段会给出当前需要校验的密码的剩余校验次数，如果不存在密码输入请求则该字段为空。
     *     <puk_times>: 剩余的PUK输入次数，最大输入次数是10次。
     *     <pin_times>: 剩余的PIN输入次数，最大输入次数是3次。
     *     <puk2_times>: 剩余的PUK2输入次数，最大输入次数是10次。
     *     <pin2_times>: 剩余的PIN2输入次数，最大输入次数是3次。
     * [示例]:
     *     · PIN验证操作
     *       AT^CPIN="1234"
     *       OK
     *     · PIN解锁操作，第一个参数是PUK码，第二个参数是新的PIN码
     *       AT^CPIN="11111111","1234"
     *       OK
     *     · 查询当前PIN码状态和剩余次数
     *       AT^CPIN?
     *       ^CPIN: SIM PIN2,3,10,3,10,3
     *       OK
     *     · 测试CPIN
     *       AT^CPIN=?
     *       OK
     */
    { AT_CMD_CPIN_2,
      At_SetCpinPara, AT_SET_PARA_TIME, At_QryCpinPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CPIN", (TAF_UINT8 *)"(@pin),(@newpin)" },

    /*
     * [类别]: 协议AT-安全管理
     * [含义]: 运营商锁
     * [说明]: 该命令主要用于对MT进行解锁，即如果发现当前使用SIM/USIM卡不是运营商指定SIM/USIM卡，那么要求用户输入unlock_code，否则将不能够使用该数据卡。
     *         每个数据卡对应一个唯一的unlock_code。在用户输入10次不正确的unlock_code后，将永久锁定该数据卡，即永远不能够使用非运营商指定的SIM/USIM卡，如果为运营商指定的SIM/USIM卡，仍然可以正常使用。
     *         如果用户在任意一次输入正确之后，密码将会被永久性清除，用户可以任意使用该数据卡，即使更换了另一张非指定卡，也不再需要重新输入密码。
     * [语法]:
     *     [命令]: ^CARDLOCK=<unlock_code>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CARDLOCK?
     *     [结果]: <CR><LF>^CARDLOCK: <status>,<times>,<operator><CR><LF><CR><LF>OK<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CARDLOCK=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <unlock_code>: 该数据卡的解锁码，长度为8的字符串，由数字[0, 9]组成。需要使用引号。
     *     <status>: 整型值，表示当前的数据卡的状态，取值1～3。
     *             1：需要用户提供<unlock_code>；
     *             2：不需要用户提供<unlock_code>；
     *             3：数据卡已经被锁定。
     *     <times>: 整型值，范围是[0, 10]，表示剩余的可以输入解锁码的次数。最大输入次数是10次。
     *     <operator>: 整型值，表示提供该CARDLOCK业务的运营商。目前只支持返回0。
     * [示例]:
     *     · 查询运营商锁状态
     *       AT^CARDLOCK?
     *       ^CARDLOCK: 1,10,0
     *       OK
     *     · 如果为非运营商指定卡,需要解锁，并且当前的对应的unlock_code为11111111
     *       AT^CARDLOCK="11111111"
     *       OK
     *     · 测试CARDLOCK
     *       AT^CARDLOCK=?
     *       OK
     */
    { AT_CMD_CARDLOCK,
      At_SetCardlockPara, AT_SET_PARA_TIME, At_QryCardlockPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CARDLOCK", (TAF_UINT8 *)"(nlock_code)" },

    /*
     * [类别]: 协议AT-安全管理
     * [含义]: PIN2管理
     * [说明]: PIN2码验证和解锁。
     *         验证时<pin2>输入的是PIN2码，<newpin2>不用输入；
     *         解锁时<pin2>输入的是PUK2码，<newpin2>是替换旧PIN2码的新PIN2码。
     *         查询命令返回PIN2码的当前状态。
     *         只有在AT+CPIN?命令或者AT^CPIN?命令返回“READY”的情况下，此命令才有正确的返回，其他情况下返回错误”Sim failure”。
     * [语法]:
     *     [命令]: ^CPIN2=<pin2>[,<newpin2>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CPIN2?
     *     [结果]: <CR><LF>^CPIN2: <code>,[<times>],<puk_times>,<pin_times>,<puk2_times>,<pin2_times><CR><LF>OK<CR><LF>
     *     [命令]: ^CPIN2=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <pin2>,<newpin2>: 字符串类型，长度4～8。
     *     <code>: 字符串参数（不带双引号），取值如下：
     *             SIM PIN2：(U)SIM PIN2密码请求；
     *             SIM PUK2：(U)SIM PUK2密码请求。
     *     <times>: 整型值，剩余的输入次数，对于PIN2，最大输入次数是3次；对于PUK2最大输入次数是10次。
     *     <puk_times>: 整型值，剩余的PUK输入次数，最大输入次数是10次。
     *     <pin_times>: 整型值，剩余的PIN输入次数，最大输入次数是3次。
     *     <puk2_times>: 整型值，剩余的PUK2输入次数，最大输入次数是10次。
     *     <pin2_times>: 整型值，剩余的PIN2输入次数，最大输入次数是3次。
     * [示例]:
     *     · PIN2验证操作
     *       AT^CPIN2="1234"
     *       OK
     *     · PIN2解锁操作，第一个参数是PUK2码，第二个参数是新的PIN2码
     *       AT^CPIN2="11111111","1234"
     *       OK
     *     · 查询当前PIN2码状态和剩余次数
     *       AT^CPIN2?
     *       ^CPIN: SIM PIN2,3,10,3,10,3
     *       OK
     *     · 测试CPIN2
     *       AT^CPIN2=?
     *       OK
     */
    { AT_CMD_CPIN2,
      At_SetCpin2Para, AT_SET_PARA_TIME, At_QryCpin2Para, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CPIN2", (TAF_UINT8 *)"(@pin),(@newpin)" },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { AT_CMD_CMLCK,
      At_SetCmlckPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCmlckPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"^CMLCK", (VOS_UINT8 *)"(\"PS\"),(0,1),(psw),(str)" },

    { AT_CMD_CPDW,
      At_SetCpdwPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"^CPDW", VOS_NULL_PTR },
#endif
};

/* 注册MM SAFETY AT命令表 */
VOS_UINT32 AT_RegisterSafetyMmCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atSafetyMmCmdTbl, sizeof(g_atSafetyMmCmdTbl) / sizeof(g_atSafetyMmCmdTbl[0]));
}

