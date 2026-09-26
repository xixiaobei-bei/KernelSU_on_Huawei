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

#include "dms_dev.h"

#include "vos.h"
#include "imm_interface.h"
#include "at_app_vcom_interface.h"
#include "dms_dev_i.h"
#include "dms_debug.h"
#include "dms_buff.h"

#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_C

VOS_INT DMS_COMM_CBCheck(DMS_PortIdUint16 portId)
{
    const struct DMS_PortFuncTbl* portTblInfo = VOS_NULL_PTR;
    VOS_INT32 handle;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("portId:%d is invalid.", portId);
        return VOS_ERROR;
    }

    handle = DMS_DEV_GetDevHandlerByPortId(portId);
    if (handle == UDI_INVALID_HANDLE) {
        DMS_PORT_LOGE(portId, "handle is null.");
        return VOS_ERROR;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "portTblInfo is not registered.");
        return VOS_ERROR;
    }

    return VOS_OK;
}

VOS_VOID *DMS_DEV_InitCtx(DMS_PortIdUint16 portId, const struct DMS_PortFuncTbl *portFuncTbl)
{
    struct DMS_DevInfo  *devInfo = VOS_NULL_PTR;

    DMS_PORT_LOGI(portId, "enter.");

    if (portFuncTbl == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "portFuncTbl null.");
        return VOS_NULL_PTR;
    }

    devInfo = DMS_DEV_GetDevEntityByPortId(portId);
    devInfo->portFuncTbl = portFuncTbl;

    return devInfo;
}

STATIC VOS_VOID DMS_DEV_RcvConnectStateInd(DMS_PortIdUint16 portId, VOS_UINT8 connState)
{
    const struct DMS_PortFuncTbl* portTblInfo = VOS_NULL_PTR;

    DMS_PORT_LOGI(portId, "enter.");

    DMS_MNTN_IncStatsInfo(portId, DMS_READY_CB);

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "portTblInfo not registered.");
        return;
    }

    if (portTblInfo->RcvConnectInd == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "RcvConnectInd is not registered.");
        return;
    }

    portTblInfo->RcvConnectInd(portId, connState);
}

STATIC VOS_VOID DMS_DEV_RcvUsbEnableInd(VOS_VOID)
{
    DMS_LOGI("enter.");

    DMS_DEV_RcvConnectStateInd(DMS_PORT_MODEM, DMS_CONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA, DMS_CONNECT);
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA1, DMS_CONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA2, DMS_CONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA3, DMS_CONNECT);
#endif
    DMS_DEV_RcvConnectStateInd(DMS_PORT_PCUI, DMS_CONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_CTRL, DMS_CONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_PCUI2, DMS_CONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_CTRL, DMS_CONNECT);
}

STATIC VOS_VOID DMS_DEV_RcvUsbDisableInd(VOS_VOID)
{
    DMS_LOGI("enter.");

    DMS_DEV_RcvConnectStateInd(DMS_PORT_MODEM, DMS_DISCONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA, DMS_DISCONNECT);
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA1, DMS_DISCONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA2, DMS_DISCONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_DATA3, DMS_DISCONNECT);
#endif
    DMS_DEV_RcvConnectStateInd(DMS_PORT_PCUI, DMS_DISCONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_CTRL, DMS_DISCONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_PCUI2, DMS_DISCONNECT);
    DMS_DEV_RcvConnectStateInd(DMS_PORT_NCM_CTRL, DMS_DISCONNECT);
}

STATIC VOS_UINT32 DMS_DEV_IsAlwaysOnPort(DMS_PortIdUint16 portId)
{
    /* usb相关端口都为非一直在线端口 */
    if (portId == DMS_PORT_MODEM || portId == DMS_PORT_NCM_DATA
#if (FEATURE_MULTI_NCM == FEATURE_ON)
     || portId == DMS_PORT_NCM_DATA1 || portId == DMS_PORT_NCM_DATA2
     || portId == DMS_PORT_NCM_DATA3
#endif
     || (portId >= DMS_PORT_PCUI && portId <= DMS_PORT_NCM_CTRL)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_VOID DMS_DEV_InitDev(VOS_VOID)
{
    struct DMS_DevInfo      *devInfo = VOS_NULL_PTR;
    VOS_UINT32               i;

    DMS_LOGI("enter.");

    for (i = 0; i < DMS_PORT_BUTT; i++) {
        if (DMS_DEV_IsAlwaysOnPort((DMS_PortIdUint16)i) == VOS_TRUE) {
            devInfo = DMS_DEV_GetDevEntityByPortId((VOS_UINT8)i);
            DMS_DEV_Open(devInfo);
        }
    }

    /* 初始化AT通道使用的静态内存 */
    DMS_BUF_StaticBufInit();

    /* NDIS DATA 和modem口通过usb enable cb打开 */
    mdrv_usb_reg_enablecb((usb_udi_enable_cb)DMS_DEV_RcvUsbEnableInd);
    mdrv_usb_reg_disablecb((usb_udi_enable_cb)DMS_DEV_RcvUsbDisableInd);

    /* 开启维测定时器 */
    if (DMS_StartTimer(DMS_DEFAULT_ERR_RPT_TIME, TI_DMS_TIMER_MNTN_INFO) != VOS_OK) {
        DMS_LOGE("DMS_DEV_InitDev:ERROR:DMS_StartTimer!");
    }
}

STATIC VOS_UINT32 DMS_DEV_CommCheck(struct DMS_DevInfo *dev)
{
    DMS_LOGI("enter.");

    if (dev == VOS_NULL_PTR) {
        DMS_LOGE("dev is null.");
        return VOS_ERR;
    }

    if (dev->handle == UDI_INVALID_HANDLE) {
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    if (dev->ops == VOS_NULL_PTR) {
        DMS_PORT_LOGE(dev->portId, "dev ops is null.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_DEV_SendDataSync(VOS_VOID *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    /* 由上层保证下发的data和len不为空指针和0 */
    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->SendDataSync == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->SendDataSync(dev, data, len);
}

VOS_UINT32 DMS_DEV_SendDataAsync(VOS_VOID *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    /* 由上层保证下发的data和len不为空指针和0 */
    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->SendDataAsync == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->SendDataAsync(dev, data, len);
}

VOS_UINT32 DMS_DEV_SendPrivData(VOS_VOID *dev, IMM_Zc *immZc)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    /* 由上层保证下发的immZc不为空指针 */
    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        IMM_ZcFree(immZc);
        return VOS_ERR;
    }

    if (devInfo->ops->SendPrivData == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        IMM_ZcFree(immZc);
        return VOS_ERR;
    }

    return devInfo->ops->SendPrivData(dev, immZc);
}

VOS_UINT32 DMS_DEV_SendMscCmd(VOS_VOID *dev, struct DMS_MscInfo *msc)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->SendMscCmd == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->SendMscCmd(dev, msc);
}

VOS_UINT32 DMS_DEV_SetIpv6Dns(VOS_VOID *dev, struct DMS_NcmIpv6Dns *dns)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->SetIpv6Dns == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->SetIpv6Dns(dev, dns);
}

VOS_UINT32 DMS_DEV_ChangeFlowCtrl(VOS_VOID *dev, VOS_UINT32 enableFlg)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->ChangeFlowCtrl == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->ChangeFlowCtrl(dev, enableFlg);
}

VOS_UINT32 DMS_DEV_ChangeConnSpeed(VOS_VOID *dev, struct DMS_ConnSpeed *connSpeed)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->ChangeConnSpeed == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->ChangeConnSpeed(dev, connSpeed);
}

VOS_UINT32 DMS_DEV_ChangeConnLinkState(VOS_VOID *dev, VOS_UINT32 linkStatus)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->ChangeConnLinkState == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->ChangeConnLinkState(dev, linkStatus);
}

VOS_UINT32 DMS_DEV_Open(VOS_VOID *dev)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (devInfo == VOS_NULL_PTR) {
        DMS_LOGE("dev is null.");
        return VOS_ERR;
    }

    if (devInfo->handle != UDI_INVALID_HANDLE) {
        DMS_PORT_LOGE(devInfo->portId, "dev is opened.");
        return VOS_ERR;
    }

    if (devInfo->ops == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "ops is null.");
        return VOS_ERR;
    }

    if (devInfo->ops->Open == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->Open(dev);
}

VOS_UINT32 DMS_DEV_Close(VOS_VOID *dev)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->Close == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->Close(dev);
}

VOS_INT32 DMS_DEV_GetHandle(VOS_VOID *dev)
{
    struct DMS_DevInfo *devInfo = dev;
    return devInfo->handle;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_UINT32 DMS_DEV_ConfigFlowCtrl(VOS_VOID *dev, VOS_UINT32 rtsFlag, VOS_UINT32 ctsFlag)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->ConfigFlowCtrl == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->ConfigFlowCtrl(dev, rtsFlag, ctsFlag);
}

VOS_UINT32 DMS_DEV_SetCharFrame(VOS_VOID *dev, VOS_UINT32 udiDataLen, VOS_UINT32 udiStpLen, VOS_UINT32 udiParity)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->SetCharFrame == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->SetCharFrame(dev, udiDataLen, udiStpLen, udiParity);
}

VOS_UINT32 DMS_DEV_SetBaud(VOS_VOID *dev, VOS_UINT32 baudRate)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->SetBaud == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->SetBaud(dev, baudRate);
}

VOS_UINT32 DMS_DEV_SetACShell(VOS_VOID *dev, VOS_UINT32 mode)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->SetACShell == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->SetACShell(dev, mode);
}

VOS_UINT32 DMS_DEV_FlushTxData(VOS_VOID *dev)
{
    struct DMS_DevInfo *devInfo = dev;
    VOS_UINT32 ret;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->FlushTxData == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops func is null.");
        return VOS_ERR;
    }

    ret = devInfo->ops->FlushTxData(dev);

    return ret;
}
#endif

VOS_UINT32 DMS_DEV_EnableEscapeSequenceDetect(VOS_VOID *dev, VOS_BOOL enable)
{
    struct DMS_DevInfo *devInfo = dev;

    DMS_LOGI("enter.");

    if (DMS_DEV_CommCheck(devInfo) != VOS_OK) {
        return VOS_ERR;
    }

    if (devInfo->ops->EnableEscapeSeqDetect == VOS_NULL_PTR) {
        DMS_PORT_LOGE(devInfo->portId, "dev ops EnableEscapeSeqDetect func is null.");
        return VOS_ERR;
    }

    return devInfo->ops->EnableEscapeSeqDetect(dev, enable);
}

STATIC const struct DMS_DevOps g_dmsDevOps[] = {
    [DMS_DEV_TYPE_APPVCOM] = {
        .Open                   = DMS_APP_Open,
        .SendDataAsync          = DMS_APP_SendDataAsync,
    },

    [DMS_DEV_TYPE_ACM_COM] = {
        .Open                   = DMS_ACM_Open,
        .SendDataAsync          = DMS_ACM_SendDataAsync,
        .Close                  = DMS_ACM_Close,
    },

    [DMS_DEV_TYPE_NCM_CTRL] = {
        .Open                   = DMS_NCM_CtrlOpen,
        .Close                  = DMS_NCM_CtrlClose,
        .SendDataAsync          = DMS_NCM_CtrlSendDataAsync,
    },

    [DMS_DEV_TYPE_NCM_DATA] = {
        .Open                   = DMS_NCM_DataOpen,
        .Close                  = DMS_NCM_DataClose,
        .SetIpv6Dns             = DMS_NCM_SetIpv6Dns,
        .ChangeFlowCtrl         = DMS_NCM_ChangeFlowCtrl,
        .ChangeConnSpeed        = DMS_NCM_ChangeConnSpeed,
        .ChangeConnLinkState    = DMS_NCM_ChangeConnLinkState,
    },

    [DMS_DEV_TYPE_ACM_MODEM] = {
        .Open                   = DMS_MDM_Open,
        .Close                  = DMS_MDM_Close,
        .SendDataAsync          = DMS_MDM_SendDataAsync,
        .SendMscCmd             = DMS_MDM_SendMscCmd,
        .SendPrivData           = DMS_MDM_SendPrivData,
    },

    [DMS_DEV_TYPE_UART] = {
        .Open                   = DMS_UART_Open,
#if (FEATURE_IOT_CMUX == FEATURE_ON)
        .Close                  = DMS_UART_ClosePort,
#endif
        .SendDataSync           = DMS_UART_SendDataSync,
        .SetBaud                = DMS_UART_SetBaud,
        .SendPrivData           = DMS_UART_SendPrivData,
    },
#if (FEATURE_AT_HSUART == FEATURE_ON)
    [DMS_DEV_TYPE_HSUART] = {
        .Open                   = DMS_HSUART_Open,
#if (FEATURE_IOT_CMUX == FEATURE_ON)
        .Close                  = DMS_HSUART_ClosePort,
#endif
        .SendDataSync           = DMS_UART_SendDataSync,
        .SendDataAsync          = DMS_HSUART_SendDataAsync,
        .SendMscCmd             = DMS_HSUART_SendMscCmd,
        .ConfigFlowCtrl         = DMS_HSUART_CfgFlowCtrl,
        .SetCharFrame           = DMS_HSUART_SetCharFrame,
        .SetBaud                = DMS_HSUART_SetBaud,
        .SetACShell             = DMS_HSUART_SetACShell,
        .SendPrivData           = DMS_HSUART_SendPrivData,
        .FlushTxData            = DMS_HSUART_FlushTxData,
        .EnableEscapeSeqDetect  = DMS_HSUART_EnableEscapeSequenceDetect,
    },
#endif
    [DMS_DEV_TYPE_SOCK] = {
        .Open                   = DMS_SOCK_Open,
        .SendDataAsync          = DMS_SOCK_SendDataAsync,
    },
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    [DMS_DEV_TYPE_CMUX] = {
        .Open                   = DMS_CMUX_Open,
        .SendDataAsync          = DMS_CMUX_SendDataAsync,
        .SendPrivData           = DMS_CMUX_SendPrivData,
        .SendMscCmd             = DMS_CMUX_SendMscCmd,
    },
#endif
};

struct DMS_DevInfo g_dmsDevInfo[] =  {
    DMS_DEV_ITEM(DMS_PORT_APP,      APP_VCOM_DEV_INDEX_0,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP1,     APP_VCOM_DEV_INDEX_1,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    DMS_DEV_ITEM(DMS_PORT_APP2,     APP_VCOM_DEV_INDEX_2,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP3,     APP_VCOM_DEV_INDEX_3,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP4,     APP_VCOM_DEV_INDEX_4,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP5,     APP_VCOM_DEV_INDEX_5,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP6,     APP_VCOM_DEV_INDEX_6,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP7,     APP_VCOM_DEV_INDEX_7,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP8,     APP_VCOM_DEV_INDEX_8,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP9,     APP_VCOM_DEV_INDEX_9,       &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP10,    APP_VCOM_DEV_INDEX_10,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP11,    APP_VCOM_DEV_INDEX_11,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP12,    APP_VCOM_DEV_INDEX_12,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP13,    APP_VCOM_DEV_INDEX_13,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP14,    APP_VCOM_DEV_INDEX_14,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP15,    APP_VCOM_DEV_INDEX_15,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP16,    APP_VCOM_DEV_INDEX_16,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP17,    APP_VCOM_DEV_INDEX_17,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP18,    APP_VCOM_DEV_INDEX_18,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP19,    APP_VCOM_DEV_INDEX_19,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP20,    APP_VCOM_DEV_INDEX_20,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP21,    APP_VCOM_DEV_INDEX_21,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP22,    APP_VCOM_DEV_INDEX_22,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP23,    APP_VCOM_DEV_INDEX_23,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP24,    APP_VCOM_DEV_INDEX_24,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP25,    APP_VCOM_DEV_INDEX_25,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP26,    APP_VCOM_DEV_INDEX_26,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP27,    APP_VCOM_DEV_INDEX_27,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP28,    APP_VCOM_DEV_INDEX_28,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP29,    APP_VCOM_DEV_INDEX_29,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP30,    APP_VCOM_DEV_INDEX_30,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP31,    APP_VCOM_DEV_INDEX_31,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP32,    APP_VCOM_DEV_INDEX_32,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP33,    APP_VCOM_DEV_INDEX_33,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP34,    APP_VCOM_DEV_INDEX_34,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP35,    APP_VCOM_DEV_INDEX_35,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP36,    APP_VCOM_DEV_INDEX_36,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP37,    APP_VCOM_DEV_INDEX_37,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP38,    APP_VCOM_DEV_INDEX_38,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP39,    APP_VCOM_DEV_INDEX_39,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP40,    APP_VCOM_DEV_INDEX_40,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP41,    APP_VCOM_DEV_INDEX_41,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP42,    APP_VCOM_DEV_INDEX_42,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP43,    APP_VCOM_DEV_INDEX_43,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP44,    APP_VCOM_DEV_INDEX_44,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP45,    APP_VCOM_DEV_INDEX_45,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP46,    APP_VCOM_DEV_INDEX_46,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP47,    APP_VCOM_DEV_INDEX_47,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP48,    APP_VCOM_DEV_INDEX_48,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP49,    APP_VCOM_DEV_INDEX_49,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP50,    APP_VCOM_DEV_INDEX_50,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP51,    APP_VCOM_DEV_INDEX_51,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
    DMS_DEV_ITEM(DMS_PORT_APP52,    APP_VCOM_DEV_INDEX_52,      &g_dmsDevOps[DMS_DEV_TYPE_APPVCOM]),
#endif

    DMS_DEV_ITEM(DMS_PORT_PCUI,     UDI_ACM_AT_ID,              &g_dmsDevOps[DMS_DEV_TYPE_ACM_COM]),
    DMS_DEV_ITEM(DMS_PORT_CTRL,     UDI_ACM_CTRL_ID,            &g_dmsDevOps[DMS_DEV_TYPE_ACM_COM]),
    DMS_DEV_ITEM(DMS_PORT_PCUI2,    UDI_ACM_SKYTONE_ID,         &g_dmsDevOps[DMS_DEV_TYPE_ACM_COM]),
    DMS_DEV_ITEM(DMS_PORT_NCM_CTRL, UDI_NCM_CTRL_ID,            &g_dmsDevOps[DMS_DEV_TYPE_NCM_CTRL]),
    DMS_DEV_ITEM(DMS_PORT_UART,     UDI_UART_0_ID,              &g_dmsDevOps[DMS_DEV_TYPE_UART]),
    DMS_DEV_ITEM(DMS_PORT_SOCK,     CPM_AT_COMM,                &g_dmsDevOps[DMS_DEV_TYPE_SOCK]),
    DMS_DEV_ITEM(DMS_PORT_MODEM,    UDI_ACM_MODEM_ID,           &g_dmsDevOps[DMS_DEV_TYPE_ACM_MODEM]),

#if (FEATURE_AT_HSUART == FEATURE_ON)
    DMS_DEV_ITEM(DMS_PORT_HSUART,   UDI_HSUART_0_ID,            &g_dmsDevOps[DMS_DEV_TYPE_HSUART]),
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    DMS_DEV_ITEM(DMS_PORT_CMUXAT,   UDI_CMUX_AT_ID,             &g_dmsDevOps[DMS_DEV_TYPE_CMUX]),
    DMS_DEV_ITEM(DMS_PORT_CMUXMDM,  UDI_CMUX_MDM_ID,            &g_dmsDevOps[DMS_DEV_TYPE_CMUX]),
    DMS_DEV_ITEM(DMS_PORT_CMUXEXT,  UDI_CMUX_EXT_ID,            &g_dmsDevOps[DMS_DEV_TYPE_CMUX]),
    DMS_DEV_ITEM(DMS_PORT_CMUXGPS,  UDI_CMUX_GPS_ID,            &g_dmsDevOps[DMS_DEV_TYPE_CMUX]),
#endif

    DMS_DEV_ITEM(DMS_PORT_NCM_DATA, UDI_NCM_NDIS_ID,            &g_dmsDevOps[DMS_DEV_TYPE_NCM_DATA]),
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    DMS_DEV_ITEM(DMS_PORT_NCM_DATA1, UDI_NCM_NDIS1_ID,          &g_dmsDevOps[DMS_DEV_TYPE_NCM_DATA]),
    DMS_DEV_ITEM(DMS_PORT_NCM_DATA2, UDI_NCM_NDIS2_ID,          &g_dmsDevOps[DMS_DEV_TYPE_NCM_DATA]),
    DMS_DEV_ITEM(DMS_PORT_NCM_DATA3, UDI_NCM_NDIS3_ID,          &g_dmsDevOps[DMS_DEV_TYPE_NCM_DATA]),
#endif
};

struct DMS_DevInfo* DMS_DEV_GetDevEntityByPortId(DMS_PortIdUint16 portId)
{
    return &g_dmsDevInfo[portId];
}

const struct DMS_PortFuncTbl* DMS_DEV_GetDevPortTblByPortId(DMS_PortIdUint16 portId)
{
    return g_dmsDevInfo[portId].portFuncTbl;
}

VOS_INT32 DMS_DEV_GetDevHandlerByPortId(DMS_PortIdUint16 portId)
{
    return g_dmsDevInfo[portId].handle;
}

