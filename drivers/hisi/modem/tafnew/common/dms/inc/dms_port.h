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

#ifndef _DMS_PORT_H_
#define _DMS_PORT_H_

#include "vos.h"
#include "hi_list.h"
#include "dms_port_i.h"
#include "dms_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

struct DMS_PORT_ConnectEventInfo {
    HI_LIST_S    list;
    VOS_SPINLOCK lock;
};

struct DMS_PORT_ConnectEventNode {
    HI_LIST_S        list;
    DMS_PortIdUint16 portId;
    VOS_UINT8        connState;
    VOS_UINT8        reserv[5];
};

enum DMS_PortType {
    DMS_PORT_TYPE_COM,          /*该port类型包含pcui、ctrl、pcui2、ndis ctrl、appvcom、uart、sock等端口*/
    DMS_PORT_TYPE_MODEM,
#if (FEATURE_AT_HSUART == FEATURE_ON)
    DMS_PORT_TYPE_HSUART,
#endif
    DMS_PORT_TYPE_NDIS_DATA,
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    DMS_PORT_TYPE_CMUX,
#endif
    DMS_PORT_TYPE_BUTT,
};
typedef VOS_UINT8 DMS_PortTypeUint8;


enum DMS_IoLevel {
    DMS_IO_LEVEL_LOW  = 0,
    DMS_IO_LEVEL_HIGH = 1,

    DMS_IO_LEVEL_BUTT
};
typedef VOS_UINT8 DMS_IoLevelUint8;

struct DMS_PORT_UartLineCtrl {
    DMS_PortUartDcdModeUint8        dcdMode;
    DMS_PortUartDtrModeUint8        dtrMode;
    DMS_PortUartDsrModeUint8        dsrMode;
    VOS_UINT8                       reserve;
};

struct DMS_PORT_UartCtx {
    struct DMS_PORT_UartLineCtrl    uartLineCtrl;
    VOS_UINT32                      txWmHighFlg; /* UART TX高水线标识 */
};

struct DMS_PORT_Subscriber {
    VOS_UINT32      pid;
    HI_LIST_S       list;
};

struct DMS_PORT_SubscripEventInfo {
    HI_LIST_S       subscriperList;
};

struct DMS_Port_ModeInfo {
    DMS_PortModeUint8               mode;
    DMS_PortDataModeUint8           dataMode;
    DMS_PortDataStateUint8          dataState;
    VOS_UINT8                       reserve1;
};

struct DMS_DMS_MscInd {
    VOS_MSG_HEADER
    DMS_IntraMsgIdUint32        msgId;      /* 消息类型     */

    DMS_PortIdUint16            portId;
    VOS_UINT16                  reserv;
    struct DMS_MscInfo          msc;
};

struct DMS_PortInfo {
    DMS_PortIdUint16                portId;
    DMS_PortTypeUint8               portType;

    /* 端口连接状态 */
    DMS_ConnectStatUint8            connState;

    /* 管脚信号状态保存 */
    VOS_UINT8                       mscStatus;
    VOS_UINT8                       reserv[3];

    /* 通道传输相关的信息，包括端口模式，传输状态等 */
    struct DMS_Port_ModeInfo        modeInfo;

    struct DMS_ClientInfo           clientInfo;
    VOS_VOID                       *devInfo;
};

struct DMS_PortInfo* DMS_PORT_GetPortInfo(DMS_PortIdUint16 portId);
struct DMS_ClientInfo* DMS_PORT_GetClientInfo(DMS_PortIdUint16 portId);
struct DMS_Port_ModeInfo* DMS_PORT_GetModeInfo(DMS_PortIdUint16 portId);

VOS_UINT32 DMS_PORT_IsPortInited(DMS_PortIdUint16 portId);
VOS_VOID DMS_PORT_RcvConnStatusChgInd(DMS_PortIdUint16 portId, DMS_NcmStatusUint32 status, VOS_UINT32 udiId);
VOS_VOID  DMS_PORT_RcvSwitchGwModeInd(DMS_PortIdUint16 portId);
VOS_VOID  DMS_PORT_RcvMsc(DMS_PortIdUint16 portId, struct DMS_MscInfo *dceMsc);
VOS_VOID  DMS_PORT_RcvEscapeSequence(DMS_PortIdUint16 portId);
VOS_VOID  DMS_PORT_RcvWaterMarkInd(DMS_PortIdUint16 portId, DMS_WaterLevelUint32 level);
VOS_VOID  DMS_PORT_RcvConnectInd(DMS_PortIdUint16 portId, VOS_UINT8 connState);
VOS_VOID  DMS_PORT_ProcConnectEvent(DMS_PortIdUint16 portId, VOS_UINT8 connState);

/* dms port数据传输相关接口 */
VOS_VOID DMS_PORT_SetCbtPortId(DMS_PortIdUint16 portId);
VOS_INT32 DMS_PORT_RcvData(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len);
VOS_INT32  DMS_PORT_RcvPrivData(DMS_PortIdUint16 portId, IMM_Zc *data);

/* dms port管脚信号相关接口 */
struct DMS_PORT_UartLineCtrl* DMS_PORT_GetUartLineCtrl(VOS_VOID);
VOS_VOID DMS_PORT_ProcDtrDeasserted(DMS_PortIdUint16 portId);
VOS_VOID DMS_PORT_ProcDtrAsserted(DMS_PortIdUint16 portId);

/* 订阅事件链表头初始化 */
VOS_VOID DMS_PORT_InitSubscripEventInfo(VOS_VOID);
VOS_VOID DMS_ProcIntraPortEvent(const MsgBlock *msg);
VOS_VOID DMS_PORT_InitConnectEventInfo(VOS_VOID);

VOS_VOID DMS_SendDmsMscInd(DMS_PortIdUint16 portId, struct DMS_MscInfo *msc);

VOS_VOID DMS_PORT_ClosePort(DMS_PortIdUint16 portId);

struct DMS_PORT_ConnectEventInfo *DMS_PORT_GetConnectEventInfo(VOS_VOID);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

