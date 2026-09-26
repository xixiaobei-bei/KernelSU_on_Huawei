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
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_NCM_C

STATIC VOS_VOID DMS_NCM_RcvConnStatusChg(enum ncm_ioctrl_connect_stus status, VOS_UINT32 udiId)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    DMS_NcmStatusUint32           ncmStatus = DMS_NCM_IOCTL_STUS_CONNECT;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_CONN_STUS);

    if (status != NCM_IOCTL_STUS_CONNECT && status != NCM_IOCTL_STUS_BREAK) {
        DMS_LOGE("status uncorrect.");
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_CONN_STUS_ERR);
        return;
    }

    if (DMS_COMM_CBCheck(DMS_PORT_NCM_CTRL) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_NCM_CTRL);
    if (portTblInfo->RcvConnStatusChgInd == VOS_NULL_PTR) {
        DMS_LOGE("RcvConnStatusChgInd is not registered.");
        return;
    }

    if (status == NCM_IOCTL_STUS_BREAK) {
        ncmStatus = DMS_NCM_IOCTL_STUS_BREAK;
    }

    portTblInfo->RcvConnStatusChgInd(DMS_PORT_NCM_CTRL, ncmStatus, udiId);
}

STATIC VOS_INT DMS_NCM_CtrlRcvData(VOS_UINT8 *data, VOS_UINT16 uslength)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    struct DMS_DevInfo           *devInfo     = VOS_NULL_PTR;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_RD_CB);

    if (data == VOS_NULL_PTR) {
        DMS_LOGE("data is VOS_NULL_PTR PTR.");
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_RD_DATA_NULL);
        return VOS_ERROR;
    }

    if (uslength == 0) {
        DMS_LOGE("len is 0.");
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_RD_LEN_INVALID);
        return VOS_ERROR;
    }

    if (DMS_COMM_CBCheck(DMS_PORT_NCM_CTRL) != VOS_OK) {
        return VOS_ERROR;
    }

    devInfo     = DMS_DEV_GetDevEntityByPortId(DMS_PORT_NCM_CTRL);
    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_NCM_CTRL);

    if (portTblInfo->RcvData == VOS_NULL_PTR) {
        DMS_LOGE("ReadData is not registered.");
        return VOS_ERROR;
    }

    /* 设置NDIS通道状态为可上报数据 */
    devInfo->chanStat = ACM_EVT_DEV_READY;

    return portTblInfo->RcvData(DMS_PORT_NCM_CTRL, data, (VOS_UINT32)uslength);
}

VOS_VOID DMS_NCM_CtrlRcvWriteDone(char *doneBuff, int status)
{
    DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_WRT_CB);

    if (DMS_BUF_IsStaticBuf((VOS_UINT8 *)doneBuff)) {
        DMS_BUF_FreeStaticBuf((VOS_UINT8 *)doneBuff);
    } else {
        if (doneBuff != VOS_NULL_PTR) {
            kfree(doneBuff);
            doneBuff = VOS_NULL_PTR;
        }
    }
}

VOS_UINT32 DMS_NCM_CtrlOpen(struct DMS_DevInfo *dev)
{
    struct udi_open_param stParam;
    VOS_INT               handle = UDI_INVALID_HANDLE;

    DMS_PORT_LOGI(dev->portId, "enter.");

    stParam.devid = (enum udi_device_id)dev->devId;

    handle = mdrv_udi_open(&stParam);
    if (handle == UDI_INVALID_HANDLE) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_OPEN_ERR);
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    dev->handle = handle;

    /* 注册NDIS设备网卡连接状态回调 */
    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC, DMS_NCM_RcvConnStatusChg) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_REG_CONN_STUS_ERR);
        DMS_PORT_LOGE(dev->portId, "NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC fail.");
        return VOS_ERR;
    }

    /* 注册NDIS设备AT命令状态处理回调 */
    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_REG_NDIS_RESP_STATUS_FUNC, DMS_NCM_CtrlRcvWriteDone) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_REG_WRT_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "NCM_IOCTL_REG_NDIS_RESP_STATUS_FUNC fail.");
        return VOS_ERR;
    }

    /* 注册NDIS设备接收AT命令回调 */
    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_REG_AT_PROCESS_FUNC, DMS_NCM_CtrlRcvData) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_REG_RD_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "NCM_IOCTL_REG_AT_PROCESS_FUNC fail.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_NCM_CtrlSendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8        *senBuf = VOS_NULL_PTR;
    struct ncm_at_rsp atResponse = {0};
    errno_t           memResult;

    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_WRT_ASYNC);

    senBuf = DMS_BUF_GetStaticBuf(len);
    if (senBuf == VOS_NULL_PTR) {
        DMS_PORT_LOGE(dev->portId, "DMS_BUF_GetStaticBuf return VOS_NULL_PTR.");
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_WRT_GET_BUF_ERR);
        return VOS_ERR;
    }

    memResult = memcpy_s(senBuf, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    (VOS_VOID)memset_s(&atResponse, sizeof(atResponse), 0x00, sizeof(atResponse));

    atResponse.at_answer = senBuf;
    atResponse.length    = len;

    if (dev->chanStat == ACM_EVT_DEV_SUSPEND) {
        DMS_PORT_LOGE(dev->portId, "DEV SUSPEND.");
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_WRT_CHAN_STAT_ERR);
        DMS_BUF_FreeStaticBuf(senBuf);
        return VOS_ERR;
    }

    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_AT_RESPONSE, &atResponse) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_WRT_ASYNC_ERR);
        DMS_PORT_LOGE(dev->portId, "write data async error.");
        DMS_BUF_FreeStaticBuf(senBuf);
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_NCM_CtrlClose(struct DMS_DevInfo *dev)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (dev->handle != UDI_INVALID_HANDLE) {
        if (mdrv_udi_close(dev->handle) != VOS_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_NCM_CTRL_CLOSE_ERR);
            return VOS_ERR;
        }

        dev->handle   = UDI_INVALID_HANDLE;
        dev->chanStat = ACM_EVT_DEV_SUSPEND;
    }

    return VOS_OK;
}

 VOS_UINT32 DMS_NCM_DataOpen(struct DMS_DevInfo *dev)
{
    struct udi_open_param stParam;
    VOS_INT handle = UDI_INVALID_HANDLE;

    DMS_PORT_LOGI(dev->portId, "enter.");

    stParam.devid = (enum udi_device_id)dev->devId;

    handle = mdrv_udi_open(&stParam);
    if (handle == UDI_INVALID_HANDLE) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_DATA_OPEN_ERR);
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    dev->handle = handle;

    return VOS_OK;
}

VOS_UINT32 DMS_NCM_DataClose(struct DMS_DevInfo *dev)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (dev->handle != UDI_INVALID_HANDLE) {
        if (mdrv_udi_close(dev->handle) != VOS_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_NCM_DATA_CLOSE_ERR);
            return VOS_ERR;
        }

        dev->handle = UDI_INVALID_HANDLE;
    }

    return VOS_OK;
}

VOS_UINT32 DMS_NCM_SetIpv6Dns(struct DMS_DevInfo *dev, struct DMS_NcmIpv6Dns *dmsDns)
{
    struct ncm_ipv6_dns udiDns;

    DMS_PORT_LOGI(dev->portId, "enter.");

    udiDns.ipv6_dns_info = dmsDns->ipv6DnsInfo;
    udiDns.length = dmsDns->length;

    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_SET_IPV6_DNS, &udiDns) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_DATA_SET_IPV6_ERR);
        DMS_PORT_LOGE(dev->portId, "set ipv6 dns error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_NCM_ChangeFlowCtrl(struct DMS_DevInfo *dev, VOS_UINT32 enableFlg)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_FLOW_CTRL_NOTIF, &enableFlg) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_DATA_FLOW_CTRL_ERR);
        DMS_PORT_LOGE(dev->portId, "ntf flow ctrl error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_NCM_ChangeConnSpeed(struct DMS_DevInfo *dev, struct DMS_ConnSpeed *dmsSpeed)
{
    struct ncm_ioctl_connection_speed udiSpeed;

    DMS_PORT_LOGI(dev->portId, "enter.");

    udiSpeed.down_bitrate = dmsSpeed->downSpeed;
    udiSpeed.up_bitrate = dmsSpeed->upSpeed;

    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF, &udiSpeed) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_DATA_CONN_SPEED_ERR);
        DMS_PORT_LOGE(dev->portId, "ntf speed error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_NCM_ChangeConnLinkState(struct DMS_DevInfo *dev, VOS_UINT32 linkStatus)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (mdrv_udi_ioctl(dev->handle, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, &linkStatus) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_NCM_DATA_CONN_NTF_ERR);
        DMS_PORT_LOGE(dev->portId, "ntf conn error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

