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

/*lint --e{438,537,666,713}*/
#include "hardtimer_core.h"
#include <linux/printk.h>
#include <linux/platform_device.h>

#include <osl_module.h>
#include <bsp_dt.h>
#include <bsp_hardtimer.h>
#include <bsp_slice.h>
#include <securec.h>

#undef THIS_MODU
#define THIS_MODU mod_hardtimer
static LIST_HEAD(timer_drivers_head);

#define DRIVER_NAME "balontimer_device"

struct wakeup_timer_ctrl {
    u32 count;
    int request_id[TIMER_ID_MAX];
};

struct timercore_ctrl {
    struct timer_device timer[TIMER_ID_MAX];
    struct wakeup_timer_ctrl wakeup_timer;
    spinlock_t list_lock;
} g_core_timer_ctrl;

static inline u32 check_timer_valid(u32 timer_id)
{
    if (timer_id < TIMER_ID_MAX && g_core_timer_ctrl.timer[timer_id].clk) {
        return 1;
    } else {
        return 0;
    }
}

void bsp_hardtimer_load_value(u32 timer_id, u32 value)
{
    unsigned long flags;

    if (!check_timer_valid(timer_id)) {
        return;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    if (value < 1) {
        value = 1;
    }
    g_core_timer_ctrl.timer[timer_id].driver->ops->timer_set_init_value(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id], value);
    g_core_timer_ctrl.timer[timer_id].timer_ctrl.timeout = value;
    spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_id].lock, flags);
}

u32 bsp_get_timer_current_value(u32 timer_id)
{
    u32 ret;
    unsigned long flags;

    if (!check_timer_valid(timer_id)) {
        return 0;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    ret = g_core_timer_ctrl.timer[timer_id].driver->ops->timer_get_cur_value(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id]);
    spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    return ret;
}

int bsp_get_timer_rest_time(u32 timer_id, drv_timer_unit_e unit, unsigned int *rest)
{
    u32 timer_clk, ret;

    if (!check_timer_valid(timer_id)) {
        return BSP_ERROR;
    }
    if (rest == NULL) {
        return BSP_ERROR;
    }
    if (!g_core_timer_ctrl.timer[timer_id].driver->ops->check_timer_enabled(
            (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id])) { /* if 0, it is to say not
                                                                                            enabled */
        *rest = 0; /* osa timer request return 0 if not enabled */
        return BSP_OK;
    }
    timer_clk = g_core_timer_ctrl.timer[timer_id].clk;
    ret = bsp_get_timer_current_value(timer_id);
    if (unit == TIMER_UNIT_NONE) {
        *rest = ret;
    } else if (unit == TIMER_UNIT_US) {
        /* ret/timer_clk得到以秒为单位的时间，再乘以10^6得到对应的以微秒为单位的时间。10^6=1000*1000 */
        *rest = (ret * 1000) / (timer_clk / 1000);
    } else if (unit == TIMER_UNIT_MS) {
        /* ret/timer_clk得到以秒为单位的时间，再乘以1000得到对应的以毫秒为单位的时间 */
        *rest = ret / (timer_clk / 1000);
    } else {
        return BSP_ERROR;
    }
    return BSP_OK;
}

void bsp_hardtimer_int_unmask(u32 timer_id)
{
    unsigned long flags;

    if (!check_timer_valid(timer_id)) {
        return;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    g_core_timer_ctrl.timer[timer_id].driver->ops->timer_unmask(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id]);
    spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_id].lock, flags);
}

void bsp_hardtimer_int_mask(u32 timer_id)
{
    unsigned long flags;

    if (!check_timer_valid(timer_id)) {
        return;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    g_core_timer_ctrl.timer[timer_id].driver->ops->timer_mask(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id]);
    spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_id].lock, flags);
}

u32 bsp_hardtimer_int_status(u32 timer_id)
{
    if (!check_timer_valid(timer_id)) {
        return 0;
    }
    return g_core_timer_ctrl.timer[timer_id].driver->ops->timer_int_status(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id]);
}

void bsp_hardtimer_int_clear(u32 timer_id)
{
    if (!check_timer_valid(timer_id)) {
        return;
    }
    g_core_timer_ctrl.timer[timer_id].driver->ops->timer_int_clear(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id]);
}

s32 bsp_hardtimer_disable(u32 timer_id)
{
    unsigned long flags;
    s32 ret;

    if (!check_timer_valid(timer_id)) {
        return BSP_ERROR;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    ret = g_core_timer_ctrl.timer[timer_id].driver->ops->timer_disable(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id]);
    spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    return ret;
}

s32 bsp_hardtimer_config_init(struct bsp_hardtimer_control *timer_ctrl)
{
    s32 ret;
    unsigned long flags;

    if (!check_timer_valid(timer_ctrl->timer_id)) {
        return BSP_ERROR;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.timer[timer_ctrl->timer_id].lock, flags);
    ret = memcpy_s((void *)&g_core_timer_ctrl.timer[timer_ctrl->timer_id].timer_ctrl,
                   (size_t)sizeof(struct bsp_hardtimer_control), (const void *)timer_ctrl,
                   (size_t)sizeof(struct bsp_hardtimer_control));
    if (ret) {
        spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_ctrl->timer_id].lock, flags);
        hardtimer_print_error("memcpy_s failed,ret = %d\n", ret);
        return BSP_ERROR;
    }
    ret = g_core_timer_ctrl.timer[timer_ctrl->timer_id].driver->ops->timer_init(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_ctrl->timer_id]);
    spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_ctrl->timer_id].lock, flags);
    if (timer_ctrl->func) {
        ret = request_irq(g_core_timer_ctrl.timer[timer_ctrl->timer_id].irq, (irq_handler_t)timer_ctrl->func,
                          timer_ctrl->irq_flags, g_core_timer_ctrl.timer[timer_ctrl->timer_id].name,
                          (void *)(timer_ctrl->para));
        if (ret) {
            hardtimer_print_error("request_irq error,ret = %d\n", ret);
            return BSP_ERROR;
        }
    }
    return ret;
}

static s32 timer_start_ms(struct bsp_hardtimer_control *timer_ctrl)
{
    u32 timer_clk;
    s32 ret;

    timer_clk = g_core_timer_ctrl.timer[timer_ctrl->timer_id].clk;
    if (timer_clk % 1000) { /* 32k,minish computation error。判断时钟频率能够被1000整除，减少后续计算过程中的精度损失 */
        if (timer_ctrl->timeout < (0xFFFFFFFF / timer_clk)) {
            timer_ctrl->timeout = timer_clk * timer_ctrl->timeout / HARDTIMER_SEC_TO_MSEC;
        } else {
            timer_ctrl->timeout = timer_clk / HARDTIMER_SEC_TO_MSEC * timer_ctrl->timeout;
        }
    } else {
        timer_ctrl->timeout = timer_clk / HARDTIMER_SEC_TO_MSEC * timer_ctrl->timeout;
    }
    ret = bsp_hardtimer_config_init(timer_ctrl);
    if (ret != BSP_OK) {
        hardtimer_print_error("bsp_hardtimer_config_init error \n");
        return BSP_ERROR;
    }
    ret = bsp_hardtimer_enable(timer_ctrl->timer_id);
    return ret;
}

static s32 timer_start_us(struct bsp_hardtimer_control *timer_ctrl)
{
    u32 timer_clk;
    s32 ret;

    timer_clk = g_core_timer_ctrl.timer[timer_ctrl->timer_id].clk;
    if (timer_clk % 1000) { /* 32k,minish computation error。判断时钟频率能够被1000整除，减少后续计算过程中的精度损失 */
        if (timer_ctrl->timeout > TIMER_32K_US_BOUNDARY) {
            timer_ctrl->timeout = timer_clk * timer_ctrl->timeout / 1000000; /* 1秒=1000000微秒 */
        } else { /* 对于不大于31us的定时，直接往load寄存器写1 */
            timer_ctrl->timeout = 1;
        }
    } else {
        /* 将10^6拆成两个1000，减少除法计算的精度损失 */
        timer_ctrl->timeout = timer_clk / 1000 * timer_ctrl->timeout / 1000;
    }
    ret = bsp_hardtimer_config_init(timer_ctrl);
    if (ret != BSP_OK) {
        hardtimer_print_error("bsp_hardtimer_config_init error \n");
        return BSP_ERROR;
    }
    ret = bsp_hardtimer_enable(timer_ctrl->timer_id);
    return ret;
}

static s32 timer_start_none(struct bsp_hardtimer_control *timer_ctrl)
{
    s32 ret;

    ret = bsp_hardtimer_config_init(timer_ctrl);
    if (ret != BSP_OK) {
        hardtimer_print_error("bsp_hardtimer_config_init error \n");
        return BSP_ERROR;
    }
    ret = bsp_hardtimer_enable(timer_ctrl->timer_id);
    return ret;
}

s32 bsp_hardtimer_start(struct bsp_hardtimer_control *timer_ctrl)
{
    s32 ret;

    if (timer_ctrl == NULL) {
        hardtimer_print_error("timer_ctrl is NULL \n");
        return BSP_ERROR;
    }
    /* 直接操作寄存器 */
    if (timer_ctrl->unit == TIMER_UNIT_NONE) {
        ret = timer_start_none(timer_ctrl);
        return ret;
    } else if (timer_ctrl->unit == TIMER_UNIT_MS) {
        ret = timer_start_ms(timer_ctrl);
        return ret;
    } else if (timer_ctrl->unit == TIMER_UNIT_US) {
        ret = timer_start_us(timer_ctrl);
        return ret;
    } else {
        hardtimer_print_error("timer_ctrl->unit error \n");
        return BSP_ERROR;
    }
}

s32 bsp_hardtimer_enable(u32 timer_id)
{
    unsigned long flags;
    s32 ret;

    if (!check_timer_valid(timer_id)) {
        return BSP_ERROR;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    ret = g_core_timer_ctrl.timer[timer_id].driver->ops->timer_enable(
        (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[timer_id]);
    spin_unlock_irqrestore(&g_core_timer_ctrl.timer[timer_id].lock, flags);
    return ret;
}

s32 bsp_hardtimer_free(u32 timer_id)
{
    if (!check_timer_valid(timer_id)) {
        return BSP_ERROR;
    }
    if (bsp_hardtimer_disable(timer_id)) {
        return BSP_ERROR;
    }
    osl_free_irq(g_core_timer_ctrl.timer[timer_id].irq, g_core_timer_ctrl.timer[timer_id].timer_ctrl.func,
                 g_core_timer_ctrl.timer[timer_id].timer_ctrl.para);
    g_core_timer_ctrl.timer[timer_id].timer_ctrl.func = NULL;
    g_core_timer_ctrl.timer[timer_id].timer_ctrl.para = NULL;
    return BSP_OK;
}

/* 获取唤醒源timer的下一个最近到时时间，供低功耗模块使用 */
u32 get_next_schedule_time(u32 *min_timer_id)
{
    u32 i;
    u32 min = 0xffffffff; /*lint !e123*/
    u32 ret = 0;
    u32 request_id;
    u32 tmp = 0;

    if (min_timer_id == NULL) {
        return (u32)BSP_ERROR;
    }
    for (i = 0; i < g_core_timer_ctrl.wakeup_timer.count; i++) {
        request_id = g_core_timer_ctrl.wakeup_timer.request_id[i];
        if (!g_core_timer_ctrl.timer[request_id].driver->ops->check_timer_enabled(
                (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[request_id])) {
            ret = 0xFFFFFFFF;
        } else {
            (void)bsp_get_timer_rest_time(request_id, TIMER_UNIT_MS, (unsigned int *)&ret);
        }
        if (ret < min) { /*lint !e123*/
            min = ret;   /*lint !e123*/
            tmp = request_id;
        }
    }
    *min_timer_id = tmp;
    return min; /*lint !e123*/
}

void bsp_timer_driver_register(struct timer_driver *driver)
{
    unsigned long flags;

    if (driver == NULL) {
        return;
    }
    spin_lock_irqsave(&g_core_timer_ctrl.list_lock, flags);
    list_add_tail(&driver->timer_drivers, &timer_drivers_head);
    spin_unlock_irqrestore(&g_core_timer_ctrl.list_lock, flags);
    return;
}

static struct timer_driver *get_timer_driver(struct timer_device *device)
{
    struct timer_driver *drv = NULL;
    unsigned long flags;

    spin_lock_irqsave(&g_core_timer_ctrl.list_lock, flags);
    if (!list_empty(&timer_drivers_head)) {
        list_for_each_entry(drv, &timer_drivers_head, timer_drivers)
        {
            if (drv->periphid == device->periphid) {
                spin_unlock_irqrestore(&g_core_timer_ctrl.list_lock, flags);
                return drv;
            }
        }
    }
    spin_unlock_irqrestore(&g_core_timer_ctrl.list_lock, flags);

    return NULL;
}

int get_timer_info(device_node_s *child_node, u32 timer_id)
{
    int ret_value;

    /* get timer base info,include baseaddr\irq\clk\sr_flag\name\periphid */
    g_core_timer_ctrl.timer[timer_id].base_addr = bsp_dt_iomap(child_node, 0);
    if (g_core_timer_ctrl.timer[timer_id].base_addr == NULL) {
        hardtimer_print_error("timer request_id %d iomap failed.\n", timer_id);
        return BSP_ERROR;
    }

    g_core_timer_ctrl.timer[timer_id].irq = bsp_dt_irq_parse_and_map(child_node, 0);
    ret_value = bsp_dt_property_read_u32(child_node, "clock-frequency", &g_core_timer_ctrl.timer[timer_id].clk);
    if (ret_value || g_core_timer_ctrl.timer[timer_id].clk <= 0) {
        hardtimer_print_error("timer %d get frequency failed.\n", timer_id);
        return BSP_ERROR;
    }
    ret_value = bsp_dt_property_read_u32(child_node, "sr_flag", &g_core_timer_ctrl.timer[timer_id].sr_flag);
    if (ret_value) {
        hardtimer_print_error("timer %d get sr_flag failed.\n", timer_id);
        return BSP_ERROR;
    }
    ret_value = bsp_dt_property_read_string(child_node, "timer_name",
        (const char **)&g_core_timer_ctrl.timer[timer_id].name);
    if (ret_value) {
        hardtimer_print_error("timer %d get timer_name failed.\n", timer_id);
        return BSP_ERROR;
    }
    ret_value = bsp_dt_property_read_u32(child_node, "periphid", &g_core_timer_ctrl.timer[timer_id].periphid);
    if (ret_value) {
        hardtimer_print_error("timer %d get periphid failed.\n", timer_id);
        return BSP_ERROR;
    }
    return BSP_OK;
}

static int _timer_device_init(device_node_s *_node)
{
    device_node_s *node = NULL;
    device_node_s *child_node = NULL;
    u32 index = 0;
    u32 wake_flag = 0;
    int ret_value;
    struct timer_driver *drv = NULL;

    node = _node;
    bsp_dt_for_each_available_child_of_node(node, child_node)
    {
        ret_value = bsp_dt_property_read_u32(child_node, "request_id", &index);
        if (ret_value) {
            hardtimer_print_error("request_id get failed.\n");
            return BSP_ERROR;
        }
        /* judge is wake timer or not */
        if (bsp_dt_property_read_u32(child_node, "wakesrc", &wake_flag)) {
            hardtimer_print_error("wakesrc get failed.\n");
            return BSP_ERROR;
        }
        if (wake_flag) {
            g_core_timer_ctrl.wakeup_timer.request_id[g_core_timer_ctrl.wakeup_timer.count] = index;
            g_core_timer_ctrl.wakeup_timer.count++;
        }

        if (get_timer_info(child_node, index) == BSP_ERROR) {
            return BSP_ERROR;
        }

        drv = get_timer_driver(&g_core_timer_ctrl.timer[index]);
        if (drv != NULL) {
            g_core_timer_ctrl.timer[index].driver = drv;
        } else {
            hardtimer_print_error("timer %d drv get failed.\n", index);
            return BSP_ERROR;
        }
        spin_lock_init(&g_core_timer_ctrl.timer[index].lock);
        // 自研timer的频率设置从之前的sysctrl中移到ip内部
        g_core_timer_ctrl.timer[index].driver->ops->set_freq(&g_core_timer_ctrl.timer[index]);
    }

    return BSP_OK;
}

static int timer_device_init(void)
{
    device_node_s *node = NULL;

    node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,timer_device");
    if (node != NULL) {
        return _timer_device_init(node);
    } else {
        return BSP_ERROR;
    }
}

static int bsp_hardtimer_init(void)
{
    s32 ret;

    ret = memset_s(&g_core_timer_ctrl, sizeof(struct timercore_ctrl), 0x0, sizeof(struct timercore_ctrl));
    if (ret) {
        hardtimer_print_error("memsets failed,ret=0x%x\n", ret);
        return BSP_ERROR;
    }
    spin_lock_init(&g_core_timer_ctrl.list_lock);
    arm_timer_drviver_init();
    ret = timer_device_init();
    if (ret) {
        hardtimer_print_error("timer_device_init err\n");
        return BSP_ERROR;
    }
    ret = set_adp_timer_isr_and_pm();
    if (ret) {
        hardtimer_print_error("set_adp_timer_isr_and_pm err\n");
        return BSP_ERROR;
    } else {
        hardtimer_print_error("hardtimer init ok\n");
        return BSP_OK;
    }
}

static int bsp_timer_probe_stub(struct platform_device *dev)
{
    return BSP_OK;
}

#ifdef CONFIG_PM

static s32 timer_suspend_noirq(struct device *dev)
{
    int i;
    u32 stamp;

    pr_info("%s +\n", __func__);
    /* 遍历timer，保存sr_flag为1，即为掉电区timer的sysctrl reg和load reg 寄存器值 */
    for (i = 0; i < TIMER_ID_MAX; i++) {
        if (g_core_timer_ctrl.timer[i].sr_flag && g_core_timer_ctrl.timer[i].driver->ops->suspend) {
            g_core_timer_ctrl.timer[i].driver->ops->suspend(
                (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[i]);
        } else {/* g_core_timer_ctrl[i].sr_flag */
            continue;
        }
    }

    stamp = bsp_get_slice_value();
    while (get_timer_slice_delta(stamp, bsp_get_slice_value()) < 0x2) {
    }
    pr_info("%s -\n", __func__);
    return BSP_OK;
}

static s32 timer_resume_noirq(struct device *dev)
{
    u32 i;

    pr_info("%s +\n", __func__);
    for (i = 0; i < TIMER_ID_MAX; i++) {
        if (g_core_timer_ctrl.timer[i].sr_flag && g_core_timer_ctrl.timer[i].driver->ops->resume) {
            g_core_timer_ctrl.timer[i].driver->ops->resume(
                (struct timer_device *)(uintptr_t)&g_core_timer_ctrl.timer[i]);
        } else {
            continue;
        }
    }
    pr_info("%s -\n", __func__);
    return BSP_OK;
}

static const struct dev_pm_ops g_timer_pm_ops = {
    .suspend_noirq = timer_suspend_noirq,
    .resume_noirq = timer_resume_noirq,
};

#define BALONG_DEV_PM_OPS (&g_timer_pm_ops)
#else
#define BALONG_DEV_PM_OPS NULL
#endif
static struct platform_driver g_balong_timer_driver = {
    .probe = bsp_timer_probe_stub,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .pm = BALONG_DEV_PM_OPS,
    },
};

static struct platform_device g_timer_device = {
    .name = DRIVER_NAME,
    .id = -1,
    .num_resources = 0,
};

int hi_timer_init(void)
{
    s32 ret;

    ret = bsp_hardtimer_init();
    if (ret) {
        hardtimer_print_error("bsp_hardtimer_init failed!\n");
        return ret;
    }

    // 注册timer低功耗钩子，初始化probe打桩
    ret = platform_device_register(&g_timer_device);
    if (ret) {
        hardtimer_print_error("g_balong_timer_driver device register is failed!\n");
        return ret;
    }

    ret = platform_driver_register(&g_balong_timer_driver);
    if (ret) {
        hardtimer_print_error("g_balong_timer_driver deriver register is failed!\n");
        platform_device_unregister(&g_timer_device);
        return ret;
    }
    return BSP_OK;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
arch_initcall(hi_timer_init);
#endif

void bsp_timer_show(void)
{
    u32 i;

    hardtimer_print_error("idx periphid   freq   irq   srflag    name\n");
    for (i = 0; i < TIMER_ID_MAX; i++) {
        hardtimer_print_error("%d  0x%x    %d     %d      %d      %s\n", i, g_core_timer_ctrl.timer[i].periphid,
                              g_core_timer_ctrl.timer[i].clk, g_core_timer_ctrl.timer[i].irq,
                              g_core_timer_ctrl.timer[i].sr_flag, g_core_timer_ctrl.timer[i].name);
    }
}

/*lint -save -e19*/
EXPORT_SYMBOL(bsp_hardtimer_int_clear);
EXPORT_SYMBOL(bsp_hardtimer_int_mask);
EXPORT_SYMBOL(bsp_hardtimer_int_unmask);
EXPORT_SYMBOL(bsp_hardtimer_int_status);
EXPORT_SYMBOL(bsp_hardtimer_config_init);
EXPORT_SYMBOL(bsp_hardtimer_start);
EXPORT_SYMBOL(bsp_hardtimer_enable);
EXPORT_SYMBOL(bsp_hardtimer_disable);
EXPORT_SYMBOL(bsp_hardtimer_free);
EXPORT_SYMBOL(get_next_schedule_time);
/*lint -restore +e19*/
