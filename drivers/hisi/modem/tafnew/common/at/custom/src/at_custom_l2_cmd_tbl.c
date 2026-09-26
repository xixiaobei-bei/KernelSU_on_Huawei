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
#include "at_custom_l2_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_l2_set_cmd_proc.h"



#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_l2_CMD_TBL_C

static const AT_ParCmdElement g_atCustomL2CmdTbl[] = {
#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: LTE L2命令配置
     * [说明]: 通过AT设置LTE L2的配置命令。
     *         说明：
     *         1、L2通过调整BSR实现上行限速。BSR与上行速率是粗粒度的映射关系，有一定浮动范围，不是精确按照指定参数限制上行速率。
     *         2、不支持限速为零，可限定的最小速率和基站实现相关，可能大于命令参数。
     *         3、为避免调用方因稳定性的各种原因，在给Modem下发了速率限制命令后，不能按时给Modem下发恢复正常的命令。因此需要各调用方在模块初始化时，下发恢复正常的命令。
     * [语法]:
     *     [命令]: ^LL2COMCFG=<cmd_type>,<para1>[,<para2>[,<para3>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LL2COMCFG?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cmd_type>: 整型值，命令类型，取值0~255。
     *             0：限制上行速率。
     *     <para1>: 整型值，对应命令类型的第1项参数，取值0~4294967295。
     *             cmd_type 0：0表示取消限制，恢复默认速率；其他值表示限制上行速率值，单位0.1Mbps。
     *     <para2>: 整型值，对应命令类型的第2项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足2个参数则不配置该参数。
     *     <para3>: 整型值，对应命令类型的第3项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足3个参数则不配置该参数。
     * [示例]:
     *     · 配置LTE上行限速，限制值为1Mbps
     *       AT^LL2COMCFG=0,10
     *       OK
     *     · 关闭LTE上行限速
     *       AT^LL2COMCFG=0,0
     *       OK
     */
    { AT_CMD_LL2COMCFG,
      AT_SetLL2ComCfgPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^LL2COMCFG", (VOS_UINT8 *)"(0-255),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: LTE L2命令查询
     * [说明]: 通过AT查询L2的命令配置情况。
     * [语法]:
     *     [命令]: ^LL2COMQRY=<cmd_type>
     *     [结果]: <CR><LF>^LL2COMQRY: <cmd_type>[,<para1>,<para2>,<para3>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LL2COMQRY?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cmd_type>: 整型值，命令类型，取值0~255。
     *             0：限制上行速率。
     *     <para1>: 整型值，对应命令类型的第1项参数，取值0~4294967295。
     *             cmd_type 0：0表示取消限制，恢复默认速率；其他值表示限制上行速率值，单位0.1Mbps。
     *     <para2>: 整型值，对应命令类型的第2项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足2个参数则不返回该参数。
     *     <para3>: 整型值，对应命令类型的第3项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足3个参数则不返回该参数。
     * [示例]:
     *     · 查询LTE上行限速值
     *       AT^LL2COMQRY=0
     *       ^LL2COMQRY: 0,10,0,0
     *       OK
     */
    { AT_CMD_LL2COMQRY,
      AT_SetLL2ComQryPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^LL2COMQRY", (VOS_UINT8 *)"(0-255)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR L2命令配置
     * [说明]: 通过AT设置NR L2的配置命令。
     *         说明：
     *         1、L2通过调整BSR实现上行限速。BSR与上行速率是粗粒度的映射关系，有一定浮动范围，不是精确按照指定参数限制上行速率。
     *         2、不支持限速为零，可限定的最小速率和基站实现相关，可能大于命令参数。
     *         3、为避免调用方因稳定性的各种原因，在给Modem下发了速率限制命令后，不能按时给Modem下发恢复正常的命令。因此需要各调用方在模块初始化时，下发恢复正常的命令。
     * [语法]:
     *     [命令]: ^NL2COMCFG=<cmd_type>,<para1>[,<para2>[,<para3>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NL2COMCFG?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cmd_type>: 整型值，命令类型，取值0~255。
     *             0：限制上行速率。
     *     <para1>: 整型值，对应命令类型的第1项参数，取值0~4294967295。
     *             cmd_type 0：0表示取消限制，恢复默认速率；其他值表示限制上行速率值，单位0.1Mbps。
     *     <para2>: 整型值，对应命令类型的第2项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足2个参数则不配置该参数。
     *             当前命令下，此参数未投入使用，无实际意义。
     *     <para3>: 整型值，对应命令类型的第3项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足3个参数则不配置该参数。
     *             当前命令下，此参数未投入使用，无实际意义。
     * [示例]:
     *     · 配置NR上行限速，限制值为1Mbps
     *       AT^NL2COMCFG=0,10
     *       OK
     *     · 关闭NR上行限速
     *       AT^NL2COMCFG=0,0
     *       OK
     */
    { AT_CMD_NL2COMCFG,
      AT_SetNL2ComCfgPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^NL2COMCFG", (VOS_UINT8 *)"(0-255),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR L2命令查询
     * [说明]: 通过AT查询NR L2的命令配置情况
     * [语法]:
     *     [命令]: ^NL2COMQRY=<cmd_type>
     *     [结果]: <CR><LF>^NL2COMQRY: <cmd_type>[,<para1>,<para2>,<para3>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NL2COMQRY?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cmd_type>: 整型值，命令类型，取值0~255。
     *             0：限制上行速率。
     *     <para1>: 整型值，对应命令类型的第1项参数，取值0~4294967295。
     *             cmd_type 0：0表示取消限制，恢复默认速率；其他值表示限制上行速率值，单位0.1Mbps。
     *     <para2>: 整型值，对应命令类型的第2项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足2个参数则不返回该参数。
     *             当前命令下，此参数未投入使用，无实际意义。
     *     <para3>: 整型值，对应命令类型的第3项参数，取值0~4294967295。
     *             说明：该参数是可选项，如果该命令类型不足3个参数则不返回该参数。
     *             当前命令下，此参数未投入使用，无实际意义。
     * [示例]:
     *     · 查询NR上行限速值
     *       AT^NL2COMQRY=0
     *       ^NL2COMQRY: 0,10,0,0
     *       OK
     */
    { AT_CMD_NL2COMQRY,
      AT_SetNL2ComQryPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^NL2COMQRY", (VOS_UINT8 *)"(0-255)" },

    { AT_CMD_NPDCP_SLEEPTHRES,
      AT_SetNPdcpSleepThresCfgPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^NPDCPALIVECFG", (VOS_UINT8 *)"(0-1),(0-4294967295)" },
#endif
#endif
};

/* 注册L2定制AT命令表 */
VOS_UINT32 AT_RegisterCustomL2CmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomL2CmdTbl, sizeof(g_atCustomL2CmdTbl) / sizeof(g_atCustomL2CmdTbl[0]));
}

