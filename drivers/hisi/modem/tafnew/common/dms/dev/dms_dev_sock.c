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

#include "dms_dev_i.h"
#include "dms_debug.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_DEV_SOCK_C

STATIC VOS_UINT32 DMS_SOCK_RcvData(VOS_UINT8 *data, VOS_UINT32 len)
{
    const struct DMS_PortFuncTbl* portTblInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");
    DMS_MNTN_IncComStatsInfo(DMS_SOCK_RD_CB);

    if (data == VOS_NULL_PTR) {
        DMS_LOGE("data is NULL PTR.");
        DMS_MNTN_IncComStatsInfo(DMS_SOCK_RD_DATA_NULL);
        return VOS_ERR;
    }

    if (len == 0) {
        DMS_LOGE("len is 0.");
        DMS_MNTN_IncComStatsInfo(DMS_SOCK_RD_LEN_INVALID);
        return VOS_ERR;
    }

    if (DMS_COMM_CBCheck(DMS_PORT_SOCK) != VOS_OK) {
        return VOS_ERR;
    }

    portTblInfo = DMS_DEV_GetDevPortTblByPortId(DMS_PORT_SOCK);
    if (portTblInfo->RcvData == VOS_NULL_PTR) {
        DMS_LOGE("ReadData is not registered.");
        return VOS_ERR;
    }

    if (portTblInfo->RcvData(DMS_PORT_SOCK, data, len) != VOS_OK) {
        return VOS_ERR;
    } else {
        return VOS_OK;
    }
}

VOS_UINT32 DMS_SOCK_Open(struct DMS_DevInfo *dev)
{
    DMS_PORT_LOGI(dev->portId, "enter.");

    dev->handle = DMS_INIT_HANDLE;

    /* 注册Sock上行数据接收回调 */
    (VOS_VOID)mdrv_cpm_logic_rcv_reg((mdrv_cpm_logic_port_type_e)dev->devId, DMS_SOCK_RcvData);

    return VOS_OK;
}

VOS_UINT32 DMS_SOCK_SendDataAsync(struct DMS_DevInfo *dev, VOS_UINT8 *data, VOS_UINT16 len)
{
    DMS_PORT_LOGI(dev->portId, "enter.");
    DMS_MNTN_IncComStatsInfo(DMS_SOCK_WRT_ASYNC);

    if (mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) != BSP_MODULE_SUPPORT) {
        DMS_MNTN_IncComStatsInfo(DMS_SOCK_BSP_SUPPORT_ERR);
        DMS_PORT_LOGE(dev->portId, "bsp module not support.");
        return VOS_ERR;
    }

    if (mdrv_cpm_com_send(dev->devId, data, VOS_NULL_PTR, len) != VOS_OK) {
        DMS_MNTN_IncComStatsInfo(DMS_SOCK_WRT_ASYNC_ERR);
        return VOS_ERR;
    }

    return VOS_OK;
}

