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

#include "at_app_vcom_interface.h"
#include "dms_dev_i.h"
#include "dms_debug.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_APP_C

STATIC VOS_INT DMS_APP_RcvData(VOS_UINT8 devId, VOS_UINT8 *data, VOS_UINT32 len)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    DMS_PortIdUint16 portId = DMS_PORT_APP + devId;

    DMS_LOGI("enter.");

    if (devId >= APP_VCOM_DEV_INDEX_BUTT) {
        DMS_LOGE("devid: %d incorrect.", devId);
        return VOS_ERROR;
    }

    if (data == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "data is NULL PTR.");
        return VOS_ERROR;
    }

    if (len == 0) {
        DMS_PORT_LOGE(portId, "len is 0.");
        return VOS_ERROR;
    }

    if (DMS_COMM_CBCheck(portId) != VOS_OK) {
        return VOS_ERROR;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo->RcvData == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "ReadData is not registered.");
        return VOS_ERROR;
    }

    return portTblInfo->RcvData(portId, data, len);
}

STATIC VOS_VOID DMS_APP_RcvSwitchGwMode(VOS_VOID)
{
    const struct DMS_PortFuncTbl  *portTblInfo = VOS_NULL_PTR;

    if (DMS_COMM_CBCheck(DMS_PORT_APP) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_APP);

    if (portTblInfo->RcvSwitchGwModeInd == VOS_NULL_PTR) {
        DMS_LOGE("APPVCOM0 RcvSwitchGwModeInd is NULL.");
        return;
    }

    portTblInfo->RcvSwitchGwModeInd(DMS_PORT_APP);
}

VOS_UINT32 DMS_APP_Open(struct DMS_DevInfo *dev)
{
    VOS_INT32  ret;

    DMS_PORT_LOGI(dev->portId, "enter.");

    dev->handle = DMS_INIT_HANDLE;

    if (dev->devId == APP_VCOM_DEV_INDEX_0) {
            /* 注册DRV回调函数指针 */
        if (DRV_USB_NAS_SWITCH_GATEWAY_REGFUNC((USB_NET_DEV_SWITCH_GATEWAY)DMS_APP_RcvSwitchGwMode) == VOS_ERROR) {
            DMS_PORT_LOGE(dev->portId, "DRV_USB_NAS_SWITCH_GATEWAY_REGFUNC Failed.");
        }
    }

    ret = (VOS_INT32)APP_VCOM_REG_DATA_CALLBACK((VOS_UINT8)dev->devId, DMS_APP_RcvData);
    if (ret != 0) {
        DMS_MNTN_IncAppStatsInfo(dev->portId, DMS_APP_REG_RD_CB_ERR);
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_APP_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (APP_VCOM_SEND((VOS_UINT8)dev->devId, data, len) != VOS_OK) {
        DMS_MNTN_IncAppStatsInfo(dev->portId, DMS_APP_WRT_ASYNC_ERR);
        DMS_PORT_LOGE(dev->portId, "write data async error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

