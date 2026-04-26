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
#include "dms_dev_i.h"
#include "dms_debug.h"
#include "dms_buff.h"

#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_ACM_C

typedef VOS_VOID (*comRcvData)(VOS_VOID);
typedef VOS_VOID (*comRcvEvtCB)(ACM_EVT_E evt);

STATIC VOS_VOID DMS_ACM_RcvData(DMS_PortIdUint16 portId)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    struct DMS_DevInfo           *devInfo     = VOS_NULL_PTR;
    struct acm_wr_async_info      acmInfo;

    DMS_PORT_LOGI(portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_ACM_RD_CB);

    if (DMS_COMM_CBCheck(portId) != VOS_OK) {
        return;
    }

    if (portId < DMS_PORT_PCUI || portId > DMS_PORT_PCUI2) {
        DMS_PORT_LOGE(portId, "<DMS_ACM_RcvData>: portId is uncorrect, %d!", portId);
        return;
    }

    devInfo = DMS_DEV_GetDevEntityByPortId(portId);

    (VOS_VOID)memset_s(&acmInfo, sizeof(acmInfo), 0x00, sizeof(acmInfo));

    if (mdrv_udi_ioctl(devInfo->handle, ACM_IOCTL_GET_RD_BUFF, &acmInfo) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_RD_BUFF_ERR);
        DMS_PORT_LOGE(portId, "ACM_IOCTL_GET_RD_BUFF failed!");
        return;
    }

    if (acmInfo.virt_addr == VOS_NULL_PTR || acmInfo.size == 0) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_BUFF_NULL);
        DMS_PORT_LOGE(portId, "acmInfo invalid [acmInfo.size %d]!", acmInfo.size);
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(portId);
    if (portTblInfo->RcvData == VOS_NULL_PTR) {
        DMS_PORT_LOGE(portId, "ReadData is not registered!");
        return;
    }

    devInfo->chanStat = ACM_EVT_DEV_READY;

    portTblInfo->RcvData(portId, (VOS_UINT8 *)acmInfo.virt_addr, acmInfo.size); /* 调用DMS port模块的回调函数 */

    if (mdrv_udi_ioctl(devInfo->handle, ACM_IOCTL_RETURN_BUFF, &acmInfo) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_RETURN_BUFF_ERR);
        DMS_PORT_LOGE(portId, "ACM_IOCTL_RETURN_BUFF failed!");
        return;
    }
}

STATIC VOS_VOID DMS_ACM_PcuiRcvData(VOS_VOID)
{
    DMS_ACM_RcvData(DMS_PORT_PCUI);
}

STATIC VOS_VOID DMS_ACM_CtrlRcvData(VOS_VOID)
{
    DMS_ACM_RcvData(DMS_PORT_CTRL);
}

STATIC VOS_VOID DMS_ACM_Pcui2RcvData(VOS_VOID)
{
    DMS_ACM_RcvData(DMS_PORT_PCUI2);
}

STATIC comRcvData DMS_ACM_GetRcvData(DMS_PortIdUint16 portId)
{
    comRcvData acmComRcvData = VOS_NULL_PTR;

    if (portId == DMS_PORT_PCUI) {
        acmComRcvData  = DMS_ACM_PcuiRcvData;
    } else if (portId == DMS_PORT_CTRL) {
        acmComRcvData  = DMS_ACM_CtrlRcvData;
    } else {
        acmComRcvData  = DMS_ACM_Pcui2RcvData;
    }

    return acmComRcvData;
}

STATIC VOS_VOID DMS_ACM_RcvWriteDone(VOS_CHAR *virAddr, VOS_CHAR *phyAddr, VOS_INT doneSize)
{
    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_ACM_WRT_CB);

    if (doneSize < 0) {
        DMS_LOGE("doneSize invalid!\r");
        DMS_MNTN_IncComStatsInfo(DMS_ACM_WRT_DONE_SIZE_ERR);
    }

    if (DMS_BUF_IsStaticBuf((VOS_UINT8 *)virAddr) == VOS_TRUE) {
        DMS_BUF_FreeStaticBuf((VOS_UINT8 *)virAddr);
    } else {
        if (virAddr != VOS_NULL_PTR) {
            kfree(virAddr);
            virAddr = VOS_NULL_PTR;
        }
    }
}

STATIC VOS_VOID DMS_ACM_RcvEvtCB(DMS_PortIdUint16 portId, ACM_EVT_E evt)
{
    struct DMS_DevInfo *devInfo = VOS_NULL_PTR;

    DMS_PORT_LOGI(portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_ACM_EVT_CB);

    if (portId < DMS_PORT_PCUI || portId > DMS_PORT_PCUI2) {
        DMS_PORT_LOGE(portId, "portId is uncorrect, %d!", portId);
        return;
    }

    devInfo = DMS_DEV_GetDevEntityByPortId(portId);

    if (evt == ACM_EVT_DEV_READY) {
        devInfo->chanStat = ACM_EVT_DEV_READY;
    } else {
        devInfo->chanStat = ACM_EVT_DEV_SUSPEND;
    }
}

STATIC VOS_VOID DMS_ACM_PcuiRcvEvtCB(ACM_EVT_E evt)
{
    DMS_ACM_RcvEvtCB(DMS_PORT_PCUI, evt);
}

STATIC VOS_VOID DMS_ACM_CtrlRcvEvtCB(ACM_EVT_E evt)
{
    DMS_ACM_RcvEvtCB(DMS_PORT_CTRL, evt);
}

STATIC VOS_VOID DMS_ACM_Pcui2RcvEvtCB(ACM_EVT_E evt)
{
    DMS_ACM_RcvEvtCB(DMS_PORT_PCUI2, evt);
}

STATIC comRcvEvtCB DMS_ACM_GetRcvEvtCB(DMS_PortIdUint16 portId)
{
    comRcvEvtCB acmComRcvEvtCB = VOS_NULL_PTR;

    if (portId == DMS_PORT_PCUI) {
        acmComRcvEvtCB = DMS_ACM_PcuiRcvEvtCB;
    } else if (portId == DMS_PORT_CTRL) {
        acmComRcvEvtCB = DMS_ACM_CtrlRcvEvtCB;
    } else {
        acmComRcvEvtCB = DMS_ACM_Pcui2RcvEvtCB;
    }

    return acmComRcvEvtCB;
}

VOS_UINT32 DMS_ACM_Open(struct DMS_DevInfo *dev)
{
    comRcvData                acmComRcvData = VOS_NULL_PTR;
    comRcvEvtCB               acmComRcvEvtCB = VOS_NULL_PTR;
    struct acm_read_buff_info readBuffInfo;
    struct udi_open_param     stParam;
    VOS_INT                   handle = UDI_INVALID_HANDLE;

    DMS_PORT_LOGI(dev->portId, "enter.");

    stParam.devid = (enum udi_device_id)dev->devId;

    handle = mdrv_udi_open(&stParam);
    if (handle == UDI_INVALID_HANDLE) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_OPEN_ERR);
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    dev->handle = handle;

    readBuffInfo.buff_size  = DMS_ACM_UL_DATA_BUFF_SIZE;
    readBuffInfo.buff_num   = DMS_ACM_UL_DATA_BUFF_NUM;

    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_RELLOC_READ_BUFF, &readBuffInfo) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_REG_RELLOC_BUFF_ERR);
        DMS_PORT_LOGE(dev->portId, "ACM_IOCTL_RELLOC_READ_BUFF failed!");
        return VOS_ERR;
    }

    /* 注册下行数据内存释放接口 */
    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_SET_WRITE_CB, DMS_ACM_RcvWriteDone) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_REG_WRT_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "ACM_IOCTL_SET_WRITE_CB failed!");
        return VOS_ERR;
    }

    /* 注册下行数据不需要拷贝 */
    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_WRITE_DO_COPY, (void *)0) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_REG_WRT_COPY_ERR);
        DMS_PORT_LOGE(dev->portId, "ACM_IOCTL_WRITE_DO_COPY failed!");
        return VOS_ERR;
    }

    acmComRcvData = DMS_ACM_GetRcvData(dev->portId);
    acmComRcvEvtCB = DMS_ACM_GetRcvEvtCB(dev->portId);

    /* 注册事件处理回调 */
    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_SET_EVT_CB, acmComRcvEvtCB) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_REG_EVT_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "ACM_IOCTL_SET_EVT_CB failed!");
        return VOS_ERR;
    }

    /* 注册上行数据接收回调 */
    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_SET_READ_CB, acmComRcvData) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_REG_RD_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "ACM_IOCTL_SET_READ_CB failed!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_ACM_Close(struct DMS_DevInfo *dev)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (dev->handle != UDI_INVALID_HANDLE) {
        if (mdrv_udi_close(dev->handle) != VOS_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_ACM_CLOSE_ERR);
            return VOS_ERR;
        }

        dev->handle   = UDI_INVALID_HANDLE;
        dev->chanStat = ACM_EVT_DEV_SUSPEND;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_ACM_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8               *senBuf = VOS_NULL_PTR;
    errno_t                  memResult;
    struct acm_wr_async_info acmInfo = {0};

    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_ACM_WRT_ASYNC);

    senBuf = DMS_BUF_GetStaticBuf(len);
    if (senBuf == VOS_NULL_PTR) {
        DMS_PORT_LOGE(dev->portId, "DMS_BUF_GetStaticBuf return VOS_NULL_PTR.");
        DMS_MNTN_IncComStatsInfo(DMS_ACM_WRT_BUFF_ERR);
        return VOS_ERR;
    }

    memResult = memcpy_s(senBuf, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    (VOS_VOID)memset_s(&acmInfo, sizeof(acmInfo), 0x00, sizeof(acmInfo));

    acmInfo.virt_addr = (VOS_CHAR *)senBuf;
    acmInfo.phy_addr  = VOS_NULL_PTR;
    acmInfo.size      = len;

    if (dev->chanStat == ACM_EVT_DEV_SUSPEND) {
        DMS_PORT_LOGE(dev->portId, "DEV SUSPEND.");
        DMS_MNTN_IncComStatsInfo(DMS_ACM_WRT_CHAN_STAT_ERR);
        DMS_BUF_FreeStaticBuf(senBuf);
        return VOS_ERR;
    }

    if (mdrv_udi_ioctl(dev->handle, ACM_IOCTL_WRITE_ASYNC, &acmInfo) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_ACM_WRT_ASYNC_ERR);
        DMS_PORT_LOGE(dev->portId, "write data async error.");
        DMS_BUF_FreeStaticBuf(senBuf);
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

