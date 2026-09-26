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
#include "at_sms_taf_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_sms_taf_set_cmd_proc.h"
#include "at_sms_taf_qry_cmd_proc.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SMS_TAF_CMD_TBL_C

/* SMS命令表 */

/* 主动上报相关命令设置和查询时需要等待回复，添加等待设置回复时间和等待查询回复时间 */
/*
 * [类别]: 协议AT-短信业务
 * [含义]: 短信操作命令参数说明
 * [说明]: 短信相关参数请参考3GPP TS 23.038、3GPP TS 23.040和3GPP TS 24.011等协议。
 * [表]：短信存储相关参数说明
 *       参数,     说明
 *       <index>,  整型值，短信在存储器中的索引值。
 *                 存储器为Flash时，取值范围取决于ME的容量（ME容量由NV项定制，最大支持500条），从0开始取值；
 *                 存储器为(U)SIM时，取值范围取决于(U)SIM的容量，从0开始取值。,
 *       <mem>,    字符串类型，短信操作存储器。
 *                 SM：SIM卡存储；
 *                 ME：Flash存储。,
 *       <mem1>,   字符串类型，短信读取和删除存储器，取值范围同<mem>。,
 *       <mem2>,   字符串类型，短信发送和写存储器，取值范围同<mem>。,
 *       <mem3>,   字符串类型，短信接收存储器，取值范围同<mem>。,
 *       <total>,  整型值，当前存储器中的短信容量。,
 *       <used>,   整型值，当前存储器中的短信数目。,
 *       <unread>, 整型值，当前存储器中的未读短信数目。,
 *       <stat>,   PDU模式下是整型值，文本模式下是字符串类型。
 *                 0：“REC UNREAD”：接收到的未读短信；
 *                 1：“REC READ”：接收到的已读短信；
 *                 2：“STO UNSENT”：存储的未发短信；
 *                 3：“STO SENT”：存储的已发短信；
 *                 4：“ALL”：所有短信。,
 * [表]: 短信数据相关参数说明
 *       参数,     说明,
 *       <ackpdu>, 16进制数字符串，确认PDU。,
 *       <alpha>,  字符串，号码在电话本中对应的姓名，目前不处理此参数。,
 *       <cdata>,  16进制数字符串，短信命令内容。,
 *       <ct>,     整型值，取值范围为0～255，短信命令类型。,
 *       <da>,     字符串类型，发送短信的目的地址。,
 *       <data>,   16进制数字符串，短信内容。,
 *       <dcs>,    整型值，取值范围为0～255，短信内容的编码方案。
 *                 0：GSM 7bit；
 *                 4：8bit；
 *                 8：UNICODE。
 *                 该参数还可以设置消息等级等参数。具体内容请参见“3GPP TS 23.038”的“4 SMS Data Coding Scheme”。,
 *       <dt>,     时间字符串类型，短信中心发出短信的时间。
 *       <fo>,     整型值，取值范围为0～255，TPDU的第1个字节。具体内容请参见“3GPP TS 23.040”的“9.2 Service provided by the SM?TL”。,
 *       <length>, 整型值，长度指示。,
 *       <mn>,     整型值，取值范围为0～255，短信命令中所要操作的短信的编号。,
 *       <mr>,     整型值，取值范围为0～255，短信编号。,
 *       <oa>,     字符串类型，源地址。,
 *       <pdu>,    16进制数字符串，PDU内容。,
 *       <pid>,    整型值，取值范围为0～255，协议标识。,
 *       <ra>,     字符串类型，目的地址。,
 *       <sca>,    字符串类型，短信中心地址。号码由“*”、“#”、“+”、“a”、“b”、“c”、“0”～“9”组成，号码长度不超过20（不包含“+”号）个字符。,
 *       <scts>,   时间字符串类型，时间戳，“yy/mm/dd,hh:mm:ss±zz”。
 *                 如“94/05/06,22:10:00+08”是1994年5月6日，东8区，22时10分0秒。,
 *       <st>,     整型值，取值范围为0～255，短信中心把短信发给MT的执行结果。,
 *       <toda>,   整型值，取值范围为0～255，目的地址类型。,
 *       <tooa>,   整型值，取值范围为0～255，源地址类型。,
 *       <tora>,   整型值，取值范围为0～255，前一次短信的目的地址类型。,
 *       <tosca>,  整型值，取值范围为0～255，短信中心地址类型。,
 *       <vp>,     短信有效期，格式由<fo>中有效期格式决定。
 *                 如果是相对时间格式，则是整型值，取值范围为0～255。
 *                 0～143：（VP+1）*5分钟；
 *                 144～167：12小时+（（VP–143）*30分钟）；
 *                 168～196：（VP–166）*1日；
 *                 197～255：（VP–192）*1周。
 *                 如果是绝对时间格式，则是时间字符串类型。具体内容请参见“3GPP TS 23.040”的“9.2.3.11 TP-Service-Centre-Time-Stamp (TP-SCTS)”。,
 */
const AT_SMS_CmdTab g_atSmsCmdTab[] = {
    /* SMS */
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 发送短信
     * [说明]: 发送一条短信。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: iftextmode(+CMGF=1):
     *             +CMGS=<da>[,<toda>]<CR>
     *             textisentered<ctrl-Z/ESC>
     *     [结果]: if text mode (+CMGF=1) and sending successful:
     *             <CR><LF>+CMGS: <mr>[,<scts>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if sending fails:
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: ifPDUmode(+CMGF=0):
     *             +CMGS=<length><CR>
     *             PDUisgiven<ctrl-Z/ESC>
     *     [结果]: if PDU mode (+CMGF=0) and sending successful:
     *             <CR><LF>+CMGS: <mr>[,<ackpdu>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if sending fails:
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMGS=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <CR>: MT在命令行结束符<CR>返回4个字符<CR><LF><greater_than><space>（ASCII码值13，10，62，32）。
     *     <ctrl-Z>: 字符“0x1A”，表示内容输入结束并发送短信。
     *     <ESC>: 字符“0x1B”，表示取消本次发送短信操作。
     *     <其他>: 请参见2.5.1 短信操作命令参数说明。
     * [示例]:
     *     · 文本模式发短信
     *       AT+CMGS="13902100077"
     *       >This the first line<CR>
     *       >This is the last line<ctrl-Z>
     *       +CMGS: 252
     *       OK
     *     · +CSCS设置TE和MT之间的字符集为GSM7BIT编码，短信内容通过二进制码流输出，例如：文本@123
     *       AT+CMGS="13902100096"
     *       >00313233<ctrl-Z>
     *       +CMGS: 0
     *       OK
     *     · PDU模式发短信
     *       AT+CMGS=18
     *       >0015660B813109120090F60008FF044F60597D<ctrl-Z>
     *       +CMGS: 229
     *       OK
     *     · 测试CMGS
     *       AT+CMGS=?
     *       OK
     */
    { AT_CMD_CMGS,
      At_SetCmgsPara, AT_SMS_CMGS_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
#if (FEATURE_MBB_CUST == FEATURE_ON)
      AT_AbortCmgsPara, AT_NOT_SET_TIME,
#else
      VOS_NULL_PTR, AT_NOT_SET_TIME,
#endif
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMGS", (VOS_UINT8 *)"(da),(0-255)", (TAF_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 存储短信
     * [说明]: 写一条短信并存储（mem2）。如果输入<stat>，则短信设置为指定的状态，默认情况下，短信状态设置为“存储未发送”状态。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: iftextmode(+CMGF=1):
     *             +CMGW[=<oa/da>[,<tooa/toda>[,<stat>]]]<CR>
     *             textisentered<ctrl-Z/ESC>
     *             ifPDUmode(+CMGF=0):
     *             +CMGW=<length>[,<stat>]<CR>
     *             PDUisgiven<ctrl-Z/ESC>
     *     [结果]: <CR><LF>+CMGW: <index><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMGW=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <CR>: MT在命令行结束符<CR>返回4个字符<CR><LF><greater_than><space>（ASCII码值13，10，62，32）。
     *     <ctrl-Z>: 字符“0x1A”，表示内容输入结束并储存短信。
     *     <ESC>: 字符“0x1B”，表示取消本次写短信操作。
     *     <其他>: 请参见2.5.1 短信操作命令参数说明。
     * [示例]:
     *     · 文本模式写短信
     *       AT+CMGW="13902100096"
     *       >1651<ctrl-Z>
     *       +CMGW: 0
     *       OK
     *     · +CSCS设置TE和MT之间的字符集为GSM7BIT编码，短信内容通过二进制码流输出，例如：文本@123
     *       AT+CMGW="13902100096"
     *       >00313233<ctrl-Z>
     *       +CMGW: 0
     *       OK
     *     · PDU模式写短信
     *       AT+CMGW=18
     *       >0015660B813109120090F60004FF0461626364<ctrl-Z>
     *       +CMGW: 3
     *       OK
     *     · 测试CMGW
     *       AT+CMGW=?
     *       OK
     */
    { AT_CMD_CMGW,
      At_SetCmgwPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMGW", (VOS_UINT8 *)"(oa),(0-255),(\"REC UNREAD\",\"REC READ\",\"STO UNSENT\",\"STO SENT\",\"ALL\")", (TAF_UINT8 *)"(0-255),(0-4)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 按照索引读取短信
     * [说明]: 按照索引从存储器（mem1）中读一条短信，如果消息状态为“接收未读”，则变为“接收已读”；否则，状态不变。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CMGR=<index>
     *     [结果]: if text mode (+CMGF=1), command successful and SMS-DELIVER:
     *             <CR><LF>+CMGR: <stat>,<oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if text mode (+CMGF=1), command successful and SMS-SUBMIT:
     *             <CR><LF>+CMGR: <stat>,<da>,[<alpha>][,<toda>,<fo>,<pid>,<dcs>,[<vp>],<sca>,<tosca>,<length>]<CR><LF><data><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if text mode (+CMGF=1), command successful and SMS-STATUS-REPORT:
     *             <CR><LF>+CMGR: <stat>,<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if text mode (+CMGF=1), command successful and SMS-COMMAND:
     *             <CR><LF>+CMGR: <stat>,<fo>,<ct>[,<pid>,[<mn>],[<da>],[<toda>],<length><CR><LF><cdata>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if PDU mode (+CMGF=0) and command successful:
     *             <CR><LF>+CMGR: <stat>,[<alpha>],<length><CR><LF><pdu><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             otherwise:
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMGR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     * [示例]:
     *     · 文本模式按照索引读第一条短信
     *       AT+CMGR=0
     *       +CMGR: "STO SENT","13902100096",,129,21,0,8,255,"+8613800773501",145,4
     *       4F60597D "你好"的UNICODE码
     *       OK
     *     · +CSCS设置TE和MT之间的字符集为GSM7BIT编码，短信内容通过二进制码流输出，例如：文本@123
     *       AT+CMGR=0
     *       +CMGR: "STO SENT","13902100096",,129,21,0,8,255,"+8613800773501",145,4
     *       00313233
     *       OK
     *     · PDU模式按照索引读第一条短信
     *       AT+CMGR=0
     *       +CMGR: 1,,24
     *       0891683108703705F1040D91683109120090F70008603070
     *       OK
     *     · 测试CMGR
     *       AT+CMGR=?
     *       OK
     */
    { AT_CMD_CMGR,
      At_SetCmgrPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMGR", (VOS_UINT8 *)"(0-65535)", (TAF_UINT8 *)"(0-65535)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 按照状态读取短信
     * [说明]: 按照状态从存储器（mem1）中读所有短信，如果消息状态为“接收未读”，默认会修改为“接收已读”，受私有命令^CMSR控制。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CMGL[=<stat>]
     *     [结果]: if text mode (+CMGF=1), command successful and SMS-SUBMITs and/or SMS-DELIVERs:
     *             <CR><LF>+CMGL: <index>,<stat>,<oa/da>,[<alpha>],[<scts>][,<tooa/toda>,<length>]<CR><LF><data>
     *             [<CR><LF>+CMGL: <index>,<stat>,<da/oa>,[<alpha>],[<scts>][,<tooa/toda>,<length>]<CR><LF><data>
     *             [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if text mode (+CMGF=1), command successful and SMS-STATUS-REPORTs:
     *             <CR><LF>+CMGL: <index>,<stat>,<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>
     *             [<CR><LF>
     *             +CMGL: <index>,<stat>,<fo>,<mr>,[<ra>],[<tora>],<scts>,<dt>,<st>
     *             [...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if text mode (+CMGF=1), command successful and SMS-COMMANDs:
     *             <CR><LF>+CMGL: <index>,<stat>,<fo>,<ct>
     *             [<CR><LF>+CMGL: <index>,<stat>,<fo>,<ct>[...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if PDU mode (+CMGF=0) and command successful:
     *             <CR><LF>+CMGL: <index>,<stat>,[<alpha>],<length><CR><LF><pdu>
     *             [<CR><LF>+CMGL: <index>,<stat>,[<alpha>],<length><CR><LF><pdu>[...]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             otherwise:
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMGL=?
     *     [结果]: <CR><LF>+CMGL: (list of supported <stat>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     * [示例]:
     *     · 文本模式下按照状态读所有短信
     *       AT+CMGL="ALL"
     *       +CMGL: 0,"REC READ","+8613902100097",,"06/03/06,16:34:24+00",
     *       4F60597D "你好"的UNICODE码
     *       +CMGL: 1,"REC READ”,"+8613902100064",,"06/03/06,19:35:29+00",
     *       Happy New Year
     *       OK
     *     · +CSCS设置TE和MT之间的字符集为GSM7BIT编码，短信内容通过二进制码流输出，例如：文本@123
     *       AT+CMGL="ALL"
     *       +CMGL: 0,"REC READ","+8613902100097",,"06/03/06,16:34:24+00",
     *       00313233
     *       OK
     *     · PDU模式列表所有短信
     *       AT+CMGL=4
     *       +CMGL: 0,1,,18
     *       0891683108703705F115660B813109120090
     *       +CMGL: 1,2,,24
     *       0891683108703705F1040D91683109120090F60008603070
     *       OK
     *     · 测试CMGL（文本模式下）
     *       AT+CMGL=?
     *       +CMGL:("REC UNREAD","REC READ","STO UNSENT","STO SENT","ALL")
     *       OK
     */
    { AT_CMD_CMGL,
      At_SetCmglPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMGL", (VOS_UINT8 *)"(\"REC UNREAD\",\"REC READ\",\"STO UNSENT\",\"STO SENT\",\"ALL\")", (TAF_UINT8 *)"(0-4)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 发送短信命令
     * [说明]: 发送一条短信命令。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: iftextmode(+CMGF=1):
     *             +CMGC=<fo>,<ct>[,<pid>[,<mn>[,<da>[,<toda>]]]]<CR>
     *             textisentered<ctrl-Z/ESC>
     *     [结果]: if text mode (+CMGF=1) and sending successful:
     *             <CR><LF>+CMGC: <mr>[,<scts>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if sending fails:<CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: ifPDUmode(+CMGF=0):
     *             +CMGC=<length><CR>
     *             PDUisgiven<ctrl-Z/ESC>
     *     [结果]: if PDU mode (+CMGF=0) and sending successful:
     *             <CR><LF>+CMGC: <mr>[,<ackpdu>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if sending fails:
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMGC=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <CR>: MT在命令行结束符<CR>返回4个字符<CR><LF><greater_than><space>（ASCII码值13，10，62，32）。
     *     <ctrl-Z>: 字符“0x1A”，表示内容输入结束并发送短信命令。
     *     <ESC>: 字符“0x1B”，表示取消本次发送短信命令操作。
     *     <其他>: 请参见2.5.1 短信操作命令参数说明。
     * [示例]:
     *     · 文本模式下发送短信命令
     *       AT+CMGC=0,0,0,2,"13902100096"
     *       ><ctrl-Z>
     *       +CMGC: 0
     *       OK
     *     · 发送PDU短信命令
     *       AT+CMGC=14
     *       >0002A90001A70B813109210090F600<ctrl-Z>
     *     · 测试CMGC
     *       AT+CMGC=?
     *       OK
     */
    { AT_CMD_CMGC,
      At_SetCmgcPara, AT_SMS_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMGC", (VOS_UINT8 *)"(0-255),(0-255),(0-255),(0-255),(da),(0-255)", (TAF_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 从存储器中发送短信
     * [说明]: 从存储器（mem2）中发送一条短信，如果输入目的地址<da>，则在短信发送时该地址会替换存储器中的短信中所包含的目的地址。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: +CMSS=<index>[,<da>[,<toda>]]
     *     [结果]: if text mode (+CMGF=1) and sending successful:
     *             <CR><LF>+CMSS: <mr>[,<scts>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if PDU mode (+CMGF=0) and sending successful:
     *             <CR><LF>+CMSS: <mr>[,<ackpdu>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if sending fails:
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CMSS=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     * [示例]:
     *     · 文本模式从存储器中发短信
     *       AT+CMSS=0
     *       +CMSS: 239
     *       OK
     *     · PDU模式从存储器中发短信
     *       AT+CMSS=3
     *       +CMSS: 240
     *       OK
     *     · 测试CMSS
     *       AT+CMSS=?
     *       OK
     */
    { AT_CMD_CMSS,
      At_SetCmssPara, AT_SMS_CMSS_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CMSS", (VOS_UINT8 *)"(0-255),(da),(0-255)", (TAF_UINT8 *)"(0-255),(da),(0-255)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 新短信确认
     * [说明]: 新短信（包括接收的短信和短信状态报告）确认命令，通过执行该命令来确认是否收到一条直接发送给TE的新短信。在上一条短信得到确认前，MT不会向TE上报另外一条短信。如果规定的时间内没有得到确认（网络超时），则MT会回复相应错误原因给网络。
     *         本命令遵从3GPP TS 27.005协议。
     * [语法]:
     *     [命令]: iftextmode(+CMGF=1):
     *             +CNMA
     *             ifPDUmode(+CMGF=0):
     *             +CNMA[=<n>[,<length>[<CR>
     *             PDUisgiven<ctrl-Z/ESC>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: +CNMA=?
     *     [结果]: if PDU mode (+CMGF=0):
     *             <CR><LF>+CNMA: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             if text mode (+CMGF=1):
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值。PDU模式下有效。
     *             0: 相当于文本模式下发送AT+CNMA短信确认命令
     *             1: 发送RP-ACK
     *             2: 发送RP-ERROR
     *     <length>: 整型值。
     *             TPDU字节数。
     * [示例]:
     *     · 发送文本短信确认命令
     *       AT+CNMA
     *       OK
     *     · 发送PDU短信确认命令
     *       AT+CNMA=2,7
     *       >0116410300D000<ctrl-Z>
     *     · 文本模式测试CNMA命令
     *       AT+CNMA=?
     *       OK
     *     · PDU模式测试CNMA命令
     *       AT+CNMA=?
     *       +CNMA: (0-2)
     *       OK
     */
    { AT_CMD_CNMA,
      At_SetCnmaPara, AT_SMS_SET_PARA_TIME, AT_TestCnmaPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CNMA", TAF_NULL_PTR, (TAF_UINT8 *)"(0-2),(0-65535)" },
};

const TAF_UINT16 g_atSmsCmdNum = sizeof(g_atSmsCmdTab) / sizeof(AT_SMS_CmdTab);

static const AT_ParCmdElement g_atSmsTafCmdTbl[] = {
    /* SMS */
    { AT_CMD_CMST,
      At_SetCmstPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (TAF_UINT8 *)"^CMST", (TAF_UINT8 *)"(0-255),(da),(0-255),(1-255)" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 设置读取短信标记
     * [说明]: SET命令在读取未读短信（普通短信，不包含短信报告）后，是否需要改变将未读标记改成已读的设置。
     *         如果设置^CMSR=1，使用（+CMGR,+CMGL）读取未读短信，读取后该短信未读标记不改变。
     *         如果设置^CMSR=0（默认），使用（+CMGR,+CMGL）读取未读短信，读取后将未读标记改成已读标记。
     *         READ命令返回当前该命令的设置值。
     *         TEST命令返回该命令参数<ReadTag>的取值范围。
     * [语法]:
     *     [命令]: ^CMSR=<ReadTag>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: ^CMSR?
     *     [结果]: <CR><LF>^CMSR: <ReadTag><CR><LF>
     *     [命令]: ^CMSR=?
     *     [结果]: <CR><LF>^CMSR: (list of support <readTag>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <readTag>: 整型值。
     *             0：读取未读短信后改变未读标记的设置（默认）；
     *             1：读取未读短信后不改变未读标记的设置。
     * [示例]:
     *     · 读取未读短信后改变未读标记
     *       AT^CMSR=0
     *       OK
     *     · 读取未读短信后不改变未读标记
     *       AT^CMSR=1
     *       OK
     */
    { AT_CMD_CMSR,
      At_SetCmsrPara, AT_NOT_SET_TIME, At_QryCmsrPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CMSR", (VOS_UINT8 *)"(0,1)" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 获取短信类型的索引
     * [说明]: 从<meml>中返回状态值由<Stat>参数指定的所有短信的索引值。
     * [语法]:
     *     [命令]: ^CMGI=<stat>
     *     [结果]: <CR><LF>^CMGI: index1,index2,…indexN<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: ^CMGI=?
     *     [结果]: <CR><LF>^CMGI: (list of supported <stat>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <stat>: 整型值，短信状态。
     *             0：接收到的未读短信；
     *             1：接收到的已读短信；
     *             2：存储的未发短信；
     *             3：存储的已发短信；
     *             4：所有短信。
     * [示例]:
     *     · 获取接收到的未读短信
     *       AT^CMGI=0
     *       OK
     */
    { AT_CMD_CMGI,
      At_SetCmgiPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CMGI", (TAF_UINT8 *)"(0-4)" },
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 修改短信标记
     * [说明]: 从<meml>中，将该位置的短信标记修改为<stat>参数，该命令用于将接收的短信，修改成已读或未读短信。
     * [语法]:
     *     [命令]: ^CMMT=<index>,<stat>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CMS ERROR: <err><CR><LF>
     *     [命令]: ^CMMT=?
     *     [结果]: <CR><LF>^CMMT: (list of supported <index>s),(list of supported <stat>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Index>: 整型值，在存储器中的位置。
     *     <Stat>: 0：接收到的未读短信；
     *             1：接收到的已读短信。
     * [示例]:
     *     · 把第一条短信的状态修改为未读
     *       AT^CMMT=0,0
     *       OK
     */
    { AT_CMD_CMMT,
      At_SetCmmtPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CMMT", (TAF_UINT8 *)"(0-254),(0,1)" },

    { AT_CMD_CMSTUB,
      At_SetCmStubPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CMS_OPERATION_NOT_ALLOWED, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CMSTUB", (VOS_UINT8 *)"(0-11),(0-6)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: AP指示MODEM是否还有空间存储短信
     * [说明]: AP-MODEM形态下，短信存储在AP，该命令供AP通知MODEM是否还有空间存储短信。
     * [语法]:
     *     [命令]: ^CSASM=<memstat>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CSASM=?
     *     [结果]: <CR><LF>^CSASM: (list of supported <memstat>s) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <memstat>: 整数型，MODEM当前短信存储空间状态：
     *             0：当前短信存储空间已满。
     *             1：当前短信存储空间未满，AP可以接收新短信。
     * [示例]:
     *     · AP通知MODEM短信存储空间已满
     *       AT^CSASM=0
     *       OK
     */
    { AT_CMD_CSASM,
      AT_SetMemStatusPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CSASM", (TAF_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 配置短信优选域
     * [说明]: 该命令主要用于配置短信优选域。
     * [语法]:
     *     [命令]: ^SMSDOMAIN=<domain>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: ^SMSDOMAIN?
     *     [结果]: <CR><LF>^SMSDOMAIN: <domain><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: ^SMSDOMAIN=?
     *     [结果]: <CR><LF>^SMSDOMAIN: (list of supported <domain>s) <CR><LF>
     * [参数]:
     *     <domain>: 整型值，短信优选域，取值范围(0,1)：
     *             0：不使用IMS发送短信；
     *             1：优先使用IMS发送短信。
     * [示例]:
     *     · 设置短信优选域
     *       AT^SMSDOMAIN=1
     *       OK
     *     · 查询短信优选域
     *       AT^SMSDOMAIN?
     *       ^SMSDOMAIN: 1
     *       OK
     *     · 测试命令
     *       AT^SMSDOMAIN=?
     *       ^SMSDOMAIN: (0,1)
     *       OK
     */
    { AT_CMD_SMSDOMAIN,
      AT_SetSmsDomainPara, AT_SET_PARA_TIME, AT_QrySmsDomainPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^SMSDOMAIN", (VOS_UINT8 *)SMSDOMAIN_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: 查询注册至网络的2/3/4G CS/PS短信能力命令
     * [说明]: 该命令用于查询modem当前是否向网络注册支持2/3/4G下CS/PS短信。
     * [语法]:
     *     [命令]: ^SMSNASCAP?
     *     [结果]: 查询成功：
     *             <CR><LF>^SMSNASCAP: <state>
     *             <CR><LF>OK<CR><LF>
     *             查询失败：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <state>: 整型值，注册至网络的2/3/4G CS/PS短信能力。
     *             0：不支持；
     *             1：支持；
     * [示例]:
     *       AT^SMSNASCAP?
     *       ^SMSNASCAP: 1
     *       OK
     */
    { AT_CMD_SMSNASCAP,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QrySmsNasCapPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^SMSNASCAP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-短信业务
     * [含义]: MT短信可信名单控制命令
     * [说明]: ^TRUSTNUM用于设置/查询MT短信可信名单。
     *         设置MT短信可信名单时，参数中索引和短信号码字段（短信号码字段需要加引号）都不为空时，该命令用于将索引指定的号码写入呼叫可信名单；如果只有一个参数，且索引字段不为空，则读取短信可信名单中索引指定的短信号码。
     *         当开启MT短信可信名单时，UE需要过滤短信可信名单，只有存在于短信可信名单中的号码才能被存储和上报。
     *         如果MT短信可信名单已开启，但短信可信名单为空，则所有短信都无法接收。
     *         该功能FEATURE_MBB_CUST宏开启时生效。
     * [语法]:
     *     [命令]: ^TRUSTNUM=<index>[,<number>]
     *     [结果]: 在设置的情况下：
     *             <CR><LF>OK<CR><LF>
     *             在查询的情况下：
     *             [<CR><LF>^TRUSTNUM: <index>,<number><CR><LF>]<CR><LF>OK<CR><LF>
     *             有相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TRUSTNUM?
     *     [结果]: [<CR><LF>^TRUSTNUM: <index>,<number><CR><LF>^TRUSTNUM: <index>,<number>[...]<CR><LF>]<CR><LF>OK<CR><LF>
     *     [命令]: ^TRUSTNUM=?
     *     [结果]: <CR><LF>^TRUSTNUM: (list of supported <index>s)<CR><LF><CR><LF>OK<CR><LR>
     * [参数]:
     *     <index>: 整数值。取值范围：0~19。
     * [示例]:
     *     · 将索引为0的号码“1111111111”添加到短信可信名单中。
     *       AT^TRUSTNUM=0,"1111111111"
     *       OK
     *     · 读取短信可信名单中的所有号码：
     *       AT^TRUSTNUM?
     *       ^TRUSTNUM: 1,"1111111111"
     *       ^TRUSTNUM: 2,"2222222222"
     *       ^TRUSTNUM: 3,"3333333333"
     *       OK
     *     · 查询支持的索引列表：
     *       AT^TRUSTNUM=?
     *       ^TRUSTNUM: (0-19)
     *       OK
     */
    { AT_CMD_TRUSTNUM,
    AT_SetSmsTrustNumPara, AT_NOT_SET_TIME, AT_QrySmsTrustNumPara, AT_NOT_SET_TIME, AT_TestSmsTrustNumPara,
    AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8 *)"^TRUSTNUM", (VOS_UINT8 *)"(0-19),(number)" },

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /*
     * [类别]: 协议AT-短信业务
     * [含义]: HSMF状态控制命令
     */
    { AT_CMD_HSMF,
    AT_SetHsmfPara, AT_SET_PARA_TIME, AT_QryHsmfPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8 *)"^HSMF", (VOS_UINT8 *)"(0,1)" },
#endif
};

/* 注册taf短信业务AT命令表 */
VOS_UINT32 AT_RegisterSmsTafCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atSmsTafCmdTbl, sizeof(g_atSmsTafCmdTbl) / sizeof(g_atSmsTafCmdTbl[0]));
}

