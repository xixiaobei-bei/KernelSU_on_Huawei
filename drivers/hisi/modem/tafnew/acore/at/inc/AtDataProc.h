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


#ifndef __AT_DATA_PROC_H__
#define __AT_DATA_PROC_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mdrv.h"
#include "AtCtx.h"
#include "AtInputProc.h"
#include "AtNdisInterface.h"
#include "AtRnicInterface.h"
#include "AdsInterface.h"
#include "AtPppInterface.h"
#include "FcInterface.h"
#include "PppInterface.h"
#include "PsIfaceGlobalDef.h"
#include "TafPsApi.h"
#include "AtCmdMsgProc.h"
#include "AtInternalMsg.h"
#include "acore_nv_stru_gucttf.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*lint -e778 */
/*lint -e572 */
#ifndef VOS_NTOHL                   /* 大小字节序转换*/
#if VOS_BYTE_ORDER==VOS_BIG_ENDIAN
#define VOS_NTOHL(x)    (x)
#define VOS_HTONL(x)    (x)
#define VOS_NTOHS(x)    (x)
#define VOS_HTONS(x)    (x)
#else
#define VOS_NTOHL(x)    ((((x) & 0x000000ffU) << 24) | \
             (((x) & 0x0000ff00U) <<  8) | \
             (((x) & 0x00ff0000U) >>  8) | \
             (((x) & 0xff000000U) >> 24))

#define VOS_HTONL(x)    ((((x) & 0x000000ffU) << 24) | \
             (((x) & 0x0000ff00U) <<  8) | \
             (((x) & 0x00ff0000U) >>  8) | \
             (((x) & 0xff000000U) >> 24))

#define VOS_NTOHS(x)    ((((x) & 0x00ff) << 8) | \
             (((x) & 0xff00) >> 8))

#define VOS_HTONS(x)    ((((x) & 0x00ff) << 8) | \
             (((x) & 0xff00) >> 8))
#endif  /* _BYTE_ORDER==_LITTLE_ENDIAN */
#endif
/*lint -e572 */
/*lint -e778 */

#define AT_PPP_CODE_REQ                 (1)
#define AT_PPP_CODE_ACK                 (2)
#define AT_PPP_DEF_ID                   (1)

#define AT_AP_PPP_CODE_CHALLENGE        (1)
#define AT_AP_PPP_CODE_RESPONSE         (2)

#define AT_AP_PPP_PRIMARY_DNS           (129)
#define AT_AP_PPP_PRIMARY_WINNS         (130)
#define AT_AP_PPP_SECONDARY_DNS         (131)
#define AT_AP_PPP_SECONDARY_WINNS       (132)

#define AT_AP_PPP_CHAP_CHALLENGE_LEN    (16)
#define AT_AP_PPP_CHAP_RESPONSE_LEN     (16)
#define AT_AP_MAX_IPV4_ADDR_LEN         (15)

#define AT_AP_AUTH_NO_PROTO             (0)
#define AT_AP_AUTH_PAP_PROTO            (1)
#define AT_AP_AUTH_CHAP_PROTO           (2)

#define AT_DEF_DISPLAY_SPEED            (81920000)                              /* 10 M*/

#define AT_MAX_IPV4_DNS_LEN             (16)
#define AT_MAX_IPV4_NBNS_LEN            (16)
#define AT_MAX_IPV6_DNS_LEN             (16)
#define AT_MAX_IPV6_NBNS_LEN            (16)

#define AT_MAX_IPV6_STR_DOT_NUM         (3)
#define AT_MAX_IPV4V6_STR_COLON_NUM     (6)
#define AT_MAX_IPV6_STR_COLON_NUM       (7)

#define AT_IPV6_STR_MAX_TOKENS          (16)                            /* IPV6字符串格式使用的分隔符标记最大个数 */
#define AT_IPV4_STR_DELIMITER           '.'                             /* RFC协议使用的IPV4文本表达方式使用的分隔符 */
#define AT_IPV6_STR_DELIMITER           ':'                             /* RFC2373使用的IPV6文本表达方式使用的分隔符 */

#if (FEATURE_ON == FEATURE_IPV6)
#define AT_GetIpv6Capability()          (AT_GetCommPsCtxAddr()->ucIpv6Capability)
#endif

#define AT_PS_GET_SHARE_PDP_FLG()       (AT_GetCommPsCtxAddr()->ucSharePdpFlag)

/* 获取APP客户端ID */
#define AT_APP_GET_CLIENT_ID()          (gastAtClientTab[AT_CLIENT_TAB_APP_INDEX].usClientId)

/*----------------------------------------------------------------------
   ^NDISSTAT: <stat>[,<err_code>[,<wx_stat>[,<PDP_type]]]
   <err_code> value defined as follows:
   0      - Unknown error/Unspecified error
   others - (E)SM Cause
            SM Cause is defined in TS 24.008 section 10.5.6.6
            ESM Cause is defined in TS 24.301 section 9.9.4.4
*---------------------------------------------------------------------*/
#define AT_NDISSTAT_ERR_UNKNOWN         (0)

/* NDIS拨号命令NDISCONN，NDISDUP的输入参数的 index */
#define AT_NDIS_CID_IDX                 (0)
#define AT_NDIS_CONN_IDX                (1)
#define AT_NDIS_APN_IDX                 (2)
#define AT_NDIS_NAME_IDX                (3)
#define AT_NDIS_PWD_IDX                 (4)
#define AT_NDIS_AUTH_IDX                (5)

/* 获取NDIS客户端ID */
#define AT_NDIS_GET_CLIENT_ID()         (gastAtClientTab[AT_CLIENT_TAB_NDIS_INDEX].usClientId)

#define AT_APS_IP6_ADDR_PREFIX_BYTE_LEN (8)

#define AT_MAC_LEN                      (6)
#define AT_IPV6_ADDR_PREFIX_BYTE_LEN    (8)
#define AT_IPV6_ADDR_PREFIX_BIT_LEN     (64)

#define AT_REG_FC                       (1)
#define AT_DEREG_FC                     (0)

/* QOS_TRAFFIC_CLASS */
#define AT_QOS_TRAFFIC_CLASS_SUBSCRIBE      (0)
#define AT_QOS_TRAFFIC_CLASS_CONVERSATIONAL (1)
#define AT_QOS_TRAFFIC_CLASS_STREAMING      (2)
#define AT_QOS_TRAFFIC_CLASS_INTERACTIVE    (3)
#define AT_QOS_TRAFFIC_CLASS_BACKGROUND     (4)

#define AT_IPV6_STR_RFC2373_TOKENS      (8)                 /* 分隔符标记的最大个数 */

#define AT_IPPROTO_UDP                  (0x11)              /* IP头部中UDP协议号*/
#define AT_IP_VERSION                   (4)                 /* IP头部中IP V4版本号 */
#define AT_IP_DEF_TTL                   (0xFF)              /* IP头部中IP TTL缺省值 */
#define AT_IP_RAND_ID                   (61468)             /* IP头部ID值，随机取*/
#define AT_IP_HDR_LEN                   (20)                /* IP 头部长度 */
#define AT_UDP_HDR_LEN                  (8)                 /* UDP 头部长度 */

/* 定义用户的CID，目前应用只会下发1，2，3 */
#define AT_PS_USER_CID_1                (1)
#define AT_PS_USER_CID_2                (2)
#define AT_PS_USER_CID_3                (3)

/* PS域呼叫无效CID */
#define AT_PS_CALL_INVALID_CID          (0xFF)
#define AT_PS_CALL_INVALID_CALLID       (0xFF)

/* IPV6地址后8个字节全零，则认为是无效的 */
#define AT_PS_IS_IPV6_ADDR_IID_VALID(aucIpv6Addr)\
            !((aucIpv6Addr[8] == 0x00) && (aucIpv6Addr[9] == 0x00)\
             && (aucIpv6Addr[10] == 0x00) && (aucIpv6Addr[11] == 0x00)\
             && (aucIpv6Addr[12] == 0x00) && (aucIpv6Addr[13] == 0x00)\
             && (aucIpv6Addr[14] == 0x00) && (aucIpv6Addr[15] == 0x00))

#if (FEATURE_ON == FEATURE_IPV6)
/* IPv6 接口ID, 长度为64bit */
#define AT_PS_IPV6_IID_LEN              (8)
#define AT_PS_IPV6_IID_OFFSET           (8)
#endif

#define AT_PS_INVALID_RMNET_ID          (0xFFFF)
#define AT_PS_INVALID_IFACE_ID          (0xFF)

#if (FEATURE_ON == FEATURE_IPV6)
#define AT_PS_IS_PDP_TYPE_SUPPORT(pdptype)  \
            ( (TAF_PDP_IPV4 == (pdptype))   \
             || (TAF_PDP_IPV6 == (pdptype)) \
             || (TAF_PDP_IPV4V6 == (pdptype)))
#else
#define AT_PS_IS_PDP_TYPE_SUPPORT(pdptype)  \
            (TAF_PDP_IPV4 == (pdptype))
#endif

#define AT_PS_IS_RABID_VALID(ucRabId)\
    (((ucRabId) >= AT_PS_MIN_RABID) && ((ucRabId) <= AT_PS_MAX_RABID))

#define AT_PS_GET_CURR_CMD_OPT(index)   (gastAtClientTab[index].CmdCurrentOpt)
#define AT_PS_GET_CURR_DATA_MODE(index) (gastAtClientTab[index].DataMode)
#define AT_PS_GET_PPPID(index)          (gastAtClientTab[index].usPppId)

#define AT_PS_GET_RPT_CONN_RSLT_FUNC_TBL_PTR()      (g_astAtRptConnectedResultTab)
#define AT_PS_GET_RPT_CONN_RSLT_FUNC_TBL_SIZE()     (AT_ARRAY_SIZE(g_astAtRptConnectedResultTab))

#define AT_PS_GET_RPT_END_RSLT_FUNC_TBL_PTR()       (g_astAtRptEndedResultTab)
#define AT_PS_GET_RPT_END_RSLT_FUNC_TBL_SIZE()      (AT_ARRAY_SIZE(g_astAtRptEndedResultTab))

#define AT_PS_GET_SND_PDP_ACT_IND_FUNC_TBL_PTR()    (g_astAtSndPdpActIndTab)
#define AT_PS_GET_SND_PDP_ACT_IND_FUNC_TBL_SIZE()   (AT_ARRAY_SIZE(g_astAtSndPdpActIndTab))

#define AT_PS_GET_SND_PDP_DEACT_IND_FUNC_TBL_PTR()  (g_astAtSndPdpDeActIndTab)
#define AT_PS_GET_SND_PDP_DEACT_IND_FUNC_TBL_SIZE() (AT_ARRAY_SIZE(g_astAtSndPdpDeActIndTab))

#define AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR()    (g_astAtChdataRnicRmNetIdTab)
#define AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE()   (AT_ARRAY_SIZE(g_astAtChdataRnicRmNetIdTab))

#define AT_PS_GET_APP_CALL_RNIC_IFACE_ID_TBL_PTR()  (g_astAtPsAppCallRnicIFaceIdTab)
#define AT_PS_GET_APP_CALL_RNIC_IFACE_ID_TBL_SIZE() (AT_ARRAY_SIZE(g_astAtPsAppCallRnicIFaceIdTab))

#define AT_PS_GET_NDIS_CALL_IFACE_ID_TBL_PTR()      (g_astAtPsNdisCallIFaceIdTab)
#define AT_PS_GET_NDIS_CALL_IFACE_ID_TBL_SIZE()     (AT_ARRAY_SIZE(g_astAtPsNdisCallIFaceIdTab))

#define AT_PS_GET_FC_IFACE_ID_TBL_PTR()             (g_astAtPsFcIFaceIdTab)
#define AT_PS_GET_FC_IFACE_ID_TBL_SIZE()            (AT_ARRAY_SIZE(g_astAtPsFcIFaceIdTab))

#define AT_PS_DIAL_RAT_TYPE_NO_ASTRICT              (0)
#define AT_PS_DIAL_RAT_TYPE_1X_OR_HRPD              (24)
#define AT_PS_DIAL_RAT_TYPE_LTE_OR_EHRPD            (36)

#define IMS_PCSCF_ADDR_PORT_MAX         (65535)

#define AT_BUILD_EXRABID(i,j)           (((i << 6) & 0xC0) | (j & 0x3F))
#define AT_GET_RABID_FROM_EXRABID(i)    (i & 0x3F)

#define AT_IPV6_ADDR_MASK_FORMAT_STR_LEN        (256)
#define AT_IPV6_ADDR_DEC_FORMAT_STR_LEN         (128)
#define AT_IPV6_ADDR_COLON_FORMAT_STR_LEN       (40)
#define AT_IPV6_ADDR_DEC_TOKEN_NUM              (16)
#define AT_IPV6_ADDR_HEX_TOKEN_NUM              (8)
#define AT_IP_STR_DOT_DELIMITER                 '.'
#define AT_IP_STR_COLON_DELIMITER               ':'

#define AT_WAIT_WLAN_ACT_PDN_CNF_TIMER_LEN       (25 * 1000)                    /* AT等待Wlan 回复active cnf 定时器时长 */

/* AT_WAIT_WLAN_ACT_PDN_CNF_TIMER定时器名称 */
#define AT_SET_WLAN_ACT_PDN_CNF_TMR_NAME(ulTmrName)\
            (ulTmrName)  = AT_WAIT_WLAN_ACT_PDN_CNF_TIMER;\
            (ulTmrName) |= AT_INTERNAL_PROCESS_TYPE

/* AT_WAIT_WLAN_ACT_PDN_CNF_TIMER定时器参数 */
#define AT_SET_WLAN_ACT_PDN_CNF_TMR_PARAM(ulTmrParam, ucIndex, ucCallId)\
            ((ulTmrParam) = ((ucCallId) << 8) | (ucIndex))

/* 从AT_WAIT_WLAN_ACT_PDN_CNF_TIMER定时器超时消息中获取CALLID */
#define AT_GET_WLAN_ACT_PDN_CNF_CALLID_FROM_TMR_PARAM(ulTmrParam)\
            ((VOS_UINT8)(((ulTmrParam) & 0x0000FF00) >> 8))

/* 从AT_WAIT_WLAN_ACT_PDN_CNF_TIMER定时器超时消息中获取端口ID */
#define AT_GET_WLAN_ACT_PDN_CNF_CLIENTID_FROM_TMR_PARAM(ulTmrParam)\
            ((VOS_UINT8)((ulTmrParam) & 0x000000FF))

#define AT_WAIT_WLAN_DEACT_PDN_CNF_TIMER_LEN       (7 * 1000)                   /* AT等待Wlan 回复deactive cnf 定时器时长 */

/* AT_WAIT_WLAN_DEACT_PDN_CNF_TIMER定时器名称 */
#define AT_SET_WLAN_DEACT_PDN_CNF_TMR_NAME(ulTmrName)\
            (ulTmrName)  = AT_WAIT_WLAN_DEACT_PDN_CNF_TIMER;\
            (ulTmrName) |= AT_INTERNAL_PROCESS_TYPE

/* AT_WAIT_WLAN_DEACT_PDN_CNF_TIMER定时器参数 */
#define AT_SET_WLAN_DEACT_PDN_CNF_TMR_PARAM(ulTmrParam, ucIndex, ucCallId)\
            ((ulTmrParam) = ((ucCallId) << 8) | (ucIndex))

/* 从AT_WAIT_WLAN_DEACT_PDN_CNF_TIMER定时器超时消息中获取CALLID */
#define AT_GET_WLAN_DEACT_PDN_CNF_CALLID_FROM_TMR_PARAM(ulTmrParam)\
            ((VOS_UINT8)(((ulTmrParam) & 0x0000FF00) >> 8))

/* 从AT_WAIT_WLAN_DEACT_PDN_CNF_TIMER定时器超时消息中获取端口ID */
#define AT_GET_WLAN_DEACT_PDN_CNF_CLIENTID_FROM_TMR_PARAM(ulTmrParam)\
            ((VOS_UINT8)((ulTmrParam) & 0x000000FF))


/* AT_PROTECT_PDN_IN_DATA_SYS_TIMER定时器名称 */
#define AT_SET_PROTECT_PDN_IN_DATA_SYS_TMR_NAME(ulTmrName)\
            (ulTmrName)  = AT_PROTECT_PDN_IN_DATA_SYS_TIMER;\
            (ulTmrName) |= AT_INTERNAL_PROCESS_TYPE

/* AT_PROTECT_PDN_IN_DATA_SYS_TIMER定时器参数 */
#define AT_SET_PROTECT_PDN_IN_DATA_SYS_TMR_PARAM(ulTmrParam, ucIndex, ucCallId)\
            ((ulTmrParam) = ((ucCallId) << 8) | (ucIndex))

/* 从AT_PROTECT_PDN_IN_DATA_SYS_TIMER定时器超时消息中获取CALLID */
#define AT_GET_PROTECT_PDN_IN_DATA_SYS_CALLID_FROM_TMR_PARAM(ulTmrParam)\
            ((VOS_UINT8)(((ulTmrParam) & 0x0000FF00) >> 8))

/* 从AT_PROTECT_PDN_IN_DATA_SYS_TIMER定时器超时消息中获取端口ID */
#define AT_GET_PROTECT_PDN_IN_DATA_SYS_CLIENTID_FROM_TMR_PARAM(ulTmrParam)\
            ((VOS_UINT8)((ulTmrParam) & 0x000000FF))

/* 从CID获得SERVICE TYPE */
#define AT_PS_CALL_GET_SERVICE_TYPE_FROM_CID(ucCid)\
            ((VOS_INT8)(ucCid + 0x20))

/* 从SERVICE TYPE获得CID */
#define AT_PS_CALL_GET_CID_FROM_SERVICE_TYPE(cServiceType)\
            ((VOS_UINT8)(cServiceType - 0x20))

/* SERVICE TYPE是否无效 */
#define AT_PS_IS_SERVICE_TYPE_VALID(cServiceType) \
            (cServiceType > 0x20)

/* A类地址0.0.0.0 ~ 127.255.255.255 */
#define AT_IPV4_CLASS_A_BEGIN           (0x00000000)
#define AT_IPV4_CLASS_A_END             (0x7FFFFFFF)

/* B类地址128.0.0.0 ~ 191.255.255.255 */
#define AT_IPV4_CLASS_B_BEGIN           (0x80000000)
#define AT_IPV4_CLASS_B_END             (0xBFFFFFFF)

/* C类地址192.0.0.0 ~ 223.255.255.255 */
#define AT_IPV4_CLASS_C_BEGIN           (0xC0000000)
#define AT_IPV4_CLASS_C_END             (0xDFFFFFFF)

/* IPV4 是否为A类地址 */
#define AT_IS_CLASS_A_IPV4_ADDR(ulIpv4Addr)  \
            (ulIpv4Addr <= AT_IPV4_CLASS_A_END)

/* IPV4 是否为B类地址 */
#define AT_IS_CLASS_B_IPV4_ADDR(ulIpv4Addr)  \
            ((ulIpv4Addr >= AT_IPV4_CLASS_B_BEGIN) && (ulIpv4Addr <= AT_IPV4_CLASS_B_END))

/* A类地址默认子网掩码 255.0.0.0 */
#define AT_IPV4_CLASS_A_SUBNET_MASK     (0xFF000000)

/* B类地址默认子网掩码 255.255.0.0 */
#define AT_IPV4_CLASS_B_SUBNET_MASK     (0xFFFF0000)

/* C类地址默认子网掩码 255.255.255.0 */
#define AT_IPV4_CLASS_C_SUBNET_MASK     (0xFFFFFF00)


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum AT_PDP_STATUS_ENUM
{
    AT_PDP_STATUS_DEACT                    = 0,
    AT_PDP_STATUS_ACT                      = 1,
    AT_PDP_STATUS_HOLD                     = 2,
    AT_PDP_STATUS_BUTT
};
typedef VOS_UINT32 AT_PDP_STATUS_ENUM_UINT32;

/*****************************************************************************
 枚举名称   : AT_PDP_TYPE_ENUM_ENUM
 协议表格   :
 ASN.1 描述 :
 枚举说明   : PDP类型
*****************************************************************************/
enum AT_PDP_TYPE_ENUM
{
    AT_PDP_IPV4                         = 1,
    AT_PDP_IPV6                         = 2,
    AT_PDP_IPV4V6                       = 3,
    AT_PDP_PPP                          = 4,
    AT_PDP_TYPE_BUTT
};
typedef VOS_UINT8 AT_PDP_TYPE_ENUM_ENUM_U8;

/*****************************************************************************
 结构名称   : AT_IPV6_STR_TYPE_ENUM
 协议表格   :
 ASN.1 描述 :
 结构说明   : IPV6 String格式枚举
              HEX为RFC2373要求使用':'作为分隔符
              DEX为RFC要求使用'.'作为分隔符
*****************************************************************************/
enum AT_IPV6_STR_TYPE_ENUM
{
    AT_IPV6_STR_TYPE_HEX                = 0x01,
    AT_IPV6_STR_TYPE_DEC                = 0x02,

    AT_IPV6_STR_TYPE_BUTT               = 0xFF
};
typedef VOS_UINT8 AT_IPV6_STR_TYPE_ENUM_UINT8;

/*****************************************************************************
 结构名称   : AT_IP_ADDR_TYPE_ENUM
 协议表格   :
 ASN.1 描述 :
 结构说明   : 区分是SOURCE还是LOCAL的IP ADDR
*****************************************************************************/
enum AT_IP_ADDR_TYPE_ENUM
{
    AT_IP_ADDR_TYPE_SOURCE              = 0x01,
    AT_IP_ADDR_TYPE_LOCAL               = 0x02,

    AT_IP_ADDR_TYPE_BUTT                = 0xFF
};
typedef VOS_UINT8 AT_IP_ADDR_TYPE_ENUM_UINT8;

/*****************************************************************************
 枚举名称   : AT_HILINK_MODE_ENUM
 协议表格   :
 ASN.1 描述 :
 枚举说明   : HiLink模式
*****************************************************************************/
enum AT_HILINK_MODE_ENUM
{
    AT_HILINK_NORMAL_MODE,
    AT_HILINK_GATEWAY_MODE,
    AT_HILINK_MODE_BUTT
};
typedef VOS_UINT8 AT_HILINK_MODE_ENUM_U8;

/* APP拨号状态*/
/*****************************************************************************
 枚举名称   : AT_APP_CONN_STATE_ENUM
 协议表格   :
 ASN.1 描述 :
 枚举说明   : APP拨号状态
*****************************************************************************/
enum AT_APP_CONN_STATE_ENUM
{
    AT_APP_DIALING,                     /*0: indicates is connecting*/
    AT_APP_DIALED,                      /*1: indicates connected*/
    AT_APP_UNDIALED,                    /*2: indicates disconnected*/
    AT_APP_BUTT
};
typedef VOS_UINT32 AT_APP_CONN_STATE_ENUM_U32;

/*****************************************************************************
 PPP拨号后的速率气泡显示，分两种情况处理:
 1.2G模式下，则根据当前驻留的小区模式是GSM/GPRS/EDGE来决定速率气泡的显示,对应如下:
     GSM          - 9600
     GPRS         - 85600
     GPRS Class33 - 107800
     EDGE         - 236800
     EDGE Class33 - 296000
     默认 -
 2.3g模式下，则根据RRC版本和HSDPA的category信息来决定速率气泡的显示，对应如下:
     RRC版本为R99   - 384000
     RRC版本为非R99 - 判断HSDPA的category信息
                   6  - 3600000
                   8  - 7200000
                   9  - 10200000
                   10 - 14400000
                   12 - 1800000
                   14 - 21600000
                   18 - 28800000
                   24 - 43200000
                   26 - 57600000
                   28 - 86400000
     有扩展的category的话，默认 - 21600000
     无扩展的category的话，默认 - 7200000
*****************************************************************************/
enum PPP_RATE_DISPLAY_ENUM
{
    PPP_RATE_DISPLAY_GSM = 0,
    PPP_RATE_DISPLAY_GPRS,
    PPP_RATE_DISPLAY_GPRS_CALSS33,
    PPP_RATE_DISPLAY_EDGE,
    PPP_RATE_DISPLAY_EDGE_CALSS33,

    PPP_RATE_DISPLAY_R99,
    PPP_RATE_DISPLAY_DPA_CATEGORY_6,
    PPP_RATE_DISPLAY_DPA_CATEGORY_8,
    PPP_RATE_DISPLAY_DPA_CATEGORY_9,
    PPP_RATE_DISPLAY_DPA_CATEGORY_10,
    PPP_RATE_DISPLAY_DPA_CATEGORY_12,
    PPP_RATE_DISPLAY_DPA_CATEGORY_14,
    PPP_RATE_DISPLAY_DPA_CATEGORY_18,
    PPP_RATE_DISPLAY_DPA_CATEGORY_24,
    PPP_RATE_DISPLAY_DPA_CATEGORY_26,
    PPP_RATE_DISPLAY_DPA_CATEGORY_28,

    PPP_RATE_DISPLAY_BUTT
};
typedef VOS_UINT32 PPP_RATE_DISPLAY_ENUM_UINT32;

/*****************************************************************************
 枚举名    : AT_CH_DATA_CHANNEL_ENUM
 结构说明  : AT^CHDATA命令设置的<datachannelid>的取值
*****************************************************************************/
enum AT_CH_DATA_CHANNEL_ENUM
{
    AT_CH_DATA_CHANNEL_ID_1             = 1,
    AT_CH_DATA_CHANNEL_ID_2,
    AT_CH_DATA_CHANNEL_ID_3,
    AT_CH_DATA_CHANNEL_ID_4,
    AT_CH_DATA_CHANNEL_ID_5,
    AT_CH_DATA_CHANNEL_ID_6,
    AT_CH_DATA_CHANNEL_ID_7,

    AT_CH_DATA_CHANNEL_BUTT
};
typedef VOS_UINT32 AT_CH_DATA_CHANNEL_ENUM_UINT32;


enum AT_PS_DATA_SYS_ENUM
{
    AT_PS_DATA_SYS_NONE                      = 0,
    AT_PS_DATA_SYS_CELLULAR                  = 1,
    AT_PS_DATA_SYS_WLAN                      = 2,

    AT_PS_DATA_SYS_BUTT
};
typedef VOS_UINT8 AT_PS_DATA_SYS_ENUM_UINT32;


/*****************************************************************************
  4 消息头定义
*****************************************************************************/

/*****************************************************************************
  5 消息定义
*****************************************************************************/

/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/

typedef struct
{
    VOS_UINT32                          ulDLEnqueuedPkts;                       /*下行数据包总个数*/
    VOS_UINT32                          ulDLDequeuedPkts;                       /*下行发送个数*/
    VOS_UINT32                          ulDLDroppedPkts;                        /*下行丢包个数*/
    VOS_UINT32                          ulDLMaxBlkPkts;                         /*下行队列最大阻塞个数*/
    VOS_UINT32                          ulDLMaxDequeueOnce;                     /*下行单次发送包个数最大值*/
} AT_AP_DL_DATA_Q_STAT_ST;


/* DHCP配置，全主机序*/
typedef struct
{
    VOS_UINT32                          ulIPAddr;                               /* IP 地址，网侧分配*/
    VOS_UINT32                          ulSubNetMask;                           /* 子网掩码地址，根据IP地址计算*/
    VOS_UINT32                          ulGateWay;                              /* 网关地址，也是本DHCP Server的地址*/
    VOS_UINT32                          ulPrimDNS;                              /* 主 DNS地址，网侧分配*/
    VOS_UINT32                          ulSndDNS;                               /* 次 DNS地址，网侧分配*/
}AT_DHCP_CONFIG_STRU;


typedef struct
{
    VOS_UINT16                          usClientID;                             /* Client ID*/
    VOS_UINT8                           ucRabID;                                /* Rab ID*/
    VOS_UINT8                           ucCid;                                  /* CID*/
    AT_PDP_STATE_ENUM_U8                enState;                                /* State*/
    VOS_UINT8                           aucReserved[3];
    VOS_UINT32                          ulFlowCtrlState;                        /* Flow Ctrl State ; 1: flow ctrl ,0: no flow ctrl*/
    VOS_UINT32                          ulSpeed;                                /* Um Speed*/

    VOS_UINT32                          ulDLMaxRate;                            /* 理论最大下行速率*/
    VOS_UINT32                          ulULMaxRate;                            /* 理论最大上行速率*/
    VOS_UINT32                          ulDLCurrentRate;                        /* 当前下行速率*/
    VOS_UINT32                          ulULCurrentRate;                        /* 当前上行速率*/
    VOS_UINT32                          ulRateCalcPeriod;                       /* 速率统计周期*/
    AT_DHCP_CONFIG_STRU                 stDhcpCfg;
}AT_CTRL_ENTITY_STRU;

typedef struct AT_DHCP_PARA
{
    VOS_UINT16                          usClientID;                             /* Client ID*/
    VOS_UINT8                           ucRabID;                                /* Rab ID*/
    VOS_UINT8                           ucCid;                                  /* CID*/
    VOS_UINT32                          ulSpeed;                                /* Um Speed*/
    VOS_UINT32                          ulDLMaxRate;                            /* 理论最大下行速率*/
    VOS_UINT32                          ulULMaxRate;                            /* 理论最大上行速率*/
    AT_DHCP_CONFIG_STRU                 stDhcpCfg;
}AT_DHCP_PARA_STRU;

typedef struct
{
  VOS_UINT8                             ucCode;                                 /* Request code */
  VOS_UINT8                             ucId;                                   /* Identification */
  VOS_UINT16                            usLength;                               /* Length of packet */
}AT_PPP_FRAME_HEAD_STRU;

/* IP地址结构 U32类型，网络字节序存储*/
typedef struct
{
    VOS_UINT32                          ulIpAddr;                               /*U32类型，网络字节序存储*/
}IN_ADDR_ST;


typedef struct
{
    VOS_UINT8                           ucType;
    VOS_UINT8                           ucCode;
    VOS_UINT16                          usCheckSum;
    VOS_UINT16                          usId;
    VOS_UINT16                          usSeq;
}ICMP_ECHO_HEAD_ST;

typedef struct
{
    VOS_UINT32                          ulIPAddr;                               /* IP 地址*/
    VOS_UINT32                          ulPrimDNS;                              /* 主 DNS地址*/
    VOS_UINT32                          ulSndDNS;                               /* 次 DNS地址*/
    VOS_UINT32                          ulPrimWINNS;                            /* 鱓INNS*/
    VOS_UINT32                          ulSncWINNS;                             /* 副WINNS */
}AT_DHCP_SETUP_PARAM_ST;


/* DHCP 服务器信息结构*/
typedef struct
{
    AT_DHCP_CONFIG_STRU                 stDHCPConfig;                           /* DHCP服务器配置*/
    VOS_UINT32                          ulDHCPServerOn;                         /* DHCP是否已经打开*/
}AT_DHCP_ENTITY_ST;

/* PDP状态上报函数使用的结构体*/
typedef struct
{
    VOS_UINT32                          ulspeed;                                /* 连接速度，单位bps */
    AT_PDP_STATUS_ENUM_UINT32           enActiveSatus;                          /* 激活结果，0为成功，其他为失败 */
}AT_NDIS_PRO_STRU;

/*****************************************************************************
 结构名称   : AT_PDP_SAVE_STATE_STRU
 协议表格   :
 ASN.1 描述 :
 结构说明   : 保存拨号状态
*****************************************************************************/
typedef struct
{

    AT_PDP_STATE_ENUM_U8                enIpv4State;
    AT_PDP_STATE_ENUM_U8                enIpv6State;
    AT_PDP_STATE_ENUM_U8                enIpv4v6State;

} AT_PDP_SAVE_STATE_INFO_STRU;

/*****************************************************************************
 结构名    : AT_CLINTID_RABID_MAP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulUsed;   /* 指定FCID对应的结点是否有效，VOS_TRUE:有效，VOS_FALSE:无效 */
    VOS_UINT32                          ulRabIdMask;
    MODEM_ID_ENUM_UINT16                enModemId;
    FC_PRI_ENUM_UINT8                   enFcPri;
    VOS_UINT8                           aucReserve[1];                          /* 保留 */
} AT_FCID_MAP_STRU;

/*****************************************************************************
 结构名     : AT_IPHDR_STRU
 协议表格   :
 ASN.1描述  :
 结构说明   : IPv4 packet header 结构
*****************************************************************************/
typedef struct
{
#if (VOS_LITTLE_ENDIAN == VOS_BYTE_ORDER)                   /* 小端字节序*/
    VOS_UINT8                           ucIpHdrLen  :4;     /* IP头部长度 */
    VOS_UINT8                           ucIpVer     :4;     /* IP版本号*/
#elif (VOS_BIG_ENDIAN == VOS_BYTE_ORDER)                    /* 大端字节序*/
    VOS_UINT8                           ucIpVer     :4;     /* IP版本号*/
    VOS_UINT8                           ucIpHdrLen  :4;     /* IP头部长度 */
#else
#error  "Please fix Macro VOS_BYTE_ORDER"                   /* VOS_BYTE_ORDER未定义*/
#endif
    VOS_UINT8                           ucServiceType;      /* IP TOS字段*/
    VOS_UINT16                          usTotalLen;         /* IP数据包总长度*/
    VOS_UINT16                          usIdentification;   /* IP数据包ID*/
    VOS_UINT16                          usOffset;           /* IP分片偏移量*/
    VOS_UINT8                           ucTTL;              /* IPTTL*/
    VOS_UINT8                           ucProtocol;         /* IP数据载荷部分协议*/
    VOS_UINT16                          usCheckSum;         /* IP头部校验和*/
    VOS_UINT32                          ulSrcAddr;          /* 源IP地址*/
    VOS_UINT32                          ulDstAddr;          /* 目的IP地址*/
} AT_IPHDR_STRU;

/*****************************************************************************
 结构名     : AT_UDP_HDR_STRU
 协议表格   :
 ASN.1描述  :
 结构说明   : UDP header 结构
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usSrcPort;          /* 源端口 */
    VOS_UINT16                          usDstPort;          /* 目的端口 */
    VOS_UINT16                          usLen;              /* UDP包长度 */
    VOS_UINT16                          usChecksum;         /* UDP校验和 */
} AT_UDP_HDR_STRU;

/*****************************************************************************
 结构名     : AT_UDP_PACKET_FORMAT_STRU
 协议表格   :
 ASN.1描述  :
 结构说明   : UDP packet 结构
*****************************************************************************/
typedef struct
{

    AT_IPHDR_STRU                       stIpHdr;            /* IP头 */
    AT_UDP_HDR_STRU                     stUdpHdr;           /* UDP头 */
    VOS_UINT32                          ulBody;
} AT_UDP_PACKET_FORMAT_STRU;

/*****************************************************************************
 结构名    : AT_PS_RMNET_ID_TAB
 结构说明  : PS域拨号网卡和ModemId,cid,FcId的映射表
*****************************************************************************/
typedef struct
{
    MODEM_ID_ENUM_UINT16                enModemId;
    FC_ID_ENUM_UINT8                    enFcId;
    VOS_UINT8                           ucUsrCid;
}AT_PS_RMNET_ID_TAB;

/*****************************************************************************
 结构名    : AT_CHDATA_RNIC_RMNET_ID_STRU
 结构说明  : AT^CHDATA与RNIC网卡映射关系的结构
*****************************************************************************/
typedef struct
{
    AT_CH_DATA_CHANNEL_ENUM_UINT32  enChdataValue;
    RNIC_DEV_ID_ENUM_UINT8          enRnicRmNetId;
    PS_IFACE_ID_ENUM_UINT8          enIfaceId;
    VOS_UINT8                       aucReserved[2];

}AT_CHDATA_RNIC_RMNET_ID_STRU;

/*****************************************************************************
 结构名    : AT_PS_APP_CALL_RNIC_IFACE_ID_STRU
 结构说明  : WAN PS APP CALL与RNIC网卡映射关系的结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                       ucCallIndex;
    RNIC_DEV_ID_ENUM_UINT8          enRnicRmNetId;
    PS_IFACE_ID_ENUM_UINT8          enIfaceId;
    VOS_UINT8                       ucReserved;
}AT_PS_APP_CALL_RNIC_IFACE_ID_STRU;

/*****************************************************************************
 结构名    : AT_PS_NDIS_CALL_IFACE_ID_STRU
 结构说明  : WAN PS NDIS CALL与IFACE映射关系的结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                       ucCallIndex;
    PS_IFACE_ID_ENUM_UINT8          enIfaceId;
    VOS_UINT8                       ucReserved1;
    VOS_UINT8                       ucReserved2;
}AT_PS_NDIS_CALL_IFACE_ID_STRU;

/*****************************************************************************
 结构名    : AT_PS_FC_IFACE_ID_STRU
 结构说明  : FC与IFACE ID映射关系的结构
*****************************************************************************/
typedef struct
{
    FC_ID_ENUM_UINT8                enFcId;
    PS_IFACE_ID_ENUM_UINT8          enIfaceId;
    VOS_UINT8                       ucReserved1;
    VOS_UINT8                       ucReserved2;
}AT_PS_FC_IFACE_ID_STRU;

/*****************************************************************************
 结构名    : AT_DISPLAY_RATE_STRU
 结构说明  : 速率显示分为上行和下行速率
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucDlSpeed[AT_AP_SPEED_STRLEN + 1];
    VOS_UINT8                           ucUlSpeed[AT_AP_SPEED_STRLEN + 1];
    VOS_UINT8                           aucReserved[2];
}AT_DISPLAY_RATE_STRU;

/*****************************************************************************
 结构名  : AT_PS_WLAN_PDN_ACT_ERR_CODE_MAP_STRU
 结构说明: PS错误码与WLAN PDN ACT错误码映射结构
*****************************************************************************/
typedef struct
{
    TAF_PS_CAUSE_ENUM_UINT32            enPsCause;
    VOS_UINT32                          ulWlanCause;
} AT_PS_WLAN_PDN_ACT_ERR_CODE_MAP_STRU;

/*****************************************************************************
 结构名  : AT_PS_WLAN_PDN_DEACT_ERR_CODE_MAP_STRU
 结构说明: PS错误码与WLAN PDN DEACT错误码映射结构
*****************************************************************************/
typedef struct
{
    TAF_PS_CAUSE_ENUM_UINT32            enPsCause;
    VOS_UINT32                          ulWlanCause;
} AT_PS_WLAN_PDN_DEACT_ERR_CODE_MAP_STRU;


/*****************************************************************************
  7 UNION定义
*****************************************************************************/

/*****************************************************************************
  8 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  9 全局变量声明
*****************************************************************************/

/*记录闪电卡版本从上电到拨号成功启动时间，单位秒 */
extern VOS_UINT32                       g_ulLcStartTime;

extern AT_HILINK_MODE_ENUM_U8           g_enHiLinkMode;

extern AT_FCID_MAP_STRU                 g_stFcIdMaptoFcPri[];

extern CONST AT_PS_REPORT_CONN_RESULT_STRU    g_astAtRptConnectedResultTab[];

extern CONST AT_PS_REPORT_END_RESULT_STRU     g_astAtRptEndedResultTab[];

extern CONST AT_CHDATA_RNIC_RMNET_ID_STRU     g_astAtChdataRnicRmNetIdTab[];


/*****************************************************************************
  10 函数声明
*****************************************************************************/

/*****************************************************************************
 函 数 名  : AT_GetDhcpPara
 功能描述  : 获取DHCP参数(DHCP参数为网络序)
 输入参数  : pstConfig                  - DHCP参数(网络序)
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID AT_GetDhcpPara(
    AT_DHCP_PARA_STRU                  *pstConfig,
    AT_IPV4_DHCP_PARAM_STRU            *pstIpv4Dhcp
);

/******************************************************************************
 函 数 名  : AT_GetDisplayRate
 功能描述  : 获取空口理论带宽，从NAS获取，且将字符串型式转为整形
 输入参数  : *pstSpeed     ----    上下行速率结构
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :
******************************************************************************/
VOS_UINT32 AT_GetDisplayRate(
    VOS_UINT16                          usClientId,
    AT_DISPLAY_RATE_STRU               *pstSpeed
);

/******************************************************************************
 Function:      AT_CtrlGetPDPAuthType
 Description:    获取PC设置的PDP上下文中对应类型的数据
 Calls:
 Data Accessed:
 Data Updated:
 Input:
                 usTotalLen     PDP上下文内存长度
 Output:
 Return:        0   no auth
                1   pap
                2   chap
******************************************************************************/
PPP_AUTH_TYPE_ENUM_UINT8 AT_CtrlGetPDPAuthType(
    VOS_UINT32                          Value,
    VOS_UINT16                          usTotalLen
);

TAF_PDP_AUTH_TYPE_ENUM_UINT8 AT_ClGetPdpAuthType(
    VOS_UINT32                          Value,
    VOS_UINT16                          usTotalLen
);


VOS_UINT32 AT_Ipv4AddrAtoi(VOS_CHAR *pcString, VOS_UINT8 *pucNumber);


VOS_UINT32 AT_Ipv4AddrItoa(VOS_CHAR *pcString, VOS_UINT8 *pucNumber);

/*****************************************************************************
 函 数 名  : AT_Ipv4Addr2Str
 功能描述  : IPV4类型的地址转换为字符串类型
 输入参数  : pucNumber      - IPV4地址
 输出参数  : pcString       - 字符串
 返 回 值  : VOS_UINT32

*****************************************************************************/
VOS_UINT32 AT_Ipv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber
);

VOS_UINT32 AT_Ipv6AddrAtoi(VOS_CHAR *pcString, VOS_UINT8 *pucNumber);

VOS_VOID AT_PcscfIpv4Addr2Str(
    VOS_CHAR                           *pcString,
    VOS_UINT8                          *pucNumber
);

VOS_UINT32  AT_PortAtoI(
    VOS_CHAR                         *pcString,
    VOS_UINT32                       *pulValue
);

VOS_UINT32  AT_Ipv6PcscfDataToAddr(
    VOS_UINT8                          *pucStr,
    VOS_UINT8                          *pucIpAddr,
    VOS_UINT32                         *pulPortExistFlg,
    VOS_UINT32                         *pulPortNum

);

/*****************************************************************************
 函 数 名  : AT_Ipv6AddrToStr
 功能描述  : 将IPV6地址格式转换为字符串格式
 输入参数  : aucIpAddr[]    - IPV6地址(协议格式)
             enIpStrType    - IPV6字符串格式表达类型
 输出参数  : aucAddrStr[]   - IPV6地址(字符串格式)
 返 回 值  : VOS_OK         - 转换成功
             VOS_ERR        - 转换失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_Ipv6AddrToStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    AT_IPV6_STR_TYPE_ENUM_UINT8         enIpStrType
);

/*****************************************************************************
 函 数 名  : AT_Itoa
 功能描述  : 根据转换基数(10或16), 将整数转换为ASCII码, 将结果输出至字符串
 输入参数  : usValue    - 待转换为ASCII码的整数
             pcStr      - 输出结果的字符串
             usRadix    - 转换基数
 输出参数  : 无
 返 回 值  : VOS_CHAR*
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_CHAR* AT_Itoa(
    VOS_UINT16                          usValue,
    VOS_CHAR                           *pcStr,
    VOS_UINT16                          usRadix,
    VOS_UINT32                          ulLength
);
/******************************************************************************
 函 数 名  : AT_AtoI
 功能描述  : 字符串转整形
 输入参数  : pString 字符串
 输出参数  : 无
 返 回 值  : 整形 IP地址
 调用函数  :
 被调函数  :
******************************************************************************/
VOS_UINT64  AT_AtoI(
    VOS_UINT8                         *pString
);


/******************************************************************************
 函 数 名  : AT_AtoI
 功能描述  : 字符串转整形(可以带符号转换)
 输入参数  : pString 字符串
 输出参数  : 无
 返 回 值  : 整形 IP地址
 调用函数  :
 被调函数  :
******************************************************************************/
VOS_INT32  AT_AtoInt(
    VOS_UINT8                          *pString,
    VOS_INT32                          *pOut
);

/*****************************************************************************
 函 数 名  : AT_ConvertIpv6AddrToCompressedStr
 功能描述  : 将IPV6地址格式转换为字符串压缩格式
 输入参数  : aucIpAddr[]    - IPV6地址(协议格式)
             ucTokensNum    - 地址段个数
 输出参数  : aucAddrStr[]   - IPV6地址(字符串格式)
 返 回 值  : VOS_OK         - 转换成功
             VOS_ERR        - 转换失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_ConvertIpv6AddrToCompressedStr(
    VOS_UINT8                           aucAddrStr[],
    VOS_UINT8                           aucIpAddr[],
    VOS_UINT8                           ucTokensNum
);

/*****************************************************************************
 函 数 名  : AT_BuildUdpHdr
 功能描述  : 构造IP和UDP头部信息(用于构造测试使用的UDP包)
 输入参数  : pstUdpPkt  - UDP包信息
             usLen      - UDP包长度
             ulSrcAddr  - 源IP地址
             ulDstAddr  - 目标IP地址
             usSrcPort  - 源端口号
             usDstPort  - 目标端口号
 输出参数  : 无
 返 回 值  : VOS_OK     - 构造成功
             VOS_ERR    - 构造失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_BuildUdpHdr(
    AT_UDP_PACKET_FORMAT_STRU          *pstUdpPkt,
    VOS_UINT16                          usLen,
    VOS_UINT32                          ulSrcAddr,
    VOS_UINT32                          ulDstAddr,
    VOS_UINT16                          usSrcPort,
    VOS_UINT16                          usDstPort
);


/*****************************************************************************
 函 数 名  : AT_NidsGet3gppSmCauseByPsCause
 功能描述  : 将PS域呼叫错误码转换成3GPP协议定义的(E)SM Cause, 非3GPP协议定义
             的错误码统一转换成0(Unknown error/Unspecified error)
 输入参数  : enCause - PS域呼叫错误码
 输出参数  : 无
 返 回 值  : VOS_UINT16
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_Get3gppSmCauseByPsCause(
    TAF_PS_CAUSE_ENUM_UINT32            enCause
);

VOS_VOID AT_NDIS_ConnStatusChgProc(NCM_IOCTL_CONNECT_STUS_E enStatus);

#if (FEATURE_ON == FEATURE_IPV6)

VOS_VOID AT_PS_ProcIpv6RaFail(
    TAF_PS_IPV6_INFO_IND_STRU          *pstRaInfoNotifyInd,
    VOS_UINT8                           ucCallId
);


VOS_VOID AT_PS_ProcHoIpv6RaInfo(
    TAF_PS_IPV6_INFO_IND_STRU          *pstRaInfoNotifyInd,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucCallId
);


VOS_VOID AT_PS_FillIpv6RaInfo(
    TAF_PS_IPV6_INFO_IND_STRU          *pstRaInfoNotifyInd,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);
#endif


VOS_VOID  AT_ModemPsRspPdpActEvtRejProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pEvent
);



VOS_VOID  AT_ModemPsRspPdpActEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pEvent
);


VOS_VOID  AT_ModemPsRspPdpDeactEvtCnfProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pEvent
);

/*****************************************************************************
 函 数 名  : AT_MODEM_ProcCallEndedEvent
 功能描述  : 处理PS_CALL_END_CNF事件
 输入参数  : ucIndex  - 端口索引
             pstEvent - ID_EVT_TAF_PS_CALL_END_CNF事件指针
 输出参数  : 无
 返 回 值  : VOS_VOID
*****************************************************************************/
VOS_VOID AT_MODEM_ProcCallEndCnfEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_END_CNF_STRU           *pstEvent
);


/*****************************************************************************
 函 数 名  : AT_MODEM_HangupCall
 功能描述  : 挂断PPP拨号连接
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX  - ATC返回码
*****************************************************************************/
VOS_UINT32 AT_MODEM_HangupCall(VOS_UINT8 ucIndex);


VOS_VOID  AT_AnswerPdpActInd(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
);


VOS_VOID  AT_ModemPsRspPdpDeactivatedEvtProc(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_IND_STRU *pEvent
);

/*****************************************************************************
 函 数 名  : AT_DeRegModemPsDataFCPoint
 功能描述  : 去注册Modem端口流控点。
 输入参数  : MN_CLIENT_ID_T                      usClientID,
             VOS_UINT8                           ucRabId
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
extern VOS_UINT32 AT_DeRegModemPsDataFCPoint(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucRabId
);

#if( FEATURE_ON == FEATURE_CSD )
/*****************************************************************************
 函 数 名  : AT_RegModemVideoPhoneFCPoint
 功能描述  : 注册Modem端口CST流控点。
 输入参数  : FC_ID_ENUM_UINT8                    enFcId
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_RegModemVideoPhoneFCPoint(
    VOS_UINT8                           ucIndex,
    FC_ID_ENUM_UINT8                    enFcId
);

/*****************************************************************************
 函 数 名  : AT_DeRegModemVideoPhoneFCPoint
 功能描述  : 去注册Modem端口CST流控点。
 输入参数  :
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_DeRegModemVideoPhoneFCPoint(VOS_UINT8 ucIndex);
#endif

VOS_VOID AT_PS_SendNdisIPv6PdnInfoCfgReq(
    MODEM_ID_ENUM_UINT16                 enModemId,
    AT_PS_CALL_ENTITY_STRU              *pstCallEntity,
    TAF_PS_IPV6_INFO_IND_STRU           *pIPv6RaNotify
);

#if(FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
VOS_VOID AT_PS_SendNdisIPv6IfaceUpCfgInd(
    AT_PS_CALL_ENTITY_STRU              *pstCallEntity,
    TAF_PS_IPV6_INFO_IND_STRU           *pIPv6RaNotify
);
#endif


VOS_VOID AT_NotifyFcWhenPdpModify(
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent,
    FC_ID_ENUM_UINT8                    enFcId
);

#if (FEATURE_ON == FEATURE_LTE)
/* AT模块给FTM 模块发送消息 */
VOS_UINT32 atSendFtmDataMsg(VOS_UINT32 TaskId, VOS_UINT32 MsgId, VOS_UINT32 ulClientId, VOS_VOID* pData, VOS_UINT32 uLen);
#endif

/*****************************************************************************
 函 数 名  : AT_PS_AddIpAddrRabIdMap
 功能描述  : 添加承载IP与RABID的映射
 输入参数  : usClientId - 客户端ID
             pstEvent   - PS域呼叫事件
 输出参数  : 无
 返 回 值  : VOS_VOID
*****************************************************************************/
VOS_VOID AT_PS_AddIpAddrRabIdMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
);


/*****************************************************************************
 函 数 名  : AT_PS_DeleteIpAddrRabIdMap
 功能描述  : 删除承载IP与RABID的映射
 输入参数  : usClientId - 客户端ID
             pstEvent   - PS域呼叫事件
 输出参数  : 无
 返 回 值  : VOS_VOID
*****************************************************************************/
VOS_VOID AT_PS_DeleteIpAddrRabIdMap(
    VOS_UINT16                          usClientId,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
);


/*****************************************************************************
 函 数 名  : AT_PS_GetIpAddrByRabId
 功能描述  : 根据RABID获取承载IP地址
 输入参数  : usClientId - 客户端ID
             ucRabId    - RABID [5,15]
 输出参数  : 无
 返 回 值  : ulIpAddr   - IP地址(主机序)
*****************************************************************************/
VOS_UINT32 AT_PS_GetIpAddrByRabId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucRabId
);

VOS_VOID AT_PS_SetPsCallErrCause(
    VOS_UINT16                          usClientId,
    TAF_PS_CAUSE_ENUM_UINT32            enPsErrCause
);

TAF_PS_CAUSE_ENUM_UINT32 AT_PS_GetPsCallErrCause(
    VOS_UINT16                          usClientId
);

VOS_UINT32 AT_PS_GetChDataValueFromRnicRmNetId(
    RNIC_DEV_ID_ENUM_UINT8              enRnicRmNetId,
    AT_CH_DATA_CHANNEL_ENUM_UINT32     *penDataChannelId
);

CONST AT_CHDATA_RNIC_RMNET_ID_STRU *AT_PS_GetChDataCfgByChannelId(
    VOS_UINT8                           ucIndex,
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId
);

AT_PS_CALL_ENTITY_STRU* AT_PS_GetCallEntity(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
);

VOS_UINT8 AT_PS_TransCidToCallId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCid
);

VOS_UINT32 AT_PS_IsCallIdValid(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
);

VOS_VOID AT_PS_ReportDCONN(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);

VOS_VOID AT_PS_ReportNdisStatConn(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);

VOS_VOID AT_PS_ReportDEND(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
);

VOS_VOID AT_PS_ReportNdisStatEnd(
    AT_PS_USER_INFO_STRU               *pstUsrInfo,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
);

AT_PS_CALL_TYPE_ENUM_U8 AT_PS_GetPsCallType(VOS_UINT8 ucIndex);

VOS_VOID  AT_PS_SndRnicPdpActInd(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);

VOS_VOID AT_PS_SndRnicPdpDeactInd(
    AT_PS_CALL_ENTITY_STRU              *pstCallEntity,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);

VOS_VOID AT_PS_ProcNdisPdpActInd(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);

VOS_VOID AT_PS_ProcNdisPdpDeActInd(
    AT_PS_CALL_ENTITY_STRU              *pstCallEntity,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8              enPdpType
);

#if (FEATURE_ON == FEATURE_IPV6)
VOS_UINT32 AT_PS_IsIpv6CapabilityValid(VOS_UINT8 ucCapability);

VOS_UINT32 AT_CheckIpv6Capability(
    VOS_UINT8                           ucPdpType
);

VOS_UINT32 AT_PS_GenIpv6LanAddrWithRadomIID(
    VOS_UINT8                          *pucPrefix,
    VOS_UINT32                          ulPrefixByteLen,
    VOS_UINT8                          *pucIpv6LanAddr
);

VOS_UINT8 AT_CalcIpv6PrefixLength(
    VOS_UINT8                           *pucLocalIpv6Mask
);

VOS_VOID AT_GetIpv6MaskByPrefixLength(
    VOS_UINT8                            ucLocalIpv6Prefix,
    VOS_UINT8                           *pucLocalIpv6Mask
);
#endif

VOS_VOID AT_PS_ProcCallConnectedEvent(
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
);

VOS_VOID AT_PS_ProcCallRejectEvent(
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU   *pstEvent
);

VOS_VOID AT_PS_ProcCallEndedEvent(
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
);

VOS_VOID AT_PS_ProcCallOrigCnfEvent(TAF_PS_CALL_ORIG_CNF_STRU *pstCallOrigCnf);

VOS_VOID AT_PS_ProcCallEndCnfEvent(TAF_PS_CALL_END_CNF_STRU *pstCallEndCnf);

VOS_UINT32 AT_PS_ProcCallModifyEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_MODIFY_CNF_STRU    *pstEvent
);

VOS_VOID AT_PS_FreeCallEntity(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
);

VOS_UINT32 AT_PS_ValidateDialParam(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
);

VOS_VOID AT_PS_ParseUsrInfo(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType,
    AT_PS_USER_INFO_STRU               *pstUserInfo
);

TAF_PDP_TYPE_ENUM_UINT8 AT_PS_GetCurrCallType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
);

VOS_UINT32 AT_PS_SetupCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    AT_DIAL_PARAM_STRU                 *pstCallDialParam
);

VOS_UINT32 AT_PS_HangupCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause
);

VOS_UINT32 AT_PS_ProcDialCmd(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_TYPE_ENUM_U8             enPsCallType
);

VOS_UINT32 AT_CheckApnFormat(
    VOS_UINT8                          *pucApn,
    VOS_UINT16                          usApnLen
);

/*****************************************************************************
 函 数 名  : AT_PS_SndCallConnectedResult
 功能描述  : 连接建立状态上报
 输入参数  : ucCallId  - 呼叫实体索引
             enPdpType - PDP类型
 输出参数  : 无
 返 回 值  : VOS_VOID
*****************************************************************************/
VOS_VOID AT_PS_SndCallConnectedResult(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);

VOS_VOID AT_PS_ReportCustomPcoInfo(
    TAF_PS_CUSTOM_PCO_INFO_STRU        *pstPcoCustInfo,
    TAF_PS_PDN_OPERATE_TYPE_ENUM_UINT8  enOperateType,
    VOS_UINT8                           ucCid,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_CLIENT_TAB_INDEX_UINT8           enPortIndex
);


VOS_UINT32 AT_PS_IsLinkDown(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);


VOS_UINT32 AT_PS_IsNeedClearCurrCall(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enDataSys
);

/*****************************************************************************
 函 数 名  : AT_PS_BuildExClientId
 功能描述  : 生成扩展的ClientId(ModemId + ClientId)
 输入参数  : usClientId                 - ClientId
 输出参数  : 无
 返 回 值  : VOS_UINT16                 - 生成的扩展ClientId
*****************************************************************************/
VOS_UINT16 AT_PS_BuildExClientId(VOS_UINT16 usClientId);

/*****************************************************************************
 函 数 名  : AT_PS_BuildPsCallExClientId
 功能描述  : 生成扩展的ClientId(ModemId + ClientId)
 输入参数  : usClientId                 - ClientId
 输出参数  : 无
 返 回 值  : VOS_UINT16                 - 生成的扩展ClientId
*****************************************************************************/
VOS_UINT16 AT_PS_BuildPsCallExClientId(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId
);

/*****************************************************************************
 函 数 名  : AT_PS_BuildNdisExClientId
 功能描述  : 生成扩展的ClientId(ModemId + ClientId)
 输入参数  : usPortIndex                - 拨号端口
             usClientId                 - ClientId
 输出参数  : 无
 返 回 值  : VOS_UINT16                 - 生成的扩展ClientId
*****************************************************************************/
VOS_UINT16 AT_PS_BuildNdisExClientId(
    VOS_UINT16                          usPortIndex,
    VOS_UINT16                          usClientId
);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_UINT32 AT_PS_CheckDialRatType(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucBitRatType
);
#endif

VOS_VOID AT_PS_ProcRabidChangedEvent(
    TAF_PS_CALL_PDP_RABID_CHANGE_IND_STRU  *pstEvent
);


VOS_UINT32 AT_Ipv6AddrMask2FormatString(
    VOS_CHAR                           *pcIpv6FormatStr,
    VOS_UINT8                           aucIpv6Addr[],
    VOS_UINT8                           aucIpv6Mask[]
);

VOS_VOID AT_PS_SetWlanCurrCallType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);
VOS_VOID AT_PS_SetHoCallType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);
VOS_VOID AT_PS_SetCallHandOverFlg(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    VOS_UINT8                           ucIsHandOver
);
VOS_VOID AT_PS_SetCallHandOverFailCause(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PS_CAUSE_ENUM_UINT32            enHoFailCause
);
TAF_PS_CAUSE_ENUM_UINT32 AT_PS_GetCallHandOverFailCause(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);
AT_PS_DATA_SYS_ENUM_UINT32 AT_PS_GetPreDataSystem(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    VOS_UINT8                           ucIndex
);
VOS_VOID AT_PS_SetPsCallCurrentDataSys(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PS_APN_DATA_SYS_ENUM_UINT8      enCurrentDataSys
);
TAF_PS_APN_DATA_SYS_ENUM_UINT8 AT_PS_GetPsCallCurrentDataSys(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);
VOS_UINT8 AT_PS_GetPsCallHandOverFlg(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);
VOS_VOID AT_PS_RegDataSysChgNtf(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);
VOS_VOID AT_PS_DeRegDataSysChgNtf(
    VOS_UINT16                              usClientId,
    VOS_UINT8                               ucCallId
);
VOS_UINT32 AT_PS_DialUpInWlanAllowed(
    VOS_UINT8                               ucBitDataSystem,
    TAF_PS_APN_DATA_SYS_POLICY_ENUM_UINT8   enDataSysPolicy
);
VOS_UINT32 AT_PS_DialUpInCellularAllowed(
    VOS_UINT8                               ucBitDataSystem,
    TAF_PS_APN_DATA_SYS_POLICY_ENUM_UINT8   enDataSysPolicy
);
VOS_UINT32 AT_PS_IsWlanThrotAllowed(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);
VOS_VOID AT_PS_SetWlanCallStateByType(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    AT_PDP_STATE_ENUM_U8                enPdpState
);
VOS_UINT32 AT_PS_ProcMapconMsg (
    VOS_UINT16                          usClientId,
    AT_MAPCON_CTRL_MSG_STRU            *pstAtMapConMsgPara
);
VOS_UINT32 AT_PS_HangupWlanCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    VOS_INT8                            cIsLocal,
    VOS_INT8                            cIsHandover
);
VOS_VOID AT_PS_PreProcPsCallDataSysChgNtf(
    VOS_UINT16                   usClientId,
    VOS_UINT32                   ulBitDataSysSwitchCid
);
VOS_UINT32 AT_PS_SndWlanMsgPdnActivateReq(
    VOS_UINT8                           ucIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enIpType,
    VOS_INT8                            cIsHandover,
    VOS_UINT8                           ucCallId
);
VOS_UINT32 AT_PS_SndWlanMsgPdnDeActivateReq(
    VOS_UINT8                           ucIndex,
    VOS_INT8                            cIsLocal,
    VOS_INT8                            cIsHandover,
    VOS_UINT8                           ucCallId
);
VOS_VOID AT_PS_ReportImsCtrlMsgu(
    VOS_UINT8                                       ucIndex,
    AT_IMS_CTRL_MSG_RECEIVE_MODULE_ENUM_UINT8       enModule,
    VOS_UINT32                                      ulMsgLen,
    VOS_UINT8                                      *pucDst
);
VOS_VOID AT_PS_SelectApnDataSysConfig(
    VOS_UINT8                           ucIndex,
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity
);
VOS_VOID AT_PS_ProcWlanPowerOff (
    VOS_UINT16                          usClientId
);
TAF_PS_APN_DATA_SYS_POLICY_INFO_STRU* AT_PS_GetApnDataSysPolicyInfo(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucIndex
);
TAF_PDP_TYPE_ENUM_UINT8 AT_PS_ConvertPdpType2Cellular(IMSA_WIFI_IP_TYPE_ENUM_INT8 enImsaPdpType);
VOS_VOID AT_PS_RcvTiWlanActPdnCnfExpired(REL_TIMER_MSG *pstTmrMsg);
VOS_VOID AT_PS_RcvTiWlanDeActPdnCnfExpired(REL_TIMER_MSG *pstTmrMsg);
VOS_VOID AT_PS_RcvTiProtectPdnInDataSysExpired(REL_TIMER_MSG *pstTmrMsg);

AT_PDP_STATE_ENUM_U8 AT_PS_GetCallStateByType(
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);

FC_ID_ENUM_UINT8 AT_PS_GetFcIdByIFaceId(
    VOS_UINT32                          ulIFaceId
);

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __AT_DATA_PROC_H__ */

