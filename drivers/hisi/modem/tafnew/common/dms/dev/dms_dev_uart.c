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

#include "securec.h"
#include "dms_dev.h"

#include "dms_dev_i.h"
#include "dms_debug.h"
#include "mdrv_hsuart.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_UART_C

STATIC VOS_VOID DMS_UART_RcvData(VOS_VOID)
{
    const struct DMS_PortFuncTbl   *portTblInfo = VOS_NULL_PTR;
    struct acm_wr_async_info        ctlParam;
    VOS_INT32 handle;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_UART_RD_CB);

    if (DMS_COMM_CBCheck(DMS_PORT_UART) != VOS_OK) {
        return;
    }

    handle = DMS_DEV_GetDevHandlerByPortId(DMS_PORT_UART);

    /* 获取底软上行数据BUFFER */
    ctlParam.virt_addr = VOS_NULL_PTR;
    ctlParam.phy_addr  = VOS_NULL_PTR;
    ctlParam.size      = 0;
    ctlParam.priv      = VOS_NULL_PTR;

    if (mdrv_udi_ioctl(handle, UART_IOCTL_GET_RD_BUFF, &ctlParam) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_UART_RD_BUFF_ERR);
        DMS_LOGE("get buffer failed.");
        return;
    }

    if ((ctlParam.virt_addr == VOS_NULL_PTR) || (ctlParam.size == 0)) {
        DMS_MNTN_IncComStatsInfo(DMS_UART_RD_ADDR_INVALID);
        DMS_LOGE("data buffer error.");
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_UART);
    if (portTblInfo->RcvData == VOS_NULL_PTR) {
        DMS_LOGE("ReadData is not registered.");
        return;
    }

    DMS_PORT_LOGI(DMS_PORT_UART, "data: %s", ctlParam.virt_addr);

    (VOS_VOID)portTblInfo->RcvData(DMS_PORT_UART, (VOS_UINT8 *)ctlParam.virt_addr, ctlParam.size);
}

VOS_UINT32 DMS_UART_Open(struct DMS_DevInfo *dev)
{
    struct udi_open_param   stParam;
    VOS_INT                 handle = UDI_INVALID_HANDLE;

    DMS_PORT_LOGI(dev->portId, "enter.");

    stParam.devid = (enum udi_device_id)dev->devId;

    handle = mdrv_udi_open(&stParam);
    if (handle == UDI_INVALID_HANDLE) {
        DMS_MNTN_IncComStatsInfo(DMS_UART_OPEN_ERR);
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    dev->handle = handle;

    /* 注册UART设备上行数据接收回调 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_READ_CB, DMS_UART_RcvData) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_UART_REG_RD_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg uart read callback failed.\r");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_UART_SendDataSync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_UART_WRT_SYNC);

    if (mdrv_udi_write(dev->handle, data, len) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_UART_WRT_SYNC_ERR);
        DMS_PORT_LOGE(dev->portId, "write data sync is error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_UART_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    /* 使用同步接口发送 */
    if (DMS_UART_SendDataSync(dev, IMM_ZcGetDataPtr(data), IMM_ZcGetUsedLen(data)) != VOS_OK) {
        DMS_PORT_LOGE(dev->portId, "write data sync is error.");
        IMM_ZcFree(data);
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    IMM_ZcFree(data);
    return VOS_OK;
}

VOS_UINT32 DMS_UART_SetBaud(struct DMS_DevInfo *dev, VOS_UINT32 baudRate)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    /* 调用DRV函数设置串口的波特率 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_BAUD, (VOS_VOID *)&baudRate) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_UART_SET_BAUD_ERR);
        DMS_PORT_LOGE(dev->portId, "set baud rate error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

#if (FEATURE_IOT_CMUX == FEATURE_ON)

VOS_UINT32 DMS_UART_ClosePort(struct DMS_DevInfo *dev)
{
    DMS_LOGI("DMS_UART_ClosePort: enter.");
    if (dev->handle != UDI_INVALID_HANDLE) {
        if (mdrv_udi_close(dev->handle) != VOS_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_CLOSE_UART_ERR);
            return VOS_ERR;
        }
    }

    dev->handle   = UDI_INVALID_HANDLE;

    return VOS_OK;
}
#endif

