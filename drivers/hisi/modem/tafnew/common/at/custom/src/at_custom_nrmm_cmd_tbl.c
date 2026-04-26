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
#include "at_custom_nrmm_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_nrmm_set_cmd_proc.h"
#include "at_custom_nrmm_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_NRMM_CMD_TBL_C

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
static const AT_ParCmdElement g_atCustomNrmmCmdTbl[] = {
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置和查询5G接入模式选项
     * [说明]: 本命令用于设置和查询UE的5G接入模式选项。
     *         该命令仅在单板支持NR时才能使用（即当前单板支持NR能力）；
     *         目前仅支持Option3 Only、Option2 Only、Option2+3三种配置；
     *         使用完此AT命令以后重启生效。
     * [语法]:
     *     [命令]: ^C5GOPTION=<nr_sa_support_flag>,<nr_dc_mode>,<5gc_access_mode>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^C5GOPTION?
     *     [结果]: 执行查询成功时：
     *             <CR><LF>^C5GOPTION: <nr_sa_support_flag>,<nr_dc_mode>,<5gc_access_mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +C5GOPTION=?
     *     [结果]: <CR><LF>+C5GOPTION:  (list of supported <nr_sa_support_flag>s), (list of supported <nr_dc_mode>s), (list of supported <5gc_access_mode>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <nr_sa_support_flag>: 是否支持NR接入网。
     *             0：不支持NR接入网以SA工作；
     *             1：支持NR接入网以SA工作。
     *     <nr_dc_mode>: NR的DC支持模式
     *             0：不支持辅连接；
     *             1：仅仅支持ENDC；
     *             2：仅仅支持NEDC；
     *             3：ENDC和NEDC都支持。
     *     <5gc_access_mode>: 允许接入5gc的制式
     *             0：不允许接入5gc；
     *             1：仅仅允许NR接入5gc；
     *             2：仅仅允许LTE接入5gc；
     *             3：允许LTE和NR接入5gc。
     * [示例]:
     *     · 设置5G接入模式选项（OPTION 2）
     *       AT^C5GOPTION=1,0,1
     *       OK
     *     · 设置5G接入模式选项（OPTION 3）
     *       AT^C5GOPTION=0,1,0
     *       OK
     *     · 设置5G接入模式选项（OPTION 2+3）
     *       AT^C5GOPTION=1,1,1
     *       OK
     *     · 查询5G接入模式选项
     *       AT^C5GOPTION?
     *       ^C5GOPTION: 1,1,1
     *       OK
     *     · 测试5G接入模式选项
     *       AT^C5GOPTION=?
     *       ^C5GOPTION: (0,1),(0-3),(0-3)
     *       OK
     */
    { AT_CMD_C5GOPTION,
      At_Set5gOptionPara, AT_SET_PARA_TIME, At_Qry5gOptionPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^C5GOPTION", (VOS_UINT8 *)"(0,1),(0-3),(0-3)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询、设置、取消NR的Band受限名单和高频开关
     * [说明]: 该命令用于查询、设置、取消NR的Band 受限名单和高频开关功能。
     *         本命令多次操作效果可叠加；
     *         最大受限名单支持数目为10；如果超过这个数目，增加受限名单的操作将返回失败；
     *         增加受限名单操作或者关闭高频的操作如果导致UE支持的NR Band全部不可用，本次操作将失败。即不允许通过本命令将NR的所有Band都禁止掉。
     *         AT命令返回成功，表示该设置命令成功，band真正起效是异步过程（不立刻生效）。
     *         该命令仅在单板支持NR时才能使用（即当前单板支持NR能力）。
     *         该命令只能通过一个通道下发，不支持多通道下发命令。
     * [语法]:
     *     [命令]: ^NRBANDBLOCKLIST=<opmode>[,<bandnum>,[<bandlist>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NRBANDBLOCKLIST?
     *     [结果]: <CR><LF>^NRBANDBLOCKLIST: <HibandSwitch>,< bandnum >[<bandlist>]
     *             <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <opmode>: 整型值，标识本次操作类型。
     *             0：放开所有高频频段；
     *             1：禁止所有高频频段；
     *             2：将指定频段从受限名单去除；
     *             3：将指定频段加入受限名单；
     *             注意：本操作效果可叠加；高频开关和受限名单的操作各自独立。
     *     <bandnum>: 整型值，Bandlist 的数目，最大为10。
     *     <bandlist>: 字符串类型，Band列表，最多存储有10个band号，使用逗号隔开。
     *     <HibandSwitch>: 整型值，查询接口返回的高频开关状态。
     *             0：高频频段全部放开；
     *             1：高频频段全部禁止。
     * [示例]:
     *     · 设置高频开关
     *       AT^NRBANDBLOCKLIST=1
     *       OK
     *     · 设置band受限名单（下发的LIST中如果存在重复则返回失败）
     *       AT^NRBANDBLOCKLIST=3,2,"77,78"
     *       OK
     *     · 查询band受限名单
     *       AT^NRBANDBLOCKLIST?
     *       ^NRBANDBLOCKLIST: 0,2,77,78
     *       OK
     *     · 测试band受限名单
     *       AT^NRBANDBLOCKLIST=?
     *       ^NRBANDBLOCKLIST: (0-3),(1-10),(bandlist)
     *       OK
     */
    { AT_CMD_NRBANDBLOCKLIST,
      AT_SetNrBandBlockListPara, AT_SET_PARA_TIME, At_QryNrBandBlockListPara, AT_QRY_PARA_TIME, VOS_NULL_PTR,
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRBANDBLOCKLIST", (VOS_UINT8 *)"(0-3),(1-10),(bandlist)" },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
#if (FEATURE_NSSAI_AUTH == FEATURE_ON)
    { AT_CMD_C5GNSSAA,
      AT_SetC5gNssaaPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^C5GNSSAA", (VOS_UINT8 *)"(sNssaiStr),(eapMsgStr)" },
#endif
#endif
};

/* 注册NRMM定制AT命令表 */
VOS_UINT32 AT_RegisterCustomNrmmCmdTable(VOS_VOID)
{
   return AT_RegisterCmdTable(g_atCustomNrmmCmdTbl, sizeof(g_atCustomNrmmCmdTbl) / sizeof(g_atCustomNrmmCmdTbl[0]));
}
#endif

