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

#include "at_extend_cmd.h"
#include "at_cmd_proc.h"
#include "taf_tafm_remote.h"
#include "at_device_cmd.h"

#include "at_test_para_cmd.h"
#include "at_sim_pam_set_cmd_proc.h"
#include "at_sim_pam_qry_cmd_proc.h"
#include "at_custom_pam_set_cmd_proc.h"
#include "at_custom_pam_qry_cmd_proc.h"
#include "at_custom_mm_set_cmd_proc.h"
#include "at_custom_mm_qry_cmd_proc.h"
#include "at_custom_taf_set_cmd_proc.h"
#include "at_custom_taf_qry_cmd_proc.h"
#include "at_general_pam_set_cmd_proc.h"
#include "at_general_pam_qry_cmd_proc.h"
#include "at_general_lmm_set_cmd_proc.h"
#include "at_general_lmm_qry_cmd_proc.h"
#include "at_general_mm_set_cmd_proc.h"
#include "at_general_mm_qry_cmd_proc.h"
#include "at_safety_mm_qry_cmd_proc.h"
#include "at_safety_mm_set_cmd_proc.h"
#include "at_general_taf_set_cmd_proc.h"
#include "at_general_taf_qry_cmd_proc.h"
#include "at_general_drv_set_cmd_proc.h"
#include "at_data_taf_set_cmd_proc.h"
#include "at_data_taf_qry_cmd_proc.h"
#include "at_data_lnas_set_cmd_proc.h"
#include "at_data_lnas_qry_cmd_proc.h"
#include "at_data_basic_set_cmd_proc.h"
#include "at_data_basic_qry_cmd_proc.h"
#include "at_voice_hifi_set_cmd_proc.h"
#include "at_voice_hifi_qry_cmd_proc.h"
#include "at_voice_taf_set_cmd_proc.h"
#include "at_voice_taf_qry_cmd_proc.h"
#include "at_ss_taf_set_cmd_proc.h"
#include "at_ss_taf_qry_cmd_proc.h"
#include "at_sms_taf_set_cmd_proc.h"
#include "at_sms_taf_qry_cmd_proc.h"
#include "at_phone_mm_set_cmd_proc.h"
#include "at_phone_mm_qry_cmd_proc.h"
#include "at_phone_ims_set_cmd_proc.h"
#include "at_phone_ims_qry_cmd_proc.h"
#include "at_phone_taf_set_cmd_proc.h"
#include "at_phone_taf_qry_cmd_proc.h"
#include "at_cdma_qry_cmd_proc.h"
#include "at_cdma_set_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_EXTENDCMD_C

/*
 * 示例: ^CMDX 命令是不受E5密码保护命令，且在+CLAC列举所有命令时不显示，第一个参数是不带双引号的字符串,
 *      第二个参数是带双引号的字符串，第三个参数是整数型参数
 * !!!!!!!!!!!注意: param1和param2是示例，实际定义命令时应尽量定义的简短(可提高解析效率)!!!!!!!!!!!!!
 *  {AT_CMD_CMDX,
 *  At_SetCmdxPara, AT_SET_PARA_TIME, At_QryCmdxPara, AT_QRY_PARA_TIME, At_TestCmdxPara, AT_NOT_SET_TIME,
 *  AT_ERROR, CMD_TBL_E5_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
 *  (VOS_UINT8 *)"^CMDX", (VOS_UINT8 *)"(@param1),(param2),(0-255)"},
 */
static const AT_ParCmdElement g_atExtendCmdTbl[] = {
    /* PHONEBOOK */
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 设置电话本存储器
     * [说明]: 设置电话本存储器。
     *         本命令遵从3GPP TS 27.007协议。
     *         该命令在MBB模式下生效，PHONE模式不可用。
     * [语法]:
     *     [命令]: +CPBS=<storage>[,<password>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPBS?
     *     [结果]: <CR><LF>+CPBS: <storage>[,<used>,<total>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CPBS=?
     *     [结果]: <CR><LF>+CPBS: (list of supported <storage>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <storage>: 字符串类型，默认值“SM”。
     *             “SM”：(U)SIM电话本；
     *             “ON”：本机号码MSISDN；
     *             “EN”：紧急呼叫号码；
     *             “FD”：(U)SIM卡上的FDN文件。
     *     <used>: 整型值，当前存储器中已经储存的记录数。
     *     <total>: 整型值，当前存储器支持的最大记录数。
     *     <password>: 10进制数的字符串类型，电话本密码，长度4～8，目前不处理此参数。
     * [示例]:
     *     · 设置当前存储器类型
     *       AT+CPBS="SM"
     *       OK
     *     · 查询当前存储器类型、已用记录数和最大记录数
     *       AT+CPBS?
     *       +CPBS: "SM",50,100
     *       OK
     *     · 列出所有的可用存储器类型
     *       AT+CPBS=?
     *       +CPBS: ("SM","ON","EN","FD")
     *       OK
     */
    { AT_CMD_CPBS,
      At_SetCpbsPara, AT_SET_PARA_TIME, At_QryCpbsPara, AT_QRY_PARA_TIME, AT_TestCpbsPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"+CPBS", (TAF_UINT8 *)"(\"SM\",\"ON\",\"EN\",\"FD\")" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 读取电话本
     * [说明]: 读取当前存储器中位置在<index1>与<index2>之间（闭区间）的电话本记录。
     *         如果只输入<index1>，则只返回在<index1>位置的电话本记录。
     *         测试命令返回当前电话本存储器支持的<number>、<text>的最大长度。
     *         本命令遵从3GPP TS 27.007协议。
     *         注意：建议index1-index2差值不大于200，否则会概率出现查询结果返回不全问题，原因是USB缓存为256条，如果一次查询过多，PC处理不过来，会在USB缓存队列中缓存，缓存队列满后则会返回发送失败，整个上报通道没有反压机制，只能丢弃消息，导致上报结果不全。
     * [语法]:
     *     [命令]: +CPBR=<index1>[,<index2>]
     *     [结果]: <CR><LF> [+CPBR: <index1>,<number>,<type>,<text>[<CR><LF>+CPBR: <index2>,<number>,<type>,<text>]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPBR=?
     *     [结果]: <CR><LF>+CPBR: (list of supported <index>s),[<nlength>],[<tlength>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <index1>、\n<index2>: 整型值，电话号码在存储器中的位置。
     *             <index1>、<index2>取值大于零并且小于等于+CPBS?命令返回的<total>。
     *             <index1>小于等于<index2>。
     *             说明
     *             当电话本中实际存储的电话本条数小于由<index1>和<index2>确定的条数时，按照实际存储的电话本记录数显示。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <text>: 字符串类型，姓名。
     *     <nlength>: 整型值，电话号码的最大长度。
     *     <tlength>: 整型值，姓名的最大长度。
     * [示例]:
     *     · 读取记录（AT＋CPBS设置为“SM”）
     *       AT+CPBR=1,3
     *       +CPBR: 1,"12345678",129,"reinhardt",
     *       +CPBR: 2,"7654321",129,"sophia",
     *       +CPBR: 3,"111111111",129,"David"
     *       OK
     *     · 读取记录（AT+CPBS设置为“ON”）
     *       AT+CPBR=1
     *       +CPBR: 1,"+8612345678",145,"reinhardt",
     *       OK
     *     · 查询位置范围、号码和姓名的最大长度
     *       AT+CPBR=?
     *       +CPBR: (1-65535),20,100
     *       OK
     */
    { AT_CMD_CPBR2,
      At_SetCpbr2Para, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpbrPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"+CPBR", (TAF_UINT8 *)"(1-65535),(1-65535)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 写入电话本
     * [说明]: 在当前存储器中添加、修改、删除一条电话本记录。
     *         如果命令参数中只含有index，则index相应位置的电话本条目将被删除。
     *         如果index字段被省略，但参数中含有number字段，则这条电话本条目将被写到第一个空位置。如果此时没有空位置，则上报：+CME ERROR: memory full。仅当所选电话本类型为"SM"和"ON"中的一种时可进行写操作，在其余类型下执行写操作会在AT命令中返回操作不允许的错误提示。
     *         测试命令返回当前存储器中<number>、<text>的最大长度。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CPBW=[<index>][,<number>[,<type>[,<text>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPBW=?
     *     [结果]: <CR><LF>+CPBW: (list of supported <index>s),[<nlength>],(list of supported <type>s),[<tlength>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <text>: 字符串类型，姓名。
     *     <index>: 整型值，电话号码在存储器中的位置。
     *     <number>: 电话号码，字符串类型（合法字符为：“0”～“9”，“#”，“*”，“+”，“(”，“)”，“-”）。“(”，“)”，“-”三个字符无论出现在任何位置，处理程序都忽略它们，不会作为非法字符报错。“+”只能出现在号码的起始位置。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <nlength>: 整型值，电话号码的最大长度。
     *     <tlength>: 整型值，姓名的最大长度。
     * [示例]:
     *     · 修改一条记录
     *       AT+CPBW=3,"12345678",129,"reinhardt"
     *       OK
     *     · 添加一条记录
     *       AT+CPBW=,"12345678",129,"reinhardt"
     *       OK
     *     · 删除一条记录
     *       AT+CPBW=7
     *       OK
     *     · 查询存储器位置范围，号码最大长度，号码类型和姓名最大长度
     *       AT+CPBW=?
     *       +CPBW: (1-65535),20,(0-255),14
     *       OK
     */
    { AT_CMD_CPBW2,
      At_SetCpbwPara2, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpbrPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"+CPBW", (TAF_UINT8 *)"(1-65535),(number),(0-255),(text)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 查询本机号码
     * [说明]: 读取当前(U)SIM卡中所有本机号码记录，并显示。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CNUM
     *     [结果]: <CR><LF>+CNUM: [<text1],<number1>,<type1>
     *             [<CR><LF>+CNUM: [<text2>],<number2>,<type2>]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CNUM=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <text>: 字符串类型，姓名。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     * [示例]:
     *     · 读取记录
     *       AT+CNUM
     *       +CNUM: "reinhardt","12345678",129,
     *       OK
     *     · 测试CNUM
     *       AT+CNUM=?
     *       OK
     */
    { AT_CMD_CNUM,
      At_SetCnumPara, AT_QRY_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"+CNUM", TAF_NULL_PTR },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 搜索电话本
     * [说明]: 查询当前存储器中姓名与<findtext>匹配的记录。
     *         测试命令返回当前存储器支持的<number>、<text>的最大长度。
     *         本命令遵从3GPP TS 27.007协议。
     *         备注：使用AT+CPBW新增联系人，中文字符为UCS2-81格式，AT+CPBF无法支持此格式的中文搜索。
     * [语法]:
     *     [命令]: +CPBF=<findtext>
     *     [结果]: <CR><LF> [+CPBF: <index1>,<number>,<type>,<text>]
     *             <CR><LF>+CPBF: <index2>,<number>,<type>,<text>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPBF=?
     *     [结果]: <CR><LF>+CPBF: [<nlength>],[<tlength>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <findtext>,<text>: 字符串类型，关键字字符串。
     *     <index1>,<index2>: 整型值，电话号码在存储器中的位置。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <nlength>: 整型值，电话号码的最大长度。
     *     <tlength>: 整型值，姓名的最大长度。
     * [示例]:
     *     · 搜索匹配记录
     *       AT+CPBF="reinhardt"
     *       +CPBF: 3,"12345678",129,"reinhardt",
     *       +CPBF: 7,"7654321",129,"reinhardt",
     *       +CPBF: 11,"111111111",129,"reinhardt"
     *       OK
     *     · 查询记录的号码和姓名的最大长度
     *       AT+CPBF=?
     *       +CPBF: 20,100
     *       OK
     */
    { AT_CMD_CPBF,
      At_SetCpbfPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpbrPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"+CPBF", (TAF_UINT8 *)"(number),(text)" },

    /* USIM */
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 通用UICC基本信道访问
     * [说明]: 该命令用于远程应用程序中直接控制当前选择的卡槽上安装的SIM卡，TE则负责在GSM/UMTS指定的框架内处理SIM信息。
     *         与限制SIM卡接入命令CRSM相比，CSIM的定义允许TE对SIM-MT接口进行更多的控制，接口的锁定和解锁可以通过特殊的<command>值或通过解释<command>参数来自动完成。如果TE应用没有使用解锁命令（或者不发送< command >导致自动解锁），则MT可能会释放锁定。
     * [语法]:
     *     [命令]: +CSIM=<length>,<command>
     *     [结果]: 设置正确：
     *             +CSIM: <length>,<response>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF> +CME ERROR: <err><CR><LF>
     * [参数]:
     *     <length>: 整形值，<command>或<response>中发送给TE的字符长度，通常是<command>或<response>长度的两倍，取值范围4-520。
     *     <command>: 整型值，SIM访问命令：
     *             MT通过3GPP TS 51.011 [28]中的格式向SIM卡传递的命令。（16进制字符格式，参考+CSCS命令）
     *     <response>: 携带命令执行成功后上报的状态码或数据。
     *             SIM卡通过3GPP TS 51.011 [28]中的格式向MT返回的命令格式（16进制字符格式，参考+CSCS命令）
     * [示例]:
     *     · 选择OPL文件
     *       AT+CSIM=14,"00A40004026F5A"
     *       +CSIM: 4,"6134"
     *       OK
     */
    { AT_CMD_CSIM,
      At_SetCsimPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5 | CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"+CSIM", (TAF_UINT8 *)"(4-520),(cmd)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 打开逻辑通道
     * [语法]:
     *     [命令]: +CCHO=<dfname>
     *     [结果]: <CR><LF><sessionid><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CCHO=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <dfname>: 整型值，应用名称，长度1~16字节。
     *             UICC卡中所有可以被选择的应用都有一个对应的DF名称。
     *     <sessionid>: 整型值，逻辑通道号。
     * [示例]:
     *     · 打开逻辑通道
     *       AT+CCHO="A0000000871002FF86FFFF89FFFFFFFF"
     *       +CCHO: 1465853795
     *       OK
     *     · 关闭逻辑通道
     *       AT+CCHC=1465853795
     *       OK
     *     · 测试CCHO
     *       AT+CCHO=?
     *       OK
     */
    { AT_CMD_CCHO,
      At_SetCchoPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"+CCHO", (TAF_UINT8 *)"(dfname)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 打开逻辑通道
     * [说明]: 打开逻辑通道，允许传入APDU命令的P2参数。
     *         该命令暂时没有相关的协议参考，是为了支持OMAPI3.0标准。
     * [语法]:
     *     [命令]: +CCHP=<dfname><P2>
     *     [结果]: <CR><LF><sessionid><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CCHP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <dfname>: 整型值，应用名称，长度1~16字节。
     *             UICC卡中所有可以被选择的应用都有一个对应的DF名称。
     *     <P2>: 整型值，APDU命令的P2参数。取值0~255。目前支持：
     *             0x00:激活应用。
     *             0x40:终止应用。
     *             0x04:打开逻辑通道，需要带回FCP。
     *             0x0C:打开逻辑通道，没有数据带回。
     *     <sessionid>: 整型值，逻辑通道号。
     * [示例]:
     *     · 打开逻辑通道
     *       AT+CCHP="A0000000871002FF86FFFF89FFFFFFFF",4
     *       +CCHP: 2056116483
     *       OK
     *     · 关闭逻辑通道
     *       AT+CCHC=2056116483
     *       OK
     *     · 测试CCHP
     *       AT+CCHP=?
     *       OK
     */
    { AT_CMD_CCHP,
      At_SetCchpPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"+CCHP", (TAF_UINT8 *)"(dfname),(0-255)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 关闭逻辑通道
     * [说明]: 关闭逻辑通道。
     *         该命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CCHC=<sessionid>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CCHC=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <sessionid>: 整形值，逻辑通道号。
     * [示例]:
     *     · 打开逻辑通道
     *       AT+CCHO="A0000000871002FF86FFFF89FFFFFFFF"
     *       +CCHO: 1465853795
     *       OK
     *     · 关闭逻辑通道
     *       AT+CCHC=1465853795
     *       OK
     *     · 测试CCHC
     *       AT+CCHC=?
     *       OK
     */
    { AT_CMD_CCHC,
      At_SetCchcPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"+CCHC", (TAF_UINT8 *)"(0-4294967295)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 通用UICC逻辑信道访问
     * [说明]: 设置<command>中的命令到指定的UICC通道。
     *         该命令仅供终端APK测试使用。
     * [语法]:
     *     [命令]: +CGLA=<sessionid>,<length>,<command>
     *     [结果]: 正确设置时：
     *             +CGLA: <length>,<response>
     *             错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <sessionid>: UINT32类型（0-0xFFFFFFFF）:这是AT命令中的标示符，用来发送APDU命令到UICC通道
     *     <length>: UINT32类型（8-522）：后面参数<command>的长度
     *     <command>: 发送给UICC的命令（请参考3GPP TS 31.010）
     *     <response>: 返回值
     * [示例]:
     *     · 执行设置命令
     *       AT+CGLA=1,8,"INCREASE"
     *       +CGLA: 8,"INCREASE"
     */
    { AT_CMD_CGLA,
      At_SetCglaPara, AT_SET_CGLA_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"+CGLA", (TAF_UINT8 *)"(0-4294967295),(8-522),(command)" },

    /*
     * [类别]: 协议AT-安全管理
     * [含义]: PIN管理
     * [说明]: PIN码验证和解锁。
     *         验证时<pin>输入的是PIN码，<newpin>不用输入；
     *         解锁时<pin>输入的是PUK码，<newpin>是替换旧PIN码的新PIN码（解锁锁网锁卡时无需此参数）。
     *         查询命令返回PIN码的当前状态。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CPIN=<pin>[,<newpin>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPIN?
     *     [结果]: <CR><LF>+CPIN: <code><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CPIN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <pin>,<newpin>: 字符串类型，长度4～8。
     *     <code>: 字符串参数（不带双引号）。
     *             READY：MT无密码输入请求；
     *             SIM PIN：(U)SIM PIN密码请求；
     *             SIM PUK：(U)SIM PUK密码请求；
     *             SIM PIN2：(U)SIM PIN2密码请求；
     *             SIM PUK2：(U)SIM PUK2密码请求；
     *             PH-NET PIN：锁网络PIN密码请求；
     *             PH-NET PUK：锁网络PUK密码请求；
     *             PH-NETSUB PIN：锁子网PIN密码请求；
     *             PH-NETSUB PUK：锁子网PUK密码请求；
     *             PH-SP PIN：锁服务提供商PIN密码请求；
     *             PH-SP PUK：锁服务提供商PUK密码请求。
     * [示例]:
     *     · PIN验证操作
     *       AT+CPIN="1234"
     *       OK
     *     · PIN解锁操作，第一个参数是PUK码，第二个参数是新的PIN码
     *       AT+CPIN="11111111","1234"
     *       OK
     *     · 查询当前PIN码状态
     *       AT+CPIN?
     *       +CPIN: SIM PIN
     *       OK
     *     · 测试CPIN
     *       AT+CPIN=?
     *       OK
     */
    { AT_CMD_CPIN,
      At_SetCpinPara, AT_SET_PARA_TIME, At_QryCpinPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"+CPIN", (TAF_UINT8 *)"(@pin),(@newpin)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 受限(U)SIM访问
     * [说明]: 该命令用于访问(U)SIM卡。
     *         SIM卡，请参见“3GPP 51.011”的“9.2 Coding of the commands”对于命令的编码。
     *         USIM卡，请参见“ETSI TS 102.221”的“11 Commands”对于命令的编码。
     *         本命令遵从3GPP TS 27.007协议
     *         对于FEATURE_UE_UICC_MULTI_APP_SUPPORT宏打开的版本（dallas 分支是打开的，austin 是关闭的），<fileid>存在的时候<pathid>一定要存在。
     *         在FEATURE_UE_UICC_MULTI_APP_SUPPORT宏的版本上面，由于卡模块支持了多应用通道，因此该命令默认访问的是基本通道的卡文件，而基本通道的卡应用是受控于NV 4001，如果NV没有设置，那么将按照EFDIR的AID顺序进行初始化。
     * [语法]:
     *     [命令]: +CRSM=<command>[,<fileid>[,<P1>,<P2>,<P3>[,<data>[,<pathid>]]]]
     *     [结果]: <CR><LF>+CRSM: <sw1>,<sw2>[,<response>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CRSM=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <command>: 整型值，SIM访问命令：
     *             162：查询记录文件有效记录信息；
     *             176：读取二进制文件；
     *             178：读取记录内容；
     *             192：获取响应；
     *             214：更新二进制文件；
     *             220：更新记录内容；
     *             242：状态查询。
     *     <fileid>: 整型值，EF文件ID，除状态查询以外的命令均必须下发该参数。
     *     <P1>,<P2>,<P3>: 整型值，除了状态查询命令，这些参数必须指定，最大255。参见3GPP TS 51.011  9.2 Coding of the commands。
     *     <data>: 以十六进制格式的信息字段。最长514。长度为2的整数倍。
     *     <pathid>: 字符串类型，包含SIM/USIM上EF文件的路径（请参考ETSI TS 102 221）。除状态查询以外的命令均必须下发该参数。
     *     <sw1>,<sw2>: 整型值，命令执行后SIM卡返回的响应。
     *     <response>: 携带命令执行成功后上报的数据，对于二进制更新和记录更新命令，<response>不返回。
     * [示例]:
     *     · 卡状态查询
     *       AT+CRSM=242
     *       +CRSM: 90,0,"622D8202782184 0CA0000000871002FF49FF0589A503DA01038A01058B03 2F0603C60C90016083010183010C830181"
     *       OK
     *     · 读取IMSI
     *       AT+CRSM=176,28423,0,0,9,,"3F007FFF"
     *       +CRSM: 90,0,"084906202811908074"
     *       OK
     *     · 获取响应
     *       AT+CRSM=192,20272,0,0,15,,"3F007F105F3A"
     *       +CRSM: 144,0,"621E82054221004C0283024F30A5038001718A01058B036F0605800200988800"
     *       OK
     *     · 查询记录文件有效记录信息
     *       AT+CRSM=162,0x6F3B,1,4,0x1C,"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF","3F007FFF"
     *       +CRSM: 144,0,"0102030405060708090A"
     *       OK
     */
    { AT_CMD_CRSM,
      At_SetCrsmPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"+CRSM", (TAF_UINT8 *)"(0-255),(1-65535),(0-255),(0-255),(0-255),(cmd),(pathid)" },

    /* SMS CONFIG */
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置TE字符集
     * [说明]: 设置TE和MT之间的字符集，用于指定参数的编码类型（IRA、GSM、UCS2三种类型）。以下AT命令中用斜体字表示的参数，其编码类型为+CSCS设置的编码类型，默认编码类型为“IRA”。
     *         D><str>[I][G][;]
     *         +CUSD=[<n>[,<str>[,<dcs>]]]
     *         +CPBR:<index1>,<number>,<type>,<text>
     *         +CPBF=<findtext>
     *         +CPBF:<index1>,<number>,<type>,<text>
     *         +CPBW=<index>][,<number>[,<type>[,<text>]]]
     *         +CMGS=<da>[,<toda>]<CR><text><ctrl-Z/ESC>
     *         +CSCA=<sca>[,<tosca>]
     *         +CMSS=<index>[,<da>[,<toda>]]
     *         +CMGW[=<oa/da>[,<tooa/toda>[,<stat>]]]<CR><text><ctrl-Z/ESC> +CMGC=<fo>,<ct>[,<pid>[,<mn>[,<da>[,<toda>]]]]<CR><text><ctrl-Z/ESC>
     *         +CMT:<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>
     *         +CDS:<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>
     *         +CMGL:<index>,<stat>,<oa/da>,[<alpha>],[<scts>][,<tooa/toda>,
     *         <length>]<CR><LF><data>
     *         +CMGL:<index>,<stat>,<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>
     *         +CMGR:<stat>,<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>
     *         +CMGR:<stat>,<da>,[<alpha>][,<toda>,<fo>,<pid>,<dcs>,[<vp>],<sca>,<tosca>,<length>]<CR><LF><data>
     *         +CMGR:<stat>,<fo>,<ct>[,<pid>,[<mn>],[<da>],[<toda>],<length>
     *         <CR><LF><cdata>]
     *         ^CMGL:<index>,<stat>,<oa/da>,[<alpha>],[<scts>][,<tooa/toda>,
     *         <length>]<CR><LF><data>
     *         ^CMGL:<index>,<stat>,<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>
     *         ^CMGR:<stat>,<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>
     *         ^CMGR:<stat>,<da>,[<alpha>][,<toda>,<fo>,<pid>,<dcs>,[<vp>],<sca>,<tosca>,<length>]<CR><LF><data>
     *         ^CMGR:<stat>,<fo>,<ct>[,<pid>,[<mn>],[<da>],[<toda>],<length>
     *         <CR><LF><cdata>]
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CSCS=[<chset>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CSCS?
     *     [结果]: <CR><LF>+CSCS: <chset><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSCS=?
     *     [结果]: <CR><LF>+CSCS: (list of supported <chset>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <chset>: 字符串类型，字符集，默认值为“IRA”。
     *             “IRA”：ASCII码；
     *             “UCS2”：UNICODE码；
     *             “GSM”：GSM 7 bit default alphabet码。
     * [示例]:
     *     · 设置字符集为UNICODE
     *       AT+CSCS="UCS2"
     *       OK
     *     · 设置短信中心号码1234，"0031"转为0x31，即ASCII码字符"1"
     *       AT+CSCA="0031003200330034"
     *       OK
     *     · 发送中文UNICODE短信，号码是123456，内容是"ABC"
     *       AT+CMGS="003100320033003400350036"
     *       >004100420043<ctr-Z>
     *     · 设置字符集为IRA
     *       AT+CSCS="IRA"
     *       OK
     *     · 设置短信中心号码1234
     *       AT+CSCA="1234"
     *       OK
     *     · 发送英文短信，号码是123456，内容是"ABC"
     *       AT+CMGS="123456"
     *       >ABC<ctr-Z>
     *     · 查询CSCS
     *       AT+CSCS?
     *       +CSCS: "IRA"
     *       OK
     *     · 测试CSCS
     *       AT+CSCS=?
     *       +CSCS: ("IRA","UCS2","GSM")
     *       OK
     */
    { AT_CMD_CSCS,
      At_SetCscsPara, AT_NOT_SET_TIME, At_QryCscsPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"+CSCS", (TAF_UINT8 *)"(\"IRA\",\"UCS2\",\"GSM\")" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置上报错误码
     * [说明]: 设置是否启用错误码。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CMEE=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CMEE?
     *     [结果]: <CR><LF>+CMEE: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CMEE=?
     *     [结果]: <CR><LF>+CMEE: (list of supported <n>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，错误码上报格式，默认值为0。
     *             0：不启用+CME ERROR: <err>，只上报ERROR；
     *             1：启用+CME ERROR: <err>，<err>取值为错误编号方式；
     *             2：启用+CME ERROR: <err>，<err>取值为文本描述方式。
     * [示例]:
     *     · <n>为0时
     *       AT+CMEE=0
     *       OK
     *       AT+CLIP=2
     *       ERROR参数错误，仅上报ERROR
     *     · <n>为1时
     *       AT+CMEE=1
     *       OK
     *       AT+CLIP=2
     *       +CME ERROR: 50参数错误，上报错误码
     *     · <n>为2时
     *       AT+CMEE=2
     *       OK
     *       AT+CLIP=2
     *       +CME ERROR: Incorrect parameters参数错误，上报错误内容
     *     · 查询CMEE
     *       AT+CMEE?
     *       +CMEE: 2
     *       OK
     *     · 测试CMEE
     *       AT+CMEE=?
     *       +CMEE: (0-2)
     *       OK
     */
    { AT_CMD_CMEE,
      At_SetCmeePara, AT_NOT_SET_TIME, At_QryCmeePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"+CMEE", (TAF_UINT8 *)"(0-2)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询CS、PS域错误码
     * [说明]: 用于查询UE(GUL) CS域和PS域错误码。
     *         本命令遵从3GPP TS 27.007协议。
     *         本命令仅满足测试需求。
     * [语法]:
     *     [命令]: +CEER
     *     [结果]: <CR><LF>+CEER: <cs cause>,<ps cause><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CEER=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cs_cause>: 最后一次CS域呼叫错误码。整型值，详见“4.5 CS域错误码”
     *     <ps_cause>: 最后一次PS域呼叫错误码。整数值，错误码
     *             0~65535：见具体错误码
     * [示例]:
     *     · 查询CS、PS域错误码
     *       AT+CEER
     *       +CEER: 99,99
     *       OK
     *     · 测试命令
     *       AT+CEER=?
     *       OK
     */
    { AT_CMD_CEER,
      At_SetCeerPara, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (TAF_UINT8 *)"+CEER", TAF_NULL_PTR },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 文本模式下参数显示
     * [说明]: 设置文本模式下信息上报格式。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CSDH=[<show>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CSDH?
     *     [结果]: <CR><LF>+CSDH: <show><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSDH=?
     *     [结果]: <CR><LF>+CSDH: (list of supported <show>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <show>: 整型值，默认值为0。
     *             0：在短信命令+CMT、+CMGL、+CMGR结果码中不显示下述头信息：
     *             对于SMS-DELIVERs和SMS-SUBMITs类型短信，+CMT、+CMGL、+CMGR命令结果码中不显示<length>、<toda>、<tooa>和<sca>、<tosca>、<fo>、<vp>、<pid>、<dcs>。
     *             对于SMS-COMMANDs类型短信，+CMGR命令结果码中不显示<pid>、<mn>、<da>、<toda>、<length>、<cdata>。
     *             1：在短信命令+CMT、+CMGL、+CMGR结果码中显示头信息。
     * [示例]:
     *     · 设置文本参数上报格式为显示
     *       AT+CSDH=1
     *       OK
     *     · 查询文本参数上报格式
     *       AT+CSDH?
     *       +CSDH: 1
     *       OK
     *     · 测试CSDH
     *       AT+CSDH=?
     *       +CSDH: (0,1)
     *       OK
     */
    { AT_CMD_CSDH,
      At_SetCsdhPara, AT_NOT_SET_TIME, At_QryCsdhPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"+CSDH", (TAF_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 新短信通知
     * [说明]: 该命令的设置值在MT重新启动后将被清0，此时不会上报任何新短信。建议不使用AT+CNMI=0,0,0,0,0的设置方式。
     *         短信通知缓存在易失性存储器中，如果MT在发送之前关闭电源，短信则有可能丢失。所以，在<mode>=0或2时，不能使用短信直接转发（<mt>=2和3），也不能使用短信状态报告直接转发（<ds>=1）；<bm>因为BALONG只支持CBM上报，所以没有将约束应用到<BM>参数。
     *         设置新短信上报方式。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CNMI[=<mode>[,<mt>[,<bm>[,<ds>[,<bfr>]]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CNMI?
     *     [结果]: <CR><LF>+CNMI: <mode>,<mt>,<bm>,<ds>,<bfr><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CNMI=?
     *     [结果]: <CR><LF>+CNMI: (list of supported <mode>s),(list of supported <mt>s),(list of supported <bm>s),(list of supported <ds>s),(list of supported <bfr>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，短信通知方式，取值范围为0～3，目前仅支持0～2。
     *             0：将短信通知缓存在ME中，如果ME的缓存已满，则用新通知覆盖最老的通知；
     *             1：将短信通知直接发送给TE。当无法发送时（例如，处在online data模式），则丢弃通知；
     *             2：将短信通知和短信状态报告直接发送给TE。当无法发送时（例如，处在online data模式），将短信通知缓存在ME中，当可以发送时一次性发送给TE。
     *             注意：
     *             短信通知缓存在易失性存储器中，如果MT在发送之前关闭电源，短信则有可能丢失。所以，在<mode>=0或2时，不能使用短信直接转发（<mt>=2和3），也不能使用短信状态报告直接转发（<ds>=1）；<bm>因为BALONG只支持CBM上报，所以没有将约束应用到<BM>参数。
     *     <mt>: 整型值，新短信上报方式，取值范围为0～3。
     *             0：不将新短信发给TE；
     *             1：新短信通过+CMTI: <mem>,<index>方式发给TE；
     *             2：新短信通过+CMT: [<alpha>],<length><CR><LF><pdu>（PDU模式）或者+CMT: <oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>（文本模式）方式发给TE；
     *             3：在收到CLASS3短信时候，新短信通过+CMT: [<alpha>],<length><CR><LF><pdu>（PDU 模式）或者+CMT:<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>（文本模式）方式发给TE；
     *             非CLASS3短信，新短信通过+CMTI: <mem>,<index>方式发给TE。
     *     <bm>: 整型值，新广播消息的上报方式，取值范围为0、2。
     *             0：不将小区广播消息发给TE；
     *             2：新广播短信通过+CBM。
     *     <ds>: 整型值，短信状态报告的上报方式，取值范围为0～2。
     *             0：不将状态报告发给TE；
     *             1：状态报告通过+CDS: <length><CR><LF><pdu>（PDU模式）或者+CDS: <fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>（文本模式）方式发给TE；
     *             2：状态报告通过+CDSI: <mem>,<index>方式发给TE。
     *     <bfr>: 整型值，用于设置从<mode>=0模式进入<mode>=1-2模式后缓存的处理，取值范围为0～1。
     *             0：进入<mode>1-2模式后，将缓存的unsolicited result code一次性发送给TE；
     *             1：进入<mode>1-2模式后，将缓存的unsolicited result code清空。
     * [示例]:
     *     · 设置新短信上报方式为+CMTI: <mem>,<index>；状态报告为+CDSI: <mem>,<index>
     *       AT+CNMI=2,1,0,2,0
     *       OK
     *     · 查询新短信上报方式
     *       AT+CNMI?
     *       +CNMI: 2,1,0,2,0
     *       OK
     *     · 测试 CNMI
     *       AT+CNMI=?
     *       +CNMI: (0-2),(0-3),(0,2),(0-2),(0,1)
     *       OK
     */
    { AT_CMD_CNMI,
      At_SetCnmiPara, AT_SET_PARA_TIME, At_QryCnmiPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"+CNMI", (TAF_UINT8 *)"(0-2),(0-3),(0,2),(0-2),(0,1)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 控制短信链路
     * [说明]: 设置控制短信中继协议（RP）层的链接的连续性。当使能了该特性，并且网络也支持时，发送连续多条短信时RP层的链接将一直存在，而无需在每两条短信之间再进行拆链和建链的过程，从而发送的速度将快很多。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CMMS=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMMS?
     *     [结果]: <CR><LF>+CMMS: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CMMS=?
     *     [结果]: <CR><LF>+CMMS: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，禁止或使能控制短信中继协议（RP）层的链接的连续性功能，默认值为2。
     *             0：禁止；
     *             1：使能一次。当前一条发送短信的返回和后一条发送短信的间隔小于1s～5s（具体的取值取决于MT的实现），RP链接保持不断。当超过了1s～5s时，RP链接中断，并且<n>值将自动设置回0，即不再继续使能。
     *             2：一直使能。当前一条发送短信的返回和后一条发送短信的间隔小于1s～5s（具体的取值取决于MT的实现），RP链接保持不断。当超过了1s～5s时，RP链接中断，并且<n>值仍保持为2，即继续使能。
     * [示例]:
     *     · 设置控制短信中继协议（RP）层的链接的连续性功能一直使能
     *       AT+CMMS=2
     *       OK
     *     · 查询当前的使能状态
     *       AT+CMMS?
     *       +CMMS: 2
     *       OK
     *     · 测试CMMS
     *       AT+CMMS=?
     *       +CMMS: (0-2)
     *       OK
     */
    { AT_CMD_CMMS,
      At_SetCmmsPara, AT_SET_PARA_TIME, At_QryCmmsPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMMS", (VOS_UINT8 *)"(0-2)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 设置短信格式
     * [说明]: 设置短信采用的格式。格式有两种模式，由<mode>参数决定，分别是PDU模式和text模式。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CMGF=[<mode>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMGF?
     *     [结果]: <CR><LF>+CMGF: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CMGF=?
     *     [结果]: <CR><LF>+CMGF: (list of supported <mode>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，短信格式，默认值为0。
     *             0：PDU模式；
     *             1：文本模式。
     * [示例]:
     *     · 设置短信格式为文本模式
     *       AT+CMGF=1
     *       OK
     *     · 查询短信格式
     *       AT+CMGF?
     *       +CMGF: 1
     *       OK
     *     · 测试CMGF
     *       AT+CMGF=?
     *       +CMGF: (0,1)
     *       OK
     */
    { AT_CMD_CMGF,
      At_SetCmgfPara, AT_NOT_SET_TIME, At_QryCmgfPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CMGF", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 设置短信发送域
     * [说明]: 设置发送短信时的服务域或优选服务域。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CGSMS=[<service>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGSMS?
     *     [结果]: <CR><LF>+CGSMS: <service><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGSMS=?
     *     [结果]: <CR><LF>+CGSMS: (list of currently available <service>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <service>: 整型值，选择服务域，默认值为1。
     *             0：只选择PS域；
     *             1：只选择CS域；
     *             2：优先选择PS域；
     *             3：优先选择CS域。
     *             注意：
     *             为提高短信发送成功率，实际配置如下：
     *             0、2：优先选择PS域；
     *             1、3：优先选择CS域。
     * [示例]:
     *     · 设置优先选择CS域发送短信
     *       AT+CGSMS=3
     *       OK
     *     · 查询短信发送域
     *       AT+CGSMS?
     *       +CGSMS: 3
     *       OK
     *     · 测试CGSMS
     *       AT+CGSMS=?
     *       +CGSMS: (0-3)
     *       OK
     */
    { AT_CMD_CGSMS,
      At_SetCgsmsPara, AT_SET_PARA_TIME, At_QryCgsmsPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGSMS", (VOS_UINT8 *)"(0-3)" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 设置短信中心地址
     * [说明]: 设置短信中心地址。
     *         本命令遵从3GPP TS 27.005协议。
     *         在不同模式下此命令的功能如下：
     *         文本模式下，写短信和发短信命令使用该命令设置的短信中心地址。
     *         PDU模式下，仅当<pdu>参数中短信中心地址长度为0时，使用该命令设置短信中心地址。
     * [语法]:
     *     [命令]: +CSCA=<sca>[,<tosca>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CSCA?
     *     [结果]: <CR><LF>+CSCA: <sca>,<tosca><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSCA=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     * [示例]:
     *     · 设置短信中心地址
     *       AT+CSCA="13800000000"
     *       OK
     *     · 查询短信中心地址
     *       AT+CSCA?
     *       +CSCA: "13800000000",129
     *       OK
     *     · 测试CSCA
     *       AT+CSCA=?
     *       OK
     */
    { AT_CMD_CSCA,
      At_SetCscaPara, AT_SET_PARA_TIME, At_QryCscaPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CSCA", (VOS_UINT8 *)"(sca),(0-255)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 设置短信服务类型
     * [说明]: 设置短信服务类型。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CSMS=<service>
     *     [结果]: <CR><LF>+CSMS: <mt>,<mo>,<bm><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CSMS?
     *     [结果]: <CR><LF>+CSMS: <service>,<mt>,<mo>,<bm><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSMS=?
     *     [结果]: <CR><LF>+CSMS: (list of supported <service>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <service>: 整型值，消息服务类型，默认值为1。
     *             0：3G TS 23.040、3G TS 23.041 Phase 2版本；
     *             1：3G TS 23.040、3G TS 23.041 Phase 2+版本。
     *     <mt>: 整型值，指示MT是否支持接收短信。
     *             0：不支持；
     *             1：支持。
     *     <mo>: 整型值，指示MT是否支持发送短信
     *             0：不支持；
     *             1：支持。
     *     <bm>: 整型值，指示MT是否支持广播消息服务
     *             0：不支持；
     *             1：支持。
     * [示例]:
     *     · 设置短信服务类型为Phase 2+版本
     *       AT+CSMS=1
     *       +CSMS: 1,1,0
     *       OK
     *     · 查询短信服务类型
     *       AT+CSMS?
     *       +CSMS: 1,1,1,1
     *       OK
     *     · 测试CSMS
     *       AT+CSMS=?
     *       +CSMS: (0,1)
     *       OK
     */
    { AT_CMD_CSMS,
      At_SetCsmsPara, AT_SET_PARA_TIME, At_QryCsmsPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CSMS", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 设置文本模式参数
     * [说明]: 文本模式下设置发送和存储短信的相关参数，有效期有两种类型：
     *         相对时间（<vp>的取值范围为0～255）
     *         绝对时间（<vp>为字符串类型）
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CSMP=[<fo>[,<vp>[,<pid>[,<dcs>]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CSMP?
     *     [结果]: <CR><LF>+CSMP: <fo>,<vp>,<pid>,<dcs><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSMP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <fo>: TPDU的第一个字节。
     *             对于不同的消息类型，<fo>的含义如下：
     *             对于消息类型为SMS-SUBMIT类型，<fo>含义请参见表2-12。
     *             对于消息类型为SMS-COMMAND类型，<fo>含义请参见表2-14。
     *     <其他>: 请参见2.5.1 短信操作命令参数说明。
     * [表]: 消息类型为SMS-submit时<fo>含义
     *       Abbr.,   Reference,                     P（note 1）, P（note 2）, Description,
     *       TP-MTI,  TP-Message-Type-Indicator,     M,           2b,          TP层消息类型。,
     *       TP-RD,   TP-Reject-Duplicates,          M,           b,           指示SC是否需要拒收一条在短信中心有保存的短信，这条短信与保存在短信中心的短信有相同的TP-OA，TP-MR和TP-DA。,
     *       TP-VPF,  TP-Validity-Period-Format,     M,           2b,          指示TP-VP是否有效。,
     *       TP-RP,   TP-Reply-Path,                 M,           b,           指示是否请求回复路径。,
     *       TP-UDHI, TP-User-Data-Header-Indicator, O,           b,           指示TP-UD有一个头。,
     *       TP-SRR,  TP-Status-Report-Request,      O,           b,           指示是否请求短信状态报告。,
     * [表]: bit[7:0]具体排列
     *       b7,    b6,      b5,     b4,     b3,     b2,    b1,     b0,
     *       TP-RP, TP-UDHI, TP-SRR, TP-VPF, TP-VPF, TP-RD, TP-MTI, TP-MTI,
     * [表]: 消息类型为SMS-COMMAND时<fo>含义
     *       Abbr.,   Reference,                      P（note 1）, R（note 2）, Description,
     *       TP-MTI,  TP-Message-Type-Indicator,      M,           2b,          TP-消息类型。,
     *       TP-UDHI, TP-User-Data-Header-Indication, O,           b,           指示TP-UD有一个头。,
     *       TP-SRR,  TP-Status-Report-Request,       O,           b,           指示是否请求短信状态报告。,
     * [表]: bit[7:0]具体排列
     *       b7, b6,      b5,     b4, b3, b2, b1,     b0,
     *       0,  TP-UDHI, TP-SRR, 0,  0,  0,  TP-MTI, TP-MTI,
     * [示例]:
     *     · 设置有效期为相对时间格式，并且编码类型为GSM 7bit
     *       AT+CSMP=17,255,0,0
     *       OK
     *     · 设置有效期为绝对时间格式，并且编码类型为UNICODE
     *       AT+CSMP=25,"94/05/06,22:10:00+08",0,8
     *       OK
     *     · 查询文本模式参数
     *       AT+CSMP?
     *       +CSMP: 17,255,0,0
     *       OK
     *     · 测试CSMP
     *       AT+CSMP=?
     *       OK
     */
    { AT_CMD_CSMP,
      At_SetCsmpPara, AT_SET_PARA_TIME, At_QryCsmpPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CSMP", TAF_NULL_PTR },
#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 设置短信存储器
     * [说明]: 设置短信读、删、写、发和接收操作分别对应的存储器。
     *         CL模式下该命令不支持。
     *         手机形态不支持将存储介质设置为ME。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CPMS=<mem1>[,<mem2>[,<mem3>]]
     *     [结果]: <CR><LF>+CPMS: <used1>,<total1>,<used2>,<total2>,<used3>,<total3><CR><LF> <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CPMS?
     *     [结果]: <CR><LF>+CPMS: <mem1>,<used1>,<total1>,<mem2>,<used2>,<total2>,<mem3>, <used3>,<total3><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +CPMS=?
     *     [结果]: <CR><LF>+CPMS: (list of supported <mem1>s),(list of supported <mem2>s), (list of supported <mem3>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mem1>: 字符串值，表示短信读取和删除操作所用的介质。可选值为：
     *             “SM”，表示(U)SIM卡；“ME”，表示FLASH存储。
     *     <mem2>: 字符串值，表示短信写入和发送操作所用的介质。可选值同<mem1>。
     *     <mem3>: 字符串值，表示接收操作所用的介质。可选值同<mem1>。
     *     <total1>: 整数值，表示<mem1>存储短信的容量。
     *     <total2>: 整数值，表示<mem2>存储短信的容量。
     *     <total3>: 整数值，表示<mem3>存储短信的容量。
     *     <used1>: 整数值，表示<mem1>里现有的短信数目。
     *     <used2>: 整数值，表示<mem2>里现有的短信数目。
     *     <used3>: 整数值，表示<mem3>里现有的短信数目。
     * [示例]:
     *     · 设置短信操作存储器为(U)SIM
     *       AT+CPMS="SM","SM","SM"
     *       +CPMS: 0,16,0,16,0,16
     *       OK
     *     · 查询短信操作存储器
     *       AT+CPMS?
     *       +CPMS: "SM",0,16,"SM",0,16,"SM",0,16
     *       OK
     *       测试CPMS
     *       依据NV配置而不同：
     *     · 支持ME存储
     *       AT+CPMS=?
     *       +CPMS: ("SM","ME"), ("SM","ME"), ("SM","ME")
     *       OK
     *     · 不支持ME存储
     *       AT+CPMS=?
     *       +CPMS: ("SM"), ("SM"), ("SM")
     *       OK
     */
    { AT_CMD_CPMS,
      At_SetCpmsPara, AT_SET_PARA_TIME, At_QryCpmsPara, AT_QRY_PARA_TIME, AT_TestCpmsPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CPMS", (VOS_UINT8 *)"(\"SM\",\"ME\"),(\"SM\",\"ME\"),(\"SM\",\"ME\")" },
#else

    { AT_CMD_CPMS,
      At_SetCpmsPara, AT_SET_PARA_TIME, At_QryCpmsPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CPMS", (VOS_UINT8 *)"(\"SM\"),(\"SM\"),(\"SM\")" },
#endif

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 删除短信
     * [说明]: 从存储器（mem1）中删除短信，如果<delflag>存在且不为0，则忽略<index>。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CMGD=<index>[,<delflag>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMGD=?
     *     [结果]: <CR><LF>+CMGD: ([list of supported <index>s]),(list of supported <delflag>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <delflag>: 整型值，删除方式，默认为0。
     *             0：删除<index>指定的短信；
     *             1：删除当前存储器上所有的已读短信；
     *             2：删除当前存储器上所有的已读和已发短信；
     *             3：删除当前存储器上所有的已读、已发和未发短信；
     *             4：删除当前存储器上所有短信。
     *     <其他>: 请参见2.5.1 短信操作命令参数说明。
     * [示例]:
     *     · 删除当前存储器第一条短信
     *       AT+CMGD=0
     *       OK
     *     · 测试CMGD（假设当前存储区中索引为0，1，2的位置存有短信）
     *       AT+CMGD=?
     *       +CMGD: (0,1,2),(0-4)
     *       OK
     */
    { AT_CMD_CMGD,
      At_SetCmgdPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_QryCmgdPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMGD", (VOS_UINT8 *)"(0-254),(0-4)" },
#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 选择小区消息类型
     * [说明]: PC客户端用此命令设置小区广播订阅方式。
     *         CL模式下该命令不支持。
     *         本命令遵从3GPP TS 27.005协议。
     *         使用+CNMI命令的<bm>参数设置PC客户端接收消息的方法。设置为MT侧采用+CBM命令的PDU模式的主动上报消息。
     * [语法]:
     *     [命令]: +CSCB=[<mode>
     *             [,<mids>[,<dcss>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CSCB?
     *     [结果]: <CR><LF>+CSCB: <mode>,<mids>,<dcss><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSCB=?
     *     [结果]: <CR><LF>+CSCB: (list of supported <mode>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，消息类型接收方式，默认为0。
     *             0：接收<mids>和<dcss>中指定的消息类型；
     *             1：不接收<mids>和<dcss>中指定的消息类型。
     *     <mids>: 字符串类型，接收的所有消息ID列表。如："0,1,5,320-478,922"
     *     <dcss>: 字符串类型，接收的所有编码DCS列表。如："0-3,5"
     * [示例]:
     *     · 设置用户选择小区消息类型的接收方式为接收<mids>和<dcss>中指定的消息类型
     *       AT+CSCB=0
     *       OK
     *     · 查询当前用户选择小区消息类型，目前查询返回的只能是接收列表。
     *       AT+CSCB?
     *       +CSCB: 0,"0-65535",""
     *       OK
     *     · 测试CSCB命令
     *       AT+CSCB=?
     *       +CSCB: (0,1)
     *       OK
     */
    { AT_CMD_CSCB,
      At_SetCscbPara, AT_SET_PARA_TIME, At_QryCscbPara, AT_QRY_PARA_TIME, AT_TestCscbPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"+CSCB", (VOS_UINT8 *)"(0,1),(MIDS),(DCSS)" },
#endif

    /* 语音新增AT命令 */
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置听筒音量
     * [说明]: 设置通话时听筒或扬声器的音量。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CLVL=<level>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CLVL?
     *     [结果]: <CR><LF>+CLVL: <level><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CLVL=?
     *     [结果]: <CR><LF>+CLVL: (list of supported <level>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <level>: 整型值，音量的级别。
     *             0～5：0为最小音量，5为最大音量。
     * [示例]:
     *     · 设置音量级别为3
     *       AT+CLVL=3
     *       OK
     *     · 查询音量级别
     *       AT+CLVL?
     *       +CLVL: 3
     *       OK
     *     · 测试CLVL
     *       AT+CLVL=?
     *       +CLVL: (0-5)
     *       OK
     */
    { AT_CMD_CLVL,
      At_SetClvlPara, AT_SET_VC_PARA_TIME, At_QryClvlPara, AT_QRY_VC_PARA_TIME, AT_TestClvlPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CLVL", (VOS_UINT8 *)"(0-12)" },

    /* CS */
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 挂断当前所有呼叫
     * [说明]: 挂断当前所有的呼叫。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CHUP
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: +CHUP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     * [示例]:
     *     · 挂断当前所有的呼叫
     *       AT+CHUP
     *       OK
     *     · 测试CHUP
     *       AT+CHUP=?
     *       OK
     */
    { AT_CMD_CHUP,
      At_SetChupPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CHUP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 列出当前呼叫信息
     * [说明]: 列出当前所有的呼叫。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CLCC
     *     [结果]: <CR><LF> [+CLCC: <id1>,<dir>,<state>,<mode>,<mpty>[,<number>
     *             ,<type>[,<alpha>[,<priority>[,<CLI validity>]]]]
     *             [<CR><LF>+CLCC: < id2>,<dir>,<stat>,<mode>,<mpty>[,
     *             <number>,<type>[,<alpha>[,<priority>[,<CLI validity>]]]] <CR><LF>[...]]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CLCC=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <idx>: 整型值，呼叫标识。取值范围1~7。
     *     <dir>: 整型值，呼叫方向。
     *             0：主叫；
     *             1：被叫。
     *     <state>: 整型值，呼叫状态。
     *             0：激活状态；
     *             1：呼叫保持状态；
     *             2：主叫，拨号状态；
     *             3：主叫，回铃音状态；
     *             4：被叫，来电状态；
     *             5：被叫，呼叫等待状态。
     *     <mode>: 整型值，呼叫模式。
     *             0：语音呼叫；
     *             1：数据呼叫；
     *             2：传真。
     *     <mpty>: 整型值，多方通话状态。X模modem不负责识别三方通话，X模在CLCC查询信息中固定填写非3方通话。
     *             0：不在多方通话中；
     *             1：在多方通话中。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <alpha>: 号码在电话本中对应的姓名，不支持。
     *     <priority>: 整型值，表示呼叫的eMLPP优先级，暂不支持
     *     <CLI validity>: CLI有效性，暂不支持，
     *             1：CLI有效；
     *             2：发起人已保留CLI；
     *             3：由于主叫方为付费电话类型，因此无法使用CLI；
     *             4：其他原因，CLI不可用。
     * [示例]:
     *     · 显示当前呼叫信息
     *       AT+CLCC?
     *       +CLCC: 1,0,0,0,0,0,0,0,"18888888888",129
     *       OK
     *     · 测试CLCC
     *       AT+CLCC=?
     *       OK
     */
    { AT_CMD_CLCC,
      At_SetClccPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CLCC", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询MT当前状态
     * [说明]: 查询MT当前状态。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CPAS
     *     [结果]: <CR><LF>+CPAS: <pas><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPAS=?
     *     [结果]: <CR><LF>+CPAS: (list of supported <pas>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <pas>: 整型值，MT的活动状态。
     *             0：就绪（TE可以向MT发出命令）；
     *             1：不可用（MT拒绝TE的命令）；
     *             2：未知（MT不保证对命令作出响应）；
     *             3：响铃（正在响铃，但是MT可以执行TE的命令）；
     *             4：呼叫进行中（有呼叫正在进行，但是MT可以执行TE的命令）；
     *             5：休眠（休眠状态，MT不能处理TE的命令）。
     * [示例]:
     *     · 获取MT当前状态
     *       AT+CPAS
     *       +CPAS: 0
     *       OK
     *     · 测试CPAS
     *       AT+CPAS=?
     *       +CPAS: (0-5)
     *       OK
     */
    { AT_CMD_CPAS,
      At_SetCpasPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpasPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CPAS", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置承载类型
     * [说明]: 设置承载类型。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CBST=[<speed>[,<name>[,<ce>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: +CBST?
     *     [结果]: <CR><LF>+CBST: <speed>,<name>,<ce><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CBST=?
     *     [结果]: <CR><LF>+CBST: (list of supported <speed>s),(list of supported <name>s),(list of supported <ce>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <speed>: 整型值，速率。
     *             134：64000 bps (multimedia)。
     *     <name>: 整型值，业务类型。
     *             1：data circuit synchronous (UDI or 3.1 kHz modem)。
     *     <ce>: 整型值，传输类型。
     *             0：transparent。
     * [示例]:
     *     · 设置成可视电话类型
     *       AT+CBST=134,1,0
     *       OK
     *     · 查询承载类型
     *       AT+CBST?
     *       +CBST: 134,1,0
     *       OK
     *     · 测试CBST
     *       AT+CBST=?
     *       +CBST: (134),(1),(0)
     *       OK
     */
    { AT_CMD_CBST,
      At_SetCbstPara, AT_NOT_SET_TIME, At_QryCbstPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CBST", (VOS_UINT8 *)"(134),(1),(0)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置呼叫模式
     * [说明]: 设置呼叫模式。
     *         本命令遵从3GPP TS 27.007协议。
     *         呼叫模式分为单一和交替两种模式：
     *         单一模式是指在一个呼叫中只能支持一种基本业务（语音或传真业务的一种）。
     *         交替模式是指在一个呼叫中可以支持两种基本业务（语音和传真业务交替进行）。
     *         目前仅支持单一模式呼叫。
     * [语法]:
     *     [命令]: +CMOD=[<mode>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CMOD?
     *     [结果]: <CR><LF>+CMOD: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CMOD=?
     *     [结果]: <CR><LF>+CMOD: (list of supported <mode>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，呼叫模式，默认值为0。
     *             0：single mode。
     * [示例]:
     *     · 设置呼叫模式为单一模式
     *       AT+CMOD=0
     *       OK
     *     · 查询呼叫模式
     *       AT+CMOD?
     *       +CMOD: 0
     *       OK
     *     · 测试CMOD
     *       AT+CMOD=?
     *       +CMOD: (0)
     *       OK
     */
    { AT_CMD_CMOD,
      At_SetCmodPara, AT_NOT_SET_TIME, At_QryCmodPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMOD", (VOS_UINT8 *)"(0)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 选择地址类型
     * [说明]: 选择发起呼叫时的被叫号码的号码类型。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CSTA=<type>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: +CSTA?
     *     [结果]: <CR><LF>+CSTA: <type><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSTA=?
     *     [结果]: <CR><LF>+CSTA: (list of supported <type>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     * [示例]:
     *     · 设置地址类型
     *       AT+CSTA=129
     *       OK
     *     · 查询地址类型
     *       AT+CSTA?
     *       +CSTA: 129
     *       OK
     *     · 测试地址类型
     *       AT+CSTA=?
     *       +CSTA: (0-255)
     *       OK
     *     · 不带参数
     *       AT+CSTA=
     *       +CME ERROR: Incorrect parameters
     */
    { AT_CMD_CSTA,
      At_SetCstaPara, AT_NOT_SET_TIME, At_QryCstaPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CSTA", (VOS_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 通话中发送DTMF音
     * [说明]: 该命令用于通话中向网侧发送DTMF（Dual Tone Multi-Frequency）音。当FEATURE_MBB_CUST宏打开时可打断向网侧发送DTMF音。
     *         本命令遵从3GPP TS 27.007协议。
     *         此命令暂不提供给应用使用。
     * [语法]:
     *     [命令]: +VTS=<DTMF>[,<duration>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +VTS=?
     *     [结果]: <CR><LF>+VTS: (0-9,A-D,*,#),(1-255)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <DTMF>: 需要发出的一个ASCII字符，取值范围为0～9，#，*，A～D。
     *     <duration>: 设置DTMF持续时长，取值范围为1-255。
     *             不带<duration>参数默认时长100ms；
     *             带有<duration>参数时长为duration * 100ms。
     * [示例]:
     *     · 发出DTMF
     *       AT+VTS=A
     *       OK
     *     · 测试VTS
     *       AT+VTS=?
     *       +VTS: (0-9,A-D,*,#),(1-255)
     *       OK
     */
    { AT_CMD_VTS,
      At_SetVtsPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestVtsPara, AT_NOT_SET_TIME,
      AT_AbortVtsPara, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+VTS", (VOS_UINT8 *)"(@key),(1-255)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 使能/禁止来电指示扩展上报格式
     * [说明]: 选择呼入指示或网络发起的PDP（Packet Data Protocol）激活请求时，是否使能扩展上报格式。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CRC=[<mode>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CRC?
     *     [结果]: <CR><LF>+CRC: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CRC=?
     *     [结果]: <CR><LF>+CRC: (list of supported <mode>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，来电或激活指示上报格式，默认值为0。
     *             0：禁止扩展格式，使用RING上报；
     *             1：使能扩展格式，使用+CRING: <type>上报。
     * [示例]:
     *     · 使能来电的扩展格式
     *       AT+CRC=1
     *       OK
     *     · 查询CRC
     *       AT+CRC?
     *       +CRC: 0
     *       OK
     *     · 测试CRC
     *       AT+CRC=?
     *       +CRC: (0,1)
     *       OK
     */
    { AT_CMD_CRC,
      At_SetCrcPara, AT_NOT_SET_TIME, At_QryCrcPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CRC", (VOS_UINT8 *)"(0,1)" },
#if (FEATURE_MBB_CUST== FEATURE_ON)
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主叫号码显示
     * [说明]: 使能或禁止主叫号码的显示。如果使能主叫号码显示并且网络允许，则来电时会上报主叫号码。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CLIP=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CLIP?
     *     [结果]: <CR><LF>+CLIP: <n>,<m><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CLIP=?
     *     [结果]: <CR><LF>+CLIP: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，禁止或使能+CLIP的主动上报，默认值为0。
     *             0：禁止；
     *             1：使能。
     *     <m>: 整型值，CLIP业务在网络的签约状态。
     *             0：CLIP业务未提供；
     *             1：CLIP业务已提供；
     *             2：未知（网络原因）。
     * [示例]:
     *     · 使能主叫号码显示
     *       AT+CLIP=1
     *       OK
     *     · 查询主叫号码显示
     *       AT+CLIP?
     *       +CLIP: 1,2
     *       OK
     *     · 测试CLIP
     *       AT+CLIP=?
     *       +CLIP: (0,1)
     *       OK
     */
    { AT_CMD_CLIP,
      At_SetClipPara, AT_NOT_SET_TIME, At_QryClipPara, AT_QRY_SS_PARA_TIME, VOS_NULL_PTR,
      AT_NOT_SET_TIME, AT_AbortClipPara, AT_NOT_SET_TIME, AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CLIP", (VOS_UINT8 *)"(0,1)"
    },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主叫号码限制
     * [说明]: 使能或禁止主叫号码限制。如果使能主叫号码限制并且网络允许，则发起呼叫时禁止向被叫方显示主叫号码。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CLIR=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CLIR?
     *     [结果]: <CR><LF>+CLIR: <n>,<m><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CLIR=?
     *     [结果]: <CR><LF>+CLIR: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，禁止或使能+CLIR功能，默认值为0。
     *             0：根据签约CLIR的服务来显示；
     *             1：使能；
     *             2：禁止。
     *     <m>: 整型值，CLIR业务在网络的签约状态。
     *             0：CLIR业务未提供；
     *             1：CLIR业务以永久模式提供；
     *             2：未知（网络原因）；
     *             3：CLIR业务临时限制；
     *             4：CLIR业务临时允许。
     * [示例]:
     *     · 使能主叫号码限制
     *       AT+CLIR=1
     *       OK
     *     · 查询主叫号码限制
     *       AT+CLIR?
     *       +CLIR: 1,1
     *       OK
     *     · 测试CLIR
     *       AT+CLIR=?
     *       +CLIR: (0,1,2)
     *       OK
     */
    { AT_CMD_CLIR,
      At_SetClirPara, AT_NOT_SET_TIME, At_QryClirPara, AT_QRY_SS_PARA_TIME, VOS_NULL_PTR,
      AT_NOT_SET_TIME, AT_AbortClirPara, AT_NOT_SET_TIME, AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CLIR", (VOS_UINT8 *)"(0,1,2)"
    },
#else
    { AT_CMD_CLIP,
      At_SetClipPara, AT_NOT_SET_TIME, At_QryClipPara, AT_QRY_SS_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CLIP", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_CLIR,
      At_SetClirPara, AT_NOT_SET_TIME, At_QryClirPara, AT_QRY_SS_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CLIR", (VOS_UINT8 *)"(0,1,2)" },
#endif
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 连接号码显示
     * [说明]: 使能或禁止连接号码的显示。如果使能连接号码显示并且网络允许，则发起主叫后在连接建立完成时显示实际连接的对方号码。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +COLP=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +COLP?
     *     [结果]: <CR><LF>+COLP: <n>,<m><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +COLP=?
     *     [结果]: <CR><LF>+COLP: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，禁止或使能+COLP的主动上报，默认值为0。
     *             0：禁止；
     *             1：使能。
     *     <m>: 整型值，显示COLP业务在网络的签约状态。
     *             0：COLP业务未提供；
     *             1：COLP业务已提供；
     *             2：未知（网络原因）。
     * [示例]:
     *     · 使能连接号码显示
     *       AT+COLP=1
     *       OK
     *     · 查询连接号码显示
     *       AT+COLP?
     *       +COLP: 1,1
     *       OK
     *     · 测试COLP
     *       AT+COLP=?
     *       +COLP: (0,1)
     *       OK
     */
    { AT_CMD_COLP,
      At_SetColpPara, AT_NOT_SET_TIME, At_QryColpPara, AT_QRY_SS_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+COLP", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 闭合用户群
     * [说明]: 管理闭合用户群（CUG）补充业务。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CCUG=[<n>[,<index>[,<info>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CCUG?
     *     [结果]: <CR><LF>+CCUG: <n>,<index>,<info><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CCUG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，禁止或使能CUG模式，默认值为0。
     *             0：禁止CUG模式；
     *             1：使能CUG模式。
     *     <index>: 整型值，闭合用户群的索引值，默认值为0。
     *             0～9：群组的索引值；
     *             10：无索引（用户在网侧签约的首选的群组）。
     *     <info>: 整型值，默认值为0。
     *             0：无信息；
     *             1：禁止OA（Outgoing Access）；
     *             2：禁止首选群组；
     *             3：禁止OA和首选群组。
     * [示例]:
     *     · 设置闭合用户群业务
     *       AT+CCUG=1,2,2
     *       OK
     *     · 查询闭合用户群业务
     *       AT+CCUG?
     *       +CCUG: 1,2,2
     *       OK
     *     · 测试CCUG
     *       AT+CCUG=?
     *       OK
     */
    { AT_CMD_CCUG,
      At_SetCcugPara, AT_NOT_SET_TIME, At_QryCcugPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CCUG", (VOS_UINT8 *)"(0,1),(0-10),(0-3)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 补充业务通知
     * [说明]: 使能或禁止补充业务的通知，补充业务通知包括+CSSI和+CSSU两种。
     *         +CSSI是指在发起主叫的过程中收到的通知。
     *         +CSSU是指在被叫时或通话过程中收到的通知。
     *         CL模式下该命令不支持。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CSSN=[<n>[,<m>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CSSN?
     *     [结果]: <CR><LF>+CSSN: <n>,<m><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CSSN=?
     *     [结果]: <CR><LF>+CSSN: (list of supported <n>s),( list of supported <m>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，禁止或使能+CSSI的补充业务通知，默认值为0。
     *             0：禁止；
     *             1：使能。
     *     <m>: 整型值，禁止或使能+CSSU的补充业务通知，默认值为0。
     *             0：禁止；
     *             1：使能。
     * [示例]:
     *     · 使能CSSI和CSSU上报
     *       AT+CSSN=1,1
     *       OK
     *     · 查询CSSI和CSSU上报
     *       AT+CSSN?
     *       +CSSN: 1,1
     *       OK
     *     · 测试CSSN
     *       AT+CSSN=?
     *       +CSSN: (0,1),(0,1)
     *       OK
     */
    { AT_CMD_CSSN,
      At_SetCssnPara, AT_SET_PARA_TIME, At_QryCssnPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CSSN", (VOS_UINT8 *)"(0,1),(0,1)" },
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 呼叫相关补充业务
     * [说明]: 通过这个命令可以控制下列补充业务：
     *         呼叫保持和恢复
     *         多方通话
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CHLD=<n>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CHLD=?
     *     [结果]: <CR><LF>+CHLD: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，操作码。
     *             0：挂断所有被保持的或者等待的呼叫；
     *             1：挂断正在通话的呼叫，并接听等待的呼叫或者恢复被保持的呼叫；
     *             2：保持正在通话的呼叫，并接听等待的呼叫或者恢复被保持的呼叫；
     *             3：建立多方通话；
     *             1x：挂断第x个呼叫；
     *             2x：保持除第x个呼叫以外的所有其他呼叫；
     *             4：将保持的呼叫和当前的来电建立通话，主叫方结束呼叫，或将正在通话的呼叫和等待的呼叫建立通话，主叫方结束呼叫；
     *             5：激活或接听网络发起的呼叫（CCBS）。
     *             说明
     *             这个序号一直保持直到呼叫释放，新的呼叫又会使用当前callid+1的序号。
     *             在保持的呼叫和等待的呼叫同时存在的情况下，当发生冲突的时候，上述过程只对等待的呼叫起作用。
     * [示例]:
     *     · 查询所有支持的参数<n>
     *       AT+CHLD=?
     *       +CHLD: (0,1,1x,2,2x,3,4,5)
     *       OK
     *     · 保持除第3个呼叫以外的所有其他呼叫
     *       AT+CHLD=23
     *       OK
     */
    { AT_CMD_CHLD,
      At_SetChldPara, AT_SET_CALL_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestChldPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CHLD", (VOS_UINT8 *)"(0-5,11-19,21-29)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 用户信令服务1
     * [说明]: 该命令用于控制用户信令服务1（UUS1），具体可参考协议3G 22.087。
     *         消息类型和UUIE同时存在，表明需要激活该消息类型的用户信令服务1。
     *         仅设置消息类型，UUIE信息不存在，表明需要去激活该消息类型的用户信令服务1。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CUUS1=[<n>[,<m>[,<message>[,<UUIE>
     *             [,<message>[,<UUIE>[,...]]]]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CUUS1?
     *     [结果]: <CR><LF>+ CUUS1: <n>,<m>[,<message>,<UUIE>
     *             [,<message>,<UUIE>[,...]]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CUUS1=?
     *     [结果]: <CR><LF>+ CUUS1: (list of supported <n>s), (list of supported <m>s), (list of supported <message>s), (list of supported <messageI>s),
     *             (list of supported <messageU>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 控制+CUUS1I的结果码是否上报，默认值为0。
     *             0：禁止；
     *             1：使能。
     *     <m>: 控制+CUUS1U的结果码是否上报，默认值为0。
     *             0：禁止；
     *             1：使能。
     *     <message>: 发送消息中包含User-to-User Information Element的消息类型。
     *             0：ANY，等同于SETUP；
     *             1：SETUP；
     *             2：ALERT；
     *             3：CONNECT；
     *             4：DISCONNECT；
     *             5：RELEASE；
     *             6：RELEASE_COMPLETE。
     *     <UUIE>: 16进制字符，等同于协议24008规定的User-to-User Information Element协议格式。
     *     <messageI>: +CUUS1I的结果码上报的消息类型。
     *             0：ANY；
     *             1：ALERT；
     *             2：PROGRESS；
     *             3：CONNECT (sent after +COLP if enabled)；
     *             4：RELEASE。
     *     <messageU>: +CUUS1U的结果码上报的消息类型。
     *             0：ANY；
     *             1：SETUP (returned after +CLIP if presented, otherwise after every RING or +CRING)；
     *             2：DISCONNECT；
     *             3：RELEASE_COMPLETE。
     * [示例]:
     *     · 激活SETUP消息的用户信令服务1，同时使能上报+CUUS1I和+CUUS1U的结果码
     *       AT+CUUS1=1,1,1,7e0122
     *       OK
     *       该命令设置完成后，每次发往网络的SETUP消息中必然会携带该UUIE内容。
     *     · 去激活CONNECT消息的用户信令服务1，同时禁止上报+CUUS1I和+CUUS1U的结果码
     *       AT+CUUS1=0,0,3
     *       OK
     *       该命令设置完成后，每次发往网络的CONNECT消息中不会携带UUIE内容。
     *     · 查询已激活CUUS1I业务的消息类型以及+CUUS1I和+CUUS1U的上报使能
     *       AT+CUUS1?
     *       +CUUS1: 1,1,1,7e0110,4,7e022020
     *       OK
     *       表明当前SETUP和DISCONNECT消息中激活了UUS1信息。
     *     · 测试CUUS1
     *       AT+CUUS1=?
     *       +CUUS1: (0,1),(0,1),(0-6),(0-4),(0-3)
     *       OK
     */
    { AT_CMD_CUUS1,
      At_SetCuus1Para, AT_SET_PARA_TIME, At_QryCuus1Para, AT_QRY_PARA_TIME, At_TestCuus1Para, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CUUS1", (VOS_UINT8 *)"(0,1),(0,1),(0-6)" },

    /* SSA */
#if (FEATURE_MBB_CUST == FEATURE_ON)
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 呼叫等待
     * [说明]: 补充业务呼叫等待的操作，包括激活、去激活和状态查询。呼叫等待业务被激活后，则当前通话过程中有来电时会上报呼叫等待信息。
     *         CL模式下该命令不支持。
     *         本命令遵从3GPP TS 27.007协议。
     *         状态查询过程可以被打断。
     * [语法]:
     *     [命令]: +CCWA=[<n>[,<mode>][,<class>]]
     *     [结果]: when <mode>=2 and command successful
     *             <CR><LF>+CCWA: <status>,<class1>[,<ss-status>]
     *             [<CR><LF>+CCWA: <status>,<class2>[,<ss-status>]
     *             [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CCWA?
     *     [结果]: <CR><LF>+CCWA: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CCWA=?
     *     [结果]: <CR><LF>+CCWA: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，禁止或使能结果码的主动上报，默认值为0。
     *             0：禁止；
     *             1：使能。
     *     <mode>: 整型值，操作类型。
     *             0：去激活；
     *             1：激活；
     *             2：状态查询。
     *     <class>: 整型值，业务类别，取值为以下业务类别的组合，默认值为255。
     *             1：voice (telephony)；
     *             2：data；
     *             4：fax (facsimile services)；
     *             8：short message service；
     *             16：data circuit sync；
     *             32：data circuit async；
     *             64：dedicated packet access；
     *             128：dedicated PAD access。
     *     <status>: 整型值，当前呼叫等待的业务状态。
     *             0：未激活；
     *             1：激活。
     *     <ss-status>: 整型值，私有字段接口，输出完整的业务状态，受网侧空口控制，含义如下。
     *             BIT3：业务可操作状态，业务激活状态有效。
     *             1：业务用户不可操作；
     *             0：业务用户可操作；
     *             BIT 2：签约状态。
     *             1：业务已签约；
     *             0：业务未签约；
     *             BIT1：业务注册状态。
     *             1：业务已注册；
     *             0：业务未注册；
     *             BIT0：业务激活状态。
     *             1：业务已激活；
     *             0：业务未激活；
     * [示例]:
     *     · 成功激活呼叫等待
     *       AT+CCWA=1,1,1
     *       OK
     *     · 查询呼叫等待状态
     *       AT+CCWA=1,2,1
     *       +CCWA: 1,1
     *       OK
     *     · 查询呼叫等待是否使能
     *       AT+CCWA?
     *       +CCWA: 1
     *       OK
     *     · 测试CCWA
     *       AT+CCWA=?
     *       +CCWA: (0,1)
     *       OK
     */
    { AT_CMD_CCWA,
      At_SetCcwaPara, AT_SET_SS_PARA_TIME, At_QryCcwaPara, AT_NOT_SET_TIME, At_TestCcwaPara,
      AT_NOT_SET_TIME, AT_AbortCcwaPara, AT_NOT_SET_TIME, AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CCWA", (VOS_UINT8 *)"(0,1),(0-2),(1-255)"
    },
#else
    { AT_CMD_CCWA,
      At_SetCcwaPara, AT_SET_SS_PARA_TIME, At_QryCcwaPara, AT_NOT_SET_TIME, At_TestCcwaPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CCWA", (VOS_UINT8 *)"(0,1),(0-2),(1-255)" },
#endif
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 呼叫转移
     * [说明]: 补充业务呼叫转移的操作，包括注册、删除、激活、去激活和状态查询。
     *         其中，查询时<class>必须为单一的具体业务类型。
     *         CL模式下该命令不支持。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CCFC=<reason>,<mode>[,<number>[,<type>[,<class>[,<subaddr>[,<satype>[,<time>]]]]]]
     *     [结果]: when <mode>=2 and command successful:
     *             <CR><LF>+CCFC: <status>,<class1>[,<number>,<type>[,<subaddr>,<satype>[,<time>]]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CCFC=?
     *     [结果]: <CR><LF>+CCFC: (list of supported <reason>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <reason>: 整型值，呼叫转移的类型。
     *             0：无条件转移；
     *             1：遇忙转移；
     *             2：无应答转移；
     *             3：不可达转移（无网络服务或者关机时）；
     *             4：所有呼叫转移；
     *             5：所有条件转移。
     *     <mode>: 整型值，呼叫转移的操作模式。
     *             0：去激活；
     *             1：激活；
     *             2：状态查询（查询时，reason不能为4或5）；
     *             3：注册；
     *             4：删除。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <subaddr>: 字符串类型，子地址，不处理此参数。
     *     <satype>: 整型值，子地址类型，不处理此参数。
     *     <class>: 整型值，业务类别，取值为以下业务类别的组合，默认值为255。
     *             1：voice (telephony)；
     *             2：data；
     *             4：fax (facsimile services)；
     *             8：short message service；
     *             16：data circuit sync；
     *             32：data circuit async；
     *             64：dedicated packet access；
     *             128：dedicated PAD access。
     *     <time>: 整型值，当设置或查询“无应答转移”时，此参数可以表示在转移呼叫前等待多长时间。
     *     <status>: 整型值，状态。
     *             0：未激活；
     *             1：激活。
     * [示例]:
     *     · 注册一个遇忙转移业务
     *       AT+CCFC=1,3,"12345678",129,1
     *       OK
     *     · 注册一个无应答转移业务，网络无相应业务
     *       AT+CCFC=2,3,"12345678",129,1
     *       +CME ERROR: 30
     *     · 查询遇忙转移业务
     *       AT+CCFC=1,2
     *       +CCFC: 1,1,"+8612345678",145
     *       OK
     *     · 删除一个遇忙转移业务
     *       AT+CCFC=1,4
     *       OK
     *     · 测试CCFC
     *       AT+CCFC=?
     *       +CCFC: (0-5)
     *       OK
     */
    { AT_CMD_CCFC,
      At_SetCcfcPara, AT_SET_SS_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_TestCcfcPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CCFC", (VOS_UINT8 *)"(0-5),(0-4),(number),(0-255),(1-255),(subaddr),(0-255),(1-30)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: USSD业务
     * [说明]: 提供补充业务USSD（Unstructured Supplementary Service Data）的控制，支持网络或TE发起的操作。
     *         CL模式下该命令不支持。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CUSD=[<n>[,<str>[,<dcs>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CUSD?
     *     [结果]: <CR><LF>+CUSD: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CUSD=?
     *     [结果]: <CR><LF>+CUSD: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，控制结果码上报，默认值为0。
     *             0：禁止结果码上报；
     *             1：使能结果码上报；
     *             2：退出会话。
     *     <str>: USSD字符串，最大长度为160个字符。
     *     <dcs>: 整型值，编码方式。
     *             15：7bit编码（默认7bit编码）；
     *             68：8bit编码（不建议使用）；
     *             72：UCS2编码（不建议使用）。
     *             商用透传模式下允许应用下发满足23038 CBS DCS协议的DCS参数，包括0、1、2…等等；
     * [示例]:
     *     · 发起一个USSD操作，7bit编码，使能上报
     *       AT+CUSD=1,"AAD86C3602",15
     *       OK
     *     · 发起一个USSD操作，不带任何参数，默认按照AT+CUSD=0处理
     *       AT+CUSD=
     *       OK
     *     · 查询USSD业务
     *       AT+CUSD?
     *       +CUSD: 1
     *       OK
     *     · 退出会话
     *       AT+CUSD=2
     *       OK
     *     · 测试USSD
     *       AT+CUSD=?
     *       +CUSD: (0-2)
     *       OK
     */
    { AT_CMD_CUSD,
      At_SetCusdPara, AT_SS_CUSD_SET_PARA_TIME, At_QryCusdPara, AT_QRY_PARA_TIME, At_TestCusdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CUSD", (VOS_UINT8 *)"(0-2),(str),(0-255)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询产品软件版本信息
     * [说明]: 此2条命令都用于查询产品软件版本信息。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +GMR
     *     [结果]: <CR><LF><revision><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +GMR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <revision>: 字符串，软件版本信息。
     * [示例]:
     *     · 查询软件版本信息
     *       AT+CGMR
     *       V100
     *       OK
     *       AT+GMR
     *       V100
     *       OK
     *     · 测试CGMR
     *       AT+CGMR=?
     *       OK
     *       AT+GMR=?
     *       OK
     */
    { AT_CMD_GMR,
      At_SetCgmrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+GMR", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询制造商信息
     * [说明]: 此2条命令都用于查询制造商信息。
     *         本命令遵从3GPP TS 27.007协议。
     *         此2条命令查询结果受产品线定制NV8203控，默认为“Huawei”
     * [语法]:
     *     [命令]: +GMI
     *     [结果]: <CR><LF><manufacturer><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +GMI=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <manufacturer>: 字符串，制造商信息。
     * [示例]:
     *     · 查询制造商信息
     *       AT+CGMI
     *       Huawei
     *       OK
     *       AT+GMI
     *       Huawei
     *       OK
     *     · 测试CGMI
     *       AT+CGMI=?
     *       OK
     *       AT+GMI=?
     *       OK
     */
    { AT_CMD_GMI,
      At_SetCgmiPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+GMI", VOS_NULL_PTR },

    /* OTHER */
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 呼叫偏转
     * [说明]: 将当前的来电或呼叫等待的电话偏转到另外一个号码。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CTFR=<number>[,<type>[,<subaddr>[,<satype>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CTFR=?
     *     [结果]: <CR><LF>+CTFR: <number>, (list of supported <type>s),<subaddr>,(list of supported <satype>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 整型值，号码地址类型，其结构如表2-7所示。
     *             Type-of-number（bit[6:4]），取值如下：
     *             000：用户不了解目的地址号码时，选用此值，此时目的地址号码由网络填写。
     *             001：国际号码。
     *             010：国内号码。
     *             011：特定号码，禁用。
     *             101：保留。
     *             110：保留。
     *             111：保留。
     *             Numbering-plan-identification（bit[3:0]），取值如下：
     *             0000：号码由网络的号码方案确定。
     *             0001：ISDN/telephone numbering plan。
     *             0011：Data numbering plan。
     *             0100：Telex numbering plan。
     *             1000：National numbering plan。
     *             1001：Private numbering plan。
     *             1010：ERMES numbering plan。
     *             说明：当bit[6:4]取值为000、001、010时bit[3:0]才有效。
     *     <subaddr>: 字符串类型，子地址，软件不处理。
     *     <satype>: 整型值，子地址类型，软件不处理。
     * [表]: 号码地址类型的结构
     *       bit:   7,    6,              5,              4,              3,                             2,                             1,                             0,
     *              1,    Type-of-number, Type-of-number, Type-of-number, Numbering-plan-identification, Numbering-plan-identification, Numbering-plan-identification, Numbering-plan-identification,
     * [示例]:
     *     · 将来电偏转到号码1234
     *       AT+CTFR="1234",129
     *       OK
     *     · 测试CTFR
     *       AT+CTFR=?
     *       +CTFR: (number),(0-255),(subaddr),(0-255)
     *       OK
     */
    { AT_CMD_CTFR,
      At_SetCtfrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"+CTFR", (VOS_UINT8 *)"(number),(0-255),(subaddr),(0-255)" },

    /*
     * [类别]: 协议AT-安全管理
     * [含义]: 修改密码
     * [说明]: 用于修改+CLCK定义的设备锁定密码。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CPWD=<fac>,<oldpwd>,<newpwd>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPWD=?
     *     [结果]: <CR><LF>+CPWD: list of supported (<fac>,<pwdlength>)s<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <fac>: 字符串类型，指定该命令操作的对象。
     *             “SC”：SIM PIN1；
     *             “P2”：SIM PIN2；
     *             “AO”：禁止所有呼出；
     *             “OI”：禁止所有国际呼出；
     *             “OX”：禁止所有国际呼出，归属国除外；
     *             “AI”：禁止所有呼入；
     *             “IR”：归属地以外漫游时，禁止所有呼入；
     *             “AB”：禁止所有业务（仅在<mode>=0时适用）；
     *             “AG”：禁止呼出业务（仅在<mode>=0时适用）；
     *             “AC”：禁止呼入业务（仅在<mode>=0时适用）。
     *     <oldpwd>、<newpwd>: 字符串类型，旧密码和新密码。
     *     <pwdlength>: 整型值，<oldpwd>、<newpwd>的最大长度。
     * [示例]:
     *     · 改变密码成功
     *       AT+CPWD="AO","1234","5678"
     *       OK
     *     · 改变密码失败
     *       AT+CPWD="AO","5678","1234"
     *       +CME ERROR: 16
     *     · 测试CPWD
     *       AT+CPWD=?
     *       +CPWD: ("P2",8),("SC",8),("AO",4)("OI",4),("OX",4),("AI",4),("IR",4), ("AB",4),("AG",4),("AC",4)
     *       OK
     */
    { AT_CMD_CPWD,
      At_SetCpwdPara, AT_SET_SS_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCpwdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CPWD", (VOS_UINT8 *)"(\"P2\",\"SC\",\"AO\",\"OI\",\"OX\",\"AI\",\"IR\",\"AB\",\"AG\",\"AC\",\"PS\"),(@oldpwd),(@newpwd)" },

    /*
     * [类别]: 协议AT-安全管理
     * [含义]: 设备锁定
     * [说明]: 该命令用于加锁、解锁MT或者网络功能，以及查询锁定状态。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CLCK=<fac>,<mode>[,<passwd>[,<class>]]
     *     [结果]: when <mode>=2 and command successful:
     *             <CR><LF>+CLCK: <status>[,<class1>[,<ss-status>]
     *             [<CR><LF>+CLCK: <status>,<class2>[,<ss-status>]
     *             [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CLCK=?
     *     [结果]: <CR><LF>+CLCK: (list of supported <fac>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <fac>: 字符串类型，指定该命令操作的对象。
     *             “SC”：SIM PIN1；
     *             “AO”：禁止所有呼出；
     *             “OI”：禁止所有国际呼出；
     *             “OX”：禁止所有国际呼出，归属国除外；
     *             “AI”：禁止所有呼入；
     *             “IR”：归属地以外漫游时，禁止所有呼入；
     *             “AB”：禁止所有业务（仅在<mode>=0时适用）；
     *             “AG”：禁止呼出业务（仅在<mode>=0时适用）；
     *             “AC”：禁止呼入业务（仅在<mode>=0时适用）；
     *             “FD”：FDN；
     *             “PN”：锁网络；
     *             “PU”：锁子网；
     *             “PP”：锁SP。
     *     <mode>: 整型值，操作模式。
     *             0：去激活（当<fac>参数为"PN","PU","PP"时，为解锁操作)；
     *             1：激活（当<fac>参数为"PN","PU","PP"时，不支持激活）；
     *             2：查询。
     *     <status>: 整型值，当前状态,当<fac>参数为"PN","PU","PP"时，表示的是锁网锁卡前三层锁的激活状态。
     *             0：未激活；
     *             1：激活。
     *     <passwd>: 字符串类型；与修改密码命令+CPWD所设定的密码相同。
     *     <class>: 整型值，业务类别，取值为以下业务类别的组合，默认值为255。
     *             1：voice (telephony)；
     *             2：data；
     *             4：fax (facsimile services)；
     *             8：short message service；
     *             16：data circuit sync；
     *             32：data circuit async；
     *             64：dedicated packet access；
     *             128：dedicated PAD access。
     *     <ss-status>: 整型值，私有字段接口，输出完整的业务状态，受网侧空口控制，含义如下。
     *             BIT3：业务可操作状态，业务激活状态有效。
     *             1：业务用户不可操作；
     *             0：业务用户可操作；
     *             BIT 2：签约状态。
     *             1：业务已签约；
     *             0：业务未签约；
     *             BIT1：业务注册状态。
     *             1：业务已注册；
     *             0：业务未注册；
     *             BIT0：业务激活状态。
     *             1：业务已激活；
     *             0：业务未激活；
     * [示例]:
     *     · 激活PIN码，则开机需要输入PIN码
     *       AT+CLCK="SC",1,"1234"
     *       OK
     *     · 激活禁止所有呼出
     *       AT+CLCK="AO",1,"1234"
     *       OK
     *     · 查询禁止所有呼出状态
     *       AT+CLCK="AO",2
     *       +CLCK: 1,1
     *       OK
     *     · 测试CLCK
     *       AT+CLCK=?
     *       +CLCK: ("SC","AO","OI","OX","AI","IR","AB","AG","AC","FD","PN","PU","PP")
     *       OK
     */
    { AT_CMD_CLCK,
      At_SetClckPara, AT_SET_SS_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestClckPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CLCK", (VOS_UINT8 *)"(\"P2\",\"SC\",\"AO\",\"OI\",\"OX\",\"AI\",\"IR\",\"AB\",\"AG\",\"AC\",\"PS\",\"FD\",\"PN\",\"PU\",\"PP\"),(0-2),(psw),(1-255)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: PS域附着
     * [说明]: PS域附着相关操作。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGATT=<state>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGATT?
     *     [结果]: <CR><LF>+CGATT: <state><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGATT=?
     *     [结果]: <CR><LF>+CGATT: (list of supported <state>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值，PS域服务状态。
     *             0：分离状态；
     *             1：附着状态。
     * [示例]:
     *     · 发起PS域附着
     *       AT+CGATT=1
     *       OK
     *     · 查询PS域附着状态
     *       AT+CGATT?
     *       +CGATT: 1
     *       OK
     *     · 测试CGATT
     *       AT+CGATT=?
     *       +CGATT: (0,1)
     *       OK
     */
    { AT_CMD_CGATT,
      At_SetCgattPara, AT_CGATT_SET_PARA_TIME, At_QryCgattPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGATT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 选择PLMN
     * [说明]: 手动、自动方式的PLMN（Public Land Mobile Network）选择。
     *         查询当前注册的PLMN。
     *         获取PLMN列表。
     *         获取PLMN列表过程中，可通过输入打断字符串（该字符串可通过NV en_NV_Item_AT_ABORT_CMD_PARA配置，若配置打断字符串为ANY的ASCII码，则表示任意字符打断，否则打断字符串按NV中配置的字符处理）打断该过程，打断完成后输出“OK”（输出字符串可通过相同NV项配置）。
     *         仅CSIM卡在位的CL模式下，支持AT+COPS=0，AT+COPS=3,<format>和 AT+COPS?查询。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +COPS=[<mode>[,<format>
     *             [,<oper>[,<rat>]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +COPS?
     *     [结果]: <CR><LF>+COPS: <mode>[,<format>,<oper>[,<rat>]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +COPS=?
     *     [结果]: <CR><LF>+COPS: [list of supported (<stat>,long alphanumeric <oper>,short alphanumeric <oper>,numeric <oper>[,<rat>])s][,,(list of supported <mode>s),(list of supported <format>s)] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，网络选择模式，默认值为0。
     *             0：自动搜网，<mode>为0时，参数<format>、<oper>无效；
     *             1：手动搜网；
     *             2：去注册网络；
     *             3：仅为+COPS?查询命令设置返回格式<format>；
     *             4：手动与自动的联合，如果手动搜网失败，则自动转入自动搜网模式。
     *     <format>: 整型值，运营商信息<oper>的格式，默认值为0。
     *             0：长字符串格式的运营商信息<oper>；
     *             1：短字符串格式的运营商信息<oper>；
     *             2：数字格式的运营商信息<oper>。
     *     <oper>: 字符串类型，运营商信息。
     *     <stat>: 整型值，网络的状态标识。
     *             0：未知；
     *             1：可用；
     *             2：已注册；
     *             3：禁止。
     *     <rat>: 整型值，无线接入技术，默认值与SYSCFG中设置的优先级相同。
     *             0：GSM/GPRS；
     *             2：WCDMA；
     *             7：LTE；
     *             注意：该值仅在单板支持GUL或CL多模时生效，不支持LTE时，该参数不能选择。
     *             12: NR；
     *             注意：该值仅在单板支持GULNR模时生效，不支持NR时，该参数不能选择。
     *             13：endc;
     *             注意：该值仅在同时支持LTE和NR时生效，不支持endc时，该参数不能选择。
     * [示例]:
     *     · 获取PLMN列表
     *       AT+COPS=?
     *       +COPS: (2,"CHINA MOBILE","CMCC","46000",0),(0,"CHN-UNICOM","UNICOM","46001",0)
     *       OK
     *     · 自动搜网
     *       AT+COPS=0
     *       OK
     *     · 手动搜网
     *       AT+COPS=1,2,"46000"
     *       OK
     *     · +COPS?查询命令设置返回格式
     *       AT+COPS=3,2
     *       OK
     *     · 查询当前注册的PLMN
     *       AT+COPS?
     *       +COPS: 1,2,"46000",0
     *       OK
     *       获取PLMN列表过程被打断
     *     · 执行获取PLMN列表命令
     *       AT+COPS=?
     *       AT+CSQ
     *       AT
     *       OK
     *     · 再次查询信号强度，返回信号强度
     *       AT+CSQ
     *       +CSQ: 5,99
     *       OK
     */
    { AT_CMD_COPS,
      At_SetCopsPara, AT_COPS_SPEC_SRCH_TIME, At_QryCopsPara, AT_QRY_PARA_TIME, At_TestCopsPara, AT_COPS_LIST_SRCH_TIME,
      At_AbortCopsPara, AT_COPS_LIST_ABORT_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+COPS", (VOS_UINT8 *)COPS_CMD_PARA_STRING },

#if (FEATURE_CSG == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: CSG IDS搜索
     * [说明]: 可用于进行指定的CSG ID的搜索，指定CSG搜网时只允许携带一个CSG ID；
     *         可用于当前驻留CSG ID的查询；
     *         可用于进行CSG ID列表搜索；
     *         目前只支持LTE的CSG ID的搜索；
     *         此AT命令为私有AT命令。
     * [语法]:
     *     [命令]: ^CSGIDSRCH=<PlmnId>,<CSGID>,<rat>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CSGIDSRCH?
     *     [结果]: <CR><LF>^CSGIDSRCH: <PlmnId>,<CSG ID > ,<rat><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CSGIDSRCH=?
     *     [结果]: <CR><LF>^CSGIDSRCH: <OperNameLong >,< OperNameShort >,<PlmnId>,<CSG ID > , <CSG ID type>,<home NodeB Name>,< CSG type >,<rat>,< sSignalValue1>,< sSignalValue2>,<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <OperNameLong>: 字符串类型的运营商长名
     *     <OperNameShort>: 字符串类型的运营商短名
     *     <PlmnId>: 数字格式的运营商信息，即PLMN信息
     *     <CSG ID>: Closed Subscriber Group Identification，CSG小区的标识
     *     <CSG ID type>: CSG ID类型
     *             1：CSG ID在Allowed CSG List中
     *             2：CSG ID在Operator CSG List中且不在Forbidden CSG List中
     *             3：CSG ID在Operator CSG List中且在Forbidden CSG List中
     *             4：CSG ID在Allowed CSG List和Operator CSG List
     *     <home NodeB Name>: home NodeB Name
     *     <CSG type>: CSG类型
     *     <rat>: 整型值，无线接入技术，默认值与SYSCFG 中设置的优先级相同。
     *             0：GSM/GPRS；
     *             2：WCDMA；
     *             7：LTE（目前只支持LTE的CSG功能）
     *     <sSignalValue1>: rat为LTE时表示RSRP，rat为WCDMA时表示RSCP，rat为GSM时表示RSSI
     *     <sSignalValue2>: rat为LTE时表示RSRQ，rat为WCDMA时表示EC/IO，rat为GSM时默认为0
     * [示例]:
     *     · CSG指定搜310410 CSG ID为1234的网络
     *       AT^CSGIDSRCH="310410","1234",7
     *       OK
     *     · 查询当前驻留CSG ID信息
     *       AT^CSGIDSRCH?
     *       ^CSGIDSRCH: "310410","1234",7
     *       OK
     *     · CSG ID列表搜索
     *       AT^CSGIDSRCH=?
     *       ^CSGIDSRCH: "Vodafone CZ","Vodafone","23003","BB8",4,0203,0202020202020202,7,-60,-82
     *       OK
     */
    { AT_CMD_CSGIDSEARCH,
      AT_SetCsgIdSearchPara, AT_CSG_SPEC_SRCH_TIME, At_QryCampCsgIdInfo, AT_QRY_PARA_TIME, AT_TestCsgIdSearchPara, AT_CSG_LIST_SRCH_TIME,
      AT_AbortCsgIdSearchPara, AT_CSG_LIST_ABORT_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CSGIDSRCH", (VOS_UINT8 *)"(@oper),(@csg),(0,2,7)" },
#endif

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置MT模式
     * [说明]: 设置MT模式。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGCLASS=[<class>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGCLASS?
     *     [结果]: <CR><LF>+CGCLASS: <class><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGCLASS=?
     *     [结果]: <CR><LF>+CGCLASS: (list of supported <class>s)
     *             <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <class>: 字符串类型，指示MT模式。
     *             A：同时支持CS域和PS域（WCDMA网络模式下有效，在GSM网络模式下MT模式A将自动转换为MT模式B）（暂不支持）；
     *             B：同一时刻只能支持CS域或PS域中的一个域（GSM网络模式下有效，在WCDMA网络模式下MT模式B将自动转换为MT模式A）（暂不支持）；
     *             CG：只支持PS域（暂不支持）；
     *             CC：只支持CS域（暂不支持）。
     * [示例]:
     *     · 设置为MT模式A
     *       AT+CGCLASS="A"
     *       OK
     *     · 查询CGCLASS
     *       AT+CGCLASS?
     *       +CGCLASS: "A"
     *       OK
     *     · 测试CGCLASS
     *       AT+CGCLASS=?
     *       +CGCLASS: ("A")
     *       OK
     *     · 不带参数
     *       AT+CGCLASS=
     *       +CME ERROR: Incorrect parameters
     */
    { AT_CMD_CGCLASS,
      At_SetCgclassPara, AT_SET_PARA_TIME, At_QryCgclassPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGCLASS", (VOS_UINT8 *)"(\"A\")" },

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置MT功能模式
     * [说明]: 设置MT功能模式。
     *         本命令遵从3GPP TS 27.007协议。
     *         AP-MODEM和E5形态AT+CFUN=8下电功能需要由应用发起，如果测试人员通过AT命令下发CFUN命令，不会触发下电。
     * [语法]:
     *     [命令]: +CFUN=[<fun>[,<rst>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CFUN?
     *     [结果]: <CR><LF>+CFUN: <fun><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CFUN=?
     *     [结果]: <CR><LF>+CFUN: (list of supported <fun>s), (list of supported <rst>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <fun>: 整型值，功能模式，默认值为0。
     *             0：设置为最小模式（可用NV2594控制是否允许从offline模式切换到最小模式，默认不允许）；
     *             1：设置为online模式（开机后的默认值）（可用NV2594控制是否允许从offline模式切换到online模式，默认不允许）；
     *             4：设置为offline模式（MT之前的设置必须为非FTM模式）；
     *             5：设置为FTM模式（MT之前的设置必须为online模式，但为了校准使用，可以从最小模式切换到工厂模式）；
     *             6：重启MT（MT之前的设置必须为offline模式）；
     *             7：设置为最小模式（MT之前的设置必须为非offline模式），为VODAFONE运营商定制；
     *             8：关机下电（如充电器或USB在位，则进入关机充电模式），仅用于E5形态和AP-Modem形态；
     *             9：SIM卡温保关机模式（供内部模块使用，用户不能设置）；
     *             10：快速关机，关机后效果等同于模式0；
     *             11：快速开机，(MT之前的设置必须为offline模式)。
     *             注：参数值10和11仅在FEATURE_MBB_CUST宏开启时生效。
     *     <rst>: 整型值，是否复位。
     *             0：不触发复位。
     *             1：触发复位。当前只支持online模式下AT+CFUN=1,1触发复位。
     *             触发复位是modem单独复位还是整机复位由NV2391控制。
     * [示例]:
     *     · online模式下发复位
     *       AT+CFUN=1,1
     *       OK
     *     · 设置为online模式
     *       AT+CFUN=1
     *       OK
     *     · 查询CFUN
     *       AT+CFUN?
     *       +CFUN: 1
     *       OK
     *     · 测试CFUN
     *       AT+CFUN=?
     *       +CFUN: (0,1,4,5,6,7,8,10,11),(0,1)
     *       OK
     */
    { AT_CMD_CFUN,
      At_SetCfunPara, AT_SET_CFUN_TIME, At_QryCfunPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CFUN", (VOS_UINT8 *)"(0,1,4,5,6,7,8,10,11),(0,1)" },
#else
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置MT功能模式
     * [说明]: 设置MT功能模式。
     *         本命令遵从3GPP TS 27.007协议。
     *         AP-MODEM和E5形态AT+CFUN=8下电功能需要由应用发起，如果测试人员通过AT命令下发CFUN命令，不会触发下电。
     * [语法]:
     *     [命令]: +CFUN=[<fun>[,<rst>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CFUN?
     *     [结果]: <CR><LF>+CFUN: <fun><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CFUN=?
     *     [结果]: <CR><LF>+CFUN: (list of supported <fun>s), (list of supported <rst>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <fun>: 整型值，功能模式，默认值为0。
     *             0：设置为最小模式（可用NV2594控制是否允许从offline模式切换到最小模式，默认不允许）；
     *             1：设置为online模式（开机后的默认值）（可用NV2594控制是否允许从offline模式切换到online模式，默认不允许）；
     *             4：设置为offline模式（MT之前的设置必须为非FTM模式）；
     *             5：设置为FTM模式（MT之前的设置必须为online模式，但为了校准使用，可以从最小模式切换到工厂模式）；
     *             6：重启MT（MT之前的设置必须为offline模式）；
     *             7：设置为最小模式（MT之前的设置必须为非offline模式），为VODAFONE运营商定制；
     *             8：关机下电（如充电器或USB在位，则进入关机充电模式），仅用于E5形态和AP-Modem形态；
     *             9：SIM卡温保关机模式（供内部模块使用，用户不能设置）；
     *             10：快速关机，关机后效果等同于模式0；
     *             11：快速开机，(MT之前的设置必须为offline模式)。
     *             注：参数值10和11仅在FEATURE_MBB_CUST宏开启时生效。
     *     <rst>: 整型值，是否复位。
     *             0：不触发复位。
     *             1：触发复位。当前只支持online模式下AT+CFUN=1,1触发复位。
     *             触发复位是modem单独复位还是整机复位由NV2391控制。
     * [示例]:
     *     · online模式下发复位
     *       AT+CFUN=1,1
     *       OK
     *     · 设置为online模式
     *       AT+CFUN=1
     *       OK
     *     · 查询CFUN
     *       AT+CFUN?
     *       +CFUN: 1
     *       OK
     *     · 测试CFUN
     *       AT+CFUN=?
     *       +CFUN: (0,1,4,5,6,7,8,10,11),(0,1)
     *       OK
     */
    { AT_CMD_CFUN,
      At_SetCfunPara, AT_SET_CFUN_TIME, At_QryCfunPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CFUN", (VOS_UINT8 *)"(0,1,4,5,6,7,8),(0,1)" },
#endif


    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询制造商信息
     * [说明]: 此2条命令都用于查询制造商信息。
     *         本命令遵从3GPP TS 27.007协议。
     *         此2条命令查询结果受产品线定制NV8203控，默认为“Huawei”
     * [语法]:
     *     [命令]: +CGMI
     *     [结果]: <CR><LF><manufacturer><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGMI=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <manufacturer>: 字符串，制造商信息。
     * [示例]:
     *     · 查询制造商信息
     *       AT+CGMI
     *       Huawei
     *       OK
     *       AT+GMI
     *       Huawei
     *       OK
     *     · 测试CGMI
     *       AT+CGMI=?
     *       OK
     *       AT+GMI=?
     *       OK
     */
    { AT_CMD_CGMI,
      At_SetCgmiPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGMI", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询MT型号
     * [说明]: 查询MT型号。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGMM
     *     [结果]: <CR><LF><model><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGMM=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <model>: 字符串，MT型号信息。
     * [示例]:
     *     · 查询MT型号
     *       AT+CGMM
     *       H8180
     *       OK
     *     · 测试CGMM
     *       AT+CGMM=?
     *       OK
     */
    { AT_CMD_CGMM,
      At_SetCgmmPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGMM", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询MT型号
     * [说明]: 查询MT型号。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +GMM
     *     [结果]: <CR><LF><model><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +GMM=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <model>: 字符串，MT型号信息。
     * [示例]:
     *     · 查询MT型号
     *       AT+CGMM
     *       H8180
     *       OK
     *     · 测试CGMM
     *       AT+CGMM=?
     *       OK
     */
    { AT_CMD_GMM,
      At_SetCgmmPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+GMM", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询产品软件版本信息
     * [说明]: 此2条命令都用于查询产品软件版本信息。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGMR
     *     [结果]: <CR><LF><revision><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGMR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <revision>: 字符串，软件版本信息。
     * [示例]:
     *     · 查询软件版本信息
     *       AT+CGMR
     *       V100
     *       OK
     *       AT+GMR
     *       V100
     *       OK
     *     · 测试CGMR
     *       AT+CGMR=?
     *       OK
     *       AT+GMR=?
     *       OK
     */
    { AT_CMD_CGMR,
      At_SetCgmrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGMR", VOS_NULL_PTR },
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询产品序列号
     * [说明]: 查询产品序列号，即国际移动设备ID（International Mobile Equipment Identity）。+GSN与+CGSN功能完全一样，命令使用格式也一样。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGSN
     *     [结果]: <CR><LF><sn><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGSN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <sn>: 字符串，产品序列号，构成请参见表2-3。
     * [表]: <sn>构成描述
     *       8byte,                   6byte,             1byte,
     *       TAC（设备分配的类型码）, SNR（设备序列号）, Spare（备用）,
     *       例如：如果TAC为“35154800”，SNR为“122544”，Spare为“0”，则IMEI为351548001225440。,
     * [示例]:
     *     · 查询产品序列号
     *       AT+CGSN
     *       351548001225440
     *       OK
     *     · 测试CGSN
     *       AT+CGSN=?
     *       OK
     */
    { AT_CMD_CGSN,
      At_SetCgsnPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGSN", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询产品序列号
     * [说明]: 查询产品序列号，即国际移动设备ID（International Mobile Equipment Identity）。+GSN与+CGSN功能完全一样，命令使用格式也一样。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +GSN
     *     [结果]: <CR><LF><sn><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +GSN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <sn>: 字符串，产品序列号，构成请参见表2-3。
     * [表]: <sn>构成描述
     *       8byte,                   6byte,             1byte,
     *       TAC（设备分配的类型码）, SNR（设备序列号）, Spare（备用）,
     *       例如：如果TAC为“35154800”，SNR为“122544”，Spare为“0”，则IMEI为351548001225440。,
     * [示例]:
     *     · 查询产品序列号
     *       AT+CGSN
     *       351548001225440
     *       OK
     *     · 测试CGSN
     *       AT+CGSN=?
     *       OK
     */
    { AT_CMD_GSN,
      At_SetCgsnPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+GSN", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询IMSI
     * [说明]: 查询(U)SIM卡的IMSI（International Mobile Subscriber Identity）值。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CIMI
     *     [结果]: <CR><LF><IMSI><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CIMI=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <IMSI>: 国际移动用户识别码。
     * [表]: IMSI构成描述
     *       最多15个字符,
     *       3个字符,         2个或3个字符,    其余,
     *       MCC（国家代号）, MNC（网络代号）, MSIN（标识移动用户身份）,
     *       例如：如果MCC为“230”，MNC为“02”，MSIN为“0216666831”，则IMSI为：230020216666831。,
     * [示例]:
     *     · 查询IMSI值
     *       AT+CIMI
     *       230020216666831
     *       OK
     *     · 测试CIMI
     *       AT+CIMI=?
     *       OK
     */
    { AT_CMD_CIMI,
      At_SetCimiPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"+CIMI", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询信号强度
     * [说明]: 查询当前服务小区信号强度与信道误码率。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CSQ
     *     [结果]: <CR><LF>+CSQ: <rssi>,<ber><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CSQ: <99>,<99><CR><LF>
     *     [命令]: +CSQ=?
     *     [结果]: <CR><LF>+CSQ: (list of supported <rssi>s),(list of supported <ber>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <rssi>: 整型值，接收信号强度指示。
     *             0：等于或小于–113dBm；
     *             1：–111dBm；
     *             2～30：–109dBm～–53dBm，步长为2dBm（2：–109dBm；3：–107dBm；
     *             ……30：–53dBm）；
     *             31：等于或大于–51dBm；
     *             99：未知或不可测。
     *     <ber>: 0~7：整型值，比特误码率百分比。
     *             99：未知或不可测。
     * [示例]:
     *     · 查询信号强度和误码率
     *       AT+CSQ
     *       +CSQ: 5,99
     *       OK
     *     · 测试CSQ
     *       AT+CSQ=?
     *       +CSQ: (0-31,99),(99)
     *       OK
     */
    { AT_CMD_CSQ,
      At_SetCsqPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CSQ", (VOS_UINT8 *)"(0-31,99),(99)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 列出所有有效的AT命令
     * [说明]: 此命令查询支持的所有代码中规定的可显示的AT命令。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CLAC
     *     [结果]: <CR><LF><AT Command1> [<CR><LF><AT Command2>[…]] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     */
    { AT_CMD_CLAC,
      At_SetCLACPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CLAC", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询MS当前所支持的传输能力域
     * [说明]: 查询MS当前所支持的传输能力域。
     *         本命令遵从ITU T Recommendation V.250协议。
     * [语法]:
     *     [命令]: +GCAP
     *     [结果]: <CR><LF>+GCAP: (list of supported<name>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +GCAP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <name>: MS支持的传输能力的命令名列表。
     *             CGSM：GSM能力；
     *             DS：数据压缩能力；
     *             ES：错误控制能力。
     * [示例]:
     *     · 查询MS当前所支持的传输能力域
     *       AT+GCAP
     *       +GCAP: +CGSM,+DS,+ES
     *       OK
     *     · GCAP测试命令
     *       AT+GCAP=?
     *       OK
     */
    { AT_CMD_GCAP,
      At_SetGcapPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+GCAP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 选择优先网络列表
     * [说明]: 选择当前处理的优先网络列表。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CPLS=<list>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPLS?
     *     [结果]: <CR><LF>+CPLS: <list><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CPLS=?
     *     [结果]: <CR><LF>+CPLS: (list of supported <list>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <list>: 整型值，选择优先网络列表，取值0～2。
     *             0：用户优选PLMN列表，对应USIM卡中的EFPLMNwAcT文件或SIM卡中的EFPLMNsel文件；
     *             1：运营商优选PLMN列表，对应USIM/SIM卡中的EFOPLMNwAcT文件；
     *             2：归属地优选PLMN列表，对应USIM/SIM卡中的EFHPLMNwAcT文件。
     * [示例]:
     *     · 设置选择优选网络列表为用户优选PLMN列表
     *       AT+CPLS=0
     *       OK
     *     · 查询当前选择优选网络列表
     *       AT+CPLS?
     *       +CPLS: 0
     *       OK
     *     · 选择优选网络列表的测试命令
     *       AT+CPLS=?
     *       +CPLS: (0,1,2)
     *       OK
     */
    { AT_CMD_CPLS,
      At_SetCplsPara, AT_SET_PARA_TIME, At_QryCplsPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CPLS", (VOS_UINT8 *)"(0,1,2)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 配置优先网络列表
     * [说明]: 对+CPLS选定的优先网络列表进行查询、修改PLMN ID和接入技术、删除、增加操作,受卡文件ADM权限限制，不能通过此AT命令修改HPLMN和OPLMN列表。
     *         本命令遵从3GPP TS 27.007协议。
     *         1、同一个PLMN ID，支持的接入技术不同，可以添加和修改多次。如果接入技术有重复，则不能添加，可以修改。
     *         2、当NR宏打开时，使用该AT命令进行配置时，与NV6006的配置相关（卡里的UPLMN/OPLMN是否需要补齐NR制式），6006配置使能UPLMN/OPLMN支持NR时，只要AT配置了新的UPLMN/OPLMN，而不管<NG-RAN_AcTn>是否支持，新增的UPLMN/OPLMN都会认为支持了NR。
     * [语法]:
     *     [命令]: +CPOL=[<index>][,<format>[,<oper>[,<GSM_AcT>,<GSM_Compact_AcT>,<UTRAN_AcT>,[<E-UTRAN_AcT>],[NG-RAN_AcT]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPOL?
     *     [结果]: <CR><LF>+CPOL: <index1>,<format>,<oper1>[,<GSM_AcT1>, <GSM_Compact_AcT1>,<UTRAN_AcT1>,<E-UTRAN_AcT>,<NG-RAN_AcT1>][<CR><LF>
     *             +CPOL: <index2>,<format>,<oper2>[,<GSM_AcT2>,<GSM_Compact_AcT2>,<UTRAN_AcT2>,<E-UTRAN_AcT>,<NG-RAN_AcT2>][...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CPOL=?
     *     [结果]: <CR><LF>+CPOL: (list of supported <index>s),(list of supported <format>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <indexn>: 整型值，USIM/SIM卡上PLMN的顺序。
     *     <format>: 整型值，运营商信息<oper>的格式，默认值为2。
     *             0：长字符串格式的运营商信息<oper>；
     *             1：短字符串格式的运营商信息<oper>；
     *             2：数字格式的运营商信息<oper>。
     *     <opern>: 字符串类型，运营商信息。
     *     <GSM_AcTn>: GSM的接入能力。
     *             0：不支持；
     *             1：支持。
     *     <GSM_Compact_AcTn>: GSM_Compact的接入能力。
     *             0：不支持；
     *             1：支持。
     *     <UTRA_AcTn>: UTRAN的接入能力。
     *             0：不支持；
     *             1：支持。
     *     <E-UTRAN_AcT>: LTE的接入能力。
     *             0：不支持；
     *             1：支持。
     *             注意：该参数仅在单板支持GUL多模时生效，不支持LTE时，该参数不能选择。
     *     <NG-RAN_AcTn>: NR的接入能力。
     *             0：不支持；
     *             1：支持。
     *             注意：该参数仅在单板支持NR时生效，不支持NR时，该参数不能选择。
     * [示例]:
     *     · 删除当前优先网络列表索引号为5的PLMN
     *       AT+CPOL=5
     *       OK
     *     · 查询当前优选网络列表
     *       AT+CPOL?
     *       +CPOL: 2,2,"222222",1,0,1,0
     *       +CPOL: 3,1,"CMCC",1,0,1,0
     *       +CPOL: 4,2,"444444",1,0,1,1
     *       +CPOL: 8,2,"123456",1,0,0,1
     *       OK
     *     · 当前优选网络列表的个数为8，测试+CPOL
     *       AT+CPOL=?
     *       +CPOL: (1-8),(0-2)
     *       OK
     *     · 平台能力支持NR，查询当前优选网络列表
     *       AT+CPOL?
     *       +CPOL: 2,2,"222222",1,0,1,1,1
     *       OK
     */
    { AT_CMD_CPOL,
      At_SetCpolPara, AT_SET_PARA_TIME, At_QryCpolPara, AT_QRY_PARA_TIME, At_TestCpolPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CPOL", (VOS_UINT8 *)CPOL_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询CS域注册状态
     * [说明]: 查询当前CS（Circuit Switched domain）域网络注册状态，以及设置状态上报方式。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CREG=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CREG?
     *     [结果]: <CR><LF>+CREG: <n>,<stat>[,<lac>,<ci>[,<AcT>]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CREG=?
     *     [结果]: <CR><LF>+CREG: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，主动上报方式，默认值为0。
     *             0：禁止+CREG的主动上报；
     *             1：使能+CREG: <stat>的主动上报；
     *             2：使能+CREG: <stat>[,<lac>,<ci>[,<AcT>]]的主动上报。
     *     <stat>: 整型值，注册状态。
     *             0：没有注册，MT没有处于搜索和注册新的运营商状态；
     *             1：注册了归属网络；
     *             2：没有注册，MT正在搜索并注册新的运营商；
     *             3：注册被拒绝；
     *             4：未知状态；
     *             5：注册了漫游网络。
     *     <lac>: 2byte的16进制数的字符串类型，位置码信息，例如：“00C3”表示10进制的195。
     *     <ci>: 4byte的16进制数的字符串类型，小区信息。
     *     <AcT>: 整型值，当前网络的接入技术。
     *             0：GSM；
     *             1：GSM Compact；
     *             2：UTRAN；
     *             3：GSM EGPRS；
     *             4：UTRAN HSDPA；
     *             5：UTRAN HSUPA；
     *             6：UTRAN HSDPA和HSUPA；
     *             7：E-UTRAN；
     * [示例]:
     *     · 设置CS域注册状态上报
     *       AT+CREG=1
     *       OK
     *     · <n>=1时查询注册状态
     *       AT+CREG?
     *       +CREG: 1,1
     *       OK
     *     · <n>=2时查询注册状态
     *       AT+CREG?
     *       +CREG: 2,1,"00C3","0000001A",4
     *       OK
     *     · 测试CREG
     *       AT+CREG=?
     *       +CREG: (0-2)
     *       OK
     */
    { AT_CMD_CREG,
      At_SetCregPara, AT_SET_PARA_TIME, At_QryCregPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CREG", (VOS_UINT8 *)"(0-2)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询PS域注册状态
     * [说明]: 查询当前PS域网络注册状态，及设置状态上报方式。
     * [语法]:
     *     [命令]: +CGREG=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGREG?
     *     [结果]: <CR><LF>+CGREG: <n>,<stat>[,<lac>,<ci>[,<AcT>]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGREG=?
     *     [结果]: <CR><LF>+CGREG: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，主动上报方式，默认值为0。
     *             0：禁止+CGREG的主动上报；
     *             1：使用+CGREG: <stat>的主动上报方式；
     *             2：使用+CGREG: <stat>[,<lac>,<ci>[,<AcT>]]的主动上报方式。
     *     <stat>: 整型值，注册状态。
     *             0：没有注册，MT现在没有搜索和注册新的运营商；
     *             1：注册了归属网络；
     *             2：没有注册，MT正在搜索并注册新的运营商；
     *             3：注册被拒绝；
     *             4：未知状态；
     *             5：注册了漫游网络。
     *     <lac>: 2byte的16进制数的字符串类型，位置码信息，例如：“00C3”表示10进制的195。
     *     <ci>: 4byte的16进制数的字符串类型，小区信息。
     *     <AcT>: 整型值，当前网络的接入技术。
     *             0：GSM；
     *             1：GSM Compact；
     *             2：UTRAN；
     *             3：GSM EGPRS；
     *             4：UTRAN HSDPA；
     *             5：UTRAN HSUPA；
     *             6：UTRAN HSDPA和HSUPA；
     *             7：E-UTRAN；
     *             10：EUTRAN-5GC；
     *             11：NR-5GC；
     * [示例]:
     *     · 设置PS域注册状态上报
     *       AT+CGREG=1
     *       OK
     *     · <n>=1时查询注册状态
     *       AT+CGREG?
     *       +CGREG: 1,1
     *       OK
     *     · <n>=2时查询注册状态
     *       AT+CGREG?
     *       +CGREG: 2,1,"00C3","000001A",4
     *       OK
     *     · 测试CGREG
     *       AT+CGREG=?
     *       +CGREG: (0-2)
     *       OK
     */
    { AT_CMD_CGREG,
      At_SetCgregPara, AT_SET_PARA_TIME, At_QryCgregPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CGREG", (VOS_UINT8 *)"(0-2)" },

#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: EPS域注册状态
     * [说明]: SET命令控制+CEREG主动上报事件的方式。
     *         当<n>=1，网络注册的状态发生改变的时候，上报+CEREG:<stat>
     *         当<n>=2，小区信息发生改变时，上报+CEREG: <stat>[,<lac>,<ci>[,<AcT>]]
     *         读命令返回当前的注册状态<stat>，位置信息<lac>,<ci>仅当<n>=2时上报。
     *         本命令遵从3GPP TS 27.007协议。
     *         该命令仅在单板支持LTE时才能使用（即当前单板支持LTE能力）。
     * [语法]:
     *     [命令]: +CEREG=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: +CEREG?
     *     [结果]: <CR><LF>+CEREG: <n>,<stat>[,<lac>,<ci>[,<AcT>]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             备注：n=2时根据当前注册的接入技术不同，查询结果不同
     *             UE注册在G/U上时查询命令结果只上报<stat>参数，即
     *             <CR><LF>+CEREG: <n>,<stat><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             UE注册在L上时其他参数正常上报。
     *     [命令]: +CEREG=?
     *     [结果]: <CR><LF>+CEREG: (list of supported <n>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，默认值为0。
     *             0：禁止+CEREG的主动上报；
     *             1：使能+CEREG: <stat>的主动上报；
     *             2：使能+CEREG: <stat>[,<lac>,<ci>[,<AcT>]]的主动上报。
     *     <stat>: 0：没有注册，MT现在并没有在搜寻要注册的新的运营商；
     *             1：注册了本地网络；
     *             2：没有注册，但MT正在搜寻要注册的新的运营商；
     *             3：注册被拒绝；
     *             4：未知原因；
     *             5：注册了漫游网络。
     *     <lac>: 位置码信息，四个字符，16进制表示。（例：“00C3”＝10进制的195）
     *     <ci>: 小区信息，八个字符，16进制表示。
     *     <AcT>: 整型值，当前网络的接入技术。
     *             0：GSM；
     *             1：GSM Compact；
     *             2：UTRAN；
     *             3：GSM EGPRS；
     *             4：UTRAN HSDPA；
     *             5：UTRAN HSUPA；
     *             6：UTRAN HSDPA 和HSUPA；
     *             7：E-UTRAN；
     *             10：EUTRAN-5GC；
     *             11：NR-5GC。
     * [示例]:
     *     · 设置EPS域注册状态上报
     *       AT+CEREG=1
     *       OK
     *     · <n>=1时查询注册状态
     *       AT+CEREG?
     *       +CEREG: 1,1
     *       OK
     *     · <n>=2时注册在L上时查询注册状态
     *       AT+CEREG?
     *       +CEREG: 2,1,"00C3","0000001A",7
     *       OK
     *     · <n>=2注册在U(或G，NR)上时查询注册状态
     *       AT+CEREG?
     *       +CEREG: 2,1
     *       OK
     *     · 测试CEREG
     *       AT+CEREG=?
     *       +CEREG: (0-2)
     *       OK
     */
    { AT_CMD_CEREG,
      AT_SetCeregPara, AT_SET_PARA_TIME, At_QryCeregPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CEREG", (VOS_UINT8 *)"(0-2)" },
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 5GC注册状态
     * [说明]: 查询当前5G网络注册状态，及设置状态上报方式。
     *         该命令仅在单板支持NR时才能使用（即当前单板支持NR能力）。
     * [语法]:
     *     [命令]: +C5GREG=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +C5GREG?
     *     [结果]: <CR><LF>+C5GREG: <n>,<stat>[,[<tac>],[<ci>],[<AcT>],[<Allowed_NSSAI_length>],[<Allowed_NSSAI>] ]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             备注：
     *             n=0时查询命令结果上报<n>,<stat>参数。
     *             n=1时查询命令结果上报<n>,<stat>参数。
     *             n=2时根据当前注册的接入技术不同，查询结果不同
     *             UE注册在G/U/L上时查询命令结果上报<n>,<stat>，即
     *             <CR><LF>+C5GREG: <n>,<stat><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             UE注册在NR上时其他参数正常上报。
     *     [命令]: +C5GREG=?
     *     [结果]: <CR><LF>+C5GREG: (list of supported <n>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，默认值为0。
     *             0：禁止+C5GREG的主动上报；
     *             1：使能+C5GREG: <stat>的主动上报；
     *             2：使能+C5GREG:
     *             <stat>[,[<tac>],[<ci>],[<AcT>],[<Allowed_NSSAI_length>],
     *             [<Allowed_NSSAI>] ]的主动上报。
     *     <stat>: 0：没有注册，MT现在并没有在搜寻要注册的新的运营商；
     *             1：注册了本地网络；
     *             2：没有注册，但MT正在搜寻要注册的新的运营商；
     *             3：注册被拒绝；
     *             4：未知原因；
     *             5：注册了漫游网络。
     *             8：registered for emergency services only；
     *     <tac>: 位置码信息，六个字符，16进制表示。（例：“0000C3”＝10进制的195）
     *     <ci>: 小区信息，8个字节，16个字符，16进制表示。使用Half-Byte码（半字节）编码，例如000000010000001A，表示高4字节是0x00000001，低4字节是0x0000001A。
     *     <AcT>: 整型值，当前网络的接入技术。
     *             10：EUTRAN-5GC
     *             11：NR-5GC
     *     <Allowed_NSSAI_length>: 整型值，表示<Allowed_NSSAI>信息中切片字节个数；
     *             无<Allowed_NSSAI>信息时值为0。
     *     <Allowed_NSSAI>: 字符串类型。每个S-NSSAI使用“:”区分。单个S-NSSAI的格式
     *             "sst" if only slice/service type (SST) is present
     *             "sst;mapped_sst" if SST and mapped configured SST are present
     *             "sst.sd" if SST and slice differentiator (SD) are present
     *             "sst.sd;mapped_sst" if SST, SD and mapped configured SST are present
     *             "sst.sd;mapped_sst.mapped_sd" if SST, SD, mapped configured SST and mapped configured SD are present
     *             注意：字符串中的数据为16进制，字符取值范围：0-9，A-F，a-f。
     *             无Allowed_NSSAI列表信息时值为空。
     * [示例]:
     *     · 设置NRPS域注册状态上报
     *       AT+C5GREG=1
     *       OK
     *     · <n>=1时查询注册状态
     *       AT+C5GREG?
     *       +C5GREG: 1,1
     *       OK
     *     · <n>=2时注册在NR上时查询注册状态
     *       AT+C5GREG?
     *       +C5GREG: 2,1,"0000C3","000000010000001A",11,9,"01:01.123456;2F.654321"
     *       OK
     *     · <n>=2注册在U(或G)上时查询注册状态
     *       AT+C5GREG?
     *       +C5GREG: 2,1
     *       OK
     *     · 测试C5GREG
     *       AT+C5GREG=?
     *       +C5GREG: (0-2)
     *       OK
     */
    { AT_CMD_C5GREG,
      AT_SetC5gregPara, AT_SET_PARA_TIME, At_QryC5gregPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+C5GREG", (VOS_UINT8 *)"(0-2)" },
#endif

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 定义主PDP上下文
     * [说明]: 设置或删除主PDP上下文。
     *         本命令遵从3GPP TS 27.007协议。
     *         如果命令中只有<cid>一个参数，如AT+CGDCONT=<cid>，表示删除此<cid>对应的主PDP上下文。
     *         V9R1 <CID 0>不支持删除操作。
     * [语法]:
     *     [命令]: +CGDCONT=[<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>[,<IPv4AddrAlloc>[,<emergencyindication>[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>[,<NSLPI>[,<securePCO>[,<IPv4_MTU_discovery>[,<Local_Addr_Ind>[,<Non-IP_MTU_discovery>[,<Reliable_Data_Service>[,<SSC_mode>[,<S-NSSAI>[,<Pref_access_type>[,<RQoS_ind>[,<MH6-PDU>[,<Always-onind>]]]]]]]]]]]]]]]]]]]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGDCONT?
     *     [结果]: <CR><LF>[+CGDCONT: <cid>,<PDP_type>,<APN>,<PDP_addr>,<d_comp>,<h_comp>[,<IPv4AddrAlloc>[,<emergencyindication>[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>[,<NSLPI>[,<securePCO>[,<IPv4_MTU_discovery>[,<Local_Addr_Ind>[,<Non-IP_MTU_discovery>[,<Reliable_Data_Service>[,<SSC_mode>[,<S-NSSAI>[,<Pref_access_type>[,<RQoS_ind>[,<MH6-PDU>[,<Always-on ind>]]]]]]]]]]]]]]]]
     *             [<CR><LF>+CGDCONT: <cid>,<PDP_type>,<APN>,<PDP_addr>,<d_comp>,<h_comp>[,<IPv4AddrAlloc>[,<emergencyindication>[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>[,<NSLPI>[,<securePCO>[,<IPv4_MTU_discovery>[,<Local_Addr_Ind>[,<Non-IP_MTU_discovery>[,<Reliable_Data_Service>[,<SSC_mode>[,<S-NSSAI>[,<Pref_access_type>[,<RQoS_ind>[,<MH6-PDU>[,<Always-on ind>]]]]]]]]]]]]]]]]
     *             [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGDCONT=?
     *     [结果]: <CR><LF>+CGDCONT: ( range of supported <cid>s),<PDP_type>,,,(list of supported <d_comp>s),(list of supported <h_comp>s), (list of supported <IPv4AddrAlloc>s),(list of supported <emergency indication>s)[,(list of supported <P-CSCF_discovery>s),(list of supported <IM_CN_Signalling_Flag_Ind>s) ,(list of supported < NSLPI >s)[,,,,,,(list of supported <SSC_mode>s),,(list of supported <Pref_access_type>s),(list of supported <RQoS_ind>s),(list of supported <MH6-PDU>s) ,(list of supported<Always-on ind>s)]]
     *             [<CR><LF>+CGDCONT: (range of supported <cid>s), <PDP_type>,,,(list of supported <d_comp>s),(list of supported <h_comp>s) ,(list of supported <IPv4AddrAlloc>s),(list of supported <emergencyindication>s)[,(list of supported <P-CSCF_discovery>s),(list of supported <IM_CN_Signalling_Flag_Ind>s) ,(list of supported < NSLPI >s) [,,,,,,(list of supported <SSC_mode>s),,(list of supported <Pref_access_type>s),(list of supported <RQoS_ind>s),(list of supported <MH6-PDU>s) ,(list of supported<Always-on ind>s)]]
     *             [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             支持版本的取值范围0~31。
     *     <PDP_type>: 字符串类型，分组数据协议类型。
     *             IP：网际协议（IPV4）。
     *             IPV6：IPV6协议。
     *             IPV4V6：IPV4和IPV6。
     *             PPP：PPP协议。
     *             Ethernet：以太网协议。（此类型的设置受NV6066控制）
     *     <APN>: 字符串类型，接入点名称，用于选择GGSN或外部分组数据网络；若该值为空，则使用签约值。
     *     <PDP_address>: 字符串类型，IPV4类型的PDP地址。
     *     <d_comp>: 整型值，PDP数据压缩方式（仅对SNDCP有效）。
     *             0：关（缺省值）；
     *             1：开（制造商指定）；
     *             2：V.42；
     *             3：V.44（暂不支持）。
     *     <h_comp>: 整型值，PDP头压缩方式。
     *             0：关（缺省值）；
     *             1：开（制造商指定）；
     *             2：RFC1144（仅对SNDCP有效）；
     *             3：RFC2507；
     *             4：RFC3095（仅对PDCP有效，暂不支持）。
     *     <IPv4AddrAlloc>: 控制获取IPV4地址的方式（暂不支持）。
     *             0：通过NAS信令分配IPV4地址；
     *             1：通过DHCP分配IPV4地址。
     *     <emergency indication>: PDP是否用于紧急承载服务（暂不支持）。
     *             0：PDP不用于紧急承载服务；
     *             1：PDP用于紧急承载服务。
     *     <P-CSCF_discovery>: 整形值，控制MT/TA获取P-CSCF 地址的方式。
     *             0：不受+CGDCONT控制
     *             1：优先通过NAS信令获取
     *             2：优先通过DHCP获取
     *     <IM_CN_Signalling_Flag_Ind>: 整形值，指示网络此PDP是否仅用于 IM CN
     *             0：此PDP是否不仅用于 IM CN
     *             1：此PDP是否仅用于 IM CN
     *     <NSLPI>: 整型值，指示PDP的NAS 信令的优先级:
     *             0: PDP上下文在激活时，其低优先级参数由MT设置.
     *             1: PDP上下文在激活时，其低优先级参数需设置为 "MS is not configured for NAS ignalling low priority".（默认值）
     *     <securePCO>: 整型值，Specifies if security protected transmission of PCO is requested or not (applicable for EPS only)
     *             0：Security protected transmission of PCO is not requested
     *             1：Security protected transmission of PCO is requested
     *             注：该参数暂不支持
     *     <IPv4_MTU_discovery>: 整型值，influences how the MT/TA requests to get the IPv4 MTU size
     *             0：Preference of IPv4 MTU size discovery not influenced by +CGDCONT
     *             1：Preference of IPv4 MTU size discovery through NAS signalling
     *             注：该参数暂不支持
     *     <Local_Addr_Ind>: 整型值，indicates to the network whether or not the MS supports local IP address in TFTs
     *             0：indicates that the MS does not support local IP address in TFTs
     *             1：indicates that the MS supports local IP address in TFTs
     *             注：该参数暂不支持
     *     <Non-IP_MTU_discovery>: 整型值，influences how the MT/TA requests to get the Non-IP MTU size
     *             0：Preference of Non-IP MTU size discovery not influenced by +CGDCONT
     *             1：Preference of Non-IP MTU size discovery through NAS signalling
     *             注：该参数暂不支持
     *     <Reliable_Data_Service>: 整型值，indicates whether the UE is using Reliable Data Service for a PDN connection or not
     *             0：Reliable Data Service is not being used for the PDN connection
     *             1：Reliable Data Service is being used for the PDN connection
     *             注：该参数暂不支持
     *     <SSC_mode>: 整型值，数据业务的会话和服务连续性模式
     *             0：SSC mode 1
     *             1：SSC mode 2
     *             2：SSC mode 3
     *     <S-NSSAI>: 字符串类型，5GS的分片信息，
     *             "sst" if only slice/service type (SST) is present
     *             "sst;mapped_sst" if SST and mapped configured SST are present
     *             "sst.sd" if SST and slice differentiator (SD) are present
     *             "sst.sd;mapped_sst" if SST, SD and mapped configured SST are present
     *             "sst.sd;mapped_sst.mapped_sd" if SST, SD, mapped configured SST and mapped configured SD are present
     *             注意：字符串中的数据为16进制，字符取值范围：0-9，A-F,a-f。
     *     <Pref_access_type>: 整型值，5GS系统中优先接入类型
     *             0：3GPP access
     *             1：non-3GPP access
     *             注：该参数只支持0
     *     <RQoS_ind>: 整型值，PDU session是否支持反射QoS，默认值0
     *             0：indicates that reflective QoS is not supported for the PDU session
     *             1：indicates that reflective QoS is supported for the PDU session
     *     <MH6-PDU>: 整型值，UE建立PDU session时是否支持IPv6 multi-homing
     *             0：不支持
     *             1：支持
     *             注：该参数只支持0
     *     <Always-on ind>: 整型值，是否为  always-on PDU session
     *             0：否
     *             1：是
     * [示例]:
     *       定义主PDP上下文
     *     · 不支持5GS时
     *       AT+CGDCONT=1,"IP","ABCDEF","200.1.1.80",1,2,0,0,0,0,0
     *       OK
     *     · 支持5GS时
     *       AT+CGDCONT=1,"IP","ABCDEF","200.1.1.80",1,2,0,0,0,0,0,,,,,,1,"1.123456;2F.654321",0,1,0,0
     *       OK
     *       查询命令
     *     · 不支持5GS时
     *       AT+CGDCONT?
     *       +CGDCONT: 1,"IP","ABCDEF","200.1.1.80",1,2,0,0,0,0,0
     *       +CGDCONT: 2,"IP6","abc.com",,0,0,0,0,0,0
     *       +CGDCONT: 3,"IPV4V6","huawei.com",,0,0,0,0,1,1
     *       OK
     *     · 支持5GS时
     *       AT+CGDCONT?
     *       +CGDCONT: 1,"IP","ABCDEF","200.1.1.80",1,2,0,0,0,0,0,,,,,,
     *       1,"1.1234",0,1,0,0
     *       +CGDCONT: 2,"IP6","abc.com",,0,0,0,0,0,0,,,,,,1,"1.1234",
     *       0,0,0,0
     *       +CGDCONT: 3,"IPV4V6","huawei.com",,0,0,0,0,1,1,,,,,,
     *       0,"1.1234",0,1,0,0
     *       +CGDCONT: 4,"Ethernet","huawei.com",,0,0,0,0,0,1
     *       OK
     *       测试命令（支持IPV6特性时）
     *     · 不支持5GS时
     *       AT+CGDCONT=?
     *       +CGDCONT: (0-31),"IP",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)
     *       +CGDCONT: (0-31),"IPV6",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)
     *       +CGDCONT: (0-31),"IPV4V6",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)
     *       +CGDCONT: (0-31),"PPP",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1)
     *       OK
     *     · 支持5GS且NV6066使能时
     *       AT+CGDCONT=?
     *       +CGDCONT: (0-31),"IP",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0-1),(0-1),(0-1),(0-1)
     *       +CGDCONT: (0-31),"IPV6",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0-1),(0-1),(0-1),(0-1)
     *       +CGDCONT: (0-31),"IPV4V6",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0-1),(0-1),(0-1),(0-1)
     *       +CGDCONT: (0-31),"PPP",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0-1),(0-1),(0-1),(0-1)
     *       (0-31),"Ethernet",,,(0-2),(0-3),(0,1),(0,1),(0-2),(0,1),(0,1),,,,,,(0-2),,(0-1),(0-1),(0-1),(0-1)
     *       OK
     */
    { AT_CMD_CGDCONT,
      AT_SetCgdcontPara, AT_SET_PARA_TIME, AT_QryCgdcontPara, AT_QRY_PARA_TIME, At_TestCgdcont, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGDCONT", (VOS_UINT8 *)CGDCONT_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 定义二次PDP上下文
     * [说明]: 设置或删除二次PDP上下文。
     *         如果命令中只有<cid>一个参数，如AT+CGDSCONT=<cid>，表示删除此<cid>对应的二次PDP上下文。
     *         一个PDP上下文不能既是主PDP上下文，又是二次PDP上下文。若对某CID先后执行了+CGDCONT命令和+CGDSCONT命令，则最后一次执行的AT命令有效。
     *         二次PDP上下文和其对应的主PDP上下文都只能是IP类型的PDP上下文。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGDSCONT=[<cid>,<p_cid>[,<d_comp>[,<h_comp>[,<IM_CN_Signalling_Flag_Ind>]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGDSCONT?
     *     [结果]: <CR><LF>+CGDSCONT: <cid>,<p_cid>,<d_comp>,<h_comp>
     *             [<CR><LF>+CGDSCONT: <cid>,<p_cid>,<d_comp>,<h_comp>,<IM_CN_Signalling_Flag_Ind> [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGDSCONT=?
     *     [结果]: <CR><LF>+CGDSCONT: (list of supported <cid>s),(list of <p_cid>s for active primary contexts),(list of supported <d_comp>s),(list of supported <h_comp>s) ,(list of
     *             supported <IM_CN_Signalling_Flag_Ind>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             V9R1版本的取值范围1~31。
     *     <p_cid>: 整型值，主PDP上下文标识符。
     *             V9R1版本的取值范围0~31。
     *     <d_comp>: 整型值，PDP数据压缩方式（仅对SNDCP有效）。
     *             0：关（缺省值）；
     *             1：开（制造商指定）；
     *             2：V.42；
     *             3：V.44（暂不支持）。
     *     <h_comp>: 整型值，PDP头压缩方式。
     *             0：关（缺省值）；
     *             1：开（制造商指定）；
     *             2：RFC1144（仅对SNDCP有效）；
     *             3：RFC2507；
     *             4：RFC3095（仅对PDCP有效，暂不支持）。
     *     <IM_CN_Signalling_Flag_Ind>: IMS承载标志：
     *             0：不是与IM CN子系统相关的信号
     *             1：是与IM CN子系统相关的信号
     * [示例]:
     *     · 定义二次PDP上下文
     *       AT+CGDSCONT=2,1,1,2,0
     *       OK
     *     · 查询命令
     *       AT+CGDSCONT?
     *       +CGDSCONT: 2,1,1,2,0
     *       +CGDSCONT: 3,1,1,0
     *       OK
     *     · 测试命令
     *       AT+CGDSCONT=?
     *       +CGDSCONT: (1-31),(0-31),(0-2),(0-3),(0-1)
     *       OK
     */
    { AT_CMD_CGDSCONT,
      AT_SetCgdscontPara, AT_SET_PARA_TIME, AT_QryCgdscontPara, AT_QRY_PARA_TIME, At_TestCgdscont, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGDSCONT", (VOS_UINT8 *)CGDSCONT_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置TFT
     * [说明]: 设置或删除PDP上下文的报文过滤器（每个PDP上下文只支持一个报文过滤器）。
     *         如果命令中只有<cid>一个参数，如AT+CGTFT=<cid>，表示删除此<cid>对应的PDP上下文的报文过滤器。
     *         本命令遵从3GPP TS 27.007协议。
     *         该命令所设置的参数组合遵循表2-16中的组合要求。
     *         注：表格中的“X”符号表示该类有效组合中选定该参数，为空则表示该类有效组合中不能选定该参数。
     * [语法]:
     *     [命令]: +CGTFT=[<cid>,[<packetfilteridentifier>,<evaluationprecedenceindex>[,<remoteaddressandsubnetmask>[,<protocolnumber(ipv4)/nextheader(ipv6)>[,<sourceportrange>[,<destinationportrange>[,<ipsecsecurityparameterindex(spi)>[,<typeofservice(tos)(ipv4)andmask/trafficclass(ipv6)andmask>[,<flowlabel(ipv6)>[,<direction>[,<localaddressandsubnetmask>[,<QRI>]]]]]]]]]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGTFT?
     *     [结果]: <CR><LF> [+CGTFT: <cid>, <packet filter identifier>, <evaluation precedence index>, < remote address and subnet mask>, <protocol number (ipv4) / next header (ipv6)>, <source port range>,<destination port range>,<ipsec security parameter index (spi)>,<type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>,<flow label (ipv6)>,<direction>,<local address and subnet mask>,<QRI>]
     *             [<CR><LF>+CGTFT: <cid>, <packet filter identifier>, <evaluation precedence index>, < remote address and subnet mask>, <protocol number (ipv4) / next header (ipv6)>,<source port range>,<destination port range>,<ipsec security parameter index (spi)>,<type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>,<flow label (ipv6)>,<direction>,<local address and subnet mask>,<QRI>]
     *             [...]] <CR><LF>
     *     [命令]: +CGTFT=?
     *     [结果]: <CR><LF>+CGTFT: <PDP_type>, (list of supported <packet filter identifier>s), (list of supported <evaluation precedence index>s), (list of supported < remote address and subnet mask>s), (list of supported <protocol number (ipv4) / next header (ipv6)>s), (list of supported<source port range>s),(list of supported<destination port range>s) ,(list of supported <ipsec security parameter index (spi)>s),(list of supported <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>s),(list of supported <flow label (ipv6)>s),(list of supported <direction>s),(list of supported <local address and subnet mask>s),(range of supported <QRI>s)
     *             [<CR><LF>+CGTFT: <PDP_type>, (list of supported <packet filter identifier>s), (list of supported <evaluation precedence index>s), (list of supported < remote address and subnet mask>s), ((list of supported <protocol number (ipv4) / next header (ipv6)>s), (list of supported<source port range>s),(list of supported<destination port range>s),(list of supported <ipsec security parameter index (spi)>s),(list of supported <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>s),(list of supported <flow label (ipv6)>s),(list of supported <direction>s),(list of supported <local address and subnet mask>s),(range of supported <QRI>s)
     *             [...]] <CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             V9R1版本的取值范围1~31。
     *     <packet filter identifier>: 整型值，报文过滤器标识，取值范围为1～16，目前仅支持1。
     *     <remote address and subnet mask>: IP地址字符串类型，远端IP地址及子网掩码，目前仅支持IPV4地址类型，格式为“a1.a2.a3.a4.m1.m2.m3.m4”。
     *     <protocol number (ipv4) / next header (ipv6)>: 整型值，IPV4的协议号，IPV6 next header，取值范围为0～255。
     *     <source port range>: 字符串类型，源端口范围，取值范围为0～65535，格式为“from.to”。
     *     <destination port range>: 字符串类型，目标端口范围，取值范围为0～65535，格式为“from.to”。
     *     <ipsec security parameter index (spi)>: 字符串类型的16进制数，ipsec安全参数索引，取值范围为0x0000_0000～0xFFFF_FFFF。
     *     <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>: 字符串类型，（IPV4）服务类型，（IPV6）业务流类别和掩码，取值范围为0～255，格式为“tos.mask”。
     *     <evaluation precedence index>: 整型值，优先级，取值范围为0～255。
     *     <flow label (ipv6)>: 流标签，取值范围0~1048575。
     *     <direction>: 过滤的方向。
     *             0：R7之前协议使用；
     *             1：上行；
     *             2：下行；
     *             3：双向。
     *     <local address and subnet mask>: IP地址字符串类型，本地IP地址及子网掩码，目前仅支持IPV4地址类型，格式为“a1.a2.a3.a4.m1.m2.m3.m4”。
     *             注意：只在R11以上版本支持此参数
     *     <QRI>: 整型值，Qos rule的identity，协议支持0-255.
     *             注意：只在R15以上版本支持此参数
     * [示例]:
     *     · 设置命令
     *       AT+CGTFT=1,1,1,"10.10.10.10.255.255.0.0",,,,,"255.255"
     *       OK
     *     · 查询命令
     *       AT+CGTFT?
     *       +CGTFT: 1,1,1,"10.10.10.10.255.255.0.0",,,,,"255.255",,0,0
     *       OK
     *     · 测试命令（R11以下版本）
     *       AT+CGTFT=?
     *       +CGTFT: "IP",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3)
     *       +CGTFT: "IPV6",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3)
     *       +CGTFT: "IPV4V6",(1-16),(0-255),,(0-255),,,(00000000-FFFFFFFF),,(00000-FFFFF),(0-3)
     *       OK
     */
    { AT_CMD_CGTFT,
      AT_SetCgtftPara, AT_SET_PARA_TIME, AT_QryCgtftPara, AT_QRY_PARA_TIME, At_TestCgtft, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGTFT", (VOS_UINT8 *)CGTFT_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 激活PDP
     * [说明]: 激活或去激活<cid>标识的PDP上下文。
     *         目前Balong不支持一次激活多个PDP，目前一次只支持激活一个PDP。
     *         若之前通过PPP拨号的方式激活了PDP，则不能使用该AT命令进行相应PDP的去激活操作。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGACT=[<state>,<cid>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGACT?
     *     [结果]: <CR><LF>+CGACT: <cid>,<state>
     *             [<CR><LF>+CGACT: <cid>,<state>[...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGACT=?
     *     [结果]: <CR><LF>+CGACT: (list of supported <state>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值，指示PDP上下文激活的状态。
     *             0：未激活；
     *             1：已激活。
     *     <cid>: 整型值，PDP上下文标识符。
     *             V9R1版本的取值范围1~31。
     * [示例]:
     *     · 激活、去激活PDP上下文
     *       AT+CGACT=1,1
     *       OK
     *       AT+CGACT=0,1
     *       OK
     *     · 测试命令
     *       AT+CGACT=?
     *       +CGACT: (0,1)
     *       OK
     *       查询命令
     *       注:下属示例仅供参考，实际返回结果与用户实际使用cid情况有关 。
     *     · cid21－31默认返回
     *       AT+CGACT?
     *       +CGACT: 1,1
     *       +CGACT: 21,0
     *       +CGACT: 22,0
     *       +CGACT: 23,0
     *       +CGACT: 24,0
     *       +CGACT: 25,0
     *       +CGACT: 26,0
     *       +CGACT: 27,0
     *       +CGACT: 28,0
     *       +CGACT: 29,0
     *       +CGACT: 30,0
     *       +CGACT: 31,0
     *       OK
     */
    { AT_CMD_CGACT,
      AT_SetCgactPara, AT_ACT_PDP_TIME, AT_QryCgactPara, AT_QRY_PARA_TIME, At_TestCgact, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGACT", (VOS_UINT8 *)CGACT_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 修改PDP
     * [说明]: 当PDP处于激活状态，使用+CGEQREQ或+CGTFT命令设置PDP上下文的QoS或TFT（Traffic Flow Template）后，其设置值不会立即生效。使用+CGCMOD可修改<cid>所标识的PDP上下文，使QoS或TFT设置值生效。
     *         目前Balong不支持一次修改多个PDP，目前一次只支持修改一个PDP。
     *         NR模式下，使用此命令新增一个QOS RULE，或者一个或多个PF（不超过4个）；暂不支持使用此命令修改网络下发的TFT参数；不支持对于同一个CID做多次MODIFY操作。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGCMOD=<cid>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGCMOD=?
     *     [结果]: <CR><LF>+CGCMOD: (list of <cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             支持LTE的平台取值范围为1~31。
     *             不支持LTE的平台取值1~11。
     * [示例]:
     *     · 发起PDP上下文修改
     *       AT+CGCMOD=1
     *       OK
     *     · 测试命令
     *       AT+CGCMOD=?
     *       +CGCMOD: (1)
     *       OK
     */
    { AT_CMD_CGCMOD,
      AT_SetCgcmodPara, AT_MOD_PDP_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgcmod, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGCMOD", (VOS_UINT8 *)CGCMOD_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询PDP地址
     * [说明]: 查询<cid>所标识的PDP上下文实际使用的IP地址，如果不输入<cid>则返回所有激活的PDP上下文实际使用的IP地址。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGPADDR[=<cid>[,<cid>[,...]]]
     *     [结果]: [<CR><LF>+CGPADDR: <cid>,<PDP_addr>
     *             [<CR><LF>+CGPADDR: <cid>,<PDP_addr>[…]] <CR><LF>]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGPADDR
     *     [结果]: [<CR><LF>+CGPADDR: <cid>,<PDP_addr>
     *             [<CR><LF>+CGPADDR: <cid>,<PDP_addr>[…]] <CR><LF>]
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGPADDR=?
     *     [结果]: <CR><LF>+CGPADDR: (list of defined <cid>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *     <PDP_addr>: 字符串类型，PDP地址。
     * [示例]:
     *     · 获取某个IPV4已激活的PDP上下文使用的PDP地址
     *       AT+CGPADDR=1
     *       +CGPADDR: 1,"10.101.2.15"
     *       OK
     *     · 获取某个IPV6已激活的PDP上下文使用的PDP地址
     *       AT+CGPADDR=1
     *       +CGPADDR: 1,"32.8.0.2.0.2.0.1.255.255.255.255.255.255.255.255"
     *       OK
     *     · 测试命令：列出所有已经定义的CID
     *       AT+CGDCONT=1,"ip","huawei.com"
     *       OK
     *       AT+CGDCONT=2,"ip","huawei1.com"
     *       OK
     *       AT+CGPADDR=?
     *       +CGPADDR: (1,2)
     *       OK
     */
    { AT_CMD_CGPADDR,
      AT_SetCgpaddrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgpaddr, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGPADDR", (VOS_UINT8 *)CGPADDR_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询协商QoS
     * [说明]: 查询<cid>所标识的PDP上下文激活后经过与网络协商后而实际使用的QoS。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGEQNEG[=<cid>[,<cid>[,...]]]
     *     [结果]: <CR><LF> [+CGEQNEG: <cid>, <Traffic class> ,<Maximum bitrate UL>,<Maximum bitrate DL> ,<Guaranteed bitrate UL>, <Guaranteedbitrate DL> ,<Delivery order> ,<Maximum SDU size> ,<SDU error ratio> ,<Residual bit error ratio> ,<Delivery of erroneous SDUs> ,<Transfer delay> ,<Traffic handling priority>[…]] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGEQNEG=?
     *     [结果]: <CR><LF>+CGEQNEG: (list of <cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             V9R1版本的取值范围1~31。
     *     <Traffic class>: 整型值，业务类型。
     *             0：Conversational（会话类型）；
     *             1：Streaming（流类型）；
     *             2：Interactive（交互类型）；
     *             3：Background（背景类型）；
     *             4：Subscribed value（签约值）。
     *     <Maximum bitrate UL>: 整型值，上行最大比特率（单位为kbit/s），取值范围为0～10000000。
     *             0：协商值；
     *             1～10000000：1kbit/s～10000000kbit/s。
     *     <Maximum bitrate DL>: 整型值，下行最大比特率（单位为kbit/s），取值范围为0～10000000。
     *             0：协商值；
     *             1～10000000：1kbit/s～10000000kbit/s。
     *     <Guaranteed bitrate UL>: 整型值，上行可保证比特率（单位为kbit/s），取值范围为0～10000000。
     *             0：协商值；
     *             1～10000000：1kbit/s～10000000  kbit/s。
     *     <Guaranteed bitrate DL>: 整型值，下行可保证比特率（单位为kbit/s），取值范围为0～10000000。
     *             0：协商值；
     *             1～10000000：1kbit/s～10000000kbit/s。
     *     <Delivery order>: 整型值，是否顺序发送SDU。
     *             0：无顺序性；
     *             1：有顺序性；
     *             2：协商值。
     *     <Maximum SDU size>: 整型值，最大SDU长度（单位为byte），取值范围为0～1520。
     *             0：协商值；
     *             1～1520：1byte～1520byte。
     *     <SDU error ratio>: 字符串类型，SDU误码率。
     *             0E0：协商值；
     *             1E2：1*10-2；
     *             7E3：7*10-3；
     *             1E3：1*10-3；
     *             1E4：1*10-4；
     *             1E5：1*10-5；
     *             1E6：1*10-6；
     *             1E1：1*10-1。
     *     <Residual bit error ratio>: 字符串类型，SDU残留比特误码率。
     *             0E0：协商值；
     *             5E2：5*10-2；
     *             1E2：1*10-2；
     *             5E3：5*10-3；
     *             4E3：4*10-3；
     *             1E3：1*10-3；
     *             1E4：1*10-4；
     *             1E5：1*10-5；
     *             1E6：1*10-6；
     *             6E8：6*10-8。
     *     <Delivery of erroneous SDUs>: 整型值，错误的SDU是否被发送。
     *             0：不发送；
     *             1：发送；
     *             2：不检测；
     *             3：协商值。
     *     <Transfer delay>: 整型值，传输时延（单位为ms），取值范围为0～4100。
     *             0：协商值；
     *             1～4100：1ms～4100ms。
     *     <Traffic handling priority>: 整型值，优先级。
     *             0：协商值；
     *             1：优先级1；
     *             2：优先级2；
     *             3：优先级3。
     *     <PDP_type>: 字符串类型，分组数据协议类型。
     *             IP：网际协议（IPV4）。
     *             IPV6：IPV6协议。
     *             IPV4V6：IPV4和IPV6。
     * [示例]:
     *     · 查询PDP上下文激活后实际使用的QoS
     *       AT+CGEQNEG=1
     *       +CGEQNEG: 1,2,64,64,0,0,1,320,"1E4","1E5",1,0,3
     *       OK
     *     · 测试命令
     *       AT+CGEQNEG=?
     *       +CGEQNEG: (1)
     *       OK
     */
    { AT_CMD_CGEQNEG,
      AT_SetCgeqnegPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgeqnegPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGEQNEG", (VOS_UINT8 *)CGEQNEG_CMD_PARA_STRING },

    { AT_CMD_CGANS,
      AT_SetCgansPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgansPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGANS", (VOS_UINT8 *)CGANS_CMD_PARA_STRING },

    { AT_CMD_CGANS_EXT,
      AT_SetCgansPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgansExtPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CGANS", (VOS_UINT8 *)CGANS_EXT_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 进入数传状态
     * [说明]: 进入数传状态，如果<cid>所标识的PDP上下文没有激活，则先激活再进入数传状态。
     *         本命令遵从3GPP TS 27.007协议。
     *         注意：无论在那个端口发送该AT命令，都会使该端口从命令态切换成数传态，此后该端口将不能处理AT命令。
     * [语法]:
     *     [命令]: +CGDATA=[<L2P>,[<cid>]]
     *     [结果]: <CR><LF>CONNECT<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGDATA=?
     *     [结果]: <CR><LF>+CGDATA: (list of supported <L2P>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <L2P>: 字符串类型，标识TE和MT之间使用的层二的协议，该参数目前仅支持NULL（无层二协议）。
     *             NULL：none, for PDP type OSP:IHOSS (Obsolete)
     *     <cid>: 整型值，PDP上下文标识符。
     *             V9R1版本的取值范围1~31。
     * [示例]:
     *     · 进入数传状态
     *       AT+CGDATA="NULL",1
     *       CONNECT
     *     · 测试命令
     *       AT+CGDATA=?
     *       +CGDATA: ("NULL")
     *       OK
     */
    { AT_CMD_CGDATA,
      At_SetCgdataPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgdataPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGDATA", (VOS_UINT8 *)CGDATA_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置QoS参数
     * [说明]: 设置或删除PDP上下文请求的QoS（Quality of Service）参数。
     *         如果命令中只有<cid>一个参数，如AT+CGEQREQ=<cid>，表示删除此<cid>对应的PDP上下文的QoS。
     *         若未使用该命令设置QoS参数，则PDP激活后所使用的QoS参数值取决于HLR的签约信息。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: AT+CGEQREQ=[<cid>[,<Trafficclass>[,<MaximumbitrateUL>[,<MaximumbitrateDL>[,<GuaranteedbitrateUL>[,<GuaranteedbitrateDL>[,<Deliveryorder>[,<MaximumSDUsize>[,<SDUerrorratio>[,<Residualbiterrorratio>[,<DeliveryoferroneousSDUs>[,<Transferdelay>[,<Traffichandlingpriority>[,<Sourcestatisticsdescriptor>[,<Signallingindication>]]]]]]]]]]]]]]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: +CGEQREQ?
     *     [结果]: <CR><LF>+CGEQREQ: <cid>,<Traffic
     *             class>,<Maximum bitrate UL> ,<Maximum bitrate
     *             DL> ,<Guaranteed bitrate UL> ,<Guaranteed bitrate
     *             DL> ,<Delivery order> ,<Maximum SDU
     *             size> ,<SDU error ratio> ,<Residual bit error
     *             ratio> ,<Delivery of erroneous SDUs> ,<Transfer
     *             delay> ,<Traffic handling priority>,<Source Statistics Descriptor>,<Signalling Indication>
     *             [<CR><LF>+CGEQREQ: <cid>, <Traffic
     *             class> ,<Maximum bitrate UL> ,<Maximum bitrate
     *             DL> ,<Guaranteed bitrate UL> ,<Guaranteed bitrate
     *             DL> ,<Delivery order> ,<Maximum SDU
     *             size> ,<SDU error ratio> ,<Residual bit error
     *             ratio> ,<Delivery of erroneous SDUs> ,<Transfer
     *             delay> ,<Traffic handling priority>,<Source Statistics Descriptor>,<Signalling Indication>
     *             [...]] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGEQREQ=?
     *     [结果]: +CGEQREQ: <PDP_type>, (list of supported <Traffic class>s) ,(list of supported <Maximum bitrate UL>s), (list of supported <Maximum bitrate DL>s), (list of supported <Guaranteed bitrate UL>s), (list of supported <Guaranteed bitrate DL>s),(list of supported <Delivery order>s) ,(list of supported <Maximum SDU size>s) ,(list of supported <SDU error ratio>s) ,(list of supported <Residual bit error ratio>s) ,(list of supported <Delivery of erroneous SDUs>s) ,(list of supported <Transfer delay>s) ,(list of supported <Traffic handling priority>s) ,(list of supported <Source statistics descriptor>s),(list of supported <Signalling indication>s)
     *             [<CR><LF>+CGEQREQ: <PDP_type>, (list of supported <Traffic class>s) ,(list of supported <Maximum bitrate UL>s), (list of supported <Maximum bitrate DL>s), (list of supported <Guaranteed bitrate UL>s), (list of supported <Guaranteed bitrate DL>s),(list of supported <Delivery order>s) ,(list of supported <Maximum SDU size>s) ,(list of supported <SDU error ratio>s) ,(list of supported <Residual bit error ratio>s) ,(list of supported <Delivery of erroneous SDUs>s) ,(list of supported <Transfer delay>s) ,(list of supported <Traffic handling priority>s) ,(list of supported <Source statistics descriptor>s),(list of supported <Signalling indication>s)
     *             [...]]
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。取值范围1~31。
     *     <Traffic class>: 整型值，业务类型。
     *             0：Conversational（会话类型）；
     *             1：Streaming（流类型）；
     *             2：Interactive（交互类型）；
     *             3：Background（背景类型）；
     *             4：Subscribed value（签约值）。
     *     <Maximum bitrate UL>: 整型值，上行最大比特率（单位为kbit/s），取值范围为0～11480。
     *             0：协商值；
     *             1～11480：1kbit/s～11480kbit/s。
     *     <Maximum bitrate DL>: 整型值，下行最大比特率（单位为kbit/s），取值范围为0～42000。
     *             0：协商值；
     *             1～42000：1kbit/s～42000kbit/s。
     *     <Guaranteed bitrate UL>: 整型值，上行可保证比特率（单位为kbit/s），取值范围为0～11480。
     *             0：协商值；
     *             1～11480：1kbit/s～11480kbit/s。
     *     <Guaranteed bitrate DL>: 整型值，下行可保证比特率（单位为kbit/s），取值范围为0～42000。
     *             0：协商值；
     *             1～42000：1kbit/s～42000kbit/s。
     *     <Delivery order>: 整型值，是否顺序发送SDU。
     *             0：无顺序性；
     *             1：有顺序性；
     *             2：协商值。
     *     <Maximum SDU size>: 整型值，最大SDU长度（单位为byte），取值范围为0～1520。
     *             0：协商值；
     *             1～1520：1byte～1520byte。
     *     <SDU error ratio>: 字符串类型，SDU误码率。
     *             0E0：协商值；
     *             1E2：1*10-2；
     *             7E3：7*10-3；
     *             1E3：1*10-3；
     *             1E4：1*10-4；
     *             1E5：1*10-5；
     *             1E6：1*10-6；
     *             1E1：1*10-1。
     *     <Residual bit error ratio>: 字符串类型，SDU残留比特误码率。
     *             0E0：协商值；
     *             5E2：5*10-2；
     *             1E2：1*10-2；
     *             5E3：5*10-3；
     *             4E3：4*10-3；
     *             1E3：1*10-3；
     *             1E4：1*10-4；
     *             1E5：1*10-5；
     *             1E6：1*10-6；
     *             6E8：6*10-8。
     *     <Delivery of erroneous SDUs>: 整型值，错误的SDU是否被发送。
     *             0：不发送；
     *             1：发送；
     *             2：不检测；
     *             3：协商值。
     *     <Transfer delay>: 整型值，传输时延（单位为ms），取值范围为0～4100。
     *             0：协商值；
     *             1～4100：1ms～4100ms。
     *     <Traffic handling priority>: 整型值，优先级。
     *             0：协商值；
     *             1：优先级1；
     *             2：优先级2；
     *             3：优先级3。
     *     <PDP_type>: 字符串类型，分组数据协议类型。
     *             IP：网际协议（IPV4）。
     *             IPV6：IPV6协议。
     *             IPV4V6：IPV4和IPV6。
     *     <Source Statistics Descriptor>: 整型值，统计源描述符。
     *             0：未知的SDU特性
     *             1：基于源语的SDU特性
     *     <Signalling Indication>: 整型值，信令描述。
     *             0：未优化信令
     *             1：已优化信令
     * [示例]:
     *     · 定义PDP上下文的QoS
     *       AT+CGEQREQ=2,2,64,64,,,1,320,"1E4","1E5",1,,3,0,0
     *       OK
     *     · 查询命令
     *       AT+CGEQREQ?
     *       +CGEQREQ: 2,2,64,64,,,1,320,"1E4","1E5",1,,3,0,0
     *       OK
     *       测试命令
     *     · 测试命令的输出根据版本支持的协议版本而不同。例如，支持R9的协议版本则测试命令输出的参数列表如下：
     *       AT+CGEQREQ=?
     *       +CGEQREQ: "IP",(0-4),(0-11480),(0-42000),(0-11480),(0-42000),(0-2),(0-1520),("0E0","1E2","7E3","1E3","1E4","1E5","1E6","1E1"),("0E0","5E2","1E2","5E3","4E3","1E3","1E4","1E5","1E6","6E8"),(0-3),(0-4100),(0-3),(0-1),(0-1)
     *       +CGEQREQ: "IPV6",(0-4),(0-11480),(0-42000),(0-11480),(0-42000),(0-2),(0-1520),("0E0","1E2","7E3","1E3","1E4","1E5","1E6","1E1"),("0E0","5E2","1E2","5E3","4E3","1E3","1E4","1E5","1E6","6E8"),(0-3),(0-4100),(0-3),(0-1),(0-1)
     *       +CGEQREQ: "IPV4V6",(0-4),(0-11480),(0-42000),(0-11480),(0-42000),(0-2),(0-1520), ("0E0","1E2","7E3","1E3","1E4","1E5","1E6","1E1"), ("0E0","5E2","1E2","5E3","4E3","1E3","1E4","1E5","1E6","6E8"),(0-3),(0-4100),(0-3),(0-1),(0-1)
     *       OK
     */
    { AT_CMD_CGEQREQ,
      AT_SetCgeqreqPara, AT_SET_PARA_TIME, AT_QryCgeqreqPara, AT_QRY_PARA_TIME, At_TestCgeqreqPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGEQREQ", (VOS_UINT8 *)CGEQREQ_CMD_PARA_STRING },

    /* +CGEQMIN命令的测试显示结果与+CGEQREQ是一致的，故使用同一个测试函数 */
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置最小QoS
     * [说明]: 设置或删除PDP上下文的最小可接受服务质量（QoS）。
     *         如果命令中只有<cid>一个参数，如AT+CGEQMIN=<cid>，表示删除此<cid>对应的PDP上下文的最小可接受服务质量。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGEQMIN=[<cid>[,<Trafficclass>[,<MaximumbitrateUL>[,<MaximumbitrateDL>[,<GuaranteedbitrateUL>[,<GuaranteedbitrateDL>[,<Deliveryorder>[,<MaximumSDUsize>[,<SDUerrorratio>[,<Residualbiterrorratio>[,<DeliveryoferroneousSDUs>[,<Transferdelay>[,<Traffichandlingpriority>[,<Sourcestatisticsdescriptor>[,<Signallingindication>]]]]]]]]]]]]]]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: +CGEQMIN?
     *     [结果]: +CGEQMIN: <cid>, <Traffic class> ,<Maximum bitrate UL>, <Maximum bitrate DL> ,<Guaranteed bitrate UL> ,<Guaranteed bitrate DL>, <Delivery order> ,<Maximum SDU size> ,<SDU error ratio> ,<Residual bit error ratio> ,<Delivery of erroneous SDUs> ,<Transfer delay> ,<Traffic handling priority>,<Source Statistics Descriptor>,<Signalling Indication>
     *             [<CR><LF>+CGEQMIN: <cid>, <Traffic class> ,<Maximum bitrate UL> ,<Maximum bitrate DL> ,<Guaranteed bitrate UL> ,<Guaranteed bitrate DL>, <Delivery order> ,<Maximum SDU size> ,<SDU error ratio> ,<Residual bit error ratio> ,<Delivery of erroneous SDUs> ,<Transfer delay> ,<Traffic handling priority>,<Source Statistics Descriptor>,<Signalling Indication>
     *             [...]]
     *     [命令]: +CGEQMIN=?
     *     [结果]: +CGEQMIN: <PDP_type>, (list of supported <Traffic class>s) ,(list of supported <Maximum bitrate UL>s) ,(list of supported <Maximum bitrate DL>s), (list of supported <Guaranteed bitrate UL>s), (list of supported <Guaranteed bitrate DL>s) ,(list of supported <Delivery order>s) ,(list of supported <Maximum SDU size>s) ,(list of supported <SDU error ratio>s) ,(list of supported <Residual bit error ratio>s) ,(list of supported <Delivery of erroneous SDUs>s) ,(list of supported <Transfer delay>s) ,(list of supported <Traffic handling priority>s) ,(list of supported <Source statistics descriptor>s),(list of supported <Signalling indication>s)
     *             [<CR><LF>+CGEQMIN: <PDP_type>, (list of supported <Traffic class>s) ,(list of supported <Maximum bitrate UL>s), (list of supported <Maximum bitrate DL>s) ,(list of supported <Guaranteed bitrate UL >s), (list of supported <Guaranteed bitrate DL >s) ,(list of supported <Delivery order>s) ,(list of supported <Maximum SDU size>s) ,(list of supported <SDU error ratio>s) ,(list of supported <Residual bit error ratio>s) ,(list of supported <Delivery of erroneous SDUs>s) ,(list of supported <Transfer delay>s) ,(list of supported <Traffic handling priority>s) ,(list of supported <Source statistics descriptor>s),(list of supported <Signalling indication>s)
     *             [...]]
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。取值范围1~31。
     *     <Traffic class>: 整型值，业务类型。
     *             0：Conversational（会话类型）；
     *             1：Streaming（流类型）；
     *             2：Interactive（交互类型）；
     *             3：Background（背景类型）；
     *             4：Subscribed value（签约值）。
     *     <Maximum bitrate UL>: 整型值，上行最大比特率（单位为kbit/s），取值范围为0～11480。
     *             0：协商值；
     *             1～11480：1kbit/s～11480kbit/s。
     *     <Maximum bitrate DL>: 整型值，下行最大比特率（单位为kbit/s），取值范围为0～42000。
     *             0：协商值；
     *             1～42000：1kbit/s～42000kbit/s。
     *     <Guaranteed bitrate UL>: 整型值，上行可保证比特率（单位为kbit/s），取值范围为0～11480。
     *             0：协商值；
     *             1～11480：1kbit/s～11480kbit/s。
     *     <Guaranteed bitrate DL>: 整型值，下行可保证比特率（单位为kbit/s），取值范围为0～42000。
     *             0：协商值；
     *             1～42000：1kbit/s～42000kbit/s。
     *     <Delivery order>: 整型值，是否顺序发送SDU。
     *             0：无顺序性；
     *             1：有顺序性；
     *             2：协商值。
     *     <Maximum SDU size>: 整型值，最大SDU长度（单位为byte），取值范围为0～1520。
     *             0：协商值；
     *             1～1520：1byte～1520byte。
     *     <SDU error ratio>: 字符串类型，SDU误码率。
     *             0E0：协商值；
     *             1E2：1*10-2；
     *             7E3：7*10-3；
     *             1E3：1*10-3；
     *             1E4：1*10-4；
     *             1E5：1*10-5；
     *             1E6：1*10-6；
     *             1E1：1*10-1。
     *     <Residual bit error ratio>: 字符串类型，SDU残留比特误码率。
     *             0E0：协商值；
     *             5E2：5*10-2；
     *             1E2：1*10-2；
     *             5E3：5*10-3；
     *             4E3：4*10-3；
     *             1E3：1*10-3；
     *             1E4：1*10-4；
     *             1E5：1*10-5；
     *             1E6：1*10-6；
     *             6E8：6*10-8。
     *     <Delivery of erroneous SDUs>: 整型值，错误的SDU是否被发送。
     *             0：不发送；
     *             1：发送；
     *             2：不检测；
     *             3：协商值。
     *     <Transfer delay>: 整型值，传输时延（单位为ms），取值范围为0～4100。
     *             0：协商值；
     *             1～4100：1ms～4100ms。
     *     <Traffic handling priority>: 整型值，优先级。
     *             0：协商值；
     *             1：优先级1；
     *             2：优先级2；
     *             3：优先级3。
     *     <PDP_type>: 字符串类型，分组数据协议类型。
     *             IP：网际协议（IPV4）。
     *             IPV6：IPV6协议。
     *             IPV4V6：IPV4和IPV6。
     *     <Source Statistics Descriptor>: 整型值
     *             0：SDU的特性未知。
     *             1：SDU的特性和语音源相关。
     *     <Signalling Indication>: 整型值
     *             0：信令不优化PDP上下文。
     *             1：信令优化PDP上下文。
     * [示例]:
     *     · 定义PDP上下文的最小可接受QoS
     *       AT+CGEQMIN=2,2,64,64,,,1,320,"1E4","1E5",1,,3,0,0
     *       OK
     *     · 查询命令
     *       AT+CGEQMIN?
     *       +CGEQMIN: 2,2,64,64,,,1,320,"1E4","1E5",1,,3,0,0
     *       OK
     *     · 测试命令
     *       AT+CGEQMIN=?
     *       +CGEQMIN: "IP",(0-4),(0-5760),(0-21600),(0-5760),(0-21600),(0-2),(0-1520),("0E0","1E2","7E3","1E3","1E4","1E5","1E6","1E1"),("0E0","5E2","1E2","5E3","4E3","1E3","1E4","1E5","1E6","6E8"),(0-3),(0-4100),(0-3),(0-1),(0-1)
     *       +CGEQMIN: "IPV6",(0-4),(0-11480),(0-42000),(0-11480),(0-42000),(0-2),(0-1520),("0E0","1E2","7E3","1E3","1E4","1E5","1E6","1E1"),("0E0","5E2","1E2","5E3","4E3","1E3","1E4","1E5","1E6","6E8"),(0-3),(0-4100),(0-3),(0-1),(0-1)
     *       +CGEQMIN: "IPV4V6",(0-4),(0-11480),(0-42000),(0-11480),(0-42000),(0-2),(0-1520), ("0E0","1E2","7E3","1E3","1E4","1E5","1E6","1E1"), ("0E0","5E2","1E2","5E3","4E3","1E3","1E4","1E5","1E6","6E8"),(0-3),(0-4100),(0-3),(0-1),(0-1)
     *       OK
     */
    { AT_CMD_CGEQMIN,
      AT_SetCgeqminPara, AT_SET_PARA_TIME, At_QryCgeqminPara, AT_QRY_PARA_TIME, At_TestCgeqreqPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGEQMIN", (VOS_UINT8 *)CGEQMIN_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置PDP激活自动应答
     * [说明]: 打开或关闭PS域的自动应答，该应答用于接收网络发起的PDP激活请求。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGAUTO=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CGAUTO?
     *     [结果]: <CR><LF>+CGAUTO: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGAUTO=?
     *     [结果]: <CR><LF>+CGAUTO: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，打开或者关闭PS域的自动应答。
     *             0：关闭；
     *             1：打开。
     * [示例]:
     *     · 设置命令
     *       AT+CGAUTO=1
     *       OK
     *     · 测试命令
     *       AT+CGAUTO=?
     *       +CGAUTO: (0,1)
     *       OK
     *     · 查询命令
     *       AT+CGAUTO?
     *       +CGAUTO: 1
     *       OK
     */
    { AT_CMD_CGAUTO,
      AT_SetCgautoPara, AT_SET_PARA_TIME, AT_QryCgautoPara, AT_QRY_PARA_TIME, At_TestCgautoPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CGAUTO", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置是否允许时区上报
     * [说明]: 网络可通过MM INFORMATION或GMM INFORMATION将当前时区信息通知终端。
     *         该命令用来设置终端是否上报给应用网络下发的时区信息及上报的格式。
     * [语法]:
     *     [命令]: +CTZR=<value>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CTZR?
     *     [结果]: <CR><LF>+CTZR: <value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CTZR=?
     *     [结果]: <CR><LF>+CTZR: (list of supported <value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值
     *             0： 不上报
     *             1： 使用+ctzv上报
     *             2： 使用+ctze上报
     * [示例]:
     *     · 设置打开网络时区上报，上报方式使用+ctzv
     *       AT+CTZR=1
     *       OK
     *     · 查询
     *       AT+CTZR?
     *       +CTZR: 1
     *       OK
     */
    { AT_CMD_CTZR,
      At_SetCtzrPara, AT_SET_PARA_TIME, At_QryCtzrPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CTZR", (VOS_UINT8 *)"(0-2)" },

#if (FEATURE_LTE == FEATURE_ON)

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 读取缺省PDP上下文动态参数
     * [说明]: Execution命令读取缺省PDP的相关动态上下文参数。
     *         Read命令返回处于激活状态的缺省PDP上下文关联的<p_cid>列表。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGCONTRDP=[<p_cid>]
     *     [结果]: <CR><LF>+CGCONTRDP: <p_cid>,<bearer_id>,<apn>[,<local_addr and subnet_mask>[,<gw_addr>[,<DNS_prim_addr>[,<DNS_sec_addr>[,<P-CSCF_prim_addr>[,<P-CSCF_sec_addr>[,<IM_CN_Signalling_Flag>[,[,<IPv4_MTU>[,[,[,[,[,[,[,<PDU_session_id>,<QFI>[,<SSC_mode>[,<S-NSSAI>[,<Access_type>[,<RQ_timer>[,<Always-on_ind>]]]]]]]]]]]]]]]]]]]]]
     *             [<CR><LF>+CGCONTRDP: <p_cid>,<bearer_id>,<apn>[,<local_addr and subnet_mask>[,<gw_addr>[,<DNS_prim_addr>[,<DNS_sec_addr>[,<P-CSCF_prim_addr>[,<P-CSCF_sec_addr>[,<IM_CN_Signalling_Flag>[,[,<IPv4_MTU>[,[,[,[,[,[,[,<PDU_session_id>,<QFI>[,<SSC_mode>[,<S-NSSAI>[,<Access_type>[,<RQ_timer>[,<Always-on_ind>]]]]]]]]]]]]]]]]]]]]]
     *             […]] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: +CGCONTRDP=?
     *     [结果]: <CR><LF>+CGCONTRDP: (list of <p_cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <p_cid>: 数字参数，标识Default PDP。
     *     <bearer_id>: 数字参数，EPS承载业务的标识，或UMTS/GPRS的NSAPI标识，取值范围5~15。
     *     <APN>: 字符串参数，指定GGSN或外部分组数据网络。
     *     <local_addr_and_subnet_mask>: 字符串参数，终端IP地址和子网掩码，点分格式，格式如下：
     *             IPv4："a1.a2.a3.a4.m1.m2.m3.m4"
     *             Ipv6："a1.a2.a3.a4.a5.a6.a7.a8.a9.a10.a11.a12.a13.a14.a15.a16.m1.m2.m3.m4.m5.m6.m7.m8.m9.m10.m11.m12.m13. m14.m15.m16"
     *     <gw_addr>: 字符串参数，终端的网关地址。
     *     <DNS_prim_addr>: 字符串参数，主DNS服务器IP地址。
     *     <DNS_sec_addr>: 字符串参数，次DNS服务器IP地址。
     *     <P_CSCF_prim_addr>: 字符串参数，主CSCF服务器地址。
     *     <P_CSCF_sec_addr>: 字符串参数，次CSCF服务器地址。
     *     <IM_CN_Signalling_Flag>: 整型值，指示此PDP是否仅用于 IM CN
     *     <IPv4_MTU>: 整型值，IPv4栈的MTU值
     *     <PDU_session_id>: 整型值，PDU session ID
     *     <QFI>: 整型值，QoS flow ID
     *     <SSC_mode>: 整型值，SSC mode
     *     <S-NSSAI>: 字符串类型，分片信息
     *             "sst" if only slice/service type (SST) is present
     *             "sst;mapped_sst" if SST and mapped configured SST are present
     *             "sst.sd" if SST and slice differentiator (SD) are present
     *             "sst.sd;mapped_sst" if SST, SD and mapped configured SST are present
     *             "sst.sd;mapped_sst.mapped_sd" if SST, SD, mapped configured SST and mapped configured SD are present
     *             注意：字符串中的数据为16进制，字符取值范围：0-9，A-F，a-f。
     *     <Access_type>: 整型值，接入类型
     *     <RQ_timer>: 整型值，反射QoS的时长，单位s
     *     <Always-on_ind>: 整型值，Always-on指示标记，
     *             0：指示 PDU session 不是 always-on的
     *             1：指示 PDU session 是 always-on的
     * [示例]:
     *       查询已激活的CID=1的缺省PDP信息
     *     · 不支持5GS时
     *       AT+CGCONTRDP=1
     *       +CGCONTRDP: 1,5,"CMCC","192.168.0.23","255.255.255.0"
     *       OK
     *       支持5GS时
     *          AT+CGCONTRDP=1
     *          +CGCONTRDP: 1,5,"CMCC","192.168.0.23","255.255.255.0",,1500,,,,,,,5,1,0,"1.123456;2F.654321",0,1000,0
     *          OK
     *     · 返回处于激活状态的缺省PDP关联的<p_cid>列表（例如已激活CID1和9）
     *       AT+CGCONTRDP=?
     *       +CGCONTRDP: (1,9)
     *       OK
     */
    { AT_CMD_CGCONTRDP,
      atSetCgcontrdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgcontrdp, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGCONTRDP", (VOS_UINT8 *)"(1-31)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 读取Secondary PDP上下文动态参数
     * [说明]: Execution命令读取Secondary PDP相关的动态上下文参数。
     *         Read命令返回处于激活状态的Secondary PDP关联的<cid>列表。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGSCONTRDP[=<cid>]
     *     [结果]: <CR><LF>+CGSCONTRDP: <cid>, <p_cid>, <bearer_id>[,[,[,<PDU_session_id>,<QFI>]]]]
     *             [<CR><LF>+CGSCONTRDP: <cid>, <p_cid>,  <bearer_id>[,[,[,<PDU_session_id>,<QFI>]]]]
     *             […]] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: +CGSCONTRDP=?
     *     [结果]: <CR><LF>+CGSCONTRDP: (list of <cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 数字参数，标识Secondary PDP的CID。
     *     <p_cid>: 数字参数，标识Default PDP的CID。
     *     <bearer_id>: 数字参数，EPS承载业务的EPSBID标识，或UMTS/GPRS的NSAPI标识，取值范围5~15。
     *     <PDU_session_id>: 整型值，PDU session ID
     *     <QFI>: 整型值，QoS flow ID
     * [示例]:
     *       查询激活的CID=3的Secondary PDP信息
     *     · 不支持5GS时
     *       AT+CGSCONTRDP=3
     *       +CGSCONTRDP: 3,1,5
     *       OK
     *     · 支持5GS时
     *       AT+CGSCONTRDP=3
     *       +CGSCONTRDP: 3,1,5,,,5,1
     *       OK
     *     · 返回处于激活状态的Secondary PDP关联的<cid>列表（例如已激活CID2和21）
     *       AT+CGSCONTRDP=?
     *       +CGSCONTRDP: (2,21)
     *       OK
     */
    { AT_CMD_CGSCONTRDP,
      atSetCgscontrdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgscontrdp, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGSCONTRDP", (VOS_UINT8 *)"(1-31)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 读取TFT参数
     * [说明]: Execution命令读取<cid>标识的TFT相关参数。
     *         Read命令返回处于激活状态的EPS承载关联的<cid>列表。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGTFTRDP[=<cid>]
     *     [结果]: <CR><LF>+CGTFTRDP: <cid>, <packet filter identifier>, <evaluation precedence index>, <source address and subnet mask>, <protocol number (ipv4) / next header (ipv6)>, <destination port range>, <source port range>, <ipsec security parameter index (spi)>, <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>, <flow label (ipv6)>, <direction>, <NW packet filter Identifier>,<local address and subnetmask>,<QRI>
     *             [<CR><LF>+CGTFTRDP: <cid>, <packet filter identifier>, <evaluation precedence index>, <source address and subnet mask>, <protocol number (ipv4) / next header (ipv6)>, <destination port range>, <source port range>, <ipsec security parameter index (spi)>, <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>, <flow label (ipv6)>, <direction>,<NW packet filter Identifier>,<local address and subnetmask>,<QRI>
     *             […]] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: +CGTFTRDP=?
     *     [结果]: <CR><LF>+CGTFTRDP: (list of <cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <packet filter identifier>: 数字参数，过滤器ID，取值范围1~16。
     *     <evaluation precedence index>: Packet filter的优先级，当有多个filter匹配数据包成功时，取优先级高的filter。数字参数，取值范围0-255。
     *     <source address and subnet mask>: 字符串类型，源地址和子网掩码，点分格式。
     *             IPv4："a1.a2.a3.a4.m1.m2.m3.m4"
     *             IPv6："a1.a2.a3.a4.a5.a6.a7.a8.
     *             a9.a10.a11.a12.a13.a14.a15.a16.
     *             m1.m2.m3.m4.m5.m6.m7.m8.
     *             m9.m10.m11.m12.m13.m14.m15.m16"
     *     <protocol number (ipv4) / next header (ipv6)>: IP头中指示的上层协议号，如TCP，UDP等。数字参数，取值范围0~255。
     *     <destination port range>: 字符串类型，目的端口范围，十六进制点分形式，点左右的数字不能超过65535。
     *             比如："f.t"。f表示端口下限，t表示端口上限。
     *     <source port range>: 字符串类型，源端口范围，十六进制点分形式，点左右的数字不能超过65535。
     *             比如："f.t"。f表示端口下限，t表示端口上限。
     *     <ipsec security parameter index (spi)>: IPV6中安全扩展头（如AH、ESP）的参数。数字参数，十六进制形式，取值范围0x00000000~0xFFFFFFFF。
     *     <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>: 服务类型/业务类型，字符串类型，点分形式，点左右的数字不能超过255。
     *             比如："f.m"。f表示服务最小值，m表示服务最大值。
     *     <flow label (ipv6)>: 流标签，IPV6基本头中的参数，标识源地址和目的地址相同的一些包。数字参数，十六进制形式，取值范围0x00000~0xFFFFF，仅对IPv6有效。
     *     <direction>: 数字参数，指定过滤器的方向。
     *             0：Release 7之前的TFT过滤器（参见3GPP TS 24.008）；
     *             1：上行方向；
     *             2：下行方向；
     *             3：双向（缺省值）。
     *     <NW packet filter Identifier>: 网侧用来标识FILTER的ID，数字参数，取值范围1~16。
     *     <local address and subnet mask>: IP地址字符串类型，本地IP地址及子网掩码，格式为IPv4:“a1.a2.a3.a4.m1.m2.m3.m4”
     *             IPv6:"a1.a2.a3.a4.a5.a6.a7.a8.a9.a10.a11.a12.a13.a14.a15.a16.m1.m2.m3.m4.m5.m6.m7.m8.m9.m10.m11.m12.m13.m14.m15.m16"。
     *             注意：只在R11以上版本支持此参数
     *     <QRI>: 整型值，Qos rule的identity，协议支持0-255.
     *             注意：只在R15以上版本支持此参数
     * [示例]:
     *     · 读取CID=3对应承载的TFT相关参数
     *       AT+CGTFTRDP=3
     *       +CGTFTRDP: 3,0,0,"192.168.0.2.255.255.255.0",0,"0.65535","0.65535",0,"0.0",0,0
     *       OK
     *     · 返返回处于激活状态的EPS承载关联的<cid>列表
     *       AT+CGTFTRDP=?
     *       +CGTFTRDP: (1,2,21)
     *       OK
     */
    { AT_CMD_CGTFTRDP,
      atSetCgtftrdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgtftrdp, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGTFTRDP", (VOS_UINT8 *)"(1-31)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 定义EPS QoS参数
     * [说明]: 设置命令定义EPS Traffic Flow对应的QoS参数。
     *         查询命令获取已经定义的QoS参数。
     *         测试命令返回系统支持的参数范围。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGEQOS=[<cid>[,<QCI>[,<DL_GBR>,<UL_GBR>[,<DL_MBR>,<UL_MBR]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err> <CR><LF>
     *     [命令]: +CGEQOS?
     *     [结果]: <CR><LF>+CGEQOS: <cid>, <QCI>, [<DL_GBR>,<UL_GBR>], [<DL_MBR>,<UL_MBR>]
     *             [<CR>>LF>+CGEQOS: <cid>, <QCI>, [<DL_GBR>,<UL_GBR>], [<DL_MBR>,<UL_MBR>]
     *             […]] <CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +CGEQOS=?
     *     [结果]: <CR><LF>+CGEQOS: (list of supported <cid>s) ,(list of supported <QCI>s) ,(list of supported <DL_GBR>s), (list of supported <UL_GBR>s), (list of supported <DL_MBR>s) ,(list of supported <UL_MBR>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 数字参数，标识EPS Traffic Flow，取值范围1~31。
     *     <QCI>: 服务质量等级，每个对应一组参数，如丢包率，时延等。
     *             0：由网络选择QCI，（0是预留给网络的，不可用于设置）；
     *             1~4, 71~76, 82~85：GBR Traffic Flows取值范围；
     *             5~9, 79：non-GBR Traffic Flows取值范围。
     *             128~254：运营商专用QCI取值范围。
     *             参考3GPP TS 23.203。
     *     <DL_GBR>: 数字参数，下行方向GBR参考值（目前UE端不支持设置GBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     *     <UL_GBR>: 数字参数，上行方向GBR参考值（目前UE端不支持设置GBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     *     <DL_MBR>: 数字参数，下行方向MBR参考值（目前UE端不支持设置MBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     *     <UL_MBR>: 数字参数，上行方向MBR参考值（目前UE端不支持设置MBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     * [示例]:
     *     · 设置命令定义EPS Traffic Flow对应的QoS参数。
     *       AT+CGEQOS=3,5
     *       OK
     *     · 查询命令获取已经定义的QoS参数。
     *       AT+CGEQOS?
     *       +CGEQOS: 3,5,0,0,0,0
     *       OK
     *     · 测试命令返回系统支持的参数范围
     *       AT+CGEQOS=?
     *       +CGEQOS: (1-31),(0-9,71-76,79,128-254),(0-16777216),(0-16777216),(0-16777216),(0-16777216)
     *       OK
     */
    { AT_CMD_CGEQOS,
      atSetCgeqosPara, AT_SET_PARA_TIME, atQryCgeqosPara, AT_QRY_PARA_TIME, At_TestCgeqos, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGEQOS", (VOS_UINT8 *)"(1-31),(0-9,71-76,79,128-254),(0-16777216),(0-16777216),(0-16777216),(0-16777216)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 读取EPS QoS参数
     * [说明]: 设置命令读取<cid>标识的EPS PDN连接对应的QoS参数。
     *         测试命令返回处于激活状态的EPS承载对应的<cid>列表。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGEQOSRDP[=<cid>]
     *     [结果]: <CR><LF>+CGEQOSRDP: <cid>, <QCI>, [<DL_GBR>,<UL_GBR>], [<DL_MBR>,<UL_MBR>]
     *             [<CR>>LF>+CGEQOSRDP: <cid>, <QCI>, [<DL_GBR>,<UL_GBR>], [<DL_MBR>,<UL_MBR>]
     *             […]] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGEQOSRDP=?
     *     [结果]: <CR><LF>+CGEQOSRDP: (list of <cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 数字参数，标识EPS Traffic Flow。
     *     <QCI>: 0：由网络选择QCI；
     *             1~4：GBR Traffic Flows取值范围；
     *             5~9：non-GBR Traffic Flows取值范围。
     *             参考3GPP TS 23.203。
     *     <DL_GBR>: 数字参数，下行方向GBR参考值（目前UE端不支持设置GBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     *     <UL_GBR>: 数字参数，上行方向GBR参考值（目前UE端不支持设置GBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     *     <DL_MBR>: 数字参数，下行方向MBR参考值（目前UE端不支持设置MBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     *     <UL_MBR>: 数字参数，上行方向MBR参考值（目前UE端不支持设置MBR速率），单位为kbit/s。此参数对GBR QCI有效，对non-GBR QCI，此参数省略。参考3GPP TS 24.301。
     * [示例]:
     *     · 设置命令读取CID=3的EPS PDN连接对应的QoS参数。
     *       AT+CGEQOSRDP=3
     *       +CGEQOSRDP: 3,5
     *       OK
     *     · 测试命令返回处于激活状态的EPS承载对应的<cid>列表
     *       AT+CGEQOSRDP=?
     *       +CGEQOSRDP: (1,9)
     *       OK
     */
    { AT_CMD_CGEQOSRDP,
      atSetCgeqosrdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestCgeqosrdp, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CGEQOSRDP", (VOS_UINT8 *)"(1-31)" },

#endif

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 移动发起定位请求
     * [说明]: 该命令用于移动端发起或取消定位请求（MO-LR），同时在发起定位请求时配置定位的方式及位置报告的格式等信息。
     * [语法]:
     *     [命令]: +CMOLR=[<enable>[,<method>[,<hor-acc-set>[,<hor-acc>[,<ver-req>[,<ver-acc-set>[,<ver-acc>[,<vel-req>[,<rep-mode>[,<timeout>[,<interval>[,<shape-rep>[,<plane>[,<NMEA-rep>[,<third-party-address>]]]]]]]]]]]]]]]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CMOLR?
     *     [结果]: <CR><LF>+CMOLR: <enable>,<method>, <hor-acc-set>,[<hor-acc>],<ver-req>, [<ver-acc-set>],[<ver-acc>],<vel-req>, <rep-mode>,<timeout>,[<interval>], <shape-rep>,[<plane>],[<NMEA-rep>], [<third-party-address>]<CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CMOLR=?
     *     [结果]: <CR><LF>+CMOLR: (list of supported <enable>s),(list of supported <method>s),(list of supported <hor-acc-set>s),(list of supported <hor-acc>s),(list of supported <ver-req>s),(list of supported <ver-acc-set>s),(list of supported <ver-acc>s),(list of supported <vel-req>s),(list of supported <rep-mode>s),(list of supported <timeout>s),(list of supported <interval>s),(list of supported <shape-rep>s),(list of supported <plane>s),(list of supported <NMEA-rep>s),(list of supported <third-party-address>s)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，开启或关闭定位请求结果的位置上报，同一时间只能有一种<method>可以被开启，默认值为0：
     *             0：关闭上报和定位，其余参数均忽略；
     *             1：开启NMEA字符串的主动上报，格式为+CMOLRN: <NMEA-string>，当超时没有数据时则主动上报+CMOLRE；
     *             2：开启GAD形状的主动上报，格式为+CMOLRG: <location_parameters>，当超时没有数据时则主动上报+CMOLRE；
     *             3：同时开启NMEA字符串和GAD形状的主动上报，格式为+CMOLRN: <NMEA-string>和+CMOLRG: <location_parameters>，当超时没有数据时则主动上报+CMOLRE。
     *     <method>: 整型值，MO-LR的方法，默认值为0：
     *             0：独立GPS，仅自主的GPS，无辅助数据；
     *             1：辅助GPS；
     *             2：辅助GANSS；
     *             3：辅助GPS和GANSS；
     *             4：基于自身定位（由网络决定定位技术）；
     *             5：转给第三方，此方法不关注参数<shape-rep>和<NMEA-rep>（下发的值将被忽略），第三方地址由参数<third-party-address>提供；
     *             6：从第三方收回，第三方地址由参数<third-party-address>提供。
     *             注：若指定上报给第三方，则表示当超时没有数据时不主动上报+CMOLRE。
     *     <hor-acc-set>: 整型值，水平精度设置标志，默认值为0：
     *             0：不要求水平精度；
     *             1：水平精度由参数<hor-acc>指定。
     *     <hor-acc>: 整型值，请求的水平精度不确定指数，取值范围为0~127，默认值为60。
     *     <ver-req>: 整型值，垂直坐标请求，默认值为0：
     *             0：不请求垂直坐标（高度），满足二维定位，参数<ver-acc-set>和<ver-acc>不适用；
     *             1：请求垂直坐标（高度），需要三维定位。
     *     <ver-acc-set>: 整型值，垂直精度设置标志，默认值为0：
     *             0：不设置垂直精度；
     *             1：垂直精度由参数<ver-acc>指定。
     *     <ver-acc>: 整型值，请求的垂直精度不确定指数，取值范围为0~127，默认值为60。
     *     <vel-req>: 整型值，请求的速度类型，默认值为0：
     *             0：不需要速度；
     *             1：请求水平速度；
     *             2：请求水平及垂直速度；
     *             3：不确定要求的水平速度；
     *             4：不确定要求的水平速度及垂直速度。
     *     <rep-mode>: 整型值，上报模式，默认值为0：
     *             0：单个报告，MO-LR响应的超时时间由<timeout>指定；
     *             1：周期上报，每次MO-LR响应的超时时间由<timeout>指定，每次MO-LR的间隔由<interval>指定。
     *     <timeout>: 整型值，指示MS等待MO-LR响应的时间，取值范围为1~65535，单位秒，默认值为180。
     *     <interval>: 整型值，此参数仅用于周期上报，决定周期MO-LR间的间隔，取值范围为1~65535，单位秒，取值要大于等于<timeout>，默认值为180。
     *     <shape-rep>: 整型值，此参数是主动上报结果码<location_parameters>中可接受的代表某个GAD形状的集合，取值范围为1-127，默认值为1：
     *             1：椭圆点；
     *             2：不确定圆的椭圆点；
     *             4：不确定椭圆的椭圆点；
     *             8：多边形；
     *             16：带垂直坐标的椭圆点；
     *             32：带垂直坐标的不确定椭圆的椭圆点；
     *             64：椭圆弧。
     *             注：每个主动上报结果码中仅表示一个GAD形状。
     *     <plane>: 整型值，平面类型，默认值为0：
     *             0：控制面；
     *             1：安全用户面（SUPL）。
     *     <NMEA-rep>: 字符串类型，支持NMEA字符串，格式为以逗号分隔的字符串。如果此参数省略或为空字符串，则没有限制，所有的NMEA字符串均支持。默认值为“$GPGGA,$GPRMC,$GPGLL”。
     *     <third-party-address>: 字符串类型，此参数仅用于上报给第三方，指定了第三方的地址，仅在<method>值为5或6时有效。
     * [示例]:
     *     · 发起移动定位请求
     *       AT+CMOLR=3,1,1,20,0,,,0,1,60,100,9,0,"$GPRMC"
     *       OK
     *     · 查询上行定位请求参数
     *       AT+CMOLR?
     *       +CMOLR: 0,0,0,,0,,,0,0,180,,1,,,
     *       OK
     *     · 执行测试命令
     *       AT+CMOLR=?
     *       +CMOLR: (0-3),(0-6),(0,1),(0-127),(0,1),(0,1),(0-127),(0-4),(0,1), (1-65535),(1-65535),(1-127),(0,1),"$GPGGA,$GPRMC,$GPGLL","<third-party-addr>"
     *       OK
     */
    { AT_CMD_CMOLR,
      AT_SetCmolrPara, AT_SET_PARA_TIME, AT_QryCmolrPara, AT_QRY_PARA_TIME, AT_TestCmolrPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CMOLR", (VOS_UINT8 *)"(0-3),(0-6),(0,1),(0-127),(0,1),(0,1),(0-127),(0-4),(0,1),(1-65535),(1-65535),(1-127),(0,1),(nmea),(thirdparty)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 设置上行定位请求错误上报格式
     * [说明]: 该命令用于控制移动端发起的定位请求（MO-LR）功能错误指示的上报格式。
     * [语法]:
     *     [命令]: +CMOLRE=[<n>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CMOLRE?
     *     [结果]: <CR><LF>+CMOLRE: <n><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CMOLRE=?
     *     [结果]: <CR><LF>+CMOLRE: (list of supported <n>s)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，开启或关闭字符串错误描述上报格式，默认值为0：
     *             0：关闭；
     *             1：开启。
     * [示例]:
     *     · 设置上行定位请求错误为字符串格式
     *       AT+CMOLRE=1
     *       OK
     *     · 查询上行定位请求上报格式
     *       AT+CMOLRE?
     *       +CMOLRE: 1
     *       OK
     *     · 执行测试命令
     *       AT+CMOLRE=?
     *       +CMOLRE: (0,1)
     *       OK
     */
    { AT_CMD_CMOLRE,
      AT_SetCmolrePara, AT_NOT_SET_TIME, AT_QryCmolrePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CMOLRE", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 设置下行定位请求指示方式
     * [说明]: 该命令用于控制网络发起的定位请求（MT-LR）通知的方式。
     * [语法]:
     *     [命令]: +CMTLR=[<subscribe>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CMTLR?
     *     [结果]: <CR><LF>+CMTLR: <subscribe><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CMTLR=?
     *     [结果]: <CR><LF>+CMTLR: (list of supported <subscribe>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <subscribe>: 整型值，开启或关闭下行定位请求的上报，默认值为1：
     *             0：关闭上报和定位；
     *             1：通过控制面上报MT-LR通知；
     *             2：通过SUPL上报MT-LR通知；
     *             3：通过控制面和SUPL上报MT-LR通知。
     * [示例]:
     *     · 设置下行定位请求通过控制面上报
     *       AT+CMTLR=1
     *       OK
     *     · 查询下行定位请求上报的方式（通过SUPL上报）
     *       AT+CMTLR?
     *       +CMTLR: 2
     *       OK
     *     · 执行测试命令
     *       AT+CMTLR=?
     *       +CMTLR: (0-3)
     *       OK
     */
    { AT_CMD_CMTLR,
      AT_SetCmtlrPara, AT_SET_PARA_TIME, AT_QryCmtlrPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CMTLR", (VOS_UINT8 *)"(0-3)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 下行网络定位请求应答
     * [说明]: 该命令用于接收或拒绝网络发起的定位请求（MT-LR）。
     * [语法]:
     *     [命令]: +CMTLRA=<allow>,<handle_id>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CMTLRA?
     *     [结果]: <CR><LF>+CMTLRA: <allow>,<handle_id> <CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +CMTLRA=?
     *     [结果]: <CR><LF>+CMTLRA: (list of supported <allow> values)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <allow>: 整型值，允许或不允许网络的位置发现请求：
     *             0：允许位置发现；
     *             1：不允许位置发现。
     *     <handle_id>: 整型值，每个MT-LR的ID用于区分多个请求中的指定请求，取值范围为0~255，由+CMTLR命令给出。
     * [示例]:
     *     · 允许网络的位置发现请求：
     *       AT+CMTLRA=0,2
     *       OK
     *     · 查询下行定位请求的响应配置
     *       AT+CMTLRA?
     *       +CMTLRA: 0,2
     *       OK
     *     · 执行测试命令
     *       AT+CMTLRA=?
     *       +CMTLRA: (0,1)
     *       OK
     */
    { AT_CMD_CMTLRA,
      AT_SetCmtlraPara, AT_SET_PARA_TIME, AT_QryCmtlraPara, AT_QRY_PARA_TIME, AT_TestCmtlraPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CMTLRA", (VOS_UINT8 *)"(0,1),(0-255)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 定位控制
     * [说明]: 该命令用于AP-Modem形态下GPS芯片将XML格式的定位信息，发送到Modem侧。
     *         注：受AT命令最大长度限制，允许将一个超长的XML格式数据分多次下发。XML格式的根节点为<pos>。
     *         命令使用场景：AP侧GPS芯片下发。
     *         命令使用限制：只限定于AP-Modem形态使用。
     *         命令使用通道：只能通过与AP对接的AT通道或MUX通道下发。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CPOS<CR>
     *             textisentered<ctrl-Z/ESC>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPOS=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <CR>: MT在命令行结束符<CR>返回4个字符<CR><LF><greater_than><space>（ASCII码值13，10，62，32）。
     *     <ctrl-Z>: 字符“0x1A”，表示内容输入结束并发送定位命令。
     *     <ESC>: 字符“0x1B”，表示取消本次发送定位命令操作。
     * [示例]:
     *     · GPS下发XML数据
     *       AT+CPOS
     *       ><?xml version="1.0" encoding="UTF-8"?>
     *       ><pos>
     *       ><assist_data>
     *           ><GPS_assist>
     *               ><ref_time>
     *                   ><GPS_time>
     *                       ><GPS_TOW_msec>518935680</GPS_TOW_msec>
     *                       ><GPS_week>282</GPS_week>
     *                   ></GPS_time>
     *               ></ref_time>
     *           ></GPS_assist>
     *       ></assist_data>
     *       ></pos><ctrl-Z>
     *       OK
     *     · 测试命令
     *       AT+CPOS=?
     *       OK
     */
    { AT_CMD_CPOS,
      AT_SetCposPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CPOS", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置是否主动上报定位信息
     * [说明]: 该命令用于AP-Modem形态下，设置是否主动上报网侧定位信息到AP侧，使GPS芯片获取网络的测量指示，上电开机默认为不允许主动上报。
     *         命令使用场景：AP侧GPS芯片下发。
     *         命令使用限制：只限定于AP-Modem形态使用。
     *         命令使用通道：只能通过与AP对接的AT通道或MUX通道下发。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CPOSR=[<mode>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CPOSR?
     *     [结果]: <CR><LF>+CPOSR: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CPOSR=?
     *     [结果]: <CR><LF>+CPOSR: (list of supported <mode>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，是否启用定位信息的主动上报，取值0或1。
     *             0：不允许主动上报；
     *             1：允许主动上报。
     * [示例]:
     *     · 设置定位结果主动上报
     *       AT+CPOSR=1
     *       OK
     *     · 查询命令
     *       AT+CPOSR?
     *       +CPOSR: 1
     *       OK
     *     · 测试命令
     *       AT+CPOSR=?
     *       +CPOSR: (0,1)
     *       OK
     */
    { AT_CMD_CPOSR,
      AT_SetCposrPara, AT_NOT_SET_TIME, AT_QryCposrPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CPOSR", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置静音控制
     * [说明]: 设置上行静音控制。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CMUT=<n>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CMUT?
     *     [结果]: <CR><LF>+CMUT: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CMUT=?
     *     [结果]: <CR><LF>+CMUT: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值。
     *             0：解除静音；
     *             1：设置静音。
     * [示例]:
     *     · 设置命令
     *       AT+CMUT=0
     *       OK
     *     · 查询命令
     *       AT+CMUT?
     *       ^CMUT: 1
     *       OK
     *     · 测试命令
     *       AT+CMUT=?
     *       +CMUT: (0-1)
     *       OK
     */
    { AT_CMD_CMUT,
      AT_SetCmutPara, AT_SET_VC_PARA_TIME, AT_QryCmutPara, AT_QRY_VC_PARA_TIME, AT_TestCmutPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CMUT", (VOS_UINT8 *)"(0,1)" },
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 读取运营商名称
     * [说明]: 获取MT保存的所有运营商名称信息。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +COPN
     *     [结果]: 执行成功时：
     *             <CR><LF>+COPN: <numeric1>,<alpha1>[<CR><LF>+COPN: <numeric2>,<alpha2>[…]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +COPN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <numericn>: 字符串类型，数字格式的运营商信息
     *     <alphan>: 字符串类型，长字符串格式的运营商信息
     * [示例]:
     *     · 查询运营商信息
     *       AT+COPN
     *       +COPN: "00101","Test PLMN 1-1"
     *       …
     *       +COPN: "00011","INMARSAT"
     *       OK
     *     · 测试COPN
     *       AT+COPN=?
     *       OK
     */
    { AT_CMD_COPN,
      AT_SetCopnPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+COPN", VOS_NULL_PTR },

    { AT_CMD_RELEASERRC,
      At_SetReleaseRrc, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"+CNMPSD", VOS_NULL_PTR },

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 查询IMS域注册状态
     * [说明]: 查询当前IMS域网络注册状态，以及设置状态上报方式。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CIREG=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CIREG?
     *     [结果]: <CR><LF>+CIREG: <n>,<reg_info>[,<ext_info>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CIREG=?
     *     [结果]: <CR><LF>+CIREG: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，主动上报方式，默认值为0。
     *             0：禁止+CIREGU的主动上报；
     *             1：使能+CIREGU: <reg_info>的主动上报；
     *             2：使能+CIREGU: <reg_info>[,<ext_info>]的主动上报。
     *     <reg_info>: 整型值，注册状态。
     *             0：没有注册；
     *             1：注册了IMS网络。
     *     <ext_info>: 整型值，IMS域能力值，取值范围[1,0xFFFFFFFF]，每个bit位表示不同的能力。
     *             1:  RTP-based transfer of voice
     *             2:  RTP-based transfer of text
     *             4:  SMS using IMS functionality
     *             8： RTP-based transfer of video
     *             例如：=5的时候，表示支持RTP-based transfer of voice和SMS using IMS functionality
     * [示例]:
     *     · 设置IMS域注册状态上报
     *       AT+CIREG=1
     *       OK
     *     · <n>=1时查询注册状态
     *       AT+CIREG?
     *       +CIREG: 1,1
     *       OK
     *     · <n>=2时查询注册状态
     *       AT+CIREG?
     *       +CIREG: 2,1,5
     *       OK
     *     · 测试CIREG
     *       AT+CIREG=?
     *       +CIREG: (0-2)
     *       OK
     */
    { AT_CMD_CIREG,
      AT_SetCiregPara, AT_SET_PARA_TIME, AT_QryCiregPara, AT_QRY_PARA_TIME, AT_TestCiregPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CIREG", (VOS_UINT8 *)"(0-2)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 查询SRVCC状态
     * [说明]: 查询SRVCC状态，以及设置状态上报方式。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CIREP=[<reporting>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CIREP?
     *     [结果]: <CR><LF>+CIREP: < reporting>,< nwimsvops><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CIREP=?
     *     [结果]: <CR><LF>+CIREP: (list of supported <reporting>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <reporting>: 整型值，主动上报方式，默认值为0。
     *             0：禁止+CIREPH和+CIREPI的主动上报；
     *             1：使能+CIREPH和+CIREPI的主动上报。
     *     <nwimsvops>: 整型值，注册状态。
     *             0：网络不支持IMSVOPS；
     *             1：网络支持IMSVOPS。
     * [示例]:
     *     · 设置SRVCC状态主动上报
     *       AT+CIREP=1
     *       OK
     *     · 查询网络是否支持IMSVOPS
     *       AT+CIREP?
     *       +CIREP: 1,0
     *       OK
     *     · 测试CIREP
     *       AT+CIREP=?
     *       +CIREP: (0,1)
     *       OK
     */
    { AT_CMD_CIREP,
      AT_SetCirepPara, AT_SET_PARA_TIME, AT_QryCirepPara, AT_QRY_PARA_TIME, AT_TestCirepPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CIREP", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置语音呼叫优选模式
     * [说明]: The set command is used to set the MT to operate according to the specified voice domain preference for E-UTRAN
     * [语法]:
     *     [命令]: +CEVDP=[<setting>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             如果IMS语音业务功能关闭或者IMS不支持的情况下，下发AT+CEVDP=4设置为语音模式PS_ONLY时返回失败
     *             在IMS语音业务存在时，不允许设置语音呼叫优选模式。
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CEVDP?
     *     [结果]: <CR><LF>+CEVDP: <setting><CR><LF>
     *     [命令]: +CEVDP=?
     *     [结果]: <CR><LF>+CEVDP: (list of supported <setting>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <setting>: 整型值，语音呼叫模式，默认值由具体制造商决定。
     *             1：CS Voice only
     *             2：CS Voice preferred, IMS PS Voice as secondary
     *             3：IMS PS Voice preferred, CS Voice as secondary
     *             4：IMS PS Voice only
     * [示例]:
     *     · 设置语音CS域优先模式
     *       AT+CEVDP=2
     *       OK
     */
    { AT_CMD_CEVDP,
      AT_SetCevdpPara, AT_SET_PARA_TIME, AT_QryCevdpPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CEVDP", (VOS_UINT8 *)"(1-4)" },

#endif

#if (FEATURE_AT_HSUART == FEATURE_ON)
    { AT_CMD_IPR,
      AT_SetIprPara, AT_NOT_SET_TIME, AT_QryIprPara, AT_NOT_SET_TIME, AT_TestIprPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+IPR", (VOS_UINT8 *)"(0-4000000)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置串口通信的帧格式和校验算法
     * [说明]: 该命令用于设置串口通信的帧格式和校验算法。
     *         本命令只支持HSUART端口下发，其他端口下发直接返回OK。
     *         只用于MBB模块产品。
     * [语法]:
     *     [命令]: AT+ICF[=<format>[,<parity>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: AT+ICF?
     *     [结果]: <CR><LF>+ICF: <format>[,<parity>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: AT+ICF=?
     *     [结果]: <CR><LF>+ICF: (list of supported <format>s),(list of supported <parity>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <format>: 整型值，帧格式。
     *             0：自动检测（暂不支持）；
     *             1：8个数据位2个停止位；
     *             2：8个数据位1个校验位1个停止位；
     *             3：8个数据位1个停止位；
     *             4：7个数据位2个停止位；
     *             5：7个数据位1个校验位1个停止位；
     *             6：7个数据为1个停止位。
     *     <parity>: 整型值，校验算法。
     *             0：奇校验；
     *             1：偶校验；
     *             2：标记校验（不支持）；
     *             3：空校验（不支持）。
     * [示例]:
     *     · 设置串口通信的帧格式和校验算法
     *       AT+ICF=3,0
     *       OK
     *     · 查询串口通信的帧格式和校验算法
     *       AT+ICF?
     *       +ICF: 3,0
     *       OK
     *     · 测试命令
     *       AT+ICF=?
     *       +ICF: (0-6),(0-1)
     *       OK
     */
    { AT_CMD_ICF,
      AT_SetIcfPara, AT_NOT_SET_TIME, AT_QryIcfPara, AT_NOT_SET_TIME, AT_TestIcfPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+ICF", (VOS_UINT8 *)"(1-6),(0-3)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置串口的流控方式
     * [说明]: 该命令用于设置串口通信的流控方式。
     *         本命令只支持HSUART端口形态，其他端口形态直接返回OK。
     *         只用于MBB模块产品，且上下行流控只能同时开启活同时关闭。
     * [语法]:
     *     [命令]: AT+IFC[=<DCE_by_DTE>
     *             [,<DTE_by_DCE>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: AT+IFC?
     *     [结果]: <CR><LF>+IFC: <DCE_by_DTE>,<DTE_by_DCE><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: AT+IFC=?
     *     [结果]: <CR><LF>+IFC: (list of supported <DCE_by_DTE>s),(list of supported <DTE_by_DCE>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <DCE_by_DTE>: 整型值，DTE控制DCE数据方式
     *             0：无
     *             1：XON/XOFF控制，并从数据流中去掉XON/XOFF字符（暂不支持）；
     *             2：硬件流控；
     *             3：XON/XOFF控制，不去掉数据流中XON/XOFF字符（暂不支持）。
     *     <DTE_by_DCE>: 整型值，DCE控制DTE数据方式
     *             0：无；
     *             1：XON/XOFF控制，并从数据流中去掉XON/XOFF字符（暂不支持）；
     *             2：硬件流控。
     * [示例]:
     *     · 设置串口通信的流控方式
     *       AT+IFC=2,2
     *       OK
     *     · 查询串口通信的流控方式
     *       AT+IFC?
     *       +ICF: 2,2
     *       OK
     *     · 测试命令
     *       AT+IFC=?
     *       +ICF: (0,2),(0,2)
     *       OK
     */
    { AT_CMD_IFC,
      AT_SetIfcPara, AT_NOT_SET_TIME, AT_QryIfcPara, AT_NOT_SET_TIME, AT_TestIfcPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+IFC", (VOS_UINT8 *)"(0,2),(0,2)" },
#endif
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 时钟命令
     * [说明]: 该命令是时钟命令，包括设置、查询时钟值。目前查询的时钟值非实时时间，且只在开机时读取一次时间，之后查询到的时间不变。
     * [语法]:
     *     [命令]: +CCLK=<time>
     *     [结果]: 成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: +CCLK?
     *     [结果]: <CR><LF +CCLK: <time><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CCLK=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <time>: 字符串类型，格式为："yy/MM/dd,hh:mm:ss±zz"或者"yyyy/MM/dd,hh:mm:ss±zz"。
     * [示例]:
     *     · 设置时钟
     *       AT+CCLK="17/12/12,11:35:26+10"
     *       OK
     *     · 查询时钟
     *       AT+CCLK?
     *       +CCLK: "17/12/12,11:35:26+10"
     *       OK
     *     · 测试命令
     *       AT+CCLK=?
     *       OK
     */
    { AT_CMD_CCLK,
      AT_SetCclkPara, AT_NOT_SET_TIME,
#if (FEATURE_MBB_CUST == FEATURE_ON)
      At_MbbQryCclkPara,
#else
      AT_QryCclkPara,
#endif
      AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CCLK", (VOS_UINT8 *)"(time)" },

    { AT_CMD_CTZU,
      AT_SetCtzuPara, AT_NOT_SET_TIME, AT_QryCtzuPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CTZU", (VOS_UINT8 *)"(0-1)" },
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    { AT_CMD_CRM,
      AT_SetCrmPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CRM", (VOS_UINT8 *)"(1,2)" },

    { AT_CMD_CTA,
      AT_SetCtaPara, AT_SET_PARA_TIME, AT_QryCtaPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CTA", (VOS_UINT8 *)"(0-255)" },
#endif

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置语音中心数据中心
     * [说明]: 设置当前UE模式为语音中心模式或者数据中心模式。
     * [语法]:
     *     [命令]: +CEUS=[<setting>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CEUS?
     *     [结果]: <CR><LF>+CEUS: <setting><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CEUS=?
     *     [结果]: <CR><LF>+CEUS: (list of supported <setting>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <setting>: 整形值，语音数据中心模式，参数不带时默认设置语音中心
     *             0：语音中心
     *             1：数据中心
     * [示例]:
     *     · 设置语音中心
     *       AT+CEUS=0
     *       OK
     *     · 查询语音数据中心状态
     *       AT+CEUS?
     *       +CEUS: 0
     *       OK
     *     · 测试命令
     *       AT+CEUS=?
     *       +CEUS: (0,1)
     *       OK
     */
    { AT_CMD_UE_CENTER,
      AT_SetUECenterPara, AT_SET_PARA_TIME, AT_QryUECenterPara, AT_QRY_PARA_TIME, AT_TestUECenterPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CEUS", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置快速双卡切换场景
     * [说明]: 该AT命令用于通知modem快速双卡切换场景，在软关机之后下发。
     *         对该AT命令的约束：
     *         （1）该AT命令主要在如下场景使用：
     *         Ⅰ: EMUI界面，用户点击数据业务切换导致的切卡
     *         Ⅱ: 双卡场景下，用户去激活主卡（原有的副卡会切成主卡）；
     *         Ⅲ: 双卡去激活的情况下，激活原来的副卡(此时它变成主卡)。
     *         （2）该AT命令只能在modem软关机后下发；
     * [语法]:
     *     [命令]: ^QUICKCARDSWITCH=<SAMPLMN>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <SAMPLMN>: 指示是否为相同运营商快速双卡切换场景：
     *             0：为双卡不同运营商场景。
     *             1：为双卡相同运营商场景。
     *             以下为快速双卡切换场景：
     *             1）：EMUI界面，用户点击数据业务切换导致的切卡；
     *             2）：双卡场景下，用户去激活主卡（原有的副卡会切成主卡）；
     *             3）：双卡去激活的情况下，激活原来的副卡(此时它变成主卡)。
     * [示例]:
     *     · 指示为相同运营商快速双卡切换场景：
     *       AT^QUICKCARDSWITCH=1；
     *       OK
     */
    { AT_CMD_QUICK_CARD_SWITCH,
      AT_SetQuickCardSwitchPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^QUICKCARDSWITCH", (VOS_UINT8 *)"(0,1)" },
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置终端适配器模式
     * [说明]: 此命令把终端适配器设置成数据、传真或者语音的特殊操作模式。
     *         目前暂不支持，返回ERROR。
     * [语法]:
     *     [命令]: +FCLASS=<cmd>
     *     [结果]: 设置执行时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <cmd>: 终端适配器模式，整数型
     * [示例]:
     *     · 执行设置命令
     *       AT+FCLASS=1
     *       ERROR
     *     · 执行查询命令
     *       AT+FCLASS?
     *       ERROR
     *     · 执行测试命令
     *       AT+FCLASS=?
     *       ERROR
     */
    { AT_CMD_FCLASS,
      AT_SetFclassPara, AT_SET_PARA_TIME, AT_QryFclassPara, AT_QRY_PARA_TIME, At_TestFclass, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+FCLASS", (VOS_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置T.35国家码
     * [说明]: 此命令用于设置T.35国家码。
     *         目前暂不支持返回ERROR。
     * [语法]:
     *     [命令]: +GCI=<cmd>
     *     [结果]: 设置执行时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <cmd>: 设置T.35国家码，整数型
     * [示例]:
     *     · 执行设置命令
     *       AT+GCI=1
     *       ERROR
     *     · 执行查询命令
     *       AT+GCI?
     *       ERROR
     *     · 执行测试命令
     *       AT+GCI=?
     *       ERROR
     */
    { AT_CMD_GCI,
      AT_SetGciPara, AT_SET_PARA_TIME, AT_QryGciPara, AT_QRY_PARA_TIME, At_TestGci, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+GCI", (VOS_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 主叫名称显示
     * [说明]: 打开或关闭主叫名称的显示功能，查询用户主叫名称显示补充业务的签约情况。如果打开了主叫名称显示功能且用户签约了主叫名称显示补充业务，则来电时会通过主动上报AT命令+CNAP上报主叫名称。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CNAP=<n>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CNAP?
     *     [结果]: <CR><LF>+CNAP: <n>,<m><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CNAP=?
     *     [结果]: <CR><LF>+CNAP: (list of supported <n>s)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，打开或关闭+CNAP命令的主动上报，默认值为0：
     *             0：关闭；
     *             1：打开。
     *     <m>: 整型值，主叫名称显示业务在网络的签约状态：
     *             0：CNAP业务未提供；
     *             1：CNAP业务已提供；
     *             2：未知（网络原因）。
     * [示例]:
     *     · 打开主叫名称显示功能
     *       AT+CNAP=1
     *       OK
     *     · 查询主叫名称显示业务签约状态
     *       AT+CNAP?
     *       +CNAP: 1,1
     *       OK
     *     · 执行测试命令
     *       AT+CNAP=?
     *       +CNAP: (0,1)
     *       OK
     */
    { AT_CMD_CNAP,
      AT_SetCnapPara, AT_SET_PARA_TIME, AT_QryCnapPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CNAP", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 控制IPv6地址打印格式
     * [说明]: 此命令用于控制AT命令参数包含IPv6地址的打印格式。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +CGPIAF=[<IPv6_AddressFormat>[,<IPv6_SubnetNotation>[,<IPv6_LeadingZeros>[,<IPv6_CompressZeros>]]]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: +CGPIAF?
     *     [结果]: <CR><LF>+CGPIAF: <IPv6_AddressFormat>, <IPv6_SubnetNotation>, <IPv6_LeadingZeros>, <IPv6_CompressZeros><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CGPIAF=?
     *     [结果]: <CR><LF>+CGPIAF: (list of supported <IPv6_AddressFormat>s),(list of supported <IPv6_SubnetNotation>s),(list of supported <IPv6_LeadingZeros>s),(list of supported <IPv6_CompressZeros>s)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <IPv6_AddressFormat>: 整型值，IPv6地址格式，默认值为0：
     *             0：使用类似IPv4的点分隔符，IP地址和子网掩码（如果存在）也通过点分隔；
     *             1：使用类似IPv6的冒号分隔符，IP地址和子网掩码（如果存在）通过空格分隔。
     *     <IPv6_SubnetNotation>: 整型值，控制子网掩码参数<remote address and subnet mask>的格式，<IPv6_AddressFormat>参数为1生效：
     *             0：IP地址和子网掩码完整显示并通过空格分隔；
     *             1：通过斜线分隔子网前缀无类域与IP地址。
     *     <IPv6_LeadingZeros>: 整型值，控制是否省略前导0，<IPv6_AddressFormat>参数为1生效：
     *             0：省略前导0；
     *             1：包含前导0。
     *     <IPv6_CompressZeros>: 整型值，控制多个16bit值为0的实例是否使用“::”替换（压缩0格式），<IPv6_AddressFormat>参数为1生效：
     *             0：不压缩0；
     *             1：压缩0。
     * [示例]:
     *     · 设置IP打印格式参数
     *       AT+CGPIAF=0,0,0,0
     *       OK
     *     · 查询IP打印格式参数
     *       AT+CGPIAF?
     *       +CGPIAF: 0,0,0,0
     *       OK
     *     · 测试命令
     *       AT+CGPIAF=?
     *       +CGPIAF: (0,1),(0,1),(0,1),(0,1)
     *       OK
     */
    { AT_CMD_CGPIAF,
      AT_SetCgpiafPara, AT_NOT_SET_TIME, AT_QryCgpiafPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CGPIAF", (VOS_UINT8 *)"(0,1),(0,1),(0,1),(0,1)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 设置日期格式
     * [说明]: 该命令主要用于设置日期格式样式，目前只支持部分功能。
     * [语法]:
     *     [命令]: +CSDF=[[<mode>][,<auxmode>]]
     *     [结果]: 成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: +CSDF?
     *     [结果]: <CR><LF +CSDF: <mode>[,<auxmode>] <CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CSDF=?
     *     [结果]: <CR><LF> +CSDF: (list of supported <mode>s)[,(list of supported <auxmode>s)]<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，日期显示格式（功能暂时不支持），取值范围为(1-255)：
     *             1：DD-MMM-YYYY；
     *             2：DD-MM-YY；
     *             3：MM/DD/YY；
     *             4：DD/MM/YY；
     *             5：DD.MM.YY；
     *             6：YYMMDD；
     *             7：YY-MM-DD；
     *             8-255：保留。
     *     <auxmode>: +CCLK和+CALA（暂不支持）中<time>格式的样式，整型值，取值范围(1,2)，默认值为1：
     *             1：yy/MM/dd；
     *             2：yyyy/MM/dd。
     * [示例]:
     *     · 设置日期格式样式
     *       AT+CSDF=1,1
     *       OK
     *     · 查询日期格式样式
     *       AT+CSDF?
     *       +CSDF: 1,1
     *       OK
     *     · 测试命令
     *       AT+CSDF=?
     *       +CSDF: (1-255),(1-2)
     *       OK
     */
    { AT_CMD_CSDF,
      AT_SetCsdfPara, AT_NOT_SET_TIME, AT_QryCsdfPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CSDF", (VOS_UINT8 *)"(1-255),(1-2)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 分组域事件上报策略设置命令
     * [说明]: 该命令主要用于分组域事件上报策略的制定，主要控制+CGEV命令（暂不支持）。
     * [语法]:
     *     [命令]: +CGEREP=[<mode>[,<bfr>]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: +CGEREP?
     *     [结果]: <CR><LF>+CGEREP: <mode>,<bfr><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CGEREP=?
     *     [结果]: <CR><LF>+CGEREP: (list of supported <mode>s),(list of supported
     *             <bfr>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，控制主动上报存储策略，取值范围(0-2)，默认值为0。
     *     <bfr>: 整型值，控制缓存的主动上报行为策略，取值范围(0-1)，默认值为0。
     * [示例]:
     *     · 设置分组域事件上报策略
     *       AT+CGEREP=0,0
     *       OK
     *     · 查询分组域事件上报策略
     *       AT+CGEREP?
     *       +CGEREP: 0,0
     *       OK
     *     · 测试命令
     *       AT+CGEREP=?
     *       +CGEREP: (0-2),(0,1)
     *       OK
     */
    { AT_CMD_CGEREP,
      AT_SetCgerepPara, AT_SET_PARA_TIME, AT_QryCgerepPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CGEREP", (VOS_UINT8 *)"(0-2),(0,1)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 指标控制命令
     * [说明]: 该命令是指标控制命令，目前只支持信号指标控制。
     * [语法]:
     *     [命令]: +CIND=[<ind>[,<ind>[,...]]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: +CIND?
     *     [结果]: <CR><LF>+CIND: <ind>[,<ind>[,...]]<CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: +CIND=?
     *     [结果]: <CR><LF>+CIND: (<descr>,(list of supported
     *             <ind>s))[,(<descr>,(list of supported
     *             <ind>s))[,...]]<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <ind>: 整型值，指标控制值，取值范围参考<descr>的描述。
     *     <descr>: 目前只支持”signal”指标。
     *             "signal"：信号质量(0-5)。
     * [示例]:
     *     · 设置指标
     *       AT+CIND=1
     *       OK
     *     · 查询指标
     *       AT+CIND?
     *       +CIND: 1
     *       OK
     *     · 测试命令
     *       AT+CIND=?
     *       +CIND: "signal",(0-5)
     *       OK
     */
    { AT_CMD_CIND,
      AT_SetCindPara, AT_SET_PARA_TIME, AT_QryCindPara, AT_QRY_PARA_TIME, AT_TestCindPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CIND", (VOS_UINT8 *)"(0-5)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 语音挂断控制
     * [说明]: 设置命令控制ATH是否可以挂断语音呼叫。
     *         读取命令返回当前模式
     *         测试命令返回支持的参数范围
     *         默认初始值为0，即支持ATH挂断语音通话。（初始值可通过NV
     *         en_NV_Item_Custom_Call_Cfg 配置，若配置为1，则不支持ATH挂断语音
     * [语法]:
     *     [命令]: +CVHU=[<mode>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CVHU?
     *     [结果]: <CR><LF> +CVHU: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: +CVHU=?
     *     [结果]: <CR><LF>+CVHU: (list of supported <mode>s)
     *             <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 0 ：下发ATH可以挂断语音通话
     *             1 ：下发ATH只是返回OK，不会影响语音通话
     * [示例]:
     *       下发AT+CVHU=0后，语音通话中下发ATH挂断呼叫，呼叫断开。
     *       下发AT+CVHU=1后，语音通话中下发ATH挂断呼叫，只是返回OK，呼叫未断开。
     */
    { AT_CMD_CVHU,
      AT_SetCvhuPara, AT_SET_PARA_TIME, AT_QryCvhuPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CVHU", (VOS_UINT8 *)"(0-1)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 设置5G QoS
     * [说明]: 设置或删除指定cid的5G QoS参数。
     *         如果命令中只有<cid>一个参数，如AT+C5GQOS=<cid>，表示删除此<cid>对应的5G QoS参数。
     *         本命令遵从3GPP TS 27.007 （R15以上）协议。
     * [语法]:
     *     [命令]: +C5GQOS=[<cid>[,<5QI>[,<DL_GFBR>,<UL_GFBR>[,<DL_MFBR>,<UL_MFBR]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err> <CR><LF>
     *     [命令]: +C5GQOS?
     *     [结果]: [+C5GQOS: <cid>,<5QI>[,<DL_GFBR>,<UL_GFBR>[,<DL_MFBR>,<UL_MFBR>]]
     *             [<CR><LF>+C5GQOS: <cid>,<5QI>,[<DL_GFBR>,<UL_GFBR>[,<DL_MFBR>,<UL_MFBR>]]
     *             [...]]<CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +C5GQOS=?
     *     [结果]: +C5GQOS: (range of supported <cid>s),(list of supported <5QI>s),(list of supported <DL_GFBR>s),(list of supported <UL_GFBR>s),(list of supported <DL_MFBR>s),(list of supported <UL_MFBR>s)
     *             <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             取值范围1~31。
     *     <5QI>: 整型值，（AT命令设置时，modem不对此参数进行检查）
     *             0: 表示此值由网侧确定；
     *             1-4 65,66,,67, 71-76:    values for guaranteed bit rate QoS flows
     *             5-9 69,70,79,80:  value range for non-guaranteed bit rate QoS flows
     *             10-12 16,17:     value range for delay critical guaranteed bit rate QoS flows
     *             128-254: 运营商定制值
     *     <DL_GFBR>: 整型值，0~4294967295，表示GBR下的DL GFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     *     <UL_GFBR>: 整型值，0~4294967295，表示GBR下的UL GFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     *     <DL_MFBR>: 整型值，0~4294967295，表示GBR下的DL MFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     *     <UL_MFBR>: 整型值，0~4294967295，表示GBR下的UL MFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     * [示例]:
     *     · 设置命令
     *       AT+C5GQOS=1,1,1000,2000,3000,4000
     *       OK
     *     · 查询命令
     *       AT+C5GQOS?
     *       +C5GQOS: 1,1,1000,2000,3000,4000
     *       +C5GQOS: 2,65,1000,1000,1000,1000
     *       OK
     *     · 测试命令
     *       AT+C5GQOS=?
     *       +C5GQOS: (1-31),(0-12,16-17,65-67,69-76,79-80,128-254),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)
     *       OK
     */
    { AT_CMD_C5GQOS,
      AT_SetC5gqosPara, AT_SET_PARA_TIME, AT_QryC5gqosPara, AT_QRY_PARA_TIME, At_TestC5gqos, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+C5GQOS", (VOS_UINT8 *)"(1-31),(0-12,16-17,65-67,69-76,79-80,128-254),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询5G 动态QoS参数
     * [说明]: 查询指定cid或者全部已激活的cid的网络侧分配的5G QoS参数。
     *         如果命令中只有<cid>一个参数，如AT+C5GQOSRDP=<cid>，表示查询<cid>对应的5G QoS参数。
     *         如果命令中不携带<cid>参数，如AT+C5GQOSRDP，表示查询所有已激活的QoS flow对应的5G QoS参数。
     *         测试命令返回处于激活状态的PDP上下文关联的<cid>列表。
     *         本命令遵从3GPP TS 27.007 （R15以上）协议。
     * [语法]:
     *     [命令]: +C5GQOSRDP[=<cid>]
     *     [结果]: [+C5GQOSRDP: <cid>,<5QI>[,<DL_GFBR>,<UL_GFBR>[,<DL_MFBR>,<UL_MFBR>[,<DL_SAMBR>,<UL_SAMBR>[,<Averaging_window>]]]]]
     *             [<CR><LF>+C5GQOSRDP: <cid>,<5QI>[,<DL_GFBR>,<UL_GBR>[,<DL_MFBR>,<UL_MFBR>[,<DL_SAMBR>,<UL_SAMBR>[,<Averaging_window>]]]]
     *             [...]]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +C5GQOSRDP=?
     *     [结果]: <CR><LF>+C5GQOSRDP: (list of <cid>s associated with active contexts) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cid>: 整型值，PDP上下文标识符。
     *             取值范围1~31。
     *     <5QI>: 整型值，（AT命令设置时，modem不对此参数进行检查）
     *             0:             表示此值由网侧确定；
     *             1-4 65,66,75:    values for guaranteed bit rate QoS flows
     *             5-9 69,70,79,80:  value range for non-guaranteed bit rate QoS flows
     *             10-12 16,17:     value range for delay critical guaranteed bit rate QoS flows
     *             128-254:        运营商定制值
     *     <DL_GFBR>: 整型值，0~4294967295，表示GBR下的DL GFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     *     <UL_GFBR>: 整型值，0~4294967295，表示GBR下的UL GFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     *     <DL_MFBR>: 整型值，0~4294967295，表示GBR下的DL MFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     *     <UL_MFBR>: 整型值，0~4294967295，表示GBR下的UL MFBR，单位是kbit/s。此参数仅在5QI为GBR取值时有效
     *     <UL_SAMBR>: 整型值，0~4294967295，表示UL PDU session AMBR，单位是kbit/s。
     *     <DL_SAMBR>: 整型值，0~4294967295，表示DL PDU session AMBR，单位是kbit/s。
     *     <Averaging_window>: 整型值，0~65535，表示averaging window，单位是ms。
     * [示例]:
     *     · 查询命令
     *       AT+C5GQOSRDP=1
     *       +C5GQOSRDP: 1,1,1000,2000,3000,4000,10000,10000,2000
     *       OK
     *     · 测试命令
     *       AT+C5GQOSRDP=?
     *       +C5GQOSRDP: (1,9)
     *       OK
     */
    { AT_CMD_C5GQOSRDP,
      AT_SetC5gqosRdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestC5gQosRdp, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+C5GQOSRDP", (VOS_UINT8 *)"(1-31)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置优选切片
     * [说明]: 该命令用于设置用户的优选切片信息，该命令用于设置3GPP的优选切片，目前NON_3GPP的优选切片不支持配置。Prefer切片是跟HPLMN绑定的，设置的Prefer切片不含Mapped SST、Mapped SD信息。
     * [语法]:
     *     [命令]: +C5GPNSSAI=
     *             [<Preferred_NSSAI_3gpp_length>,
     *             [<Preferred_NSSAI_3gpp>]],
     *             [<Preferred_NSSAI_non3gpp_length>,
     *             [<Preferred_NSSAI_non3gpp>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +C5GPNSSAI?
     *     [结果]: <CR><LF>+C5GPNSSAI: <Preferred_NSSAI_3gpp_length>,
     *             <Preferred_NSSAI_3gpp>,
     *             <Preferred_NSSAI_non3gpp_length>,
     *             <Preferred_NSSAI_non3gpp><C
     *             R><LF><CR><LF>OK<CR><LF>
     *     [命令]: +C5GPNSSAI=?
     *     [结果]: +C5GPNSSAI: (range of supported <Preferred_NSSAI_3gpp_length>s),(range of supported <Preferred_NSSAI_non3gpp_length>s)
     *             <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <Preferred_NSSAI_3gpp_length>: AT命令设置的3GPP优选切片字节个数
     *     <Preferred_NSSAI_3gpp>: 3GPP优选切片码流，目前支持最多8个Prefer切片的设置，
     *             单个切片的格式：SST或者SST.SD，
     *             不同的切片用符号“:”分开，举例：“SST:SST.SD”
     *     <Preferred_NSSAI_non3gpp_length>: NON_3GPP目前不支持
     *     <Preferred_NSSAI_non3gpp>: NON_3GPP目前不支持
     * [示例]:
     *     · 设置优选切片
     *       AT+C5GPNSSAI=8,"01:02:03:04:05:06:07:08"
     *       OK
     *     · 查询优选切片
     *       AT+C5GPNSSAI?
     *       +C5GPNSSAI: 8,"01:02:03:04:05:06:07:08"
     *       OK
     */
    { AT_CMD_C5GPNSSAI,
      AT_SetC5gPNssaiPara, AT_SET_PARA_TIME, AT_QryC5gPNssaiPara, AT_QRY_PARA_TIME, At_TestC5gPNssai, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+C5GPNSSAI", (VOS_UINT8 *)"(0-159),(pnssaiStr)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置Default Config切片
     * [说明]: 该命令用于设置用户的default Config切片信息
     * [语法]:
     *     [命令]: +C5GNSSAI=
     *             <default_configured_nssai_length>,
     *             <default_configured_nssai>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +C5GNSSAI?
     *     [结果]: <CR><LF>+C5GNSSAI: [<default_configured_nssai_length>,
     *             <default_configured_nssai>]<C
     *             R><LF><CR><LF>OK<CR><LF>
     *     [命令]: +C5GNSSAI=?
     *     [结果]: +C5GNSSAI: (range of supported <default_configured_nssai_length>s),(range of supported <default_configured_nssai>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <default_configured_nssai_length>: AT命令设置的Default Config切片字节个数
     *     <default_configured_nssai>: 3GPP优选切片码流，目前支持最多16个Default Config切片的设置
     *             单个切片的格式：“SST”或者“SST.SD”，
     *             不同的切片用符号“:”分开，举例：“SST:SST.SD”
     * [示例]:
     *     · 设置Default config切片
     *       AT+C5GNSSAI=8,"01:02:03:04:05:06:07:08"
     *       OK
     *     · 查询Default config切片
     *       AT+C5GNSSAI?
     *       +C5GNSSAI: 8,"01:02:03:04:05:06:07:08"
     *       OK
     */
    { AT_CMD_C5GNSSAI,
      AT_SetC5gNssaiPara, AT_SET_PARA_TIME, AT_QryC5gNssaiPara, AT_QRY_PARA_TIME, At_TestC5gNssai, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+C5GNSSAI", (VOS_UINT8 *)"(0-319),(nssaiStr)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 查询动态切片信息
     * [说明]: 该命令用于查询用户动态分配的Default Config Nssai、Reject Nssai、Config Nssai、Allow Nssai切片信息。
     * [语法]:
     *     [命令]: +C5GNSSAIRDP
     *             [=<nssai_type>[,<plmn_id>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             [+C5GNSSAIRDP: [<default_configured_nssai_length>,
     *             <default_configured_nssai>[,<rejected_nssai_3gpp_length>,<rejected_nssai_3gpp>[,<rejected_nssai_non3gpp_length>,<rejected_nssai_non3gpp>]]][<CR><LF>+C5GNSSAIRDP: <plmn_id>[,<configured_nssai_length>,<configured_nssai>
     *             [,<allowed_nssai_3gpp_length>,<allowed_nssai_3gpp>,
     *             <allowed_nssai_non3gpp_length>,<allowed_nssai_non3gpp>]][<CR><LF>+C5GNSSAIRDP: <plmn_id>[,<configured_nssai_length>,<configured_nssai>
     *             [,<allowed_nssai_3gpp_length>,<allowed_nssai_3gpp>,<allowed_nssai_non3gpp_length>,<allowed_nssai_non3gpp>]][...]]]]
     *     [命令]: +C5GNSSAIRDP=?
     *     [结果]: +C5GNSSAIRDP: (list of supported <nssai_type>s),(range of supported <plmn_id>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <nssai_type>: 查询的切片类型
     *             0：返回default configured NSSAI
     *             1：返回default configured NSSAI以及rejected NSSAI(s)
     *             2：返回default configured NSSAI, rejected NSSAI(s) 以及  configured NSSAI(s)
     *             3：返回default configured NSSAI, rejected NSSAI(s), configured NSSAI(s) 以及allowed NSSAI(s)
     *     <plmn_id>: 码流形式，PLMN信息，五个或六个字符，其中前三位是MCC，后两位或三位是MNC。
     *     <default_configured_nssai_length>: Default config nssai切片字节个数
     *     <default_configured_nssai>: Default config nssai
     *     <rejected_nssai_3gpp_length>: Reject nssai切片字节个数
     *     <rejected_nssai_3gpp>: 当前驻留PLMN对应的Reject nssai切片码流
     *     <rejected_nssai_non3gpp_length>: NON_3GPP参数目前不支持
     *     <rejected_nssai_non3gpp>: NON_3GPP参数目前不支持
     *     <configured_nssai_length>: Config nssai切片字节个数
     *     <configured_nssai>: Config nssai码流
     *     <allowed_nssai_3gpp_length>: 3gpp Allowed nssai切片字节个数
     *     <allowed_nssai_3gpp>: 3gpp Allowed nssai码流
     *     <allowed_nssai_non3gpp_length>: NON_3GPP参数目前不支持
     *     <allowed_nssai_non3gpp>: NON_3GPP参数目前不支持
     * [示例]:
     *     · 查询所有切片信息
     *       AT+C5GNSSAIRDP=3,"46010"
     *       +C5GNSSAIRDP: 1,"01",2,"02:03"
     *       +C5GNSSAIRDP: "46010",1,"01",1,"01"
     *       OK
     */
    { AT_CMD_C5GNSSAIRDP,
      AT_SetC5gNssaiRdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestC5gNssaiRdp, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+C5GNSSAIRDP", (VOS_UINT8 *)"(0,1,2,3),(plmnStr)" },
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 设置无卡快速开机
     * [说明]: 该命令用于一次性设置无卡快速开机功能，可设置快速开机功能的使能和去使能，重新上下电后自动变为去使能。
     *         该AT命令设置的快速开机使能状态和通过NV6656设置的快速开机使能状态是一样的。唯一的区别是AT命令设置的状态在重新上下电后会失效。
     *         该AT命令设置快速开机的功能是作为NV6656配置功能的一种补充，优点是无需重启单板。
     * [语法]:
     *     [命令]: ^NOCARD=<card_mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NOCARD=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <card_mode>: 设置快速开机使能和去使能。1：使能；0：去使能。
     * [示例]:
     *       AT^NOCARD=1
     *       OK
     *       AT^NOCARD=0
     *       OK
     */
    { AT_CMD_NOCARD,
      AT_SetNoCard, AT_SET_PARA_TIME, AT_QueryNoCard, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NOCARD", (VOS_UINT8 *)"(0,1)" },
#endif
#endif
};

VOS_UINT32 At_RegisterExCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atExtendCmdTbl, sizeof(g_atExtendCmdTbl) / sizeof(g_atExtendCmdTbl[0]));
}

const AT_ParCmdElement* At_GetExtendCmdTable(VOS_VOID)
{
    return g_atExtendCmdTbl;
}

VOS_UINT32 At_GetExtendCmdNum(VOS_VOID)
{
    return sizeof(g_atExtendCmdTbl) / sizeof(g_atExtendCmdTbl[0]);
}

