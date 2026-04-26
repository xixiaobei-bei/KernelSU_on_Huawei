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

/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/rtc.h>
#include <osl_list.h>
#include <product_config.h>
#include <mdrv_sysboot.h>
#include <bsp_icc.h>
#include <bsp_onoff.h>
#include <securec.h>
#include <bsp_print.h>
#include <hi_sysboot.h>
#include <bsp_slice.h>
#include "power_off_mbb.h"
#include "onoff_msg.h"
#include "power_exchange.h"
#include "bsp_modem_boot.h"
#define THIS_MODU mod_onoff

static unsigned int g_modem_lr_state = 0;
static unsigned int g_modem_nr_state = 0;

/*
 * 功能描述: C核核间通信函数
 */
static void bsp_power_icc_send_state(void)
{
    int ret;
    int mode;
    u32 icc_channel_id = (ICC_CHN_IFC << 16U) | IFC_RECV_FUNC_ONOFF;

    mode = bsp_start_mode_get();

    ret = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (u8 *)&mode, (u32)sizeof(mode));
    if (ret != (int)sizeof(mode)) {
        bsp_err("send len(%x) != expected len(%lu)\n", ret, (unsigned long)sizeof(mode));
    }
}
/*
 * 功能描述: 5GCPU状态读取接口函数
 */
int mdrv_sysboot_is_nr_modem_ready(void)
{
    return g_modem_nr_state;
}
/*
 * 功能描述: 4GCPU状态读取接口函数
 */
int mdrv_sysboot_is_modem_ready(void)
{
    return g_modem_lr_state;
}

/*
 * 功能描述: modem ready后清除warm boot cnt标志
 */
void onoff_clear_warm_boot_cnt(void)
{
    if ((g_modem_lr_state == 1) && (g_modem_nr_state == 1)) {
        /* clear warm boot cnt */
        power_on_wdt_cnt_set();
        bsp_err("clear warm boot cnt \n");
    }
}

/*
 * 功能描述: C核核间回调函数
 */
static s32 bsp_power_ctrl_read_cb(u32 channel_id, u32 len, void *context)
{
    int ret = 0;
    int read_len;
    tag_ctrl_msg_s msg = {0};
    read_len = bsp_icc_read(channel_id, (u8 *)&msg, (u32)sizeof(tag_ctrl_msg_s));
    if (read_len != (int)sizeof(tag_ctrl_msg_s)) {
        bsp_err("read len(%x) != expected len(%lu)\n", read_len, (unsigned long)sizeof(tag_ctrl_msg_s));
        return -1;
    }

    bsp_info("[onoff]bsp_power_ctrl_read_cb is called, msg: 0x%x\n", msg.pwr_type);

    switch (msg.pwr_type) {
        case E_POWER_ON_MODE_GET:
            bsp_power_icc_send_state();
            break;
        case E_MODE_NRCPU_DRV_READY:
            bsp_err("modem nrcpu drv ready, time slice %d\n", bsp_get_elapse_ms());
            break;
        case E_MODE_LRCPU_DRV_READY:
            bsp_err("modem lrcpu drv ready, time slice %d\n", bsp_get_elapse_ms());
            break;
        case E_MODE_NRCPU_READY:
            g_modem_nr_state = 1;
            onoff_clear_warm_boot_cnt();
            bsp_err("bsp_power_ctrl_read_cb modem nrcpu ready, time slice %d\n", bsp_get_elapse_ms());
            break;
        case E_MODE_LRCPU_READY:
            g_modem_lr_state = 1;
            onoff_clear_warm_boot_cnt();
            bsp_err("bsp_power_ctrl_read_cb modem lrcpu ready, time slice %d\n", bsp_get_elapse_ms());
            (void)mdrv_set_modem_state(MODEM_INIT_OK);
            break;
        case E_POWER_SHUT_DOWN:
            /* Here is to judge whether to shut down or restart */
            if (msg.reason == DRV_SHUTDOWN_RESET) {
                bsp_drv_power_reboot();
            } else {
                drv_shut_down(msg.reason, POWER_OFF_MONITOR_TIMEROUT);
            }
            break;
        case E_POWER_POWER_OFF:
            /* To shut down right now */
            drv_shut_down(DRV_SHUTDOWN_POWER_KEY, 0);
            break;
        case E_POWER_POWER_REBOOT:
            bsp_drv_power_reboot();
            break;
        default:
            bsp_err("invalid ctrl by ccore\n");
    }

    return ret;
}


int onoff_icc_init_process(void)
{
    int ret;
    ret = bsp_icc_event_register(ICC_CHN_IFC << 16U | IFC_RECV_FUNC_ONOFF, (read_cb_func)bsp_power_ctrl_read_cb, NULL,
                                 NULL, NULL);
    if (ret != 0) {
        bsp_err("icc event register failed.\n");
        return ret;
    }

#ifdef HI_ONOFF_WITH_NR
    ret = bsp_icc_event_register(ICC_CHN_NRIFC << 16U | NRIFC_RECV_FUNC_SYSBOOT, (read_cb_func)bsp_power_ctrl_read_cb,
                                 NULL, NULL, NULL);
    if (ret != 0) {
        bsp_err("icc event register failed.\n");
        return ret;
    }
#endif

    return ret;
}

