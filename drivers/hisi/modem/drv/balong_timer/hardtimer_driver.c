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

#include <bsp_hardtimer.h>
#include "hardtimer_core.h"
#include "hardtimer_driver.h"
#undef THIS_MODU
#define THIS_MODU mod_hardtimer
/*lint --e{129, 63 ,666,732,746}*/
/*lint -save -e14*/
#define TIMER_SET_MIN_VALUE 2

static u32 is_timer_enabled(struct timer_device *device)
{
    return device->ctrl_reg.timeren;
}

static void timer_set_value(struct timer_device *device, unsigned int value)
{
    if (value < TIMER_SET_MIN_VALUE) {
        value = TIMER_SET_MIN_VALUE;
    }
    writel_relaxed(value, (void *)((uintptr_t)device->base_addr + TIMER_LOAD_OFFSET));
}

static u32 timer_current_value(struct timer_device *device)
{
    if (device->ctrl_reg.timeren) {
        return readl_relaxed((void *)((uintptr_t)device->base_addr + TIMER_VALUE_OFFSET));
    } else {
        return 0x0;
    }
}

static u32 timer_int_status(struct timer_device *device)
{
    u32 ret;
    ret = readl_relaxed((void *)((uintptr_t)device->base_addr + TIMER_INTMIS_OFFSET));
    return ret & 0x1U;
}

static void timer_int_clear(struct timer_device *device)
{
    writel_relaxed(0x1, (void *)((uintptr_t)device->base_addr + TIMER_INTCLR_OFFSET));
}

static s32 timer_disable(struct timer_device *device)
{
    device->ctrl_reg.timeren = 0;
    writel_relaxed(device->ctrl_reg.ctrl_val, (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
    timer_int_clear(device);
    return BSP_OK;
}

/*lint -save -e550*/
static void timer_set_freq(struct timer_device *device)
{
    return;
}

static s32 timer_config_init(struct timer_device *device)
{
    if (device->ctrl_reg.timeren == 1) {
        return BSP_ERROR;
    }

    device->ctrl_reg.ctrl_val = 0;  // need clear ctrl val
    device->ctrl_reg.timersize = 1; // 使用32bit位宽计时
    if (device->timer_ctrl.int_enable) {
        device->ctrl_reg.internable = TIMER_INT_ENABLE;
    } else {
        device->ctrl_reg.internable = TIMER_INT_DISABLE;
    }

    device->ctrl_reg.timer_reload = 1; // 自研timer定时器计数到0后重载timer_init值

    if (device->timer_ctrl.mode == TIMER_ONCE_COUNT) {
        device->ctrl_reg.oneshot = 1;
        device->ctrl_reg.timermode = 0;
    } else if (device->timer_ctrl.mode == TIMER_PERIOD_COUNT) {
        device->ctrl_reg.oneshot = 0;
        device->ctrl_reg.timermode = 1;
    } else {
        hardtimer_print_error("timer_ctrl->mode error \n");
        return BSP_ERROR;
    }
    writel_relaxed(device->ctrl_reg.ctrl_val, (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
    timer_set_value(device, device->timer_ctrl.timeout);
    return BSP_OK;
}

static s32 timer_enable(struct timer_device *device)
{
    if (device->ctrl_reg.timeren == 1) {
        return BSP_ERROR;
    }
    device->ctrl_reg.timeren = 1;
    writel_relaxed(device->ctrl_reg.ctrl_val, (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
    return BSP_OK;
}

static void timer_int_mask(struct timer_device *device)
{
    device->ctrl_reg.internable = TIMER_INT_DISABLE;
    writel_relaxed(device->ctrl_reg.ctrl_val, (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
}

static void timer_int_unmask(struct timer_device *device)
{
    device->ctrl_reg.internable = TIMER_INT_ENABLE;
    writel_relaxed(device->ctrl_reg.ctrl_val, (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
}

/*lint -restore +e550*/

static void timer_suspend(struct timer_device *device)
{
    if (device->timer_ctrl.timer_id != TIMER_ACPU_OM_TCXO_ID) {
        device->lp_timer_valuereg = readl((void *)((uintptr_t)device->base_addr + TIMER_LOAD_OFFSET));
    } else {
        device->lp_timer_valuereg = timer_current_value(device);
    }
    device->lp_timer_ctrlreg = readl((void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
    if (IS_TIMER_ENABLE(device->lp_timer_ctrlreg)) {
        /* 1. disable & mask timer */
        writel(DISABLE_AND_MASK_TIMER(device->lp_timer_ctrlreg),
            (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
        /* 2. clear timer interrupt */
        writel(0x1, (void *)((uintptr_t)device->base_addr + TIMER_INTCLR_OFFSET));
    }
    return;
}

static void timer_resume(struct timer_device *device)
{
    u32 ret;
    /* 1. 自研timer设置频率，arm timer打桩，在sysctrl中整体配置        */
    timer_set_freq(device);
    /* 1. 在load值之前，先统一配为32bit计时，load值之后，再恢复保存的控制寄存器的值 */
    ret = readl_relaxed((void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
    writel_relaxed(ret | 0x2, (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
    /* 2.恢复配置初始值 */
    timer_set_value(device, device->lp_timer_valuereg);
    /* 3.恢复控制寄存器值 */
    writel(device->lp_timer_ctrlreg, (void *)((uintptr_t)device->base_addr + TIMER_CTRL_OFFSET));
    return;
}

static struct timer_core_ops g_arm_timer_ops = {
    .timer_init = timer_config_init,
    .timer_enable = timer_enable,
    .timer_disable = timer_disable,
    .timer_mask = timer_int_mask,
    .timer_unmask = timer_int_unmask,
    .timer_get_cur_value = timer_current_value,
    .timer_int_clear = timer_int_clear,
    .timer_int_status = timer_int_status,
    .timer_set_init_value = timer_set_value,
    .check_timer_enabled = is_timer_enabled,
    .suspend = timer_suspend,
    .resume = timer_resume,
    .set_freq = timer_set_freq,
};

static struct timer_driver g_arm_timer_driver = {
    .ops = &g_arm_timer_ops,
    .periphid = 0x0000A000,
    .driver_name = "arm timer",
};

void arm_timer_drviver_init(void)
{
    INIT_LIST_HEAD(&g_arm_timer_driver.timer_drivers);
    bsp_timer_driver_register(&g_arm_timer_driver);
}

/*lint -restore +e14*/
