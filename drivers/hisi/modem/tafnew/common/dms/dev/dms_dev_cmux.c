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

#include "taf_type_def.h"

#include "dms_dev.h"
#include "imm_interface.h"
#include "dms_dev_i.h"
#include "dms_debug.h"

#include "dms_port.h"
#include "mdrv_usb.h"
#include "dms_mntn.h"
#include "securec.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_CMUX_C


#if (FEATURE_IOT_CMUX == FEATURE_ON)

int DMS_CMUX_CheckRawDataMode(VOS_UINT8 priIndex)
{
    struct DMS_PortInfo      *portInfo = VOS_NULL_PTR;
    DMS_PortIdUint16          portId;

    portId   = DMS_PORT_CMUXAT + priIndex - 1;

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_ERR;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);

    if (portInfo->modeInfo.mode == DMS_PORT_MODE_ONLINE_DATA) {
        return 0;
    }

    return 1;
}

STATIC VOS_VOID DMS_CMUX_RcvWriteDone(IMM_Zc *immZc)
{
    DMS_MNTN_IncComStatsInfo(DMS_CMUX_FREE_CB);

    if (immZc != VOS_NULL_PTR) {
        IMM_ZcFree(immZc);
    }
}

VOS_UINT32 DMS_CMUX_SendMscCmd(struct DMS_DevInfo *dev, struct DMS_MscInfo *msc)
{
    struct modem_msc_stru udiMsc;

    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_MDM_WRT_MSC);

    (VOS_VOID)memset_s(&udiMsc, sizeof(udiMsc), 0x00, sizeof(udiMsc));
    udiMsc.op_dsr = msc->dsrFlag;
    udiMsc.op_cts = msc->ctsFlag;
    udiMsc.op_dcd = msc->dcdFlag;
    udiMsc.op_ri  = msc->riFlag;
    udiMsc.uc_dsr = msc->dsr;
    udiMsc.uc_cts = msc->cts;
    udiMsc.uc_dcd = msc->dcd;
    udiMsc.uc_ri  = msc->ri;

    if (mdrv_udi_ioctl(dev->handle, UDI_CMUX_IOCTL_MSC_WRITE_CMD, &udiMsc) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_CMUX_WRT_MSC_ERR);
        DMS_PORT_LOGE(dev->portId, "set msc error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_CMUX_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data)
{
    struct acm_wr_async_info ctlParam;

    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_CMUX_WRT_ASYNC);

    /* 将数据写往hsuart设备，写成功后内存由底软负责释放 */
    ctlParam.virt_addr = (VOS_CHAR *)data;
    ctlParam.phy_addr = VOS_NULL_PTR;
    ctlParam.size  = data->len;
    ctlParam.priv = VOS_NULL_PTR;

    if (mdrv_udi_ioctl(dev->handle, UDI_CMUX_IOCTL_WRITE_ASYNC, &ctlParam) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_CMUX_WRT_ASYNC_ERR);
        DMS_PORT_LOGE(dev->portId, "write async fail.");
        IMM_ZcFree(data);
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_CMUX_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    IMM_Zc   *immZc   = VOS_NULL_PTR;
    VOS_CHAR *putData = VOS_NULL_PTR;
    errno_t   memResult;

    DMS_PORT_LOGI(dev->portId, "enter.");

    /* 从A核数传内存中分配空间 */
    immZc = IMM_ZcStaticAlloc((VOS_UINT16)len);
    if (immZc == VOS_NULL_PTR) {
        DMS_PORT_LOGE(dev->portId, "alloc fail.");
        DMS_MNTN_IncComStatsInfo(DMS_CMUX_ALLOC_BUF_ERR);
        return VOS_ERR;
    }

    /* 偏移数据尾指针 */
    putData = (VOS_CHAR *)IMM_ZcPut(immZc, len);

    /* 拷贝数据 */
    memResult = memcpy_s(putData, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    return DMS_CMUX_SendPrivData(dev, immZc);
}

VOS_VOID DMS_CMUX_GetUlDataBuff(DMS_PortIdUint16 portId, IMM_Zc **pstImmZc, VOS_UINT32 *pulLen)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    IMM_Zc                       *immZc   = VOS_NULL_PTR;
    struct DMS_DevInfo           *devInfo = VOS_NULL_PTR;
    struct acm_wr_async_info      ctlParam;
    VOS_INT32                     result;

    /* 检查UDI句柄有效性 */
    devInfo = DMS_PORT_GetPortInfo(portId)->devInfo;
    if (devInfo->handle == UDI_INVALID_HANDLE) {
        DMS_PORT_LOGE(portId, "Invalid UDI handle");
        return;
    }

    /* 填写待释放的内存地址 */
    ctlParam.virt_addr = VOS_NULL_PTR;
    ctlParam.phy_addr  = VOS_NULL_PTR;
    ctlParam.size      = 0;
    ctlParam.priv      = VOS_NULL_PTR;

    result = mdrv_udi_ioctl(devInfo->handle, UDI_CMUX_IOCTL_GET_RD_BUFF, &ctlParam);
    if (result != VOS_OK) {
        DMS_PORT_LOGE(portId, "Get buffer failed!");
        return;
    }

    if ((ctlParam.virt_addr == VOS_NULL_PTR) || (ctlParam.size == 0)) {
        DMS_LOGE("data buffer error.");
        return;
    }

    immZc = (IMM_Zc *)ctlParam.virt_addr;

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo->RcvPrivData == VOS_NULL_PTR) {
        DMS_LOGE("ReadSkbData is not registered.");
        return;
    }

    (VOS_VOID)portTblInfo->RcvPrivData(portId, immZc);
}

VOS_VOID DMS_CMUX_UlDataReadCB(VOS_UINT8 priIndex)
{
    IMM_Zc                   *immZc = VOS_NULL_PTR;
    VOS_UINT32                len = 0;
    DMS_PortIdUint16          portId = DMS_PORT_CMUXAT + priIndex - 1;

    DMS_CMUX_GetUlDataBuff(portId, &immZc, &len);
}

VOS_VOID DMS_CMUX_ClosePortCB(VOS_VOID)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;

    DMS_LOGI("DMS_CMUX_ClosePortCB: enter.");

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_CMUXAT);
    if (portTblInfo->RcvCmuxCloseUart == VOS_NULL_PTR) {
        DMS_LOGE("DMS_CMUX_ClosePortCB is not registered.");
        return;
    }

    (VOS_VOID)portTblInfo->RcvCmuxCloseUart();
}

DMS_PortIdUint16 DMS_CMUX_GetPortIdFromIndex(VOS_UINT8 priIndex)
{
    DMS_PortIdUint16 portId;

    if (priIndex == 0) {
        if (DMS_PORT_GetMode(DMS_PORT_CMUXAT) != DMS_PORT_MODE_CMD) {
            portId = DMS_PORT_CMUXAT;
        } else if (DMS_PORT_GetMode(DMS_PORT_CMUXMDM) != DMS_PORT_MODE_CMD) {
            portId = DMS_PORT_CMUXMDM;
        } else if (DMS_PORT_GetMode(DMS_PORT_CMUXEXT) != DMS_PORT_MODE_CMD) {
            portId = DMS_PORT_CMUXEXT;
        } else if (DMS_PORT_GetMode(DMS_PORT_CMUXGPS) != DMS_PORT_MODE_CMD) {
            portId = DMS_PORT_CMUXGPS;
        } else {
            DMS_LOGE("Index is invalid!.");
            portId = DMS_PORT_BUTT;
        }
    }
    else {
        portId = DMS_PORT_CMUXAT + priIndex - 1;
    }

    return portId;
}

VOS_VOID DMS_CMUX_MscReadCB(VOS_UINT8 priIndex, VOS_VOID *param)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    struct modem_msc_stru        *dceMsc = VOS_NULL_PTR;
    struct DMS_MscInfo            dmsMsc;
    DMS_PortIdUint16              portId;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_CMUX_SET_MSC_READ_CB);
    dceMsc = (struct modem_msc_stru *)param;

    if (dceMsc == VOS_NULL_PTR) {
        DMS_LOGE("dceMsc is null.");
        DMS_MNTN_IncComStatsInfo(DMS_CMUX_SET_MSC_READ_CB_NULL);
        return;
    }

    /* priIndex为0表示从硬件管脚发过来的状态，将portId转换为非命令态的portId */
    portId = DMS_CMUX_GetPortIdFromIndex(priIndex);
    if (portId == DMS_PORT_BUTT) {
        DMS_LOGE("Index is invalid!.");
        return;
    }

    /* 检查cmux口是否有效，是否有cmux口有有效实体 */
    if (DMS_COMM_CBCheck(portId) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo->RcvMscInd == VOS_NULL_PTR) {
        DMS_LOGE("ReadMsc is not registered.");
        return;
    }

    (VOS_VOID)memset_s(&dmsMsc, sizeof(dmsMsc), 0x00, sizeof(dmsMsc));
    dmsMsc.dtrFlag = dceMsc->op_dtr;
    dmsMsc.dtr = dceMsc->uc_dtr;


    portTblInfo->RcvMscInd(portId, &dmsMsc);
}

VOS_VOID DMS_CMUX_MscModeReadCB(VOS_UINT8 priIndex)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    DMS_PortIdUint16 portId = DMS_PORT_CMUXAT;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_CMUX_MSC_MODE_READ_CB);

    /* priIndex从1开始 */
    if (priIndex != 0) {
        portId = DMS_PORT_CMUXAT + priIndex - 1;
    }

    if (DMS_COMM_CBCheck(portId) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo->RcvMscModeRead == VOS_NULL_PTR) {
        DMS_LOGE("MscModeReadCB is not registered.");
        return;
    }

    portTblInfo->RcvMscModeRead(portId);
}

VOS_VOID DMS_CMUX_RcvEscapeSequence(VOS_UINT8 priIndex)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    DMS_PortIdUint16 portId = DMS_PORT_CMUXAT;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_CMUX_SWITCH_MODE);

    /* priIndex从1开始 */
    if (priIndex != 0) {
        portId = DMS_PORT_CMUXAT + priIndex - 1;
    }

    if (DMS_COMM_CBCheck(portId) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo->RcvEscapeSequence == VOS_NULL_PTR) {
        DMS_LOGE("RcvEscapeSequence is not registered.");
        return;
    }

    portTblInfo->RcvEscapeSequence(portId);
}

VOS_UINT32 DMS_CMUX_Open(struct DMS_DevInfo *dev)
{
    struct udi_open_param stParam;
    VOS_INT               handle = UDI_INVALID_HANDLE;

    /* 配置端口消息映射--TO DO */
    stParam.devid = (enum udi_device_id)(dev->devId);
    stParam.private = (VOS_VOID *)(VOS_UINT_PTR)(dev->devId - UDI_CMUX_AT_ID + 1);

    /* 打开Device，获得ID */
    handle = mdrv_udi_open(&stParam);

    if (handle != UDI_INVALID_HANDLE) {
        dev->handle = handle;

        /* 注册上行数据接收回调 */
        if (mdrv_udi_ioctl(handle, UDI_CMUX_IOCTL_SET_READ_CB, DMS_CMUX_UlDataReadCB) != MDRV_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_REG_RD_CB_ERR);
            DMS_PORT_LOGE(dev->portId, "Reg cmux read callback failed.\r");
            return VOS_ERR;
        }

        /* 注册下行数据内存释放接口 */
        if (mdrv_udi_ioctl(handle, UDI_CMUX_IOCTL_SET_FREE_CB, DMS_CMUX_RcvWriteDone) != MDRV_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_REG_FREE_CB_ERR);
            DMS_PORT_LOGE(dev->portId, "Reg cmux free callback failed.\r");
            return VOS_ERR;
        }

        /* 注册关闭复用模式接口 */
        if (mdrv_udi_ioctl(handle, UDI_CMUX_IOCTL_CLOSE_PORT_CB, DMS_CMUX_ClosePortCB) != MDRV_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_REG_CLOSE_CB_ERR);
            DMS_PORT_LOGE(dev->portId, "Reg cmux close callback failed.\r");
            return VOS_ERR;
        }

        /* 注册管脚信号通知回调 */
        if (mdrv_udi_ioctl(handle, UDI_CMUX_IOCTL_SET_MSC_READ_CB, DMS_CMUX_MscReadCB) != MDRV_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_SET_MSC_READ_CB_ERR);
            DMS_PORT_LOGE(dev->portId, "Set cmux msc callback failed.\r");
            return VOS_ERR;
        }

        /* 注册管脚模式查询回调 */
        if (mdrv_udi_ioctl(handle, UDI_CMUX_IOCTL_MSC_MODE_READ_CB, DMS_CMUX_MscModeReadCB) != MDRV_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_MSC_MODE_READ_CB_ERR);
            DMS_PORT_LOGE(dev->portId, "cmux msc mode read callback failed.\r");
            return VOS_ERR;
        }

        /* 注册+++切换命令模式回调 */
        if (mdrv_udi_ioctl(handle, UDI_CMUX_IOCTL_SWITCH_MODE_CB, DMS_CMUX_RcvEscapeSequence) != MDRV_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_SWITCH_MODE_CB_ERR);
            DMS_PORT_LOGE(dev->portId, "cmux switch mode callback failed.\r");
            return VOS_ERR;
        }

    }else {
        DMS_MNTN_IncComStatsInfo(DMS_CMUX_OPEN_ERR);
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    return VOS_OK;
}

#endif

