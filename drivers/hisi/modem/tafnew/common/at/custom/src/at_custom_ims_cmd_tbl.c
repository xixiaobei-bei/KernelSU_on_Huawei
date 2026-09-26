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
#include "at_custom_ims_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_ims_set_cmd_proc.h"
#include "at_custom_ims_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_IMS_CMD_TBL_C

static const AT_ParCmdElement g_atCustomImsCmdTbl[] = {
#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: IMS短信业务能力开关
     * [说明]: 通过这个命令可以打开和关闭IMS在各接入技术下的短信收发能力。
     *         AP通过此命令打开IMS域的短信能力时，需要同时打开IMS在对应接入技术下的支持能力，以便在该接入技术下正常收发IMS短信。
     * [语法]:
     *     [命令]: ^IMSSMSCFG=<wifi_enable>,<lte_enable>,<utran_enable>,<gsm_enable>
     *     [结果]: 设置执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置执行错误情况：
     *             短信业务过程中下发此命令返回失败
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IMSSMSCFG?
     *     [结果]: <CR><LF>^IMSSMSCFG: <wifi_enable>,<lte_enable>,<utran_enable>,<gsm_enable><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^IMSSMSCFG=?
     *     [结果]: <CR><LF>^ IMSSMSCFG: (list of supported <wifi_enable>s), (list of supported <lte_enable>s),  (list of supported <utran_enable>s), (list of supported  <gsm_enable>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <wifi_enable>: 整型值， IMS在WIFI下短信功能的开关状态。
     *             0：关闭WIFI下IMS短信收发功能；
     *             1：打开WIFI下IMS短信收发功能。
     *     <lte_enable>: 整型值， IMS在LTE下短信功能的开关状态。
     *             0：关闭LTE下IMS短信收发功能；
     *             1：打开LTE下IMS短信收发功能。
     *     <utran_enable>: 整型值， IMS在Utran下短信功能的开关状态。
     *             0：关闭Utran下IMS短信收发功能；
     *             1：打开Utran下IMS短信收发功能。
     *     <gsm_enable>: 整型值， IMS在Gsm下短信功能的开关状态。
     *             0：关闭Gsm下IMS短信收发功能；
     *             1：打开Gsm下IMS短信收发功能。
     * [示例]:
     *     · 打开WIFI和LTE域的IMS短信收发功能
     *       AT^IMSSMSCFG=1,1,0,0
     *       OK
     *     · 查询IMS短信能力配置
     *       AT^IMSSMSCFG?
     *       ^IMSSMSCFG: 1,1,0,0
     *       OK
     *     · 测试命令
     *       AT^IMSSMSCFG=?
     *       ^IMSSMSCFG: (0-1),(0-1),(0-1),(0-1)
     *       OK
     */
    { AT_CMD_IMSSMSCFG,
      AT_SetImsSmsCfgPara, AT_SET_PARA_TIME, AT_QryImsSmsCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSSMSCFG", (VOS_UINT8 *)"(0,1),(0,1),(0,1),(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 控制IMS注册流程错误原因值上报
     * [说明]: 控制IMS注册失败时，错误原因值是否通过AT命令主动上报；
     * [语法]:
     *     [命令]: ^IMSREGERRRPT=<value>
     *     [结果]: 设置执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置执行错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^IMSREGERRRPT?
     *     [结果]: <CR><LF>^IMSREGERRRPT: <value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^IMSREGERRRPT=?
     *     [结果]: <CR><LF>^IMSREGERRRPT: (list of supported <value >s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值，是否主动上报IMS注册失败原因值，默认值0。
     *             0：关闭主动上报IMS注册失败原因值功能
     *             1：打开主动上报IMS注册失败原因值功能
     * [示例]:
     *     · 打开主动上报IMS注册失败原因值功能
     *       AT^IMSREGERRRPT=1
     *       OK
     *     · 查询主动上报IMS注册失败原因值功能是否打开
     *       AT^IMSREGERRRPT?
     *       ^IMSREGERRRPT: 0
     *       OK
     *     · 查询主动上报IMS注册失败原因值功能存在的控制类型
     *       AT^IMSREGERRRPT=?
     *       ^IMSREGERRRPT: (0-1)
     *       OK
     */
    { AT_CMD_IMSREGERRRPT,
      AT_SetImsRegErrRpt, AT_SET_PARA_TIME, AT_QryImsRegErrRpt, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMSREGERRRPT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询最后一次IMS域呼叫错误码
     * [说明]: 查询最后一次IMS域上报的呼叫错误原因值和文本信息。
     * [语法]:
     *     [命令]: ^CIMSERR?
     *     [结果]: <CR><LF>^CIMSERR: <cause>,<text><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CIMSERR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cause>: 整型值，详见“4.5 CS域错误码”。
     *     <text>: 错误码的文本信息
     * [示例]:
     *     · IMS域呼叫失败，查询原因值
     *       AT^CIMSERR?
     *       ^CIMSERR: 18737,"USE PROXY"
     *       OK
     *     · 测试命令
     *       AT^CIMSERR=?
     *       OK
     */
    { AT_CMD_CIMSERR,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCimsErrPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CIMSERR", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置IMS服务状态更新信息上报
     * [说明]: 本命令用于设置IMS服务状态更新信息上报配置。打开和关闭^IMSSRVSTATUS命令的主动上报。
     * [语法]:
     *     [命令]: ^IMSSRVSTATRPT=<enable>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR:  <err_code><CR><LF>
     *     [命令]: ^IMSSRVSTATRPT?
     *     [结果]: <CR><LF>^IMSSRVSTATRPT: <enable><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^IMSSRVSTATRPT=?
     *     [结果]: <CR><LF>^IMSSRVSTATRPT: (list of supported <enable>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，主动上报控制开关：
     *             0：关闭主动上报；
     *             1：开启主动上报。
     * [示例]:
     *     · 开启IMS服务状态更新信息上报
     *       AT^IMSSRVSTATRPT=1
     *       OK
     *     · 查询IMS服务状态更新信息上报配置
     *       AT^IMSSRVSTATRPT?
     *       ^IMSSRVSTATRPT: 1
     *       OK
     */
    { AT_CMD_IMSSRVSTATRPT,
      AT_SetImsSrvStatRptCfgPara, AT_SET_PARA_TIME, AT_QryImsSrvStatRptCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMSSRVSTATRPT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询IMS服务状态
     * [说明]: 查询IMS服务状态或者注册状态。
     * [语法]:
     *     [命令]: ^IMSSRVSTATUS?
     *     [结果]: <CR><LF>^IMSSRVSTATUS: <sms_srv_status>,<sms_srv_rat>,<voip_srv_status>,<voip_srv_rat>,<vt_srv_status>,<vt_srv_rat>,<vs_srv_status>,<vs_srv_rat><CR><LF>
     * [参数]:
     *     <sms_srv_status>: 整型值，IMS短信的服务状态：
     *             0： IMS SMS service is not available
     *             1： IMS SMS is in limited service
     *             2： IMS SMS is in full service
     *     <sms_srv_rat>: 整型值，IMS短信的服务域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2： IMS service is registered on Utran
     *             3： IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <voip_srv_status>: 整型值，IMS VoIP的服务状态：
     *             0： IMS VoIP service is not available
     *             1： IMS VoIP is in limited service
     *             2： IMS VoIP is in full service
     *     <voip_srv_rat>: 整型值，IMS VoIP的服务域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered on Utran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <vt_srv_status>: 整型值，IMS VT的服务状态：
     *             0： IMS VT service is not available
     *             1： IMS VT is in limited service
     *             2： IMS VT is in full service
     *     <vt_srv_rat>: 整型值，IMS VT的服务状态：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered on Utran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     *     <vs_srv_status>: 整型值，IMS VS的服务状态：
     *             0： IMS VS service is not available
     *             1： IMS VS is in limited service
     *             2： IMS VS is in full service
     *     <vs_srv_rat>: 整型值，IMS VS的服务域：
     *             0：IMS service is registered on LTE
     *             1：IMS service is registered on WIFI
     *             2：IMS service is registered on Utran
     *             3：IMS service is registered on Gsm
     *             4：IMS service is registered on NR
     * [示例]:
     *     · 查询IMS的服务状态
     *       AT^IMSSRVSTATUS?
     *       ^IMSSRVSTATUS: 2,0,2,0,2,0,2,0
     *       OK
     */
    { AT_CMD_IMSSRVSTATUS,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryImsSrvStatusPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMSSRVSTATUS", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置WIFI紧急服务 Address-ID
     * [说明]: 设置WIFI紧急服务 Address-ID。
     * [语法]:
     *     [命令]: ^WIEMCAID=<Address_ID>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^WIEMCAID=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Address_ID>: 字符串。最大长度是256个字符。
     * [示例]:
     *     · 设置WIFI紧急服务 Address-ID
     *       AT^WIEMCAID="fsfs"
     *       OK
     */
    { AT_CMD_WIEMCAID,
      AT_SetEmcAIdPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^WIEMCAID", (VOS_UINT8 *)"(str)" },
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: RCS DM参数配置命令
     * [说明]: 该命令用于配置RCS的DM参数。
     * [语法]:
     *     [命令]: ^DMRCSCFG=<featuretag>,<devcfg>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^DMRCSCFG=?
     *     [结果]: <CR><LF>^DMRCSCFG: (0-4294967295),(0-4294967295)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <featuretag>: 32位整型值，RCS能力信息，发生变化后需要发起刷新注册
     *     <devcfg>: 32位整型值，RCS配置信息，发生变化后需要发起刷新注册
     * [示例]:
     *     · 设置RCS DM参数成功：
     *       AT^DMRCSCFG=1024,26
     *       OK
     *     · 测试命令
     *       AT^DMRCSCFG=?
     *       ^DMRCSCFG: (0-4294967295),(0-4294967295)
     *       OK
     */
    { AT_CMD_DMRCSCFG,
      AT_SetDmRcsCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DMRCSCFG", (VOS_UINT8 *)"(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: User Agent 信息配置命令
     * [说明]: 该命令用于配置User Agent相关信息。
     * [语法]:
     *     [命令]: ^USERAGENTCFG=<para1>,<para2>,<para3>,<para4>,<para5>,<para6>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^USERAGENTCFG=?
     *     [结果]: <CR><LF>^USERAGENTCFG: (str),(str),(str),(str),(str),(str)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <para1>: 字符串类型，字符串最大长度为16字节。
     *             User Agent信息参数1；
     *     <para2>: 字符串类型，字符串最大长度为16字节。
     *             User Agent信息参数2；
     *     <para3>: 字符串类型，字符串最大长度为16字节。
     *             User Agent信息参数3；
     *     <para4>: 字符串类型，字符串最大长度为16字节。
     *             User Agent信息参数4；
     *     <para5>: 字符串类型，字符串最大长度为16字节。
     *             User Agent信息参数5；
     *     <para6>: 字符串类型，字符串最大长度为16字节。
     *             User Agent信息参数6；
     * [示例]:
     *     · 设置UA版本信息成功：
     *       AT^USERAGENTCFG="VKY-L09","C50B100","5.1","7.0",,
     *       OK
     *     · 测试命令
     *       AT^USERAGENTCFG=?
     *       ^USERAGENTCFG: (str),(str),(str),(str),(str),(str)
     *       OK
     */
    { AT_CMD_USERAGENTCFG,
      AT_SetUserAgentCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^USERAGENTCFG", (VOS_UINT8 *)"(str),(str),(str),(str),(str),(str)" },

    { AT_CMD_SIPPORT,
      At_SetSipTransPort, AT_SET_PARA_TIME, At_QrySipTransPort, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^TCPTHRESHOLD", (TAF_UINT8 *)"(400-16500)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 通话补包控制命令
     * [说明]: 此命令用于透传FUSIONCALL给IMSA和IMS SDK的命令请求消息内容。该消息主要是用于通话补包业务控制。
     * [语法]:
     *     [命令]: ^FUSIONCALLRAW=<msg_id>,<msg_len>,<msg_context>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <msg_id>: 对应接收模块
     *             0：IMSA
     *             1：IMS SDK
     *     <msg_len>: 消息内容<msg_context>的长度，消息长度最大不超过500
     *     <msg_context>: 模块间接口消息码流
     * [示例]:
     *     · AP侧请求启动通话补包业务
     *       AT^FUSIONCALLRAW=0,12,"010000000000000001000000"
     *       OK
     */
    { AT_CMD_FUSIONCALLRAW,
      AT_SetFusionCallRawPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^FUSIONCALLRAW", (VOS_UINT8 *)"(0,1),(1-280),(msg)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询卡的鉴权用户名
     * [说明]: 查询卡的IMPI（鉴权用户名）。
     * [语法]:
     *     [命令]: ^VOLTEIMPI
     *     [结果]: <CR><LF>^VOLTEIMPI: <impi><CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^VOLTEIMPI?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^VOLTEIMPI=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <impi>: 鉴权用户名称，长度不大于128的字符串。
     * [示例]:
     *     · 查询IMPI
     *       AT^VOLTEIMPI
     *       ^VOLTEIMPI: <target>tel:1234;phone-context=ims.mnc000.mcc460.3gppnetwork.org</target>
     *       OK
     */
    { AT_CMD_VOLTEIMPI,
      AT_SetVolteImpiPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^VOLTEIMPI", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询卡的归属网络域名
     * [说明]: 查询卡的Home Network Domain Name（归属网络域名）。
     * [语法]:
     *     [命令]: ^VOLTEDOMAIN
     *     [结果]: <CR><LF>^VOLTEDOMAIN: <domain><CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^VOLTEDOMAIN?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^VOLTEDOMAIN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <domain>: 归属网络域名(Home Network Doamin Name)，长度不大于128的字符串。
     * [示例]:
     *     · 查询Home Network Domain Name
     *       AT^VOLTEDOMAIN
     *       ^VOLTEDOMAIN: 3gppnetwork.org
     *       OK
     */
    { AT_CMD_VOLTEDOMAIN,
      AT_SetVolteDomainPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^VOLTEDOMAIN", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: IMS EMC PDN动态参数查询
     * [说明]: 该命令用于查询IMS EMC PDN动态参数，包括IP类型、IP地址、DNS地址和MTU。
     *         说明：用于IMS紧急呼叫定位特性。
     * [语法]:
     *     [命令]: ^IMSEMCRDP?
     *     [结果]: <CR><LF>^IMSEMCRDP: <ip_type>,<ip_addr>
     *             ,<prim_dns>,<sec_dns>,<mtu><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：<CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IMSEMCRDP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <ip_type>: 字符串类型，分组数据协议类型。   IPV4：IPV4 协议；
     *             IPV6：IPV6 协议。
     *     <ip_addr>: 字符串类型，IP地址。
     *     <prim_dns>: 字符串类型，Primary DNS地址。
     *     <sec_dns>: 字符串类型，Secondary DNS地址。
     *     <mtu>: 整型值，MTU。
     * [示例]:
     *     · IMS EMC PDN类型为IPV4
     *       AT^IMSEMCRDP?
     *       ^IMSEMCRDP: "IPV4","10.10.1.1","10.11.11.1","10.11.11.2",1280
     *       OK
     *     · IMS EMC PDN类型为IPV6
     *       AT^IMSEMCRDP?
     *       ^IMSEMCRDP: "IPV6","2001::1","2002::1","2002::2",1280
     *       OK
     *     · IMS EMC PDN类型为IPV4V6
     *       AT^IMSEMCRDP?
     *       ^IMSEMCRDP: "IPV4","10.10.1.1","10.11.11.1","10.11.11.2",1280
     *       ^IMSEMCRDP: "IPV6","2001::1","2002::1","2002::2",1280
     *       OK
     *     · 测试命令
     *       AT^IMSEMCRDP=?
     *       OK
     */
    { AT_CMD_IMSEMCRDP,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryImsEmcRdpPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSEMCRDP", VOS_NULL_PTR },
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 发送IMS URSP码流
     * [说明]: 该命令用于AP侧在收到网络侧下发的UE POLICY 信息后，将其中有关IMS的URSP配置信息发送给Modem。
     *         <length >参数要与后续输入的字符串码流的长度相同。
     *         在输入^IMSURSP命令之后才可以输入ims ursp信息（类似于发短信的处理），在ims ursp信息结束后以ctrl-z结尾。
     *         当ims ursp信息字符串长度超过1500，需要分段下发，每段码流使用CR（回车符）结尾，字符串总长度不超过3000。
     * [语法]:
     *     [命令]: ^IMSURSP=<version>,<length>
     *     [结果]: 执行设置成功时：<CR><LF>OK<CR><LF>
     *             错误情况：<CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <version>: AP与IMSA约定的码流解析版本，当前仅支持取值1
     *     <length>: 整型值，IMS URSP字符串长度，取值范围（0-3000）
     * [示例]:
     *     · AP下发IMS URSP信息
     *       AT^IMSURSP=1,20
     *       >0123456789abcdef0123
     *       >ctrl-z
     *       OK
     *       AT^IMSURSP=1,20
     *       >0123456789
     *       >abcdef0123
     *       >ctrl-z
     *       OK
     *     · AP下发删除IMS URSP信息
     *       AT^IMSURSP=1,0
     *       OK
     */
    { AT_CMD_IMSURSP,
      AT_SetImsUrsp, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMSURSP", (VOS_UINT8 *)"(0-255),(0-3000)" },
#endif

#if (FEATURE_ECALL == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 定制IMS ECALL配置命令
     * [说明]: 该命令用于定制设置ecall over ims优选域和定制SIP消息中的Content Type值。
     * [语法]:
     *     [命令]: ^ECLIMSCFG=<n>[,<ContentType>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^ECLIMSCFG?
     *     [结果]: <CR><LF>^ECLIMSCFG: <n>[,<Content Type>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 0 – ecall over ims优选域强制cs prefer；
     *             1 - 定制ECALL OVER IMS 模式，优选域ps prefer，必须同时设置content type;
     *             2 - 标准ecall over ims 模式，优选域ps prefer，不需要设置content type。
     *     <[<Content Type>]>: 可选项，字符串类型，代表用户定制的Content type 字符串，用于测试。戴姆勒定制Content Type
     * [示例]:
     *     · 设置ECLIMSCFG
     *       AT^ECLIMSCFG=0
     *       OK
     *     · 查询ECLIMSCFG
     *       AT^ECLIMSCFG?
     *       ECLIMSCFG: 0
     *     · 设置ECLIMSCFG
     *       AT^ECLIMSCFG=1,"application/MbCallData.sCall.MSD"
     *       OK
     *     · 查询ECLIMSCFG
     *       AT^ECLIMSCFG?
     *       ECLIMSCFG: 1,"application/MbCallData.sCall.MSD"
     */
    { AT_CMD_ECLIMSCFG,
      AT_SetEclImsCfgPara, AT_SET_PARA_TIME, AT_QryEclImsCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ECLIMSCFG", (VOS_UINT8 *)"(0-2),(content_type)" },
#endif
};

/* 注册IMS定制AT命令表 */
VOS_UINT32 AT_RegisterCustomImsCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomImsCmdTbl, sizeof(g_atCustomImsCmdTbl) / sizeof(g_atCustomImsCmdTbl[0]));
}

