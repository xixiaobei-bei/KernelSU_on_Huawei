/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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


/*****************************************************************************
  1 头文件包含
**************************************************************************** */
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include "scm_cnf_src.h"
#include "scm_cnf_dst.h"
#include "diag_port_manager.h"
#include "diag_system_debug.h"
#include "OmCommonPpm.h"
#include "OmUsbPpm.h"


/* ****************************************************************************
  2 全局变量定义
**************************************************************************** */

/*****************************************************************************
  3 外部引用声明
*****************************************************************************/


/*****************************************************************************
  4 函数实现
*****************************************************************************/




u32 PPM_UsbCfgSendData(u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen)
{
    return PPM_PortSend(OM_USB_CFG_PORT_HANDLE, pucVirAddr, pucPhyAddr, ulDataLen);
}



void PPM_UsbCfgPortClose(void)
{
    PPM_PortCloseProc(OM_USB_CFG_PORT_HANDLE, CPM_CFG_PORT);

    return;
}


void PPM_UsbCfgStatusCB(ACM_EVT_E enPortState)
{
    PPM_PortStatus(OM_USB_CFG_PORT_HANDLE, CPM_CFG_PORT, enPortState);

    return;
}


void PPM_UsbCfgWriteDataCB(u8* pucVirData, u8* pucPhyData, s32 lLen)
{
    PPM_PortWriteAsyCB(OM_USB_CFG_PORT_HANDLE, pucVirData, lLen);

    return;
}


s32 PPM_UsbCfgReadDataCB(void)
{
    return PPM_ReadPortData(CPM_CFG_PORT, g_astOMPortUDIHandle[OM_USB_CFG_PORT_HANDLE], OM_USB_CFG_PORT_HANDLE);
}


void PPM_UsbCfgPortOpen(void)
{
    PPM_ReadPortDataInit(CPM_CFG_PORT,
                            OM_USB_CFG_PORT_HANDLE,
                            PPM_UsbCfgReadDataCB,
                            PPM_UsbCfgWriteDataCB,
                            PPM_UsbCfgStatusCB);

    diag_crit("usb cfg port open\n");
    return;
}


void PPM_UsbIndStatusCB(ACM_EVT_E enPortState)
{
    PPM_PortStatus(OM_USB_IND_PORT_HANDLE, CPM_IND_PORT, enPortState);

    return;
}


void PPM_UsbIndWriteDataCB(u8* pucVirData, u8* pucPhyData, s32 lLen)
{

    PPM_PortWriteAsyCB(OM_USB_IND_PORT_HANDLE, pucVirData, lLen);

    return;
}


void PPM_UsbIndPortOpen(void)
{
    PPM_ReadPortDataInit(CPM_IND_PORT,
                            OM_USB_IND_PORT_HANDLE,
                            NULL,
                            PPM_UsbIndWriteDataCB,
                            PPM_UsbIndStatusCB);
    
    diag_crit("usb ind port open\n");
    return;
}


void PPM_UsbIndPortClose(void)
{
    PPM_PortCloseProc(OM_USB_IND_PORT_HANDLE, CPM_IND_PORT);

    return;
}


u32 PPM_UsbIndSendData(u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen)
{
    return PPM_PortSend(OM_USB_IND_PORT_HANDLE, pucVirAddr, pucPhyAddr, ulDataLen);
}


u32 PPM_UsbCfgPortInit(void)
{
    mdrv_usb_reg_enablecb((USB_UDI_ENABLE_CB_T)PPM_UsbCfgPortOpen);

    mdrv_usb_reg_disablecb((USB_UDI_DISABLE_CB_T)PPM_UsbCfgPortClose);

    CPM_PhySendReg(CPM_CFG_PORT,  PPM_UsbCfgSendData);

    return BSP_OK;
}


u32 PPM_UsbIndPortInit(void)
{
    mdrv_usb_reg_enablecb((USB_UDI_ENABLE_CB_T)PPM_UsbIndPortOpen);

    mdrv_usb_reg_disablecb((USB_UDI_DISABLE_CB_T)PPM_UsbIndPortClose);

    CPM_PhySendReg(CPM_IND_PORT,  PPM_UsbIndSendData);

    return BSP_OK;
}



u32 PPM_UsbPortInit(void)
{
    /* USB 承载的OM主动上报端口的初始化 */
    if (BSP_OK != PPM_UsbIndPortInit())
    {
        return (u32)BSP_ERROR;
    }

    /* USB 承载的OM配置端口的初始化 */
    if (BSP_OK != PPM_UsbCfgPortInit())
    {
        return (u32)BSP_ERROR;
    }

    return BSP_OK;
}





