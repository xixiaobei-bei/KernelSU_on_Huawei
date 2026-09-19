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

#ifndef __AT_RNIC_INTERFACE_H__
#define __AT_RNIC_INTERFACE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#include "vos.h"
#include "rnic_dev_def.h"


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

#if (VOS_OS_VER == VOS_WIN32)
/* 测试ST用例定义的IP数据包长度，单位字节 */
#define RNIC_IP_DATA_LEN                (12)
#endif

/* APP拨号后默认空闲时间 */
#define AT_APP_IDLE_TIME                (600)

/* 最大IPV6地址长度，不包括":" */
#define RNICITF_MAX_IPV6_ADDR_LEN       (16)

#define RNIC_RMNET_NAME_MAX_LEN         (16)


/*******************************************************************************
  3 枚举定义
*******************************************************************************/

/*****************************************************************************
 枚举名称: AT_RNIC_USB_TETHER_CONN_ENUM
 结构说明: USB Tethering连接状态
*****************************************************************************/
enum AT_RNIC_USB_TETHER_CONN_ENUM
{
    AT_RNIC_USB_TETHER_DISCONNECT       = 0x00,                                 /* USB tethering未连接 */
    AT_RNIC_USB_TETHER_CONNECTED        = 0x01,                                 /* USB tethering已连接 */
    AT_RNIC_USB_TETHER_CONN_BUTT
};
typedef VOS_UINT8 AT_RNIC_USB_TETHER_CONN_ENUM_UINT8;

/*****************************************************************************
 枚举名称: AT_RNIC_MSG_ID_ENUM
 枚举说明: AT与RNIC的消息定义
*****************************************************************************/
enum AT_RNIC_MSG_ID_ENUM
{
    /* AT发给RNIC的消息枚举 */
    /* 0x0001, 0x0003 ~ 0x0006, 0x0008 删除 */
    ID_AT_RNIC_DIAL_MODE_REQ            = 0x0002,   /* 拨号模式查询 */
    ID_AT_RNIC_DSFLOW_IND               = 0x0007,   /* 流量上报指示 */
    ID_AT_RNIC_PDN_INFO_CFG_IND         = 0x0009,
    ID_AT_RNIC_PDN_INFO_REL_IND         = 0x000A,
    ID_AT_RNIC_USB_TETHER_INFO_IND      = 0x000B,   /* USB Tethering信息指示 */

    /* RNIC发给AT的消息枚举 */
    /* 0x1001 删除 */
    ID_RNIC_AT_DSFLOW_RSP               = 0x1002,   /* 流量回复 */
    ID_RNIC_AT_DIAL_MODE_CNF            = 0x1003,   /* 拨号模式查询回复 */
    ID_RNIC_AT_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 AT_RNIC_MSG_ID_ENUM_UINT32;

/*****************************************************************************
  4 STRUCT&UNION定义
*****************************************************************************/

/*****************************************************************************
 结构名称: AT_RNIC_DSFLOW_IND_STRU
 结构说明: AT给RNIC发送流量查询请求
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT16                          usClientId;                   /* Client ID */
    VOS_UINT8                           aucRsv[2];                  /* 保留 */
} AT_RNIC_DIAL_MODE_REQ_STRU;

/*****************************************************************************
 结构名称: RNIC_AT_DIAL_MODE_CNF_STRU
 结构说明: RNIC给AT拨号模式查询回复
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT16                          usClientId;                   /* Client ID */
    VOS_UINT8                           aucRsv[2];                  /* 保留 */
    VOS_UINT32                          ulDialMode;                 /* Dial模式 */
    VOS_UINT32                          ulIdleTime;                 /* 定时器长度 */
    VOS_UINT32                          ulEventReportFlag;          /* 是否给应用上报标识 */
} RNIC_AT_DIAL_MODE_CNF_STRU;

/*****************************************************************************
 结构名称: AT_RNIC_DSFLOW_IND_STRU
 结构说明: AT给RNIC发送流量上报指示
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT16                          usClientId;                   /* Client ID */
    RNIC_DEV_ID_ENUM_UINT8              enRnicRmNetId;              /* RNIC网卡ID */
    VOS_UINT8                           aucRsv[1];                  /* 保留 */
} AT_RNIC_DSFLOW_IND_STRU;

/*****************************************************************************
 结构名称: RNIC_DATA_RATE_STRU
 结构说明: RNIC统计的流量速率
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulDLDataRate;                           /* 当前下行速率 */
    VOS_UINT32                          ulULDataRate;                           /* 当前上行速率 */
} RNIC_DATA_RATE_STRU;

/*****************************************************************************
 结构名称: RNIC_AT_DSFLOW_RSP_STRU
 结构说明: RNIC给AT发送流量数据结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT16                          usClientId;                   /* Client ID */
    VOS_UINT8                           aucRsv[2];                  /* 保留 */
    RNIC_DATA_RATE_STRU                 stRnicDataRate;             /* 当前流量速率 */
} RNIC_AT_DSFLOW_RSP_STRU;

/*****************************************************************************
 结构名称: AT_RNIC_PDN_INFO_CFG_IND_STRU
 结构说明: PDN配置请求消息
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT32                          bitOpIpv4PdnInfo    : 1;
    VOS_UINT32                          bitOpIpv6PdnInfo    : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           ucRmNetId;                              /* 网卡ID */
    VOS_UINT8                           ucIfaceId;                              /* 全局网卡ID */
    VOS_UINT8                           aucRsv[3];
    union
    {
        VOS_UINT8                       ucRabId;                                /* 承载号, 范围[5, 15] */
        VOS_UINT8                       ucPduSessionId;                         /* PduSession ID */
    };
    MODEM_ID_ENUM_UINT16                enModemId;                              /* Modem ID */

    VOS_UINT32                          ulIpv4Addr;                             /* IPV4的IP地址，主机序 */
    VOS_UINT8                           aucIpv6Addr[RNICITF_MAX_IPV6_ADDR_LEN]; /* 从 PDP上下文带来的IPV6地址长度，不包括":" */
} AT_RNIC_PDN_INFO_CFG_IND_STRU;

/*****************************************************************************
 结构名称: AT_RNIC_PDN_INFO_REL_IND_STRU
 结构说明: PDN释放请求消息
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT32                          bitOpIpv4PdnInfo    : 1;
    VOS_UINT32                          bitOpIpv6PdnInfo    : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           ucRmNetId;                              /* 网卡ID */
    VOS_UINT8                           ucIfaceId;                              /* 全局网卡ID */
    VOS_UINT8                           aucRsv[2];

} AT_RNIC_PDN_INFO_REL_IND_STRU;

/*****************************************************************************
 结构名称: AT_RNIC_USB_TETHER_INFO_IND_STRU
 结构说明: USB共享信息指示消息
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT8                           aucRmnetName[RNIC_RMNET_NAME_MAX_LEN];  /* Rmnet网卡名 */
    AT_RNIC_USB_TETHER_CONN_ENUM_UINT8  enTetherConnStat;                       /* USB Tethering连接状态 */
    VOS_UINT8                           aucRsv[3];

} AT_RNIC_USB_TETHER_INFO_IND_STRU;


/*****************************************************************************
  5 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  6 函数声明
*****************************************************************************/

extern VOS_UINT32 RNIC_StartFlowCtrl(RNIC_DEV_ID_ENUM_UINT8 enRmNetId);
extern VOS_UINT32 RNIC_StopFlowCtrl(RNIC_DEV_ID_ENUM_UINT8 enRmNetId);



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

#endif /* __AT_RNIC_INTERFACE_H__ */

