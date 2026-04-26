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
#include "dms_debug.h"
#if (FEATURE_IOT_CMUX == FEATURE_ON)
#include "dms_port_cmux.h"
#include "cmux_drv_ifc.h"
#endif



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_DMS_PORT_CMUX_C


#if (FEATURE_IOT_CMUX == FEATURE_ON)
struct DMS_CMUX_Info g_dmsCmuxInfo = {0};

VOS_UINT32 DMS_PORT_IsCmuxPort(DMS_PortIdUint16 portId)
{
    if(DMS_PORT_GetCmuxOpenFlg() != VOS_TRUE) {
        return VOS_FALSE;
    }

    if ((portId == DMS_PORT_CMUXAT) ||
        (portId == DMS_PORT_CMUXMDM) ||
        (portId == DMS_PORT_CMUXEXT) ||
        (portId == DMS_PORT_CMUXGPS)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 DMS_PORT_SwitchUart2Cmux(DMS_PortIdUint16 portId, cmux_info_type* CmuxInfo)
{
    VOS_UINT32 cmuxPhyPortIsHsuart = VOS_FALSE;

    /* 关闭原有的HSUART端口 */
    DMS_PORT_ClosePort(portId);

    /* 当前CMUX是基于UART还是HSUART */
    DMS_PORT_SetCmuxPhyPort(portId);

    cmuxPhyPortIsHsuart = (portId == DMS_PORT_HSUART) ? VOS_TRUE : VOS_FALSE;

    /* 初始化CMUX功能接口 */
    if (cmux_init_port(CmuxInfo, cmuxPhyPortIsHsuart) == VOS_FALSE) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_VOID DMS_PORT_SwitchCmux2Uart(VOS_VOID)
{
    struct DMS_PortInfo    *portInfo = VOS_NULL_PTR;
    DMS_LOGI("enter.");

    portInfo = DMS_PORT_GetPortInfo(DMS_PORT_GetCmuxPhyPort());

    DMS_DEV_Open(portInfo->devInfo);

    DMS_SendIntraEvent(ID_DMS_EVENT_UART_INIT, portInfo->portId, VOS_NULL_PTR, 0);

    return;
}

VOS_VOID DMS_PORT_RcvMscModeRead(DMS_PortIdUint16 portId)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;
    lineCtrl = DMS_PORT_GetUartLineCtrl();

    DMS_LOGI("enter.");

    /* 如果DSR管脚设置成一直有效，发送DSR管脚为高电平 */
    if (lineCtrl->dsrMode == DMS_PORT_UART_DSR_MODE_ALWAYS_ON) {
        DMS_PORT_AssertDsr(portId);
    }

    /* 如果DCD管脚设置成一直有效，发送DCD管脚为高电平 */
    if (lineCtrl->dcdMode == DMS_PORT_UART_DCD_MODE_ALWAYS_ON) {
        DMS_PORT_AssertDcd(portId);
    }

    return;
}

VOS_UINT8 DMS_PORT_GetCmuxActivePppPstaskDlc(VOS_VOID)
{
    return cmux_get_active_ppp_pstask_dlc();
}

VOS_VOID DMS_PORT_SetCmuxActivePppPstaskDlc(VOS_UINT8 dlc)
{
    cmux_set_active_ppp_pstask_dlc(dlc);
    return;
}

DMS_PortIdUint16 DMS_PORT_GetCmuxPhyPort(VOS_VOID)
{
    return g_dmsCmuxInfo.cmuxPhyPort;
}

VOS_VOID DMS_PORT_SetCmuxPhyPort(DMS_PortIdUint16 portId)
{
#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (portId == DMS_PORT_HSUART) {
        g_dmsCmuxInfo.cmuxPhyPort = DMS_PORT_HSUART;
    }
#endif

    if (portId == DMS_PORT_UART) {
        g_dmsCmuxInfo.cmuxPhyPort = DMS_PORT_UART;
    }
}

VOS_UINT32 DMS_PORT_GetCmuxOpenFlg(VOS_VOID)
{
    return g_dmsCmuxInfo.cmuxOpen;
}

VOS_VOID DMS_PORT_SetCmuxOpenFlg(VOS_UINT32 cmuxOpenFlg)
{
    g_dmsCmuxInfo.cmuxOpen = cmuxOpenFlg;
}

VOS_UINT32 DMS_PORT_CmuxPhyPortIsHsuart(VOS_VOID)
{
#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (DMS_PORT_GetCmuxPhyPort() == DMS_PORT_HSUART) {
        return VOS_TRUE;
    }
#endif

    return VOS_FALSE;
}

cmux_info_type* DMS_PORT_GetCmuxInfoType(VOS_VOID)
{
    return cmux_get_cmux_info_type();
}
#endif

