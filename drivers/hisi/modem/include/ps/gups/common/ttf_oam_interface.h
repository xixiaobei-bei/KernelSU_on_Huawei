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

#ifndef TTFOAMINTERFACE_H
#define TTFOAMINTERFACE_H

#include "vos.h"
#include "ttf_link_interface.h"
#include "PsTypeDef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(push, 4)

/* 脱敏开发定义的数据无效值 */
#define TTF_SENSITIVE_INVALID_VALUE 0

#define GRM_MNTN_TS_TOTAL 8     /* 时隙个数，ts0-ts7共8个 */
#define GRLC_MNTN_MAX_TBF_NUM 1 /* 当前支持的最大TBF规格为1，不支持多TBF */
#define WTTF_MNTN_MAX_AM_RB_NUM 8 /* 3个AM业务RB,外加RB2-RB4 三个信令RB,共计6个AM RB. 25.306 R9协议Table 5.1b升级到8个AM RB */
#define WTTF_MNTN_MAX_TM_RB_NUM 4 /* TM RB个数,RB0不计人, 3个AMR语音RB + 1个TM CSD共计4个TM RB, */
#define WTTF_MNTN_MAX_UM_RB_NUM 6 /* UM RB个数, CBS所对应的RB和RB0不计入,GCF用例要求三个以上UM业务RB,RB1为UM信令RB,共计3个UM RB ，与AM实体个数对应 */

#define WTTF_MNTN_MAX_RB_ID 32                  /* INTEGER (1..32) */
#define WTTF_MNTN_MAX_FLX_RLC_SIZE_LEVEL_CNT 10 /* 最大可用变长SIZE档位数 */
#define WTTF_MNTN_RLC_AM_MAX_SN_NUM 4096



/* 勾包PID范围 */
#define IS_WUEPS_PID_PDCP(x) ((x == I0_WUEPS_PID_PDCP) || (x == I1_WUEPS_PID_PDCP))
#define IS_WUEPS_PID_RLC(x) ((x == I0_WUEPS_PID_RLC) || (x == I1_WUEPS_PID_RLC))
#define IS_WUEPS_PID_RLCDL(x) ((x == I0_WUEPS_PID_RLCDL) || (x == I1_WUEPS_PID_RLCDL))
#define IS_WUEPS_PID_MAC(x) ((x == I0_WUEPS_PID_MAC) || (x == I1_WUEPS_PID_MAC))
#define IS_WUEPS_PID_MACDL(x) ((x == I0_WUEPS_PID_MACDL) || (x == I1_WUEPS_PID_MACDL))
#define IS_UEPS_PID_SN(x) ((x == I0_UEPS_PID_SN) || (x == I1_UEPS_PID_SN) || (x == I2_UEPS_PID_SN))
#define IS_UEPS_PID_LL(x) ((x == I0_UEPS_PID_LL) || (x == I1_UEPS_PID_LL) || (x == I2_UEPS_PID_LL))
#define IS_UEPS_PID_GRM(x) ((x == I0_UEPS_PID_GRM) || (x == I1_UEPS_PID_GRM) || (x == I2_UEPS_PID_GRM))
#define IS_UEPS_PID_DL(x) ((x == I0_UEPS_PID_DL) || (x == I1_UEPS_PID_DL) || (x == I2_UEPS_PID_DL))
#define IS_UEPS_PID_GMM(x) ((x == I0_WUEPS_PID_GMM) || (x == I1_WUEPS_PID_GMM) || (x == I2_WUEPS_PID_GMM))
#define IS_UEPS_PID_SMS(x) ((x == I0_WUEPS_PID_SMS) || (x == I1_WUEPS_PID_SMS) || (x == I2_WUEPS_PID_SMS))
#define IS_UEPS_PID_GAS(x) ((x == I0_UEPS_PID_GAS) || (x == I1_UEPS_PID_GAS) || (x == I2_UEPS_PID_GAS))
#define IS_UEPS_PID_GPHY(x) ((x == I0_DSP_PID_GPHY) || (x == I1_DSP_PID_GPHY) || (x == I2_DSP_PID_GPHY))
#define IS_UEPS_PID_WPHY(x) ((x == I0_DSP_PID_WPHY) || (x == I1_DSP_PID_WPHY))
#define IS_WUEPS_PID_WAS(x) \
    ((x == I0_WUEPS_PID_WRR) || (x == I1_WUEPS_PID_WRR) || (x == I0_WUEPS_PID_WCOM) || (x == I1_WUEPS_PID_WCOM))

#define IS_UEPS_PID_RABM(x) ((x == I0_WUEPS_PID_RABM) || (x == I1_WUEPS_PID_RABM) || (x == I2_WUEPS_PID_RABM))
#define IS_UEPS_PID_TAF(x) ((x == I0_WUEPS_PID_TAF) || (x == I1_WUEPS_PID_TAF) || (x == I2_WUEPS_PID_TAF))
#define IS_PID_USIM(x) (((x) == I0_WUEPS_PID_USIM) || ((x) == I1_WUEPS_PID_USIM) || ((x) == I2_WUEPS_PID_USIM))
#define IS_PID_EHSM(x) ((x) == UEPS_PID_EHSM)
/* 枚举说明  : 可维可测消息类型(透明消息), 把所有的透明消息的ID都定义在这里 */
enum TTF_MntnMsgType {
    /* TTF的透明消息目前都不区分MODEM */

    /* W模可维可测消息的编号以DO开头 */
    ID_OM_WTTF_RLC_DATA_TRANS_INFO_REQ      = 0xD001, /* W模下RLC 实体信息上报请求,其中共包括AM，UM，TM三种模式，根据用户的请求全部或部分显示 */
    ID_WTTF_OM_RLC_DATA_TRANS_INFO_CNF      = 0xD002, /* W模下RLC 实体信息上报回复 */
    ID_WTTF_OM_RLC_DATA_TRANS_INFO_IND      = 0xD003, /* W模下RLC 实体信息上报指示 */
    ID_OM_WTTF_ADVANCED_TRACE_CONFIG_REQ    = 0xD004, /* W模下Trace 高级配置 */
    ID_WTTF_OM_ADVANCED_TRACE_CONFIG_CNF    = 0xD005, /* W模下Trace 高级配置指示 */
    ID_OM_WTTF_RLC_PDU_SIZE_TRANS_INFO_REQ  = 0xD006, /* W模下RLC PDU SIZE统计信息上报请求 */
    ID_WTTF_OM_RLC_PDU_SIZE_TRANS_INFO_CNF  = 0xD007, /* W模下RLC PDU SIZE统计信息上报回复 */
    ID_WTTF_OM_RLC_PDU_SIZE_TRANS_INFO_IND  = 0xD008, /* W模下RLC PDU SIZE统计信息上报指示 */
    ID_OM_WTTF_UPA_DATA_STATUS_G_INFO_REQ   = 0xD009, /* W模下UPA 要数信息G值上报请求 */
    ID_WTTF_OM_UPA_DATA_STATUS_G_INFO_CNF   = 0xD00a, /* W模下UPA 要数信息G值上报回复 */
    ID_WTTF_OM_UPA_DATA_STATUS_G_INFO_IND   = 0xD00b, /* W模下UPA 要数信息G值上报指示 */
    ID_OM_TTF_MNTN_MSG_CTTF_OM_XXX_REQ      = 0xD00c,
    ID_OM_TTF_MNTN_MSG_CTTF_OM_YYY_REQ      = 0xD00d,
    ID_TTF_OM_MNTN_MSG_OM_CTTF_XXX_DATA_IND = 0xD00e,
    ID_TTF_OM_MNTN_MSG_OM_CTTF_ZZZ_DATA_IND = 0xD00f,
    ID_OM_WTTF_MAC_DATA_INFO_REQ            = 0xD010, /* W模下MAC 实体信息上报 */
    ID_OM_WTTF_MAC_DATA_INFO_CNF            = 0xD011, /* W模下MAC 实体信息回复 */
    ID_OM_WTTF_MAC_DATA_INFO_IND            = 0xD012, /* W模下MAC 实体信息指示 */

    /* G模可维可测消息的编号以D1开头 */
    ID_GRM_MNTN_OM_GRM_INFO_CFG_REQ          = 0xD104, /* 配置GRM信息观测功能 */
    ID_GRM_MNTN_GRM_OM_INFO_CFG_CNF          = 0xD105,
    ID_GRM_MNTN_GRM_OM_INFO                  = 0xD106,
    ID_GRM_MNTN_OM_GRM_THROUGHPUT_CFG_REQ    = 0xD107, /* OM向GRM请求配置GRM吞吐率统计信息观测功能消息项 */
    ID_GRM_MNTN_GRM_OM_THROUGHPUT_CFG_CNF    = 0xD108, /* GRM向OM回复配置GRM吞吐率统计信息观测功能消息项 */
    ID_GRM_MNTN_GRM_OM_THROUGHPUT_INFO       = 0xD109, /* GRM向OM上报GRM吞吐率统计信息消息项 */
    ID_GRM_MNTN_OM_GTTF_ADV_TRACE_SWITCH_REQ = 0xD10a, /* G模下Trace 高级配置开关请求 */
    ID_GRM_MNTN_GTTF_OM_ADV_TRACE_SWITCH_CNF = 0xD10b, /* G模下Trace 高级配置开关回复 */
    ID_SN_MNTN_OM_THROUGHPUT_CFG_REQ         = 0xD10c, /* OM向SNDCP请求配置SNDCP吞吐率统计信息观测功能消息项 */
    ID_SN_MNTN_OM_THROUGHPUT_CFG_CNF         = 0xD10d, /* SNDCP向OM回复配置SNDCP吞吐率统计信息观测功能消息项 */
    ID_SN_MNTN_OM_THROUGHPUT_INFO            = 0xD10e, /* SNDCP向OM上报GRM吞吐率统计信息消息项 */
    ID_LL_MNTN_OM_THROUGHPUT_CFG_REQ         = 0xD10f, /* OM向LLC请求配置LLC吞吐率统计信息观测功能消息项 */
    ID_LL_MNTN_OM_THROUGHPUT_CFG_CNF         = 0xD110, /* LLC向OM回复配置LLC吞吐率统计信息观测功能消息项 */
    ID_LL_MNTN_OM_THROUGHPUT_INFO            = 0xD111, /* LLC向OM上报GRM吞吐率统计信息消息项 */

    /* RRM的可维可测消息的编号以D2开头 */
    ID_RRM_OM_RESOURCE_STATE_INFO_IND = 0xD210, /* RRM上报的资源状态信息 */

    /* AP上的TCP协议栈的可维可测消息, 编号以D3开头 */
    ID_OM_IPS_MNTN_TRAFFIC_CTRL_REQ         = 0xD312, /* 开启IPS LOG流控功能 */
    ID_OM_IPS_ADVANCED_TRACE_CONFIG_REQ     = 0xD313, /* 捕获TCP/IP协议栈 报文配置 */
    ID_IPS_OM_ADVANCED_TRACE_CONFIG_CNF     = 0xD314, /* 捕获TCP/IP协议栈 报文指示 */
    ID_OM_IPS_MNTN_INFO_CONFIG_REQ          = 0xD315, /* 周期性捕获TCP/IP协议栈 基础信息配置 */
    ID_IPS_OM_MNTN_INFO_CONFIG_CNF          = 0xD316, /* 周期性捕获TCP/IP协议栈 基础信息配置指示 */
    ID_IPS_TRACE_INPUT_DATA_INFO            = 0xD317, /* TCP/IP可维可测接收报文 */
    ID_IPS_TRACE_OUTPUT_DATA_INFO           = 0xD318, /* TCP/IP可维可测发送报文 */
    ID_IPS_TRACE_BRIDGE_DATA_INFO           = 0xD319, /* TCP/IP网桥中转报文 */
    ID_IPS_TRACE_RECV_ARP_PKT               = 0xD31A, /* TCP/IP协议栈接收的ARP控制报文 */
    ID_IPS_TRACE_SEND_ARP_PKT               = 0xD31B, /* TCP/IP协议栈发送的ARP控制报文 */
    ID_IPS_TRACE_RECV_DHCPC_PKT             = 0xD31C, /* TCP/IP协议栈接收的DHCP控制报文 */
    ID_IPS_TRACE_SEND_DHCPC_PKT             = 0xD31D, /* TCP/IP协议栈发送的DHCP控制报文 */
    ID_IPS_TRACE_RECV_DHCPS_PKT             = 0xD31E, /* TCP/IP协议栈接收的DHCP控制报文 */
    ID_IPS_TRACE_SEND_DHCPS_PKT             = 0xD31F, /* TCP/IP协议栈发送的DHCP控制报文 */
    ID_IPS_TRACE_APP_CMD                    = 0xD320, /* APP控制命令信息 */
    ID_IPS_TRACE_MNTN_INFO                  = 0xD321, /* TCP/IP协议栈基础信息 */
    ID_IPS_TRACE_BRIDGE_PRE_ROUTING_INFO    = 0xD322,
    ID_IPS_TRACE_BRIDGE_POST_ROUTING_INFO   = 0xD323,
    ID_IPS_TRACE_BRIDGE_LOCAL_IN_INFO       = 0xD324,
    ID_IPS_TRACE_BRIDGE_LOCAL_OUT_INFO      = 0xD325,
    ID_IPS_TRACE_IP4_FORWARD_INFO           = 0xD326,
    ID_IPS_TRACE_IP4_LOCAL_IN_INFO          = 0xD327,
    ID_IPS_TRACE_IP4_LOCAL_OUT_INFO         = 0xD328,
    ID_IPS_TRACE_IP6_FORWARD_INFO           = 0xD329,
    ID_IPS_TRACE_IP6_LOCAL_IN_INFO          = 0xD32A,
    ID_IPS_TRACE_IP6_LOCAL_OUT_INFO         = 0xD32B,
    ID_IPS_TRACE_BR_FORWARD_FLOW_CTRL_START = 0xD32C,
    ID_IPS_TRACE_BR_FORWARD_FLOW_CTRL_STOP  = 0xD32D,
    ID_IPS_TRACE_ADS_UL                     = 0xD330,
    ID_IPS_TRACE_ADS_DL                     = 0xD331,

    /* CP上关于IP报文、PPP帧之类的维测都归入D5中 */
    /*
     * CP上的TCP协议栈的可维可测消息, 编号以D51开头
     * 考虑IP类的需求可能比较多, 多预留一些ID空间, D51~D55
     */
    ID_OM_IPS_CCORE_ADVANCED_TRACE_CONFIG_REQ = 0xD513, /* 捕获TCP/IP协议栈 报文配置 */
    ID_IPS_CCORE_OM_ADVANCED_TRACE_CONFIG_CNF = 0xD514, /* 捕获TCP/IP协议栈 报文指示 */
    ID_OM_IPS_CCORE_MNTN_INFO_CONFIG_REQ      = 0xD515, /* 周期性捕获TCP/IP协议栈 基础信息配置 */
    ID_IPS_CCORE_OM_MNTN_INFO_CONFIG_CNF      = 0xD516, /* 周期性捕获TCP/IP协议栈 基础信息配置指示 */
    ID_IPS_CCORE_TRACE_INPUT_DATA_INFO        = 0xD517, /* TCP/IP可维可测接收报文 */
    ID_IPS_CCORE_TRACE_OUTPUT_DATA_INFO       = 0xD518, /* TCP/IP可维可测发送报文 */
    ID_IPS_CCORE_TRACE_BRIDGE_DATA_INFO       = 0xD519, /* TCP/IP网桥中转报文 */
    ID_IPS_CCORE_TRACE_RECV_ARP_PKT           = 0xD51A, /* TCP/IP协议栈接收的ARP控制报文 */
    ID_IPS_CCORE_TRACE_SEND_ARP_PKT           = 0xD51B, /* TCP/IP协议栈发送的ARP控制报文 */
    ID_IPS_CCORE_TRACE_RECV_DHCPC_PKT         = 0xD51C, /* TCP/IP协议栈接收的DHCP控制报文 */
    ID_IPS_CCORE_TRACE_SEND_DHCPC_PKT         = 0xD51D, /* TCP/IP协议栈发送的DHCP控制报文 */
    ID_IPS_CCORE_TRACE_RECV_DHCPS_PKT         = 0xD51E, /* TCP/IP协议栈接收的DHCP控制报文 */
    ID_IPS_CCORE_TRACE_SEND_DHCPS_PKT         = 0xD51F, /* TCP/IP协议栈发送的DHCP控制报文 */
    ID_IPS_CCORE_TRACE_APP_CMD                = 0xD520, /* APP控制命令信息 */
    ID_IPS_CCORE_TRACE_MNTN_INFO              = 0xD521, /* TCP/IP协议栈基础信息 */
    ID_IPS_MNTN_CCORE_IMS_NIC_INFO            = 0xD522, /* IMS虚拟网卡统计信息 */
    ID_IPS_CCORE_TRACE_NIC_DATA_IND           = 0xD523, /* TCP/IP可维可测接收报文 */
    ID_IPS_CCORE_TRACE_NIC_DATA_REQ           = 0xD524, /* TCP/IP可维可测发送报文 */

    /* CP上的PPPC的可维可测消息, 编号以D56开头 */
    ID_OM_PPP_MNTN_CONFIG_REQ       = 0xD560, /* PPP收到om配置信息 */
    ID_PPP_OM_MNTN_CONFIG_CNF       = 0xD561, /* PPP确认收到om配置信息 */
    ID_PPP_OM_MNTN_STATISTIC_INFO   = 0xD562, /* PPP返回维测信息 */
    ID_OM_PPP_MNTN_TRACE_CONFIG_REQ = 0xD563, /* PPP收到OM 勾包配置信息 */

    ID_IMS_NIC_MNTN_TRACE_RCV_CONFIG_INFO     = 0xD5F1,
    ID_IMS_NIC_MNTN_TRACE_CONFIG_PROC_RSLT    = 0xD5F2,
    ID_IMS_NIC_MNTN_TRACE_RCV_CONFIG_DEL      = 0xD5F3,
    ID_IMS_NIC_MNTN_TRACE_RCV_CONFIG_DEL_RSLT = 0xD5F4,
    ID_IMS_NIC_MNTN_TRACE_CONFIG_STATUS       = 0xD5F5,
    ID_IMS_NIC_MNTN_TRACE_RX_PACKET_REG       = 0xD5F6, /* IMSNIC接收异常包注册信息 */
    ID_IMS_NIC_MNTN_TRACE_RX_PACKET_DEREG     = 0xD5F7, /* IMSNIC接收异常包去注册信息 */
    ID_IMS_NIC_MNTN_TRACE_RX_PACKE_NOTIFY     = 0xD5F8, /* IMSNIC异常包检测通知信息 */

    TTF_MNTN_MSG_TYPE_BUTT = 0xFFFF
};
typedef VOS_UINT16 TTF_MntnMsgTypeUint16;

enum TTF_MntnRptCmd {
    TTF_MNTN_RPT_CMD_STOP  = 0x00, /* 停止上报 */
    TTF_MNTN_RPT_CMD_START = 0x01  /* 开始上报 */
};
typedef VOS_UINT8 TTF_MntnRptCmdUint8;

enum TTF_MntnRlpRptCmd {
    TTF_MNTN_RLP_RPT_CMD_STOP  = 0x00, /* 停止上报 */
    TTF_MNTN_RLP_RPT_CMD_START = 0x01, /* 开始上报 */
    TTF_MNTN_RLP_RPT_CMD_CLEAR = 0x02  /* 清除 */
};
typedef VOS_UINT8 TTF_MntnRlpRptCmdUint8;

enum TTF_MntnCommRptCmd {
    TTF_MNTN_COMM_RPT_CMD_STOP  = 0x00, /* 停止上报 */
    TTF_MNTN_COMM_RPT_CMD_START = 0x01, /* 开始上报 */
    TTF_MNTN_COMM_RPT_CMD_CLEAR = 0x02  /* 清除 */
};
typedef VOS_UINT8 TTF_MntnCommRptCmdUint8;

enum IPS_MntnInfoAction {
    IPS_MNTN_INFO_REPORT_STOP  = 0, /* 开启周期性捕获TCP/IP协议栈基础信息 */
    IPS_MNTN_INFO_REPORT_START = 1  /* 关闭周期性捕获TCP/IP协议栈基础信息 */
};
typedef VOS_UINT16 IPS_MntnInfoActionUint16;

enum IPS_MntnResultType {
    IPS_MNTN_RESULT_OK               = 0,
    IPS_MNTN_RESULT_START_TIMER_FAIL = 1,
    IPS_MNTN_RESULT_ALLOC_MEM_FAIL   = 2,
    IPS_MNTN_RESULT_ALLOC_MSG_FAIL   = 3,
    IPS_MNTN_RESULT_INPUT_PARAM_ERR  = 4,
    IPS_MNTN_RESULT_LOCAL_PARAM_ERR  = 5
};
typedef VOS_UINT32 IPS_MntnResultTypeUint32;

enum GTTF_MntnOmRptAction {
    GTTF_MNTN_OM_REPORT_STOP  = 0,
    GTTF_MNTN_OM_REPORT_START = 1
};

typedef VOS_UINT16 GTTF_MntnOmRptActionUint16;

enum GTTF_ResultType {
    GTTF_RESULT_OK               = 0,
    GTTF_RESULT_START_TIMER_FAIL = 1,
    GTTF_RESULT_ALLOC_MEM_FAIL   = 2,
    GTTF_RESULT_ALLOC_MSG_FAIL   = 3,
    GTTF_RESULT_INPUT_PARAM_ERR  = 4,
    GTTF_RESULT_LOCAL_PARAM_ERR  = 5
};
typedef VOS_UINT32 GTTF_ResultTypeUint32;

enum TOOL_GttfCfgSwitch {
    TOOL_GTTF_CFG_SWITCH_OFF = 0,
    TOOL_GTTF_CFG_SWITCH_ON  = 1
};
typedef VOS_UINT32 TOOL_GttfCfgSwitchUint32;

enum GRM_TbfMode {
    Non_Extend_TBF = 0,
    Extend_TBF     = 1
};
typedef VOS_UINT8 GRM_TbfModeUint8;

enum GRM_TbfRlcMode {
    AM = 0, /* RLC AM mode */
    UM = 1  /* RLC UM mode */
};
typedef VOS_UINT8 GRM_TbfRlcModeUint8;

enum GRM_TbfCountDownFlag {
    No_Start_CV = 0,
    Start_CV    = 1
};
typedef VOS_UINT8 GRM_TbfCountDownFlagUint8;

enum GRM_TbfTi {
    TLLI_Absent     = 0,
    TLLI_Not_Absent = 1
};
typedef VOS_UINT8 GRM_TbfTiUint8;

enum GRM_TbfFinalBlkRxed {
    Not_received = 0, /* The Final Block the current TBF is not received. */
    Received     = 1  /* The Final Block the current TBF is received. */
};
typedef VOS_UINT8 GRM_TbfFinalBlkRxedUint8;

enum GRM_TbfEdgeCodeScheme {
    CS_1    = 0x0000,
    CS_2    = 0x0001,
    CS_3    = 0x0002,
    CS_4    = 0x0003,
    MCS_1   = 0x1000,
    MCS_2   = 0x1001,
    MCS_3   = 0x1002,
    MCS_4   = 0x1003,
    MCS_5   = 0x1004,
    MCS_6   = 0x1005,
    MCS_7   = 0x1006,
    MCS_8   = 0x1007,
    MCS_9   = 0x1008,
    MCS_5_7 = 0x1009,
    MCS_6_9 = 0x100a
};
typedef VOS_UINT16 GRM_TbfEdgeCodeSchemeUint16;

enum GRM_TbfTestMode {
    Test_Mode_A_Infinite   = 0, /* A模式，PDU个数无限 */
    Test_Mode_A_Normal     = 1, /* A模式，PDU个数有限 */
    Test_Mode_B            = 2, /* B模式 */
    Test_Mode_SRB_LoopBack = 3  /* EGPRS 小环回模式 */
};
typedef VOS_UINT8 GRM_TbfTestModeUint8;

enum GRM_TbfTimeSlotFlag {
    Not_Existed = 0, /* This TS is not existed */
    Existed     = 1  /* This TS is existed */
};
typedef VOS_UINT8 GRM_TbfTimeSlotFlagUint8;

enum IPS_MntnCcoreAction {
    IPS_MNTN_CCORE_INFO_REPORT_STOP  = 0, /* 开启周期性捕获TCP/IP协议栈基础信息 */
    IPS_MNTN_CCORE_INFO_REPORT_START = 1  /* 关闭周期性捕获TCP/IP协议栈基础信息 */
};
typedef VOS_UINT16 IPS_MntnCcoreActionUint16;

enum IPS_MntnCcoreResultType {
    IPS_MNTN_CCORE_RESULT_OK               = 0,
    IPS_MNTN_CCORE_RESULT_START_TIMER_FAIL = 1,
    IPS_MNTN_CCORE_RESULT_ALLOC_MEM_FAIL   = 2,
    IPS_MNTN_CCORE_RESULT_ALLOC_MSG_FAIL   = 3,
    IPS_MNTN_CCORE_RESULT_INPUT_PARAM_ERR  = 4,
    IPS_MNTN_CCORE_RESULT_LOCAL_PARAM_ERR  = 5
};
typedef VOS_UINT32 IPS_MntnCcoreResultTypeUint32;

/* 用于标识PHY-->MAC ID_WTTF_PHY_MAC_DATA_IND MAC-->PHY ID_WTTF_MAC_PHY_DATA_REQ */
enum WTTF_tracePhyMacSubAttrib { 
    WTTF_TRACE_PHY_MAC_NULL  = 0,
    WTTF_TRACE_CTRL_HEADER = 1,
    WTTF_TRACE_WHOLE_DATA  = 2
};
typedef VOS_UINT8 WTTF_tracePhyMacSubAttribUint8;

enum WTTF_TraceMsgSimpleAttrib {
    WTTF_TRACE_MSG_NO  = 0,
    WTTF_TRACE_MSG_YES = 1
};
typedef VOS_UINT8 WTTF_TraceMsgSimpleAttribUint8;

enum IPS_MntnCcoreTraceChosen {
    IPS_MNTN_CCORE_TRACE_NULL_CHOSEN             = 0, /* 不捕获报文信息 */
    IPS_MNTN_CCORE_TRACE_MSG_HEADER_CHOSEN       = 1, /* 捕获报文头部 */
    IPS_MNTN_CCORE_TRACE_CONFIGURABLE_LEN_CHOSEN = 2, /* 按照配置捕获报文 */
    IPS_MNTN_CCORE_TRACE_WHOLE_DATA_LEN_CHOSEN   = 3  /* 捕获报文全部内容 */
};
typedef VOS_UINT32 IPS_MntnCcoreTraceChosenUint32;

enum IPS_MntnTraceChosen {
    IPS_MNTN_TRACE_NULL_CHOSEN             = 0, /* 不捕获报文信息 */
    IPS_MNTN_TRACE_MSG_HEADER_CHOSEN       = 1, /* 捕获报文头部 */
    IPS_MNTN_TRACE_CONFIGURABLE_LEN_CHOSEN = 2, /* 按照配置捕获报文 */
    IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN   = 3  /* 捕获报文全部内容 */
};
typedef VOS_UINT32 IPS_MntnTraceChosenUint32;

enum TTF_MntnTraceChosen {
    TTF_MNTN_TRACE_NULL_CHOSEN             = 0, /* 不捕获报文信息 */
    TTF_MNTN_TRACE_MSG_HEADER_CHOSEN       = 1, /* 捕获报文头部 */
    TTF_MNTN_TRACE_CONFIGURABLE_LEN_CHOSEN = 2, /* 按照配置捕获报文 */
    TTF_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN   = 3  /* 捕获报文全部内容 */
};
typedef VOS_UINT32 TTF_MntnTraceChosenUint32;

/* 说明: ppp维测信息通知结果 */
enum PPP_MntnOmResultType {
    PPP_MNTN_OM_RESULT_OK              = 0,
    PPP_MNTN_OM_RESULT_INPUT_PARAM_ERR = 1,
    PPP_MNTN_OM_RESULT_BUTT
};
typedef VOS_UINT8 PPP_MntnOmResultTypeUint8;

enum WTTF_RlcMntnRbChosen {
    WTTF_RLC_MNTN_RB_NULL_CHOSEN       = 0x0,        /* 取消跟踪 */
    WTTF_RLC_MNTN_RB_SIGNALLING_CHOSEN = 0x0000000F, /* 信令RB */
    WTTF_RLC_MNTN_RB_PS_CHOSEN         = 0xFFFFFFF0, /* 业务RB */
    WTTF_RLC_MNTN_RB_ALL_CHOSEN        = 0xFFFFFFFF  /* ALL RB */
};
typedef VOS_UINT32 WTTF_RlcMntnRbChosenUint32;

/* 定义VISP日志的输出等级 */
enum IPS_MntnCcoreTcpipLogLevel {
    IPS_MNTN_CCORE_TCPIP_LOG_LEVEL_CRITICAL = 0,
    IPS_MNTN_CCORE_TCPIP_LOG_LEVEL_ERROR,
    IPS_MNTN_CCORE_TCPIP_LOG_LEVEL_WARNING,
    IPS_MNTN_CCORE_TCPIP_LOG_LEVEL_INFO,
    IPS_MNTN_CCORE_TCPIP_LOG_LEVEL_DEBUG,

    IPS_MNTN_CCORE_TCPIP_LOG_LEVEL_BUIT
};
typedef VOS_UINT8 IPS_MntnCcoreTcpipLogLevelUint8;

/* 枚举说明: 脱敏信息状态 */
enum TTF_SensitiveTraceStatus {
    TTF_SENSITIVE_TRACE_NOT_SENSITIVE_STATUS = 0x00,
    TTF_SENSITIVE_TRACE_IN_SENSITIVE_STATUS  = 0x01
};
typedef VOS_UINT32 TTF_SensitiveTraceStatusUint32;

typedef struct {
    VOS_MSG_HEADER

    VOS_UINT32 msgId;

    VOS_UINT16 originalId;
    VOS_UINT16 terminalId;
    VOS_UINT32 timeStamp;
    VOS_UINT32 sn;
} DIAG_TransMsgHeader;

/* W模可维可测消息 */
typedef struct {
    WTTF_RlcMntnRbChosenUint32 config; /* 共32bit,从低到高分别代表RB1~RB32,RB0默认钩取  */
                                                  /* 信令RB: 0x000F */
                                                  /* 业务RB: 0xFFF0 */
                                                  /* ALL RB: 0xFFFF */
                                                  /* 取消跟踪: 0x0000 */
    VOS_UINT16 tracePduSize;
    VOS_UINT16 traceCipherPduSize;
    VOS_UINT16 tracePduCnt;
    VOS_UINT8  rsv[2];
} WTTF_TraceMacRlcDataIndCfg;
typedef WTTF_TraceMacRlcDataIndCfg WTTF_TraceRlcMacDataReqCfg;

typedef struct {
    WTTF_tracePhyMacSubAttribUint8 config;
    VOS_UINT8                      rsv[3];
} WTTF_TracePhyMacDataIndCfg;

typedef WTTF_TracePhyMacDataIndCfg WTTF_TraceMacPhyDataReqCfg;
typedef WTTF_TracePhyMacDataIndCfg WTTF_TracePhyMacHsPduDataIndCfg;
typedef WTTF_TracePhyMacDataIndCfg WTTF_TraceUpaPhyDataReqCfg;

typedef struct {
    WTTF_TraceMsgSimpleAttribUint8 config;
    VOS_UINT8                      rsv[3];
} WTTF_TraceMsgSimpleCfg;

typedef struct {
    TTF_MntnTraceChosenUint32 choice;
    VOS_UINT32                traceDataLen; /* 当选择TTF_MNTN_TRACE_CONFIGURABLE_LEN_CHOSEN时，此值有效 */
} WTTF_MntnTraceDataCfg;

typedef WTTF_MntnTraceDataCfg WTTF_TracePdcpRabmDataIndCfg;
typedef WTTF_MntnTraceDataCfg WTTF_TraceRabmPdcpDataReqCfg;

typedef struct {
    IPS_MntnTraceChosenUint32 choice;
    VOS_UINT32                traceDataLen; /* 当选择TRACE_CONFIGURABLE_LEN_CHOSEN时，此值有效 */
} IPS_MntnTraceCfg;

typedef IPS_MntnTraceCfg IPS_MntnBridgeTraceCfg;
typedef IPS_MntnTraceCfg IPS_MntnInputTraceCfg;
typedef IPS_MntnTraceCfg IPS_MntnOutputTraceCfg;
typedef IPS_MntnTraceCfg IPS_MntnLocalTraceCfg;

typedef struct {
    IPS_MntnCcoreTraceChosenUint32 choice;
    VOS_UINT32                     traceDataLen; /* 当选择TRACE_CONFIGURABLE_LEN_CHOSEN时，此值有效 */
} IPS_MntnCcoreTraceCfg;

typedef IPS_MntnCcoreTraceCfg IPS_MntnCcoreBridgeTraceCfg;
typedef IPS_MntnCcoreTraceCfg IPS_MntnCcoreInputTraceCfg;
typedef IPS_MntnCcoreTraceCfg IPS_MntnCcoreOutputTraceCfg;


/* 对应消息: ID_OM_WTTF_RLC_DATA_TRANS_INFO_REQ */
typedef struct {
    DIAG_TransMsgHeader            transHdr;
    TTF_MntnRptCmdUint8            command; /* 开始或停止上报 */
    WTTF_TraceMsgSimpleAttribUint8 rptAm;
    WTTF_TraceMsgSimpleAttribUint8 rptUm;
    WTTF_TraceMsgSimpleAttribUint8 rptTm;
    VOS_UINT32                     rptPeriod; /* 【1..5】单位：秒,报告周期 */
} OM_WttfRlcDataTransReq;

/* 对应消息: ID_OM_WTTF_MAC_DATA_INFO_REQ */
typedef struct {
    DIAG_TransMsgHeader transHdr;
    TTF_MntnRptCmdUint8 command; /* 开始或停止上报 */
    VOS_UINT8           rsv[3];
    VOS_UINT32          rptPeriod; /* 【1..5】单位：秒,报告周期 */
} OM_WttfMacDataReq;

/* 对应消息: ID_OM_WTTF_MAC_DATA_INFO_CNF */
typedef struct {
    DIAG_TransMsgHeader  transHdr;
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败 */
} OM_WttfMacDataCnf;

/* 对应消息: ID_WTTF_OM_RLC_DATA_TRANS_INFO_CNF */
typedef struct {
    DIAG_TransMsgHeader      transHdr;
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败 */
} WTTF_OmRlcDataTransCnf;

typedef WTTF_OmRlcDataTransCnf OM_WttfRlcPduSizeTransCnf;
typedef WTTF_OmRlcDataTransCnf OM_WttfUpaDataStatusGCfg;

typedef struct {
    WTTF_TraceMacRlcDataIndCfg traceMacRlcDataIndCfg;
    WTTF_TraceRlcMacDataReqCfg traceRlcMacDataReqCfg;

    WTTF_TracePhyMacDataIndCfg tracePhyMacDataIndCfg;
    WTTF_TraceMacPhyDataReqCfg traceMacPhyDataReqCfg;

    WTTF_TracePhyMacHsPduDataIndCfg tracePhyMacHsPduDataIndCfg;
    WTTF_TraceUpaPhyDataReqCfg      traceUpaPhyDataReqCfg;

    WTTF_TraceMsgSimpleCfg tracePhyMacInfoIndCfg;
    WTTF_TraceMsgSimpleCfg traceUpaStatusIndCfg;
    WTTF_TraceMsgSimpleCfg traceUpaSchedStausIndCfg;

    WTTF_TraceMsgSimpleCfg traceMacPhyAmrDataIndCfg;
    WTTF_TraceMsgSimpleCfg tracePhyRlcAmrDataReqCfg;

    WTTF_TracePdcpRabmDataIndCfg tracePdcpRabmDataIndCfg;
    WTTF_TraceRabmPdcpDataReqCfg traceRabmPdcpDataReqCfg;

    WTTF_TraceMsgSimpleCfg traceRlcMacIPDataCfg;
    WTTF_TraceMsgSimpleCfg traceMacRlcIPDataCfg;
} WTTF_TraceMsgCfg;

typedef struct {
    DIAG_TransMsgHeader transHdr;
    WTTF_TraceMsgCfg    advancedConfig;
} OM_WttfAdvancedTraceCfgReq;

/* 对应消息: ID_WTTF_OM_ADVANCED_TRACE_CONFIG_CNF */
typedef struct {
    DIAG_TransMsgHeader      transHdr;
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败 */
} WTTF_OmAdvancedTraceCfgCnf;

/* ID_OM_WTTF_RLC_PDU_SIZE_TRANS_INFO_REQ */
typedef struct {
    DIAG_TransMsgHeader transHdr;
    TTF_MntnRptCmdUint8 command; /* 开始或停止上报 */
    VOS_UINT8           rsv[3];
    VOS_UINT32          rptPeriod; /* 【1..5】单位：秒,报告周期 */
} OM_WttfRlcPduSizeTransReq;

typedef struct {
    VOS_UINT16 pduSize;
    VOS_UINT8  rsv[2];
    VOS_UINT32 pduCount;
} WTTF_MntnRlcPdu;

typedef struct {
    VOS_UINT8          rbId;
    PS_BOOL_ENUM_UINT8 isExist;           /* PS_TRUE:表示此RbId存在并且支持RLC PDU SIZE可变。 */
    VOS_UINT8          rlcSizeLevelCnt; /* 有效的档位等级 */
    VOS_UINT8          rsv[1];
    WTTF_MntnRlcPdu    pduSize[WTTF_MNTN_MAX_FLX_RLC_SIZE_LEVEL_CNT]; /* 有效的RLC SIZE,下标对应ucPduSizeIdx */
} OM_WttfRlcPduSizeTrans;

/* 结构说明: WTTF MNTN RLC Data Trans Info 对象 */
typedef struct {
    VOS_UINT32                simuClock;
    OM_WttfRlcPduSizeTransReq omRlcPduSizeReq;
    OM_WttfRlcPduSizeTrans    pduSizeTrans[WTTF_MNTN_MAX_RB_ID + 1];
} WTTF_MntnRlcPduSizeTransObj;

/* 对应消息: ID_WTTF_OM_RLC_PDU_SIZE_TRANS_INFO_CNF */
typedef struct {
    DIAG_TransMsgHeader      transHdr;
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败 */
} WTTF_OmRlcPduSizeTransCnf;

/* 对应消息:  ID_OM_WTTF_UPA_DATA_STATUS_G_INFO_REQ */
typedef OM_WttfRlcPduSizeTransReq TTF_OmWttfUpaDataStatusGReq;

/* 对应消息:  ID_WTTF_OM_UPA_DATA_STATUS_G_INFO_CNF */
typedef struct {
    DIAG_TransMsgHeader      transHdr;
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败 */
} WTTF_OmUpaDataStatusGCnf;

typedef struct {
    VOS_UINT32 rlcResetNum; /* 发生Reset次数 */
    /*
     * 以下三种情况发送方会发起Reset
     * 1)"No_Discard after MaxDAT number of transmissions" is configured and VT(DAT) equals the value MaxDAT (see
     * subclause 9.7.3.4) 2)VT(MRW) equals the value MaxMRW; 3)A STATUS PDU or a piggybacked STATUS PDU including
     * "erroneous Sequence Number" is received (see clause 10)
     */

    VOS_UINT32 lowRlcPduTxBytes;  /* 低位:发送的RLC PDU字节数：统计的是从查询时刻开始计时，总共发送到MAC层的RLC PDUs的字节数（其中包括新产成的数据和控制PDU，重传的PDU，Reset PDU） */
    VOS_UINT32 highRlcPduTxBytes; /* 高位:发送的RLC PDU字节数：统计的是从查询时刻开始计时，总共发送到MAC层的RLC PDUs的字节数（其中包括新产成的数据和控制PDU，重传的PDU，Reset PDU） */

    VOS_UINT32 lowRlcSduTxBytes; /* 低位:发送的RLC SDU字节数：统计的是从查询时刻开始计时，总共被分段成PDU的SDUs的字节数 */
    VOS_UINT32 highRlcSduTxBytes; /* 高位:发送的RLC SDU字节数：统计的是从查询时刻开始计时，总共被分段成PDU的SDUs的字节数 */

    VOS_UINT32 curPrdRlcSduTxBytes; /* 当前上报周期内发送RLC SDU字节数 */
    VOS_UINT32 rlcDataPduTxNum; /* 发送的数据PDU个数：统计的是从查询时刻开始计时，总共发送的到MAC层的新产生的数据PDUs块数（其中不包括重传的数据PDU） */
    VOS_UINT32 rlcDataPduRetxNum; /* 发生重传的数据PDU个数：统计的是从查询时刻开始计时，总共发生重传的PDU个数（同一个PDU重传多次，则当多次累加）,新产生的数据PDU个数＝发送的数据PDU个数－ 发生重传的数据PDU个数 */
    VOS_UINT32 rlcNackedPduNum;   /* 负确认的PDU个数:统计的是从查询时刻开始计时,总共从对端收到负确认的PDU个数 */
    VOS_UINT32 rlcCtrlPduTxNum; /* 发送的控制PDU个数:统计的是从查询时刻开始计时,总共发送到MAC层的控制PDU个数(其中包括状态PDU，Reset，Reset Ack PDU) */
    VOS_UINT16 rlcLastTxPduSn;       /* [0..4095]最近一次发送的SN值:VtS-1 */
    VOS_UINT16 rlcLastAckInseqPduSn; /* [0..4095]最后一个按序被确认的SN:VtA */
    VOS_UINT16 rlcVacantTxWinSize;   /* [1..4095]指VtMs-VtS */
    VOS_UINT8  rsv[2];
    VOS_UINT32 rlcBo;                  /* 实体BO */
    VOS_UINT32 rlcDataPdu3ReTxNum;     /* 重传次数达到三次的数据PDU个数 */
    VOS_UINT32 rlcDataPdu4ReTxNum;     /* 重传次数达到四次的数据PDU个数 */
    VOS_UINT32 rlcDataPdu5ReTxNum;     /* 重传次数达到五次的数据PDU个数 */
    VOS_UINT32 rlcDataPdu6ReTxNum;     /* 重传次数达到六次的数据PDU个数 */
    VOS_UINT32 rlcDataPduMore7ReTxNum; /* 重传次数达到七次以上（包括七次）的数据PDU个数 */
    VOS_UINT32 reserve[7];             /* RTSOck cache-line32字节对齐，防止伪共享 */
} WTTF_MntnRlcAmUlDataTrans;

typedef struct {
    VOS_UINT32 rlcResetNum;       /* 发生Reset次数 */
    VOS_UINT32 lowRlcPduRxBytes;  /* 低位:接收到的RLC PDU 字节数: 统计的是从查询时刻开始计时，总共从MAC层接收到的RLC PDUs的字节数（其中包括了正确或错误的数据PDU和控制PDU，以及重复接收或不在接收窗内的数据PDU，甚至是CRC错误），所指的是MAC接收的吞吐量 */
    VOS_UINT32 highRlcPduRxBytes; /* 高位:接收到的RLC PDU 字节数: 统计的是从查询时刻开始计时，总共从MAC层接收到的RLC PDUs的字节数（其中包括了正确或错误的数据PDU和控制PDU，以及重复接收或不在接收窗内的数据PDU，甚至是CRC错误），所指的是MAC接收的吞吐量 */

    VOS_UINT32 lowRlcSduRxBytes;    /* 低位:接收到的SDU字节数: 统计的是从查询时刻开始计时，总共重组完成的SDU字节数 */
    VOS_UINT32 highRlcSduRxBytes;   /* 高位:接收到的SDU字节数: 统计的是从查询时刻开始计时，总共重组完成的SDU字节数 */
    VOS_UINT32 curPrdRlcSduRxBytes; /* 当前上报周期内接收RLC SDU字节数 */
    VOS_UINT32 rlcCrcErrPduRxNum; /* Crc Error PDU个数：统计的是从查询时刻开始计时，总共从MAC接收上来的CRC错误的PDU个数 */
    VOS_UINT32 rlcInvalidDataPduRxNum; /* 无效数据PDU个数: 统计的是从查询时刻开始计时，总共从MAC接收上来的无效数据PDU个数（其中包括解析错误的数据PDU，重复接收或不在接收窗内的数据PDU） */
    VOS_UINT32 rlcDataPduRxNum; /* 接收到的数据PDU个数：统计的是从查询时刻开始计时,总共从MAC接收上来的有效数据PDU个数（其中不包括重复接收或窗外的数据PDU） */
    VOS_UINT32 rlcCtrlPduRxNum; /* 接收到的控制PDU个数：统计的是从查询时刻开始计时，总共从MAC接收上来的控制PDU个数（其中包括了解析错误的控制PDU） */
    VOS_UINT32 rlcNackedPduNum;     /* 负确认的PDU个数:统计的是从查询时刻开始计时,总共发送到对端负确认的PDU个数 */
    VOS_UINT16 rlcLastRxInseqPduSn; /* [0..4095]，下一个要接收的SN值  */
    VOS_UINT16 rlcHighestPduSn;     /* [0..4095]，最高接收的PDU序号 */
    VOS_UINT32 rsv[4];          /* RTSOck cache-line32字节对齐，防止伪共享 */
} WTTF_MntnRlcAmDlDataTrans;

typedef struct {
    PS_BOOL_ENUM_UINT8        isExist; /* 表示此实体是否存在，0表示不存在，1表示存在 */
    VOS_UINT8                 rbId;
    VOS_UINT8                 rsv[2];
    WTTF_MntnRlcAmUlDataTrans rlcAmUlDataTrans;
    WTTF_MntnRlcAmDlDataTrans rlcAmDlDataTrans;
} WTTF_MntnRlcAmDataTrans;

typedef struct {
    VOS_UINT32 lowRlcPduTxBytes; /* 低位: 发送的RLC PDU字节数：统计的是从查询时刻开始计时，总共发送到MAC层的RLC PDUs的字节数 */
    VOS_UINT32 highRlcPduTxBytes; /* 高位: 发送的RLC PDU字节数：统计的是从查询时刻开始计时，总共发送到MAC层的RLC PDUs的字节数 */
    VOS_UINT32 lowRlcSduTxBytes; /* 低位: 发送的RLC SDU字节数：统计的是从查询时刻开始计时，总共被分段成PDU的SDUs的字节数 */
    VOS_UINT32 highRlcSduTxBytes; /* 高位: 发送的RLC SDU字节数：统计的是从查询时刻开始计时，总共被分段成PDU的SDUs的字节数 */
    VOS_UINT32 rlcPduTxNum;       /* 发送的RLC PDU 个数，统计的是从查询时刻开始计时，总共发送到MAC层的RLC PDUs个数 */
    VOS_UINT16 rlcLastTxPduSn; /* [0..4095]，最近一次发送的SN值:VtUs-1 */
    VOS_UINT8  rsv[2];
    VOS_UINT32 rlcBo; /* 实体BO */
} WTTF_MntnRlcUmUlDataTrans;

typedef struct {
    VOS_UINT32 lowRlcPduRxBytes; /* 低位: 接收的RLC PDU字节数：统计的是从查询时刻开始计时，总共从MAC层接收到的RLC PDUs的字节数，包括解析错误的PDU */
    VOS_UINT32 highRlcPduRxBytes; /* 高位: 接收的RLC PDU字节数：统计的是从查询时刻开始计时，总共从MAC层接收到的RLC PDUs的字节数，包括解析错误的PDU */
    VOS_UINT32 lowRlcSduRxBytes;  /* 低位: 接收到的SDU字节数:统计的是从查询时刻开始计时，总共重组完成的SDU字节数 */
    VOS_UINT32 highRlcSduRxBytes; /* 高位: 接收到的SDU字节数:统计的是从查询时刻开始计时，总共重组完成的SDU字节数 */
    VOS_UINT32 rlcCrcErrPduRxNum; /* Crc Error PDU个数：统计的是从查询时刻开始计时，总共从MAC接收上来的CRC错误的PDU个数 */
    VOS_UINT32 rlcInvalidPduRxNum; /* 无效PDU个数: 统计的是从查询时刻开始计时，总共从MAC接收上来的无效PDU个数（其中包括序号不连续、Li错误、指示前一个PDU有误的PDU） */
    VOS_UINT32 rlcValidPduRxNum; /* 有效PDU个数: 统计的是从查询时刻开始计时，总共从MAC接收上来的有效PDU个数(除CrcError,和invalid PDU外的PDU) */
    VOS_UINT16 rlcLastRxInseqPduSn; /* [0..4095]，最近一次接收到的SN值:VrUs - 1 */
    VOS_UINT8  rsv[2];
} WTTF_MntnRlcUmDlDataTrans;

typedef struct {
    PS_BOOL_ENUM_UINT8        isExist; /* 表示此实体是否存在，0表示不存在，1表示存在 */
    VOS_UINT8                 rbId;
    VOS_UINT8                 rsv[2];
    WTTF_MntnRlcUmUlDataTrans rlcUmUlDataTrans;
    WTTF_MntnRlcUmDlDataTrans rlcUmDlDataTrans;
} WTTF_MntnRlcUmDataTrans;

typedef struct {
    VOS_UINT32 lowRlcSduTxBytes; /* 低位: 发送的RLC PDU字节数：统计的是从查询时刻开始计时，总共发送到MAC层的RLC SDUs的字节数，不是8bit的按8整除并向上取整 */
    VOS_UINT32 highRlcSduTxBytes; /* 高位: 发送的RLC PDU字节数：统计的是从查询时刻开始计时，总共发送到MAC层的RLC SDUs的字节数，不是8bit的按8整除并向上取整 */
    VOS_UINT32 lowRlcDiscardSduTxBytes; /* 低位: Discard RLC SDU字节数：统计的是从查询时刻开始计时，总共丢弃的RLC SDUs的字节数，不是8bit的按8整除并向上取整 */
    VOS_UINT32 hightRlcDiscardSduTxBytes; /* 高位: Discard RLC SDU字节数：统计的是从查询时刻开始计时，总共丢弃的RLC SDUs的字节数，不是8bit的按8整除并向上取整 */
    VOS_UINT32 rlcPduTxNum; /* 发送的RLC PDU个数：统计的是从查询时刻开始计时，总共发送到MAC层的RLC PDUs的个数 */
    VOS_UINT32 rsv[2];
} WTTF_MntnRlcTmUlDataTrans;

typedef struct {
    VOS_UINT32 lowRlcSduRxBytes; /* 低位: 接收到的SDU字节数:统计的是从查询时刻开始计时，总共接收到的SDU字节数，不是8bit的按8整除并向上取整 */
    VOS_UINT32 highRlcSduRxBytes; /* 高位: 接收到的SDU字节数:统计的是从查询时刻开始计时，总共接收到的SDU字节数，不是8bit的按8整除并向上取整 */
    VOS_UINT32 lowRlcPduRxBytes;  /* 低位:接收到的RLC PDU 字节数: 统计的是从查询时刻开始计时，总共从MAC层接收到的RLC PDUs的字节数（其中包括了CrcError的PDU），所指的是MAC接收的吞吐量，不是8bit的按8整除并向上取整 */
    VOS_UINT32 highRlcPduRxBytes; /* 高位:接收到的RLC PDU 字节数: 统计的是从查询时刻开始计时，总共从MAC层接收到的RLC PDUs的字节数（其中包括了CrcError的PDU），所指的是MAC接收的吞吐量，不是8bit的按8整除并向上取整 */
    VOS_UINT32 rlcCrcErrPduRxNum; /* Crc Error PDU个数：统计的是从查询时刻开始计时，总共从MAC接收上来的CRC错误的PDU个数 */
    VOS_UINT32 rlcValidPduRxNum; /* 有效PDU个数: 统计的是从查询时刻开始计时，总共从MAC接收上来的有效PDU个数(除CrcError外的PDU) */
    VOS_UINT32 rsv[2]; /* RTSOck cache-line32字节对齐，防止伪共享 */
} WTTF_MntnRlcTmDlDataTrans;

typedef struct {
    PS_BOOL_ENUM_UINT8        isExist; /* 表示此实体是否存在，0表示不存在，1表示存在 */
    VOS_UINT8                 rbId;
    VOS_UINT8                 rsv[2];
    WTTF_MntnRlcTmUlDataTrans rlcTmUlDataTrans;
    WTTF_MntnRlcTmDlDataTrans rlcTmDlDataTrans;
} WTTF_MntnRlcTmDataTrans;

/* 功能描述: 语音类型 */
enum WTTFMNTN_AmrType {
    WTTFMNTN_AMR_TYPE_AMR2  = 0,
    WTTFMNTN_AMR_TYPE_AMR   = 1,
    WTTFMNTN_AMR_TYPE_AMRWB = 2,
    WTTFMNTN_AMR_TYPE_BUTT
};
typedef VOS_UINT16 WTTFMNTN_AmrTypeUint16;

/* 功能描述: AMR窄带和宽带定义, 参见3GPP TS 26.101 */
enum WTTFMNTN_AmrBandwidthType {
    WTTFMNTN_AMR_BANDWIDTH_TYPE_NB = 0,
    WTTFMNTN_AMR_BANDWIDTH_TYPE_WB = 1,
    WTTFMNTN_AMR_BANDWIDTH_TYPE_BUTT
};
typedef VOS_UINT16 WTTFMNTN_AmrBandwidthTypeUint16;

/* 对应消息:ID_WTTF_OM_RLC_DATA_TRANS_INFO_IND */
typedef struct {
    WTTFMNTN_AmrTypeUint16          amrType;      /* AMR 类型 */
    WTTFMNTN_AmrBandwidthTypeUint16 codecType;    /* 0: NB;  1: WB */
    VOS_UINT16                      frameType;    /* AMR编码帧类型 取值范围:0-15 */
    VOS_UINT8                       rsv[2]; /* 保留位 */
} WTTF_OmMacVocoder;

/* 结构说明: ID_WTTF_OM_RLC_DATA_TRANS_INFO_IND上报查询内容 */
typedef struct {
    VOS_UINT32              curPrdRlcSduTxRate; /* 当前上报周期内发送RLC SDU字节数 */
    VOS_UINT32              curPrdRlcSduRxRate; /* 当前上报周期内发送RLC SDU字节数 */
    WTTF_MntnRlcAmDataTrans rlcAmDataTrans[WTTF_MNTN_MAX_AM_RB_NUM];
    WTTF_MntnRlcUmDataTrans rlcUmDataTrans[WTTF_MNTN_MAX_UM_RB_NUM];
    WTTF_MntnRlcTmDataTrans rlcTmDataTrans[WTTF_MNTN_MAX_TM_RB_NUM];
} WTTF_OmRlcDataTransInd;

/* 结构说明: WTTF MNTN RLC Data Trans Info 对象 */
typedef struct {
    VOS_UINT32             simuClock;
    OM_WttfRlcDataTransReq omRlcReq;
    WTTF_OmRlcDataTransInd rlcOmInd;
} WTTF_MntnRlcDataTransObj;

/* 结构说明: WTTF MNTN MAC Data Info 对象 */
typedef struct {
    VOS_UINT32           simuClock;
    VOS_UINT32           rptPeriod; /* 【1..5】单位：秒,报告周期 */
    TTF_MntnRptCmdUint8  command;
    VOS_UINT8            rsv[3];                 /* 保留位 */
    WTTF_OmMacVocoder rlcUlVocoder; /* 增加上行vocoder 帧类型和帧速率上报，下行帧类型和帧速率由HIFI上报 */
} WTTF_MntnMacDataObj;

/* 对应消息:  ID_OM_IPS_ADVANCED_TRACE_CONFIG_REQ */
typedef struct {
    IPS_MntnBridgeTraceCfg          bridgeArpTraceCfg;   /* 配置捕获网桥消息 */
    IPS_MntnInputTraceCfg           preRoutingTraceCfg;  /* 配置捕获TCP/IP协议栈接收消息 */
    IPS_MntnOutputTraceCfg          postRoutingTraceCfg; /* 配置捕获TCP/IP协议栈发送消息 */
    IPS_MntnLocalTraceCfg           localTraceCfg;       /* 配置捕获TCP/IP协议栈发送消息 */
    WTTF_TraceMsgSimpleAttribUint8  adsIpConfig;
    VOS_UINT8                       rsv[3]; /* 保留位 */
} IPS_MntnTraceCfgReq;

/* 对应消息:  OM_IpsAdvancedTracCfgReq */
typedef struct {
    DIAG_TransMsgHeader diagHdr;
    IPS_MntnTraceCfgReq ipsAdvanceCfgReq;
} OM_IpsAdvancedTraceCfgReq;

/* 对应消息:  ID_IPS_OM_ADVANCED_TRACE_CONFIG_CNF */
typedef struct {
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败，PS_SUCC表示成功，PS_FAIL表示失败 */
} IPS_MntnTraceCfgCnf;

typedef struct {
    DIAG_TransMsgHeader diagHdr;
    IPS_MntnTraceCfgCnf ipsAdvanceCfgCnf;
} IPS_OmAdvancedTraceCfgCnf;

/* 对应消息:  ID_OM_IPS_MNTN_TRAFFIC_CTRL_REQ */
typedef struct {
    DIAG_TransMsgHeader diagHdr;
    VOS_UINT16          trafficCtrlOn;
    VOS_UINT16          speedKBps; /* 单位：KB/s */
} OM_IpsMntnTrafficCtrlReq;

/* 对应消息:  ID_OM_IPS_MNTN_INFO_CONFIG_REQ */
typedef struct {
    IPS_MntnInfoActionUint16 command;
    VOS_UINT16               timeLen; /* 单位：秒 */
} IPS_MntnCfgReq;

typedef struct {
    DIAG_TransMsgHeader diagHdr;
    IPS_MntnCfgReq      ipsMntnCfgReq;
} OM_IpsMntnCfgReq;

/* 对应消息:  ID_IPS_OM_MNTN_INFO_CONFIG_CNF */
typedef struct {
    IPS_MntnResultTypeUint32 result;
    IPS_MntnInfoActionUint16 command;
    VOS_UINT8                rsv[2];
} IPS_MntnCfgCnf;

typedef struct {
    DIAG_TransMsgHeader diagHdr;
    IPS_MntnCfgCnf      ipsMntnCfgCnf;
} IPS_OmMntnCfgCnf;

/* G模可维可测消息 */

/* 对应消息:  GRM_MNTN_GRM_OM_INFO */
typedef struct {
    VOS_UINT32           tlli;     /* 0xffffffff:无效值 */
    GRM_TbfTestModeUint8 testMode; /* GPRS环回时的实际工作模式, 0xff:无效值 */
    VOS_UINT8            tsv[1];
    VOS_UINT16           llcPduNum; /* a模式下待环回的LLC PDU数目, 0xffff:无效值 */
} GRM_MntnCom;

/* 对应消息:  GRM_MNTN_GRM_OM_INFO */
typedef struct {
    GRM_TbfRlcModeUint8        ulRlcMode;     /* 0:GRLC_MODE_AM, 1:GRLC_MODE_UM, 0xff:无效值 */
    GRM_TbfCountDownFlagUint8   countdownFlag; /* 此标志只在非扩展上行TBF下使用，表示:是否进入倒计数过程.VOS_YES:进入了倒计数, VOS_NO:没有进入, 0xff:无效值 */
    GRM_TbfEdgeCodeSchemeUint16 ulCodeType; /* EDGE_CODE_SCHEME_BUTT: 0xffff 无效值 */
    VOS_UINT8                   ulTfi;      /* 0xff: 无效值 */
    GRM_TbfTiUint8              ti;         /* 1: 冲突解决未完成, 0: 冲突解决完成, 0xff: 无效值 */
    VOS_UINT8                   rsv[2];
} GRM_MntnUlTbf;

/* 对应消息:  GRM_MNTN_GRM_OM_INFO */
typedef struct {
    GRM_TbfModeUint8        ulTbfMode; /* 1:Extend 扩展TBF, 0: no-extend 非扩展, 0xff:无效值 */
    VOS_UINT8                bsCvMax;   /* Range: 0 to 15. 0xff:无效值 */
    GRM_TbfTimeSlotFlagUint8 ulTsFlag[GRM_MNTN_TS_TOTAL]; /* 0xff: 无效值 */
    VOS_UINT8                rsv[2];
    GRM_MntnUlTbf            tbf[GRLC_MNTN_MAX_TBF_NUM]; /* Tbf信息 */
} GRM_MntnUlEntity;

/* 下行信息 */

/* 对应消息:  GRM_MNTN_GRM_OM_INFO */
typedef struct {
    VOS_FLOAT afCs1Data;   /* Percentage occupied by each CS_1 time slot. */
    VOS_FLOAT afCs2Data;   /* Percentage occupied by each CS_2 time slot. */
    VOS_FLOAT afCs3Data;   /* Percentage occupied by each CS_3 time slot. */
    VOS_FLOAT afCs4Data;   /* Percentage occupied by each CS_4 time slot. */
    VOS_FLOAT afMcs1Data;  /* Percentage occupied by each MCS_1 time slot. */
    VOS_FLOAT afMcs2Data;  /* Percentage occupied by each MCS_2 time slot. */
    VOS_FLOAT afMcs3Data;  /* Percentage occupied by each MCS_3 time slot. */
    VOS_FLOAT afMcs4Data;  /* Percentage occupied by each MCS_4 time slot. */
    VOS_FLOAT afMcs5Data;  /* Percentage occupied by each MCS_5 time slot. */
    VOS_FLOAT afMcs6Data;  /* Percentage occupied by each MCS_6 time slot. */
    VOS_FLOAT afMcs7Data;  /* Percentage occupied by each MCS_7 time slot. */
    VOS_FLOAT afMcs8Data;  /* Percentage occupied by each MCS_8 time slot. */
    VOS_FLOAT afMcs9Data;  /* Percentage occupied by each MCS_9 time slot. */
    VOS_FLOAT afMcs57Data; /* Percentage occupied by each MCS_5_7 time slot. */
    VOS_FLOAT afMcs69Data; /* Percentage occupied by each MCS_6_9 time slot. */
    VOS_FLOAT afGmskData;  /* Percentage occupied by each GMSK time slot. */
    VOS_FLOAT af8PskData;  /* Percentage occupied by each 8PSK time slot. */
} GRM_MntnCodeModulatePercentage;

/* 对应消息:  GRM_MNTN_GRM_OM_INFO */
typedef struct {
    /*
     * AM:用于可维可测
     * UM模式:最后接收的块的编码方式，提供给UM模式下填充使用，
     * 初始值为EDGE_CODE_SCHEME_BUTT，代表尚未收到正确的下行块
     */
    GRM_TbfEdgeCodeSchemeUint16 edgeCodeScheme;   /* EDGE_CODE_SCHEME_BUTT: 0xffff 无效值 */
    GRM_TbfFinalBlkRxedUint8    finalBlkRxed; /* FBI=1的块是否已经接收到, 0xff: 无效值 */
    GRM_TbfRlcModeUint8        dlRlcMode;    /* 0:AM, 1:UM, 0xff: 无效值 */
    VOS_UINT8                   dlTfi;        /* TFI value 0xff: 无效值 */
    VOS_UINT8                   rsv[3];
    GRM_MntnCodeModulatePercentage dlCodeType[GRM_MNTN_TS_TOTAL];
} GRM_MntnDlTbf;

/* 对应消息:  GRM_MNTN_GRM_OM_INFO */
typedef struct {
    GRM_TbfTimeSlotFlagUint8 dlTsFlag[GRM_MNTN_TS_TOTAL]; /* 下行时隙掩码 0xff: 无效值 */
    GRM_MntnDlTbf            tbf[GRLC_MNTN_MAX_TBF_NUM];  /* 当前只支持单TBF */
} GRM_MntnDlEntity;

/* 上报OM的总体结构信息 */

/* 对应消息:  GRM_MNTN_GRM_OM_INFO */
typedef struct {
    GRM_MntnCom      grmMntnComInfo; /* 公共信息 */
    GRM_MntnUlEntity grmMntnUlInfo;  /* 上行信息 */
    GRM_MntnDlEntity grmMntnDlInfo;  /* 下行信息 */
} GRM_MntnGrmOm;

/* 对应消息:  ID_GRM_MNTN_OM_GRM_INFO_CFG_REQ */
typedef struct {
    DIAG_TransMsgHeader        diagHdr; /* 透传消息头 */
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT16                 rptPeriod; /* 单位: s */
} GRM_MntnOmCfgReq;

/*
 * 结构名:  GRM_MntnOmCfgCnf
 * 对应消息:  ID_GRM_MNTN_GRM_OM_INFO_CFG_CNF
 */
typedef struct {
    DIAG_TransMsgHeader        diagHdr; /* 透传消息头 */
    GTTF_ResultTypeUint32      result;
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT16                 rptPeriod; /* 单位: s */
} GRM_MntnOmCfgCnf;

/* 对应消息:  ID_GRM_MNTN_OM_GRM_THROUGHPUT_CFG_REQ */
typedef struct {
    DIAG_TransMsgHeader        diagHdr;
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT8                  rsv[2];
} GRM_MntnOmThroughputCfgReq;

/* 对应消息:  ID_GRM_MNTN_GRM_OM_THROUGHPUT_CFG_CNF */
typedef struct {
    DIAG_TransMsgHeader        diagHdr;
    GTTF_ResultTypeUint32      result;
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT8                  rsv[2];
} GRM_MntnThroughputCfgCnf;

typedef WTTF_MntnTraceDataCfg GTTF_MntnLlGrmTraceCfg;
typedef WTTF_MntnTraceDataCfg GTTF_MntnSnLlTraceCfg;
typedef WTTF_MntnTraceDataCfg GTTF_MntnRabmSnTraceCfg;

typedef struct {
    GTTF_MntnLlGrmTraceCfg  llGrmCfg;
    GTTF_MntnSnLlTraceCfg   snLlCfg;
    GTTF_MntnRabmSnTraceCfg rabmSnCfg;
} GTTF_TraceMsgCfg;

/* 对应消息:  ID_GRM_MNTN_OM_GTTF_ADV_TRACE_SWITCH_REQ */
typedef struct {
    DIAG_TransMsgHeader      transHdr;
    TOOL_GttfCfgSwitchUint32 l2CfgSwitch; /* TRACE开关0:关闭1:打开 */
    GTTF_TraceMsgCfg         advancedConfig;
} GRM_MntnOmGttfAdvTraceSwitchReq;

/* 对应消息:  ID_GRM_MNTN_GTTF_OM_ADV_TRACE_SWITCH_CNF */
typedef struct {
    DIAG_TransMsgHeader      transHdr;
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败 */
} GRM_MntnGttfOmAdvTraceSwitchCnf;

/* CP上关于IP报文、PPP帧之类的维测 */
typedef struct {
    WTTF_TraceMsgSimpleAttribUint8  vispLogOutConfig;          /* VISP日志总开关 */
    IPS_MntnCcoreTcpipLogLevelUint8 vispRunLogLevelConfig;     /* VISP RUN日志级别配置 */
    WTTF_TraceMsgSimpleAttribUint8  vispUnEncryptPktCapConfig; /* VISP非加密报文抓包开关 */
    WTTF_TraceMsgSimpleAttribUint8  vispEncryptPktCapConfig;   /* VISP加密报文抓包开关 */
} IPS_MntnCcoreVispLogCfgReq;

/* 对应消息:  ID_OM_IPS_CCORE_ADVANCED_TRACE_CONFIG_REQ */
typedef struct {
    IPS_MntnCcoreBridgeTraceCfg bridgeTraceCfg; /* 配置捕获网桥消息 */
    IPS_MntnCcoreInputTraceCfg  inputTraceCfg;  /* 配置捕获TCP/IP协议栈接收消息 */
    IPS_MntnCcoreOutputTraceCfg outputTraceCfg; /* 配置捕获TCP/IP协议栈发送消息 */
    IPS_MntnCcoreVispLogCfgReq  vispLogConfig;
} IPS_MntnCcoreTraceCfgReq;

typedef struct {
    DIAG_TransMsgHeader      diagHdr;
    IPS_MntnCcoreTraceCfgReq ipsCcoreCfgReq;
} OM_IpsCcoreAdvTraceCfgReq;

/* 对应消息:  ID_IPS_CCORE_OM_ADVANCED_TRACE_CONFIG_CNF */
typedef struct {
    PS_RSLT_CODE_ENUM_UINT32 result; /* 返回查询结构成功或失败，PS_SUCC表示成功，PS_FAIL表示失败 */
} IPS_MntnCcoreTraceCfgCnf;

typedef struct {
    DIAG_TransMsgHeader      diagHdr;
    IPS_MntnCcoreTraceCfgCnf ipsCcoreCfgCnf;
} IPS_CcoreOmAdvTraceCfgCnf;

/* 对应消息:  ID_OM_IPS_CCORE_MNTN_INFO_CONFIG_REQ */
typedef struct {
    IPS_MntnCcoreActionUint16 command;
    VOS_UINT16                timeLen; /* 单位：秒 */
} IPS_MntnCcoreCfgReq;

typedef struct {
    DIAG_TransMsgHeader diagHdr;
    IPS_MntnCcoreCfgReq ipsCcoreMntnCfgReq;
} OM_IpsCcoreCfgReq;

/* 对应消息:  ID_IPS_CCORE_OM_MNTN_INFO_CONFIG_CNF */
typedef struct {
    IPS_MntnCcoreResultTypeUint32 result;
    IPS_MntnCcoreActionUint16     command;
    VOS_UINT8                     rsv[2];
} IPS_MntnCcoreCfgCnf;

typedef struct {
    DIAG_TransMsgHeader diagHdr;
    IPS_MntnCcoreCfgCnf ipsCcoreMntnCfgCnf;
} IPS_CcoreOmMntnCfgCnf;

/* 对应消息:  ID_SN_MNTN_OM_THROUGHPUT_CFG_REQ */
typedef struct {
    DIAG_TransMsgHeader        diagHdr;
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT8                  rsv[2];
} SNDCP_MntnOmThroughputCfgReq;

/* 对应消息:  ID_SN_MNTN_OM_THROUGHPUT_CFG_CNF */
typedef struct {
    DIAG_TransMsgHeader        diagHdr;
    VOS_UINT32                 result;
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT8                  rsv[2];
} SNDCP_MntnOmThroughputCfgCnf;

/*
 * SNDCP可维可测消息实体
 * 对应消息:  ID_SN_MNTN_OM_THROUGHPUT_INFO
 */
typedef struct {
    VOS_UINT32 ulThroughput; /* 上行吞吐率信息 */
    VOS_UINT32 dlThroughput; /* 下行吞吐率信息 */
} SNDCP_MntnOmThroughputInfo;

/* 对应消息:  ID_LL_MNTN_OM_THROUGHPUT_CFG_REQ */
typedef struct {
    DIAG_TransMsgHeader        diagHdr;
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT8                  rsv[2];
} LLC_MntnOmThroughputCfgReq;

/* 对应消息:  ID_LL_MNTN_OM_THROUGHPUT_CFG_CNF */
typedef struct {
    DIAG_TransMsgHeader        diagHdr;
    VOS_UINT32                 result;
    GTTF_MntnOmRptActionUint16 rptAction;
    VOS_UINT8                  rsv[2];
} LLC_MntnOmThroughputCfgCnf;

/*
 * LLC可维可测消息实体
 * 对应消息:  ID_LL_MNTN_OM_THROUGHPUT_INFO
 */
typedef struct {
    VOS_UINT32 ulThroughput;       /* 上行吞吐率信息 */
    VOS_UINT32 dlThroughput;       /* 下行吞吐率信息 */
    VOS_UINT32 ulRetransPduNumber; /* 确认模式上行重传PDU个数 */
    VOS_UINT32 ulTotalNumber;      /* 确认模式上行发送总的PDU个数 */
    VOS_UINT32 dlRetransPduNumber; /* 确认模式下行重传PDU个数 */
    VOS_UINT32 dlTotalNumber;      /* 确认模式下行发送总的PDU个数 */
} LLC_MntnOmThroughputInfo;


/* 对应消息:  pppc接口 */
typedef struct {
    VOS_UINT32 succCnt;
    VOS_UINT32 failCnt;
    VOS_UINT32 nameOrPwdAuthFailCnt;
    VOS_UINT32 invalidIpAddrCnt;
    VOS_UINT32 invalidDnsAddrCnt;
    VOS_UINT32 otherFail;
} PPP_MntnHrpdStatisticsReport;


/* 对应消息:  ID_OM_PPP_MNTN_CONFIG_REQ */
typedef struct {
    DIAG_TransMsgHeader transHdr;

    TTF_MntnCommRptCmdUint8 command; /* 开始、停止或清除 */
    VOS_UINT8                        reserve[3];
} PPP_MntnOmConfigReq;

/*
 * 对应消息:  ID_PPP_OM_MNTN_STATISTIC_INFO
 * 说明:  主动上报消息, 在API调用时用指针指向上报内容即可
 * 结构说明: 1X公共信道的统计值
 */
typedef struct {
    VOS_UINT16                                primId;
    VOS_UINT16                                toolId;
    PPP_MntnHrpdStatisticsReport pppcStatisticInfo;
} PPP_MntnOmStatisticsInd;

/* 对应消息:  ID_CTTF_OM_PPP_MNTN_CONFIG_CNF */
typedef struct {
    DIAG_TransMsgHeader diagHdr;
    PS_RSLT_CODE_ENUM_UINT32 rslt;   /* 配置成功或失败 */
    VOS_UINT32               reason; /* 带具体失败的原因 */
} PPP_MntnOmConfigCnf;


#ifndef COMM_ITT
extern VOS_VOID TTF_TraceModuleSet(VOS_UINT8* moduleId);
#endif
extern VOS_VOID WTTF_TraceModuleCfgReq(VOS_VOID* msg);
extern VOS_VOID WMAC_R99MailBoxReadForIpcHalt(MODEM_ID_ENUM_UINT16 modemId);
extern VOS_VOID WTTF_R99MailboxInitForCbt(MODEM_ID_ENUM_UINT16 modemId);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

