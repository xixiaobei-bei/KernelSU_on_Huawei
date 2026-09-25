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

#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include "osl_types.h"
#include "osl_io.h"
#include "osl_bio.h"
#include "bsp_dump.h"
#include "bsp_ipc.h"
#include "bsp_memmap.h"
#include "bsp_wdt.h"
#include "bsp_icc.h"
#include "bsp_onoff.h"
#include "bsp_nvim.h"
#include "bsp_softtimer.h"
#include "bsp_version.h"
#include "of.h"
#include "dump_config.h"
#undef	THIS_MODU
#define THIS_MODU mod_dump
#if defined(BSP_CONFIG_PHONE_TYPE)
u32 g_wdt_int_no = 0;

wdt_timeout_cb g_wdt_reboot_func = NULL;
wdt_timeout_cb g_nr_wdt_reboot_func = NULL;
#define NR_WDT_MAX_NUM  (4)
struct wdt_irq_info{
    u32 irq_no;
    u32 flag;
    const char * wdt_name;
};
struct wdt_irq_info g_wdt_info[NR_WDT_MAX_NUM] = {{0},{0},{0},{0}};
static irqreturn_t watchdog_mdm_int(int irq, void *dev_id)
{
    disable_irq_nosync(g_wdt_int_no);

    dump_ok("wdt irq,irq:0x%x\n",irq);
    if(g_wdt_reboot_func)
    {
        g_wdt_reboot_func();
    }
    return IRQ_HANDLED;
}

static irqreturn_t watchdog_nr_mdm_int(int irq, void *dev_id)
{
    disable_irq_nosync(irq);

    dump_ok("nr wdt irq,irq:0x%x\n",irq);
    if(g_nr_wdt_reboot_func)
    {
        g_nr_wdt_reboot_func();
    }
    return IRQ_HANDLED;
}

void bsp_wdt_irq_disable(WDT_CORE_ID core_id)
{
    int i = 0;
    if(g_wdt_int_no && core_id == WDT_CCORE_ID)
    {
        disable_irq_nosync(g_wdt_int_no);
    }
    else if (core_id == WDT_NRCCPU_ID)
    {
        for (i = 0; i < NR_WDT_MAX_NUM; i++)
        {
            if (g_wdt_info[i].irq_no)
            {
                disable_irq_nosync(g_wdt_info[i].irq_no);
            }
        }
    }
}

void bsp_wdt_irq_enable(WDT_CORE_ID core_id)
{

}

int bsp_nr_wdt_init(void * func)
{
    unsigned int flag = 0;
    int ret = 0;
    u32 i = 0;
    struct device_node *child_node = NULL;
    struct device_node *node = NULL;
    node = of_find_compatible_node(NULL, NULL, "hisilicon,watchdog_app_nr");
    if (node == NULL)
    {
        dump_error("no wdt nr dts node\n");
        return BSP_ERROR;
    }
    for_each_child_of_node(node, child_node)
    {
		(void)of_property_read_u32(child_node, "flag", &flag);
		if(flag)
        {
	        g_wdt_info[i].irq_no = (u32)irq_of_parse_and_map(child_node, 0);
	        if(g_wdt_info[i].irq_no == 0)
	        {
	            dump_error("read wdt nr %d irq fail\n", i);
	            return BSP_ERROR;
	        }
	        g_wdt_info[i].wdt_name = of_get_property(child_node, "wdt_name", NULL);
	        if (g_wdt_info[i].wdt_name == NULL)
	        {
	            dump_error("read wdt nr name %d irq fail\n", i);
	            return BSP_ERROR;
	        }
	        ret = request_irq(g_wdt_info[i].irq_no, watchdog_nr_mdm_int, 0, g_wdt_info[i].wdt_name, NULL);
	        if (ret != 0)
	        {
	            dump_error("request wdt nr %d irq fail\n", i);
	            return BSP_ERROR;
	        }
		}
        i++;
        if (i >= NR_WDT_MAX_NUM)
        {
            dump_error("child node num is %d\n", i);
            break;
        }
    }
    g_nr_wdt_reboot_func = func;
	return BSP_OK; 
}
int bsp_lr_wdt_init(void *func)
{
    unsigned int flag = 0;

    struct device_node *node = NULL;
    node = of_find_compatible_node(NULL, NULL, "hisilicon,watchdog_app");
    if(node)
    {
        (void)of_property_read_u32(node, "flag", &flag);
        if(flag)
        {
            g_wdt_int_no = (u32)irq_of_parse_and_map(node, 0);
            if (BSP_OK != request_irq(g_wdt_int_no, watchdog_mdm_int, 0, "watchdog mdm", NULL))
            {
                dump_error(" mdm watchdog int err\n");
            }
        }
    }
    else
    {
        return BSP_ERROR;
    }
    g_wdt_reboot_func = func;
    return BSP_OK;    
}
int bsp_wdt_register_hook(WDT_CORE_ID core_id, void *func)
{
    if (core_id == WDT_CCORE_ID)
    {
        return bsp_lr_wdt_init(func);
    }
    else if (core_id == WDT_NRCCPU_ID)
    {
        return bsp_nr_wdt_init(func);
    }
    dump_error(" wdt core id err %d\n", (s32)core_id);
    return BSP_ERROR;
}
void bsp_wdt_reinit(void)
{
    int i = 0;
    if (g_wdt_reboot_func != NULL && g_wdt_int_no) {
        free_irq(g_wdt_int_no, NULL);
        if (BSP_OK != request_irq(g_wdt_int_no, watchdog_mdm_int, 0, "watchdog mdm", NULL)) {
            dump_error(" mdm watchdog reint err\n");
        }
    }
    if (g_nr_wdt_reboot_func) {
        for (i = 0; i < NR_WDT_MAX_NUM && g_wdt_info[i].irq_no; i++) {
             free_irq(g_wdt_info[i].irq_no, NULL);
            if (BSP_OK != request_irq(g_wdt_info[i].irq_no, watchdog_nr_mdm_int, 0, g_wdt_info[i].wdt_name, NULL)) {
                dump_error(" nr watchdog reint err %d\n", i);
            }
        }
    }
}
#endif
