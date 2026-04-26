/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef _NAS_URSP_H
#define _NAS_URSP_H

#include "nas_comm_packet_ser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * #pragma pack(*)    设置字节对齐方式
 */
#pragma pack(push, 4)

#define PS_URSP_DEST_FQDN_MAX_LEN 255
#define PS_URSP_OS_ID_LEN 16
#define PS_URSP_OS_APP_ID_MAX_LEN 255
#define PS_URSP_MAX_RSD_NUM     8
#define PS_URSP_MAX_S_NSSAI_NUM 4
#define PS_URSP_MAX_DNN_NUM     4

enum PS_URSP_MatchAll {
    PS_URSP_NOT_MATCH_ALL = 0x00,
    PS_URSP_MATCH_ALL     = 0x01,

    PS_URSP_MATCH_ALL_BUTT
};
typedef VOS_UINT8 PS_URSP_MatchAllUint8;

enum PS_URSP_ProtocalVersion {
    PS_URSP_PROTOCAL_VERSION_R15_F00 = 0x1500,
    PS_URSP_PROTOCAL_VERSION_R15_F10 = 0x1510,
    PS_URSP_PROTOCAL_VERSION_R15_F20 = 0x1520,
    PS_URSP_PROTOCAL_VERSION_R15_F30 = 0x1530,
    PS_URSP_PROTOCAL_VERSION_BUTT
};
typedef VOS_UINT32 PS_URSP_ProtocalVersionUint32;

enum PS_URSP_RuleType {
    PS_URSP_NON_DEFAULT_RULE = 0,
    PS_URSP_DEFAULT_RULE = 1,
    PS_URSP_RULE_TYPE_BUTT
};
typedef VOS_UINT8 PS_URSP_RuleTypeUnit8;

enum PS_URSP_TdType {
    PS_URSP_TD_DNN_TYPE = 1,
    PS_URSP_TD_OS_ID_AND_OS_APP_ID_TYPE= 2,
    PS_URSP_TD_OS_APP_ID_TYPE = 4,
    PS_URSP_TD_IP_TUPLE_TYPE = 8,
    PS_URSP_TD_NON_IP_TUPLE_TYPE = 16,
    PS_URSP_TD_FQDN_TYPE = 32,
    PS_URSP_TD_CONN_CAP_TYPE = 64,
    PS_URSP_TD_TYPE_BUTT
};
typedef VOS_UINT32 PS_URSP_TdTypeUint32;

typedef struct {
    VOS_UINT8 value[PS_URSP_OS_ID_LEN];
} PS_URSP_OsId;

typedef struct {
    VOS_UINT32 len; /* 取值0-255 */
    VOS_UINT8 value[PS_URSP_OS_APP_ID_MAX_LEN + 1];
} PS_URSP_OsAppId;

typedef struct {
    PS_URSP_OsId osId;
    PS_URSP_OsAppId osAppId;
} PS_URSP_OsIdAndOsAppId;

typedef struct {
    VOS_UINT8 value[PS_URSP_DEST_FQDN_MAX_LEN + 1];
} PS_URSP_Fqdn;

typedef struct {
    VOS_UINT32 opProtocolId : 1;
    VOS_UINT32 opSingleRmtPort : 1;
    VOS_UINT32 opSecuParaIndex : 1;
    VOS_UINT32 opFlowLabelType : 1;
    VOS_UINT32 opTos : 1;
    VOS_UINT32 opRmtPortRange: 1;
    VOS_UINT32 opRmtIpv4Addr : 1;
    VOS_UINT32 opRmtIpv6Addr : 1;
    VOS_UINT32 opSpare : 24;

    VOS_UINT8 protocolId; /* 协议号 */
    VOS_UINT8 rsv;
    VOS_UINT16 singleRmtPort;
    VOS_UINT32 secuParaIndex;     /* SPI */
    VOS_UINT32 flowLabelType;
    PS_TypeOfSer tos;
    PS_PortRange rmtPortRange;
    PS_Ipv4AddrMask rmtIpv4Addr;
    PS_Ipv6AddrPrefix rmtIpv6Addr;
} PS_URSP_IpTuple;

typedef struct {
    VOS_UINT32 opDstMacAddr: 1;
    VOS_UINT32 opEtherType : 1;
    VOS_UINT32 opCTagVid : 1;
    VOS_UINT32 opSTagVid : 1;
    VOS_UINT32 opCTagPcpDei: 1;
    VOS_UINT32 opSTagPcpDei: 1;
    VOS_UINT32 opSpare : 26;

    PS_MacAddr dstMacAddr;
    VOS_UINT16 etherType;
    VOS_UINT16 cTagVid;
    VOS_UINT16 sTagVid;
    VOS_UINT8  cTagPcpDei; /* 其中bit1保存dei, bit2~4保存pcp */
    VOS_UINT8  sTagPcpDei; /* 其中bit1保存dei, bit2~4保存pcp */
} PS_URSP_NonIpTuple;

typedef struct {
    VOS_UINT32 type; /* 0-6bit分别代表一种类型 */
    PS_URSP_MatchAllUint8 matchAllFlg;
    VOS_UINT8 connCap; /* connection capabilities */
    VOS_UINT8 rsv[2];
    PS_URSP_OsAppId osAppId;
    PS_URSP_OsIdAndOsAppId osIdAndOsAppId;
    PS_APN dnn;
    PS_URSP_Fqdn fqdn;
    PS_URSP_IpTuple ipTuple;
    PS_URSP_NonIpTuple nonIpTuple;
} PS_URSP_Td;

typedef struct {
    VOS_UINT32 opSscMode : 1;
    VOS_UINT32 opPduSessionType : 1;
    VOS_UINT32 opPrefAccType : 1;
    VOS_UINT32 opSpare : 29;

    VOS_UINT8               urspPrecedence;
    VOS_UINT8               rsdPrecedence;
    PS_SSC_ModeUint8        sscMode;
    PS_PDU_SessionTypeUint8 pduSessionType;
    PS_PREF_AccessTypeUint8 prefAccType;
    PS_NON_SeamlessNon3GppOffloadIndUint8 offloadFlg;  /* non-seamless non-3GPP offload indication type */
    PS_URSP_RuleTypeUnit8 type;  /* URSP RULE TYPE, 0 NON-DEFAULT RULE; 1 DEFAULT RULE */
    VOS_UINT8       sNssaiNum;
    VOS_UINT8       dnnNum;
    VOS_UINT8       rsv[3];
    PS_S_NSSAI_STRU sNssai[PS_URSP_MAX_S_NSSAI_NUM];
    PS_APN          dnn[PS_URSP_MAX_DNN_NUM];
} PS_URSP_Rsd;

typedef struct {
    VOS_UINT8 rsdNum;
    VOS_UINT8 rsv[3];
    PS_URSP_Rsd rsd[PS_URSP_MAX_RSD_NUM];
} PS_URSP_RsdList;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

