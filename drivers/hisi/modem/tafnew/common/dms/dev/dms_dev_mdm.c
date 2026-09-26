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

#include "taf_type_def.h"
#include "imm_interface.h"
#include "dms_dev_i.h"
#include "dms_debug.h"

#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_MDM_C

STATIC VOS_VOID DMS_MDM_RcvData(VOS_VOID)
{
    const struct DMS_PortFuncTbl   *portTblInfo = VOS_NULL_PTR;
    struct acm_wr_async_info        ctlParam;
    VOS_INT32 handle;
    IMM_Zc *immZc = VOS_NULL_PTR;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_MDM_RD_CB);

    if (DMS_COMM_CBCheck(DMS_PORT_MODEM) != VOS_OK) {
        return;
    }

    handle = DMS_DEV_GetDevHandlerByPortId(DMS_PORT_MODEM);

    /* 获取底软上行数据BUFFER */
    ctlParam.virt_addr = VOS_NULL_PTR;
    ctlParam.phy_addr  = VOS_NULL_PTR;
    ctlParam.size      = 0;
    ctlParam.priv      = VOS_NULL_PTR;

    if (mdrv_udi_ioctl(handle, ACM_IOCTL_GET_RD_BUFF, &ctlParam) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_RD_BUFF_ERR);
        DMS_LOGE("get buffer failed.");
        return;
    }

    if ((ctlParam.virt_addr == VOS_NULL_PTR) ||
        (ctlParam.size == 0)) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_RD_ADDR_INVALID);
        DMS_LOGE("data buffer error.");
        return;
    }

    immZc = (IMM_Zc *)ctlParam.virt_addr;

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_MODEM);
    if (portTblInfo->RcvPrivData == VOS_NULL_PTR) {
        DMS_LOGE("ReadSkbData is not registered.");
        return;
    }

    (VOS_VOID)portTblInfo->RcvPrivData(DMS_PORT_MODEM, immZc);
}

STATIC VOS_VOID DMS_MDM_RcvWriteDone(VOS_CHAR *buf)
{
    DMS_MNTN_IncComStatsInfo(DMS_MDM_FREE_CB);
    if (buf != VOS_NULL_PTR) {
        IMM_ZcFree((IMM_Zc *)buf);
    }
}

STATIC VOS_VOID DMS_MDM_RcvMscCmd(MODEM_MSC_STRU *rcvedMsc)
{
    const struct DMS_PortFuncTbl   *portTblInfo = VOS_NULL_PTR;
    struct modem_msc_stru          *udiMsc = VOS_NULL_PTR;
    struct DMS_MscInfo              dmsMsc;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_MDM_RD_MSC);

    if (rcvedMsc == VOS_NULL_PTR) {
        DMS_LOGE("udiMsc is null.");
        DMS_MNTN_IncComStatsInfo(DMS_MDM_RD_MSC_NULL);
        return;
    }

    if (DMS_COMM_CBCheck(DMS_PORT_MODEM) != VOS_OK) {
        return;
    }

    udiMsc = (struct modem_msc_stru *)rcvedMsc;

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_MODEM);
    if (portTblInfo->RcvMscInd == VOS_NULL_PTR) {
        DMS_LOGE("ReadMsc is not registered.");
        return;
    }

    (VOS_VOID)memset_s(&dmsMsc, sizeof(dmsMsc), 0x00, sizeof(dmsMsc));
    dmsMsc.dtrFlag = udiMsc->op_dtr;
    dmsMsc.dtr = udiMsc->uc_dtr;

    portTblInfo->RcvMscInd(DMS_PORT_MODEM, &dmsMsc);
}

VOS_UINT32 DMS_MDM_Open(struct DMS_DevInfo *dev)
{
    struct acm_read_buff_info   readBuffInfo;
    struct udi_open_param       stParam;
    VOS_INT                     handle = UDI_INVALID_HANDLE;

    DMS_PORT_LOGI(dev->portId, "enter.");

    stParam.devid = (enum udi_device_id)dev->devId;

    handle = mdrv_udi_open(&stParam);
    if (handle == UDI_INVALID_HANDLE) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_OPEN_ERR);
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    dev->handle = handle;

    /* 注册MODEM设备上行数据接收回调 */
    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_SET_READ_CB, DMS_MDM_RcvData) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_REG_RD_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg MODEM read callback failed.\r");
        return VOS_ERR;
    }

    /* 注册UART下行数据内存释放接口 */
    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_SET_FREE_CB, DMS_MDM_RcvWriteDone) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_REG_FREE_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg MODEM write done callback failed.\r");
        return VOS_ERR;
    }

    /* 注册MODEM下行数据不需要拷贝 */
    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_WRITE_DO_COPY, (void *)0) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_REG_WRT_COPY_ERR);
        DMS_PORT_LOGE(dev->portId, "Set not do copy for modem failed.\r");
        return VOS_ERR;
    }

    /* 注册管脚信号通知回调 */
    if (mdrv_udi_ioctl(dev->handle, ACM_MODEM_IOCTL_SET_MSC_READ_CB, DMS_MDM_RcvMscCmd) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_REG_RD_MSC_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg MODEM msc callback failed.\r");
        return VOS_ERR;
    }

    readBuffInfo.buff_size  = DMS_MODEM_UL_DATA_BUFF_SIZE;
    readBuffInfo.buff_num   = DMS_MODEM_UL_DATA_BUFF_NUM;

    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_RELLOC_READ_BUFF, &readBuffInfo) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_REG_RELLOC_BUFF_ERR);
        DMS_PORT_LOGE(dev->portId, "Initialize data buffer failed.\r");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_MDM_Close(struct DMS_DevInfo *dev)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (dev->handle != UDI_INVALID_HANDLE) {
        if (mdrv_udi_close(dev->handle) != VOS_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_MDM_CLOSE_ERR);
            return VOS_ERR;
        }

        dev->handle = UDI_INVALID_HANDLE;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_MDM_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data)
{
    struct acm_wr_async_info    ctlParam;

    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_MDM_WRT_ASYNC);

    /* 将数据写往MODEM设备，写成功后内存由底软负责释放 */
    ctlParam.virt_addr = (VOS_CHAR *)data;
    ctlParam.phy_addr = VOS_NULL_PTR;
    ctlParam.size  = 0;
    ctlParam.priv = VOS_NULL_PTR;

    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_WRITE_ASYNC, &ctlParam) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_WRT_ASYNC_ERR);
        DMS_PORT_LOGE(dev->portId, "write async fail.");
        IMM_ZcFree(data);
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_MDM_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *dataBuff, VOS_UINT16 len)
{
    IMM_Zc    *data        = VOS_NULL_PTR;
    VOS_CHAR  *zcPutData   = VOS_NULL_PTR;
    errno_t    memResult;

    DMS_PORT_LOGI(dev->portId, "enter.");

    data = IMM_ZcStaticAlloc((VOS_UINT16)len);

    if (data == VOS_NULL_PTR) {
        DMS_PORT_LOGE(dev->portId, "alloc fail.");
        DMS_MNTN_IncComStatsInfo(DMS_MDM_WRT_BUFF_ERR);
        return VOS_ERR;
    }

    /* 此步骤不能少，用来偏移数据尾指针 */
    zcPutData = (VOS_CHAR *)IMM_ZcPut(data, len);

    memResult = memcpy_s(zcPutData, len, dataBuff, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    return DMS_MDM_SendPrivData(dev, data);
}

VOS_UINT32 DMS_MDM_SendMscCmd(struct DMS_DevInfo *dev, struct DMS_MscInfo *dmsMsc)
{
    struct modem_msc_stru udiMsc;

    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_MDM_WRT_MSC);

    (VOS_VOID)memset_s(&udiMsc, sizeof(udiMsc), 0x00, sizeof(udiMsc));
    udiMsc.op_dsr = dmsMsc->dsrFlag;
    udiMsc.op_cts = dmsMsc->ctsFlag;
    udiMsc.op_dcd = dmsMsc->dcdFlag;
    udiMsc.op_ri  = dmsMsc->riFlag;
    udiMsc.uc_dsr = dmsMsc->dsr;
    udiMsc.uc_cts = dmsMsc->cts;
    udiMsc.uc_dcd = dmsMsc->dcd;
    udiMsc.uc_ri  = dmsMsc->ri;

    if (mdrv_udi_ioctl(dev->handle, ACM_MODEM_IOCTL_MSC_WRITE_CMD, &udiMsc) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_MDM_WRT_MSC_ERR);
        DMS_PORT_LOGE(dev->portId, "set msc error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

