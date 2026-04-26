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
#ifndef _RNIC_CTX_H_
#define _RNIC_CTX_H_

#include "vos.h"
#include "PsLogdef.h"
#include "PsTypeDef.h"
#include "ps_common_def.h"
#include "ps_iface_global_def.h"
#include "imm_interface.h"
#include "imsa_rnic_interface.h"
#include "at_rnic_interface.h"
#include "rnic_timer_mgmt.h"
#include "mdrv_nvim.h"
#include "acore_nv_stru_gucnas.h"
#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#include "mdrv_eipf.h"
#include "ads_dev_i.h"
#else
#include "ads_device_interface.h"
#endif
#include <linux/pm_wakeup.h>
#include <linux/bitops.h>
#include "nv_stru_gucnas.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#define RNIC_NET_ID_MAX_NUM (RNIC_DEV_ID_BUTT)
#define RNIC_3GPP_NET_ID_MAX_NUM (RNIC_DEV_ID_DATA_MAX)
#define RNIC_MODEM_ID_MAX_NUM (MODEM_ID_BUTT)
#define RNIC_3GPP_NET_MAX_NUM_MASK GENMASK(RNIC_3GPP_NET_ID_MAX_NUM, 0)

#define RNIC_IPV4_VERSION (4)       /* IP头部中IP V4版本号 */
#define RNIC_IPV6_VERSION (6)       /* IP头部中IP V6版本号 */
#define RNIC_MAX_IPV6_ADDR_LEN (16) /* IPV6地址长度 */

#define RNIC_MAX_DSFLOW_BYTE (0xFFFFFFFF) /* RNIC最大流量值 */

#define RNIC_DIAL_DEMA_IDLE_TIME (600)

/* RabId的高两位表示ModemId, 00表示Modem0, 01表示Modem1, 10表示Modem1 */
#define RNIC_RABID_TAKE_MODEM_1_MASK (0x40) /* Rabid携带Modem1的掩码 */
#define RNIC_RABID_TAKE_MODEM_2_MASK (0x80) /* Rabid携带Modem2的掩码 */
#define RNIC_RABID_UNTAKE_MODEM_MASK (0x3F) /* Rabid去除Modem的掩码 */

/* 最大RABID值 */
#define RNIC_RAB_ID_MAX_NUM (11)
#define RNIC_RAB_ID_OFFSET (5)
/* Rab Id的最小值 */
#define RNIC_RAB_ID_MIN (5)
/* Rab Id的最大值 */
#define RNIC_RAB_ID_MAX (15)
#define RNIC_RAB_ID_INVALID (0xFF)

/* NAPI 一次Poll轮询的默认最大包个数 */
#define RNIC_NAPI_POLL_DEFAULT_WEIGHT (16)
/* NAPI 一次Poll轮询的协议栈支持的最大包个数 */
#define RNIC_NAPI_POLL_MAX_WEIGHT (64)
#define RNIC_POLL_QUEUE_DEFAULT_MAX_LEN (15000) /* NAPI Poll队列的最大长度 */

#define RNIC_INVALID_IFACE_ID (0xFF)
#define RNIC_INVALID_PDU_SESSION_ID (0xFF)
#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#define RNIC_INVALID_FC_HEAD EIPF_FCHEAD_BOTTOM
#endif
/* RNIC 接收NV消息最大次数 */
#define RNIC_NV_CFG_MAX_NUM 1

#if !defined(RNIC_ARRAY_SIZE)
#define RNIC_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#define RNIC_RAB_ID_IS_VALID(ucRabId) (((ucRabId) >= RNIC_RAB_ID_MIN) && ((ucRabId) <= RNIC_RAB_ID_MAX))

#define RNIC_RMNET_IS_VALID(RmNetId) ((RmNetId) < RNIC_DEV_ID_BUTT)
#define RNIC_IFACE_ID_IS_VALID(ucIfaceId) ((ucIfaceId) < PS_IFACE_ID_BUTT)
#define RNIC_MODEM_ID_IS_VALID(enModemId) ((enModemId) < MODEM_ID_BUTT)

#define RNIC_BIT8_MASK(bit) ((VOS_UINT8)(1 << (bit)))
#define RNIC_BIT8_SET(val, bit) ((val) | RNIC_BIT8_MASK(bit))
#define RNIC_BIT8_CLR(val, bit) ((val) & ~RNIC_BIT8_MASK(bit))
#define RNIC_BIT8_IS_SET(val, bit) ((val)&RNIC_BIT8_MASK(bit))

#define RNIC_BIT32_MASK(bit) ((VOS_UINT32)(1 << (bit)))
#define RNIC_BIT32_SET(val, bit) ((val) = ((val) | RNIC_BIT32_MASK(bit)))
#define RNIC_BIT32_CLR(val, bit) ((val) = ((val) & ~RNIC_BIT32_MASK(bit)))
#define RNIC_BIT32_IS_SET(val, bit) ((val)&RNIC_BIT32_MASK(bit))

#if (FEATURE_ON == FEATURE_MULTI_MODEM)
#define RNIC_RMNET_R_IS_EMC_BEAR(RmNetId) \
    (((RmNetId) == RNIC_DEV_ID_RMNET_R_IMS01) || ((RmNetId) == RNIC_DEV_ID_RMNET_R_IMS11))
#else
#define RNIC_RMNET_R_IS_EMC_BEAR(RmNetId) ((RmNetId) == RNIC_DEV_ID_RMNET_R_IMS01)
#endif

/* 获取RNIC上下文地址 */
#define RNIC_GET_RNIC_CTX_ADR() (&g_rnicCtx)
/* 获取指定网卡的上下文地址 */
#define RNIC_GET_IFACE_CTX_ADR(indexNum) (&(g_rnicCtx.ifaceCtx[indexNum]))
/* 获取指定网卡的PDP上下文地址 */
#define RNIC_GET_IFACE_PDN_INFO_ADR(indexNum) (&(g_rnicCtx.ifaceCtx[indexNum].psIfaceInfo))
/* 获取IPV4类型PDN激活状态 */
#define RNIC_GET_IFACE_PDN_IPV4_ACT_STATE(indexNum) (g_rnicCtx.ifaceCtx[indexNum].psIfaceInfo.opIpv4Act)
/* 获取IPV6类型PDN激活状态 */
#define RNIC_GET_IFACE_PDN_IPV6_ACT_STATE(indexNum) (g_rnicCtx.ifaceCtx[indexNum].psIfaceInfo.opIpv6Act)

/* 获取当前接收报文数 */
#define RNIC_GET_IFACE_PERIOD_RECV_PKT(indexNum) (g_rnicCtx.ifaceCtx[indexNum].dsFlowStats.periodRecvPktNum)
/* 获取当前发送报文数 */
#define RNIC_GET_IFACE_PERIOD_SEND_PKT(indexNum) (g_rnicCtx.ifaceCtx[indexNum].dsFlowStats.periodSendPktNum)
/* 设置当前发送报文数 */
#define RNIC_SET_IFACE_PERIOD_SEND_PKT(indexNum, val) \
    (g_rnicCtx.ifaceCtx[indexNum].dsFlowStats.periodSendPktNum = (val))
/* 清除当前接收报文数 */
#define RNIC_CLEAN_IFACE_PERIOD_RECV_PKT(indexNum) (g_rnicCtx.ifaceCtx[indexNum].dsFlowStats.periodRecvPktNum = 0)
/* 清除当前发送报文数 */
#define RNIC_CLEAN_IFACE_PERIOD_SEND_PKT(indexNum) (g_rnicCtx.ifaceCtx[indexNum].dsFlowStats.periodSendPktNum = 0)

/* 获取指定网卡NAPI特性开关 */
#define RNIC_GET_NAPI_FEATURE(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiEnable)
/* 获取指定网卡NAPI特性开关 */
#define RNIC_GET_GRO_FEATURE(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.groEnable)
/* 获取网卡NAPI weight值调整模式 */
#define RNIC_GET_NAPI_WEIGHT_ADJ_MODE(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiWeightAdjMode)
/* 获取网卡NAPI weight值 */
#define RNIC_GET_NAPI_WEIGHT(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiPollWeight)
/* 获取网卡NAPI Poll队列长度最大值 */
#define RNIC_GET_NAPI_POLL_QUE_MAX_LEN(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiPollQueMaxLen)
/* 获取指定网卡的NAPI Weight动态调整配置地址 */
#define RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum) \
    (&(g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiWeightDynamicAdjCfg))
/* 获取网卡NAPI下行每秒收包个数档位值1 */
#define RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL1(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel1)
/* 获取网卡NAPI下行每秒收包个数档位值2 */
#define RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL2(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel2)
/* 获取网卡NAPI下行每秒收包个数档位值3 */
#define RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL3(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel3)
/* 获取网卡NAPI下行每秒收包个数档位值4 */
#define RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL4(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel4)

/* 获取网卡NAPI weight档位值1 */
#define RNIC_GET_NAPI_WEIGHT_LEVEL1(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel1)
/* 获取网卡NAPI weight档位值2 */
#define RNIC_GET_NAPI_WEIGHT_LEVEL2(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel2)
/* 获取网卡NAPI weight档位值3 */
#define RNIC_GET_NAPI_WEIGHT_LEVEL3(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel3)
/* 获取网卡NAPI weight档位值4 */
#define RNIC_GET_NAPI_WEIGHT_LEVEL4(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel4)
/* 获取网卡NAPI weight档位值5 */
#define RNIC_GET_NAPI_WEIGHT_LEVEL5(indexNum) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel5)

/* 设置网卡NAPI特性开关 */
#define RNIC_SET_NAPI_FEATURE(indexNum, val) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiEnable = (val))
/* 设置网卡NAPI特性开关 */
#define RNIC_SET_GRO_FEATURE(indexNum, val) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.groEnable = (val))
/* 设置网卡NAPI weight值调整模式 */
#define RNIC_SET_NAPI_WEIGHT_ADJ_MODE(indexNum, val) \
    (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiWeightAdjMode = (val))
/* 设置网卡NAPI weight值 */
#define RNIC_SET_NAPI_WEIGHT(indexNum, val) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiPollWeight = (val))
/* 设置网卡NAPI Poll队列长度最大值 */
#define RNIC_SET_NAPI_POLL_QUE_MAX_LEN(indexNum, val) \
    (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiCfg.napiPollQueMaxLen = (val))

/* 设置网卡NAPI下行每秒收包个数档位值1 */
#define RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL1(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel1 = (val))
/* 设置网卡NAPI下行每秒收包个数档位值2 */
#define RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL2(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel2 = (val))
/* 设置网卡NAPI下行每秒收包个数档位值3 */
#define RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL3(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel3 = (val))
/* 设置网卡NAPI下行每秒收包个数档位值4 */
#define RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL4(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel4 = (val))
/* 设置网卡NAPI下行每秒收包个数档位值5 */
#define RNIC_SET_DL_PKT_NUM_PER_SEC_LEVEL5(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->dlPktNumPerSecLevel.dlPktNumPerSecLevel5 = (val))

/* 设置网卡NAPI weight档位值1 */
#define RNIC_SET_NAPI_WEIGHT_LEVEL1(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel1 = (val))
/* 设置网卡NAPI weight档位值2 */
#define RNIC_SET_NAPI_WEIGHT_LEVEL2(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel2 = (val))
/* 设置网卡NAPI weight档位值3 */
#define RNIC_SET_NAPI_WEIGHT_LEVEL3(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel3 = (val))
/* 设置网卡NAPI weight档位值4 */
#define RNIC_SET_NAPI_WEIGHT_LEVEL4(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel4 = (val))
/* 设置网卡NAPI weight档位值5 */
#define RNIC_SET_NAPI_WEIGHT_LEVEL5(indexNum, val) \
    (RNIC_GET_NAPI_DYNAMIC_CFG_ADR(indexNum)->napiWeightLevel.napiWeightLevel5 = (val))

/* 获取RNIC上下文地址 */
#define RNIC_GET_RNIC_TIMER_ADR() (g_rnicCtx.timerCtx)

#if (defined(CONFIG_BALONG_SPE))
/* 获取指定网卡的SPE配置地址 */
#define RNIC_GET_SPE_CFG_ADR(indexNum) (&(g_rnicCtx.ifaceCtx[indexNum].featureCfg.stSpeCfg))
#endif

/* 按需拨号节点操作 */
#define RNIC_GET_DIAL_MODE_ADR() (&(g_rnicCtx.dialMode))
#define RNIC_GET_PROC_DIAL_MODE() (g_rnicCtx.dialMode.dialMode)
#define RNIC_SET_PROC_DIAL_MODE(val) (g_rnicCtx.dialMode.dialMode = (val))
#define RNIC_GET_PROC_IDLE_TIME() (g_rnicCtx.dialMode.idleTime)
#define RNIC_SET_PROC_IDLE_TIME(val) (g_rnicCtx.dialMode.idleTime = (val))
#define RNIC_GET_PROC_EVENT_REPORT() (g_rnicCtx.dialMode.eventReportFlag)
#define RNIC_SET_PROC_EVENT_REPORT(val) (g_rnicCtx.dialMode.eventReportFlag = (val))

/* 获取当前的IPF模式 */
#define RNIC_GET_IPF_MODE() (g_rnicCtx.ipfMode)
/* 获取复位信号 */
#define RNIC_GET_RESET_SEM() (g_rnicCtx.resetSem)

/* 按需断开计数操作 */
#define RNIC_GET_TI_DIALDOWN_EXP_CONT() (g_rnicCtx.tiDialDownExpCount)
#define RNIC_CLEAR_TI_DIALDOWN_EXP_CONT() (g_rnicCtx.tiDialDownExpCount = 0)
#define RNIC_ADD_TI_DIALDOWN_EXP_CONT() (g_rnicCtx.tiDialDownExpCount++)

/* Tethering操作 */
#define RNIC_GET_TETHER_ORIG_GRO_FEATURE() (g_rnicCtx.tetherInfo.origGroEnable)
#define RNIC_SET_TETHER_ORIG_GRO_FEATURE(val) (g_rnicCtx.tetherInfo.origGroEnable = (val))

/* 获取指定网卡NAPI LB配置地址 */
#define RNIC_GET_NAPI_LB_CFG_ADR(indexNum) (&(g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg))

/* 获取指定网卡NAPI特性开关 */
#define RNIC_GET_NAPI_LB_FEATURE(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbEnable)
/* 获取指定网卡NAPI负载均衡CPU掩码 */
#define RNIC_GET_NAPI_LB_CPUMASK(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbCpumask)
/* 获取指定网卡NAPI负载均衡当前档位 */
#define RNIC_GET_NAPI_LB_CUR_LEVEL(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbCurLevel)
/* 获取指定网卡NAPI负载均衡档位对应的pps */
#define RNIC_GET_NAPI_LB_LEVEL_PPS(indexNum, level) \
    (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbLevelCfg[(level)].pps)
/* 获取指定网卡NAPI负载均衡档位配置 */
#define RNIC_GET_NAPI_LB_LEVEL_CFG(indexNum) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbLevelCfg)

/* 设置网卡NAPI特性开关 */
#define RNIC_SET_NAPI_LB_FEATURE(indexNum, val) (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbEnable = (val))
/* 设置指定网卡NAPI负载均衡CPU掩码 */
#define RNIC_SET_NAPI_LB_CPUMASK(indexNum, val) \
    (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbCpumask = (val))
/* 设置指定网卡NAPI负载均衡当前档位 */
#define RNIC_SET_NAPI_LB_CUR_LEVEL(indexNum, val) \
    (g_rnicCtx.ifaceCtx[indexNum].featureCfg.napiLbCfg.napiLbCurLevel = (val))


enum RNIC_NetcardStatusType {
    RNIC_NETCARD_STATUS_CLOSED, /* RNIC为关闭状态 */
    RNIC_NETCARD_STATUS_OPENED, /* RNIC为打开状态 */
    RNIC_NETCARD_STATUS_BUTT
};
typedef VOS_UINT8 RNIC_NetcardStatusUint8;


enum RNIC_FlowCtrlStatus {
    RNIC_FLOW_CTRL_STATUS_STOP  = 0x00, /* 流控停止 */
    RNIC_FLOW_CTRL_STATUS_START = 0x01, /* 流控启动 */
    RNIC_FLOW_CTRL_STATUS_BUTT  = 0xFF
};
typedef VOS_UINT32 RNIC_FlowCtrlStatusUint32;


enum RNIC_IPF_Mode {
    RNIC_IPF_MODE_INT  = 0x00, /* 中断上下文 */
    RNIC_IPF_MODE_THRD = 0x01, /* 线程上下文 */
    RNIC_IPF_MODE_BUTT
};
typedef VOS_UINT8 RNIC_IPF_ModeUint8;


enum RNIC_FeatureEnable {
    RNIC_FEATURE_OFF = 0x00,
    RNIC_FEATURE_ON  = 0x01,
    RNIC_FEATURE_BUTT
};
typedef VOS_UINT8 RNIC_FeatureEnableUint8;


enum RNIC_DialMode {
    RNIC_DIAL_MODE_MANUAL,            /* Manual dial mode */
    RNIC_DIAL_MODE_DEMAND_CONNECT,    /* Demand dial mode */
    RNIC_DIAL_MODE_DEMAND_DISCONNECT, /* Demand dial mode */
    RNIC_DIAL_MODE_BUTT
};
typedef VOS_UINT32 RNIC_DialModeUint32;

enum RNIC_DialEventReportFlag {
    RNIC_FORBID_EVENT_REPORT = 0x0000, /* 不给应用上报 */
    RNIC_ALLOW_EVENT_REPORT  = 0X0001, /* 允许给应用上报 */

    RNIC_DIAL_EVENT_REPORT_FLAG_BUTT
};
typedef VOS_UINT32 RNIC_DialEventReportFlagUint32;


enum RNIC_PS_RatType {
    RNIC_PS_RAT_TYPE_3GPP  = 0x00,
    RNIC_PS_RAT_TYPE_IWLAN = 0x01,
    RNIC_PS_RAT_TYPE_BUTT
};
typedef VOS_UINT8 RNIC_PS_RatTypeUint8;

typedef VOS_VOID (*RNIC_SPE_RX_PUSH_CB)(VOS_VOID);
typedef VOS_INT (*RNIC_SPE_MEM_RECYCLE_CB)(IMM_Zc *pstImmZc);

/*
 * 结构说明: 流量统计结构
 */
typedef struct {
    /* 当前下行速率，保存PDP激活后1秒的速率，去激活后清空 */
    VOS_UINT32 currentRecvRate;
    /* 当前上行速率，保存PDP激活后1秒的速率，去激活后清空 */
    VOS_UINT32 currentSendRate;

    /* 下行收到数据包个数,统计一个拨号断开定时器周期内收到的个数，超时后清空 */
    VOS_UINT32 periodRecvPktNum;
    /* 上行发送有效个数,统计一个拨号断开定时器周期内收到的个数，超时后清空 */
    VOS_UINT32 periodSendPktNum;

    /* 下行单位时间内收到的非TCP包个数 */
    VOS_UINT32 periodRecvNonTcpPktNum;
    /* 下行单位时间内收到的TCP包个数 */
    VOS_UINT32 periodRecvTcpPktNum;
} RNIC_DSFLOW_Stats;


typedef struct {
    VOS_UINT8                  rmnetName[RNIC_RMNET_NAME_MAX_LEN]; /* Rmnet网卡名 */
    AT_RNIC_UsbTetherConnUint8 tetherConnStat;                     /* Tethering连接状态 */
    VOS_UINT8                  origGroEnable;                      /* Tethering连接状态 */
    VOS_UINT8                  rsv[6];                             /* 保留位 */
} RNIC_TetherInfo;


typedef struct {
    VOS_UINT8     feature;                           /* 拥塞控制特性开关 */
    VOS_UINT8     rhcLevel;                          /* 当前接收高速控制档位 */
    VOS_UINT8     qccEnable;                         /* NAPI队列拥塞控制功能开关 */
    VOS_UINT8     napiWtExpEnable;                   /* NAPI队列拥塞控制Weight扩张功能开关 */
    VOS_UINT8     ddrReqEnable;                      /* 根据NAPI队列拥塞程度给DDR投票功能开关 */
    VOS_UINT8     qccTimeout;                        /* NAPI队列拥塞控制持续时长 */
    VOS_UINT8     napiPollMax;                       /* NAPI poll最大连续执行次数 */
    VOS_UINT8     rpsMaskEnable;                     /* RPS Mask动态更新功能开关 */
    VOS_INT32     ddrMidBd;                          /* 中档位DDR带宽 */
    VOS_INT32     ddrHighBd;                         /* 高档位DDR带宽 */
    RNIC_RhcLevel levelCfg[RNIC_NVIM_RHC_MAX_LEVEL]; /* 接收高速控制档位配置 */
} RNIC_RHC_Cfg;


typedef struct {
    RNIC_DialModeUint32            dialMode;        /* Dial模式 */
    VOS_UINT32                     idleTime;        /* 定时器长度，单位为秒 */
    RNIC_DialEventReportFlagUint32 eventReportFlag; /* 是否给应用上报标识 */
    VOS_UINT32                     reserved;
} RNIC_DialModePara;


typedef struct {
    VOS_UINT8 napiEnable; /* NAPI开启 */
    /* NAPI Weight调整模式, 0: 静态模式，1：动态调整模式 */
    NAPI_WEIGHT_AdjModeUint8 napiWeightAdjMode;
    /* RNIC网卡NAPI方式一次poll的最大报文数 */
    VOS_UINT8                     napiPollWeight;
    VOS_UINT8                     groEnable;               /* GRO开启 */
    VOS_UINT16                    napiPollQueMaxLen;       /* Napi poll队列最大支持长度 */
    VOS_UINT8                     reserved[2];             /* 保留位 */
    RNIC_NAPI_WeightDynamicAdjCfg napiWeightDynamicAdjCfg; /* Napi Weight动态调整配置 */

} RNIC_NapiCfg;


typedef struct {
    VOS_UINT8            napiLbEnable;                                /* NAPI负载均衡功能开关 */
    VOS_UINT8            napiLbCurLevel;                              /* 当前负载均衡档位 */
    VOS_UINT16           napiLbCpumask;                               /* 参与Napi负载均衡的CPU掩码 */
    VOS_UINT8            reserved0;                                   /* 保留位 */
    VOS_UINT8            reserved1;                                   /* 保留位 */
    VOS_UINT8            reserved2;                                   /* 保留位 */
    VOS_UINT8            reserved3;                                   /* 保留位 */
    VOS_UINT32           rsv0;                                        /* 保留位 */
    VOS_UINT32           rsv1;                                        /* 保留位 */
    RNIC_NAPI_LbLevelCfg napiLbLevelCfg[RNIC_NVIM_NAPI_LB_MAX_LEVEL]; /* Napi负载均衡档位配置 */

} RNIC_NapiLbCfg;

#if (defined(CONFIG_BALONG_SPE))

typedef struct {
    VOS_UINT32              tdDepth;           /* TD深度 */
    VOS_UINT32              rdDepth;           /* RD深度 */
    RNIC_SPE_RX_PUSH_CB     speRxPushFunc;     /* SPE上行中断完成回调 */
    RNIC_SPE_MEM_RECYCLE_CB speMemRecylceFunc; /* SPE内存回收回调 */

} RNIC_SpeCfg;
#endif


typedef struct {
    RNIC_NapiCfg   napiCfg;   /* NAPI,GRO特性配置 */
    RNIC_NapiLbCfg napiLbCfg; /* NAPI软中断负载均衡配置 */

#if (defined(CONFIG_BALONG_SPE))
    RNIC_SpeCfg stSpeCfg; /* SPE配置 */
#endif
} RNIC_FeatureCfg;


typedef struct {
    VOS_UINT32 opIpv4Act : 1;
    VOS_UINT32 opIpv6Act : 1;
    VOS_UINT32 opEthAct : 1;
    VOS_UINT32 opSpare : 29;

    VOS_UINT8  ipv4RabId;
    VOS_UINT8  ipv4ExRabId; /* 扩展承载:Modem ID + Rab ID */
    VOS_UINT8  reserved1[2];
    VOS_UINT32 ipv4Addr; /* IPv4地址 */

    VOS_UINT8 ipv6RabId;
    VOS_UINT8 ipv6ExRabId; /* 扩展承载:Modem ID + Rab ID */

    VOS_UINT8  ethRabId;
    VOS_UINT8  ethExRabId;
    /* 从AT带来的IPV6地址长度，不包括":" */
    VOS_UINT8 ipv6Addr[RNIC_MAX_IPV6_ADDR_LEN];

    ModemIdUint16        modemId; /* 网卡归属哪个modem */
    RNIC_PS_RatTypeUint8 ratType; /* ps注册域 */
    VOS_UINT8            reserved3[5];

    IMM_ZcHeader imsQue; /* IMS上行数据发送缓存队列 */

} RNIC_PS_IfaceInfo;


typedef struct {
    RNIC_DEV_ID_ENUM_UINT8 rmNetId; /* 网卡设备对应的网卡ID */
    PS_IFACE_IdUint8       ifaceId;
    VOS_UINT8              reserved[6];
    RNIC_PS_IfaceInfo      psIfaceInfo; /* 网卡设备对应的PDN信息 */
    RNIC_FeatureCfg        featureCfg;  /* RNIC特性配置 */
    RNIC_DSFLOW_Stats      dsFlowStats; /* 流量信息 */

} RNIC_IFACE_Ctx;


typedef struct {
    /* RNIC网卡设备上下文 */
    RNIC_IFACE_Ctx ifaceCtx[RNIC_NET_ID_MAX_NUM];

    /* RNIC定时器上下文 */
    RNIC_TIMER_Ctx timerCtx[RNIC_MAX_TIMER_NUM];

    /* RNIC公共上下文 */
    /* 拨号模式 */
    RNIC_DialModePara dialMode;
    /* 拨号断开定时器超时次数参数统计 */
    VOS_UINT32 tiDialDownExpCount;
    /* IPF处理ADS下行数据的模式, 0: 中断上下文(默认)，1：线程上下文 */
    ADS_IpfModeUint8 ipfMode;
    VOS_UINT8 devReady;
    VOS_UINT8 rsv[2];
    /* 二进制信号量，用于复位处理 */
    VOS_SEM         resetSem;
    RNIC_TetherInfo tetherInfo;
    /* 接收高速控制配置 */
    RNIC_RHC_Cfg         rhcCfg;
    struct wakeup_source onDemandDisconnWakeLock;
    VOS_UINT32           dsflowTimerMask;
    VOS_UINT16           rnicCfgIndNum;
    VOS_UINT16           rnicLbCfgIndNum;
    VOS_UINT16           rhcCfgIndNum;
    VOS_UINT16           reserved;
    VOS_UINT32           reserved1;
} RNIC_CTX;

extern RNIC_CTX g_rnicCtx;

VOS_VOID RNIC_InitCtx(VOS_VOID);
VOS_VOID RNIC_InitPdnInfo(RNIC_PS_IfaceInfo *pdnInfo);
VOS_VOID RNIC_InitNapiCfg(RNIC_DEV_ID_ENUM_UINT8 rmNetId, RNIC_NapiCfg *napiCfg);
VOS_VOID RNIC_InitNapiLbCfg(RNIC_DEV_ID_ENUM_UINT8 rmNetId, RNIC_NapiLbCfg *napiLbCfg);
VOS_VOID RNIC_InitIpv4PdnInfo(RNIC_PS_IfaceInfo *pdnInfo);
VOS_VOID RNIC_InitIpv6PdnInfo(RNIC_PS_IfaceInfo *pdnInfo);
VOS_VOID RNIC_InitEthPdnInfo(RNIC_PS_IfaceInfo *pdnInfo);
VOS_VOID RNIC_InitOnDemandDialInfo(VOS_VOID);
VOS_VOID        RNIC_InitResetSem(VOS_VOID);
VOS_VOID        RNIC_InitTetherInfo(VOS_VOID);

#if (defined(CONFIG_BALONG_SPE))
VOS_VOID RNIC_InitSpeCfg(RNIC_DEV_ID_ENUM_UINT8 rmNetId);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _RNIC_CTX_H_ */
