/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef _DMS_DEV_I_H_
#define _DMS_DEV_I_H_

#include "vos.h"
#include "dms_port_i.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

struct DMS_PortFuncTbl {
    union {
        VOS_INT  (*RcvData)(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len);
        VOS_INT  (*RcvPrivData)(DMS_PortIdUint16 portId, IMM_Zc *data);
    };

    VOS_VOID (*RcvConnectInd)(DMS_PortIdUint16 portId, VOS_UINT8 connState);
    VOS_VOID (*RcvMscInd)(DMS_PortIdUint16 portId, struct DMS_MscInfo *dceMsc);
    VOS_VOID (*RcvEscapeSequence)(DMS_PortIdUint16 portId);
    VOS_VOID (*RcvWaterMarkInd)(DMS_PortIdUint16 portId, DMS_WaterLevelUint32 level);
    /* 仅NDIS CTRL端口使用 */
    VOS_VOID (*RcvConnStatusChgInd)(DMS_PortIdUint16 portId, DMS_NcmStatusUint32 status, VOS_UINT32 udiId);
    /* 仅appvcom0使用 */
    VOS_VOID (*RcvSwitchGwModeInd)(DMS_PortIdUint16 portId);
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    VOS_VOID (*RcvCmuxCloseUart)(VOS_VOID);
    VOS_VOID (*RcvMscModeRead)(DMS_PortIdUint16 portId);
#endif
};

VOS_VOID *DMS_DEV_InitCtx(DMS_PortIdUint16 portId, const struct DMS_PortFuncTbl *portFuncTbl);
VOS_VOID DMS_DEV_InitDev(VOS_VOID);

VOS_UINT32 DMS_DEV_Open(VOS_VOID *dev);
VOS_UINT32 DMS_DEV_Close(VOS_VOID *dev);

VOS_UINT32 DMS_DEV_SendDataSync(VOS_VOID *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_DEV_SendDataAsync(VOS_VOID *dev, VOS_UINT8 *data, VOS_UINT16 len);
VOS_UINT32 DMS_DEV_SendPrivData(VOS_VOID *dev, IMM_Zc *data);
VOS_UINT32 DMS_DEV_SendMscCmd(VOS_VOID *dev, struct DMS_MscInfo *msc);

VOS_INT32 DMS_DEV_GetHandle(VOS_VOID *dev);

/*当前NDIS DATA通道使用*/
VOS_UINT32 DMS_DEV_SetIpv6Dns(VOS_VOID *dev, struct DMS_NcmIpv6Dns *dns);
VOS_UINT32 DMS_DEV_ChangeFlowCtrl(VOS_VOID *dev, VOS_UINT32 enableFlg);
VOS_UINT32 DMS_DEV_ChangeConnSpeed(VOS_VOID *dev, struct DMS_ConnSpeed *connSpeed);
VOS_UINT32 DMS_DEV_ChangeConnLinkState(VOS_VOID *dev, VOS_UINT32 linkStatus);

#if (FEATURE_AT_HSUART == FEATURE_ON)
VOS_UINT32 DMS_DEV_ConfigFlowCtrl(VOS_VOID *dev, VOS_UINT32 rtsFlag, VOS_UINT32 ctsFlag);
VOS_UINT32 DMS_DEV_SetCharFrame(VOS_VOID *dev, VOS_UINT32 udiDataLen, VOS_UINT32 udiStpLen, VOS_UINT32 udiParity);
VOS_UINT32 DMS_DEV_SetBaud(VOS_VOID *dev, VOS_UINT32 baudRate);
VOS_UINT32 DMS_DEV_SetACShell(VOS_VOID *dev, VOS_UINT32 mode);
VOS_UINT32 DMS_DEV_FlushTxData(VOS_VOID *dev);
#endif
VOS_UINT32 DMS_DEV_EnableEscapeSequenceDetect(VOS_VOID *dev, VOS_BOOL enable);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

