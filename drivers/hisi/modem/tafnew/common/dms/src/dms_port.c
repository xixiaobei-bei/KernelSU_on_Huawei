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

#include "dms_port.h"
#include "dms_port_i.h"
#include "dms_dev_i.h"
#include "dms_port_cmux.h"
#include "vos_pid_def.h"
#include "mn_client.h"
#include "dms_debug.h"
#include "nv_stru_gucnas.h"
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#include "at_app_vcom_interface.h"
#endif
#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_DMS_PORT_C

struct DMS_PortInfo g_dmsPortInfo[DMS_PORT_BUTT] = {{0}};

STATIC const struct DMS_PortFuncTbl g_dmsPortFuncTbl[] = {
    [DMS_PORT_TYPE_COM] = {
        {.RcvData               = DMS_PORT_RcvData,},
        .RcvConnStatusChgInd    = DMS_PORT_RcvConnStatusChgInd,
        .RcvSwitchGwModeInd     = DMS_PORT_RcvSwitchGwModeInd,
        .RcvConnectInd          = DMS_PORT_RcvConnectInd,
    },

    [DMS_PORT_TYPE_MODEM] = {
        {.RcvPrivData           = DMS_PORT_RcvPrivData,},
        .RcvMscInd              = DMS_PORT_RcvMsc,
        .RcvConnectInd          = DMS_PORT_RcvConnectInd,
    },
#if (FEATURE_AT_HSUART == FEATURE_ON)
    [DMS_PORT_TYPE_HSUART] = {
        {.RcvPrivData           = DMS_PORT_RcvPrivData,},
        .RcvMscInd              = DMS_PORT_RcvMsc,
        .RcvEscapeSequence      = DMS_PORT_RcvEscapeSequence,
        .RcvWaterMarkInd        = DMS_PORT_RcvWaterMarkInd,
        .RcvConnectInd          = DMS_PORT_RcvConnectInd,
    },
#endif
    [DMS_PORT_TYPE_NDIS_DATA] = {
        .RcvConnectInd          = DMS_PORT_RcvConnectInd,
    },
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    [DMS_PORT_TYPE_CMUX] = {
        {.RcvPrivData           = DMS_PORT_RcvPrivData,},
        .RcvCmuxCloseUart       = DMS_PORT_SwitchCmux2Uart,
        .RcvMscInd              = DMS_PORT_RcvMsc,
        .RcvMscModeRead         = DMS_PORT_RcvMscModeRead,
        .RcvEscapeSequence      = DMS_PORT_RcvEscapeSequence,
    },
#endif
};

struct DMS_PORT_UartCtx g_uartCtx;

struct DMS_PortInfo* DMS_PORT_GetPortInfo(DMS_PortIdUint16 portId)
{
    return &g_dmsPortInfo[portId];
}

struct DMS_ClientInfo* DMS_PORT_GetClientInfo(DMS_PortIdUint16 portId)
{
    return &g_dmsPortInfo[portId].clientInfo;
}

struct DMS_Port_ModeInfo* DMS_PORT_GetModeInfo(DMS_PortIdUint16 portId)
{
    return &(g_dmsPortInfo[portId].modeInfo);
}

STATIC const struct DMS_PortFuncTbl* DMS_PORT_GetPortFuncTbl(DMS_PortTypeUint8 portType)
{
    return &(g_dmsPortFuncTbl[portType]);
}

struct DMS_PORT_UartLineCtrl* DMS_PORT_GetUartLineCtrl(VOS_VOID)
{
    return &(g_uartCtx.uartLineCtrl);
}

VOS_UINT32 DMS_PORT_GetUartWaterMarkFlg(VOS_VOID)
{
    return g_uartCtx.txWmHighFlg;
}

VOS_VOID DMS_PORT_SetUartWaterMarkFlg(VOS_UINT32 txWmHighFlg)
{
    g_uartCtx.txWmHighFlg = txWmHighFlg;
}

STATIC VOS_VOID DMS_PORT_ReadPortDebugCfgNV(VOS_VOID)
{
    TAF_NV_PortDebugCfg portDebugNVCfg;

    memset_s(&portDebugNVCfg, sizeof(portDebugNVCfg), 0x00, sizeof(portDebugNVCfg));

    /* 读取NV项 */
    if (NV_OK != TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_DMS_DEBUG_CFG, &portDebugNVCfg,
                                   sizeof(TAF_NV_PortDebugCfg))) {
        return;
    }

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    APP_VCOM_SendDebugNvCfg(portDebugNVCfg.appVcomPortIdMask1, portDebugNVCfg.appVcomPortIdMask2,
                            portDebugNVCfg.appVcomDebugLevel);
#endif
    DMS_SendPortDebugNvCfg(portDebugNVCfg.dmsDebugLevel, portDebugNVCfg.dmsPortIdMask1, portDebugNVCfg.dmsPortIdMask2);
}

STATIC VOS_VOID DMS_PORT_InitUartCtx(VOS_VOID)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;

    lineCtrl = DMS_PORT_GetUartLineCtrl();

    (VOS_VOID)memset_s(lineCtrl, sizeof(*lineCtrl), 0x00, sizeof(*lineCtrl));

    lineCtrl->dcdMode = DMS_PORT_UART_DEFAULT_DCD_MODE;
    lineCtrl->dtrMode = DMS_PORT_UART_DEFAULT_DTR_MODE;
    lineCtrl->dsrMode = DMS_PORT_UART_DEFAULT_DSR_MODE;

    DMS_PORT_SetUartWaterMarkFlg(VOS_FALSE);
}

STATIC DMS_PortTypeUint8 DMS_PORT_GetPortTypeByPortId(DMS_PortIdUint16 portId)
{
    switch (portId) {
        case DMS_PORT_MODEM:
            return DMS_PORT_TYPE_MODEM;

#if (FEATURE_AT_HSUART == FEATURE_ON)
        case DMS_PORT_HSUART:
            return DMS_PORT_TYPE_HSUART;
#endif

        case DMS_PORT_NCM_DATA:
#if (FEATURE_MULTI_NCM == FEATURE_ON)
        case DMS_PORT_NCM_DATA1:
        case DMS_PORT_NCM_DATA2:
        case DMS_PORT_NCM_DATA3:
#endif
            return DMS_PORT_TYPE_NDIS_DATA;

#if (FEATURE_IOT_CMUX == FEATURE_ON)
        case DMS_PORT_CMUXAT:
        case DMS_PORT_CMUXMDM:
        case DMS_PORT_CMUXEXT:
        case DMS_PORT_CMUXGPS:
            return DMS_PORT_TYPE_CMUX;
#endif
        default:
            return DMS_PORT_TYPE_COM;
    }
}

MODULE_EXPORTED VOS_VOID DMS_PORT_Init(VOS_VOID)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;
    DMS_PortIdUint16     portId;
    VOS_UINT32           i;

    DMS_LOGI("enter.");

    DMS_PORT_ReadPortDebugCfgNV();

    DMS_PORT_InitUartCtx();
    DMS_PORT_InitSubscripEventInfo();
    DMS_PORT_InitConnectEventInfo();
    DMS_PORT_SetCbtPortId(DMS_PORT_BUTT);

    for (i = 0; i < DMS_PORT_BUTT; i++) {
        portId   = (DMS_PortIdUint16)i;
        portInfo = DMS_PORT_GetPortInfo(portId);
        portInfo->portId    = portId;
        portInfo->connState = DMS_CONNECT_BUTT;

        /* 其中modemstatus、pppid不做显示初始化，全局变量默认初始化为0 */
        portInfo->modeInfo.mode      = DMS_PORT_MODE_CMD;
        portInfo->modeInfo.dataMode  = DMS_PORT_DATA_BUTT;
        portInfo->modeInfo.dataState = DMS_PORT_DATA_STATE_STOP;

        portInfo->portType = DMS_PORT_GetPortTypeByPortId(portId);
        portInfo->devInfo = DMS_DEV_InitCtx(portId, DMS_PORT_GetPortFuncTbl(portInfo->portType));
    }

    DMS_DEV_InitDev();
}

VOS_UINT32 DMS_PORT_RegClientInfo(DMS_PortIdUint16 portId, struct DMS_ClientInfo *clientInfo)
{
    struct DMS_ClientInfo *portClientInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("portId Abnormal, %d", portId);
        return VOS_ERR;
    }

    if (clientInfo == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "clientInfo is null.");
        return VOS_ERR;
    }

    if (clientInfo->type >= DMS_CLIENT_TYPE_BUTT) {
        DMS_PORT_LOGE(portId, "clientType Abnormal, %d", clientInfo->type);
        return VOS_ERR;
    }

    if (clientInfo->id >= AT_CLIENT_ID_BUTT) {
        DMS_PORT_LOGE(portId, "clientId Abnormal, %d", clientInfo->id);
        return VOS_ERR;
    }

    portClientInfo = DMS_PORT_GetClientInfo(portId);
    portClientInfo->type       = clientInfo->type;
    portClientInfo->id         = clientInfo->id;
    portClientInfo->RcvCmdData = clientInfo->RcvCmdData;

    return VOS_OK;
}

VOS_UINT32 DMS_PORT_IsPortInited(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("portId:%d is invalid.", portId);
        return VOS_FALSE;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);

    if (portInfo->portId != portId || portInfo->devInfo == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "Get PortId is %d .", portInfo->portId);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_VOID DMS_PORT_OpenPort(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo    *portInfo = VOS_NULL_PTR;
    VOS_UINT32              ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("open the port after port inited.");
        return;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    ret = DMS_DEV_Open(portInfo->devInfo);

    if (ret != VOS_OK) {
        DMS_PORT_LOGE(portId, "open the port failed.");
    }
}

VOS_VOID DMS_PORT_ClosePort(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo    *portInfo = VOS_NULL_PTR;
    VOS_UINT32              ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("closen the port after port inited.");
        return;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    ret = DMS_DEV_Close(portInfo->devInfo);

    if (ret != VOS_OK) {
        DMS_PORT_LOGE(portId, "close the port failed.");
    }
}

DMS_PortIdUint16 DSM_GetNcmDataPortByUdi(VOS_UINT32 udiId)
{
    DMS_PortIdUint16 dmsPortId = DMS_PORT_NCM_DATA;
    switch (udiId) {
        case UDI_NCM_NDIS_ID:
            dmsPortId = DMS_PORT_NCM_DATA;
            break;
#if (FEATURE_MULTI_NCM == FEATURE_ON)
        case UDI_NCM_NDIS1_ID:
            dmsPortId = DMS_PORT_NCM_DATA1;
            break;
        case UDI_NCM_NDIS2_ID:
            dmsPortId = DMS_PORT_NCM_DATA2;
            break;
        case UDI_NCM_NDIS3_ID:
            dmsPortId = DMS_PORT_NCM_DATA3;
            break;
#endif
        default:
            dmsPortId = DMS_PORT_NCM_DATA;
            break;
    }
    return dmsPortId;
}

VOS_VOID DMS_PORT_RcvConnStatusChgInd(DMS_PortIdUint16 portId, DMS_NcmStatusUint32 status, VOS_UINT32 udiId)
{
    struct DMS_NcmConnBreakInfo info;

    DMS_LOGI("enter.");
    (VOS_VOID)memset_s(&info, sizeof(info), 0, sizeof(info));

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    if (status == DMS_NCM_IOCTL_STUS_BREAK) {
        info.ncmDataPortId = DSM_GetNcmDataPortByUdi(udiId);
        DMS_SendIntraEvent(ID_DMS_EVENT_NCM_CONN_BREAK, portId, (VOS_VOID *)&info, sizeof(info));
    }
}

VOS_VOID DMS_PORT_RcvSwitchGwModeInd(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    DMS_SendIntraEvent(ID_DMS_EVENT_SWITCH_GW_MODE, portId, VOS_NULL_PTR, 0);
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_VOID DMS_PORT_RcvEscapeSequence(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    DMS_SendIntraEvent(ID_DMS_EVENT_ESCAPE_SEQUENCE, portId, VOS_NULL_PTR, 0);
}

VOS_VOID DMS_PORT_RcvWaterMarkInd(DMS_PortIdUint16 portId, DMS_WaterLevelUint32 level)
{
    VOS_UINT32              txWmHighFlg;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    /*
     * (1) 更新TX高水线标识
     * (2) 如果TX达到低水线, 发送低水线内部消息, 处理相关流程
     */
    txWmHighFlg = (level == DMS_LEVEL_HIGH) ? VOS_TRUE : VOS_FALSE;
    DMS_PORT_SetUartWaterMarkFlg(txWmHighFlg);

    if (level == DMS_LEVEL_LOW) {
        DMS_SendIntraEvent(ID_DMS_EVENT_LOW_WATER_MARK, portId, VOS_NULL_PTR, 0);
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
        IOT_VCOM_WaterMarkProc();
#endif
    }
}
#endif

STATIC VOS_VOID DMS_PORT_AddConnectEvent(DMS_PortIdUint16 portId, VOS_UINT8 connState)
{
    struct DMS_PORT_ConnectEventNode *node = VOS_NULL_PTR;
    struct DMS_PORT_ConnectEventInfo *eventInfo = VOS_NULL_PTR;
    VOS_ULONG lockLevel = 0;

    eventInfo = DMS_PORT_GetConnectEventInfo();

    /* 申请事件节点内存 */
    node = (struct DMS_PORT_ConnectEventNode *)VOS_MemAlloc(PS_PID_DMS, DYNAMIC_MEM_PT,
        sizeof(struct DMS_PORT_ConnectEventNode));
    if (node == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "VOS_MemAlloc Failed!");
        return;
    }

    memset_s(node, sizeof(*node), 0x00, sizeof(*node));
    node->portId = portId;
    node->connState = connState;

    /* 将节点增加到事件链表队尾中 */
    VOS_SpinLockIntLock(&(eventInfo->lock), lockLevel);
    msp_list_add_tail(&node->list, &eventInfo->list);
    VOS_SpinUnlockIntUnlock(&(eventInfo->lock), lockLevel);
}

VOS_VOID DMS_PORT_RcvConnectInd(DMS_PortIdUint16 portId, VOS_UINT8 connState)
{
    VOS_UINT32 taskId = DMS_GetTaskId();

    DMS_LOGI("enter.");

    /* 连接状态事件入队 */
    DMS_PORT_AddConnectEvent(portId, connState);

    /* 触发事件 */
    if (taskId != VOS_NULL_LONG) {
        (VOS_VOID)VOS_EventWrite(taskId, DMS_EVENT_READY_CB_IND);
    }
}

VOS_VOID DMS_PORT_ProcConnectEvent(DMS_PortIdUint16 portId, VOS_UINT8 connState)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    /* 需要关注下具体端口 */
    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    /* 检查更新连接状态 */
    if (portInfo->connState == connState) {
        DMS_PORT_LOGW(portId, "connState not change, is %d.", connState);
        return;
    }

#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    /* 为保证拔插USB后，上位机、用户态、kernel状态保持一致，用户态透传模式下忽略连接状态变更 */
    if (portInfo->modeInfo.dataMode == DMS_PORT_DATA_RAW_DATA) {
        DMS_LOGI("raw data mode ignore connect event");
        return;
    }
#endif

    portInfo->connState = connState;

    /* 端口连上 */
    if (connState == DMS_CONNECT) {
        portInfo->modeInfo.mode      = DMS_PORT_MODE_CMD;
        portInfo->modeInfo.dataMode  = DMS_PORT_DATA_BUTT;
        portInfo->modeInfo.dataState = DMS_PORT_DATA_STATE_STOP;

        DMS_PORT_OpenPort(portId);
    } else {
        /* usb断开事件只需要上报一次，当前仅在modem port上上报，其他端口不上报 */
        if (portId == DMS_PORT_MODEM) {
            DMS_SendIntraEvent(ID_DMS_EVENT_USB_DISCONNECT, portId, VOS_NULL_PTR, 0);
        }

        DMS_PORT_ClosePort(portId);
    }
}

VOS_UINT32 DMS_PORT_SetIpv6Dns(DMS_PortIdUint16 portId, VOS_UINT8 *ipv6DnsInfo, VOS_UINT32 len)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_NcmIpv6Dns    dns;

    DMS_LOGI("enter.");

    dns.ipv6DnsInfo = ipv6DnsInfo;
    dns.length      = len;

    if (ipv6DnsInfo == VOS_NULL_PTR || len != DMS_NCM_IPV6_DNS_LEN) {
        DMS_LOGE("dns info abnormal, %d.", len);
        return VOS_ERR;
    }

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_SetIpv6Dns(portInfo->devInfo, &dns);
}

VOS_UINT32 DMS_PORT_ChangeFlowCtrl(DMS_PortIdUint16 portId, VOS_UINT32 enableFlg)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_ChangeFlowCtrl(portInfo->devInfo, enableFlg);
}

VOS_UINT32 DMS_PORT_ChangeConnSpeed(DMS_PortIdUint16 portId, VOS_UINT32 downBitrate, VOS_UINT32 upBitrate)
{
    struct DMS_PortInfo               *portInfo  = VOS_NULL_PTR;
    struct DMS_ConnSpeed               connSpeed;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    connSpeed.upSpeed   = upBitrate;
    connSpeed.downSpeed = downBitrate;

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_ChangeConnSpeed(portInfo->devInfo, &connSpeed);
}

VOS_UINT32 DMS_PORT_ChangeConnLinkState(DMS_PortIdUint16 portId, VOS_UINT32 linkStatus)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_ChangeConnLinkState(portInfo->devInfo, linkStatus);
}

VOS_INT32 DMS_PORT_GetPortHandle(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;
    VOS_INT32 handle;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERROR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    handle = DMS_DEV_GetHandle(portInfo->devInfo);
    return handle;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_UINT32 DMS_PORT_ConfigFlowCtrl(DMS_PortIdUint16 portId, VOS_UINT32 rtsFlag, VOS_UINT32 ctsFlag)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_ConfigFlowCtrl(portInfo->devInfo, rtsFlag, ctsFlag);
}

VOS_UINT32 DMS_PORT_SetCharFrame(DMS_PortIdUint16 portId, VOS_UINT32 udiDataLen, VOS_UINT32 udiStpLen, VOS_UINT32 udiParity)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_SetCharFrame(portInfo->devInfo, udiDataLen, udiStpLen, udiParity);
}

VOS_UINT32 DMS_PORT_SetBaud(DMS_PortIdUint16 portId, VOS_UINT32 baudRate)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_SetBaud(portInfo->devInfo, baudRate);
}

VOS_UINT32 DMS_PORT_SetACShell(DMS_PortIdUint16 portId, VOS_UINT32 mode)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    return DMS_DEV_SetACShell(portInfo->devInfo, mode);
}

VOS_UINT32 DMS_PORT_FlushTxData(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo    *portInfo = VOS_NULL_PTR;
    VOS_UINT32              ret;

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);
    ret = DMS_DEV_FlushTxData(portInfo->devInfo);

    return ret;
}

VOS_UINT32 DMS_PORT_IsHsuartBasePort(DMS_PortIdUint16 portId)
{
    if (portId == DMS_PORT_HSUART) {
        return VOS_TRUE;
    }

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    if ((DMS_PORT_GetCmuxPhyPort() == DMS_PORT_HSUART) && (DMS_PORT_IsCmuxPort(portId) == VOS_TRUE)) {
        return VOS_TRUE;
    }
#endif

    return VOS_FALSE;
}
#endif

