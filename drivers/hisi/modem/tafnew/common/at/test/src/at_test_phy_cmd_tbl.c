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
#include "at_test_phy_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_test_phy_set_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_PHY_CMD_TBL_C

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
static const AT_ParCmdElement g_atTestPhyCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置TxTas天线算法参数
     * [说明]: 该命令用于设置天线算法的参数。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^TXTASTEST=<RatMode>,<Para0>,<Para1>,<Para2>,<Para3>
     *     [结果]: 正确设置时：
     *             <CR><LF>OK<CR><LF>
     *             错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TXTASTEST=?
     *     [结果]: <CR><LF>^TXTASTEST: (0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <RatMode>: 接入技术，UINT32类型（0-0xFFFFFFFF）：
     *             0代表GSM
     *             1代表WCDMA
     *             2.代表LTE
     *             3代表TD_SCDMA
     *             4代表 1x
     *             5代表Hrpd
     *     <Para0>: TxSar强制档位开关UINT32类型（0-0xFFFFFFFF）：
     *             0取消TxSar强制档位的算法
     *             1执行TxSar强制档位的算法
     *     <para1>: 下行最优天线UINT32类型（0-0xFFFFFFFF）
     *     <para2>: TxSar 档位UINT32类型（0-0xFFFFFFFF）
     *     <para3>: 预留 UINT32类型（0-0xFFFFFFFF）
     * [示例]:
     *     · 设置TxTas天线算法参数
     *       AT^TXTASTEST=1,5,10,15,0
     *       OK
     *     · 测试命令
     *       AT^TXTASTEST=?
     *       ^TXTASTEST: (0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)
     *       OK
     */
    { AT_CMD_TAS_TEST,
      AT_SetTasTestCfg, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TXTASTEST", (VOS_UINT8 *)"(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询TxTas 当前档位和档位表
     * [说明]: 根据输入的参数，得到对应的tas档位参数，以及总的档位表。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^TXTASTESTQRY=
     *             <RatMode>
     *     [结果]: <CR><LF>level index: <ratmode>(para0,para1,parm2,param3)
     *             Level table: (para0,para1,parm2,param3)
     *             (para0,para1,parm2,param3)……
     *             <CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TXTASTESTQRY=?
     *     [结果]: <CR><LF>^TXTASTESTQRY: (0-4294967295)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <RAT>: 接入技术，UINT32类型（0-0xFFFFFFFF）：
     *             0代表GSM
     *             1代表WCDMA
     *             2代表LTE
     *             3代表TD_SCDMA
     *             4代表 1x
     *             5 代表Hrpd
     * [示例]:
     *     · 查询TxTas 当前档位、当前档位参数和档位表
     *       AT^TXTASTESTQRY=1
     *       level index: 1(3,128,9,5)
     *       level table:
     *       (2,19,40,8)
     *       (3,128,9,5)
     *     · 测试命令
     *       AT^TXTASTESTQRY=?
     *       ^TXTASTESTQRY: (0-4294967295)
     *       OK
     */
    { AT_CMD_TAS_TEST_QUERY,
      AT_QryTasTestCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TXTASTESTQRY", (VOS_UINT8 *)"(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置SAR场景生效参数
     * [说明]: AT命令打桩LTE PHY调用DSDS3.5/DSDA场景SAR生效。
     *         该命令暂不支持。
     * [语法]:
     *     [命令]: ^LTESARSTUB=<mode>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行设置失败时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <mode>: 取值0或1
     *             0：设置未生效
     *             1：设置生效
     * [示例]:
     *     · set命令
     *       AT^LTESARSTUB=0
     *       ERROR
     *       AT^LTESARSTUB=1
     *       OK
     */
    { AT_CMD_LTESARSTUB,
      AT_SetLteSarStubPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LTESARSTUB", (VOS_UINT8 *)"(0-1)" },
};

/* 注册PHY组件测试相关AT命令表 */
VOS_UINT32 AT_RegisterTestPhyCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atTestPhyCmdTbl, sizeof(g_atTestPhyCmdTbl) / sizeof(g_atTestPhyCmdTbl[0]));
}
#endif

