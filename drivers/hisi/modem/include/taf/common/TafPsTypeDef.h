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


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/* TAF下数据业务的消息分配范围: 0 ~ 0x1000, 见文件TafMntn.h TAF_MNTN_MSG_ENUM_UINT32

   DSM与APS数据业务模块间的消息分配范围原则限定: 高8位为base，低8位为消息标识
*/
#define TAF_PS_MSG_ID_MASK                                  (0x1F00)

#define TAF_PS_MSG_ID_BASE                                  (0x0000)
#define TAF_PS_APP_PROFILE_ID_BASE                          (0x0100)
#define TAF_PS_APS_INTERNAL_ID_BASE                         (0x0700)
#define TAF_PS_MNTN_ID_BASE                                 (0x0770)
#define TAF_PS_EVT_ID_BASE                                  (0x0800)
#define TAF_PS_DSM_APS_ID_BASE                              (0x0B00)
#define TAF_PS_APS_DSM_ID_BASE                              (0x0C00)

#define TAF_PS_INVALID_SNSSAI                               (0xffffffff)
/* "a1...a16" */

#define TAF_MAX_PREFIX_NUM_IN_RA                            (6)

/* "a1.a2.a3.a4 " */
#define TAF_IPV4_ADDR_LEN                                   (4)
#define TAF_MAX_APN_LEN                                     (99)
#define TAF_IPV6_ADDR_LEN                                   (16)
#define TAF_MAX_SDF_PF_NUM                                  (16)
#define TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM                 (3)
#define TAF_PS_MAX_CUSTOM_PCO_CONTAINER_CONTENT_LEN         (53)
/*****************************************************************************
  Maximum length of challenge used in authentication. The maximum length
  challenge name is the same as challenge.
*****************************************************************************/
#define TAF_PS_PPP_CHAP_CHALLNGE_LEN                        (16)

/*****************************************************************************
  Maximum length of challenge name used in authentication.
*****************************************************************************/
#define TAF_PS_PPP_CHAP_CHALLNGE_NAME_LEN                   (16)

/*****************************************************************************
  Maximum length of response used in authentication.
*****************************************************************************/
#define TAF_PS_PPP_CHAP_RESPONSE_LEN                        (16)

#define TAF_MAX_USERNAME_LEN                                (99)                                    /*AUTHLEN 为100，包含1个字节的Peer-ID Length和99字节的Peer-ID，故此处定义为99*/
#define TAF_MAX_PASSCODE_LEN                                (99)                                    /*PASSWORDLEN 为100，包含1个字节的Passwd-Length和99字节的Passwd，故此处定义为99*/

/* UE支持的最大的P-CSCF地址个数 */
#define TAF_PCSCF_ADDR_MAX_NUM                              (8)

#define TAF_PS_CAUSE_APS_SECTION_BEGIN                      (0x0000)
#define TAF_PS_CAUSE_DSM_SECTION_BEGIN                      (0x0040)
#define TAF_PS_CAUSE_SM_SECTION_BEGIN                       (0x0080)
#define TAF_PS_CAUSE_SM_NW_SECTION_BEGIN                    (0x0100)
#define TAF_PS_CAUSE_SM_NW_SECTION_END                      (0x01FF)
#define TAF_PS_CAUSE_GMM_SECTION_BEGIN                      (0x0200)
#define TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN                   (0x0300)
#define TAF_PS_CAUSE_XCC_SECTION_BEGIN                      (0x0400)
#define TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN                   (0x0500)
#define TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN                   (0x0600)
#define TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN                (0X0700)
#define TAF_PS_CAUSE_PPPC_SECTION_BEGIN                     (0X0900)
#define TAF_PS_CAUSE_MIP_SECTION_BEGIN                      (0X0A00)

/* 偏移量0X0800已被占用，ril侧将对0 ~ 255的内部原因值往后偏移0X0800 */

#define TAF_PS_CAUSE_EMM_SECTION_BEGIN                      (0X0B00)
#define TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN                   (0X0C00)

#define TAF_PS_CAUSE_WLAN_SECTION_BEGIN                     (0X0D00)

#define TAF_PS_CAUSE_NRSM_SECTION_BEGIN                     (0X0E00)
#define TAF_PS_CAUSE_NRMM_SECTION_BEGIN                     (0X0F00)


/* ^AUTHDATA */
#define TAF_MAX_AUTHDATA_USERNAME_LEN                       (99)
#define TAF_MAX_AUTHDATA_PASSWORD_LEN                       (99)
#define TAF_MAX_AUTHDATA_PLMN_LEN                           (6)

#define TAF_CGACT_DEACT                                     (0)
#define TAF_CGACT_ACT                                       (1)

/* 目前先放在这里，等后面APS代码删除调整之后，更名 */
#define TAF_PS_PPP_IP_ADDR_ID                               0x03
#define TAF_PS_PPP_PRI_DNS_ID                               0x81
#define TAF_PS_PPP_PRI_NBNS_ID                              0x82
#define TAF_PS_PPP_SEC_DNS_ID                               0x83
#define TAF_PS_PPP_SEC_NBNS_ID                              0x84

/*PCO头的长度*/
#define TAF_PS_PCO_CONFIG_HDR_LEN                           (4)

/*IP地址长度*/
#define TAF_PS_PPP_IP_ADDR_LEN                              (6)

/*主NBNS地址长度*/
#define TAF_PS_PPP_PRI_NBNS_LEN                             (6)

/*主DNS地址长度*/
#define TAF_PS_PPP_PRI_DNS_LEN                              (6)

/*副NBNS地址长度*/
#define TAF_PS_PPP_SEC_NBNS_LEN                             (6)

/*副NBNS地址长度*/
#define TAF_PS_PPP_SEC_DNS_LEN                              (6)

/***************************************************************
AUTH_PAP (RFC 1334)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Code      |  Identifier   |            Length             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Peer-ID Length|  Peer-Id ...
+-+-+-+-+-+-+-+-+-+-+-+-+
| Passwd-Length |  Password ...
+-+-+-+-+-+-+-+-+-+-+-+-+-+
****************************************************************/
/* code(1B)+id(1B)+length(2B)+username_size(1B)+username+password_size(1B)+password */
#define TAF_PS_PPP_PAP_CODE_LEN                             1

#define TAF_PS_PPP_PAP_HEAD_OFFSET                          4

/* code(1B)+id(1B)+length(2B)+challenge_size(1B)+challenge+name */
/* code(1B)+id(1B)+length(2B)+response_size(1B)+response+name */
#define TAF_PS_PPP_CHAP_CODE_LEN                            1
#define TAF_PS_PPP_CHAP_ID_LEN                              1
#define TAF_PS_PPP_CHAP_LEN                                 2
#define TAF_PS_PPP_CHAP_CHALLENGE_SIZE                      1
#define TAF_PS_PPP_CHAP_RESPONSE_SIZE                       1

#define TAF_PS_PPP_CHAP_HEAD_OFFSET                         (TAF_PS_PPP_CHAP_CODE_LEN + TAF_PS_PPP_CHAP_ID_LEN + TAF_PS_PPP_CHAP_LEN)

#define TAF_PS_PPP_CHAP_CHALLENGE_HEAD_OFFSET               (TAF_PS_PPP_CHAP_HEAD_OFFSET + TAF_PS_PPP_CHAP_CHALLENGE_SIZE)
#define TAF_PS_PPP_CHAP_RESPONSE_HEAD_OFFSET                (TAF_PS_PPP_CHAP_HEAD_OFFSET + TAF_PS_PPP_CHAP_RESPONSE_SIZE)

/*--------------------------------------------------------------
   IPCP OPTION长度必须等于6
   OPTION TYPE(1 byte) + OPTION LEN(1 btye) + VALUE(4 bytes)
--------------------------------------------------------------*/
#define TAF_PS_PPP_IPCP_OPTION_TYPE_LEN                     1
#define TAF_PS_PPP_IPCP_OPTION_LEN                          1

#define TAF_PS_PPP_IPCP_OPTION_HEAD_LEN                     (TAF_PS_PPP_IPCP_OPTION_TYPE_LEN + TAF_PS_PPP_IPCP_OPTION_LEN)

#define TAF_PS_PPP_IPCP_OPTION_HEAD_LEN_OFFSET              (TAF_PS_PPP_IPCP_OPTION_HEAD_LEN + TAF_PS_PPP_IPCP_OPTION_HEAD_LEN)

#define TAF_PS_PPP_CODE_REQ                                 1
#define TAF_PS_PPP_CODE_ACK                                 2
#define TAF_PS_PPP_CODE_NAK                                 3
#define TAF_PS_PPP_CODE_REJ                                 4

#define TAF_PS_INVALID_CID                                  (0xFF)                  /* 无效的CID */


#define TAF_PS_3GPP_QOS_MEANTHROUGH_TOP                     18
#define TAF_PS_3GPP_QOS_MEANTHROUGH_BESTEFFORT              31
#define TAF_PS_3GPP_QOS_MEANTHROUGH_BOT                     0

/* 终端可配置需求:2G拨号请求禁止Spare_bit3等参数 */
#define TAF_PS_2G_BIT3_DISABLE                              0    /*2G拨号时PDP激活消息携带Spare_bit3等参数  */
#define TAF_PS_2G_BIT3_ENABLE                               1    /*2G拨号时PDP激活请求消息不携带bit3等参数  */


#define TAF_PS_PDU_SESSION_ID_MIN                           (5)
#define TAF_PS_PDU_SESSION_ID_MAX                           (15)
#define TAF_PS_INVALID_PDU_SESSION_ID                       (0)


#define TAF_MAX_IPV4_EPDG_NUM                       (2)
#define TAF_MAX_IPV6_EPDG_NUM                       (2)

#define TAF_PS_NSSAI_MAX_NUM                                (8)
#define TAF_PS_INVALID_IFACE_ID                             (0)



enum TAF_PDP_ACCESS_TYPE_ENUM
{

    TAF_PDP_ACCESS_TYPE_3GPP                                = 0x0000,          /* */
    TAF_PDP_ACCESS_TYPE_NON_3GPP                            = 0x0001,          /* */

    TAF_PDP_ACCESS_TYPE_BUTT

};
typedef VOS_UINT8 TAF_PS_PDP_ACCESS_TYPE_ENUM_UINT8;


enum TAF_PDP_REQUEST_TYPE_ENUM
{
    TAF_PDP_REQUEST_TYPE_INITIAL        = 0x1,
    TAF_PDP_REQUEST_TYPE_HANDOVER       = 0x2,
    TAF_PDP_REQUEST_TYPE_UNUSED         = 0x3,
    TAF_PDP_REQUEST_TYPE_EMERGENCY      = 0x4,
    TAF_PDP_REQUEST_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PDP_REQUEST_TYPE_ENUM_UINT8;

/*****************************************************************************
 枚举名称: TAF_PDP_AUTH_TYPE_ENUM

 枚举说明: 鉴权类型
*****************************************************************************/
enum TAF_PDP_AUTH_TYPE_ENUM
{
    TAF_PDP_AUTH_TYPE_NONE              = 0x00,
    TAF_PDP_AUTH_TYPE_PAP               = 0x01,
    TAF_PDP_AUTH_TYPE_CHAP              = 0x02,

    TAF_PDP_AUTH_TYPE_PAP_OR_CHAP       = 0x03,

    TAF_PDP_AUTH_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PDP_AUTH_TYPE_ENUM_UINT8;


enum TAF_PDP_H_COMP_ENUM
{
    TAF_PDP_H_COMP_OFF                  = 0x00,             /* default if value is omitted */
    TAF_PDP_H_COMP_ON                   = 0x01,             /* manufacturer preferred compression */
    TAF_PDP_H_COMP_RFC1144              = 0x02,             /* RFC 1144 */
    TAF_PDP_H_COMP_RFC2507              = 0x03,             /* RFC 2507 */

    TAF_PDP_H_COMP_BUTT                 = 0xFF
};
typedef VOS_UINT8 TAF_PDP_H_COMP_ENUM_UINT8;


enum TAF_PDP_D_COMP_ENUM
{
    TAF_PDP_D_COMP_OFF                  = 0x00,             /* default if value is omitted */
    TAF_PDP_D_COMP_ON                   = 0x01,             /* manufacturer preferred compression */
    TAF_PDP_D_COMP_V42                  = 0x02,             /* V.42 compression */

    TAF_PDP_D_COMP_BUTT                 = 0xFF
};
typedef VOS_UINT8 TAF_PDP_D_COMP_ENUM_UINT8;


enum TAF_PDP_IPV4_ADDR_ALLOC_TYPE_ENUM
{
    TAF_PDP_IPV4_ADDR_ALLOC_TYPE_NAS    = 0x00,
    TAF_PDP_IPV4_ADDR_ALLOC_TYPE_DHCP   = 0x01,

    TAF_PDP_IPV4_ADDR_ALLOC_BUTT        = 0xFF
};
typedef VOS_UINT8 TAF_PDP_IPV4_ADDR_ALLOC_TYPE_ENUM_UINT8;


enum TAF_PDP_EMC_IND_ENUM
{
    TAF_PDP_NOT_FOR_EMC                 = 0x00,
    TAF_PDP_FOR_EMC                     = 0x01,

    TAF_PDP_EMC_IND_BUTT                = 0xFF
};
typedef VOS_UINT8 TAF_PDP_EMC_IND_ENUM_UINT8;


enum TAF_PDP_PCSCF_DISCOVERY_ENUM
{
    TAF_PDP_PCSCF_DISCOVERY_NOT_INFLUENCED  = 0x00,
    TAF_PDP_PCSCF_DISCOVERY_THROUGH_NAS_SIG = 0x01,
    TAF_PDP_PCSCF_DISCOVERY_THROUGH_DHCP    = 0x02,

    TAF_PDP_PCSCF_DISCOVERY_BUTT            = 0xFF
};
typedef VOS_UINT8 TAF_PDP_PCSCF_DISCOVERY_ENUM_UINT8;


enum TAF_PDP_IM_CN_SIG_FLAG_ENUM
{
    TAF_PDP_NOT_FOR_IMS_CN_SIG_ONLY     = 0x00,
    TAF_PDP_FOR_IM_CN_SIG_ONLY          = 0x01,

    TAF_PDP_IM_CN_SIG_FLAG_BUTT         = 0xFF
};
typedef VOS_UINT8 TAF_PDP_IM_CN_SIG_FLAG_ENUM_UINT8;

/*****************************************************************************
 结构名称   : TAF_NV_PDP_NSPLI_ENUM
 结构说明   :
*****************************************************************************/
enum TAF_NV_PDP_NSPLI_ENUM
{
    TAF_NV_PDP_NAS_SIG_NORMAL_PRIO      = 0x00,
    TAF_NV_PDP_NAS_SIG_LOW_PRIO         = 0x01,

    TAF_NV_PDP_NAS_SIG_PRIO_BUTT        = 0xFF
};
typedef VOS_UINT8 TAF_NV_PDP_NSPLI_ENUM_UINT8;

enum TAF_PDP_NAS_SIG_PRIO_IND_ENUM
{
    TAF_PDP_NAS_SIG_LOW_PRIO            = 0x00,
    TAF_PDP_NAS_SIG_NORMAL_PRIO         = 0x01,

    TAF_PDP_NAS_SIG_PRIO_BUTT           = 0xFF
};
typedef VOS_UINT8 TAF_PDP_NAS_SIG_PRIO_IND_ENUM_UINT8;



enum TAF_PS_RAT_TYPE_ENUM
{
    TAF_PS_RAT_TYPE_NULL,          /* 当前未在任何网络上驻留 */
    TAF_PS_RAT_TYPE_GSM,           /* GSM接入技术 */
    TAF_PS_RAT_TYPE_WCDMA,         /* WCDMA接入技术 */
    TAF_PS_RAT_TYPE_LTE,           /* LTE接入技术 */
    TAF_PS_RAT_TYPE_1X,            /* CDMA-1X接入技术 */
    TAF_PS_RAT_TYPE_HRPD,          /* CDMA-HRPD接入技术 */
    TAF_PS_RAT_TYPE_EHRPD,         /* CDMA-EHRPD接入技术 */
    TAF_PS_RAT_TYPE_NR,            /* NR核心网接入技术 */
    TAF_PS_RAT_TYPE_BUTT           /* 无效的接入技术 */
};
typedef VOS_UINT32 TAF_PS_RAT_TYPE_ENUM_UINT32;


enum TAF_PDP_ANSWER_MODE_ENUM
{
    TAF_PDP_ANSWER_MODE_MANUAL             = 0x00,             /* 人工应答方式 */
    TAF_PDP_ANSWER_MODE_AUTO               = 0x01,             /* 自动应答方式 */

    TAF_PDP_ANSWER_MODE_BUTT
};
typedef VOS_UINT8 TAF_PDP_ANSWER_MODE_ENUM_UINT8;


enum TAF_PDP_ANSWER_TYPE_ENUM
{
    TAF_PDP_ANSWER_TYPE_ACCEPT             = 0x00,             /* 接受呼叫 */
    TAF_PDP_ANSWER_TYPE_REJECT             = 0x01,             /* 拒绝呼叫 */

    TAF_PDP_ANSWER_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PDP_ANSWER_TYPE_ENUM_UINT8;


enum TAF_PS_CDATA_DIAL_MODE_ENUM
{
    TAF_PS_CDATA_DIAL_MODE_ASYNC_CALL   = 0x00,
    TAF_PS_CDATA_DIAL_MODE_RELAY        = 0x01,                                 /* 数据传输模式中继模式 */
    TAF_PS_CDATA_DIAL_MODE_NETWORK      = 0x02,                                 /* 数据传输模式网络模式  */
    TAF_PS_CDATA_DIAL_MODE_BUTT
};
typedef VOS_UINT32 TAF_PS_CDATA_DIAL_MODE_ENUM_UINT32;

/*****************************************************************************
 枚举名称: TAF_PDP_DIAL_TYPE_ENUM

 枚举说明: 鉴权类型
*****************************************************************************/
enum TAF_PDP_DIAL_TYPE_ENUM
{
    TAF_PDP_DAIL_TYPE_NDISDUP                               = 0x00,
    TAF_PDP_DAIL_TYPE_CGACT                                 = 0x01,
    TAF_PDP_DAIL_TYPE_PPP                                   = 0x02,

    TAF_PDP_DAIL_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PS_PDP_DIAL_TYPE_ENUM_UINT8;


enum TAF_PS_CAUSE_ENUM
{
    /*----------------------------------------------------------------------
       TAF上报的内部原因值, 取值范围[0x0000, 0x007F]
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_SUCCESS                                    = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 0),
    TAF_PS_CAUSE_INVALID_PARAMETER                          = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_CID_INVALID                                = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_PDP_ACTIVATE_LIMIT                         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_SIM_INVALID                                = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_APS_TIME_OUT                               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_OPERATION_CONFLICT                         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_XCC_TIME_OUT                               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_PPP_TIME_OUT                               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_PPP_NEGOTIATE_FAIL                         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_1X_NO_SERVICE                              = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_HRPD_NO_SERVICE                            = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_HSM_TIME_OUT                               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_IP_ADDRESS_CHANGE                          = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_EHSM_TIME_OUT                              = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_RAT_TYPE_CHANGE                            = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_DHCP_TIME_OUT                              = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_PDP_TYPE_IPV4_ONLY_ALLOWED                 = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_PDP_TYPE_IPV6_ONLY_ALLOWED                 = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_PPP_NW_DISC                                = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_UNSUPPORT_PCSCF                            = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_SYSCFG_MODE_CHANGE                         = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 21),

    TAF_PS_CAUSE_L2C_HANDOVER_FAIL                          = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_NOT_SUPPORT_CONCURRENT                     = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_READ_3GPD_FILE                             = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_1X_HRPD_NOT_SUPPORT_MUTI_PDN               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED                    = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 26),
    TAF_PS_CAUSE_1X_DO_NO_SERVICE                           = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 27),
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_IGNORE             = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 28),
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_PERM               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 29),
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_TEMP               = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 30),
    TAF_PS_CAUSE_BACKOFF_ALG_NOT_ALLOWED_RSV                = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 31),

    TAF_PS_CAUSE_IP_TYPE_CHANGE                             = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 32),

    TAF_PS_CAUSE_MIP_DISABLE                                = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 33),

    TAF_PS_CAUSE_IPV6_ADDRESS_DISABLE                       = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 34),
    TAF_PS_CAUSE_NOT_SUPPROT_OPERATION                      = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 35),
    TAF_PS_CAUSE_THROT_ALG_NOT_ALLOWED                      = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 36),

    TAF_PS_CAUSE_SYSTEM_CHG_TO_LTE                          = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 37),
    TAF_PS_CAUSE_SYSTEM_CHG_TO_EHRPD                        = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 38),
    TAF_PS_CAUSE_INTERNAL_DATA_SWITCH_OFF                   = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 39),

    TAF_PS_CAUSE_NO_AVAILABLE_DOMAIN                        = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 40),
    TAF_PS_CAUSE_IP_ADDRESS_CHG_IN_HANDOVER                 = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 41),

    TAF_PS_CAUSE_UNKNOWN                                    = (TAF_PS_CAUSE_APS_SECTION_BEGIN + 63),

    TAF_PS_CAUSE_DSM_INVALID_PARAMETER                      = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_DSM_CID_INVALID                            = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_DSM_TIME_OUT                               = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_DSM_OPERATION_CONFLICT                     = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_DSM_MIP_DISABLE                            = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_DSM_POWER_OFF                              = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_DSM_SESSION_ENTITY_ALLOC_FAIL              = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_DSM_UNKNOWN                                = (TAF_PS_CAUSE_DSM_SECTION_BEGIN + 63),


    /*----------------------------------------------------------------------
       TAF上报的SM内部原因值, 取值范围[0x0080, 0x00FF]
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_SM_CONN_ESTABLISH_MAX_TIME_OUT             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_SM_MAX_TIME_OUT                            = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_SM_INVALID_NSAPI                           = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_SM_MODIFY_COLLISION                        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_SM_DUPLICATE                               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_SM_RAB_SETUP_FAILURE                       = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_SM_SGSN_VER_PRE_R99                        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_SM_FAILURE                                 = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_SM_ERR_UNSPECIFIED_ERROR                   = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_SM_ERR_INSUFFICIENT_RESOURCES              = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_SM_CALL_CID_INVALID                        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_SM_CALL_CID_ACTIVE                         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_SM_CALL_CID_NOT_MATCH_BEARER               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_SM_BEARER_TYPE_NOT_DEDICATED               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_SM_BEARER_INACTIVE                         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_SM_LINK_CID_INVALID                        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_SM_LINK_BEARER_INACTIVE                    = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_SM_APN_LEN_ILLEGAL                         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_SM_APN_SYNTACTICAL_ERROR                   = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_SM_AUTH_TYPE_ILLEGAL                       = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_SM_USER_NAME_TOO_LONG                      = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_SM_USER_PASSWORD_TOO_LONG                  = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_SM_ACCESS_NUM_TOO_LONG                     = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_SM_CALL_CID_IN_OPERATION                   = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_SM_BEARER_TYPE_NOT_DEFAULT                 = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_SM_BEARER_TYPE_ILLEGAL                     = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 26),
    TAF_PS_CAUSE_SM_MUST_EXIST_DEFAULT_TYPE_CID             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 27),
    TAF_PS_CAUSE_SM_PDN_TYPE_ILLEGAL                        = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 28),
    TAF_PS_CAUSE_SM_IPV4_ADDR_ALLOC_TYPE_ILLEGAL            = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 29),
    TAF_PS_CAUSE_SM_SUSPENDED                               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 30),
    TAF_PS_CAUSE_SM_MULTI_EMERGENCY_NOT_ALLOWED             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 31),
    TAF_PS_CAUSE_SM_NON_EMERGENCY_NOT_ALLOWED               = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 32),
    TAF_PS_CAUSE_SM_MODIFY_EMERGENCY_NOT_ALLOWED            = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 33),
    TAF_PS_CAUSE_SM_DEDICATED_FOR_EMERGENCY_NOT_ALLOWED     = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 34),
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED                 = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 35),
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_IGNORE          = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 36),
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_PERM            = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 37),
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_TEMP            = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 38),
    TAF_PS_CAUSE_SM_BACKOFF_ALG_NOT_ALLOWED_RSV             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 39),
    TAF_PS_CAUSE_SM_MM_NOT_SYNC                             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 40),
    TAF_PS_CAUSE_SM_NW_RECONFIGURE                          = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 41),
    TAF_PS_CAUSE_SM_DELETE_SDF                              = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 42),

    TAF_PS_CAUSE_SM_NOT_IN_ACL_LIST                         = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 43),
    TAF_PS_CAUSE_SM_APN_DISABLE                             = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 44),
    TAF_PS_CAUSE_SM_BEARER_ALLOC_NOT_ALLOWED                = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 45),
    TAF_PS_CAUSE_SM_THROT_ALG_NOT_ALLOWED                   = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 46),
    TAF_PS_CAUSE_SM_THROT_T3396_IS_RUNNING                  = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 47),
    TAF_PS_CAUSE_SM_THROT_BACKOFF_IS_RUNNING                = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 48),

    TAF_PS_CAUSE_SM_UNKNOWN                                 = (TAF_PS_CAUSE_SM_SECTION_BEGIN + 127),

    /*----------------------------------------------------------------------
       TAF上报的SM网络原因值, 取值范围[0x0100, 0x01FF]
       由于3GPP协议已经定义了具体的(E)SM网络原因值, (E)SM上报的取值为协议
       定义的原因值加上偏移量(0x100)
       (1) SM Cause : Refer to TS 24.008 section 10.5.6.6
       (2) ESM Cause: Refer to TS 24.301 section 9.9.4.4
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_SM_NW_OPERATOR_DETERMINED_BARRING          = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_SM_NW_MBMS_BC_INSUFFICIENT                 = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_SM_NW_LLC_OR_SNDCP_FAILURE                 = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_SM_NW_INSUFFICIENT_RESOURCES               = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 26),
    TAF_PS_CAUSE_SM_NW_MISSING_OR_UKNOWN_APN                = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 27),
    TAF_PS_CAUSE_SM_NW_UNKNOWN_PDP_ADDR_OR_TYPE             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 28),
    TAF_PS_CAUSE_SM_NW_USER_AUTHENTICATION_FAIL             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 29),
    TAF_PS_CAUSE_SM_NW_ACTIVATION_REJECTED_BY_GGSN_SGW_OR_PGW = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 30),
    TAF_PS_CAUSE_SM_NW_ACTIVATION_REJECTED_UNSPECIFIED      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 31),
    TAF_PS_CAUSE_SM_NW_SERVICE_OPTION_NOT_SUPPORTED         = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 32),
    TAF_PS_CAUSE_SM_NW_REQUESTED_SERVICE_NOT_SUBSCRIBED     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 33),
    TAF_PS_CAUSE_SM_NW_SERVICE_OPTION_TEMP_OUT_ORDER        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 34),
    TAF_PS_CAUSE_SM_NW_NSAPI_ALREADY_USED                   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 35),
    TAF_PS_CAUSE_SM_NW_REGULAR_DEACTIVATION                 = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 36),
    TAF_PS_CAUSE_SM_NW_QOS_NOT_ACCEPTED                     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 37),
    TAF_PS_CAUSE_SM_NW_NETWORK_FAILURE                      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 38),
    TAF_PS_CAUSE_SM_NW_REACTIVATION_REQUESTED               = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 39),
    TAF_PS_CAUSE_SM_NW_FEATURE_NOT_SUPPORT                  = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 40),
    TAF_PS_CAUSE_SM_NW_SEMANTIC_ERR_IN_TFT                  = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 41),
    TAF_PS_CAUSE_SM_NW_SYNTACTIC_ERR_IN_TFT                 = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 42),
    TAF_PS_CAUSE_SM_NW_UKNOWN_PDP_CONTEXT                   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 43),
    TAF_PS_CAUSE_SM_NW_SEMANTIC_ERR_IN_PACKET_FILTER        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 44),
    TAF_PS_CAUSE_SM_NW_SYNCTACTIC_ERR_IN_PACKET_FILTER      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 45),
    TAF_PS_CAUSE_SM_NW_PDP_CONTEXT_WITHOUT_TFT_ACTIVATED    = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 46),
    TAF_PS_CAUSE_SM_NW_MULTICAST_GROUP_MEMBERHHSHIP_TIMEOUT = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 47),
    TAF_PS_CAUSE_SM_NW_REQUEST_REJECTED_BCM_VIOLATION       = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 48),
    TAF_PS_CAUSE_SM_NW_LAST_PDN_DISCONN_NOT_ALLOWED         = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 49),
    TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV4_ONLY_ALLOWED           = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 50),
    TAF_PS_CAUSE_SM_NW_PDP_TYPE_IPV6_ONLY_ALLOWED           = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 51),
    TAF_PS_CAUSE_SM_NW_SINGLE_ADDR_BEARERS_ONLY_ALLOWED     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 52),
    TAF_PS_CAUSE_SM_NW_INFORMATION_NOT_RECEIVED             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 53),
    TAF_PS_CAUSE_SM_NW_PDN_CONNECTION_DOES_NOT_EXIST        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 54),
    TAF_PS_CAUSE_SM_NW_SAME_APN_MULTI_PDN_CONNECTION_NOT_ALLOWED = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 55),
    TAF_PS_CAUSE_SM_NW_COLLISION_WITH_NW_INITIATED_REQUEST  = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 56),
    TAF_PS_CAUSE_SM_NW_UNSUPPORTED_QCI_VALUE                = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 59),
    TAF_PS_CAUSE_SM_NW_BEARER_HANDLING_NOT_SUPPORTED        = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 60),
    TAF_PS_CAUSE_SM_NW_MAXIMUM_NUMBER_OF_EPS_BEARERS_REACHED     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 65),
    TAF_PS_CAUSE_SM_NW_APN_NOT_SUPPORT_IN_CURRENT_RAT_AND_PLMN   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 66),
    TAF_PS_CAUSE_SM_NW_INSUFFICIENT_RESOURCES_FOR_SPC_SLICE_AND_DNN     = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 67),
    TAF_PS_CAUSE_SM_NW_NOT_SUPPORTED_SSC_MODE               = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 68),
    TAF_PS_CAUSE_SM_NW_INSUFFICIENT_RESOURCES_FOR_SPC_SLICE = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 69),
    TAF_PS_CAUSE_SM_NW_INVALID_TI                           = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 81),
    TAF_PS_CAUSE_SM_NW_SEMANTICALLY_INCORRECT_MESSAGE       = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 95),
    TAF_PS_CAUSE_SM_NW_INVALID_MANDATORY_INFO               = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 96),
    TAF_PS_CAUSE_SM_NW_MSG_TYPE_NON_EXISTENT                = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 97),
    TAF_PS_CAUSE_SM_NW_MSG_TYPE_NOT_COMPATIBLE              = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 98),
    TAF_PS_CAUSE_SM_NW_IE_NON_EXISTENT                      = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 99),
    TAF_PS_CAUSE_SM_NW_CONDITIONAL_IE_ERR                   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 100),
    TAF_PS_CAUSE_SM_NW_MSG_NOT_COMPATIBLE                   = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 101),
    TAF_PS_CAUSE_SM_NW_PROTOCOL_ERR_UNSPECIFIED             = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 111),
    TAF_PS_CAUSE_SM_NW_APN_RESTRICTION_INCOMPATIBLE         = (TAF_PS_CAUSE_SM_NW_SECTION_BEGIN + 112),

    /*----------------------------------------------------------------------
       TAF上报的GMM内部原因值, 取值范围[0x0200, 0x02FF]
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_GMM_GPRS_NOT_SUPPORT                       = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_GMM_FORBID_LA                              = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_GMM_AUTHENTICATION_FAIL                    = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_GMM_PS_DETACH                              = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_GMM_ACCESS_BARRED                          = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_GMM_NO_PDP_CONTEXT                         = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_GMM_ATTACH_MAX_TIMES                       = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_GMM_RRC_EST_FAIL                           = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_GMM_T3310_EXPIRED                          = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_GMM_T3317_EXPIRED                          = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_GMM_TIMER_SIGNALING_PROTECT_EXPIRED        = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_GMM_NULL                                   = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_GMM_DETACH_NOT_REATTACH                    = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_GMM_RPM_FORBID_ATTACH                      = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_GMM_PS_SERVICE_NOT_TRIG_REG                = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_GMM_PS_SERVICE_NOT_TRIG_REG_T3302RUNNING   = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_GMM_SERVICE_FORBID                         = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_GMM_PS_SERVICE_NOT_TRIG_REG_T3311RUNNING   = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_GMM_UNKNOWN                                = (TAF_PS_CAUSE_GMM_SECTION_BEGIN + 127),

    /*----------------------------------------------------------------------
       TAF上报的GMM网络原因值, 取值范围[0x0300, 0x03FF]
       由于3GPP协议已经定义了具体的GMM网络原因值, 平台上报的GMM网络原因值
       取值为协议定义的原因值加上偏移量(0x100)
       Gmm Cause: Refer to TS 24.008 section 10.5.5.14
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_GMM_NW_IMSI_UNKNOWN_IN_HLR                 = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_GMM_NW_ILLEGAL_MS                          = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_GMM_NW_IMSI_NOT_ACCEPTED                   = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_GMM_NW_ILLEGAL_ME                          = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_GMM_NW_GPRS_SERV_NOT_ALLOW                 = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_GMM_NW_GPRS_SERV_AND_NON_GPRS_SERV_NOT_ALLOW = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_GMM_NW_MS_ID_NOT_DERIVED                   = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_GMM_NW_IMPLICIT_DETACHED                   = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_GMM_NW_PLMN_NOT_ALLOW                      = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_GMM_NW_LA_NOT_ALLOW                        = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_GMM_NW_ROAMING_NOT_ALLOW_IN_LA             = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_GMM_NW_GPRS_SERV_NOT_ALLOW_IN_PLMN         = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_GMM_NW_NO_SUITABL_CELL                     = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_GMM_NW_MSC_UNREACHABLE                     = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_GMM_NW_NETWORK_FAILURE                     = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_GMM_NW_MAC_FAILURE                         = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_GMM_NW_SYNCH_FAILURE                       = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_GMM_NW_PROCEDURE_CONGESTION                = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_GMM_NW_GSM_AUT_UNACCEPTABLE                = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_GMM_NW_NOT_AUTHORIZED_FOR_THIS_CSG         = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_GMM_NW_NO_PDP_CONTEXT_ACT                  = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 40),
    TAF_PS_CAUSE_GMM_NW_RETRY_UPON_ENTRY_CELL               = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 60),
    TAF_PS_CAUSE_MM_NW_5GMM_MESSAGE_WAS_NOT_FORWARDED       = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 90),
    TAF_PS_CAUSE_GMM_NW_SEMANTICALLY_INCORRECT_MSG          = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 95),
    TAF_PS_CAUSE_GMM_NW_INVALID_MANDATORY_INF               = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 96),
    TAF_PS_CAUSE_GMM_NW_MSG_NONEXIST_NOTIMPLEMENTE          = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 97),
    TAF_PS_CAUSE_GMM_NW_MSG_TYPE_NOT_COMPATIBLE             = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 98),
    TAF_PS_CAUSE_GMM_NW_IE_NONEXIST_NOTIMPLEMENTED          = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 99),
    TAF_PS_CAUSE_GMM_NW_CONDITIONAL_IE_ERROR                = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 100),
    TAF_PS_CAUSE_GMM_NW_MSG_NOT_COMPATIBLE                  = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 101),
    TAF_PS_CAUSE_GMM_NW_PROTOCOL_ERROR                      = (TAF_PS_CAUSE_GMM_NW_SECTION_BEGIN + 111),

    /*----------------------------------------------------------------------
       TAF上报的CDMA 1X网络原因值, 取值范围[0x400, 0x04FF]
       取值为协议定义的原因值加上偏移量(0x400)
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_XCC_OTHER_SERVICE_IN_TCH                   = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_XCC_CCS_NOT_SUPPORT                        = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_XCC_CONNECT_ORDER_ACK_FAILURE              = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_XCC_INCOMING_RSP_TIME_OUT                  = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_XCC_L2_ACK_TIME_OUT                        = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_XCC_POWER_DOWN_IND                         = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_XCC_CONNID_NOT_FOUND                       = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_XCC_ACCESS_CNF_TIMEOUT                     = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_XCC_ACCESS_TCH_TIMEOUT                     = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_XCC_T53M_TIME_OUT                          = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_XCC_XCALL_HANGUP                           = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_XCC_CALL_NOT_ALLOWED                       = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_XCC_ABNORMAL_STATE                         = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_XCC_ENCODE_FAIL                            = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_XCC_WAIT_L2_ACK_CALL_RELEASE               = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_XCC_CANNOT_FIND_CALL_INSTANCE              = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_XCC_ABORTED_BY_RECEIVED_MO_CALL            = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_XCC_ABORTED_BY_RECEIVED_MT_CALL            = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_XCC_CAUSE_L2_ACK_FAIL                      = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_XCC_CAUSE_ALLOC_SRID_FAIL                  = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_XCC_CAUSE_NO_RF                            = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_XCC_CAUSE_PROTOTAL_REV_NOT_SUPPORT         = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_XCC_MT_SMS_IN_TCH                          = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_XCC_EMC_CALL_BACK_MODE                     = (TAF_PS_CAUSE_XCC_SECTION_BEGIN + 24),

    /*----------------------------------------------------------------------
       接入层上报原因值对应到PS域，取值范围[0x500, 0x05FF]
       由于3GPP2协议已经定义了具体的1X网络原因值, 平台上报的1X网络原因值
       取值为协议定义的原因值加上偏移量(0x500)
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_XCC_AS_NO_SERVICE                          = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_XCC_AS_MAX_ACCESS_PROBES                   = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_XCC_AS_REORDER                             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_XCC_AS_INTERCEPT                           = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_XCC_AS_ACCESS_DENIED                       = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_XCC_AS_LOCK                                = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_XCC_AS_ACCT_BLOCK                          = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_XCC_AS_NDSS                                = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_XCC_AS_REDIRECTION                         = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_XCC_AS_ACCESS_IN_PROGRESS                  = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_XCC_AS_ACCESS_FAIL                         = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_XCC_AS_ABORT                               = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_XCC_AS_SIGNAL_FADE_IN_ACH                  = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_XCC_AS_CHANNEL_ASSIGN_TIMEOUT              = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_XCC_AS_BS_RELEASE                          = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_XCC_AS_NOT_ACCEPT_BY_BS                    = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_XCC_AS_SO_REJ                              = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_XCC_AS_RELEASE_TIME_OUT                    = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_XCC_AS_MS_NORMAL_RELEASE                   = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_XCC_AS_RETRY_FAIL                          = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_XCC_AS_NW_NORMAL_RELEASE                   = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_XCC_AS_SIGNAL_FADE_IN_TCH                  = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_XCC_AS_MS_END                              = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_XCC_AS_TCH_LOST                            = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_XCC_AS_T50_EXPIRED                         = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_XCC_AS_T51_EXPIRED                         = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 26),
    TAF_PS_CAUSE_XCC_AS_TCH_WAIT_BS_ACK_EXPIRED             = (TAF_PS_CAUSE_XCC_AS_SECTION_BEGIN + 27),

    TAF_PS_CAUSE_HSM_AS_CONN_FAIL                           = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_HSM_AS_MAX_ACCESS_PROBES                   = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_HSM_AS_CONNECTION_DENY                     = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_HSM_AS_RTC_RTCACK_FAILURE                  = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_HSM_AS_SIGNAL_FADE                         = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_HSM_AS_SESSION_NOT_EXIST                   = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_HSM_AS_CAN_NOT_EST_CONN                    = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_HSM_AS_TIMEOUT                             = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_HSM_AS_HRPD_SLAVE                          = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_HSM_AS_HANDOFF_FAIL                        = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_HSM_AS_IN_UATI_PROCEDURE                   = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_HSM_AS_IN_SCP_PROCEDURE                    = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_HSM_AS_CONNECTION_EXIST                    = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_HSM_AS_NO_RF                               = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_HSM_AS_REDIRECT                            = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_HSM_AS_HRPD_HANDOFF_TO_1X                  = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 16),

    TAF_PS_CAUSE_HSM_AS_HRPD_PREFER_CH_NOT_VALID            = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_HSM_AS_OHM_NOT_CURRENT                     = (TAF_PS_CAUSE_HSM_AS_SECTION_BEGIN + 18),

    TAF_PS_CAUSE_EHSM_CTTF_NOT_SUPPORT_EHRPD                = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_EHSM_NOT_SUPPORT_EHRPD                     = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_EHSM_IN_SUSPEND                            = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_EHSM_LAST_PDN                              = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_EHSM_CONN_EST_FAIL                         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_EHSM_POWER_OFF                             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_EHSM_INVALID_PDN_ID                        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_EHSM_PPP_DETACH                            = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_EHSM_HSM_CONN_FAIL                         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_EHSM_HSM_MAX_ACCESS_PROBES                 = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_EHSM_HSM_CONN_DENY                         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_EHSM_HSM_RTCACK_RAILURE                    = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_EHSM_HSM_SIGNAL_FADE                       = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_EHSM_HSM_CAN_NOT_EST_CONN                  = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_EHSM_HSM_TIME_OUT                          = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_EHSM_HSM_HRPD_SLAVE                        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_EHSM_HSM_SESSION_NOT_EXSIT                 = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_GENERAL_ERROR               = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_UNAUTHORIZED_APN            = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PDN_LIMIT_EXCEEDED          = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_NO_PGW_AVAILABLE            = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PGW_UNREACHABLE             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PGW_REJECT                  = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_INSUFFICIENT_PARAMETERS     = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_RESOURCE_UNAVAILABLE        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_ADMIN_PROHIBITED            = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 26),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PDNID_ALREADY_IN_USE        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 27),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_SUBSCRIPTION_LIMITATION     = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 28),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_PDN_CONNECTION_ALREADY_EXISTS_FOR_THIS_APN      = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 29),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_EMERGENCY_SERVICES_NOT_SUPPORTED                = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 30),
    TAF_PS_CAUSE_EHSM_PPP_ERROR_RECONNECT_TO_THIS_APN_NOT_ALLOWED               = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 31),

    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_AN_EAP_FAILURE         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 32), /**< AT received EAP-Failure from AN in EAP authentication. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_AT_EAP_FAILURE         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 33), /**< AT local EAP authentication failure in EAP authentication. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_EAP_TIMEROUT           = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 34), /**< AT hasn`t received AN EAP message in EAP authentication. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_LCP_TIMEROUT           = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 35), /**< AT hasn`t received AN ack in LCP negotiation. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_LCP_TERMINATE          = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 36), /**< LCP terminate in LCP negotiation and EAP authentication. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_UNSPECIFIC_FAILURE     = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 37), /**< All failure except above failure in LCP negotiation and EAP authentication. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_VSNCP_TIMEROUT         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 38), /**< AT hasn`t received AN ack in VSNDCP negotiation. */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_PARA_ERROR             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 39), /**< CNAS attach req para error  */
    TAF_PS_CAUSE_EHSM_NW_DISC_IND                           = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 40),
    TAF_PS_CAUSE_EHSM_HSM_RSLT_IN_UATI_PROCEDURE            = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 41),
    TAF_PS_CAUSE_EHSM_HSM_RSLT_IN_SCP_PROCEDURE             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 42),
    TAF_PS_CAUSE_EHSM_HSM_NO_RF                             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 43),
    TAF_PS_CAUSE_EHSM_HSM_REDIRECT                          = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 44),
    TAF_PS_CAUSE_EHSM_ABORT                                 = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 45),
    TAF_PS_CAUSE_EHSM_HSM_NO_NETWORK                        = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 46),

    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_LTE_DETACH             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 47), /**< LTE DETACH  */
    TAF_PS_CAUSE_EHSM_PPP_ERROR_CODE_ACCESS_AUTH_FAILURE    = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 48), /**< ACCESS AUTH FAILURE  */

    TAF_PS_CAUSE_EHSM_OHM_NOT_CURRENT                       = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 49), /**< ACCESS AUTH FAILURE  */

    TAF_PS_CAUSE_EHSM_PS_DETACH                             = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 50), /**< ACCESS AUTH FAILURE  */

    TAF_PS_CAUSE_EHSM_PPPC_ACTED_PDN_LESS_THAN_EHSM         = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 51), /**< ACCESS AUTH FAILURE  */
    TAF_PS_CAUSE_EHSM_PPPC_EXIST_NOT_OPEN_PDN               = (TAF_PS_CAUSE_EHSM_CTTF_SECTION_BEGIN + 52), /**< ACCESS AUTH FAILURE  */

    TAF_PS_CAUSE_PPPC_NEGOTIATE_FAIL                        = (TAF_PS_CAUSE_PPPC_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_PPPC_PDSN_AUTH_TYPE_FAIL                   = (TAF_PS_CAUSE_PPPC_SECTION_BEGIN + 2),

    TAF_PS_CAUSE_MIP_FA_REASON_UNSPECIFIED                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 64),
    TAF_PS_CAUSE_MIP_FA_ADMIN_PROHIBITED                    = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 65),            /* not fallback */
    TAF_PS_CAUSE_MIP_FA_INSUFFICIENT_RESOURCES              = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 66),
    TAF_PS_CAUSE_MIP_FA_MOBILE_NODE_FAILED_AUTH             = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 67),            /* not fallback */
    TAF_PS_CAUSE_MIP_FA_HA_FAILED_AUTH                      = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 68),
    TAF_PS_CAUSE_MIP_FA_REQUESTED_LIFETIME_TOO_LONG         = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 69),
    TAF_PS_CAUSE_MIP_FA_MALFORMED_REQUEST                   = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 70),
    TAF_PS_CAUSE_MIP_FA_MALFORMED_REPLY                     = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 71),
    TAF_PS_CAUSE_MIP_FA_ENCAPSULATION_UNAVAILABLE           = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 72),
    TAF_PS_CAUSE_MIP_FA_VJHC_UNAVAILABLE                    = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 73),
    TAF_PS_CAUSE_MIP_FA_CANT_REV_TUN                        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 74),
    TAF_PS_CAUSE_MIP_FA_MUST_REV_TUN                        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 75),
    TAF_PS_CAUSE_MIP_FA_BAD_TTL                             = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 76),
    TAF_PS_CAUSE_MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 79),
    TAF_PS_CAUSE_MIP_HOME_NETWORK_UNREACHABLE               = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 80),
    TAF_PS_CAUSE_MIP_HA_HOST_UNREACHABLE                    = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 81),
    TAF_PS_CAUSE_MIP_HA_PORT_UNREACHABLE                    = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 82),
    TAF_PS_CAUSE_MIP_HA_UNREACHABLE                         = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 88),
    TAF_PS_CAUSE_MIP_FA_VS_REASON                           = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 89),
    TAF_PS_CAUSE_MIP_NON_ZERO_HA_ADDR_IN_RRQ                = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 90),
    TAF_PS_CAUSE_MIP_NON_ZERO_HOME_ADDR_IN_RRQ              = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 96),
    TAF_PS_CAUSE_MIP_MISSING_NAI                            = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 97),
    TAF_PS_CAUSE_MIP_MISSING_HA                             = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 98),
    TAF_PS_CAUSE_MIP_MISSING_HOME_ADDR                      = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 99),
    TAF_PS_CAUSE_MIP_UNKNOWN_CHALLENGE                      = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 104),
    TAF_PS_CAUSE_MIP_MISSING_CHALLENGE                      = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 105),
    TAF_PS_CAUSE_MIP_STALE_CHALLENGE                        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 106),
    TAF_PS_CAUSE_MIP_MISSING_MN_FA                          = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 107),
    TAF_PS_CAUSE_MIP_HA_REASON_UNSPECIFIED                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 128),
    TAF_PS_CAUSE_MIP_HA_ADMIN_PROHIBITED                    = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 129),
    TAF_PS_CAUSE_MIP_HA_INSUFFICIENT_RESOURCES              = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 130),
    TAF_PS_CAUSE_MIP_HA_MOBILE_NODE_FAILED_AUTH             = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 131),
    TAF_PS_CAUSE_MIP_HA_FA_FAILED_AUTH                      = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 132),
    TAF_PS_CAUSE_MIP_HA_REG_ID_MISMATCH                     = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 133),
    TAF_PS_CAUSE_MIP_HA_MALFORMED_REQUEST                   = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 134),
    TAF_PS_CAUSE_MIP_HA_TOO_MANY_SIMULTANEOUS_BINDING       = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 135),
    TAF_PS_CAUSE_MIP_UNKNOWN_HA                             = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 136),
    TAF_PS_CAUSE_MIP_HA_CANT_REV_TUN                        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 137),
    TAF_PS_CAUSE_MIP_HA_MUST_REV_TUN                        = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 138),
    TAF_PS_CAUSE_MIP_HA_ENCAPSULATION_UNAVAILABLE           = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 139),
    TAF_PS_CAUSE_MIP_REG_WITH_REDIRECTED_HA                 = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 143),
    TAF_PS_CAUSE_MIP_HA_TO_HA_BAD_AAA_AUTH                  = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 144),

    /* MIP协议未定义或者网侧发来的其它原因值 */
    TAF_PS_CAUSE_MIP_PROTOCOL_UNSPECIFIED                   = (TAF_PS_CAUSE_MIP_SECTION_BEGIN + 255),

    /* TAF APS定义EMM内部拒绝原因值 */
    TAF_PS_CAUSE_EMM_NW_BEAR_NOT_SYNC                        = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 0),
    TAF_PS_CAUSE_EMM_REL_IND                                = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_EMM_AUTH_TIME_OUT                          = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_EMM_T3410_TIME_OUT                         = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_EMM_LINK_ERR                               = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_EMM_AUTH_FAIL                              = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_EMM_MM_THROT                               = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_EMM_PLMN_SRCH_INTERRUPT                    = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_EMM_SYS_INFO_INTERRUPT                     = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_EMM_SUSPEND_INTERRUPT                      = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_EMM_DETACH                                 = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_EMM_LRRC_ERR_IND                           = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_EMM_IMSI_PAGING_DETACH                     = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_EMM_ATTACH_EST_FAIL                        = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_EMM_STATUS_CANT_TRIGGER                    = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_EMM_POWER_OFF                              = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_EMM_AREA_LOST                              = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_EMM_SYS_CFG_OR_UE_CAP_CHG                  = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_EMM_GU_ATTACH_FAIL                         = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_EMM_GU_DETACH                              = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_EMM_ATTACH_MMC_REL                         = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_EMM_ATTACH_CS_CALL_INTERRUPT               = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_EMM_USIM_PULL_OUT                          = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 22),

    TAF_PS_CAUSE_EMM_UNKNOWN                                = (TAF_PS_CAUSE_EMM_SECTION_BEGIN + 199),

    TAF_PS_CAUSE_EMM_NW_REJ_NULL                            = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 0),
    TAF_PS_CAUSE_EMM_NW_REJ_IMSI_UNKNOWN_IN_HSS             = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_EMM_NW_REJ_ILLEGAL_UE                      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_EMM_NW_REJ_IMEI_NOT_ACCEPTED               = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_EMM_NW_REJ_ILLEGAL_ME                      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_EMM_NW_REJ_EPS_SERV_NOT_ALLOW              = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_EMM_NW_REJ_EPS_SERV_AND_NON_EPS_SERV_NOT_ALLOW = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_EMM_NW_REJ_UE_ID_NOT_DERIVED               = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_EMM_NW_REJ_IMPLICIT_DETACHED               = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_EMM_NW_REJ_PLMN_NOT_ALLOW                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_EMM_NW_REJ_TA_NOT_ALLOW                    = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_EMM_NW_REJ_ROAM_NOT_ALLOW                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_EMM_NW_REJ_EPS_SERV_NOT_ALLOW_IN_PLMN      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_EMM_NW_REJ_NO_SUITABL_CELL                 = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_EMM_NW_REJ_MSC_UNREACHABLE                 = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_EMM_NW_REJ_NETWORK_FAILURE                 = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_EMM_NW_REJ_CS_NOT_AVAIL                    = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_EMM_NW_REJ_ESM_FAILURE                     = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_EMM_NW_REJ_MAC_FAILURE                     = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_EMM_NW_REJ_SYNCH_FAILURE                   = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_EMM_NW_REJ_PROCEDURE_CONGESTION            = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_EMM_NW_REJ_UE_SECU_CAP_MISMATCH            = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_EMM_NW_REJ_SECU_MODE_REJECTED_UNSPECIFIED  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_EMM_NW_REJ_NOT_AUTHORIZED_FOR_THIS_CSG     = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_EMM_NW_REJ_REQUESTED_SER_OPTION_NOT_AUTHORIZED_IN_PLMN = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 35),
    TAF_PS_CAUSE_EMM_NW_REJ_CS_FALLBACK_CALL_EST_NOT_ALLOWED = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 38),
    TAF_PS_CAUSE_EMM_NW_REJ_CS_DOMAIN_TMP_NOT_ALLOWED       = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 39),
    TAF_PS_CAUSE_EMM_NW_REJ_NO_EPS_BEARER_CONTEXT_ACTIVATED = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 40),
    TAF_PS_CAUSE_EMM_NW_REJ_SERVER_NETWORK_FAILURE          = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 42),
    TAF_PS_CAUSE_EMM_NW_REJ_SEMANTICALLY_INCORRECT_MSG      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 95),
    TAF_PS_CAUSE_EMM_NW_REJ_INVALID_MANDATORY_INF           = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 96),
    TAF_PS_CAUSE_EMM_NW_REJ_MSG_NONEXIST_NOTIMPLEMENTE      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 97),
    TAF_PS_CAUSE_EMM_NW_REJ_MSG_TYPE_NOT_COMPATIBLE         = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 98),
    TAF_PS_CAUSE_EMM_NW_REJ_IE_NONEXIST_NOTIMPLEMENTED      = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 99),
    TAF_PS_CAUSE_EMM_NW_REJ_CONDITIONAL_IE_ERROR            = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 100),
    TAF_PS_CAUSE_EMM_NW_REJ_MSG_NOT_COMPATIBLE              = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 101),
    TAF_PS_CAUSE_EMM_NW_REJ_PROTOCOL_ERROR                  = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 111),
    TAF_PS_CAUSE_EMM_NW_REJ_OTHERS                          = (TAF_PS_CAUSE_EMM_NW_SECTION_BEGIN + 255),

    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PARA_ERROR             = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_LINKLOST               = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_TIMEOUT                = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SM_ERROR               = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_OTHER_ERROR            = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 5),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SIM_AUTH_NO_RESP       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 6),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_AUTH_NETWORK_ERROR     = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 7),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PARSE_NETWORK_PAYLOAD_ERROR    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 8),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ADD_SA_SP_ERROR        = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 9),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CLIENT_PAYLOAD_ERROR   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 10),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_MSG_NETWORK_OVERTIME_ERROR     = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 11),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ENC_ALG_ERROR          = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 12),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_APN_ID_ERROR           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 13),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NETWORK_AUTH_SIM_ERROR = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 14),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_STATUS_ERROR           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 15),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SA_INIT_ERROR          = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 16),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NON_3GPP_ACCESS_TO_EPC_NOT_ALLOWED     = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 17),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_USER_UNKNOWN           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 18),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PLMN_NOT_ALLOWED       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 19),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_AUTHORIZATION_REJECTED = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 20),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_RAT_TYPE_NOT_ALLOWED   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 21),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NETWORK_FAILURE        = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 22),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NO_APN_SUBSCRIPTION    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 23),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PDN_CONNECTION_REJECTION   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 24),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CONNECTION_REACHED     = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 25),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_PROTECLE_ERROR         = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 26),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_INTERNAL_ADDRESS_ERROR = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 27),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV4_ADDRESS_CONFLICT  = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 28),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_CERT_ERROR             = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 29),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_NOT_SUPPORT_CONCURRENCE    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 30),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SHUTDOWN               = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 31),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_DONE_ERROR             = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 32),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_REDIRECT_FAILURE       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 33),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_ROAM_FORBID            = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 50),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_HIFI_STATUS_OVERTIME   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 51),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IKED_ABNORMAL          = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 52),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_UNKNOWN_NETWORK_ERROR  = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 53),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_SWITCH_CACHE_MAX_REACHED   = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 54),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_APN_MAX_REACHED        = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 55),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_REQUEST_REPEATED       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 56),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_UNKNOWN                = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 200),

    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_KEEP_LIVE            = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 201),
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_NETWORK_DELETE       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 202),
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_IKED_RESET           = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 203),
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_REKEY_ERROR          = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 204),
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_MAPCONN_TEAR         = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 205),
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_BEARS_CONNCURRENT_NOT_SUPPOER    = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 206),
    TAF_PS_CAUSE_WLAN_PDN_DEACT_RESULT_UNKNOWN              = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 230),

    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IP_ADDRESS_CHG_IN_HANDOVER = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 231),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV4_NOT_ALLOWED       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 232),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_IPV6_NOT_ALLOWED       = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 233),
    TAF_PS_CAUSE_WLAN_PDN_ACT_RESULT_THROT_NOT_ALLOWED      = (TAF_PS_CAUSE_WLAN_SECTION_BEGIN + 234),

    /*----------------------------------------------------------------------
       NRSM的内部原因值, 取值范围[0x0000, 0x00FF]
       方向: NRSM -> TAF
       说明: UNKNOWN为错误码分段的最后一个错误码
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_NRSM_T3580_TIME_OUT                        = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 1),
    TAF_PS_CAUSE_NRSM_T3396_ALG_NOT_ALLOWED                 = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 2),
    TAF_PS_CAUSE_NRSM_SESSION_EST_FAILED                    = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 3),
    TAF_PS_CAUSE_NRSM_SUSPEND                               = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 4),
    TAF_PS_CAUSE_NRSM_UNKNOWN                               = (TAF_PS_CAUSE_NRSM_SECTION_BEGIN + 255),

    /*----------------------------------------------------------------------
       NRMM的内部原因值, 取值范围[0x0200, 0x02FF]
       方向: NRMM -> NRSM -> TAF
       说明: UNKNOWN为错误码分段的最后一个错误码
    *---------------------------------------------------------------------*/
    TAF_PS_CAUSE_NRMM_REL_IND                               = (TAF_PS_CAUSE_NRMM_SECTION_BEGIN + 0),


    TAF_PS_CAUSE_BUTT                                       = 0xFFFFFFFF
};
typedef VOS_UINT32 TAF_PS_CAUSE_ENUM_UINT32;


enum TAF_PS_CALL_END_CAUSE_ENUM
{
    TAF_PS_CALL_END_CAUSE_NORMAL        = 0x00,
    TAF_PS_CALL_END_CAUSE_FORCE         = 0x01,                                 /* 强制去激活 */

    /* 业务场景的去激活原因 */
    TAF_PS_CALL_END_CAUSE_IPV6_ADDR_ALLOC_FAIL      = 0x10,                     /* IPV6路由公告分配失败 */
    TAF_PS_CALL_END_CAUSE_IPV6_ADDR_TIME_OUT        = 0x11,                     /* IPV6地址失效 */
    TAF_PS_CALL_END_CAUSE_PCSCF_ADDR_ALLOC_FAIL     = 0x12,                     /* PCSCF地址不存在 */
    TAF_PS_CALL_END_CAUSE_LOCAL                     = 0x13,                     /* 本地去激活 */

    TAF_PS_CALL_END_CAUSE_CELLULAR2W_HO             = 0x14,

    TAF_PS_CALL_END_CAUSE_BUTT
};
typedef VOS_UINT8 TAF_PS_CALL_END_CAUSE_ENUM_UINT8;


enum TAF_PF_TRANS_DIRECTION_ENUM
{
    TAF_PF_TRANS_DIRECTION_PRE_REL7     = 0x00,             /* 0 - Pre-Release 7 TFT filter (see 3GPP TS 24.008 [8], table 10.5.162) */
    TAF_PF_TRANS_DIRECTION_UPLINK       = 0x01,             /* 1 - Uplink */
    TAF_PF_TRANS_DIRECTION_DOWNLINK     = 0x02,             /* 2 - Downlink */
    TAF_PF_TRANS_DIRECTION_BIDIRECTION  = 0x03,             /* 3 - Birectional (Up & Downlink) (default if omitted) */

    TAF_PF_TRANS_DIRECTION_BUTT         = 0xFF
};
typedef VOS_UINT8 TAF_PF_TRANS_DIRECTION_ENUM_UINT8;


enum TAF_PDP_TYPE_ENUM
{
    TAF_PDP_IPV4                        = 0x01,
    TAF_PDP_IPV6                        = 0x02,
    TAF_PDP_IPV4V6                      = 0x03,
    TAF_PDP_PPP                         = 0x04,

    TAF_PDP_TYPE_BUTT                   = 0xFF
};
typedef VOS_UINT8 TAF_PDP_TYPE_ENUM_UINT8;


enum TAF_PS_BEARER_TYPE_ENUM
{
    TAF_PS_BEARER_TYPE_DEFAULT          = 0x00,
    TAF_PS_BEARER_TYPE_DEDICATED        = 0x01,
    TAF_PS_BEARER_TYPE_BUTT
};
typedef VOS_UINT8 TAF_PS_BEARER_TYPE_ENUM_UINT8;


enum TAF_PS_SSC_MODE_ENUM
{
    TAF_PS_SSC_MODE_1                   = 0x01,
    TAF_PS_SSC_MODE_2                   = 0x02,
    TAF_PS_SSC_MODE_3                   = 0x03,
    TAF_PS_SSC_MODE_BUTT
};
typedef VOS_UINT8 TAF_PS_SSC_MODE_ENUM_UINT8;


typedef struct
{
    VOS_UINT32                          bitOpIpAddr         :1;
    VOS_UINT32                          bitOpPrimDns        :1;
    VOS_UINT32                          bitOpSecDns         :1;
    VOS_UINT32                          bitOpPrimNbns       :1;
    VOS_UINT32                          bitOpSecNbns        :1;
    VOS_UINT32                          bitOpSpare          :27;

    VOS_UINT8                           aucIpAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucPrimDns[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucSecDns[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucPrimNbns[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucSecNbns[TAF_IPV4_ADDR_LEN];

} TAF_PS_IPCP_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulChallengeLen;
    VOS_UINT8                           aucChallenge[TAF_PS_PPP_CHAP_CHALLNGE_LEN];
    VOS_UINT32                          ulChallengeNameLen;
    VOS_UINT8                           aucChallengeName[TAF_PS_PPP_CHAP_CHALLNGE_NAME_LEN];
    VOS_UINT32                          ulResponseLen;
    VOS_UINT8                           aucResponse[TAF_PS_PPP_CHAP_RESPONSE_LEN];
    VOS_UINT8                           ucResponseNameLen;
    VOS_UINT8                           aucResponseName[TAF_MAX_AUTHDATA_USERNAME_LEN];

} TAF_PDP_CHAP_PARAM_STRU;


typedef struct
{
    VOS_UINT8                           ucUsernameLen;
    VOS_UINT8                           aucUsername[TAF_MAX_AUTHDATA_USERNAME_LEN];
    VOS_UINT8                           ucPasswordLen;
    VOS_UINT8                           aucPassword[TAF_MAX_AUTHDATA_PASSWORD_LEN];

} TAF_PDP_PAP_PARAM_STRU;


typedef struct
{
    TAF_PDP_AUTH_TYPE_ENUM_UINT8        enAuthType;
    VOS_UINT8                           ucAuthId;
    VOS_UINT8                           aucRsv[2];

    union
    {
        TAF_PDP_CHAP_PARAM_STRU         stChapParam;
        TAF_PDP_PAP_PARAM_STRU          stPapParam;
    } unAuthParam;

} TAF_PS_AUTH_INFO_STRU;

/*****************************************************************************
 结构名称  : TAF_PDP_AUTH_CONFIG_INFO_STRU

 结构说明  : 鉴权配置结构体
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitOpUserName       : 1;
    VOS_UINT32                          bitOpPassWord       : 1;
    VOS_UINT32                          bitOpAuthParam      : 1;
    VOS_UINT32                          bitOpSpare          : 29;

    TAF_PDP_AUTH_TYPE_ENUM_UINT8        enAuthType;                                         /* NDISDUP & PPP拨号类型用此参数 */
    VOS_UINT8                           ucAuthId;                                           /* PPP拨号类型用此参数 */
    VOS_UINT8                           aucReserve[2];
    VOS_UINT8                           aucUserName[TAF_MAX_AUTHDATA_USERNAME_LEN + 1];              /* NDISDUP拨号类型用此参数 */
    VOS_UINT8                           aucPassWord[TAF_MAX_AUTHDATA_PASSWORD_LEN + 1];              /* NDISDUP拨号类型用此参数 */

    union
    {
        TAF_PDP_CHAP_PARAM_STRU         stChapParam;
        TAF_PDP_PAP_PARAM_STRU          stPapParam;
    } unAuthParam;                                                                          /* PPP拨号类型用此参数 */
}TAF_PDP_AUTH_CONFIG_INFO_STRU;

/*****************************************************************************
 结构名称: TAF_PDP_APN_STRU

 结构说明: PDP上下文中APN信息
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucLength;
    VOS_UINT8                           aucValue[TAF_MAX_APN_LEN];

} TAF_PDP_APN_STRU;


typedef struct
{
    /* IPV4:1, IPV6:2, IPV4V6:3, PPP:4 */
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           aucIpv4Addr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucIpv6Addr[TAF_IPV6_ADDR_LEN];
} TAF_PDP_ADDR_STRU;


typedef struct
{
    VOS_UINT32                          bitOpPrimNbnsAddr   : 1;
    VOS_UINT32                          bitOpSecNbnsAddr    : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           aucPrimNbnsAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucSecNbnsAddr[TAF_IPV4_ADDR_LEN];

} TAF_PDP_NBNS_STRU;


typedef struct
{
    VOS_UINT32                          bitOpGateWayAddr    : 1;
    VOS_UINT32                          bitOpSpare          : 31;

    VOS_UINT8                           aucGateWayAddr[TAF_IPV4_ADDR_LEN];

} TAF_PDP_GATEWAY_STRU;


typedef struct
{
    VOS_UINT32                          bitOpPrimDnsAddr    : 1;
    VOS_UINT32                          bitOpSecDnsAddr     : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           aucPrimDnsAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucSecDnsAddr[TAF_IPV4_ADDR_LEN];

} TAF_PDP_DNS_STRU;


typedef struct
{
    VOS_UINT32                          bitOpPrimDnsAddr    : 1;
    VOS_UINT32                          bitOpSecDnsAddr     : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           aucPrimDnsAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                           aucSecDnsAddr[TAF_IPV6_ADDR_LEN];

} TAF_PDP_IPV6_DNS_STRU;


typedef struct
{
    TAF_PDP_ADDR_STRU                   stPdpAddr;                              /* 用户设置的静态IP地址 */
    TAF_PDP_APN_STRU                    stApn;
    TAF_PDP_D_COMP_ENUM_UINT8           enPdpDcomp;
    TAF_PDP_H_COMP_ENUM_UINT8           enPdpHcomp;

    /* 0 - IPv4 Address Allocation through NAS Signaling (default if omitted)
       1 - IPv4 Address Allocated through DHCP */
    TAF_PDP_IPV4_ADDR_ALLOC_TYPE_ENUM_UINT8 enIpv4AddrAlloc;

    /* 0 - PDP context is not for emergency bearer services
       1 - PDP context is for emergency bearer services */
    TAF_PDP_EMC_IND_ENUM_UINT8          enEmergencyInd;

    TAF_PDP_PCSCF_DISCOVERY_ENUM_UINT8  enPcscfDiscovery;

    TAF_PDP_IM_CN_SIG_FLAG_ENUM_UINT8   enImCnSignalFlg;

    TAF_PDP_NAS_SIG_PRIO_IND_ENUM_UINT8 enNasSigPrioInd;

    VOS_UINT8                           aucReserved2[1];

}TAF_PDP_PRIM_CONTEXT_STRU;


typedef struct
{
    VOS_UINT8                           ucTrafficClass;                         /*UMTS承载服务优化类型*/
    VOS_UINT8                           ucDeliverOrder;                         /*SDU队列是否发送UMTS承载*/
    VOS_UINT8                           ucDeliverErrSdu;                        /*SDU错误是否发送*/
    VOS_UINT8                           aucReserved1[1];
    VOS_UINT16                          usMaxSduSize;                           /*SDU大小最大值*/
    VOS_UINT8                           aucReserved2[2];
    VOS_UINT32                          ulMaxBitUl;                             /*上行最大速率 kbits/s*/
    VOS_UINT32                          ulMaxBitDl;                             /*下行最大速率 kbits/s*/
    VOS_UINT8                           ucResidualBer;
    VOS_UINT8                           ucSduErrRatio;                          /*SDU误码率*/
    VOS_UINT16                          usTransDelay;                           /*传输延时, 单位毫秒*/
    VOS_UINT8                           ucTraffHandlePrior;                     /*处理UMTS承载的SDU对比其他承载的SDU*/
    VOS_UINT8                           aucReserved3[1];
    VOS_UINT8                           ucSrcStatisticsDescriptor;
    VOS_UINT8                           ucSignallingIndication;
    VOS_UINT32                          ulGuarantBitUl;                         /*保证上行传输速率 kbits/s*/
    VOS_UINT32                          ulGuarantBitDl;                         /*保证下行传输速率 kbits/s*/
}TAF_UMTS_QOS_STRU;


typedef struct
{
    /* 0 QCI is selected by network
       [1 - 4]value range for guranteed bit rate Traffic Flows
       [5 - 9]value range for non-guarenteed bit rate Traffic Flows */
    VOS_UINT8                           ucQCI;

    VOS_UINT8                           aucReserved[3];

    /* DL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32                          ulDLGBR;

    /* UL GBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32                          ulULGBR;

    /* DL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32                          ulDLMBR;

    /* UL MBR in case of GBR QCI, The value is in kbit/s */
    VOS_UINT32                          ulULMBR;
}TAF_EPS_QOS_STRU;


typedef struct
{
    VOS_UINT32                          bitOpRmtIpv4AddrAndMask     : 1;
    VOS_UINT32                          bitOpRmtIpv6AddrAndMask     : 1;
    VOS_UINT32                          bitOpProtocolId             : 1;
    VOS_UINT32                          bitOpSingleLocalPort        : 1;
    VOS_UINT32                          bitOpLocalPortRange         : 1;
    VOS_UINT32                          bitOpSingleRemotePort       : 1;
    VOS_UINT32                          bitOpRemotePortRange        : 1;
    VOS_UINT32                          bitOpSecuParaIndex          : 1;
    VOS_UINT32                          bitOpTypeOfService          : 1;
    VOS_UINT32                          bitOpFlowLabelType          : 1;
    VOS_UINT32                          bitOpLocalIpv4AddrAndMask   : 1;
    VOS_UINT32                          bitOpLocalIpv6AddrAndMask   : 1;
    VOS_UINT32                          bitOpSpare                  : 20;

    VOS_UINT8                           ucPacketFilterId;
    VOS_UINT8                           ucNwPacketFilterId;
    TAF_PF_TRANS_DIRECTION_ENUM_UINT8   enDirection;
    VOS_UINT8                           ucPrecedence;                           /* packet filter evaluation precedence */

    VOS_UINT32                          ulSecuParaIndex;                        /* SPI */
    VOS_UINT16                          usSingleLcPort;
    VOS_UINT16                          usLcPortHighLimit;
    VOS_UINT16                          usLcPortLowLimit;
    VOS_UINT16                          usSingleRmtPort;
    VOS_UINT16                          usRmtPortHighLimit;
    VOS_UINT16                          usRmtPortLowLimit;
    VOS_UINT8                           ucProtocolId;                           /* 协议号 */
    VOS_UINT8                           ucTypeOfService;                        /* TOS */
    VOS_UINT8                           ucTypeOfServiceMask;                    /* TOS Mask */
    VOS_UINT8                           aucReserved[1];

    /* aucRmtIpv4Address[0]为IP地址高字节位
       aucRmtIpv4Address[3]为低字节位 */
    VOS_UINT8                           aucRmtIpv4Address[TAF_IPV4_ADDR_LEN];

    /* aucRmtIpv4Mask[0]为IP地址高字节位 ,
       aucRmtIpv4Mask[3]为低字节位*/
    VOS_UINT8                           aucRmtIpv4Mask[TAF_IPV4_ADDR_LEN];

    /* ucRmtIpv6Address[0]为IPv6接口标识高字节位
       ucRmtIpv6Address[7]为IPv6接口标识低字节位 */
    VOS_UINT8                           aucRmtIpv6Address[TAF_IPV6_ADDR_LEN];

    /* ucRmtIpv6Mask[0]为高字节位
       ucRmtIpv6Mask[7]为低字节位*/
    VOS_UINT8                           aucRmtIpv6Mask[TAF_IPV6_ADDR_LEN];

    VOS_UINT32                          ulFlowLabelType;                        /*FlowLabelType*/

    VOS_UINT8                           aucLocalIpv4Addr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucLocalIpv4Mask[TAF_IPV4_ADDR_LEN];
    VOS_UINT8                           aucLocalIpv6Addr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                           ucLocalIpv6Prefix;
    VOS_UINT8                           aucReserved2[3];
}TAF_PDP_PF_STRU;


typedef struct
{
    VOS_UINT32                          ulPfNum;                                /* pf个数 */
    TAF_PDP_PF_STRU                     astPf[TAF_MAX_SDF_PF_NUM];              /* pf表 */
}TAF_PDP_TFT_STRU;


typedef struct
{
    VOS_UINT8                           aucPcscfAddr[TAF_IPV4_ADDR_LEN];
} TAF_PDP_IPV4_PCSCF_STRU;


typedef struct
{
    VOS_UINT8                           ucIpv4PcscfAddrNum;                     /* IPV4的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8                           aucRsv[3];                              /* 保留 */

    TAF_PDP_IPV4_PCSCF_STRU             astIpv4PcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_PDP_IPV4_PCSCF_LIST_STRU;


typedef struct
{
    VOS_UINT8                           aucPcscfAddr[TAF_IPV6_ADDR_LEN];
} TAF_PDP_IPV6_PCSCF_STRU;


typedef struct
{
    VOS_UINT8                           ucIpv6PcscfAddrNum;                     /* IPV6的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8                           aucRsv[3];                              /* 保留 */

    TAF_PDP_IPV6_PCSCF_STRU             astIpv6PcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_PDP_IPV6_PCSCF_LIST_STRU;


typedef struct
{
    VOS_UINT16                          usContainerId;
    VOS_UINT8                           ucContentLen;
    VOS_UINT8                           aucContents[TAF_PS_MAX_CUSTOM_PCO_CONTAINER_CONTENT_LEN];
}TAF_PS_CUSTOM_PCO_CONTAINER_STRU;


typedef struct
{
    VOS_UINT32                          ulContainerNum;
    TAF_PS_CUSTOM_PCO_CONTAINER_STRU    astContainerList[TAF_PS_MAX_CUSTOM_PCO_CONTAINER_NUM];
}TAF_PS_CUSTOM_PCO_INFO_STRU;

typedef struct
{
    VOS_UINT8                           aucIpV4Addr[TAF_IPV4_ADDR_LEN];
}TAF_IPV4_EPDG_STRU;

typedef struct
{
    VOS_UINT8                           aucIpV6Addr[TAF_IPV6_ADDR_LEN];
}TAF_IPV6_EPDG_STRU;


typedef struct
{
    VOS_UINT16                          usIpv4EpdgNum;
    VOS_UINT16                          usIpv6EpdgNum;
    TAF_IPV4_EPDG_STRU                  astIpv4EpdgList[TAF_MAX_IPV4_EPDG_NUM];
    TAF_IPV6_EPDG_STRU                  astIpv6EpdgList[TAF_MAX_IPV6_EPDG_NUM];
}TAF_EPDG_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulBitL          :1;                     /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          ulBitA          :1;
    VOS_UINT32                          ulBitPrefixLen  :8;
    VOS_UINT32                          ulBitRsv        :22;

    VOS_UINT32                          ulValidLifeTime;
    VOS_UINT32                          ulPreferredLifeTime;
    VOS_UINT8                           aucPrefix[TAF_IPV6_ADDR_LEN];
}TAF_PDP_IPV6_PREFIX_STRU;


typedef struct
{
    VOS_UINT32                          bitOpMtu            :1;
    VOS_UINT32                          bitOpSpare          :31;

    VOS_UINT32                          ulBitCurHopLimit    :8;
    VOS_UINT32                          ulBitM              :1;
    VOS_UINT32                          ulBitO              :1;
    VOS_UINT32                          ulBitRsv            :22;

    VOS_UINT32                          ulMtu;                                  /* MTU大小 */
    VOS_UINT32                          ulPrefixNum;                            /* IPV6地址前缀个数 */
    TAF_PDP_IPV6_PREFIX_STRU            astPrefixList[TAF_MAX_PREFIX_NUM_IN_RA];/* IPV6地址前缀数组 */

} TAF_PDP_IPV6_RA_INFO_STRU;


typedef struct
{
    TAF_PDP_AUTH_TYPE_ENUM_UINT8        enAuthType;
    VOS_UINT8                           aucPlmn[TAF_MAX_AUTHDATA_PLMN_LEN + 1];
    VOS_UINT8                           aucPassword[TAF_MAX_AUTHDATA_PASSWORD_LEN + 1];
    VOS_UINT8                           aucUsername[TAF_MAX_AUTHDATA_USERNAME_LEN + 1];
}TAF_PDP_AUTHDATA_STRU;

typedef struct
{
    VOS_UINT32                          bitOpAddr           : 1;
    VOS_UINT32                          bitOpSpare          : 31;

    VOS_UINT8                           aucAddr[TAF_IPV4_ADDR_LEN];
} TAF_PDP_IPV4_ADDR_STRU;

typedef struct
{
    VOS_UINT32                          bitOpAddr           : 1;
    VOS_UINT32                          bitOpSpare          : 31;

    VOS_UINT8                           aucAddr[TAF_IPV6_ADDR_LEN];
} TAF_PDP_IPV6_ADDR_STRU;


typedef struct
{
    VOS_UINT32                          bitOpSd             : 1;
    VOS_UINT32                          bitOpMappedSst      : 1;
    VOS_UINT32                          bitOpMappedSd       : 1;
    VOS_UINT32                          bitOpSpare          : 29;

    VOS_UINT8                           ucSst;
    VOS_UINT8                           ucMappedSst;
    VOS_UINT8                           aucReserved[2];
    VOS_UINT32                          ulSd;
    VOS_UINT32                          ulMappedSd;
} TAF_PS_S_NSSAI_STRU;


typedef struct
{
    VOS_UINT8                           ucNum;
    VOS_UINT8                           aucReserved[3];
    TAF_PS_S_NSSAI_STRU                 astSNssai[TAF_PS_NSSAI_MAX_NUM];
} TAF_PS_ALLOW_NSSAI_INFO_STRU;


typedef struct
{
    TAF_PS_ALLOW_NSSAI_INFO_STRU        stNssai;
} TAF_PS_NR_REG_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucUnit;             /* 单位 */
    VOS_UINT8                           ucRsv;
    VOS_UINT16                          usValue;            /* 数值 */
}TAF_PS_NR_BIT_RATE_STRU;


typedef struct
{
    VOS_UINT32                          bitOpAveragWindow   : 1;
    VOS_UINT32                          bitOpULMaxRate      : 1;
    VOS_UINT32                          bitOpDLMaxRate      : 1;
    VOS_UINT32                          bitOpULGMaxRate     : 1;
    VOS_UINT32                          bitOpDLGMaxRate     : 1;
    VOS_UINT32                          bitOpSpare          : 27;

    VOS_UINT8                           uc5QI;
    VOS_UINT8                           ucRsv;
    VOS_UINT16                          usAveragWindow;     /* Averaging Window */
    TAF_PS_NR_BIT_RATE_STRU             stULMaxRate;        /* MFBR uplink */
    TAF_PS_NR_BIT_RATE_STRU             stDLMaxRate;        /* MFBR downlink */
    TAF_PS_NR_BIT_RATE_STRU             stULGMaxRate;       /* GFBR uplink */
    TAF_PS_NR_BIT_RATE_STRU             stDLGMaxRate;       /* GFBR downlink */
}TAF_PS_NR_QOS_STRU;

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of Taf_Aps.h*/
