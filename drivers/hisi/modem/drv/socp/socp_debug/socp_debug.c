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
#include "socp_debug.h"
#include <securec.h>
#include "osl_sem.h"
#include "osl_thread.h"
#include "osl_types.h"
#include "bsp_slice.h"

socp_mntn_info_s g_socp_mntn_info;

/*
 * 函 数 名: bsp_SocpEncDstQueryIntInfo
 * 功能描述: 提供给diag_debug查询socp数据通道目的端中断信息
 * 输入参数: 无
 * 输出参数:
 * 返 回 值: 无
 */
void bsp_socp_mntn_enc_dst_int_info(u32 *trf_info, u32 *thrh_ovf_info)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(SOCP_CODER_DST_OM_IND);

    *trf_info = g_socp_mntn_info.dst_tfr_int_cnt[chan_id];
    *thrh_ovf_info = g_socp_mntn_info.dst_overflow_int_cnt[chan_id];
}

/*
 * 函 数 名: bsp_clear_socp_encdst_int_info
 * 功能描述: 清空socp目的端上溢统计值
 * 输入参数: 无
 * 输出参数:
 * 返 回 值: 无
 */
void bsp_clear_socp_encdst_int_info(void)
{
    u32 chan_id = SOCP_REAL_CHAN_ID(SOCP_CODER_DST_OM_IND);
    g_socp_mntn_info.dst_tfr_int_cnt[chan_id] = 0;
    g_socp_mntn_info.dst_overflow_int_cnt[chan_id] = 0;
}

/*
 * 函 数 名: bsp_get_socp_ind_dst_int_slice
 * 功能描述: 上报本次read cb调用时间戳
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: read_cb调用时间戳
 */
u32 bsp_get_socp_ind_dst_int_slice(void)
{
    u32 i = g_socp_mntn_info.ind_int_time.ptr;
    return g_socp_mntn_info.ind_int_time.read_cb_start_slice[i];
}


void socp_mntn_record_read_done_cnt(u32 chan_id)
{
    g_socp_mntn_info.dst_read_done_cnt[chan_id]++;
}

void socp_mntn_record_mode_swt_fail_cnt(u32 chan_id)
{
    g_socp_mntn_info.mode_swt_fail_cnt[chan_id]++;
}

void socp_mntn_record_read_done_start_slice(void)
{
    u32 i = g_socp_mntn_info.read_done_time.ptr;
    g_socp_mntn_info.read_done_time.read_done_start[i] = bsp_get_slice_value();
}

void socp_mntn_record_read_done_end_slice(void)
{
    u32 i = g_socp_mntn_info.read_done_time.ptr;
    g_socp_mntn_info.read_done_time.read_done_end[i] = bsp_get_slice_value();
    g_socp_mntn_info.read_done_time.ptr = (g_socp_mntn_info.read_done_time.ptr + 1) % SOCP_ENC_DST_RECORD_MAX;
}

void socp_mntn_record_read_done_fail_cnt(u32 chan_id)
{
    g_socp_mntn_info.dst_read_done_fail_cnt[chan_id]++;
}

void socp_mntn_record_read_done_succ_cnt(u32 chan_id)
{
    g_socp_mntn_info.dst_read_done_succ_cnt[chan_id]++;
}

void socp_mntn_record_header_error(u32 chan_id)
{
    g_socp_mntn_info.src_header_error_cnt[chan_id]++;
}

void socp_mntn_record_ind_int_start_slice(void)
{
    u32 i = g_socp_mntn_info.ind_int_time.ptr;

    g_socp_mntn_info.ind_int_time.int_start_slice[i] = bsp_get_slice_value();
}

void socp_mntn_record_dst_tfr_int_cnt(u32 chan_id)
{
    g_socp_mntn_info.dst_tfr_int_cnt[chan_id]++;
}

void socp_mntn_record_dst_overflow_int(u32 chan_id)
{
    g_socp_mntn_info.dst_overflow_int_cnt[chan_id]++;
}

void socp_mntn_record_tfr_task_cb_cnt(u32 chan_id)
{
    g_socp_mntn_info.tfr_task_read_cb_cnt[chan_id]++;
}

void socp_mntn_record_throvf_task_cb_cnt(u32 chan_id)
{
    g_socp_mntn_info.overflow_task_read_cb_cnt[chan_id]++;
    socp_report_overflow(chan_id);
}

void socp_mntn_record_ind_read_cb_start_slice(void)
{
    u32 i = g_socp_mntn_info.ind_int_time.ptr;
    g_socp_mntn_info.ind_int_time.read_cb_start_slice[i] = bsp_get_slice_value();
}

void socp_mntn_record_ind_read_cb_end_slice(void)
{
    u32 i = g_socp_mntn_info.ind_int_time.ptr;
    g_socp_mntn_info.ind_int_time.read_cb_end_slice[i] = bsp_get_slice_value();
    g_socp_mntn_info.ind_int_time.ptr = (g_socp_mntn_info.ind_int_time.ptr + 1) % SOCP_ENC_DST_RECORD_MAX;
}

void socp_clear_mntn_info(void)
{
    (void)memset_s(&g_socp_mntn_info, sizeof(g_socp_mntn_info), 0, sizeof(g_socp_mntn_info));
}

void socp_report_overflow(u32 chan_id)
{
    struct socp_report_chr socp_chr_data = { { 0 }, { { 0 }, { 0 } } };
    int ret;
    socp_chr_data.chr_head.alarmid = CHR_DRV_SOCP_DST_FULL_ALARMID;
    socp_chr_data.chr_head.faultid = 0;
    socp_chr_data.chr_head.msg_type = CHR_ALARM_ID_ERR_LOG_IND;
    socp_chr_data.chr_head.msgsn = 0;
    socp_chr_data.chr_head.msglen = sizeof(socp_overflow_info_s);
    socp_chr_data.socp_info.errinfo.m_almid = CHR_DRV_SOCP_DST_FULL_ALARMID;
    socp_chr_data.socp_info.errinfo.m_modem_id = 0;
    socp_chr_data.socp_info.errinfo.m_msg_moduleid = CHR_MOUDLE_ID_DRV;
    socp_chr_data.socp_info.errinfo.m_alm_HighSlice = 0;
    socp_chr_data.socp_info.errinfo.m_alm_LowSlice = bsp_get_slice_value();
    socp_chr_data.socp_info.errinfo.m_alm_Length = sizeof(socp_overflow_report_s);
    socp_chr_data.socp_info.report_info.chan_id = chan_id;
    socp_chr_data.socp_info.report_info.read_ptr = socp_hal_get_dst_read_ptr(chan_id);
    socp_chr_data.socp_info.report_info.write_ptr = socp_hal_get_dst_write_ptr(chan_id);
    ret = bsp_chr_drv_ind(&socp_chr_data, sizeof(struct socp_report_chr));
    if (ret != 0) {
        socp_error("report chr info error:%x\n", ret);
    }
}
void socp_show_mntn_info(void)
{
    u32 i;

    for (i = 0; i < SOCP_MAX_ENCSRC_CHN; i++) {
        socp_crit("src_header_error_cnt[%d] = %d\n", i, g_socp_mntn_info.src_header_error_cnt[i]);
        socp_crit("***********************************\n");
        socp_crit("\n");
    }

    for (i = 0; i < SOCP_MAX_ENCDST_CHN; i++) {
        socp_crit("dst_tfr_int_cnt[%d] = %d\n", i, g_socp_mntn_info.dst_tfr_int_cnt[i]);
        socp_crit("dst_overflow_int_cnt[%d] = %d\n", i, g_socp_mntn_info.dst_overflow_int_cnt[i]);
        socp_crit("tfr_task_read_cb_cnt[%d] = %d\n", i, g_socp_mntn_info.tfr_task_read_cb_cnt[i]);
        socp_crit("overflow_task_read_cb_cnt[%d] = %d\n", i, g_socp_mntn_info.overflow_task_read_cb_cnt[i]);
        socp_crit("dst_read_done_cnt[%d] = %d\n", i, g_socp_mntn_info.dst_read_done_cnt[i]);
        socp_crit("dst_read_done_fail_cnt[%d] = %d\n", i, g_socp_mntn_info.dst_read_done_fail_cnt[i]);
        socp_crit("dst_read_done_succ_cnt[%d] = %d\n", i, g_socp_mntn_info.dst_read_done_succ_cnt[i]);
        socp_crit("mode_swt_fail_cnt[%d] = %d\n", i, g_socp_mntn_info.mode_swt_fail_cnt[i]);
        socp_crit("***********************************\n");
        socp_crit("\n");
    }
}

void socp_show_mntn_time_info(void)
{
    u32 i;

    for (i = 0; i < g_socp_mntn_info.ind_int_time.ptr; i++) {
        socp_crit("ptr = %d\n", i);
        socp_crit("int_start_slice = 0x%x\n", g_socp_mntn_info.ind_int_time.int_start_slice[i]);
        socp_crit("read_cb_start_slice = 0x%x\n", g_socp_mntn_info.ind_int_time.read_cb_start_slice[i]);
        socp_crit("read_cb_end_slice = 0x%x\n", g_socp_mntn_info.ind_int_time.read_cb_end_slice[i]);
        socp_crit("***********************************\n");
        socp_crit("\n");
    }

    return;
}


