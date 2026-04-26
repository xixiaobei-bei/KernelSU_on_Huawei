/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "dms_debug.h"
#include "dms_port.h"
#include "dms_dev.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_DMS_DEBUG_C

struct DMS_PortDebugCfg g_dmsPortDebugCfg = {0};

struct DMS_PortDebugCfg *DMS_PORT_GetDebugCfg(VOS_VOID)
{
    return &g_dmsPortDebugCfg;
}

VOS_BOOL DMS_TestBit(VOS_UINT64 bitMaskValue, VOS_UINT32 bitNum)
{
    return (VOS_BOOL)(1ULL & (bitMaskValue >> bitNum));
}

MODULE_EXPORTED VOS_VOID DMS_ShowDebugCfgInfo(VOS_VOID)
{
    struct DMS_PortDebugCfg *debugCfg = VOS_NULL_PTR;

    debugCfg = DMS_PORT_GetDebugCfg();

    PS_PRINTF_INFO("<DMS_ShowDebugCfgInfo> debugLevel:     %d\n",  debugCfg->debugLevel);
    PS_PRINTF_INFO("<DMS_ShowDebugCfgInfo> portIdMask:     %llx\n", debugCfg->portIdMask);
}

VOS_VOID DMS_ShowPortInfo(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    if (portId >= DMS_PORT_BUTT) {
        PS_PRINTF_INFO("<DMS_ShowPortInfo> portId:%d is invalid\n", portId);
        return;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);

    PS_PRINTF_INFO("<DMS_ShowPortInfo> portId:      %d\n",  portInfo->portId);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> portType:    %d\n",  portInfo->portType);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> mscStatus:   %d\n",  portInfo->mscStatus);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> mode:        %d\n",  portInfo->modeInfo.mode);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> dataMode:    %d\n",  portInfo->modeInfo.dataMode);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> dataState:   %d\n",  portInfo->modeInfo.dataState);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> pppId:       %d\n",  portInfo->clientInfo.pppId);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> clientType:  %d\n",  portInfo->clientInfo.type);
    PS_PRINTF_INFO("<DMS_ShowPortInfo> clientId:    %d\n",  portInfo->clientInfo.id);

    if (portInfo->clientInfo.RcvCmdData == VOS_NULL_PTR) {
        PS_PRINTF_INFO("<DMS_ShowPortInfo> %d RcvCmdData is null\n", portId);
    }

    if (portInfo->clientInfo.RcvRawData == VOS_NULL_PTR) {
        PS_PRINTF_INFO("<DMS_ShowPortInfo> %d RcvCbtData is null\n", portId);
    }

    if (portInfo->devInfo == VOS_NULL_PTR) {
        PS_PRINTF_INFO("<DMS_ShowPortInfo> %d devInfo is null\n", portId);
    }
}

VOS_VOID DMS_ShowDevInfo(DMS_PortIdUint16 portId)
{
    struct DMS_DevInfo *devInfo = VOS_NULL_PTR;

    if (portId >= DMS_PORT_BUTT) {
        PS_PRINTF_INFO("<DMS_ShowDevInfo> portId:%d is invalid\n", portId);
        return;
    }

    devInfo = DMS_DEV_GetDevEntityByPortId(portId);

    PS_PRINTF_INFO("<DMS_ShowDevInfo> handle:   %d\n",  devInfo->handle);
    PS_PRINTF_INFO("<DMS_ShowDevInfo> portId:   %d\n",  devInfo->portId);
    PS_PRINTF_INFO("<DMS_ShowDevInfo> devId:    %d\n",  devInfo->devId);
    PS_PRINTF_INFO("<DMS_ShowDevInfo> chanStat: %d\n",  devInfo->chanStat);

    if (devInfo->portFuncTbl == VOS_NULL_PTR) {
        PS_PRINTF_INFO("<DMS_ShowDevInfo> %d portFuncTbl is null\n", portId);
    }

    if (devInfo->ops == VOS_NULL_PTR) {
        PS_PRINTF_INFO("<DMS_ShowDevInfo> %d ops is null\n", portId);
    }
}

VOS_VOID DMS_ShowUartLineCtrlInfo(VOS_VOID)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;

    lineCtrl = DMS_PORT_GetUartLineCtrl();

    PS_PRINTF_INFO("<DMS_ShowUartLineCtrlInfo> dcdmode: %d\n", lineCtrl->dcdMode);
    PS_PRINTF_INFO("<DMS_ShowUartLineCtrlInfo> dtrmode: %d\n", lineCtrl->dtrMode);
    PS_PRINTF_INFO("<DMS_ShowUartLineCtrlInfo> dsrmode: %d\n", lineCtrl->dsrMode);
}

