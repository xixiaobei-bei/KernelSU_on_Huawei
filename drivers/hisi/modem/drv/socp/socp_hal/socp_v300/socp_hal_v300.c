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

#include "socp_hal_common.h"
#include <bsp_dt.h>
#include "bsp_print.h"


socp_hal_info_s g_socp_hal_info;

s32 socp_hal_init(void)
{
    s32 ret;

    g_socp_hal_info.dev = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,socp_balong_app");
    if (g_socp_hal_info.dev == NULL) {
        socp_error("socp dev find failed\n");
        return BSP_ERROR;
    }

    g_socp_hal_info.base_addr = bsp_dt_iomap(g_socp_hal_info.dev, 0);
    if (g_socp_hal_info.base_addr == NULL) {
        socp_error("socp base addr init failed\n");
        return BSP_ERROR;
    }

    ret = bsp_dt_property_read_u32_array(g_socp_hal_info.dev, "chan_group", g_socp_hal_info.socp_rate_info.chan_group,
        (size_t)SOCP_MAX_ENCSRC_CHN);
    if (ret) {
        socp_crit("no rate_ctrl group, ret=0x%x\n", ret);
    }
    ret = bsp_dt_property_read_u32_array(g_socp_hal_info.dev, "group_limits",
        g_socp_hal_info.socp_rate_info.group_limit, (size_t)SOCP_MAX_ENCSRC_CHN);
    if (ret) {
        socp_crit("no rate_ctrl group limits, ret=0x%x\n", ret);
    }

    g_socp_hal_info.socp_version = SOCP_REG_READ(SOCP_REG_SOCP_VERSION);

    socp_hal_global_reset();
    while (socp_hal_get_global_reset_state() != 0) {
        ;
    }

    return BSP_OK;
}

void socp_hal_set_rate_ctrl_threshold(u32 rate_limits)
{
    unsigned int temp;
    u32 index;
    u32 *chan_group = (u32 *)&g_socp_hal_info.socp_rate_info.chan_group;
    u32 *group_limit = (u32 *)&g_socp_hal_info.socp_rate_info.group_limit;

    socp_crit("set rate limits:%d\n", rate_limits);

    /* 流控阈值配置 */
    for (index = 0; index < SOCP_MAX_ENCSRC_CHN; index++) {
        temp = (u64)rate_limits * (u64)1024 * (u64)1024 * (u64)group_limit[chan_group[index]] / (u64)100;
        temp &= 0xfffffff8;
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RATE_THR(index), temp);

        socp_crit("set socp chan rate, chan:%d group:%d, group limit:%x\n", index, chan_group[index],
            group_limit[chan_group[index]]);
    }

    return;
}

/* lint -save -e679 -e647 */
void socp_hal_unmask_header_error_int(void)
{
    /* 打开socp源通道包头错中断 */
    SOCP_REG_WRITE(SOCP_REG_APP_MASK1_L, 0);
    SOCP_REG_WRITE(SOCP_REG_APP_MASK1_H, 0);
}

void socp_hal_get_header_error_int_state(u32 *low_chan, u32 *high_chan)
{
    *low_chan = SOCP_REG_READ(SOCP_REG_APP_INTSTAT1_L);
    *high_chan = SOCP_REG_READ(SOCP_REG_APP_INTSTAT1_H);
}

void socp_hal_clear_header_error_raw_int(u32 low_chan, u32 high_chan)
{
    SOCP_REG_WRITE(SOCP_REG_ENC_RAWINT1_L, low_chan);
    SOCP_REG_WRITE(SOCP_REG_ENC_RAWINT1_H, high_chan);
}

void socp_hal_set_header_error_int_mask(u32 chan_id, u32 value)
{
    u32 reg = chan_id < 0x20 ? SOCP_REG_APP_MASK1_L : SOCP_REG_APP_MASK1_H;
    SOCP_REG_SETBITS(reg, (chan_id % 0x20), 1, value);
}

void socp_hal_set_rate_ctrl_cfg(u32 value1, u32 value2)
{
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_RATE_EN_L, value1);
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_RATE_EN_H, value2);
}

void socp_hal_set_rate_ctrl_peroid(u32 value)
{
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_RATE_PERIOD, value);
}

void socp_hal_set_dst_chan_trans_id_en(u32 chan_id, u32 value)
{
    SOCP_REG_SETBITS(SOCP_REG_ENCDEST_BUFMODE_CFG(chan_id), 5, 1, value);
}

void socp_hal_set_rptr_img_addr(u32 chan_id, u32 addr_low, u32 addr_high)
{
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFM_RPTRIMG_L(chan_id), addr_low);
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFM_RPTRIMG_H(chan_id), addr_high);
}

void socp_hal_set_src_rd_timeout(u32 chan_id, u32 value)
{
    if (g_socp_hal_info.socp_version >= SOCP_VERSION_V303) {
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(chan_id), 16, 16, value); /*lint !e647*/
    }
}

void socp_hal_set_rate_ctrl_group(u32 chan_id, u32 value)
{
    if (g_socp_hal_info.socp_version >= SOCP_VERSION_V303) {
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RATE_CTRL(chan_id), 24, 6, value);
    }
}

void socp_hal_clear_src_chan_cfg(u32 chan_id)
{
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFCFG(chan_id), 0x108); /* 该寄存器默认值为0x108 */
}

void socp_hal_clear_rate_ctrl_cnt(u32 chan_id)
{
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RATE_CTRL(chan_id), 31, 1, 1); /* 清除限流计数 */
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RATE_CTRL(chan_id), 31, 1, 0); /* 重启限流计数 */
}

void socp_hal_clear_src_rd_raw_int(u32 chan_id)
{
    u32 reg = chan_id < 0x20 ? SOCP_REG_ENC_RAWINT3_L : SOCP_REG_ENC_RAWINT3_H;
    SOCP_REG_SETBITS(reg, chan_id % 0x20, 1, 1);
}

u32 socp_hal_get_src_state(void)
{
    return (SOCP_REG_READ(SOCP_REG_ENCSTAT_L) || SOCP_REG_READ(SOCP_REG_ENCSTAT_H));
}

u32 socp_hal_get_single_src_state(u32 chan_id)
{
    u32 reg = chan_id < 0x20 ? SOCP_REG_ENCSTAT_L : SOCP_REG_ENCSTAT_H;
    return SOCP_REG_GETBITS(reg, chan_id % 0x20, 1);
}

s32 socp_hal_get_rate_ctrl_cnt(u32 chan_id)
{
    if (g_socp_hal_info.socp_version >= SOCP_VERSION_V303) {
        return SOCP_REG_GETBITS(SOCP_REG_ENCSRC_RATE_CTRL(chan_id), 0, 24);
    } else {
        return 0;
    }
}

void socp_hal_set_single_src_rd_timeout_int_mask(u32 chan_id, u32 value)
{
    u32 reg = chan_id < 0x20 ? SOCP_REG_RD_TIMEOUT_MODEM_MASK_L : SOCP_REG_RD_TIMEOUT_MODEM_MASK_H;
    if (g_socp_hal_info.socp_version >= SOCP_VERSION_V303) {
        SOCP_REG_SETBITS(reg, chan_id % 0x20, 1, value);
    }
}

void socp_hal_clear_src_rd_timout_raw_int(u32 chan_id)
{
    u32 reg = chan_id < 0x20 ? SOCP_REG_RD_TIMEOUT_RAWINT_L : SOCP_REG_RD_TIMEOUT_RAWINT_H;
    SOCP_REG_SETBITS(reg, chan_id % 0x20, 1, 1);
}

void socp_hal_set_single_src_rd_int_mask(u32 chan_id, u32 value)
{
    u32 reg = chan_id < 0x20 ? SOCP_REG_APP_MASK3_L : SOCP_REG_APP_MASK3_H;
    SOCP_REG_SETBITS(reg, chan_id % 0x20, 1, value);
}


void socp_hal_src_chan_init(u32 chan_id, socp_src_chan_cfg_s *src_attr)
{
    u32 buf_len, rd_buf_len;

    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 1, 2, (u32)src_attr->mode);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 3, 1, (u32)src_attr->trans_id_en);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 4, 2, SOCP_REAL_CHAN_ID(src_attr->dest_chan_id));
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 8, 2, (u32)src_attr->priority);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 10, 1, (u32)src_attr->bypass_en);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 11, 1, (u32)src_attr->data_type_en);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 12, 1, (u32)src_attr->rptr_img_en);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 16, 8, (u32)src_attr->data_type);
    SOCP_REG_SETBITS(SOCP_REG_ENCSRC_BUFCFG(chan_id), 31, 1, (u32)src_attr->debug_en);
    buf_len = (uintptr_t)src_attr->coder_src_setbuf.input_end - (uintptr_t)src_attr->coder_src_setbuf.input_start + 1;
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFDEPTH(chan_id), buf_len);

    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFADDR_L(chan_id), (uintptr_t)src_attr->coder_src_setbuf.input_start);
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFADDR_H(chan_id), 0);

    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFWPTR(chan_id), 0);
    SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFRPTR(chan_id), 0);

    if (src_attr->rptr_img_en) { /* 配置读指针镜像 */
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFM_RPTRIMG_L(chan_id), src_attr->read_ptr_img_phy_addr);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_BUFM_RPTRIMG_H(chan_id), 0);
    }

    if (src_attr->mode == SOCP_ENCSRC_CHNMODE_LIST) {
        rd_buf_len = (uintptr_t)src_attr->coder_src_setbuf.rd_input_end -
            (uintptr_t)src_attr->coder_src_setbuf.rd_input_start + 1;
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(chan_id), 0, 16, rd_buf_len);
        SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(chan_id), 16, 16, 0);

        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQADDR_L(chan_id), (uintptr_t)src_attr->coder_src_setbuf.rd_input_start);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQADDR_H(chan_id), 0);

        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQWPTR(chan_id), 0);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQRPTR(chan_id), 0);

        if (g_socp_hal_info.socp_version >= SOCP_VERSION_V303) {
            SOCP_REG_SETBITS(SOCP_REG_ENCSRC_RDQCFG(chan_id), 16, 16,
                src_attr->coder_src_setbuf.rd_timeout); /*lint !e647 */
        }

        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQ_WPTRIMG_L(chan_id), src_attr->rd_write_ptr_img_addr_low);
        SOCP_REG_WRITE(SOCP_REG_ENCSRC_RDQ_WPTRIMG_H(chan_id), src_attr->rd_write_ptr_img_addr_high);
    }
}


/* lint -restore +e679 +e647 */
