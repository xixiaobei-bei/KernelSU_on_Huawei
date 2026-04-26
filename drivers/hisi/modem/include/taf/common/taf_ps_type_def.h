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

#ifndef _PACKET_DATA_DEF_
#define _PACKET_DATA_DEF_

#include "vos.h"
#include "nas_comm_packet_ser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/*
 * TAF下数据业务的消息分配范围: 0 ~ 0x1000, 见文件taf_mntn.h TAF_MntnMsgUint32
 * DSM与APS数据业务模块间的消息分配范围原则限定: 高8位为base，低8位为消息标识
 */
#define TAF_PS_MSG_ID_MASK (0x1F00)

#define TAF_PS_MSG_ID_BASE (0x0000)
#define TAF_PS_APP_PROFILE_ID_BASE (0x0100)
#define TAF_PS_APP_PCF_ID_BASE (0x0200)
#define TAF_PS_IFACE_ID_BASE (0x0400)
#define TAF_PS_APS_INTERNAL_ID_BASE (0x0700)
#define TAF_PS_MNTN_ID_BASE (0x0770)
#define TAF_PS_EVT_ID_BASE (0x0800)
#define TAF_PS_DSM_APS_ID_BASE (0x0B00)
#define TAF_PS_APS_DSM_ID_BASE (0x0C00)

#define TAF_PS_INVALID_SNSSAI (0xffffffff)
/* "a1...a16" */
#define TAF_MAX_PREFIX_NUM_IN_RA (6)

/* "a1.a2.a3.a4 " */
#define TAF_IPV4_ADDR_LEN (4)
#define TAF_MAX_APN_LEN (99)
#define TAF_IPV6_ADDR_LEN (16)
/* "a1.a2.a3.a4.m1.m2.m3.m4 " */
#define TAF_IPV4_ADDR_AND_MASK_LEN 8
#define TAF_MAX_SDF_PF_NUM (16)
/*
 * Maximum length of challenge used in authentication. The maximum length
 * challenge name is the same as challenge.
 */
#define TAF_PS_PPP_CHAP_CHALLNGE_LEN (16)

/*
 * Maximum length of challenge name used in authentication.
 */
#define TAF_PS_PPP_CHAP_CHALLNGE_NAME_LEN (16)

/*
 * Maximum length of response used in authentication.
 */
#define TAF_PS_PPP_CHAP_RESPONSE_LEN (16)

/* AUTHLEN 为100，包含1个字节的Peer-ID Length和99字节的Peer-ID，故此处定义为99 */
#define TAF_MAX_USERNAME_LEN (99)
/* PASSWORDLEN 为100，包含1个字节的Passwd-Length和99字节的Passwd，故此处定义为99 */
#define TAF_MAX_PASSCODE_LEN (99)

/* UE支持的最大的P-CSCF地址个数 */
#define TAF_PCSCF_ADDR_MAX_NUM (8)

#define TAF_PS_CAUSE_APS_SECTION_BEGIN (0x0000)
#define TAF_PS_CAUSE_DSM_SECTION_BEGIN (0x0040)
#define TAF_PS_CAUSE_SM_SECTION_BEGIN (0x0080)
#define TAF_PS_CAUSE_SM_NW_SECTION_BEGIN (0x0100)
#define TAF_PS_CAUSE_SM_NW_SECTION_END (0x01FF)
#define TAF_PS_CAUSE_GMM_SECTION_BEGIN (0x0200)
#define TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN (0x0300)
#define TAF_PS_CAUSE_XCC_SECTION_BEGIN (0x0400)
#define TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN (0x0500)
#define TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN (0x0600)
#define TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN (0X0700)
#define TAF_PS_CAUSE_PPPC_SECTION_BEGIN (0X0900)
#define TAF_PS_CAUSE_MIP_SECTION_BEGIN (0X0A00)

/* 偏移量0X0800已被占用，ril侧将对0 ~ 255的内部原因值往后偏移0X0800 */
#define TAF_PS_CAUSE_EMM_SECTION_BEGIN (0X0B00)
#define TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN (0X0C00)

#define TAF_PS_CAUSE_WLAN_SECTION_BEGIN (0X0D00)

#define TAF_PS_CAUSE_NRSM_SECTION_BEGIN (0X0E00)
#define TAF_PS_CAUSE_NRMM_SECTION_BEGIN (0X0E80)
#define TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN (0X0F00)
#define TAF_PS_CAUSE_CDS_SECTION_BEGIN (0X1000)
#define TAF_PS_CAUSE_SDAP_SECTION_BEGIN (0X1100)

/* ^AUTHDATA */
#define TAF_MAX_AUTHDATA_USERNAME_LEN (99)
#define TAF_MAX_AUTHDATA_PASSWORD_LEN (99)
#define TAF_MAX_AUTHDATA_PLMN_LEN (6)

#define TAF_CGACT_DEACT (0)
#define TAF_CGACT_ACT (1)

/* 目前先放在这里，等后面APS代码删除调整之后，更名 */
#define TAF_PS_PPP_IP_ADDR_ID 0x03
#define TAF_PS_PPP_PRI_DNS_ID 0x81
#define TAF_PS_PPP_PRI_NBNS_ID 0x82
#define TAF_PS_PPP_SEC_DNS_ID 0x83
#define TAF_PS_PPP_SEC_NBNS_ID 0x84

/* PCO头的长度 */
#define TAF_PS_PCO_CONFIG_HDR_LEN (4)

/* IP地址长度 */
#define TAF_PS_PPP_IP_ADDR_LEN (6)

/* 主NBNS地址长度 */
#define TAF_PS_PPP_PRI_NBNS_LEN (6)

/* 主DNS地址长度 */
#define TAF_PS_PPP_PRI_DNS_LEN (6)

/* 副NBNS地址长度 */
#define TAF_PS_PPP_SEC_NBNS_LEN (6)

/* 副NBNS地址长度 */
#define TAF_PS_PPP_SEC_DNS_LEN (6)

/*
 * AUTH_PAP (RFC 1334)
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Code      |  Identifier   |            Length             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Peer-ID Length|  Peer-Id ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+
 * | Passwd-Length |  Password ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
/* code(1B)+id(1B)+length(2B)+username_size(1B)+username+password_size(1B)+password */
#define TAF_PS_PPP_PAP_CODE_LEN 1

#define TAF_PS_PPP_PAP_HEAD_OFFSET 4

/* code(1B)+id(1B)+length(2B)+challenge_size(1B)+challenge+name */
/* code(1B)+id(1B)+length(2B)+response_size(1B)+response+name */
#define TAF_PS_PPP_CHAP_CODE_LEN 1
#define TAF_PS_PPP_CHAP_ID_LEN 1
#define TAF_PS_PPP_CHAP_LEN 2
#define TAF_PS_PPP_CHAP_CHALLENGE_SIZE 1
#define TAF_PS_PPP_CHAP_RESPONSE_SIZE 1

#define TAF_PS_PPP_CHAP_HEAD_OFFSET (TAF_PS_PPP_CHAP_CODE_LEN + TAF_PS_PPP_CHAP_ID_LEN + TAF_PS_PPP_CHAP_LEN)

#define TAF_PS_PPP_CHAP_CHALLENGE_HEAD_OFFSET (TAF_PS_PPP_CHAP_HEAD_OFFSET + TAF_PS_PPP_CHAP_CHALLENGE_SIZE)
#define TAF_PS_PPP_CHAP_RESPONSE_HEAD_OFFSET (TAF_PS_PPP_CHAP_HEAD_OFFSET + TAF_PS_PPP_CHAP_RESPONSE_SIZE)

/*
 * IPCP OPTION长度必须等于6
 * OPTION TYPE(1 byte) + OPTION LEN(1 btye) + VALUE(4 bytes)
 */
#define TAF_PS_PPP_IPCP_OPTION_TYPE_LEN 1
#define TAF_PS_PPP_IPCP_OPTION_LEN 1

#define TAF_PS_PPP_IPCP_OPTION_HEAD_LEN (TAF_PS_PPP_IPCP_OPTION_TYPE_LEN + TAF_PS_PPP_IPCP_OPTION_LEN)

#define TAF_PS_PPP_IPCP_OPTION_HEAD_LEN_OFFSET (TAF_PS_PPP_IPCP_OPTION_HEAD_LEN + TAF_PS_PPP_IPCP_OPTION_HEAD_LEN)

#define TAF_PS_PPP_CODE_REQ 1
#define TAF_PS_PPP_CODE_ACK 2
#define TAF_PS_PPP_CODE_NAK 3
#define TAF_PS_PPP_CODE_REJ 4

#define TAF_PS_INVALID_CID (0xFF) /* 无效的CID */

#define TAF_PS_3GPP_QOS_MEANTHROUGH_TOP 18
#define TAF_PS_3GPP_QOS_MEANTHROUGH_BESTEFFORT 31
#define TAF_PS_3GPP_QOS_MEANTHROUGH_BOT 0

/* 终端可配置需求:2G拨号请求禁止Spare_bit3等参数 */
#define TAF_PS_2G_BIT3_DISABLE 0 /* 2G拨号时PDP激活消息携带Spare_bit3等参数  */
#define TAF_PS_2G_BIT3_ENABLE 1  /* 2G拨号时PDP激活请求消息不携带bit3等参数  */

#define TAF_PS_INVALID_PDU_SESSION_ID (0)

#define TAF_PS_INVALID_IFACE_ID (0)

enum TAF_PDP_RequestType {
    TAF_PDP_REQUEST_TYPE_INITIAL   = 0x1,
    TAF_PDP_REQUEST_TYPE_HANDOVER  = 0x2,
    TAF_PDP_REQUEST_TYPE_UNUSED    = 0x3,
    TAF_PDP_REQUEST_TYPE_EMERGENCY = 0x4,
    TAF_PDP_REQUEST_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PDP_RequestTypeUint8;

/*
 * 枚举说明: 鉴权类型
 */
enum TAF_PDP_AuthType {
    TAF_PDP_AUTH_TYPE_NONE = 0x00,
    TAF_PDP_AUTH_TYPE_PAP  = 0x01,
    TAF_PDP_AUTH_TYPE_CHAP = 0x02,

    TAF_PDP_AUTH_TYPE_PAP_OR_CHAP = 0x03,

    TAF_PDP_AUTH_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PDP_AuthTypeUint8;


enum TAF_PDP_HComp {
    TAF_PDP_H_COMP_OFF     = 0x00, /* default if value is omitted */
    TAF_PDP_H_COMP_ON      = 0x01, /* manufacturer preferred compression */
    TAF_PDP_H_COMP_RFC1144 = 0x02, /* RFC 1144 */
    TAF_PDP_H_COMP_RFC2507 = 0x03, /* RFC 2507 */

    TAF_PDP_H_COMP_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_HCompUint8;


enum TAF_PDP_DComp {
    TAF_PDP_D_COMP_OFF = 0x00, /* default if value is omitted */
    TAF_PDP_D_COMP_ON  = 0x01, /* manufacturer preferred compression */
    TAF_PDP_D_COMP_V42 = 0x02, /* V.42 compression */

    TAF_PDP_D_COMP_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_DCompUint8;


enum TAF_PDP_Ipv4AddrAllocType {
    TAF_PDP_IPV4_ADDR_ALLOC_TYPE_NAS  = 0x00,
    TAF_PDP_IPV4_ADDR_ALLOC_TYPE_DHCP = 0x01,

    TAF_PDP_IPV4_ADDR_ALLOC_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_Ipv4AddrAllocTypeUint8;


enum TAF_PDP_EmcInd {
    TAF_PDP_NOT_FOR_EMC = 0x00,
    TAF_PDP_FOR_EMC     = 0x01,

    TAF_PDP_EMC_IND_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_EmcIndUint8;


enum TAF_PDP_PcscfDiscovery {
    TAF_PDP_PCSCF_DISCOVERY_NOT_INFLUENCED  = 0x00,
    TAF_PDP_PCSCF_DISCOVERY_THROUGH_NAS_SIG = 0x01,
    TAF_PDP_PCSCF_DISCOVERY_THROUGH_DHCP    = 0x02,

    TAF_PDP_PCSCF_DISCOVERY_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_PcscfDiscoveryUint8;


enum TAF_PDP_ImCnSigFlag {
    TAF_PDP_NOT_FOR_IMS_CN_SIG_ONLY = 0x00,
    TAF_PDP_FOR_IM_CN_SIG_ONLY      = 0x01,

    TAF_PDP_IM_CN_SIG_FLAG_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_ImCnSigFlagUint8;

/*
 * 结构说明:
 */
enum TAF_NvPdpNspli {
    TAF_NV_PDP_NAS_SIG_NORMAL_PRIO = 0x00,
    TAF_NV_PDP_NAS_SIG_LOW_PRIO    = 0x01,

    TAF_NV_PDP_NAS_SIG_PRIO_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_NvPdpNspliUint8;

enum TAF_PDP_NasSigPrioInd {
    TAF_PDP_NAS_SIG_LOW_PRIO    = 0x00,
    TAF_PDP_NAS_SIG_NORMAL_PRIO = 0x01,

    TAF_PDP_NAS_SIG_PRIO_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_NasSigPrioIndUint8;


enum TAF_PS_RatType {
    TAF_PS_RAT_TYPE_NULL,  /* 当前未在任何网络上驻留 */
    TAF_PS_RAT_TYPE_GSM,   /* GSM接入技术 */
    TAF_PS_RAT_TYPE_WCDMA, /* WCDMA接入技术 */
    TAF_PS_RAT_TYPE_LTE,   /* LTE接入技术 */
    TAF_PS_RAT_TYPE_1X,    /* CDMA-1X接入技术 */
    TAF_PS_RAT_TYPE_HRPD,  /* CDMA-HRPD接入技术 */
    TAF_PS_RAT_TYPE_EHRPD, /* CDMA-EHRPD接入技术 */
    TAF_PS_RAT_TYPE_NR,    /* NR核心网接入技术 */
    TAF_PS_RAT_TYPE_BUTT   /* 无效的接入技术 */
};
typedef VOS_UINT32 TAF_PS_RatTypeUint32;


enum TAF_PDP_AnswerMode {
    TAF_PDP_ANSWER_MODE_MANUAL = 0x00, /* 人工应答方式 */
    TAF_PDP_ANSWER_MODE_AUTO   = 0x01, /* 自动应答方式 */

    TAF_PDP_ANSWER_MODE_BUTT
};
typedef VOS_UINT8 TAF_PDP_AnswerModeUint8;


enum TAF_PDP_AnswerType {
    TAF_PDP_ANSWER_TYPE_ACCEPT = 0x00, /* 接受呼叫 */
    TAF_PDP_ANSWER_TYPE_REJECT = 0x01, /* 拒绝呼叫 */

    TAF_PDP_ANSWER_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PDP_AnswerTypeUint8;


enum TAF_PS_CdataDialMode {
    TAF_PS_CDATA_DIAL_MODE_ASYNC_CALL = 0x00,
    TAF_PS_CDATA_DIAL_MODE_RELAY      = 0x01, /* 数据传输模式中继模式 */
    TAF_PS_CDATA_DIAL_MODE_NETWORK    = 0x02, /* 数据传输模式网络模式  */
    TAF_PS_CDATA_DIAL_MODE_BUTT
};
typedef VOS_UINT32 TAF_PS_CdataDialModeUint32;

/*
 * 枚举说明: 鉴权类型
 */
enum TAF_PDP_DialType {
    TAF_PDP_DAIL_TYPE_NDISDUP = 0x00,
    TAF_PDP_DAIL_TYPE_CGACT   = 0x01,
    TAF_PDP_DAIL_TYPE_PPP     = 0x02,

    TAF_PDP_DAIL_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PDP_DialTypeUint8;


enum TAF_PS_Cause {
    /*
     * TAF上报的内部原因值, 取值范围[0x0000, 0x007F]
     */

    /* 说明：成功 */
    TAF_PS_CAUSE_SUCCESS                    = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 0),

    /* 说明：参数错误 */
    TAF_PS_CAUSE_INVALID_PARAMETER          = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 1),

    /* 说明：CID错误 */
    TAF_PS_CAUSE_CID_INVALID                = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 2),

    /* 说明：PDP激活超过最大次数(定制) */
    TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 3),

    /* 说明：SIM卡无效 */
    TAF_PS_CAUSE_SIM_INVALID                = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 4),

    /* 说明：APS内部定时器超时 */
    TAF_PS_CAUSE_APS_TIME_OUT               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 5),

    /* 说明：操作冲突（建议重拨） */
    TAF_PS_CAUSE_OPERATION_CONFLICT         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_XCC_TIME_OUT               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 7),

    /* 说明：等待PPP回复超时 */
    TAF_PS_CAUSE_PPP_TIME_OUT               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 8),

    /* 说明：PPP协商失败 */
    TAF_PS_CAUSE_PPP_NEGOTIATE_FAIL         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_1X_NO_SERVICE              = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_HRPD_NO_SERVICE            = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_HSM_TIME_OUT               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 12),

    /* 说明：PPP重协商获取IP地址 */
    TAF_PS_CAUSE_IP_ADDRESS_CHANGE          = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_EHSM_TIME_OUT              = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 14),

    /* 说明：接入模式改变 */
    TAF_PS_CAUSE_RAT_TYPE_CHANGE            = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 15),

    /* 说明：DHCP模块协商超时 */
    TAF_PS_CAUSE_DHCP_TIME_OUT              = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 16),

    /* 说明：当前只允许IPV4类型 */
    TAF_PS_CAUSE_PDP_TYPE_IPV4_ONLY_ALLOWED = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 17),

    /* 说明：当前只允许IPV6类型 */
    TAF_PS_CAUSE_PDP_TYPE_IPV6_ONLY_ALLOWED = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 18),

    /* 说明：网络测发起PPP去激活 */
    TAF_PS_CAUSE_PPP_NW_DISC = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 19),

    /* 说明：不支持PCSCF */
    TAF_PS_CAUSE_UNSUPPORT_PCSCF = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 20),

    /* 说明：系统模式改变 */
    TAF_PS_CAUSE_SYSCFG_MODE_CHANGE = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 21),

    /* 说明：L2C切换失败 */
    TAF_PS_CAUSE_L2C_HANDOVER_FAIL = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 22),

    /* 说明：不支持同时连接 */
    TAF_PS_CAUSE_NOT_SUPPORT_CONCURRENT = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 23),

    /* 说明：读取3GPD文件失败 */
    TAF_PS_CAUSE_READ_3GPD_FILE         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_1X_HRPD_NOT_SUPPORT_MUTI_PDN   = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 25),

    /* 说明：BACKOFF算法不允许激活 */
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED        = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 26),
    TAF_PS_CAUSE_1X_DO_NO_SERVICE               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 27),

    /* 说明：由于网络无响应BACKOFF算法不允许激活 */
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_IGNORE = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 28),

    /* 说明：由于永久拒绝原因值BACKOFF算法不允许激活 */
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_PERM   = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 29),

    /* 说明：由于临时拒绝原因值BACKOFF算法不允许激活 */
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_TEMP   = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 30),

    /* 说明：BACKOFF算法拒绝保留原因值（现在不使用） */
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_RSV    = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 31),

    /* 说明：IP类型改变 */
    TAF_PS_CAUSE_IP_TYPE_CHANGE = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 32),

    /* 说明：MIP特性未使能 */
    TAF_PS_CAUSE_MIP_DISABLE = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 33),

    /* 说明：IPV6地址不可用 */
    TAF_PS_CAUSE_IPV6_ADDRESS_DISABLE  = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 34),

    /* 说明：不支持的操作 */
    TAF_PS_CAUSE_NOT_SUPPROT_OPERATION = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 35),

    /* 说明：调节算法不允许 */
    TAF_PS_CAUSE_THROT_ALG_NOT_ALLOWED = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 36),

    /* 说明：异系统切换到LTE */
    TAF_PS_CAUSE_SYSTEM_CHG_TO_LTE        = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 37),
    TAF_PS_CAUSE_SYSTEM_CHG_TO_EHRPD      = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 38),

    /* 说明：Data Off导致数据业务去激活 */
    TAF_PS_CAUSE_INTERNAL_DATA_SWITCH_OFF = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 39),

    /* 说明：没有可用的系统域（CELLULAR和WLAN都不可用） */
    TAF_PS_CAUSE_NO_AVAILABLE_DOMAIN        = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 40),

    /* 说明：CELLULAR和WLAN之间HANDOVER过程中IP地址改变 */
    TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 41),

    /* 说明：LTE切换到NR时失败 */
    TAF_PS_CAUSE_L2NR_HANDOVER_FAIL = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 42),

    /* 说明：NR切换到LTE时失败 */
    TAF_PS_CAUSE_NR2L_HANDOVER_FAIL = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 43),

    /* 说明：IPv6前缀地址分配失败 */
    TAF_PS_CAUSE_IPV6_ADDR_ALLOC_FAIL   = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 44),

    /* 说明：IPv6前缀地址刷新失败 */
    TAF_PS_CAUSE_IPV6_ADDR_REFRESH_FAIL = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 45),

    /* 这个并不是sm给aps的原因值，而是aps的内部原因值，为了复用内部去激活消息添加 */

    /* 说明：内部的DATA OFF */
    TAF_PS_CAUSE_INTERNAL_DATA_OFF    = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 46),

    /* 说明：APP本地去激活 */
    TAF_PS_CAUSE_APP_LOCAL_DEACTIVATE = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 47),

    /* 说明：存在未指定编码的QFI或者QRI */
    TAF_PS_CAUSE_EXIST_UNASSIGNED_QFI_OR_QRI = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 48),

    /* 说明：未知错误 */
    TAF_PS_CAUSE_UNKNOWN = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 63),

    /* 说明：参数错误 */
    TAF_PS_CAUSE_DSM_INVALID_PARAMETER            = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 1),

    /* 说明：CID错误 */
    TAF_PS_CAUSE_DSM_CID_INVALID                  = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 2),

    /* 说明：DSM内部定时器超时 */
    TAF_PS_CAUSE_DSM_TIME_OUT                     = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 3),

    /* 说明：操作冲突（建议重拨） */
    TAF_PS_CAUSE_DSM_OPERATION_CONFLICT           = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 4),

    /* 说明：MIP特性未使能 */
    TAF_PS_CAUSE_DSM_MIP_DISABLE                  = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 5),

    /* 说明：关机状态 */
    TAF_PS_CAUSE_DSM_POWER_OFF                    = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 6),

    /* 说明：DSM申请GROUP实体失败 */
    TAF_PS_CAUSE_DSM_GROUP_ENTITY_ALLOC_FAIL      = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 7),

    /* 说明：DSM申请Pdn实体失败 */
    TAF_PS_CAUSE_DSM_PDN_ENTITY_ALLOC_FAIL        = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 8),

    /* 说明：DSM申请Bearer实体失败 */
    TAF_PS_CAUSE_DSM_BEARER_ENTITY_ALLOC_FAIL     = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 9),

    /* 说明：打断了DSM Bearer承载流程 */
    TAF_PS_CAUSE_DSM_BREAK_BEARER_PROCEDURE       = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 10),

    /* 说明：Iface Id异常 */
    TAF_PS_CAUSE_DSM_IFACEID_INVALID              = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 11),

    /* 说明：拨号用户类型冲突 */
    TAF_PS_CAUSE_DSM_USER_TYPE_INCONSISTENT       = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 12),

    /* 说明：DSM申请Iface实体失败 */
    TAF_PS_CAUSE_DSM_IFACE_ENTITY_ALLOC_FAIL      = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 13),

    /* 说明：DSM发送消息失败 */
    TAF_PS_CAUSE_DSM_SEND_MSG_FAIL                = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 14),

    /* 说明：UE Policy信息正在上报中 */
    TAF_PS_CAUSE_DSM_UE_POLICY_IS_REPORTING       = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 15),

    /* 说明：Iface ID分配失败 */
    TAF_PS_CAUSE_DSM_IFACEID_ALLOC_FAIL           = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 16),

    /* 说明：拨号类型不匹配 */
    TAF_PS_CAUSE_DSM_DAIL_TYPE_UNMATCH            = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 17),

    /* 说明：端口不匹配 */
    TAF_PS_CAUSE_DSM_CLIENT_ID_UNMATCH            = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_DSM_CID_USED_BY_DEDICATED_BEARER = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_DSM_ACT_DEDICATED_BEARER_CONFLICT = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 20),

    /* 说明：未知错误 */
    TAF_PS_CAUSE_DSM_UNKNOWN = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 63),

    /*
     * TAF上报的SM内部原因值, 取值范围[0x0080, 0x00FF]
     */

    /* 说明：建链超时错误 */
    TAF_PS_CAUSE_SM_CONN_ESTABLISH_MAX_TIME_OUT         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 1),

    /* 说明：超时错误 */
    TAF_PS_CAUSE_SM_MAX_TIME_OUT                        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 2),

    /* 说明：错误的NSAPI */
    TAF_PS_CAUSE_SM_INVALID_NSAPI                       = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 3),

    /* 说明：PDP修改流程冲突 */
    TAF_PS_CAUSE_SM_MODIFY_COLLISION                    = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 4),

    /* 说明：PDP重复激活 */
    TAF_PS_CAUSE_SM_DUPLICATE                           = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 5),

    /* 说明：RAB建立失败 */
    TAF_PS_CAUSE_SM_RAB_SETUP_FAILURE                   = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 6),

    /* 说明：SGSN不支持GPRS */
    TAF_PS_CAUSE_SM_SGSN_VER_PRE_R99                    = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 7),

    /* 说明：ESM返回失败 */
    TAF_PS_CAUSE_SM_FAILURE                             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 8),

    /* 说明：ESM返回未指定错误 */
    TAF_PS_CAUSE_SM_ERR_UNSPECIFIED_ERROR               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 9),

    /* 说明：ESM返回资源不足 */
    TAF_PS_CAUSE_SM_ERR_INSUFFICIENT_RESOURCES          = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 10),

    /* 说明：ESM返回该CID无效 */
    TAF_PS_CAUSE_SM_CALL_CID_INVALID                    = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 11),

    /* 说明：ESM返回该CID已经激活 */
    TAF_PS_CAUSE_SM_CALL_CID_ACTIVE                     = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 12),

    /* 说明：ESM返回该CID与承载不匹配 */
    TAF_PS_CAUSE_SM_CALL_CID_NOT_MATCH_BEARER           = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 13),

    /* 说明：ESM返回该承载类型不为专有承载 */
    TAF_PS_CAUSE_SM_BEARER_TYPE_NOT_DEDICATED           = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 14),

    /* 说明：ESM返回承载未激活 */
    TAF_PS_CAUSE_SM_BEARER_INACTIVE                     = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 15),

    /* 说明：ESM返回相关联的CID无效 */
    TAF_PS_CAUSE_SM_LINK_CID_INVALID                    = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 16),

    /* 说明：ESM返回相关联的承载未激活 */
    TAF_PS_CAUSE_SM_LINK_BEARER_INACTIVE                = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 17),

    /* 说明：ESM返回APN长度异常 */
    TAF_PS_CAUSE_SM_APN_LEN_ILLEGAL                     = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 18),

    /* 说明：ESM返回APN异常 */
    TAF_PS_CAUSE_SM_APN_SYNTACTICAL_ERROR               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 19),

    /* 说明：ESM返回Auth类型非法 */
    TAF_PS_CAUSE_SM_AUTH_TYPE_ILLEGAL                   = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 20),

    /* 说明：ESM返回用户名长度过长 */
    TAF_PS_CAUSE_SM_USER_NAME_TOO_LONG                  = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 21),

    /* 说明：ESM返回用户密码过长 */
    TAF_PS_CAUSE_SM_USER_PASSWORD_TOO_LONG              = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 22),

    /* 说明：ESM返回Access码过长 */
    TAF_PS_CAUSE_SM_ACCESS_NUM_TOO_LONG                 = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 23),

    /* 说明：ESM返回该CID正在操作流程中 */
    TAF_PS_CAUSE_SM_CALL_CID_IN_OPERATION               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 24),

    /* 说明：ESM返回该承载类型不是缺省承载 */
    TAF_PS_CAUSE_SM_BEARER_TYPE_NOT_DEFAULT             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 25),

    /* 说明：ESM返回该承载类型是非法的 */
    TAF_PS_CAUSE_SM_BEARER_TYPE_ILLEGAL                 = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 26),

    /* 说明：ESM返回不存在缺省承载类型的CID */
    TAF_PS_CAUSE_SM_MUST_EXIST_DEFAULT_TYPE_CID         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 27),

    /* 说明：ESM返回PDN类型非法 */
    TAF_PS_CAUSE_SM_PDN_TYPE_ILLEGAL                    = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 28),

    /* 说明：ESM返回IPV4地址分配类型异常 */
    TAF_PS_CAUSE_SM_IPV4_ADDR_ALLOC_TYPE_ILLEGAL        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 29),

    /* 说明：ESM返回L模被挂起了 */
    TAF_PS_CAUSE_SM_SUSPENDED                           = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 30),

    /* 说明：ESM返回不允许多个紧急承载 */
    TAF_PS_CAUSE_SM_MULTI_EMERGENCY_NOT_ALLOWED         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 31),

    /* 说明：ESM返回不允许发起不是紧急承载的激活请求 */
    TAF_PS_CAUSE_SM_NON_EMERGENCY_NOT_ALLOWED           = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 32),

    /* 说明：ESM返回不允许修改紧急承载 */
    TAF_PS_CAUSE_SM_MODIFY_EMERGENCY_NOT_ALLOWED        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 33),

    /* 说明：ESM返回不允许该专有承载的主承载为紧急承载 */
    TAF_PS_CAUSE_SM_DEDICATED_FOR_EMERGENCY_NOT_ALLOWED = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 34),

    /* 说明：ESM返回BACKOFF算法不允许发起激活请求 */
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 35),

    /* 说明：ESM返回由于网络无响应BACKOFF算法不允许发起激活请求 */
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_IGNORE      = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 36),

    /* 说明：ESM返回由于永久拒绝原因值BACKOFF算法不允许发起激活请求 */
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_PERM        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 37),

    /* 说明：ESM返回由于临时拒绝原因值BACKOFF算法不允许发起激活请求 */
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_TEMP        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 38),

    /* 说明：ESM返回BACKOFF算法不允许发起激活请求保留原因值 */
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_RSV         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 39),

    /* 说明：ESM返回与MM不同步 */
    TAF_PS_CAUSE_SM_MM_NOT_SYNC                         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 40),

    /* 说明：ESM返回网络重配导致承载释放 */
    TAF_PS_CAUSE_SM_NW_RECONFIGURE                      = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 41),

    /* 说明：ESM 返回add或creat PF空口携带的预添加PF在承载下存在，且存在的pf所属的SDF与用户预操作的不一致 */
    TAF_PS_CAUSE_SM_DELETE_SDF                          = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 42),

    /* 说明：ESM返回APN不在允许激活列表中 */
    TAF_PS_CAUSE_SM_NOT_IN_ACL_LIST          = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 43),

    /* 说明：ESM返回该APN未启动 */
    TAF_PS_CAUSE_SM_APN_DISABLE              = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 44),

    /* 说明：ESM返回不允许申请承载实体 */
    TAF_PS_CAUSE_SM_BEARER_ALLOC_NOT_ALLOWED = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 45),

    /* 说明：ESM返回调节算法不允许 */
    TAF_PS_CAUSE_SM_THROT_ALG_NOT_ALLOWED    = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 46),

    /* 说明：ESM返回T3396正在运行中 */
    TAF_PS_CAUSE_SM_THROT_T3396_IS_RUNNING   = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 47),

    /* 说明：ESM返回BACKOFF算法在惩罚过程中 */
    TAF_PS_CAUSE_SM_THROT_BACKOFF_IS_RUNNING = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 48),

    /* 说明：ESM上报NR下的QoS flow映射失败 */
    TAF_PS_CAUSE_SM_NR_QOS_FLOW_MAP_FAIL     = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 49),

    /* 说明：ESM 返回需要进行重拨处理 */
    TAF_PS_CAUSE_ERR_SM_REDIAL = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 50),

    TAF_PS_CAUSE_SM_MAX_TIME_OUT_LOCAL = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 51),

    /* 说明：ESM返回未知错误原因值 */
    TAF_PS_CAUSE_SM_UNKNOWN = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 127),

    /*
     * TAF上报的SM网络原因值, 取值范围[0x0100, 0x01FF]
     * 由于3GPP协议已经定义了具体的(E)SM网络原因值, (E)SM上报的取值为协议
     * 定义的原因值加上偏移量(0x100)
     * (1) SM Cause : Refer to TS 24.008 section 10.5.6.6
     * (2) ESM Cause: Refer to TS 24.301 section 9.9.4.4
     */

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_OPERATOR_DETERMINED_BARRING               = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 8),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6章节”。 */
    TAF_PS_CAUSE_SM_NW_MBMS_BC_INSUFFICIENT                      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 24),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6章节”。 */
    TAF_PS_CAUSE_SM_NW_LLC_OR_SNDCP_FAILURE                      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 25),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_INSUFFICIENT_RESOURCES                    = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 26),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_MISSING_OR_UKNOWN_APN                     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 27),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_UNKNOWN_PDP_ADDR_OR_TYPE                  = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 28),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_USER_AUTHENTICATION_FAIL                  = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 29),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_ACTIVATION_REJECTED_BY_GGSN_SGW_OR_PGW    = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 30),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_ACTIVATION_REJECTED_UNSPECIFIED           = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 31),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_SERVICE_OPTION_NOT_SUPPORTED              = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 32),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_REQUESTED_SERVICE_NOT_SUBSCRIBED          = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 33),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_SERVICE_OPTION_TEMP_OUT_ORDER             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 34),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_NSAPI_ALREADY_USED                        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 35),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_REGULAR_DEACTIVATION                      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 36),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_QOS_NOT_ACCEPTED                          = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 37),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_NETWORK_FAILURE                           = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 38),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_REACTIVATION_REQUESTED                    = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 39),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6章节”。 */
    TAF_PS_CAUSE_SM_NW_FEATURE_NOT_SUPPORT                       = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 40),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_SEMANTIC_ERR_IN_TFT                       = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 41),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_SYNTACTIC_ERR_IN_TFT                      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 42),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_UKNOWN_PDP_CONTEXT                        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 43),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_SEMANTIC_ERR_IN_PACKET_FILTER             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 44),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_SYNCTACTIC_ERR_IN_PACKET_FILTER           = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 45),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PDP_CONTEXT_WITHOUT_TFT_ACTIVATED         = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 46),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_MULTICAST_GROUP_MEMBERHHSHIP_TIMEOUT      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 47),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6章节”。 */
    TAF_PS_CAUSE_SM_NW_REQUEST_REJECTED_BCM_VIOLATION            = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 48),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_LAST_PDN_DISCONN_NOT_ALLOWED              = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 49),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV4_ONLY_ALLOWED                = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 50),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV6_ONLY_ALLOWED                = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 51),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_SINGLE_ADDR_BEARERS_ONLY_ALLOWED          = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 52),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_INFORMATION_NOT_RECEIVED                  = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 53),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PDN_CONNECTION_DOES_NOT_EXIST             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 54),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_SAME_APN_MULTI_PDN_CONNECTION_NOT_ALLOWED = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 55),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_COLLISION_WITH_NW_INITIATED_REQUEST       = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 56),

    /* 说明：标准协议值偏移256，请参见“24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV4V6_ONLY_ALLOWED              = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 57),

    /* 说明：标准协议值偏移256，请参见“24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PDP_TYPE_UNSTRUCTURED_ONLY_ALLOWED        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 58),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_UNSUPPORTED_QCI_VALUE                     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 59),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6章节”。 */
    TAF_PS_CAUSE_SM_NW_BEARER_HANDLING_NOT_SUPPORTED             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 60),

    /* 说明：标准协议值偏移256，请参见“24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PDP_TYPE_ETHERNET_ONLY_ALLOWED            = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 61),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.008 10.5.6.6 章节”。 */
    TAF_PS_CAUSE_SM_NW_MAXIMUM_NUMBER_OF_EPS_BEARERS_REACHED   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 65),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.008 10.5.6.6 章节”。 */
    TAF_PS_CAUSE_SM_NW_APN_NOT_SUPPORT_IN_CURRENT_RAT_AND_PLMN = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 66),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_INSUFFICIENT_RESOURCES_FOR_SPC_SLICE_AND_DNN   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 67),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_NOT_SUPPORTED_SSC_MODE                         = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 68),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_INSUFFICIENT_RESOURCES_FOR_SPC_SLICE           = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 69),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_MISSING_OR_UNKNOWN_DNN_IN_A_SLICE              = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 70),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_INVALID_TI                                     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 81),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_MAX_DATA_RATE_FOR_INTEGRITY_PROTECTION_TOO_LOW = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 82),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_SEMANTIC_ERROR_IN_THE_QOS_OPERATION            = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 83),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_SYNTACTICAL_ERROR_IN_THE_QOS_OPERATION         = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 84),

    /* 说明：标准协议值偏移 256，请参见“3GPP TS 24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_INVALID_MAPPED_EPS_BEARER_IDENTITY             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 85),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_SEMANTICALLY_INCORRECT_MESSAGE                 = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 95),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_INVALID_MANDATORY_INFO                         = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 96),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_MSG_TYPE_NON_EXISTENT                          = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 97),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_MSG_TYPE_NOT_COMPATIBLE                        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 98),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_IE_NON_EXISTENT                                = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 99),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_CONDITIONAL_IE_ERR                             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 100),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_MSG_NOT_COMPATIBLE                             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 101),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4/24.501 9.11.4.2章节”。 */
    TAF_PS_CAUSE_SM_NW_PROTOCOL_ERR_UNSPECIFIED                       = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 111),

    /* 说明：标准协议值偏移256，请参见“3GPP TS 24.008 10.5.6.6/24.301 9.9.4.4章节”。 */
    TAF_PS_CAUSE_SM_NW_APN_RESTRICTION_INCOMPATIBLE                   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 112),

    /*
     * TAF上报的GMM内部原因值, 取值范围[0x0200, 0x02FF]
     */

    /* 说明：不支持GPRS */
    TAF_PS_CAUSE_GMM_GPRS_NOT_SUPPORT    = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 1),

    /* 说明：当前驻留在禁止小区 */
    TAF_PS_CAUSE_GMM_FORBID_LA           = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 2),

    /* 说明：鉴权失败 */
    TAF_PS_CAUSE_GMM_AUTHENTICATION_FAIL = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 3),

    /* 说明：PS域去附着 */
    TAF_PS_CAUSE_GMM_PS_DETACH           = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 4),

    /* 说明：接入禁止 */
    TAF_PS_CAUSE_GMM_ACCESS_BARRED       = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 5),

    /* 说明：网络侧无此PDP上下文 */
    TAF_PS_CAUSE_GMM_NO_PDP_CONTEXT      = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 6),

    /* 说明：ATTACH达到最大次数 */
    TAF_PS_CAUSE_GMM_ATTACH_MAX_TIMES                     = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 7),

    /* 说明：RRC建链失败 */
    TAF_PS_CAUSE_GMM_RRC_EST_FAIL                         = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 8),

    /* 说明：T3310定时器超时 */
    TAF_PS_CAUSE_GMM_T3310_EXPIRED                        = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 9),

    /* 说明：T3317定时器超时 */
    TAF_PS_CAUSE_GMM_T3317_EXPIRED                        = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 10),

    /* 说明：信令建立保护定时器超时 */
    TAF_PS_CAUSE_GMM_TIMER_SIGNALING_PROTECT_EXPIRED      = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 11),

    /* 说明：GMM注册失败 */
    TAF_PS_CAUSE_GMM_NULL                                 = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 12),

    /* 说明：去注册没有重新注册 */
    TAF_PS_CAUSE_GMM_DETACH_NOT_REATTACH                  = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 13),

    /* 说明：RPM禁止注册 */
    TAF_PS_CAUSE_GMM_RPM_FORBID_ATTACH                    = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 14),

    /* 说明：PS业务在特定GMM状态下不触发注册 */
    TAF_PS_CAUSE_GMM_PS_SERVICE_NOT_TRIG_REG              = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 15),

    /* 说明：PS业务在特定GMM状态下且T3302正在运行时不触发注册 */
    TAF_PS_CAUSE_GMM_PS_SERVICE_NOT_TRIG_REG_T3302RUNNING = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 16),

    /* 说明：服务被禁 */
    TAF_PS_CAUSE_GMM_SERVICE_FORBID                       = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 17),

    /* 说明：PS业务在特定GMM状态下且T3311正在运行时不触发注册 */
    TAF_PS_CAUSE_GMM_PS_SERVICE_NOT_TRIG_REG_T3311RUNNING = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 18),

    /* 说明：GMM返回未知原因值 */
    TAF_PS_CAUSE_GMM_UNKNOWN                              = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 127),

    /*
     * TAF上报的GMM网络原因值, 取值范围[0x0300, 0x03FF]
     * 由于3GPP协议已经定义了具体的GMM网络原因值, 平台上报的GMM网络原因值
     * 取值为协议定义的原因值加上偏移量(0x100)
     * Gmm Cause: Refer to TS 24.008 section 10.5.5.14
     */

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_IMSI_UNKNOWN_IN_HLR                   = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 2),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_ILLEGAL_MS                            = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 3),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_IMSI_NOT_ACCEPTED                     = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 5),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_ILLEGAL_ME                            = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 6),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_GPRS_SERV_NOT_ALLOW                   = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 7),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_GPRS_SERV_AND_NON_GPRS_SERV_NOT_ALLOW = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 8),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_MS_ID_NOT_DERIVED                     = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 9),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_IMPLICIT_DETACHED                     = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 10),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_PLMN_NOT_ALLOW                        = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 11),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_LA_NOT_ALLOW                          = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 12),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_ROAMING_NOT_ALLOW_IN_LA               = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 13),

    /* 说明：标准协议值偏移768，请参见“24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_GMM_NW_GPRS_SERV_NOT_ALLOW_IN_PLMN           = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 14),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_NO_SUITABL_CELL                       = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 15),

    /* 说明：标准协议值偏移768，请参见“24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_GMM_NW_MSC_UNREACHABLE                       = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 16),

    /* 说明：标准协议值偏移768，请参见“24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_GMM_NW_NETWORK_FAILURE                       = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 17),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_MAC_FAILURE                           = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 20),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_SYNCH_FAILURE                         = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 21),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_PROCEDURE_CONGESTION                  = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 22),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_GSM_AUT_UNACCEPTABLE                  = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 23),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_NOT_AUTHORIZED_FOR_THIS_CSG           = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 25),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_NO_PDP_CONTEXT_ACT                    = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 40),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_RETRY_UPON_ENTRY_CELL                 = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 60),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_SEMANTICALLY_INCORRECT_MSG            = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 95),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_INVALID_MANDATORY_INF                 = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 96),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_MSG_NONEXIST_NOTIMPLEMENTE            = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 97),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_MSG_TYPE_NOT_COMPATIBLE               = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 98),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_IE_NONEXIST_NOTIMPLEMENTED            = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 99),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_CONDITIONAL_IE_ERROR                  = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 100),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_MSG_NOT_COMPATIBLE                    = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 101),

    /* 说明：标准协议值偏移768，请参见“3GPP TS 24.008 10.5.5.14章节”。 */
    TAF_PS_CAUSE_GMM_NW_PROTOCOL_ERROR                        = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 111),

    /*
     * TAF上报的CDMA 1X网络原因值, 取值范围[0x400, 0x04FF]
     * 取值为协议定义的原因值加上偏移量(0x400)
     */

    /* 说明：已经在TCH信道 */
    TAF_PS_CAUSE_XCC_OTHER_SERVICE_IN_TCH        = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 1),

    /* 说明：不支持并发业务 */
    TAF_PS_CAUSE_XCC_CCS_NOT_SUPPORT             = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 2),

    /* 说明：未收到Connect order确认 */
    TAF_PS_CAUSE_XCC_CONNECT_ORDER_ACK_FAILURE   = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 3),

    /* 说明：等待来电应答超时 */
    TAF_PS_CAUSE_XCC_INCOMING_RSP_TIME_OUT       = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 4),

    /* 说明：等待L2 Ack超时 */
    TAF_PS_CAUSE_XCC_L2_ACK_TIME_OUT             = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 5),

    /* 说明：MS关机 */
    TAF_PS_CAUSE_XCC_POWER_DOWN_IND              = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 6),

    /* 说明：Connect ID未找到 */
    TAF_PS_CAUSE_XCC_CONNID_NOT_FOUND            = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 7),

    /* 说明：接入确认超时 */
    TAF_PS_CAUSE_XCC_ACCESS_CNF_TIMEOUT          = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 8),

    /* 说明：接入TCH信道超时 */
    TAF_PS_CAUSE_XCC_ACCESS_TCH_TIMEOUT          = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 9),

    /* 说明：T53定时器超时 */
    TAF_PS_CAUSE_XCC_T53M_TIME_OUT               = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 10),

    /* 说明：用户挂断 */
    TAF_PS_CAUSE_XCC_XCALL_HANGUP                = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 11),

    /* 说明：不允许发起呼叫 */
    TAF_PS_CAUSE_XCC_CALL_NOT_ALLOWED            = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 12),

    /* 说明：异常状态 */
    TAF_PS_CAUSE_XCC_ABNORMAL_STATE              = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 13),

    /* 说明：编码失败 */
    TAF_PS_CAUSE_XCC_ENCODE_FAIL                 = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 14),

    /* 说明：等待L2 ACK时，呼叫释放 */
    TAF_PS_CAUSE_XCC_WAIT_L2_ACK_CALL_RELEASE    = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 15),

    /* 说明：找不到呼叫实体 */
    TAF_PS_CAUSE_XCC_CANNOT_FIND_CALL_INSTANCE   = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 16),

    /* 说明：被其他业务主叫打断 */
    TAF_PS_CAUSE_XCC_ABORTED_BY_RECEIVED_MO_CALL = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 17),

    /* 说明：被其他业务被叫打断 */
    TAF_PS_CAUSE_XCC_ABORTED_BY_RECEIVED_MT_CALL = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 18),

    /* 说明：L2 ACK失败 */
    TAF_PS_CAUSE_XCC_CAUSE_L2_ACK_FAIL           = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 19),

    /* 说明：SRID分配失败 */
    TAF_PS_CAUSE_XCC_CAUSE_ALLOC_SRID_FAIL       = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 20),

    /* 说明：射频资源申请失败 */
    TAF_PS_CAUSE_XCC_CAUSE_NO_RF                 = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 21),

    /* 说明：协议版本不支持 */
    TAF_PS_CAUSE_XCC_CAUSE_PROTOTAL_REV_NOT_SUPPORT = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 22),

    /* 说明：在MT SMS TCH状态 */
    TAF_PS_CAUSE_XCC_MT_SMS_IN_TCH = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 23),

    /* 说明：处于紧急呼回呼模式 */
    TAF_PS_CAUSE_XCC_EMC_CALL_BACK_MODE = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 24),

    /*
     * 接入层上报原因值对应到PS域，取值范围[0x500, 0x05FF]
     * 由于3GPP2协议已经定义了具体的1X网络原因值, 平台上报的1X网络原因值
     * 取值为协议定义的原因值加上偏移量(0x500)
     */

    /* 说明：无服务 */
    TAF_PS_CAUSE_XCC_AS_NO_SERVICE             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 1),

    /* 说明：最大接入次数 */
    TAF_PS_CAUSE_XCC_AS_MAX_ACCESS_PROBES      = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 2),

    /* 说明：收到Reorder Order，呼叫失败 */
    TAF_PS_CAUSE_XCC_AS_REORDER                = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 3),

    /* 说明：收到Intercept order，呼叫失败 */
    TAF_PS_CAUSE_XCC_AS_INTERCEPT              = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 4),

    /* 说明：接入被拒 */
    TAF_PS_CAUSE_XCC_AS_ACCESS_DENIED          = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 5),

    /* 说明：网络Lock，呼叫失败 */
    TAF_PS_CAUSE_XCC_AS_LOCK                   = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 6),

    /* 说明：Accept阻塞 */
    TAF_PS_CAUSE_XCC_AS_ACCT_BLOCK             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 7),

    /* 说明：NDSS导致的呼叫失败 */
    TAF_PS_CAUSE_XCC_AS_NDSS                   = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 8),

    /* 说明：重定向导致呼叫失败 */
    TAF_PS_CAUSE_XCC_AS_REDIRECTION            = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 9),

    /* 说明：当前在接入过程中 */
    TAF_PS_CAUSE_XCC_AS_ACCESS_IN_PROGRESS     = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 10),

    /* 说明：接入失败 */
    TAF_PS_CAUSE_XCC_AS_ACCESS_FAIL            = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 11),

    /* 说明：接入层无法处理，呼叫释放 */
    TAF_PS_CAUSE_XCC_AS_ABORT                  = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 12),

    /* 说明：信号弱导致失败 */
    TAF_PS_CAUSE_XCC_AS_SIGNAL_FADE_IN_ACH     = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 13),

    /* 说明：信道指派超时 */
    TAF_PS_CAUSE_XCC_AS_CHANNEL_ASSIGN_TIMEOUT = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 14),

    /* 说明：基站指示释放 */
    TAF_PS_CAUSE_XCC_AS_BS_RELEASE             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 15),

    /* 说明：基站不接受呼叫 */
    TAF_PS_CAUSE_XCC_AS_NOT_ACCEPT_BY_BS       = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 16),

    /* 说明：So被拒 */
    TAF_PS_CAUSE_XCC_AS_SO_REJ                 = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 17),

    /* 说明：释放呼叫超时 */
    TAF_PS_CAUSE_XCC_AS_RELEASE_TIME_OUT       = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 18),

    /* 说明：手机侧正常释放呼叫 */
    TAF_PS_CAUSE_XCC_AS_MS_NORMAL_RELEASE      = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 19),

    /* 说明：尝试失败 */
    TAF_PS_CAUSE_XCC_AS_RETRY_FAIL             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 20),

    /* 说明：网络侧释放链路 */
    TAF_PS_CAUSE_XCC_AS_NW_NORMAL_RELEASE = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 21),

    /* 说明：业务信道上信号衰弱 */
    TAF_PS_CAUSE_XCC_AS_SIGNAL_FADE_IN_TCH = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 22),

    /* 说明：AS主动断链 */
    TAF_PS_CAUSE_XCC_AS_MS_END = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 23),

    /* 说明：TCH 丢失 */
    TAF_PS_CAUSE_XCC_AS_TCH_LOST                = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 24),

    /* 说明：T50定时器超时 */
    TAF_PS_CAUSE_XCC_AS_T50_EXPIRED             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 25),

    /* 说明：T51定时器超时 */
    TAF_PS_CAUSE_XCC_AS_T51_EXPIRED             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 26),

    /* 说明：等待BS BACK定时器超时 */
    TAF_PS_CAUSE_XCC_AS_TCH_WAIT_BS_ACK_EXPIRED = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 27),

    TAF_PS_CAUSE_HSM_AS_CONN_FAIL          = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_HSM_AS_MAX_ACCESS_PROBES  = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_HSM_AS_CONNECTION_DENY    = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_HSM_AS_RTC_RTCACK_FAILURE = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_HSM_AS_SIGNAL_FADE        = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_HSM_AS_SESSION_NOT_EXIST  = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_HSM_AS_CAN_NOT_EST_CONN   = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_HSM_AS_TIMEOUT            = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_HSM_AS_HRPD_SECONDARY     = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_HSM_AS_HANDOFF_FAIL       = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_HSM_AS_IN_UATI_PROCEDURE  = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_HSM_AS_IN_SCP_PROCEDURE = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_HSM_AS_CONNECTION_EXIST = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_HSM_AS_NO_RF            = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_HSM_AS_REDIRECT = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_HSM_AS_HRPD_HANDOFF_TO_1X = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 16),

    TAF_PS_CAUSE_HSM_AS_HRPD_PREFER_CH_NOT_VALID = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_HSM_AS_OHM_NOT_CURRENT = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 18),

    TAF_PS_CAUSE_EHSM_CTTF_NOT_SUPPORT_EHRPD            = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_EHSM_NOT_SUPPORT_EHRPD                 = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_EHSM_IN_SUSPEND                        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 3),

    /* 说明：最后一个PDN */
    TAF_PS_CAUSE_EHSM_LAST_PDN                          = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_EHSM_CONN_EST_FAIL                     = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_EHSM_POWER_OFF                         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 6),

    /* 说明：无效PdnId */
    TAF_PS_CAUSE_EHSM_INVALID_PDN_ID                    = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 7),

    /* 说明：PPP发起Detach */
    TAF_PS_CAUSE_EHSM_PPP_DETACH                        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_EHSM_HSM_CONN_FAIL                     = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_EHSM_HSM_MAX_ACCESS_PROBES             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_EHSM_HSM_CONN_DENY                     = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_EHSM_HSM_RTCACK_RAILURE                = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_EHSM_HSM_SIGNAL_FADE                   = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_EHSM_HSM_CAN_NOT_EST_CONN              = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_EHSM_HSM_TIME_OUT                      = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_EHSM_HSM_HRPD_SECONDARY                = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_EHSM_HSM_SESSION_NOT_EXSIT             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 17),

    /* 说明：PPP General Error */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_GENERAL_ERROR           = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 18),

    /* 说明：未认证的APN */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_UNAUTHORIZED_APN        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 19),

    /* 说明：PDN Limit Exceeded */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PDN_LIMIT_EXCEEDED      = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 20),

    /* 说明：No PGW Available */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_NO_PGW_AVAILABLE        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 21),

    /* 说明：网络端得不到访问 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PGW_UNREACHABLE         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 22),

    /* 说明：网侧拒绝 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PGW_REJECT              = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 23),

    /* 说明：参数不全 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_INSUFFICIENT_PARAMETERS = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 24),

    /* 说明：资源不可用 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_RESOURCE_UNAVAILABLE    = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 25),

    /* 说明：认证失败 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_ADMIN_PROHIBITED        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 26),

    /* 说明：PdnId已经在使用 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PDNID_ALREADY_IN_USE    = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 27),

    /* 说明：Subscription limitation */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_SUBSCRIPTION_LIMITATION = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 28),

    /* 说明：该APN已经被其他PDN使用 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PDN_CONNECTION_ALREADY_EXISTS_FOR_THIS_APN =
        (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 29),

    /* 说明：紧急服务不支持 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_EMERGENCY_SERVICES_NOT_SUPPORTED  = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 30),

    /* 说明：重建不支持 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_RECONNECT_TO_THIS_APN_NOT_ALLOWED = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 31),

    /* *< AT received EAP-Failure from AN in EAP authentication. */

    /* 说明：Lcp EAP Failure */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_AN_EAP_FAILURE = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 32),
    /* *< AT local EAP authentication failure in EAP authentication. */

    /* 说明：Lcp other Failure */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_AT_EAP_FAILURE = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 33),
    /* *< AT hasn`t received AN EAP message in EAP authentication. */

    /* 说明：PPP模块超时 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_EAP_TIMEROUT = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 34),
    /* *< AT hasn`t received AN ack in LCP negotiation. */

    /* 说明：PPP网络侧去激活 */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_LCP_TIMEROUT = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 35),
    /* *< LCP terminate in LCP negotiation and EAP authentication. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_LCP_TERMINATE = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 36),
    /* *< All failure except above failure in LCP negotiation and EAP authentication. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_UNSPECIFIC_FAILURE = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 37),
    /* *< AT hasn`t received AN ack in VSNDCP negotiation. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_VSNCP_TIMEROUT = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 38),
    /* *< CNAS attach req para error  */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_PARA_ERROR = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 39),

    /* 说明：网络断开连接 */
    TAF_PS_CAUSE_EHSM_NW_DISC_IND               = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 40),
    TAF_PS_CAUSE_EHSM_HSM_RSLT_IN_UATI_PROCEDURE = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 41),
    TAF_PS_CAUSE_EHSM_HSM_RSLT_IN_SCP_PROCEDURE  = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 42),
    TAF_PS_CAUSE_EHSM_HSM_NO_RF                  = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 43),
    TAF_PS_CAUSE_EHSM_HSM_REDIRECT               = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 44),
    TAF_PS_CAUSE_EHSM_ABORT          = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 45),
    TAF_PS_CAUSE_EHSM_HSM_NO_NETWORK = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 46),

    /* *< LTE DETACH  */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_LTE_DETACH = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 47),
    /* *< ACCESS AUTH FAILURE  */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_ACCESS_AUTH_FAILURE = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 48),

    /* *< ACCESS AUTH FAILURE  */
    TAF_PS_CAUSE_EHSM_OHM_NOT_CURRENT = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 49),

    /* *< ACCESS AUTH FAILURE  */
    TAF_PS_CAUSE_EHSM_PS_DETACH = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 50),

    /* *< ACCESS AUTH FAILURE  */
    TAF_PS_CAUSE_EHSM_PPPC_ACTED_PDN_LESS_THAN_EHSM = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 51),
    /* *< ACCESS AUTH FAILURE  */

    /* 说明：PPPC存在PDN且状态不为OPEN态 */
    TAF_PS_CAUSE_EHSM_PPPC_EXIST_NOT_OPEN_PDN = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 52),

    TAF_PS_CAUSE_EHSM_ESM_ATTACH_FAIL   = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 53),
    TAF_PS_CAUSE_EHSM_BEARER_SYNC_ERROR = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 54),
    TAF_PS_CAUSE_EHSM_LOCAL_DEACT       = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 55),

    /* 说明：HRPD PPP 协商失败 */
    TAF_PS_CAUSE_PPPC_NEGOTIATE_FAIL      = (TAF_PS_CAUSE_PPPC_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_PPPC_PDSN_AUTH_TYPE_FAIL = (TAF_PS_CAUSE_PPPC_SECTION_BEGIN + 2),

    TAF_PS_CAUSE_MIP_FA_REASON_UNSPECIFIED = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 64),
    /* not fallback */
    TAF_PS_CAUSE_MIP_FA_ADMIN_PROHIBITED       = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 65),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_INSUFFICIENT_RESOURCES = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 66),
    /* not fallback */

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_MOBILE_NODE_FAILED_AUTH       = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 67),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_HA_FAILED_AUTH                = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 68),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_REQUESTED_LIFETIME_TOO_LONG   = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 69),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_MALFORMED_REQUEST             = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 70),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_MALFORMED_REPLY               = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 71),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_ENCAPSULATION_UNAVAILABLE     = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 72),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_VJHC_UNAVAILABLE              = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 73),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_CANT_REV_TUN                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 74),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_MUST_REV_TUN                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 75),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_BAD_TTL                       = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 76),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 79),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HOME_NETWORK_UNREACHABLE         = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 80),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_HOST_UNREACHABLE              = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 81),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_PORT_UNREACHABLE              = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 82),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_UNREACHABLE                   = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 88),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_FA_VS_REASON                     = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 89),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_NON_ZERO_HA_ADDR_IN_RRQ          = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 90),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_NON_ZERO_HOME_ADDR_IN_RRQ        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 96),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_MISSING_NAI                      = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 97),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_MISSING_HA                       = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 98),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_MISSING_HOME_ADDR                = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 99),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_UNKNOWN_CHALLENGE                = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 104),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_MISSING_CHALLENGE                = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 105),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_STALE_CHALLENGE                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 106),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_MISSING_MN_FA                    = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 107),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_REASON_UNSPECIFIED            = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 128),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_ADMIN_PROHIBITED              = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 129),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_INSUFFICIENT_RESOURCES        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 130),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_MOBILE_NODE_FAILED_AUTH       = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 131),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_FA_FAILED_AUTH                = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 132),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_REG_ID_MISMATCH               = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 133),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_MALFORMED_REQUEST             = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 134),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_TOO_MANY_SIMULTANEOUS_BINDING = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 135),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_UNKNOWN_HA                       = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 136),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_CANT_REV_TUN                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 137),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_MUST_REV_TUN                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 138),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_ENCAPSULATION_UNAVAILABLE     = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 139),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_REG_WITH_REDIRECTED_HA           = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 143),

    /* 说明：参考RFC2002 */
    TAF_PS_CAUSE_MIP_HA_TO_HA_BAD_AAA_AUTH            = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 144),

    /* MIP协议未定义或者网侧发来的其它原因值 */
    TAF_PS_CAUSE_MIP_PROTOCOL_UNSPECIFIED = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 255),

    /* TAF APS定义EMM内部拒绝原因值 */

    /* 说明：EMM返回TAU ACP中携带承载与UE不同步 */
    TAF_PS_CAUSE_EMM_NW_BEAR_NOT_SYNC         = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 0),

    /* 说明：EMM返回ESM收到EMM REL_IND消息 */
    TAF_PS_CAUSE_EMM_REL_IND                  = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 1),

    /* 说明：EMM返回鉴权超时 */
    TAF_PS_CAUSE_EMM_AUTH_TIME_OUT            = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 2),

    /* 说明：EMM返回定时器T3410超时 */
    TAF_PS_CAUSE_EMM_T3410_TIME_OUT           = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 3),

    /* 说明：EMM返回建链失败 */
    TAF_PS_CAUSE_EMM_LINK_ERR                 = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 4),

    /* 说明：EMM返回鉴权失败 */
    TAF_PS_CAUSE_EMM_AUTH_FAIL                = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 5),

    /* 说明：EMM返回调节算法启动，被调节 */
    TAF_PS_CAUSE_EMM_MM_THROT                 = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 6),

    /* 说明：EMM返回被搜网打断 */
    TAF_PS_CAUSE_EMM_PLMN_SRCH_INTERRUPT      = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 7),

    /* 说明：EMM返回被系统消息打断 */
    TAF_PS_CAUSE_EMM_SYS_INFO_INTERRUPT       = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 8),

    /* 说明：EMM返回被挂起打断 */
    TAF_PS_CAUSE_EMM_SUSPEND_INTERRUPT        = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 9),

    /* 说明：EMM返回DETACH */
    TAF_PS_CAUSE_EMM_DETACH                   = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 10),

    /* 说明：EMM返回LRRC ERR IND 原因 */
    TAF_PS_CAUSE_EMM_LRRC_ERR_IND             = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 11),

    /* 说明：EMM返回IMSI_PAGING_DETACH原因值 */
    TAF_PS_CAUSE_EMM_IMSI_PAGING_DETACH       = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 12),

    /* 说明：EMM返回ATTACH建链失败 */
    TAF_PS_CAUSE_EMM_ATTACH_EST_FAIL          = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 13),

    /* 说明：EMM返回状态不能触发原因 */
    TAF_PS_CAUSE_EMM_STATUS_CANT_TRIGGER      = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 14),

    /* 说明：EMM返回关机原因 */
    TAF_PS_CAUSE_EMM_POWER_OFF                = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 15),

    /* 说明：EMM返回丢网原因 */
    TAF_PS_CAUSE_EMM_AREA_LOST                = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 16),

    /* 说明：EMM返回SYS_CFG_OR_UE_CAP_CHG原因 */
    TAF_PS_CAUSE_EMM_SYS_CFG_OR_UE_CAP_CHG    = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 17),

    /* 说明：EMM返回GU ATTACH失败 */
    TAF_PS_CAUSE_EMM_GU_ATTACH_FAIL           = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 18),

    /* 说明：EMM返回GU DETACH */
    TAF_PS_CAUSE_EMM_GU_DETACH                = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 19),

    /* 说明：EMM返回MMC REL原因 */
    TAF_PS_CAUSE_EMM_ATTACH_MMC_REL           = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 20),

    /* 说明：EMM返回被CS电话打断 */
    TAF_PS_CAUSE_EMM_ATTACH_CS_CALL_INTERRUPT = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 21),

    /* 说明：EMM返回拔卡原因 */
    TAF_PS_CAUSE_EMM_USIM_PULL_OUT            = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 22),

    /* 说明：EMM返回网络不支持紧急承载 */
    TAF_PS_CAUSE_EMM_NW_NOT_SUPPORT_EMC_BEARER = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 23),

    /* 说明：EMM返回APP Abort流程 */
    TAF_PS_CAUSE_EMM_APP_ABORT_NOTIFY = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 24),

    /* 说明：EMM返回未知原因值 */
    TAF_PS_CAUSE_EMM_UNKNOWN = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 199),

    /* 说明：EMM返回网络拒绝异常原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_NULL                                        = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 0),

    /* 说明：EMM返回网络拒绝Imsi Unknown原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_IMSI_UNKNOWN_IN_HSS                         = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 2),

    /* 说明：EMM返回网络拒绝非法UE原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_ILLEGAL_UE                                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 3),

    /* 说明：EMM返回网络拒绝未识别的IMSI原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_IMEI_NOT_ACCEPTED                           = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 5),

    /* 说明：EMM返回网络拒绝非法ME原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_ILLEGAL_ME                                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 6),

    /* 说明：EMM返回网络拒绝PS服务不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_EPS_SERV_NOT_ALLOW                          = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 7),

    /* 说明：EMM返回网络拒绝PS、CS服务不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_EPS_SERV_AND_NON_EPS_SERV_NOT_ALLOW         = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 8),

    /* 说明：EMM返回网络拒绝UE ID异常原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_UE_ID_NOT_DERIVED                           = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 9),

    /* 说明：EMM返回网络拒绝已经DETACH原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_IMPLICIT_DETACHED                           = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 10),

    /* 说明：EMM返回网络拒绝PLMN不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_PLMN_NOT_ALLOW                              = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 11),

    /* 说明：EMM返回网络拒绝TA不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_TA_NOT_ALLOW                                = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 12),

    /* 说明：EMM返回网络拒绝漫游不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_ROAM_NOT_ALLOW                              = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 13),

    /* 说明：EMM返回网络拒绝PS服务在该PLMN下不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_EPS_SERV_NOT_ALLOW_IN_PLMN                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 14),

    /* 说明：EMM返回网络拒绝不允许的小区原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_NO_SUITABL_CELL                             = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 15),

    /* 说明：EMM返回网络拒绝Msc Unreachable原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_MSC_UNREACHABLE                             = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 16),

    /* 说明：EMM返回网络拒绝网络失败原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_NETWORK_FAILURE                             = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 17),

    /* 说明：EMM返回网络拒绝CS不可用原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_CS_NOT_AVAIL                                = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 18),

    /* 说明：EMM返回网络拒绝ESM层的原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_ESM_FAILURE                                 = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 19),

    /* 说明：EMM返回网络拒绝MAC层的原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_MAC_FAILURE                                 = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 20),

    /* 说明：EMM返回网络拒绝同步失败原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_SYNCH_FAILURE                               = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 21),

    /* 说明：EMM返回网络拒绝Procedure Congestion原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_PROCEDURE_CONGESTION                        = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 22),

    /* 说明：EMM返回网络拒绝安全能力不匹配原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_UE_SECU_CAP_MISMATCH                        = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 23),

    /* 说明：EMM返回网络拒绝未指定的安全模式原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_SECU_MODE_REJECTED_UNSPECIFIED              = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 24),

    /* 说明：EMM返回网络拒绝未授权原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_NOT_AUTHORIZED_FOR_THIS_CSG                 = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 25),

    /* 说明：EMM返回网络拒绝在该PLMN下资源服务未授权原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_REQUESTED_SER_OPTION_NOT_AUTHORIZED_IN_PLMN = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 35),

    /* 说明：EMM返回网络拒绝CSFB建链不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_CS_FALLBACK_CALL_EST_NOT_ALLOWED            = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 38),

    /* 说明：EMM返回网络拒绝CS域不允许原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_CS_DOMAIN_TMP_NOT_ALLOWED                   = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 39),

    /* 说明：EMM返回网络拒绝没有PS承载激活原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_NO_EPS_BEARER_CONTEXT_ACTIVATED             = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 40),

    /* 说明：EMM返回网络拒绝服务网络失败原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_SERVER_NETWORK_FAILURE                      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 42),

    /* 说明：EMM返回网络拒绝语义错误原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_SEMANTICALLY_INCORRECT_MSG                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 95),

    /* 说明：EMM返回网络拒绝无效的必要信息原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_INVALID_MANDATORY_INF                       = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 96),

    /* 说明：EMM返回网络拒绝不存在必要的消息原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_MSG_NONEXIST_NOTIMPLEMENTE                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 97),

    /* 说明：EMM返回网络拒绝不合适的消息类型原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_MSG_TYPE_NOT_COMPATIBLE                     = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 98),

    /* 说明：EMM返回网络拒绝不存在必要的IE项原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_IE_NONEXIST_NOTIMPLEMENTED                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 99),

    /* 说明：EMM返回网络拒绝IE项错误原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_CONDITIONAL_IE_ERROR                        = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 100),

    /* 说明：EMM返回网络拒绝不合适的消息原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_MSG_NOT_COMPATIBLE                          = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 101),

    /* 说明：EMM返回网络拒绝协议错误原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_PROTOCOL_ERROR                              = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 111),

    /* 说明：EMM返回网络拒绝其他原因值 */
    TAF_PS_CAUSE_EMM_NW_REJ_OTHERS                                      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 255),

    /* 说明：WLAN下PDP激活参数错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PARA_ERROR                         = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 1),

    /* 说明：WLAN下PDP激活链路丢失 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_LINKLOST                           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 2),

    /* 说明：WLAN下PDP激活建立超时 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_TIMEOUT                            = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 3),

    /* 说明：WLAN下PDP激活MAPCON状态错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SM_ERROR                           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 4),

    /* 说明：WLAN下PDP激活其他错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_OTHER_ERROR                        = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 5),

    /* 说明：WLAN下PDP激活(U)SIM鉴权无响应 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SIM_AUTH_NO_RESP                   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 6),

    /* 说明：WLAN下PDP激活网络鉴权错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_AUTH_NETWORK_ERROR                 = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 7),

    /* 说明：WLAN下PDP激活解析网络负载错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PARSE_NETWORK_PAYLOAD_ERROR        = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 8),

    /* 说明：WLAN下PDP激活SA/SP错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ADD_SA_SP_ERROR                    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 9),

    /* 说明：WLAN下PDP激活客户端负载错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CLIENT_PAYLOAD_ERROR               = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 10),

    /* 说明：WLAN下PDP激活网络超时错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_MSG_NETWORK_OVERTIME_ERROR         = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 11),

    /* 说明：WLAN下PDP激活ENC算法错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ENC_ALG_ERROR                      = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 12),

    /* 说明：WLAN下PDP激活APN错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_APN_ID_ERROR                       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 13),

    /* 说明：WLAN下PDP激活网络鉴权(U)SIM错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NETWORK_AUTH_SIM_ERROR             = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 14),

    /* 说明：WLAN下PDP激活状态错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_STATUS_ERROR                       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 15),

    /* 说明：WLAN下PDP激活SA初始化错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SA_INIT_ERROR                      = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 16),

    /* 说明：WLAN下PDP激活无3GPP可接入错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NON_3GPP_ACCESS_TO_EPC_NOT_ALLOWED = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 17),

    /* 说明：WLAN下PDP激活未知用户名错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_USER_UNKNOWN                       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 18),

    /* 说明：WLAN下PDP激活PLMN不允许错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PLMN_NOT_ALLOWED                   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 19),

    /* 说明：WLAN下PDP激活鉴权被拒错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_AUTHORIZATION_REJECTED             = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 20),

    /* 说明：WLAN下PDP激活RAT类型不允许错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_RAT_TYPE_NOT_ALLOWED               = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 21),

    /* 说明：WLAN下PDP激活网络错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NETWORK_FAILURE                    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 22),

    /* 说明：WLAN下PDP激活没有订阅该APN错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NO_APN_SUBSCRIPTION                = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 23),

    /* 说明：WLAN下PDP激活连接被拒 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PDN_CONNECTION_REJECTION           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 24),

    /* 说明：WLAN下PDP激活连接到达错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CONNECTION_REACHED                 = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 25),

    /* 说明：WLAN下PDP激活Protecle错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PROTECLE_ERROR                     = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 26),

    /* 说明：WLAN下PDP激活内部地址错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_INTERNAL_ADDRESS_ERROR             = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 27),

    /* 说明：WLAN下PDP激活IPV4地址冲突错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV4_ADDRESS_CONFLICT              = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 28),

    /* 说明：WLAN下PDP激活CERT错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CERT_ERROR                         = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 29),

    /* 说明：WLAN下PDP激活不支持错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NOT_SUPPORT_CONCURRENCE            = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 30),

    /* 说明：WLAN下PDP激活关机错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SHUTDOWN                           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 31),

    /* 说明：WLAN下PDP激活未知错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_DONE_ERROR                         = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 32),

    /* 说明：WLAN下PDP激活重定向错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_REDIRECT_FAILURE                   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 33),

    /* 说明：WLAN下PDP激活漫游被禁错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ROAM_FORBID                        = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 50),

    /* 说明：WLAN下PDP激活HIFI状态超时错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_HIFI_STATUS_OVERTIME               = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 51),

    /* 说明：WLAN下PDP激活IKED异常错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IKED_ABNORMAL                      = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 52),

    /* 说明：WLAN下PDP激活未知网络错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_UNKNOWN_NETWORK_ERROR              = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 53),

    /* 说明：WLAN下PDP激活切换达到最大次数错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SWITCH_CACHE_MAX_REACHED           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 54),

    /* 说明：WLAN下PDP激活APN达到最大次数 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_APN_MAX_REACHED                    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 55),

    /* 说明：WLAN下PDP激活请求重复错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_REQUEST_REPEATED                   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 56),

    /* 说明：WLAN下PDP激活未知原因错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_UNKNOWN                            = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 200),

    /* 说明：WLAN下PDP去激活保活包没有回复 */
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_KEEP_LIVE                     = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 201),

    /* 说明：WLAN下PDP去激活网络主动释放隧道 */
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_NETWORK_DELETE                = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 202),

    /* 说明：WLAN下PDP去激活IKED异常重启 */
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_IKED_RESET                    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 203),

    /* 说明：WLAN下PDP去激活REKEY失败 */
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_REKEY_ERROR                   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 204),

    /* 说明：WLAN下PDP去激活MAPCON自主释放隧道 */
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_MAPCONN_TEAR                  = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 205),

    /* 说明：WLAN下PDP去激活不支持承载并发 */
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_BEARS_CONNCURRENT_NOT_SUPPOER = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 206),

    /* 说明：WLAN下PDP去激活未知原因 */
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_UNKNOWN                       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 230),

    /* 说明：WLAN下PDP激活在HANDOVER中地址改变错误 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IP_ADDRESS_CHG_IN_HANDOVER = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 231),

    /* 说明：WLAN下PDP激活不支持IPV4 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV4_NOT_ALLOWED           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 232),

    /* 说明：WLAN下PDP激活不支持IPV6 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV6_NOT_ALLOWED           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 233),

    /* 说明：WLAN下PDP激活被调节模块进行调节后不允许发起 */
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_THROT_NOT_ALLOWED          = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 234),

    /*
     * NRSM的内部原因值, 取值范围[0x0000, 0x00FF]
     * 方向: NRSM -> TAF
     * 说明: UNKNOWN为错误码分段的最后一个错误码
     */

    /* 说明：T3580协议定时器超时(每轮激活请求都成功发送到网络，网络未应答)，参考24501 */
    TAF_PS_CAUSE_NRSM_T3580_TIME_OUT                                 = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 1),

    /* 说明：在NRSM记录的禁止列表中，参考24501 */
    TAF_PS_CAUSE_NRSM_IN_FORBIDDEN_LIST                              = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 2),

    /* 说明：Pdu session建立失败 */
    TAF_PS_CAUSE_NRSM_SESSION_EST_FAILED                             = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 3),

    /* 说明：收到NRMM的suspend ind消息 */
    TAF_PS_CAUSE_NRSM_SUSPEND                                        = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 4),

    /* 说明：NRSM失败 */
    TAF_PS_CAUSE_NRSM_FAILURE                                        = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 5),

    /* 说明：正在进行pdu session重建 */
    TAF_PS_CAUSE_NRSM_REBUILD_PDU_SESSION                            = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 6),

    /* 说明：Pdu session修改失败 */
    TAF_PS_CAUSE_NRSM_SESSION_MODIFY_FAILED                          = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 7),

    /* 说明：T3581协议定时器超时，参考24501 */
    TAF_PS_CAUSE_NRSM_T3581_TIME_OUT                                 = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 8),

    /* 说明：在backofftimer禁止时长之内 */
    TAF_PS_CAUSE_NRSM_BACK_OFF_TIMER_NOT_ALLOWED                     = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 9),

    /* 说明：紧急呼状态下不允许建立普通pdu session */
    TAF_PS_CAUSE_NRSM_NOT_ALLOW_EST_NORMAL_SESSION_IN_EMC_STATUS     = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 10),

    /* 说明：当前PLMN下已经建立了最大数量的pdu session */
    TAF_PS_CAUSE_NRSM_REACHED_PLMN_MAX_PDU_SESSION_NUM               = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 11),

    /* 说明：等待APS的pdu session info update ntf消息超时 */
    TAF_PS_CAUSE_NRSM_PDU_SESSION_INFO_UPDATE_FAIL                   = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 12),

    /* 说明：申请pdu session 上下文实体失败 */
    TAF_PS_CAUSE_NRSM_ALLOC_ENTITY_FAIL                              = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 13),

    /* 说明：Ue发起的会话修改流程和网侧（或NRSM内部）发起的release流程冲突，参考24501 */
    TAF_PS_CAUSE_NRSM_UE_MODIFICATION_COLLISION_WITH_NW_RELEASE      = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 14),

    /* 说明：网侧发起的modify流程和UE（或NRSM内部）发起的modify流程冲突 */
    TAF_PS_CAUSE_NRSM_NW_MODIFICATION_COLLISION_WITH_UE_MODIFICATION = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 15),

    /* 说明：无射频资源 */
    TAF_PS_CAUSE_NRSM_NO_RF                                          = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 16),

    /* 说明：T3582协议定时器超时，参考24501 */
    TAF_PS_CAUSE_NRSM_T3582_TIME_OUT                                 = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 17),

    /* 说明：不允许建立多个紧急pdu session */
    TAF_PS_CAUSE_NRSM_MULTI_EMERGENCY_NOT_ALLOWED                    = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 18),

    /* 说明：不支持建立紧急pdu session */
    TAF_PS_CAUSE_NRSM_EMERGENCY_NOT_SUPPORTED                        = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 19),

    /* 说明：收到NRMM的发送结果指示重选或切到导致的失败 */
    TAF_PS_CAUSE_NRSM_FAILURE_RESEL_OR_HO                            = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 20),

    /* 说明：收到NRMM的发送结果指示连接释放导致的失败 */
    TAF_PS_CAUSE_NRSM_FAILURE_RADIO_LINK_REL                         = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 21),

    /* 说明：收到NRMM的发送结果指示没有SRB发送NAS直传消息 */
    TAF_PS_CAUSE_NRSM_FAILURE_CTRL_NOT_CONN                          = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 22),

    /* 说明：收到NRMM的发送结果指示连接重建、RLF等导致直传消息发送失败 */
    TAF_PS_CAUSE_NRSM_FAILURE_RLF                                    = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 23),

    /* 说明：收到NRMM的发送结果指示连接恢复或连接重建流程，网络触发连接新建，导致NAS直传消息发送失败 */
    TAF_PS_CAUSE_NRSM_NEWSETUP_IN_CONN_RESUME_OR_REEST               = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 24),

    /* 说明：收到NRMM的发送结果指示INACTIVE下NAS直传消息触发连接恢复，但网络拒绝恢复导致发送失败 */
    TAF_PS_CAUSE_NRSM_NW_REJ_IN_INACTIVE_TO_CONN                     = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 25),

    /* 说明：收到NRMM的发送结果指示INACTIVE下NAS直传消息触发连接恢复，但连接恢复异常导致发送失败 */
    TAF_PS_CAUSE_NRSM_FAILURE_IN_INACTIVE_TO_CONN                    = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 26),

    /* 说明：收到NRMM的发送结果指示NRRC缓存队列满了导致发送失败 */
    TAF_PS_CAUSE_NRSM_RRC_BUFF_FULL                                  = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 27),

    /* 说明：收到NRMM的发送结果指示消息参数非法 */
    TAF_PS_CAUSE_NRSM_MSG_PARA_INVALID                               = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 28),

    /* 说明：收到NRMM的发送结果指示ACC CATEGORY被禁止导致的失败 */
    TAF_PS_CAUSE_NRSM_FAILURE_ACC_BAR                                = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 29),

    /* 说明：收到NRMM的发送结果指示LADN不允许 */
    TAF_PS_CAUSE_NRSM_FAILURE_LADN_NOT_ALLOWED                       = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 30),

    /* 说明：编码失败 */
    TAF_PS_CAUSE_NRSM_ENCODE_FAILURE                                 = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 31),

    /* 说明：不允许的切片信息 */
    TAF_PS_CAUSE_NRSM_NOT_ALLOWED_NSSAI                      = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 32),

    /* 说明：不在正常注册状态 */
    TAF_PS_CAUSE_NRSM_NOT_REGISTER                           = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 33),

    /* 说明：收到NRMM的发送结果指示连接已经存在 */
    TAF_PS_CAUSE_NRSM_PRESENT                                = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 34),

    /* 说明：收到NRMM的发送结果指示连接正在建立 */
    TAF_PS_CAUSE_NRSM_ESTING                                 = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 35),

    /* 说明：收到NRMM的发送结果指示连接正在释放 */
    TAF_PS_CAUSE_NRSM_DELING                                 = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 36),

    /* 说明：收到NRMM的发送结果指示连接建立失败 */
    TAF_PS_CAUSE_NRSM_CONN_FAIL                              = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 37),

    /* 说明：收到NRMM的发送结果指示连接建立拒绝，切换到异系统 */
    TAF_PS_CAUSE_NRSM_INTER_RAT_SUSPEND                      = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 38),

    /* 说明：收到NRMM的发送结果指示连接被网络拒绝 */
    TAF_PS_CAUSE_NRSM_CONN_REJ_BY_NW                         = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 39),

    /* 说明：收到NRMM的发送结果指示随机接入失败 */
    TAF_PS_CAUSE_NRSM_RANDOM_ACCESS_REJECT                   = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 40),

    /* 说明：收到NRMM的发送结果指示正在搜网 */
    TAF_PS_CAUSE_NRSM_CELL_SEARCHING                         = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 41),

    /* 说明：收到NRMM的发送结果指示丢网了 */
    TAF_PS_CAUSE_NRSM_AREA_LOST                              = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 42),

    /* 说明：收到NRMM的发送结果指示不支持紧急呼叫 */
    TAF_PS_CAUSE_NRSM_NOT_SUPPORT_IMS_EMC_CALL               = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 43),

    /* 说明：收到NRMM的发送结果指示不支持的呼叫类型 */
    TAF_PS_CAUSE_NRSM_NOT_SUPPORT_CALL_TYPE                  = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 44),

    /* 说明：收到NRMM的发送结果指示PLMN不在SIBI PLMN LIST中 */
    TAF_PS_CAUSE_NRSM_SELECT_PLMN_INVALID                    = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 45),

    /* 说明：收到NRMM的发送结果指示等待NRRC的建立回复超时 */
    TAF_PS_CAUSE_NRSM_WAIT_NRRC_EST_CNF_EXPIRED              = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 46),

    /* 说明：Pdu session id已经被占用 */
    TAF_PS_CAUSE_NRSM_PDU_SESSION_ALREADY_EXIST              = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 47),

    /* 说明：无效LADN */
    TAF_PS_CAUSE_NRSM_LADN_NOT_ALLOW                         = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 48),

    /* 说明：Pdu session不存在 */
    TAF_PS_CAUSE_NRSM_PDU_NOT_EXIST                          = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 49),

    /* 说明：取消反射QoS能力 */
    TAF_PS_CAUSE_NRSM_REVOKE_REFLECTIVE_QOS                  = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 50),

    /* 说明：UE发起的modify流程和网侧鉴权流程对冲 */
    TAF_PS_CAUSE_NRSM_UE_MODIFICATION_COLLISION_WITH_NW_AUTH = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 51),

    /* 说明：NRSM发送data req消息失败 */
    TAF_PS_CAUSE_NRSM_SND_DATA_REQ_FAIL        = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 52),

    /* 说明：N26标志发生变化 */
    TAF_PS_CAUSE_NRSM_N26_FLAG_CHANGE          = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 53),

    /* 说明：网侧PDU SESSION ID信息更新了 */
    TAF_PS_CAUSE_NRSM_PDU_SESSION_ID_UPDATE    = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 54),

    /* 说明：QoS错误 */
    TAF_PS_CAUSE_NRSM_QOS_ERROR                = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 55),

    /* 说明：去激活了默认EPS bearer */
    TAF_PS_CAUSE_NRSM_DEACT_DEFAULT_EPS_BEARER = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 56),

    /* 说明：流程保护定时器超时 */
    TAF_PS_CAUSE_NRSM_PROCESS_PROTECT_TIME_OUT = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 57),

    /* 说明：T3580协议定时器超时(存在至少一轮激活请求发送失败场景，未收到网络激活请求超时)，参考24501 */
    TAF_PS_CAUSE_NRSM_RETRANSMIT_T3580_TIME_OUT = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 58),

    /* 说明：未知原因值 */
    TAF_PS_CAUSE_NRSM_UNKNOWN = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 127),

    /*
     * NRMM的内部原因值, 取值范围[0x0200, 0x02FF]
     * 方向: NRMM -> NRSM -> TAF
     * 说明: UNKNOWN为错误码分段的最后一个错误码
     */

    /* 说明：收到了关机指示消息 */
    TAF_PS_CAUSE_NRMM_POWER_OFF_IND              = (TAF_PS_CAUSE_NRMM_SECTION_BEGIN + 0),

    /* 说明：收到去激活非紧急pdu session指示消息 */
    TAF_PS_CAUSE_NRMM_DEACT_NON_EMC_PDU_IND      = (TAF_PS_CAUSE_NRMM_SECTION_BEGIN + 1),

    /* 说明：收到了NRMM的去注册指示消息且类型是网侧主动去注册需要重新注册 */
    TAF_PS_CAUSE_NRMM_REL_IND_NW_DEREG_REREG_REQUIRED = (TAF_PS_CAUSE_NRMM_SECTION_BEGIN + 2),

    /* 说明：收到了NRMM的去注册指示消息且类型是网侧主动去注册不需要重新注册 */
    TAF_PS_CAUSE_NRMM_REL_IND_NW_DEREG_REREG_NOT_REQUIRED = (TAF_PS_CAUSE_NRMM_SECTION_BEGIN + 3),

    /* 说明：收到了NRMM的去注册指示消息且类型不是网侧主动去注册 */
    TAF_PS_CAUSE_NRMM_REL_IND_NOT_NW_DEREG = (TAF_PS_CAUSE_NRMM_SECTION_BEGIN + 4),

    /*
     * NRMM的网络原因值, 取值范围[0x0F00, 0x0FFF]
     * 方向: NRMM -> NRSM -> TAF
     * 说明: UNKNOWN为错误码分段的最后一个错误码
     */

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_ILLEGAL_UE                               = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 3),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_PEI_NOT_ACCEPTED                         = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 5),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_ILLEGAL_ME                               = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 6),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_5GS_SERVICES_NOT_ALLOWED                 = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 7),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_IMPLICIT_DEREGISTERED                    = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 10),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_PLMN_NOT_ALLOWED                         = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 11),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_TA_NOT_ALLOWED                           = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 12),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_ROAMING_NOT_ALLOWED_IN_THIS_TA           = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 13),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_MAC_FAILURE                              = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 20),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_SYNCH_FAILURE                            = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 21),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_CONGESTION                               = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 22),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_N1_MODE_NOT_ALLOWED                      = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 27),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_RSTRIC_SERVICE_AREA                      = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 28),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_LADN_NOT_AVAIL                           = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 43),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_REACHED_PLMN_MAX_PDU_SESSION_NUM         = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 65),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_INSUFFICIENT_RSRC_FOR_SPEC_SLICE_AND_DNN = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 67),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_INSUFFICIENT_RSRC_FOR_SPEC_SLICE         = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 69),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_PAYLOAD_NOT_FORWARDED                    = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 90),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_DNN_NOT_SUPPORT_OR_NOT_SUB_IN_THE_SLICE  = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 91),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_SEMANTICALLY_INCORRECT_MSG               = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 95),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_INVALID_MANDATORY_INF                    = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 96),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_MSG_NONEXIST_NOTIMPLEMENTE               = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 97),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_MSG_TYPE_NOT_COMPATIBLE                  = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 98),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_IE_NONEXIST_NOTIMPLEMENTED               = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 99),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_CONDITIONAL_IE_ERROR                     = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 100),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_MSG_NOT_COMPATIBLE                       = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 101),

    /* 说明：标准协议值偏移3840，请参见“3GPP TS 24.501 9.11.3.2章节”。 */
    TAF_PS_CAUSE_NRMM_NW_PROTOCOL_ERROR                           = (TAF_PS_CAUSE_NRMM_NW_SECTION_BEGIN + 111),

    TAF_PS_CAUSE_CDS_IPF_FILTER_ERROR = (TAF_PS_CAUSE_CDS_SECTION_BEGIN + 0),

    /*
     * SDAP的原因值, 取值范围[0x1100, 0x11FF]
     * 方向: SDAP -> NRSM -> TAF
     */

    /* 说明：SDAP触发的PDU去激活 */
    TAF_PS_CAUSE_SDAP_RELEASE_PDU     = (TAF_PS_CAUSE_SDAP_SECTION_BEGIN + 0),

    TAF_PS_CAUSE_BUTT = 0xFFFFFFFF
};
typedef VOS_UINT32 TAF_PS_CauseUint32;


enum TAF_PS_CallEndCause {
    TAF_PS_CALL_END_CAUSE_NORMAL = 0x00,
    TAF_PS_CALL_END_CAUSE_FORCE  = 0x01, /* 强制去激活 */

    /* 业务场景的去激活原因 */
    TAF_PS_CALL_END_CAUSE_IPV6_ADDR_ALLOC_FAIL  = 0x10, /* IPV6路由公告分配失败 */
    TAF_PS_CALL_END_CAUSE_IPV6_ADDR_TIME_OUT    = 0x11, /* IPV6地址失效 */
    TAF_PS_CALL_END_CAUSE_PCSCF_ADDR_ALLOC_FAIL = 0x12, /* PCSCF地址不存在 */
    TAF_PS_CALL_END_CAUSE_LOCAL                 = 0x13, /* 本地去激活 */

    TAF_PS_CALL_END_CAUSE_CELLULAR2W_HO = 0x14,

    TAF_PS_CALL_END_CAUSE_IMS_LOCAL_DEACT = 0x15,

    TAF_PS_CALL_END_CAUSE_BUTT
};
typedef VOS_UINT8 TAF_PS_CallEndCauseUint8;


enum TAF_PF_TransDirection {
    /* 0 - Pre-Release 7 TFT filter (see 3GPP TS 24.008 [8], table 10.5.162) */
    TAF_PF_TRANS_DIRECTION_PRE_REL7    = 0x00,
    TAF_PF_TRANS_DIRECTION_UPLINK      = 0x01, /* 1 - Uplink */
    TAF_PF_TRANS_DIRECTION_DOWNLINK    = 0x02, /* 2 - Downlink */
    TAF_PF_TRANS_DIRECTION_BIDIRECTION = 0x03, /* 3 - Birectional (Up & Downlink) (default if omitted) */

    TAF_PF_TRANS_DIRECTION_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PF_TransDirectionUint8;


enum TAF_PDP_Type {
    TAF_PDP_IPV4     = 0x01,
    TAF_PDP_IPV6     = 0x02,
    TAF_PDP_IPV4V6   = 0x03,
    TAF_PDP_PPP      = 0x04,
    TAF_PDP_ETHERNET = 0x08,

    TAF_PDP_TYPE_BUTT = 0xFF
};
typedef VOS_UINT8 TAF_PDP_TypeUint8;


enum TAF_PS_BearerType {
    TAF_PS_BEARER_TYPE_DEFAULT   = 0x00,
    TAF_PS_BEARER_TYPE_DEDICATED = 0x01,
    TAF_PS_BEARER_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PS_BearerTypeUint8;


enum TAF_PS_SscMode {
    TAF_PS_SSC_MODE_1 = 0x00,
    TAF_PS_SSC_MODE_2 = 0x01,
    TAF_PS_SSC_MODE_3 = 0x02,
    TAF_PS_SSC_MODE_BUTT
};
typedef VOS_UINT8 TAF_PS_SscModeUint8;


enum TAF_PS_SnssaiSst {
    TAF_PS_SNSSAI_SST_INVALID = 0x00,
    TAF_PS_SNSSAI_SST_EMBB    = 0x01,
    TAF_PS_SNSSAI_SST_URLLC   = 0x02,
    TAF_PS_SNSSAI_SST_MIOT    = 0x03,
    TAF_PS_SNSSAI_SST_BUTT
};
typedef VOS_UINT8 TAF_PS_SnssaiSstUint8;


enum TAF_PS_PrefAccessType {
    TAF_PS_PREF_ACCESS_TYPE_3GPP     = 0x0000,
    TAF_PS_PREF_ACCESS_TYPE_NON_3GPP = 0x0001,

    TAF_PS_PREF_ACCESS_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PS_PrefAccessTypeUint8;


enum TAF_PS_AccessType {
    TAF_PS_ACCESS_TYPE_3GPP     = 0x0000,
    TAF_PS_ACCESS_TYPE_NON_3GPP = 0x0001,

    TAF_PS_ACCESS_TYPE_BUTT
};
typedef VOS_UINT16 TAF_PS_AccessTypeUint16;


enum TAF_PS_ReflectQosInd {
    TAF_PS_REFLECT_QOS_IND_NOT_SUPPORT = 0,
    TAF_PS_REFLECT_QOS_IND_SUPPORT     = 1,

    TAF_PS_REFLECT_QOS_IND_BUTT
};
typedef VOS_UINT8 TAF_PS_ReflectQosIndUint8;


enum TAF_PS_Ipv6MultiHomingInd {
    TAF_PS_IPV6_MULTI_HOMING_IND_NOT_SUPPORT = 0,
    TAF_PS_IPV6_MULTI_HOMING_IND_SUPPORT     = 1,

    TAF_PS_IPV6_MULTI_HOMING_IND_BUTT
};
typedef VOS_UINT8 TAF_PS_Ipv6MultiHomingIndUint8;


enum TAF_PS_AlwaysOnInd {
    TAF_PS_ALWAYS_ON_IND_NOT = 0,
    TAF_PS_ALWAYS_ON_IND     = 1,

    TAF_PS_ALWAYS_ON_IND_BUTT
};
typedef VOS_UINT8 TAF_PS_AlwaysOnIndUint8;


enum TAF_PS_DomainType {
    TAF_PS_DOMAIN_TYPE_3GPP  = 0,
    TAF_PS_DOMAIN_TYPE_EPDG  = 1,
    TAF_PS_DOMAIN_TYPE_N3IWF = 2,

    /* TODO:是否需要增加3GPP2类型待定 */
    TAF_PS_DOMAIN_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PS_DomainTypeUint8;


enum TAF_PS_HandoverResult {
    TAF_PS_HANDOVER_RESULT_SUCCESS = 0,
    TAF_PS_HANDOVER_RESULT_FAIL    = 1,

    TAF_PS_HANDOVER_RESULTBUTT
};
typedef VOS_UINT8 TAF_PS_HandoverResultUint8;


typedef struct {
    VOS_UINT32 bitOpIpAddr : 1;
    VOS_UINT32 bitOpPrimDns : 1;
    VOS_UINT32 bitOpSecDns : 1;
    VOS_UINT32 bitOpPrimNbns : 1;
    VOS_UINT32 bitOpSecNbns : 1;
    VOS_UINT32 bitOpSpare : 27;

    VOS_UINT8 ipAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 primDns[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 secDns[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 primNbns[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 secNbns[TAF_IPV4_ADDR_LEN];

} TAF_PS_IpcpInfo;


typedef struct {
    VOS_UINT32 challengeLen;
    VOS_UINT8  challenge[TAF_PS_PPP_CHAP_CHALLNGE_LEN];
    VOS_UINT32 challengeNameLen;
    VOS_UINT8  challengeName[TAF_PS_PPP_CHAP_CHALLNGE_NAME_LEN];
    VOS_UINT32 responseLen;
    VOS_UINT8  response[TAF_PS_PPP_CHAP_RESPONSE_LEN];
    VOS_UINT8  responseNameLen;
    VOS_UINT8  responseName[TAF_MAX_AUTHDATA_USERNAME_LEN];

} TAF_PDP_ChapParam;


typedef struct {
    VOS_UINT8 usernameLen;
    VOS_UINT8 username[TAF_MAX_AUTHDATA_USERNAME_LEN];
    VOS_UINT8 passwordLen;
    VOS_UINT8 password[TAF_MAX_AUTHDATA_PASSWORD_LEN];

} TAF_PDP_PapParam;


typedef struct {
    TAF_PDP_AuthTypeUint8 authType;
    VOS_UINT8             authId;
    VOS_UINT8             rsv[2];

    union {
        TAF_PDP_ChapParam chapParam;
        TAF_PDP_PapParam  papParam;
    } unAuthParam;

} TAF_PS_AuthInfo;

/*
 * 结构说明: 鉴权配置结构体
 */
typedef struct {
    VOS_UINT32 bitOpUserName : 1;
    VOS_UINT32 bitOpPassWord : 1;
    VOS_UINT32 bitOpAuthParam : 1;
    VOS_UINT32 bitOpSpare : 29;

    /* NDISDUP & PPP拨号类型用此参数 */
    TAF_PDP_AuthTypeUint8 authType;
    VOS_UINT8             authId; /* PPP拨号类型用此参数 */
    VOS_UINT8             reserve[2];
    /* NDISDUP拨号类型用此参数 */
    VOS_UINT8 userName[TAF_MAX_AUTHDATA_USERNAME_LEN + 1];
    /* NDISDUP拨号类型用此参数 */
    VOS_UINT8 passWord[TAF_MAX_AUTHDATA_PASSWORD_LEN + 1];

    union {
        TAF_PDP_ChapParam chapParam;
        TAF_PDP_PapParam  papParam;
    } unAuthParam; /* PPP拨号类型用此参数 */
} TAF_PDP_AuthConfigInfo;

/*
 * 结构说明: PDP上下文中APN信息
 */
typedef struct {
    VOS_UINT8 length;
    VOS_UINT8 value[TAF_MAX_APN_LEN];

} TAF_PDP_Apn;


typedef struct {
    /* IPV4:1, IPV6:2, IPV4V6:3, PPP:4, Ethernet:8 */
    TAF_PDP_TypeUint8 pdpType;
    VOS_UINT8         reserved[3];
    VOS_UINT8         ipv4Addr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8         ipv6Addr[TAF_IPV6_ADDR_LEN];
} TAF_PDP_Addr;


typedef struct {
    VOS_UINT32 bitOpPrimNbnsAddr : 1;
    VOS_UINT32 bitOpSecNbnsAddr : 1;
    VOS_UINT32 bitOpSpare : 30;

    VOS_UINT8 primNbnsAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 secNbnsAddr[TAF_IPV4_ADDR_LEN];

} TAF_PDP_Nbns;


typedef struct {
    VOS_UINT32 bitOpGateWayAddr : 1;
    VOS_UINT32 bitOpSpare : 31;

    VOS_UINT8 gateWayAddr[TAF_IPV4_ADDR_LEN];

} TAF_PDP_Gateway;


typedef struct {
    VOS_UINT32 bitOpPrimDnsAddr : 1;
    VOS_UINT32 bitOpSecDnsAddr : 1;
    VOS_UINT32 bitOpSpare : 30;

    VOS_UINT8 primDnsAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 secDnsAddr[TAF_IPV4_ADDR_LEN];

} TAF_PDP_Dns;


typedef struct {
    VOS_UINT32 bitOpPrimDnsAddr : 1;
    VOS_UINT32 bitOpSecDnsAddr : 1;
    VOS_UINT32 bitOpSpare : 30;

    VOS_UINT8 primDnsAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8 secDnsAddr[TAF_IPV6_ADDR_LEN];

} TAF_PDP_Ipv6Dns;


typedef struct {
    TAF_PDP_Addr       pdpAddr; /* 用户设置的静态IP地址 */
    TAF_PDP_Apn        apn;
    TAF_PDP_DCompUint8 pdpDcomp;
    TAF_PDP_HCompUint8 pdpHcomp;

    /*
     * 0 - IPv4 Address Allocation through NAS Signaling (default if omitted)
     * 1 - IPv4 Address Allocated through DHCP
     */
    TAF_PDP_Ipv4AddrAllocTypeUint8 ipv4AddrAlloc;

    /*
     * 0 - PDP context is not for emergency bearer services
     * 1 - PDP context is for emergency bearer services
     */
    TAF_PDP_EmcIndUint8 emergencyInd;

    TAF_PDP_PcscfDiscoveryUint8 pcscfDiscovery;

    TAF_PDP_ImCnSigFlagUint8 imCnSignalFlg;

    TAF_PDP_NasSigPrioIndUint8 nasSigPrioInd;

    TAF_PS_SscModeUint8            sscMode;
    TAF_PS_PrefAccessTypeUint8     prefAccessType;
    TAF_PS_ReflectQosIndUint8      rQosInd;
    TAF_PS_Ipv6MultiHomingIndUint8 mh6Pdu;
    TAF_PS_AlwaysOnIndUint8        alwaysOnInd;
    PS_S_NSSAI_STRU                sNssai;

} TAF_PDP_PrimContext;


typedef struct {
    VOS_UINT8  trafficClass;  /* UMTS承载服务优化类型 */
    VOS_UINT8  deliverOrder;  /* SDU队列是否发送UMTS承载 */
    VOS_UINT8  deliverErrSdu; /* SDU错误是否发送 */
    VOS_UINT8  reserved1[1];
    VOS_UINT16 maxSduSize; /* SDU大小最大值 */
    VOS_UINT8  reserved2[2];
    VOS_UINT32 maxBitUl; /* 上行最大速率 kbits/s */
    VOS_UINT32 maxBitDl; /* 下行最大速率 kbits/s */
    VOS_UINT8  residualBer;
    VOS_UINT8  sduErrRatio; /* SDU误码率 */
    VOS_UINT16 transDelay;  /* 传输延时, 单位毫秒 */
    /* 处理UMTS承载的SDU对比其他承载的SDU */
    VOS_UINT8  traffHandlePrior;
    VOS_UINT8  reserved3[1];
    VOS_UINT8  srcStatisticsDescriptor;
    VOS_UINT8  signallingIndication;
    VOS_UINT32 guarantBitUl; /* 保证上行传输速率 kbits/s */
    VOS_UINT32 guarantBitDl; /* 保证下行传输速率 kbits/s */
} TAF_UMTS_Qos;


typedef struct {
    /*
     * 0 QCI is selected by network
     * [1 - 4]value range for guranteed bit rate Traffic Flows
     * [5 - 9]value range for non-guarenteed bit rate Traffic Flows
     */
    VOS_UINT8 qCI;

    VOS_UINT8 reserved[3];

    /* DL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dLGBR;

    /* UL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 uLGBR;

    /* DL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 dLMBR;

    /* UL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32 uLMBR;
} TAF_EPS_Qos;


typedef struct {
    VOS_UINT32 bitOpRmtIpv4AddrAndMask : 1;
    VOS_UINT32 bitOpRmtIpv6AddrAndMask : 1;
    VOS_UINT32 bitOpProtocolId : 1;
    VOS_UINT32 bitOpSingleLocalPort : 1;
    VOS_UINT32 bitOpLocalPortRange : 1;
    VOS_UINT32 bitOpSingleRemotePort : 1;
    VOS_UINT32 bitOpRemotePortRange : 1;
    VOS_UINT32 bitOpSecuParaIndex : 1;
    VOS_UINT32 bitOpTypeOfService : 1;
    VOS_UINT32 bitOpFlowLabelType : 1;
    VOS_UINT32 bitOpLocalIpv4AddrAndMask : 1;
    VOS_UINT32 bitOpLocalIpv6AddrAndMask : 1;
    VOS_UINT32 bitOpQri : 1;
    VOS_UINT32 bitOpSpare : 19;

    VOS_UINT8                  packetFilterId;
    VOS_UINT8                  nwPacketFilterId;
    TAF_PF_TransDirectionUint8 direction;
    /* packet filter evaluation precedence */
    VOS_UINT8 precedence;

    VOS_UINT32 secuParaIndex; /* SPI */
    VOS_UINT16 singleLcPort;
    VOS_UINT16 lcPortHighLimit;
    VOS_UINT16 lcPortLowLimit;
    VOS_UINT16 singleRmtPort;
    VOS_UINT16 rmtPortHighLimit;
    VOS_UINT16 rmtPortLowLimit;
    VOS_UINT8  protocolId;        /* 协议号 */
    VOS_UINT8  typeOfService;     /* TOS */
    VOS_UINT8  typeOfServiceMask; /* TOS Mask */
    VOS_UINT8  reserved[1];

    /*
     * rmtIpv4Address[0]为IP地址高字节位
     * rmtIpv4Address[3]为低字节位
     */
    VOS_UINT8 rmtIpv4Address[TAF_IPV4_ADDR_LEN];

    /*
     * rmtIpv4Mask[0]为IP地址高字节位 ,
     * rmtIpv4Mask[3]为低字节位
     */
    VOS_UINT8 rmtIpv4Mask[TAF_IPV4_ADDR_LEN];

    /*
     * ucRmtIpv6Address[0]为IPv6接口标识高字节位
     * ucRmtIpv6Address[7]为IPv6接口标识低字节位
     */
    VOS_UINT8 rmtIpv6Address[TAF_IPV6_ADDR_LEN];

    /*
     * ucRmtIpv6Mask[0]为高字节位
     * ucRmtIpv6Mask[7]为低字节位
     */
    VOS_UINT8 rmtIpv6Mask[TAF_IPV6_ADDR_LEN];

    VOS_UINT32 flowLabelType; /* FlowLabelType */

    VOS_UINT8 localIpv4Addr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 localIpv4Mask[TAF_IPV4_ADDR_LEN];
    VOS_UINT8 localIpv6Addr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8 localIpv6Prefix;
    VOS_UINT8 qri;
    VOS_UINT8 reserved2[2];
} TAF_PDP_Pf;


typedef struct {
    VOS_UINT32 pfNum;                  /* pf个数 */
    TAF_PDP_Pf pf[TAF_MAX_SDF_PF_NUM]; /* pf表 */
} TAF_PDP_Tft;


typedef struct {
    VOS_UINT8 pcscfAddr[TAF_IPV4_ADDR_LEN];
} TAF_PDP_Ipv4Pcscf;


typedef struct {
    /* IPV4的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8 ipv4PcscfAddrNum;
    VOS_UINT8 rsv[3]; /* 保留 */

    TAF_PDP_Ipv4Pcscf ipv4PcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_PDP_Ipv4PcscfList;


typedef struct {
    VOS_UINT8 pcscfAddr[TAF_IPV6_ADDR_LEN];
} TAF_PDP_Ipv6Pcscf;


typedef struct {
    /* IPV6的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8 ipv6PcscfAddrNum;
    VOS_UINT8 rsv[3]; /* 保留 */

    TAF_PDP_Ipv6Pcscf ipv6PcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_PDP_Ipv6PcscfList;

typedef struct {
    VOS_UINT32 ulBitL : 1; /* 1: VALID; 0: INVALID */
    VOS_UINT32 ulBitA : 1;
    VOS_UINT32 ulBitPrefixLen : 8;
    VOS_UINT32 ulBitRsv : 22;

    VOS_UINT32 validLifeTime;
    VOS_UINT32 preferredLifeTime;
    VOS_UINT8  prefix[TAF_IPV6_ADDR_LEN];
} TAF_PDP_Ipv6Prefix;


typedef struct {
    VOS_UINT32 bitOpMtu : 1;
    VOS_UINT32 bitOpSpare : 31;

    VOS_UINT32 ulBitCurHopLimit : 8;
    VOS_UINT32 ulBitM : 1;
    VOS_UINT32 ulBitO : 1;
    VOS_UINT32 ulBitRsv : 22;

    VOS_UINT32         mtu;                                  /* MTU大小 */
    VOS_UINT32         prefixNum;                            /* IPV6地址前缀个数 */
    TAF_PDP_Ipv6Prefix prefixList[TAF_MAX_PREFIX_NUM_IN_RA]; /* IPV6地址前缀数组 */

} TAF_PDP_Ipv6RaInfo;


typedef struct {
    TAF_PDP_AuthTypeUint8 authType;
    VOS_UINT8             plmn[TAF_MAX_AUTHDATA_PLMN_LEN + 1];
    VOS_UINT8             password[TAF_MAX_AUTHDATA_PASSWORD_LEN + 1];
    VOS_UINT8             username[TAF_MAX_AUTHDATA_USERNAME_LEN + 1];
} TAF_PDP_Authdata;

typedef struct {
    VOS_UINT32 bitOpAddr : 1;
    VOS_UINT32 bitOpSpare : 31;

    VOS_UINT8 addr[TAF_IPV4_ADDR_LEN];
} TAF_PDP_Ipv4Addr;

typedef struct {
    VOS_UINT32 bitOpAddr : 1;
    VOS_UINT32 bitOpSpare : 31;

    VOS_UINT8 addr[TAF_IPV6_ADDR_LEN];
} TAF_PDP_Ipv6Addr;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of Taf_Aps.h */
