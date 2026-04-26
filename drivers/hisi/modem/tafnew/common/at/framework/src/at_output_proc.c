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

#include "at_output_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "at_input_proc.h"
#include "at_data_proc.h"
#include "at_taf_agent_interface.h"
#include "at_event_report.h"
#include "mn_comm_api.h"
#include "dms_port_i.h"
#include "at_sms_event_report.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_OUTPUTPROC_C

const AT_RETURN_TabType g_atReturnCodeTab[] = {
    { AT_SUCCESS, { TAF_NULL_PTR, TAF_NULL_PTR }},
    { AT_FAILURE, { TAF_NULL_PTR, TAF_NULL_PTR }},
    { AT_WAIT_SMS_INPUT, { (TAF_UINT8 *)"\r\n> ", (TAF_UINT8 *)"\r\n> " }},
    { AT_WAIT_XML_INPUT, { (TAF_UINT8 *)"\r\n> ", (TAF_UINT8 *)"\r\n> " }},
    { AT_WAIT_UE_POLICY_INPUT, { (TAF_UINT8 *)"\r\n> ", (TAF_UINT8 *)"\r\n> " }},
    { AT_WAIT_IMSA_URSP_INPUT, { (TAF_UINT8 *)"\r\n> ", (TAF_UINT8 *)"\r\n> " }},
    { AT_WAIT_ASYNC_RETURN, { TAF_NULL_PTR, TAF_NULL_PTR }},

    { AT_BASIC_CODE_ENUM_BEGAIN, { TAF_NULL_PTR, TAF_NULL_PTR }},

    /* 说明：表示一行命令执行完成，无错误。 */
    { AT_OK, { (TAF_UINT8 *)"0", (TAF_UINT8 *)"OK" }},
    /* 说明：连接已建立。 */
    { AT_CONNECT, { (TAF_UINT8 *)"1", (TAF_UINT8 *)"CONNECT" }},
    /* 说明：来电。 */
    { AT_RING, { (TAF_UINT8 *)"2", (TAF_UINT8 *)"RING" }},
    /* 说明：连接终止。 */
    { AT_NO_CARRIER, { (TAF_UINT8 *)"3", (TAF_UINT8 *)"NO CARRIER" }},
    /* 说明：一般错误。 */
    { AT_ERROR, { (TAF_UINT8 *)"4", (TAF_UINT8 *)"ERROR" }},
    /* 说明：无拨号音。 */
    { AT_NO_DIALTONE, { (TAF_UINT8 *)"6", (TAF_UINT8 *)"NO DIALTONE" }},
    /* 说明：对方电话忙。 */
    { AT_BUSY, { (TAF_UINT8 *)"7", (TAF_UINT8 *)"BUSY" }},
    /* 说明：连接完成超时，无应答。 */
    { AT_NO_ANSWER, { (TAF_UINT8 *)"8", (TAF_UINT8 *)"NO ANSWER" }},
    { AT_BASIC_CODE_ENUM_END, { TAF_NULL_PTR, TAF_NULL_PTR }},
    { AT_CME_ERROR_ENUM_BEGAIN, { TAF_NULL_PTR, TAF_NULL_PTR }},
    /* 说明：电话故障。 */
    { AT_CME_PHONE_FAILURE, { (TAF_UINT8 *)"0", (TAF_UINT8 *)"phone failure" }},
    /* 说明：与电话无连接。 */
    { AT_CME_NO_CONNECTION_TO_PHONE, { (TAF_UINT8 *)"1", (TAF_UINT8 *)"NO CONNECTION TO PHONE" }},
    /* 说明：电话适配链路已被占用。 */
    { AT_CME_PHONE_ADAPTOR_LINK_RESERVED, { (TAF_UINT8 *)"2", (TAF_UINT8 *)"PHONE ADAPTOR LINK RESERVED" }},
    /* 说明：操作不允许。 */
    { AT_CME_OPERATION_NOT_ALLOWED, { (TAF_UINT8 *)"3", (TAF_UINT8 *)"operation not allowed" }},
    /* 说明：操作不支持。 */
    { AT_CME_OPERATION_NOT_SUPPORTED, { (TAF_UINT8 *)"4", (TAF_UINT8 *)"operation not supported" }},
    /* 说明：需要输入PH SIM PIN码。 */
    { AT_CME_PH_SIM_PIN_REQUIRED, { (TAF_UINT8 *)"5", (TAF_UINT8 *)"PH SIM PIN REQUIRED" }},
    /* 说明：需要输入PH-FSIM PIN码。 */
    { AT_CME_PH_FSIM_PIN_REQUIRED, { (TAF_UINT8 *)"6", (TAF_UINT8 *)"PH-FSIM PIN REQUIRED" }},
    /* 说明：需要输入PH-FSIM PUK码。 */
    { AT_CME_PH_FSIM_PUK_REQUIRED, { (TAF_UINT8 *)"7", (TAF_UINT8 *)"PH-FSIM PUK REQUIRED" }},
    /* 说明：没有SIM卡。 */
    { AT_CME_SIM_NOT_INSERTED, { (TAF_UINT8 *)"10", (TAF_UINT8 *)"SIM not inserted" }},
    /* 说明：需要输入SIM PIN码。 */
    { AT_CME_SIM_PIN_REQUIRED, { (TAF_UINT8 *)"11", (TAF_UINT8 *)"SIM PIN required" }},
    /* 说明：需要输入SIM PUK码。 */
    { AT_CME_SIM_PUK_REQUIRED, { (TAF_UINT8 *)"12", (TAF_UINT8 *)"SIM PUK required" }},
    /* 说明：SIM卡失败。 */
    { AT_CME_SIM_FAILURE, { (TAF_UINT8 *)"13", (TAF_UINT8 *)"SIM failure" }},
    /* 说明：SIM卡忙。 */
    { AT_CME_SIM_BUSY, { (TAF_UINT8 *)"14", (TAF_UINT8 *)"SIM busy" }},
    /* 说明：SIM卡错误。 */
    { AT_CME_SIM_WRONG, { (TAF_UINT8 *)"15", (TAF_UINT8 *)"SIM wrong" }},
    /* 说明：密码错误。 */
    { AT_CME_INCORRECT_PASSWORD, { (TAF_UINT8 *)"16", (TAF_UINT8 *)"incorrect password" }},
    /* 说明：需要输入SIM PIN2码。 */
    { AT_CME_SIM_PIN2_REQUIRED, { (TAF_UINT8 *)"17", (TAF_UINT8 *)"SIM PIN2 required" }},
    /* 说明：需要输入SIM PUK2码。 */
    { AT_CME_SIM_PUK2_REQUIRED, { (TAF_UINT8 *)"18", (TAF_UINT8 *)"SIM PUK2 required" }},
    /* 说明：存储器满。 */
    { AT_CME_MEMORY_FULL, { (TAF_UINT8 *)"20", (TAF_UINT8 *)"memory full" }},
    /* 说明：无效的索引。 */
    { AT_CME_INVALID_INDEX, { (TAF_UINT8 *)"21", (TAF_UINT8 *)"invalid index" }},
    /* 说明：没有找到。 */
    { AT_CME_NOT_FOUND, { (TAF_UINT8 *)"22", (TAF_UINT8 *)"not found" }},
    /* 说明：存储失败。 */
    { AT_CME_MEMORY_FAILURE, { (TAF_UINT8 *)"23", (TAF_UINT8 *)"memory failure" }},
    /* 说明：文本太长。 */
    { AT_CME_TEXT_STRING_TOO_LONG, { (TAF_UINT8 *)"24", (TAF_UINT8 *)"text string too long" }},
    /* 说明：文本中有无效字符。 */
    { AT_CME_INVALID_CHARACTERS_IN_TEXT_STRING, { (TAF_UINT8 *)"25", (TAF_UINT8 *)"INVALID CHARACTERS IN TEXT STRING" }},
    /* 说明：电话号码太长。 */
    { AT_CME_DIAL_STRING_TOO_LONG, { (TAF_UINT8 *)"26", (TAF_UINT8 *)"dial string too long" }},
    /* 说明：号码中有无效字符。 */
    { AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING, { (TAF_UINT8 *)"27", (TAF_UINT8 *)"invalid characters in dial string" }},

    /* 说明：SPN文件内容错误 */
    { AT_CME_SPN_FILE_CONTENT_ERROR, { (TAF_UINT8 *)"65284", (TAF_UINT8 *)"SPN FILE CONTENT ERROR" }},
    /* 说明：读SPN文件被拒 */
    { AT_CME_SPN_FILE_QUERY_REJECT, { (TAF_UINT8 *)"65285", (TAF_UINT8 *)"READ SPN FILE REJECTED" }},
    /* 说明：SPN文件不存在 */
    { AT_CME_SPN_FILE_NOT_EXISTS, { (TAF_UINT8 *)"65286", (TAF_UINT8 *)"SPN FILE NOT EXIST" }},

    /* 说明：无网络服务。 */
    { AT_CME_NO_NETWORK_SERVICE, { (TAF_UINT8 *)"30", (TAF_UINT8 *)"NO NETWORK SERVICE" }},
    /* 说明：网络超时。 */
    { AT_CME_NETWORK_TIMEOUT, { (TAF_UINT8 *)"31", (TAF_UINT8 *)"NETWORK TIMEOUT" }},
    /* 说明：网络只支持紧急呼叫。 */
    { AT_CME_NETWORK_NOT_ALLOWED_EMERGENCY_CALLS_ONLY, { (TAF_UINT8 *)"32", (TAF_UINT8 *)"NETWORK NOT ALLOWED - EMERGENCY CALLS ONLY" }},
    /* 说明：需要输入网络PIN码。 */
    { AT_CME_NETWORK_PERSONALIZATION_PIN_REQUIRED, { (TAF_UINT8 *)"40", (TAF_UINT8 *)"NETWORK PERSONALIZATION PIN REQUIRED" }},
    /* 说明：需要输入网络PUK码。 */
    { AT_CME_NETWORK_PERSONALIZATION_PUK_REQUIRED, { (TAF_UINT8 *)"41", (TAF_UINT8 *)"NETWORK PERSONALIZATION PUK REQUIRED" }},
    /* 说明：需要输入网络子PIN码。 */
    { AT_CME_NETWORK_SUBSET_PERSONALIZATION_PIN_REQUIRED, { (TAF_UINT8 *)"42", (TAF_UINT8 *)"NETWORK SUBSET PERSONALIZATION PIN REQUIRED" }},
    /* 说明：需要输入网络子PUK码。 */
    { AT_CME_NETWORK_SUBSET_PERSONALIZATION_PUK_REQUIRED, { (TAF_UINT8 *)"43", (TAF_UINT8 *)"NETWORK SUBSET PERSONALIZATION PUK REQUIRED" }},
    /* 说明：需要输入服务提供商的PIN码。 */
    { AT_CME_SERVICE_PROVIDER_PERSONALIZATION_PIN_REQUIRED, { (TAF_UINT8 *)"44", (TAF_UINT8 *)"SERVICE PROVIDER PERSONALIZATION PIN REQUIRED" }},
    /* 说明：需要输入服务提供商的PUK码。 */
    { AT_CME_SERVICE_PROVIDER_PERSONALIZATION_PUK_REQUIRED, { (TAF_UINT8 *)"45", (TAF_UINT8 *)"SERVICE PROVIDER PERSONALIZATION PUK REQUIRED" }},
    /* 说明：需要输入卡商的PIN码。 */
    { AT_CME_CORPORATE_PERSONALIZATION_PIN_REQUIRED, { (TAF_UINT8 *)"46", (TAF_UINT8 *)"CORPORATE PERSONALIZATION PIN REQUIRED" }},
    /* 说明：需要输入卡商的PUK码。 */
    { AT_CME_CORPORATE_PERSONALIZATION_PUK_REQUIRED, { (TAF_UINT8 *)"47", (TAF_UINT8 *)"CORPORATE PERSONALIZATION PUK REQUIRED" }},
    /* 说明：隐藏密码等待输入。 */
    { AT_CME_HIDDEN_KEY_REQUIRED, { (TAF_UINT8 *)"48", (TAF_UINT8 *)"hidden key required" }},
    /* 说明：不支持EAP。 */
    { AT_CME_EAP_METHOD_NOT_SUPPORTED, { (TAF_UINT8 *)"49", (TAF_UINT8 *)"EAP method not supported" }},
    /* 说明：参数错误。 */
    { AT_CME_INCORRECT_PARAMETERS, { (TAF_UINT8 *)"50", (TAF_UINT8 *)"Incorrect parameters" }},
    /* 说明：电话故障。 */
    { AT_CME_RX_DIV_NOT_SUPPORTED, { (TAF_UINT8 *)"0", (TAF_UINT8 *)"NOT SUPPORTED" }},
    /* 说明：与电话无连接。 */
    { AT_CME_RX_DIV_OTHER_ERR, { (TAF_UINT8 *)"1", (TAF_UINT8 *)"OTHER ERROR" }},
    /* 说明：未知错误。 */
    { AT_CME_UNKNOWN, { (TAF_UINT8 *)"100", (TAF_UINT8 *)"UNKNOWN" }},

    /* 说明：MS非法。 */
    { AT_CME_ILLEGAL_MS, { (TAF_UINT8 *)"103", (TAF_UINT8 *)"Illegal MS" }},
    /* 说明：ME非法。 */
    { AT_CME_ILLEGAL_ME, { (TAF_UINT8 *)"106", (TAF_UINT8 *)"Illegal ME" }},
    /* 说明：GPRS服务不允许。 */
    { AT_CME_GPRS_SERVICES_NOT_ALLOWED, { (TAF_UINT8 *)"107", (TAF_UINT8 *)"GPRS services not allowed" }},
    /* 说明：PLMN不允许。 */
    { AT_CME_PLMN_NOT_ALLOWED, { (TAF_UINT8 *)"111", (TAF_UINT8 *)"PLMN not allowed" }},
    /* 说明：本区域不允许。 */
    { AT_CME_LOCATION_AREA_NOT_ALLOWED, { (TAF_UINT8 *)"112", (TAF_UINT8 *)"Location area not allowed" }},
    /* 说明：本区域不允许漫游。 */
    { AT_CME_ROAMING_NOT_ALLOWED_IN_THIS_LOCATION_AREA, { (TAF_UINT8 *)"113", (TAF_UINT8 *)"Roaming not allowed in this location area" }},

    /* 说明：业务类型不支持。 */
    { AT_CME_SERVICE_OPTION_NOT_SUPPORTED, { (TAF_UINT8 *)"132", (TAF_UINT8 *)"service option not supported" }},
    /* 说明：请求的业务未定制。 */
    { AT_CME_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED, { (TAF_UINT8 *)"133", (TAF_UINT8 *)"requested service option not subscribed" }},
    /* 说明：业务类型暂时不可用。 */
    { AT_CME_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER, { (TAF_UINT8 *)"134", (TAF_UINT8 *)"service option temporarily out of order" }},
    /* 说明：PDP鉴权失败。 */
    { AT_CME_PDP_AUTHENTICATION_FAILURE, { (TAF_UINT8 *)"149", (TAF_UINT8 *)"PDP authentication failure" }},

    /* 说明：无效的手机等级 */
    { AT_CME_INVALID_MOBILE_CLASS, { (TAF_UINT8 *)"150", (TAF_UINT8 *)"invalid mobile class" }},
    { AT_CME_UNSPECIFIED_GPRS_ERROR, { (TAF_UINT8 *)"148", (TAF_UINT8 *)"unspecified GPRS error" }},

    /* 说明：VBS/VGCS网络不支持 */
    { AT_CME_VBS_VGCS_NOT_SUPPORTED_BY_THE_NETWORK, { (TAF_UINT8 *)"151", (TAF_UINT8 *)"VBS/VGCS not supported by the network" }},
    /* 说明：SIM没有订阅服务 */
    { AT_CME_NO_SERVICE_SUBSCRIPTION_ON_SIM, { (TAF_UINT8 *)"152", (TAF_UINT8 *)"No service subscription on SIM" }},
    /* 说明：小组ID没有订阅 */
    { AT_CME_NO_SUBSCRIPTION_FOR_GROUP_ID, { (TAF_UINT8 *)"153", (TAF_UINT8 *)"No subscription for group ID" }},
    /* 说明：小组ID在SIM卡中没有激活 */
    { AT_CME_GROUP_ID_NOT_ACTIVATED_ON_SIM, { (TAF_UINT8 *)"154", (TAF_UINT8 *)"Group Id not activated on SIM" }},
    /* 说明：没有匹配的通知 */
    { AT_CME_NO_MATCHING_NOTIFICATION, { (TAF_UINT8 *)"155", (TAF_UINT8 *)"No matching notification" }},
    /* 说明：VBS/VGCS呼叫已经存在 */
    { AT_CME_VBS_VGCS_CALL_ALREADY_PRESENT, { (TAF_UINT8 *)"156", (TAF_UINT8 *)"VBS/VGCS call already present" }},
    /* 说明：拥塞 */
    { AT_CME_CONGESTION, { (TAF_UINT8 *)"157", (TAF_UINT8 *)"Congestion" }},
    /* 说明：网络失败 */
    { AT_CME_NETWORK_FAILURE, { (TAF_UINT8 *)"158", (TAF_UINT8 *)"Network failure" }},
    /* 说明：上行忙 */
    { AT_CME_UPLINK_BUSY, { (TAF_UINT8 *)"159", (TAF_UINT8 *)"Uplink busy" }},
    /* 说明：Sim文件不能访问 */
    { AT_CME_NO_ACCESS_RIGHTS_FOR_SIM_FILE, { (TAF_UINT8 *)"160", (TAF_UINT8 *)"No access rights for SIM file" }},
    /* 说明：无优先级订阅 */
    { AT_CME_NO_SUBSCRIPTION_FOR_PRIORITY, { (TAF_UINT8 *)"161", (TAF_UINT8 *)"No subscription for priority" }},
    /* 说明：操作不可应用或者不可能 */
    { AT_CME_OPERATION_NOT_APPLICABLE_OR_NOT_POSSIBLE, { (TAF_UINT8 *)"162", (TAF_UINT8 *)"operation not applicable or not possible" }},
    /* 说明：文件不存在 */
    { AT_CME_FILE_NOT_EXISTS, { (TAF_UINT8 *)"163", (TAF_UINT8 *)"FILE NOT EXIST" }},

    { AT_CME_1X_RAT_NOT_SUPPORTED, { (VOS_UINT8 *)"170", (VOS_UINT8 *)"1X RAT NOT SUPPORTED" }},

    /* 说明：业务未签约。 */
    { AT_CME_SERVICE_NOT_PROVISIONED, { (VOS_UINT8 *)"171", (VOS_UINT8 *)"Service not provisioned" }},

    /* 3GPP 27007 9.2.3 Also all other values below 256 are reserved */
    /* 说明：PDP激活受限 */
    { AT_CME_PDP_ACT_LIMIT, { (TAF_UINT8 *)"300", (TAF_UINT8 *)"PDP ACT LIMIT" }},
    /* 说明：网络选择菜单不使能 */
    { AT_CME_NET_SEL_MENU_DISABLE, { (TAF_UINT8 *)"301", (TAF_UINT8 *)"NETWORK SELECTION MENU DISABLE" }},

    /* 说明：CS业务存在时，不允许设置SYSCFGEX，AT回复”+CME ERROR: CS SERVICE EXIST” */
    { AT_CME_CS_IMS_SERV_EXIST, { (TAF_UINT8 *)"302", (TAF_UINT8 *)"CS SERVICE EXIST" }},

    /* 说明：FDN检查失败 */
    { AT_CME_FDN_FAILED, { (TAF_UINT8 *)"303", (TAF_UINT8 *)"FDN Failed" }},
    /* 说明：Call Control检查失败 */
    { AT_CME_CALL_CONTROL_FAILED, { (TAF_UINT8 *)"304", (TAF_UINT8 *)"Call Control Failed" }},
    /* 说明：Call Control检查结果为允许但需要修改，修改的业务超出UE能力 */
    { AT_CME_CALL_CONTROL_BEYOND_CAPABILITY, { (TAF_UINT8 *)"305", (TAF_UINT8 *)"Call Control beyond Capability" }},

    /* 说明：不支持IMS */
    { AT_CME_IMS_NOT_SUPPORT, { (VOS_UINT8 *)"306", (VOS_UINT8 *)"IMS Not Support" }},

    /* 说明：IMS业务存在时，不允许设置^IMSSWITCH关闭IMS功能，不允许通过CEVDP设置语音优选模式 */
    { AT_CME_IMS_SERVICE_EXIST, { (VOS_UINT8 *)"307", (VOS_UINT8 *)"IMS Service Exist" }},
    /* 说明：在语音业务PS_ONLY模式下，不允许通过^IMSSWITCH关闭IMS功能 */
    { AT_CME_IMS_VOICE_DOMAIN_PS_ONLY, { (VOS_UINT8 *)"308", (VOS_UINT8 *)"IMS Voice Domain PS Only" }},
    /* 说明：打开或者关闭IMS协议栈失败 */
    { AT_CME_IMS_STACK_TIMEOUT, { (VOS_UINT8 *)"309", (VOS_UINT8 *)"IMS Stack Time Out" }},

    /* 说明：无射频资源可用 */
    { AT_CME_NO_RF, { (TAF_UINT8 *)"310", (TAF_UINT8 *)"NO RF" }},

    /* 说明：打开IMS开关时，当前不支持Lte */
    { AT_CME_IMS_OPEN_LTE_NOT_SUPPORT, { (TAF_UINT8 *)"311", (TAF_UINT8 *)"IMS Open,LTE Not Support" }},

    /* 说明：APN长度不合法 */
    { AT_CME_APN_LEN_ILLEGAL, { (VOS_UINT8 *)"700", (VOS_UINT8 *)"APN length illegal" }},
    /* 说明：APN语法错误 */
    { AT_CME_APN_SYNTACTICAL_ERROR, { (VOS_UINT8 *)"701", (VOS_UINT8 *)"APN syntactical error" }},
    /* 说明：鉴权之前设置APN */
    { AT_CME_SET_APN_BEFORE_SET_AUTH, { (VOS_UINT8 *)"702", (VOS_UINT8 *)"set APN before set auth" }},
    /* 说明：鉴权类型非法 */
    { AT_CME_AUTH_TYPE_ILLEGAL, { (VOS_UINT8 *)"703", (VOS_UINT8 *)"auth type illegal" }},
    /* 说明：用户名称太长 */
    { AT_CME_USER_NAME_TOO_LONG, { (VOS_UINT8 *)"704", (VOS_UINT8 *)"user name too long" }},
    /* 说明：用户密码太长 */
    { AT_CME_USER_PASSWORD_TOO_LONG, { (VOS_UINT8 *)"705", (VOS_UINT8 *)"user password too long" }},
    /* 说明：接入号码太长 */
    { AT_CME_ACCESS_NUM_TOO_LONG, { (VOS_UINT8 *)"706", (VOS_UINT8 *)"access number too long" }},
    /* 说明：呼叫cid在操作中 */
    { AT_CME_CALL_CID_IN_OPERATION, { (VOS_UINT8 *)"707", (VOS_UINT8 *)"call cid in operation" }},
    /* 说明：承载类型不是默认的 */
    { AT_CME_BEARER_TYPE_NOT_DEFAULT, { (VOS_UINT8 *)"708", (VOS_UINT8 *)"bearer type not default" }},
    /* 说明：错误的call cid */
    { AT_CME_CALL_CID_INVALID, { (VOS_UINT8 *)"709", (VOS_UINT8 *)"call cid invalid" }},
    /* 说明：call cid已激活 */
    { AT_CME_CALL_CID_ACTIVE, { (VOS_UINT8 *)"710", (VOS_UINT8 *)"call cid active" }},
    /* 说明：承载类型非法 */
    { AT_CME_BEARER_TYPE_ILLEGAL, { (VOS_UINT8 *)"711", (VOS_UINT8 *)"bearer type illegal" }},
    /* 说明：必须存在默认类型的cid */
    { AT_CME_MUST_EXIST_DEFAULT_TYPE_CID, { (VOS_UINT8 *)"712", (VOS_UINT8 *)"must exist default type cid" }},
    /* 说明：PDN类型非法 */
    { AT_CME_PDN_TYPE_ILLEGAL, { (VOS_UINT8 *)"713", (VOS_UINT8 *)"PDN type illegal" }},
    /* 说明：IPV4地址申请类型非法 */
    { AT_CME_IPV4_ADDR_ALLOC_TYPE_ILLEGAL, { (VOS_UINT8 *)"714", (VOS_UINT8 *)"IPV4 address alloc type illegal" }},
    /* 说明：关联的CID无效 */
    { AT_CME_LINK_CID_INVALID, { (VOS_UINT8 *)"715", (VOS_UINT8 *)"link cid invalid" }},
    /* 说明：逻辑通道上没有对应的数据单元 */
    { AT_CME_NO_SUCH_ELEMENT, { (VOS_UINT8 *)"716", (VOS_UINT8 *)"no such element" }},
    /* 说明：无剩余通道资源 */
    { AT_CME_MISSING_RESOURCE, { (VOS_UINT8 *)"717", (VOS_UINT8 *)"missing resource" }},

    /* 说明：HIDS连接态LogPort不允许切换为VCOM */
    { AT_CME_USB_TO_VCOM_IN_CONN_ERROR, { (TAF_UINT8 *)"750", (TAF_UINT8 *)"USB change to VCOM at diag connect" }},

    /* 说明：CL模式不支持该操作 */
    { AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE, { (VOS_UINT8 *)"718", (VOS_UINT8 *)"operation not allowed in CL mode" }},

    /* 说明：PIN密文解密失败 */
    { AT_CME_SILENT_AES_DEC_PIN_ERROR, { (TAF_UINT8 *)"751", (TAF_UINT8 *)"Silent Operate: Dec Encryptpin Fail" }},
    /* 说明：PIN码解锁失败 */
    { AT_CME_SILENT_VERIFY_PIN_ERROR, { (TAF_UINT8 *)"752", (TAF_UINT8 *)"Silent Operate: Verify Pin Fail" }},
    /* 说明：PIN码加密失败 */
    { AT_CME_SILENT_AES_ENC_PIN_ERROR, { (TAF_UINT8 *)"753", (TAF_UINT8 *)"Silent Operate: Enc Pin Fail" }},
    /* 说明：文件不存在 */
    { AT_CME_NOT_FIND_FILE, { (TAF_UINT8 *)"754", (TAF_UINT8 *)"Not Find File" }},
    /* 说明：没有发现NV */
    { AT_CME_NOT_FIND_NV, { (TAF_UINT8 *)"755", (TAF_UINT8 *)"Not Find NV" }},
    /* 说明：Modem id 参数错误 */
    { AT_CME_MODEM_ID_ERROR, { (TAF_UINT8 *)"756", (TAF_UINT8 *)"Modem Id Error" }},
    { AT_CME_WRITE_NV_TimeOut, { (TAF_UINT8 *)"757", (TAF_UINT8 *)"Write NV TimeOut" }},
    /* 说明：Nv 不支持错误 */
    { AT_CME_NV_NOT_SUPPORT_ERR, { (TAF_UINT8 *)"758", (TAF_UINT8 *)"NV Not Support" }},
    /* 说明：伪基站功能不支持 */
    { AT_CME_FUNC_DISABLE, { (TAF_UINT8 *)"759", (TAF_UINT8 *)"Function Disable" }},
    /* 说明：和SIM卡交互发生SCI错误 */
    { AT_CME_SCI_ERROR, { (TAF_UINT8 *)"760", (TAF_UINT8 *)"SCI Error" }},

    /* 说明：EMAT 打开逻辑通道错误 */
    { AT_ERR_EMAT_OPENCHANNEL_ERROR, { (TAF_UINT8 *)"761", (TAF_UINT8 *)"EMAT open channel error" }},
    /* 说明：EMAT 打开逻辑通道回复错误 */
    { AT_ERR_EMAT_OPENCHANNEL_CNF_ERROR, { (TAF_UINT8 *)"762", (TAF_UINT8 *)"EMAT open channel cnf error" }},
    /* 说明：EMAT 关闭逻辑通道错误 */
    { AT_ERR_EMAT_CLOSECHANNEL_ERROR, { (TAF_UINT8 *)"763", (TAF_UINT8 *)"EMAT close channel error" }},
    /* 说明：EMAT 关闭逻辑通道回复错误 */
    { AT_ERR_EMAT_CLOSECHANNEL_CNF_ERROR, { (TAF_UINT8 *)"764", (TAF_UINT8 *)"EMAT close channel cnf error" }},
    /* 说明：EMAT 获取EID错误 */
    { AT_ERR_EMAT_GETEID_ERROR, { (TAF_UINT8 *)"765", (TAF_UINT8 *)"EMAT get eid error" }},
    /* 说明：EMAT 获取EID的数据错误 */
    { AT_ERR_EMAT_GETEID_DATA_ERROR, { (TAF_UINT8 *)"766", (TAF_UINT8 *)"EMAT get eid data error" }},
    /* 说明：EMAT 获取PKID错误 */
    { AT_ERR_EMAT_GETPKID_ERROR, { (TAF_UINT8 *)"767", (TAF_UINT8 *)"EMAT get pkid error" }},
    /* 说明：EMAT 获取PKID的数据错误 */
    { AT_ERR_EMAT_GETPKID_DATA_ERROR, { (TAF_UINT8 *)"768", (TAF_UINT8 *)"EMAT get pkid data error" }},
    /* 说明：EMAT 清除Profile错误 */
    { AT_ERR_EMAT_CLEANPROFILE_ERROR, { (TAF_UINT8 *)"769", (TAF_UINT8 *)"EMAT clean profile error" }},
    /* 说明：EMAT 清除Profile的回复数据错误 */
    { AT_ERR_EMAT_CLEANPROFILE_DATA_ERROR, { (TAF_UINT8 *)"770", (TAF_UINT8 *)"EMAT clean profile data error" }},
    /* 说明：EMAT 检测Profile错误 */
    { AT_ERR_EMAT_CHECKPROFILE_ERROR, { (TAF_UINT8 *)"771", (TAF_UINT8 *)"EMAT check profile error" }},
    /* 说明：EMAT 检测Profile的回复数据错误 */
    { AT_ERR_EMAT_CHECKPROFILE_DATA_ERROR, { (TAF_UINT8 *)"772", (TAF_UINT8 *)"EMAT check profile data error" }},
    /* 说明：EMAT 获取数据的TPDU回复错误 */
    { AT_ERR_EMAT_TPDU_CNF_ERROR, { (TAF_UINT8 *)"773", (TAF_UINT8 *)"EMAT TPDU cnf error" }},
    /* 说明：EMAT 保存TPDU回复的数据错误 */
    { AT_ERR_EMAT_TPDU_DATASTORE_ERROR, { (TAF_UINT8 *)"774", (TAF_UINT8 *)"EMAT TPDU data store error" }},
    /* 说明：eSIM切换设置drv返回错误 */
    { AT_ERR_ESIMSWITCH_SET_ERROR, { (TAF_UINT8 *)"775", (TAF_UINT8 *)"PIH switch drv error" }},
    /* 说明：eSIM切换设置当前卡激活，不允许切换 */
    { AT_ERR_ESIMSWITCH_SET_NOT_ENABLE_ERROR, { (TAF_UINT8 *)"776", (TAF_UINT8 *)"PIH switch is not Enable" }},
    /* 说明：eSIM切换查询错误 */
    { AT_ERR_ESIMSWITCH_QRY_ERROR, { (TAF_UINT8 *)"777", (TAF_UINT8 *)"PIH switch query error" }},

    /* 说明：申请内存失败 */
    { AT_ERR_CARRIER_MALLOC_FAIL, { (TAF_UINT8 *)"778", (TAF_UINT8 *)"CARRIER malloc fail" }},
    /* 说明：读取NV原始数据错误 */
    { AT_ERR_CARRIER_READ_NV_ORIGINAL_DATA_ERR, { (TAF_UINT8 *)"779", (TAF_UINT8 *)"CARRIER read NV original data error" }},
    /* 说明：随卡文件长度错误 */
    { AT_ERR_CARRIER_FILE_LEN_ERROR, { (TAF_UINT8 *)"780", (TAF_UINT8 *)"CARRIER file len error" }},
    /* 说明：NV项长度错误 */
    { AT_ERR_CARRIER_NV_LEN_ERROR, { (TAF_UINT8 *)"781", (TAF_UINT8 *)"CARRIER NV len error" }},
    /* 说明：写内存释放锁失败 */
    { AT_ERR_CARRIER_WRITE_NV_FREE_FAIL, { (TAF_UINT8 *)"782", (TAF_UINT8 *)"CARRIER write NV free fail" }},
    /* 说明：NV错误 */
    { AT_ERR_CARRIER_NV_ERROR, { (TAF_UINT8 *)"783", (TAF_UINT8 *)"CARRIER NV error" }},

    /* 说明：暂时没有发现同步源 */
    { AT_CME_NOT_FOUND_SYNC_SOURCE, { (VOS_UINT8 *)"784", (VOS_UINT8 *)"NOT FOUND SYNC SOURCE TEMPORARILY" }},

    { AT_ERR_CARRIER_HMAC_VERIFY_FAIL, { (TAF_UINT8 *)"785", (TAF_UINT8 *)"CARRIER HMAC verify fail" }},

    { AT_ERR_NOT_IN_NR_NORMAL_SERVICE, { (VOS_UINT8 *)"786", (VOS_UINT8 *)"NOT IN NR NORMAL SERVICE"}},
    { AT_ERR_NO_NEED_REPEAT_AGAINS, { (VOS_UINT8 *)"787", (VOS_UINT8 *)"NO NEED REPEAT AGAINS"}},
    { AT_ERR_REPEAT_START_CDMA, { (VOS_UINT8 *)"788", (VOS_UINT8 *)"REPEAT START CDMA"}},

    { AT_CME_ERROR_ENUM_END, { TAF_NULL_PTR, TAF_NULL_PTR }},

    /* 装备AT命令错误码提示 */
    { AT_DEVICE_ERROR_BEGIN, { VOS_NULL_PTR, VOS_NULL_PTR }},
    { AT_DEVICE_MODE_ERROR, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"Mode Error" }},
    { AT_FCHAN_BAND_NOT_MATCH, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"Band not match" }},
    { AT_FCHAN_SET_CHANNEL_FAIL, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Channel number set fail" }},
    { AT_FCHAN_BAND_CHANNEL_NOT_MATCH, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"Band and Channel not match" }},
    { AT_FCHAN_OTHER_ERR, { (VOS_UINT8 *)"4", (VOS_UINT8 *)"Other error" }},
    { AT_FCHAN_WIFI_BAND_ERR, { (VOS_UINT8 *)"5", (VOS_UINT8 *)"Wifi Band Error" }},
    { AT_NOT_SUPPORT_WIFI, { (VOS_UINT8 *)"6", (VOS_UINT8 *)"Not Support Wifi" }},
    { AT_WIFI_NOT_ENABLE, { (VOS_UINT8 *)"7", (VOS_UINT8 *)"Wifi Not Enable" }},
    { AT_FCHAN_RAT_ERR, { (VOS_UINT8 *)"8", (VOS_UINT8 *)"Rat Error" }},
    { AT_FCHAN_BAND_WIDTH_ERR, { (VOS_UINT8 *)"9", (VOS_UINT8 *)"Band Width Err" }},
    { AT_FCHAN_SCS_ERR, { (VOS_UINT8 *)"10", (VOS_UINT8 *)"Scs Err" }},
    { AT_FCHAN_NO_SCS, { (VOS_UINT8 *)"13", (VOS_UINT8 *)"No Scs PARAMETERS" }},
    { AT_SND_MSG_FAIL, { (VOS_UINT8 *)"11", (VOS_UINT8 *)"Snd Msg Fail" }},
    { AT_FCHAN_LOAD_DSP_ERR, { (VOS_UINT8 *)"12", (VOS_UINT8 *)"Load Dsp Fail" }},
    { AT_NOT_LOAD_DSP, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"Not Load Dsp" }},
    { AT_FWAVE_TYPE_ERR, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Fave Type Err" }},
    { AT_TSELRF_PATH_ERR, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"Path Err" }},
    { AT_ESELRF_TX_OR_RX_ERR, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"TX Or RX Err" }},
    { AT_MIMO_PARA_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"MIMO TYPE ERR" }},
    { AT_DPDT_RAT_ERR, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"Dpdt Rat err" }},
    { AT_TRXTAS_CMD_PARA_ERR, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Trx Tas Cmd Para err" }},
    { AT_TRXTAS_RAT_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"Trx Tas Rat Para err" }},
    { AT_TRXTAS_FCHAN_RAT_NOT_MATCH, { (VOS_UINT8 *)"4", (VOS_UINT8 *)"Trx Tas FCHAN Rat not Match" }},
    { AT_TRXTAS_TX_NOT_SWITCH_ON, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"Trx Tas TX Not Switch On" }},

    { AT_FDAC_CHANNEL_NOT_SET, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"channel not set" }},
    { AT_FDAC_SET_FAIL, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Set DAC fail" }},
    { AT_CHANNEL_NOT_SET, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"channel not set" }},
    { AT_FTXON_SET_FAIL, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Set FTXON fail" }},
    { AT_FTXON_OTHER_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"other FTXON Error" }},
    { AT_NOT_SET_PATH, { (VOS_UINT8 *)"4", (VOS_UINT8 *)"Not Set Path(^TSELRF)" }},

    { AT_DATA_UNLOCK_ERROR, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"Data UNLock Error:0" }},
    { AT_DPAUPA_ERROR, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"DPA UPA Error:1" }},
    { AT_SN_LENGTH_ERROR, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"SN LENGTH Error:1" }},
    { AT_FRXON_OTHER_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"other FRXON Error" }},
    { AT_FRXON_SET_FAIL, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Set FRXON fail" }},
    { AT_FPA_OTHER_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"Other FPA error" }},
    { AT_FPA_LEVEL_ERR, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Fpa Level Error" }},

    { AT_FLNA_OTHER_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"Other LNA error" }},
    { AT_FRSSI_OTHER_ERR, { (VOS_UINT8 *)"4", (VOS_UINT8 *)"Other FRSSI error" }},
    { AT_FRSSI_RX_NOT_OPEN, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"RX Not Open" }},
    { AT_SD_CARD_NOT_EXIST, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"SD Card Not Exist" }},
    { AT_SD_CARD_INIT_FAILED, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"SD Card init fail" }},
    { AT_SD_CARD_OPRT_NOT_ALLOWED, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Sd Oprt not allowed" }},
    { AT_SD_CARD_ADDR_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"Sd Card Address Error" }},
    { AT_SD_CARD_OTHER_ERR, { (VOS_UINT8 *)"4", (VOS_UINT8 *)"Sd Card Other Error" }},
    { AT_DEVICE_OTHER_ERROR, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"1" }},
    { AT_PHYNUM_LENGTH_ERR, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"Physical Number length error" }},
    { AT_PHYNUM_NUMBER_ERR, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"Physical Number number error" }},
    { AT_PHYNUM_TYPE_ERR, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Physical Number type error" }},
    { AT_SIMLOCK_PLMN_NUM_ERR, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Simlock Plmn num invalid" }},
    { AT_SIMLOCK_PLMN_MNC_LEN_ERR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"Simlock Plmn MNC len Err" }},

    /* 说明：未实现 */
    {AT_DEVICE_NOT_SUPPORT,               {(VOS_UINT8*)"500", (VOS_UINT8*)"Undone",                            }},
    /* 说明：未知错误 */
    {AT_DEVICE_ERR_UNKNOWN,               {(VOS_UINT8*)"501", (VOS_UINT8*)"Unknown error",                     }},
    /* 说明：输入参数非法，包括不在合法范围等错误。 */
    {AT_DEVICE_INVALID_PARAMETERS,        {(VOS_UINT8*)"502", (VOS_UINT8*)"Invalid parameters",                }},
    /* 说明：NV不存在 */
    {AT_DEVICE_NV_NOT_SUPPORT_ID,         {(VOS_UINT8*)"520", (VOS_UINT8*)"NVIM Not Exist",                    }},
    /* 说明：NV读取失败 */
    {AT_DEVICE_NV_READ_FAILURE,           {(VOS_UINT8*)"521", (VOS_UINT8*)"Read NVIM Failure",                 }},
    /* 说明：NV写失败，长度过长 */
    {AT_DEVICE_NV_WRITE_FAIL_OVERLEN,     {(VOS_UINT8*)"522", (VOS_UINT8*)"Write Error for Length Overflow",   }},
    /* 说明：NV写失败，Flash坏块 */
    {AT_DEVICE_NV_WRITE_FAIL_BADFLASH,    {(VOS_UINT8*)"523", (VOS_UINT8*)"Write Error for Flash Bad Block",   }},
    /* 说明：NV写失败，其它错误 */
    {AT_DEVICE_NV_WRITE_FAIL_UNKNOWN,     {(VOS_UINT8*)"524", (VOS_UINT8*)"Write Error for Unknown Reason",    }},
    /* 说明：设置VCTCXO电压过高 */
    {AT_DEVICE_VCTCXO_OVER_HIGH,          {(VOS_UINT8*)"525", (VOS_UINT8*)"Higher Voltage",                    }},
    /* 说明：单板模式错误(比如信令模式) */
    {AT_DEVICE_UE_MODE_ERR,               {(VOS_UINT8*)"526", (VOS_UINT8*)"UE Mode Error",                     }},
    /* 说明：没有设置相关信道 */
    {AT_DEVICE_NOT_SET_CHAN,              {(VOS_UINT8*)"527", (VOS_UINT8*)"Not Set Appointed Channel",         }},
    /* 说明：打开发射机失败 */
    {AT_DEVICE_OPEN_UL_CHAN_ERROR,        {(VOS_UINT8*)"528", (VOS_UINT8*)"Open TX Transmitter Failure",       }},
    /* 说明：打开接收机失败 */
    {AT_DEVICE_OPEN_DL_CHAN_ERROR,        {(VOS_UINT8*)"529", (VOS_UINT8*)"Open RX Transmitter Failure",       }},
    /* 说明：通道打开失败 */
    {AT_DEVICE_OPEN_CHAN_ERROR,           {(VOS_UINT8*)"530", (VOS_UINT8*)"Open Channel Failure",              }},
    /* 说明：通道关闭失败 */
    {AT_DEVICE_CLOSE_CHAN_ERROR,          {(VOS_UINT8*)"531", (VOS_UINT8*)"Close Channel Failure",             }},
    /* 说明：不支持（如打开通道2的发送通道，DD800项目采用1T2R模式） */
    {AT_DEVICE_OPERATION_NOT_SUPPORT,     {(VOS_UINT8*)"532", (VOS_UINT8*)"Not Support",                       }},
    /* 说明：非法操作（比如读取特殊NV） */
    {AT_DEVICE_INVALID_OP,                {(VOS_UINT8*)"533", (VOS_UINT8*)"Invalid Operation",                 }},
    /* 说明：输入频段信息无法对应 */
    {AT_DEVICE_CHAN_BAND_INVALID,         {(VOS_UINT8*)"534", (VOS_UINT8*)"Band No Match",                     }},
    /* 说明：设置信道信息失败 */
    {AT_DEVICE_SET_CHAN_INFO_FAILURE,     {(VOS_UINT8*)"535", (VOS_UINT8*)"Set Channel Information Failure",   }},
    /* 说明：输入频段和信道信息无法组合 */
    {AT_DEVICE_CHAN_BAND_CHAN_NOT_MAP,    {(VOS_UINT8*)"536", (VOS_UINT8*)"Band And Channel Not Combined",     }},
    /* 说明：设置TX的发射功率失败 */
    {AT_DEVICE_SET_TX_POWER_FAILURE,      {(VOS_UINT8*)"537", (VOS_UINT8*)"Set TX Transmitter Power Error",    }},
    /* 说明：设置PA级失败 */
    {AT_DEVICE_SET_PA_LEVEL_FAILURE,      {(VOS_UINT8*)"538", (VOS_UINT8*)"Set PA Level Failure",              }},
    /* 说明：未设置当前信道 */
    {AT_DEVICE_NOT_SET_CURRENT_CHAN,      {(VOS_UINT8*)"539", (VOS_UINT8*)"Not Set Current Channel",           }},
    /* 说明：无法读取当前设置的APC值 APC Value Can' */
    {AT_DEVICE_CUR_APC_UNAVAILABLE,       {(VOS_UINT8*)"540", (VOS_UINT8*)"APC Value Can't Be Read",           }},
    /* 说明：设置APC操作失败 */
    {AT_DEVICE_SET_APC_ERR,               {(VOS_UINT8*)"541", (VOS_UINT8*)"Write APC Failure",                 }},
    /* 说明：读取APC操作失败 */
    {AT_DEVICE_RD_APC_ERR,                {(VOS_UINT8*)"542", (VOS_UINT8*)"Read APC Failure",                  }},
    /* 说明：设置LNA失败 */
    {AT_DEVICE_SET_LNA_ERR,               {(VOS_UINT8*)"543", (VOS_UINT8*)"Set AAGC Failure",                  }},
    /* 说明：没有打开接收机 */
    {AT_DEVICE_NOT_OPEN_DL_CHAN,          {(VOS_UINT8*)"544", (VOS_UINT8*)"RX Transmitter Not Open",           }},
    /* 说明：没有打开发射机 */
    {AT_DEVICE_NOT_OPEN_UL_CHAN,          {(VOS_UINT8*)"545", (VOS_UINT8*)"TX Transmitter Not Open",           }},
    /* 说明：无信号输入或检测不到信号 */
    {AT_DEVICE_NO_SIGNAL,                 {(VOS_UINT8*)"546", (VOS_UINT8*)"No Signal",                         }},
    /* 说明：物理号长度错误 */
    {AT_DEVICE_PHYNUM_LEN_ERR,            {(VOS_UINT8*)"547", (VOS_UINT8*)"PHY Number Length Error",           }},
    /* 说明：物理号不合法 */
    {AT_DEVICE_PHYNUM_INVALID,            {(VOS_UINT8*)"548", (VOS_UINT8*)"Invalid PHY Number",                }},
    /* 说明：物理号类型错误 */
    {AT_DEVICE_PHYNUM_TYPE_ERR,           {(VOS_UINT8*)"549", (VOS_UINT8*)"Invalid PHY Type",                  }},
    /* 说明：无法读取平台信息 Platform Information Can' */
    {AT_DEVICE_PLATFORM_INFO_UNAVAILABLE, {(VOS_UINT8*)"550", (VOS_UINT8*)"Platform Infomation Can't Be Read", }},
    /* 说明：表示写数据时，数据保护未解锁, 无法操作保护数据 Not unlock, */
    {AT_DEVICE_DATA_LOCK,                 {(VOS_UINT8*)"551", (VOS_UINT8*)"Not unlock, write error",           }},
    /* 说明：输入的号段多于20段 */
    {AT_DEVICE_PLMN_OVER_20,              {(VOS_UINT8*)"552", (VOS_UINT8*)"Card Number More than 20",          }},
    /* 说明：<mnc-digital-num>输入超出范围 <mnc-digital-num> */
    {AT_DEVICE_MNC_NUM_INVALID,           {(VOS_UINT8*)"553", (VOS_UINT8*)"<mnc-digital-num> Enter Error",     }},
    /* 说明：版本信息读取失败 */
    {AT_DEVICE_GET_VER_ERR,               {(VOS_UINT8*)"554", (VOS_UINT8*)"Read Version Failure",              }},
    /* 说明：操作失败 */
    {AT_DEVICE_PORTLOCK_ERR,              {(VOS_UINT8*)"555", (VOS_UINT8*)"Operation Failure",                 }},
    /* 说明：输入密码错误 */
    {AT_DEVICE_PWD_ERR,                   {(VOS_UINT8*)"556", (VOS_UINT8*)"Password Error",                    }},
    /* 说明：超时错误 */
    {AT_DEVICE_TIMEOUT_ERR,               {(VOS_UINT8*)"557", (VOS_UINT8*)"Timeout",                           }},
    /* 说明：内存不足 */
    {AT_DEVICE_NOT_ENOUGH_MEMORY,         {(VOS_UINT8*)"558", (VOS_UINT8*)"No Memory",                         }},
    /* 说明：启用了SIMLOCK功能 */
    {AT_DEVICE_SIMM_LOCK,                 {(VOS_UINT8*)"559", (VOS_UINT8*)"SIM Lock Active",                   }},
    /* 说明：关闭发射机失败 */
    {AT_DEVICE_CLOSE_UL_CHAN_FAILURE,     {(VOS_UINT8*)"560", (VOS_UINT8*)"Close TX Transmitter Failure",      }},
    /* 说明：关闭接收机失败 */
    {AT_DEVICE_CLOSE_DL_CHAN_FAILURE,     {(VOS_UINT8*)"561", (VOS_UINT8*)"Close RX Transmitter Failure",      }},
    /* 说明：读取的NV项内容非法 */
    {AT_DEVICE_NV_DATA_INVALID,           {(VOS_UINT8*)"562", (VOS_UINT8*)"NV DATA INVALID",                   }},

    { AT_PERSONALIZATION_ERR_BEGIN, { VOS_NULL_PTR, VOS_NULL_PTR }},
    /* 说明：产线鉴权失败 */
    {AT_PERSONALIZATION_IDENTIFY_FAIL,              {(VOS_UINT8*)"301", (VOS_UINT8*)"Identify failure",                  } },
    /* 说明：文件签名校验失败 */
    {AT_PERSONALIZATION_SIGNATURE_FAIL,             {(VOS_UINT8*)"302", (VOS_UINT8*)"Signature verification failed",     } },
    /* 说明：通信端口锁密码错误 */
    {AT_PERSONALIZATION_DK_INCORRECT,               {(VOS_UINT8*)"303", (VOS_UINT8*)"Debug port password incorrect",     } },
    /* 说明：物理号长度错误 */
    {AT_PERSONALIZATION_PH_PHYNUM_LEN_ERROR,        {(VOS_UINT8*)"304", (VOS_UINT8*)"Phone physical number length error",} },
    /* 说明：物理号值非法 */
    {AT_PERSONALIZATION_PH_PHYNUM_VALUE_ERROR,      {(VOS_UINT8*)"305", (VOS_UINT8*)"Phone physical number value error", } },
    /* 说明：物理号类型错误 */
    {AT_PERSONALIZATION_PH_PHYNUM_TYPE_ERROR,       {(VOS_UINT8*)"306", (VOS_UINT8*)"Phone physical number type error",  } },
    /* 说明：RSA加密失败 */
    {AT_PERSONALIZATION_RSA_ENCRYPT_FAIL,           {(VOS_UINT8*)"307", (VOS_UINT8*)"RSA encryption failed",             } },
    /* 说明：RSA解密失败 */
    {AT_PERSONALIZATION_RSA_DECRYPT_FAIL,           {(VOS_UINT8*)"308", (VOS_UINT8*)"RSA decryption failed",             } },
    /* 说明：生成随机码失败 */
    {AT_PERSONALIZATION_GET_RAND_NUMBER_FAIL,       {(VOS_UINT8*)"309", (VOS_UINT8*)"Generate random number failed",     } },
    /* 说明：HUK写入失败 */
    {AT_PERSONALIZATION_WRITE_HUK_FAIL,             {(VOS_UINT8*)"310", (VOS_UINT8*)"Write HUK failed",                  } },
    /* 说明：Flash存储读写失败 */
    {AT_PERSONALIZATION_FLASH_ERROR,                {(VOS_UINT8*)"311", (VOS_UINT8*)"Flash error",                       } },
    /* 说明：其他错误 */
    {AT_PERSONALIZATION_OTHER_ERROR,                {(VOS_UINT8*)"312", (VOS_UINT8*)"Other error",                       } },
    /* 说明：OTA数据更新 */
    {AT_PERSONALIZATION_OTA_DATA_UPDATE,            {(VOS_UINT8*)"313", (VOS_UINT8*)"OTA data update",                   } },
    /* 说明：OTA随机数比较失败 */
    {AT_PERSONALIZATION_OTA_RAND_CMP_FAIL,          {(VOS_UINT8*)"314", (VOS_UINT8*)"OTA rand cmp fail",                 } },
    /* 说明：OTA支持等级校验失败 */
    {AT_PERSONALIZATION_OTA_SUPPORT_CAT_CHECK_FAIL, {(VOS_UINT8*)"315", (VOS_UINT8*)"OTA support cat check fail",        } },
    /* 说明：OTA支持mdoem数校验失败 */
    {AT_PERSONALIZATION_OTA_SUPPORT_MODEM_NUM_CHECK_FAIL, {(VOS_UINT8*)"316", (VOS_UINT8*)"OTA support modem num check fail", } },
    /* 说明：OTA支持数据配置校验失败 */
    {AT_PERSONALIZATION_OTA_SUPPORT_DATA_CFG_CHECK_FAIL,  {(VOS_UINT8*)"317", (VOS_UINT8*)"OTA support data cfg check fail",  } },
    /* 说明：OTA数据层校验失败 */
    {AT_PERSONALIZATION_OTA_DATA_LAYER_CHECK_FAIL,        {(VOS_UINT8*)"318", (VOS_UINT8*)"OTA data layer check fail",        } },
    /* 说明：OTA数据长度校验失败 */
    {AT_PERSONALIZATION_OTA_DATA_SIZE_CHECK_FAIL,         {(VOS_UINT8*)"319", (VOS_UINT8*)"OTA data size check fail",         } },
    /* 说明：完整性类型校验失败 */
    {AT_PERSONALIZATION_INTEGRITY_CHECK_TYPE_FAIL,        {(VOS_UINT8*)"320", (VOS_UINT8*)"Integrity check type fail",        } },
    { AT_PERSONALIZATION_ERR_END, { VOS_NULL_PTR, VOS_NULL_PTR }},

    { AT_DEVICE_ERROR_END, { VOS_NULL_PTR, VOS_NULL_PTR }},

    { AT_CMS_ERROR_ENUM_BEGAIN, { TAF_NULL_PTR, TAF_NULL_PTR }},
    /* 说明(0~127)：请参见“3GPP TS 24.011 [6]”。 */
    { AT_CMS_UNASSIGNED_UNALLOCATED_NUMBER, { (TAF_UINT8 *)"1  ", (TAF_UINT8 *)"Unassigned (unallocated) number" }},
    { AT_CMS_OPERATOR_DETERMINED_BARRING, { (TAF_UINT8 *)"8  ", (TAF_UINT8 *)"Operator determined barring" }},
    { AT_CMS_CALL_BARRED, { (TAF_UINT8 *)"10 ", (TAF_UINT8 *)"Call barred" }},
    { AT_CMS_SHORT_MESSAGE_TRANSFER_REJECTED, { (TAF_UINT8 *)"21 ", (TAF_UINT8 *)"Short message transfer rejected" }},
    { AT_CMS_DESTINATION_OUT_OF_SERVICE, { (TAF_UINT8 *)"27 ", (TAF_UINT8 *)"Destination out of service" }},
    { AT_CMS_UNIDENTIFIED_SUBSCRIBER, { (TAF_UINT8 *)"28 ", (TAF_UINT8 *)"Unidentified subscriber" }},
    { AT_CMS_FACILITY_REJECTED, { (TAF_UINT8 *)"29 ", (TAF_UINT8 *)"Facility rejected" }},
    { AT_CMS_UNKNOWN_SUBSCRIBER, { (TAF_UINT8 *)"30 ", (TAF_UINT8 *)"Unknown subscriber" }},
    { AT_CMS_NETWORK_OUT_OF_ORDER, { (TAF_UINT8 *)"38 ", (TAF_UINT8 *)"Network out of order" }},
    { AT_CMS_TEMPORARY_FAILURE, { (TAF_UINT8 *)"41 ", (TAF_UINT8 *)"Temporary failure" }},
    { AT_CMS_CONGESTION, { (TAF_UINT8 *)"42 ", (TAF_UINT8 *)"Congestion" }},
    { AT_CMS_RESOURCES_UNAVAILABLE_UNSPECIFIED, { (TAF_UINT8 *)"47 ", (TAF_UINT8 *)"Resources unavailable, unspecified" }},
    { AT_CMS_REQUESTED_FACILITY_NOT_SUBSCRIBED, { (TAF_UINT8 *)"50 ", (TAF_UINT8 *)"Requested facility not subscribed" }},
    { AT_CMS_REQUESTED_FACILITY_NOT_IMPLEMENTED, { (TAF_UINT8 *)"69 ", (TAF_UINT8 *)"Requested facility not implemented" }},
    { AT_CMS_INVALID_SHORT_MESSAGE_TRANSFER_REFERENCE_VALUE, { (TAF_UINT8 *)"81 ", (TAF_UINT8 *)"Invalid short message transfer reference value" }},
    { AT_CMS_INVALID_MESSAGE_UNSPECIFIED, { (TAF_UINT8 *)"95 ", (TAF_UINT8 *)"Invalid message, unspecified" }},
    { AT_CMS_INVALID_MANDATORY_INFORMATION, { (TAF_UINT8 *)"96 ", (TAF_UINT8 *)"Invalid mandatory information" }},
    { AT_CMS_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED, { (TAF_UINT8 *)"97 ", (TAF_UINT8 *)"Message type non existent or not implemented" }},
    { AT_CMS_MESSAGE_NOT_COMPATIBLE_WITH_SHORT_MESSAGE_PROTOCOL_STATE, { (TAF_UINT8 *)"98 ", (TAF_UINT8 *)"Message not compatible with short message protocol state" }},
    { AT_CMS_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED, { (TAF_UINT8 *)"99 ", (TAF_UINT8 *)"Information element non existent or not implemented" }},
    { AT_CMS_PROTOCOL_ERROR_UNSPECIFIED, { (TAF_UINT8 *)"111", (TAF_UINT8 *)"Protocol error, unspecified" }},
    { AT_CMS_INTERWORKING_UNSPECIFIED, { (TAF_UINT8 *)"127", (TAF_UINT8 *)"Interworking, unspecified" }},

    /* 说明(128~255)：请参见“3GPP TS 24.011 [3]”。 */
    { AT_CMS_TELEMATIC_INTERWORKING_NOT_SUPPORTED, { (TAF_UINT8 *)"128", (TAF_UINT8 *)"Telematic interworking not supported" }},
    { AT_CMS_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED, { (TAF_UINT8 *)"129", (TAF_UINT8 *)"Short message Type 0 not supported" }},
    { AT_CMS_CANNOT_REPLACE_SHORT_MESSAGE, { (TAF_UINT8 *)"130", (TAF_UINT8 *)"Cannot replace short message" }},
    { AT_CMS_UNSPECIFIED_TPPID_ERROR, { (TAF_UINT8 *)"143", (TAF_UINT8 *)"Unspecified TPPID error" }},
    { AT_CMS_DATA_CODING_SCHEME_ALPHABET_NOT_SUPPORTED, { (TAF_UINT8 *)"144", (TAF_UINT8 *)"Data coding scheme (alphabet) not supported" }},
    { AT_CMS_MESSAGE_CLASS_NOT_SUPPORTED, { (TAF_UINT8 *)"145", (TAF_UINT8 *)"Message class not supported" }},
    { AT_CMS_UNSPECIFIED_TPDCS_ERROR, { (TAF_UINT8 *)"159", (TAF_UINT8 *)"Unspecified TPDCS error" }},
    { AT_CMS_COMMAND_CANNOT_BE_ACTIONED, { (TAF_UINT8 *)"160", (TAF_UINT8 *)"Command cannot be actioned" }},
    { AT_CMS_COMMAND_UNSUPPORTED, { (TAF_UINT8 *)"161", (TAF_UINT8 *)"Command unsupported" }},
    { AT_CMS_UNSPECIFIED_TPCOMMAND_ERROR, { (TAF_UINT8 *)"175", (TAF_UINT8 *)"Unspecified TPCommand erro" }},
    { AT_CMS_TPDU_NOT_SUPPORTED, { (TAF_UINT8 *)"176", (TAF_UINT8 *)"TPDU not supported" }},
    { AT_CMS_SC_BUSY, { (TAF_UINT8 *)"192", (TAF_UINT8 *)"SC busy" }},
    { AT_CMS_NO_SC_SUBSCRIPTION, { (TAF_UINT8 *)"193", (TAF_UINT8 *)"No SC subscription" }},
    { AT_CMS_SC_SYSTEM_FAILURE, { (TAF_UINT8 *)"194", (TAF_UINT8 *)"SC system failure" }},
    { AT_CMS_INVALID_SME_ADDRESS, { (TAF_UINT8 *)"195", (TAF_UINT8 *)"Invalid SME address" }},
    { AT_CMS_DESTINATION_SME_BARRED, { (TAF_UINT8 *)"196", (TAF_UINT8 *)"Destination SME barred" }},
    { AT_CMS_SM_REJECTEDDUPLICATE_SM, { (TAF_UINT8 *)"197", (TAF_UINT8 *)"SM RejectedDuplicate SM" }},
    { AT_CMS_TPVPF_NOT_SUPPORTED, { (TAF_UINT8 *)"198", (TAF_UINT8 *)"TPVPF not supported" }},
    { AT_CMS_TPVP_NOT_SUPPORTED, { (TAF_UINT8 *)"199", (TAF_UINT8 *)"TPVP not supported" }},
    { AT_CMS_SIM_SMS_STORAGE_FULL, { (TAF_UINT8 *)"208", (TAF_UINT8 *)"(U)SIM SMS storage full" }},
    { AT_CMS_NO_SMS_STORAGE_CAPABILITY_IN_SIM, { (TAF_UINT8 *)"209", (TAF_UINT8 *)"No SMS storage capability in (U)SIM" }},
    { AT_CMS_ERROR_IN_MS, { (TAF_UINT8 *)"210", (TAF_UINT8 *)"Error in MS" }},
    { AT_CMS_MEMORY_CAPACITY_EXCEEDED, { (TAF_UINT8 *)"211", (TAF_UINT8 *)"Memory Capacity Exceeded" }},
    { AT_CMS_SIM_APPLICATION_TOOLKIT_BUSY, { (TAF_UINT8 *)"212", (TAF_UINT8 *)"(U)SIM Application Toolkit Busy" }},
    { AT_CMS_SIM_DATA_DOWNLOAD_ERROR, { (TAF_UINT8 *)"213", (TAF_UINT8 *)"(U)SIM data download error" }},
    { AT_CMS_UNSPECIFIED_ERROR_CAUSE, { (TAF_UINT8 *)"255", (TAF_UINT8 *)"Unspecified error cause" }},

    /* 说明：ME故障。 */
    { AT_CMS_ME_FAILURE, { (TAF_UINT8 *)"300", (TAF_UINT8 *)"ME failure" }},
    /* 说明：ME的SMS业务保留。 */
    { AT_CMS_SMS_SERVICE_OF_ME_RESERVED, { (TAF_UINT8 *)"301", (TAF_UINT8 *)"SMS service of ME reserved" }},
    /* 说明：操作不允许。 */
    { AT_CMS_OPERATION_NOT_ALLOWED, { (TAF_UINT8 *)"302", (TAF_UINT8 *)"operation not allowed" }},
    /* 说明：操作不支持。 */
    { AT_CMS_OPERATION_NOT_SUPPORTED, { (TAF_UINT8 *)"303", (TAF_UINT8 *)"operation not supported" }},
    /* "invalid PDU mode parameter" */
    /* 说明：PDU模式下无效的参数。 */
    { AT_CMS_INVALID_PDU_MODE_PARAMETER, { (TAF_UINT8 *)"304", (TAF_UINT8 *)"304" }},
    /* "invalid text mode parameter" */
    /* 说明：TEXT模式下无效的参数。 */
    { AT_CMS_INVALID_TEXT_MODE_PARAMETER, { (TAF_UINT8 *)"305", (TAF_UINT8 *)"305" }},
    /* 说明：SIM卡未插入。 */
    { AT_CMS_U_SIM_NOT_INSERTED, { (TAF_UINT8 *)"310", (TAF_UINT8 *)"(U)SIM not inserted" }},
    /* 说明：需要输入SIM PIN码。 */
    { AT_CMS_U_SIM_PIN_REQUIRED, { (TAF_UINT8 *)"311", (TAF_UINT8 *)"(U)SIM PIN required" }},
    /* 说明：需要输入PH-SIM PIN码。 */
    { AT_CMS_PH_U_SIM_PIN_REQUIRED, { (TAF_UINT8 *)"312", (TAF_UINT8 *)"PH-(U)SIM PIN required" }},
    /* 说明：SIM卡故障。 */
    { AT_CMS_U_SIM_FAILURE, { (TAF_UINT8 *)"313", (TAF_UINT8 *)"313" }},
    /* 说明：SIM卡遇忙。 */
    { AT_CMS_U_SIM_BUSY, { (TAF_UINT8 *)"314", (TAF_UINT8 *)"(U)SIM busy" }},
    /* 说明：SIM卡错误。 */
    { AT_CMS_U_SIM_WRONG, { (TAF_UINT8 *)"315", (TAF_UINT8 *)"(U)SIM wrong" }},
    /* 说明：需要输入SIM PUK码。 */
    { AT_CMS_U_SIM_PUK_REQUIRED, { (TAF_UINT8 *)"316", (TAF_UINT8 *)"(U)SIM PUK required" }},
    /* 说明：需要输入SIM PIN2码。 */
    { AT_CMS_U_SIM_PIN2_REQUIRED, { (TAF_UINT8 *)"317", (TAF_UINT8 *)"(U)SIM PIN2 required" }},
    /* 说明：需要输入SIM PUK2码。 */
    { AT_CMS_U_SIM_PUK2_REQUIRED, { (TAF_UINT8 *)"318", (TAF_UINT8 *)"(U)SIM PUK2 required" }},
    /* 说明：存储故障。 */
    { AT_CMS_MEMORY_FAILURE, { (TAF_UINT8 *)"320", (TAF_UINT8 *)"memory failure" }},
    /* 说明：无效存储索引。 */
    { AT_CMS_INVALID_MEMORY_INDEX, { (TAF_UINT8 *)"321", (TAF_UINT8 *)"invalid memory index" }},
    /* 说明：存储已满。 */
    { AT_CMS_MEMORY_FULL, { (TAF_UINT8 *)"322", (TAF_UINT8 *)"memory full" }},
    /* "SMSC address unknown" */
    /* 说明：短信中心地址未知。 */
    { AT_CMS_SMSC_ADDRESS_UNKNOWN, { (TAF_UINT8 *)"330", (TAF_UINT8 *)"330" }},
    /* 说明：无网络服务。 */
    { AT_CMS_NO_NETWORK_SERVICE, { (TAF_UINT8 *)"331", (TAF_UINT8 *)"no network service" }},
    /* 说明：网络超时。 */
    { AT_CMS_NETWORK_TIMEOUT, { (TAF_UINT8 *)"332", (TAF_UINT8 *)"network timeout" }},
    /* 说明：无预期的+CNMA确认。 */
    { AT_CMS_NO_CNMA_ACKNOWLEDGEMENT_EXPECTED, { (TAF_UINT8 *)"340", (TAF_UINT8 *)"no +CNMA acknowledgement expected" }},
    /* 说明：短信接收方号码FDN检查失败或者短信接收方号码和短信中心号码FDN均检查失败 */
    { AT_CMS_FDN_DEST_ADDR_FAILED, { (TAF_UINT8 *)"341", (TAF_UINT8 *)"FDN Failed" }},
    /* 说明：短信中心号码FDN检查失败 */
    { AT_CMS_FDN_SERVICE_CENTER_ADDR_FAILED, { (TAF_UINT8 *)"342", (TAF_UINT8 *)"Service Centre Address FDN failed" }},
    /* 说明：MO SMS Control检查失败 */
    { AT_CMS_MO_SMS_CONTROL_FAILED, { (TAF_UINT8 *)"343", (TAF_UINT8 *)"MO SMS Control Failed" }},
    /* "unknown error" */
    /* 说明：未知错误。 */
    { AT_CMS_UNKNOWN_ERROR, { (TAF_UINT8 *)"500", (TAF_UINT8 *)"500" }},

    { AT_CMS_ERROR_ENUM_END, { TAF_NULL_PTR, TAF_NULL_PTR }},

    { AT_CMOLRE_ERR_ENUM_BEGIN, { VOS_NULL_PTR, VOS_NULL_PTR }},
    { AT_CMOLRE_METHOD_NOT_SUPPORTED, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"Method not supported" }},
    { AT_CMOLRE_ADDITIONAL_ASSIS_DATA_REQIRED, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"Additional assistance data required" }},
    { AT_CMOLRE_NOT_ENOUGH_SATELLITES, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"Not enough satellites" }},
    { AT_CMOLRE_UE_BUSY, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"UE busy" }},
    { AT_CMOLRE_NETWORK_ERROR, { (VOS_UINT8 *)"4", (VOS_UINT8 *)"Network error" }},
    { AT_CMOLRE_TOO_MANY_CONNECTIONS, { (VOS_UINT8 *)"5", (VOS_UINT8 *)"Failed to open internet connection, too many connections" }},
    { AT_CMOLRE_TOO_MANY_USERS, { (VOS_UINT8 *)"6", (VOS_UINT8 *)"Failed to open internet connection, too many users" }},
    { AT_CMOLRE_FAILURE_DUE_TO_HANDOVER, { (VOS_UINT8 *)"7", (VOS_UINT8 *)"Failure due to handover" }},
    { AT_CMOLRE_INTERNET_CONN_FAILURE, { (VOS_UINT8 *)"8", (VOS_UINT8 *)"Internet connection failure" }},
    { AT_CMOLRE_MEMORY_ERROR, { (VOS_UINT8 *)"9", (VOS_UINT8 *)"Memory error" }},
    { AT_CMOLRE_UNKNOWN_ERROR, { (VOS_UINT8 *)"255", (VOS_UINT8 *)"Unknown error" }},
    { AT_CMOLRE_ERR_ENUM_END, { VOS_NULL_PTR, VOS_NULL_PTR }},

    { AT_DIAG_ENUM_BEGIN, { VOS_NULL_PTR, VOS_NULL_PTR }},
    { AT_DIAG_USB_NOT_SUPPORT_CPS, { (VOS_UINT8 *)"0", (VOS_UINT8 *)"DIAG_USB_NOT_SUPPORT_CPS" }},
    { AT_DIAG_USB_NOT_SUPPORT_CFG, { (VOS_UINT8 *)"1", (VOS_UINT8 *)"DIAG_USB_NOT_SUPPORT_CFG" }},
    { AT_DIAG_CPS_SET_ERROR, { (VOS_UINT8 *)"2", (VOS_UINT8 *)"DIAG_CPS_SET_ERROR" }},
    { AT_DIAG_GET_PORT_ERROR, { (VOS_UINT8 *)"3", (VOS_UINT8 *)"DIAG_GET_PORT_ERROR" }},
    { AT_DIAG_VCOM_SET_CPS_ERROR, { (VOS_UINT8 *)"4", (VOS_UINT8 *)"DIAG_VCOM_SET_CPS_ERROR" }},
    { AT_DIAG_VCOM_SET_CFG_ERROR, { (VOS_UINT8 *)"5", (VOS_UINT8 *)"DIAG_VCOM_SET_CFG_ERROR" }},
    { AT_DIAG_VCOM_SET_NOCPS_ERROR, { (VOS_UINT8 *)"6", (VOS_UINT8 *)"DIAG_VCOM_SET_NOCPS_ERROR" }},
    { AT_DIAG_CFG_SET_ERROR, { (VOS_UINT8 *)"7", (VOS_UINT8 *)"AT_DIAG_CFG_SET_ERROR" }},
    { AT_DIAG_GET_PORT_NOT_USB_OR_VCOM, { (VOS_UINT8 *)"8", (VOS_UINT8 *)"AT_DIAG_GET_PORT_NOT_USB_OR_VCOM" }},
    { AT_DIAG_ENUM_END, { VOS_NULL_PTR, VOS_NULL_PTR }},
};

static const AT_SubSysModeConnectRatePair g_subSysModeConnectRateArr[] = {
    { TAF_SYS_SUBMODE_GSM, At_GetGsmConnectRate },   { TAF_SYS_SUBMODE_GPRS, At_GetGprsConnectRate },
    { TAF_SYS_SUBMODE_EDGE, At_GetEdgeConnectRate }, { TAF_SYS_SUBMODE_WCDMA, At_GetWcdmaConnectRate },
    { TAF_SYS_SUBMODE_HSDPA, At_GetDpaConnectRate }, { TAF_SYS_SUBMODE_HSDPA_HSUPA, At_GetDpaConnectRate },
};

/*
 * 3 函数、变量声明
 */



VOS_VOID AT_DisplayResultData(VOS_UINT8 indexNum, VOS_UINT16 len)
{
    VOS_UINT32                  ret;
    DMS_PortIdUint16            portId;
    DMS_PortModeUint8           mode;
    DMS_PortDataStateUint8      dataState;

    portId    = g_atClientTab[indexNum].portNo;
    mode      = DMS_PORT_GetMode(portId);
    dataState = DMS_PORT_GetDataState(portId);

    /* 如果是数传状态，直接返回 */
    if (dataState == DMS_PORT_DATA_STATE_START) {
        AT_LOG1("At_SendResultData DataState:", dataState);
        return;
    }

    if ((dataState == DMS_PORT_DATA_STATE_STOP) && (mode == DMS_PORT_MODE_ONLINE_DATA)) {
        DMS_PORT_SetDataState(portId, DMS_PORT_DATA_STATE_START);
    }

    ret = DMS_PORT_SendData(portId, g_atSendDataBuffer, len);

    if (ret != VOS_OK) {
        AT_ERR_LOG1("ERROR:data send fail,client id is ", indexNum);
    }
}

VOS_VOID AT_DisplaySelResultData(VOS_UINT16 len, VOS_UINT8 indexNum)
{
    DMS_PortModeUint8   mode;

    mode = DMS_PORT_GetMode(g_atClientTab[indexNum].portNo);

    if ((mode == DMS_PORT_MODE_CMD) || (mode == DMS_PORT_MODE_ONLINE_CMD)) {
        /* UART定制特性开启，UART通道支持主动命令上报 */
#if (FEATURE_IOT_UART_CUST == FEATURE_OFF)
        if (indexNum == AT_CLIENT_ID_UART) {
            return;
        }
#endif

        (VOS_VOID)DMS_PORT_SendData(g_atClientTab[indexNum].portNo, g_atSendDataBuffer, len);
    }
}

VOS_VOID At_BufferorSendResultData(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 length)
{
    errno_t    memResult;
    VOS_UINT32 bufferLength;
    VOS_UINT8 *buffer = VOS_NULL_PTR;

    if (length == 0) {
        return;
    }

    bufferLength = (VOS_UINT32)g_atCombineSendData[indexNum].bufLen + (VOS_UINT32)length;

    /* 若缓冲区存储空间不够用则将当前已缓存信息输出 */
    if (bufferLength > AT_CMD_OUTPUT_MAX_LEN) {
        At_SendResultData(indexNum, g_atCombineSendData[indexNum].buffer, g_atCombineSendData[indexNum].bufLen);
        g_atCombineSendData[indexNum].bufLen = 0;
    }

    /* 缓存待输出的新数据 */
    buffer = g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen;
    if (g_atCombineSendData[indexNum].bufLen < AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN) {
        memResult = memcpy_s(buffer, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                             data, length);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                            length);
    }
    g_atCombineSendData[indexNum].bufLen += length;
}

VOS_VOID AT_SendBroadCastResultData(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8     count;
    ModemIdUint16 broadCastModemId;
    ModemIdUint16 curModemId = MODEM_ID_0;
    VOS_UINT32    rslt;
    errno_t       memResult;
    AT_ClientCtx *atClientCtx = VOS_NULL_PTR;

    switch (indexNum) {
        case AT_BROADCAST_CLIENT_INDEX_MODEM_0:
            broadCastModemId = MODEM_ID_0;
            break;
        case AT_BROADCAST_CLIENT_INDEX_MODEM_1:
            broadCastModemId = MODEM_ID_1;
            break;
        case AT_BROADCAST_CLIENT_INDEX_MODEM_2:
            broadCastModemId = MODEM_ID_2;
            break;
        default:
            broadCastModemId = MODEM_ID_0;
            break;
    }

    for (count = 0; count < AT_MAX_CLIENT_NUM; count++) {
        atClientCtx = AT_GetClientCtxAddr(count);

        /* 该端口未使用 */
        if (g_atClientTab[count].used == AT_CLIENT_NULL) {
            continue;
        }

        /* 该端口不允许上报 */
        if (atClientCtx->clientConfiguration.reportFlg == VOS_FALSE) {
            continue;
        }

        rslt = AT_GetModemIdFromClient(count, &curModemId);

        if (rslt != VOS_OK) {
            continue;
        }

        /* 判断该端口是否属于要广播的Modem */
        if (broadCastModemId != curModemId) {
            continue;
        }

        if (g_atClientTab[count].indMode == AT_IND_MODE) {
            /* 为了保证发送的数据是4字节对齐格式，故做一次拷贝 */
            if ((len > 0) && (len <= AT_CMD_MAX_LEN)) {
                memResult = memcpy_s(g_atSendDataBuffer, AT_CMD_MAX_LEN, data, len);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN, len);
            }
            AT_DisplaySelResultData(len, count);
        }
    }
}

VOS_VOID AT_FlushSmsIndication(VOS_VOID)
{
    AT_ModemSmsCtx     *smsCtx      = VOS_NULL_PTR;
    AT_PortBuffCfgInfo *portBuffCfg = VOS_NULL_PTR;
    VOS_UINT16          i;
    VOS_UINT32          clientId;
    ModemIdUint16       modemId;
    VOS_UINT32          rslt;
    VOS_UINT8           modemFlag[MODEM_ID_BUTT];
    VOS_UINT32          j = 0;

    portBuffCfg = AT_GetPortBuffCfgInfo();
    memset_s(modemFlag, sizeof(modemFlag), 0x00, sizeof(modemFlag));

    if (portBuffCfg->num > AT_MAX_CLIENT_NUM) {
        portBuffCfg->num = AT_MAX_CLIENT_NUM;
    }

    /* 根据clientId查找需要flush 短信modem id */
    for (i = 0; i < portBuffCfg->num; i++) {
        clientId = portBuffCfg->usedClientId[i];
        rslt     = AT_GetModemIdFromClient((VOS_UINT8)clientId, &modemId);
        if (rslt != VOS_OK) {
            AT_ERR_LOG("AT_FlushSmsIndication: Get modem id fail");
            continue;
        }

        modemFlag[modemId] = VOS_TRUE;
    }

    /* flush SMS */
    for (i = 0; i < MODEM_ID_BUTT; i++) {
        if (modemFlag[i] == VOS_TRUE) {
            smsCtx = AT_GetModemSmsCtxAddrFromClientId(i);

            for (j = 0; j < AT_BUFFER_SMT_EVENT_MAX; j++) {
                if (smsCtx->smtBuffer.used[j] == AT_MSG_BUFFER_USED) {
                    At_ForwardMsgToTe(MN_MSG_EVT_DELIVER, &(smsCtx->smtBuffer.event[j]));
                }
            }

            memset_s(&(smsCtx->smtBuffer), sizeof(smsCtx->smtBuffer), 0x00, sizeof(smsCtx->smtBuffer));
        }
    }
}

VOS_VOID At_SendResultData(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    errno_t memResult;
    g_atSendDataBuff.bufLen = 0;

    /* 参数检查 */
    if ((len == 0) || (len >= AT_CMD_MAX_LEN)) {
        AT_LOG1("At_SendResultData usLen:", len);
        return;
    }

    data[len] = 0;

    if (AT_GetPrivacyFilterEnableFlg() == VOS_TRUE) {
        AT_LOG1("At_SendResultData:LogPrivacyFlag is ", VOS_TRUE);
    }

    AT_LOG1("At_SendResultData:", len);


    /* 通过ucIndex获取端口类型 */
    AT_MntnTraceCmdResult(indexNum, data, len);

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_SendBroadCastResultData(indexNum, data, len);
        AT_MntnTraceRPTPORT();
    } else {
        if (indexNum >= AT_MAX_CLIENT_NUM) {
            AT_LOG1("At_SendResultData ucIndex:", indexNum);
            return;
        }

        /* 注意遗留问题: 此处需要增加链路类型判断，数传和命令状态数据不能混在一起 */
        if (g_atClientTab[indexNum].used == AT_CLIENT_NULL) {
            AT_LOG1("At_SendResultData ucUsed:", g_atClientTab[indexNum].used);
            AT_ParseCmdOver(indexNum);
            return;
        }

        /* 为了保证发送的数据是4字节对齐格式，故做一次拷贝 */
        memResult = memcpy_s(g_atSendDataBuffer, AT_CMD_MAX_LEN, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN, len);

        AT_DisplayResultData(indexNum, len);

        /* 判断是否需要Flash 短信,若需要，则将短信flash出去 */
        if (AT_IsClientBlock() == VOS_FALSE) {
            AT_FlushSmsIndication();
        }
    }
}

/*
 * Description: 根据命令的返回结果来判断后续命令的处理
 *              1.若后续无命令，则直接返回
 *              2.若返回结果为错误，但后续还有命令，则终止后续命令的处理，并做相应的
 *                清空操作
 *              3.若返回结果为正确，且后续还有命令，则继续执行后续命令
 * History:
 *  1.Date: 2009-05-30
 *    Modification: Created function
 */
TAF_UINT32 At_JudgeCombineCmdSubSequentProcess(TAF_UINT8 indexNum, TAF_UINT32 returnCode)
{
    /* 判读是否有未处理完成的命令 */
    if (g_parseContext[indexNum].combineCmdInfo.totalNum == 0) {
        /*
         * 该分支有如下两种情况:
         * 1.无待处理命令，属于主动上报
         * 2.解析组合AT命令(或仅为独立AT命令)失败
         */
        return AT_COM_CMD_SUB_PROC_FINISH;
    }

    /* 判断是否为最后一个AT命令 */
    if ((g_parseContext[indexNum].combineCmdInfo.procNum + 1) == g_parseContext[indexNum].combineCmdInfo.totalNum) {
        return AT_COM_CMD_SUB_PROC_FINISH;
    }

    switch (returnCode) {
        case AT_SUCCESS:
        case AT_OK:
            return AT_COM_CMD_SUB_PROC_CONTINUE;
        case AT_FAILURE:
        case AT_CMD_NOT_SUPPORT:
        case AT_TOO_MANY_PARA:
        case AT_CONNECT:
        case AT_RING:
        case AT_NO_CARRIER:
        case AT_ERROR:
        case AT_NO_DIALTONE:
        case AT_BUSY:
        case AT_NO_ANSWER:
            return AT_COM_CMD_SUB_PROC_ABORT;
        default:
            if (((returnCode > AT_CME_ERROR_ENUM_BEGAIN) && (returnCode < AT_CME_ERROR_ENUM_END)) ||
                ((returnCode > AT_CMS_ERROR_ENUM_BEGAIN) && (returnCode < AT_CMS_ERROR_ENUM_END)) ||
                (returnCode > AT_TOO_MANY_PARA)) {
                return AT_COM_CMD_SUB_PROC_ABORT;
            } else {
                return AT_COM_CMD_SUB_PROC_CONTINUE;
            }
    }
}

VOS_UINT32 AT_GetReturnCodeId(VOS_UINT32 returnCode, VOS_UINT32 *indexNum)
{
    VOS_UINT32 i;

    for (i = 0; i < sizeof(g_atReturnCodeTab) / sizeof(g_atReturnCodeTab[0]); i++) {
        if (returnCode == g_atReturnCodeTab[i].index) {
            *indexNum = i;
            return VOS_OK;
        }
    }

    AT_ERR_LOG("AT_GetResultDataId, ReturnCode is out of g_atReturnCodeTab!");
    return VOS_ERR;
}

VOS_VOID AT_FormatAtCmdNotSupportResult(VOS_UINT8 indexNum, AT_RreturnCodeUint32 returnCode)
{
    VOS_UINT16 length = 0;
    VOS_UINT8 *dataAt = VOS_NULL_PTR;
    VOS_UINT32 tmp = 0;
    errno_t    memResult;

    dataAt = g_atSndCodeAddress;

    memResult = memcpy_s((dataAt + length), AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);

    length += AT_CRLF_STR_LEN;

    tmp = VOS_StrLen((VOS_CHAR *)g_atCmdNotSupportStr);

    if (tmp > 0) {
        /* length长度为AT_CRLF_STR_LEN，确认减法不会反转 */
        memResult = memcpy_s((dataAt + length), AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atCmdNotSupportStr, tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, tmp);
    }

    length += (VOS_UINT16)tmp;

    if ((AT_CRLF_STR_LEN + length) <= AT_SEND_CODE_ADDR_MAX_LEN) {
        memResult = memcpy_s((dataAt + length), AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);
    }

    length += AT_CRLF_STR_LEN;

    if (((g_atCombineSendData[indexNum].bufLen + length <= AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
        memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                             AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                             dataAt, length);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen, length);
        g_atCombineSendData[indexNum].bufLen += length;
    }

    At_SendResultData(indexNum, g_atCombineSendData[indexNum].buffer, g_atCombineSendData[indexNum].bufLen);
}

VOS_VOID AT_FormatAtTooManyParaResult(VOS_UINT8 indexNum, AT_RreturnCodeUint32 returnCode)
{
    VOS_UINT16 length = 0;
    VOS_UINT8 *dataAt = VOS_NULL_PTR;
    VOS_UINT32 tmp = 0;
    errno_t    memResult;

    dataAt = g_atSndCodeAddress;

    memResult = memcpy_s((dataAt + length), AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);

    length += AT_CRLF_STR_LEN;

    tmp = VOS_StrLen((VOS_CHAR *)g_atTooManyParaStr);

    if (tmp > 0) {
        /* length长度为AT_CRLF_STR_LEN，确认减法不会反转 */
        memResult = memcpy_s((dataAt + length), AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atTooManyParaStr, tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, tmp);
    }

    length += (VOS_UINT16)tmp;

    if ((AT_CRLF_STR_LEN + length) <= AT_SEND_CODE_ADDR_MAX_LEN ) {
        memResult = memcpy_s((dataAt + length), AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);
    }

    length += AT_CRLF_STR_LEN;

    if (((g_atCombineSendData[indexNum].bufLen + length <= AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
        memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                             AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                             dataAt, length);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen, length);
        g_atCombineSendData[indexNum].bufLen += length;
    }

    At_SendResultData(indexNum, g_atCombineSendData[indexNum].buffer, g_atCombineSendData[indexNum].bufLen);
}

VOS_VOID AT_FormatAtAbortResult(VOS_UINT8 indexNum, AT_RreturnCodeUint32 returnCode)
{
    VOS_UINT8   *dataAt = VOS_NULL_PTR;
    VOS_UINT16   length = 0;
    VOS_UINT32   tmp = 0;
    errno_t      memResult;

    dataAt = g_atSndCodeAddress;

    memResult = memcpy_s(dataAt + length, AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);

    length += AT_CRLF_STR_LEN;

    /* 获取打断回复 */
    tmp = VOS_StrLen((VOS_CHAR *)AT_GetAbortRspStr());
    if (tmp > 0) {
        /* length长度为常量AT_CRLF_STR_LEN，确认减法不会发生反转 */
        memResult = memcpy_s(dataAt + length, AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)AT_GetAbortRspStr(), tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, tmp);
    }
    length += (VOS_UINT16)tmp;

    /* 在打断回复后面，加上回车换行 */
    if ((AT_CRLF_STR_LEN + length) <= AT_SEND_CODE_ADDR_MAX_LEN) {
        memResult = memcpy_s(dataAt + length, AT_SEND_CODE_ADDR_MAX_LEN - length, (VOS_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);
    }

    length += AT_CRLF_STR_LEN;

    /* 把打断回复和回车换行等输出信息保存到全局变量中 */
    if (((g_atCombineSendData[indexNum].bufLen + length <= AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
        memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                             (sizeof(g_atCombineSendData[indexNum].buffer) - g_atCombineSendData[indexNum].bufLen),
                              dataAt, length);
        TAF_MEM_CHK_RTN_VAL(memResult,
                            (sizeof(g_atCombineSendData[indexNum].buffer) - g_atCombineSendData[indexNum].bufLen),
                            length);
        g_atCombineSendData[indexNum].bufLen += length;
    }

    /* 发送AT命令的数据给APP */
    At_SendResultData(indexNum, g_atCombineSendData[indexNum].buffer, g_atCombineSendData[indexNum].bufLen);
}

VOS_VOID At_ProcWaitSmsOrXmlInputReturnCode(VOS_UINT16 *length, VOS_UINT32 returnCodeIndex, VOS_UINT8 indexNum)
{
    VOS_UINT8 *dataAt3 = VOS_NULL_PTR;
    VOS_UINT32 tmp = 0;
    errno_t    memResult;

    dataAt3 = g_atSndCodeAddress;

    tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[g_atVType]);
    if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
        memResult = memcpy_s(dataAt3 + *length, AT_SEND_CODE_ADDR_MAX_LEN - *length,
                             (TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[0],
                             tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
    }
    *length += (TAF_UINT16)tmp;

    if (((g_atCombineSendData[indexNum].bufLen + *length <= AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
        if (*length > 0) {
            memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                                 AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                                 dataAt3,
                                 *length);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen, *length);
        }
        g_atCombineSendData[indexNum].bufLen += *length;
    }

    At_SendResultData(indexNum, dataAt3, *length);

    /* 清除g_atCombineSendData的内容 */
    (VOS_VOID)memset_s(&g_atCombineSendData[indexNum], sizeof(AT_SEND_DataBuffer), 0x00, sizeof(AT_SEND_DataBuffer));
}

VOS_UINT32 At_ProcCmeErrorReturnCodeAccordingToAtCmeeType(VOS_UINT32 returnCodeIndex, VOS_UINT16 *length)
{
    VOS_UINT8 *dataAt3 = VOS_NULL_PTR;
    VOS_UINT32 atErrIndex = 0;
    VOS_UINT32 atErrRslt;
    VOS_UINT32 tmp = 0;
    errno_t    memResult;

    dataAt3    = g_atSndCodeAddress;

    switch (g_atCmeeType) {
        case AT_CMEE_ONLY_ERROR:
            atErrRslt = AT_GetReturnCodeId(AT_ERROR, &atErrIndex);

            if (atErrRslt != VOS_OK) {
                AT_ERR_LOG("At_ProcCmeErrorReturnCodeAccordingToAtCmeeType: AT ERROR index is err!");
                return VOS_FALSE;
            }
            tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[atErrIndex].result[g_atVType]);
            if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
                memResult = memcpy_s(dataAt3 + *length, AT_SEND_CODE_ADDR_MAX_LEN - *length,
                                     (TAF_CHAR *)g_atReturnCodeTab[atErrIndex].result[g_atVType], tmp);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
            }
            *length += (TAF_UINT16)tmp;
            break;

        case AT_CMEE_ERROR_CODE:
            if (*length < AT_SEND_CODE_ADDR_MAX_LEN) {
                memResult = memcpy_s(dataAt3 + *length, AT_SEND_CODE_ADDR_MAX_LEN - *length, (TAF_CHAR *)g_atCmeErrorStr,
                                 AT_CME_ERROR_STR_LEN);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, AT_CME_ERROR_STR_LEN);
            }

            *length += AT_CME_ERROR_STR_LEN;

            tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[0]);
            if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
                memResult = memcpy_s(dataAt3 + *length, AT_SEND_CODE_ADDR_MAX_LEN - *length,
                                     (TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[0],
                                     tmp);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
            }
            *length += (TAF_UINT16)tmp;
            break;

        default:
            if (*length < AT_SEND_CODE_ADDR_MAX_LEN) {
                memResult = memcpy_s(dataAt3 + *length, AT_SEND_CODE_ADDR_MAX_LEN - *length, (TAF_CHAR *)g_atCmeErrorStr,
                                 AT_CME_ERROR_STR_LEN);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, AT_CME_ERROR_STR_LEN);
            }

            *length += AT_CME_ERROR_STR_LEN;

            tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[1]);
            if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
                memResult = memcpy_s(dataAt3 + *length,
                                     AT_SEND_CODE_ADDR_MAX_LEN - *length,
                                     (TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[1],
                                     tmp);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
            }
            *length += (TAF_UINT16)tmp;
            break;
    }

    return VOS_TRUE;
}

VOS_UINT32 At_ProcCmsErrorReturnCodeAccordingToAtCmeeType(VOS_UINT32 returnCodeIndex, VOS_UINT16 *length)
{
    VOS_UINT8 *dataAt3 = VOS_NULL_PTR;
    VOS_UINT32 atErrIndex = 0;
    VOS_UINT32 atErrRslt;
    VOS_UINT32 tmp = 0;
    errno_t    memResult;

    dataAt3    = g_atSndCodeAddress;

    switch (g_atCmeeType) {
        case AT_CMEE_ONLY_ERROR:
            atErrRslt = AT_GetReturnCodeId(AT_ERROR, &atErrIndex);

            if (atErrRslt != VOS_OK) {
                AT_ERR_LOG("At_ProcCmsErrorReturnCodeAccordingToAtCmeeType: AT ERROR index is err!");
                return VOS_FALSE;
            }

            tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[atErrIndex].result[g_atVType]);
            if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
                memResult = memcpy_s(dataAt3 + *length, AT_SEND_CODE_ADDR_MAX_LEN - *length,
                                     (TAF_CHAR *)g_atReturnCodeTab[atErrIndex].result[g_atVType], tmp);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
            }
            *length += (TAF_UINT16)tmp;
            break;

        case AT_CMEE_ERROR_CODE:

            memResult = memcpy_s(dataAt3 + *length, AT_CMS_ERROR_STR_LEN, (TAF_CHAR *)g_atCmsErrorStr,
                                 AT_CMS_ERROR_STR_LEN);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_CMS_ERROR_STR_LEN, AT_CMS_ERROR_STR_LEN);
            *length += AT_CMS_ERROR_STR_LEN;

            tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[0]);
            if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
                memResult = memcpy_s(dataAt3 + *length,
                                     AT_SEND_CODE_ADDR_MAX_LEN - *length,
                                     (TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[0],
                                     tmp);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
            }
            *length += (TAF_UINT16)tmp;
            break;

        default:
            memResult = memcpy_s(dataAt3 + *length, AT_CMS_ERROR_STR_LEN, (TAF_CHAR *)g_atCmsErrorStr,
                                 AT_CMS_ERROR_STR_LEN);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_CMS_ERROR_STR_LEN, AT_CMS_ERROR_STR_LEN);
            *length += AT_CMS_ERROR_STR_LEN;

            tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[1]);
            if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
                memResult = memcpy_s(dataAt3 + *length,
                                     AT_SEND_CODE_ADDR_MAX_LEN - *length,
                                     (TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[1],
                                     tmp);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
            }
            *length += (TAF_UINT16)tmp;
            break;
    }

    return VOS_TRUE;
}

VOS_VOID At_SaveNvDialRateIndexAccordingToSubSysMode(VOS_UINT8 subSysMode, VOS_UINT32 *nvDialRateIndex)
{
    AT_GET_CONNECT_RATE_FUNC_PTR getConnectRateFunc = VOS_NULL_PTR;

    VOS_UINT32 count;
    VOS_UINT32 i;

    count = sizeof(g_subSysModeConnectRateArr) / sizeof(g_subSysModeConnectRateArr[0]);

    for (i = 0; i < count; i++) {
        if (subSysMode == (VOS_UINT8)(g_subSysModeConnectRateArr[i].subSysMode)) {
            getConnectRateFunc = g_subSysModeConnectRateArr[i].getConnectRateFunc;

            break;
        }
    }

    if (getConnectRateFunc == VOS_NULL_PTR) {
        AT_INFO_LOG("At_SaveNvDialRateIndexAccordingToSubSysMode: cannot find func");

        *nvDialRateIndex = 0;
    } else {
        *nvDialRateIndex = (VOS_UINT32)getConnectRateFunc();
    }
}

VOS_VOID At_AddDialRateToConnect(VOS_UINT8 indexNum, VOS_UINT16 *length, VOS_UINT8 *dataAt1)
{
    const TAF_CHAR       *dialRateTmp = VOS_NULL_PTR;
    TAF_AGENT_SysMode     sysMode;
    VOS_UINT32            ret;
    VOS_UINT32            nvDialRateIndex;
    VOS_UINT8             subSysMode;
    PPP_RateDisplayUint32 rateDisplay;
    VOS_UINT32            tmp = 0;
    errno_t               memResult;
    VOS_UINT8             dlCategoryIndex = 0;

    memset_s(&sysMode, sizeof(sysMode), 0x00, sizeof(sysMode));

    /* 从C核获取ucRatType和ucSysSubMode */
    ret = TAF_AGENT_GetSysMode(indexNum, &sysMode);

    subSysMode = TAF_SYS_SUBMODE_BUTT;
    if (ret == VOS_OK) {
        subSysMode = sysMode.sysSubMode;
    }

    At_SaveNvDialRateIndexAccordingToSubSysMode(subSysMode, &nvDialRateIndex);

    if ((nvDialRateIndex == 0) || (nvDialRateIndex > AT_DIAL_RATE_DISPALY_NV_ARRAY_LEN)) {
        if ((sysMode.ratType == TAF_PH_INFO_WCDMA_RAT) || (sysMode.ratType == TAF_PH_INFO_TD_SCDMA_RAT)) {
            rateDisplay = AT_GetRateDisplayIndexForWcdma(&g_atDlRateCategory);
            tmp         = VOS_StrLen((TAF_CHAR *)AT_GetPppDialRateDisplay(rateDisplay));
            dialRateTmp = AT_GetPppDialRateDisplay(rateDisplay);
        }
#if (FEATURE_LTE == FEATURE_ON)
        else if (sysMode.ratType == TAF_PH_INFO_LTE_RAT) {
            dlCategoryIndex = AT_GetLteUeDlCategoryIndex();

            tmp         = VOS_StrLen((TAF_CHAR *)(AT_GetLteRateDisplay(dlCategoryIndex).strDlSpeed));
            dialRateTmp = AT_GetLteRateDisplay(dlCategoryIndex).strDlSpeed;
        }
#endif
        else {
            rateDisplay = AT_GetRateDisplayIndexForGsm(&sysMode);
            tmp         = VOS_StrLen((TAF_CHAR *)AT_GetPppDialRateDisplay(rateDisplay));
            dialRateTmp = AT_GetPppDialRateDisplay(rateDisplay);
        }
    }
    else {
        tmp         = VOS_StrLen((TAF_CHAR *)AT_GetDialRateDisplayNv(nvDialRateIndex - 1));
        dialRateTmp = AT_GetDialRateDisplayNv(nvDialRateIndex - 1);
    }

    /* 在CONNECT后附上速率信息 */
    if ((tmp > 0) && (*length < AT_SEND_CRLF_ADDR_MAX_LEN) && (dialRateTmp != VOS_NULL_PTR)) {
        memResult = memcpy_s(dataAt1 + *length, AT_SEND_CRLF_ADDR_MAX_LEN - *length, dialRateTmp, tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CRLF_ADDR_MAX_LEN - *length, tmp);
    }

    *length += (TAF_UINT16)tmp;
}

VOS_VOID At_ProcReturnCodeAtConnect(VOS_UINT8 indexNum, VOS_UINT16 *length, VOS_UINT32 *mdmMscFlg, VOS_UINT8 *dataAt1)
{
    DMS_PortIdUint16    portId;

    portId = g_atClientTab[indexNum].portNo;

    if (DMS_PORT_IsPppPacketTransMode(portId) == VOS_TRUE) {
        /* 设置管脚信号标志 */
        *mdmMscFlg = VOS_TRUE;

        /* 若g_atXType大于0，在CONNECT后附上速率信息 */
        if (g_atXType > 0) {
            At_AddDialRateToConnect(indexNum, length, dataAt1);
        }
    } else {
    }
}

/*
 * 功能描述: 指示命令返回码类型为AT_V_ENTIRE_TYPE的其它类型，非BASIC返回值的处理
 */
LOCAL VOS_UINT32 At_ProcUnBasicReturnCodeAtVEntireType(VOS_UINT8 *dataAt1, VOS_UINT32 returnCode,
    VOS_UINT32 returnCodeIndex, VOS_UINT16 *length, VOS_UINT8 indexNum)
{
    errno_t    memResult;
    VOS_UINT32 tmp = 0;
    VOS_UINT32 mdmMscFlg = VOS_FALSE;

    /* Code前面加\r\n */
    if ((AT_CRLF_STR_LEN + (*length)) <= AT_SEND_CRLF_ADDR_MAX_LEN) {
        memResult = memcpy_s((TAF_CHAR *)(dataAt1 + (*length)), AT_SEND_CRLF_ADDR_MAX_LEN - (*length),
            (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CRLF_ADDR_MAX_LEN - (*length), AT_CRLF_STR_LEN);
    }

    (*length) += AT_CRLF_STR_LEN;

    tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[g_atVType]);

    /* Code */
    if ((tmp > 0) && ((*length) < AT_SEND_CRLF_ADDR_MAX_LEN)) {
        memResult = memcpy_s(dataAt1 + (*length), AT_SEND_CRLF_ADDR_MAX_LEN - (*length),
                             (TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[g_atVType], tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CRLF_ADDR_MAX_LEN - (*length), tmp);
    }

    (*length) += (TAF_UINT16)tmp;

    /* 如果是PPP拨号且g_atXType大于0，则CONNECT返回码需要附上速率信息，格式依照:CONNECT <Rate> */
    if (returnCode == AT_CONNECT) {
        At_ProcReturnCodeAtConnect(indexNum, length, &mdmMscFlg, dataAt1);
    }

    if (returnCode == AT_NO_CARRIER) {
        if (AT_IsPppUser(indexNum) == VOS_TRUE) {
            mdmMscFlg = VOS_TRUE;
        }
    }

    /* Code后面加\r\n */
    if ((AT_CRLF_STR_LEN + (*length)) <= AT_SEND_CRLF_ADDR_MAX_LEN) {
        memResult = memcpy_s(dataAt1 + (*length), AT_SEND_CRLF_ADDR_MAX_LEN - (*length), (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CRLF_ADDR_MAX_LEN - (*length), AT_CRLF_STR_LEN);
    }

    (*length) += AT_CRLF_STR_LEN;

    return mdmMscFlg;
}

VOS_VOID At_FinishAllCmdOrAbortSubCmd_AtVEntireType(VOS_UINT8 *dataAt1, VOS_UINT32 returnCode,
                                                    VOS_UINT32 returnCodeIndex, VOS_UINT16 length, VOS_UINT8 indexNum)
{
    errno_t    memResult;
    VOS_UINT32 mdmMscFlg = VOS_FALSE;
    VOS_UINT8  dialRateTmp[AT_AP_RATE_STRLEN];

    (VOS_VOID)memset_s(dialRateTmp, sizeof(dialRateTmp), 0x00, sizeof(dialRateTmp));

    if ((returnCode > AT_BASIC_CODE_ENUM_BEGAIN) && (returnCode < AT_BASIC_CODE_ENUM_END)) {
        mdmMscFlg = At_ProcUnBasicReturnCodeAtVEntireType(dataAt1, returnCode, returnCodeIndex, &length, indexNum);
    }

    /* 拷贝<text>和结果码 */
    if ((length != 0) && ((g_atCombineSendData[indexNum].bufLen + length) <= (AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
        memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                             AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                             dataAt1,
                             length);
        TAF_MEM_CHK_RTN_VAL(memResult, length, length);

        g_atCombineSendData[indexNum].bufLen += length;
    }

    if ((mdmMscFlg == VOS_TRUE) && (returnCode == AT_NO_CARRIER)) {
        DMS_PORT_ProcNoCarrierMsc(g_atClientTab[indexNum].portNo);
    }

    At_SendResultData(indexNum, g_atCombineSendData[indexNum].buffer, g_atCombineSendData[indexNum].bufLen);

    if ((mdmMscFlg == VOS_TRUE) && (returnCode == AT_CONNECT)) {
        DMS_PORT_ProcConnectMsc(g_atClientTab[indexNum].portNo);
    }

    At_ResetCombinParseInfo(indexNum);

    AT_CheckProcBlockCmd();
}

VOS_VOID At_ContinueSubCmd_AtVentireType(VOS_UINT8 *dataAt1, VOS_UINT16 length, VOS_UINT8 indexNum)
{
    errno_t memResult;
    if ((length != 0) && ((g_atCombineSendData[indexNum].bufLen + length) <= (AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
        memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                             AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                             dataAt1,
                             length);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen, length);
        g_atCombineSendData[indexNum].bufLen += length;
    }

    AT_ParseCmdOver(indexNum);
}

/*
 * 功能描述: 指示命令返回码类型为非AT_V_ENTIRE_TYPE的其它类型，BASIC返回值的处理
 */
LOCAL VOS_VOID At_ProcBasicReturnCodeAtVBreviteType(VOS_UINT8 *dataAt3, VOS_UINT32 returnCodeIndex, VOS_UINT16 *length)
{
    errno_t    memResult;
    VOS_UINT32 tmp = 0;

    if (((*length) != 0) && (*length <= (AT_SEND_CODE_ADDR_MAX_LEN - AT_CRLF_STR_LEN))) {
        memResult = memcpy_s(dataAt3 + (*length), AT_SEND_CODE_ADDR_MAX_LEN - (*length),
            (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - (*length), AT_CRLF_STR_LEN);

        (*length) += AT_CRLF_STR_LEN;
    }
    tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[g_atVType]);

    if ((tmp > 0) && ((*length) < AT_SEND_CODE_ADDR_MAX_LEN)) {
        memResult = memcpy_s(dataAt3 + (*length), AT_SEND_CODE_ADDR_MAX_LEN - (*length),
                             (TAF_CHAR *)g_atReturnCodeTab[returnCodeIndex].result[g_atVType], tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - (*length), tmp);
    }

    (*length) += (TAF_UINT16)tmp;

    if ((*length) < AT_SEND_CODE_ADDR_MAX_LEN) {
        memResult = memcpy_s(dataAt3 + (*length), AT_SEND_CODE_ADDR_MAX_LEN - (*length), &g_atS3, AT_CR_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - (*length), AT_CR_STR_LEN);
    }

    (*length) += AT_CR_STR_LEN;
}

VOS_VOID At_FinishAllCmdOrAbortSubCmd_AtVBreviteType(VOS_UINT8 *dataAt3, VOS_UINT32 returnCode,
                                                     VOS_UINT32 returnCodeIndex, VOS_UINT16 length, VOS_UINT8 indexNum)
{
    errno_t    memResult;

    if ((returnCode > AT_BASIC_CODE_ENUM_BEGAIN) && (returnCode < AT_BASIC_CODE_ENUM_END)) {
        At_ProcBasicReturnCodeAtVBreviteType(dataAt3, returnCodeIndex, &length);
    } else {
        if (length != 0) {
            if (g_atCmeeType == AT_CMEE_ONLY_ERROR) {
                if (length < AT_SEND_CODE_ADDR_MAX_LEN) {
                    memResult = memcpy_s(dataAt3 + length, AT_SEND_CODE_ADDR_MAX_LEN - length, &g_atS3, AT_CR_STR_LEN);
                    TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CR_STR_LEN);
                }

                length += AT_CR_STR_LEN;
            } else {
                if (length <= (AT_SEND_CODE_ADDR_MAX_LEN - AT_CRLF_STR_LEN)) {
                    memResult = memcpy_s(dataAt3 + length, AT_SEND_CODE_ADDR_MAX_LEN - length, (TAF_CHAR *)g_atCrLf,
                        AT_CRLF_STR_LEN);
                    TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);
                }

                length += AT_CRLF_STR_LEN;
            }
        }
    }

    if ((length != 0) && ((g_atCombineSendData[indexNum].bufLen + length) <= (AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
        memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                             AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                             dataAt3, length);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen, length);

        g_atCombineSendData[indexNum].bufLen += length;
    }

    At_SendResultData(indexNum, g_atCombineSendData[indexNum].buffer, g_atCombineSendData[indexNum].bufLen);

    At_ResetCombinParseInfo(indexNum);

    AT_CheckProcBlockCmd();
}

VOS_VOID At_ContinueSubCmd_AtVBreviteType(VOS_UINT8 *dataAt3, VOS_UINT16 length, VOS_UINT8 indexNum)
{
    errno_t memResult;
    if (length != 0) {
        if (length <= (AT_SEND_CODE_ADDR_MAX_LEN - AT_CRLF_STR_LEN)) {
            memResult = memcpy_s(dataAt3 + length, AT_SEND_CODE_ADDR_MAX_LEN - length, (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);
        }

        length += AT_CRLF_STR_LEN;

        if (((g_atCombineSendData[indexNum].bufLen + length <= AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN))) {
            memResult = memcpy_s((g_atCombineSendData[indexNum].buffer + g_atCombineSendData[indexNum].bufLen),
                                 AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen,
                                 dataAt3, length);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - g_atCombineSendData[indexNum].bufLen, length);
            g_atCombineSendData[indexNum].bufLen += length;
        }
    }

    AT_ParseCmdOver(indexNum);
}

VOS_UINT32 AT_IsStreamProcReturnCode(VOS_UINT32 returnCode)
{
    if ((returnCode == AT_WAIT_SMS_INPUT) || (returnCode == AT_WAIT_XML_INPUT) ||
        (returnCode == AT_WAIT_UE_POLICY_INPUT) || (returnCode == AT_WAIT_IMSA_URSP_INPUT)) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

VOS_UINT32 AT_IsCmeErrorReturnCode(VOS_UINT32 returnCode)
{
    if ((returnCode > AT_CME_ERROR_ENUM_BEGAIN) && (returnCode < AT_CME_ERROR_ENUM_END)) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

VOS_UINT32 AT_IsDeviceErrorReturnCode(VOS_UINT32 returnCode)
{
    if ((returnCode > AT_DEVICE_ERROR_BEGIN) && (returnCode < AT_DEVICE_ERROR_END)) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

VOS_UINT32 AT_IsCmsErrorReturnCode(VOS_UINT32 returnCode)
{
    if ((returnCode > AT_CMS_ERROR_ENUM_BEGAIN) && (returnCode < AT_CMS_ERROR_ENUM_END)) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

VOS_UINT32 AT_IsDiagErrorReturnCode(VOS_UINT32 returnCode)
{
    if ((returnCode > AT_DIAG_ENUM_BEGIN) && (returnCode < AT_DIAG_ENUM_END)) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

/*
 * 功能描述: 处理BUTT返回码
 */
LOCAL VOS_UINT32 At_ProcReturnCodeButt(VOS_UINT8 indexNum)
{
    AT_ERR_LOG("At_ProcReturnCodeButt: return code is invalid!");

    At_ResetCombinParseInfo(indexNum);

    AT_CheckProcBlockCmd();

    return VOS_FALSE;
}

/*
 * 功能描述: 处理NOT SUPPORT返回码
 */
LOCAL VOS_UINT32 At_ProcReturnCodeNotSupport(VOS_UINT8 indexNum, VOS_UINT32 returnCode)
{
    AT_LOG1("At_ProcReturnCodeNotSupport: return code is ", returnCode);

    AT_FormatAtCmdNotSupportResult(indexNum, (AT_RreturnCodeUint32)returnCode);

    At_ResetCombinParseInfo(indexNum);

    AT_CheckProcBlockCmd();

    return VOS_FALSE;
}

/*
 * 功能描述: 处理TOO MANY PARA返回码
 */
LOCAL VOS_UINT32 At_ProcReturnCodeTooManyPara(VOS_UINT8 indexNum, VOS_UINT32 returnCode)
{
    AT_LOG1("At_ProcReturnCodeTooManyPara: return code is ", returnCode);

    AT_FormatAtTooManyParaResult(indexNum, (AT_RreturnCodeUint32)returnCode);

    At_ResetCombinParseInfo(indexNum);

    AT_CheckProcBlockCmd();

    return VOS_FALSE;
}

/*
 * 功能描述: 处理ABORT返回码
 */
LOCAL VOS_UINT32 At_ProcReturnCodeAbort(VOS_UINT8 indexNum, VOS_UINT32 returnCode)
{
    AT_LOG1("At_ProcReturnCodeAbort: return code is ", returnCode);

    AT_FormatAtAbortResult(indexNum, (AT_RreturnCodeUint32)returnCode);

    At_ResetCombinParseInfo(indexNum);

    AT_CheckProcBlockCmd();

    return VOS_FALSE;
}

/*
 * 功能描述: 处理DEVICE ERROR返回码
 */
LOCAL VOS_VOID At_ProcReturnCodeDeviceError(VOS_UINT32 *returnCodeIndex, VOS_UINT16 *length, VOS_UINT8 *dataAt3)
{
    VOS_UINT32 tmp = 0;
    errno_t    memResult;

    if (*length < AT_SEND_CODE_ADDR_MAX_LEN) {
        memResult = memcpy_s(dataAt3 + *length, AT_SEND_CODE_ADDR_MAX_LEN - *length, (TAF_CHAR *)g_atCmeErrorStr,
                         AT_CME_ERROR_STR_LEN);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, AT_CME_ERROR_STR_LEN);
    }

    *length += AT_CME_ERROR_STR_LEN;

    tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[*returnCodeIndex].result[0]);
    if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
        memResult = memcpy_s(dataAt3 + *length,
                             AT_SEND_CODE_ADDR_MAX_LEN - *length,
                             (TAF_CHAR *)g_atReturnCodeTab[*returnCodeIndex].result[0],
                             tmp);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
    }
    *length += (TAF_UINT16)tmp;
}

/*
 * 功能描述: 处理DIAG ERROR返回码
 */
LOCAL VOS_VOID At_ProcReturnCodeDiagError(VOS_UINT32 returnCode, VOS_UINT32 *returnCodeIndex,
    VOS_UINT16 *length, VOS_UINT8 *dataAt3)
{
    VOS_UINT32 tmp = 0;
    errno_t    memResult;

    if (AT_IsDiagErrorReturnCode(returnCode) == VOS_TRUE) {
        tmp = VOS_StrLen((TAF_CHAR *)g_atReturnCodeTab[*returnCodeIndex].result[1]);
        if ((tmp > 0) && (*length < AT_SEND_CODE_ADDR_MAX_LEN)) {
            memResult = memcpy_s(dataAt3 + *length,
                                 AT_SEND_CODE_ADDR_MAX_LEN - *length,
                                 (TAF_CHAR *)g_atReturnCodeTab[*returnCodeIndex].result[1],
                                 tmp);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - *length, tmp);
        }
        *length += (TAF_UINT16)tmp;
    }
}

VOS_UINT32 At_ProcReturnCode(VOS_UINT32 *returnCodeIndex, VOS_UINT16 *length, VOS_UINT32 returnCode, VOS_UINT8 indexNum)
{
    VOS_UINT8 *dataAt3 = VOS_NULL_PTR;

    dataAt3 = g_atSndCodeAddress;

    if (returnCode > AT_RRETURN_CODE_BUTT) {
        return At_ProcReturnCodeButt(indexNum);
    }

    if (returnCode == AT_CMD_NOT_SUPPORT) {
        return At_ProcReturnCodeNotSupport(indexNum, returnCode);
    }

    if (returnCode == AT_TOO_MANY_PARA) {
        return At_ProcReturnCodeTooManyPara(indexNum, returnCode);
    }

    if (returnCode == AT_ABORT) {
        return At_ProcReturnCodeAbort(indexNum, returnCode);
    }

    if (AT_GetReturnCodeId(returnCode, returnCodeIndex) != VOS_OK) {
        AT_ERR_LOG("At_FormatResultData: result code index is err!");
        return VOS_FALSE;
    }
    /* 进入SMS输入模式或XML文本输入模式 */
    if (AT_IsStreamProcReturnCode(returnCode) == VOS_TRUE) {
        At_ProcWaitSmsOrXmlInputReturnCode(length, *returnCodeIndex, indexNum);

        return VOS_FALSE;
    } else if (AT_IsCmeErrorReturnCode(returnCode) == VOS_TRUE) {
        if (At_ProcCmeErrorReturnCodeAccordingToAtCmeeType(*returnCodeIndex, length) == VOS_FALSE) {
            return VOS_FALSE;
        }
    } else if (AT_IsDeviceErrorReturnCode(returnCode) == VOS_TRUE) {
        At_ProcReturnCodeDeviceError(returnCodeIndex, length, dataAt3);
    } else if (AT_IsCmsErrorReturnCode(returnCode) == VOS_TRUE) {
        if (At_ProcCmsErrorReturnCodeAccordingToAtCmeeType(*returnCodeIndex, length) == VOS_FALSE) {
            return VOS_FALSE;
        }
    } else {
        At_ProcReturnCodeDiagError(returnCode, returnCodeIndex, length, dataAt3);
    }

    return VOS_TRUE;
}

VOS_VOID At_FormatResultData(VOS_UINT8 indexNum, VOS_UINT32 returnCode)
{
    errno_t    memResult;
    VOS_UINT16 length;
    VOS_UINT8 *dataAt1 = VOS_NULL_PTR;
    VOS_UINT8 *dataAt3 = VOS_NULL_PTR;

    VOS_UINT32 comCmdSubProc;
    VOS_UINT8  dialRateTmp[AT_AP_RATE_STRLEN];
    VOS_UINT32 returnCodeIndex;

    /* 变量初始化 */
    length                  = g_atSendDataBuff.bufLen;
    g_atSendDataBuff.bufLen = 0;
    dataAt1                 = g_atSndCrLfAddr;
    dataAt3                 = g_atSndCodeAddress;
    returnCodeIndex         = 0;

    (VOS_VOID)memset_s(dialRateTmp, sizeof(dialRateTmp), 0x00, sizeof(dialRateTmp));

    comCmdSubProc = At_JudgeCombineCmdSubSequentProcess(indexNum, returnCode);

    /* 先处理return code */
    if (At_ProcReturnCode(&returnCodeIndex, &length, returnCode, indexNum) == VOS_FALSE) {
        return;
    }

    if (g_atVType == AT_V_ENTIRE_TYPE) {
        if (length != 0) {
            /* Text前面加\r\n */
            memResult = memcpy_s(dataAt1, AT_SEND_CRLF_ADDR_MAX_LEN, (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CRLF_ADDR_MAX_LEN, AT_CRLF_STR_LEN);

            /* Text后面加\r\n */
            if (length <= (AT_SEND_CODE_ADDR_MAX_LEN - AT_CRLF_STR_LEN)) {
                memResult = memcpy_s(dataAt3 + length, AT_SEND_CODE_ADDR_MAX_LEN - length, (TAF_CHAR *)g_atCrLf, AT_CRLF_STR_LEN);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_SEND_CODE_ADDR_MAX_LEN - length, AT_CRLF_STR_LEN);
            }

            length += AT_CRLF_STR_LEN;

            length += AT_CRLF_STR_LEN;
        }

        if ((comCmdSubProc == AT_COM_CMD_SUB_PROC_FINISH) || (comCmdSubProc == AT_COM_CMD_SUB_PROC_ABORT)) {
            At_FinishAllCmdOrAbortSubCmd_AtVEntireType(dataAt1, returnCode, returnCodeIndex, length, indexNum);
        } else {
            At_ContinueSubCmd_AtVentireType(dataAt1, length, indexNum);
        }
    } else {
        if ((comCmdSubProc == AT_COM_CMD_SUB_PROC_FINISH) || (comCmdSubProc == AT_COM_CMD_SUB_PROC_ABORT)) {
            At_FinishAllCmdOrAbortSubCmd_AtVBreviteType(dataAt3, returnCode, returnCodeIndex, length, indexNum);
        } else {
            At_ContinueSubCmd_AtVBreviteType(dataAt3, length, indexNum);
        }
    }
}

VOS_UINT32 AT_GetDlRateDisplayIndex(VOS_UINT8 wasCategory)
{
    PPP_RateDisplayUint32 dlRateDisplayIndex;

    switch (wasCategory) {
        case AT_RATE_WCDMA_CATEGORY_6:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_6;
            break;

        case AT_RATE_WCDMA_CATEGORY_8:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_8;
            break;

        case AT_RATE_WCDMA_CATEGORY_9:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_9;
            break;

        case AT_RATE_WCDMA_CATEGORY_10:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_10;
            break;

        case AT_RATE_WCDMA_CATEGORY_12:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_12;
            break;

        default:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_10;
            break;
    }

    return dlRateDisplayIndex;
}

VOS_UINT32 AT_GetR7DlRateDisplayIndex(VOS_UINT8 wasCategory)
{
    PPP_RateDisplayUint32 dlRateDisplayIndex;

    switch (wasCategory) {
        case AT_RATE_WCDMA_CATEGORY_6:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_6;
            break;

        case AT_RATE_WCDMA_CATEGORY_8:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_8;
            break;

        case AT_RATE_WCDMA_CATEGORY_9:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_9;
            break;

        case AT_RATE_WCDMA_CATEGORY_10:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_10;
            break;

        case AT_RATE_WCDMA_CATEGORY_12:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_12;
            break;

        case AT_RATE_WCDMA_CATEGORY_14:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_14;
            break;

        case AT_RATE_WCDMA_CATEGORY_18:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_18;
            break;

        default:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_18;
            break;
    }

    return dlRateDisplayIndex;
}

VOS_UINT32 AT_GetR8DlRateDisplayIndex(VOS_UINT8 wasCategory)
{
    PPP_RateDisplayUint32 dlRateDisplayIndex;

    switch (wasCategory) {
        case AT_RATE_WCDMA_CATEGORY_6:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_6;
            break;

        case AT_RATE_WCDMA_CATEGORY_8:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_8;
            break;

        case AT_RATE_WCDMA_CATEGORY_9:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_9;
            break;

        case AT_RATE_WCDMA_CATEGORY_10:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_10;
            break;

        case AT_RATE_WCDMA_CATEGORY_12:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_12;
            break;

        case AT_RATE_WCDMA_CATEGORY_14:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_14;
            break;

        case AT_RATE_WCDMA_CATEGORY_18:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_18;
            break;

        case AT_RATE_WCDMA_CATEGORY_24:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_24;
            break;

        default:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_24;
            break;
    }

    return dlRateDisplayIndex;
}

VOS_UINT32 AT_GetR9DlRateDisplayIndex(VOS_UINT8 wasCategory)
{
    PPP_RateDisplayUint32 dlRateDisplayIndex;

    switch (wasCategory) {
        case AT_RATE_WCDMA_CATEGORY_6:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_6;
            break;

        case AT_RATE_WCDMA_CATEGORY_8:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_8;
            break;

        case AT_RATE_WCDMA_CATEGORY_9:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_9;
            break;

        case AT_RATE_WCDMA_CATEGORY_10:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_10;
            break;

        case AT_RATE_WCDMA_CATEGORY_12:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_12;
            break;

        case AT_RATE_WCDMA_CATEGORY_14:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_14;
            break;

        case AT_RATE_WCDMA_CATEGORY_18:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_18;
            break;

        case AT_RATE_WCDMA_CATEGORY_24:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_24;
            break;

        case AT_RATE_WCDMA_CATEGORY_26:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_26;
            break;

        case AT_RATE_WCDMA_CATEGORY_28:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_28;
            break;

        default:
            dlRateDisplayIndex = PPP_RATE_DISPLAY_DPA_CATEGORY_28;
            break;
    }

    return dlRateDisplayIndex;
}

VOS_UINT32 AT_GetRateDisplayIndexForGsm(TAF_AGENT_SysMode *sysMode)
{
    PPP_RateDisplayUint32 rateIndex;

    if (sysMode->sysSubMode == TAF_SYS_SUBMODE_EDGE) {
        if (g_atDlRateCategory.gasMultislotClass33Flg == VOS_TRUE) {
            rateIndex = PPP_RATE_DISPLAY_EDGE_CALSS33;
        } else {
            rateIndex = PPP_RATE_DISPLAY_EDGE;
        }
    } else {
        if (g_atDlRateCategory.gasMultislotClass33Flg == VOS_TRUE) {
            rateIndex = PPP_RATE_DISPLAY_GPRS_CALSS33;
        } else {
            rateIndex = PPP_RATE_DISPLAY_GPRS;
        }
    }

    return rateIndex;
}

VOS_UINT32 AT_GetRateDisplayIndexForWcdma(AT_DOWNLINK_RateCategory *dlRateCategory)
{
    PPP_RateDisplayUint32 rateIndex;

    switch (dlRateCategory->wasRelIndicator) {
        case PS_PTL_VER_R99:
        case PS_PTL_VER_R3:
        case PS_PTL_VER_R4:
            rateIndex = PPP_RATE_DISPLAY_R99;
            break;

        case PS_PTL_VER_R5:
        case PS_PTL_VER_R6:
            rateIndex = AT_GetDlRateDisplayIndex(dlRateCategory->wasCategory);
            break;

        case PS_PTL_VER_R7:
            rateIndex = AT_GetR7DlRateDisplayIndex(dlRateCategory->wasCategory);
            break;

        case PS_PTL_VER_R8:
            rateIndex = AT_GetR8DlRateDisplayIndex(dlRateCategory->wasCategory);
            break;

        case PS_PTL_VER_R9:
            rateIndex = AT_GetR9DlRateDisplayIndex(dlRateCategory->wasCategory);
            break;

        default:
            rateIndex = AT_GetDlRateDisplayIndex(dlRateCategory->wasCategory);
            break;
    }

    return rateIndex;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_UINT8 AT_GetHsuartBaseClientId(VOS_VOID)
{
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    if ((DMS_PORT_GetCmuxPhyPort() == DMS_PORT_HSUART) && (DMS_PORT_GetCmuxOpenFlg() == VOS_TRUE)) {
        return AT_CLIENT_ID_CMUXAT;
    }
#endif

    return AT_CLIENT_ID_HSUART;
}

VOS_UINT32 AT_IsSmsRingingTe(VOS_VOID)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;

    riStateInfo = AT_GetUartRiStateInfo();

    if ((riStateInfo->runFlg == VOS_TRUE) && (riStateInfo->type == AT_UART_RI_TYPE_SMS)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_VOID AT_SmsRingOn(VOS_VOID)
{
    AT_UartRiCfg       *riCfgInfo   = VOS_NULL_PTR;
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT32          tmrName;
    VOS_UINT32          tmrParam;
    VOS_UINT8           indexNum;

    indexNum     = AT_GetHsuartBaseClientId();
    riCfgInfo    = AT_GetUartRiCfgInfo();
    riStateInfo  = AT_GetUartRiStateInfo();
    clientManage = AT_GetClientManage(indexNum);

    AT_SET_SMS_RI_TMR_NAME(tmrName);
    AT_SET_SMS_RI_TMR_PARAM(tmrParam, indexNum);

    /* RI信号设为高电平 */
    DMS_PORT_AssertRi(clientManage->portNo);

    /* 启动RI信号电平保持定时器 */
    AT_StartRelTimer(&(riStateInfo->hSmsRiTmrHdl), riCfgInfo->smsRiOnInterval, tmrName, tmrParam, VOS_RELTIMER_NOLOOP);

    riStateInfo->smsRiTmrStatus = AT_TIMER_STATUS_RUNNING;
}

VOS_VOID AT_SmsRingOff(VOS_VOID)
{
    AT_UartRiCfg       *riCfgInfo   = VOS_NULL_PTR;
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT32          tmrName;
    VOS_UINT32          tmrParam;
    VOS_UINT8           indexNum;

    indexNum     = AT_GetHsuartBaseClientId();
    riCfgInfo    = AT_GetUartRiCfgInfo();
    riStateInfo  = AT_GetUartRiStateInfo();
    clientManage = AT_GetClientManage(indexNum);

    AT_SET_SMS_RI_TMR_NAME(tmrName);
    AT_SET_SMS_RI_TMR_PARAM(tmrParam, indexNum);

    if (riCfgInfo->smsRiOffInterval == 0) {
        AT_SmsStopRingTe();
    } else {
        /* RI信号设为低电平 */
        DMS_PORT_DeassertRi(clientManage->portNo);

        /* 启动RI信号电平保持定时器 */
        AT_StartRelTimer(&(riStateInfo->hSmsRiTmrHdl), riCfgInfo->smsRiOffInterval, tmrName, tmrParam,
                         VOS_RELTIMER_NOLOOP);

        riStateInfo->smsRiTmrStatus = AT_TIMER_STATUS_RUNNING;
    }
}

VOS_VOID AT_SmsStartRingTe(VOS_UINT32 newSmsFlg)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;

    riStateInfo = AT_GetUartRiStateInfo();

    /* 检查增加短信通知波形计数的标识 */
    if (newSmsFlg == VOS_TRUE) {
        riStateInfo->smsRiOutputCount++;
    }

    /* 检查是否正在输出波形 */
    if (riStateInfo->runFlg == VOS_TRUE) {
        return;
    }

    /* 输出RI信号高电平波形 */
    AT_SmsRingOn();

    /* 记录RI信号状态 */
    riStateInfo->runFlg = VOS_TRUE;
    riStateInfo->type   = AT_UART_RI_TYPE_SMS;
}

VOS_VOID AT_SmsStopRingTe(VOS_VOID)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT32          tmrName;
    VOS_UINT8           indexNum;

    indexNum     = AT_GetHsuartBaseClientId();
    riStateInfo  = AT_GetUartRiStateInfo();
    clientManage = AT_GetClientManage(indexNum);

    AT_SET_SMS_RI_TMR_NAME(tmrName);

    /* RI信号设为低电平 */
    if (DMS_PORT_IsRiAsserted(clientManage->portNo) == VOS_TRUE) {
        DMS_PORT_DeassertRi(clientManage->portNo);
    }

    /* 如果电平保持定时器已经停止, 说明短信通知波形已经完整输出 */
    if (riStateInfo->smsRiTmrStatus == AT_TIMER_STATUS_RUNNING) {
        /* 停止RI信号电平保持定时器 */
        AT_StopRelTimer(tmrName, &(riStateInfo->hSmsRiTmrHdl));
        riStateInfo->smsRiTmrStatus = AT_TIMER_STATUS_STOP;
    } else {
        /* 更新短信通知计数 */
        if (riStateInfo->smsRiOutputCount > 0) {
            riStateInfo->smsRiOutputCount--;
        }
    }

    /* 清除RI信号状态 */
    riStateInfo->runFlg = VOS_FALSE;
    riStateInfo->type   = AT_UART_RI_TYPE_BUTT;
}

VOS_VOID AT_RcvTiSmsRiExpired(REL_TimerMsgBlock *tmrMsg)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT8           indexNum;

    riStateInfo     = AT_GetUartRiStateInfo();
    indexNum        = AT_GET_SMS_RI_CLIENTID_FROM_TMR_PARAM(tmrMsg->para);
    clientManage    = AT_GetClientManage(indexNum);

    /* 短信通知波形输出已停止 */
    if (AT_IsSmsRingingTe() != VOS_TRUE) {
        return;
    }

    /* 更新定时器状态 */
    riStateInfo->smsRiTmrStatus = AT_TIMER_STATUS_STOP;

    /* RI信号电平控制 */
    if (DMS_PORT_IsRiAsserted(clientManage->portNo) == VOS_TRUE) {
        AT_SmsRingOff();
    } else {
        AT_SmsStopRingTe();
    }

    /* SMS RI配置低电平持续时间为0时, 需要重新开始RING TE */
    if ((AT_IsSmsRingingTe() != VOS_TRUE) && (riStateInfo->smsRiOutputCount != 0)) {
        AT_SmsStartRingTe(VOS_FALSE);
    }
}

VOS_UINT32 AT_IsVoiceRingingTe(VOS_VOID)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;

    riStateInfo = AT_GetUartRiStateInfo();

    if ((riStateInfo->runFlg == VOS_TRUE) && (riStateInfo->type == AT_UART_RI_TYPE_VOICE)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

AT_UartRiStatusUint8 AT_GetRiStatusByCallId(VOS_UINT8 callId)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;

    riStateInfo = AT_GetUartRiStateInfo();

    /* 检查CALLID有效性 */
    if (callId > MN_CALL_MAX_NUM) {
        return AT_UART_RI_STATUS_STOP;
    }

    return riStateInfo->voiceRiStatus[callId];
}

VOS_VOID AT_VoiceRingOn(VOS_UINT8 callId)
{
    AT_UartRiCfg       *riCfgInfo   = VOS_NULL_PTR;
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT32          tmrName;
    VOS_UINT32          tmrParam;
    VOS_UINT8           indexNum;

    indexNum     = AT_GetHsuartBaseClientId();
    riCfgInfo    = AT_GetUartRiCfgInfo();
    riStateInfo  = AT_GetUartRiStateInfo();
    clientManage = AT_GetClientManage(indexNum);

    AT_SET_VOICE_RI_TMR_NAME(tmrName);
    AT_SET_VOICE_RI_TMR_PARAM(tmrParam, indexNum, callId);

    /* RI信号设为高电平 */
    DMS_PORT_AssertRi(clientManage->portNo);

    /* 启动RI信号电平保持定时器 */
    AT_StartRelTimer(&(riStateInfo->hVoiceRiTmrHdl), riCfgInfo->voiceRiOnInterval, tmrName, tmrParam,
                     VOS_RELTIMER_NOLOOP);

    riStateInfo->voiceRiTmrStatus = AT_TIMER_STATUS_RUNNING;
}

VOS_VOID AT_VoiceRingOff(VOS_UINT8 callId)
{
    AT_UartRiCfg       *riCfgInfo   = VOS_NULL_PTR;
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT32          tmrName;
    VOS_UINT32          tmrParam;
    VOS_UINT8           indexNum;

    indexNum     = AT_GetHsuartBaseClientId();
    riCfgInfo    = AT_GetUartRiCfgInfo();
    riStateInfo  = AT_GetUartRiStateInfo();
    clientManage = AT_GetClientManage(indexNum);

    AT_SET_VOICE_RI_TMR_NAME(tmrName);
    AT_SET_VOICE_RI_TMR_PARAM(tmrParam, indexNum, callId);

    /* RI信号设为低电平 */
    DMS_PORT_DeassertRi(clientManage->portNo);

    /* 启动RI信号电平保持定时器 */
    AT_StartRelTimer(&(riStateInfo->hVoiceRiTmrHdl), riCfgInfo->voiceRiOffInterval, tmrName, tmrParam,
                     VOS_RELTIMER_NOLOOP);

    riStateInfo->voiceRiTmrStatus = AT_TIMER_STATUS_RUNNING;
}

VOS_VOID AT_VoiceStartRingTe(VOS_UINT8 callId)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;

    riStateInfo = AT_GetUartRiStateInfo();

    /* 检查CALLID有效性 */
    if (callId > MN_CALL_MAX_NUM) {
        return;
    }

    /* 检查当前是否正在输出来电通知波形 */
    if (AT_IsVoiceRingingTe() == VOS_TRUE) {
        return;
    }

    /* 检查当前是否正在短信通知波形 */
    if (AT_IsSmsRingingTe() == VOS_TRUE) {
        /* 打断短信通知波形输出 */
        AT_SmsStopRingTe();
    }

    /* 输出RI信号高电平波形 */
    AT_VoiceRingOn(callId);

    /* 记录RI信号状态 */
    riStateInfo->runFlg                = VOS_TRUE;
    riStateInfo->type                  = AT_UART_RI_TYPE_VOICE;
    riStateInfo->voiceRiStatus[callId] = AT_UART_RI_STATUS_RINGING;
}

VOS_VOID AT_VoiceStopRingTe(VOS_UINT8 callId)
{
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT32          tmrName;
    VOS_UINT8           indexNum;

    indexNum     = AT_GetHsuartBaseClientId();
    riStateInfo  = AT_GetUartRiStateInfo();
    clientManage = AT_GetClientManage(indexNum);

    AT_SET_VOICE_RI_TMR_NAME(tmrName);

    /* 检查CALLID有效性 */
    if (callId > MN_CALL_MAX_NUM) {
        return;
    }

    /* 检查当前是否正在输出来电通知波形 */
    if (AT_IsVoiceRingingTe() != VOS_TRUE) {
        return;
    }

    /* 检查CALLID对应的RI状态 */
    if (AT_GetRiStatusByCallId(callId) != AT_UART_RI_STATUS_RINGING) {
        return;
    }

    /* RI信号设为低电平 */
    if (DMS_PORT_IsRiAsserted(clientManage->portNo) == VOS_TRUE) {
        DMS_PORT_DeassertRi(clientManage->portNo);
    }

    /* 停止RI信号电平保持定时器 */
    if (riStateInfo->voiceRiTmrStatus == AT_TIMER_STATUS_RUNNING) {
        AT_StopRelTimer(tmrName, &(riStateInfo->hVoiceRiTmrHdl));
        riStateInfo->voiceRiTmrStatus = AT_TIMER_STATUS_STOP;
    }

    /* 清除RI信号状态 */
    riStateInfo->runFlg                = VOS_FALSE;
    riStateInfo->type                  = AT_UART_RI_TYPE_BUTT;
    riStateInfo->voiceRiStatus[callId] = AT_UART_RI_STATUS_STOP;
    riStateInfo->voiceRiCycleCount     = 0;

    /* 恢复输出短信通知波形 */
    if (riStateInfo->smsRiOutputCount > 0) {
        AT_SmsStartRingTe(VOS_FALSE);
    }
}

VOS_VOID AT_RcvTiVoiceRiExpired(REL_TimerMsgBlock *tmrMsg)
{
    AT_UartRiCfg       *riCfgInfo   = VOS_NULL_PTR;
    AT_UartRiStateInfo *riStateInfo = VOS_NULL_PTR;
    AT_ClientManage    *clientManage = VOS_NULL_PTR;
    VOS_UINT8           callId;
    VOS_UINT8           indexNum;

    riCfgInfo    = AT_GetUartRiCfgInfo();
    riStateInfo  = AT_GetUartRiStateInfo();
    callId       = AT_GET_VOICE_RI_CALLID_FROM_TMR_PARAM(tmrMsg->para);
    indexNum     = AT_GET_VOICE_RI_CLIENTID_FROM_TMR_PARAM(tmrMsg->para);
    clientManage = AT_GetClientManage(indexNum);

    /* 检查语音RI是否已经停止 */
    if (AT_IsVoiceRingingTe() != VOS_TRUE) {
        return;
    }

    /* 检查CALLID对应的RI状态 */
    if (AT_GetRiStatusByCallId(callId) != AT_UART_RI_STATUS_RINGING) {
        return;
    }

    /* 更新定时器状态 */
    riStateInfo->voiceRiTmrStatus = AT_TIMER_STATUS_STOP;

    /* RI信号电平控制 */
    if (DMS_PORT_IsRiAsserted(clientManage->portNo) == VOS_TRUE) {
        AT_VoiceRingOff(callId);
    } else {
        /* 波形输出计数达到配置值, 停止RING TE, 否则继续 */
        if ((++riStateInfo->voiceRiCycleCount) >= riCfgInfo->voiceRiCycleTimes) {
            AT_VoiceStopRingTe(callId);
        } else {
            AT_VoiceRingOn(callId);
        }
    }
}
#endif

