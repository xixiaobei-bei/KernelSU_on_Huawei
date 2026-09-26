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

#include "vos.h"
#include "taf_ps_api.h"
#include "imsa_wifi_interface.h"
#include "ps_iface_global_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef WIN32
#pragma warning(disable : 4200) /* zero-sized array in struct/union */
#endif

#pragma pack(push, 4)

#define TAF_IFACE_MAX_NUM (PS_IFACE_ID_BUTT)

/* 1~11 BIT CID信息 */
#define TAF_IFACE_ALL_BIT_NV_CID (0x0FFE)


enum TAF_IFACE_MsgId {
    /* TAF->DSM, A_TAF->DSM */
    /* _MSGPARSE_Interception TAF_ELF_IFACE_UP_REQ_STRU */
    ID_MSG_TAF_IFACE_UP_REQ               = TAF_PS_IFACE_ID_BASE + 0x0001,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_DOWN_REQ_STRU */
    ID_MSG_TAF_IFACE_DOWN_REQ             = TAF_PS_IFACE_ID_BASE + 0x0002,
    /* _MSGPARSE_Interception TAF_ELF_GET_IFACE_DYNAMIC_PARA_REQ_STRU */
    ID_MSG_TAF_GET_IFACE_DYNAMIC_PARA_REQ = TAF_PS_IFACE_ID_BASE + 0x0003,

    ID_MSG_TAF_IFACE_BUTT
};
typedef VOS_UINT32 TAF_IFACE_MsgIdUint32;


enum TAF_IFACE_EvtId {
    /* DSM->DSM */
    /* _MSGPARSE_Interception TAF_ELF_IFACE_UP_CNF_STRU */
    ID_EVT_TAF_IFACE_UP_CNF                 = TAF_PS_IFACE_ID_BASE + 0x0081,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_DOWN_CNF_STRU */
    ID_EVT_TAF_IFACE_DOWN_CNF               = TAF_PS_IFACE_ID_BASE + 0x0082,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_STATUS_IND_STRU */
    ID_EVT_TAF_IFACE_STATUS_IND             = TAF_PS_IFACE_ID_BASE + 0x0083,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_DATA_CHANNEL_STATE_IND_STRU */
    ID_EVT_TAF_IFACE_DATA_CHANNEL_STATE_IND = TAF_PS_IFACE_ID_BASE + 0x0084,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_USBNET_OPER_IND_STRU */
    ID_EVT_TAF_IFACE_USBNET_OPER_IND        = TAF_PS_IFACE_ID_BASE + 0x0085,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_GET_DYNAMIC_PARA_CNF_STRU */
    ID_EVT_TAF_IFACE_GET_DYNAMIC_PARA_CNF   = TAF_PS_IFACE_ID_BASE + 0x0088,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_RAB_INFO_IND_STRU */
    ID_EVT_TAF_IFACE_RAB_INFO_IND           = TAF_PS_IFACE_ID_BASE + 0x0089,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_IpChangeInd */
    ID_EVT_TAF_IFACE_IP_CHANGE_IND          = TAF_PS_IFACE_ID_BASE + 0x008A,
    /* _MSGPARSE_Interception TAF_ELF_IFACE_Ipv6PdnInfoInd */
    ID_EVT_TAF_IFACE_IPV6_PDN_INFO_IND      = TAF_PS_IFACE_ID_BASE + 0x008B,

    ID_EVT_TAF_IFACE_BUTT
};
typedef VOS_UINT32 TAF_IFACE_EvtIdUint32;


enum TAF_IFACE_Status {
    TAF_IFACE_STATUS_ACT   = 0x01,
    TAF_IFACE_STATUS_DEACT = 0x02,

    TAF_IFACE_STATUS_BUTT
};
typedef VOS_UINT8 TAF_IFACE_StatusUint8;


enum TAF_IFACE_UserType {
    TAF_IFACE_USER_TYPE_APP  = 0,
    TAF_IFACE_USER_TYPE_NDIS = 1,
    TAF_IFACE_USER_TYPE_PPP  = 2,
    TAF_IFACE_USER_TYPE_BUTT
};
typedef VOS_UINT8 TAF_IFACE_UserTypeUint8;


enum TAF_IFACE_State {
    TAF_IFACE_STATE_IDLE     = 0,
    TAF_IFACE_STATE_ACTED    = 1,
    TAF_IFACE_STATE_ACTING   = 2,
    TAF_IFACE_STATE_DEACTING = 3,
    TAF_IFACE_STATE_BUTT
};
typedef VOS_UINT8 TAF_IFACE_StateUint8;


enum TAF_IFACE_RabOperType {
    TAF_IFACE_RAB_OPER_ADD    = 0,
    TAF_IFACE_RAB_OPER_DELETE = 1,
    TAF_IFACE_RAB_OPER_CHANGE = 2,
    TAF_IFACE_RAB_OPER_BUTT
};
typedef VOS_UINT8 TAF_IFACE_RabOperTypeUint8;


enum TAF_IFACE_IpChangeOperType {
    TAF_IFACE_IP_CHANGE_OPER_ADD    = 0,
    TAF_IFACE_IP_CHANGE_OPER_DELETE = 1,
    TAF_IFACE_IP_CHANGE_OPER_CHANGE = 2,
    TAF_IFACE_IP_CHANGE_OPER_BUTT
};
typedef VOS_UINT8 TAF_IFACE_IpChangeOperTypeUint8;


typedef struct {
    VOS_UINT32 opIPv4ValidFlag : 1;
    VOS_UINT32 bitOpSNssai : 1;
    VOS_UINT32 opSpare : 30;

    TAF_IFACE_UserTypeUint8 userType; /* 用户呼叫类型 */
    VOS_UINT8               cid;
    TAF_PDP_TypeUint8       pdpType;
    VOS_UINT8               apnLen;
    VOS_UINT8               apn[TAF_MAX_APN_LEN + 1];
    VOS_UINT8               usernameLen;
    VOS_UINT8               username[TAF_MAX_AUTHDATA_USERNAME_LEN + 1];
    VOS_UINT8               passwordLen;
    VOS_UINT8               password[TAF_MAX_AUTHDATA_PASSWORD_LEN + 1];
    VOS_UINT8               authType;
    VOS_UINT8               ratType;
    VOS_UINT8               ipv4Addr[TAF_MAX_IPV4_ADDR_STR_LEN];
    TAF_PS_SscModeUint8            sscMode; /* SSC Mode参数 */
    TAF_PS_AlwaysOnIndUint8        alwaysonType; /* 是否支持Always on类型； 0：不支持 1：支持 */
    TAF_PS_Ipv6MultiHomingIndUint8 multiHomeIPv6; /* 是否支持MultiHomeIPV6；0：不支持 1：支持 */
    TAF_PS_ReflectQosIndUint8      rQosFlag; /* 是否支持反射QOS；0：不支持 1：支持 */
    TAF_PS_PrefAccessTypeUint8     accessDomain;
    VOS_UINT8                      reserved[3];
    PS_S_NSSAI_STRU                sNssai;
    TAF_PPP_ReqConfigInfo          pppReqConfigInfo;
} TAF_IFACE_DialParam;


typedef struct {
    TAF_IFACE_DialParam usrDialParam;
    VOS_UINT8           ifaceId;
    VOS_UINT8           reserved[3];
} TAF_IFACE_Active;


typedef struct {
    VOS_UINT8                ifaceId;
    TAF_PS_CallEndCauseUint8 cause;
    TAF_IFACE_UserTypeUint8  userType; /* 用户呼叫类型 */
    VOS_UINT8                reserved[1];
} TAF_IFACE_Deactive;


typedef struct {
    VOS_UINT8  pcscfAddrNum; /* IPV4的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8  rsv[3];       /* 保留 */
    VOS_UINT32 pcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_IFACE_Ipv4PcscfList;


typedef struct {
    VOS_UINT32 opPriDns : 1;
    VOS_UINT32 opSecDns : 1;
    VOS_UINT32 opSpare : 30;

    VOS_UINT32              addr;      /* IPV4的IP地址，主机序 */
    VOS_UINT32              netMask;   /* IPV4的掩码，主机序 */
    VOS_UINT32              gateWay;   /* IPV4的网关地址，主机序 */
    VOS_UINT32              primDNS;   /* IPV4的主DNS，主机序 */
    VOS_UINT32              secDNS;    /* IPV4的副DNS，主机序 */
    TAF_IFACE_Ipv4PcscfList pcscfList; /* 8组Pcscf */
} TAF_IFACE_Ipv4DhcpParam;


typedef struct {
    TAF_IFACE_StateUint8    state;        /* IPv4 状态 */
    VOS_UINT8               rabId;        /* RAB标识，取值范围:[5,15] */
    VOS_UINT8               pduSessionId; /* ucPduSessionId标识 */
    VOS_UINT8               rsv[3];
    VOS_UINT16              mtu;      /* IPv4 MTU */
    TAF_IFACE_Ipv4DhcpParam dhcpInfo; /* IPv4 DHCP信息 */
} TAF_IFACE_Ipv4Info;


typedef struct {
    VOS_UINT8 pcscfAddr[TAF_IPV6_ADDR_LEN];
} TAF_IFACE_Ipv6Pcscf;


typedef struct {
    VOS_UINT8           pcscfAddrNum; /* IPV6的P-CSCF地址个数，有效范围[0,8] */
    VOS_UINT8           rsv[7];       /* 保留 */
    TAF_IFACE_Ipv6Pcscf pcscfAddrList[TAF_PCSCF_ADDR_MAX_NUM];
} TAF_IFACE_Ipv6PcscfList;


typedef struct {
    VOS_UINT32 opPriDns : 1;
    VOS_UINT32 opSecDns : 1;
    VOS_UINT32 opSpare : 30;

    VOS_UINT8               tmpAddr[TAF_IPV6_ADDR_LEN]; /* 从 PDP上下文带来的IPV6地址长度，不包括":" */
    VOS_UINT8               addr[TAF_IPV6_ADDR_LEN];    /* 从 PDP上下文带来的IPV6地址长度，不包括":" */
    VOS_UINT8               paddr[TAF_IPV6_ADDR_LEN];   /* 从 PDP上下文带来的IPV6地址长度，不包括":" */
    VOS_UINT8               primDns[TAF_IPV6_ADDR_LEN]; /* 从 PDP上下文带来的IPV6主DNS长度，不包括":" */
    VOS_UINT8               secDns[TAF_IPV6_ADDR_LEN];  /* 从 PDP上下文带来的IPV6副DNS长度，不包括":" */
    TAF_IFACE_Ipv6PcscfList pcscfList;
} TAF_IFACE_Ipv6DhcpParam;


typedef struct {
    VOS_UINT32 opLanAddr : 1;
    VOS_UINT32 opPrefixAddr : 1;
    VOS_UINT32 opMtuSize : 1;
    VOS_UINT32 opPreferredLifetime : 1;
    VOS_UINT32 opValidLifetime : 1;
    VOS_UINT32 opSpare : 27;

    VOS_UINT32 curHopLimit : 8;
    VOS_UINT32 mflag : 1;
    VOS_UINT32 oflag : 1;
    VOS_UINT32 lflag : 1; /* 1: VALID; 0: INVALID */
    VOS_UINT32 aflag : 1;
    VOS_UINT32 rsv : 20;

    VOS_UINT8  lanAddr[TAF_IPV6_ADDR_LEN];    /* IPv6 路由器LAN端口地址 */
    VOS_UINT8  prefixAddr[TAF_IPV6_ADDR_LEN]; /* IPv6前缀 */
    VOS_UINT32 prefixBitLen;                  /* IPv6前缀长度 */
    VOS_UINT32 mtuSize;                       /* RA消息中广播的IPv6的MTU的取值 */
    VOS_UINT32 preferredLifetime;             /* IPv6前缀的Preferred lifetime */
    VOS_UINT32 validLifetime;                 /* IPv6前缀的Valid lifetime */
} TAF_IFACE_Ipv6RaInfo;


typedef struct {
    TAF_IFACE_StateUint8    state; /* IPv6 状态 */
    VOS_UINT8               rabId; /* RAB标识，取值范围:[5,15] */
    VOS_UINT8               pduSessionId;
    VOS_UINT8               rsv[5];   /* 保留位 */
    TAF_IFACE_Ipv6RaInfo    raInfo;   /* IPv6 路由公告信息 */
    TAF_IFACE_Ipv6DhcpParam dhcpInfo; /* IPv6 DHCP信息 */
} TAF_IFACE_Ipv6Info;


typedef struct {
    TAF_Ctrl         ctrl;
    TAF_IFACE_Active ifaceUp;
} TAF_IFACE_UpReq;


typedef struct {
    TAF_Ctrl           ctrl;
    TAF_IFACE_Deactive ifaceDown;
} TAF_IFACE_DownReq;


typedef struct {
    TAF_Ctrl           ctrl;
    TAF_PS_CauseUint32 cause;
    VOS_UINT8          cid;
    VOS_UINT8          reserved[3];
} TAF_IFACE_UpCnf;


typedef struct {
    TAF_Ctrl           ctrl;
    TAF_PS_CauseUint32 cause;
} TAF_IFACE_DownCnf;


typedef struct {
    TAF_Ctrl                ctrl;
    VOS_UINT32              opBackOffTimer : 1;
    VOS_UINT32              opAllowedSscMode : 1;
    VOS_UINT32              opSpare : 30;
    TAF_IFACE_StatusUint8   status;       /* 上报状态类型 */
    VOS_UINT8               cid;          /* Cid信息 */
    TAF_PDP_TypeUint8       pdpType;      /* PDP TYPE类型 */
    TAF_IFACE_UserTypeUint8 userType;     /* 拨号用户类型 */
    TAF_PS_CauseUint32      cause;        /* 去激活原因值 */
    VOS_UINT32              backOffTimer; /* backoff tiemr 单位s */
    union {
        struct {
            VOS_UINT8 sscMode1Allowed : 1; /* VOS_TRUE :allowed，VOS_FALSE :not allowed */
            VOS_UINT8 sscMode2Allowed : 1; /* VOS_TRUE :allowed，VOS_FALSE :not allowed */
            VOS_UINT8 sscMode3Allowed : 1; /* VOS_TRUE :allowed，VOS_FALSE :not allowed */
            VOS_UINT8 spare : 5;
        };
        VOS_UINT8 allowedSscMode; /* allowed SSC MODE */
    };
    VOS_UINT8 resv[3];
} TAF_IFACE_StatusInd;


typedef struct {
    TAF_Ctrl ctrl;

    VOS_UINT32 opActFlg : 1;
    VOS_UINT32 opCleanFlg : 1;
    VOS_UINT32 opSpare : 30;

    VOS_UINT8 cid;    /* Cid信息 */
    VOS_UINT8 rsv[3]; /* 保留位 */
} TAF_IFACE_DataChannelStateInd;


typedef struct {
    TAF_Ctrl   ctrl;
    VOS_UINT32 opActUsbNet : 1;   /* 通知激活USB NET标志位 */
    VOS_UINT32 opDeactUsbNet : 1; /* 通知去激活USB NET标志位 */
    VOS_UINT32 opSpare : 30;
    VOS_UINT8  ifaceId;
    VOS_UINT8  resv[3];
} TAF_IFACE_UsbnetOperInd;


typedef struct {
    TAF_Ctrl   ctrl;
    VOS_UINT32 cid; /* Bit Cid信息 */
} TAF_IFACE_GetDynamicParaReq;


typedef struct {
    TAF_IFACE_StateUint8    state;        /* 状态 */
    VOS_UINT8               pduSessionId; /* PduSessionId标识 */
    VOS_UINT16              mtu;          /* Ethernet MTU的值 */
} TAF_IFACE_EtherInfo;


typedef struct {
    VOS_UINT32 opIpv4Valid : 1;
    VOS_UINT32 opIpv6Valid : 1;
    VOS_UINT32 opEtherValid : 1;
    VOS_UINT32 opSpare : 29;

    TAF_IFACE_UserTypeUint8 userType;                 /* 用户呼叫类型 */
    VOS_UINT8               apnLen;                   /* APN长度 */
    VOS_UINT8               apn[TAF_MAX_APN_LEN + 1]; /* APN信息 */
    VOS_UINT8               rsv[6];                   /* 保留位 */
    TAF_IFACE_Ipv4Info      ipv4Info;                 /* 呼叫IPV4相关信息记录 */
    TAF_IFACE_Ipv6Info      ipv6Info;                 /* 呼叫IPV6相关信息记录 */
    TAF_IFACE_EtherInfo     etherInfo;                /* 呼叫Ethernet相关信息记录 */
} TAF_IFACE_DynamicInfo;


typedef struct {
    TAF_Ctrl              ctrl;
    VOS_UINT32            cid;
    TAF_IFACE_DynamicInfo dynamicInfo[TAF_MAX_CID_NV + 1]; /* 动态详细信息 */
} TAF_IFACE_GetDynamicParaCnf;


typedef struct {
    TAF_Ctrl ctrl;

    VOS_UINT32 opPdpAddr : 1;
    VOS_UINT32 opSpare : 31;

    VOS_UINT8                  oldRabId;
    VOS_UINT8                  newRabId;
    TAF_IFACE_RabOperTypeUint8 operType;
    PS_IFACE_IdUint8           ifaceId;
    TAF_PDP_TypeUint8          pdpType;
    VOS_UINT8                  reserved[3];
    VOS_UINT32                 ipAddr;
} TAF_IFACE_RabInfoInd;


typedef struct {
    TAF_Ctrl                        ctrl;
    VOS_UINT8                       cid;
    PS_IFACE_IdUint8                ifaceId;
    TAF_IFACE_IpChangeOperTypeUint8 operateType;
    TAF_PDP_TypeUint8               pdpType;
    VOS_UINT32                      ipv4Addr;                    /* IPV4的IP地址，主机序 */
    VOS_UINT8                       ipv6Addr[TAF_IPV6_ADDR_LEN]; /* 从PDP上下文带来的IPV6地址长度，不包括":" */
} TAF_IFACE_IpChangeInd;


typedef struct {
    TAF_Ctrl ctrl;
    TAF_IFACE_Ipv6Info ipv6PdnInfo;
} TAF_IFACE_Ipv6PdnInfoInd;

/*
 * 功能描述: 调用DSM IFACE接口发起激活IFACE网卡
 */
VOS_UINT32 TAF_IFACE_Up(const TAF_Ctrl *ctrl, const TAF_IFACE_Active *ifaceUp);

/*
 * 功能描述: 调用DSM IFACE接口断开IFACE网卡
 */
VOS_UINT32 TAF_IFACE_Down(const TAF_Ctrl *ctrl, const TAF_IFACE_Deactive *ifaceDown);

/*
 * 功能描述: 调用DSM IFACE接口发起查询对应Bit Cid的动态信息
 */
VOS_UINT32 TAF_IFACE_GetDynamicPara(const TAF_Ctrl *ctrl, const VOS_UINT32 bitCid);

#if ((FEATURE_SHARE_APN == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
VOS_UINT32 TAF_IFACE_PppUp(const TAF_PPP_ReqConfigInfo *pppReqConfigInfo, VOS_UINT32 moduleId,
    VOS_UINT16 exClientId, VOS_UINT8 opId, VOS_UINT8 cid);

VOS_UINT32 TAF_IFACE_PppDown(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
