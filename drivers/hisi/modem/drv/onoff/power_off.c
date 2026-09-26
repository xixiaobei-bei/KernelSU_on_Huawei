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

/*lint --e{537,559,715} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include <linux/reboot.h>

#include <product_config.h>
#include <osl_bio.h>
#include <osl_list.h>
#include <osl_types.h>
#include <osl_spinlock.h>
#include <mdrv_chg.h>
#include <mdrv_sysboot.h>
#include <bsp_pmu.h>
#include "power_exchange.h"
#include <bsp_reset.h>
#include <bsp_onoff.h>
#include <bsp_sysctrl.h>
#include "mdrv_chg.h"
#include "bsp_dump.h"
#include <hi_sysboot.h>
#include "bsp_llt.h"
#include "bsp_icc.h"
#include "power_off_mbb.h"
#include <bsp_print.h>

#define THIS_MODU mod_onoff

/*
 * Function:  drv_shut_down
 * Description: start the power off process right now or a few second later, and no reboot flow.
 * eReason : shutdown reason.
 * delay_in_ms: timing shutdown time in ms
 * Note: the ONLY entry of normal shutdown
 */
void drv_shut_down(drv_shutdown_reason_e en_reason, unsigned int delay_in_ms)
{
}
EXPORT_SYMBOL_GPL(drv_shut_down);
/*
 * Function:  bsp_drv_power_reboot
 * Description: same as drv_power_off, the public API
 * Note: the ONLY entry of Normal restart
 */
void bsp_drv_power_reboot(void)
{
    if (!is_in_llt()) {
        bsp_err("bsp_drv_power_reboot is called, modem reset...\n");
        system_error(DRV_ERROR_USER_RESET, 0, 0, NULL, 0);
    }
}

/*
 * The following is hisi drv native packaged interfaces for other modules( OM/TAF and so on )
 */
void mdrv_sysboot_restart(void)
{
    bsp_drv_power_reboot();
}
EXPORT_SYMBOL_GPL(mdrv_sysboot_restart);

/*
 * Function:  bsp_drv_power_reboot_direct
 * Note: reboot immediately
 */
void bsp_drv_power_reboot_direct(void)
{
    if (!is_in_llt()) {
        system_error(DRV_ERROR_USER_RESET, 0, 0, NULL, 0);
    }
}
EXPORT_SYMBOL_GPL(bsp_drv_power_reboot_direct);

void mdrv_sysboot_power_off_cb_register(void (*hook)(void))
{
}
EXPORT_SYMBOL_GPL(mdrv_sysboot_power_off_cb_register);

void mdrv_sysboot_reboot_cb_register(void (*hook)(void))
{
}
EXPORT_SYMBOL_GPL(mdrv_sysboot_reboot_cb_register);

void mdrv_sysboot_shutdown(void)
{
    drv_shut_down(DRV_SHUTDOWN_TEMPERATURE_PROTECT, 0);
}
EXPORT_SYMBOL_GPL(mdrv_sysboot_shutdown);
