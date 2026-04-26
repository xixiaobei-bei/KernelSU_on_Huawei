/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "at_basic_cmd.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_test_para_cmd.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "at_lte_common.h"
#endif

#include "at_general_drv_set_cmd_proc.h"
#include "at_voice_taf_set_cmd_proc.h"
#include "at_voice_taf_qry_cmd_proc.h"
#include "at_data_basic_set_cmd_proc.h"
#include "at_data_basic_qry_cmd_proc.h"
#include "at_general_taf_set_cmd_proc.h"
#include "at_general_taf_qry_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_BASICCMD_C

/*
 * 示例: ^CMDX 命令是不受E5密码保护命令，且在+CLAC列举所有命令时不显示，第一个参数是不带双引号的字符串,
 *      第二个参数是带双引号的字符串，第三个参数是整数型参数
 * !!!!!!!!!!!注意: param1和param2是示例，实际定义命令时应尽量定义的简短(可提高解析效率)!!!!!!!!!!!!!
 *  {AT_CMD_CMDX,
 *  At_SetCmdxPara, AT_SET_PARA_TIME, At_QryCmdxPara, AT_QRY_PARA_TIME, At_TestCmdxPara, AT_NOT_SET_TIME,
 *  AT_ERROR, CMD_TBL_E5_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
 *  (VOS_UINT8 *)"^CMDX", (VOS_UINT8 *)"(@param1),(param2),(0-255)"},
 */

static const AT_ParCmdElement g_atBasicCmdTbl[] = {
    /* CS */
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 挂断电话
     * [说明]: 在单模式（参见+CMOD命令）下断开和远端用户的连接。多方呼叫时，连接的每个用户都会被断开。
     *         本命令遵从ITU T Recommendation V.250协议。
     *         +CHUP命令和H命令的区别：+CHUP命令是应用于多模式呼叫，不能代替H命令。
     * [语法]:
     *     [命令]: H[<value>]
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值，仅支持取值为0。
     *             取其他值时，响应结果均为ERROR。
     *             如果此命令不带<value>参数，其功能和ATH0一样（即ATH和ATH0的功能一样）。
     * [示例]:
     *     · 挂断当前所有的呼叫
     *       ATH
     *       OK
     */
    { AT_CMD_H,
      At_SetHPara, AT_DETACT_PDP_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"H", (VOS_UINT8 *)"(0)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 接听来电
     * [说明]: 接听当前的来电。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: A
     *     [结果]: <CR><LF>OK<CR><LF>
     *             接听操作失败：
     *             <CR><LF>NO CARRIER<CR><LF>
     *             其他错误：
     *             <CR><LF> ERROR<CR><LF>
     *             或者
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     * [示例]:
     *     · 接听一个来电
     *       ATA
     *       OK
     */
    { AT_CMD_A,
      At_SetAPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"A", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 发起呼叫
     * [说明]: 发起一个呼叫（该过程可以被打断）。
     *         本命令遵从ITU T Recommendation V.250协议。
     *         MT执行在TE和外部PDN（Packet Data Network）间建立通信所必需的操作，并进入数传状态。
     *         本命令遵从3GPP TS 27.007协议。
     *         不支持CUG补充业务。
     *         不支持ME存储介质的电话本，所以不支持从ME中的电话本发起呼叫。
     * [语法]:
     *     [命令]: D[<dial_string>][I][G][;]
     *     [结果]: 用号码<dial_string>发起一个呼叫。
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             对方用户忙
     *             <CR><LF>BUSY<CR><LF>
     *             无法建立连接
     *             <CR><LF>NO CARRIER<CR><LF>
     *             其它错误
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: D>mem<n>[I][G][;]
     *     [结果]: 从指定存储器mem的存储位置<n>获取被叫号码，并用该号码发起呼叫，存储器可以通过命令+CPBS=?查询。
     *     [命令]: D><n>[I][G][;]
     *     [结果]: 从当前存储器的存储位置<n>获取被叫号码，并用该号码发起呼叫。
     *     [命令]: D><name>[I][G][;]
     *     [结果]: 根据电话本中存储的姓名<name>获取被叫号码，并用该号码发起呼叫。
     *     [命令]: D*<mmi_string>[#]
     *     [结果]: 发送MMI字符串mmi_string。
     *     [命令]: D*<GPRS_SC>[*[<called_address>][*[<L2P>][*[<cid>]]]]#
     *     [结果]: 请求分组域业务。
     *             <CR><LF>CONNECT<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: D*<GPRS_SC_IP>[*<cid>]#
     *     [结果]: 请求分组域IP业务。
     *             <CR><LF>CONNECT<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <I>: 如果命令中带有I，表示在这次呼叫中禁止显示主叫号码；
     *             如果命令中带有i，表示在这次呼叫中允许显示主叫号码。
     *     <G>: 如果命令中带有G或者g，表示在这次呼叫中使能CUG补充业务。
     *     <分号“;”>: 如果命令末尾有分号，表示发起语音呼叫；
     *             如果没有分号，则表示发起数据呼叫。
     *     <dial_string>: 呼叫号码。
     *     <GPRS_SC>: GPRS服务代码，值为99。
     *     <called_address>: 目前忽略该参数。
     *     <L2P>: TE和MT之间的层二协议类型，支持以下取值，默认值为PPP协议。
     *             NULL：不使用层二协议；
     *             PPP：层二使用PPP协议。
     *     <cid>: 整型值，PDP上下文标识，如果不输入则按照默认cid激活一个PDP。
     *     <GPRS_SC_IP>: GPRS为IP提供的服务代码，值为98。
     * [示例]:
     *     · 发起语音呼叫，并在这次呼叫中禁止显示主叫号码
     *       ATD13900000000I;
     *       OK
     *     · 用电话本的第28个存储位置存储的号码发起一个语音呼叫
     *       ATD>28;
     *       OK
     *     · 请求分组域业务，拨号发起PDP激活
     *       ATD*99**PPP*1#
     *       CONNECT
     *     · 请求分组域IP业务，拨号发起PDP激活
     *       ATD*98*1#
     *       CONNECT
     */
    { AT_CMD_D,
      At_SetDPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"D", VOS_NULL_PTR },

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 命令模式切换到数据模式
     * [说明]: 该命令用于切换在线命令模式到数据模式。
     *         只用于MBB模块产品。
     * [语法]:
     *     [命令]: O
     *     [结果]: <CR><LF>CONNECT [<value >]<CR><LF>
     *             无数据连接情况：
     *             <CR><LF>NO CARRIER<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <value>: 整型值。
     *             0：命令模式切换到数据模式。
     * [示例]:
     *     · 命令模式切换到数据模式
     *       ATO
     *       CONNECT 9600
     */
    { AT_CMD_O,
      AT_SetOPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"O", (VOS_UINT8 *)"(0)" },
#endif

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置自动应答
     * [说明]: 设置自动应答。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: S0=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: S0?
     *     [结果]: <CR><LF><n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: S0=?
     *     [结果]: <CR><LF>S0: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，默认值为0。
     *             0：关闭自动应答；
     *             1～255：启动自动应答的时间，单位为s。
     * [示例]:
     *     · 开启自动应答，时间为5s
     *       ATS0=5
     *       OK
     *     · 查询自动应答设置
     *       ATS0?
     *       5
     *       OK
     *     · 测试S0
     *       ATS0=?
     *       S0: (0-255)
     *       OK
     */
    { AT_CMD_S0,
      At_SetS0Para, AT_NOT_SET_TIME, At_QryS0Para, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"S0", (VOS_UINT8 *)"(0-255)" },

    /* ATC */
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 命令行结束符
     * [说明]: 设置命令行结束符S3。
     *         输入S3对应的ASCII码值进行设置。
     *         由TE发出，表示一行命令的终止，由MT识别确认。
     *         由MT发出，参与构成返回值中结果码和响应信息的头部、尾部和结束标志。
     *         在使用S3=<value>命令来设置S3时，须使用当前的S3字符来作为此行命令的结束符，而此行命令的结果码中会立即使用新设置的S3字符，不会等到下一行命令。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: ATS3=[<value>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ATS3?
     *     [结果]: <CR><LF><value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ATS3=?
     *     [结果]: <CR><LF>S3: (list of supported <value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值，命令行结束符，默认值为13。
     *             0～127：以ASCII码值形式设置S3字符。
     * [示例]:
     *     · 设置S3
     *       ATS3=13
     *       OK
     *     · 查询S3
     *       ATS3?
     *       013
     *       OK
     *     · 测试S3
     *       ATS3=?
     *       S3: (0-127)
     *       OK
     */
    { AT_CMD_S3,
      At_SetS3Para, AT_NOT_SET_TIME, At_QryS3Para, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"S3", (VOS_UINT8 *)"(0-127)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 命令行换行符
     * [说明]: 设置命令行换行符S4。用户应该输入S4对应的ASCII码值进行设置。此字符也可由MT发出，参与构成返回值中结果码和响应信息的头部、尾部和结束标志。
     *         如果在一行命令中改变了S4字符，那么在此行命令的结果码中将立即使用新的S4字符，而不会等到下一行命令。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: ATS4=[<value>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ATS4?
     *     [结果]: <CR><LF><value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ATS4=?
     *     [结果]: <CR><LF>S4: (list of supported <value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值，命令行换行符，默认值为10。
     *             0～127：以ASCII码值形式设置S4字符。
     * [示例]:
     *     · 设置S4
     *       ATS4=10
     *       OK
     *     · 查询S4
     *       ATS4?
     *       010
     *       OK
     *     · 测试S4
     *       ATS4=?
     *       S4: (0-127)
     *       OK
     */
    { AT_CMD_S4,
      At_SetS4Para, AT_NOT_SET_TIME, At_QryS4Para, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"S4", (VOS_UINT8 *)"(0-127)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 命令行退格符
     * [说明]: 设置命令行退格符S5。用户应该输入S5对应的ASCII码值进行设置。此字符由TE发出，表示删除前一个字符，由MT识别确认。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: ATS5=[<value>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ATS5?
     *     [结果]: <CR><LF><value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ATS5=?
     *     [结果]: <CR><LF>S5: (list of supported <value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值，命令行退格符，默认值为8。
     *             0～127：以ASCII码值形式设置S5字符。
     * [示例]:
     *     · 设置S5
     *       ATS5=8
     *       OK
     *     · 查询S5
     *       ATS5?
     *       008
     *       OK
     *     · 测试S5
     *       ATS5=?
     *       S5: (0-127)
     *       OK
     */
    { AT_CMD_S5,
      At_SetS5Para, AT_NOT_SET_TIME, At_QryS5Para, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"S5", (VOS_UINT8 *)"(0-127)" },

    { AT_CMD_S6,
      At_SetS6Para, AT_NOT_SET_TIME, At_QryS6Para, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"S6", (VOS_UINT8 *)"(2-10)" },

    { AT_CMD_S7,
      At_SetS7Para, AT_NOT_SET_TIME, At_QryS7Para, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"S7", (VOS_UINT8 *)"(1-255)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 命令回显
     * [说明]: 设置MT是否回送从TE接收的字符。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: E[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <n>: 整型值，回显控制，默认值为1。
     *             0：MT不回送从TE接收的字符；
     *             1：MT回送从TE接收的字符。
     * [示例]:
     *     · 设置回显
     *       ATE1
     *       OK
     *     · 关闭回显
     *       ATE0
     *       OK
     */
    { AT_CMD_E,
      At_SetEPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"E", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置返回信息格式
     * [说明]: 设置AT命令结果码和响应信息的格式。其中，结果码有数字和字符串两种形式。
     *         本命令遵从ITU T Recommendation V.250协议。
     *         注1：<CR>表示S3字符；
     *         注2：<LF>表示S4字符。
     * [语法]:
     *     [命令]: V[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <n>: 整型值，返回信息格式，默认值为1。
     *             0：发送缩略的头部和尾部，并采用数字形式的结果码；
     *             1：发送完全的头部和尾部，并采用字符串形式的结果码。
     * [示例]:
     *     · <n>为0时
     *       ATV0
     *       0
     *     · <n>为1时
     *       ATV1
     *       OK
     *     · 测试命令
     *       ATV=?
     *       ERROR
     */
    { AT_CMD_V,
      At_SetVPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"V", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询MS所有ID信息
     * [说明]: 查询MS的相关ID信息：厂商信息（+GMI）、产品模型号（+GMM）、软件版本号（+GMR）、ESN/IMEI（+GSN）、能力列表（+GCAP）。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: I[<n>]
     *     [结果]: <CR><LF><GMI>, <GMM>, <GMR>, <GSN>, <GCAP><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，查询控制，默认值为0。
     *             0～255：查询MS所有ID信息。
     *     <GMI>: 厂商信息。
     *     <GMM>: 产品模型号。
     *     <GMR>: 软件版本号。
     *     <GSN>: ESN/IMEI。
     *     <GCAP>: 能力列表。
     * [示例]:
     *     · 查询MS所有ID信息
     *       ATI0
     *       Manufacturer: Huawei Technologies CO.,Ltd.
     *       Model: H8180
     *       Revision: TEMP_VERSION
     *       IMEI: 350447370464090
     *       +GCAP: +CGSM,+DS,+ES
     *       OK
     */
    { AT_CMD_I,
      At_SetMsIdInfo, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"I", (VOS_UINT8 *)"(0-255)" },

    { AT_CMD_T,
      At_SetTPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (TAF_UINT8 *)"T", TAF_NULL_PTR },

    { AT_CMD_P,
      At_SetPPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"P", TAF_NULL_PTR },

    { AT_CMD_X,
      At_SetXPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"X", (VOS_UINT8 *)"(0-4)" },

    { AT_CMD_Z,
      At_SetZPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"Z", (VOS_UINT8 *)"(0)" },

    { AT_CMD_Q,
      At_SetQPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"Q", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置DCD信号模式
     * [说明]: 该命令用于设置DCD信号模式。
     *         本命令只支持HSUART端口下发，其他端口下发直接返回OK。
     *         只用于MBB模块产品。
     * [语法]:
     *     [命令]: &C[value]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <value>: 整型值
     *             0：DCD信号一直保持ON
     *             1：检测到网络侧的载波信号时，DCD信号保持ON；没有检测到载波信号时，DCD信号保持OFF。
     * [示例]:
     *     · 设置DCD信号模式
     *       AT&C0
     *       OK
     */
    { AT_CMD_AMP_C,
      At_SetAmpCPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"&C", (VOS_UINT8 *)"(0-2)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置DTR信号模式
     * [说明]: 该命令用于设置在端口处于数据模式时DTR信号变化从ON到OFF时，DCE的相应处理。
     *         本命令只支持HSUART端口下发，其他端口下发直接返回OK。
     *         只用于MBB模块产品。
     * [语法]:
     *     [命令]: &D[value]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <value>: 整型值。
     *             0：DCE忽略DTR管脚信号变化；
     *             1：DCE切换至命令模式；
     *             2：DCE释放数据连接。
     * [示例]:
     *     · 设置DTR信号模式
     *       AT&D2
     *       OK
     */
    { AT_CMD_AMP_D,
      At_SetAmpDPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"&D", (VOS_UINT8 *)"(0-2)" },

#if (FEATURE_AT_HSUART == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置DSR信号模式
     * [说明]: 该命令用于设置DSR信号模式。
     *         本命令只支持HSUART端口下发，其他端口下发直接返回OK。
     *         只用于MBB模块产品。
     * [语法]:
     *     [命令]: &S[value]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <value>: 整型值
     *             0：DSR信号一直保持ON；
     *             1：通信线路连接时，DSR信号保持ON。
     * [示例]:
     *     · 设置DSR信号模式
     *       AT&S0
     *       OK
     */
    { AT_CMD_AMP_S,
      At_SetAmpSPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"&S", (VOS_UINT8 *)"(0-1)" },
#endif

    { AT_CMD_L,
      AT_SetLPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"L", (VOS_UINT8 *)"(0)" },

    { AT_CMD_M,
      AT_SetMPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"M", (VOS_UINT8 *)"(1)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 恢复出厂设置
     * [说明]: 该命令用于恢复出厂设置。对于存在APP通道的E5和闪电卡产品，恢复出厂设置操作成功后，会在APP通道主动上报^FACTORY字符串通知应用恢复出厂设置。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: &F[<value>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <value>: 整型值，默认值为0。
     *             Stick形态：值0将恢复射频参数；
     *             E5形态：值0将恢复射频参数和用户参数。
     * [示例]:
     *     · 还原成出厂设置
     *       AT&F
     *       0
     *       OK
     */
    { AT_CMD_AMP_F,
      atSetNVFactoryRestore, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"&F", (VOS_UINT8 *)"(0)" },
};

VOS_UINT32 At_RegisterBasicCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atBasicCmdTbl, sizeof(g_atBasicCmdTbl) / sizeof(g_atBasicCmdTbl[0]));
}

const AT_ParCmdElement* At_GetBasicCmdTable(VOS_VOID)
{
    return g_atBasicCmdTbl;
}

VOS_UINT32 At_GetBasicCmdNum(VOS_VOID)
{
    return sizeof(g_atBasicCmdTbl) / sizeof(g_atBasicCmdTbl[0]);
}

VOS_UINT32 At_CheckIfDCmdValidChar(VOS_UINT8 charValue)
{
    /*
     * LINUX系统下的拨号命令格式为:ATDT"*99#"，比规范的拨号命令多了引号，而引号不在
     * 27007和ITUT-V.250规范中的拨号命令合法字符集中，但为了保证LINUX拨号成功，增加
     * 引号为拨号命令的合法字符
     */
    const VOS_UINT8 dCmdValidChar[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '*', '#', '+', 'A',
        'B', 'C', ',', 'T', 'P', '!', 'W', '@', '>', 'I', 'i', 'G', 'g', '"'
    };

    VOS_UINT32 indexNum = 0;

    while (indexNum < sizeof(dCmdValidChar)) {
        if (charValue == dCmdValidChar[indexNum]) {
            return AT_SUCCESS;
        }

        indexNum++;
    }

    return AT_FAILURE;
}

VOS_UINT8* At_GetFirstBasicCmdAddr(VOS_UINT8 *data, VOS_UINT32 *len)
{
    VOS_UINT32 i           = 0;
    VOS_UINT32 j           = 0;
    VOS_UINT8 *addr        = NULL;
    VOS_UINT8 *curAddr     = NULL;
    VOS_UINT32 dCmdLen     = 0;
    VOS_UINT32 basicCmdNum = sizeof(g_atBasicCmdTbl) / sizeof(AT_ParCmdElement);

    for (i = 0; i < basicCmdNum; i++) {
        if (g_atBasicCmdTbl[i].cmdName == NULL) {
            break;
        }

        curAddr = (VOS_UINT8 *)strstr((VOS_CHAR *)data, (VOS_CHAR *)(g_atBasicCmdTbl[i].cmdName));
        if (curAddr != NULL) {
            if ((addr == NULL) || (curAddr < addr)) {
                if (g_atBasicCmdTbl[i].cmdIndex == AT_CMD_D) { /* D命令中可能有特殊字符，需要特殊处理 */
                    dCmdLen = strlen((VOS_CHAR *)curAddr);

                    /* 依次检查D命令后面的有效字符，1表示'D'字符不检测 */
                    for (j = 1; j < dCmdLen; j++) {
                        if (At_CheckIfDCmdValidChar(*(curAddr + j)) != AT_SUCCESS) {
                            /* 遇到非D命令的参数字符后退出 */
                            break;
                        }
                    }

                    *len = j;
                } else {
                    *len = strlen((VOS_CHAR *)(g_atBasicCmdTbl[i].cmdName));
                }

                addr = curAddr;
            }
        }
    }

    return addr;
}

