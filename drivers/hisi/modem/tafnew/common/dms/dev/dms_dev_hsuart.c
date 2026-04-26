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
#include "mdrv_hsuart.h"

#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_HSUART_C

#if (FEATURE_AT_HSUART == FEATURE_ON)

STATIC VOS_VOID DMS_HSUART_RcvData(VOS_VOID)
{
    const struct DMS_PortFuncTbl   *portTblInfo = VOS_NULL_PTR;
    IMM_Zc                         *immZc   = VOS_NULL_PTR;
    struct acm_wr_async_info        ctlParam;
    VOS_INT32                       handle;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_HSUART_RD_CB);

    if (DMS_COMM_CBCheck(DMS_PORT_HSUART) != VOS_OK) {
        return;
    }

    handle = DMS_DEV_GetDevHandlerByPortId(DMS_PORT_HSUART);

    /* 获取底软上行数据BUFFER */
    ctlParam.virt_addr = VOS_NULL_PTR;
    ctlParam.phy_addr  = VOS_NULL_PTR;
    ctlParam.size      = 0;
    ctlParam.priv      = VOS_NULL_PTR;

    if (mdrv_udi_ioctl(handle, UART_IOCTL_GET_RD_BUFF, &ctlParam) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_RD_BUFF_ERR);
        DMS_LOGE("get buffer failed.");
        return;
    }

    if ((ctlParam.virt_addr == VOS_NULL_PTR) || (ctlParam.size == 0)) {
        DMS_LOGE("data buffer error.");
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_RD_ADDR_INVALID);
        return;
    }

    immZc = (IMM_Zc *)ctlParam.virt_addr;

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_HSUART);
    if (portTblInfo->RcvPrivData == VOS_NULL_PTR) {
        DMS_LOGE("ReadSkbData is not registered.");
        return;
    }

    (VOS_VOID)portTblInfo->RcvPrivData(DMS_PORT_HSUART, immZc);
}

STATIC VOS_VOID DMS_HSUART_RcvMscCmd(struct modem_msc_stru *udiMsc)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    struct DMS_MscInfo            dmsMsc;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_HSUART_RD_MSC);

    if (udiMsc == VOS_NULL_PTR) {
        DMS_LOGE("udiMsc is null.");
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_RD_MSC_NULL);
        return;
    }

    if (DMS_COMM_CBCheck(DMS_PORT_HSUART) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_HSUART);
    if (portTblInfo->RcvMscInd == VOS_NULL_PTR) {
        DMS_LOGE("ReadMsc is not registered.");
        return;
    }

    (VOS_VOID)memset_s(&dmsMsc, sizeof(dmsMsc), 0x00, sizeof(dmsMsc));
    dmsMsc.dtrFlag = udiMsc->op_dtr;
    dmsMsc.dtr = udiMsc->uc_dtr;

    portTblInfo->RcvMscInd(DMS_PORT_HSUART, &dmsMsc);
}

STATIC VOS_VOID DMS_HSUART_RcvEscapeSequence(VOS_VOID)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_HSUART_SWITCH_MODE);

    if (DMS_COMM_CBCheck(DMS_PORT_HSUART) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_HSUART);
    if (portTblInfo->RcvEscapeSequence == VOS_NULL_PTR) {
        DMS_LOGE("RcvEscapeSequence is not registered.");
        return;
    }

    portTblInfo->RcvEscapeSequence(DMS_PORT_HSUART);
}

STATIC VOS_VOID DMS_HSUART_RcvWaterMarkDetectInd(water_level level)
{
    const struct DMS_PortFuncTbl *portTblInfo = VOS_NULL_PTR;
    DMS_WaterLevelUint32          waterLevel = DMS_LEVEL_LOW;
    DMS_PortIdUint16              portId = DMS_PORT_HSUART;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_HSUART_WATER_CB);

    if (level != HIGH_LEVEL && level != LOW_LEVEL) {
        DMS_LOGE("level abnormal: %d.", level);
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_WATER_LEVEL_ERR);
        return;
    }

    /*
     * CMUX复用HSUART注册的水线函数，来控制CMUX的电话本读取。
     * 尽管HSUART的UDI关闭了，还是使用HSUART的回调，
     * 如果当前为CMUX端口的话，需要检查CMUX的PORTID。
     */
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    if ((DMS_PORT_GetCmuxPhyPort() == DMS_PORT_HSUART) && (DMS_PORT_GetCmuxOpenFlg() == VOS_TRUE)) {
        portId = DMS_PORT_CMUXAT;
    }
#endif

    if (DMS_COMM_CBCheck(portId) != VOS_OK) {
        return;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_HSUART);
    if (portTblInfo->RcvWaterMarkInd == VOS_NULL_PTR) {
        DMS_LOGE("RcvWaterMarkInd is not registered.");
        return;
    }

    if (level == HIGH_LEVEL) {
        waterLevel = DMS_LEVEL_HIGH;
    }

    portTblInfo->RcvWaterMarkInd(DMS_PORT_HSUART, waterLevel);
}

STATIC VOS_VOID DMS_HSUART_RcvWriteDone(IMM_Zc *immZc)
{
    DMS_MNTN_IncComStatsInfo(DMS_HSUART_FREE_CB);

    if (immZc != VOS_NULL_PTR) {
        IMM_ZcFree(immZc);
    }
}

VOS_UINT32 DMS_HSUART_Open(struct DMS_DevInfo *dev)
{
    struct acm_read_buff_info   stReadBuffInfo;
    struct udi_open_param       stParam;
    VOS_INT                     handle = UDI_INVALID_HANDLE;

    DMS_PORT_LOGI(dev->portId, "enter.");

    stParam.devid = (enum udi_device_id)dev->devId;

    handle = mdrv_udi_open(&stParam);
    if (handle == UDI_INVALID_HANDLE) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_OPEN_ERR);
        DMS_PORT_LOGE(dev->portId, "dev handle is invalid.");
        return VOS_ERR;
    }

    dev->handle = handle;

    stReadBuffInfo.buff_size = DMS_UART_UL_DATA_BUFF_SIZE;
    stReadBuffInfo.buff_num  = DMS_UART_UL_DATA_BUFF_NUM;

    /* 注册HSUART设备上行数据接收回调 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_READ_CB, DMS_HSUART_RcvData) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_REG_RD_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg hsuart read callback failed.\r");
        return VOS_ERR;
    }

    /* 设置UART设备上行数据缓存规格 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_RELLOC_READ_BUFF, &stReadBuffInfo) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_REG_RELLOC_BUFF_ERR);
        DMS_PORT_LOGE(dev->portId, "Set read buff callback for hsuart failed.\r");
        return VOS_ERR;
    }

    /* 注册UART下行数据内存释放接口 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_FREE_CB, DMS_HSUART_RcvWriteDone) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_REG_FREE_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg hsuart write done callback failed.\r");
        return VOS_ERR;
    }

    /* 注册管脚信号通知回调 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_MSC_READ_CB, DMS_HSUART_RcvMscCmd) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_REG_RD_MSC_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg hsuart msc callback failed.\r");
        return VOS_ERR;
    }

    /* 注册"+++"命令检测回调 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SWITCH_MODE_CB, VOS_NULL_PTR) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_REG_SWITCH_MODE_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg hsuart escape sequence callback failed.\r");
        return VOS_ERR;
    }

    /* 注册HSUART端口水线监测回调 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_WATER_CB, DMS_HSUART_RcvWaterMarkDetectInd) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_REG_WATER_CB_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg hsuart watermark callback failed.\r");
        return VOS_ERR;
    }
    return VOS_OK;
}

VOS_UINT32 DMS_HSUART_EnableEscapeSequenceDetect(struct DMS_DevInfo *dev, VOS_BOOL enable)
{
    VOS_VOID *param = VOS_NULL_PTR;

    if (dev == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    DMS_PORT_LOGI(dev->portId, "enter.");
    param = (enable == VOS_TRUE) ? DMS_HSUART_RcvEscapeSequence : VOS_NULL_PTR;

    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SWITCH_MODE_CB, param) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_REG_SWITCH_MODE_ERR);
        DMS_PORT_LOGE(dev->portId, "Reg hsuart escape sequence callback failed.\r");
        return VOS_ERR;
    }

    return VOS_OK;
}

#if (FEATURE_IOT_CMUX == FEATURE_ON)

VOS_UINT32 DMS_HSUART_ClosePort(struct DMS_DevInfo *dev)
{
    DMS_LOGI("DMS_HSUART_ClosePort: enter.");
    if (dev->handle != UDI_INVALID_HANDLE) {
        if (mdrv_udi_close(dev->handle) != VOS_OK) {
            DMS_MNTN_IncComStatsInfo(DMS_CMUX_CLOSE_HSUART_ERR);
            return VOS_ERR;
        }
    }

    dev->handle   = UDI_INVALID_HANDLE;

    return VOS_OK;
}
#endif

VOS_UINT32 DMS_HSUART_SendPrivData(struct DMS_DevInfo *dev, IMM_Zc *data)
{
    struct acm_wr_async_info ctlParam;

    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_HSUART_WRT_ASYNC);

    /* 将数据写往hsuart设备，写成功后内存由底软负责释放 */
    ctlParam.virt_addr = (VOS_CHAR *)data;
    ctlParam.phy_addr = VOS_NULL_PTR;
    ctlParam.size  = 0;
    ctlParam.priv = VOS_NULL_PTR;

    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_WRITE_ASYNC, &ctlParam) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_WRT_ASYNC_ERR);
        DMS_PORT_LOGE(dev->portId, "write async fail.");
        IMM_ZcFree(data);
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_HSUART_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    IMM_Zc                *immZc     = VOS_NULL_PTR;
    VOS_CHAR                   *putData   = VOS_NULL_PTR;
    errno_t                     memResult;

    DMS_PORT_LOGI(dev->portId, "enter.");

    /* 从A核数传内存中分配空间 */
    immZc = IMM_ZcStaticAlloc((VOS_UINT16)len);
    if (immZc == VOS_NULL_PTR) {
        DMS_PORT_LOGE(dev->portId, "alloc fail.");
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_ALLOC_BUF_ERR);
        return VOS_ERR;
    }

    /* 偏移数据尾指针 */
    putData = (VOS_CHAR *)IMM_ZcPut(immZc, len);

    /* 拷贝数据 */
    memResult = memcpy_s(putData, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    return DMS_HSUART_SendPrivData(dev, immZc);
}

VOS_UINT32 DMS_HSUART_SendMscCmd(struct DMS_DevInfo *dev, struct DMS_MscInfo *dmsMsc)
{
    struct modem_msc_stru udiMsc;

    DMS_PORT_LOGI(dev->portId, "enter.");

    (VOS_VOID)memset_s(&udiMsc, sizeof(udiMsc), 0x00, sizeof(udiMsc));
    udiMsc.op_dsr = dmsMsc->dsrFlag;
    udiMsc.op_cts = dmsMsc->ctsFlag;
    udiMsc.op_dcd = dmsMsc->dcdFlag;
    udiMsc.op_ri  = dmsMsc->riFlag;
    udiMsc.uc_dsr = dmsMsc->dsr;
    udiMsc.uc_cts = dmsMsc->cts;
    udiMsc.uc_dcd = dmsMsc->dcd;
    udiMsc.uc_ri  = dmsMsc->ri;

    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_MSC_WRITE_CMD, (VOS_VOID*)&udiMsc) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_WRT_MSC_ERR);
        DMS_PORT_LOGE(dev->portId, "set msc error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_HSUART_CfgFlowCtrl(struct DMS_DevInfo *dev, VOS_UINT32 rtsFlag, VOS_UINT32 ctsFlag)
{
    uart_flow_ctrl_union unFlowCtrlValue;

    DMS_PORT_LOGI(dev->portId, "enter.");

    (VOS_VOID)memset_s(&unFlowCtrlValue, sizeof(unFlowCtrlValue), 0x00, sizeof(unFlowCtrlValue));

    unFlowCtrlValue.reg.rtsen = rtsFlag;
    unFlowCtrlValue.reg.ctsen = ctsFlag;

    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_FLOW_CONTROL, &unFlowCtrlValue) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_FLOW_CONTRL_ERR);
        DMS_PORT_LOGE(dev->portId, "cfg flow ctrl error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_HSUART_SetCharFrame(struct DMS_DevInfo *dev, VOS_UINT32 udiDataLen, VOS_UINT32 udiStpLen,
                                   VOS_UINT32 udiParity)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    /* 调用DRV函数设置串口数据位长度 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_WLEN, (VOS_VOID *)&udiDataLen) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_SET_WLEN_ERR);
        DMS_PORT_LOGE(dev->portId, "set wlen error.");
        return VOS_ERR;
    }

    /* 调用DRV函数设置串口停止位长度 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_STP2, (VOS_VOID *)&udiStpLen) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_SET_STP_ERR);
        DMS_PORT_LOGE(dev->portId, "set stp error.");
        return VOS_ERR;
    }

    /* 调用DRV函数设置串口的校验位 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_EPS, (VOS_VOID *)&udiParity) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_SET_EPS_ERR);
        DMS_PORT_LOGE(dev->portId, "set eps error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_HSUART_SetBaud(struct DMS_DevInfo *dev, VOS_UINT32 baudRate)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    /* 调用DRV函数设置串口的波特率 */
    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_BAUD, (VOS_VOID *)&baudRate) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_SET_BAUD_ERR);
        DMS_PORT_LOGE(dev->portId, "set baud rate error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_HSUART_SetACShell(struct DMS_DevInfo *dev, VOS_UINT32 mode)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_SET_AC_SHELL, &mode) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_SET_ACSHELL_ERR);
        DMS_PORT_LOGE(dev->portId, "set ac shell error.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}

VOS_UINT32 DMS_HSUART_FlushTxData(struct DMS_DevInfo *dev)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    if (mdrv_udi_ioctl(dev->handle, UART_IOCTL_RELEASE_BUFF, VOS_NULL_PTR) != MDRV_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_HSUART_FLUSH_BUFF_ERR);
        DMS_PORT_LOGE(dev->portId, "Release buffer failed.");
        return VOS_ERR;
    }

    DMS_PORT_LOGI(dev->portId, "success.");
    return VOS_OK;
}
#endif

