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
#include "at_phone_taf_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_phone_taf_set_cmd_proc.h"
#include "at_phone_taf_qry_cmd_proc.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atPhoneTafCmdTbl[] = {
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 指示RSSI变化
     * [说明]: 当RSSI变化超过5dBm时，MT主动上报此指示给TE。
     * [语法]:
     *     [命令]: ^RSSI=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^RSSI=?
     *     [结果]: <CR><LF>^RSSI: (list of supported <value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: -
     *     [结果]: <CR><LF>^RSSI: <rssi><CR><LF>
     * [参数]:
     *     <n>: 整数型，设置是否主动上报系统模式，取值0～1。
     *             0：禁止主动上报系统模式；
     *             1：启动主动上报系统模式。
     *             不带该参数时，按禁止主动上报处理
     *     <rssi>: 整型值，接收信号强度，取值0～31、99。
     *             0：小于或等于–113 dBm
     *             1：–111 dBm
     *             2～30：–109 dBm～–53 dBm
     *             31：等于或大于–51 dBm
     *             99：未知或不可测。
     * [示例]:
     *       主动上报当前RSSI值
     *       ^RSSI: 16
     *     · 测试RSSI
     *       AT^RSSI=?
     *       ^RSSI: (0,1)
     *       OK
     */
    { AT_CMD_RSSI,
      At_SetRssiPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^RSSI", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置是否允许网络时间上报
     * [说明]: 网络可通过MM INFORMATION或GMM INFORMATION将当前时间及时区信息通知终端，用于终端实时调整本地时间。该命令用来设置终端是否上报给应用网络下发的时间信息。
     * [语法]:
     *     [命令]: ^TIME=<value>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TIME?
     *     [结果]: <CR><LF>^TIME: <value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^TIME=?
     *     [结果]: <CR><LF>^TIME: (list of supported <value>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 0：不上报；
     *             1：上报。
     * [示例]:
     *     · 设置打开网络时间上报
     *       AT^TIME=1
     *       OK
     *     · 查询
     *       AT^TIME?
     *       ^TIME: 1
     *       OK
     */
    { AT_CMD_TIME,
      At_SetTimePara, AT_SET_PARA_TIME, At_QryTimePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^TIME", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 网络侧系统时间查询
     * [说明]: SET命令不支持。
     *         READ命令用于查询当前EMM/MM INFO中的网络侧系统时间。如果网络侧没下发EMM/MM INFO，则需要返回当前注册上PLMN所在的时区。如果之前没收到过EMM INFO也没驻留到任何网络，返回ERROR。
     *         TEST命令不支持。
     * [语法]:
     *     [命令]: AT^TIMEQRY?
     *     [结果]: <CR><LF>^TIMEQRY: < date >,< time>,<dt><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             命令执行失败
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <date>: 日期和时间信息，格式为yy/MM/dd。
     *     <time>: 格式为"hh:mm:ss+tz"。<time>字段信息由两部分组成：时间和时区，比如“05:56:13+32”。tz:时区信息，时区单位为15分钟,  +32 表示32x15分钟 ＝ + 8小时。
     *     <dt>: 夏令时，当网络没有下发此参数时单板上报0，否则上报对应的夏令时。具体取值及其代表的意义参考table 10.5.97a/3GPP TS 24.008
     * [示例]:
     *       如果单板注册到网络上，网络侧下发GMM information，里面包括日期、时间、时区
     *       主动上报如下：^TIME: 13/03/22,12:30:00+32,0
     *     · 一个小时之后，下发查询命令：
     *       AT^TIMEQRY?
     *       ^TIMEQRY: 13/03/22,13:30:00+32,0
     *       OK
     *     · 如果单板注册到网络上，网络侧没有下发EMM/GMM/MM information,当前注册网络为”46001”，通过MNC(国家码)判断在中国，时区为东八区。下发查询命令：
     *       AT^TIMEQRY?
     *       ^TIMEQRY: 90/01/06,08:00:00+32,0
     *       OK
     */
    { AT_CMD_TIMEQRY,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryTimeQryPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TIMEQRY", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置运营商定制紧急呼号码
     * [说明]: 用于设置运营商定制紧急呼号码。
     * [语法]:
     *     [命令]: ^NVM=<index>,<total>,<ecc_num>,<category>,<simpresent>,<mcc>[,<abnormal_service>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NVM=?
     *     [结果]: <CR><LF>^NVM: (list of supported <index>s),( list of supported<total>s),<ecc_num>,( list of supported <category>s),(list of supported <simpresent>s),( list of supported <mcc>s), (list of supported <abnormal_service>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <index>: 整型值，号码条数索引，从1开始，取值范围[1-20]。
     *     <total>: 整型值，号码总条数，取值范围[1-20]。
     *     <ecc_num>: 字符串格式的紧急呼号码。
     *     <category>: 紧急呼类型，取值范围[0-255]。
     *             1：匪警；
     *             2：救护；
     *             4：火警；
     *             8：海警；
     *             16：高山营救。
     *     <simpresent>: 表示号码是在有卡时有效还是无卡时有效，取值范围[0,1]。
     *             0：无卡时有效；
     *             1：有卡时有效。
     *     <mcc>: 整型值，国家码，取值范围[0-65535]。
     *     <abnormal_service>: 整型值，表示号码有效性是否区分CS域非正常服务状态，此参数仅在<simpresent>为1即有卡时使用，取值范围[0,1]：
     *             0：所有状态均有效；
     *             1：仅在CS域非正常服务时有效。
     * [示例]:
     *     · 定制紧急呼号码
     *       AT^NVM=1,3,"999",2,1,460,0
     *       OK
     *       AT^NVM=2,3,"120",1,1,460,1
     *       OK
     *       AT^NVM=3,3,"120",1,0,460
     *       OK
     */
    { AT_CMD_NVM,
      AT_SetNvmEccNumPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NVM", (VOS_UINT8 *)"(1-20),(1-20),(eccnum),(0-255),(0,1),(0-65535),(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 紧急呼号码指示
     * [说明]: 查询命令：用户主动查询紧急呼号码，上报紧急呼号码列表。
     *         XLEMA来源：
     *         1.标准协议定义的紧急号码，包括有卡紧急号码和无卡紧急号码
     *         2.插卡时SIM（USIM）卡里保存的紧急号码
     *         3.驻网后网侧下发的紧急号码
     *         4.产品线根据需要定制的紧急号码（参考NV手册）
     *         主动上报：紧急呼叫号码变化时，主动上报紧急呼号码列表。触发主动上报条件：
     *         1.需要主动上报ECC的NV激活（参考NV手册）
     *         2.收到网侧下发ECC变化或者通过^NVM设置成功后触发主动上报功能。
     * [语法]:
     *     [命令]: ^XLEMA?
     *     [结果]: <CR><LF>^XLEMA: <index1>,<total>,<ecc_num>,<category>,<simpresent>, <mcc>,<mnc>,<abnormal_service>,<urns>,<source><CR><LF>
     *             ^XLEMA: <index2>,<total>,<ecc_num>,<category>,<simpresent>, <mcc>,<mnc>,<abnormal_service>,<urns>,<source><CR><LF>
     *             …
     *             <CR><LF>OK<CR><LF>
     *     [命令]: -
     *     [结果]: <CR><LF>^XLEMA: <index>,<total>,<ecc_num>,<category>,<simpresent>, <mcc>,<mnc>,<abnormal_service>,<urns>,<source><CR><LF>
     * [参数]:
     *     <index>: 号码条数索引，从1开始。
     *     <total>: 号码总条数。
     *     <ecc_num>: 紧急呼号码。
     *     <category>: 紧急呼类型。值可以组合。
     *             0：发起紧急呼时空口消息不携带CATEGORY IE项；
     *             1：匪警；
     *             2：救护；
     *             4：火警；
     *             8：海警；
     *             16：高山营救；
     *             31：所有紧急呼叫类型同时有效。
     *     <simpresent>: 表示号码是在有卡时有效还是无卡时有效。
     *             0：无卡时有效；
     *             1：有卡时有效。
     *     <mcc>: 国家码，默认值为fff。
     *     <mnc>: 移动网络码，默认值为ff。
     *     <abnormal_service>: 整型值，表示号码有效性是否区分CS域非正常服务状态，此参数仅在<simpresent>为1即有卡时生效：
     *             0：所有状态均有效；
     *             1：仅在CS域非正常服务时有效。
     *     <urns>: 紧急呼叫号码统一资源名称（扩展紧急呼叫号码独有），字符串类型，最大长度为64，默认为空。
     *     <source>: 整型值，取值0-3，表示紧急呼号码的来源：
     *             0：来源于APP设置（AT^NVM，即运营商定制）；
     *             1：来源于协议规定（要求modem硬编码的）；
     *             2：来源于卡文件；
     *             3：来源于网络下发。
     * [示例]:
     *     · XLEMA查询
     *       AT^XLEMA?
     *       ^XLEMA: 1,17,110,0,1,fff,ff,0,"",2
     *       ^XLEMA: 2,17,119,0,1,fff,ff,0,"",2
     *       ^XLEMA: 3,17,120,0,1,fff,ff,0,"",2
     *       ^XLEMA: 4,17,112,0,1,fff,ff,0,"",2
     *       ^XLEMA: 5,17,119,4,1,460,ff,0,"",3
     *       ^XLEMA: 6,17,112,2,1,460,ff,0,"",3
     *       ^XLEMA: 7,17,911,31,1,460,ff,0,"",3
     *       ^XLEMA: 8,17,110,1,1,460,ff,0,"",3
     *       ^XLEMA: 9,17,122,1,1,460,ff,0,"",3
     *       ^XLEMA: 10,17,123,31,1,460,ff,0,"",3
     *       ^XLEMA: 11,17,110,0,0,fff,ff,0,"",1
     *       ^XLEMA: 12,17,112,0,0,fff,ff,0,"",1
     *       ^XLEMA: 13,17,911,0,0,fff,ff,0,"",1
     *       ^XLEMA: 14,17,999,0,0,fff,ff,0,"",1
     *       ^XLEMA: 15,17,119,0,0,fff,ff,0,"",1
     *       ^XLEMA: 16,17,120,0,0,fff,ff,0,"",1
     *       ^XLEMA: 17,17,122,0,0,fff,ff,0,"",1
     *       OK
     *       主动上报
     *       ^XLEMA: 5,5,118,0,1,460,11,0,"urn:service:sos",3
     */
    { AT_CMD_XLEMA,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryXlemaPara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^XLEMA", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 使能主动上报网络选择菜单
     * [说明]: 该指示应用网络选择菜单使能或者去使能。去使能时，网络选择菜单不可用，用户不能通过网络选择菜单进行选网。
     *         该命令只有在网络选择菜单控制NV2668打开，并且卡支持该特性(卡文件6F15的PLMN_MODE位是否存在)时才会上报。
     *         该命令为私有命令，特定运营商定制。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>+PACSP <status><CR><LF>
     *     [命令]: +PACSP?
     *     [结果]: <CR><LF>+PACSP <status><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <status>: 整数型，取值0～1。
     *             0：去使能网络选择菜单，搜网模式为自动；
     *             1：使能网络选择菜单，搜网模式不改变。
     * [示例]:
     *       去使能网络选择菜单（卡模块上报消息EFCSP）
     *       +PACSP0
     *     · 查询网络选择菜单是否使能
     *       AT+PACSP?
     *       +PACSP1
     *       OK
     */
    { AT_CMD_PACSP,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryPacspPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+PACSP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 干扰检测参数配置
     * [说明]: 该命令用于配置干扰检测相关参数。
     * [语法]:
     *     [命令]: ^JDCFG=<rat>,<value1>,<value2>[,<value3>,<value4>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^JDCFG?
     *     [结果]: <CR><LF>^JDCFG: <rat>,<value1>,<value2>[,<value3>,<value4>]
     *             [<CR><LF>^JDCFG: <rat>,<value1>,<value2>[,<value3>,<value4>][…]]
     *             <CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^JDCFG=?
     *     [结果]: <CR><LF>^JDCFG: <rat>,(list of supported <value1>s),(list of supported <value2>s)[,(list of supported <value3>s),(list of supported <value4>s)]
     *             [<CR><LF>^JDCFG: <rat>,(list of supported <value1>s),(list of supported <value2>s)[,(list of supported <value3>s),(list of supported <value4>s)] […]]<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <rat>: 整型值，网络制式。
     *             0：GSM
     *             1：WCDMA
     *             2：LTE
     *     <rssi_thresh>: 整数值，测量RSSI时，接收信号强度测量值大于该门限值为可疑干扰频点。取值范围：0-70。
     *             GSM\\WCDMA制式下，rssi_thresh对应的实际门限值为：
     *             0：-70dbm
     *             1~69：-69~-1dbm
     *             70：0dbm
     *             LTE制式下，rssi_thresh对应的实际门限值为：
     *             0：-120dbm
     *             1~69：-119~-51dbm
     *             70：-50dbm
     *     <rssi_num>: 整数值，测量RSSI时，检测干扰频点个数超过该阈值，判定为受到干扰。取值范围：0-255。
     *     <rssi_percent>: 整数值，测量RSSI时，检测可疑干扰频点占频段范围频点总数的百分比。取值范围：0-100。
     *     <psch_thresh>: 整数值，测量PSCH时，测量值小于等于该阈值的频点才会被判定为干扰频点。取值范围：0-65535。
     *     <psch_percent>: 整数值，测量PSCH时，频段内干扰频点个数占可疑干扰范围内频点个数的百分比超过该阈值，该频段才被判定为受到干扰。取值范围：0-100。
     *     <pssratio_thresh>: 整数值，测量PSS Ratio时，测量值小于该阈值的频点才会被判定为干扰频点。取值范围：0-1000。
     *     <pssratio_percent>: 整数值，测量PSS Ratio时，频段内干扰频点个数占可疑干扰范围内频点个数的百分比超过该阈值，该频段才被判定为受到干扰。取值范围：0-100。
     * [表]: <value1>,<value2>,<value3>,<value4>：不同制式下对应的干扰检测配置参数，参考下面表格说明：
     *       <rat>, <value1>,    <value2>,     <value3>,        <value4>,
     *       GSM,   rssi_thresh, rssi_num,     ,                ,
     *       WCDMA, rssi_thresh, rssi_percent, psch_thresh,     psch_percent,
     *       LTE,   rssi_thresh, rssi_percent, pssratio_thresh, pssratio_percent,
     * [示例]:
     *     · 查询当前干扰检测配置参数
     *       AT^JDCFG?
     *       ^JDCFG: 0,10,30
     *       ^JDCFG: 1,40,70,0,80
     *       ^JDCFG: 2,10,70,0,80
     *       OK
     *     · 设置GSM制式下干扰检测参数
     *       AT^JDCFG=0,10,70
     *       OK
     *     · 查询干扰检测配置参数的取值范围
     *       AT^JDCFG=?
     *       ^JDCFG: (0),(0-70),(0-255)
     *       ^JDCFG: (1),(0-70),(0-100),(0-65535),(0-100)
     *       ^JDCFG: (2),(0-70),(0-100),(0-1000),(0-100)
     *       OK
     */
    { AT_CMD_JDCFG,
      AT_SetJDCfgPara, AT_SET_PARA_TIME, AT_QryJDCfgPara, AT_QRY_PARA_TIME, AT_TestJDCfgPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^JDCFG", (VOS_UINT8 *)"(0,1,2),(0-70),(0-255),(0-65535),(0-100)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 干扰检测开关命令
     * [说明]: 该命令用于配置干扰检测开关，并提供查询各个制式下干扰检测开关状态功能。干扰检测功能开启，在下一次扫频阶段生效。
     *         该AT仅对JAM_DET特性支持的产品生效，不支持该特性的产品，AT命令输入后，返回error，（Miami，Baltimore等产品不支持该AT）。
     * [语法]:
     *     [命令]: ^JDSWITCH=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^JDSWITCH?
     *     [结果]: <CR><LF>^JDSWITCH: <mode>,<rat>[<CR><LF>^JDSWITCH: <mode>,<rat>[…]]<CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^JDSWITCH=?
     *     [结果]: <CR><LF>^JDSWITCH: (list of supported <mode>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，干扰检测功能开关。
     *             0：关闭（默认值）
     *             1：开启
     *             注意：该命令用于配置GUL干扰检测功能，如果某一制式下干扰检测功能开关失败，会返回失败。此时，建议使用查询命令读取各个制式下，干扰检测功能开关状态，并重新设置。
     *     <rat>: 整型值，网络制式。
     *             0：GSM
     *             1：WCDMA
     *             2：LTE
     * [示例]:
     *     · 查询当前干扰检测开关状态
     *       AT^JDSWITCH?
     *       ^JDSWITCH: 1,0
     *       ^JDSWITCH: 1,1
     *       ^JDSWITCH: 1,2
     *       OK
     *     · 开启干扰检测功能
     *       AT^JDSWITCH=1
     *       OK
     */
    { AT_CMD_JDSWITCH,
      AT_SetJDSwitchPara, AT_SET_PARA_TIME, AT_QryJDSwitchPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^JDSWITCH", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置通信打字机模式
     * [说明]: 该命令用于设置通信打字机模式，用于上层将TTY模式配置到modem，以及通过此命令查询modem当前的TTY模式。
     *         modem开机初始化默认为TTY_OFF模式。
     *         双卡双通形态手机要求应用主副卡设置的TTY模式一致，如果不一致modem将以最后一次设置的TTY模式为准。
     * [语法]:
     *     [命令]: ^TTYMODE=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TTYMODE?
     *     [结果]: <CR><LF>^TTYMODE: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TTYMODE=?
     *     [结果]: <CR><LF>^TTYMODE: (list of supported <mode>s) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整数型，TTY模式，取值0~3。
     *             0：TTY_OFF模式；
     *             1：TTY_FULL模式；
     *             2：TTY_VCO模式；
     *             3：TTY_HCO模式。
     * [示例]:
     *     · 设置TTY模式为TTY_FULL模式
     *       AT^TTYMODE=1
     *       OK
     *     · 查询TTY模式
     *       AT^TTYMODE?
     *       ^TTYMODE: 1
     *       OK
     *     · 执行测试命令
     *       AT^TTYMODE=?
     *       ^TTYMODE: (0-3)
     *       OK
     */
    { AT_CMD_TTYMODE,
      AT_SetTTYModePara, AT_SET_PARA_TIME, AT_QryTTYModePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TTYMODE", (VOS_UINT8 *)"(0-3)" },

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    { AT_CMD_CMUX,
      At_SetCmuxPara, AT_SET_PARA_TIME, At_QryCmuxPara, AT_QRY_PARA_TIME, At_TestCmuxPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8*)"+CMUX", (VOS_UINT8*)"(0),(0),(1-6),(31-1540),(10-250),(0-10),(10-250),(1-255),(1-7)"},
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /* AUTO TEST */
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: C核CPU负载查询
     * [说明]: 该命令用于查询C核CPU负载。
     * [语法]:
     *     [命令]: ^CPULOAD?
     *     [结果]: <CR><LF>^CPULOAD: <cpu_load>[…] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cpu_load>: 浮点型，每个核的负载占用百分比。
     *             结果保留2位小数。
     * [示例]:
     *     · 查询命令
     *       AT^CPULOAD?
     *       ^CPULOAD: 1.20,1.00,0.00,0.00
     *       OK
     */
    { AT_CMD_CPULOAD,
      TAF_NULL_PTR, AT_NOT_SET_TIME, At_QryCpuLoadPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CPULOAD", TAF_NULL_PTR },
#endif
};

/* 注册taf phone AT命令表 */
VOS_UINT32 AT_RegisterPhoneTafCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atPhoneTafCmdTbl, sizeof(g_atPhoneTafCmdTbl) / sizeof(g_atPhoneTafCmdTbl[0]));
}
