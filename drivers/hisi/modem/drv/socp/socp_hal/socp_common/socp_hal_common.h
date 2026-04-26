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

#ifndef _SOCP_HAL_COMMON_H
#define _SOCP_HAL_COMMON_H

#include "osl_types.h"
#include "osl_bio.h"
#include "bsp_dt.h"
#include "drv_comm.h"
#include "mdrv_socp.h"
#include "socp_hal.h"


#ifdef __cplusplus
extern "C" {
#endif

#undef THIS_MODU
#define THIS_MODU mod_socp

#define socp_crit(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define socp_error(fmt, ...) bsp_err("<%s %d>" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define socp_info(fmt, ...) bsp_info("<%s %d>" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define socp_warning(fmt, ...) bsp_wrn("<%s %d>" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

extern socp_hal_info_s g_socp_hal_info;
#define SOCP_REG_WRITE(reg, data) (writel((data), (g_socp_hal_info.base_addr + (reg))))
#define SOCP_REG_READ(reg) (readl(g_socp_hal_info.base_addr + (reg)))
#define SOCP_REG_WRITE_RELAXED(reg, data) (writel_relaxed(data, (g_socp_hal_info.base_addr + (reg))))
#define SOCP_REG_READ_RELAXED(reg, result) ((result) = readl_relaxed(g_socp_hal_info.base_addr + (reg)))
#define SOCP_REG_SETBITS(reg, pos, bits, val) BSP_REG_SETBITS(g_socp_hal_info.base_addr, reg, pos, bits, val)
#define SOCP_REG_GETBITS(reg, pos, bits) BSP_REG_GETBITS(g_socp_hal_info.base_addr, reg, pos, bits)

s32 socp_hal_init(void);
device_node_s *socp_hal_get_dev_info(void);
u8 *socp_hal_get_socp_base_addr(void);
void socp_hal_set_rate_ctrl_threshold(u32 rate_limits);
void socp_hal_set_timeout_threshold(u32 mode, u32 timeout);
void socp_hal_set_clk_ctrl(u32 value);
void socp_hal_global_reset(void);
u32 socp_hal_get_global_reset_state(void);
u32 socp_hal_get_timeout_mode(void);
void socp_hal_set_timeout_mode(u32 value);
void socp_hal_unmask_header_error_int(void);
u32 socp_hal_get_src_debug_cfg(u32 chan_id);
void socp_hal_set_header_error_int_mask(u32 chan_id, u32 value);
void socp_hal_clear_header_error_raw_int(u32 low_chan, u32 high_chan);
void socp_hal_get_header_error_int_state(u32 *low_chan, u32 *high_chan);
void socp_hal_get_header_error_int_state(u32 *low_chan, u32 *high_chan);
u32 socp_hal_get_tfr_modeswt_int_state(void);
u32 socp_hal_get_tfr_modeswt_int_mask(void);
void socp_hal_set_tfr_int_mask(u32 value);
u32 socp_hal_get_overflow_int_mask(void);
void socp_hal_set_overflow_int_mask(u32 value);
void socp_hal_clear_tfr_raw_int(u32 value);
u32 socp_hal_get_overflow_int_state(void);
void socp_hal_clear_overflow_raw_int(u32 value);
u32 socp_hal_get_overflow_raw_int(void);
void socp_hal_set_modeswt_int_mask(u32 value);
void socp_hal_clear_modeswt_raw_int(u32 value);
u32 socp_hal_get_dst_chan_mode(u32 chan_id);
void socp_hal_clear_single_chan_tfr_raw_int(u32 chan_id);
void socp_hal_set_single_chan_tfr_mask(u32 chan_id, u32 value);
void socp_hal_clear_single_chan_ovf_raw_int(u32 chan_id);
void socp_hal_set_single_chan_ovf_mask(u32 chan_id, u32 value);
void socp_hal_clear_single_chan_thr_ovf_raw_int(u32 chan_id);
void socp_hal_set_single_chan_thr_ovf_mask(u32 chan_id, u32 value);
void socp_hal_set_src_buffer_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void socp_hal_set_src_rd_buffer_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void socp_hal_set_src_write_ptr(u32 chan_id, u32 value);
void socp_hal_set_src_read_ptr(u32 chan_id, u32 value);
void socp_hal_set_src_rd_write_ptr(u32 chan_id, u32 value);
void socp_hal_set_src_rd_read_ptr(u32 chan_id, u32 value);
void socp_hal_set_src_buffer_length(u32 chan_id, u32 value);
void socp_hal_set_src_rd_buffer_length(u32 chan_id, u32 value);
void socp_hal_set_rptr_img_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void socp_hal_set_src_rd_timeout(u32 chan_id, u32 value);
void socp_hal_set_src_rd_wptr_addr(u32 chan_id, u32 addr_low, u32 addr_high);
void socp_hal_set_rate_ctrl_group(u32 chan_id, u32 value);
void socp_hal_set_src_chan_start_stop(u32 chan_id, u32 value);
void socp_hal_clear_src_chan_cfg(u32 chan_id);
void socp_hal_clear_rate_ctrl_cnt(u32 chan_id);
void socp_hal_clear_src_rd_raw_int(u32 chan_id);
u32 socp_hal_get_src_write_ptr(u32 chan_id);
u32 socp_hal_get_src_read_ptr(u32 chan_id);
u32 socp_hal_get_src_rd_write_ptr(u32 chan_id);
u32 socp_hal_get_src_start_stop_state(u32 chan_id);
u32 socp_hal_get_src_state(void);
u32 socp_hal_get_single_src_state(u32 chan_id);
s32 socp_hal_get_rate_ctrl_cnt(u32 chan_id);
u32 socp_hal_get_global_int_state(void);
void socp_hal_set_global_halt(u32 value);
void socp_hal_set_single_src_rd_timeout_int_mask(u32 chan_id, u32 value);
void socp_hal_set_single_src_rd_int_mask(u32 chan_id, u32 value);
void socp_hal_src_chan_init(u32 chan_id, socp_src_chan_cfg_s *src_attr);
u32 socp_hal_get_dst_buffer_length(u32 chan_id);
void socp_hal_set_dst_threshold_length(u32 chan_id, u32 value);
void socp_hal_set_dst_arbitrate_length(u32 chan_id, u32 value);
void socp_hal_set_dst_chan_start_stop(u32 chan_id, u32 value);
void socp_hal_set_rate_ctrl_cfg(u32 value1, u32 value2);
void socp_hal_set_rate_ctrl_peroid(u32 value);
void socp_hal_set_dst_write_ptr(u32 chan_id, u32 value);
void socp_hal_set_dst_read_ptr(u32 chan_id, u32 value);
void socp_hal_set_dst_buffer_length(u32 chan_id, u32 value);
void socp_hal_set_single_chan_timeout_mode(u32 chan_id, u32 value);
u32 socp_hal_get_dst_read_ptr(u32 chan_id);
u32 socp_hal_get_dst_write_ptr(u32 chan_id);
void socp_hal_set_dst_chan_trans_id_en(u32 chan_id, u32 value);
void socp_hal_set_dst_chan_mode(u32 chan_id, u32 value);
void socp_hal_set_dst_buffer_addr(u32 chan_id, u32 value1, u32 value2);
void socp_hal_clear_src_rd_raw_int(u32 chan_id);
void socp_hal_clear_src_rd_timout_raw_int(u32 chan_id);


#ifdef __cplusplus
}
#endif

#endif
