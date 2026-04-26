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

#ifndef _SOCP_MNTN_H
#define _SOCP_MNTN_H

#include "mdrv_chr.h"
#include "bsp_chr.h"
#include "socp_hal_common.h"
#include <osl_thread.h>
#include "bsp_memmap.h"
#include "bsp_socp.h"
#include "bsp_print.h"


#ifdef __cplusplus
extern "C" {
#endif

#define SOCP_ENC_DST_RECORD_MAX 64

typedef struct {
    u32 ptr;
    u32 read_done_start[SOCP_ENC_DST_RECORD_MAX];
    u32 read_done_end[SOCP_ENC_DST_RECORD_MAX];
} read_done_time_s;

typedef struct {
    u32 ptr;
    u32 int_start_slice[SOCP_ENC_DST_RECORD_MAX];
    u32 read_cb_start_slice[SOCP_ENC_DST_RECORD_MAX];
    u32 read_cb_end_slice[SOCP_ENC_DST_RECORD_MAX];
} int_proc_time_s;


typedef struct {
    /* enc src cnt */
    u32 src_header_error_cnt[SOCP_MAX_ENCSRC_CHN];
    /* enc dst cnt */
    u32 dst_read_done_cnt[SOCP_MAX_ENCDST_CHN];
    u32 dst_read_done_fail_cnt[SOCP_MAX_ENCDST_CHN];
    u32 dst_read_done_succ_cnt[SOCP_MAX_ENCDST_CHN];
    u32 mode_swt_fail_cnt[SOCP_MAX_ENCDST_CHN];
    u32 dst_tfr_int_cnt[SOCP_MAX_ENCDST_CHN];
    u32 dst_overflow_int_cnt[SOCP_MAX_ENCDST_CHN];
    u32 tfr_task_read_cb_cnt[SOCP_MAX_ENCDST_CHN];
    u32 overflow_task_read_cb_cnt[SOCP_MAX_ENCDST_CHN];
    /* proc time length */
    read_done_time_s read_done_time;
    int_proc_time_s ind_int_time;
} socp_mntn_info_s;


typedef struct {
    unsigned int m_msg_moduleid;
    unsigned short m_modem_id;
    unsigned short m_almid;
    unsigned short m_alm_level;
    unsigned short m_alm_type;
    unsigned int m_alm_LowSlice;
    unsigned int m_alm_HighSlice;
    unsigned int m_alm_Length;
} om_errorinfo_payload_stru;

typedef struct {
    unsigned int chan_id;
    unsigned int read_ptr;
    unsigned int write_ptr;
} socp_overflow_report_s;

typedef struct {
    om_errorinfo_payload_stru errinfo;
    socp_overflow_report_s report_info;
} socp_overflow_info_s;
struct socp_report_chr {
    chr_drv_ind_s chr_head;
    socp_overflow_info_s socp_info;
};
void socp_report_overflow(u32 chan_id);
void socp_mntn_record_read_done_cnt(u32 chan_id);
void socp_mntn_record_mode_swt_fail_cnt(u32 chan_id);
void socp_mntn_record_read_done_start_slice(void);
void socp_mntn_record_read_done_end_slice(void);
void socp_mntn_record_read_done_fail_cnt(u32 chan_id);
void socp_mntn_record_read_done_succ_cnt(u32 chan_id);
void socp_mntn_record_header_error(u32 chan_id);
void socp_mntn_record_ind_int_start_slice(void);
void socp_mntn_record_dst_tfr_int_cnt(u32 chan_id);
void socp_mntn_record_dst_overflow_int(u32 chan_id);
void socp_mntn_record_tfr_task_cb_cnt(u32 chan_id);
void socp_mntn_record_throvf_task_cb_cnt(u32 chan_id);
void socp_mntn_record_ind_read_cb_start_slice(void);
void socp_mntn_record_ind_read_cb_end_slice(void);

#ifdef __cplusplus
}
#endif

#endif
