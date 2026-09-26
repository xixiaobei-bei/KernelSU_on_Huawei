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

#ifndef _DMS_DEV_H_
#define _DMS_DEV_H_

#include "vos.h"
#include "mdrv_udi.h"
#include "dms_port_i.h"
#if (VOS_OS_VER != VOS_LINUX)
#include "Linuxstub.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define DMS_INIT_HANDLE             0  /* 端口初始化handle值，用于初始化不需要open的端口，便于流程统一 */

#define DMS_UART_UL_DATA_BUFF_SIZE  1536
#define DMS_UART_UL_DATA_BUFF_NUM   16

#define DMS_MODEM_UL_DATA_BUFF_SIZE 1536
#define DMS_MODEM_UL_DATA_BUFF_NUM  16

#define DMS_ACM_UL_DATA_BUFF_SIZE   1024
#define DMS_ACM_UL_DATA_BUFF_NUM    16

#if (FEATURE_IOT_CMUX == FEATURE_ON)
#define DMS_CMUX_PORT_NUM 4
#endif

#define DMS_DEV_ITEM(port_id, dev_id,dev_ops)  \
    [port_id] = { \
    .handle = UDI_INVALID_HANDLE, \
    .portId = port_id, \
    .devId = dev_id, \
    .chanStat = ACM_EVT_DEV_SUSPEND, \
    .portFuncTbl = VOS_NULL_PTR, \
    .ops = dev_ops,}

enum DMS_DevType {
    DMS_DEV_TYPE_APPVCOM,
    DMS_DEV_TYPE_ACM_COM,
    DMS_DEV_TYPE_NCM_CTRL,
    DMS_DEV_TYPE_NCM_DATA,
    DMS_DEV_TYPE_ACM_MODEM,
    DMS_DEV_TYPE_UART,
#if (FEATURE_AT_HSUART == FEATURE_ON)
    DMS_DEV_TYPE_HSUART,
#endif
    DMS_DEV_TYPE_SOCK,
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    DMS_DEV_TYPE_CMUX,
#endif

    DMS_DEV_TYPE_BUTT,
};
typedef VOS_UINT8 DMS_DevTypeUint8;

struct DMS_DevInfo {
    VOS_INT32                           handle;
    DMS_PortIdUint16                    portId;
    VOS_UINT16                          devId;
    VOS_UINT8                           chanStat;
    VOS_UINT8                           reserv[7];

    const struct DMS_PortFuncTbl       *portFuncTbl;
    const struct DMS_DevOps            *ops;
};

struct DMS_DevOps {
    VOS_UINT32 (*Open)(struct DMS_DevInfo *dev);
    VOS_UINT32 (*Close)(struct DMS_DevInfo *dev);
    VOS_UINT32 (*SendDataSync)(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
    VOS_UINT32 (*SendDataAsync)(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
    VOS_UINT32 (*SendPrivData)(struct DMS_DevInfo *dev, IMM_Zc *data);
    VOS_UINT32 (*SendMscCmd)(struct DMS_DevInfo *dev, struct DMS_MscInfo *msc);
    VOS_UINT32 (*SetIpv6Dns)(struct DMS_DevInfo *dev, struct DMS_NcmIpv6Dns *dns);
    VOS_UINT32 (*ChangeFlowCtrl)(struct DMS_DevInfo *dev, VOS_UINT32 enableFlg);
    VOS_UINT32 (*ChangeConnSpeed)(struct DMS_DevInfo *dev, struct DMS_ConnSpeed *connSpeed);
    VOS_UINT32 (*ChangeConnLinkState)(struct DMS_DevInfo *dev, VOS_UINT32 linkStatus);
    VOS_UINT32 (*ConfigFlowCtrl)(struct DMS_DevInfo *dev, VOS_UINT32 rtsFlag, VOS_UINT32 ctsFlag);
    VOS_UINT32 (*SetCharFrame)(struct DMS_DevInfo *dev, VOS_UINT32 udiDataLen, VOS_UINT32 udiStpLen, VOS_UINT32 udiParity);
    VOS_UINT32 (*SetBaud)(struct DMS_DevInfo *dev, VOS_UINT32 baudRate);
    VOS_UINT32 (*SetACShell)(struct DMS_DevInfo *dev, VOS_UINT32 mode);
    VOS_UINT32 (*FlushTxData)(struct DMS_DevInfo *dev);
    VOS_UINT32 (*EnableEscapeSeqDetect)(struct DMS_DevInfo *dev, VOS_BOOL enable);
};

struct DMS_DevInfo* DMS_DEV_GetDevEntityByPortId(DMS_PortIdUint16 portId);
const struct DMS_PortFuncTbl* DMS_DEV_GetDevPortTblByPortId(DMS_PortIdUint16 portId);
VOS_INT32  DMS_DEV_GetDevHandlerByPortId(DMS_PortIdUint16 portId);
VOS_INT    DMS_COMM_CBCheck(DMS_PortIdUint16 portId);

VOS_UINT32 DMS_APP_Open(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_APP_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_SOCK_Open(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_SOCK_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_ACM_Open(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_ACM_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_ACM_Close(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_NCM_CtrlOpen(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_NCM_CtrlClose(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_NCM_CtrlSendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_NCM_DataOpen(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_NCM_DataClose(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_NCM_SetIpv6Dns(struct DMS_DevInfo *dev, struct DMS_NcmIpv6Dns *dns);
VOS_UINT32 DMS_NCM_ChangeFlowCtrl(struct DMS_DevInfo *dev, VOS_UINT32 enableFlg);
VOS_UINT32 DMS_NCM_ChangeConnSpeed(struct DMS_DevInfo *dev, struct DMS_ConnSpeed *connSpeed);
VOS_UINT32 DMS_NCM_ChangeConnLinkState(struct DMS_DevInfo *dev, VOS_UINT32 linkStatus);
VOS_UINT32 DMS_MDM_Open(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_MDM_Close(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_MDM_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *dataBuff, VOS_UINT16 len);
VOS_UINT32 DMS_MDM_SendMscCmd(struct DMS_DevInfo *dev, struct DMS_MscInfo *msc);
VOS_UINT32 DMS_MDM_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data);
VOS_UINT32 DMS_UART_Open(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_UART_SendDataSync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_UART_SetBaud(struct DMS_DevInfo *dev, VOS_UINT32 baudRate);
VOS_UINT32 DMS_UART_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data);
#if (FEATURE_IOT_CMUX == FEATURE_ON)
VOS_UINT32 DMS_UART_ClosePort(struct DMS_DevInfo *dev);
#endif
#if (FEATURE_AT_HSUART == FEATURE_ON)
VOS_UINT32 DMS_HSUART_Open(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_HSUART_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_HSUART_SendMscCmd(struct DMS_DevInfo *dev, struct DMS_MscInfo *msc);
VOS_UINT32 DMS_HSUART_CfgFlowCtrl(struct DMS_DevInfo *dev, VOS_UINT32 rtsFlag, VOS_UINT32 ctsFlag);
VOS_UINT32 DMS_HSUART_SetCharFrame(struct DMS_DevInfo *dev, VOS_UINT32 udiDataLen, VOS_UINT32 udiStpLen, VOS_UINT32 udiParity);
VOS_UINT32 DMS_HSUART_SetBaud(struct DMS_DevInfo *dev, VOS_UINT32 baudRate);
VOS_UINT32 DMS_HSUART_SetACShell(struct DMS_DevInfo *dev, VOS_UINT32 mode);
VOS_UINT32 DMS_HSUART_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data);
VOS_UINT32 DMS_HSUART_FlushTxData(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_HSUART_EnableEscapeSequenceDetect(struct DMS_DevInfo *dev, VOS_BOOL enable);
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
VOS_UINT32 DMS_CMUX_Open(struct DMS_DevInfo *dev);
VOS_UINT32 DMS_CMUX_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data);
VOS_UINT32 DMS_CMUX_SendMscCmd(struct DMS_DevInfo *dev, struct DMS_MscInfo *msc);
VOS_UINT32 DMS_CMUX_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_VOID DMS_CMUX_GetUlDataBuff(DMS_PortIdUint16 portId, IMM_Zc **pstImmZc, VOS_UINT32 *pulLen);
VOS_VOID DMS_CMUX_UlDataReadCB(VOS_UINT8 priIndex);
VOS_UINT32 DMS_HSUART_ClosePort(struct DMS_DevInfo *dev);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

