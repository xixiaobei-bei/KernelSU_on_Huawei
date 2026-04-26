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

#ifndef __ATCTXPACKET_H__
#define __ATCTXPACKET_H__

#include "vos_pid_def.h"
#include "v_id.h"
#include "hi_list.h"
#include "at_type_def.h"
#include "mn_client.h"
#include "taf_ps_api.h"
#include "nv_stru_gucnas.h"
#include "at_timer.h"
#include "imsa_wifi_interface.h"
#include "taf_iface_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_IPV6_CAPABILITY_IPV4_ONLY 1
#define AT_IPV6_CAPABILITY_IPV6_ONLY 2
#define AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP 4
#define AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP 8

#define AT_PS_RABID_OFFSET 5     /* RABID偏移 */
#define AT_PS_RABID_MAX_NUM 11   /* RABID数量 */
#define AT_PS_MIN_RABID 5        /* RABID最小值 */
#define AT_PS_MAX_RABID 15       /* RABID最大值 */
#define AT_PS_INVALID_RABID 0xFF /* RABID无效值 */
#define AT_PS_RABID_MODEM_1_MASK 0x40

#define IPV6_ADDRESS_TEST_MODE_ENABLE 0x55aa55aa /* 0x55aa55aa值IPV6地址为测试模式 */

#define AT_PS_APN_CUSTOM_CHAR_MAX_NUM 16 /* 需要与TAF_NVIM_APN_CUSTOM_CHAR_MAX_NUM保持一致 */
#define AT_PS_ASCII_UINT_SEPARATOR 31
#define AT_PS_ASCII_SLASH 47
#define AT_PS_ASCII_BACKSLASH 92
#define AT_PS_ASCII_DELETE 127
#define AT_PS_RAC_STRING_LENGTH 3
#define AT_PS_LAC_STRING_LENGTH 3
#define AT_PS_SGSN_STRING_LENGTH 4
#define AT_PS_RNC_STRING_LENGTH 3
#define AT_PS_GRPS_STRING_LENGTH 4

/* 消息处理函数指针 */
typedef VOS_VOID (*AT_PS_RPT_IFACE_RSLT_FUNC)(TAF_IFACE_StatusInd *pstIfaceStatus);

/*
 * 结构说明: AT^CHDATA命令设置的<datachannelid>的取值
 */
enum AT_CH_DataChannel {
    AT_CH_DATA_CHANNEL_ID_0 = 0,
    AT_CH_DATA_CHANNEL_ID_1,
    AT_CH_DATA_CHANNEL_ID_2,
    AT_CH_DATA_CHANNEL_ID_3,
    AT_CH_DATA_CHANNEL_ID_4,
    AT_CH_DATA_CHANNEL_ID_5,
    AT_CH_DATA_CHANNEL_ID_6,
    AT_CH_DATA_CHANNEL_ID_7,
    AT_CH_DATA_CHANNEL_ID_8,
    AT_CH_DATA_CHANNEL_ID_9,
    AT_CH_DATA_CHANNEL_ID_10,
    AT_CH_DATA_CHANNEL_ID_11,
    AT_CH_DATA_CHANNEL_ID_12,
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    AT_CH_DATA_CHANNEL_ID_13,
    AT_CH_DATA_CHANNEL_ID_14,
    AT_CH_DATA_CHANNEL_ID_15,
#endif
    AT_CH_DATA_CHANNEL_BUTT
};
typedef VOS_UINT32 AT_CH_DataChannelUint32;


/*lint -e958 -e959 原因:64bit*/
typedef struct {
    TAF_IFACE_UserTypeUint8   userType;
    AT_PS_RPT_IFACE_RSLT_FUNC rptIfaceRsltFunc;
} AT_PS_ReportIfaceResult;
/*lint +e958 +e959 原因:64bit*/


typedef struct {
    /* 指定CID是否已经通过CHDATA配置了数传通道，VOS_TRUE:已经配置；VOS_FALSE:未配置 */
    VOS_UINT32 used;
    VOS_UINT32 rmNetId; /* 数据通道ID
                             VCOM通道 :RNIC_DEV_ID_RMNET0 ~ RNIC_DEV_ID_RMNET4
                             */
    VOS_UINT32 ifaceId;
    /* 指定CID是否已经PDP激活，VOS_TRUE:已经激活；VOS_FALSE:未激活 */
    VOS_UINT32        ifaceActFlg;
    ModemIdUint16     modemId;
    AT_ClientIdUint16 portIndex;
    VOS_UINT8         cid;
    VOS_UINT8         reserved[3];
} AT_PS_DataChanlCfg;

/*
 * 结构说明: IMS EMC IPV4 PDN信息
 */
typedef struct {
    VOS_UINT8  ipAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8  dnsPrimAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT8  dnsSecAddr[TAF_IPV4_ADDR_LEN];
    VOS_UINT16 mtu;
    VOS_UINT16 reserved;

} AT_IMS_EmcIpv4PdnInfo;

/*
 * 结构说明: IMS EMC IPV6 PDN信息
 */
typedef struct {
    VOS_UINT8  ipAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8  dnsPrimAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8  dnsSecAddr[TAF_IPV6_ADDR_LEN];
    VOS_UINT16 mtu;
    VOS_UINT16 reserved[3];

} AT_IMS_EmcIpv6PdnInfo;

/*
 * 结构说明: IMS EMC 动态参数结构
 */
typedef struct {
    VOS_UINT32 opIPv4PdnInfo : 1;
    VOS_UINT32 opIPv6PdnInfo : 1;
    VOS_UINT32 opSpare : 30;

    AT_IMS_EmcIpv4PdnInfo iPv4PdnInfo;
    AT_IMS_EmcIpv6PdnInfo iPv6PdnInfo;

} AT_IMS_EmcRdp;


typedef struct {
    VOS_UINT8 customCharNum;
    VOS_UINT8 protocolStringCheckFlag;
    VOS_UINT8 reserved1;
    VOS_UINT8 reserved2;
    VOS_UINT8 reserved3;
    VOS_UINT8 reserved4;
    VOS_UINT8 reserved5;
    VOS_UINT8 reserved6;
    VOS_UINT8 customChar[AT_PS_APN_CUSTOM_CHAR_MAX_NUM];
} AT_PS_ApnCustomFormatCfg;


typedef struct {
    VOS_UINT8 ipv6Capability;
    /* IPv6地址格式 0: 十进制点分格式; 1: 16进制冒号分隔格式 */
    VOS_UINT8 opIpv6AddrFormat : 1;
    /* IPv6子网掩码格式 0: IP地址和子网掩码完整显示并通过空格分隔; 1: 通过斜线分隔掩码 */
    VOS_UINT8 opIpv6SubnetNotation : 1;
    /* 控制IPv6地址是否省略前导0 0: 省略前导0; 1: 不省略前导0 */
    VOS_UINT8 opIpv6LeadingZeros : 1;
    /* 控制IPv6地址是否压缩地址 0: 不压缩0; 1: 压缩0 */
    VOS_UINT8 opIpv6CompressZeros : 1;
    VOS_UINT8 opSpare : 4;
    VOS_UINT8  etherCap;                    /* Ethernet类型能力 */
    TAF_PS_CallRptCmdUint8  stickCmd;       /* stick拨号上报命令配置 */
    VOS_UINT32 ipv6AddrTestModeCfg;

    VOS_UINT8 sharePdpFlag;
    VOS_UINT8 reserved2[7];

    VOS_INT32  spePort;
    VOS_UINT32 ipfPortFlg;

    AT_PS_ApnCustomFormatCfg apnCustomFormatCfg[MODEM_ID_BUTT];

    /* 数传通道关系 */
    AT_PS_DataChanlCfg channelCfg[AT_CH_DATA_CHANNEL_BUTT + 1];
} AT_CommPsCtx;


typedef struct {
    /* PS域呼叫错误码 */
    TAF_PS_CauseUint32 psErrCause;

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    /* IP地址与IFACEID的映射表, IP地址为主机序 */
    VOS_UINT32 ipAddrIfaceIdMap[PS_IFACE_ID_BUTT];
#else
    /* IP地址与RABID的映射表, IP地址为主机序 */
    VOS_UINT32 ipAddrRabIdMap[AT_PS_RABID_MAX_NUM];
    VOS_UINT32 reserved;
#endif

#if (FEATURE_IMS == FEATURE_ON)
    /* IMS EMC PDN 动态信息 */
    AT_IMS_EmcRdp imsEmcRdp;
#endif

} AT_ModemPsCtx;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_ctx_packet.h */
