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

#include "socp.h"
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <osl_thread.h>
#include <securec.h>
#include "soc_socp_adapter.h"
#include "bsp_dt.h"
#include "bsp_print.h"
#include "bsp_slice.h"
#include "bsp_softtimer.h"
#include "socp_debug.h"


socp_int_manager_info_s g_socp_int_info;
u32 g_socp_stress_flag;
static void socp_handler_encsrc(void);
static void socp_handler_encdst(void);
static void socp_encdst_trf_int_handler(int *is_handle);
static void socp_encdst_overflow_int_handler(int *is_handle);
static void socp_encdst_modeswt_int_handler(void);
static void socp_stress_test_proc(u32 chan_id, int *is_handle);

void bsp_socp_set_stress_flag(u32 flag)
{
    g_socp_stress_flag = flag;
}

s32 socp_int_manager_init(void)
{
    int ret;

    ret = socp_int_proc_task_init();
    if (ret) {
        return ret;
    }
    ret = socp_int_handler_init();
    if (ret) {
        return ret;
    }

    return BSP_OK;
}

/*
 * 函 数 名: socp_int_proc_task_init
 * 功能描述: 中断处理任务初始化函数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 创建成功与否的标识码
 */
s32 socp_int_proc_task_init(void)
{
    spin_lock_init(&g_socp_int_info.lock);

    /* 编码源通道任务 */
    sema_init(&g_socp_int_info.enc_src_sem_id, 0);
    if (!g_socp_int_info.enc_src_task_id) {
        if (BSP_OK != osl_task_init("socp_src", SOCP_ENCSRC_TASK_PRO, 0x1000, (OSL_TASK_FUNC)socp_encsrc_task, NULL,
            (OSL_TASK_ID *)&g_socp_int_info.enc_src_task_id)) {
            socp_error("create socp_encsrc_task failed.\n");
            return BSP_ERR_SOCP_TSK_CREATE;
        }
    }

    /* 编码输出通道任务 */
    sema_init(&g_socp_int_info.enc_dst_sem_id, 0);
    if (!g_socp_int_info.enc_dst_task_id) {
        if (BSP_OK != osl_task_init("socp_dst", SOCP_ENCDST_TASK_PRO, 0x1000, (OSL_TASK_FUNC)socp_encdst_task, NULL,
            (OSL_TASK_ID *)&g_socp_int_info.enc_dst_task_id)) {
            socp_error("create socp_encdst_task failed.\n");
            return BSP_ERR_SOCP_TSK_CREATE;
        }
    }

    return BSP_OK;
}

s32 socp_int_handler_init(void)
{
    device_node_s *dev = NULL;
    int ret, irq;

    dev = socp_hal_get_dev_info();
    if (dev == NULL) {
        socp_error("get dev null\n");
        return BSP_ERROR;
    }

    irq = bsp_dt_irq_parse_and_map(dev, 0);
    ret = request_irq(irq, (irq_handler_t)socp_enc_handler, 0, "SOCP_IRQ", BSP_NULL);
    if (ret) {
        socp_error("[init]set acore int failed, ret=0x%x\n", ret);
        return BSP_ERROR;
    }

    socp_hal_unmask_header_error_int();

    return BSP_OK;
}

irqreturn_t socp_enc_handler(int irq, void *dev_info)
{
    socp_handler_encsrc();
    socp_handler_encdst();

    return IRQ_HANDLED;
}

static void socp_headerror_proc(u32 chan_id)
{
    /* debug模式屏蔽包头错误中断 */
    if (socp_hal_get_src_debug_cfg(chan_id)) {
        socp_hal_set_header_error_int_mask(chan_id, 1);
    }

    socp_mntn_record_header_error(chan_id);
}
/*
 * 函 数 名: socp_handler_encsrc
 * 功能描述: 编码源通道处理函数，RD处理由上层完成，驱动RD中断可以不做处理
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
static void socp_handler_encsrc(void)
{
    u32 int_flag;
    u32 state_low_chan = 0;
    u32 state_high_chan = 0;
    int is_handle = BSP_FALSE;
    u32 i = 0;

    int_flag = socp_hal_get_global_int_state();
    /* 处理包头错误 */
    if (IS_ENC_SRC_HEADER_ERROR_INT(int_flag)) {
        socp_hal_get_header_error_int_state(&state_low_chan, &state_high_chan);
        socp_hal_clear_header_error_raw_int(state_low_chan, state_high_chan);

        g_socp_int_info.int_enc_src_header |= ((((u64)state_high_chan) << 32) | state_low_chan);
        is_handle = BSP_TRUE;

        for (i = 0; i < SOCP_MAX_ENCSRC_CHN; i++) {
            if (g_socp_int_info.int_enc_src_header & ((u64)1 << i)) {
                socp_headerror_proc(i);
            }
        }
    }

    /* 不再处理RD完成中断，初始化时保持屏蔽 */
    if (is_handle) {
        up(&g_socp_int_info.enc_src_sem_id);
    }

    return;
}

/*
 * 函 数 名: socp_handler_encdst
 * 功能描述: 编码目的中断处理函数
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
/*lint -save -e550*/
static void socp_handler_encdst(void)
{
    u32 int_flag;
    int is_handle = BSP_FALSE;

    /* 编码目的传输中断 */
    int_flag = socp_hal_get_global_int_state();
    if (IS_ENC_DST_TFR_INT(int_flag)) {
        socp_encdst_trf_int_handler(&is_handle);
    } else if (IS_ENC_DST_OVERFLOW_INT(int_flag)) { /* 上溢中断与阈值中断共用一个寄存器 */
        socp_encdst_overflow_int_handler(&is_handle);
    } else if (IS_ENC_DST_MODE_SWT_INT(int_flag)) {
        socp_encdst_modeswt_int_handler();
    } else {
        is_handle = BSP_FALSE;
    }

    if (is_handle) {
        up(&g_socp_int_info.enc_dst_sem_id);
    }

    return;
    /*lint -restore +e550*/
}

static void socp_stress_test_proc(u32 chan_id, int *is_handle)
{
    unsigned long lock_flag;
    u32 value;

    if (g_socp_stress_flag != 0) {
        spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);
        value = socp_hal_get_dst_write_ptr(chan_id);
        socp_hal_set_dst_read_ptr(chan_id, value);
        socp_hal_clear_single_chan_tfr_raw_int(chan_id);
        socp_hal_set_single_chan_tfr_mask(chan_id, 0);
        /* overflow int */
        socp_hal_clear_single_chan_thr_ovf_raw_int(chan_id);
        socp_hal_set_single_chan_thr_ovf_mask(chan_id, 0);
        spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);

        *is_handle = BSP_FALSE;
    }
}

static inline void socp_check_overflow_int_raw_state(void)
{
    if (IS_IND_DST_THRD_OVF_RAW_INT(socp_hal_get_overflow_raw_int())) {
        g_socp_int_info.thrh_ovf_flag = 1;
    }
}

static void socp_encdst_trf_int_handler(int *is_handle)
{
    u32 tfr_state, i, mask, mask2;
    unsigned long lock_flag;

    spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);
    tfr_state = socp_hal_get_tfr_modeswt_int_state();
    mask = socp_hal_get_tfr_modeswt_int_mask();
    socp_hal_set_tfr_int_mask((tfr_state | mask));
    socp_hal_clear_tfr_raw_int(tfr_state);

    socp_check_overflow_int_raw_state();

    mask2 = socp_hal_get_overflow_int_mask();
    socp_hal_set_overflow_int_mask((tfr_state << 16) | mask2);

    g_socp_int_info.int_enc_dst_tfr |= tfr_state;
    spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);
    *is_handle = BSP_TRUE;

    for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
        if (tfr_state & ((u32)1 << i)) {
            socp_stress_test_proc(i, is_handle); /* 用于socp压力测试 */
            if (i == SOCP_REAL_CHAN_ID(SOCP_CODER_DST_OM_IND)) {
                socp_mntn_record_ind_int_start_slice();
            }
        }
    }
}

static void socp_encdst_overflow_int_handler(int *is_handle)
{
    u32 int_state;
    u32 i, mask;
    unsigned long lock_flag;

    int_state = socp_hal_get_overflow_int_state();
    /* 编码目的buffer阈值中断处理 */
    if (IS_ENC_DST_THRESHOLD_OVER_FLOW(int_state)) {
        spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);
        mask = socp_hal_get_overflow_int_mask();
        socp_hal_set_overflow_int_mask((int_state | mask));
        socp_hal_clear_overflow_raw_int(int_state);
        g_socp_int_info.int_enc_dst_thrh_ovf |= int_state;
        spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);

        *is_handle = BSP_TRUE;

        for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
            if ((int_state & ((u32)1 << (i + SOCP_ENC_DST_BUFF_THRESHOLD_OVF_BEGIN))) || (int_state & ((u32)1 << i))) {
                socp_mntn_record_dst_overflow_int(i);
            }
        }
    }

    return;
}

static void socp_encdst_modeswt_int_handler(void)
{
    u32 int_state;
    u32 i, mask, mode_state;
    unsigned long lock_flag;

    spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);

    int_state = socp_hal_get_tfr_modeswt_int_state();
    mask = socp_hal_get_tfr_modeswt_int_mask();
    socp_hal_set_modeswt_int_mask(((int_state | mask) >> 16) & 0x7f);
    socp_hal_clear_modeswt_raw_int((int_state >> 16) & 0x7f); /* 清原始中断状态 */

    mask = 0;
    for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
        mode_state = socp_hal_get_dst_chan_mode(i);
        if (mode_state) {
            mask |= (1 << i);
        }
    }

    /* 屏蔽处于循环模式通道的传输中断和阈值溢出中断 */
    socp_hal_set_tfr_int_mask(mask);
    socp_hal_set_overflow_int_mask(mask);
    socp_hal_set_overflow_int_mask(mask << 16);

    spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);
}


/*
 * 函 数 名: socp_encsrc_task
 * 功能描述: 模块任务函数:编码源中断，双核
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
int socp_encsrc_task(void *data)
{
    u32 i;
    u64 int_head_state = 0;
    unsigned long lock_flag;

    do {
        /* 超时或者被中断，非正常返回 */
        if (0 != down_interruptible(&g_socp_int_info.enc_src_sem_id)) {
            continue;
        }

        spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);
        int_head_state = g_socp_int_info.int_enc_src_header;
        g_socp_int_info.int_enc_src_header = 0;

        spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);

        /* 处理编码包头'HISI'检验错误 */
        for (i = 0; i < SOCP_MAX_ENCSRC_CHN; i++) {
            if (int_head_state & ((u64)1 << i)) {
                socp_info("enc src header warn, chan_id=%d\n", i);
            }
        }
    } while (1);

    return 0;
}

/*
 * 函 数 名: socp_rdreq_init
 * 功能描述: 模块任务函数:编码源中断，双核
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
static void socp_encdst_rdreq_clr(u32 dst_chan)
{
    g_socp_int_info.dst_int_info[dst_chan].dst_send_req = 0;
}

static void socp_encdst_rdreq_set(u32 dst_chan)
{
    g_socp_int_info.dst_int_info[dst_chan].dst_send_req = 1;
}

static u32 socp_encdst_rdreq_get(u32 dst_chan)
{
    return g_socp_int_info.dst_int_info[dst_chan].dst_send_req;
}

static u32 socp_encdst_rdreq_getall(void)
{
    u32 rst = 0;
    u32 i = 0;

    for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
        rst |= g_socp_int_info.dst_int_info[i].dst_send_req;
    }
    return rst;
}

static void socp_encdst_task_trf(u32 chan_id)
{
    socp_mntn_record_tfr_task_cb_cnt(chan_id);

    if (chan_id == SOCP_REAL_CHAN_ID(SOCP_CODER_DST_OM_IND)) {
        socp_mntn_record_ind_read_cb_start_slice();
    }

    socp_mntn_record_dst_tfr_int_cnt(chan_id);
    socp_enc_dst_read_cb(chan_id);

    if (chan_id == SOCP_REAL_CHAN_ID(SOCP_CODER_DST_OM_IND)) {
        socp_mntn_record_ind_read_cb_end_slice();
    }
}

static void socp_encdst_task_trf_proc(u32 int_trf_state)
{
    u32 i;

    for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
        /* 检测通道是否配置 */
        if ((int_trf_state & ((u32)1 << i)) || socp_encdst_rdreq_get(i)) {
            socp_encdst_rdreq_clr(i);
            socp_encdst_task_trf(i);
        }
    }
}

static void socp_encdst_task_thresholdovf_proc(u32 int_thrh_ovf_state)
{
    u32 i;

    for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
        if (int_thrh_ovf_state & ((u32)1 << (i + SOCP_ENC_DST_BUFF_THRESHOLD_OVF_BEGIN))) {
            socp_mntn_record_throvf_task_cb_cnt(i);
            socp_enc_dst_read_cb(i);
        }
    }
}

/*
 * 函 数 名: socp_encdst_task
 * 功能描述: 模块任务函数:编码目的，App核
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
int socp_encdst_task(void *data)
{
    u32 int_trf_state = 0;
    u32 int_thrh_ovf_state = 0;
    unsigned long lock_flag;
    u32 i;

    for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
        /* 检测通道是否配置 */
        if (socp_encdst_rdreq_get(i)) {
            socp_encdst_rdreq_clr(i);
        }
    }

    do {
        /* 超时或者被中断，非正常返回 */
        if (0 != down_interruptible(&g_socp_int_info.enc_dst_sem_id)) {
            continue;
        }

        spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);
        int_trf_state = g_socp_int_info.int_enc_dst_tfr;
        g_socp_int_info.int_enc_dst_tfr = 0;
        int_thrh_ovf_state = g_socp_int_info.int_enc_dst_thrh_ovf;
        g_socp_int_info.int_enc_dst_thrh_ovf = 0;
        spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);

        /* 处理编码传输完成中断 */
        if (int_trf_state || socp_encdst_rdreq_getall()) {
            socp_encdst_task_trf_proc(int_trf_state);
        }

        /* 处理编码目的 buffer 阈值溢出中断 */
        if (int_thrh_ovf_state) {
            socp_encdst_task_thresholdovf_proc(int_thrh_ovf_state);
        }

        if (g_socp_int_info.thrh_ovf_flag) {
            socp_mntn_record_dst_overflow_int(SOCP_REAL_CHAN_ID(SOCP_CODER_DST_OM_IND));
            socp_report_overflow(SOCP_REAL_CHAN_ID(SOCP_CODER_DST_OM_IND));
            g_socp_int_info.thrh_ovf_flag = 0;
        }
    } while (1);

    return 0;
}

/*
 * 函 数 名: bsp_socp_encdst_dsm_init
 * 功能描述: socp编码目的端中断状态初始化
 * 输入参数: enc_dst_chan_id: 编码目的端通道号
 * b_enable: 初始化的中断状态
 * 输出参数: 无
 * 返 回 值: 无
 */
void bsp_socp_encdst_dsm_init(u32 enc_dst_chan_id, u32 b_enable)
{
    u32 real_chan_id = SOCP_REAL_CHAN_ID(enc_dst_chan_id);

    if (b_enable == SOCP_DEST_DSM_DISABLE) {
        socp_hal_clear_single_chan_tfr_raw_int(real_chan_id);
        socp_hal_set_single_chan_tfr_mask(real_chan_id, 1);
        socp_hal_clear_single_chan_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_ovf_mask(real_chan_id, 1);
        socp_hal_clear_single_chan_thr_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_thr_ovf_mask(real_chan_id, 1);
        g_socp_int_info.dst_int_info[real_chan_id].dst_int_state = SOCP_DEST_DSM_DISABLE;
    } else if (b_enable == SOCP_DEST_DSM_ENABLE) {
        socp_hal_clear_single_chan_tfr_raw_int(real_chan_id);
        socp_hal_set_single_chan_tfr_mask(real_chan_id, 0);
        socp_hal_clear_single_chan_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_ovf_mask(real_chan_id, 0);
        socp_hal_clear_single_chan_thr_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_thr_ovf_mask(real_chan_id, 0);
        g_socp_int_info.dst_int_info[real_chan_id].dst_int_state = SOCP_DEST_DSM_ENABLE;
    }
}

void socp_set_dst_int_open(u32 chan_id)
{
    socp_hal_clear_single_chan_tfr_raw_int(chan_id);
    socp_hal_set_single_chan_tfr_mask(chan_id, 0);
    socp_hal_clear_single_chan_ovf_raw_int(chan_id);
    socp_hal_set_single_chan_ovf_mask(chan_id, 0);
    socp_hal_clear_single_chan_thr_ovf_raw_int(chan_id);
    socp_hal_set_single_chan_thr_ovf_mask(chan_id, 0);

    return;
}
/*
 * 函 数 名: bsp_socp_data_send_continue
 * 功能描述: socp编码目的端数据上报使能，在readdone中调用
 * 根据diag连接状态判断是否上报
 * 注    意: 该函数调用时，需要调用者保证同步
 * 输入参数: enc_dst_chan_id: 编码目的端通道号
 * b_enable: 中断使能
 * 输出参数: 无
 * 返 回 值: 无
 */
void bsp_socp_data_send_continue(u32 enc_dst_chan_id)
{
    socp_buffer_rw_s buffer;
    u32 chan_id = SOCP_REAL_CHAN_ID(enc_dst_chan_id);
    socp_enc_dst_info_s *enc_dst = socp_get_dst_chan_info(chan_id);

    if (g_socp_int_info.dst_int_info[chan_id].dst_int_state == SOCP_DEST_DSM_ENABLE) {
        bsp_socp_get_read_buff(chan_id, &buffer);
        if (buffer.size + buffer.rb_size >= enc_dst->thrhold_low) {
            socp_encdst_rdreq_set(chan_id);
            up(&g_socp_int_info.enc_dst_sem_id);
        } else {
            socp_set_dst_int_open(chan_id);
        }
    }
}
/*
 * 函 数 名: bsp_socp_data_send_manager
 * 功能描述: socp编码目的端上报数据
 * 输入参数: enc_dst_chan_id: 编码目的端通道号
 * b_enable: 中断使能
 * 输出参数: 无
 * 返 回 值: 无
 */
void bsp_socp_data_send_manager(u32 enc_dst_chan_id, u32 b_enable)
{
    unsigned long lock_flag;
    u32 real_chan_id = SOCP_REAL_CHAN_ID(enc_dst_chan_id);
    if ((b_enable == SOCP_DEST_DSM_DISABLE) &&
        (g_socp_int_info.dst_int_info[real_chan_id].dst_int_state == SOCP_DEST_DSM_ENABLE)) {
        spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);
        socp_hal_clear_single_chan_tfr_raw_int(real_chan_id);
        socp_hal_set_single_chan_tfr_mask(real_chan_id, 1);
        socp_hal_clear_single_chan_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_ovf_mask(real_chan_id, 1);
        socp_hal_clear_single_chan_thr_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_thr_ovf_mask(real_chan_id, 1);

        g_socp_int_info.dst_int_info[real_chan_id].dst_int_state = SOCP_DEST_DSM_DISABLE;
        spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);
    } else if ((b_enable == SOCP_DEST_DSM_ENABLE) &&
        (g_socp_int_info.dst_int_info[real_chan_id].dst_int_state == SOCP_DEST_DSM_DISABLE)) {
        spin_lock_irqsave(&g_socp_int_info.lock, lock_flag);
        socp_hal_clear_single_chan_tfr_raw_int(real_chan_id);
        socp_hal_set_single_chan_tfr_mask(real_chan_id, 0);
        socp_hal_clear_single_chan_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_ovf_mask(real_chan_id, 0);
        socp_hal_clear_single_chan_thr_ovf_raw_int(real_chan_id);
        socp_hal_set_single_chan_thr_ovf_mask(real_chan_id, 0);

        g_socp_int_info.dst_int_info[real_chan_id].dst_int_state = SOCP_DEST_DSM_ENABLE;
        spin_unlock_irqrestore(&g_socp_int_info.lock, lock_flag);
    } else {
        ;
    }
}

void socp_rd_int_mask_proc(u32 chan_id, socp_rd_mask_e rd_mask_state)
{
    if (rd_mask_state == SOCP_RD_MASK_SET) {
        socp_hal_set_single_src_rd_timeout_int_mask(chan_id, 1);
        socp_hal_set_single_src_rd_int_mask(chan_id, 1);
    } else {
        socp_hal_set_single_src_rd_timeout_int_mask(chan_id, 0);
        socp_hal_set_single_src_rd_int_mask(chan_id, 0);
    }

    return;
}

void socp_clear_enc_src_int_state(u32 chan_id, u32 mode)
{
    /* 清除原始rd中断 */
    if (mode == SOCP_ENCSRC_CHNMODE_LIST) {
        socp_hal_clear_src_rd_raw_int(chan_id);
        socp_hal_clear_src_rd_timout_raw_int(chan_id);
        socp_rd_int_mask_proc(chan_id, SOCP_RD_MASK_CLEAR);
    }
}

socp_int_manager_info_s *socp_get_int_info(void)
{
    return &g_socp_int_info;
}

