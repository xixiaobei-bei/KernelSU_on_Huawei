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
#ifndef _TAF_IFACE_API_H_
#define _TAF_IFACE_API_H_


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "TafPsApi.h"
#include "ImsaWifiInterface.h"
#include "PsIfaceGlobalDef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

#pragma pack(4)


/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define TAF_IFACE_MAX_NUM                   (PS_IFACE_ID_BUTT)

/* 1~11 BIT CID信息 */
#define TAF_IFACE_ALL_BIT_NV_CID            (0x0FFE)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


enum TAF_IFACE_MSG_ID_ENUM
{
    ID_MSG_TAF_IFACE_UP_REQ                                 = TAF_PS_IFACE_ID_BASE + 0x0001,
    ID_MSG_TAF_IFACE_DOWN_REQ                               = TAF_PS_IFACE_ID_BASE + 0x0002,
    ID_MSG_TAF_GET_IFACE_DYNAMIC_PARA_REQ                   = TAF_PS_IFACE_ID_BASE + 0x0003,

    ID_MSG_TAF_IFACE_BUTT
};
typedef VOS_UINT32 TAF_IFACE_MSG_ID_ENUM_UINT32;


enum TAF_IFACE_EVT_ID_ENUM
{
    ID_EVT_TAF_IFACE_UP_CNF                                 = TAF_PS_IFACE_ID_BASE + 0x0081,
    ID_EVT_TAF_IFACE_DOWN_CNF                               = TAF_PS_IFACE_ID_BASE + 0x0082,
    ID_EVT_TAF_IFACE_STATUS_IND                             = TAF_PS_IFACE_ID_BASE + 0x0083,
    ID_EVT_TAF_IFACE_DATA_CHANNEL_STATE_IND                 = TAF_PS_IFACE_ID_BASE + 0x0084,
    ID_EVT_TAF_IFACE_USBNET_OPER_IND                        = TAF_PS_IFACE_ID_BASE + 0x0085,
    ID_EVT_TAF_IFACE_REG_FC_IND                             = TAF_PS_IFACE_ID_BASE + 0x0086,
    ID_EVT_TAF_IFACE_DEREG_FC_IND                           = TAF_PS_IFACE_ID_BASE + 0x0087,
    ID_EVT_TAF_IFACE_GET_DYNAMIC_PARA_CNF                   = TAF_PS_IFACE_ID_BASE + 0x0088,
    ID_EVT_TAF_IFACE_RAB_INFO_IND                           = TAF_PS_IFACE_ID_BASE + 0x0089,

    ID_EVT_TAF_IFACE_BUTT
};
typedef VOS_UINT32 TAF_IFACE_EVT_ID_ENUM_UINT32;


enum TAF_IFACE_STATUS_ENUM
{
    TAF_IFACE_STATUS_ACT                = 0x01,
    TAF_IFACE_STATUS_DEACT              = 0x02,

    TAF_IFACE_STATUS_BUTT
};
typedef VOS_UINT8 TAF_IFACE_STATUS_ENUM_UINT8;


enum TAF_IFACE_USER_TYPE_ENUM
{
    TAF_IFACE_USER_TYPE_APP             = 0,
    TAF_IFACE_USER_TYPE_NDIS            = 1,
    TAF_IFACE_USER_TYPE_BUTT
};
typedef VOS_UINT8 TAF_IFACE_USER_TYPE_ENUM_U8;


enum TAF_IFACE_STATE_ENUM
{
    TAF_IFACE_STATE_IDLE                = 0,
    TAF_IFACE_STATE_ACTED               = 1,
    TAF_IFACE_STATE_ACTING              = 2,
    TAF_IFACE_STATE_DEACTING            = 3,
    TAF_IFACE_STATE_BUTT
};
typedef VOS_UINT8 TAF_IFACE_STATE_ENUM_U8;


enum TAF_IFACE_RAB_OPER_TYPE_ENUM
{
    TAF_IFACE_RAB_OPER_ADD              = 0,
    TAF_IFACE_RAB_OPER_DELETE           = 1,
    TAF_IFACE_RAB_OPER_CHANGE           = 2,
    TAF_IFACE_RAB_OPER_BUTT
};
typedef VOS_UINT8 TAF_IFACE_RAB_OPER_TYPE_ENUM_U8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


typedef struct
{
    VOS_UINT32                          bitOpIPv4ValidFlag          : 1;
    VOS_UINT32                          bitOpSpare                  : 31;

    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType;                             /* 用户呼叫类型 */
    VOS_UINT8                           ucCid;
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;
    VOS_UINT8                           ucApnLen;
    VOS_UINT8                           aucApn[TAF_MAX_APN_LEN + 1];
    VOS_UINT8                           ucUsernameLen;
    VOS_UINT8                           aucUsername[TAF_MAX_AUTHDATA_USERNAME_LEN + 1];
    VOS_UINT8                           ucPasswordLen;
    VOS_UINT8                           aucPassword[TAF_MAX_AUTHDATA_PASSWORD_LEN + 1];
    VOS_UINT8                           ucAuthType;
    VOS_UINT8                           ucBitRatType;
    VOS_UINT8                           aucIPv4Addr[TAF_MAX_IPV4_ADDR_STR_LEN];
}TAF_IFACE_DIAL_PARAM_STRU;


typedef struct
{
    TAF_IFACE_DIAL_PARAM_STRU           stUsrDialParam;
    VOS_UINT8                           ucIfaceId;
    VOS_UINT8                           aucReserved[3];
}TAF_IFACE_UP_STRU;


typedef struct
{
    VOS_UINT8                           ucIfaceId;
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause;
    TAF_IFACE_USER_TYPE_ENUM_U8         enUserType;                             /* 用户呼叫类型 */
    VOS_UINT8                           aucReserved[1];
}TAF_IFACE_DOWN_STRU;


typedef struct
{
    VOS_UINT8                           ucPcscfAddrNum;                         /* IPV4的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8                           aucRsv[3];                              /* 保留 */
    VOS_UINT32                          aulPcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_IFACE_IPV4_PCSCF_LIST_STRU;


typedef struct
{
    VOS_UINT32                              bitOpPriDns   : 1;
    VOS_UINT32                              bitOpSecDns   : 1;
    VOS_UINT32                              bitOpSpare    : 30;

    VOS_UINT32                              ulAddr;                             /* IPV4的IP地址，主机序 */
    VOS_UINT32                              ulNetMask;                          /* IPV4的掩码，主机序 */
    VOS_UINT32                              ulGateWay;                          /* IPV4的网关地址，主机序 */
    VOS_UINT32                              ulPrimDNS;                          /* IPV4的主DNS，主机序 */
    VOS_UINT32                              ulSecDNS;                           /* IPV4的副DNS，主机序 */
    TAF_IFACE_IPV4_PCSCF_LIST_STRU          stPcscfList;                        /* 8组Pcscf */
}TAF_IFACE_IPV4_DHCP_PARAM_STRU;


typedef struct
{
    TAF_IFACE_STATE_ENUM_U8                 enState;                            /* IPv4 状态 */
    VOS_UINT8                               ucRabId;                            /* RAB标识，取值范围:[5,15] */
    VOS_UINT8                               ucPduSessionId;                     /* ucPduSessionId标识 */
    VOS_UINT8                               aucRsv[3];
    VOS_UINT16                              usMtu;                              /* IPv4 MTU */
    TAF_IFACE_IPV4_DHCP_PARAM_STRU          stDhcpInfo;                         /* IPv4 DHCP信息 */
} TAF_IFACE_IPV4_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucPcscfAddr[TAF_IPV6_ADDR_LEN];
} TAF_IFACE_IPV6_PCSCF_STRU;


typedef struct
{
    VOS_UINT8                           ucPcscfAddrNum;                         /* IPV6的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8                           aucRsv[7];                              /* 保留 */
    TAF_IFACE_IPV6_PCSCF_STRU           astPcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_IFACE_IPV6_PCSCF_LIST_STRU;


typedef struct
{
    VOS_UINT32                              bitOpPriDns   : 1;
    VOS_UINT32                              bitOpSecDns   : 1;
    VOS_UINT32                              bitOpSpare    : 30;

    VOS_UINT8                               aucTmpAddr[TAF_IPV6_ADDR_LEN];      /* 从 PDP上下文带来的IPV6地址长度，不包括":" */
    VOS_UINT8                               aucAddr[TAF_IPV6_ADDR_LEN];         /* 从 PDP上下文带来的IPV6地址长度，不包括":" */
    VOS_UINT8                               aucPrimDns[TAF_IPV6_ADDR_LEN];      /* 从 PDP上下文带来的IPV6主DNS长度，不包括":" */
    VOS_UINT8                               aucSecDns[TAF_IPV6_ADDR_LEN];       /* 从 PDP上下文带来的IPV6副DNS长度，不包括":" */
    TAF_IFACE_IPV6_PCSCF_LIST_STRU          stPcscfList;
}TAF_IFACE_IPV6_DHCP_PARAM_STRU;


typedef struct
{
    VOS_UINT32                          bitOpLanAddr            : 1;
    VOS_UINT32                          bitOpPrefixAddr         : 1;
    VOS_UINT32                          bitOpMtuSize            : 1;
    VOS_UINT32                          bitOpPreferredLifetime  : 1;
    VOS_UINT32                          bitOpValidLifetime      : 1;
    VOS_UINT32                          bitOpSpare              : 27;

    VOS_UINT32                          ulBitCurHopLimit        :8;
    VOS_UINT32                          ulBitM                  :1;
    VOS_UINT32                          ulBitO                  :1;
    VOS_UINT32                          ulBitL                  :1;             /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          ulBitA                  :1;
    VOS_UINT32                          ulBitRsv                :20;

    VOS_UINT8                           aucLanAddr[TAF_IPV6_ADDR_LEN];          /* IPv6 路由器LAN端口地址 */
    VOS_UINT8                           aucPrefixAddr[TAF_IPV6_ADDR_LEN];       /* IPv6前缀 */
    VOS_UINT32                          ulPrefixBitLen;                         /* IPv6前缀长度 */
    VOS_UINT32                          ulMtuSize;                              /* RA消息中广播的IPv6的MTU的取值 */
    VOS_UINT32                          ulPreferredLifetime;                    /* IPv6前缀的Preferred lifetime */
    VOS_UINT32                          ulValidLifetime;                        /* IPv6前缀的Valid lifetime */
} TAF_IFACE_IPV6_RA_INFO_STRU;


typedef struct
{
    TAF_IFACE_STATE_ENUM_U8                 enState;                            /* IPv6 状态 */
    VOS_UINT8                               ucRabId;                            /* RAB标识，取值范围:[5,15] */
    VOS_UINT8                               ucPduSessionId;
    VOS_UINT8                               aucRsv[5];                          /* 保留位 */
    TAF_IFACE_IPV6_RA_INFO_STRU             stRaInfo;                           /* IPv6 路由公告信息 */
    TAF_IFACE_IPV6_DHCP_PARAM_STRU          stDhcpInfo;                         /* IPv6 DHCP信息 */
} TAF_IFACE_IPV6_INFO_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_IFACE_UP_STRU                   stIfaceUp;
} TAF_IFACE_UP_REQ_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_IFACE_DOWN_STRU                 stIfaceDown;
} TAF_IFACE_DOWN_REQ_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_PS_CAUSE_ENUM_UINT32            enCause;
} TAF_IFACE_UP_CNF_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_PS_CAUSE_ENUM_UINT32            enCause;
} TAF_IFACE_DOWN_CNF_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    TAF_IFACE_STATUS_ENUM_UINT8             enStatus;                           /* 上报状态类型 */
    VOS_UINT8                               ucCid;                              /* Cid信息 */
    TAF_PDP_TYPE_ENUM_UINT8                 enPdpType;                          /* PDP TYPE类型 */
    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* 拨号用户类型 */
    TAF_PS_CAUSE_ENUM_UINT32                enCause;                            /* 去激活原因值 */
} TAF_IFACE_STATUS_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;

    VOS_UINT32                              bitOpActFlg         : 1;
    VOS_UINT32                              bitOpCleanFlg       : 1;
    VOS_UINT32                              bitOpSpare          : 30;

    VOS_UINT8                               ucCid;                              /* Cid信息 */
    VOS_UINT8                               aucRsv[3];                          /* 保留位 */
} TAF_IFACE_DATA_CHANNEL_STATE_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    VOS_UINT32                              bitOpActUsbNet      : 1;            /* 通知激活USB NET标志位 */
    VOS_UINT32                              bitOpDeactUsbNet    : 1;            /* 通知去激活USB NET标志位 */
    VOS_UINT32                              bitOpSpare          : 30;
} TAF_IFACE_USBNET_OPER_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;

    VOS_UINT32                              bitOpUmtsQos        : 1;
    VOS_UINT32                              bitOpSpare          : 31;

    TAF_UMTS_QOS_STRU                       stUmtsQos;
    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* 用户呼叫类型 */
    VOS_UINT8                               ucRabId;                            /* RABID */
    VOS_UINT8                               ucIfaceId;                          /* 全局网卡ID */
    VOS_UINT8                               aucReserved[1];
} TAF_IFACE_REG_FC_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* 用户呼叫类型 */
    VOS_UINT8                               ucRabId;                            /* RABID */
    VOS_UINT8                               ucIfaceId;                          /* 全局网卡ID */
    VOS_UINT8                               aucReserved[1];
} TAF_IFACE_DEREG_FC_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    VOS_UINT32                              ulBitCid;                           /* Bit Cid信息 */
} TAF_IFACE_GET_DYNAMIC_PARA_REQ_STRU;


typedef struct
{
    VOS_UINT32                              bitOpIpv4Valid      : 1;
    VOS_UINT32                              bitOpIpv6Valid      : 1;
    VOS_UINT32                              bitOpSpare          : 30;

    TAF_IFACE_USER_TYPE_ENUM_U8             enUserType;                         /* 用户呼叫类型 */
    VOS_UINT8                               ucApnLen;                           /* APN长度 */
    VOS_UINT8                               aucApn[TAF_MAX_APN_LEN + 1];        /* APN信息 */
    VOS_UINT8                               aucRsv[6];                          /* 保留位 */
    TAF_IFACE_IPV4_INFO_STRU                stIpv4Info;                         /* 呼叫IPV4相关信息记录 */
    TAF_IFACE_IPV6_INFO_STRU                stIpv6Info;                         /* 呼叫IPV6相关信息记录 */
} TAF_IFACE_DYNAMIC_INFO_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    VOS_UINT32                              ulBitCid;
    TAF_IFACE_DYNAMIC_INFO_STRU             astDynamicInfo[TAF_MAX_CID_NV + 1];     /* 动态详细信息 */
} TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;

    VOS_UINT32                              bitOpPdpAddr        : 1;
    VOS_UINT32                              bitOpSpare          : 31;

    VOS_UINT8                               ucOldRabId;
    VOS_UINT8                               ucNewRabId;
    TAF_IFACE_RAB_OPER_TYPE_ENUM_U8         enOperType;
    PS_IFACE_ID_ENUM_UINT8                  enIfaceId;
    TAF_PDP_TYPE_ENUM_UINT8                 enPdpType;
    VOS_UINT8                               aucReserved[3];
    VOS_UINT32                              ulIpAddr;
} TAF_IFACE_RAB_INFO_IND_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
/*****************************************************************************
 函 数 名  : TAF_IFACE_Up
 功能描述  : 调用DSM IFACE接口发起激活IFACE网卡
 输入参数  : pstCtrl                    - DSM IFACE控制结构
             pstIfaceUp                 - 激活IFACE网卡结构体
 输出参数  : 无
 返 回 值  : VOS_OK                     - 成功
             VOS_ERR                    - 失败
*****************************************************************************/
VOS_UINT32 TAF_IFACE_Up(
    const TAF_CTRL_STRU                *pstCtrl,
    const TAF_IFACE_UP_STRU            *pstIfaceUp
);

/*****************************************************************************
 函 数 名  : TAF_IFACE_Down
 功能描述  : 调用DSM IFACE接口断开IFACE网卡
 输入参数  : pstCtrl                    - DSM IFACE控制结构
             pstIfaceDown               - 断开IFACE网卡结构体
 输出参数  : 无
 返 回 值  : VOS_OK                     - 成功
             VOS_ERR                    - 失败
*****************************************************************************/
VOS_UINT32 TAF_IFACE_Down(
    const TAF_CTRL_STRU                *pstCtrl,
    const TAF_IFACE_DOWN_STRU          *pstIfaceDown
);

/*****************************************************************************
 函 数 名  : TAF_IFACE_GetDynamicPara
 功能描述  : 调用DSM IFACE接口发起查询对应Bit Cid的动态信息
 输入参数  : const TAF_CTRL_STRU                *pstCtrl,
             const VOS_UINT32                    ulBitCid
 输出参数  : 无
 返 回 值  : VOS_OK                     - 成功
             VOS_ERR                    - 失败
*****************************************************************************/
VOS_UINT32 TAF_IFACE_GetDynamicPara(
    const TAF_CTRL_STRU                *pstCtrl,
    const VOS_UINT32                    ulBitCid
);


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

#endif

