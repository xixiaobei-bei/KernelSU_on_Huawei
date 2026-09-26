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

#ifndef __AT_RNIC_INTERFACE_H__
#define __AT_RNIC_INTERFACE_H__

#include "vos.h"
#include "rnic_dev_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#if (VOS_OS_VER == VOS_WIN32)
/* 测试ST用例定义的IP数据包长度，单位字节 */
#define RNIC_IP_DATA_LEN 12
#endif

/* APP拨号后默认空闲时间 */
#define AT_APP_IDLE_TIME 600

#define RNIC_RMNET_NAME_MAX_LEN 16

/*
 * 结构说明: USB Tethering连接状态
 */
enum AT_RNIC_UsbTetherConn {
    AT_RNIC_USB_TETHER_DISCONNECT = 0x00, /* USB tethering未连接 */
    AT_RNIC_USB_TETHER_CONNECTED  = 0x01, /* USB tethering已连接 */
    AT_RNIC_USB_TETHER_CONN_BUTT
};
typedef VOS_UINT8 AT_RNIC_UsbTetherConnUint8;

/*
 * 结构说明: RNIC数据业务网卡特性配置模式
 */
enum AT_RNIC_FeatureMode {
    AT_RNIC_SINGLE_RMNET = 0x00, /* 单网卡特性配置 */
    AT_RNIC_ALL_RMNET    = 0x01, /* 所有数据业务网卡特性配置 */
    AT_RNIC_FEATURE_MODE_BUTT
};
typedef VOS_UINT8 AT_RNIC_FeatureModeUint8;

/*
 * 枚举说明: AT与RNIC的消息定义
 */
enum AT_RNIC_MsgId {
    /* AT发给RNIC的消息枚举 */
    /* 0x0001, 0x0003 ~ 0x0008 删除 */
    ID_AT_RNIC_DIAL_MODE_REQ = 0x0002, /* 拨号模式查询 */
    ID_AT_RNIC_USB_TETHER_INFO_IND = 0x000B, /* USB Tethering信息指示 */
    ID_AT_RNIC_RMNET_CFG_REQ = 0x000C, /* 数据网卡特性配置请求 */

    /* RNIC发给AT的消息枚举 */
    /* 0x1001 0x1002删除 */
    ID_RNIC_AT_DIAL_MODE_CNF = 0x1003, /* 拨号模式查询回复 */
    ID_AT_RNIC_RMNET_CFG_CNF = 0x1004, /* 数据网卡特性配置回复 */
    ID_RNIC_AT_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 AT_RNIC_MsgIdUint32;

/*
 * 4 STRUCT&UNION定义
 */

/*
 * 结构说明: AT给RNIC发送流量查询请求
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT16 clientId; /* Client ID */
    VOS_UINT8  rsv[2];   /* 保留 */
} AT_RNIC_DialModeReq;

/*
 * 结构说明: RNIC给AT拨号模式查询回复
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT16 clientId;        /* Client ID */
    VOS_UINT8  rsv[2];          /* 保留 */
    VOS_UINT32 dialMode;        /* Dial模式 */
    VOS_UINT32 idleTime;        /* 定时器长度 */
    VOS_UINT32 eventReportFlag; /* 是否给应用上报标识 */
} RNIC_AT_DialModeCnf;

/*
 * 结构说明: USB共享信息指示消息
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT8                  rmnetName[RNIC_RMNET_NAME_MAX_LEN]; /* Rmnet网卡名 */
    AT_RNIC_UsbTetherConnUint8 tetherConnStat;                     /* USB Tethering连接状态 */
    VOS_UINT8                  rsv[3];

} AT_RNIC_UsbTetherInfoInd;

/*
 * 结构说明: 数据网卡特性配置指示消息
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT16               clientId;
    AT_RNIC_FeatureModeUint8 featureMode;
    VOS_UINT8                weight;
    VOS_UINT8                rmnetName[RNIC_RMNET_NAME_MAX_LEN];

} AT_RNIC_RmnetCfgReq;

/*
 * 结构说明: 数据网卡特性配置指示消息
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgId;

    VOS_UINT16 clientId;
    VOS_UINT8  rsv[2];
    VOS_INT    result;

} RNIC_AT_RmnetCfgCnf;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __AT_RNIC_INTERFACE_H__ */
