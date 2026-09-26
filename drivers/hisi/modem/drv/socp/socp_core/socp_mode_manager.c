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
#include "socp_debug.h"
#include "socp_hal.h"


#undef THIS_MODU
#define THIS_MODU mod_socp

socp_mode_swt_info_s g_socp_mode_swt_info;

void socp_mode_swt_init(void)
{
    spin_lock_init(&g_socp_mode_swt_info.lock);
}

/*
 * 函 数 名: socp_set_enc_dst_mode
 * 功能描述: SOCP循环模式设置
 * 输入参数: 通道号、模式
 * 输出参数: 无
 * 返 回 值: 无
 */
s32 socp_set_enc_dst_mode(u32 dst_chan, u32 mode)
{
    u32 mod_state;
    unsigned long lock_flag;
    u32 real_chan_id = SOCP_REAL_CHAN_ID(dst_chan);
    u32 waittime = 10000;
    u32 i;

    /* 关闭自动时钟门控，否则上报模式配置不生效 */
    socp_hal_set_clk_ctrl(0);

    mod_state = socp_hal_get_dst_chan_mode(real_chan_id);
    if ((mode == SOCP_IND_MODE_DIRECT || mode == SOCP_IND_MODE_DELAY) && mod_state) {
        spin_lock_irqsave(&g_socp_mode_swt_info.lock, lock_flag);
        socp_hal_set_dst_chan_mode(real_chan_id, 0);
        spin_unlock_irqrestore(&g_socp_mode_swt_info.lock, lock_flag);

        for (i = 0; i < waittime; i++) {
            mod_state = socp_hal_get_dst_chan_mode(real_chan_id);
            if (mod_state == 0) {
                break;
            }
        }

        if (waittime == i) {
            socp_mntn_record_mode_swt_fail_cnt(real_chan_id);
            socp_error("set encdst cycle off timeout!\n");
            socp_hal_set_clk_ctrl(1);
            return BSP_ERROR;
        }

        bsp_socp_data_send_manager(dst_chan, SOCP_DEST_DSM_ENABLE);
    } else if ((mode == SOCP_IND_MODE_CYCLE) && (!mod_state)) {
        bsp_socp_data_send_manager(dst_chan, SOCP_DEST_DSM_DISABLE);

        spin_lock_irqsave(&g_socp_mode_swt_info.lock, lock_flag);
        socp_hal_set_dst_chan_mode(real_chan_id, 1);
        spin_unlock_irqrestore(&g_socp_mode_swt_info.lock, lock_flag);

        for (i = 0; i < waittime; i++) {
            mod_state = socp_hal_get_dst_chan_mode(real_chan_id);
            if (mod_state == 1) {
                break;
            }
        }

        if (waittime == i) {
            socp_mntn_record_mode_swt_fail_cnt(real_chan_id);
            socp_error("set encdst cycle on timeout!\n");
            SOCP_REG_SETBITS(SOCP_REG_CLKCTRL, 0, 1, 1);
            return BSP_ERROR;
        }
    }

    /* 配置完成，打开socp clk ctrl，启动自动时钟门控 */
    socp_hal_set_clk_ctrl(1);
    return BSP_OK;
}

void socp_set_mode_direct(void)
{
    u32 ret;

    if (g_socp_mode_swt_info.socp_cur_mode == SOCP_IND_MODE_DIRECT) {
        socp_crit("the ind mode direct is already config!\n");
        return;
    }

    bsp_socp_set_timeout(SOCP_TIMEOUT_TFR_LONG, SOCP_TIMEOUT_TFR_LONG_MIN);
    g_socp_mode_swt_info.cur_timeout = SOCP_TIMEOUT_TFR_LONG_MIN;

    bsp_socp_set_enc_dst_threshold(SOCP_IND_MODE_DIRECT, SOCP_CODER_DST_OM_IND);
    ret = socp_set_enc_dst_mode(SOCP_CODER_DST_OM_IND, SOCP_IND_MODE_DIRECT);
    if (ret) {
        socp_error("direct mode config failed!\n");
    } else {
        g_socp_mode_swt_info.socp_cur_mode = SOCP_IND_MODE_DIRECT;
        socp_crit("direct mode config sucess!\n");
    }
}

void socp_set_mode_delay(void)
{
    u32 ret;

    if (g_socp_mode_swt_info.socp_cur_mode == SOCP_IND_MODE_DELAY) {
        socp_crit("socp:the ind mode delay is already config!\n");
        return;
    }

    /* if logbuffer is not configed, can't enable delay mode */
    if (socp_get_mem_log_on_flag() == SOCP_DST_CHAN_DELAY) {
        bsp_socp_set_timeout(SOCP_TIMEOUT_TFR_LONG, SOCP_TIMEOUT_TFR_LONG_MAX);
        g_socp_mode_swt_info.cur_timeout = SOCP_TIMEOUT_TFR_LONG_MAX;

        bsp_socp_set_enc_dst_threshold(SOCP_IND_MODE_DELAY, SOCP_CODER_DST_OM_IND);
        ret = socp_set_enc_dst_mode(SOCP_CODER_DST_OM_IND, SOCP_IND_MODE_DELAY);
        if (ret) {
            socp_error("delay mode config failed!\n");
        } else {
            g_socp_mode_swt_info.socp_cur_mode = SOCP_IND_MODE_DELAY;
            socp_crit("delay mode config sucess!\n");
            socp_crit("delay mode, socp read/write pointer:%x/%x!\n", socp_hal_get_dst_read_ptr(1),
                socp_hal_get_dst_write_ptr(1));
        }
    } else {
        socp_crit("delay mode can't config:mem can't be setted!\n");
    }
}

void socp_set_mode_cycle(void)
{
    u32 ret;
    u32 log_on_flag;

    if (g_socp_mode_swt_info.socp_cur_mode == SOCP_IND_MODE_CYCLE) {
        socp_crit("the ind mode cycle is already config!\n");
        return;
    }

    log_on_flag = socp_get_mem_log_on_flag();
    /* 如果dst方式malloc 10M内存,也支持配置循环模式，支持商用10Mlog抓取 */
    if ((log_on_flag == SOCP_DST_CHAN_DELAY) || (log_on_flag == SOCP_DST_CHAN_DTS)) {
        ret = socp_set_enc_dst_mode(SOCP_CODER_DST_OM_IND, SOCP_IND_MODE_CYCLE);
        if (ret) {
            socp_error("cycle mode config failed, ret=0x%x\n", ret);
        } else {
            g_socp_mode_swt_info.socp_cur_mode = SOCP_IND_MODE_CYCLE;
            socp_crit("the ind cycle mode config sucess!\n");
        }
    } else {
        socp_crit("ind delay can't config:mem can't be setted!\n");
    }
}

/*
 * 函 数 名: bsp_socp_get_ind_chan_mode
 * 功能描述: 上报模式接口
 * 输入参数: 模式参数
 * 输出参数: 无
 * 返 回 值: socp mode
 */
s32 bsp_socp_get_ind_chan_mode(void)
{
    return g_socp_mode_swt_info.socp_cur_mode;
}

/*
 * 函 数 名: bsp_socp_set_ind_mode
 * 功能描述: 上报模式接口
 * 输入参数: 模式参数
 * 输出参数: 无
 * 返 回 值: BSP_S32 BSP_OK:成功 BSP_ERROR:失败
 */
s32 bsp_socp_set_ind_mode(socp_ind_mode_e mode)
{
    switch (mode) {
        case SOCP_IND_MODE_DIRECT: {
            socp_set_mode_direct();
            break;
        }
        case SOCP_IND_MODE_DELAY: {
            socp_set_mode_delay();
            break;
        }
        case SOCP_IND_MODE_CYCLE: {
            socp_set_mode_cycle();
            break;
        }
        default: {
            socp_error("set invalid mode=0x%x\n", mode);
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_get_log_cfg_state
 * 功能描述: 查询获取当前上报模式
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
s32 bsp_socp_get_log_cfg_state(u32 *cfg_ind_mode)
{
    *cfg_ind_mode = bsp_socp_get_ind_chan_mode();
    socp_crit("ind chan mode=0x%x\n", *cfg_ind_mode);
    return BSP_OK;
}

/*
 * 函 数 名: bsp_socp_set_log_cfg_mode
 * 功能描述: 设置当前上报模式
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 */
s32 bsp_socp_set_log_cfg_mode(socp_ind_mode_e mode)
{
    return bsp_socp_set_ind_mode(mode);
}

u32 socp_get_ind_dst_cur_timouet(void)
{
    return g_socp_mode_swt_info.cur_timeout;
}

