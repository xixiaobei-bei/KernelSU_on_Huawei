/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#ifndef __DMS_MNTN_H__
#define __DMS_MNTN_H__

#include "vos.h"
#include "ps_tag.h"
#include "dms_port_def.h"
#include "dms_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* Log Print Module Define */
#ifndef THIS_MODU
#define THIS_MODU ps_nas
#endif

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF) || (defined(LLT_OS_VER))
#define DMS_DUMP_FIELD_ID       OM_AP_DMS
#else
#define DMS_DUMP_FIELD_ID       OM_CP_DMS
#endif

#define DMS_MNTN_SAVE_EXC_LOG_LENGTH 4096
#define DMS_MNTN_DUMP_BEGIN_TAG      0xAA55AA55
#define DMS_MNTN_DUMP_END_TAG        0xAA55AA55

#define DMS_MNTN_STATS_VERSION 100
#define DMS_MNTN_DEV_TYPE_NUM  7 /* 除NDIS DATA外，DEV的设备类型个数 */

/* APPVCOM最多有53个端口，上报错误枚举单独列出 */
enum DMS_MntnAppStatsId {
    DMS_APP_OPEN_ERR,
    DMS_APP_REG_READY_CB_ERR,
    DMS_APP_READY_CB,
    DMS_APP_REG_RD_CB_ERR,
    DMS_APP_RD_CB,
    DMS_APP_REG_WRT_CB_ERR,     /* 5 */
    DMS_APP_WRT_CB,
    DMS_APP_USER_DATA_ERR,
    DMS_APP_WRT_ASYNC,
    DMS_APP_WRT_ASYNC_ERR,
    DMS_APP_RETURN_BUFF_ERR,    /* 10 */
    DMS_APP_RD_BUFF_ERR,

    DMS_APP_ERR_ID_MAX
};
typedef VOS_UINT32 DMS_MntnAppStatsIdUint32;

/* 除APPVCOM外其余所有端口的错误类型 */
enum DMS_MntnComPortStatsId {
    DMS_ACM_ERR_ID_START = 0,
    DMS_ACM_REG_RD_CB_ERR = DMS_ACM_ERR_ID_START,
    DMS_ACM_RD_CB,
    DMS_ACM_BUFF_NULL,
    DMS_ACM_RD_BUFF_ERR,
    DMS_ACM_RETURN_BUFF_ERR,
    DMS_ACM_OPEN_ERR,                               /* 5 */
    DMS_ACM_CLOSE_ERR,
    DMS_ACM_REG_RELLOC_BUFF_ERR,
    DMS_ACM_REG_WRT_COPY_ERR,
    DMS_ACM_EVT_CB,
    DMS_ACM_REG_EVT_CB_ERR,                         /* 10 */
    DMS_ACM_WRT_BUFF_ERR,
    DMS_ACM_WRT_CHAN_STAT_ERR,
    DMS_ACM_WRT_ASYNC,
    DMS_ACM_WRT_ASYNC_ERR,
    DMS_ACM_WRT_DONE_SIZE_ERR,                     /* 15 */
    DMS_ACM_READY_CB,
    DMS_ACM_REG_READY_CB_ERR,
    DMS_ACM_USER_DATA_ERR,
    DMS_ACM_WRT_CB,
    DMS_ACM_REG_WRT_CB_ERR,                        /* 20 */
    DMS_ACM_ERR_ID_END = DMS_ACM_REG_WRT_CB_ERR,

    DMS_NCM_CTRL_ERR_ID_START,
    DMS_NCM_CTRL_OPEN_ERR = DMS_NCM_CTRL_ERR_ID_START,
    DMS_NCM_CTRL_CONN_STUS,
    DMS_NCM_CTRL_CONN_STUS_ERR,
    DMS_NCM_CTRL_REG_CONN_STUS_ERR,
    DMS_NCM_CTRL_RD_CB,                            /* 25 */
    DMS_NCM_CTRL_REG_RD_CB_ERR,
    DMS_NCM_CTRL_RD_DATA_NULL,
    DMS_NCM_CTRL_RD_LEN_INVALID,
    DMS_NCM_CTRL_WRT_CB,
    DMS_NCM_CTRL_REG_WRT_CB_ERR,                  /* 30 */
    DMS_NCM_CTRL_WRT_ASYNC,
    DMS_NCM_CTRL_WRT_GET_BUF_ERR,
    DMS_NCM_CTRL_WRT_CHAN_STAT_ERR,
    DMS_NCM_CTRL_WRT_ASYNC_ERR,
    DMS_NCM_CTRL_REG_READY_CB_ERR,                /* 35 */
    DMS_NCM_CTRL_READY_CB,
    DMS_NCM_CTRL_USER_DATA_ERR,
    DMS_NCM_CTRL_RETURN_BUFF_ERR,
    DMS_NCM_CTRL_CLOSE_ERR,
    DMS_NCM_CTRL_ERR_ID_END = DMS_NCM_CTRL_CLOSE_ERR,

    DMS_NCM_DATA_ERR_ID_START,                    /* 40 */
    DMS_NCM_DATA_OPEN_ERR = DMS_NCM_DATA_ERR_ID_START,
    DMS_NCM_DATA_SET_IPV6_ERR,
    DMS_NCM_DATA_FLOW_CTRL_ERR,
    DMS_NCM_DATA_CONN_SPEED_ERR,
    DMS_NCM_DATA_CONN_NTF_ERR,
    DMS_NCM_DATA_CLOSE_ERR,                       /* 45 */
    DMS_NCM_DATA_ERR_ID_END = DMS_NCM_DATA_CLOSE_ERR,

    DMS_MDM_ERR_ID_START,
    DMS_MDM_OPEN_ERR = DMS_MDM_ERR_ID_START,
    DMS_MDM_RD_CB,
    DMS_MDM_REG_RD_CB_ERR,
    DMS_MDM_RD_ADDR_INVALID,
    DMS_MDM_RD_BUFF_ERR,                          /* 50 */
    DMS_MDM_FREE_CB,
    DMS_MDM_FREE_CB_PPP,
    DMS_MDM_FREE_CB_AT,
    DMS_MDM_REG_FREE_CB_ERR,
    DMS_MDM_REG_WRT_COPY_ERR,                     /* 55 */
    DMS_MDM_RD_MSC,
    DMS_MDM_RD_MSC_NULL,
    DMS_MDM_REG_RD_MSC_ERR,
    DMS_MDM_WRT_MSC,
    DMS_MDM_WRT_MSC_ERR,                          /* 60 */
    DMS_MDM_REG_RELLOC_BUFF_ERR,
    DMS_MDM_REG_READY_CB_ERR,
    DMS_MDM_READY_CB,
    DMS_MDM_USER_DATA_ERR,
    DMS_MDM_RETURN_BUFF_ERR,                      /* 65 */
    DMS_MDM_WRT_ASYNC,
    DMS_MDM_WRT_BUFF_ERR,
    DMS_MDM_WRT_ASYNC_ERR,
    DMS_MDM_CLOSE_ERR,
    DMS_MDM_ERR_ID_END = DMS_MDM_CLOSE_ERR,

    DMS_UART_ERR_ID_START,                        /* 70 */
    DMS_UART_REG_RD_CB_ERR = DMS_UART_ERR_ID_START,
    DMS_UART_RD_CB,
    DMS_UART_RD_ADDR_INVALID,
    DMS_UART_RD_BUFF_ERR,
    DMS_UART_OPEN_ERR,
    DMS_UART_REG_READY_CB_ERR,                    /* 75 */
    DMS_UART_READY_CB,
    DMS_UART_USER_DATA_ERR,
    DMS_UART_RETURN_BUFF_ERR,
    DMS_UART_WRT_SYNC,
    DMS_UART_WRT_SYNC_ERR,                        /* 80 */
    DMS_UART_SET_BAUD_ERR,
    DMS_UART_ERR_ID_END = DMS_UART_SET_BAUD_ERR,

    DMS_HSUART_ERR_ID_START,
    DMS_HSUART_REG_RD_CB_ERR = DMS_HSUART_ERR_ID_START,
    DMS_HSUART_RD_CB,
    DMS_HSUART_RD_ADDR_INVALID,
    DMS_HSUART_RD_BUFF_ERR,                       /* 85 */
    DMS_HSUART_WRT_ASYNC,
    DMS_HSUART_WRT_ASYNC_ERR,
    DMS_HSUART_ALLOC_BUF_ERR,
    DMS_HSUART_OPEN_ERR,
    DMS_HSUART_REG_RELLOC_BUFF_ERR,               /* 90 */
    DMS_HSUART_FREE_CB,
    DMS_HSUART_REG_FREE_CB_ERR,
    DMS_HSUART_SWITCH_MODE,
    DMS_HSUART_REG_SWITCH_MODE_ERR,
    DMS_HSUART_WATER_CB,                          /* 95 */
    DMS_HSUART_WATER_LEVEL_ERR,
    DMS_HSUART_REG_WATER_CB_ERR,
    DMS_HSUART_RD_MSC,
    DMS_HSUART_RD_MSC_NULL,
    DMS_HSUART_REG_RD_MSC_ERR,                    /* 100 */
    DMS_HSUART_WRT_MSC_ERR,
    DMS_HSUART_REG_READY_CB_ERR,
    DMS_HSUART_READY_CB,
    DMS_HSUART_USER_DATA_ERR,
    DMS_HSUART_RETURN_BUFF_ERR,                   /* 105 */
    DMS_HSUART_FLOW_CONTRL_ERR,
    DMS_HSUART_SET_WLEN_ERR,
    DMS_HSUART_SET_STP_ERR,
    DMS_HSUART_SET_EPS_ERR,
    DMS_HSUART_SET_BAUD_ERR,                      /* 110 */
    DMS_HSUART_SET_ACSHELL_ERR,
    DMS_HSUART_FLUSH_BUFF_ERR,
    DMS_HSUART_ERR_ID_END = DMS_HSUART_FLUSH_BUFF_ERR,

    DMS_SOCK_ERR_ID_START,
    DMS_SOCK_RD_CB = DMS_SOCK_ERR_ID_START,
    DMS_SOCK_RD_DATA_NULL,
    DMS_SOCK_RD_LEN_INVALID,                      /* 115 */
    DMS_SOCK_BSP_SUPPORT_ERR,
    DMS_SOCK_OPEN_ERR,
    DMS_SOCK_REG_RD_CB_ERR,
    DMS_SOCK_REG_WRT_CB_ERR,
    DMS_SOCK_WRT_CB,                              /* 120 */
    DMS_SOCK_REG_READY_CB_ERR,
    DMS_SOCK_READY_CB,
    DMS_SOCK_USER_DATA_ERR,
    DMS_SOCK_RETURN_BUFF_ERR,
    DMS_SOCK_WRT_ASYNC,                           /* 125 */
    DMS_SOCK_WRT_ASYNC_ERR,
    DMS_SOCK_ERR_ID_END = DMS_SOCK_WRT_ASYNC_ERR,

    DMS_CMUX_ERR_ID_START,
    DMS_CMUX_OPEN_ERR = DMS_CMUX_ERR_ID_START,
    DMS_CMUX_REG_RD_CB_ERR,
    DMS_CMUX_REG_FREE_CB_ERR,
    DMS_CMUX_REG_CLOSE_CB_ERR,                      /* 130 */
    DMS_CMUX_ALLOC_BUF_ERR,
    DMS_CMUX_WRT_ASYNC,
    DMS_CMUX_WRT_ASYNC_ERR,
    DMS_CMUX_REG_SNC_CB_ERR,
    DMS_CMUX_CLOSE_UART_ERR,                        /* 135 */
    DMS_CMUX_SET_MSC_READ_CB,
    DMS_CMUX_SET_MSC_READ_CB_NULL,
    DMS_CMUX_SET_MSC_READ_CB_ERR,
    DMS_CMUX_WRT_MSC_ERR,
    DMS_CMUX_FREE_CB,                               /* 140 */
    DMS_CMUX_SWITCH_MODE,
    DMS_CMUX_SWITCH_MODE_CB_ERR,
    DMS_CMUX_CLOSE_HSUART_ERR,
    DMS_CMUX_MSC_MODE_READ_CB,
    DMS_CMUX_MSC_MODE_READ_CB_ERR,                  /* 145 */
    DMS_CMUX_ERR_ID_END = DMS_CMUX_MSC_MODE_READ_CB_ERR,

    DMS_COM_ERR_ID_MAX
};
typedef VOS_UINT32 DMS_MntnComPortStatsIdUint32;

enum DMS_MntnStatsType {
    DMS_OPEN_ERR,
    DMS_REG_READY_CB_ERR,
    DMS_READY_CB,
    DMS_REG_RD_CB_ERR,
    DMS_RD_CB,
    DMS_REG_WRT_CB_ERR,
    DMS_WRT_CB,
    DMS_USER_DATA_ERR,
    DMS_WRT_ASYNC,
    DMS_WRT_ASYNC_ERR,
    DMS_RETURN_BUFF_ERR,
    DMS_RD_BUFF_ERR,

    DMS_STATS_TYPE_MAX
};
typedef VOS_UINT32 DMS_MntnStatsTypeUint32;

struct DMS_MntnInfo {
    VOS_UINT32  version; /* 注意：补充DMS维测枚举值需要更新该版本号，以防解析不匹配出错！！！ */
    VOS_UINT32  vcomExtFlag; /* 表示APPVCOM扩展端口是否打开，1表示打开，0表示关闭 */
    VOS_UINT32  appstatsInfo[DMS_APP_PORT_SIZE][DMS_APP_ERR_ID_MAX];   /* APP端口上报信息表 */
    VOS_UINT32  comStatsInfo[DMS_COM_ERR_ID_MAX];                      /* 除APP外其他所有端口上报信息 */
};

struct DMS_MntnMsg {
    VOS_MSG_HEADER
    DMS_IntraMsgIdUint32        msgId;      /* 消息类型     */
    VOS_UINT32                  portId;     /* 端口ID */
    VOS_UINT8                   data[0];
};

struct DMS_MntnExcLogInfo {
    VOS_UINT32 beginTag;
    VOS_UINT32 reserv1;

    struct DMS_MntnInfo mntnInfo;

    VOS_UINT32 reserv2;
    VOS_UINT32 endTag;
};

/* 上报APPVCOM端口错误类型信息 */
extern VOS_VOID DMS_MNTN_IncAppStatsInfo(DMS_PortIdUint16 portId, DMS_MntnAppStatsIdUint32 statsId);

/* 上报除APPVCOM外其余所有端口错误类型信息 */
extern VOS_VOID DMS_MNTN_IncComStatsInfo(DMS_MntnComPortStatsIdUint32 statsId);

/* 新平台上报端口错误类型信息 */
extern VOS_VOID DMS_MNTN_IncStatsInfo(DMS_PortIdUint16 portId, DMS_MntnStatsTypeUint32 statsType);

/* DMS事件上报接口 */
extern VOS_VOID DMS_MNTN_TraceReport(DMS_PortIdUint16 portId, DMS_IntraMsgIdUint32 msgId, VOS_UINT8 *data, VOS_UINT32 len);

extern struct DMS_MntnInfo *DMS_MNTN_GetDmsMntnInfo(VOS_VOID);

extern VOS_VOID DMS_SendPortDebugNvCfg(VOS_UINT32 debugLevel, VOS_UINT32 portIdMask1, VOS_UINT32 portIdMask2);

extern VOS_VOID DMS_RegisterDumpCallBack(VOS_VOID);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /*__DMS_MNTN_H__ */
