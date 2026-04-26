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
#include "at_ltev_as_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_ltev_as_set_cmd_proc.h"
#include "at_ltev_as_qry_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_AS_CMD_TBL_C

#if (FEATURE_LTEV == FEATURE_ON)
static const AT_ParCmdElement g_atLtevAsCmdTbl[] = {
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 激活测试模式
     * [说明]: 本命令用于设置激活测试模式。
     * [语法]:
     *     [命令]: +CATM=[<status>[,<test_loop_mode>]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CATM?
     *     [结果]: 执行成功时：
     *             +CATM: <status>[,<test_loop_mode>]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CATM=?
     *     [结果]: +CATM: (list of supported <status>s),(list of supported <test_loop_mode>s)
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <status>: 测试模式激活状态，取值范围0~1，默认值0。
     *             0表示去激活，1表示激活
     *     <test_loop_mode>: 测试模式类型，取值范围1。
     *             1表示测试模式E
     * [示例]:
     *     · 设置激活测试模式
     *       AT+CATM=1,1
     *       OK
     *     · 查询测试模式激活状态（当前处于测试模式E激活状态）
     *       AT+CATM?
     *       +CATM: 1,1
     *       OK
     */
    { AT_CMD_CATM,
      AT_SetCatm, AT_SET_PARA_TIME, AT_QryCatm, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CATM", (VOS_UINT8 *)"(0,1),(1)" },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 修改测试模式发包的L2ID
     * [说明]: 本命令用于在 +CATM激活后，使用+CCUTLE或CV2XDTS命令进行发包时，数据包的L2 src Id和dst Id。
     * [语法]:
     *     [命令]: ^CV2XL2ID=<srcId>,<dstId>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^CV2XL2ID?
     *     [结果]: 执行成功时：
     *             ^CV2XL2ID: <src Id>,<dst Id>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <src Id>: 测试模式发包的L2ID源侧ID，范围0-16777215。
     *     <dst Id>: 测试模式发包的L2ID目标ID，范围0-16777215。
     * [示例]:
     *     · 设置测试模式L2ID
     *       AT^CV2XL2ID=187,187
     *       OK
     *     · 查询测试模式L2ID
     *       AT^CV2XL2ID?
     *       ^CV2XL2ID: 187,187
     *       OK
     */
    { AT_CMD_CV2XL2ID,
      AT_SetCv2xL2Id, AT_SET_PARA_TIME, AT_QryCv2xL2Id, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CV2XL2ID", (VOS_UINT8 *)"(0-16777215),(0-16777215)" },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 闭环测试模式E
     * [说明]: 本命令用于设置闭环测试模式E（在3GPP TS 35.509子章节5.4.2、5.4.4c、5.4.5中定义），该命令在测试模式激活情况下才能使用。
     * [语法]:
     *     [命令]: +CCUTLE=<status>[,<direction>[,<format>,<length>,<monitor_list>]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CCUTLE?
     *     [结果]: 执行成功时：
     *             +CCUTLE: <status>[,<direction>[,<format>,<length>,<monitor_list>]]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CCUTLE=?
     *     [结果]: +CCUTLE: (list of supported <status>s),(list of supported <direction>s),(list of supported <format>s)(maximum supported <length>)
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <status>: 测试模式E闭环状态，取值范围0~1，默认值1。
     *             0表示闭环，1表示开环
     *     <direction>: 通信方向，取值范围0~1。
     *             0表示数据接收，1表示数据发送
     *     <format>: 监控列表参数格式，取值范围1。
     *             1表示二进制格式
     *     <length>: 监控列表中以24bit表示的目的Layer-2 ID数量，最多支持16组
     *     <monitor_list>: V2X通讯监控列表
     * [示例]:
     *     · 设置进入测试模式E数据接收状态
     *       AT+CCUTLE=0,0,1,1,"000000"
     *       OK
     *     · 查询测试模式状态（当前处于测试模式E数据接收状态）
     *       AT+CCUTLE?
     *       +CCUTLE: 0,0,1,1,"000000"
     *       OK
     */
    { AT_CMD_CCUTLE,
      AT_SetCcutle, AT_SET_PARA_TIME, AT_QryCcutle, AT_QRY_PARA_TIME, AT_TestCcutle, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CCUTLE", (VOS_UINT8 *)"(0,1),(0,1),(1),(0-16),(str)" },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 侧路数据包计数
     * [说明]: 本命令用于请求UE报告侧路成功接收到的PSCCH传输块、STCH PDCP SDU包、PSSCH传输块数量。该命令在测试模式激活情况下才能使用。
     * [语法]:
     *     [命令]: +CUSPCREQ
     *     [结果]: 执行成功时：
     *             +CUSPCREQ: [<type1>,<format>,<length1>,<counter1>],[<type2>,<format>,<length2>,<counter2>],[<type3>,<format>,<length3>,<counter3>]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <type>: V2X通信类型，取值范围1。
     *             1表示PSCCH传输块
     *     <type2>: V2X通信类型，取值范围1。
     *             1表示STCH PDCP SDU包
     *     <type3>: V2X通信类型，取值范围1。
     *             1表示PSSCH传输块
     *     <format>: 请求包计数的格式，取值范围1。
     *             1表示二进制格式
     *     <length1>: <counter1>中元素字节数
     *     <length2>: <counter2>中元素字节数
     *     <length3>: <counter3>中元素字节数
     *     <counter1>: PSCCH传输块数量
     *     <counter2>: STCH PDCP SDU包数量
     *     <counter3>: PSSCH传输块数量
     * [示例]:
     *     · 请求侧路数据包数量（PSCCH传输块、STCH PDCP SDU包、PSSCH传输块数量各有一个Layer-2 ID且只收到一个数据包）
     *       AT+CUSPCREQ
     *       +CUSPCREQ: 1,1,4,"00000000000000000000000000000001",1,1,4,"00000000000000000000000000000001",1,1,4,"00000000000000000000000000000001"
     *       OK
     */
    { AT_CMD_CUSPCREQ,
      AT_SetCuspcreq, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CUSPCREQ", VOS_NULL_PTR },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: UTC时间重置
     * [说明]: 本命令用于重置当前UTC时间。该命令在测试模式激活情况下才能使用。
     * [语法]:
     *     [命令]: +CUTCR
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     * [示例]:
     *     · UTC时间重置
     *       AT+CUTCR
     *       OK
     */
    { AT_CMD_CUTCR,
      AT_SetCutcr, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CUTCR", VOS_NULL_PTR },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 测试模式下信道忙率请求
     * [说明]: 本命令用于请求PC5上测量到的V2X通信信道忙率。该命令在测试模式激活情况下才能使用。
     * [语法]:
     *     [命令]: +CCBRREQ
     *     [结果]: 执行成功时：
     *             +CCBRREQ: <cbr-pssch>[,<cbr-pscch>]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <cbr-pssch>: PC5上V2X通信PSSCH信道忙率。
     *             整型值，取值范围：1-100
     *             0表示CBR=0.0，
     *             1表示CBR=0.01，
     *             2表示CBR=0.02，
     *             ……
     *             100表示CBR=1.00
     *     <cbr-pscch>: PC5上V2X通信PSCCH信道忙率。
     *             整型值,取值范围：1-100
     *             0表示CBR=0.0，
     *             1表示CBR=0.01，
     *             2表示CBR=0.02，
     *             ……
     *             100表示CBR=1.00
     *             仅当在3GPP TS 36.214子条款5.1.30中指定的非相邻资源块中传输PSSCH和PSCCH时，UE才返回此参数
     * [示例]:
     *     · 信道忙率请求
     *       AT+CCBRREQ
     *       +CCBRREQ: 7
     *       OK
     */
    { AT_CMD_CCBRREQ,
      AT_SetCcbrreq, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CCBRREQ", VOS_NULL_PTR },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: PC5上V2X数据传输
     * [说明]: 本命令用于触发UE开始和停止在PC5上发送V2X数据（数据格式定义在3GPP TS 36.213）。该命令在测试模式激活情况下才能使用。
     * [语法]:
     *     [命令]: +CV2XDTS=<action>[,<data_size>,<periodicity>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CV2XDTS?
     *     [结果]: 执行成功时：
     *             +CV2XDTS: <action>[,<data_size>,<periodicity>]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CV2XDTS=?
     *     [结果]: +CV2XDTS: (list of supported <action>s)[,(maximum supported <data_size>),(minimum supported <periodicity>)]
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <action>: UE开始或停止在PC5上发送数据，取值范围0~1，默认值0。
     *             0表示停止发送数据，1表示开始发送数据
     *     <data_size>: 指示要发送的数据字节数，取值范围0~8176
     *     <periodicity>: UE传输指定字节data_size需要的时间，单位ms，取值范围10~10000。注：如果是20MB带宽，最小周期为10ms，如果是10MB带宽，实际最小周期为20ms
     * [示例]:
     *     · 设置开始发送V2X数据，100ms发送100字节
     *       AT+CV2XDTS=1,100,100
     *       OK
     *     · 查询V2X数据发送状态（当前处于开始发送状态，100字节/100ms）
     *       AT+CV2XDTS?
     *       +CV2XDTS: 1,100,100
     *       OK
     */
    { AT_CMD_CV2XDTS,
      AT_SetCv2xdts, AT_SET_PARA_TIME, AT_QryCv2xdts, AT_QRY_PARA_TIME, AT_TestCv2xdts, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CV2XDTS", (VOS_UINT8 *)"(0,1),(0-8176),(10-10000)" },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 同步源查询
     * [说明]: 本命令用于获取当前同步源信息，支持主动上报。
     * [语法]:
     *     [命令]: ^VSYNCSRC?
     *     [结果]: 执行成功时：
     *             ^VSYNCSRC: <source>,<state>,<earfcn>,<slss_id>,<subslss_id>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <source>: 同步源，取值范围0~3。
     *             0表示GNSS，1表示eNodeB，2表示参考UE，3表示自同步。注：目前暂不支持eNodeB作为同步源。
     *     <state>: 同步状态，取值范围0~1。
     *             0表示同步，1表示失步
     *     <earfcn>: 当前同步源工作频点
     *     <slss_id>: 取值范围是0-335
     *     <subslss_id>: 取值范围是0-65535
     * [示例]:
     *     · 查询同步源
     *       AT^VSYNCSRC?
     *       ^VSYNCSRC: 0,0,10,200,255
     *       OK
     */
    { AT_CMD_VSYNCSRC,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryVsyncsrc, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VSYNCSRC", VOS_NULL_PTR },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 设置同步源上报
     * [说明]: 本命令用于设置同步源变化上报。
     * [语法]:
     *     [命令]: ^VSYNCSRCRPT=<status>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^VSYNCSRCRPT?
     *     [结果]: 执行成功时：
     *             ^VSYNCSRCRPT: <status>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^VSYNCSRCRPT=?
     *     [结果]: ^VSYNCSRCRPT: (0,1)
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <status>: 使能状态，取值范围0~1。
     *             0表示去使能，1表示使能
     * [示例]:
     *     · 使能同步源上报
     *       AT^VSYNCSRCRPT=1
     *       OK
     */
    { AT_CMD_VSYNCSRCRPT,
      AT_SetVsyncsrcrpt, AT_SET_PARA_TIME, AT_QryVsyncsrcrpt, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VSYNCSRCRPT", (VOS_UINT8 *)"(0,1)" },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 设置同步模式
     * [说明]: 本命令用于设置同步模式。
     * [语法]:
     *     [命令]: ^VSYNCMODE=<mode>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^VSYNCMODE?
     *     [结果]: 执行成功时：
     *             ^VSYNCMODE: <mode>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^VSYNCMODE=?
     *     [结果]: ^VSYNCMODE: (0,1)
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 同步模式，取值范围0~1。
     *             0表示仅GNSS，1表示GNSS+参考UE+自同步
     * [示例]:
     *     · 设置同步模式为GNSS+参考UE+自同步
     *       AT^VSYNCMODE=1
     *       OK
     */
    { AT_CMD_VSYNCMODE,
      AT_SetVsyncmode, AT_SET_PARA_TIME, AT_QryVsyncmode, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VSYNCMODE", (VOS_UINT8 *)"(0,1)" },
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 信道忙率请求
     * [说明]: 本命令用于请求PC5上测量到的V2X通信信道忙率，支持主动上报。
     * [语法]:
     *     [命令]: ^CBR?
     *     [结果]: 执行成功时：
     *             ^CBR: <cbr-pssch>[,<cbr-pscch>]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <cbr-pssch>: PC5上V2X通信PSSCH信道忙率。
     *             整型值，取值范围：1-100
     *             0表示CBR=0.0，
     *             1表示CBR=0.01，
     *             2表示CBR=0.02，
     *             ……
     *             100表示CBR=1.00
     *     <cbr-pscch>: PC5上V2X通信PSCCH信道忙率。
     *             整型值,取值范围：1-100
     *             0表示CBR=0.0，
     *             1表示CBR=0.01，
     *             2表示CBR=0.02，
     *             ……
     *             100表示CBR=1.00
     *             仅当在3GPP TS 36.214子条款5.1.30中指定的非相邻资源块中传输PSSCH和PSCCH时，UE才返回此参数。
     * [示例]:
     *     · 请求信道忙率
     *       AT^CBR?
     *       ^CBR: 7
     *       OK
     */
    { AT_CMD_CBR,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryCbr, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CBR", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 装备启动GNSS测试命令
     * [说明]: 本命令用于装备测试命令，启动GNSS测试。
     * [语法]:
     *     [命令]: ^GNSSTEST
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <NA>: NA
     * [示例]:
     *     · 启动命令
     *       AT^GNSSTEST
     *       OK
     */
    { AT_CMD_GNSSTEST,
      AT_SetGnssInfo, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^GNSSTEST", TAF_NULL_PTR },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 装备查询同步状态
     * [说明]: 本命令用于装备查询PC5口同步状态。
     * [语法]:
     *     [命令]: ^QUERYDATA?
     *     [结果]: 执行成功时：
     *             <CR><LF>^QUERYDATA: <1ppsStat>,<SyncStat>,<SatNumStat><CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <1ppsStat>: 1PPS秒状态，1：正常，0：不正常  (需启动^GNSSTEST获取1pps同步状态)
     *     <SyncStat>: 卫星信号质量，1：优，0：劣
     *     <SatNum>: 卫星数量
     * [示例]:
     *     · 查询命令
     *       AT^QUERYDATA?
     *       ^QUERYDATA: 1,1,1
     *       OK
     */
    { AT_CMD_QRYDATA,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryData, AT_QRY_PARA_TIME, AT_TestQryData, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^QUERYDATA", (VOS_UINT8 *)"(0,1),(0,1),(0-50)" },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 查询GNSS信息
     * [说明]: 本命令用于查询GNSS信息。
     * [语法]:
     *     [命令]: ^GNSSINFO
     *     [结果]: 执行成功时：
     *             <CR><LF>^GNSSINFO: <CR><LF><longitude>,<latitude><CR><LF><SatNum><CR><LF>No<i>:<GnssSystemType>        <CN0><CR><LF>[……]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <longitude>: 经度
     *     <latitude>: 维度
     *     <SatNum>: 卫星数量
     *     <i>: 显示序号
     *     <GnssSystemType>: 卫星系统类型，
     *             0：GPS
     *             1：Galileo
     *             2：Glonass
     *             3：QZSS
     *             4：SBAS
     *             5：BDS
     *     <CN0>: CN0值
     * [示例]:
     *     · 查询GNSS信息命令
     *       AT^GNSSINFO
     *       ^GNSSINFO：
     *       108.835647,34.201000
     *       3
     *       No0:0      45
     *       No1:0      41
     *       No2:0      57
     *       OK
     */
    { AT_CMD_GNSSINFO,
      AT_GnssInfo, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^GNSSINFO", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 查询LTE-V物理层收发统计
     * [说明]: 本命令用于查询LTE-V物理层收发统计。
     * [语法]:
     *     [命令]: ^VPHYSTAT?
     *     [结果]: 执行成功时：
     *             <CR><LF>^VPHYSTAT: <FirstSADecSucSum>,<SecSADecSucSum>,<TwiSADecSucSum>,
     *             <UplinkPackSum>,<DownlinkPackSum>,<PhyUplinkPackSum>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <FirstSADecSucSum>: SA第一次传输解析出数目
     *     <SecSADecSucSum>: SA第二次传输解析出数目
     *     <TwiSADecSucSum>: SA两次传输都解析出数目
     *     <UplinkPackSum>: 上行发包数
     *     <DownlinkPackSum>: 下行收包数
     *     <PhyUplinkPackSum>: 物理层上行发包数
     * [示例]:
     *     · 查询LTE-V物理层收发统计
     *       AT^VPHYSTAT?
     *       ^VPHYSTAT: 1,2,2,2,2,XXXXX
     *       OK
     */
    { AT_CMD_VPHYSTAT,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryVPhyStat, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VPHYSTAT", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 清除LTE-V物理层收发统计
     * [说明]: 本命令用于清除LTE-V物理层收发统计。
     * [语法]:
     *     [命令]: ^VPHYSTATCLR=0
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <NA>: NA
     * [示例]:
     *     · 清除LTE-V物理层收发统计
     *       AT^VPHYSTATCLR=0
     *       OK
     */
    { AT_CMD_VPHYSTATCLR,
      AT_SetVPhyStatClr, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VPHYSTATCLR", (VOS_UINT8 *)"(0)" },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: LTE-V PC5口SINR和RSRP查询命令
     * [说明]: 本命令用于LTE-V PC5口SINR和RSRP查询。
     * [语法]:
     *     [命令]: ^VSNRRSRP?
     *     [结果]: 执行成功时：
     *             <CR><LF>^VSNRRSRP:<MsgNum><CR><LF>
     *             ^VSNRRSRP: <SAMainSNR>      <SADiversitySNR>
     *             <DAMainSNR>      <DADiversitySNR>      <DARSRP>
     *             <DAMainMaxRsrp>      <DADiversityMaxRsrp>[<CR><LF>……]
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <MsgNum>: 整型值。
     *             返回结果数
     *     <SAMainSNR>: 整型值，输出参数占5个字符位，参数位数（含负号）小于5时用空格进行左补齐。
     *             SA主集SINR
     *     <SADiversitySNR>: 整型值，输出参数占5个字符位，参数位数（含负号）小于5时用空格进行左补齐。
     *             SA分集SINR
     *     <DAMainSNR>: 整型值，输出参数占5个字符位，参数位数（含负号）小于5时用空格进行左补齐。
     *             DA主集SINR
     *     <DADiversitySNR>: 整型值，输出参数占5个字符位，参数位数（含负号）小于5时用空格进行左补齐。
     *             DA分集SINR
     *     <DARSRP>: 整型值，输出参数占5个字符位，参数位数（含负号）小于5时用空格进行左补齐。
     *             DA RSRP
     *     <DAMainMaxRsrp>: 整型值，输出参数占5个字符位，参数位数（含负号）小于5时用空格进行左补齐。
     *             主集PSSCH RSRP最大值，周期目前为200ms
     *     <DADiversityMaxRsrp>: 整型值，输出参数占5个字符位，参数位数（含负号）小于5时用空格进行左补齐。
     *             分集PSSCH RSRP最大值，周期目前为200ms
     * [示例]:
     *     · 查询PC5口SINR和RSRP
     *       AT^VSNRRSRP?
     *       ^VSNRRSRP: 1
     *       ^VSNRRSRP:   -56        -11         15         24        -93       -126       -126
     *       OK
     */
    { AT_CMD_VSNRRSRP,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryVSnrRsrp, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VSNRRSRP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 查询LTE-V PC5口主分集RSSI命令
     * [说明]: 本命令用于LTE-V PC5口主分集RSSI命令查询。
     * [语法]:
     *     [命令]: ^V2XRSSI?
     *     [结果]: 执行成功时：
     *             <CR><LF>^V2XRSSI: <DAMainRSSIMax >
     *             <CR><LF><DADiversityRSSIMax >
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <DAMainRSSIMax>: DA主集最大RSSI值
     *     <DADiversityRSSIMax>: DA分集最大RSSI值
     * [示例]:
     *     · 查询PC5口主分集RSSI
     *       AT^V2XRSSI?
     *       ^V2XRSSI: -80,-90
     *       OK
     */
    { AT_CMD_V2XRSSI,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryV2xRssi, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^V2XRSSI", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 设置天线输出功率
     * [说明]: 本命令用于设置调整LTE-V天线输出功率。
     *         注：只支持设置命令，不支持查询和测试。
     *         如果要设置最大输出功率，可以将参数设置200~240之间即可。
     * [语法]:
     *     [命令]: ^VTXPOWER=<power>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <power>: 最小设置为（-62dbm），最大设置（30dbm）
     *             以1/8dBm为精度递增。AT命令范围-496 ~240；
     *             （例：拟设置功率为12.5dBm，则通过AT命令下的数值为100）
     * [示例]:
     *     · 设置输出功率为10dBm
     *       AT^VTXPOWER=80
     *       OK
     */
    { AT_CMD_VTXPOWER,
      AT_SetVTxPower, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VTXPOWER", (VOS_UINT8 *)"(@Power)" },
};

/* 注册AS LTEV AT命令表 */
VOS_UINT32 AT_RegisterLtevAsCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atLtevAsCmdTbl, sizeof(g_atLtevAsCmdTbl) / sizeof(g_atLtevAsCmdTbl[0]));
}
#endif

