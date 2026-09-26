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

/*lint --e{537,648 }*/
#include <linux/syscore_ops.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include <osl_module.h>
#include <osl_bio.h>

#include <mdrv_timer.h>
#include <drv_comm.h>
#include <bsp_dt.h>
#include <bsp_hardtimer.h>
#include <hardtimer_core.h>
#include <bsp_slice.h>
#include <bsp_version.h>
#include <securec.h>

#undef THIS_MODU
#define THIS_MODU mod_hardtimer
/* begin: add for timer debug */
#define ADP_TIMER_ENABLE_TIMES 30
#define ADP_TIMER_CALLBACK_TIMES 30
#define ADP_TIMER_GET_TIMES 50

struct adp_timer_enable_dbg {
    u32 enable_cnt;
    u32 enable_slice[ADP_TIMER_ENABLE_TIMES];
    u32 disable_slice[ADP_TIMER_ENABLE_TIMES];
    u32 init_time[ADP_TIMER_ENABLE_TIMES];
    u32 expect_cb_slice[ADP_TIMER_ENABLE_TIMES];
};

struct adp_timer_callback_dbg {
    u32 callback_slice_cnt;
    u32 callback_slice[ADP_TIMER_CALLBACK_TIMES];
};

struct adp_timer_get_dbg {
    u32 get_cnt;
    u32 get_times[ADP_TIMER_GET_TIMES][2]; /* 记录2条debug信息：当前时间戳和剩下的时间 */
};
struct adp_timer_control {
    char *name;
    funcptr_1 routine;
    int args;
    u32 irq_num;
    struct bsp_hardtimer_control my_hardtimer;
    /* for debug */
    struct adp_timer_enable_dbg stEnable;
    struct adp_timer_callback_dbg stCallback;
    struct adp_timer_get_dbg stGet;
    funcptr_1 debug_routine;
    int debug_args;
};

struct adp_timer_control g_adp_timer_ctrl[TIMER_ID_MAX];

void timer_enable_stamp_dbg(u32 clkid, u32 init_time)
{
    struct adp_timer_enable_dbg *pstEnable_dbg = &g_adp_timer_ctrl[clkid].stEnable;

    pstEnable_dbg->enable_slice[pstEnable_dbg->enable_cnt] = bsp_get_slice_value();
    pstEnable_dbg->init_time[pstEnable_dbg->enable_cnt] = init_time;
    pstEnable_dbg->expect_cb_slice[pstEnable_dbg->enable_cnt] =
        init_time + pstEnable_dbg->enable_slice[pstEnable_dbg->enable_cnt];
}

void timer_disable_stamp_dbg(u32 clkid)
{
    struct adp_timer_enable_dbg *pstEnable_dbg = &g_adp_timer_ctrl[clkid].stEnable;

    pstEnable_dbg->enable_cnt = pstEnable_dbg->enable_cnt % ADP_TIMER_ENABLE_TIMES;
    pstEnable_dbg->disable_slice[pstEnable_dbg->enable_cnt] = bsp_get_slice_value();
    pstEnable_dbg->enable_cnt++;
}

static void timer_callback_stamp_dbg(u32 clkid)
{
    struct adp_timer_callback_dbg *pstCallback_dbg = &g_adp_timer_ctrl[clkid].stCallback;

    pstCallback_dbg->callback_slice_cnt = pstCallback_dbg->callback_slice_cnt % ADP_TIMER_CALLBACK_TIMES;
    pstCallback_dbg->callback_slice[pstCallback_dbg->callback_slice_cnt] = bsp_get_slice_value();
    pstCallback_dbg->callback_slice_cnt++;
}

void timer_get_stamp_dbg(u32 clkid, u32 curtime)
{
    struct adp_timer_get_dbg *pstGet_dbg = &g_adp_timer_ctrl[clkid].stGet;

    pstGet_dbg->get_cnt = pstGet_dbg->get_cnt % ADP_TIMER_GET_TIMES;
    pstGet_dbg->get_times[pstGet_dbg->get_cnt][0] = bsp_get_slice_value();
    pstGet_dbg->get_times[pstGet_dbg->get_cnt][1] = curtime;
    pstGet_dbg->get_cnt++;
}

OSL_IRQ_FUNC(static irqreturn_t, adp_timer_handler, irq, para)
{
    u32 ret_value;
    u32 timer_id;
    int ret;

    timer_id = (u32)(uintptr_t)para;
    timer_callback_stamp_dbg(timer_id);
    ret_value = bsp_hardtimer_int_status(timer_id);
    if (0x0 != ret_value) {
        bsp_hardtimer_int_clear(timer_id);
        if (g_adp_timer_ctrl[timer_id].my_hardtimer.mode == TIMER_ONCE_COUNT) {
            (void)bsp_hardtimer_disable(timer_id);
        }
        if (NULL != g_adp_timer_ctrl[timer_id].routine) {
            ret = g_adp_timer_ctrl[timer_id].routine(g_adp_timer_ctrl[timer_id].args);
            if (ret) {
                hardtimer_print_error("timerid0x%x,ret=0x%x\n", timer_id, ret);
            }
        }
    }

    return IRQ_HANDLED;
}

/*
 * 函数  : mdrv_timer_get_accuracy_timestamp
 * 功能  : 获取BBP定时器的值。用于OAM 时戳
 * 输出  : p_high32bit_value指针参数不能为空，否则会返回失败。
 *         p_low32bit_value如果为空， 则只返回低32bit的值。
 */
int mdrv_timer_get_accuracy_timestamp(unsigned int *p_high32bit_value, unsigned int *p_low32bit_value)
{
    u64 cur_time;

    if (BSP_NULL == p_low32bit_value) {
        return BSP_ERROR;
    }

    if (bsp_slice_getcurtime(&cur_time)) {
        return BSP_ERROR;
    }

    if (BSP_NULL != p_high32bit_value) {
        *p_high32bit_value = (cur_time >> 32); /* 获取高32位 */
    }

    *p_low32bit_value = cur_time & 0xffffffff;

    return BSP_OK;
}

unsigned int mdrv_timer_get_normal_timestamp(void)
{
    return bsp_get_slice_value();
}

unsigned int mdrv_timer_get_hrt_timestamp(void)
{
    return bsp_get_slice_value_hrt();
}

unsigned int mdrv_get_normal_timestamp_freq(void)
{
    return bsp_get_slice_freq();
}

unsigned int mdrv_get_hrt_timestamp_freq(void)
{
    return bsp_get_hrtimer_freq();
}

int mdrv_timer_start(drv_timer_id_e timer_id, funcptr_1 routine, int arg, unsigned int timer_value,
    drv_timer_mode_e mode, drv_timer_unit_e unit_type)
{
    s32 ret;

    if (timer_id >= TIMER_ID_MAX) {
        return BSP_ERROR;
    }
    g_adp_timer_ctrl[timer_id].my_hardtimer.func = NULL;
    g_adp_timer_ctrl[timer_id].my_hardtimer.para = NULL;
    g_adp_timer_ctrl[timer_id].my_hardtimer.mode = (u32)mode;
    g_adp_timer_ctrl[timer_id].my_hardtimer.unit = unit_type;
    g_adp_timer_ctrl[timer_id].my_hardtimer.timeout = timer_value;
    g_adp_timer_ctrl[timer_id].my_hardtimer.timer_id = timer_id;
    g_adp_timer_ctrl[timer_id].my_hardtimer.int_enable = 1; // enable
    g_adp_timer_ctrl[timer_id].routine = routine;
    g_adp_timer_ctrl[timer_id].args = arg;
    timer_enable_stamp_dbg(timer_id, g_adp_timer_ctrl[timer_id].my_hardtimer.timeout);
    ret = bsp_hardtimer_start(&g_adp_timer_ctrl[timer_id].my_hardtimer);
    return ret;
}

int mdrv_timer_stop(drv_timer_id_e timer_id)
{
    if (timer_id >= TIMER_ID_MAX) {
        return BSP_ERROR;
    }
    timer_disable_stamp_dbg(timer_id);
    return bsp_hardtimer_disable(timer_id);
}

int mdrv_timer_get_rest_time(drv_timer_id_e timer_id, drv_timer_unit_e unit_type, unsigned int *p_rest_time)
{
    int ret;

    if (timer_id >= TIMER_ID_MAX) {
        return BSP_ERROR;
    }
    ret = bsp_get_timer_rest_time(timer_id, (drv_timer_unit_e) unit_type, p_rest_time);
    timer_get_stamp_dbg(timer_id, *p_rest_time);
    return ret;

}

void mdrv_timer_debug_register(drv_timer_id_e timer_id, funcptr_1 routine, int arg)
{
    if (timer_id >= TIMER_ID_MAX) {
        return;
    }
    g_adp_timer_ctrl[timer_id].debug_routine = routine;
    g_adp_timer_ctrl[timer_id].debug_args = arg;
}

static void get_timer_int_stat(void)
{
    unsigned int i;
    int ret;

    pr_info("%s +\n", __func__);
    for (i = 0; i < TIMER_ID_MAX; i++) {
        if (!g_adp_timer_ctrl[i].debug_routine) {
            continue;
        }
        if (!bsp_hardtimer_int_status(i)) {
            continue;
        }
        ret = g_adp_timer_ctrl[i].debug_routine(g_adp_timer_ctrl[i].debug_args);
        if (ret) {
            pr_info("timer%d,ret=0x%x\n", i, ret);
        }
    }
    pr_info("%s -\n", __func__);
}

static struct syscore_ops g_debug_timer_dpm_ops = {
    .resume = get_timer_int_stat,
};

static void timer_handler_connect(drv_timer_id_e timer_id, const char *timer_name, unsigned long flags)
{
    s32 ret;

    ret = request_irq(g_adp_timer_ctrl[timer_id].irq_num, (irq_handler_t)adp_timer_handler,
                      flags, timer_name, (void *)timer_id);
    if (ret) {
        hardtimer_print_error("timer_emun_id=%d  , timer name %s request_irq failed\n", timer_id, timer_name);
        return;
    }
}

static int connect_timer_isr(void)
{
    device_node_s *node = NULL;
    device_node_s *child_node = NULL;
    u32 request_id = 0;
    u32 wake_flag = 0;
    u32 flag = 0;
    u32 ret_value = 0;

    node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,timer_device");
    if (node == NULL) {
        return BSP_ERROR;
    }

    bsp_dt_for_each_available_child_of_node(node, child_node) {
        ret_value |= (u32)bsp_dt_property_read_u32(child_node, "request_id", &request_id);
        ret_value |= (u32)bsp_dt_property_read_u32(child_node, "need_adp_connect_isr", &flag);
        if (ret_value) {
            hardtimer_print_error("timer %d get request_id or need_adp_connect_isr failed.\n", request_id);
            return BSP_ERROR;
        }
        if (flag) {
            if (bsp_dt_property_read_u32(child_node, "wakesrc", &wake_flag)) {
                return BSP_ERROR;
            }
            if (bsp_dt_property_read_string(
                    child_node, "timer_name", (const char **)&g_adp_timer_ctrl[request_id].name)) {
                hardtimer_print_error("timer request_id %d property_read_string failed.\n", request_id);
                return BSP_ERROR;
            }
            g_adp_timer_ctrl[request_id].irq_num = bsp_dt_irq_parse_and_map(child_node, 0);
            if (wake_flag) {
                timer_handler_connect((drv_timer_id_e) request_id, g_adp_timer_ctrl[request_id].name,
                                      IRQF_NO_SUSPEND);
            } else {
                timer_handler_connect((drv_timer_id_e) request_id, g_adp_timer_ctrl[request_id].name, 0);
            }
        }
    }                       /* for_each_available_child_of_node */

    return BSP_OK;
}

int set_adp_timer_isr_and_pm(void)
{
    int ret;

    ret = memset_s((void *)g_adp_timer_ctrl, sizeof(struct adp_timer_control) * TIMER_ID_MAX, 0x0,
                   sizeof(struct adp_timer_control) * TIMER_ID_MAX);
    if (ret) {
        hardtimer_print_error("adp_timer_ctrl memset err, val=0x%x\n", ret);
        return BSP_ERROR;
    }
    if (connect_timer_isr()) {
        hardtimer_print_error("get dts failed,set_adp_timer_isr_and_pm failed\n");
        return BSP_ERROR;
    }
    register_syscore_ops(&g_debug_timer_dpm_ops);
    hardtimer_print_error(" ok\n");
    return BSP_OK;
}

void adp_timer_show(void)
{
    u32 i;

    hardtimer_print_error("name\n");
    for (i = 0; i < TIMER_ID_MAX; i++) {
        hardtimer_print_error("%s\n", g_adp_timer_ctrl[i].name);
    }
}

EXPORT_SYMBOL(mdrv_timer_start);
EXPORT_SYMBOL(mdrv_timer_stop);
EXPORT_SYMBOL(mdrv_timer_get_accuracy_timestamp);
EXPORT_SYMBOL(mdrv_timer_get_normal_timestamp);
EXPORT_SYMBOL(mdrv_timer_get_hrt_timestamp);
EXPORT_SYMBOL(mdrv_get_normal_timestamp_freq);
EXPORT_SYMBOL(mdrv_get_hrt_timestamp_freq);
/*lint -restore +e19*/
