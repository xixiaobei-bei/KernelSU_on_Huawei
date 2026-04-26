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
#ifndef __MN_MSG_API_H__
#define __MN_MSG_API_H__

#include "vos.h"
#include "nv_stru_gucnas.h"
#include "product_config.h"
#include "taf_type_def.h"
#include "at_mn_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* Deleted MN_MSG_7BIT_MASK */
#define MN_MSG_RP_REPORT_CAUSE_VALUE_MASK (0xff)

/* ------------------------TP-MTI(TP-Message-Type-Indicator,Bit 0 and 1)--------- */
#define MN_MSG_TP_MTI_DELIVER (0x00)        /* SMS-DELIVER       */
#define MN_MSG_TP_MTI_DELIVER_REPORT (0x00) /* SMS-DELIVER-REPORT */
#define MN_MSG_TP_MTI_STATUS_REPORT (0x02)  /* SMS-STATUS-REPORT */
#define MN_MSG_TP_MTI_COMMAND (0x02)        /* SMS-COMMAND       */
#define MN_MSG_TP_MTI_SUBMIT (0x01)         /* SMS-SUBMIT        */
#define MN_MSG_TP_MTI_SUBMIT_REPORT (0x01)  /* SMS-SUBMIT-REPORT */
#define MN_MSG_TP_MTI_RESERVE (0x03)        /* RESERVE           */

#define MN_MSG_TIMESTAMP_SIGN_MASK (0x08)
#define MN_MSG_MAX_TIMEZONE_VALUE (48)

#define MN_MSG_NVIM_ITEM_ACTIVE (1)   /* NVIM项已激活 */
#define MN_MSG_NVIM_ITEM_INACTIVE (0) /* NVIM项未激活 */

typedef VOS_UINT8 MN_MSG_ADDR_NUM_TYPE_T;

/* (2 - 1)  参考协议 24011 8.2.5.1 减去一个字节是描述长度的字节 */
#define MN_MSG_MIN_BCD_NUM_LEN (1)
#define MN_MSG_MAX_BCD_NUM_LEN (10) /* (11 - 1) 同上 */

#define MN_MSG_DCS_DATA_LGU_VALUE (0x84)
#define MN_MSG_MIN_RP_DATA_LEN (5)

/* 需要与CBS_CMAS_GEO_AREA_MAX_LEN保持一致，因L接入层协议暂无最大长度限制描述与L接入层讨论暂定1024，待接入层有详细协议依据，再根据协议进行修改 */
#define TAF_CBS_CMAS_GEO_AREA_MAX_LEN (1024)

typedef VOS_UINT8 MN_MSG_OPER_TYPE_T;
#define MN_MSG_OPER_SET 0
#define MN_MSG_OPER_GET 1
#define MN_MSG_OPER_CHANGED 2

#define MN_MSG_CMGD_PARA_MAX_LEN 16
/* Content of TPDU */
#define MN_MSG_MAX_7_BIT_LEN 160
#define MN_MSG_MAX_8_BIT_LEN 140
#define MN_MSG_MAX_LEN 255
#define MN_MAX_ASCII_ADDRESS_NUM 20
#define MSG_MAX_TPDU_MSG_LEN 232

#define MN_MSG_ABSOLUTE_TIMESTAMP_LEN 0x07

#define MN_MSG_MAX_UDH_EO_DATA_LEN 131
#define MN_MSG_MAX_UDH_EO_NUM 7

#define MN_GET_ADDRTYPE(ucAddrType, enNumType, enNumPlan) \
    ((ucAddrType) = 0x80 | (VOS_UINT8)((enNumType) << 4) | enNumPlan)

#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))

#define TAF_CBA_MAX_CBPAGE_LEN (88)  /* 广播短消息一页的最大长度 */
#define TAF_CBA_MAX_CBDATA_LEN (82)  /* 广播短消息页数据区的最大长度 */
#define TAF_CBA_MAX_CBMPAGE_NUM (15) /* 广播短消息最多包含的页数 */
/* 用户最多可输入的消息ID范围个数，在接受模式下还要受到CBMIR文件大小限制 */
#define TAF_CBA_MAX_CBMID_RANGE_NUM (100)
/* 用户最多可输入的描述信息的长度，单位BYTE */
#define TAF_CBA_MAX_LABEL_NUM (16)
#define TAF_CBA_MAX_RAW_CBDATA_LEN (93) /* 广播短消息页数据区的最大长度 */

#endif

/* DCS GROUP为1，则CB DATA字段携带语言类型，占2个7位位组长度 */
#define TAF_MSG_CBA_LANG_LENGTH (2)

#define MN_MSG_OTA_SECURITY_SMS_DSC_DATA (0xf6)


enum NAS_SMS_FailCloseLinkCtrlCfg {
    NAS_SMS_FAIL_CLOSE_LINK_CTRL_CFG_DISABLE = 0,
    NAS_SMS_FAIL_CLOSE_LINK_CTRL_CFG_ENABLE  = 1,

    NAS_SMS_FAIL_CLOSE_LINK_CTRL_CFG_BUTT
};
typedef VOS_UINT8 NAS_SMS_FailCloseLinkCtrlCfgUint8;


enum MN_MSG_MoDomainCustomizeType {
    MN_MSG_MO_DOMAIN_CUSTOMIZE_GLOBAL,
    MN_MSG_MO_DOMAIN_CUSTOMIZE_PROTOCOL,
    MN_MSG_MO_DOMAIN_CUSTOMIZE_BUTT
};
typedef VOS_UINT8 MN_MSG_MoDomainCustomizeTypeUint8;

enum MN_MSG_DateInvalidType {
    MN_MSG_DATE_INVALID_YEAR   = 0x01,
    MN_MSG_DATE_INVALID_MONTH  = 0x02,
    MN_MSG_DATE_INVALID_DAY    = 0x04,
    MN_MSG_DATE_INVALID_HOUR   = 0x08,
    MN_MSG_DATE_INVALID_MINUTE = 0x10,
    MN_MSG_DATE_INVALID_SECOND = 0x20,
    MN_MSG_DATE_INVALID_MAX
};
typedef VOS_UINT8 MN_MSG_DateInvalidTypeUint8;

/*
 * 枚举说明: 短信相关桩类型
 *          0: efsms文件不可用
 *          1: efsms文件容量为1
 *          2: efsmss溢出标志清除操作；
 *          3: efsmsp文件获取无响应；
 *          4: efsms文件获取无响应；
 *          5: 触发STK短信发送，STK短信内容需要压缩编码
 *          6: 触发STK短信发送，STK短信长度超过标准短信长度，需要分段发送；
 *          7: 去使能短信重发功能；
 *          8: 设置CLASS0类短信的定制配置；
 *          9: 设置PP DOWNLOAD操作失败，并指定失败原因值
 *         10: PS域短信连发开关
 *         11: 触发短信自动回复功能
 */
enum MN_MSG_StubType {
    MN_MSG_STUB_TYPE_SET_EFSMS_INAVAILABLE  = 0,
    MN_MSG_STUB_TYPE_SET_EFSMS_CAPACITY     = 1,
    MN_MSG_STUB_TYPE_EFSMSS_RESET           = 2,
    MN_MSG_STUB_TYPE_EFSMSP_GET_NO_RESPONSE = 3,
    MN_MSG_STUB_TYPE_EFSMS_GET_NO_RESPONSE  = 4,
    MN_MSG_STUB_TYPE_STK_8BIT_TO_7BIT       = 5,
    MN_MSG_STUB_TYPE_STK_SEGMENT            = 6,
    MN_MSG_STUB_TYPE_DISABLE_MO_RETRY       = 7,
    MN_MSG_STUB_TYPE_CLASS0_TAILOR          = 8,
    MN_MSG_STUB_TYPE_PP_DOWNLOAD_RSP_TYPE   = 9,
    MN_MSG_STUB_TYPE_PS_CONCATENATE_FLAG    = 10,
    MN_MSG_STUB_TYPE_SMS_AUTO_REPLY         = 11,
    MN_MSG_STUB_TYPE_BUTT
};
typedef VOS_UINT32 MN_MSG_StubTypeUint32;

enum MN_MSG_Link_Ctrl {
    MN_MSG_LINK_CTRL_DISABLE,
    MN_MSG_LINK_CTRL_ONEOFF_ENABLE,
    MN_MSG_LINK_CTRL_ENABLE,
    MN_MSG_LINK_CTRL_BUTT
};
typedef VOS_UINT8 MN_MSG_LinkCtrlUint8;

/* Type fo Sms TPDU */
enum MN_MSG_TpduType {
    MN_MSG_TPDU_DELIVER,         /* SMS DELIVER type */
    MN_MSG_TPDU_DELIVER_RPT_ACK, /* SMS DELIVER REPORT for RP ACK */
    MN_MSG_TPDU_DELIVER_RPT_ERR, /* SMS DELIVER REPORT for RP ERROR */
    MN_MSG_TPDU_STARPT,          /* SMS STATUS REPORT type */
    MN_MSG_TPDU_COMMAND,         /* SMS COMMAND type */
    MN_MSG_TPDU_SUBMIT,          /* SMS SUBMIT type */
    MN_MSG_TPDU_SUBMIT_RPT_ACK,  /* SMS SUBMIT REPORT for RP ACK */
    MN_MSG_TPDU_SUBMIT_RPT_ERR,  /* SMS SUBMIT REPORT for RP ERROR */
    MN_MSG_TPDU_MAX
};
typedef VOS_UINT8 MN_MSG_TpduTypeUint8;

/* Sms Rp Cause */
enum MN_MSG_RpCause {
    MN_MSG_RP_CAUSE_UNASSIGNED_NUMBER           = 1,  /* Unassigned (unallocated) number */
    MN_MSG_RP_CAUSE_OPERATOR_DETERMINED_BARRING = 8,  /* Operator determined barring */
    MN_MSG_RP_CAUSE_CALL_BARRED                 = 10, /* Call barred */
    MN_MSG_RP_CAUSE_RESERVED                    = 11, /* Reserved */
    MN_MSG_RP_CAUSE_SM_TRANSFER_REJECTED        = 21, /* Short message transfer rejected */
    MN_MSG_RP_CAUSE_MEMORY_EXCEEDED             = 22, /* Memory capacity exceeded */
    MN_MSG_RP_CAUSE_DESTINATION_OUT_OF_ORDER    = 27, /* Destination out of order */
    MN_MSG_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER     = 28, /* Unidentified subscriber */
    MN_MSG_RP_CAUSE_FACILITY_REJECTED           = 29, /* Facility rejected */
    MN_MSG_RP_CAUSE_UNKNOWN_SUBSCRIBER          = 30, /* Unknown subscriber */
    MN_MSG_RP_CAUSE_NETWORK_OUT_OF_ORDER        = 38, /* Network out of order */
    MN_MSG_RP_CAUSE_TEMPORARY_FAILURE           = 41, /* Temporary failure */
    MN_MSG_RP_CAUSE_CONGESTION                  = 42, /* Congestion */
    /* Resources unavailable, unspecified */
    MN_MSG_RP_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED = 47,
    /* Requested facility not subscribed */
    MN_MSG_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED = 50,
    /* Requested facility not implemented */
    MN_MSG_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED = 69,
    /* Invalid short message transfer reference value */
    MN_MSG_RP_CAUSE_INVALID_SM_TRANSFER_REFERENCE_VALUE = 81,
    MN_MSG_RP_CAUSE_INVALID_MSG_UNSPECIFIED             = 95, /* Semantically incorrect message */
    MN_MSG_RP_CAUSE_INVALID_MANDATORY_INFORMATION       = 96, /* Invalid mandatory information */
    /* Message type non existent or not implemented */
    MN_MSG_RP_CAUSE_MSG_TYPE_NON_EXISTENT = 97,
    /* Message not compatible with short message protocol state */
    MN_MSG_RP_CAUSE_MSG_NOT_COMPATIBLE = 98,
    /* Information element non existent or not implemented */
    MN_MSG_RP_CAUSE_IE_NON_EXISTENT          = 99,
    MN_MSG_RP_CAUSE_PROTOCOL_ERR_UNSPECIFIED = 111, /* Protocol error, unspecified */
    MN_MSG_RP_CAUSE_INTERWORKING_UNSPECIFIED = 127  /* Interworking, unspecified */
};
typedef VOS_UINT8 MN_MSG_RpCauseUint8;

/* Sms Tp Cause */
enum MN_MSG_TpCause {
    /* Telematic interworking not supported */
    MN_MSG_TP_CAUSE_TELEMATIC_NOT_SUPPORTED = 0x80,
    /* Short message Type 0 not supported */
    MN_MSG_TP_CAUSE_SM_TYPE0_NOT_SUPPORTED = 0x81,
    MN_MSG_TP_CAUSE_CANNOT_REPLACE_SM      = 0x82, /* Cannot replace short message */
    MN_MSG_TP_CAUSE_UNSPECIFIED_PID_ERR    = 0x8f, /* Unspecified TP-PID error */
    /* Data coding scheme (alphabet) not supported */
    MN_MSG_TP_CAUSE_DCS_NOT_SUPPORTED       = 0x90,
    MN_MSG_TP_CAUSE_MSG_CLASS_NOT_SUPPORTED = 0x91, /* Message class not supported */
    MN_MSG_TP_CAUSE_UNSPECIFIED_DCS_ERR     = 0x9f, /* Unspecified TP-DCS error */
    MN_MSG_TP_CAUSE_CANNOT_PERFORM_CMD      = 0xa0, /* Command cannot be actioned */
    MN_MSG_TP_CAUSE_CMD_NOT_SUPPORTED       = 0xa1, /* Command unsupported */
    MN_MSG_TP_CAUSE_UNSPECIFIED_CMD_ERR     = 0xaf, /* Unspecified TP-Command error */
    MN_MSG_TP_CAUSE_TPDU_NOT_SUPPORTED      = 0xb0, /* TPDU not supported */
    MN_MSG_TP_CAUSE_SC_BUSY                 = 0xc0, /* SC busy */
    MN_MSG_TP_CAUSE_NO_SC_SUBSCRIPTION      = 0xc1, /* No SC subscription */
    MN_MSG_TP_CAUSE_SC_FAILURE              = 0xc2, /* SC system failure */
    MN_MSG_TP_CAUSE_INVALID_SME_ADDRESS     = 0xc3, /* Invalid SME address */
    MN_MSG_TP_CAUSE_DESTINATION_SME_BARRED  = 0xc4, /* Destination SME barred */
    MN_MSG_TP_CAUSE_REJECTED_DUPLICATE_SM   = 0xc5, /* SM Rejected-Duplicate SM */
    MN_MSG_TP_CAUSE_VPF_NOT_SUPPORTED       = 0xc6, /* TP-VPF not supported */
    MN_MSG_TP_CAUSE_VP_NOT_SUPPORTED        = 0xc7, /* TP-VP not supported */
    MN_MSG_TP_CAUSE_SIM_FULL                = 0xd0, /* (U)SIM SMS storage full */
    /* No SMS storage capability in (U)SIM */
    MN_MSG_TP_CAUSE_NO_SM_STORAGE_IN_SIM  = 0xd1,
    MN_MSG_TP_CAUSE_ERR_IN_MS             = 0xd2, /* Error in MS */
    MN_MSG_TP_CAUSE_MEMORY_FULL           = 0xd3, /* Memory Capacity Exceeded */
    MN_MSG_TP_CAUSE_SAT_BUSY              = 0xd4, /* (U)SIM Application Toolkit Busy */
    MN_MSG_TP_CAUSE_SIM_DATA_DOWNLOAD_ERR = 0xd5, /* (U)SIM data download error */
    MN_MSG_TP_CAUSE_MAX_RESERVED_VALUE    = 0xdf,
    MN_MSG_TP_CAUSE_UNSPECIFIED_ERR       = 0xff
};
typedef VOS_UINT8 MN_MSG_TpCauseUint8;

/* IMS cause */
enum MN_MSG_ImsCause {
    MN_MSG_IMS_CAUSE_INNER_ERROR      = 0x01,
    MN_MSG_IMS_CAUSE_SMS_INCAPABILITY = 0x06,
    MN_MSG_IMS_CAUSE_SMS_NO_SMSC      = 0x07,
    MN_MSG_IMS_CAUSE_SMS_NO_IPSMGW    = 0x08,

    MN_MSG_IMS_CAUSE_IMS_FORBIDDEN       = 0x193, /* #403 */
    MN_MSG_IMS_CAUSE_IMS_REQUEST_TIMEOUT = 0x198, /* #408 */

    MN_MSG_IMS_CAUSE_UNSPECIFIED_ERR = 0xffff
};
typedef VOS_UINT16 MN_MSG_ImsCauseUint16;


enum MN_MSG_ImsaCause {
    MN_MSG_IMSA_CAUSE_NO_SERVICE    = 0x01,
    MN_MSG_IMSA_CAUSE_SMS_OPERATING = 0x02,

    MN_MSG_IMSA_CAUSE_NO_RF = 0x03,
    MN_MSG_IMSA_CAUSE_FT_IMS_SPECIAL_FAIL_CELLULAR_ROAM_IMS_IN_WIFI = 0x04,
    MN_MSG_IMSA_CAUSE_SEND_MESSAGE_PROTECT_TIMEOUT                  = 0x05, /* 法电需求，短信发送8s定时器超时原因值 */

    MN_MSG_IMSA_CAUSE_UNSPECIFIED_ERR = 0xff
};
typedef VOS_UINT8 MN_MSG_ImsaCauseUint8;

/* Sms Pid */
enum MN_MSG_TpPidType {
    MN_MSG_TP_PID_DEFAULT = 0x00, /* default value to use */
    /* implicit   device type is specific to this SC, or can be concluded on the basis of the address */
    MN_MSG_TP_PID_IMPLICIT = 0x20,
    /* telex (or teletex reduced to telex format) */
    MN_MSG_TP_PID_TELEX  = 0x21,
    MN_MSG_TP_PID_G3_FAX = 0x22, /* group 3 telefax */
    MN_MSG_TP_PID_G4_FAX = 0x23, /* group 4 telefax */
    /* voice telephone (i.e. conversion to speech) */
    MN_MSG_TP_PID_VOICE_PHONE = 0x24,
    /* ERMES (European Radio Messaging System) */
    MN_MSG_TP_PID_ERMES = 0x25,
    /* National Paging system (known to the SC) */
    MN_MSG_TP_PID_NATIONAL_PAGING = 0x26,
    MN_MSG_TP_PID_VIDEOTEX        = 0x27, /* Videotex */
    MN_MSG_TP_PID_TELETEX_UNSPEC  = 0x28, /* teletex, carrier unspecified */
    MN_MSG_TP_PID_TELETEX_PSPDN   = 0x29, /* teletex, in PSPDN */
    MN_MSG_TP_PID_TELETEX_CSPDN   = 0x2a, /* teletex, in CSPDN */
    MN_MSG_TP_PID_TELETEX_PSTN    = 0x2b, /* teletex, in analog PSTN */
    MN_MSG_TP_PID_TELETEX_ISDN    = 0x2c, /* teletex, in digital ISDN */
    MN_MSG_TP_PID_UCI             = 0x2d, /* UCI */
    /* a message handling facility (known to the SC) */
    MN_MSG_TP_PID_MSG_HANDLING = 0x30,
    /* any public X.400 based message handling system */
    MN_MSG_TP_PID_X400           = 0x31,
    MN_MSG_TP_PID_INTERNET_EMAIL = 0x32, /* Internet Electronic Mail */
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_SC_SPECIFIC_1 = 0x38,
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_SC_SPECIFIC_2 = 0x39,
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_SC_SPECIFIC_3 = 0x3a,
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_SC_SPECIFIC_4 = 0x3b,
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_SC_SPECIFIC_5 = 0x3c,
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_SC_SPECIFIC_6 = 0x3d,
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_SC_SPECIFIC_7 = 0x3e,
    /* values specific to each SC, usage based on mutual agreement between the SME and the SC */
    MN_MSG_TP_PID_GSM_UMTS     = 0x3f,
    MN_MSG_TP_PID_SM_TYPE_0    = 0x40, /* Short Message Type 0 */
    MN_MSG_TP_PID_REPLACE_SM_1 = 0x41, /* Replace Short Message Type 1 */
    MN_MSG_TP_PID_REPLACE_SM_2 = 0x42, /* Replace Short Message Type 2 */
    MN_MSG_TP_PID_REPLACE_SM_3 = 0x43, /* Replace Short Message Type 3 */
    MN_MSG_TP_PID_REPLACE_SM_4 = 0x44, /* Replace Short Message Type 4 */
    MN_MSG_TP_PID_REPLACE_SM_5 = 0x45, /* Replace Short Message Type 5 */
    MN_MSG_TP_PID_REPLACE_SM_6 = 0x46, /* Replace Short Message Type 6 */
    MN_MSG_TP_PID_REPLACE_SM_7 = 0x47, /* Replace Short Message Type 7 */
    MN_MSG_TP_PID_RESERVED_51  = 0x51, /* TP-PID 的RESERVED位,TPPID=0x51 */
    /* Enhanced Message Service (Obsolete) */
    MN_MSG_TP_PID_EMS              = 0x5e,
    MN_MSG_TP_PID_RETURN_CALL      = 0x5f, /* Return Call Message */
    MN_MSG_TP_PID_ANSI136_R_DATA   = 0x7c, /* ANSI-136 R-DATA */
    MN_MSG_TP_PID_ME_DATA_DOWNLOAD = 0x7d, /* ME Data download */
    /* ME De personalization Short Message */
    MN_MSG_TP_PID_ME_DEPERSONALIZE  = 0x7e,
    MN_MSG_TP_PID_SIM_DATA_DOWNLOAD = 0x7f, /* (U)SIM Data download */
    MN_MSG_TP_PID_MAX               = 0xFF
};
typedef VOS_UINT8 MN_MSG_TpPidTypeUint8;

/* Sms Class */
enum MN_MSG_MsgClass {
    MN_MSG_MSG_CLASS_0, /* Display only not store */
    MN_MSG_MSG_CLASS_1, /* Default meaning: ME-specific */
    MN_MSG_MSG_CLASS_2, /* (U)SIM specific message */
    MN_MSG_MSG_CLASS_3, /* Default meaning: TE specific */
    MN_MSG_MSG_CLASS_NONE,
    MN_MSG_MSG_CLASS_MAX
};
typedef VOS_UINT8 MN_MSG_MsgClassUint8;

/* Sms Msg Code */
enum MN_MSG_MsgCoding {
    MN_MSG_MSG_CODING_7_BIT,
    MN_MSG_MSG_CODING_8_BIT,
    MN_MSG_MSG_CODING_UCS2,
    MN_MSG_MSG_CODING_MAX
};
typedef VOS_UINT8 MN_MSG_MsgCodingUint8;

/* Sms Msg Waiting */
enum MN_MSG_MsgWaiting {
    MN_MSG_MSG_WAITING_NONE,
    /* Message Waiting Indication Group: Discard Message */
    MN_MSG_MSG_WAITING_DISCARD,
    /* Message Waiting Indication Group: Store Message */
    MN_MSG_MSG_WAITING_STORE,
    MN_MSG_MSG_WAITING_NONE_1111,
    /* Message Marked for Automatic Deletion Group */
    MN_MSG_MSG_WAITING_AUTO_DELETE,
    MN_MSG_MSG_WAITING_MAX
};
typedef VOS_UINT8 MN_MSG_MsgWaitingUint8;

/* Sms Msg Waiting King */
enum MN_MSG_MsgWaitingKind {
    MN_MSG_MSG_WAITING_VOICEMAIL, /* Voicemail Message Waiting */
    MN_MSG_MSG_WAITING_FAX,       /* Fax Message Waiting */
    MN_MSG_MSG_WAITING_EMAIL,     /* Electronic Mail Message Waiting */
    MN_MSG_MSG_WAITING_OTHER,     /* Other Message Waiting */
    MN_MSG_MSG_WAITING_KIND_MAX
};
typedef VOS_UINT8 MN_MSG_MsgWaitingKindUint8;

/* Validity Period of TP User Data */
enum MN_MSG_ValidityPeriod {
    MN_MSG_VALID_PERIOD_NONE, /* TP VP field not present */
    /* TP-VP field present - enhanced format */
    MN_MSG_VALID_PERIOD_ENHANCED,
    /* TP VP field present - relative format */
    MN_MSG_VALID_PERIOD_RELATIVE,
    /* TP VP field present - absolute format */
    MN_MSG_VALID_PERIOD_ABSOLUTE,
    MN_MSG_VALID_PERIOD_MAX
};
typedef VOS_UINT8 MN_MSG_ValidPeriodUint8;

/* Sms Tp Staus */
enum MN_MSG_TpStatus {
    /* Short message received by the SME */
    MN_MSG_TP_STATUS_RECEIVED_OK = 0x00,
    /* Short message forwarded by the SC to the SME but the SC is unable to confirm delivery */
    MN_MSG_TP_STATUS_UNABLE_TO_CONFIRM_DELIVERY  = 0x01,
    MN_MSG_TP_STATUS_REPLACED                    = 0x02, /* Short message replaced by the SC */
    MN_MSG_TP_STATUS_TRYING_CONGESTION           = 0x20, /* Congestion */
    MN_MSG_TP_STATUS_TRYING_SME_BUSY             = 0x21, /* SME busy */
    MN_MSG_TP_STATUS_TRYING_NO_RESPONSE_FROM_SME = 0x22, /* No response from SME */
    MN_MSG_TP_STATUS_TRYING_SERVICE_REJECTED     = 0x23, /* Service rejected */
    MN_MSG_TP_STATUS_TRYING_QOS_NOT_AVAILABLE    = 0x24, /* Quality of service not available */
    MN_MSG_TP_STATUS_TRYING_SME_ERR              = 0x25, /* Error in SME */
    MN_MSG_TP_STATUS_PERM_REMOTE_PROCEDURE_ERR   = 0x40, /* Remote procedure error */
    MN_MSG_TP_STATUS_PERM_INCOMPATIBLE_DEST      = 0x41, /* Incompatible destination */
    MN_MSG_TP_STATUS_PERM_REJECTED_BY_SME        = 0x42, /* Connection rejected by SME */
    MN_MSG_TP_STATUS_PERM_NOT_OBTAINABLE         = 0x43, /* Not obtainable */
    MN_MSG_TP_STATUS_PERM_QOS_NOT_AVAILABLE      = 0x44, /* Quality of service not available */
    MN_MSG_TP_STATUS_PERM_NO_INTERWORKING        = 0x45, /* No interworking available */
    MN_MSG_TP_STATUS_PERM_VP_EXPIRED             = 0x46, /* SM Validity Period Expired */
    MN_MSG_TP_STATUS_PERM_DELETED_BY_ORIG_SME    = 0x47, /* SM Deleted by originating SME */
    MN_MSG_TP_STATUS_PERM_DELETED_BY_SC_ADMIN    = 0x48, /* SM Deleted by SC Administration */
    MN_MSG_TP_STATUS_PERM_SM_NO_EXISTING         = 0x49, /* SM does not exist */
    MN_MSG_TP_STATUS_TEMP_CONGESTION             = 0x60, /* Congestion */
    MN_MSG_TP_STATUS_TEMP_SME_BUSY               = 0x61, /* SME busy */
    MN_MSG_TP_STATUS_TEMP_NO_RESPONSE_FROM_SME   = 0x62, /* No response from SME */
    MN_MSG_TP_STATUS_TEMP_SERVICE_REJECTED       = 0x63, /* Service rejected */
    MN_MSG_TP_STATUS_TEMP_QOS_NOT_AVAILABLE      = 0x64, /* Quality of service not available */
    MN_MSG_TP_STATUS_TEMP_SME_ERR                = 0x65, /* Error in SME */
    MN_MSG_TP_STATUS_MAX                         = 0xFF
};
typedef VOS_UINT8 MN_MSG_TpStatusUint8;

/* Sms Command Type */
enum MN_MSG_Command {
    /* Enquiry relating to previously submitted short message */
    MN_MSG_COMMAND_TYPE_ENQUIRY = 0x00,
    /* Cancel Status Report Request relating to previously submitted short message */
    MN_MSG_COMMAND_TYPE_CANCEL_STATUS_RPT = 0x01,
    /* Delete previously submitted Short Message */
    MN_MSG_COMMAND_TYPE_DELETE_SM = 0x02,
    /* Enable Status Report Request relating to previously submitted short message */
    MN_MSG_COMMAND_TYPE_ENABLE_STATUS_RPT = 0x03,
    MN_MSG_COMMAND_MAX
};
typedef VOS_UINT8 MN_MSG_CommandTypeUint8;

/* Sms Status */
enum MN_MSG_StatusType {
    /* unread sms */
    MN_MSG_STATUS_MT_NOT_READ = 0,
    MN_MSG_STATUS_MT_READ,     /* read sms */
    MN_MSG_STATUS_MO_NOT_SENT, /* not sent sms */
    MN_MSG_STATUS_MO_SENT,     /* sent sms */
    /* all status sms,including read,unread,sent,not sent ,and not including sms status report */
    MN_MSG_STATUS_NONE,
    MN_MSG_STATUS_STARPT, /* sms status report */
    MN_MSG_STATUS_MAX
};
typedef VOS_UINT8 MN_MSG_StatusTypeUint8;

/* Sms Memory Store */
enum MN_MSG_MemStore {
    /* not saved by PS */
    MN_MSG_MEM_STORE_NONE = 0,
    MN_MSG_MEM_STORE_SIM, /* saved in USIM */
    MN_MSG_MEM_STORE_ME,  /* saved in ME */
    MN_MSG_MEM_STORE_MAX
};
typedef VOS_UINT8 MN_MSG_MemStoreUint8;

/* Sms Write Mode */
enum MN_MSG_WriteMode {
    MN_MSG_WRITE_MODE_INSERT,  /* write mode is insert */
    MN_MSG_WRITE_MODE_REPLACE, /* write mode is replace */
    MN_MSG_WRITE_MODE_MAX
};
typedef VOS_UINT8 MN_MSG_WriteModeUint8;

/* Type of delete */
enum MN_MSG_DeleteType {
    /* delete single sms or status report or sms service para */
    MN_MSG_DELETE_SINGLE = 0,
    /* delete read sms (the para is not  applicable to status report and sms service para) */
    MN_MSG_DELETE_READ,
    /* delete unread sms (the para is not applicable to status report and sms service para) */
    MN_MSG_DELETE_UNREAD,
    /* delete sms sent sms (the para is not applicable to status report and sms service para) */
    MN_MSG_DELETE_SENT,
    /* delete sms unsend sms (the para is not  applicable to status report and sms service para) */
    MN_MSG_DELETE_NOT_SENT,
    /* delete all sms or status report or sms service para */
    MN_MSG_DELETE_ALL,
    MN_MSG_DELETE_MAX
};
typedef VOS_UINT8 MN_MSG_DeleteTypeUint8;

/* Sms Type of number */
enum MN_MSG_Ton {
    MN_MSG_TON_UNKNOWN = 0,       /* 000: unknown */
    MN_MSG_TON_INTERNATIONAL,     /* 001: international number */
    MN_MSG_TON_NATIONAL,          /* 010: national number */
    MN_MSG_TON_NETWORK_SPEC,      /* 011: network specific number */
    MN_MSG_TON_SUBSCRIBER_NUMBER, /* 100: Subscriber number */
    /* 101: Alphanumeric GSM SMS: addr value is GSM 7-bit chars */
    MN_MSG_TON_ALPHANUMERIC,
    MN_MSG_TON_ABBREVIATED, /* 110: Abbreviated number */
    MN_MSG_TON_Reserved     /* 111: Reserved for extension */
};
typedef VOS_UINT8 MN_MSG_TonUint8;

/* Sms Number Plan */
enum MN_MSG_Npi {
    MN_MSG_NPI_UNKNOWN = 0, /* 0000: unknown */
    /* 0001: ISDN/telephony numbering plan  */
    MN_MSG_NPI_ISDN  = 1,
    MN_MSG_NPI_DATA  = 3, /* 0011: data numbering plan  */
    MN_MSG_NPI_TELEX = 4, /* 0100: telex numbering plan  */
    /* 0101: Service Centre Specific plan 1) */
    MN_MSG_NPI_SC_SPECIFIC1 = 5,
    /* 0101: Service Centre Specific plan 1) */
    MN_MSG_NPI_SC_SPECIFIC2 = 6,
    MN_MSG_NPI_NATIONAL     = 8, /* 1000: national numbering plane */
    MN_MSG_NPI_PRIVATE      = 9, /* 1001: private numbering plan */
    /* 1010: ERMES numbering plan (ETSI DE/PS 3 01 3) */
    MN_MSG_NPI_ERMES = 10

};
typedef VOS_UINT8 MN_MSG_NpiUint8;


enum MN_MSG_CnmiMtType {
    /* No SMS-DELIVER indications are routed to the TE */
    MN_MSG_CNMI_MT_NO_SEND_TYPE = 0,
    MN_MSG_CNMI_MT_CMTI_TYPE    = 1, /* +CMTI */
    MN_MSG_CNMI_MT_CMT_TYPE     = 2, /* +CMT */
    /* CLASS3 MSG IS indicated by +CMT type, others is indicated by +CMTI type */
    MN_MSG_CNMI_MT_CLASS3_TYPE = 3,
    MN_MSG_CNMI_MT_TYPE_BUTT
};
typedef VOS_UINT8 MN_MSG_CnmiMtTypeUint8;

/* Action of rcv sms */
enum MN_MSG_RcvmsgAct {
    /* Discard message */
    MN_MSG_RCVMSG_ACT_DISCARD = 0,
    MN_MSG_RCVMSG_ACT_STORE, /* Store message and notify clients */
    /* Don't store msg but send it to clients */
    MN_MSG_RCVMSG_ACT_TRANSFER_ONLY,
    /* Send the message to clients and ack it */
    MN_MSG_RCVMSG_ACT_TRANSFER_AND_ACK,
    MN_MSG_RCVMSG_ACT_MAX
};
typedef VOS_UINT8 MN_MSG_RcvmsgActUint8;

/* Domain of send sms */
enum MN_MSG_SendDomain {
    /* only PS,consistent with Ps Send Domain in NAS-SMS */
    MN_MSG_SEND_DOMAIN_PS = 0,
    /* only CS,,consistent with Ps Send Domain in NAS-SMS */
    MN_MSG_SEND_DOMAIN_CS,
    MN_MSG_SEND_DOMAIN_PS_PREFERRED, /* PS PREFERRED */
    MN_MSG_SEND_DOMAIN_CS_PREFERRED, /* CS PREFERRED */
    MN_MSG_SEND_DOMAIN_NO,           /* No Domain */
    MN_MSG_SEND_DOMAIN_MAX
};
typedef VOS_UINT8 MN_MSG_SendDomainUint8;

enum MN_MSG_RcvDomain {
    MN_MSG_RCV_DOMAIN_PS = 0, /* PS域接收短信 */
    MN_MSG_RCV_DOMAIN_CS,     /* CS域接收短信 */
    MN_MSG_RCV_DOMAIN_BUTT    /* 无效值 */
};
typedef VOS_UINT8 MN_MSG_RcvDomainUint8;

/* Sms Memory Status */
enum MN_MSG_MemFlag {
    MN_MSG_MEM_FULL_SET = 0, /* memory full */
    MN_MSG_MEM_FULL_UNSET,   /* memory available */
    MN_MSG_MEM_FULL_MAX
};
typedef VOS_UINT8 MN_MSG_MemFlagUint8;

/* Status of Sms Status Report */
enum MN_MSG_RptStatus {
    MN_MSG_RPT_OK,               /* Success: submit_report_ack info */
    MN_MSG_RPT_LOWERLAYER_ERR,   /* Lower layer errors */
    MN_MSG_RPT_RP_ERR,           /* Rp Error */
    MN_MSG_RPT_CP_ERR,           /* Cp Error */
    MN_MSG_RPT_SMR_TR1M_TIMEOUT, /* TR1M Expired */
    MN_MSG_RPT_SMR_TR2M_TIMEOUT, /* TR2M Expired */
    MN_MSG_RPT_SMR_NO_RESOURCES, /* No Resources */
    MN_MSG_RPT_USER_ABORT,
    MN_MSG_RPT_MAX
};
typedef VOS_UINT8 MN_MSG_RptStatusUint8;

enum MN_MSG_EoType {
    MN_MSG_UDH_EO_PRE_DEF_SOUND,
    MN_MSG_UDH_EO_IMELODY,
    MN_MSG_UDH_EO_BLACK_WHITE_BITMAP,
    MN_MSG_UDH_EO_2BITGREYSCALE_BITMAP,
    MN_MSG_UDH_EO_6BITCOLOR_BITMAP_ANIM,
    MN_MSG_UDH_EO_VCARD,
    MN_MSG_UDH_EO_VCALENDAR,
    MN_MSG_UDH_EO_STD_WVG_OBJ,
    MN_MSG_UDH_EO_POLYPHONIC_MELODY,
    MN_MSG_UDH_EO_DATA_FORMAT_DELIVERY_REQ = 0xff
};
typedef VOS_UINT8 MN_MSG_EoTypeUint8;

enum MN_MSG_CompressionAlgorithm {
    MN_MSG_COMPRESSION_LZSS
};
typedef VOS_UINT8 MN_MSG_CompressionAlgorithmUint8;

enum MN_MSG_Class0Tailor {
    MN_MSG_CLASS0_VODAFONE, /* Vodafone Class0 短信定制 */
    /* Italy TIM Class0 短信定制（即H3G Class0 短信定制） */
    MN_MSG_CLASS0_TIM,
    /* 默认Class 0 短信处理(Balong符合3GPP协议的处理方式即可) */
    MN_MSG_CLASS0_DEF,
    MN_MSG_CLASS0_VIVO /* VIVO Class0 短信定制 */
};

typedef VOS_UINT8 MN_MSG_Class0TailorUint8;

#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))


enum MN_MSG_Cblang {
    MN_MSG_CBLANG_GERMAN      = 0x00,
    MN_MSG_CBLANG_ENGLISH     = 0x01,
    MN_MSG_CBLANG_ITALIAN     = 0x02,
    MN_MSG_CBLANG_FRENCH      = 0x03,
    MN_MSG_CBLANG_SPANISH     = 0x04,
    MN_MSG_CBLANG_DUTCH       = 0x05,
    MN_MSG_CBLANG_SWEDISH     = 0x06,
    MN_MSG_CBLANG_DANISH      = 0x07,
    MN_MSG_CBLANG_PORTUGUESE  = 0x08,
    MN_MSG_CBLANG_FINNISH     = 0x09,
    MN_MSG_CBLANG_NORWEGIAN   = 0x0a,
    MN_MSG_CBLANG_GREEK       = 0x0b,
    MN_MSG_CBLANG_TURKISH     = 0x0c,
    MN_MSG_CBLANG_HUNGARIAN   = 0x0d,
    MN_MSG_CBLANG_POLISH      = 0x0e,
    MN_MSG_CBLANG_UNSPECIFIED = 0x0f,
    MN_MSG_CBLANG_CZECH       = 0x20,
    MN_MSG_CBLANG_HEBREW      = 0x21,
    MN_MSG_CBLANG_ARABIC      = 0x22,
    MN_MSG_CBLANG_RUSSIAN     = 0x23,
    MN_MSG_CBLANG_ICELANDIC   = 0x24,
    MN_MSG_CBLANG_MAX         = 0xff
};
typedef VOS_UINT8 MN_MSG_CblangUint8;

/* CBS Function On or Off */
enum MN_MSG_Cbsstatus {
    MN_MSG_CBSTATUS_DISABLE   = 0x00,
    MN_MSG_CBSTATUS_W_ENABLE  = 0x01,
    MN_MSG_CBSTATUS_G_ENABLE  = 0x02,
    MN_MSG_CBSTATUS_WG_ENABLE = 0x03,
    MN_MSG_CBSTATUS_MAX
};
typedef VOS_UINT8 MN_MSG_CbstatusUint8;

/* Cbs Mode */
enum MN_MSG_Cbmode {
    MN_MSG_CBMODE_ACCEPT,
    MN_MSG_CBMODE_REJECT,
    MN_MSG_CBMODE_MAX
};
typedef VOS_UINT8 MN_MSG_CbmodeUint8;

#endif

/* SMS version PHASE2版本 SMS PHASE2+版本 */
enum MN_MSG_CsmsMsgVersion {
    MN_MSG_CSMS_MSG_VERSION_PHASE2,
    MN_MSG_CSMS_MSG_VERSION_PHASE2_PLUS
};
typedef VOS_UINT8 MN_MSG_CsmsMsgVersionUint8;


enum TAF_CBA_EtwsPrimNtfAuthRslt {
    TAF_CBA_ETWS_PRIM_NTF_AUTH_SUCCESS, /* ETWS主通知信息通过鉴权 */
    TAF_CBA_ETWS_PRIM_NTF_AUTH_FAILED   /* ETWS主通知信息未通过鉴权 */
};
typedef VOS_UINT8 TAF_CBA_EtwsPrimNtfAuthRsltUint8;


enum TAF_CBA_CbmiRangeRcvModeEnmu {
    TAF_CBA_CBMI_RANGE_RCV_MODE_REJECT, /* 不接收该范围的消息 */
    TAF_CBA_CBMI_RANGE_RCV_MODE_ACCEPT, /* 接收该范围的消息 */
    TAF_CBA_CBMI_RANGE_RCV_MODE_BUTT
};
typedef VOS_UINT8 TAF_CBA_CbmiRangeRcvModeEnmuUint8;


enum TAF_MSG_SignallingType {
    /* SMS基于GSM，UTRAN或LTE信令发送 */
    TAF_MSG_SIGNALLING_TYPE_NAS_SIGNALLING,
    /* SMS基于SIP信令发送 */
    TAF_MSG_SIGNALLING_TYPE_CS_OVER_IP,

    TAF_MSG_SIGNALLING_TYPE_BUTT
};
typedef VOS_UINT32 TAF_MSG_SignallingTypeUint32;


enum TAF_CBA_MsgidSrcFlg {
    TAF_CBA_MSGID_SRC_FLG_NULL    = 0x00, /* 初始标记，不属于任何文件和NV */
    TAF_CBA_MSGID_SRC_FLG_NV      = 0x01, /* NV中的MSG ID */
    TAF_CBA_MSGID_SRC_FLG_EFCBMI  = 0x02, /* EFCBMI中的MSG ID */
    TAF_CBA_MSGID_SRC_FLG_EFCBMIR = 0x04, /* EFCBMIR中的MSG ID */
    TAF_CBA_MSGID_SRC_FLG_BUTT
};
typedef VOS_UINT8 TAF_CBA_MsgidSrcFlgUint8;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: 短信业务参数结构
 */
typedef struct {
    MN_MSG_RcvmsgActUint8 rcvSmAct; /* action of received msg */
    /* Sm memory store:NVIM or USIM of received msg */
    MN_MSG_MemStoreUint8 smMemStore;
    /* action of received Status Report msg */
    MN_MSG_RcvmsgActUint8 rcvStaRptAct;
    /* Sm memory store:NVIM or USIM of Status Report msg */
    MN_MSG_MemStoreUint8 staRptMemStore;
    /* Cbm memory store:NVIM of received msg */
    MN_MSG_MemStoreUint8 cbmMemStore;
    /* 外部存储器状态,仅当协议栈不保存时有效 */
    MN_MSG_MemFlagUint8        appMemStatus;
    MN_MSG_CsmsMsgVersionUint8 smsServVersion; /* 巴西vivo短消息方案-添加变量 */

    MN_MSG_CnmiMtTypeUint8 mtType;
    MN_MSG_MoDomainCustomizeTypeUint8 protocolSendDomain;
    VOS_UINT8                         reserve1[7];
} MN_MSG_ConfigParm;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: 短信TPDU结构, 包括TPDU长度(单位: 字节)和TPDU码流
 */
/*lint -e958 -e959 原因:64bit*/
typedef struct {
    VOS_UINT32 len;
    VOS_UINT8 *tpdu;
} MN_MSG_Tpdu;
/*lint +e958 +e959 原因:64bit*/

/* Content of Sms BCD Address */
typedef struct {
    VOS_UINT8              bcdLen;
    MN_MSG_ADDR_NUM_TYPE_T addrType; /* bcd number type */
    /* bcd                               number array */
    VOS_UINT8 bcdNum[MN_MSG_MAX_BCD_NUM_LEN];
} MN_MSG_BcdAddr;

typedef struct {
    MN_MSG_TpduTypeUint8 tpduType; /* TP Message Type */
    VOS_UINT8            reserve1[3];
    VOS_UINT32           len;                  /* length of sms TPDU */
    VOS_UINT8            data[MN_MSG_MAX_LEN]; /* TPDU */
    VOS_UINT8            reserve2[1];
} MN_MSG_RawTsData;

/* Content of sms msg including TPDU and SC */
typedef struct {
    MN_MSG_BcdAddr   scAddr;    /* address of service center */
    MN_MSG_RawTsData tsRawData; /* TPDU detail */
} MN_MSG_MsgInfo;

/* Content of Alpha Identifier */
#define MN_MSG_MAX_ALPHA_ID_LEN 227 /* 255-28 */
typedef struct {
    VOS_UINT32 len; /* length of sms ALPHA IDENTIFIER */
    /* content                           of sms ALPHA IDENTIFIER */
    VOS_UINT8 data[MN_MSG_MAX_ALPHA_ID_LEN];
    VOS_UINT8 reserve1[1];
} MN_MSG_AlphaIdInfo;

/* Sms Service para */
enum MN_MSG_SrvParmExist {
    MN_MSG_SRV_PARM_PRESENT = 0,
    MN_MSG_SRV_PARM_ABSENT  = 1
};
typedef VOS_UINT8 MN_MSG_SrvParmExistUint8;

#define MN_MSG_SRV_PARM_MASK_DEST_ADDR 0x01
#define MN_MSG_SRV_PARM_MASK_SC_ADDR 0x02
#define MN_MSG_SRV_PARM_MASK_PID 0x04
#define MN_MSG_SRV_PARM_MASK_DCS 0x08
#define MN_MSG_SRV_PARM_MASK_VALIDITY 0x10
#define MN_MSG_SRV_PARM_TOTALABSENT 0x1F
typedef struct {
    VOS_UINT8          parmInd; /* Parameter Indicators */
    VOS_UINT8          reserve1[3];
    MN_MSG_BcdAddr     destAddr;    /* TP Destination Address */
    MN_MSG_BcdAddr     scAddr;      /* TS Service Centre Address */
    VOS_UINT8          pid;         /* TP Protocol Identifier */
    VOS_UINT8          dcs;         /* TP Data Coding Scheme */
    VOS_UINT8          validPeriod; /* TP Validity Period */
    VOS_UINT8          reserve2[1];
    MN_MSG_AlphaIdInfo alphaIdInfo; /* Alpha Identifier */
} MN_MSG_SrvParam;

/* Format of Ascii address */
typedef struct {
    MN_MSG_TonUint8 numType; /* type of number */
    MN_MSG_NpiUint8 numPlan; /* Numbering plan identification */
    VOS_UINT8       reserve1[2];
    VOS_UINT32      len;                                    /* Length of AsicNum */
    VOS_UINT8       asciiNum[MN_MAX_ASCII_ADDRESS_NUM + 1]; /* array  of ASCII Num */
    VOS_UINT8       reserve2[3];
} MN_MSG_AsciiAddr;

/* Sms Dcs */
typedef struct {
    VOS_BOOL compressed; /* Compressed or not */
    /* Waiting Indication Active or inactive */
    VOS_BOOL                   waitingIndiActive;
    MN_MSG_MsgClassUint8       msgClass;       /* sms msg class */
    MN_MSG_MsgCodingUint8      msgCoding;      /* sms msg coding */
    MN_MSG_MsgWaitingUint8     msgWaiting;     /* sms msg waiting */
    MN_MSG_MsgWaitingKindUint8 msgWaitingKind; /* sms msg waiting king */
    VOS_BOOL                   rawDcsValid;    /* Compressed or not */
    VOS_UINT8                  rawDcsData;     /* Raw DCS Byte */
    VOS_UINT8                  reserve1[7];
} MN_MSG_DcsCode;

/* Sms TimeStamp */
typedef struct {
    VOS_UINT8 year;   /* 0x00-0x99 */
    VOS_UINT8 month;  /* 0x01-0x12 */
    VOS_UINT8 day;    /* 0x01-0x31 */
    VOS_UINT8 hour;   /* 0x00-0x23 */
    VOS_UINT8 minute; /* 0x00-0x59 */
    VOS_UINT8 second; /* 0x00-0x59 */
    /* +/-, [-48,+48] number of 15 minutes */
    VOS_INT8  timezone;
    VOS_UINT8 reserve1[1];
} MN_MSG_Timestamp;

/* Sms Validity Period */
typedef struct {
    MN_MSG_ValidPeriodUint8 validPeriod; /* TP Validity Period Format */
    VOS_UINT8               reserve1[3];
    union {
        MN_MSG_Timestamp absoluteTime; /* TP-VP (Absolute format) */
        /* TP-VP (Relative format or Enhanced format) */
        VOS_UINT8 otherTime;
    } u;
} MN_MSG_ValidPeriod;

/* Type of TP User Data Header */
enum MN_MSG_UdhType {
    /* Concatenated short messages, 8-bit reference number */
    MN_MSG_UDH_CONCAT_8 = 0x00,
    MN_MSG_UDH_SPECIAL_SM, /* Special SMS Message Indication */
    /* Application port addressing scheme, 8 bit address */
    MN_MSG_UDH_PORT_8 = 0x04,
    /* Application port addressing scheme, 16 bit address */
    MN_MSG_UDH_PORT_16,
    MN_MSG_UDH_SMSC_CTRL, /* SMSC Control Parameters */
    MN_MSG_UDH_SOURCE,    /* UDH Source Indicator */
    /* Concatenated short message, 16-bit reference number */
    MN_MSG_UDH_CONCAT_16,
    /* Wireless Control Message Protocol */
    MN_MSG_UDH_WCMP,
    MN_MSG_UDH_TEXT_FORMATING, /* Text Formatting */
    MN_MSG_UDH_PRE_DEF_SOUND,  /* Predefined Sound */
    /* User Defined Sound (iMelody max 128 bytes) */
    MN_MSG_UDH_USER_DEF_SOUND,
    MN_MSG_UDH_PRE_DEF_ANIM, /* Predefined Animation */
    /* Large Animation (16*16 times 4 = 32*4 =128 bytes) */
    MN_MSG_UDH_LARGE_ANIM,
    /* Small Animation (8*8 times 4 = 8*4 =32 bytes) */
    MN_MSG_UDH_SMALL_ANIM,
    MN_MSG_UDH_LARGE_PIC,          /* Large Pic (32*32 = 128 bytes) */
    MN_MSG_UDH_SMALL_PIC,          /* Small Pic (16*16 = 32 bytes) */
    MN_MSG_UDH_VAR_PIC,            /* Variable Pic */
    MN_MSG_UDH_USER_PROMPT = 0x13, /* User prompt indicator */
    MN_MSG_UDH_EO,                 /* Extended Object */
    MN_MSG_UDH_REO,                /* Reused Extended Object */
    MN_MSG_UDH_CC,                 /* Compression Control */
    MN_MSG_UDH_OBJ_DISTR_IND,      /* Object Distribution Indicator */
    MN_MSG_UDH_STD_WVG_OBJ,        /* Standard WVG object */
    MN_MSG_UDH_CHAR_SIZE_WVG_OBJ,  /* Character Size WVG object */
    /* Extended Object Data Request Command */
    MN_MSG_UDH_EXT_OBJECT_DATA_REQ_CMD,
    MN_MSG_UDH_RFC822 = 0x20, /* RFC 822 E-Mail Header */
    MN_MSG_UDH_HYPERLINK_FORMAT,
    MN_MSG_UDH_REPLY_ADDR,
    MN_MSG_UDH_ENHANCED_VOICE_MAIL_INF,
    MN_MSG_UDH_MAX = 0xFFFF
};
typedef VOS_UINT8 MN_MSG_UdhTypeUint8;

/* Alignment of TP User Data */
enum MN_MSG_UdhAlignment {
    MN_MSG_UDH_ALIGN_LEFT,   /* Alignment:Left */
    MN_MSG_UDH_ALIGN_CENTER, /* Alignment:Center */
    MN_MSG_UDH_ALIGN_RIGHT,  /* Alignment:Right */
    MN_MSG_UDH_ALIGNT_DEF,   /* Alignment:Default */
    MN_MSG_UDH_ALIGN_MAX
};
typedef VOS_UINT8 MN_MSG_UdhAlignUint8;

/* Font of TP User Data */
enum MN_MSG_UdhFontSize {
    MN_MSG_UDH_FONT_NORMAL,   /* Font:Normal */
    MN_MSG_UDH_FONT_LARGE,    /* Font:Large */
    MN_MSG_UDH_FONT_SMALL,    /* Font:Small */
    MN_MSG_UDH_FONT_RESERVED, /* Font:Reserved */
    MN_MSG_UDH_FONT_MAX
};
typedef VOS_UINT8 MN_MSG_UdhFontUint8;

/* Content of Concatenated short messages, 8-bit reference number */
typedef struct {
    /* Concatenated short message reference number */
    VOS_UINT8 mr;
    /* Maximum number of short messages in the concatenated short message */
    VOS_UINT8 totalNum;
    /* Sequence number of the current short message */
    VOS_UINT8 seqNum;
    VOS_UINT8 reserve1[1];
} MN_MSG_UdhConcat8;

/* Content of Special SMS Message Indication */
enum MN_MSG_ExtmsgInd {
    MN_MSG_EXTMSG_IND_NONE,
    MN_MSG_EXTMSG_IND_VOICEMSG_WAITING,
    MN_MSG_EXTMSG_IND_MAX
};
typedef VOS_UINT8 MN_MSG_ExtmsgIndUint8;

enum MN_MSG_ProfileId {
    MN_MSG_PROFILE_ID_1,
    MN_MSG_PROFILE_ID_2,
    MN_MSG_PROFILE_ID_3,
    MN_MSG_PROFILE_ID_4,
    MN_MSG_PROFILE_ID_MAX
};
typedef VOS_UINT8 MN_MSG_ProfileIdUint8;

typedef struct {
    /* Indicates whether or not the message shall be stored */
    MN_MSG_MsgWaitingUint8     msgWaiting;
    MN_MSG_MsgWaitingKindUint8 msgWaitingKind; /* the message indication type */
    VOS_UINT8                  msgCount;       /* Message Count */
    /* Bits 6 and 5 indicate the profile ID of the Multiple Subscriber Profile (see 3GPP TS 23.097 [41]). */
    MN_MSG_ProfileIdUint8 profileId;
    /* Bits 432 indicate the extended message indication type */
    MN_MSG_ExtmsgIndUint8 extMsgInd;
    VOS_UINT8             reserve1[3];
} MN_MSG_UdhSpecialSms;

/* Content of Application port addressing scheme, 8 bit address */
typedef struct {
    VOS_UINT8 destPort; /* Destination port */
    VOS_UINT8 origPort; /* Originator port */
    VOS_UINT8 reserve1[2];
} MN_MSG_UdhAppport8;

/* Content of Application port addressing scheme, 16 bit address */
typedef struct {
    VOS_UINT16 destPort; /* Destination port */
    VOS_UINT16 origPort; /* Originator port */
} MN_MSG_UdhAppport16;

/* Content of Concatenated short message, 16-bit reference number */
typedef struct {
    /* Concatenated short messages, 16-bit reference number */
    VOS_UINT16 mr;
    /* Maximum number of short messages in the enhanced concatenated short message */
    VOS_UINT8 totalNum;
    /* Sequence number of the current short message. */
    VOS_UINT8 seqNum;
} MN_MSG_UdhConcat16;

/* Content of Text Formatting */
enum MN_MSG_Textcolor {
    MN_MSG_TEXTCOLOR_BLACK         = 0x00,
    MN_MSG_TEXTCOLOR_DARKGREY      = 0x01,
    MN_MSG_TEXTCOLOR_DARKRED       = 0x02,
    MN_MSG_TEXTCOLOR_DARKYELLOW    = 0x03,
    MN_MSG_TEXTCOLOR_DARKGREEN     = 0x04,
    MN_MSG_TEXTCOLOR_DARKCYAN      = 0x05,
    MN_MSG_TEXTCOLOR_DARKBLUE      = 0x06,
    MN_MSG_TEXTCOLOR_DARKMAGENTA   = 0x07,
    MN_MSG_TEXTCOLOR_GREY          = 0x08,
    MN_MSG_TEXTCOLOR_WHITE         = 0x09,
    MN_MSG_TEXTCOLOR_BRIGHTRED     = 0x10,
    MN_MSG_TEXTCOLOR_BRIGHTYELLOW  = 0x11,
    MN_MSG_TEXTCOLOR_BRIGHTGREEN   = 0x12,
    MN_MSG_TEXTCOLOR_BRIGHTCYAN    = 0x13,
    MN_MSG_TEXTCOLOR_BRIGHTBLUE    = 0x14,
    MN_MSG_TEXTCOLOR_BRIGHTMAGENTA = 0x15
};
typedef VOS_UINT8 MN_MSG_TextcolorUint8;

typedef struct {
    VOS_BOOL              color;
    MN_MSG_TextcolorUint8 foregroundColor;
    MN_MSG_TextcolorUint8 backgroundColor;
    VOS_UINT8             reserve1[2];
} MN_MSG_TextColor;

typedef struct {
    /* Start position of the text formatting. */
    VOS_UINT8 startPos;
    /* Text formatting length. Gives the number of formatted characters */
    VOS_UINT8            len;
    MN_MSG_UdhAlignUint8 align;           /* Alignment bit 0 and  bit 1 */
    MN_MSG_UdhFontUint8  fontSize;        /* Font Size bit 3 and  bit 2 */
    VOS_BOOL             styleBold;       /* Style bold bit 4 */
    VOS_BOOL             styleItalic;     /* Style Italic bit 5  */
    VOS_BOOL             styleUnderlined; /* Style Underlined bit 6 */
    VOS_BOOL             styleStrkthrgh;  /* Style Strikethrough bit 7 */
    MN_MSG_TextColor     color;           /* Text Colour Octet 4 */
} MN_MSG_UdhTextFormat;

/* Content of Predefined Sound */
typedef struct {
    /* position indicating in the SM data the instant after which the sound shall be played. */
    VOS_UINT8 pos;
    VOS_UINT8 ucdNumofSound; /* sound number */
    VOS_UINT8 reserve1[2];
} MN_MSG_UdhPreDefSound;

/* Content of User Defined Sound (iMelody max 128 bytes) */
#define MN_MSG_UDH_MAX_SOUND_SIZE 128
typedef struct {
    /* position indicating in the SM data the instant the after which the sound shall be played */
    VOS_UINT8 pos;
    VOS_UINT8 sizeofSound; /* size of User Defined Sound */
    VOS_UINT8 reserve1[2];
    /* This octet(s) shall contain a User Defined Sound. */
    VOS_UINT8 defSound[MN_MSG_UDH_MAX_SOUND_SIZE];
} MN_MSG_UdhUserDefSound;

/* Content of Predefined Animation */
typedef struct {
    /* position indicating in the SM data the instant the animation shall be displayed. */
    VOS_UINT8 pos;
    VOS_UINT8 numofAnim; /* animation number */
    VOS_UINT8 reserve1[2];
} MN_MSG_UdhPreDefAnim;

/* Content of Large Animation (16*16 times 4 = 32*4 =128 bytes) */
#define MN_MSG_UDH_ANIM_PIC_NUM 4
#define MN_MSG_UDH_ANIM_LARGE_PIC_SIZE 32
typedef struct {
    /* position indicating the instant the animation shall be displayed in the SM data */
    VOS_UINT8 pos;
    VOS_UINT8 reserve1[3];
    /* Protocol Data Unit */
    VOS_UINT8 data[MN_MSG_UDH_ANIM_PIC_NUM][MN_MSG_UDH_ANIM_LARGE_PIC_SIZE];
} MN_MSG_UdhLargeAnim;

/* Content of Small Animation (8*8 times 4 = 8*4 =32 bytes) */
#define MN_MSG_UDH_ANIM_SMALL_PIC_SIZE 8
typedef struct {
    /* position indicating the instant the animation shall be displayed in the SM data */
    VOS_UINT8 pos;
    VOS_UINT8 reserve1[3];
    /* Protocol Data Unit */
    VOS_UINT8 data[MN_MSG_UDH_ANIM_PIC_NUM][MN_MSG_UDH_ANIM_SMALL_PIC_SIZE];
} MN_MSG_UdhSmallAnim;

/* Content of Large Pic (32*32 = 128 bytes) */
#define MN_MSG_UDH_LARGE_PIC_SIZE 128
typedef struct {
    /* position indicating in the SM data the instant the picture shall be displayed. */
    VOS_UINT8 pos;
    VOS_UINT8 data[MN_MSG_UDH_LARGE_PIC_SIZE]; /* Protocol Data Unit */
} MN_MSG_UdhLargePic;

/* Content of Small Pic (16*16 = 32 bytes) */
#define MN_MSG_UDH_SMALL_PIC_SIZE 32
typedef struct {
    /* position indicating in the SM data the instant the picture shall be displayed in the SM data */
    VOS_UINT8 pos;
    VOS_UINT8 reserve1[3];
    VOS_UINT8 data[MN_MSG_UDH_SMALL_PIC_SIZE]; /* Protocol Data Unit */
} MN_MSG_UdhSmallPic;

/* Content of Variable Pic */
#define MN_MSG_UDH_VAR_PIC_SIZE 134
typedef struct {
    /* position indicating in the SM data the instant the picture shall be displayed in the SM data */
    VOS_UINT8 pos;
    /* Horizontal dimension of the picture */
    VOS_UINT8 horDim;
    /* Vertical dimension of the picture */
    VOS_UINT8 vertDim;
    VOS_UINT8 reserve1[1];
    VOS_UINT8 data[MN_MSG_UDH_VAR_PIC_SIZE]; /* Protocol Data Unit */
    VOS_UINT8 reserve2[2];
} MN_MSG_UdhVarPic;

/* Content of User prompt indicator */
typedef struct {
    VOS_UINT8 numofObjects; /* Number of corresponding objects */
    VOS_UINT8 reserve1[3];
} MN_MSG_UdhUserPrompt;

/* Content of RFC 822 E-Mail Header */
typedef struct {
    /* RFC 822 E-Mail Header length indicator */
    VOS_UINT8 len;
    VOS_UINT8 reserve1[3];
} MN_MSG_UdhRfc822;

/* Content of SMSC Control Parameters Header */
typedef struct {
    /* bit 0:Status Report for short message transaction completed */
    VOS_BOOL completionReport;
    /* bit 1:Status Report for permanent error when SC is not making any more transfer attempts */
    VOS_BOOL permErrReport;
    /* bit 2:Status Report for temporary error when SC is not making any more transfer attempts */
    VOS_BOOL tempErrNoneAttemptReport;
    /* bit 3:Status Report for temporary error when SC is still trying to transfer SM */
    VOS_BOOL tempErrWithAttemptReport;
    /* bit 6:A Status Report generated by this Short Message, due to a permanent error or last temporary error, cancels
     * the SRR of the rest of the Short Messages in a concatenated message. This feature can only be used where a SC is
     * aware of the segmentation of a concatenated SM and is therefore an implementation matter. */
    VOS_BOOL activation;
    /* bit 7:include original UDH into the Status Report */
    VOS_BOOL origUdhInclude;
} MN_MSG_SelstatusReport;
typedef struct {
    MN_MSG_SelstatusReport selectiveStatusReport; /* Selective Status Report */
} MN_MSG_UdhSmscCtrl;

/* Content of UDH Source Indicator Header */
enum MN_MSG_Originalentity {
    MN_MSG_ORIGINALENTITY_SENDER   = 0x01,
    MN_MSG_ORIGINALENTITY_RECEIVER = 0x02,
    MN_MSG_ORIGINALENTITY_SMSC     = 0x03
};
typedef VOS_UINT8 MN_MSG_OriginalentityUint8;

typedef struct {
    MN_MSG_OriginalentityUint8 orgEntity;
    VOS_UINT8                  reserve1[3];
} MN_MSG_UdhSourceInd;

/* MN_MSG_MAX_UDH_EO_DATA_LEN * MN_MSG_MAX_UDH_EO_NUM - 3 */
#define MN_MSG_MAX_UDH_LONG_EO_DATA_LEN 914
typedef struct {
    VOS_BOOL           firstSegment;
    VOS_UINT8          refNum;
    VOS_UINT8          reserve1[1];
    VOS_UINT16         totalLen;
    VOS_BOOL           objNotFowarded;
    VOS_BOOL           objHandledAsUserPrompt;
    MN_MSG_EoTypeUint8 type;
    VOS_UINT8          reserve2[3];
    VOS_UINT16         pos;
    VOS_UINT16         dataLen;
    VOS_UINT8          data[MN_MSG_MAX_UDH_LONG_EO_DATA_LEN];
    VOS_UINT8          reserve3[2];
} MN_MSG_UdhEo;

typedef struct {
    VOS_UINT8  refNum;
    VOS_UINT8  reserve1[1];
    VOS_UINT16 pos;
} MN_MSG_UdhReo;

/* 140 - 1(UDHL) - 1(IEI) - 1(IEIDL) - 3(data以外数据长度) */
#define MN_MSG_UDH_MAX_COMPRESSION_DATA_LEN 134
typedef struct {
    MN_MSG_CompressionAlgorithmUint8 compressionAlgorithm;
    VOS_UINT8                        reserve1[1];
    VOS_UINT16                       totalLen;
    VOS_UINT16                       dataLen;
    VOS_UINT8                        data[MN_MSG_UDH_MAX_COMPRESSION_DATA_LEN];
} MN_MSG_UdhCc;

typedef struct {
    VOS_UINT8 ieNum;
    VOS_UINT8 reserve1[3];
    VOS_BOOL  objNotFowarded;
} MN_MSG_UdhObjDistr;

/* 140 - 1(UDHL) - 1(IEI) - 1(IEIDL) - 1(data以外数据长度) */
#define MN_MSG_UDH_MAX_WVG_DATA_LEN 136
typedef struct {
    VOS_UINT8 pos;
    VOS_UINT8 len;
    VOS_UINT8 reserve1[2];
    VOS_UINT8 data[MN_MSG_UDH_MAX_WVG_DATA_LEN];
} MN_MSG_UdhWvgObj;

typedef struct {
    VOS_UINT16 pos;
    VOS_UINT8  hyperLinkTitleLen;
    VOS_UINT8  urlLen;
} MN_MSG_HyperlinkFormat;

typedef struct {
    MN_MSG_AsciiAddr replyAddr;
} MN_MSG_UdhReplyAddr;

#define MN_MSG_UDH_OTHER_SIZE 160
typedef struct {
    MN_MSG_UdhTypeUint8 udhType;

    VOS_UINT8 len; /* Length of other user header type */
    VOS_UINT8 reserve1[2];
    /* Content of other user header type */
    VOS_UINT8 data[MN_MSG_UDH_OTHER_SIZE];
} MN_MSG_UdhOther;

/* Detail of each TP User Data Header */
typedef struct {
    MN_MSG_UdhTypeUint8 headerID;
    VOS_UINT8           reserve1[3];
    union {
        MN_MSG_UdhConcat8      concat8;         /* 00 */
        MN_MSG_UdhSpecialSms   specialSms;      /* 01 */
        MN_MSG_UdhAppport8     appPort8;        /* 04 */
        MN_MSG_UdhAppport16    appPort16;       /* 05 */
        MN_MSG_UdhSmscCtrl     smscCtrl;        /* 06 */
        MN_MSG_UdhSourceInd    srcInd;          /* 07 */
        MN_MSG_UdhConcat16     concat16;        /* 08 */
        MN_MSG_UdhTextFormat   textFormat;      /* 0a */
        MN_MSG_UdhPreDefSound  preDefSound;     /* 0b */
        MN_MSG_UdhUserDefSound userDefSound;    /* 0c */
        MN_MSG_UdhPreDefAnim   preDefAnim;      /* 0d */
        MN_MSG_UdhLargeAnim    largeAnim;       /* 0e */
        MN_MSG_UdhSmallAnim    smallAnim;       /* 0f */
        MN_MSG_UdhLargePic     largePic;        /* 10 */
        MN_MSG_UdhSmallPic     smallPic;        /* 11 */
        MN_MSG_UdhVarPic       varPic;          /* 12 */
        MN_MSG_UdhUserPrompt   userPrompt;      /* 13 */
        MN_MSG_UdhEo           eo;              /* 14 */
        MN_MSG_UdhReo          reo;             /* 15 */
        MN_MSG_UdhCc           cc;              /* 16 */
        MN_MSG_UdhObjDistr     objDistr;        /* 17 */
        MN_MSG_UdhWvgObj       wvgObj;          /* 18,19 */
        MN_MSG_UdhRfc822       rfc822;          /* 20 */
        MN_MSG_HyperlinkFormat hyperLinkFormat; /* 21 */
        MN_MSG_UdhReplyAddr    replyAddr;       /* 22 */
        MN_MSG_UdhOther        other;
    } u;
} MN_MSG_UserHeaderType;

/* Content of TP User Data Header */
#define MN_MSG_MAX_UDH_NUM 7
typedef struct {
    VOS_UINT32 len;                     /* Length of sms */
    VOS_UINT8  orgData[MN_MSG_MAX_LEN]; /* sms content,not 7bit,all 8bit */

    VOS_UINT8             numofHeaders;                       /* number of user header */
    MN_MSG_UserHeaderType userDataHeader[MN_MSG_MAX_UDH_NUM]; /* detail of user header */
} MN_MSG_UserData;

/* Content of TP User Data(Concatenate Sms) */
#define MN_MSG_LONG_SMS_MAX_LEN (255 * 153)
typedef struct {
    VOS_UINT8             numofHeaders; /* number of user header */
    VOS_UINT8             reserve1[3];
    MN_MSG_UserHeaderType userDataHeader[MN_MSG_MAX_UDH_NUM]; /* detail of user header */
    VOS_UINT32            len;                                /* Length of sms */
    VOS_UINT8            *orgData;                            /* sms content,not 7bit,all 8bit */
} MN_MSG_LongUserData;

/* Content of Deliver TPDU */
typedef struct {
    VOS_BOOL              moreMsg;           /* TP-MMS */
    VOS_BOOL              replayPath;        /* TP-RP */
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    VOS_BOOL              staRptInd;         /* TP-SRI */
    MN_MSG_AsciiAddr      origAddr;          /* TP-OA */
    MN_MSG_TpPidTypeUint8 pid;               /* TP-PID */
    VOS_UINT8             reserve1[3];
    MN_MSG_DcsCode        dcs;       /* TP-DCS */
    MN_MSG_Timestamp      timeStamp; /* TP-SCTS */
    MN_MSG_UserData       userData;  /* TP-UD */
} MN_MSG_Deliver;

/* Content of Deliver TPDU(Concatenate Sms Msg) */
typedef struct {
    VOS_BOOL              moreMsg;           /* TP-MMS */
    VOS_BOOL              replayPath;        /* TP-RP */
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    VOS_BOOL              staRptInd;         /* TP-SRI */
    MN_MSG_AsciiAddr      origAddr;          /* TP-OA */
    MN_MSG_TpPidTypeUint8 pid;               /* TP-PID */
    VOS_UINT8             reserve1[3];
    MN_MSG_DcsCode        dcs;       /* TP-DCS */
    MN_MSG_Timestamp      timeStamp; /* TP-SCTS */
    VOS_UINT8             reserve2[4];
    MN_MSG_LongUserData   longUserData; /* TP-UD */
} MN_MSG_DeliverLong;

/* Content of Deliver Report-Ack TPDU */
typedef struct {
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    VOS_UINT8             paraInd;           /* TP-PI */
    MN_MSG_TpPidTypeUint8 pid;               /* TP-PID */
    VOS_UINT8             reserve1[2];
    MN_MSG_DcsCode        dcs;      /* TP-DCS */
    MN_MSG_UserData       userData; /* TP-UD */
} MN_MSG_DeliverRptAck;

/* Content of Deliver Report-Error TPDU */
typedef struct {
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    MN_MSG_TpCauseUint8   failCause;         /* TP-FCS */
    VOS_UINT8             paraInd;           /* TP-PI */
    MN_MSG_TpPidTypeUint8 pid;               /* TP-PID */
    VOS_UINT8             reserve1[1];
    MN_MSG_DcsCode        dcs;      /* TP-DCS */
    MN_MSG_UserData       userData; /* TP-UD */
} MN_MSG_DeliverRptErr;

/* Content of Submit TPDU */
typedef struct {
    VOS_BOOL              rejectDuplicates;  /* TP-RD */
    VOS_BOOL              replayPath;        /* TP-RP */
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    VOS_BOOL              staRptReq;         /* TP-SRR */
    VOS_UINT8             mr;                /* TP-MR */
    VOS_UINT8             reserve1[3];
    MN_MSG_AsciiAddr      destAddr; /* TP-DA */
    MN_MSG_TpPidTypeUint8 pid;      /* TP-PID */
    VOS_UINT8             reserve2[3];
    MN_MSG_DcsCode        dcs;         /* TP-DCS */
    MN_MSG_ValidPeriod    validPeriod; /* TP-VPF& TP-VP */
    MN_MSG_UserData       userData;    /* TP-UD */
} MN_MSG_Submit;

/* Content of Submit TPDU(Concatenate Sms Msg) */
typedef struct {
    VOS_BOOL              rejectDuplicates;  /* TP-RD */
    VOS_BOOL              replayPath;        /* TP-RP */
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    VOS_BOOL              staRptReq;         /* TP-SRR */
    VOS_UINT8             mr;                /* TP-MR */
    VOS_UINT8             reserve1[3];
    MN_MSG_AsciiAddr      destAddr; /* TP-DA */
    MN_MSG_TpPidTypeUint8 pid;      /* TP-PID */
    VOS_UINT8             reserve2[3];
    MN_MSG_DcsCode        dcs;         /* TP-DCS */
    MN_MSG_ValidPeriod    validPeriod; /* TP-VPF& TP-VP */
    VOS_UINT8             reserve3[4];
    MN_MSG_LongUserData   longUserData; /* TP-UD */
} MN_MSG_SubmitLong;

/* Content of Status Report-Ack TPDU */
typedef struct {
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    MN_MSG_Timestamp      timeStamp;         /* TP-SCTS */
    VOS_UINT8             paraInd;           /* TP-PI */
    MN_MSG_TpPidTypeUint8 pid;               /* TP-PID */
    VOS_UINT8             reserve1[2];
    MN_MSG_DcsCode        dcs;      /* TP-DCS */
    MN_MSG_UserData       userData; /* TP-UD */
} MN_MSG_SubmitRptAck;

/* Content of Submit Report-Error TPDU */
typedef struct {
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    MN_MSG_TpCauseUint8   failCause;         /* TP-FCS */
    VOS_UINT8             reserve1[3];
    MN_MSG_Timestamp      timeStamp; /* TP-SCTS */
    VOS_UINT8             paraInd;   /* TP-PI */
    MN_MSG_TpPidTypeUint8 pid;       /* TP-PID */
    VOS_UINT8             reserve2[2];
    MN_MSG_DcsCode        dcs;      /* TP-DCS */
    MN_MSG_UserData       userData; /* TP-UD */
} MN_MSG_SubmitRptErr;

/* Content of Status Report TPDU */
typedef struct {
    VOS_BOOL              userDataHeaderInd; /* TP-UDHI */
    VOS_BOOL              moreMsg;           /* TP-MMS */
    VOS_BOOL              staRptQualCommand; /* TP-SRQ */
    VOS_UINT8             mr;                /* TP-MR */
    VOS_UINT8             reserve1[3];
    MN_MSG_AsciiAddr      recipientAddr; /* TP-RA */
    MN_MSG_Timestamp      timeStamp;     /* TP-SCTS */
    MN_MSG_Timestamp      dischargeTime; /* TP-DT */
    MN_MSG_TpStatusUint8  status;        /* TP-ST */
    VOS_UINT8             paraInd;       /* TP-PI */
    MN_MSG_TpPidTypeUint8 pid;           /* TP-PID */
    VOS_UINT8             reserve2[1];
    MN_MSG_DcsCode        dcs;      /* TP-DCS */
    MN_MSG_UserData       userData; /* TP-UD */
} MN_MSG_StaRpt;

/* Content of Command TPDU */
#define MN_MSG_MAX_COMMAND_DATA_LEN 156
#define MN_MSG_MAX_COMMAND_TPDU_MSG_LEN 164
#define MN_MSG_MAX_STARPT_DATA_LEN 143
#define MN_MSG_DELIVER_TPDU_MANDATORY_IE_LEN 13
#define MN_MSG_DELIVER_RPTACK_TPDU_MANDATORY_IE_LEN 2
#define MN_MSG_DELIVER_RPTERR_TPDU_MANDATORY_IE_LEN 3
#define MN_MSG_SUBMIT_TPDU_MANDATORY_IE_LEN 7
#define MN_MSG_SUBMIT_RPTERR_TPDU_MANDATORY_IE_LEN 10
#define MN_MSG_SUBMIT_RPTACK_TPDU_MANDATORY_IE_LEN 9
#define MN_MSG_STATUS_REPORT_TPDU_MANDATORY_IE_LEN 19
#define MN_MSG_COMMAND_TPDU_MANDATORY_IE_LEN 8

typedef struct {
    VOS_BOOL                userDataHeaderInd;                    /* TP-UDHI */
    VOS_BOOL                staRptReq;                            /* TP-SRR */
    VOS_UINT8               mr;                                   /* TP-MR */
    MN_MSG_TpPidTypeUint8   pid;                                  /* TP-PID */
    MN_MSG_CommandTypeUint8 cmdType;                              /* TP-CT */
    VOS_UINT8               msgNumber;                            /* TP-MN */
    MN_MSG_AsciiAddr        destAddr;                             /* TP-DA */
    VOS_UINT8               commandDataLen;                       /* TP-CDL */
    VOS_UINT8               cmdData[MN_MSG_MAX_COMMAND_DATA_LEN]; /* TP-CD */
    VOS_UINT8               reserve1[3];
} MN_MSG_CommandParam;

/* Content of TPDU */
typedef struct {
    MN_MSG_TpduTypeUint8 tpduType;
    VOS_UINT8            reserve1[3];
    union {
        MN_MSG_Deliver       deliver;
        MN_MSG_DeliverRptAck deliverRptAck;
        MN_MSG_DeliverRptErr deliverRptErr;
        MN_MSG_Submit        submit;
        MN_MSG_SubmitRptAck  submitRptAck;
        MN_MSG_SubmitRptErr  submitRptErr;
        MN_MSG_StaRpt        staRpt;
        MN_MSG_CommandParam  command;
    } u;
} MN_MSG_TsDataInfo;

/* Sms default service para */
typedef struct {
    VOS_BOOL         replayPath;        /* TP-RP */
    VOS_BOOL         userDataHeaderInd; /* TP-UDHI */
    VOS_BOOL         staRptReq;         /* TP-SRR */
    VOS_UINT8        mr;                /* TP-MR */
    VOS_UINT8        reserve1[3];
    MN_MSG_AsciiAddr destAddr; /* TP-DA */
    MN_MSG_UserData  userData; /* TP-UD */
} MN_MSG_SubmitDefParm;

/* Sms Rp Cause Info */
typedef struct {
    /* is true while Diagnostic field present */
    VOS_BOOL diagnosticsExist;
    /* a diagnostic field giving further details of the error cause */
    VOS_UINT8           diagnostics;
    MN_MSG_RpCauseUint8 rpCause; /* rp cause */
    VOS_UINT8           reserve1[2];
} MN_MSG_RpCausePara;


enum TAF_MSG_Error {
    TAF_MSG_ERROR_NO_ERROR = 0x0000,

    TAF_MSG_ERROR_STATE_NOT_COMPATIBLE = 0x0001,

    TAF_MSG_ERROR_NO_SERVICE = 0x0002,

    TAF_MSG_ERROR_TC1M_TIMEOUT = 0x0003,

    TAF_MSG_ERROR_TR1M_TIMEOUT = 0x0004,

    TAF_MSG_ERROR_TR2M_TIMEOUT = 0x0005,

    TAF_MSG_ERROR_USER_ABORT = 0x0006,

    TAF_MSG_ERROR_TP_ERROR_BEGIN = 0x010000,

    TAF_MSG_ERROR_TP_FCS_TELEMATIC_INTERWORKING_NOT_SUPPORTED      = 0x010080,
    TAF_MSG_ERROR_TP_FCS_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED        = 0x010081,
    TAF_MSG_ERROR_TP_FCS_CANNOT_REPLACE_SHORT_MESSAGE              = 0x010082,
    TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPPID_ERROR                   = 0x01008f,
    TAF_MSG_ERROR_TP_FCS_DATA_CODING_SCHEME_ALPHABET_NOT_SUPPORTED = 0x010090,
    TAF_MSG_ERROR_TP_FCS_MESSAGE_CLASS_NOT_SUPPORTED               = 0x010091,
    TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPDCS_ERROR                   = 0x01009f,
    TAF_MSG_ERROR_TP_FCS_COMMAND_CANNOT_BE_ACTIONED                = 0x0100a0,
    TAF_MSG_ERROR_TP_FCS_COMMAND_UNSUPPORTED                       = 0x0100a1,
    TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_TPCOMMAND_ERROR               = 0x0100af,
    TAF_MSG_ERROR_TP_FCS_TPDU_NOT_SUPPORTED                        = 0x0100b0,
    TAF_MSG_ERROR_TP_FCS_SC_BUSY                                   = 0x0100c0,
    TAF_MSG_ERROR_TP_FCS_NO_SC_SUBSCRIPTION                        = 0x0100c1,
    TAF_MSG_ERROR_TP_FCS_SC_SYSTEM_FAILURE                         = 0x0100c2,
    TAF_MSG_ERROR_TP_FCS_INVALID_SME_ADDRESS                       = 0x0100c3,
    TAF_MSG_ERROR_TP_FCS_DESTINATION_SME_BARRED                    = 0x0100c4,
    TAF_MSG_ERROR_TP_FCS_SM_REJECTEDDUPLICATE_SM                   = 0x0100c5,
    TAF_MSG_ERROR_TP_FCS_TPVPF_NOT_SUPPORTED                       = 0x0100c6,
    TAF_MSG_ERROR_TP_FCS_TPVP_NOT_SUPPORTED                        = 0x0100c7,
    TAF_MSG_ERROR_TP_FCS_SIM_SMS_STORAGE_FULL                      = 0x0100d0,
    TAF_MSG_ERROR_TP_FCS_NO_SMS_STORAGE_CAPABILITY_IN_SIM          = 0x0100d1,
    TAF_MSG_ERROR_TP_FCS_ERROR_IN_MS                               = 0x0100d2,
    TAF_MSG_ERROR_TP_FCS_MEMORY_CAPACITY_EXCEEDED                  = 0x0100d3,
    TAF_MSG_ERROR_TP_FCS_SIM_APPLICATION_TOOLKIT_BUSY              = 0x0100d4,
    TAF_MSG_ERROR_TP_FCS_SIM_DATA_DOWNLOAD_ERROR                   = 0x0100d5,
    TAF_MSG_ERROR_TP_FCS_UNSPECIFIED_ERROR_CAUSE                   = 0x0100ff,

    TAF_MSG_ERROR_CP_ERROR_BEGIN = 0x020000,

    TAF_MSG_ERROR_RP_ERROR_BEGIN = 0x040000,

    TAF_MSG_ERROR_RP_CAUSE_UNASSIGNED_UNALLOCATED_NUMBER                            = 0x040001,
    TAF_MSG_ERROR_RP_CAUSE_OPERATOR_DETERMINED_BARRING                              = 0x040008,
    TAF_MSG_ERROR_RP_CAUSE_CALL_BARRED                                              = 0x04000a,
    TAF_MSG_ERROR_RP_CAUSE_SHORT_MESSAGE_TRANSFER_REJECTED                          = 0x040015,
    TAF_MSG_ERROR_RP_CAUSE_DESTINATION_OUT_OF_ORDER                                 = 0x04001b,
    TAF_MSG_ERROR_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER                                  = 0x04001c,
    TAF_MSG_ERROR_RP_CAUSE_FACILITY_REJECTED                                        = 0x04001d,
    TAF_MSG_ERROR_RP_CAUSE_UNKNOWN_SUBSCRIBER                                       = 0x04001e,
    TAF_MSG_ERROR_RP_CAUSE_NETWORK_OUT_OF_ORDER                                     = 0x040026,
    TAF_MSG_ERROR_RP_CAUSE_TEMPORARY_FAILURE                                        = 0x040029,
    TAF_MSG_ERROR_RP_CAUSE_CONGESTION                                               = 0x04002a,
    TAF_MSG_ERROR_RP_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED                        = 0x04002f,
    TAF_MSG_ERROR_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED                        = 0x040032,
    TAF_MSG_ERROR_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED                       = 0x040045,
    TAF_MSG_ERROR_RP_CAUSE_INVALID_SHORT_MESSAGE_TRANSFER_REFERENCE_VALUE           = 0x040051,
    TAF_MSG_ERROR_RP_CAUSE_INVALID_MANDATORY_INFORMATION                            = 0x040060,
    TAF_MSG_ERROR_RP_CAUSE_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED             = 0x040061,
    TAF_MSG_ERROR_RP_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_SHORT_MESSAGE_PROTOCOL_STATE = 0x040062,
    TAF_MSG_ERROR_RP_CAUSE_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED      = 0x040063,
    TAF_MSG_ERROR_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED                               = 0x04006f,
    TAF_MSG_ERROR_RP_CAUSE_INTERWORKING_UNSPECIFIED                                 = 0x04007f,

    TAF_MSG_ERROR_CS_ERROR_BEGIN = 0x080000,

    TAF_MSG_ERROR_PS_ERROR_BEGIN = 0x100000,

    TAF_MSG_ERROR_EPS_ERROR_BEGIN = 0x200000,

    TAF_MSG_ERROR_IMS_ERROR_BEGIN = 0x400000,

    TAF_MSG_ERROR_CHECK_ERROR_BEGIN = 0x800000,

    TAF_MSG_ERROR_FDN_CHECK_FAIL     = 0x800001,
    TAF_MSG_ERROR_FDN_CHECK_TIMEROUT = 0x800002,

    TAF_MSG_ERROR_CTRL_CHECK_FAIL    = 0x800010,
    TAF_MSG_ERROR_CTRL_CHECK_TIMEOUT = 0x800011,

    TAF_MSG_ERROR_PARA_CHECK_FAIL = 0x800012,

    TAF_MSG_ERROR_ERROR_BUTT = 0xFFFFFFFF
};
typedef VOS_UINT32 TAF_MSG_ErrorUint32;

/* event report:MN_MSG_SUBMIT_RPT_EVT_INFO_STRU */
typedef struct {
    TAF_MSG_ErrorUint32  errorCode;
    VOS_UINT8            mr;       /* msg reference */
    MN_MSG_MemStoreUint8 saveArea; /* save area */
    VOS_UINT8            reserve1[2];
    VOS_UINT32           saveIndex; /* save index */
    MN_MSG_RawTsData     rawData;   /* TPDU of sms submit report */
} MN_MSG_SubmitRptEvtInfo;

/* event report:MN_MSG_EVT_MSG_SENT,MN_MSG_EVT_MSG_STORED */
typedef struct {
    MN_MSG_TpduTypeUint8 tpduType;
    VOS_UINT8            reserve1[3];
} MN_MSG_SendmsgEvtInfo;

/* event report:MN_MSG_EVT_DELIVER */
typedef struct {
    VOS_UINT32 inex; /* index of sms */
    /* memory store:NVIM or USIM of received msg */
    MN_MSG_MemStoreUint8  memStore;
    MN_MSG_RcvmsgActUint8 rcvSmAct; /* action of received msg */
#if (FEATURE_ON == FEATURE_BASTET)
    VOS_UINT8 ucBlockRptFlag; /* blocklist sms flag */
    VOS_UINT8 aucReserve1[1];
#else
    VOS_UINT8 reserve1[2];
#endif
    MN_MSG_MsgInfo rcvMsgInfo; /* sms content */
} MN_MSG_DeliverEvtInfo;

/* event report:MN_MSG_EVT_DELIVER_ERR */
typedef struct {
    TAF_MSG_ErrorUint32 errorCode;
} MN_MSG_DeliverErrEvtInfo;

/* event report:MN_MSG_EVT_SM_STORAGE_LIST */
#define MN_MSG_MAX_SM_STATUS 4
typedef struct {
    MN_MSG_MemStoreUint8 memStroe; /* sms memory store:NVIM or USIM */
    VOS_UINT8            reserve1[3];
    VOS_UINT32           totalRec; /* sms capacity of NVIM or USIM */
    /* used records including all status */
    VOS_UINT32 usedRec;
    VOS_UINT32 eachStatusRec[MN_MSG_MAX_SM_STATUS]; /* sms number of each status */
} MN_MSG_StorageListEvtInfo;

/* event report:MN_MSG_EVT_STORAGE_FULL,MN_MSG_EVT_STORAGE_EXCEED */
typedef struct {
    MN_MSG_MemStoreUint8 memStroe; /* sms memory store:NVIM or USIM */
    VOS_UINT8            reserve1[3];
    VOS_UINT32           totalRec; /* sms capacity of NVIM or USIM */
    /* used records including all status */
    VOS_UINT32 usedRec;
} MN_MSG_StorageStateEvtInfo;

/* event report:MN_MSG_EVT_READ ,MN_MSG_EVT_READ_STATUSREPORT */
typedef struct {
    VOS_BOOL               success;   /* read sms success or failure */
    VOS_UINT32             failCause; /* Failure case when read failed */
    VOS_UINT32             index;     /* sms memory index */
    MN_MSG_MemStoreUint8   memStore;
    MN_MSG_StatusTypeUint8 status; /* sms status */
    VOS_UINT8              reserve1[2];
    MN_MSG_MsgInfo         msgInfo; /* sms content */
} MN_MSG_ReadEvtInfo;

typedef struct {
    VOS_UINT32             index;  /* sms memory index */
    MN_MSG_StatusTypeUint8 status; /* sms status */
    VOS_UINT8              reserve1[3];
    MN_MSG_MsgInfo         msgInfo; /* sms content */
} MN_MSG_EachsmListInfo;

typedef struct {
    MN_MSG_StatusTypeUint8 status;   /* sms status */
    MN_MSG_MemStoreUint8   memStore; /* momory store:NVIM or USIM */

    /* 消息请求类型:VOS_TRUE 表示首次请求，VOS_FALSE表示请求上报余下部分MSG */
    VOS_UINT8 isFirstTimeReq;
    VOS_UINT8 reserve1[1];
    /* change status or not change status after read or list */
    VOS_BOOL changeFlag;
} MN_MSG_ListParm;

/* Max Report Number when List Event Report */
#define MN_MSG_MAX_REPORT_EVENT_NUM 10


typedef struct {
    VOS_BOOL              success;     /* list sms success or failure */
    VOS_UINT32            failCause;   /* Failure case when list failed */
    VOS_BOOL              lastListEvt; /* The Last List Event Info */
    VOS_UINT32            reportNum;   /* the num of report evt this time */
    MN_MSG_EachsmListInfo smInfo[MN_MSG_MAX_REPORT_EVENT_NUM];
    VOS_BOOL              firstListEvt;

    MN_MSG_ListParm receivedListPara; /* 上报收到的LIST参数信息 */
} MN_MSG_ListEvtInfo;

/* event report:MN_MSG_EVT_WRITE */
typedef struct {
    VOS_BOOL               success;   /* write sms success or failure */
    VOS_UINT32             failCause; /* Failure case when write failed */
    VOS_UINT32             index;     /* sms memory index */
    MN_MSG_StatusTypeUint8 status;    /* sms status */
    MN_MSG_MemStoreUint8   memStroe;  /* sms memory store */
    VOS_UINT8              reserve1[2];
} MN_MSG_WriteEvtInfo;

/*
 * event report:MN_MSG_EVT_DELETE,MN_MSG_EVT_DELETE_SR,
 * MN_MSG_EVT_DELETE_CBS,MN_MSG_EVT_DELETE_SETSRV_PARAM
 */
typedef struct {
    VOS_BOOL               success;    /* delete success or failure */
    VOS_UINT32             failCause;  /* Failure case when delete failed */
    VOS_UINT32             index;      /* sms memory index */
    MN_MSG_MemStoreUint8   memStore;   /* sms memory store:NVIM or USIM */
    MN_MSG_DeleteTypeUint8 deleteType; /* sms delete type */
    VOS_UINT8              reserve1[2];
} MN_MSG_DeleteEvtInfo;

typedef struct {
    VOS_UINT32 smCapacity;
    VOS_UINT32 validLocMap[MN_MSG_CMGD_PARA_MAX_LEN];
} MN_MSG_DeleteTestEvtInfo;

/* event report:MN_MSG_MODIFY_STATUS */
typedef struct {
    VOS_BOOL               success;   /* modify success or failure */
    VOS_UINT32             failCause; /* Failure case when modify failed */
    VOS_UINT32             index;     /* sms memory index */
    MN_MSG_MemStoreUint8   memStore;  /* sms memory store:NVIM or USIM */
    MN_MSG_StatusTypeUint8 status;    /* sms status */
    VOS_UINT8              reserve1[2];
} MN_MSG_ModifyStatusEvtInfo;

/*
 * event report:MN_MSG_EVT_SET_SETSRV_PARAM
 *                MN_MSG_EVT_GET_SETSRV_PARAM
 */
typedef struct {
    VOS_BOOL success; /* set(get) success or failure */
    /* Failure case when set or get failed */
    VOS_UINT32 failCause;
    /* is Set or Get,Get Including unsolicited report */
    MN_MSG_OPER_TYPE_T operType;
    VOS_UINT8          reserve1[3];
    VOS_UINT32         index;   /* sms serveice para index */
    MN_MSG_SrvParam    srvParm; /* sms serveice para */
} MN_MSG_SrvParmEvtInfo;

/* event report:MN_MSG_EVT_CHANGED_SETSRV_PARAM */
/* Max record is 5 in USIM of EFSMSP */
#define MN_MSG_MAX_USIM_EFSMSP_NUM 5

/* EFSMSP中Default csca csmp 存储索引 */
#define MN_MSG_DEFAULT_CSCA_CSMP_STORAGE_INDEX (0)

typedef struct {
    VOS_UINT32      totalRec;
    VOS_UINT32      usedRec;
    MN_MSG_SrvParam srvParm[MN_MSG_MAX_USIM_EFSMSP_NUM];
} MN_MSG_SrvParmChangedEvtInfo;

/* event report:MN_MSG_EVT_SETMEMSTATUS,MN_MSG_EVT_MEMSTATUS_CHANGED */
typedef struct {
    VOS_BOOL success; /* set success */
    /* Failure case when set or get failed */
    VOS_UINT32          failCause;
    MN_MSG_MemFlagUint8 memFlag;
    VOS_UINT8           reserve1[3];
} MN_MSG_MemstatusEvtInfo;

/*
 * event report:MN_MSG_EVT_SET_RCVMSG_PATH,MN_MSG_EVT_GET_RCVMSG_PATH,
 *                MN_MSG_EVT_RCVMSG_PATH_CHANGED
 */
typedef struct {
    VOS_BOOL success; /* set(get) success or failure */
    /* Failure case when set or get failed */
    VOS_UINT32 failCause;
    /* is Set or Get,Get Including unsolicited report */
    MN_MSG_OPER_TYPE_T    operType;
    MN_MSG_RcvmsgActUint8 rcvSmAct; /* action of received msg */
    /* Sm memory store:NVIM or USIM of received msg */
    MN_MSG_MemStoreUint8 smMemStore;
    /* action of received Status Report msg */
    MN_MSG_RcvmsgActUint8 rcvStaRptAct;
    /* Sm memory store:NVIM or USIM of Status Report msg */
    MN_MSG_MemStoreUint8 staRptMemStore;
    /* Cbm memory store:NVIM of received msg */
    MN_MSG_MemStoreUint8       cbmMemStore;
    MN_MSG_CsmsMsgVersionUint8 smsServVersion; /* 巴西vivo短信方案-添加变量 */
    VOS_UINT8                  reserve1[1];
} MN_MSG_RcvmsgPathEvtInfo;

/*
 * event report:MN_MSG_EVT_INIT_RESULT
 * Para of MN_MSG_SetRcvMsgPath
 */
typedef struct {
    MN_MSG_RcvmsgActUint8 rcvSmAct; /* action of received SM msg */
    /* Sm memory store:NVIM or USIM of SM msg */
    MN_MSG_MemStoreUint8 smMemStore;
    /* action of received Status Report msg */
    MN_MSG_RcvmsgActUint8 rcvStaRptAct;
    /* Sm memory store:NVIM or USIM of Status Report msg */
    MN_MSG_MemStoreUint8       staRptMemStore;
    MN_MSG_CsmsMsgVersionUint8 smsServVersion; /* 巴西vivo短信方案-添加变量 */

    MN_MSG_CnmiMtTypeUint8 cnmiMtType;
    VOS_UINT8              reserve1[2];
} MN_MSG_SetRcvmsgPathParm;

/* event report:MN_MSG_EVT_INIT_RESULT */
typedef struct {
    VOS_UINT32 totalSmRec; /* sms capacity of USIM */
    /* used records including all status */
    VOS_UINT32 usedSmRec;
    VOS_UINT32 eachStatusRec[MN_MSG_MAX_SM_STATUS]; /* sms number of each status */
    VOS_UINT32 totalSmsrRec;                        /* smsr capacity of USIM */
    VOS_UINT32 usedSmsrRec;                         /* used records of smsr */
} MN_MSG_InitEvtInfo;

/* event report:MN_MSG_EVT_INIT_SMSP_RESULT */
typedef struct {
    VOS_UINT32               totalSrvParm;
    VOS_UINT32               usedSrvParm;
    MN_MSG_SrvParam          srvParm[MN_MSG_MAX_USIM_EFSMSP_NUM];
    MN_MSG_SetRcvmsgPathParm rcvMsgPath;
    MN_MSG_Class0TailorUint8 class0Tailor;
    VOS_UINT8 defaultSmspIndex;
    VOS_UINT8 reserved[2];
} MN_MSG_InitSmspEvtInfo;

typedef struct {
    VOS_UINT32           errorCode; /* set(get) success or failure */
    MN_MSG_LinkCtrlUint8 linkCtrl;
    VOS_UINT8            reserved[3];
} MN_MSG_LinkCtrlEvtInfo;


typedef struct {
    VOS_UINT32             errorCode;  /* 设置成功或者失败 */
    MN_MSG_SendDomainUint8 sendDomain; /* 短信发送域 */
    VOS_UINT8              reserved[3];
} MN_MSG_SendDomainEvtInfo;

/*
 * 枚举名: MN_MSG_STUB_EVT_INFO_STRU
 * 结构说明: 短信桩相关操作结果上报
 */
typedef struct {
    VOS_UINT32 errorCode; /* set(get) success or failure */
} MN_MSG_ResultEvtInfo;

#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))

typedef struct {
    /* Length of cbs user message content */
    VOS_UINT32 len;
    VOS_UINT8  content[TAF_CBA_MAX_RAW_CBDATA_LEN];
    VOS_UINT8  reserve3[3];
} MN_MSG_CbdataInfo;

typedef struct {
    VOS_UINT8             msgCodingGroup;
    MN_MSG_MsgCodingUint8 msgCoding; /* sms msg coding */
    VOS_UINT8             reserve1[2];
    VOS_BOOL              compressed;
    MN_MSG_CblangUint8    msgLang;
    MN_MSG_MsgClassUint8  msgClass;
    /* used when ucCodingGroup == 1 Refer to 23038 5. */
    VOS_UINT8 langIndLen;
    VOS_UINT8 rawDcsData; /* Raw DCS Byte */
} MN_MSG_CbdcsCode;

/* CBS消息中的GS相关信息 */
enum MN_MSG_Cbgs {
    MN_MSG_CBGS_CELL_IMME   = 0x00,
    MN_MSG_CBGS_PLMN_NORMAL = 0x01,
    MN_MSG_CBGS_LA_NORMAL   = 0x02,
    MN_MSG_CBGS_CELL_NORMAL = 0x03
};
typedef VOS_UINT8 MN_MSG_CbgsUint8;

typedef struct {
    MN_MSG_CbgsUint8 gs;
    VOS_UINT8        updateNumber;
    VOS_UINT16       messageCode;
    VOS_UINT16       rawSnData; /* Raw SN Byte */
    VOS_UINT8        reserve1[2];
} MN_MSG_Cbsn;

typedef struct {
    VOS_UINT16 mid;
    VOS_UINT8  pageIndex;
    VOS_UINT8  pageNum;

    MN_MSG_CbdcsCode  dcs;
    MN_MSG_Cbsn       sn;
    MN_MSG_CbdataInfo content;
} MN_MSG_Cbpage;


typedef struct {
    /* 小区广播消息id范围标签 */
    VOS_UINT8 label[TAF_CBA_MAX_LABEL_NUM];
    /* 小区广播消息ID的开始序号  */
    VOS_UINT16 msgIdFrom;
    /* 小区广播消息ID的结束序号 */
    VOS_UINT16 msgIdTo;
    /* 每个CBMI RANGE 的接收模式, 目前仅支持 ACCEPT的模式 */
    TAF_CBA_CbmiRangeRcvModeEnmuUint8 rcvMode;
    /* 每个CBMI RANGE 所属的来源标志，代表此MSG ID是从哪里获取的: NV, EFCBMI, EFCBMIR，有可能是多个来源标志的并集，有可能只是单独某个来源标志 */
    VOS_UINT8 msgIdSrcFlg;
    VOS_UINT8 reserve[2];
} TAF_CBA_CbmiRange;


typedef struct {
    VOS_UINT16        cbmirNum; /* 小区广播消息的ID个数 */
    VOS_UINT8         reserve[6];
    TAF_CBA_CbmiRange cbmiRangeInfo[TAF_CBA_MAX_CBMID_RANGE_NUM]; /* 小区广播消息的范围信息 */
} TAF_CBA_CbmiRangeList;


typedef struct {
    VOS_UINT32 mcc; /* MCC,3 bytes      */
    VOS_UINT32 mnc; /* MNC,2 or 3 bytes */
} TAF_CBA_PlmnId;

typedef struct {
    VOS_UINT32 len;
    VOS_UINT8  data[TAF_CBA_MAX_CBPAGE_LEN];
} MN_MSG_CbrawTsData;


typedef struct {
    VOS_BOOL           success;    /* Deliver CBM success or failure */
    VOS_UINT32         failCause;  /* Failure case when Deliver failed */
    MN_MSG_CbrawTsData cbRawData;  /* TPDU */
    VOS_UINT16         geoAreaLen; /* 地理位置坐标信息长度 */
    VOS_UINT8          geoInfoRptFlag;
    VOS_UINT8          reserved;
    VOS_UINT8          geoAreaStr[TAF_CBS_CMAS_GEO_AREA_MAX_LEN]; /* 地理位置坐标信息 */
} MN_MSG_CbDeliverEvtInfo;

/* event report:MN_MSG_EVT_ADD_CBMIDS,MN_MSG_EVT_DELETE_CBMIDS,MN_MSG_EVT_DELETE_ALL_CBMIDS */
typedef struct {
    VOS_BOOL   success;
    VOS_UINT32 failCause;
} MN_MSG_CbmidsChangeEvtInfo;

typedef struct {
    VOS_BOOL success; /* set(get) success or failure */
    /* Failure case when read or write failed */
    VOS_UINT32            failCause;
    TAF_CBA_CbmiRangeList cbMidr; /* CBS Mid Range */
} MN_MSG_CbmidsGetEvtInfo;

typedef struct {
    MN_CLIENT_ID_T    clientId;
    MN_OPERATION_ID_T opId;
    VOS_UINT8         reserved;
    VOS_UINT32        failCause; /* Failure case when read failed */
} MN_MSG_EventParm;

#if (FEATURE_ON == FEATURE_ETWS)


typedef struct {
    TAF_CBA_PlmnId plmn;     /* plmn */
    VOS_UINT16     warnType; /* 告警类型 */
    VOS_UINT16     msgId;    /* 消息ID */

    VOS_UINT16                       sn;       /* 序列号 */
    TAF_CBA_EtwsPrimNtfAuthRsltUint8 authRslt; /* 鉴权结果 */
    VOS_UINT8                        rsv;      /* 字节对齐保留 */
} TAF_CBA_EtwsPrimNtfEvtInfo;

#endif

#endif /* ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS)) */

/* Sms Event Type */
/*
 * 结构说明: TAF的短信模块事件上报的事件名称
 */
enum MN_MSG_Event {
    /* send msg success or failed */
    /* _H2ASN_MsgChoice MN_MSG_SubmitRptEvtInfo */
    MN_MSG_EVT_SUBMIT_RPT,
    /* msg has been sent */
    /* _H2ASN_MsgChoice MN_MSG_SendmsgEvtInfo */
    MN_MSG_EVT_MSG_SENT,
    MN_MSG_EVT_MSG_STORED, /* msg has been stored */
    MN_MSG_EVT_DELIVER,    /* rcv a new msg */
    /* rcv a new msg but not rcv deliver-report from APP on time */
    MN_MSG_EVT_DELIVER_ERR,
    /* number of certain status sms in nvim or usim */
    MN_MSG_EVT_SM_STORAGE_LIST,
    MN_MSG_EVT_STORAGE_FULL,      /* memory storage full */
    MN_MSG_EVT_STORAGE_EXCEED,    /* Rcv a SMS,but no memory to rcv */
    MN_MSG_EVT_READ,              /* read sms */
    MN_MSG_EVT_LIST,              /* list sms */
    MN_MSG_EVT_WRITE,             /* write sms */
    MN_MSG_EVT_DELETE,            /* delete sms */
    MN_MSG_EVT_DELETE_TEST,       /* delete sms test */
    MN_MSG_EVT_MODIFY_STATUS,     /* modify sms status */
    MN_MSG_EVT_WRITE_SRV_PARM,    /* set sms service parameter */
    MN_MSG_EVT_READ_SRV_PARM,     /* get sms service parameter */
    MN_MSG_EVT_SRV_PARM_CHANGED,  /* service para changed */
    MN_MSG_EVT_DELETE_SRV_PARM,   /* delete sms service parameter */
    MN_MSG_EVT_READ_STARPT,       /* read sms status report */
    MN_MSG_EVT_DELETE_STARPT,     /* delete sms status report */
    MN_MSG_EVT_SET_MEMSTATUS,     /* set memory status of app */
    MN_MSG_EVT_MEMSTATUS_CHANGED, /* App memory status changed */
    /* get sms from sms status report or get sms status report from sms */
    MN_MSG_EVT_MATCH_MO_STARPT_INFO,
    MN_MSG_EVT_SET_RCVMSG_PATH,     /* sms set rcv msg path */
    MN_MSG_EVT_GET_RCVMSG_PATH,     /* sms get rcv msg path */
    MN_MSG_EVT_RCVMSG_PATH_CHANGED, /* sms rcv msg path changed */
    MN_MSG_EVT_INIT_SMSP_RESULT,
    MN_MSG_EVT_INIT_RESULT,         /* initialization of sms finished */
    MN_MSG_EVT_SET_LINK_CTRL_PARAM, /* sms set link ctrl parameter */
    MN_MSG_EVT_GET_LINK_CTRL_PARAM, /* sms get link ctrl parameter */
    MN_MSG_EVT_STUB_RESULT,         /* 短信相关操作结果上报事件 */
    MN_MSG_EVT_DELIVER_CBM,              /* rcv a new msg */
    MN_MSG_EVT_GET_CBTYPE,               /* Get Cbs Type */
    MN_MSG_EVT_ADD_CBMIDS,               /* Add Cbs Mids */
    MN_MSG_EVT_DELETE_CBMIDS,            /* Delete Cbs Mids */
    MN_MSG_EVT_DELETE_ALL_CBMIDS,        /* Delete ALL Cbs Mids */
    MN_MSG_EVT_DELIVER_ETWS_PRIM_NOTIFY, /* rcv a new primary notify */
    MN_MSG_EVT_SET_SEND_DOMAIN_PARAM, /* sms set send domain parameter */
    MN_MSG_EVT_GET_SEND_DOMAIN_PARAM, /* sms get send domain parameter */
    MN_MSG_EVT_MAX
};
typedef VOS_UINT32 MN_MSG_EventUint32;

/* Sms Event Report */
typedef struct {
    MN_CLIENT_ID_T    clientId;
    MN_OPERATION_ID_T opId; /* 1-127,APP orig 128-255 Taf orig */
    VOS_UINT8         reserve1[1];
    union {
        /* event report:MN_MSG_EVT_SUBMIT_RPT */
        MN_MSG_SubmitRptEvtInfo submitRptInfo;
        /* event report:MN_MSG_EVT_MSG_SENT,MN_MSG_EVT_MSG_STORED */
        MN_MSG_SendmsgEvtInfo sendMsgInfo;
        MN_MSG_DeliverEvtInfo deliverInfo; /* event report:MN_MSG_EVT_DELIVER */
        /* event report:MN_MSG_EVT_DELIVER_ERR */
        MN_MSG_DeliverErrEvtInfo deliverErrInfo;
        /* event report:MN_MSG_EVT_SM_STORAGE_LIST */
        MN_MSG_StorageListEvtInfo storageListInfo;
        /* event report:MN_MSG_EVT_STORAGE_FULL,MN_MSG_EVT_STORAGE_EXCEED */
        MN_MSG_StorageStateEvtInfo storageStateInfo;
        /* event report:MN_MSG_EVT_READ ,MN_MSG_EVT_READ_STARPT,MN_MSG_EVT_MATCHMO_STARPT_INFO */
        MN_MSG_ReadEvtInfo  readInfo;
        MN_MSG_ListEvtInfo  listInfo;  /* event report:MN_MSG_EVT_LIST */
        MN_MSG_WriteEvtInfo writeInfo; /* event report:MN_MSG_EVT_WRITE */
        /* event report:MN_MSG_EVT_DELETE,MN_MSG_EVT_DELETE_SR,MN_MSG_EVT_DELETE_SRV_PARAM,MN_MSG_EVT_DELETE_CBS */
        MN_MSG_DeleteEvtInfo deleteInfo;
        /* event report:MN_MSG_EVT_DELETE_TEST */
        MN_MSG_DeleteTestEvtInfo deleteTestInfo;
        /* event report:MN_MSG_MODIFY_STATUS */
        MN_MSG_ModifyStatusEvtInfo modifyInfo;
        /* event report:MN_MSG_EVT_WRITE_SRV_PARM,MN_MSG_EVT_READ_SRV_PARM */
        MN_MSG_SrvParmEvtInfo srvParmInfo;
        /* event report:MN_MSG_EVT_SRV_PARM_CHANGED */
        MN_MSG_SrvParmChangedEvtInfo srvParmChangeInfo;
        /* event report:MN_MSG_EVT_SETMEMSTATUS,MN_MSG_EVT_MEMSTATUS_CHANGED */
        MN_MSG_MemstatusEvtInfo memStatusInfo;
        /* event report:MN_MSG_EVT_SET_RCVMSG_PATH,MN_MSG_EVT_GET_RCVMSG_PATH,MN_MSG_EVT_RCVMSG_PATH_CHANGED */
        MN_MSG_RcvmsgPathEvtInfo rcvMsgPathInfo;
        /* event report:MN_MSG_EVT_INIT_RESULT */
        MN_MSG_InitEvtInfo     initResultInfo;
        MN_MSG_InitSmspEvtInfo initSmspResultInfo;
        /* event report:MN_MSG_EVT_SET_LINK_CTRL_PARAM,MN_MSG_EVT_GET_LINK_CTRL_PARAM */
        MN_MSG_LinkCtrlEvtInfo linkCtrlInfo;
        /* event report:MN_MSG_EVT_GET_LINK_CTRL_PARAM, etc */
        MN_MSG_ResultEvtInfo result;
#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))
        /* event report:MN_MSG_EVT_DELIVER_CBS */
        MN_MSG_CbDeliverEvtInfo    cbsDeliverInfo;
        MN_MSG_CbmidsChangeEvtInfo cbsChangeInfo;
        MN_MSG_CbmidsGetEvtInfo    cbsCbMids;
#if (FEATURE_ON == FEATURE_ETWS)
        TAF_CBA_EtwsPrimNtfEvtInfo etwsPrimNtf; /* ETWS主通知信息上报 */
#endif

#endif
        /* event report:MN_MSG_EVT_SET_SEND_DOMAIN_PARAM,MN_MSG_EVT_GET_SEND_DOMAIN_PARAM */
        MN_MSG_SendDomainEvtInfo sendDomainInfo;
    } u;
} MN_MSG_EventInfo;

typedef struct {
    MN_MSG_LinkCtrlUint8 relayLinkCtrl;
} MN_MSG_LinkCtrlType;


typedef struct {
    MN_MSG_SendDomainUint8 smsSendDomain;
    VOS_UINT8              reserved[3];
} MN_MSG_SmsSendDomainType;

/*
 * 枚举名: MN_MSG_StubMsg
 * 结构说明: 短信相关桩消息参数结构
 *           enStubType 桩类型
 *           ulValue    设置类桩消息有效，指定类型的桩需要配置C核参数的值，
 *              <enStubType>           <Value>的含义
 *                  0             efsms文件不可用； 0: efsms文件可用；1: efsms文件不可用；
 *                  1             efsms文件容量为1；0:不启用桩，其他 : EFSMS的容量为<Value>指定容量；
 *                  2             efsmss溢出标志清除操作；0: efsmss文件溢出标志有效；1: 强制清除efsmss文件溢出标志；
 *                  3             efsmsp文件获取无响应；0: efsmsp文件操作正常；1: efsmsp文件操作无响应；
 *                  4             efsms文件获取无响应；0: efsms文件操作正常；1: efsms文件操作无响应；
 *                  5             触发STK短信发送，STK短信内容需要压缩编码
 *                  6             触发STK短信发送，STK短信长度超过标准短信长度，需要分段发送；
 *                  7             去使能短信重发功能； 0: 短信重发功能正常；1: 不启用短信重发功能
 *                  8             设置CLASS0类短信的定制配置；
 *                  9             设置PP DOWNLOAD操作失败，并指定失败原因值
 *                 10             PS域短信连发开关 0: 默认关闭PS域短信连发功能；1: 启动PS域短信连发功能；
 *                 11             触发短信自动回复功能 0: 默认关闭短信自动回复桩过程；1: 启动短信自动回复桩过程；
 */
typedef struct {
    MN_MSG_StubTypeUint32 stubType;
    VOS_UINT32            value;
} MN_MSG_StubMsg;

enum MN_MSG_ClientType {
    MN_MSG_CLIENT_NORMAL = 0, /* not saved by PS */
    MN_MSG_CLIENT_STK,
    MN_MSG_CLIENT_BUTT
};
typedef VOS_UINT8 MN_MSG_ClientTypeUint8;

/* Parm of MN_MSG_Send */
typedef struct {
    MN_MSG_MemStoreUint8   memStore;
    MN_MSG_ClientTypeUint8 clientType;
    VOS_UINT8              reserve1[2];
    TAF_MSG_SignallingTypeUint32 msgSignallingType;
    MN_MSG_MsgInfo msgInfo;
} MN_MSG_SendParm;

/* Parm of MN_MSG_SendFromMem */
typedef struct {
    MN_MSG_MemStoreUint8 memStore;
    VOS_UINT8            reserve1[3];
    TAF_MSG_SignallingTypeUint32 msgSignallingType;
    VOS_UINT32     index;    /* sms memory index */
    MN_MSG_BcdAddr destAddr; /* destination addr of sms */
} MN_MSG_SendFrommemParm;

/* Para of Send MN_MSG_SendAck */
typedef struct {
    VOS_BOOL            rpAck;   /* is Rp-Ack or Rp-Error */
    MN_MSG_RpCauseUint8 rpCause; /* used when bRpAck==FALSE */
    VOS_UINT8           reserve1[3];
    MN_MSG_RawTsData    tsRawData;
} MN_MSG_SendAckParm;

/* Para of MN_MSG_Write */
typedef struct {
    VOS_UINT32             index;
    MN_MSG_WriteModeUint8  writeMode;
    MN_MSG_MemStoreUint8   memStore;
    MN_MSG_StatusTypeUint8 status;
    VOS_UINT8              reserve1[1];
    MN_MSG_MsgInfo         msgInfo;
} MN_MSG_WriteParm;

/* Para of MN_MSG_Read,MN_MSG_ReadCbm */
typedef struct {
    VOS_UINT32           index;    /* sms memory index */
    MN_MSG_MemStoreUint8 memStore; /* momory store:nvim or usim */
    VOS_UINT8            reserve1[3];
    /* change status or not change status after read or list */
    VOS_BOOL changeFlag;
} MN_MSG_ReadParm;

/* Para of MN_MSG_Delete,MN_MSG_DeleteStaRpt,MN_MSG_DeleteSrvParm,MN_MSG_DeleteCbm */
typedef struct {
    VOS_UINT32             index;      /* sms momory index */
    MN_MSG_MemStoreUint8   memStore;   /* memory store :NVIM or USIM */
    MN_MSG_DeleteTypeUint8 deleteType; /* sms delete type */
    VOS_UINT8              reserve1[2];
} MN_MSG_DeleteParam;

/* Para of MN_MSG_ReadStaRpt,MN_MSG_ReadSrvParm */
typedef struct {
    VOS_UINT32           index;
    MN_MSG_MemStoreUint8 memStore;
    VOS_UINT8            reserve1[3];
} MN_MSG_ReadCommParam;

/* Para of MN_MSG_WriteSrvParm */
typedef struct {
    MN_MSG_MemStoreUint8  memStore;
    MN_MSG_WriteModeUint8 writeMode;
    VOS_UINT8             reserve1[2];
    VOS_UINT32            index;
    MN_MSG_SrvParam       srvParm;
} MN_MSG_WriteSrvParameter;

/* Para of MN_MSG_ModifyStatus */
typedef struct {
    VOS_UINT32             index;    /* sms memory index */
    MN_MSG_StatusTypeUint8 status;   /* sms status */
    MN_MSG_MemStoreUint8   memStore; /* sms memory store:nvim or usim */
    VOS_UINT8              reserve1[2];
} MN_MSG_ModifyStatusParm;

/* Para of MN_MSG_GetRcvMsgPath */
typedef struct {
    VOS_UINT32 reserved;
} MN_MSG_GetRcvmsgPathParm;

/* Para of af_SmsGetStorageStatus */
typedef struct {
    MN_MSG_MemStoreUint8 mem1Store;
    MN_MSG_MemStoreUint8 mem2Store;
    VOS_UINT16           reserved;
} MN_MSG_GetStorageStatusParm;

/* Parm of MN_MSG_GetStaRptFromMoMsg,MN_MSG_GetMoMsgFromStaRpt */
typedef struct {
    VOS_UINT32           index;
    MN_MSG_MemStoreUint8 memStore;
    VOS_UINT8            reserve1[3];
} MN_MSG_StarptMomsgParm;

/* Parm of MN_MSG_SetMemStatus */
typedef struct {
    MN_MSG_MemFlagUint8 memFlag;
    VOS_UINT8           reserve1[3];
} MN_MSG_SetMemstatusParm;

typedef struct {
    MN_MSG_ADDR_NUM_TYPE_T addrType;                               /* bcd number type */
    VOS_UINT8              asciiNum[MN_MAX_ASCII_ADDRESS_NUM + 1]; /* array  of ASCII Num */
    VOS_UINT8              reserve1[2];
} MN_MSG_AppAddr;

/*
 * H2ASN顶级消息结构定义
 */
typedef struct {
    MN_MSG_EventUint32 msgId; /* _H2ASN_MsgChoice_Export NAS_RRC_MSG_TYPE_ENUM_UINT32
                               * 为了兼容NAS的消息头定义，所以转换ASN.1使用NAS_RRC_MSG_TYPE_ENUM_UINT32
                               */

    /*
     * _H2ASN_MsgChoice_When_Comment          NAS_RRC_MSG_TYPE_ENUM_UINT16
     */
} MSG_APP_MsgData;

typedef struct {
    VOS_MSG_HEADER
    MSG_APP_MsgData msgData;
} MSG_APP_interfaceMsg;

/* Set command controls the continuity of SMS relay protocol link. */
VOS_UINT32 MN_MSG_SetLinkCtrl(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_LinkCtrlType *setParam);
/* Set command controls the send domain of MO SMS. */
VOS_UINT32 MN_MSG_SetSmsSendDomain(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                   const MN_MSG_SmsSendDomainType *setParam);

/* Get command controls the continuity of SMS relay protocol link. */
VOS_UINT32 MN_MSG_GetLinkCtrl(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);
/* Get command controls the send domain of MO SMS. */
VOS_UINT32 MN_MSG_GetSendDomain(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

/* Send Sm From Client directly */
VOS_UINT32 MN_MSG_Send(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_SendParm *sendDirectParm);

/* Send Sm From Mem */
VOS_UINT32 MN_MSG_SendFromMem(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                              const MN_MSG_SendFrommemParm *sendFromMemParm);

/* Send Rp-Ack,Rp-Error */
VOS_UINT32 MN_MSG_SendAck(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_SendAckParm *ackParm);

/* Write Sm into USIM Or NVIM */
VOS_UINT32 MN_MSG_Write(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_WriteParm *writeParm);

/* Read Sm */
VOS_UINT32 MN_MSG_Read(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ReadParm *readParm);

/* List Sm */
VOS_UINT32 MN_MSG_List(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ListParm *listParm);

/* Delete Sm */
VOS_UINT32 MN_MSG_Delete(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_DeleteParam *deleteParm);

/* Delete sm test command api */
VOS_UINT32 MN_MSG_Delete_Test(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ListParm *listPara);

/* Modify Sm Status */
VOS_UINT32 MN_MSG_ModifyStatus(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                               const MN_MSG_ModifyStatusParm *modifyParm);

/* Delete Status Report */
VOS_UINT32 MN_MSG_DeleteStaRpt(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_DeleteParam *deleteParm);

/* Write Sm ervice Parameter */
VOS_UINT32 MN_MSG_WriteSrvParam(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                const MN_MSG_WriteSrvParameter *srvParam);

/* Read Sm Service Parameter */
VOS_UINT32 MN_MSG_ReadSrvParam(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_ReadCommParam *readParm);

/* Delete Sm Service Parameter */
VOS_UINT32 MN_MSG_DeleteSrvParam(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_DeleteParam *deleteParm);

/* Set Rcv Path of Sm */
VOS_UINT32 MN_MSG_SetRcvMsgPath(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                const MN_MSG_SetRcvmsgPathParm *rcvPathParm);

/* Get Rcv Path of Sm */
VOS_UINT32 MN_MSG_GetRcvMsgPath(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                const MN_MSG_GetRcvmsgPathParm *rcvPathParm);

/* Get Storage Status of Sm */
VOS_UINT32 MN_MSG_GetStorageStatus(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                   const MN_MSG_GetStorageStatusParm *memParm);
/* Set App Mem Status */
VOS_UINT32 MN_MSG_SetMemStatus(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                               const MN_MSG_SetMemstatusParm *memStatusParm);

/* Encode Sm */
VOS_UINT32 MN_MSG_Encode(const MN_MSG_TsDataInfo *tsDataInfo, MN_MSG_RawTsData *rawData);

/* Decode Sm */
VOS_UINT32 MN_MSG_Decode(const MN_MSG_RawTsData *rawData, MN_MSG_TsDataInfo *tsDataInfo);

/* Segment long Sm */
VOS_UINT32 MN_MSG_Segment(const MN_MSG_SubmitLong *longSubmit, VOS_UINT8 *num, MN_MSG_RawTsData *rawData,
                          VOS_UINT32 maxSmsSegment);

/* Decode Data Coding Scheme */
VOS_UINT32 MN_MSG_DecodeDcs(VOS_UINT8 dcsData, MN_MSG_DcsCode *dcs);

/* Encode Data Coding Scheme */
VOS_UINT32 MN_MSG_EncodeDcs(const MN_MSG_DcsCode *dcs, VOS_UINT8 *dcsData);

/* Decode Relative Time */
VOS_UINT32 MN_MSG_DecodeRelatTime(VOS_UINT8 relatTimeData, MN_MSG_Timestamp *relatTime);

/* Encode Relative Time */
VOS_UINT32 MN_MSG_EncodeRelatTime(const MN_MSG_Timestamp *relatTime, VOS_UINT8 *relatTimeData);

/* 根据TP或RP层地址解析得到ASCII码表示号码的数据结构 */
VOS_UINT32 MN_MSG_DecodeAddress(const VOS_UINT8 *addr, VOS_BOOL bRpAddr, MN_MSG_AsciiAddr *asciiAddr, VOS_UINT32 *len);

/* 根据ASCII码表示号码的数据结构编码得到TP或RP层地址 */
VOS_UINT32 MN_MSG_EncodeAddress(const MN_MSG_AsciiAddr *asciiAddr, VOS_BOOL bRpAddr, VOS_UINT8 *addr, VOS_UINT32 *len);

VOS_UINT32 MN_MSG_EncodeBcdAddress(const MN_MSG_BcdAddr *bcdAddr, VOS_BOOL bRpAddr, VOS_UINT8 *addr, VOS_UINT32 *len);

/* 根据VP和VPF解析得到有效期数据结构 */
VOS_UINT32 MN_MSG_DecodeValidPeriod(MN_MSG_ValidPeriodUint8 vpf, const VOS_UINT8 *validPeriod,
                                    MN_MSG_ValidPeriod *validPeriodInfo, VOS_UINT32 *len);

/* 将MN_MSG_BCD_ADDR_STRU类型地址转换成MN_MSG_ASCII_ADDR_STRU类型地址 */
VOS_UINT32 MN_MSG_BcdAddrToAscii(const MN_MSG_BcdAddr *bcdAddr, MN_MSG_AsciiAddr *asciiAddr);

VOS_UINT32 MN_MSG_MoRetryFlag(VOS_VOID);

VOS_UINT32 MN_MSG_ReqStub(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const MN_MSG_StubMsg *stubParam);

VOS_UINT32 MN_MSG_ChkDate(const MN_MSG_Timestamp *timeStamp, MN_MSG_DateInvalidTypeUint8 *invalidType);

VOS_VOID MN_MSG_GetMoSmsCtrlFlag(VOS_UINT8 *moSmsCtrlFlag);

#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))

/* Get All Cb Mids */
VOS_UINT32 MN_MSG_GetAllCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

/* Add Cb Mids */
VOS_UINT32 MN_MSG_AddCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const TAF_CBA_CbmiRangeList *cbmirList);

/* Delete Cb Mids */
VOS_UINT32 MN_MSG_DelCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId, const TAF_CBA_CbmiRangeList *cbmirList);

/* Decode Cb Msg Page */
VOS_UINT32 MN_MSG_DecodeCbmPage(const MN_MSG_CbrawTsData *cbRawInfo, MN_MSG_Cbpage *cbmPageInfo);

VOS_UINT32 MN_MSG_EmptyCbMids(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

#endif

VOS_VOID MN_MSG_GetMsgMti(VOS_UINT8 fo, VOS_UINT8 *mti);

VOS_UINT32 MN_MSG_GetSubmitInfoForStk(MN_MSG_Tpdu *tpdu, VOS_UINT8 *userDataLen, VOS_UINT8 *udhl,
                                      MN_MSG_MsgCodingUint8 *msgCoding, VOS_UINT32 *dcsOffset);

VOS_UINT32 MSG_SendAppReq(MN_MSG_MsgtypeUint16 msgType, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                          const VOS_VOID *sendParm);


enum MN_MSG_MtCustomize {
    MN_MSG_MT_CUSTOMIZE_NONE,
    MN_MSG_MT_CUSTOMIZE_DCM,
    MN_MSG_MT_CUSTOMIZE_FT,
    MN_MSG_MT_CUSTOMIZE_KT, /* KT定制项目 NV打开的标志 */
    MN_MSG_MT_CUSTOMIZE_BUTT
};
typedef VOS_UINT8 MN_MSG_MtCustomizeUint8;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __MN_MSG_API_H__ */
