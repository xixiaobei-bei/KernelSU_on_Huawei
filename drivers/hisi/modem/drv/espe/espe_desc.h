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

#ifndef __SPE_DESC_H__
#define __SPE_DESC_H__

#include <linux/types.h>
#include "espe.h"

#define RNDIS_HDR_LEN 11

struct espe_nth32 {
    unsigned int sig;
    unsigned int nth_len : 16;
    unsigned int seq : 16;
    unsigned int total_len;
    unsigned int index;
};

struct espe_ndp32_hdr {
    unsigned int sig;
    unsigned int nth_len : 16;
    unsigned int reserve : 16;
    unsigned int next_ndp;
    unsigned int reserve1;
};

struct espe_ndp32_data {
    unsigned int index;
    unsigned int len;
};

struct espe_ncm_data {
    unsigned int addr_low;
    unsigned int addr_high;
    unsigned int len;
};

struct espe_rndis_hdr {
    unsigned int rndis_hdr[RNDIS_HDR_LEN];
};

struct espe_rndis_data {
    struct espe_rndis_hdr hdr;
    unsigned int addr_low;
    unsigned int addr_high;
};

#define SPE_ADQ_SIZE_SETTING 3
#define SPE_ADQ_DEF_SIZE (32 << SPE_ADQ_SIZE_SETTING)
#define SPE_ADQ_LONG 0x1
#define SPE_ADQ_SHORT 0x0
#define SPE_MIN_PKT_SIZE 1

#define SPE_AD0_PKT_LEN MAA_OPIPE_LEVEL_0_SIZE /* short adq */
#define SPE_AD1_PKT_LEN MAA_OPIPE_LEVEL_1_SIZE /* long adq */
#define SPE_AD_PKT_LEN_THR SPE_AD0_PKT_LEN     /* short adq */
#define SPE_V300_AD0_PKT_LEN 1560              /* short adq */
#define SPE_V300_AD1_PKT_LEN 1568              /* long adq */

#define SPE_AD_INTR_EN 0x80000000

#define ESPE_STOMS (1000 * 1000)
#define TD_MODE_INDEX(bit) ((bit) >> 0x8)
#define VCOM_IOCTL_PPP_DATA 0x5F04

int espe_ad_init(struct spe *spe);
void espe_ad_exit(struct spe *spe);
void espe_update_adq_wptr(struct spe *spe);
void espe_process_ipf_td_desc(struct spe *spe);
void espe_process_rd_desc(unsigned int portno);
void espe_process_td_desc(unsigned int portno);
void espe_rd_rewind(unsigned int portno);
void espe_adq_timer_init(struct spe *spe);
void espe_rd_per_interval_cnt(struct spe_port_ctx *port_ctx, unsigned int rd_total_num);

#ifndef SPE_MAA
int espe_ad_no_maa_init(struct spe *spe)
#endif

#endif /* __SPE_DESC_H__ */
