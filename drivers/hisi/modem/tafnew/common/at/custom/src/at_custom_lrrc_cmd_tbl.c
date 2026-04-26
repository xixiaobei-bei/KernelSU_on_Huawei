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
#include "at_custom_lrrc_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_lrrc_set_cmd_proc.h"
#include "at_custom_lrrc_qry_cmd_proc.h"
#include "at_lte_common.h"

#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_LRRC_CMD_TBL_C

static const AT_ParCmdElement g_atCustomLrrcCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置LTE空口质量信息上报
     * [说明]: 本命令用于设置VoLTE视频通话过程中LTE空口质量信息上报配置。打开和关闭^LPDCPINFORPT、^LTERLQUALINFO两个命令的主动上报。以便应用根据当前LTE的空口质量信息控制视频通话的码率，提升用户体现。
     * [语法]:
     *     [命令]: ^VTRLQUALRPT=<enable>,<threshold>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^VTRLQUALRPT=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，主动上报控制开关：
     *             0：关闭主动上报；
     *             1：开启主动上报。
     *     <threshold>: 整型值，LPDCP信息上报阈值，单位是LPDCP丢弃定时器百分比，取值范围1~100。
     * [示例]:
     *     · 开启LTE空口质量信息上报
     *       AT^VTRLQUALRPT=1,35
     *       OK
     */
    { AT_CMD_VTRLQUALRPT,
      AT_SetVtrlqualrptPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VTRLQUALRPT", (VOS_UINT8 *)"(0,1),(1-100)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 配置LTE SCell的CA参数信息
     * [说明]: 该命令用于控制LTE下的SCell的CA功能本地临时使能、去使能，并不修改平台CA的支持能力。本地临时去使能CA时需要配置CA相关A2、A4、CQI=0功能开关及Band信息、子Band频率范围等。
     * [语法]:
     *     [命令]: ^LCACFG=<enable>[,<cfg_para>,<band_num>[,<band>[,<band>[,<band>[,<band>[,<band>[,<band>[,<band>[,<band>]]]]]]]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^LCACFG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，CA功能本地临时开关：
     *             0：去使能CA功能；
     *             1：使能CA功能。
     *     <cfg_para>: 整型值，通过bit位，控制CA相关功能的开关状态，<enable>为0时下发，bit位值为0，则关闭对应功能，为1则打开对应功能，bit位与功能对应关系如下：
     *             bit 0：A2功能；
     *             bit 1：A4功能；
     *             bit 2：CQI=0功能。
     *     <band_num>: 整型值，本地临时去使能CA时，配置的Band个数，取值范围0~8，0表示去使能所有的CA SCell。
     *     <band>: 16进制字符串码流，长度为72个字符，包含Band信息及子Band频率范围（结构体码流按照小端转换）。
     * [示例]:
     *     · 本地临时去使能CA功能，打开A2和CQI=0功能
     *       AT^LCACFG=0,5,2,"2E0000017478360009F15600000000000000000000000000000000000000000000000000","300000000000000000000000000000000000000000000000000000000000000000000000"
     *       OK
     *     · 使能CA功能
     *       AT^LCACFG=1
     *       OK
     *     · 执行测试命令
     *       AT^LCACFG=?
     *       OK
     */
    { AT_CMD_LCACFG,
      AT_SetLCaCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^LCACFG", (VOS_UINT8 *)"(0,1),(0-7),(0-8),(str),(str),(str),(str),(str),(str),(str),(str)" },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { AT_CMD_LTETDDSUBFRAME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryTddSubframePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^LTETDDSUBFRAME", VOS_NULL_PTR },
    { AT_CMD_LCELLINFO,
      AT_SetLCellInfoPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LCELLINFO", (VOS_UINT8 *)"(0,1)" },
#endif
};

/* 注册LRRC定制AT命令表 */
VOS_UINT32 AT_RegisterCustomLrrcCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomLrrcCmdTbl, sizeof(g_atCustomLrrcCmdTbl) / sizeof(g_atCustomLrrcCmdTbl[0]));
}

