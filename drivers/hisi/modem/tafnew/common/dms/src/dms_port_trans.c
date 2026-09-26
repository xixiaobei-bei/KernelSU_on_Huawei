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

#include "vos_pid_def.h"
#include "imm_interface.h"
#include "dms_debug.h"
#include "dms_port_i.h"
#include "dms_port.h"
#include "dms_dev_i.h"
#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_DMS_PORT_TRANS_C

DMS_PortIdUint16 g_dmsCbtPortId = DMS_PORT_BUTT;

#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)

STATIC VOS_UINT32 DMS_Port_RcvRawDataProc(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len)
{
    const struct DMS_ClientInfo    *clientInfo = VOS_NULL_PTR;
    DMS_PORT_LOGE(portId, "enter");
    clientInfo = DMS_PORT_GetClientInfo(portId);

    if (clientInfo->RcvRawData == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "DMS_PortCom_RcvRawDataProc RcvRawData NULL PTR.");
        return VOS_ERR;
    }

    return clientInfo->RcvRawData(data, len);
}
#endif

STATIC DMS_PortIdUint16 DMS_PORT_GetCbtPortId(VOS_VOID)
{
    return g_dmsCbtPortId;
}

VOS_VOID DMS_PORT_SetCbtPortId(DMS_PortIdUint16 portId)
{
    g_dmsCbtPortId = portId;
}

STATIC VOS_UINT32 DMS_PORT_IsInputDataParaValid(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len)
{
    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT || data == VOS_NULL_PTR || len == 0) {
        DMS_LOGE("portid: %d, len: %d .", portId, len);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

STATIC VOS_BOOL DMS_PORT_IsEscapeSequence(VOS_UINT8 *data, VOS_UINT32 len)
{
    if (len == 3) {
        if (data[0] == '+' && data[1] == '+' && data[2] == '+') {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

STATIC VOS_BOOL DMS_PORT_IsZcDataValid(IMM_Zc *data)
{
    VOS_UINT8             *zcData = VOS_NULL_PTR;
    VOS_UINT16             zcDatalen;

    DMS_LOGI("enter.");
    zcData     = IMM_ZcGetDataPtr(data);
    zcDatalen  = (VOS_UINT16)IMM_ZcGetUsedLen(data);

    if  (zcData == VOS_NULL_PTR || zcDatalen == 0) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

STATIC VOS_INT32  DMS_PORT_RcvPrivDataOnCmdMode(DMS_PortIdUint16 portId, IMM_Zc *immZc)
{
    struct DMS_ClientInfo *clientInfo = VOS_NULL_PTR;
    VOS_UINT8             *zcData = VOS_NULL_PTR;
    VOS_INT32              ret;
    VOS_UINT16             zcDatalen;

    DMS_LOGI("enter.");

    clientInfo = DMS_PORT_GetClientInfo(portId);
    zcData     = IMM_ZcGetDataPtr(immZc);
    zcDatalen  = (VOS_UINT16)IMM_ZcGetUsedLen(immZc);

    if (clientInfo->RcvCmdData == NULL) {
        DMS_PORT_LOGE(portId, "RcvAtCmd is NULL");
        ret = VOS_ERR;
    } else {
        ret = clientInfo->RcvCmdData(clientInfo->id, zcData, zcDatalen);
    }

    IMM_ZcFree(immZc);
    return ret;
}

STATIC VOS_INT32  DMS_PORT_RcvPrivDataOnDataMode(DMS_PortIdUint16 portId, IMM_Zc *immZc)
{
    const struct DMS_ClientInfo    *clientInfo = VOS_NULL_PTR;
    VOS_UINT8                      *zcData = VOS_NULL_PTR;
    struct DMS_MscInfo              msc;
    VOS_INT32                       ret;
    VOS_UINT32                      zcDatalen;
    DMS_PortDataModeUint8           dataMode;

    DMS_LOGI("enter.");

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    clientInfo  = DMS_PORT_GetClientInfo(portId);
    dataMode    = DMS_PORT_GetDataMode(portId);
    zcData      = IMM_ZcGetDataPtr(immZc);
    zcDatalen   = IMM_ZcGetUsedLen(immZc);

    if (dataMode == DMS_PORT_DATA_PPP || dataMode == DMS_PORT_DATA_PPP_RAW) {
        /* 检查是否是modem口的escape sequence包，如果是就直接处理 */
        if (DMS_PORT_IsEscapeSequence(zcData, zcDatalen) == VOS_TRUE && portId == DMS_PORT_MODEM) {
            /* 模拟触发dtr管脚信号变化 */
            msc.dtrFlag = VOS_TRUE;
            msc.dtr = DMS_IO_LEVEL_LOW;
            DMS_SendDmsMscInd(DMS_PORT_MODEM, &msc);

            IMM_ZcFree(immZc);
            return VOS_OK;
        }

        if (clientInfo->RcvPppData == NULL) {
            DMS_PORT_LOGE(portId, "RcvPppData is NULL");
            IMM_ZcFree(immZc);
            return VOS_ERR;
        }

        ret = (VOS_INT32)clientInfo->RcvPppData(clientInfo->pppId, immZc);
        return ret;
    }

#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    if (dataMode == DMS_PORT_DATA_RAW_DATA) {
        ret = DMS_Port_RcvRawDataProc(portId, zcData, zcDatalen);
        IMM_ZcFree(immZc);
        return ret;
    }
#endif

    if (dataMode == DMS_PORT_DATA_OM) {
        if (clientInfo->RcvRawData == NULL) {
            DMS_PORT_LOGE(portId, "RcvCbtData is NULL");
            IMM_ZcFree(immZc);
            return VOS_ERR;
        }

        ret = (VOS_INT32)clientInfo->RcvRawData(zcData, zcDatalen);
        IMM_ZcFree(immZc);

        return ret;
    }

    DMS_PORT_LOGE(portId, "dataMode abnormal %d", dataMode);
    IMM_ZcFree(immZc);

    return VOS_ERR;
}

STATIC VOS_INT32 DMS_PORT_RcvDataOnCmdMode(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len)
{
    struct DMS_ClientInfo          *clientInfo = VOS_NULL_PTR;

    clientInfo = DMS_PORT_GetClientInfo(portId);

    if (clientInfo->RcvCmdData == NULL) {
        DMS_PORT_LOGE(portId, "RcvCmdData is NULL");
        return VOS_ERR;
    }

    return clientInfo->RcvCmdData(clientInfo->id, data, len);
}

STATIC VOS_INT32 DMS_PORT_RcvDataOnDataMode(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len)
{
    struct DMS_ClientInfo          *clientInfo = VOS_NULL_PTR;
    DMS_PortIdUint16                cbtPortId;
    VOS_INT32                       ret;

    cbtPortId = DMS_PORT_GetCbtPortId();
    clientInfo = DMS_PORT_GetClientInfo(portId);

    if (portId != cbtPortId) {
        DMS_PORT_LOGE(portId, "port id not equal cbt port id %d.", cbtPortId);
        return VOS_ERR;
    }

    if (DMS_PORT_IsEscapeSequence(data, len) == VOS_TRUE) {
        DMS_SendIntraEvent(ID_DMS_EVENT_ESCAPE_SEQUENCE, portId, VOS_NULL_PTR, 0);
        return VOS_OK;
    }

    if (clientInfo->RcvRawData == NULL) {
        DMS_PORT_LOGE(portId, "RcvCbtData is NULL");
        return VOS_ERR;
    }

    ret = (VOS_INT32)clientInfo->RcvRawData(data, len);
    return ret;
}

STATIC IMM_Zc* DMS_PORT_TransSys2ImmZc(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len)
{
    IMM_Zc *immZc = VOS_NULL_PTR;

    /* 申请IMM_Zc结构 */
    immZc = IMM_ZcStaticAlloc((VOS_UINT16)len);
    if (immZc == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "alloc skb failed");
        return VOS_NULL_PTR;
    }

    IMM_ZcPut(immZc, len);
    /* 拷贝内容 */
    if (memcpy_s(IMM_ZcGetDataPtr(immZc), IMM_ZcGetUsedLen(immZc), data, len) != EOK) {
        DMS_PORT_LOGE(portId, "memcpy failed");
        IMM_ZcFreeAny(immZc);
        return VOS_NULL_PTR;
    }

    return immZc;
}

STATIC VOS_INT32 DMS_PORT_RcvDataOnPppMode(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len)
{
    struct DMS_ClientInfo *clientInfo = VOS_NULL_PTR;
    IMM_Zc *immZc = VOS_NULL_PTR;
    VOS_INT32 ret;

    clientInfo = DMS_PORT_GetClientInfo(portId);

    if (clientInfo->RcvPppData == NULL) {
        DMS_PORT_LOGE(portId, "RcvPppData is NULL");
        return VOS_ERR;
    }

    /* 转为IMM_Zc结构 */
    immZc = DMS_PORT_TransSys2ImmZc(portId, data, len);
    if (immZc == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "trans ImmZc fail");
        return VOS_ERR;
    }

    ret = (VOS_INT32)clientInfo->RcvPppData(clientInfo->pppId, immZc);
    return ret;
}

VOS_INT32  DMS_PORT_RcvData(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT32 len)
{
    struct DMS_Port_ModeInfo       *modeInfo   = VOS_NULL_PTR;
    VOS_UINT32                      validFlg;

    DMS_PORT_LOGI(portId, "enter.");

    validFlg = DMS_PORT_IsInputDataParaValid(portId, data, len);

    if (validFlg == VOS_FALSE) {
        DMS_LOGE("data invalid.");
        return VOS_ERR;
    }

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    modeInfo   = DMS_PORT_GetModeInfo(portId);
    DMS_PORT_LOGI(portId, "mode:%d dataMode:%d", modeInfo->mode, modeInfo->dataMode);
    if (modeInfo->mode == DMS_PORT_MODE_CMD) {
        return DMS_PORT_RcvDataOnCmdMode(portId, data, len);
    }

    if (modeInfo->mode != DMS_PORT_MODE_ONLINE_DATA) {
        return VOS_ERR;
    }

    if (modeInfo->dataMode == DMS_PORT_DATA_OM) {
        return DMS_PORT_RcvDataOnDataMode(portId, data, len);
    }

#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
    if (modeInfo->dataMode == DMS_PORT_DATA_RAW_DATA) {
        return DMS_Port_RcvRawDataProc(portId, data, len);
    }
#endif

    if ((modeInfo->dataMode == DMS_PORT_DATA_PPP_RAW) ||
        (modeInfo->dataMode == DMS_PORT_DATA_PPP)) {
        return DMS_PORT_RcvDataOnPppMode(portId, data, len);
    }

    return VOS_ERR;
}

VOS_INT32  DMS_PORT_RcvPrivData(DMS_PortIdUint16 portId, IMM_Zc *data)
{
    VOS_INT32                       ret;
    DMS_PortModeUint8               mode;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE ||
        DMS_PORT_IsZcDataValid(data) == VOS_FALSE) {
        DMS_LOGE("port not inited or zcdata not valid");
        IMM_ZcFree(data);
        return VOS_ERR;
    }

    mode = DMS_PORT_GetMode(portId);

    if (mode == DMS_PORT_MODE_CMD || mode == DMS_PORT_MODE_ONLINE_CMD ) {
        ret = DMS_PORT_RcvPrivDataOnCmdMode(portId, data);
        return ret;
    }

    if (mode == DMS_PORT_MODE_ONLINE_DATA) {
        ret = DMS_PORT_RcvPrivDataOnDataMode(portId, data);
        return ret;
    }

    DMS_PORT_LOGE(portId, "mode abnormal");
    IMM_ZcFree(data);
    return VOS_ERR;
}

VOS_UINT32 DMS_PORT_SendData(DMS_PortIdUint16 portId, VOS_UINT8 *data, VOS_UINT16 len)
{
    struct DMS_PortInfo    *portInfo = VOS_NULL_PTR;
    VOS_UINT32              validFlg;

    validFlg = DMS_PORT_IsInputDataParaValid(portId, data, (VOS_UINT32)len);

    if (validFlg == VOS_FALSE) {
        DMS_LOGE("data invalid.");
        return VOS_ERR;
    }

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* hsuart口默认走异步发送接口，hsuart的同步接口由调用方显示调用 */
    if (portId != DMS_PORT_UART) {
        return DMS_DEV_SendDataAsync(portInfo->devInfo, data, len);
    } else {
        return DMS_DEV_SendDataSync(portInfo->devInfo, data, len);
    }
}

VOS_UINT32 DMS_PORT_SendPppPacket(VOS_ULONG userData, IMM_Zc *dataBuf)
{
    struct DMS_PortInfo      *portInfo = VOS_NULL_PTR;
    DMS_PortIdUint16          portId;

    DMS_LOGI("enter.");

    portId    = (DMS_PortIdUint16)userData;

    if (dataBuf == VOS_NULL_PTR) {
        DMS_LOGE("data is null.");
        return VOS_ERR;
    }

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE ||
        DMS_PORT_IsZcDataValid(dataBuf) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        IMM_ZcFree(dataBuf);
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);

    if (portInfo->modeInfo.mode != DMS_PORT_MODE_ONLINE_DATA) {
        DMS_PORT_LOGE(portId, "mode abnormal, %d.", portInfo->modeInfo.mode);
        IMM_ZcFree(dataBuf);
        return VOS_ERR;
    }

    if (portInfo->modeInfo.dataMode != DMS_PORT_DATA_PPP && portInfo->modeInfo.dataMode != DMS_PORT_DATA_PPP_RAW) {
        DMS_PORT_LOGE(portId, "data mode abnormal, %d.", portInfo->modeInfo.dataMode);
        IMM_ZcFree(dataBuf);
        return VOS_ERR;
    }

    if (portInfo->modeInfo.dataState != DMS_PORT_DATA_STATE_START) {
        DMS_PORT_LOGE(portId, "dataState abnormal, %d.", portInfo->modeInfo.dataState);
        IMM_ZcFree(dataBuf);
        return VOS_ERR;
    }

    return DMS_DEV_SendPrivData(portInfo->devInfo, dataBuf);
}


VOS_VOID DMS_PORT_SetMode(DMS_PortIdUint16 portId, DMS_PortModeUint8 mode)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("port id abnormal, %d.", portId);
        return;
    }

    if (mode >= DMS_PORT_MODE_BUTT) {
        DMS_PORT_LOGE(portId, "port mode abnormal, %d.", mode);
        return;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);
    modeInfo->mode = mode;
}

VOS_VOID DMS_PORT_SetDataMode(DMS_PortIdUint16 portId, DMS_PortDataModeUint8 dataMode)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("port id abnormal, %d.", portId);
        return;
    }

    /* datamode存在设置为butt的场景，所以此处仅判断大于，不判断等于 */
    if (dataMode > DMS_PORT_DATA_BUTT) {
        DMS_PORT_LOGE(portId, "port dataMode abnormal, %d.", dataMode);
        return;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);
    modeInfo->dataMode = dataMode;
}

VOS_VOID DMS_PORT_SetDataState(DMS_PortIdUint16 portId, DMS_PortDataStateUint8 dataState)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("port id abnormal, %d.", portId);
        return;
    }

    if (dataState >= DMS_PORT_DATA_STATE_BUTT) {
        DMS_PORT_LOGE(portId, "port dataState abnormal, %d.", dataState);
        return;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);
    modeInfo->dataState = dataState;
}

DMS_PortModeUint8 DMS_PORT_GetMode(DMS_PortIdUint16 portId)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("port id abnormal, %d.", portId);
        return DMS_PORT_MODE_BUTT;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);
    return modeInfo->mode;
}

DMS_PortDataModeUint8 DMS_PORT_GetDataMode(DMS_PortIdUint16 portId)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("port id abnormal, %d.", portId);
        return DMS_PORT_DATA_BUTT;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);
    return modeInfo->dataMode;
}

DMS_PortDataStateUint8 DMS_PORT_GetDataState(DMS_PortIdUint16 portId)
{
    struct DMS_Port_ModeInfo  *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("port id abnormal, %d.", portId);
        return DMS_PORT_DATA_STATE_BUTT;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);
    return modeInfo->dataState;
}

VOS_BOOL DMS_PORT_IsPppPacketTransMode(DMS_PortIdUint16 portId)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (portId >= DMS_PORT_BUTT) {
        DMS_LOGE("port id abnormal, %d.", portId);
        return VOS_FALSE;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);

    if (modeInfo->mode != DMS_PORT_MODE_ONLINE_DATA) {
        DMS_PORT_LOGE(portId, "mode: %d, not data mode.", modeInfo->mode);
        return VOS_FALSE;
    }

    if (modeInfo->dataMode != DMS_PORT_DATA_PPP_RAW && modeInfo->dataMode != DMS_PORT_DATA_PPP) {
        DMS_PORT_LOGE(portId, "dataMode: %d, incorrect data mode.", modeInfo->dataMode);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)

STATIC VOS_VOID DMS_PORT_RegRawPacketRcvFunc(DMS_PortIdUint16 portId, DMS_PORT_RCV_RAW_DATA_FUNC func)
{
    struct DMS_ClientInfo *clientInfo = VOS_NULL_PTR;

    DMS_PORT_LOGI(portId, "enter.");

    clientInfo = DMS_PORT_GetClientInfo(portId);
    clientInfo->RcvRawData = func;
}

VOS_VOID DMS_PORT_SwitchRawDataMode(DMS_PortIdUint16 portId, DMS_PORT_RCV_RAW_DATA_FUNC func)
{
    VOS_BOOL flag = VOS_FALSE;
    DMS_PORT_SetMode(portId, DMS_PORT_MODE_ONLINE_DATA);
    DMS_PORT_SetDataMode(portId, DMS_PORT_DATA_RAW_DATA);

    DMS_PORT_LOGI(portId, "enter.");

#if (FEATURE_AT_HSUART == FEATURE_ON)
    flag =  (portId == DMS_PORT_HSUART || portId == DMS_PORT_UART) ? VOS_TRUE : VOS_FALSE;
#else
    flag =  (portId == DMS_PORT_UART) ? VOS_TRUE : VOS_FALSE;
#endif

    if ((flag == VOS_TRUE) && (DMS_PORT_GetUartLineCtrl()->dcdMode == DMS_PORT_UART_DCD_MODE_CONNECT_ON)) {
        DMS_PORT_AssertDcd(portId);
    }

    if (portId == DMS_PORT_MODEM) {
        DMS_PORT_AssertDcd(portId);
    }

    DMS_PORT_RegRawPacketRcvFunc(portId, func);

#if (FEATURE_AT_HSUART == FEATURE_ON)
    (VOS_VOID)DMS_DEV_EnableEscapeSequenceDetect(DMS_PORT_GetPortInfo(portId)->devInfo, VOS_FALSE);
#endif
}

 VOS_VOID DMS_PORT_RawDataModeToCmdMode(DMS_PortIdUint16 portId)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;
    VOS_BOOL flag = VOS_FALSE;

    DMS_PORT_LOGI(portId, "enter.");
    (VOS_VOID)DMS_DEV_EnableEscapeSequenceDetect(DMS_PORT_GetPortInfo(portId)->devInfo, VOS_FALSE);
    if (portId == DMS_PORT_MODEM) {
        DMS_PORT_DeassertDcd(portId);
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
     flag = (portId == DMS_PORT_HSUART || portId == DMS_PORT_UART) ? VOS_TRUE : VOS_FALSE;
#else
     flag = (portId == DMS_PORT_UART) ? VOS_TRUE : VOS_FALSE;
#endif

    if (flag == VOS_TRUE) {
        lineCtrl = DMS_PORT_GetUartLineCtrl();
        if ((DMS_PORT_GetMode(portId) == DMS_PORT_MODE_CMD) && (lineCtrl->dcdMode == DMS_PORT_UART_DCD_MODE_CONNECT_ON)) {
            DMS_PORT_DeassertDcd(portId);
        }
    }
}
#endif

VOS_BOOL DMS_PORT_IsExistCbtPort(VOS_VOID)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;
    DMS_PortIdUint16          cbtPortId;

    DMS_LOGI("enter.");

    cbtPortId = DMS_PORT_GetCbtPortId();

    if (cbtPortId >= DMS_PORT_BUTT) {
        DMS_LOGE("cbt port id abnormal, %d.", cbtPortId);
        return VOS_FALSE;
    }

    modeInfo = DMS_PORT_GetModeInfo(cbtPortId);

    if (modeInfo->mode != DMS_PORT_MODE_ONLINE_DATA || modeInfo->dataMode != DMS_PORT_DATA_OM) {
        DMS_PORT_LOGE(cbtPortId, "mode is %d, datamode is %d.", modeInfo->mode, modeInfo->dataMode);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

STATIC VOS_VOID DMS_PORT_RegCbtDataRcvFunc(DMS_PortIdUint16 portId, CBTCPM_RCV_FUNC rcvFunc)
{
    struct DMS_ClientInfo  *clientInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    clientInfo = DMS_PORT_GetClientInfo(portId);

    /*
     * 整体逻辑如下：
     * 注册函数为空指针时，需要把cbtPortId清除，回调函数清除
     * 注册函数非空指针时，需要保存cbtPortId，同时保存回调函数
     */
    if (rcvFunc != VOS_NULL_PTR) {
        DMS_PORT_SetCbtPortId(portId);
        clientInfo->RcvRawData = rcvFunc;
    } else {
        DMS_PORT_SetCbtPortId(DMS_PORT_BUTT);
        clientInfo->RcvRawData = VOS_NULL_PTR;
    }
}

STATIC VOS_VOID DMS_PORT_RegPppPacketRcvFunc(DMS_PortIdUint16 portId, DMS_PORT_RCV_PPP_DATA_FUNC func, VOS_UINT16 pppId)
{
    struct DMS_ClientInfo *clientInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    clientInfo = DMS_PORT_GetClientInfo(portId);
    clientInfo->RcvPppData = func;
    clientInfo->pppId      = pppId;
}

VOS_VOID DMS_PORT_SwitchToOmDataMode(DMS_PortIdUint16 portId, CBTCPM_RCV_FUNC rcvFunc)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }


    /* 将校准通道的接收函数给AT模块 */
    DMS_PORT_RegCbtDataRcvFunc(portId, rcvFunc);

    modeInfo = DMS_PORT_GetModeInfo(portId);

    /* 将通道模式设置为OM模式 */
    modeInfo->mode      = DMS_PORT_MODE_ONLINE_DATA;
    modeInfo->dataMode  = DMS_PORT_DATA_OM;

    /* 此处dataState先置为STOP，
     * 待DMS_PORT_SwitchToOmDataMode函数调用点ok上报后，数据状态才切换为start;
     * 切换为start的函数参见AT_DisplayResultData
     */
    modeInfo->dataState = DMS_PORT_DATA_STATE_STOP;

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (FEATURE_AT_HSUART == FEATURE_ON)
    (VOS_VOID)DMS_DEV_EnableEscapeSequenceDetect(DMS_PORT_GetPortInfo(portId)->devInfo, VOS_FALSE);
#endif
#endif
}

VOS_VOID DMS_PORT_SwitchToPppDataMode(DMS_PortIdUint16 portId, DMS_PortDataModeUint8 dataMode,
                                      DMS_PORT_RCV_PPP_DATA_FUNC func, VOS_UINT16 pppId)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    if (dataMode != DMS_PORT_DATA_PPP && dataMode != DMS_PORT_DATA_PPP_RAW) {
        DMS_PORT_LOGE(portId, "data mode abnormal ", dataMode);
        return;
    }

    /* 注册ppp包接收函数 */
    DMS_PORT_RegPppPacketRcvFunc(portId, func, pppId);

    modeInfo = DMS_PORT_GetModeInfo(portId);
    modeInfo->mode      = DMS_PORT_MODE_ONLINE_DATA;
    modeInfo->dataMode  = dataMode;

    /* 此处dataState先置为STOP，
     * 待DMS_PORT_SwitchToRawDataMode函数调用点connect上报后，数据状态才切换为start;
     * 切换为start的函数参见AT_DisplayResultData
     */
    modeInfo->dataState = DMS_PORT_DATA_STATE_STOP;

    /*给底软挂接+++检测回调函数*/
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (FEATURE_AT_HSUART == FEATURE_ON)
    (VOS_VOID)DMS_DEV_EnableEscapeSequenceDetect(DMS_PORT_GetPortInfo(portId)->devInfo, VOS_TRUE);
#endif
#endif
}

VOS_VOID DMS_PORT_ResumeCmdMode(DMS_PortIdUint16 portId)
{
    struct DMS_Port_ModeInfo     *modeInfo = VOS_NULL_PTR;
    DMS_PortDataModeUint8         oldDataMode;

    DMS_PORT_LOGI(portId, "enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_PORT_LOGE(portId, "port not inited.");
        return;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);

    oldDataMode = modeInfo->dataMode;

    /*
     * 此处需要先切模式，再清除回调函数，
     * 否则底软上下文接收数据和此处切换逻辑并发时，可能存在空指针调用
     */

    /* 将通道模式恢复为cmd模式 */
    modeInfo->mode      = DMS_PORT_MODE_CMD;
    modeInfo->dataMode  = DMS_PORT_DATA_BUTT;
    modeInfo->dataState = DMS_PORT_DATA_STATE_STOP;

    /* 根据老的模式信息，清除相关函数和全局变量 */
    switch (oldDataMode)
    {
        case DMS_PORT_DATA_PPP_RAW:
        case DMS_PORT_DATA_PPP:
#if (FEATURE_AT_HSUART == FEATURE_ON)
            /*
             * HSUART端口由DATA模式切换到CMD模式:
             * 清除HSUART的缓存队列数据, 防止当前缓存队列满时, 主动上报的命令丢失
             */
            if (portId == DMS_PORT_HSUART) {
                DMS_PORT_FlushTxData(DMS_PORT_HSUART);
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
                (VOS_VOID)DMS_DEV_EnableEscapeSequenceDetect(DMS_PORT_GetPortInfo(portId)->devInfo, VOS_FALSE);
#endif
            }
#endif

            /* PPP包的接收函数注册为空，同时pppId置位0 */
            DMS_PORT_RegPppPacketRcvFunc(portId, VOS_NULL_PTR, 0);

            break;


        case DMS_PORT_DATA_OM:

            DMS_PORT_RegCbtDataRcvFunc(portId, VOS_NULL_PTR);

            break;
#if (FEATURE_IOT_RAW_DATA == FEATURE_ON)
        case DMS_PORT_DATA_RAW_DATA:
            DMS_PORT_RawDataModeToCmdMode(portId);

            break;
#endif

        default:
            DMS_LOGI("incorrect data mode %d.", oldDataMode);
            return;
    }
}

