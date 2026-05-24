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

/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/rtc.h>

#include <asm/system_misc.h>

#include <product_config.h>
#include <mdrv_sysboot.h>
#include <mdrv_chg.h>
#include <bsp_icc.h>
#include <bsp_onoff.h>
#include "power_exchange.h"
#include "mdrv_chg.h"
#include <securec.h>

#include "power_off_mbb.h"
#include <bsp_print.h>
#define THIS_MODU mod_onoff

/*****************************************************************************
 函 数 名  : bsp_start_mode_get
 功能描述  : 用于获取开机模式
 输入参数  :
 输出参数  : 无n

 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
int bsp_start_mode_get(void)
{
    return DRV_START_MODE_NORMAL;
}

/*****************************************************************************
 函 数 名  : bsp_power_icc_send_state
 功能描述  : C核核间通信函数
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static void bsp_power_icc_send_state(void)
{
    int ret;
    int mode;
    u32 icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

    mode = bsp_start_mode_get();

    ret = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (u8*)&mode, (u32)sizeof(mode));
    if (ret != (int)sizeof(mode))
    {
        bsp_debug("send len(%x) != expected len(%lu)\n", ret, (unsigned long)sizeof(mode));
    }
}

/*****************************************************************************
 函 数 名  : bsp_power_ctrl_read_cb
 功能描述  : C核核间回调函数
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static s32 bsp_power_ctrl_read_cb(void)
{
    int rt = 0;
    int read_len;
    stCtrlMsg msg;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

    read_len = bsp_icc_read(channel_id, (u8*)&msg, (u32)sizeof(stCtrlMsg));
    if(read_len != (int)sizeof(stCtrlMsg))
    {
        bsp_debug("read len(%x) != expected len(%lu)\n", read_len, (unsigned long)sizeof(stCtrlMsg));
        return -1;
    }

    bsp_debug("[onoff]bsp_power_ctrl_read_cb is called, msg: 0x%x\n", msg.pwr_type);

    switch(msg.pwr_type)
    {
    case E_POWER_ON_MODE_GET:
        bsp_power_icc_send_state();
        break;
    case E_POWER_SHUT_DOWN:
        /*Here is to judge whether to shut down or restart */
        if ( DRV_SHUTDOWN_RESET == msg.reason )
        {
            bsp_drv_power_reboot();
        }
        else
        {
            drv_shut_down( msg.reason, POWER_OFF_MONITOR_TIMEROUT );
        }
        break;
    case E_POWER_POWER_OFF:
        /*To shut down right now */
        drv_shut_down( DRV_SHUTDOWN_POWER_KEY, 0 );
        break;
    case E_POWER_POWER_REBOOT:
        bsp_drv_power_reboot();
        break;
    default:
        bsp_debug("invalid ctrl by ccore\n");
        break;
    }

    return rt;
}

/*****************************************************************************
 函 数 名  : his_boot_probe
 功能描述  : power on
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static int __init his_boot_probe(struct platform_device *pdev)
{
    int rt;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

    rt = bsp_icc_event_register(channel_id, (read_cb_func)bsp_power_ctrl_read_cb, NULL, NULL, NULL);
    if(rt != 0){
        bsp_debug("icc event register failed.\n");
    }

    return rt;
}

static struct platform_device his_boot_dev = {
    .name = "his_boot",
    .id = 0,
    .dev = {
    .init_name = "his_boot",
    },/*lint !e785*/
};/*lint !e785*/

static struct platform_driver his_boot_drv = {
    .probe      = his_boot_probe,
    .driver     = {
        .name   = "his_boot",
        .owner  = THIS_MODULE,/*lint !e64*/
    },/*lint !e785*/
};/*lint !e785*/

int __init his_boot_init(void)
{
    int ret;

    bsp_debug("his_boot_init.\n");

    ret = platform_device_register(&his_boot_dev);
    if(ret)
    {
        bsp_debug("register his_boot device failed.\n");
        return ret;
    }

    ret = platform_driver_register(&his_boot_drv);/*lint !e64*/
    if(ret)
    {
        bsp_debug("register his_boot driver failed.\n");
        platform_device_unregister(&his_boot_dev);
    }

    return ret;
}
late_initcall(his_boot_init);
