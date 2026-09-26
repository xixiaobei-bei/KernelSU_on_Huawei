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
#include "at_custom_comm_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_comm_set_cmd_proc.h"
#include "at_custom_comm_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_COMM_CMD_TBL_C

static const AT_ParCmdElement g_atCustomCommCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 发送给底层组件通用的BOOSTER命令
     * [说明]: 发送给底层组件通用的BOOSTER命令：^COMMBOOSTER。
     *         该命令暂不支持。
     *         设置的MODULEID为RRC(4)时，是指不具体指定哪个RRC设置，由modem根据当前驻留的模式发给对应的RRC，目前只支持LTE和NR，如果当前没有驻留在LTE或者NR，则回复CME ERROR。
     * [语法]:
     *     [命令]: ^COMMBOOSTER=<TYPE>,<MODULEID>,<LEN>,<PARAM>
     *     [结果]: <type>类型为设置时：
     *             发送给组件且组件返回OK时：
     *             <CR><LF>^COMMBOOSTER: 0<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             设置发送给组件且组件返回错误时：
     *             1）组件返回错误：
     *             <CR><LF>^COMMBOOSTER: <err><CR><LF>
     *             <CR><LF>ERROR<CR><LF>
     *             2）NAS直接返回错误：
     *             <CR><LF>CME ERROR<CR><LF>
     *             <type>类型为查询时：
     *             查询成功：
     *             <CR><LF>^COMMBOOSTER: <value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             查询失败时：
     *             <CR><LF>CME ERROR<CR><LF>
     *     [命令]: ^COMMBOOSTER=?
     *     [结果]: <CR><LF>^ COMMBOOSTER: (list of supported <TYPE>s), (list of supported < MODULEID > s) , (list of supported < LEN >s),(str)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <TYPE>: 命令类型，整形，取值范围(1,2)：
     *             1：设置；
     *             2：查询。
     *     <MODULEID>: 消息接收组件，整形，取值范围(0~255)：
     *             0：ERRC；
     *             1：RTTAGENT；
     *             2：MMC；
     *             3：BASTET（需BASTET宏开启才生效）；
     *             4：RRC。
     *             注意：设置命令中，当FEATURE_MBB_CUST宏开启,该参数设置为0或者在LTE模式下设置为4，命令不支持。
     *     <LEN>: <PARAM>的长度，整形，取值范围(1~250)
     *     <PARAM>: 字符串，TLV格式的参数，T和L各2个字节，具体定义由各组件维护，设置失败的错误原因值也由各个组件各自维护
     * [示例]:
     *     · 发送给LRRC的BOOSTER设置命令
     *       AT^COMMBOOSTER=1,0,4,"01021236"
     *       ^COMMBOOSTER: 0
     *       OK
     *     · 测试命令
     *       AT^COMMBOOSTER=?
     *       ^COMMBOOSTER: (1,2),(0-255),(1-250),(str)
     *       OK
     */
    { AT_CMD_COMMBOOSTER,
      AT_SetCommBoosterPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^COMMBOOSTER", (VOS_UINT8 *)"(1,2),(0-255),(1-250),(str)" },

    { AT_CMD_SARREDUCTION,
      AT_SetSARReduction, AT_SET_PARA_TIME, AT_QrySARReduction, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SARREDUCTION", (VOS_UINT8 *)"(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询软件版本号及版本编译时间
     * [说明]: 将外部软件版本号及版本编译时间的查询结果返回给AP。
     * [语法]:
     *     [命令]: ^SWVER
     *     [结果]: <CR><LF>^SWVER: <external_sw_version>_(<build_time>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^SWVER=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <external_sw_version>: 字符串类型，外部软件版本号。
     *     <build_time>: 字符串类型，版本编译时间，格式如Oct 15 2009, 16:27:00。
     * [示例]:
     *     · 查询软件版本号及版本编译时间
     *       AT^SWVER
     *       ^SWVER: 21C10B570S007C000_(May 19 2018, 16:30:49)
     *       OK
     */
    { AT_CMD_SWVER,
      At_SetSwverPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SWVER", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置故障场景上报CHR配置信息
     * [说明]: 本命令用于设置故障场景上报CHR配置信息。
     * [语法]:
     *     [命令]: ^CHRALARMRLATCFG=<alarmid>[,<alarmiddetail>]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行查询成功时：
     *             <CR><LF>^CHRALARMRLATCFG: <alarmid detail><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR:  <err_code><CR><LF>
     *             注意：如果只有<alarmid>一个参数时，执行的是查询配置信息。
     *     [命令]: ^CHRALARMRLATCFG=?
     *     [结果]: <CR><LF>^CHRALARMRLATCFG: (list of supported <alarmid>s), (list of supported <alarmid detail>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <alarmid>: 故障场景，取值范围0~39，最多40组。
     *     <alarmid detail>: 故障告警相关性，32bit，每个bit代表一种相关性，取值范围0~ 4294967295，0代表和该相关性无关。
     *             每个bit对应的关系请查看NV135的详细说明。
     * [示例]:
     *     · 设置故障场景上报CHR配置信息
     *       AT^CHRALARMRLATCFG=0,4097
     *       OK
     *     · 查询故障场景上报CHR配置信息
     *       AT^CHRALARMRLATCFG=0
     *       ^CHRALARMRLATCFG: 4097
     *       OK
     */
    { AT_CMD_CHRALARMRLATCFG,
      AT_SetChrAlarmRlatCnfInfoPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^CHRALARMRLATCFG", (VOS_UINT8 *)"(0-39),(0-4294967295)" },

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: VOIP APN的设置查询AT命令
     * [说明]: 设置、读取、查询VOIP APN KEY内容、组数时，按行返回所有定制内容。该命令受DATALOCK数据锁保护。未输入密码，查询内容返回空（已定制组数为0）。
     * [语法]:
     *     [命令]: ^VOIPAPNKEY=<group>,<content>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^VOIPAPNKEY?
     *     [结果]: <CR><LF>^VOIPAPNKEY: <number><CR><LF><CR><LF>^VOIPAPNKEY: <group>,<content><CR><LF>[…]<CR><LF>OK<CR><LF>
     *             有相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^VOIPAPNKEY=?
     *     [结果]: ^VOIPAPNKEY=?<CR><LF>^VOIPAPNKEY: <total><CR><LF><CR><LF>OK<CR><LF>
     *             有相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <group>: 分组的index值，取值范围0~256，目前取值只能为0。
     *     <content>: 对应的VOIP APN KEY的内容，加双引号，最大长度为95。
     * [示例]:
     *     · 设置VOIP APN KEY内容
     *       AT^VOIPAPNKEY=0,"xxxx"
     *       OK              //正确返回OK，错误时返回ERROR
     */
    { AT_CMD_VOIPAPNKEY,
      AT_SetVoipApnKeyPara, AT_NOT_SET_TIME, AT_QryVoipApnKeyPara, AT_NOT_SET_TIME, AT_TestVoipApnKeyPara,
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VOIPAPNKEY", (VOS_UINT8 *)"(0-1),(@password)" },
#endif

    { AT_CMD_WIFIGLOBALMAC,
      AT_SetWifiGlobalMacPara, AT_NOT_SET_TIME, AT_QryWifiGlobalMacPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^WIFIGLOBALMAC", VOS_NULL_PTR },
};

/* 注册COMM定制AT命令表 */
VOS_UINT32 AT_RegisterCustomCommCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomCommCmdTbl, sizeof(g_atCustomCommCmdTbl) / sizeof(g_atCustomCommCmdTbl[0]));
}

