/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#include "pm_om_pressure.h"
#include <bsp_softtimer.h>
#include <bsp_pm.h>
#include <bsp_pm_om.h>

#undef THIS_MODU
#define THIS_MODU mod_pmom
void pm_press_timer_cb(void *p);
struct softtimer_list g_pm_press_timer = {
    .name = "press_timer",
    .func = pm_press_timer_cb,
    .wake_type = SOFTTIMER_WAKE,
    .unit_type = TYPE_S,
};
void pm_press_timer_cb(void *p)
{
    int ret;
    ret = pm_wakeup_ccore(PM_WAKEUP_THEN_SLEEP);
    if (ret) {
        pmom_pr_err(KERN_ERR "pm_wakeup_ccore failed\n");
        return;
    }
    if (ret) {
        pmom_pr_err(KERN_ERR "pm_wakeup_nrcore failed\n");
        return;
    }
    bsp_softtimer_add(&g_pm_press_timer);
}

void pm_press_test_start(unsigned int time)
{
    int ret = 0;
    struct softtimer_list *pm_press_timer = (struct softtimer_list *)(uintptr_t)&g_pm_press_timer;
    pm_press_timer->timeout = time;
    if (pm_press_timer->init_flags == TIMER_INIT_FLAG) {
        bsp_softtimer_delete(pm_press_timer);
        bsp_softtimer_modify(pm_press_timer, time);
        bsp_softtimer_add(pm_press_timer);
    } else {
        ret = bsp_softtimer_create(pm_press_timer);
        if (ret) {
            pmom_pr_err(KERN_ERR "pm press testtimer create failed\n");
            return;
        } else {
            bsp_softtimer_add(pm_press_timer);
        }
    }
}
void pm_press_test_stop(void)
{
    struct softtimer_list *pm_press_timer = (struct softtimer_list *)(uintptr_t)&g_pm_press_timer;

    if (pm_press_timer->init_flags == TIMER_INIT_FLAG) {
        bsp_softtimer_delete(pm_press_timer);
    }
}
