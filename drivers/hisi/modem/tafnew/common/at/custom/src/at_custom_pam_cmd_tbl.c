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
#include "at_custom_pam_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_pam_set_cmd_proc.h"
#include "at_custom_pam_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_PAM_CMD_TBL_C

static const AT_ParCmdElement g_atCustomPamCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 下发STK主动命令执行结果
     * [说明]: AP执行主动上报消息的命令后，调用^CSTR命令返回命令执行结果（不是所有的主动命令都需要AP通过^CSTR命令返回执行结果）。
     * [语法]:
     *     [命令]: ^CSTR=<Len>,<CmdData>
     *     [结果]: <CR><LF>^CSTR: <SW1>,<SW2>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^CSTR=?
     *     [结果]: <CR><LF>^CSTR: (list of supported <Len>s),(cmd) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Len>: 整型值，指示<CmdData>的长度，取值范围2-512。
     *     <CmdDate>: 十六进制格式的信息字段，AP回复的Terminal Response结果。
     *     <SW1>: 下发Terminal Response返回的状态字节1。（具体含义SIM卡参见“3GPP 51.011”的“9.4 Status conditions returned by the card”，USIM卡参见“ETSI TS 102.221”的“10.2.1Status conditions returned by the UICC”）
     *     <SW2>: 下发Terminal Response返回的状态字节2。（具体含义SIM卡参见“3GPP 51.011”的“9.4 Status conditions returned by the card”，USIM卡参见“ETSI TS 102.221”的“10.2.1 Status conditions returned by the UICC”）
     * [示例]:
     *     · 下发命令执行结果后有后续STK命令上报
     *       AT^CSTR=30,"810301240082028281830100100102"
     *       ^CSTR: 145,20
     *       OK
     *     · 下发命令执行结果后无STK命令上报
     *       AT^CSTR=30,"810301240082028281830100100102"
     *       ^CSTR: 144,0
     *       OK
     */
    { AT_CMD_CSTR,
      At_SetCstrPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CSTR", (TAF_UINT8 *)"(2-512),(cmd)" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 上报和查询STK主动命令
     * [说明]: 该命令用于通知AP，卡上报了一个主动命令给UE。AP收到后解析命令并且根据当前UE状态判断是否执行该命令。执行完毕后调用^CSTR命令返回命令执行结果（不是所有的主动命令都需要AP通过^CSTR命令返回执行结果，具体命令由AP和Modem协商确定）。同时，AP可以通过^CSTR命令获取到SIM卡上报的最后一个主动命令。如果SIM卡没有上报过主动命令，查询时返回主动命令长度为0。
     * [语法]:
     *     [命令]: ^CSIN
     *     [结果]: <CR><LF>^CSIN=<Len>[,<CmdType>,<CmdData>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Len>: 整型值，用于指示<CmdData>的长度。
     *     <CmdType>: 整型值(十进制输出)，指示当前STK命令类型，请参见表2-21。
     *     <CmdData>: 十六进制格式的信息字段，该STK主动命令数据内容。
     * [表]: STK命令类型
     *       CmdType值(十六进制), STK命令类型,
     *       '00',                END SESSION,
     *       '01',                REFRESH,
     *       '02',                MORE TIME,
     *       '03',                POLL INTERVAL,
     *       '04',                POLLING OFF,
     *       '05',                SET UP EVENT LIST,
     *       '10',                SET UP CALL,
     *       '11',                SEND SS,
     *       '12',                SEND USSD,
     *       '13',                SEND SHORT MESSAGE,
     *       '14',                SEND DTMF,
     *       '15',                LAUNCH BROWSER,
     *       '20',                PLAY TONE,
     *       '21',                DISPLAY TEXT,
     *       '22',                GET INKEY,
     *       '23',                GET INPUT,
     *       '24',                SELECT ITEM,
     *       '25',                SET UP MENU,
     *       '26',                PROVIDE LOCAL INFORMATION,
     *       '27',                TIMER MANAGEMENT,
     *       '28',                SET UP IDLE MODEL TEXT,
     *       '30',                PERFORM CARD APDU,
     *       '31',                POWER ON CARD,
     *       '32',                POWER OFF CARD,
     *       '33',                GET READER STATUS,
     *       '34',                RUN AT COMMAND,
     *       '35',                LANGUAGE NOTIFICATION,
     *       '40',                OPEN CHANNEL,
     *       '41',                CLOSE CHANNEL,
     *       '42',                RECEIVE DATA,
     *       '43',                SEND DATA,
     *       '44',                GET CHANNEL STATUS,
     *       '45',                SERVICE SEARCH,
     *       '46',                GET SERVICE INFORMATION,
     *       '47',                DECLARE SERVICE,
     *       '50',                SET FRAMES,
     *       '51',                GET FRAMES STATUS,
     *       '60',                RETRIEVE MULTIMEDIA MESSAGE,
     *       '61',                SUBMIT MULTIMEDIA MESSAGE,
     *       '62',                DISPLAY MULTIMEDIA MESSAGE,
     * [示例]:
     *       SIM卡上报一个Select Item命令，命令数据长度118个字符
     *       ^CSIN: 118,36,"D0398103012400820281828F0A01805F53524D53F778018F0A02805207636253F778018F0A03807F168F9153F778018F0A0480547C53EB8F6C79FB"
     *       此次会话的主动命令结束，后续没有新的主动命令上报
     *       ^CSIN: 0,0
     *     · 查询SIM卡最后一次上报的主动命令，单板返回Refresh命令
     *       AT^CSIN
     *       ^CSIN: 22,1,"D009810301010382028182"
     *       OK
     *     · 查询SIM卡最后一次上报的主动命令，单板返回无主动命令
     *       AT^CSIN
     *       ^CSIN: 0
     *       OK
     */
    { AT_CMD_CSIN,
      At_SetCsinPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CSIN", VOS_NULL_PTR },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 下发STK Envelope
     * [说明]: 该命令是AP把Envelope命令数据下发给MODEM，MODEM收到该AT命令后，用Envelope命令把数据发送给卡，并且返回命令数据发送结果。
     * [语法]:
     *     [命令]: ^CSEN=<Len>,<CmdData>
     *     [结果]: <CR><LF>^CSTR: <SW1>,<SW2><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CSEN=?
     *     [结果]: <CR><LF>^CSEN: (list of supported <Len>s),(cmd) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Len>: 整型值，指示<CmdData>的长度，取值范围2-512。
     *     <CmdDate>: 十六进制格式的信息字段，AP回复的Envelope命令数据。
     *     <SW1>: 下发Terminal Response返回的状态字节1。（具体含义SIM卡参见“3GPP 51.011”的“9.4 Status conditions returned by the card”，USIM卡参见“ETSI TS 102.221”的“10.2.1Status conditions returned by the UICC”）
     *     <SW2>: 下发Terminal Response返回的状态字节2。（具体含义SIM卡参见“3GPP 51.011”的“9.4 Status conditions returned by the card”，USIM卡参见“ETSI TS 102.221”的“10.2.1 Status conditions returned by the UICC”）
     * [示例]:
     *     · 下发命令执行结果后有后续STK命令上报
     *       AT^CSEN=18,"D30782020181100101"
     *       ^CSEN: 145,20
     *       OK
     *     · 下发命令执行结果后无STK命令上报
     *       AT^CSEN=18,"D30782020181100101"
     *       ^CSEN: 144,0
     *       OK
     */
    { AT_CMD_CSEN,
      At_SetCsenPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CSEN", (TAF_UINT8 *)"(2-512),(cmd)" },
    { AT_CMD_CSMN,
      At_SetCsmnPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CSMN", TAF_NULL_PTR },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: Setup Call发起呼叫确认命令
     * [说明]: 在AP-Modem形态下，Modem上报Setup Call的主动命令，用户通过此命令确认是否同意发起呼叫。
     * [语法]:
     *     [命令]: ^CSTC=<Cmd>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CSTC=?
     *     [结果]: <CR><LF>^CSTC: (list of supported <Cmd>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Cmd>: 整型值。
     *             0：拒绝发起Setup Call；
     *             1：允许发起Setup Call。
     * [示例]:
     *     · AP同意发起呼叫
     *       AT^CSTC=1
     *       OK
     *     · 测试命令
     *       AT^CSTC=?
     *       ^CSTC: (0-1)
     *       OK
     */
    { AT_CMD_CSTC,
      At_SetCstcPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CSTC", (TAF_UINT8 *)"(0-1)" },

    { AT_CMD_CCIN,
      At_SetCcinPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CCIN", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_CARDATR,
      At_SetCardATRPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^CARDATR", VOS_NULL_PTR },

#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
    { AT_CMD_SPWORD,
      AT_SetSpwordPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SPWORD", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: SIM卡、eSIM卡 PASSTHROUGH上下电设置
     * [说明]: 本命令用于给SIM卡或eSIM卡进行PASSTHROUGH模式上下电。执行PASSTHROUGH设置的前提是卡要先下电。
     *         PASSTHROUGH上电卡是指Modem仅仅调用SCI驱动给卡上电，驱动完成PPS协商后就停止其他读卡动作，USIMM也不对卡做初始化动作。下电卡也是仅仅调用SCI驱动对卡做下电。
     *         不管上电下电，Modem协议都始终处于无卡状态，且不向其他模块广播卡状态。
     *         重启Modem或发AT^HVSST=1,1指令会退出PASSTHROUGH模式。
     * [语法]:
     *     [命令]: ^PASSTHROUGH=<state>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: 13<CR><LF>
     *             或
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^PASSTHROUGH?
     *     [结果]: <CR><LF>^PASSTHROUGH: <state><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^PASSTHROUGH=?
     *     [结果]: <CR><LF>^PASSTHROUGH: (list of supported <state>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值，取值1或者2
     *             1：上电
     *             2：下电
     *             查询时，取值范围0~2
     *             0：非passthrough状态（normal态）
     *             1：上电状态
     *             2：下电状态
     * [示例]:
     *     · PASSTHROUGH上电
     *       AT^PASSTHROUGH=1
     *       OK
     *     · 查询PASSTHROUGH状态
     *       AT^PASSTHROUGH?
     *       ^PASSTHROUGH: 1
     *       OK
     *     · PASSTHROUGH下电
     *       AT^PASSTHROUGH=2
     *       OK
     *     · 查询PASSTHROUGH状态
     *       AT^PASSTHROUGH?
     *       ^PASSTHROUGH: 2
     *       OK
     */
    { AT_CMD_PASSTHROUGH,
      At_SetPassThrough, AT_SET_PARA_TIME, At_QryPassThrough, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PASSTHROUGH", (VOS_UINT8 *)"(1,2)" },

#if ((FEATURE_SINGLEMODEM_DUALSLOT == FEATURE_ON) && (MULTI_MODEM_NUMBER == 1))
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 单modem支持双卡槽，USIM任务与卡槽配置关系配置
     * [说明]: 本命令用于单modem支持双卡槽场景，设置USIM任务与卡槽配置关系。
     *         1、设置USIM与卡槽对应关系AT命令需按照如下顺序使用：
     *         卡下电(HVSST=1,0)->下发AT命令(设置卡槽与USIM任务对应关系)->卡上电(HVSST=1,1)；
     *         2、SIM卡热插拔和上述2操作不能同时进行；
     *         3、卡槽切换间隔不能小于10s；
     *         4、请确保卡插入槽位和命令配置槽位一致；
     * [语法]:
     *     [命令]: ^SINGLEMODEMDUALSLOT=<slot>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: 13<CR><LF>
     *             或
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^SINGLEMODEMDUALSLOT?
     *     [结果]: <CR><LF>^SINGLEMODEMDUALSLOT: <slot><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^SINGLEMODEMDUALSLOT=?
     *     [结果]: <CR><LF>^SINGLEMODEMDUALSLOT: (0,1)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <slot>: 整型值，取值0或者1
     *             0：USIM任务对应卡槽0
     *             1：USIM任务对应卡槽1
     *             查询时，取值范围0或1
     *             USIM与卡槽配置关系
     * [示例]:
     *     · 设置USIM任务匹配卡槽1
     *       AT^SINGLEMODEMDUALSLOT=1
     *       OK
     *     · 查询USIM任务匹配的卡槽
     *       AT^SINGLEMODEMDUALSLOT?
     *       ^SINGLEMODEMDUALSLOT: 1
     *       OK
     */
    { AT_CMD_SINGLEMODEMDUALSLOT,
      At_SetSingleModemDualSlot, AT_SET_PARA_TIME, At_QrySingleModemDualSlot, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SINGLEMODEMDUALSLOT", (VOS_UINT8 *)"(0,1)" },
#endif

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 切换SIM卡卡槽命令
     * [说明]: 该命令只在多Modem形态下有效，用于配置Modem与SIM卡卡槽的对应关系。
     *         该命令下发前，需要把会发生配置改变的卡下电，修改完成后，再上电生效新的配置。
     *         任意两个Modem不能同时配置为同一卡槽。
     * [语法]:
     *     [命令]: ^SCICHG=<Modem0>,<Modem1>[,<Modem2>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SCICHG?
     *     [结果]: <CR><LF>^SCICHG: < Modem0>,<Modem1> [,<Modem2>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^SCICHG=?
     *     [结果]: <CR><LF>^SCICHG: (list of supported <Modem0>s),(list of supported <Modem1>s) [,(list of supported <Modem2>s)]<CR><LF>
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
     *       AT^SCICHG=1,0
     *       OK
     *     · 三Modem形态，设置Modem0使用SIM卡槽1，Modem1使用SIM卡槽0，Modem2使用SIM卡槽2
     *       AT^SCICHG=1,0,2
     *       OK
     *     · 双Modem形态，查询SIM卡槽配置，Modem0使用SIM卡槽0，Modem1使用SIM卡槽1
     *       AT^SCICHG?
     *       ^SCICHG: 0,1
     *       OK
     *     · 三Modem形态，查询SIM卡槽配置，Modem0使用SIM卡槽2，Modem1使用SIM卡槽0，Modem2使用SIM卡槽1
     *       AT^SCICHG?
     *       ^SCICHG: 2,0,1
     *       OK
     *     · 双Modem形态，测试命令
     *       AT^SCICHG=?
     *       ^SCICHG: (0-2),(0-2)
     *       OK
     *     · 三Modem形态，测试命令
     *       AT^SCICHG=?
     *       ^SCICHG: (0-2),(0-2),(0-2)
     *       OK
     */
    { AT_CMD_SCICHG,
      At_SetSciChgPara, AT_SET_PARA_TIME, At_QrySciChgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SCICHG", (VOS_UINT8 *)SCICHG_CMD_PARA_STRING },

#if (FEATURE_VSIM == FEATURE_ON)
#if (FEATURE_VSIM_ICC_SEC_CHANNEL == FEATURE_ON)
    { AT_CMD_ICCVSIMVER,
      VOS_NULL_PTR, AT_SET_PARA_TIME, At_QryIccVsimVer, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ICCVSIMVER", VOS_NULL_PTR },

    { AT_CMD_HVCHECKCARD,
      VOS_NULL_PTR, AT_SET_PARA_TIME, At_QryHvCheckCardPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HVCHECKCARD", VOS_NULL_PTR },

#endif
#endif /* end (FEATURE_VSIM == FEATURE_ON) */

#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))
    { AT_CMD_SILENTPIN,
      At_SetSilentPin, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SILENTPIN", (VOS_UINT8 *)"(@encryptpin),(@iv),(@hmac)" },

    { AT_CMD_SILENTPININFO,
      At_SetSilentPinInfo, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SILENTPININFO", (VOS_UINT8 *)"(@pin)" },
#endif

    { AT_CMD_PRIVATECGLA,
      At_SetPrivateCglaPara, AT_SET_CGLA_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^CGLA", (TAF_UINT8 *)"(0-4294967295),(8-522),(command)" },

    { AT_CMD_CARDVOLTAGE,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryCardVoltagePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CARDVOLTAGE", VOS_NULL_PTR },

    { AT_CMD_IMSICHG,
      At_SetImsichgPara, AT_SET_PARA_TIME, At_QryImsichgPara, AT_QRY_PARA_TIME, AT_TestImsichgPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (TAF_UINT8 *)"^IMSICHG", (TAF_UINT8 *)"(0-1),(0-1)" },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { AT_CMD_MEMQUERY,
      AT_SetMemInfoPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (TAF_UINT8 *)"^MEMQUERY", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_APSIMST,
      AT_SetApSimStPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestApSimStPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^APSIMST", (VOS_UINT8 *)"(0,1)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: Modem单独复位状态通知
     * [说明]: 该AT命令用于AP向Balong Modem通知其他拼片Modem发生了复位。
     * [语法]:
     *     [命令]: ^MODEMSTATUS=<modem>,<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^MODEMSTATUS?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^MODEMSTATUS=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <modem>: 整数型，当前复位的Modem编号。
     *             0：Balong Modem
     *             1：VIA Modem
     *             其他保留扩展
     *     <mode>: 整数型，当前的复位状态。
     *             0：开始复位
     *             1：复位成功结束
     *             2：复位失败
     * [示例]:
     *     · VIA开始复位通知
     *       AT^MODEMSTATUS=1,0
     *       OK
     */
    { AT_CMD_MODEMSTATUS,
      AT_SetModemStatusPara, AT_NOT_SET_TIME, AT_QryModemStatusPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^MODEMSTATUS", (VOS_UINT8 *)"(0,1),(0-2)" },

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (MULTI_MODEM_NUMBER == 1))
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 设置应用的AID
     * [说明]: 本命令用于配置UICC卡中USIM/ISIM/CSIM应用的AID，即修改NV4085。
     *         【注】本命令仅适用于针对产品线提出的软银USIM应用AID定制需求，提供给产品线以便修改USIM/ISIM/CSIM。
     * [语法]:
     *     [命令]: ^CARDAPPAIDSET=<enable>,<appType>,<dfNameLength>,<dfName>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <enable>: NV4085是否使能。0：默认值，不使能。1：使能。
     *     <appType>: 需要配置的应用的类型。取值范围是0-2.
     *             0：USIM
     *             1：ISIM
     *             2：CSIM
     *     <dfNameLength>: 整型值，配置应用名称的长度。不能超过16。
     *     <dfName>: 整型值，需要配置的应用AID值，长度1~16字节。
     * [示例]:
     *       AT^CARDAPPAIDSET=1,0,16,"A0000000871002FF86FF0389FFFFFFFF"
     *       OK
     */
    { AT_CMD_CARDAPPAIDSET,
      AT_SetCardAppAidPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CARDAPPAIDSET", (VOS_UINT8 *)"(0,1),(0-2),(1-16),(dfname)" },
#endif
};

/* 注册PAM定制AT命令表 */
VOS_UINT32 AT_RegisterCustomPamCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomPamCmdTbl, sizeof(g_atCustomPamCmdTbl) / sizeof(g_atCustomPamCmdTbl[0]));
}

