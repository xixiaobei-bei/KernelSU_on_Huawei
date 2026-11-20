/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef LINUX_PSAM_BALONG_H
#define LINUX_PSAM_BALONG_H

#include "mdrv_ipf_comm.h"
#include "hi_psam.h"
#include "n_bsp_psam.h"


#define PSAM_ULADQ_PLEN_TH              404
#define PSAM_DLAD0_DESC_NUM            	IPF_DLAD0_DESC_SIZE
#define PSAM_DLAD1_DESC_NUM            	IPF_DLAD1_DESC_SIZE

//-for psam test
#define PSAM_SRST_BIT					BIT(0)
#define PSAM_IDLE_BIT					BIT(1)
#define PSAM_DLRD_DESC_SIZE				IPF_DLRD_DESC_SIZE
#define PSAM_DLAD0_DESC_SIZE            IPF_DLAD0_DESC_SIZE
#define PSAM_DLAD1_DESC_SIZE            IPF_DLAD1_DESC_SIZE

typedef struct tagIPF_AD_DESC_S PSAM_AD_DESC_S;
//+for psam test


#define PSAM_ADQ_RESERVE_NUM            1
#define PSAM_ADQ_NUM            		0x2
#define P_CTRL_4K						0x00001000

#define DESC_FULL  0x10
#define DESC_EMPTY 0x01


#define HI_PSAM_INT_STAT_OFFSET(x)		(HI_PSAM_INT0_STAT_OFFSET + (0xc * (x)))
#define HI_PSAM_INT_MSTAT_OFFSET(x)		(HI_PSAM_INT0_MSTAT_OFFSET + (0xc * (x)))
#define HI_PSAM_INT_MASK_OFFSET(x)      (HI_PSAM_ADQ0_WPTR_OFFSET + (0xc * (x)))

#define PSAM_ACORE_INIT_SUC             IPF_ACORE_INIT_SUCCESS

struct psam_debug_info {
	unsigned int	adq0_epty_int1;
	unsigned int 	adq1_epty_int1;
	unsigned int	dlad_outptr_err[PSAM_ADQ_NUM];
	unsigned int	dlad_desc_null[PSAM_ADQ_NUM];
	unsigned int	dlad_type_err;
	unsigned int 	get_dlad_num;
	unsigned int	cfg_dlad_num_err;
	unsigned int	cfg_dl_ad_succ[PSAM_ADQ_NUM];
	unsigned int 	cfg_get_ad_para_null[PSAM_ADQ_NUM];
	unsigned int 	lbdq_upoverflow;
	unsigned int 	lbdq_full;
	unsigned int 	lbdq_empty;
	unsigned int 	scr_ad_addr_exc;
    unsigned int    adq_free_cnt;
    unsigned int    adq_p1_err;
    unsigned int    rd0_wbuf_full;
    unsigned int    rd1_wbuf_full;
    unsigned int    rd2_wbuf_full;
    unsigned int    rd0_wbuf_overflow;
    unsigned int    rd1_wbuf_overflow;
    unsigned int    rd2_wbuf_overflow;
    unsigned int    adq0_upoverflow;
    unsigned int    adq1_upoverflow;
    unsigned int    adq0_full;
    unsigned int    adq1_full;
	unsigned int 	init_ok;
	unsigned int	not_idle;
	unsigned int	adq0_empty;
	unsigned int	adq1_empty;
};

typedef struct tagpsam_desc_buffer_info
{
	unsigned int vir_addr; 	//desc virtual address
	unsigned int phy_addr; 	//desc physical address
	unsigned int len;		//buffer length
	unsigned int cur_wt_p;	//current write point
	unsigned int cur_rd_p;	//current read point
	unsigned int loop;		//not use
}psam_desc_buf_info;

struct psam_hal_handle
{
	u64 (*get_ad0_base)(void);
	u64 (*get_ad1_base)(void);
	void (*set_ad_base)(void);
	int (*config_ad)(IPF_AD_TYPE_E type, unsigned int n, IPF_AD_DESC_S * param);
	void (*save_reg)(unsigned int*);
	void (*restore_reg)(unsigned int*);
};

struct psam_device {
	struct device		*dev;
	void __iomem		*regs;
	struct clk		*clk;
	dma_addr_t		dma[PSAM_ADQ_NUM];
	void	*desc[PSAM_ADQ_NUM];
	unsigned int 	desc_num[PSAM_ADQ_NUM];
	unsigned int 	mem[PSAM_ADQ_NUM];
	int			irq;
	unsigned int irq_flags;
	unsigned int ad_num;
	unsigned int psam_ver;
	struct psam_debug_info debug;
	void *ipf_deb;
	adq_emtpy_cb_t adq_emtpy_cb;
	psam_desc_buf_info adq0_info;
	psam_desc_buf_info adq1_info;
	struct ipf_desc_handler_s* desc_hd;
	struct psam_hal_handle* hal;
	struct psam_pm_cb* pm;
    unsigned int ads_up;
};

#define psam_readl(x) /*lint -save -e732 */readl((unsigned char*)g_psam_device->regs + (x))/*lint -restore */
#define psam_writel(v,a)	writel(v,(unsigned char*)g_psam_device->regs + (a))

int psam_srset(void);
int psam_reinit_regs(void);
void psam_help(void);


#endif /* LINUX_PSAM_BALONG_H */

