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

#ifndef PPPA_PPPS_AT_TYPE_H
#define PPPA_PPPS_AT_TYPE_H

#include "v_typdef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(4)

#ifndef IPV4_ADDR_LEN
#define   IPV4_ADDR_LEN                     4       /*IPV4地址长度*/
#endif

#ifndef IPV6_INTERFACE_ID_LEN
#define   IPV6_INTERFACE_ID_LEN             8       /* IPV6 interface id长度 */
#endif

#define PPPS_IPV6CP_CI_LEN                  10    /* IPV6CP 协商包长度 */


#define AUTHLEN                             100    /* Size of authname/authkey(porting from BSD, not alter) */
#define PASSWORDLEN                         100    /* Size of authname/authkey */
#define CHAPCHALLENGELEN                    48     /* Maximum chap challenge(porting from BSD, not alter) */
#define MD5DIGESTSIZE                       16     /* MD5 (Message-Digest) hash size */

#define PPP_PAP_REQ_BUF_MAX_LEN             (PASSWORDLEN + AUTHLEN)

/* code(1B)+id(1B)+length(2B)+challenge_size(1B)+challenge+name */
#define PPP_CHAP_CHALLENGE_BUF_MAX_LEN      (1 + 1 + 2 + 1 + CHAPCHALLENGELEN + AUTHLEN)

/* code(1B)+id(1B)+length(2B)+response_size(1B)+response+name */
#define PPP_CHAP_RESPONSE_BUF_MAX_LEN       (1 + 1 + 2 + 1 + PASSWORDLEN + AUTHLEN)

#define PPP_AUTH_FRAME_BUF_MAX_LEN          256
#define PPP_IPCP_FRAME_BUF_MAX_LEN          256
#define PPP_IPV6CP_FRAME_BUF_MAX_LEN        256

#define PPP_DATA_PRIORITY_NORMAL 0
#define PPP_DATA_PRIORITY_HIGH 1

typedef struct {
    VOS_UINT16  papReqLen;                            /*request长度: 24.008要求在[3,253]字节*/
    VOS_UINT8   reserve[2];                          /* 对齐保留 */
    VOS_UINT8   papReqBuf[PPP_PAP_REQ_BUF_MAX_LEN];  /*request*/
} PPPA_AuthPapContent;

typedef struct {
    VOS_UINT16  chapChallengeLen;                     /*challenge长度: 24.008要求在[3,253]字节*/
    VOS_UINT16  chapResponseLen;                      /*response长度: 24.008要求在[3,253]字节*/
    VOS_UINT8   chapChallengeBuf[PPP_CHAP_CHALLENGE_BUF_MAX_LEN];  /*challenge,153B*/
    VOS_UINT8   chapResponseBuf[PPP_CHAP_RESPONSE_BUF_MAX_LEN];    /*response,205B*/
    VOS_UINT8   reserve[2];
} PPPA_AuthChapContent;

/* IP 能力 */
enum PPP_IpCapability {
    PPP_CAPABILITY_IPV4                  = 1,
    PPP_CAPABILITY_IPV6                  = 2,
    PPP_CAPABILITY_IPV4V6                = 3,
    PPP_CAPABILITY_BUTT
};
typedef VOS_UINT32 PPP_IpCapabilityUint32;

/* 鉴权类型 */
enum PPP_AuthType {
    PPP_NO_AUTH_TYPE                     = 0,
    PPP_PAP_AUTH_TYPE                    = 1,
    PPP_CHAP_AUTH_TYPE                   = 2,
    PPP_MS_CHAPV2_AUTH_TYPE              = 3,
    PPP_AUTH_TYPE_BUTT
};
typedef VOS_UINT8 PPP_AuthTypeUint8;


typedef struct {
    PPP_AuthTypeUint8  authType;
    VOS_UINT8                 reserve[3];              /* 对齐保留 */

    union {
        PPPA_AuthPapContent  papContent;
        PPPA_AuthChapContent chapContent;
    } authContent;
} PPPA_ReqAuthConfigInfo;

typedef struct { 
    VOS_UINT16  ipcpLen;                              /*Ipcp帧长度*/
    VOS_UINT8   reserve[2];                          /* 对齐保留 */
    VOS_UINT8   ipcp[PPP_IPCP_FRAME_BUF_MAX_LEN];    /*Ipcp帧*/
} PPPA_ReqIpcpConfigInfo;

typedef struct {
    VOS_UINT16  ipv6cpLen;                              /* Ipv6cp帧长度 */
    VOS_UINT8   reserve[2];                             /* 对齐保留 */
    VOS_UINT8   ipv6cp[PPP_IPV6CP_FRAME_BUF_MAX_LEN];   /* Ipv6cp帧 */
} PPPA_ReqIpv6cpConfigInfo;


typedef struct{
    PPPA_ReqAuthConfigInfo authConfig;
    PPPA_ReqIpcpConfigInfo ipcpConfig;
    PPPA_ReqIpv6cpConfigInfo ipv6cpConfig;
} PPPA_PdpActiveConfig;

typedef struct {
    VOS_UINT32                          opPriDns    : 1;             /*Primary DNS server Address*/
    VOS_UINT32                          opSecDns    : 1;             /*Secondary DNS server Address*/
    VOS_UINT32                          opGateWay   : 1;             /*Peer IP address*/
    VOS_UINT32                          opPriNbns   : 1;             /*Primary WINS DNS address*/
    VOS_UINT32                          opSecNbns   : 1;             /*Seocndary WINS DNS address*/
    VOS_UINT32                          opSpare     : 27;            /*Secondary DNS server Address*/

    VOS_UINT8                           priDns[IPV4_ADDR_LEN];
    VOS_UINT8                           secDns[IPV4_ADDR_LEN];
    VOS_UINT8                           gateWay[IPV4_ADDR_LEN];
    VOS_UINT8                           priNbns[IPV4_ADDR_LEN];
    VOS_UINT8                           secNbns[IPV4_ADDR_LEN];
} PPPA_PcoIpv4Item;

/*对应PPP_IND_CONFIG_INFO_STRU，用于消息方式交互*/
typedef struct {
    VOS_UINT8                    ipAddr[IPV4_ADDR_LEN];       /*主机IPV4地址*/
    PPPA_PcoIpv4Item             pcoIpv4Item;                  /*PCO信息*/
    VOS_UINT8                    ipv6Addr[IPV6_INTERFACE_ID_LEN]; /* 主机IPV6地址 */
    VOS_UINT8                    ipv4Flag; /* 是否配置了IPV4的地址 */
    VOS_UINT8                    ipv6Flag; /* 是否配置了IPV6的地址 */
    VOS_UINT8                    rsv[2];
} PPPA_PdpActiveResult;

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