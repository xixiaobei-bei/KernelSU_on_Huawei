/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*
 * 1 头文件包含
 */
#include "ppm_usb.h"
#include <securec.h>
#include <product_config.h>
#include <mdrv_usb.h>
#include "diag_port_manager.h"
#include "ppm_common.h"
#include "diag_system_debug.h"
#include "scm_ind_src.h"
#include "scm_ind_dst.h"


/*
 * 函 数 名: ppm_usb_cfg_send_data
 * 功能描述: 将输入的数据通过USB(CFG口)发送给PC侧
 * 输入参数: pucVirAddr:   数据虚地址
 * pucPhyAddr:   数据实地址
 * data_len: 数据长度
 * 输出参数: 无
 * 返 回 值: BSP_ERROR/BSP_OK
 */
u32 ppm_usb_cfg_send_data(u8 *virt_addr, u8 *phy_ddr, u32 len)
{
    return ppm_port_send(OM_USB_CFG_PORT_HANDLE, virt_addr, phy_ddr, len);
}

/*
 * 函 数 名: ppm_usb_cfg_port_close
 * 功能描述: USB承载的OM CFG端口已经消失，需要关闭CFG端口
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
void ppm_usb_cfg_port_close(void)
{
    ppm_port_close_proc(OM_USB_CFG_PORT_HANDLE, CPM_CFG_PORT);

    return;
}

/*
 * 函 数 名: GU_OamUsbCfgStatusCB
 * 功能描述: 用于ACPU上面处理USB物理端口断开后的OM链接断开
 * 输入参数: enPortState:端口状态
 * 输出参数: 无
 * 返 回 值: 无
 */
void ppm_usb_cfg_status_cb(ACM_EVT_E state)
{
    ppm_port_status(OM_USB_CFG_PORT_HANDLE, CPM_CFG_PORT, state);

    return;
}

/*
 * 函 数 名: GU_OamUsbCfgWriteDataCB
 * 功能描述: 用于处理USB承载的OM CFG端口的异步发送数据的回调
 * 输入参数: pucData:   需要发送的数据内容
 * data_len: 数据长度
 * 输出参数: 无
 * 返 回 值: 无
 */
void ppm_usb_cfg_write_data_cb(char *virt_adddr, char *phy_addr, int len)
{
    ppm_buff_info_s buff;

    buff.virt_addr = virt_adddr;
    buff.phy_addr = phy_addr;
    buff.length = len;

    ppm_port_write_asy_cb(OM_USB_CFG_PORT_HANDLE, &buff);
    return;
}

/*
 * 函 数 名: GU_OamUsbCfgReadDataCB
 * 功能描述: 用于ACPU上面底软把USB承载的OM CFG端口数据通过ICC发送给OM模块
 * 输入参数: phy_port: 物理端口
 * UdiHandle:设备句柄
 * 输出参数: 无
 * 返 回 值: BSP_ERROR/BSP_OK
 */
void ppm_usb_cfg_read_data_cb(void)
{
    ppm_read_port_data(CPM_CFG_PORT, ppm_get_udi_handle(OM_USB_CFG_PORT_HANDLE), OM_USB_CFG_PORT_HANDLE);
    return;
}

/*
 * 函 数 名: GU_OamUsbCfgPortOpen
 * 功能描述: 用于初始化USB承载的OM CFG端口
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
void ppm_usb_cfg_port_open(void)
{
    ppm_read_port_data_init(CPM_CFG_PORT, OM_USB_CFG_PORT_HANDLE, ppm_usb_cfg_read_data_cb, ppm_usb_cfg_write_data_cb,
        ppm_usb_cfg_status_cb);

    diag_crit("usb cfg port open\n");
    return;
}

/*
 * 函 数 名: ppm_usb_ind_status_cb
 * 功能描述: 用于ACPU上面处理USB物理端口断开后的OM链接断开
 * 输入参数: enPortState:   端口状态
 * 输出参数: 无
 * 返 回 值: 无
 */
void ppm_usb_ind_status_cb(ACM_EVT_E state)
{
    ppm_port_status(OM_USB_IND_PORT_HANDLE, CPM_IND_PORT, state);

    return;
}

ppm_usb_debug_info_s g_ppm_usb_debug_info = { 0 };

static inline u32 Max_UsbProcTime(u32 oldtime, u32 newtime)
{
    return (oldtime >= newtime ? oldtime : newtime);
}
void ppm_query_usb_info(void *usb_info, u32 len)
{
    errno_t ret;
    ret = memcpy_s(usb_info, len, &g_ppm_usb_debug_info, sizeof(ppm_usb_debug_info_s));
    if (ret != EOK) {
        diag_error("memory copy error %x\n", ret);
    }
}
void ppm_clear_usb_debug_info(void)
{
    memset_s(&g_ppm_usb_debug_info, sizeof(ppm_usb_debug_info_s), 0, sizeof(ppm_usb_debug_info_s));
}
/*
 * 函 数 名  : GU_OamUsbIndWriteDataCB
 * 功能描述  : 用于处理USB OM主动上报端口的异步发送数据的回调
 * 输入参数  : pucData:   需要发送的数据内容
 * data_len: 数据长度
 * 输出参数  : 无
 * 返 回 值  : 无
 */
#ifdef BSP_CONFIG_PHONE_TYPE
void ppm_usb_ind_write_data_cb(char *virt_addr, char *phy_addr, int len)
{
    ppm_buff_info_s buff;

    buff.virt_addr = virt_addr;
    buff.phy_addr = phy_addr;
    buff.length = len;

    ppm_port_write_asy_cb(OM_USB_IND_PORT_HANDLE, &buff);
    return;
}
#else
void ppm_usb_ind_write_data_cb(struct acm_write_info *acm_write_info)
{
    u32 delta_send_time;
    u32 delta_cb_time;
    ppm_buff_info_s buff;

    g_ppm_usb_debug_info.usb_cb_cnt++;

    if (acm_write_info == NULL) {
        diag_error("Param error\n");
        return;
    }

    buff.virt_addr = acm_write_info->virt_addr;
    buff.phy_addr = acm_write_info->phy_addr;
    buff.length = acm_write_info->size;

    delta_send_time = acm_write_info->complete_time - acm_write_info->submit_time;
    g_ppm_usb_debug_info.usb_send_time += delta_send_time;
    g_ppm_usb_debug_info.usb_max_send_time = Max_UsbProcTime(g_ppm_usb_debug_info.usb_max_send_time, delta_send_time);
    delta_cb_time = acm_write_info->done_time - acm_write_info->complete_time;
    g_ppm_usb_debug_info.usb_cb_time += delta_cb_time;
    g_ppm_usb_debug_info.usb_max_cb_time = Max_UsbProcTime(g_ppm_usb_debug_info.usb_max_cb_time, delta_cb_time);
    ppm_port_write_asy_cb(OM_USB_IND_PORT_HANDLE, &buff);

    return;
}
#endif
/*
 * 函 数 名: ppm_usb_ind_port_open
 * 功能描述: 用于初始化USB承载的OM主动上报端口
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
void ppm_usb_ind_port_open(void)
{
    ppm_read_port_data_init(CPM_IND_PORT, OM_USB_IND_PORT_HANDLE, NULL, ppm_usb_ind_write_data_cb,
        ppm_usb_ind_status_cb);

    diag_crit("usb ind port open\n");
    return;
}

/*
 * 函 数 名: ppm_usb_ind_port_close
 * 功能描述: USB承载的OM主动上报端口已经消失，需要关闭USB端口
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
void ppm_usb_ind_port_close(void)
{
    ppm_port_close_proc(OM_USB_IND_PORT_HANDLE, CPM_IND_PORT);

    return;
}

/*
 * 函 数 名: ppm_usb_ind_send_data
 * 功能描述: 将输入的数据通过USB主动上报端口发送给PC侧
 * 输入参数: pucVirAddr:   数据虚地址
 * pucPhyAddr:   数据实地址
 * data_len: 数据长度
 * 输出参数: 无
 * 返 回 值: BSP_ERROR/BSP_OK
 */
u32 ppm_usb_ind_send_data(u8 *virt_addr, u8 *phy_addr, u32 len)
{
    return ppm_port_send(OM_USB_IND_PORT_HANDLE, virt_addr, phy_addr, len);
}

/*
 * 函 数 名: ppm_usb_cfg_port_init
 * 功能描述: 用于USB上OM配置端口通道的初始化
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: BSP_ERROR -- 初始化失败
 * BSP_OK  -- 初始化成功
 */
u32 ppm_usb_cfg_port_init(void)
{
    mdrv_usb_reg_enablecb((usb_udi_enable_cb)ppm_usb_cfg_port_open);

    mdrv_usb_reg_disablecb((usb_udi_disable_cb)ppm_usb_cfg_port_close);

    cpm_phy_send_reg(CPM_CFG_PORT, ppm_usb_cfg_send_data);

    return BSP_OK;
}

/*
 * 函 数 名: ppm_usb_ind_port_init
 * 功能描述: 用于USB 上OM主动上报端口通道的初始化
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: BSP_ERROR -- 初始化失败
 * BSP_OK  -- 初始化成功
 */
u32 ppm_usb_ind_port_init(void)
{
    mdrv_usb_reg_enablecb((usb_udi_enable_cb)ppm_usb_ind_port_open);

    mdrv_usb_reg_disablecb((usb_udi_disable_cb)ppm_usb_ind_port_close);

    cpm_phy_send_reg(CPM_IND_PORT, ppm_usb_ind_send_data);

    return BSP_OK;
}

/*
 * 函 数 名: ppm_usb_port_init
 * 功能描述: USB承载的虚拟端口通道的初始化:包括OM IND、OM CNF等端口
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: BSP_OK   - 初始化成功
 * BSP_ERROR  - 初始化失败
 */
u32 ppm_usb_port_init(void)
{
    /* USB 承载的OM主动上报端口的初始化 */
    if (BSP_OK != ppm_usb_ind_port_init()) {
        return (u32)BSP_ERROR;
    }

    /* USB 承载的OM配置端口的初始化 */
    if (BSP_OK != ppm_usb_cfg_port_init()) {
        return (u32)BSP_ERROR;
    }

    return BSP_OK;
}

