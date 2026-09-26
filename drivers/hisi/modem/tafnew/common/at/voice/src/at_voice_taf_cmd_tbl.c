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
#include "at_voice_taf_cmd_tbl.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_voice_taf_set_cmd_proc.h"
#include "at_voice_taf_qry_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atVoiceTafCmdTbl[] = {
#if (FEATURE_MBB_CUST == FEATURE_OFF)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 通话中发送DTMF音
     * [说明]: 该命令用于通话过程中向网侧发送DTMF音。当FEATURE_MBB_CUST宏打开时可打断向网侧发送DTMF音。
     * [语法]:
     *     [命令]: ^DTMF=[<call_id>],[<Dtmf_Key>],<On_Length>[,<off_length>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^DTMF=?
     *     [结果]: <CR><LF> ERROR<CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫ID，取值范围1~7。
     *             该参数可携带，也可不携带。如果携带该参数，必须是当前呼叫的呼叫ID。
     *     <Dtmf_Key>: 开始向网侧发送DTMF时（即On_Length取值非0）：需要发出的一个ASCII字符，取值范围为0～9、#、*、A～D。AT命令中必须携带该参数。
     *             停止向网侧发送DTMF时（即On_Length取值为0）：不关注此参数，AT命令中可以不携带该参数，也可以携带该参数。
     *     <On_Length>: DTMF音播放的时长。
     *             0：停止向网侧发送；
     *             1：开始向网侧发送DTMF，默认时长60s。
     *             当设置为1后，需要设置0才能停止播放。
     *             非默认时长取值为：95、150、200、250、300、350，单位是毫秒。时间到期后，自动停止播放。
     *     <off_length>: DTMF发送的间隔，即STOP DTMF ACK ~ START DTMF REQ的时间间隔，单位为毫秒。
     *             取值范围0ms或者10ms-60000ms。
     *             可选参数，不带该参数时默认0ms。
     * [示例]:
     *     · 开始发送DTMF
     *       AT^DTMF=1,A,1,0
     *       OK
     *     · 停止发送DTMF
     *       AT^DTMF=1,A,0,0
     *       OK
     *     · 指定时长150ms发送DTMF
     *       AT^DTMF=1,A,150,0
     *       OK
     *     · 指定前一次DTMF的STOP DTMF ACK 到下一次START DTMF REQDTMF发送的时间间隔为70ms
     *       AT^DTMF=1,A,1,70
     *       OK
     *       AT^DTMF=1,A,0,70
     *       OK
     *     · 测试^DTMF
     *       AT^DTMF=?
     *       ERROR
     */
    { AT_CMD_DTMF,
      At_SetDtmfPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DTMF", (VOS_UINT8 *)"(0-7),(@key),(0,1,95,120,150,200,250,300,350),(0-60000)" },
#else
    { AT_CMD_DTMF,
      At_SetDtmfPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      AT_AbortDtmfPara, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DTMF", (VOS_UINT8 *)"(0-7),(@key),(0,1,95,120,150,200,250,300,350),(0-60000)" },
#endif

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询通话时长
     * [说明]: 查询正在进行通话的通话时长。
     *         注意：X模下不支持查询。
     * [语法]:
     *     [命令]: ^CDUR=<idx>
     *     [结果]: <CR><LF>^CDUR: <idx>,<duration><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CDUR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <idx>: 呼叫ID。
     *     <duration>: 通话时长，单位：秒。
     * [示例]:
     *     · 查询当前通话时长
     *       AT^CDUR=1
     *       ^CDUR: 1,136
     *       OK
     */
    { AT_CMD_CDUR,
      AT_SetCdurPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CDUR", (VOS_UINT8 *)"(0-7)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 带子地址的呼叫
     * [说明]: 发起一个带子地址的呼叫（该过程可以被打断）。
     * [语法]:
     *     [命令]: ^APDS=<dial_string>[,<sub_string>[,<I>[,<G>[,<call_type>[,<callDomain>[,<srv_type>[,<rtt>[,<CallPull_DialogId>[,<isEncrypt>]]]]]]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>NO CARRIER<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <dial_string>: 字符串类型，主叫号码（最大40个BCD编码字符）。
     *     <sub_string>: 字符串类型，子地址（最大20个字符）。
     *     <I>: 整型值，是否显示主叫号码。
     *             0：不显示；
     *             1：显示。
     *     <G>: 整型值，是否使能CUG补充业务。
     *             0：禁止；
     *             1：使能。
     *     <call_type>: 整型值，呼叫类型。
     *             0：语音呼叫；
     *             1：视频通话：单向发送视频，双向语音；
     *             2：视频通话：单向接收视频，双向语音；
     *             3：视频通话：双向视频，双向语音。
     *     <callDomain>: 整型值，呼叫域。
     *             0：CS域
     *             1：PS域
     *     <srv_type>: 整型值，业务类型。
     *             0：无业务类型
     *             1：WPS呼叫
     *     <rtt>: 整型值，指示是否发起RTT通话：
     *             0：非RTT通话；
     *             1：RTT通话。
     *     <CallPull_DialogId>: 整型值，存在多路可拉取呼叫时，应用发起Call Pull操作时的DialogId，取值范围：0~255。
     *             255：非Call Pull操作；
     *             0~254：Call Pull操作时的DialogID。
     *     <isEncrypt>: 整型值，是否语音加密：
     *             0：非语音加密
     *             1：语音加密
     * [示例]:
     *     · 带有子地址的IMS呼叫
     *       AT^APDS="12345678900","1234",1,0,0,1,0,0,0
     *       OK
     *     · 不带子地址的IMS视频电话
     *       AT^APDS="12345678900",,,,1,1,0,0,0
     *       OK
     */
    { AT_CMD_APDS,
      AT_SetApdsPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^APDS", (VOS_UINT8 *)"(@dial_string),(@subaddr),(0,1),(0,1),(0-3),(0,1),(0,1),(0,1),(0-255),(0,1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询呼叫源号码
     * [说明]: 查询GUL模呼叫源号码和存在指示。
     * [语法]:
     *     [命令]: ^CLPR=<call_id>
     *     [结果]: <CR><LF>^CLPR: <calling_num_pi>,<no_CLI_cause>,<redirect_num>, <num_type>,<redirect_subaddr>,<subaddr_type>,<redirect_num_pi><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CLPR=?
     *     [结果]: <CR><LF>^CLPR: (list of supported <call_id>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫ID，取值范围1~7。
     *     <calling_num_pi>: 整型值，主叫号码存在指示。
     *             0：允许显示
     *             1：限制显示
     *             2：号码不可用
     *     <no_CLI_cause>: 整型值，号码不存在的原因值。
     *             0：号码不可用
     *             1：被用户拒绝
     *             2：与其他服务的交互
     *             3：付费电话
     *     <redirect_num>: 字符串类型，呼叫转移号码。
     *     <num_type>: 整型值，号码地址类型。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其它值请参见2.4.11 主动上报来电号码：+CLIP的参数<type>。
     *     <redirect_subaddr>: 字符串类型，子地址。
     *     <subaddr_type>: 整型值，子地址类型。
     *     <redirect_num_pi>: 整型值，呼叫转移号码存在指示：
     *             0：允许通话
     *             1：通话受限
     *             2：无法拨打
     *             3：通话保留
     * [示例]:
     *     · 查询呼叫源号码成功
     *       AT^CLPR=1
     *       ^CLPR: 0,,"12345678900",129,"1234",0,0
     *       OK
     *     · 测试^CLPR命令
     *       AT^CLPR=?
     *       ^CLPR: (1-7)
     *       OK
     */
    { AT_CMD_CLPR,
      AT_SetClprPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestClprPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CLPR", (VOS_UINT8 *)"(1-7)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询最后一次CS域呼叫错误码
     * [说明]: 查询最后一次CS域上报的错误原因值。
     * [语法]:
     *     [命令]: ^CCSERR?
     *     [结果]: <CR><LF>^CCSERR: <cause><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CCSERR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cause>: 整型值，详见“4.5 CS域错误码”。
     * [示例]:
     *     · CS域呼叫失败，查询原因值
     *       AT^CCSERR?
     *       ^CCSERR: 1
     *       OK
     *     · 测试命令
     *       AT^CCSERR=?
     *       OK
     */
    { AT_CMD_CCSERR,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryCCsErrPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CCSERR", VOS_NULL_PTR },

#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询LTE网络下CS业务实现方式
     * [说明]: 该命令用于UE查询所注册的LTE网络是否支持指定的CS业务实现方式。
     * [语法]:
     *     [命令]: ^LTECS?
     *     [结果]: <CR><LF>^LTECS: <SMS OVER SGs>,<SMS OVER IMS>,<CSFB>,<SR-VCC>,<VoLGA><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <SMS OVER SGs>: 是否支持SMS OVER SGs。
     *             0：不支持；
     *             1：支持。
     *     <SMS OVER IMS>: 是否支持SMS OVER IMS。
     *             0：不支持；
     *             1：支持。
     *     <CSFB>: 是否支持CSFB。
     *             0：不支持；
     *             1：支持。
     *     <SR-VCC>: 是否支持SR-VCC。
     *             0：不支持；
     *             1：支持。
     *     <VoLGA>: 是否支持VoLGA。
     *             0：不支持；
     *             1：支持。
     * [示例]:
     *     · 如果当前注册的LTE网络支持SMS OVER SGs和CSFB，下发此命令查询
     *       AT^LTECS?
     *       ^LTECS: 1,0,1,0,0
     *       OK
     */
    { AT_CMD_LTECS,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryLtecsInfo, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LTECS", TAF_NULL_PTR },
#endif

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: IMS会议电话加入第三方
     * [说明]: 在IMS电话会议过程中，拉第三方时使用。
     * [语法]:
     *     [命令]: ^CACMIMS=<total_callnum>,<isEcon>,<call_address>[,call_address[….]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CACMIMS=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <total_callnum>: 需要加入多方通话中第三方号码的个数
     *     <isEcon>: 整型值，是否是增强型多方通话
     *             0：不是增强型多方通话
     *             1：是增强型多方通话
     *     <call_address>: 字符串类型，第三方电话号码，增强型多方通话过程中可以拉多个用户，非增强型多方通话仅能拉一个用户。
     * [示例]:
     *     · IMS会议电话拉第三方
     *       AT^CACMIMS=1,0,"+8613923002240"
     *       OK
     *     · IMS增强型会议电话拉入多个与会者
     *       AT^CACMIMS=2,1,"+8613923002240","+8613816980803"
     *       OK
     *     · 测试CACMIMS
     *       AT^CACMIMS=?
     *       OK
     */
    { AT_CMD_CACMIMS,
      AT_SetCacmimsPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CACMIMS", (VOS_UINT8 *)"(1-5),(0-1),(@number),(@number),(@number),(@number),(@number)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 发起增强型多方通话
     * [说明]: 用于发起增强型多方通话。
     * [语法]:
     *     [命令]: ^ECONFDIAL=<dial_count>,<conf_center_string>,<I>,<call_type>,<callDomain>,<isEcon>,<dial_string>[,<dial_string>…]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>NO CARRIER<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^ECONFDIAL?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^ECONFDIAL=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <dial_count>: 整型值，邀请进入会议的人数，最多为5个
     *     <conf_center_string>: 字符串类型，会议中心号码（最大40个BCD编码字符）
     *             目前会议中心号码保存在NV中，该参数预留以后使用
     *     <I>: 整型值，是否显示主叫号码。
     *             0：不显示；
     *             1：显示。
     *     <call_type>: <call_type> 整型值，呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     *     <callDomain>: 整型值，呼叫域
     *             0： CS 域
     *             1： PS 域
     *     <isEcon>: 整型值，是否是增强型多方通话
     *             0：不是增强型多方通话
     *             1：是增强型多方通话
     *     <dial_string>: 字符串类型，呼叫号码（最大40个BCD编码字符）
     * [示例]:
     *     · 发起增强型多方通话，携带两个用户
     *       AT^ECONFDIAL=2,"",1,0,1,1,"13812344321","13943211234"
     *       OK
     *     · 查询命令
     *       AT^ECONFDIAL?
     *       OK
     *     · 测试ECONFDIAL
     *       AT^ECONFDIAL=?
     *       OK
     */
    { AT_CMD_ECONFDIAL,
      AT_SetEconfDialPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECONFDIAL", (VOS_UINT8 *)"(1-5),(@econf_center),(0,1),(0-3),(0,1),(0,1),(@number),(@number),(@number),(@number),(@number)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 获取与会者信息
     * [说明]: 用于获取增强型多方通话会议的与会者信息，在收到^CLCCESTATE主动上报与会者状态发生改变后，上层下发^CLCCECONF来查询与会者信息。
     * [语法]:
     *     [命令]: ^CLCCECONF?
     *     [结果]: <CR><LF>  [^CLCCECONF: Maximum-user-count, n_address, entity, Display-text,Status[,entity,  Display-text,Status[..]]]
     *             <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CLCCECONF=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Maximum-user-count>: 整型值，增强型多方通话最大与会者个数。(如果增强型会议没有创建，该值为0)
     *     <n_address>: 整型值，多方通话与会者个数
     *     <entity>: 呼叫号码
     *     <Display-text>: 呼叫号码名称
     *     <Status>: 整型值，与会者状态。
     *             0：disconnected；
     *             1：connected；
     *             2：on-hold；
     *             3：pre-dialing;
     *             4：dialing
     * [示例]:
     *     · 查询增强型多方通话与会者的状态
     *       AT^CLCCECONF?
     *       ^CLCCECONF: 5,1,"13800000000","xxx",1
     *       OK
     */
    { AT_CMD_CLCCECONF,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryClccEconfInfo, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CLCCECONF", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 获取增强型多方通话会议的失败原因值
     * [说明]: 用于获取增强型多方通话会议的失败原因值
     * [语法]:
     *     [命令]: ^ECONFERR?
     *     [结果]: <CR><LF>  [^ECONFERR: <number>,<type>,<cause>]
     *             [<CR><LF>  ^ECONFERR: <number>,<type>,<cause>]
     *             [...]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^ECONFERR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <cause>: 原因值，整型值。
     * [示例]:
     *     · 查询增强型多方通话失败原因值
     *       AT^ECONFERR?
     *       ^ECONFERR: "13800000001",129,130
     *       ^ECONFERR: "13800000002",129,130
     *       OK
     */
    { AT_CMD_ECONFERR,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryEconfErrPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECONFERR", VOS_NULL_PTR },

    /* 添加了class域支持VIDEO */
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: IMS呼叫等待
     * [说明]: 该命令用于激活或者去激活IMS呼叫等待。
     * [语法]:
     *     [命令]: ^CCWAI=<mode>,<service_class>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CCWAI?
     *     [结果]: <CR><LF>^CCWAI: <mode>,<ccwa_ctrl_mode><CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CCWAI=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，控制模式，取值0~1。
     *             0：去激活；
     *             1：激活。
     *     <service_class>: 整型值，服务类型，取值1~2。
     *             1：语音。
     *             2：视频。
     *             3：通知modem CS域呼叫等待由网络控制，本地不做控制
     *     <ccwa_ctrl_mode>: 整型值，呼叫等待的控制模式，取值0~1。
     *             0：呼叫等待由3gpp网络控制；
     *             1：呼叫等待由UE控制，即终端侧方案
     * [示例]:
     *     · 激活IMS呼叫等待
     *       AT^CCWAI=1,1
     *       OK
     *     · 去激活IMS呼叫等待
     *       AT^CCWAI=0,1
     *       OK
     */
    { AT_CMD_CCWAI,
      AT_SetCcwaiPara, AT_SET_PARA_TIME, AT_QryCcwaiPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CCWAI", (VOS_UINT8 *)"(0,1),(1,2,3)" },
#endif

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 列出当前所有的呼叫信息
     * [说明]: 列出当前CS/IMS的呼叫信息。
     * [语法]:
     *     [命令]: ^CLCC?
     *     [结果]: [<CR><LF>^CLCC: <id1>,<dir>,<stat>,<mode>,<mpty>,<voice_domain>,<call_type>,<isEconference>[,<number>,<type>[,<display_name> [,<terminal_video_support>[,<imsDomain> [,<rtt>,<rtt_chan_id>,<cps>,<isEncrypt>]]]]]
     *             [<CR><LF>^CLCC: <id2>,<dir>,<stat>,<mode>,<mpty><voice_domain>,<call_type>,<isEconference>[,<number>,<type>[,<display_name> [,<terminal_video_support>[,<imsDomain>[,<rtt>,<rtt_chan_id>[,<cps>,<isEncrypt>]]]]]][...]]]<CR><LF>]
     *             <CR><LF>OK<CR><LF>
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CLCC=?
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
     *     <voice_domain>: 整型值，标识服务域。
     *             0：CS域电话
     *             1：IMS域电话
     *     <call_type>: 整型值，呼叫类型。
     *             0：语音呼叫
     *             1：视频通话：单向发送视频，双向语音
     *             2：视频通话：单向接收视频，双向语音
     *             3：视频通话：双向视频，双向语音
     *             9：紧急呼叫。
     *     <isEconference>: 整型值， 会议类型。
     *             0：不是增强型多方通话
     *             1：是IMS域下增强型多方通话
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值，其结构如表2-6所示。其中：
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值包括：
     *             Type-of-number（bit[6:4]），取值如下：
     *             000：用户不了解目的地址号码时，选用此值，此时目的地址号码由网络填写。
     *             001：若用户能识别是国际号码时，选用此值；或者认为是国内范围也可以填写。
     *             010：国内号码，不允许加前缀或者后缀。在用户发送国内电话时，选用此值。
     *             011：本网络内的特定号码，用于管理或者服务，用户不能选用此值。
     *             101：号码类型为GSM的缺省7bit编码方式。
     *             110：短号码。
     *             111：扩展保留。
     *             Numbering-plan-identification（bit[3:0]），取值如下：
     *             0000：号码由网络的号码方案确定。
     *             0001：ISDN/telephone numbering plan。
     *             0011：Data numbering plan。
     *             0100：Telex numbering plan。
     *             1000：National numbering plan。
     *             1001：Private numbering plan。
     *             1010：ERMES numbering plan。
     *             说明：当bit[6:4]取值为000、001、010时bit[3:0]才有效。
     *     <display_name>: 需要显示的名字。
     *     <terminal_video_support>: 整型值，对端是否支持视屏能力项：
     *             0：不支持；
     *             1：支持。
     *     <imsDomain>: 整型值，IMS电话是在VOLTE，VONR上还是在VOWIFI上：
     *             0：IMS电话在VOLTE上
     *             1：IMS电话在VOWIFI上
     *             2：IMS电话在VONR上
     *     <rtt>: 整型值，指示当前这路通话是否为RTT通话：
     *             0：非RTT通话；
     *             1：RTT通话；
     *             2：远端为TTY通话；
     *             3：远端不支持RTT。
     *     <rtt_chan_id>: 整型值，指示当前这路通话用户面使用的通道号，<rtt>参数为1时值有意义，-1为无效值。
     *     <cps>: 整型值，每秒钟允许发送的字符数，0表示无效。
     *     <isEncrypt>: 整型值，语音加密状态：
     *             0：非语音加密
     *             1：语音加密
     * [表]: 号码地址类型的结构
     *       bit:   7,    6,              5,              4,              3,                             2,                             1,                             0,
     *              1,    Type-of-number, Type-of-number, Type-of-number, Numbering-plan-identification, Numbering-plan-identification, Numbering-plan-identification, Numbering-plan-identification,
     * [示例]:
     *     · 显示当前存在IMS域呼叫信息
     *       AT^CLCC?
     *       ^CLCC: 1,0,0,0,0,1,0,0,"13810000000",129,,,,0,0,0
     *       ^CLCC: 2,0,1,0,0,1,0,0,"13800000000",129,,,,0,0,0
     *       OK
     *     · 测试CLCC
     *       AT^CLCC=?
     *       OK
     */
    { AT_CMD_CLCC_IMS,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryClccPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CLCC", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 挂断来电或等待中的电话
     * [说明]: 该命令用于挂断来电或等待中的电话并指定挂断的原因值。
     *         软银定制需求新增命令。
     * [语法]:
     *     [命令]: ^REJCALL=<id>,<cause>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^REJCALL=?
     *     [结果]: <CR><LF>^REJCALL:  (list of supported <cause>s) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <id>: 整型值，呼叫标识，取值范围1~7。
     *             说明
     *             与+CLCC或^CLCC命令查询结果中的<id>一致。
     *     <cause>: 整型值，挂断原因值：
     *             0：用户忙，CS域下对应#17 User busy，IMS域下对应486 Busy Here；
     *             1：呼叫拒绝，CS域下对应#21 Call rejected，IMS域下对应603 Decline。
     * [示例]:
     *     · 拒绝来电，原因值用户忙
     *       AT^REJCALL=1,0
     *       OK
     *     · 执行测试命令
     *       AT^REJCALL=?
     *       ^REJCALL: (0,1)
     *       OK
     */
    { AT_CMD_REJCALL,
      AT_SetRejCallPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestRejCallPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^REJCALL", (VOS_UINT8 *)"(1-7),(0,1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询通话过程中语音带宽信息
     * [说明]: 该命令用于通话过程中查询语音信道的带宽信息。
     * [语法]:
     *     [命令]: ^CSCHANNELINFO?
     *     [结果]: 执行成功时：
     *             <CR><LF>^ CSCHANNELINFO: <status>,<voice_domain><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF> ERROR <CR><LF>
     *     [命令]: ^CSCHANNELINFO=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <status>: 整型值，语音通道类型：
     *             0：无带内音信息；
     *             1：带内音可用，窄带语音；
     *             2：带内音可用，宽带语音；
     *             3：带内音可用，EVS NB语音；
     *             4：带内音可用，EVS WB语音；
     *             5：带内音可用，EVS SWB语音。
     *     <voice_domain>: 整型值，语音电话的呼叫域：
     *             0：3GPP CS域语音电话；
     *             1：IMS域语音电话；
     *             2：3GPP2 CS域语音电话。
     * [示例]:
     *     · 执行查询命令，CS域（GSM）有语音通话，窄带语音
     *       AT^CSCHANNELINFO?
     *       ^CSCHANNELINFO: 1,0
     *       OK
     *     · 执行查询命令，当前不存在语音通话
     *       AT^CSCHANNELINFO?
     *       ERROR
     *     · 执行测试命令
     *       AT^CSCHANNELINFO=?
     *       OK
     */
    { AT_CMD_CSCHANNELINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCsChannelInfoPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CSCHANNELINFO", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 紧急呼回呼模式状态查询
     * [说明]: EMCCBM，该命令用于查询当前是否处于紧急呼的呼叫回呼模式。
     * [语法]:
     *     [命令]: ^EMCCBM?
     *     [结果]: <CR><LF>^ EMCCBM: n<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <n>: 紧急呼叫回呼模式状态：
     *             1：当前处于回呼模式中；
     *             0：当前不处于回呼模式中；
     * [示例]:
     *     · CALLBACK状态查询
     *       AT^EMCCBM?
     *       ^ EMCCBM: 1
     *       OK
     */
    { AT_CMD_EMCCBM,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryEmcCallBack, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^EMCCBM", VOS_NULL_PTR },

#if (FEATURE_ECALL == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 会话ECALL发起
     * [说明]: 设置命令用于发起ECALL会话。
     *         测试命令用于获取当前支持的ECALL能力。
     *         说明：本命令仅用于Balong V722C60/C70, V711C60, V765C60。
     * [语法]:
     *     [命令]: ^ECLSTART=<activation_type>,
     *             <type_of_call>[,<dial_num>[,<oprt_mode>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^ECLSTART=?
     *     [结果]: <CR><LF>^ECLSTART: (list of supported <activation_type>s), (list of supported <type_of_call>s) ,(list of supported <oprt_mode>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <activation_type>: 整型值，表示发起ECALL会话的触发条件
     *             0：用户发起ECALL会话
     *             1：自动发起ECALL会话
     *             只有紧急呼叫关注该值，测试会话不关注该值。
     *     <type_of_call>: 整型值，表示当前发起的ECALL会话类型
     *             0：发起测试会话
     *             1：发起紧急呼叫
     *             2：发起Reconfiguration呼叫
     *     <dial_num>: 被叫电话号码，ASCII字符串。
     *             合法的字符仅包括：‘0’ – ‘9’, ‘*’, ‘#’, ‘+’。‘+’只能出现在号码的最前面，号码的最大长度按照3GPP TS 24.008 10.5.4.7定义。
     *             发起测试呼叫时，应该要带电话号码，如果没带号码，则从SIM卡的EEfdn中获取；发起紧急呼叫时，不将号码发送到网络侧，号码参数没有实际意义，可以提供，也可以不提供。发起Reconfiguration呼叫时，应该要带电话号码，如果没带号码，则从SIM卡的EFsdn中获取。
     *     <oprt_mode>: 整数值，表示启动eCall后的最小数据集（MSD）传输模式。缺省模式为0。
     *             0：PULL Mode
     *             1：PUSH Mode
     * [示例]:
     *     · 手动发起紧急呼叫，模式为PUSH模式
     *       AT^ECLSTART=0,1,,1
     *       OK
     *     · 发起测试呼叫，模式为PULL模式
     *       AT^ECLSTART=0,0,"150xxxx2856",0
     *       OK
     *     · 发起Reconfiguration呼叫，模式为PUSH模式
     *       AT^ECLSTART=0,2,"150xxxx2856",1
     *       OK
     *     · 测试命令
     *       AT^ECLSTART=?
     *       ^ECLSTART: (0,1),(0,1,2),(0,1)
     *       OK
     */
    { AT_CMD_ECLSTART,
      AT_SetEclstartPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestEclstartPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLSTART", (VOS_UINT8 *)"(0,1),(0-2),(PNUMBER),(0,1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: ECALL会话断开
     * [说明]: 此命令用于断开当前ECALL会话
     *         若当前没有eCall，下发此命令返回OK。
     *         说明：本命令仅用于Balong V722C60/C70, V711C60, V765C60。
     * [语法]:
     *     [命令]: ^ECLSTOP
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     * [示例]:
     *     · 断开当前ECALL会话
     *       AT^ECLSTOP
     *       OK
     */
    { AT_CMD_ECLSTOP,
      AT_SetEclstopPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLSTOP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: ECALL功能参数配置
     * [说明]: 设置命令用于配置单板获取MSD数据的模式，是否禁止语音功能，是否开启自动重拨功能。
     *         查询命令用于获取当前的设置信息。
     *         测试命令用于获取各个参数支持的取值范围。
     *         说明：本命令仅用于Balong V722C60/C70, V711C60, V765C60。
     * [语法]:
     *     [命令]: ^ECLCFG=<mode>[,<voc_config>][,<redial_config>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^ECLCFG?
     *     [结果]: <CR><LF>^ECLCFG: <mode>,<voc_config><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^ECLCFG=?
     *     [结果]: <CR><LF>^ECLCFG: (list of supported <mode>s),(list of supported <voc_config>s) ,(list of supported <redial_config>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，表示单板获取MSD数据的模式，目前只支持透传模式。
     *             0：透传模式
     *     <voc_config>: 整型值，表示在MSD传输过程中是否禁止语音数据的传输，参数缺省情况默认上一次的配置。
     *             0：在MSD传输过程中同步传输语音信号，默认值。
     *             1：在MSD传输过程中禁止语音信号的传输，避免语音信号的传输干扰MSD数据的传输。
     *     <redial_config>: 整数值，指示启用或者禁用自动重拨功能，目前不支持启动自动重拨功能。
     *             0：禁用自动重拨功能。
     *             1：启用自动重拨功能。（当前不支持）
     * [示例]:
     *     · 配置单板获取MSD数据的模式为透传模式，并且禁止语音功能
     *       AT^ECLCFG=0,1,0
     *       OK
     *     · 查询命令
     *       AT^ECLCFG?
     *       ^ECLCFG: 0,0
     *       OK
     *     · 测试命令
     *       AT^ECLCFG=?
     *       ^ECLCFG: (0),(0-1),(0-1)
     *       OK
     */
    { AT_CMD_ECLCFG,
      AT_SetEclcfgPara, AT_SET_PARA_TIME, AT_QryEclcfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLCFG", (VOS_UINT8 *)"(0),(0-1),(0-1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: MSD数据透传设置
     * [说明]: 设置命令用于下发当前ECALL会话需要传输的MSD信息。
     *         查询命令用于获取MT当前保存的最新的MSD信息。
     *         说明：本命令仅用于Balong V722C60/C70, V711C60, V765C60。
     * [语法]:
     *     [命令]: ^ECLMSD=<ecall_msd>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^ECLMSD?
     *     [结果]: <CR><LF>^ECLMSD: <ecall_msd><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <ecall_msd>: 十六进制文本字符串，采用十六进制数据编码方式，字符串长度为280个字节，表示的是MSD协议要求的140个字节原始数据。具体MSD数据编码格式请参考MSD协议。
     * [示例]:
     *     · 下发当前ECALL会话需要传输的MSD信息
     *       AT^ECLMSD="
     *       0101084141414141414141414141313131313131880000000100000001000000010100010001000100010600010db885a308d313198a2e03707334000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
     *       OK
     *     · 查询命令
     *       AT^ECLMSD?
     *       ^ECLMSD:
     *       "0101084141414141414141414141313131313131880000000100000001000000010100010001000100010600010db885a308d313198a2e03707334000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
     *       OK
     */
    { AT_CMD_ECLMSD,
      AT_SetEclmsdPara, AT_SET_PARA_TIME, AT_QryEclmsdPara, AT_QRY_PARA_TIME, AT_TestEclmsdPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLMSD", (VOS_UINT8 *)"(MSD)" },

    { AT_CMD_ECLABORT,
      AT_SetEclAbortPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLABORT", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: eCall AL_ACK信息列表查询命令
     * [说明]: 本命令用于查询PSAP发送的最后20条AL_ACK消息和接收的时间戳记录列表。如果列表为空，则返回OK。如果列表中保存的记录超过20条，最新记录将替换最早的记录。
     * [语法]:
     *     [命令]: ^ECLLIST?
     *     [结果]: <CR><LF>^ECLLIST: <timestamp>,<AL_ACK>[
     *             <CR><LF>^ECLLIST: <timestamp>,<AL_ACK>[...]]
     *             <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <timestamp>: 字符串类型，指示IVS接收PSAP发送的AL_ACK的时间。
     *             格式为：
     *             “YYYY/MM/DD,HH:MM:SS+ZZ”
     *             YYYY的取值范围从2000到2100。
     *     <AL_ACK>: 整数值，当PSAP接收并成功验证MSD数据时，PSAP将AL_ACK发送到IVS，带有Clear Down标志的AL_ACK要求IVS挂断eCall。（参考协议16062  7.5.5）
     *             AL_ACK由4 Bit组成，其中Bit3和Bit4为保留位
     *             Bit1为Format version，取值如下：
     *             0：Format version0；
     *             1：Format version1。
     *             Bit2为Status，取值如下：
     *             0：MSD数据被认证；
     *             1：MSD数据被认证，而且要求IVS挂断eCall。
     * [示例]:
     *     · AL_ACK信息列表查询
     *       AT^ECLLIST?
     *       ^ECLLIST: "2018/11/28,03:18:16+32",0
     *       OK
     */
    { AT_CMD_ECLLIST,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryEclListPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLLIST", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: eCall 工作模式配置命令
     * [说明]: 设置命令用于用于设置eCall工作模式，其中eCall only模式下不主动注册网络，处于eCall inactive状态，发起eCall会触发注网，Test eCall和Reconfiguration eCall的号码从EFFDN获取。eCall and normal模式正常注册网络，Test eCall和Reconfiguration eCall的号码从EFSDN获取。注意需要重启或SIM重启生效。
     *         查询命令用于查询当前eCall工作模式。
     *         测试命令用于获取各个参数支持的取值范围。
     * [语法]:
     *     [命令]: ^ECLMODE=<mode>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^ECLMODE?
     *     [结果]: <CR><LF>^ECLMODE: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^ECLMODE=?
     *     [结果]: <CR><LF>^ECLMODE: (list of supported <mode>s)
     *             <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，表示设置eCall工作模式。
     *             0：Auto模式，UE根据SIM文件来决定运行模式，根据3GPP协议TS31.102，如果在EFUST中eCall data (Service n°89) 和FDN (Service n°2) 开启，并且EFEST中FDN（Service n°1）开启，则以eCall only模式运行，如果在EFUST中eCall data (Service n°89) 和SDN (Service n°4) 开启，则以eCall and normal模式运行，其他情况按照normal模式运行；
     *             1：Force eCall Only模式，无论是插入的是哪种SIM卡，UE都以eCall only模式来运行；
     *             2：Force eCall and normal模式，无论是插入的是哪种SIM卡，UE都以eCall and normal模式来运行；
     * [示例]:
     *     · 设置eCall工作模式为Fore eCall only
     *       AT^ECLMODE=1
     *       OK
     */
    { AT_CMD_ECLMODE,
      AT_SetEclModePara, AT_SET_PARA_TIME, AT_QryEclModePara, AT_QRY_PARA_TIME, AT_TestEclModePara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLMODE", (VOS_UINT8 *)"(0-2)" },

#endif
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: MT呼叫可信名单控制命令
     * [说明]: 用于设置/查询被叫的呼叫可信名单。
     *         当参数中索引和呼叫号码字段都不为空时（呼叫号码字段需要加引号），该命令用于将索引指定的号码写入呼叫可信名单；如果只有一个参数，且索引字段不为空，则该命令读取呼叫可信名单中索引指定的主叫号码。
     *         当开启MT呼叫可信名单时，UE需要过滤呼叫可信名单，只有存在于呼叫可信名单中的号码才能被存储和上报。
     *         如果MT呼叫可信名单已开启，但呼叫可信名单为空，则所有呼叫都无法接入。
     *         该功能FEATURE_MBB_CUST宏开启时生效。
     * [语法]:
     *     [命令]: ^TRUSTCNUM=<index>[,<number>]
     *     [结果]: 在设置的情况下：
     *             <CR><LF>OK<CR><LF>
     *             在查询的情况下：
     *             [<CR><LF>^TRUSTCNUM: <index>,<number><CR><LF>]<CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TRUSTCNUM?
     *     [结果]: [<CR><LF>^TRUSTCNUM: <index>,<number><CR><LF>^TRUSTCNUM: <index>,<number>[...]<CR><LF>]<CR><LF>OK<CR><LF>
     *     [命令]: ^TRUSTCNUM=?
     *     [结果]: <CR><LF>^TRUSTCNUM: (list of supported <index>s)<CR><LF><CR><LF>OK<CR><LR>
     * [参数]:
     *     <index>: 整数值。取值范围：0~19。
     * [示例]:
     *     · 将索引为0的号码“1111111111”添加到呼叫可信名单中：
     *       AT^TRUSTCNUM=0,"1111111111"
     *       OK
     *     · 读取所有呼叫可信名单号码：
     *       AT^TRUSTCNUM?
     *       ^TRUSTCNUM: 1,"1111111111"
     *       ^TRUSTCNUM: 2,"2222222222"
     *       ^TRUSTCNUM: 3,"3333333333"
     *       OK
     *     · 查询支持的索引列表：
     *       AT^TRUSTCNUM=?
     *       ^TRUSTCNUM: (0-19)
     *       OK
     */
    { AT_CMD_TRUSTCNUM,
      AT_SetTrustCallNumPara, AT_NOT_SET_TIME, AT_QryTrustCallNumPara, AT_NOT_SET_TIME, AT_TestCallTrustNumPara,
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TRUSTCNUM", (VOS_UINT8 *)"(0-19),(number)" },
};

/* 注册taf呼叫AT命令表 */
VOS_UINT32 AT_RegisterVoiceTafCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atVoiceTafCmdTbl, sizeof(g_atVoiceTafCmdTbl) / sizeof(g_atVoiceTafCmdTbl[0]));
}
