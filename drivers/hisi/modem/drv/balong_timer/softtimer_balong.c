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

#include <linux/irq.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <osl_module.h>
#include <osl_sem.h>
#include <osl_list.h>
#include <osl_spinlock.h>
#include <bsp_softtimer.h>
#include <osl_thread.h>
#include <bsp_hardtimer.h>
#include <mdrv_timer.h>
#include "softtimer_balong.h"
#include <bsp_dt.h>
#include <bsp_print.h>
#include <securec.h>

#undef THIS_MODU
#define THIS_MODU mod_softtimer
#define softtimer_print(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))

struct debug_wakeup_softtimer {
    u32 wakeup_flag;
    const char *wakeup_timer_name;
    u32 wakeup_timer_id;
} g_debug_wakeup_timer;

struct softtimer_ctrl {
    unsigned char timer_id_alloc[SOFTTIMER_MAX_NUM];
    struct list_head timer_list_head;
    u32 softtimer_start_value;
    u32 hard_timer_id;
    spinlock_t timer_list_lock;
    osl_sem_id soft_timer_sem;
    OSL_TASK_ID softtimer_task;
    u32 clk;        /* hardtimer work freq */
    u32 support;    /* whether support this type softtimer */
    u32 wake_times; /* softtimer wake system times */
    u64 start_slice;
    slice_curtime get_curtime;
    slice_value get_slice_value;
    struct wakeup_source wake_lock;
};

/*lint --e{64,456}*/
static struct softtimer_ctrl g_softtimer_ctrl[2]; /* 有2个类型的softtimer，0：wake, 1：nwake */

u32 check_softtimer_support_type(enum wakeup type)
{
    if ((type != SOFTTIMER_WAKE) && (type != SOFTTIMER_NOWAKE)) {
        softtimer_print("check_softtimer_support_type wake type %d, invalid\n", type);
        return (u32)BSP_ERROR;
    }
    return g_softtimer_ctrl[(u32)type].support;
}

static void start_hard_timer(struct softtimer_ctrl *p_softtimer_ctrl, u32 ulvalue)
{
    p_softtimer_ctrl->softtimer_start_value = ulvalue;
    (void)p_softtimer_ctrl->get_curtime((u64 *)(uintptr_t)&p_softtimer_ctrl->start_slice);
    bsp_hardtimer_disable(p_softtimer_ctrl->hard_timer_id);
    bsp_hardtimer_load_value(p_softtimer_ctrl->hard_timer_id, ulvalue);
    bsp_hardtimer_enable(p_softtimer_ctrl->hard_timer_id);
}

static void stop_hard_timer(struct softtimer_ctrl *p_softtimer_ctrl)
{
    bsp_hardtimer_disable(p_softtimer_ctrl->hard_timer_id);
    p_softtimer_ctrl->softtimer_start_value = ELAPESD_TIME_INVAILD;
}

static inline u32 calculate_timer_start_value(u64 expect_cb_slice, u64 cur_slice)
{
    if (expect_cb_slice > cur_slice) {
        return (u32)(expect_cb_slice - cur_slice);
    } else {
        return 0;
    }
}

/*
 * bsp_softtimer_add,add the timer to the list;
 */
void bsp_softtimer_add(struct softtimer_list *timer)
{
    struct softtimer_list *p = NULL;
    unsigned long flags;
    u64 now_slice;

    if (timer == NULL) {
        softtimer_print("timer to be added is NULL\n");
        return;
    }
    if (unlikely((timer->wake_type != SOFTTIMER_WAKE) && (timer->wake_type != SOFTTIMER_NOWAKE))) {
        softtimer_print("wake type %d, invalid\n", timer->wake_type);
        return;
    }
    spin_lock_irqsave(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
    if (!list_empty(&timer->entry)) {
        spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
        return;
    }
    (void)g_softtimer_ctrl[timer->wake_type].get_curtime((u64 *)(uintptr_t)&now_slice);
    timer->expect_cb_slice = timer->count_num + now_slice;
    list_for_each_entry(p, &(g_softtimer_ctrl[timer->wake_type].timer_list_head), entry)
    {
        if (p->expect_cb_slice > timer->expect_cb_slice) {
            break;
        }
    }
    list_add_tail(&(timer->entry), &(p->entry));
    if (g_softtimer_ctrl[timer->wake_type].timer_list_head.next == &(timer->entry)) {
        if ((timer->entry.next) != (&(g_softtimer_ctrl[timer->wake_type].timer_list_head))) {
            p = list_entry(timer->entry.next, struct softtimer_list, entry);

            if (p->is_running == TIMER_TRUE) {
                p->is_running = TIMER_FALSE;
            }
        }
        timer->is_running = TIMER_TRUE;
        start_hard_timer(&g_softtimer_ctrl[timer->wake_type],
                         calculate_timer_start_value(timer->expect_cb_slice, now_slice));
    }
    spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
}

s32 bsp_softtimer_delete(struct softtimer_list *timer)
{
    struct softtimer_list *p = NULL;
    unsigned long flags;
    u64 now_slice;

    if (timer == NULL) {
        softtimer_print("NULL pointer \n");
        return BSP_ERROR;
    }
    if (unlikely((timer->wake_type != SOFTTIMER_WAKE) && (timer->wake_type != SOFTTIMER_NOWAKE))) {
        softtimer_print("wake type %d, invalid\n", timer->wake_type);
        return BSP_ERROR;
    }
    spin_lock_irqsave(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
    (void)g_softtimer_ctrl[timer->wake_type].get_curtime((u64 *)(uintptr_t)&now_slice);
    if (list_empty(&timer->entry)) {
        spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
        return NOT_ACTIVE;
    } else {
        /* if the timer bo be deleted is the first node */
        if ((timer->entry.prev == &(g_softtimer_ctrl[timer->wake_type].timer_list_head)) &&
            (timer->entry.next != &(g_softtimer_ctrl[timer->wake_type].timer_list_head))) {
            timer->is_running = TIMER_FALSE;
            list_del_init(&(timer->entry));
            p = list_first_entry(&(g_softtimer_ctrl[timer->wake_type].timer_list_head), struct softtimer_list, entry);

            start_hard_timer(&g_softtimer_ctrl[p->wake_type],
                             calculate_timer_start_value(p->expect_cb_slice, now_slice));
            p->is_running = TIMER_TRUE;
        } else {
            timer->is_running = TIMER_FALSE;
            list_del_init(&(timer->entry));
        }
    }
    /* if the list is empty after delete node, then stop hardtimer */
    if (list_empty(&(g_softtimer_ctrl[timer->wake_type].timer_list_head))) {
        stop_hard_timer(&g_softtimer_ctrl[timer->wake_type]);
    }
    spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
    return BSP_OK;
}

static inline s32 set_timer_expire_time_s(struct softtimer_list *timer, u32 new_expire_time)
{
    u32 timer_freq;

    timer_freq = g_softtimer_ctrl[timer->wake_type].clk;
    if (timer->timeout > U32_MAX_VALUE / timer_freq) {
        softtimer_print("time too long\n");
        return BSP_ERROR;
    }
    timer->count_num = timer_freq * new_expire_time;
    return BSP_OK;
}

static inline s32 set_timer_expire_time_ms(struct softtimer_list *timer, u32 new_expire_time)
{
    u32 timer_freq;

    timer_freq = g_softtimer_ctrl[timer->wake_type].clk;
    if (timer->timeout > U32_MAX_VALUE / timer_freq * SLICE_CONVERT_DELTA) {
        softtimer_print("time too long\n");
        return BSP_ERROR;
    }
    if (timer_freq % SLICE_CONVERT_DELTA) {
        if ((new_expire_time) < (U32_MAX_VALUE / timer_freq)) {
            timer->count_num = (timer_freq * new_expire_time) / SLICE_CONVERT_DELTA;
        } else {
            timer->count_num = timer_freq * (new_expire_time / SLICE_CONVERT_DELTA);
        }
    } else {
        timer->count_num = (timer_freq / SLICE_CONVERT_DELTA) * new_expire_time;
    }
    return BSP_OK;
}

static inline s32 set_timer_expire_time(struct softtimer_list *timer, u32 expire_time)
{
    if (timer->unit_type == TYPE_S) {
        return set_timer_expire_time_s(timer, expire_time);
    } else if (timer->unit_type == TYPE_MS) {
        return set_timer_expire_time_ms(timer, expire_time);
    } else {
        return BSP_ERROR;
    }
}

s32 bsp_softtimer_modify(struct softtimer_list *timer, u32 new_expire_time)
{
    if ((timer == NULL) || (!list_empty(&timer->entry))) {
        return BSP_ERROR;
    }
    if (unlikely((timer->wake_type != SOFTTIMER_WAKE) && (timer->wake_type != SOFTTIMER_NOWAKE))) {
        softtimer_print("wake type 0x%d, invalid\n", timer->wake_type);
        return BSP_ERROR;
    }
    timer->timeout = new_expire_time;
    return set_timer_expire_time(timer, new_expire_time);
}

s32 bsp_softtimer_create(struct softtimer_list *timer)
{
    s32 ret, i;
    unsigned long flags;

    if (timer == NULL) {
        softtimer_print("para is null\n");
        return BSP_ERROR;
    }
    if (unlikely((timer->wake_type != SOFTTIMER_WAKE) && (timer->wake_type != SOFTTIMER_NOWAKE))) {
        softtimer_print("wake type %d, invalid\n", timer->wake_type);
        return BSP_ERROR;
    }
    if (!g_softtimer_ctrl[(u32)timer->wake_type].support) {
        softtimer_print("wake type not support\n");
        return BSP_ERROR;
    }
    if (timer->init_flags == TIMER_INIT_FLAG) {
        softtimer_print("timer already create\n");
        return BSP_ERROR;
    }
    spin_lock_irqsave(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags); /*lint !e550 */
    INIT_LIST_HEAD(&(timer->entry));
    timer->is_running = TIMER_FALSE;
    ret = set_timer_expire_time(timer, timer->timeout);
    if (ret) {
        spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
        return BSP_ERROR;
    }
    for (i = 0; i < SOFTTIMER_MAX_NUM; i++) {
        if (g_softtimer_ctrl[timer->wake_type].timer_id_alloc[i] == 0) {
            timer->timer_id = i;
            g_softtimer_ctrl[timer->wake_type].timer_id_alloc[i] = 1;
            break;
        }
    }
    if (i == SOFTTIMER_MAX_NUM) {
        spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
        softtimer_print("error,not enough timerid for alloc, already 40 exists\n");
        return BSP_ERROR;
    }
    timer->init_flags = TIMER_INIT_FLAG;
    spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
    return BSP_OK;
}

s32 bsp_softtimer_free(struct softtimer_list *timer)
{
    unsigned long flags;

    if ((timer == NULL) || (!list_empty(&timer->entry))) {
        return BSP_ERROR;
    }
    if (unlikely((timer->wake_type != SOFTTIMER_WAKE) && (timer->wake_type != SOFTTIMER_NOWAKE))) {
        softtimer_print("wake type %d, invalid\n", timer->wake_type);
        return BSP_ERROR;
    }
    spin_lock_irqsave(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags); /*lint !e550*/
    timer->init_flags = 0;
    g_softtimer_ctrl[timer->wake_type].timer_id_alloc[timer->timer_id] = 0;
    spin_unlock_irqrestore(&(g_softtimer_ctrl[timer->wake_type].timer_list_lock), flags);
    return BSP_OK;
}

static void _softtimer_task_func(struct softtimer_ctrl *_p_softtimer_ctrl)
{
    struct softtimer_ctrl *p_softtimer_ctrl = NULL;
    struct softtimer_list *p = NULL, *q = NULL;
    unsigned long flags;
    u64 now_slice;
    u32 temp1, temp2;

    p_softtimer_ctrl = _p_softtimer_ctrl;
    spin_lock_irqsave(&p_softtimer_ctrl->timer_list_lock, flags);
    (void)p_softtimer_ctrl->get_curtime((u64 *)(uintptr_t)&now_slice);
    p_softtimer_ctrl->softtimer_start_value = ELAPESD_TIME_INVAILD;
    list_for_each_entry_safe(p, q, &(p_softtimer_ctrl->timer_list_head), entry)
    {
        if (!p->emergency) {
            if (now_slice >= p->expect_cb_slice) {
                list_del_init(&p->entry);
                p->is_running = TIMER_FALSE;
                spin_unlock_irqrestore(&p_softtimer_ctrl->timer_list_lock, flags);
                temp1 = bsp_get_slice_value();
                if (p->func) {
                    p->func(p->para);
                }
                temp2 = bsp_get_slice_value();
                p->run_cb_delta = get_timer_slice_delta(temp1, temp2);
                spin_lock_irqsave(&p_softtimer_ctrl->timer_list_lock, flags);
                (void)p_softtimer_ctrl->get_curtime((u64 *)(uintptr_t)&now_slice);
            } else {
                break;
            }
        } else {
            break;
        }
    }
    if (!list_empty(&(p_softtimer_ctrl->timer_list_head))) { /* 如果还有未超时定时器 */
        p = list_first_entry(&(p_softtimer_ctrl->timer_list_head), struct softtimer_list, entry);
        p->is_running = TIMER_TRUE;
        start_hard_timer(p_softtimer_ctrl, calculate_timer_start_value(p->expect_cb_slice, now_slice));
    } else {
        stop_hard_timer(p_softtimer_ctrl);
    }
    if (p_softtimer_ctrl->hard_timer_id == ACORE_SOFTTIMER_ID) {
        __pm_relax(&p_softtimer_ctrl->wake_lock); /*lint !e455*/
    }
    if (g_debug_wakeup_timer.wakeup_flag) {
        softtimer_print("wakeup timer name:%s,wakeup_timer_id:%d", g_debug_wakeup_timer.wakeup_timer_name,
                        g_debug_wakeup_timer.wakeup_timer_id);
        g_debug_wakeup_timer.wakeup_flag = 0;
    }
    spin_unlock_irqrestore(&p_softtimer_ctrl->timer_list_lock, flags);
}

int softtimer_task_func(void *data)
{
    struct softtimer_ctrl *p_softtimer_ctrl = NULL;

    p_softtimer_ctrl = (struct softtimer_ctrl *)data;
    for (;;) {
        osl_sem_down(&p_softtimer_ctrl->soft_timer_sem);
        _softtimer_task_func(p_softtimer_ctrl);
    }
    /*lint -save -e527*/
    return 0;
    /*lint -restore +e527*/
}

static void _do_softtimer_emergency(struct softtimer_ctrl *_p_softtimer_ctrl)
{
    struct softtimer_ctrl *p_softtimer_ctrl = _p_softtimer_ctrl;
    u32 temp1, temp2;
    u64 now_slice;
    struct softtimer_list *p = NULL, *q = NULL;
    unsigned long flags;

    (void)p_softtimer_ctrl->get_curtime((u64 *)(uintptr_t)&now_slice);
    spin_lock_irqsave(&p_softtimer_ctrl->timer_list_lock, flags); /*lint !e550*/
    list_for_each_entry_safe(p, q, &p_softtimer_ctrl->timer_list_head, entry)
    {
        if (p->emergency) {
            if (now_slice >= p->expect_cb_slice) {
                list_del_init(&p->entry);
                p->is_running = TIMER_FALSE;
                spin_unlock_irqrestore(&p_softtimer_ctrl->timer_list_lock, flags); /*lint !e550*/
                temp1 = bsp_get_slice_value();
                if (p->func) {
                    p->func(p->para);
                }
                temp2 = bsp_get_slice_value();
                p->run_cb_delta = get_timer_slice_delta(temp1, temp2);
                spin_lock_irqsave(&p_softtimer_ctrl->timer_list_lock, flags); /*lint !e550*/
            } else {
                break;
            }
        } else {
            break;
        }
    }
    spin_unlock_irqrestore(&p_softtimer_ctrl->timer_list_lock, flags); /*lint !e550*/
}

OSL_IRQ_FUNC(static irqreturn_t, softtimer_interrupt_call_back, irq, dev)
{
    struct softtimer_ctrl *p_softtimer_ctrl = NULL;
    u32 readValue;

    p_softtimer_ctrl = dev;
    readValue = bsp_hardtimer_int_status(p_softtimer_ctrl->hard_timer_id);
    if (readValue != 0) {
        bsp_hardtimer_int_clear(p_softtimer_ctrl->hard_timer_id);
        _do_softtimer_emergency(p_softtimer_ctrl);
        if (p_softtimer_ctrl->hard_timer_id == ACORE_SOFTTIMER_ID) {
            __pm_stay_awake(&p_softtimer_ctrl->wake_lock); /*lint !e454*/
        }
        osl_sem_up(&p_softtimer_ctrl->soft_timer_sem); /*lint !e456*/
    }
    return IRQ_HANDLED; /*lint !e454*/ /*lint !e456*/
}

static int get_softtimer_int_stat(int arg)
{
    struct softtimer_list *p = NULL;

    g_softtimer_ctrl[SOFTTIMER_WAKE].wake_times++;
    if (!list_empty(&(g_softtimer_ctrl[SOFTTIMER_WAKE].timer_list_head))) {
        p = list_first_entry(&(g_softtimer_ctrl[SOFTTIMER_WAKE].timer_list_head), struct softtimer_list,
                             entry); /*lint !e826*/
        g_debug_wakeup_timer.wakeup_timer_name = p->name;
        g_debug_wakeup_timer.wakeup_timer_id = p->timer_id;
        g_debug_wakeup_timer.wakeup_flag = 1;
    }
    return 0;
}

int get_softtimer_info(void)
{
    s32 ret;
    device_node_s *node = NULL;

    node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,softtimer_support_type");
    if (node == NULL) {
        softtimer_print("softtimer_support_type get failed.\n");
        return BSP_ERROR;
    }
    if (!bsp_dt_device_is_available(node)) {
        softtimer_print("softtimer_support_type status not ok.\n");
        return BSP_ERROR;
    }
    g_debug_wakeup_timer.wakeup_flag = 0;
    g_debug_wakeup_timer.wakeup_timer_id = ELAPESD_TIME_INVAILD;
    ret = bsp_dt_property_read_u32(node, "support_wake", &g_softtimer_ctrl[SOFTTIMER_WAKE].support);
    if (ret) {
        softtimer_print(" softtimer support_wake  get failed.\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32(node, "wake-frequency", &g_softtimer_ctrl[SOFTTIMER_WAKE].clk);
    if (ret) {
        softtimer_print(" softtimer wake-frequency  get failed.\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32(node, "support_unwake", &g_softtimer_ctrl[SOFTTIMER_NOWAKE].support);
    if (ret) {
        softtimer_print(" softtimer support_unwake  get failed.\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32(node, "unwake-frequency", &g_softtimer_ctrl[SOFTTIMER_NOWAKE].clk);
    if (ret) {
        softtimer_print(" softtimer unwake-frequency  get failed.\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

int softtimer_init(struct bsp_hardtimer_control timer_ctrl)
{
    s32 ret;

    if (g_softtimer_ctrl[SOFTTIMER_WAKE].support) {
        if (osl_task_init("softtimer_wake", TIMER_TASK_WAKE_PRI, TIMER_TASK_STK_SIZE, (void *)softtimer_task_func,
                          (void *)&g_softtimer_ctrl[SOFTTIMER_WAKE], /*lint !e611 */
                          &g_softtimer_ctrl[SOFTTIMER_WAKE].softtimer_task) == ERROR) {
            softtimer_print("softtimer_wake task create failed\n");
            return BSP_ERROR;
        }
        timer_ctrl.para = (void *)&g_softtimer_ctrl[SOFTTIMER_WAKE];
        timer_ctrl.timer_id = ACORE_SOFTTIMER_ID;
        timer_ctrl.irq_flags = IRQF_NO_SUSPEND;
        ret = bsp_hardtimer_config_init(&timer_ctrl);
        if (ret) {
            softtimer_print("bsp_hardtimer_alloc error,softtimer init failed 2\n");
            return BSP_ERROR;
        }
        wakeup_source_init(&g_softtimer_ctrl[SOFTTIMER_WAKE].wake_lock, "softtimer_wake");
        g_softtimer_ctrl[SOFTTIMER_WAKE].get_curtime = bsp_slice_getcurtime;
        g_softtimer_ctrl[SOFTTIMER_WAKE].get_slice_value = bsp_get_slice_value;
        mdrv_timer_debug_register(g_softtimer_ctrl[SOFTTIMER_WAKE].hard_timer_id, (funcptr_1)get_softtimer_int_stat, 0);
    }
    if (g_softtimer_ctrl[SOFTTIMER_NOWAKE].support) {
        if (osl_task_init("softtimer_nowake", TIMER_TASK_NOWAKE_PRI, TIMER_TASK_STK_SIZE, (void *)softtimer_task_func,
                          (void *)&g_softtimer_ctrl[SOFTTIMER_NOWAKE], /*lint !e611 */
                          &g_softtimer_ctrl[SOFTTIMER_NOWAKE].softtimer_task) == ERROR) {
            softtimer_print("softtimer_normal task create failed\n");
            return BSP_ERROR;
        }
        timer_ctrl.para = (void *)&g_softtimer_ctrl[SOFTTIMER_NOWAKE];
        timer_ctrl.timer_id = ACORE_SOFTTIMER_NOWAKE_ID;
        timer_ctrl.irq_flags = 0;
        ret = bsp_hardtimer_config_init(&timer_ctrl);
        if (ret) {
            softtimer_print("bsp_hardtimer_alloc error,softtimer init failed 2\n");
            return BSP_ERROR;
        }
        /* 判断时钟频率能够被1000整除，32K时钟频率用32K时间戳获取slice，19.2M用19.2M时间戳获取slice */
        if (g_softtimer_ctrl[SOFTTIMER_NOWAKE].clk % 1000) {
            g_softtimer_ctrl[SOFTTIMER_NOWAKE].get_curtime = bsp_slice_getcurtime;
            g_softtimer_ctrl[SOFTTIMER_NOWAKE].get_slice_value = bsp_get_slice_value;
        } else {
            g_softtimer_ctrl[SOFTTIMER_NOWAKE].get_curtime = bsp_slice_getcurtime_hrt;
            g_softtimer_ctrl[SOFTTIMER_NOWAKE].get_slice_value = bsp_get_slice_value_hrt;
        }
        wakeup_source_init(&g_softtimer_ctrl[SOFTTIMER_NOWAKE].wake_lock, "softtimer_nowake");
    }
    return BSP_OK;
}

int bsp_softtimer_init(void)
{
    s32 ret;
    struct bsp_hardtimer_control timer_ctrl;

    ret = memset_s((void *)&timer_ctrl, sizeof(struct bsp_hardtimer_control), 0, sizeof(struct bsp_hardtimer_control));
    if (ret) {
        softtimer_print("memset_s failed.ret=0x%x\n", ret);
    }
    INIT_LIST_HEAD(&(g_softtimer_ctrl[SOFTTIMER_WAKE].timer_list_head));
    INIT_LIST_HEAD(&(g_softtimer_ctrl[SOFTTIMER_NOWAKE].timer_list_head));
    g_softtimer_ctrl[SOFTTIMER_NOWAKE].hard_timer_id = ACORE_SOFTTIMER_NOWAKE_ID;
    g_softtimer_ctrl[SOFTTIMER_WAKE].hard_timer_id = ACORE_SOFTTIMER_ID;
    g_softtimer_ctrl[SOFTTIMER_WAKE].softtimer_start_value = ELAPESD_TIME_INVAILD;
    g_softtimer_ctrl[SOFTTIMER_NOWAKE].softtimer_start_value = ELAPESD_TIME_INVAILD;
    osl_sem_init(SEM_EMPTY, &(g_softtimer_ctrl[SOFTTIMER_NOWAKE].soft_timer_sem));
    osl_sem_init(SEM_EMPTY, &(g_softtimer_ctrl[SOFTTIMER_WAKE].soft_timer_sem));
    spin_lock_init(&(g_softtimer_ctrl[SOFTTIMER_WAKE].timer_list_lock));
    spin_lock_init(&(g_softtimer_ctrl[SOFTTIMER_NOWAKE].timer_list_lock));
    timer_ctrl.func = (irq_handler_t)softtimer_interrupt_call_back;
    timer_ctrl.mode = TIMER_ONCE_COUNT;
    timer_ctrl.timeout = 0xffffffff; /* default value set 0xFFFFFFFF */
    timer_ctrl.unit = TIMER_UNIT_NONE;
    timer_ctrl.int_enable = 1; // enable

    if (get_softtimer_info() == BSP_ERROR) {
        return BSP_ERROR;
    }

    if (softtimer_init(timer_ctrl) == BSP_ERROR) {
        return BSP_ERROR;
    }

    softtimer_print("softtimer init success\n");
    return BSP_OK;
}

s32 show_list(u32 wake_type)
{
    struct softtimer_list *timer = NULL;
    unsigned long flags;
    u64 now_slice;

    softtimer_print("softttimer wakeup %d times\n", g_softtimer_ctrl[wake_type].wake_times);
    (void)g_softtimer_ctrl[wake_type].get_curtime((u64 *)(uintptr_t)&now_slice);
    softtimer_print("id name  expect_cb  now_slice  cb_cost  emerg\n");
    softtimer_print("----------------------------------------------------------------------------------\n");
    spin_lock_irqsave(&(g_softtimer_ctrl[wake_type].timer_list_lock), flags);
    list_for_each_entry(timer, &(g_softtimer_ctrl[wake_type].timer_list_head), entry)
    {
        softtimer_print("%d %s  0x%x  0x%x  %d  %d\n", timer->timer_id, timer->name, (u32)timer->expect_cb_slice,
                        (u32)now_slice, timer->run_cb_delta, timer->emergency);
    }
    spin_unlock_irqrestore(&(g_softtimer_ctrl[wake_type].timer_list_lock), flags);
    return BSP_OK;
}

EXPORT_SYMBOL(bsp_softtimer_create);
EXPORT_SYMBOL(bsp_softtimer_delete);
EXPORT_SYMBOL(bsp_softtimer_modify);
EXPORT_SYMBOL(bsp_softtimer_add);
EXPORT_SYMBOL(bsp_softtimer_free);
EXPORT_SYMBOL(check_softtimer_support_type);
EXPORT_SYMBOL(show_list);
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
arch_initcall(bsp_softtimer_init);
#endif
