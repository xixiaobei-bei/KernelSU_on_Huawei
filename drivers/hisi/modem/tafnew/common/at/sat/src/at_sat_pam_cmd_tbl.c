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
#include "at_sat_pam_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_sat_pam_set_cmd_proc.h"
#include "at_sat_pam_qry_cmd_proc.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAT_PAM_CMD_TBL_C
/*
 * [类别]: 协议AT-(U)SAT业务
 * [含义]: (U)SAT类型命令参数说明
 * [表]: (U)SAT类型命令参数说明
 *        参数,                    说明,
 *        <CmdType>,               0：SIM卡已发送了一个“Setup Menu”主动命令。
 *                                 1：SIM卡已发送了一个“Display Text”主动命令。
 *                                 2：SIM卡已发送了一个“Get Inkey”主动命令。
 *                                 3：SIM卡已发送了一个“Get Input”主动命令。
 *                                 4：SIM卡已发送了一个“Setup Call”主动命令。
 *                                 5：SIM卡已发送了一个“Play Tone”主动命令。
 *                                 6：SIM卡已发送了一个“Sel Item”主动命令。
 *                                 7：SIM卡已发送了一个“Refresh”主动命令。
 *                                 8：SIM卡已发送了一个“Send SS”主动命令。
 *                                 9：SIM卡已发送了一个“Send SMS”主动命令。
 *                                 10：SIM卡已发送了一个“Send USSD”主动命令。
 *                                 11：SIM卡已发送了一个“LAUNCH BROWSER”主动命令。
 *                                 12：SIM卡已经发送了一个“SET UP IDLE MODE TEXT”主动命令。
 *                                 99：SIM卡已发送了一个“End Session”（结束会话）。,
 *        <CmdIndex>,              取值范围为0～8，表示主动命令在单板中的位置。,
 *        <isTimeOut>,             取值范围为0～1，表示该主动命令是否超时。
 *                                 0：不超时；
 *                                 1：超时。,
 *        <Alpha Identifier menu>, 主菜单的Alpha Identifier，即标题。
 *                                 解码方式为：判断第一个字节是否是80、81或82，如果是，就按照UCS2方式解码；否则按照非压缩的GSM7位编码方式解码。,
 *        <Id>,                    (0)，表示主菜单。,
 *        <Idx>,                   x取值范围为0～255，菜单项的Identifier。,
 *        <NbItems>,               取值范围为0～255，主菜单中的选项个数。,
 *        <Alpha Idx Label>,       选项的Alpha Identifier label，菜单选项名称。
 *                                 解码方式为：判断第一个字节是否是80、81或82，如果是，就按照UCS2方式解码；否则按照非压缩的GSM7位编码方式解码。
 *                                 <HelpInfo>, 取值范围为0～1。
 *                                 0：帮助信息不可用；
 *                                 1：帮助信息可用。,
 *        <NextActionId>,          包括一个主动命令Identifier，TA总可以在收到^STIN:0的通知之后获取setup menu的有关信息。,
 *        <TextInfo>,              字符串类型，要显示的文本信息。,
 *        <TextFormat>,            要显示的文本的编码方式。
 *                                 0：GSM7位压缩编码；
 *                                 4：8bit编码；
 *                                 8：UCS2编码。,
 *        <ClearMode>,             用户关闭提示模式。
 *                                 0：在一段延迟之后清显示的文本信息；
 *                                 1：等待用户清除显示的文本信息。,
 *        <DurationTime>,          显示文本请求显示的持续时间，单位为秒。,
 *        <rspFormat>,             用户输入的方式或输入内容的字符类型。
 *                                 0：GSM7编码；
 *                                 1：YES NO模式；
 *                                 2：数字（0～9，*，#，and+）；
 *                                 3：UCS2。,
 *        <Timeout>,               超时时间，单位为秒。,
 *        <PackMode>,              取值范围0～1，表示当前是否为压缩模式。
 *                                 0：未压缩模式；
 *                                 1：压缩模式。,
 *        <EchoMode>,              取值范围0～1，表示当前是否为回显模式。
 *                                 0：关闭回显；
 *                                 1：打开回显。,
 *        <SizeMin>,               取值范围为1～255，最小输入长度。,
 *        <SizeMax>,               取值范围为1～255，最大输入长度。,
 *        <DefaultTextInfo>,       文本信息，默认用户输入字符串。,
 *        <ToneType>,              Tone的类型。
 *                                 0：Tone Dial；
 *                                 1：Tone Busy；
 *                                 2：Tone Conestion；
 *                                 3：Tone Radio ack；
 *                                 4：Tone Dropped；
 *                                 5：Tone Error；
 *                                 6：Tone Call waitting；
 *                                 7：Tone Ringing；
 *                                 8：Tone General beep；
 *                                 9：Tone Positive beep；
 *                                 10：Tone Negative beep。
 *                                 当没有指定tone时，ME将使用默认的general beep。,
 *        <textCode>,              同<TextFormat>。,
 *        <icon>,                  图标信息。,
 *        <DefaultItem>,           取值范围为1～255，默认Item Identifier。,
 *        <Refresh Type>,          刷新类型。
 *                                 0：NAA初始化；
 *                                 1：NAA文件变化通知（FCN）；
 *                                 2：NAA初始化及文件变化通知（FCN）；
 *                                 3：NAA初始化及全部文件变化通知（FCN）；
 *                                 4：UICC重启；
 *                                 5：NAA应用重启；
 *                                 6：NAA会话重启。,
 *        <File List>,             需要更新的文件列表。,
 *        <URL>,                   Uniform Resource Location。,
 *        <LaunchMode>,            发起模式。
 *                                 0：如果没有启动浏览器，启动浏览器；
 *                                 2：使用已经存在的浏览器（不使用已经激活会话的浏览器）；
 *                                 3：关闭已经存在的浏览器，打开新浏览器。,
 */

static const AT_ParCmdElement g_atSatPamCmdTbl[] = {
    /* USAT */
    /*
     * [类别]: 协议AT-(U)SAT业务
     * [含义]: 设置STK
     * [说明]: 该命令用来配置与SIM卡协商支持的STK命令，配置信息写入相关NV项，在下一次开机的时候生效。
     * [语法]:
     *     [命令]: ^STSF=<Mode>[,<Config>][,<Timeout>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^STSF?
     *     [结果]: <CR><LF>^STSF: <Mode>, <Config>, <Timeout><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^STSF=?
     *     [结果]: <CR><LF>^STSF: (0-2), (160060c0-5ffffff7), (1-255) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 配置STK功能。
     *             0：使STK功能无效；
     *             1：激活STK功能；
     *             2：配置STK功能（暂不支持）。
     *     <Config>: 16进制字符串类型，手机能力信息，请参见“ETSI TS 102 223”的“5 Profile download”。
     *     <Timeout>: 参数包含了令用户响应主动命令的时间（暂不支持）。
     * [示例]:
     *     · 设置使STK功能无效
     *       AT^STSF=0
     *       OK
     *     · 语法错误
     *       AT^STSF=3
     *       ＋CME ERROR: 50
     *     · 激活STK功能
     *       AT^STSF=1
     *       OK
     */
    { AT_CMD_STSF,
      At_SetStsfPara, AT_SET_PARA_TIME, At_QryStsfPara, AT_QRY_PARA_TIME, AT_TestStsfPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^STSF", (TAF_UINT8 *)"(0-2),(cmd),(1-255)" },

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) || (defined(DMT)) || (defined(__PC_UT__)))
    /*
     * [类别]: 协议AT-(U)SAT业务
     * [含义]: 获取信息命令数据
     * [说明]: 该命令用来获取主动命令数据，TE收到主动命令通知（^STIN）后，用执行命令获取SIM发送的主动命令的信息（显示文本、菜单信息和属性等）。
     *         查询命令返回当前SIM卡上报的主动命令类型和索引。
     *         测试命令返回支持的STK命令。
     *         对于获取主菜单信息，TE可以在任何时刻用AT^STGI=0,0获取主菜单。其他的命令信息必须在有上报的情况下才能获取。
     *         本命令仅支持在MBB平台使用。
     * [语法]:
     *     [命令]: ^STGI=<CmdType>,<CmdIndex>
     *     [结果]: 见STGI参数表中各个命令详细描述。
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^STGI=?
     *     [结果]: [<CR><LF>^STGI: (list of supported <CmdType>)<CR><LF>]
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *       命令类型, 命令格式,
     *       0,        ^STGI: <Id>, <NbItems>, <Alpha Identifier menu><CR><LF>^STGI: <Id1>, <NbItems>, <Alpha Id1 Label>, <Help Info> [,<NextActionId>] <CR><LF>^STGI: <Id2>, <Nbltems>, <Alpha Id2 Label>, <Help Info> [,<NextActionId>]<CR><LF>[…],
     *       1,        ^STGI:<TextInfo>, <TextFormat>, <ClearMode>[,<DurationTime>],
     *       2,        ^STGI: <TextInfo>, <textCode>, <rspFormat>, <HelpInfo> [,<Timeout> [,Icon]],
     *       3,        ^STGI: <TextInfo>,<textCode>,<rspFormat>, <PackMode>, <EchoMode>, <SizeMin>, <SizeMax>, <HelpInfo> [,<DefaultTextInfo>,<textCode>],
     *       5,        ^STGI:<ToneType>[,<Duration>,<TextInfo>,<textCode>,<icon>],
     *       6,        ^STGI: <DefaultItem>, <NbItems>, <Alpha Identifier menu><CR><LF>
^STGI: <Id1>, <NbItems>, <Alpha Id1 Label>, <Help Info> [,<NextActionId>]<CR><LF>
^STGI: <Id2>, <NbItems>, <Alpha Id2 Label>, <Help Info> [,<NextActionId>]<CR><LF>
[…],
     *       7,        ^STGI:<Refresh Type>[,<File List>],
     *       11,       ^STGI:<URL>,<LaunchMode>,
     *       12,       ^STGI:<TextInfo>,<textCode>[,<icon>],
     * [示例]:
     *       在所有功能都激活的一开始，不要求PIN校验，并且STK功能被激活。
     *       ^STIN:0,2                              SIM已经发出主菜单
     *     · 获得关于主菜单的信息
     *       AT^STGI=0,2
     *       ^STGI: "SIM TOOKIT MAIN MENU"          主菜单包括三项
     *       ^STGI: 1,3,"BANK", 0
     *       ^STGI: 2,3,"QUIZ", 0
     *       ^STGI: 3,3,"WEATHER",0
     *       OK
     */
    { AT_CMD_STGI,
      At_SetStgiPara, AT_SET_PARA_TIME, At_QryStgiPara, AT_QRY_PARA_TIME, AT_TestStgiPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^STGI", (TAF_UINT8 *)"(0-12)" },

    /*
     * [类别]: 协议AT-(U)SAT业务
     * [含义]: STK给予响应
     * [说明]: 该命令用来把TE执行主动命令的结果告诉SIM卡。
     *         查询命令返回当前正在等待后台响应的主动命令类型和索引。
     *         测试命令返回当前支持的主动命令类型和索引。
     *         本命令仅支持在MBB平台使用。
     * [语法]:
     *     [命令]: ^STGR=<CmdType>,<CmdIndex>[,<Result>[,<Data>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^STGR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <CmdType>: 请参见2.9.1 (U)SAT类型命令参数说明。
     *     <CmdIndex>: 请参见2.9.1 (U)SAT类型命令参数说明。
     *     <Result>: 0：用户终止会话；
     *             1：执行用户功能动作；
     *             2：用户要求的帮助信息；
     *             3：返回上一层菜单；
     *             4：表示ME不支持；
     *             5：表示ME上的浏览器忙或不可见；
     *             6：表示ME正忙于SS传输。
     *     <Data>: 如果<Result>是“1：执行用户功能动作”则存在，内容请参见表2-17。
     * [表]: Data字段内容
     *       命令类型, Data内容,
     *       0,        选中菜单的<Id>号。,
     *       1,        无。,
     *       2,        <textCode>＋用户输入内容。,
     *       3,        <textCode>＋用户输入内容。,
     *       4,        无。,
     *       5,        无。,
     *       6,        选中菜单的<Id>号。,
     *       7,        无。,
     *       8,        无。,
     *       9,        无。,
     *       10,       无。,
     *       11,       无。,
     *       12,       无。,
     * [示例]:
     *       在所有功能都激活的一开始，不要求PIN校验，并且STK功能被激活。
     *       ^STIN: 0,3                             SIM已经发出主菜单
     *     · 获得关于主菜单的信息
     *       AT^STGI=0,4
     *       ^STGI: 1,3,"BANK", 0                  主菜单包括三项
     *       ^STGI: 2,3,"QUIZ", 0
     *       ^STGI: 3,3,"WEATHER",0
     *       OK
     *     · 主菜单中的item2被选择
     *       AT^STGR=0,4,1,2
     *       OK
     *       ^STIN: 6,1                             SIM已发出Sel item菜单
     *     · 获取BANK菜单的信息
     *       AT^STGI=6,1
     *       ^STGI:0, "BANK"                        BANK菜单包括2个选项
     *       ^STGI: 1,2, "PERSONAL ACCOUNT ENQUIRY", 1
     *       ^STGI: 2,2, "NEWS", 0
     *       OK
     *     · 选择item2
     *       AT^STGR=6,1,1,2
     *       OK
     */
    { AT_CMD_STGR,
      At_SetStgrPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestStgrPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^STGR", (TAF_UINT8 *)"(0-12),(0-8),(0-6)" },
#endif

    /*
     * [类别]: 协议AT-(U)SAT业务
     * [含义]: 切换SIM卡卡槽命令
     * [说明]: 该命令只在多Modem形态下有效，用于配置Modem与SIM卡卡槽的对应关系。该命令下发后，需要重启Modem才生效。
     *         任意两个Modem不能同时配置为同一卡槽。
     * [语法]:
     *     [命令]: ^SIMSLOT=<Modem0>,<Modem1>[,<Modem2>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SIMSLOT?
     *     [结果]: <CR><LF>^SIMSLOT: < Modem0>,<Modem1> [,<Modem2>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^SIMSLOT=?
     *     [结果]: <CR><LF>^SIMSLOT: (list of supported <Modem0>s),(list of supported <Modem1>s) [,(list of supported <Modem2>s)]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Modem0>: Modem0对应SIM卡槽，整型值，取值范围为0~2。
     *             0：Modem0对应SIM卡槽切换为SIM卡槽0；
     *             1：Modem0对应SIM卡槽切换为SIM卡槽1；
     *             2：Modem0对应SIM卡槽切换为SIM卡槽2。
     *     <Modem1>: Modem1对应SIM卡槽，整型值，取值范围为0~2。
     *             0：Modem1对应SIM卡槽切换为SIM卡槽0；
     *             1：Modem1对应SIM卡槽切换为SIM卡槽1；
     *             2：Modem1对应SIM卡槽切换为SIM卡槽2。
     *     <Modem2>: Modem2对应SIM卡槽，整型值，取值范围为0~2。双Modem形态时，此参数省略。
     *             0：Modem2对应SIM卡槽切换为SIM卡槽0；
     *             1：Modem2对应SIM卡槽切换为SIM卡槽1；
     *             2：Modem2对应SIM卡槽切换为SIM卡槽2。
     * [示例]:
     *     · 双Modem形态，设置Modem0使用SIM卡槽1，Modem1使用SIM卡槽0
     *       AT^SIMSLOT=1,0
     *       OK
     *     · 三Modem形态，设置Modem0使用SIM卡槽1，Modem1使用SIM卡槽0，Modem2使用SIM卡槽2
     *       AT^SIMSLOT=1,0,2
     *       OK
     *     · 双Modem形态，查询SIM卡槽配置，Modem0使用SIM卡槽0，Modem1使用SIM卡槽1
     *       AT^SIMSLOT?
     *       ^SIMSLOT: 0,1
     *       OK
     *     · 三Modem形态，查询SIM卡槽配置，Modem0使用SIM卡槽2，Modem1使用SIM卡槽0，Modem2使用SIM卡槽1
     *       AT^SIMSLOT?
     *       ^SIMSLOT: 2,0,1
     *       OK
     *     · 双Modem形态，测试命令
     *       AT^SIMSLOT=?
     *       ^SIMSLOT: (0-2),(0-2)
     *       OK
     *     · 三Modem形态，测试命令
     *       AT^SIMSLOT=?
     *       ^SIMSLOT: (0-2),(0-2),(0-2)
     *       OK
     */
    { AT_CMD_SIMSLOT,
      At_SetSIMSlotPara, AT_NOT_SET_TIME, At_QrySIMSlotPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMSLOT", (VOS_UINT8 *)SCICHG_CMD_PARA_STRING },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-(U)SAT业务
     * [含义]: 指示SIM卡移除或插入
     * [说明]: 该命令用于AP指示SIM卡移除或插入，AP检测到SIM卡移除或插入时可以通过该命令通知modem。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^SIMINSERT=<n>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^SIMINSERT=?
     *     [结果]: <CR><LF>^SIMINSERT: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: SIM卡状态，整型值，取值范围为0~1。
     *             0：SIM卡移除
     *             1：SIM卡插入
     * [示例]:
     *     · 指定SIM卡移除
     *       AT^SIMINSERT=0
     *       OK
     *     · 指定SIM卡插入
     *       AT^SIMINSERT=1
     *       OK
     *     · 测试命令
     *       AT^SIMINSERT=?
     *       ^SIMINSERT: (0,1)
     *       OK
     */
    { AT_CMD_SIMINSERT,
      At_SetSimInsertPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMINSERT", (VOS_UINT8 *)"(0,1)" },
#endif
};

/* 注册sat pam AT命令表 */
VOS_UINT32 AT_RegisterSatPamCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atSatPamCmdTbl, sizeof(g_atSatPamCmdTbl) / sizeof(g_atSatPamCmdTbl[0]));
}

