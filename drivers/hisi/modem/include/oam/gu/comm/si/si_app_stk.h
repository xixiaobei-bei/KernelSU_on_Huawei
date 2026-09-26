/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*
 * 版权所有（c）华为技术有限公司 2012-2019
 * 功能描述: SIM卡事件的头文件
 * 生成日期: 2008年10月13日
 */

#ifndef __SI_APP_STK_H__
#define __SI_APP_STK_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "si_typedef.h"
#include "vos.h"
#include "usimm_ps_comm_interface.h"

#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
#include "taf_type_def.h"
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "taf_mmi_str_parse.h"
#include "nas_stk_interface.h"
#endif

#pragma pack(push, 4)

#define SI_STK_DATA_MAX_LEN 256

/* 定义地址最大长度 */
#define SI_STK_ADDR_MAX_LEN 42

#define STK_PRINT_MAX_LEN 1024

#define ITEMNUMMAX 50

#define SI_STK_MENU_EXIST 1
#define SI_STK_MENU_NOTEXIST 0

#define SI_AT_RESULT_END_SESSION 0x0
#define SI_AT_RESULT_PERFORM_SUCC 0x1
#define SI_AT_RESULT_HELP_REQUIRED 0x2
#define SI_AT_RESULT_BACKWARD_MOVE 0x3
#define SI_AT_RESULT_ME_NOT_SUPPORT 0x4
#define SI_AT_RESULT_ME_BROWSER_BUSY 0x5
#define SI_AT_RESULT_ME_SS_BUSY 0x6

#define SI_DEVICE_KEYPAD 0x01
#define SI_DEVICE_DISPLAY 0x02
#define SI_DEVICE_EARPIECE 0x03
#define SI_DEVICE_UICC 0x81
#define SI_DEVICE_TERMINAL 0x82
#define SI_DEVICE_NETWORK 0x83

#define USAT_CMDSETSTGI 0x01
#define USAT_CMDQRYSTGI 0x02

#define STK_AS_NMR_INFO_REQ 0x08
#define STK_AS_NMR_INFO_CNF 0x11

#define STK_AS_TA_INFO_REQ 0x09
#define STK_AS_TA_INFO_CNF 0x12

#define SI_STK_DEFAULT_TA_VALUE 0
#define SI_STK_MAX_TA_VALUE 0X3F
#define SI_STK_INVALID_TA_VALUE 0xFF

enum SI_STK_MbbCmdType {
    SI_AT_CMD_SETUP_MENU = 0,
    SI_AT_CMD_DISPLAY_TEXT = 1,
    SI_AT_CMD_GET_INKEY = 2,
    SI_AT_CMD_GET_INPUT = 3,
    SI_AT_CMD_SETUP_CALL = 4,
    SI_AT_CMD_PLAY_TONE = 5,
    SI_AT_CMD_SELECT_ITEM = 6,
    SI_AT_CMD_REFRESH = 7,
    SI_AT_CMD_SEND_SS = 8,
    SI_AT_CMD_SEND_SMS = 9,
    SI_AT_CMD_SEND_USSD = 10,
    SI_AT_CMD_LAUNCH_BROWSER = 11,
    SI_AT_CMD_SETUP_IDLE_MODE_TEXT = 12,
    SI_AT_CMD_LANGUAGENOTIFICATION = 20, /* 暂时定为20，待APP开发该SAT功能后确认接口 */
    SI_AT_CMD_SETFRAMES = 21,            /* 暂时定为21，待APP开发该SAT功能后确认接口 */
    SI_AT_CMD_GETFRAMESSTATUS = 22,      /* 暂时定为22，待APP开发该SAT功能后确认接口 */
    SI_AT_CMD_END_SESSION = 99,
    SI_STK_MBBCMDTYPE_BUTT
};

enum SI_STK_Device {
    SI_STK_DEV_KEYPAD = 0x01,
    SI_STK_DEV_DISPALY = 0x02,
    SI_STK_DEV_EARPRIICE = 0x03,
    SI_STK_DEV_UICC = 0x81,
    SI_STK_DEV_TERMINAL = 0x82,
    SI_STK_DEV_NETWORK = 0x83,
    SI_STK_DEV_BUTT
};

enum {
    SI_STK_ERROR_EVENT = 0,               /* 处理错误统一回复 */
    SI_STK_CMD_IND_EVENT = 1,             /* 主动命令广播上报 */
    SI_STK_TERMINAL_RSP_EVENT = 2,        /* 主动命令执行结果下发回复 */
    SI_STK_ENVELPOE_RSP_EVENT = 3,        /* 主菜单选择回复 */
    SI_STK_PROFILESET_RSP_EVENT = 4,      /* Terminal Profile设置下发，保留未用 */
    SI_STK_GET_CMD_RSP_EVENT = 5,         /* 获取当前主动命令回复 */
    SI_STK_CMD_QUERY_RSP_EVENT = 6,       /* 查询当前主动命令回复 */
    SI_STK_CMD_TIMEOUT_IND_EVENT = 7,     /* 主动命令执行超时回复 */
    SI_STK_CMD_END_EVENT = 8,             /* 后续无主动命令执行上报 */
    SI_STK_CMD_IMSICHG_EVENT = 9,         /* 沃达丰定制流程 */
    SI_STK_CC_RESULT_IND_EVENT = 10,      /* Call Control结果上报 */
    SI_STK_SMSCTRL_RESULT_IND_EVENT = 11, /* Sms Control结果上报 */

    SI_STK_CALLBACK_BUTT
};
typedef VOS_UINT8 STK_CallbackEventUint8;

enum {
    SI_STK_EVENT_MT_CALL = 0x00,
    SI_STK_EVENT_CALL_CONNECTED = 0x01,
    SI_STK_EVENT_CALL_DISCONNECTED = 0x02,
    SI_STK_EVENT_LOCATION_STATUS = 0x03,
    SI_STK_EVENT_USER_ACTIVITY = 0x04,
    SI_STK_EVENT_IDLE_SCREEN_AVAILABLE = 0x05,
    SI_STK_EVENT_CARDREADER_STATUS = 0x06,
    SI_STK_EVENT_LANGUAGE_SELECTION = 0x07,
    SI_STK_EVENT_BROWSER_TERMINATION = 0x08,
    SI_STK_EVENT_DATA_AVAILABLE = 0x09,
    SI_STK_EVENT_CHANNEL_STATUS = 0x0A,
    SI_STK_EVENT_ACCESS_TECHN_CHANGE = 0x0B,
    SI_STK_EVENT_DISPLAY_PARA_CHANGED = 0x0C,
    SI_STK_EVENT_LOCAL_CONNECTION = 0x0D,
    SI_STK_EVENT_NET_SEARCHMODE_CHANGE = 0x0E,
    SI_STK_EVENT_BROWSING_STATUS = 0x0F,
    SI_STK_EVENT_FRAMES_INFO_CHANGE = 0x10,
    SI_STK_EVENT_IWLAN_ACCESS_STATUS = 0x11,
    SI_STK_EVENT_NETWORK_REJECTION = 0x12,
    SI_STK_EVENT_CSG_CELL_SELECTION = 0x15,
    SI_STK_EVENT_IMS_REGISTRATION = 0x17,
    SI_STK_EVENT_INCOMING_IMS_DATA = 0x18,
    SI_STK_EVENT_BUTT
};
typedef VOS_UINT32 SI_STK_EventTypeUint32;

enum SI_STK_EnvelopeType {
    SI_STK_PROCMD = 0xD0,
    SI_STK_ENVELOPE_PPDOWN = 0xD1,
    SI_STK_ENVELOPE_CBDOWN = 0xD2,
    SI_STK_ENVELOPE_MENUSEL = 0xD3,
    SI_STK_ENVELOPE_CALLCRTL = 0xD4,
    SI_STK_ENVELOPE_SMSCRTL = 0xD5,
    SI_STK_ENVELOPE_EVENTDOWN = 0xD6,
    SI_STK_ENVELOPE_TIMEEXP = 0xD7,
    SI_STK_ENVELOPE_RFU = 0xD8,
    SI_STK_ENVELOPE_USSDDOWN = 0xD9,
    SI_STK_ENVELOPE_MMSTRANSTATUS = 0xDA,
    SI_STK_ENVELOPE_MMSNOTIFY = 0xDB,
};
typedef VOS_UINT32 SI_STK_EnvelopeTypeUint32;

enum {
    SI_STK_NOCMDDATA = 0x00,
    SI_STK_REFRESH = 0x01,
    SI_STK_MORETIME = 0x02,
    SI_STK_POLLINTERVAL = 0x03,
    SI_STK_POLLINGOFF = 0x04,
    SI_STK_SETUPEVENTLIST = 0x05,
    SI_STK_SETUPCALL = 0x10,
    SI_STK_SENDSS = 0x11,
    SI_STK_SENDUSSD = 0x12,
    SI_STK_SENDSMS = 0x13,
    SI_STK_SENDDTMF = 0x14,
    SI_STK_LAUNCHBROWSER = 0x15,
    SI_STK_PLAYTONE = 0x20,
    SI_STK_DISPLAYTET = 0x21,
    SI_STK_GETINKEY = 0x22,
    SI_STK_GETINPUT = 0x23,
    SI_STK_SELECTITEM = 0x24,
    SI_STK_SETUPMENU = 0x25,
    SI_STK_PROVIDELOCALINFO = 0x26,
    SI_STK_TIMERMANAGEMENT = 0x27,
    SI_STK_SETUPIDLETEXT = 0x28,
    SI_STK_PERFORMCARDAPDU = 0x30,
    SI_STK_POWERONCARD = 0x31,
    SI_STK_POWEROFFCARD = 0x32,
    SI_STK_GETREADERSTATUS = 0x33,
    SI_STK_RUNATCOMMAND = 0x34,
    SI_STK_LANGUAGENOTIFICATION = 0x35,
    SI_STK_OPENCHANNEL = 0x40,
    SI_STK_CLOSECHANNEL = 0x41,
    SI_STK_RECEIVEDATA = 0x42,
    SI_STK_SENDDATA = 0x43,
    SI_STK_GETCHANNELSTATUS = 0x44,
    SI_STK_SERVICESEARCH = 0x45,
    SI_STK_GETSERVICEINFO = 0x46,
    SI_STK_DECLARESERVICE = 0x47,
    SI_STK_SETFRAMES = 0x50,
    SI_STK_GETFRAMESSTATUS = 0x51,
    SI_STK_CMD_BUTT
};
typedef VOS_UINT32 SI_STK_CmdTypeUint32;

enum {
    COMMAND_PERFORMED_SUCCESSFULLY = 0x00,
    COMMAND_PERFORMED_WITH_PARTIAL_COMPREHENSION = 0x01,
    COMMAND_PERFORMED_WITH_MISSING_INFORMATION = 0x02,
    REFRESH_PERFORMED_WITH_ADDITIONAL_EFS_READ = 0x03,
    COMMAND_PERFORMED_SUCCESS_BUT_ICON_COULD_NOT_BE_DISPLAYED = 0x04,
    COMMAND_PERFORMED_BUT_MODIFIED_BY_CALL_CONTROL_BY_NAA = 0x05,
    COMMAND_PERFORMED_SUCCESSFULLY_LIMITED_SERVICE = 0x06,
    COMMAND_PERFORMED_WITH_MODIFICATION = 0x07,
    REFRESH_PERFORMED_BUT_INDICATED_NAA_WAS_NOT_ACTIVE = 0x08,
    COMMAND_PERFORMED_SUCCESSFULLY_TONE_NOT_PLAYED = 0x09,
    PROACTIVE_UICC_SESSION_TERMINATED_BY_THE_USER = 0x10,
    BACKWARD_MOVE_IN_THE_PROACTIVE_UICC_SESSION_REQUESTED_BY_USER = 0x11,
    NO_RESPONSE_FROM_USER = 0x12,
    HELP_INFORMATION_REQUIRED_BY_USER = 0x13,
    SS_TRANS_TERMINATED_BY_USER = 0x14,
    TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND = 0x20,
    NETWORK_CURRENTLY_UNABLE_TO_PROCESS_COMMAND = 0x21,
    USER_DO_NOT_ACCEPT_THE_PROACTIVE_COMMAND = 0x22,
    USER_CLEARED_DOWN_CALL_BEFORE_CONNECTION_OR_NETWORK_RELEASE = 0x23,
    ACTION_IN_CONTRADICTION_WITH_THE_CURRENT_TIMER_STATE = 0x24,
    INTERACTION_WITH_CALL_CONTROL_BY_NAA_TEMPORARY_PROBLEM = 0x25,
    LAUNCH_BROWSER_GENERIC_ERROR_CODE = 0x26,
    COMMAND_BEYOND_TERMINALS_CAPABILITIES = 0x30,
    COMMAND_TYPE_NOT_UNDERSTOOD_BY_TERMINAL = 0x31,
    COMMAND_DATA_NOT_UNDERSTOOD_BY_TERMINAL = 0x32,
    COMMAND_NUMBER_NOT_KNOWN_BY_TERMINAL = 0x33,
    SS_RETURN_ERROR = 0x34,
    SMS_RP_ERROR = 0x35,
    ERROR_REQUIRED_VALUES_ARE_MISSING = 0x36,
    USSD_RETURN_ERROR = 0x37,
    MULTIPLECARD_COMMANDS_ERROR = 0x38,
    INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM = 0x39,
    BEARER_INDEPENDENT_PROTOCOL_ERROR = 0x3A,
    ACCESS_TECHNOLOGY_UNABLE_TO_PROCESS_COMMAND = 0x3B,
    FRAMES_ERROR = 0x3C,
    SI_STK_RESULT_BUTT
};
typedef VOS_UINT8 SI_STK_ResultUint8;

/*
 * 枚举说明: STK回复TERMINAL RESPONSE时RESULT为TERMINAL PROBLEM时附带的ADDITIONAL RESULT值
 * 1. 日    期: 2013年7月12日
 *    修改内容: 新增
 */
enum SI_STK_AddInfoTerminalProblem {
    NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL = 0x00,
    SCREEN_IS_BUSY = 0x01,
    ME_CURRENTLY_BUSY_ON_CALL = 0x02,
    ME_CURRENTLY_BUSY_ON_SS_TRANSACTION = 0x03,
    NO_SERVICE = 0x04,
    ACCESS_CONTROL_CLASS_BAR = 0x05,
    RADIO_RESOURCE_NOT_GRANTED = 0x06,
    NOT_IN_SPEECH_CALL = 0x07,
    ME_CURRENTLY_BUSY_ON_USSD_TRANSACTION = 0x08,
    ME_CURRENTLY_BUSY_ON_SEND_DTMF_TRANSACTION = 0x09,
    NO_USIM_ACTIVE = 0x0A,
    ADD_INFO_TERMINAL_PROBLEM_BUTT
};
typedef VOS_UINT8 SI_STK_AddInfoTerminalProblemUint8;

/*
 * 枚举说明: STK回复TERMINAL RESPONSE时CALL CONTROL时附带的ADDITIONAL RESULT值
 * 1. 日    期: 2013年7月12日
 *    修改内容: 新增
 */
enum SI_STK_AddInfoWithCallControl {
    NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_CALL_CONTROL = 0x00,
    ACTION_NOT_ALLOWED = 0x01,
    TYPE_OF_REQUEST_HAS_CHANGED = 0x02,
    ADD_INFO_WITH_CALL_CONTROL_BUTT
};
typedef VOS_UINT8 SI_STK_AddInfoWithCallControlUint8;

/*
 * 枚举说明: STK回复TERMINAL RESPONSE时NETWORK PROBLEM时附带的ADDITIONAL RESULT值
 * 1. 日    期: 2013年7月12日
 *    修改内容: 新增
 */
enum SI_STK_AddInfoWithNetworkProblem {
    NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_NETWORK = 0x00,
    USER_BUSY = 0x91,
    ADD_INFO_WITH_NETWORK_PROBLEM_BUTT
};
typedef VOS_UINT8 SI_STK_AddInfoWithNetworkProblemUint8;

/*
 * 枚举说明: STK回复TERMINAL RESPONSE时SEND SMS/SEND SS/SEND USSD ERROR时附带的ADDITIONAL RESULT值
 * 1. 日    期: 2013年7月12日
 *    修改内容: 新增
 */
enum SI_STK_AddInfoForSsUssdSmsProblem {
    NO_SPECIFIC_CAUSE_CAN_BE_GIVEN = 0x00,
    ADD_INFO_FOR_SS_USSD_SMS_PROBLEM_BUTT
};
typedef VOS_UINT8 SI_STK_AddInfoForSsUssdSmsProblemUint8;

enum SI_STK_SetupCallConfirm {
    SI_STK_SETUPCALL_REJECT = 0x00, /* 用户拒绝发起呼叫 */
    SI_STK_SETUPCALL_ALLOW = 0x01,  /* 用户同意发起呼叫 */
    SI_STK_SETUPCALL_BUTT
};
typedef VOS_UINT32 SI_STK_SetupCallConfirmUint32;

enum {
    SI_STK_TRSEND = 0,
    SI_STK_ENVELOPESEND = 1,
    SI_STK_SEND_BUTT
};
typedef VOS_UINT32 SI_SendDataTypeUint32;

enum {
    SI_STK_CTRL_ALLOW_NOMODIFY = 0,
    SI_STK_CTRL_NOT_ALLOW = 1,
    SI_STK_CTRL_ALLOW_MODIFY = 2,
    SI_STK_CTRL_BUTT
};
typedef VOS_UINT32 SI_STK_CtrlResultUint32;

/* ME 状态 */
enum {
    SI_STK_ME_STATUS_IDLE = 0,
    SI_STK_ME_STATUS_NOT_IDLE = 1,
    SI_STK_ME_STATUS_BUTT
};
typedef VOS_UINT8 SI_STK_MeStatusUint8;

enum SI_STK_CtrlResultType {
    SI_STK_MO_CALL_CTRL = 0x00,   /* mo call control类型 */
    SI_STK_SS_CALL_CTRL = 0x01,   /* ss call control类型 */
    SI_STK_USSD_CALL_CTRL = 0x02, /* ussd call control类型 */
    SI_STK_SMS_CTRL = 0x03,       /* sms call control类型 */
    SI_STK_CTRL_RESULT_BUTT
};
typedef VOS_UINT32 SI_STK_CtrlResultTypeUint32;

enum SI_CC_SpeciTag {
    SI_CC_ADDRESS_TAG = 0x06,
    SI_CC_SS_TAG = 0x09,
    SI_CC_USSD_TAG = 0x0A,
    SI_CC_PDP_CONTEXT = 0x52,
    SI_CC_EPSPDN_TAG = 0x7C,
    ST_CC_SPECI_TAG_BUTT
};

typedef VOS_UINT8 SI_CCSpeciTagUint8;

/*
 * 枚举说明: NETWORK REJECTION中UPDATE和ATTACH的值
 * 1. 日    期: 2013年7月12日
 *    修改内容: 新增 参考协议31.111 8.92章节
 */
enum SI_STK_UpdateAttachType {
    SI_STK_NETWORK_REJ_TYPE_NORMAL_LOCATION_UPDATING = 0x00,
    SI_STK_NETWORK_REJ_TYPE_PERIODIC_UPDATING_LAU = 0x01,
    SI_STK_NETWORK_REJ_TYPE_IMSI_ATTACH = 0x02,
    SI_STK_NETWORK_REJ_TYPE_GPRS_ATTACH = 0x03,
    SI_STK_NETWORK_REJ_TYPE_COMBINED_GPRS_IMSI_ATTACH = 0x04,
    SI_STK_NETWORK_REJ_TYPE_RA_UPDATING = 0x05,
    SI_STK_NETWORK_REJ_TYPE_COMBINED_RALA_UPDATING = 0x06,
    SI_STK_NETWORK_REJ_TYPE_COMBINED_RALA_UPDATING_WITH_IMSI_ATTACH = 0x07,
    SI_STK_NETWORK_REJ_TYPE_PERIODIC_UPDATING_RAU = 0x08,
    SI_STK_NETWORK_REJ_TYPE_EPS_ATTACH = 0x09,
    SI_STK_NETWORK_REJ_TYPE_COMBINED_EPS_IMSI_ATTACH = 0x0A,
    SI_STK_NETWORK_REJ_TYPE_TA_UPDATING = 0x0B,
    SI_STK_NETWORK_REJ_TYPE_COMBINED_TA_LA_UPDATING = 0x0C,
    SI_STK_NETWORK_REJ_TYPE_COMBINED_TA_LA_UPDATING_WITH_IMSI_ATTACH = 0x0D,
    SI_STK_NETWORK_REJ_TYPE_PERIODIC_UPDATING_TAU = 0x0E,

    SI_STK_NETWORK_REJ_TYPE_5G_INITIAL = 0x0F,
    SI_STK_NETWORK_REJ_TYPE_5G_MOBILITY = 0x10,
    SI_STK_NETWORK_REJ_TYPE_5G_PERIODIC = 0x11,

    SI_STK_NETWORK_REJ_TYPE_BUTT
};
typedef VOS_UINT8 SI_STK_UpdateAttachTypeUint8;

/*
 * 枚举说明: CS域服务状态
 * 1. 日    期: 2013年7月12日
 *    修改内容: 新增
 */
enum SI_STK_ServiceStatus {
    SI_STK_NORMAL_SERVICE = 0x00,
    SI_STK_LIMITED_SERVICE = 0x01,
    SI_STK_NO_SERVICE = 0x02,
    SI_STK_SERVICE_BUTT
};

typedef VOS_UINT32 SI_STK_ServiceStatusUint32;

/*
 * 枚举说明: 接入技术参考协议 102223 8.61 Access technology
 * 1. 日    期: 2013年7月12日
 *    修改内容: 新增
 */
enum SI_STK_AccessTech {
    STK_ACCESS_TECH_GSM = 0x00,
    STK_ACCESS_TECH_EIA_533 = 0x01,
    STK_ACCESS_TECH_EIA_136 = 0x02,
    STK_ACCESS_TECH_UTRAN = 0x03,
    STK_ACCESS_TECH_TETRA = 0x04,
    STK_ACCESS_TECH_EIA_95 = 0x05,
    STK_ACCESS_TECH_CDMA2000 = 0x06,
    STK_ACCESS_TECH_CDMA2000_HPPD = 0x07,
    STK_ACCESS_TECH_EUTRAN = 0x08,
    STK_ACCESS_TECH_EHRPD = 0x09,
    STK_ACCESS_TECH_NR = 0x0A,

    STK_ACCESS_TECH_BUTT = 0xFF
};

typedef VOS_UINT8 SI_STK_AccessTechUint8;

/*
 * 枚举说明: 网络测量类型枚举
 */
enum SI_STK_NmrType {
    STK_WAS_QUERYTYPE_INTRA = 0x1,
    STK_WAS_QUERYTYPE_INTER = 0x2,
    STK_WAS_QUERYTYPE_INTER_GERAN = 0x3,
    STK_WAS_QUERYTYPE_INTER_EUTRAN = 0x4,
    STK_LRRC_QUERYTYPE_INTRA_FREQ = 0x5,
    STK_LRRC_QUERYTYPE_INTER_FREQ = 0x6,
    STK_LRRC_QUERYTYPE_INTER_GERAN = 0x7,
    STK_LRRC_QUERYTYPE_INTER_UTRAN = 0x8,
    STK_NMR_QUERYTYPE_BUTT
};
typedef VOS_UINT32 SI_STK_NmrTypeUint32;

typedef VOS_UINT8 SI_STK_SearchModeUint8;

enum SI_STK_EnvelopeSenderpara {
    /* 0~FFFF是NAS内部使用，用于区分TAF的相关模块 */
    SI_STK_AT_ENVELOPE = 0x10000,
    SI_STK_XSMS_ENVELOPE = 0x20000,
    SI_STK_ENVELOPE_BUTT
};

enum STK_NAS_Op {
    STK_NAS_STEERING_OF_ROAMING_IND = 0,
    STK_NAS_BUTT
};

/*
 * 枚举说明: STK模块处理的消息ID定义
 */
enum SI_STK_Msg {
    SI_STK_NULL = 0x00,
    SI_STK_GETMAINMNUE = 0x01,       /* 获取主菜单请求， AT发送 */
    SI_STK_GETCOMMAND = 0x02,        /* 获取主动命令请求， AT发送 */
    SI_STK_QUERYCOMMAND = 0x03,      /* 查询主动命令请求，AT发送 */
    SI_STK_TRDOWN = 0x04,            /* 主动命令执行结果下发，AT发送 */
    SI_STK_SIMPLEDOWN = 0x05,        /* 透明数据下发，AT发送 */
    SI_STK_ENVELOPEDOWN = 0x06,      /* 透明信封命令下发，AT发送 */
    SI_STK_PROFILESET = 0x07,        /* 设置Terimal  Profile的内容，AT发送，保留 */
    SI_STK_IMSICHG = 0x08,           /* 更改IMSI定制 */
    SI_STK_SETUPCALL_CONFIRM = 0x09, /* 用户确认是否发起呼叫 AT发送 */
    SI_STK_MENUSELECTION = 0x0A,     /* 主菜单选中下发，AT发送 */
    SI_STK_ENVELOPEDWON_REQ = 0x0B,  /* 信封命令下发，目前只有TAF发送的CC MO */
    SI_STK_MSG_BUTT
};
typedef VOS_UINT32 SI_STK_ReqmsgUint32;

enum SI_STK_Cnfmsg {
    SI_STK_NULL_CNF = 0x00,
    SI_STK_ENVELOPE_CNF = 0x01,
    SI_STK_TERMIANLRSP_CNF = 0x02,
    SI_STK_QUERYCOMMAND_CNF = 0x03,
    SI_STK_SETUPCALL_CONFIRM_CNF = 0x04,
    SI_STK_IMSICHG_CNF = 0x05,
    SI_STK_CNFMSG_BUTT
};
typedef VOS_UINT32 SI_STK_CnfmsgUint32;

typedef struct {
    VOS_UINT32 satDataLen;
    VOS_UINT8 satCmdData[256];
} SI_STK_CmdData;

typedef struct {
    VOS_UINT8 commandNum;
    VOS_UINT8 commandType;
    VOS_UINT8 commandQua;
    VOS_UINT8 rsv;
} SI_STK_CommandDetails;

typedef struct {
    VOS_UINT8 sdId;
    VOS_UINT8 ddId;
    VOS_UINT8 rsv[2];
} SI_STK_DeviceIdentities;

typedef struct {
    SI_STK_ResultUint8 result;
    VOS_UINT8 rsv[3];
} SI_STK_Result;

typedef struct {
    VOS_UINT8 unit;
    VOS_UINT8 internal;
    VOS_UINT8 rsv[2];
} SI_STK_Duration;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *alphabet;
} SI_STK_AlphaIdentifier;

typedef struct {
    VOS_UINT8 numType;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *addr;
} SI_STK_Address;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *ccpData;
} SI_STK_CapabilityCfgPara;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *subaddr;
} SI_STK_SubAddress;

typedef struct {
    VOS_UINT8 numType;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *ssString;
} SI_STK_SSString;

typedef struct {
    VOS_UINT8 dcScheme;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *ussdString;
} SI_STK_UssdString;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *tpdu;
} SI_STK_SmsTpdu;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *cbPage;
} SI_STK_CellBroadcastPage;

typedef struct {
    VOS_UINT8 dcs;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *text;
} SI_STK_TextString;

typedef struct {
    VOS_UINT8 tone;
    VOS_UINT8 rsv1;
    VOS_UINT8 rsv2[2];
} SI_STK_Tone;

typedef struct {
    VOS_UINT8 itemId;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *itemText;
} SI_STK_Item;

typedef struct {
    VOS_UINT8 itemId;
    VOS_UINT8 rsv[3];
} SI_STK_ItemIdentifier;

typedef struct {
    VOS_UINT8 minLen;
    VOS_UINT8 maxLen;
    VOS_UINT8 rsv[2];
} SI_STK_ResponseLength;

typedef struct {
    VOS_UINT8 fileNum;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *files;
} SI_STK_FileList;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *atsli;
} SI_STK_LocationInfo;

typedef struct {
    VOS_UINT8 imei[8];
} SI_STK_Imei;

typedef struct {
    VOS_UINT8 netMeasure[16];
} SI_STK_NetMeasurement;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *inAIl;
} SI_STK_ItemsNextactionInd;

typedef struct {
    VOS_UINT8 nmr;
    VOS_UINT8 rsv[3];
} SI_STK_UtranNmr;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *eventList;
} SI_STK_EventList;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *cause;
} SI_STK_Cause;

typedef struct {
    VOS_UINT8 locStatus;
    VOS_UINT8 rsv[3];
} SI_STK_LocationStatus;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *transaction;
} SI_STK_TransActionIdentifier;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *bcList;
} SI_STK_BcchChannelList;

typedef struct {
    VOS_UINT8 qualifier;
    VOS_UINT8 id;
    VOS_UINT8 rsv[2];
} SI_STK_IconIdentifier;

typedef struct {
    VOS_UINT8 iconlistQua;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *iconIdlist;
} SI_STK_ItemIconIdList;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *atr;
} SI_STK_CardAtr;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *capdu;
} SI_STK_CApdu;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *rapdu;
} SI_STK_RApdu;

typedef struct {
    VOS_UINT8 dataTime[7];
    VOS_UINT8 rsv;
} SI_STK_DateTimeZone;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *ccra;
} SI_STK_CallControlRequestedAction;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *atCmd;
} SI_STK_AtCommand;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *atrRsp;
} SI_STK_AtResponse;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *dtmFStr;
} SI_STK_DtmfString;

typedef struct {
    VOS_UINT8 language[2];
    VOS_UINT8 rsv[2];
} SI_STK_Language;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *timeAd;
} SI_STK_TimingAdvance;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *aid;
} SI_STK_Aid;

typedef struct {
    VOS_UINT8 browserId;
    VOS_UINT8 rsv[3];
} SI_STK_BrowserIdentity;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *url;
} SI_STK_Url;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *bearer;
} SI_STK_Bearer;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *provsionRef;
} SI_STK_ProReferenceFile;

typedef struct {
    VOS_UINT8 bearerType;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *bearer;
} SI_STK_BearerDescription;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *channelData;
} SI_STK_ChannelData;

typedef struct {
    VOS_UINT8 bufferSize[2];
    VOS_UINT8 rsv[2];
} SI_STK_BufferSize;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *networkAccessName;
} SI_STK_NetworkAccessAnme;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *bufferReader;
} SI_STK_BufferCardReaderIdentifier;

typedef struct {
    VOS_UINT8 transType;
    VOS_UINT8 portNum[2];
    VOS_UINT8 rsv;
} SI_STK_UiccTerminalTranlevel;

typedef struct {
    VOS_UINT8 barerTech;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *deviceFilter;
} SI_STK_DeviceFilter;

typedef struct {
    VOS_UINT8 locBerTechId;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *servRecord;
} SI_STK_ServiceRecord;

typedef struct {
    VOS_UINT8 barerTech;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *serviceSearch;
} SI_STK_ServiceSearch;

typedef struct {
    VOS_UINT8 barerTech;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *attributeInfo;
} SI_STK_AttributeInformation;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *esn;
} SI_STK_Esn;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *netName;
} SI_STK_NetworkAccessName;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *cdmaSms;
} SI_STK_CdmaSms;

typedef struct {
    VOS_UINT8 codeType;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *remoteEntity;
} SI_STK_RemoteEntityAddress;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *text;
} SI_STK_TextAttribute;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *tal;
} SI_STK_ItemTextAttributeList;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *pdpPara;
} SI_STK_PdpContextActivationParameters;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *imeisv;
} SI_STK_Imeisv;

typedef struct {
    VOS_UINT8 frameLayout;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *rsf;
} SI_STK_FrameLayout;

typedef struct {
    VOS_UINT8 netSearchMode;
    VOS_UINT8 rsv[3];
} SI_STK_NetworkSearchMode;

typedef struct {
    VOS_UINT8 frameId;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *fil;
} SI_STK_FramesInformation;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *frameID;
} SI_STK_FrameIdentifier;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *mmReference;
} SI_STK_MmMessageReference;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *mmId;
} SI_STK_MmMessageIdentifier;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *mmTStatus;
} SI_STK_MmMessageTransfer;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *mmCId;
} SI_STK_MmMessageContentId;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *mmNotification;
} SI_STK_MmMessageNotification;

typedef struct {
    VOS_UINT8 tech;
    VOS_UINT8 rsv[3];
} SI_STK_AccessTechnology;

typedef struct {
    VOS_UINT8 batState;
    VOS_UINT8 rsv[3];
} SI_STK_BatteryState;

typedef struct {
    VOS_UINT8 readerStatus;
    VOS_UINT8 rsv[3];
} SI_STK_CardReaderStatus;

typedef struct {
    VOS_UINT8 timerId;
    VOS_UINT8 rsv[3];
} SI_STK_TimerIdentifier;

typedef struct {
    VOS_UINT8 hour;
    VOS_UINT8 minute;
    VOS_UINT8 second;
    VOS_UINT8 rsv;
} SI_STK_TimerValue;

typedef struct {
    VOS_UINT8 channelLen;
    VOS_UINT8 rsv[3];
} SI_STK_ChannelDataLength;

typedef struct {
    VOS_UINT8 channelStatus[2];
    VOS_UINT8 rsv[2];
} SI_STK_ChannelStatus;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *service;
} SI_STK_ServiceAvailability;

typedef struct {
    VOS_UINT8 btc;
    VOS_UINT8 rsv[3];
} SI_STK_BrowserTerminCause;

typedef struct {
    VOS_UINT8 paraList[3];
    VOS_UINT8 rsv;
} SI_STK_DisplayParameters;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *browseStatus;
} SI_STK_BrowsingStatus;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *oplmnList;
} SI_STK_PlmnwactList;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *policy;
} SI_STK_EnfoecePolicy;

typedef struct {
    VOS_UINT8 rsv[4];
} SI_STK_StkhelpRequest;

typedef struct {
    VOS_UINT8 bcInd;
    VOS_UINT8 rsv1;
    VOS_UINT8 rsv[2];
} SI_STK_BCRepind;

typedef struct {
    SI_CCSpeciTagUint8 tag;
    VOS_UINT8 len;
    VOS_UINT8 rsv[2];
    VOS_UINT8 *value;
} SI_STK_CallctrlSpecial;

/* 31.111 8.91 Routing Area Identification */
typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *rauInfo;
} SI_STK_RauInfo;

/* 31.111 8.92 Update/Attach Type */
typedef struct {
    VOS_UINT8 tpye;
    VOS_UINT8 rsv[3];
} SI_STK_UpdtatAttachType;

/* 31.111 8.93 Rejection Cause Code */
typedef struct {
    VOS_UINT8 cause;
    VOS_UINT8 rsv[3];
} SI_STK_RejCauseCode;

/* 31.111 8.99 Tracking Area Identification */
typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *tauInfo;
} SI_STK_TauInfo;

/* STK命令数据结构定义 */
typedef struct {
    VOS_UINT32 opText : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opDuration : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opImmediateRsp : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 26;

    SI_STK_TextString textStr;
    SI_STK_IconIdentifier iconId;
    SI_STK_Duration duration;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_DisplayText;

typedef struct {
    VOS_UINT32 opText : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opDuration : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 27;

    SI_STK_TextString textStr;
    SI_STK_IconIdentifier iconId;
    SI_STK_Duration duration;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_GetInkey;

typedef struct {
    VOS_UINT32 opText : 1;
    VOS_UINT32 opRpLen : 1;
    VOS_UINT32 opDefaultText : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 26;

    SI_STK_TextString textStr;
    SI_STK_ResponseLength rspLen;
    SI_STK_TextString defaultText;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_GetInput;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opTone : 1;
    VOS_UINT32 opDuration : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 26;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_Tone tone;
    SI_STK_Duration duration;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_PlayTone;

typedef struct {
    VOS_UINT32 opDuration : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_Duration duration;
} SI_STK_Pollinterval;

typedef struct {
    VOS_UINT32 opAlphaID : 1;
    VOS_UINT32 opINAIndicator : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opIIIlist : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opITAList : 1;
    VOS_UINT32 opItem : 1;
    VOS_UINT32 opReserved : 25;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_ItemsNextactionInd inaIndicator;
    SI_STK_IconIdentifier iconId;
    SI_STK_ItemIconIdList iIIList;
    SI_STK_TextAttribute textAttr;
    SI_STK_ItemTextAttributeList iTAList;
    VOS_UINT32 itemNum;
    SI_STK_Item item[ITEMNUMMAX];
} SI_STK_SetUpMenu;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opINAIndicator : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opIIIlist : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opITAList : 1;
    VOS_UINT32 opItemIDTag : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opItemId : 1;
    VOS_UINT32 opReserved : 23;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_ItemsNextactionInd inaIndicator;
    SI_STK_IconIdentifier iconId;
    SI_STK_ItemIconIdList iIIList;
    SI_STK_TextAttribute textAttr;
    SI_STK_ItemTextAttributeList iTAList;
    VOS_UINT32 itemIDTag;
    VOS_UINT32 frameId;
    VOS_UINT32 itemNum;
    SI_STK_Item item[ITEMNUMMAX];
} SI_STK_SelectItem;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opAddr : 1;
    VOS_UINT32 opWgtpdu : 1;
    VOS_UINT32 opCtpdu : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 25;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_Address addr;
    SI_STK_SmsTpdu wgtpdu;
    SI_STK_SmsTpdu ctpdu;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_SendShortMessage;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opSSStr : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opReserved : 28;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_SSString ssString;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
} SI_STK_SendSS;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opUSSDStr : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opReserved : 28;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_UssdString ussdString;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
} SI_STK_SendUssd;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opAddr : 1;
    VOS_UINT32 opCCPara : 1;
    VOS_UINT32 opSubaddr : 1;
    VOS_UINT32 opDuration : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opEccId : 1;
    VOS_UINT32 opAlphaId2 : 1;
    VOS_UINT32 opIconId2 : 1;
    VOS_UINT32 opTextAttr2 : 1;
    VOS_UINT32 opReserved : 20;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_Address addr;
    SI_STK_CapabilityCfgPara capCfgPara;
    SI_STK_SubAddress subaddr;
    SI_STK_Duration duration;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
    SI_STK_AlphaIdentifier alphaId2;
    SI_STK_IconIdentifier iconId2;
    SI_STK_TextAttribute textAttr2;
} SI_STK_SetUpCall;

typedef struct {
    VOS_UINT32 opFileList : 1;
    VOS_UINT32 opAid : 1;
    VOS_UINT32 opAlpha : 1;
    VOS_UINT32 opIconID : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opOplmnList : 1;
    VOS_UINT32 opRefEnforcePolicy : 1;
    VOS_UINT32 opReserved : 24;

    SI_STK_FileList fileList;
    SI_STK_Aid aid;
    SI_STK_AlphaIdentifier alpha;
    SI_STK_IconIdentifier iconID;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
    SI_STK_PlmnwactList plmnwactList;
    SI_STK_EnfoecePolicy enforcePolicy;
} SI_STK_Refresh;

typedef struct {
    VOS_UINT32 opNmr : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_UtranNmr utranNmr;
} SI_STK_PrivideLocalInfo;

typedef struct {
    VOS_UINT32 opList : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_EventList eventList;
} SI_STK_SetUpEventList;

typedef struct {
    VOS_UINT32 opCapdu : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_CApdu capdu;
} SI_STK_PerformCardApdu;

typedef struct {
    VOS_UINT32 opTimerID : 1;
    VOS_UINT32 opTimerValue : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_TimerIdentifier timerID;
    SI_STK_TimerValue timerValue;
} SI_STK_Timermanage;

typedef struct {
    VOS_UINT32 opText : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 28;

    SI_STK_TextString textStr;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_SetUpIdleModeText;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opATCmd : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 27;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_AtCommand aTCmd;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_RunAtCommand;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opDtmf : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 27;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_DtmfString dtmfStr;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_SendDtmf;

typedef struct {
    VOS_UINT32 opLanguage : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_Language language;
} SI_STK_LanguageNotification;

typedef struct {
    VOS_UINT32 opBrowserId : 1;
    VOS_UINT32 opUrl : 1;
    VOS_UINT32 opBearer : 1;
    VOS_UINT32 opTexstr : 1;
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opPFRef : 1;
    VOS_UINT32 opReserved : 23;

    SI_STK_BrowserIdentity browserId;
    SI_STK_Url url;
    SI_STK_Bearer bearer;
    SI_STK_TextString textStr;
    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
    VOS_UINT32 pfNumber;
    SI_STK_ProReferenceFile pfRef[ITEMNUMMAX];
} SI_STK_LaunchBrowser;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opAddr : 1;
    VOS_UINT32 opSubaddr : 1;
    VOS_UINT32 opDuration1 : 1;
    VOS_UINT32 opBearerDesp : 1;
    VOS_UINT32 opBufSize : 1;
    VOS_UINT32 opNetworkAccName : 1;
    VOS_UINT32 opTextstr1 : 1;
    VOS_UINT32 opUTITLevel : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opOtherAddr1 : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opDuration2 : 1;
    VOS_UINT32 opTextstr2 : 1;
    VOS_UINT32 opOtherAddr2 : 1;
    VOS_UINT32 opReserved : 16;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_Address addr;
    SI_STK_SubAddress subaddr;
    SI_STK_Duration duration1;
    SI_STK_BearerDescription bearerDesp;
    SI_STK_BufferSize bufSize;
    SI_STK_NetworkAccessAnme networkAccName;
    SI_STK_TextString textStr1;
    SI_STK_UiccTerminalTranlevel uTITLevel;
    SI_STK_TextAttribute textAttr;
    SI_STK_Address otherAddr1;
    VOS_UINT32 frameId;
    SI_STK_Duration duration2;
    SI_STK_TextString textStr2;
    SI_STK_Address otherAddr2;
} SI_STK_OpenChannel;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 28;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_CloseChannel;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opChannleDataLen : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 27;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_TextAttribute textAttr;
    SI_STK_ChannelDataLength channleDataLen;
    VOS_UINT32 frameId;
} SI_STK_ReceiveData;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opChannelData : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 27;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_ChannelData channelData;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_SendData;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opService : 1;
    VOS_UINT32 opDeviceFilter : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 26;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_ServiceSearch serviceSearch;
    SI_STK_DeviceFilter deviceFilter;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_ServiceSearchProactive;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opAttInfo : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opFrameId : 1;
    VOS_UINT32 opReserved : 27;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_AttributeInformation attrInfo;
    SI_STK_TextAttribute textAttr;
    VOS_UINT32 frameId;
} SI_STK_GetServiceInformation;

typedef struct {
    VOS_UINT32 opServSRec : 1;
    VOS_UINT32 opUTITLevel : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_ServiceRecord serviceRecord;
    SI_STK_UiccTerminalTranlevel uTITLevel;
} SI_STK_DeclareService;

typedef struct {
    VOS_UINT32 opFrameLayout : 1;
    VOS_UINT32 opFrameId1 : 1;
    VOS_UINT32 opFrameId2 : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_FrameLayout frameLayout;
    VOS_UINT32 frameId1;
    VOS_UINT32 frameId2;
} SI_STK_SetFrames;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opMMRef : 1;
    VOS_UINT32 opFileList : 1;
    VOS_UINT32 opMMCId : 1;
    VOS_UINT32 opMMId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opReserved : 25;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_MmMessageReference mMReference;
    SI_STK_FileList fileList;
    SI_STK_MmMessageContentId mMCId;
    SI_STK_MmMessageIdentifier mMId;
    SI_STK_TextAttribute textAttr;
} SI_STK_RetrieveMmMessage;

typedef struct {
    VOS_UINT32 opAlphaId : 1;
    VOS_UINT32 opIconId : 1;
    VOS_UINT32 opFileList : 1;
    VOS_UINT32 opMMId : 1;
    VOS_UINT32 opTextAttr : 1;
    VOS_UINT32 opReserved : 27;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_IconIdentifier iconId;
    SI_STK_FileList fileList;
    SI_STK_MmMessageIdentifier mMId;
    SI_STK_TextAttribute textAttr;
} SI_STK_SubmitMmMessage;

typedef struct {
    VOS_UINT32 opFileList : 1;
    VOS_UINT32 opImmediateRsp : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_FileList fileList;
    SI_STK_MmMessageIdentifier mMId;
} SI_STK_DisplayMmMessage;

/* STK命令回复数据结构定义 */
typedef struct {
    VOS_UINT32 opTextStr : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_TextString textStr;
} SI_STK_GetInkeyRsp;

typedef struct {
    VOS_UINT32 opTextStr : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_TextString textStr;
} SI_STK_GetInputRsp;

typedef struct {
    VOS_UINT32 opItemID : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_ItemIdentifier itemId;
} SI_STK_SelectItemRsp;

typedef struct {
    VOS_UINT32 opLocInfo : 1;
    VOS_UINT32 opImei : 1;
    VOS_UINT32 opNMResults : 1;
    VOS_UINT32 opDTTZone : 1;
    VOS_UINT32 opLanguage : 1;
    VOS_UINT32 opAccessTech : 1;
    VOS_UINT32 opEsn : 1;
    VOS_UINT32 opImeisv : 1;
    VOS_UINT32 opNSMode : 1;
    VOS_UINT32 opBatteryState : 1;
    VOS_UINT32 opReserved : 22;

    SI_STK_LocationInfo locInfo;
    SI_STK_Imei imei;
    SI_STK_NetMeasurement nMResults;
    SI_STK_DateTimeZone dateTZone;
    SI_STK_Language language;
    SI_STK_AccessTechnology accessTech;
    SI_STK_Esn esn;
    SI_STK_Imeisv imeisv;
    SI_STK_NetworkSearchMode nsMode;
    SI_STK_BatteryState batteryState;
} SI_STK_ProvideLocalInformationRsp;

typedef struct {
    VOS_UINT32 opCCRAction : 1;
    VOS_UINT32 opResult2 : 1;
    VOS_UINT32 opTestStr2 : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_CallControlRequestedAction callCtrlRAction;
    SI_STK_Result result2;
    SI_STK_TextString textStr;
} SI_STK_SetUpCallRsp;

typedef struct {
    VOS_UINT32 opReadStatus : 1;
    VOS_UINT32 opReadId : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_CardReaderStatus readStatus;
    SI_STK_BufferCardReaderIdentifier readId;
} SI_STK_GetReaderStatusRsp;

typedef struct {
    VOS_UINT32 opCardATR : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_CardAtr cardATR;
} SI_STK_PowerOnCardRsp;

typedef struct {
    VOS_UINT32 opRapdu : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_RApdu rapdu;
} SI_STK_PerformCardApduRsp;

typedef struct {
    VOS_UINT32 opATRsp : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_AtResponse aTRsp;
} SI_STK_RunAtCommandRsp;

typedef struct {
    VOS_UINT32 opChannelStatus : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_ChannelStatus channelStatus;
} SI_STK_GetChannelStatusRsp;

typedef struct {
    VOS_UINT32 opChannelStatus : 1;
    VOS_UINT32 opBearerDesp : 1;
    VOS_UINT32 opBufferSize : 1;
    VOS_UINT32 opOtherAddr : 1;
    VOS_UINT32 opReserved : 28;

    SI_STK_ChannelStatus channelStatus;
    SI_STK_BearerDescription bearerDesp;
    SI_STK_BufferSize bufferSize;
    SI_STK_Address otherAddr;
} SI_STK_OpenChannelRsp;

typedef struct {
    VOS_UINT32 opServiceAvail : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_ServiceAvailability serviceAvail;
} SI_STK_ServiceSearchProactiveRsp;

typedef struct {
    VOS_UINT32 opServiceRecord : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_ServiceRecord serviceRecord;
} SI_STK_GetServiceInformationRsp;

typedef struct {
    VOS_UINT32 opFrameInfo : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_FramesInformation frameInfo;
} SI_STK_SetFramesRsp;

typedef struct {
    VOS_UINT32 opFrameInfo : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_FramesInformation frameInfo;
} SI_STK_GetFramesStatusRsp;

/* 事件下发数据结构定义 */
typedef struct {
    VOS_UINT32 opTransactionId : 1;
    VOS_UINT32 opAddr : 1;
    VOS_UINT32 opSubaddr : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_TransActionIdentifier transactionId;
    SI_STK_Address addr;
    SI_STK_SubAddress subaddr;
} SI_STK_MtCallEvent;

typedef struct {
    VOS_UINT32 opTransactionId : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_TransActionIdentifier transactionId;
} SI_STK_CallConnectedEvent;

typedef struct {
    VOS_UINT32 opTransactionId : 1;
    VOS_UINT32 opCause : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_TransActionIdentifier transactionId;
    SI_STK_Cause cause;
} SI_STK_CallDisconnectedEvent;

typedef struct {
    VOS_UINT32 opLocStatus : 1;
    VOS_UINT32 opLocInfo : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_LocationStatus locStatus;
    SI_STK_LocationInfo locInfo;
} SI_STK_LocationStatusEvent;

typedef struct {
    VOS_UINT32 opCRStatus : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_CardReaderStatus cardRStatus;
} SI_STK_CardReaderStatusEvent;

typedef struct {
    VOS_UINT32 opLanguage : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_Language language;
} SI_STK_LanguageSelectionEvent;

typedef struct {
    VOS_UINT32 opBTCause : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_BrowserTerminCause bTCause;
} SI_STK_BrowserTerminationEvent;

typedef struct {
    VOS_UINT32 opChannelStatus : 1;
    VOS_UINT32 opCDLen : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_ChannelStatus channelStatus;
    SI_STK_ChannelDataLength channeldataLen;
} SI_STK_DataAvailableEvent;

typedef struct {
    VOS_UINT32 opChannelStatus : 1;
    VOS_UINT32 opBearerDesp : 1;
    VOS_UINT32 opOtherAddr : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_ChannelStatus channelStatus;
    SI_STK_BearerDescription bearerDesp;
    SI_STK_Address otherAddr;
} SI_STK_ChannelStatusEvent;

typedef struct {
    VOS_UINT32 opAccessTech : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_AccessTechnology accessTech;
} SI_STK_AccessTechnologyChangeEvent;

typedef struct {
    VOS_UINT32 opDisplayPara : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_DisplayParameters displayPara;
} SI_STK_DisplayParametersChangedEvent;

typedef struct {
    VOS_UINT32 opServiceRecord : 1;
    VOS_UINT32 opREAddr : 1;
    VOS_UINT32 opUTITlevel : 1;
    VOS_UINT32 opRETLAddr : 1;
    VOS_UINT32 opReserved : 28;

    SI_STK_ServiceRecord serviceRecord;
    SI_STK_RemoteEntityAddress reAddr;
    SI_STK_UiccTerminalTranlevel utiTlevel;
    SI_STK_Address reTLAddr;
} SI_STK_LocalConnectionEvent;

typedef struct {
    VOS_UINT32 opNSMode : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_NetworkSearchMode nsMode;
} SI_STK_NetworkSearchModeChangeEvent;

typedef struct {
    VOS_UINT32 opBrowseStatus : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_BrowsingStatus browseStatus;
} SI_STK_BrowsingStatusEvent;

typedef struct {
    VOS_UINT32 opFrameInfo : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_FramesInformation frameInfo;
} SI_STK_FramesInformationEvent;

/*
 * 结构说明: NETWORK REJECTION事件数据结构
 * 1. 日    期: 2013年7月12日
 */
typedef struct {
    VOS_UINT32 opLocaInfo : 1;
    VOS_UINT32 opRauInfo : 1;
    VOS_UINT32 opTauInfo : 1;
    VOS_UINT32 opAccTech : 1;
    VOS_UINT32 opUpdateAttachType : 1;
    VOS_UINT32 opRejCauseCode : 1;
    VOS_UINT32 opReserved : 26;

    SI_STK_LocationInfo locaInfo;
    SI_STK_RauInfo rauInfo;
    SI_STK_TauInfo tauInfo;
    SI_STK_AccessTechnology accTech;
    SI_STK_UpdtatAttachType updateAttachType;
    SI_STK_RejCauseCode rejCauseCode;
} SI_STK_NetworkRejEvent;

/* Envelope命令数据容器定义 */
typedef struct {
    VOS_UINT32 opItemIp : 1;
    VOS_UINT32 opHelp : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_ItemIdentifier itemId;
    SI_STK_StkhelpRequest help;
} SI_STK_MenuSelection;

typedef struct {
    VOS_UINT32 opCapability1 : 1;
    VOS_UINT32 opSubaddr : 1;
    VOS_UINT32 opLocInfo : 1;
    VOS_UINT32 opCapability2 : 1;
    VOS_UINT32 opBCRepInd : 1;
    VOS_UINT32 opSepcialData : 1;
    VOS_UINT32 opReserved : 26;

    SI_STK_CapabilityCfgPara capability1;
    SI_STK_SubAddress subaddr;
    SI_STK_LocationInfo locInfo;
    SI_STK_CapabilityCfgPara capability2;
    SI_STK_BCRepind bCRepInd;
    SI_STK_CallctrlSpecial specialData;
} SI_STK_CallControl;

typedef struct {
    VOS_UINT32 opAddr1 : 1;
    VOS_UINT32 opAddr2 : 1;
    VOS_UINT32 opLocal : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_Address addr1;
    SI_STK_Address addr2;
    SI_STK_LocationInfo locInfo;
} SI_STK_MosmsControl;

typedef struct {
    VOS_UINT32 opAddr : 1;
    VOS_UINT32 opTpdu : 1;
    VOS_UINT32 opXtpdu : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_Address addr;
    SI_STK_SmsTpdu tpdu;
    SI_STK_CdmaSms xtpdu;
} SI_STK_SmsPpDownload;

typedef struct {
    VOS_UINT32 opCBPage : 1;
    VOS_UINT32 opReserved : 31;

    SI_STK_CellBroadcastPage cbPage;
} SI_STK_CbDownload;

typedef struct {
    VOS_UINT32 opFileList : 1;
    VOS_UINT32 opMMId : 1;
    VOS_UINT32 opMMTStatus : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_FileList fileList;
    SI_STK_MmMessageIdentifier mmId;
    SI_STK_MmMessageTransfer mmTStatus;
} SI_STK_MmsTransferStatus;

typedef struct {
    VOS_UINT32 opMMSNotify : 1;
    VOS_UINT32 opLastEnvelope : 1;
    VOS_UINT32 opReserved : 30;

    SI_STK_MmMessageNotification mMSNotify;
    VOS_UINT8 envelope;
    VOS_UINT8 rsv[3];
} SI_STK_MmsNoitfy;

/*
 * 结构说明: CALL CONTROL结果数据结构
 */
typedef struct {
    VOS_UINT16 dataLen;
    VOS_UINT8 dataType;
    VOS_UINT8 rsv;
    VOS_UINT8 data[SI_STK_DATA_MAX_LEN];
} SI_STK_CtrlData;

/*
 * 结构说明: CALL CONTROL地址信息结构
 */
typedef struct {
    VOS_UINT8 numType;
    VOS_UINT8 addrLen;
    VOS_UINT8 addr[SI_STK_ADDR_MAX_LEN];
} SI_STK_AddrInfo;

/*
 * 结构说明: MO SMS CONTROL结果上报信息结构
 */
typedef struct {
    SI_STK_AddrInfo dstAddrInfo;
    SI_STK_AddrInfo serCenterAddrInfo;
} SI_STK_MosmsctrlInfo;

/*
 * 结构说明: ALPHAID结构
 */
typedef struct {
    VOS_UINT32 alphaLen;
    VOS_UINT8 alphaId[SI_STK_DATA_MAX_LEN];
} SI_STK_AlphaidInfo;

/*
 * 结构说明: MO SMS CONTRL 或 MO CALL CONTROL结果上报结构
 */
typedef struct {
    union {
        SI_STK_CtrlData ctrlDataInfo;
        SI_STK_MosmsctrlInfo moSmsCtrlInfo;
    } uInfo;

    VOS_UINT8 type;
    VOS_UINT8 result;
    VOS_UINT16 rsv;
    SI_STK_AlphaidInfo alphaIdInfo;
} SI_STK_CcInd;

/*
 * 结构说明: 虚拟卡SMS PP DOWNLOAD数据上报结构
 */
typedef struct {
    VOS_UINT8 len;
    VOS_UINT8 data[SI_STK_DATA_MAX_LEN - 1];
} SI_STK_SmsPpDlDataInfo;

typedef struct {
    VOS_UINT32 opCCP1 : 1;
    VOS_UINT32 opSubAddr : 1;
    VOS_UINT32 opAlaph : 1;
    VOS_UINT32 opBCRepInd : 1;
    VOS_UINT32 opCCP2 : 1;
    VOS_UINT32 opSepcialData : 1;
    VOS_UINT32 opReserved : 26;

    SI_STK_CapabilityCfgPara capabilityCfgPara1;
    SI_STK_SubAddress subAddrStr;
    SI_STK_AlphaIdentifier alphaId;
    SI_STK_BCRepind bcRepInd;
    SI_STK_CapabilityCfgPara capabilityCfgPara2;
    SI_STK_CallctrlSpecial specialData;
} SI_STK_CallctrlRsp;

typedef struct {
    VOS_UINT32 opAlaph : 1;
    VOS_UINT32 opAddr1 : 1;
    VOS_UINT32 opAddr2 : 1;
    VOS_UINT32 opReserved : 29;

    SI_STK_AlphaIdentifier alphaId;
    SI_STK_Address addr1;
    SI_STK_Address addr2;
} SI_STK_MoSmsCtrlRsp;

typedef struct {
    union {
        SI_STK_CallctrlRsp callCtrlRsp;
        SI_STK_MoSmsCtrlRsp moSmsCtrlRsp;
    } resp;

    SI_STK_EnvelopeTypeUint32 envelopeType;
    VOS_UINT32 rsult;
} SI_STK_EnvelopeRsp;

/* STK命令数据容器定义 */
typedef struct {
    union {
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
        SI_STK_DisplayText displayText;
        SI_STK_GetInkey getInkey;
        SI_STK_GetInput getInput;
        SI_STK_PlayTone playTone;
        SI_STK_Pollinterval poolInterval;
        SI_STK_SetUpMenu setupMenu;
        SI_STK_SelectItem selectItem;
        SI_STK_SendShortMessage sendSMS;
        SI_STK_SendSS sendSS;
        SI_STK_SendUssd sendSSD;
        SI_STK_SetUpCall setupCall;
        SI_STK_Refresh refresh;
        SI_STK_PrivideLocalInfo privdelocalInfo;
        SI_STK_SetUpEventList seList;
        SI_STK_PerformCardApdu pcapdu;
        SI_STK_Timermanage timerManage;
        SI_STK_SetUpIdleModeText idleText;
        SI_STK_RunAtCommand atCmd;
        SI_STK_SendDtmf dtmFCmd;
        SI_STK_LanguageNotification langNotify;
        SI_STK_LaunchBrowser launchBrowser;
        SI_STK_OpenChannel openChannel;
        SI_STK_CloseChannel closeChannel;
        SI_STK_ServiceSearchProactive serviceSearch;
        SI_STK_GetServiceInformation serviceInfo;
        SI_STK_DeclareService declareService;
        SI_STK_SetFrames setFrames;
        SI_STK_RetrieveMmMessage retMMMsg;
        SI_STK_SubmitMmMessage submitMMMsg;
        SI_STK_DisplayMmMessage dspMMMsg;
#endif
        SI_STK_CcInd ccIndInfo;
#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)) || (defined(__PC_UT__)) || \
    (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
        SI_STK_SmsPpDlDataInfo smsPpDlDataInfo;
#endif
        VOS_UINT8 padding[1000]; /* 当前最大值为920，新增和修改该结构后需要在A核和C核重新计算最大值 */
    } cmdStru;

    SI_STK_CommandDetails cmdDetail;
    SI_STK_DeviceIdentities cmdDevice;
    SI_STK_CmdTypeUint32 satType;
    SI_STK_CmdData satCmd;
} SI_STK_DataInfo;

/* STK命令回复数据容器定义 */
typedef struct {
    union {
        SI_STK_GetInkeyRsp getInkeyRsp;
        SI_STK_GetInputRsp getInputRsp;
#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)) || (defined(__PC_UT__)) || \
    (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
        SI_STK_SelectItemRsp selectItemRsp;
#endif
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
        SI_STK_SetUpCallRsp setupCallRsp;
        SI_STK_SetUpCallRsp sendSSRsp;
        SI_STK_SetUpCallRsp sendUSSDRsp;
        SI_STK_GetReaderStatusRsp readStatusRsp;
        SI_STK_PowerOnCardRsp powerOnRsp;
        SI_STK_PerformCardApduRsp performAPDURsp;
        SI_STK_RunAtCommandRsp aTCmdRsp;
        SI_STK_ServiceSearchProactiveRsp serviceSearchRsp;
        SI_STK_GetServiceInformationRsp serviceInfoRsp;
        SI_STK_SetFramesRsp setFrameRsp;
        SI_STK_GetFramesStatusRsp getFrameStatusRsp;
#endif
        VOS_UINT8 padding[100]; /* 当前最大值为40，新增和修改该结构后需要在A核和C核重新计算最大值 */
    } uResp;

    SI_STK_CmdTypeUint32 cmdType;
    SI_STK_CommandDetails cmdDetail;
    SI_STK_DeviceIdentities deviceId;
    SI_STK_Result result;
    VOS_UINT8 data[SI_STK_DATA_MAX_LEN];
} SI_STK_TerminalRsp;

/* Envelope&Event数据结构声明 */
typedef struct {
    union {
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
        SI_STK_MtCallEvent mtCallEvent;
        SI_STK_CallConnectedEvent cellConnectEvent;
        SI_STK_CallDisconnectedEvent cellDisconnectEvent;
        SI_STK_LocationStatusEvent localSatusEvent;
        SI_STK_CardReaderStatusEvent cardReadStatusEvent;
        SI_STK_LanguageSelectionEvent lanSelectEvent;
        SI_STK_BrowserTerminationEvent browserTEvent;
        SI_STK_DataAvailableEvent dataAvailableEvent;
        SI_STK_ChannelStatusEvent channelStatusEvent;
        SI_STK_AccessTechnologyChangeEvent accTecChgEvent;
        SI_STK_DisplayParametersChangedEvent displayParaCEvent;
        SI_STK_LocalConnectionEvent localConnectEvent;
        SI_STK_NetworkSearchModeChangeEvent networkSearchModeCEvent;
        SI_STK_BrowsingStatusEvent browStatusEvent;
        SI_STK_FramesInformationEvent framInfoEvent;
        SI_STK_NetworkRejEvent networkRejEvent;
#endif
        VOS_UINT8 padding[100]; /* 当前最大值为52，新增和修改该结构后需要在A核和C核重新计算最大值 */
    } event;

    SI_STK_EventTypeUint32 eventDownTag;
    SI_STK_DeviceIdentities deviceId;
    VOS_UINT8 data[SI_STK_DATA_MAX_LEN];
} SI_STK_Eventdown;

typedef struct {
    union {
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
        SI_STK_SmsPpDownload ppDown;
        SI_STK_CbDownload cbDown;
        SI_STK_CallControl callCtrl;
        SI_STK_MosmsControl moSMSCtrl;
        SI_STK_MmsTransferStatus mmSTran;
        SI_STK_MmsNoitfy mmSNotify;
#endif
#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)) || (defined(__PC_UT__)) || \
    (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
        SI_STK_MenuSelection menuSelect;
#endif
        VOS_UINT8 padding[100]; /* 当前最大值为68，新增和修改该结构后需要在A核和C核重新计算最大值 */
    } envelope;

    SI_STK_EnvelopeTypeUint32 envelopeType;
    SI_STK_DeviceIdentities deviceId;
    VOS_UINT8 data[SI_STK_DATA_MAX_LEN];
} SI_STK_Envelope;

typedef struct {
    VOS_UINT8 sw1;
    VOS_UINT8 sw2;
    VOS_UINT8 rev[2];
} SI_STK_SwInfo;

typedef struct {
    SI_ClientIdTypteUint16 clientId;
    VOS_UINT8 opId;
    STK_CallbackEventUint8 cbEvent;
    VOS_UINT32 errorNo;
    SI_STK_DataInfo cmdDataInfo;
    SI_STK_SwInfo swInfo;
} SI_STK_EventInfo;

typedef struct {
    SI_ClientIdTypteUint16 clientId;
    VOS_UINT8 opId;
    VOS_UINT8 cmdQualify;
    VOS_UINT32 dataLen;
    VOS_UINT8 data[STK_PRINT_MAX_LEN];
} SI_STK_PrintdataCnf;

typedef VOS_VOID (*pfSTKCmdATPrint)(SI_PIH_CardSlotUint32 slotId, VOS_UINT16 *dataLen, SI_STK_DataInfo *pEvent);

typedef struct {
    SI_STK_CmdTypeUint32 cmdType;
    pfSTKCmdATPrint atPrintFunc;
} SI_STK_Atprint;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    VOS_UINT32 result;
    VOS_UINT32 queryType;
} STK_AS_NmrInfoReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
} STK_AS_TaInfoReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    VOS_UINT32 result;
    VOS_UINT8 meStatus;
    VOS_UINT8 taValue;
    VOS_UINT8 reserved[2];
} STK_AS_TaInfoCnf;

/*
 * 结构说明: LRRC给STK模块的小区频率信息结构
 */
typedef struct {
    VOS_UINT16 nmrLen;
    VOS_UINT8 cellFreq[2];
    VOS_UINT8 nmrData[100];
} STK_LAS_CellFreqInfo;

/*
 * 结构说明: LRRC给STK模块的测量回复
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    VOS_UINT32 result;
    SI_STK_NmrTypeUint32 queryType;
    VOS_UINT16 secondCellFreq;
    VOS_UINT16 cellFreqInfoNum;
    STK_LAS_CellFreqInfo cellInfo[3];
} STK_LAS_NmrInfo;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    VOS_UINT32 result;
    VOS_UINT32 queryType;
    VOS_UINT32 nmrLen;
    VOS_UINT8 nmrData[4];
} STK_WAS_NmrInfo;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;
    VOS_UINT32 result;
    VOS_UINT32 nmrLen;
    VOS_UINT8 nmrData[16];
    VOS_UINT32 bcchLen;
    VOS_UINT8 bcchData[40];
} STK_GAS_NmrInfo;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;      /* 消息名 */
    VOS_UINT16 oplmnListLen; /* Oplmn的列表长度 */
    VOS_UINT8 oplmnList[6];  /* Oplmn的列表内容 */
} STK_NAS_SteeringOfRoaming;

typedef struct {
    VOS_MSG_HEADER
    SI_STK_ReqmsgUint32 msgName;
    VOS_UINT32 sendPara;
    VOS_UINT16 atClient;
    VOS_UINT8 opID;
    VOS_UINT8 rsv;
} SI_STK_Reqmsgheader;

typedef struct {
    VOS_UINT32 opItemIp : 1;
    VOS_UINT32 opHelp : 1;
    VOS_UINT32 opReserved : 30;
} SI_STK_MenuselectionFlag;

typedef struct {
    VOS_UINT32 opAddr : 1;
    VOS_UINT32 opTpdu : 1;
    VOS_UINT32 opReserved : 30;
} SI_STK_SmsppFlag;

typedef struct {
    VOS_UINT32 opCBPage : 1;
    VOS_UINT32 opReserved : 31;
} SI_STK_CbdownloadFlag;

typedef struct {
    VOS_UINT32 opCapability1 : 1;
    VOS_UINT32 opSubaddr : 1;
    VOS_UINT32 opLocInfo : 1;
    VOS_UINT32 opCapability2 : 1;
    VOS_UINT32 opBCRepInd : 1;
    VOS_UINT32 opSepcialData : 1;
    VOS_UINT32 opReserved : 26;
} SI_STK_CallctrlFlag;

typedef struct {
    VOS_UINT32 opAddr1 : 1;
    VOS_UINT32 opAddr2 : 1;
    VOS_UINT32 opLocal : 1;
    VOS_UINT32 opReserved : 29;
} SI_STK_MosmsctrlFlag;

typedef struct {
    union {
        SI_STK_SmsppFlag smsPPFlag;
        SI_STK_CbdownloadFlag cbDownFlag;
        SI_STK_CallctrlFlag callCtrlFlag;
        SI_STK_MosmsctrlFlag moSmsCtrlFlag;
    } reqFlag;

    VOS_UINT32 dataLen;
    VOS_UINT8 data[SI_STK_DATA_MAX_LEN];
} SI_STK_EnvelopeDown;

typedef struct {
    SI_STK_Reqmsgheader msgHeader;
    VOS_BOOL transFlag;
    SI_STK_EnvelopeTypeUint32 envelopeType;
    SI_STK_EnvelopeDown envelopeData;
} SI_STK_EnvelopedownReq;

typedef struct {
    VOS_MSG_HEADER
    SI_STK_CnfmsgUint32 msgName;
    VOS_UINT32 sendPara;
    VOS_UINT32 result;
    VOS_UINT32 errorNo;
    VOS_UINT16 atClientId;
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_STK_SwInfo swData;
} SI_STK_CnfMsgHeader;

typedef struct {
    VOS_UINT32 opCcp1 : 1;
    VOS_UINT32 opSubAddr : 1;
    VOS_UINT32 opAlaph : 1;
    VOS_UINT32 opBCRepInd : 1;
    VOS_UINT32 opCcp2 : 1;
    VOS_UINT32 opSepcialData : 1;
    VOS_UINT32 opReserved : 26;
} SI_STK_CallctrlCnfflag;

typedef struct {
    VOS_UINT32 opAlaph : 1;
    VOS_UINT32 opAddr1 : 1;
    VOS_UINT32 opAddr2 : 1;
    VOS_UINT32 opReserved : 29;
} SI_STK_MosmsctrlCnfflag;

typedef struct {
    union {
        SI_STK_CallctrlCnfflag callCtrlCnfFlag;
        SI_STK_MosmsctrlCnfflag moSmsCtrlCnfFlag;
    } uRespFlag;

    VOS_UINT32 rspResult;
    VOS_UINT32 rspDataLen;
    VOS_UINT8 rspData[SI_STK_DATA_MAX_LEN];
} SI_STK_EnvelopeCnfdata;

typedef struct {
    SI_STK_CnfMsgHeader cmdResult;
    SI_STK_EnvelopeTypeUint32 envelopeType;
    VOS_BOOL transFlag;
    SI_STK_EnvelopeCnfdata cnfData;
} SI_STK_EnvelopeDwonCnf;

extern VOS_UINT32 SI_STK_EnvelopeData_Code(ModemIdUint16 modemId, const SI_STK_Envelope *eNStru, VOS_UINT32 *dataLen,
    USIMM_U8LvData *eventData, VOS_UINT32 *dataOffset);

extern VOS_UINT32 SI_STK_EnvelopeRsp_Decode(ModemIdUint16 modemId, SI_STK_EnvelopeTypeUint32 dataType,
    VOS_UINT32 dataLen, const VOS_UINT8 *cmdData, SI_STK_EnvelopeRsp *rspData);

extern VOS_VOID SI_STK_EnvelopeRspDataFree(ModemIdUint16 modemId, SI_STK_EnvelopeRsp *data);

#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
extern VOS_UINT32 SI_STK_MenuSelectionFunc(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
    const SI_STK_Envelope *eNStru);

extern VOS_UINT32 SI_STK_GetMainMenu(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

extern VOS_UINT32 SI_STK_GetSTKCommand(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_STK_CmdTypeUint32 cmdType);

extern VOS_UINT32 SI_STK_QuerySTKCommand(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

extern VOS_UINT32 SI_STK_TerminalResponse(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
    const SI_STK_TerminalRsp *tRStru);

extern VOS_UINT32 SI_STK_DataSendSimple(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, SI_SendDataTypeUint32 sendType,
    VOS_UINT32 dataLen, const VOS_UINT8 *data);

extern VOS_UINT32 SI_STKDualIMSIChangeReq(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);
#endif

extern VOS_VOID SI_STKGetCurImsiSign(ModemIdUint16 modemId, VOS_UINT16 *dualIMSIEnable, VOS_UINT32 *curImsiSign);

extern VOS_UINT32 SI_STKIsDualImsiSupport(VOS_VOID);

extern VOS_VOID SI_STK_PidMsgProc(PS_SI_Msg *msg);

extern VOS_UINT32 SI_STKEventCallback(SI_PIH_CardSlotUint32 slotId, SI_STK_EventInfo *event);

extern VOS_UINT32 SI_STKCallback(SI_PIH_CardSlotUint32 slotId, SI_STK_EventInfo *event);

extern VOS_VOID SI_STKCallBack_BroadCast(SI_PIH_CardSlotUint32 slotId, SI_STK_EventInfo *event);

extern VOS_UINT32 WuepsSTKPidInit(enum VOS_InitPhaseDefine initPhrase);

#if ((1 < MULTI_MODEM_NUMBER) && (FEATURE_PHONE_USIM == FEATURE_ON))
extern VOS_VOID I1_SI_STK_PidMsgProc(PS_SI_Msg *msg);

extern VOS_UINT32 I1_WuepsSTKPidInit(enum VOS_InitPhaseDefine initPhrase);

extern VOS_VOID I2_SI_STK_PidMsgProc(PS_SI_Msg *msg);

extern VOS_UINT32 I2_WuepsSTKPidInit(enum VOS_InitPhaseDefine initPhrase);
#endif

#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
/*
 * 功能说明: 用户回复是否允许发起呼叫
 * 1. 日    期: 2012年9月14日
 */
extern VOS_UINT32 SI_STK_SetUpCallConfirm(MN_CLIENT_ID_T clientId, SI_STK_SetupCallConfirmUint32 action);
#endif

/*
 * 功能说明: 将MO CALL CONTROL的结果广播上报给AT
 *           pstRspData--指向CALL CONTROL的解码后码流
 * 1. 日    期: 2012年09月14日
 */
extern VOS_VOID SI_STK_CCResultInd(ModemIdUint16 modemId, const SI_STK_EnvelopeRsp *rspData);

/*
 * 功能说明: 将MO SMS CONTROL的结果广播上报给AT
 *           pstRspData--指向SMS CONTROL的解码后码流
 * 1. 日    期: 2012年09月14日
 */
extern VOS_VOID SI_STK_SMSCtrlResultInd(ModemIdUint16 modemId, const SI_STK_EnvelopeRsp *rspData);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of SiAppSim.h */
