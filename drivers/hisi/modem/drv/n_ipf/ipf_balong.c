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



#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <bsp_pm_om.h>
#include <linux/syscore_ops.h>
#include <bsp_ipc.h>
#include "osl_cache.h"
#include <bsp_ddr.h>
#include <n_bsp_psam.h>
#include <n_bsp_ipf.h>
#include "ipf_balong.h"
#include <bsp_reset.h>
#include <bsp_vic.h>
#include <bsp_slice.h>
#include <securec.h>

#define IPF_VIC_LEVEL		5
#define DELAY_WAIT_CIPHER   2
ipf_ctx_t g_ipf_ctx;
extern struct dev_pm_ops ipf_dev_pm_ops;
extern struct ipf_desc_handler_s ipf_desc_handler;
extern struct ipf_desc_handler_s ipf_desc64_handler;

static void ipf_get_version(void)
{
	unsigned int ver;
	ver = ipf_readl(HI_IPF_VERSION_OFFSET);
	switch(ver){
	case IPF_VER_120A:
		g_ipf_ctx.ipf_version = IPF_VERSION_120a;
		break;
	case IPF_VER_130A:
		g_ipf_ctx.ipf_version = IPF_VERSION_130a;
		break;
	case IPF_VER_130B:
		g_ipf_ctx.ipf_version = IPF_VERSION_130b;
		break;
	case IPF_VER_140A:
		g_ipf_ctx.ipf_version = IPF_VERSION_140a;
		break;
    case IPF_VER_150A:
        g_ipf_ctx.ipf_version = IPF_VERSION_150a;
        break;
    case IPF_VER_160:
        g_ipf_ctx.ipf_version = IPF_VERSION_160;
        break;
    case IPF_VER_170:
        g_ipf_ctx.ipf_version = IPF_VERSION_170;
        break;
	default:
		g_ipf_ctx.ipf_version = IPF_VERSION_0000;
		break;
	}
}

static int ipf_get_limit_addr(void)
{
    struct memblock_type *type;
	phys_addr_t mem_start;
	phys_addr_t mem_end;
    unsigned int i;

	type = &memblock.memory;
	if(0 == type->cnt){
		bsp_err("ipf limit addr not get\n");
		g_ipf_ctx.limit_addr->block_err = 1;
		return BSP_ERR_IPF_INVALID_PARA;
	}

    mem_start = type->regions[0].base;
	mem_end = type->regions[type->cnt-1].base + type->regions[type->cnt-1].size;

    *(g_ipf_ctx.memblock_show) = type->cnt;
    for(i=0; i < type->cnt; i++){
        *(g_ipf_ctx.memblock_show + i + 1) = (unsigned int)(type->regions[i].base);
        if(mem_start > type->regions[i].base){
            mem_start = type->regions[i].base;
        }
        
        if(mem_end < (type->regions[i].base + type->regions[i].size)){
            mem_end = type->regions[i].base + type->regions[i].size;
        }
    }

    if(mem_end > (phys_addr_t)0xffffffff){
        mem_end = (phys_addr_t)0xffffffff;
    }
    
	if((MODEM_START_ADDR >= mem_start) && ((MODEM_START_ADDR + MODEM_START_SIZE) <= mem_end)){
		g_ipf_ctx.limit_addr->block_sum 		=2;
		g_ipf_ctx.limit_addr->block1_start 		=(unsigned int)mem_start;
		g_ipf_ctx.limit_addr->block1_end		=MODEM_START_ADDR;
		g_ipf_ctx.limit_addr->block2_start 		=MODEM_START_ADDR + MODEM_START_SIZE;
		g_ipf_ctx.limit_addr->block2_end		=(unsigned int)mem_end;
	}
	else{
		g_ipf_ctx.limit_addr->block_sum 		=1;
		g_ipf_ctx.limit_addr->block1_start 		=(unsigned int)mem_start;
		g_ipf_ctx.limit_addr->block1_end		=(unsigned int)mem_end;
	}

	return 0;
}

static void ipf_dl_job_done_cb(void){
    if(g_ipf_ctx.dl_info.pFnDlIntCb){
        g_ipf_ctx.dl_info.pFnDlIntCb();
    }
}

static void ipf_dl_ads_empty_cb(void){
    if(g_ipf_ctx.dl_info.pAdqEmptyDlCb)
        g_ipf_ctx.dl_info.pAdqEmptyDlCb(IPF_EMPTY_ADQ0);
}

static void ipf_dl_adl_empty_cb(void){
    if(g_ipf_ctx.dl_info.pAdqEmptyDlCb)
        g_ipf_ctx.dl_info.pAdqEmptyDlCb(IPF_EMPTY_ADQ1);
}

static void ipf_ul_bd_empty_cb(void){
    if(g_ipf_ctx.ul_info.handle_bd_empty)
        g_ipf_ctx.ul_info.handle_bd_empty();
}

static void ipf_mst_sec_rd_err_cb(void){
    disable_irq_nosync(g_ipf_ctx.irq);
	bsp_err("ipf sec rd err\n");
//	system_error(DRV_ERRNO_IPF_OUT_REG, 0, 0, (char*)&sm->uad0, sizeof(ipf_uad_u)+sizeof(ipf_uad_u));
}

static void ipf_mst_sec_wr_err_cb(void){
    disable_irq_nosync(g_ipf_ctx.irq);
	bsp_err("ipf sec wr err\n");
//	system_error(DRV_ERRNO_IPF_OUT_REG, 0, 0, (char*)&sm->uad0, sizeof(ipf_uad_u)+sizeof(ipf_uad_u));
}

struct int_handler ipf_int_table[32] = {
		{"ul_rpt_int1",                 0,  NULL},                  /* [0..0] 上行结果上报中断1 */
		{"ul_timeout_int1",             0,  NULL},                  /* [1..1] 上行结果上报超时中断1 */
		{"ul_disable_end_int1",         0,  NULL},                  /* [2..2] 上行通道关闭完成中断1 */
		{"ul_idle_cfg_bd_int1",         0,  NULL},                  /* [3..3] 上行通道IDLE期间软件配置BD指示中断1 */
		{"ul_trail_cfg_bd_int1",        0,  NULL},                  /* [4..4] 上行通道关闭但BD没有处理完期间软件继续配置BD指示中断1 */
		{"ul_noidle_clrptr_int1",       0,  NULL},                  /* [5..5] 上行非IDLE态清指针指示中断1 */
		{"ul_rdq_downoverflow_int1",    0,  NULL},                  /* [6..6] 上行RDQ下溢中断1 */
		{"ul_bdq_upoverflow_int1",      0,  NULL},                  /* [7..7] 上行BDQ上溢中断1 */
		{"ul_rdq_full_int1",            0,  NULL},                  /* [8..8] 上行RDQ满中断1 */
		{"ul_bdq_epty_int1",            0,  ipf_ul_bd_empty_cb},  	/* [9..9] 上行BDQ空中断1 */
		{"ul_adq0_epty_int1",           0,  NULL},                  /* [10..10] 上行ADQ0空中断1 */
		{"ul_adq1_epty_int1",           0,  NULL},                  /* [11..11] 上行ADQ1空中断1 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"dl_rpt_int1",                 0,  ipf_dl_job_done_cb},	/* [16..16] 下行结果上报中断1 */
		{"dl_timeout_int1",             0,  ipf_dl_job_done_cb},	/* [17..17] 下行结果上报超时中断1 */
		{"dl_disable_end_int1",         0,  NULL},                  /* [18..18] 下行通道关闭完成中断1 */
		{"dl_idle_cfg_bd_int1",         0,  NULL},                  /* [19..19] 下行通道IDLE期间软件配置BD指示中断1 */
		{"dl_trail_cfg_bd_int1",        0,  NULL},                  /* [20..20] 下行通道关闭但BD没有处理完期间软件继续配置BD指示中断1 */
		{"dl_noidle_clrptr_int1",       0,  NULL},                  /* [21..21] 下行非IDLE态清指针指示中断1 */
		{"dl_rdq_downoverflow_int1",    0,  NULL},                  /* [22..22] 下行RDQ下溢中断1 */
		{"dl_bdq_upoverflow_int1",      0,  NULL},                  /* [23..23] 下行BDQ上溢中断1 */
		{"dl_rdq_full_int1",            0,  ipf_dl_job_done_cb},	/* [24..24] 下行RDQ满中断1 */
		{"dl_bdq_epty_int1",            0,  NULL},                  /* [25..25] 下行BDQ空中断1 */
		{"dl_adq0_epty_int1",           0,  ipf_dl_ads_empty_cb},	/* [26..26] 下行ADQ0空中断1 */
		{"dl_adq1_epty_int1",           0,  ipf_dl_adl_empty_cb},	/* [27..27] 下行ADQ1空中断1 */
		{"ipf_mst_sec_wr_err_int1",     0,  ipf_mst_sec_wr_err_cb},	/* [28..28] IPF MST写操作地址溢出中断1。 */
		{"ipf_mst_sec_rd_err_int1",     0,  ipf_mst_sec_rd_err_cb},	/* [29..29] IPF MST读操作地址溢出中断1。 */
		{"reserved_0",                  0,  NULL},                  /* [31..30] 保留。 */
		{"timer_cnt_eq0_int0",          0,  NULL},           		/* [31..30] 保留。 */
};

static irqreturn_t ipf_interrupt(int irq, void* dev)
{
    int bit;
    unsigned long reg[1] = {0};

    reg[0] = ipf_readl(HI_IPF_INT1_OFFSET);
    ipf_writel(reg[0], HI_IPF_INT_STATE_OFFSET);

    for_each_set_bit(bit, reg, 32)
    {
        ipf_int_table[bit].cnt++;
        if(ipf_int_table[bit].callback){
            ipf_int_table[bit].callback();
        }
    }
    
    return IRQ_HANDLED;
}

static int ipf_ctx_init(void)
{
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

	g_ipf_ctx.share_mem = sm;

    g_ipf_ctx.dma_mask = g_ipf_ctx.desc->dma_mask;
    dma_set_mask_and_coherent(g_ipf_ctx.dev, g_ipf_ctx.dma_mask);
    
    g_ipf_ctx.ul_info.pstIpfBDQ     = dmam_alloc_coherent(
                g_ipf_ctx.dev, 
				sizeof(ipf_ubd_u),
				&g_ipf_ctx.ul_info.pstIpfPhyBDQ,
				GFP_KERNEL);
    if(!g_ipf_ctx.ul_info.pstIpfBDQ)
    {
        bsp_err("BDQ no mem!\n");
        goto ERROR0;
    }
	
    g_ipf_ctx.ul_info.pu32IdleBd    = &sm->idle;
/*
    g_ipf_ctx.dl_info.pstIpfRDQ     = kmalloc(
				sizeof(ipf_drd_u)+128,	
				GFP_KERNEL | __GFP_DMA);
*/
    g_ipf_ctx.dl_info.pstIpfRDQ     = dmam_alloc_coherent(
                g_ipf_ctx.dev,
                sizeof(ipf_drd_u),
                &g_ipf_ctx.dl_info.pstIpfPhyRDQ,
                GFP_KERNEL);
    if(!g_ipf_ctx.dl_info.pstIpfRDQ)
    {
        bsp_err("RDQ no mem!\n");
        goto ERROR1;
    }
/*
    g_ipf_ctx.dl_info.pstIpfRDQ = (void *)(((unsigned long)g_ipf_ctx.dl_info.pstIpfRDQ +32) & (unsigned long)(~31));
    dma_map_single(g_ipf_ctx.dev, g_ipf_ctx.dl_info.pstIpfRDQ, sizeof(ipf_drd_u), DMA_FROM_DEVICE);
    
    g_ipf_ctx.dl_info.pstIpfPhyRDQ  = virt_to_phys(g_ipf_ctx.dl_info.pstIpfRDQ);
*/
    g_ipf_ctx.dl_info.pstIpfCDQ     = &sm->dcd;
    g_ipf_ctx.dl_info.u32IpfCdRptr = &sm->dcd_rptr;
    *(g_ipf_ctx.dl_info.u32IpfCdRptr) = 0;

    return 0;

ERROR1:
    dmam_free_coherent(g_ipf_ctx.dev, 
				sizeof(ipf_ubd_u),
				g_ipf_ctx.ul_info.pstIpfBDQ,
				g_ipf_ctx.ul_info.pstIpfPhyBDQ);
ERROR0:
    return -ENOMEM;
}

struct ipf_desc_handler_s* ipf_get_desc_handler(unsigned int version_id)
{
	if(version_id>IPF_VERSION_150a)
        return &ipf_desc64_handler;
    else
        return &ipf_desc_handler;
}

int ipf_init(void)
{
	int ret = 0;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

	ipf_get_version();
	g_ipf_ctx.desc = ipf_get_desc_handler(g_ipf_ctx.ipf_version);
	
   	ret = ipf_ctx_init();
   	if(0 != ret) {
		bsp_err("ipf_ctx_init failed\n");
	}
    g_ipf_ctx.desc->config();
    
	g_ipf_ctx.irq_hd = ipf_int_table;
	ret = request_irq(g_ipf_ctx.irq, ipf_interrupt, IRQF_SHARED, "ipf", g_ipf_ctx.dev);
	if(0 != ret) {
		bsp_err("request irq failed\n");
	}

    sm->init.status.modem = (unsigned int)IPF_FORRESET_CONTROL_ALLOW;

    if(0 != mdrv_sysboot_register_reset_notify("IPF_BALONG",bsp_ipf_reset_ccore_cb, 0, DRV_RESET_CB_PIOR_IPF))
    {
    	bsp_err("set modem reset call back func failed\n");
    }

	/*acore use first block,ccore use scnd block*/
	memset_s(sm->debug, sizeof(sm->debug), 0, sizeof(struct ipf_debug));
	g_ipf_ctx.status = (struct ipf_debug*)&sm->debug[0] ;

	g_ipf_ctx.desc = ipf_get_desc_handler(g_ipf_ctx.ipf_version);
    g_ipf_ctx.status->init_ok = IPF_ACORE_INIT_SUCCESS;
    g_ipf_ctx.modem_status = IPF_FORRESET_CONTROL_ALLOW;
	g_ipf_ctx.ccore_rst_idle = 0;

	g_ipf_ctx.psam_para.desc_hd = g_ipf_ctx.desc;
	g_ipf_ctx.psam_pm = bsp_psam_set_ipf_para(&g_ipf_ctx.psam_para);

	bsp_err("ipf init success\n");

	return ret;
}

void bsp_ipf_set_debug_para(void ** psam_get_debug)
{
	*(psam_get_debug) = (void *)(g_ipf_ctx.status);
}

int bsp_ipf_srest(void)
{
	unsigned int reg;
	int tmp_cnt = 10000;
	
	/*reset*/
	UPDATE1(reg,	HI_IPF_SRST,
			ipf_srst,	1);
	while(!ipf_readl(HI_IPF_SRST_STATE_OFFSET)){
		tmp_cnt--;
		if(!tmp_cnt){
			return -1;
		}
	}

	ipf_writel(IPF_INT_OPEN0, HI_IPF_INT_MASK0_OFFSET);
    ipf_writel(IPF_INT_OPEN1, HI_IPF_INT_MASK1_OFFSET);
	return 0;
}

int bsp_ipf_config_timeout(unsigned int u32Timeout)
{
	unsigned int reg;

    if((u32Timeout == 0) || (u32Timeout > 0xFFFF))
    {
    	bsp_err("%s para invalid\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }

	UPDATE2(reg,	HI_IPF_TIME_OUT,
			time_out_cfg,	u32Timeout,
			time_out_valid,	1);
    return IPF_SUCCESS;
}

void bsp_set_init_status(int stat)
{
    g_ipf_ctx.status->init_ok = stat;    
}

int bsp_get_init_status(void)
{
    return g_ipf_ctx.status->init_ok;    
}

void mdrv_ipf_reinit_dlreg(void)
{	
	g_ipf_ctx.desc->config();
	
	psam_reinit_regs();

    g_ipf_ctx.status->init_ok = IPF_ACORE_INIT_SUCCESS;
    bsp_err("ipf dl register reinit success\n");

    return;

}
int mdrv_ipf_get_used_dlad(IPF_AD_TYPE_E eAdType, unsigned int * pu32AdNum, IPF_AD_DESC_S * pstAdDesc)
{	
	return bsp_psam_get_used_dlad(eAdType, pu32AdNum, pstAdDesc);
}

void bsp_ipf_set_control_flag_for_ccore_reset(IPF_FORRESET_CONTROL_E eResetFlag)
{
        struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    /*设置标志，终止上行数传*/
    g_ipf_ctx.modem_status = eResetFlag;

    /*设置标志，终止下行数传*/
    sm->init.status.modem = eResetFlag;

    cache_sync();
}

unsigned int ipf_ccore_is_down(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    return (ipf_flag_reset == sm->init.status.save);
}

int bsp_ipf_reset_ccore_cb(DRV_RESET_CB_MOMENT_E eparam, int userdata)
{
    unsigned int idle_cnt = 10;
    unsigned int time_out = 2000;
    unsigned int u32DlStateValue = 0;
    int psam_status = 0;
    unsigned int tmp0,tmp1;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    (void)userdata;
    
    if(MDRV_RESET_CB_BEFORE == eparam)
    {
        bsp_ipf_set_control_flag_for_ccore_reset(IPF_FORRESET_CONTROL_FORBID);

        do{
            udelay(DELAY_WAIT_CIPHER);  //wait cipher idle
            if(g_ipf_ctx.ipf_version <  IPF_VERSION_160){
                u32DlStateValue = ipf_readl(HI_IPF32_CH1_STATE_OFFSET);
            }
            else{
                u32DlStateValue = ipf_readl(HI_IPF64_CH1_STATE_OFFSET);
            }
            psam_status = bsp_psam_idle();
            time_out--;
            if((u32DlStateValue == IPF_CHANNEL_STATE_IDLE) && psam_status){
                idle_cnt--;
            }else{
                idle_cnt = 10;
            }
        }while(idle_cnt && time_out);

        if(!idle_cnt){
            g_ipf_ctx.ccore_rst_idle = 1;
        }

        if (!time_out)
        {
            bsp_err(
                "\r IPF dl channel on after bsp_ipf_reset_ccore_cb called \n");
            g_ipf_ctx.stax.crst_timeout = bsp_get_slice_value();    
        }
        g_ipf_ctx.cb_before++;
    }
    else if(MDRV_RESET_CB_AFTER == eparam)
    {
        g_ipf_ctx.ccore_rst_idle = 0;
        g_ipf_ctx.status->cp_flag = sm->init.status.modem;
        g_ipf_ctx.status->rst_ts = bsp_get_slice_value();
        memcpy_s(g_ipf_ctx.backup.debug, sizeof(g_ipf_ctx.backup.debug),
                &sm->debug[0] ,sizeof(struct ipf_debug)*2);
        g_ipf_ctx.backup.flag = sm->init.status.modem;

        psam_get_dlad_num(&tmp0, &tmp1);
        if(tmp0==0 || tmp1==0)
        {
            bsp_ipf_show_status();
        }

        bsp_ipf_set_control_flag_for_ccore_reset(IPF_FORRESET_CONTROL_ALLOW);
        g_ipf_ctx.status->ccore_reset++;
        return IPF_SUCCESS;
    }
    else
    {
        g_ipf_ctx.cb_undef++;
        return IPF_SUCCESS;
    }

    return IPF_SUCCESS;
}


int ipf_register_wakeup_dlcb(BSP_IPF_WakeupDlCb pFnWakeupDl)
{
    /* 参数检查 */
    if(NULL == pFnWakeupDl)
    {
    	g_ipf_ctx.status->invalid_para++;
    	bsp_err("%s invalid para\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }
    g_ipf_ctx.dl_info.pFnDlIntCb = pFnWakeupDl;
    return IPF_SUCCESS;
}

int ipf_register_ul_bd_empty(ipf_bd_empty bd_handle)
{
   if(NULL == bd_handle)
    {
    	g_ipf_ctx.status->invalid_para++;
    	bsp_err("%s invalid para\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }
    g_ipf_ctx.ul_info.handle_bd_empty = bd_handle;
	return 0;
}

unsigned int mdrv_ipf_get_ulbd_num(void)
{
    unsigned int u32IdleBd;	
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

	g_ipf_ctx.status->get_bd_num_times++;

	/* if ipf suspended, return 0*/
	if(sm->init.status.acore==IPF_PWR_DOWN){
		return 0;
	}

    if(IPF_FORRESET_CONTROL_FORBID <= g_ipf_ctx.modem_status)
    {
       	g_ipf_ctx.status->mdrv_called_not_init++;
        return BSP_ERR_IPF_CCORE_RESETTING;
    }

    u32IdleBd = g_ipf_ctx.desc->get_bd_num();
    *(g_ipf_ctx.ul_info.pu32IdleBd) = u32IdleBd;

    if(0 == u32IdleBd)
	{
		g_ipf_ctx.status->bd_full++;
	}
    return u32IdleBd;
}

unsigned int mdrv_ipf_get_ulrd_num(void)
{
    return g_ipf_ctx.desc->get_ulrd_num();
}

unsigned int mdrv_ipf_get_uldesc_num(void)
{
	return mdrv_ipf_get_ulbd_num();
}

int mdrv_ipf_config_ulbd(unsigned int u32Num, IPF_CONFIG_ULPARAM_S* pstUlPara)
{
	unsigned int i;
	
	g_ipf_ctx.status->cfg_bd_times++;

    /* 参数检查 */
    if((NULL == pstUlPara)||(0 == u32Num))
    {
    	g_ipf_ctx.status->invalid_para++;
    	bsp_err("%s para invalid\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }

    /* 检查模块是否初始化 */
    if(g_ipf_ctx.status && (IPF_ACORE_INIT_SUCCESS != g_ipf_ctx.status->init_ok))
    {
		g_ipf_ctx.status->mdrv_called_not_init++;
		bsp_err("%s ipf not init\n", __func__);
        return BSP_ERR_IPF_NOT_INIT;
    }

    /* 检查Ccore是否上电*/
    if(IPF_FORRESET_CONTROL_FORBID <= g_ipf_ctx.modem_status)
    {
       	g_ipf_ctx.status->mdrv_called_not_init++;
        return BSP_ERR_IPF_CCORE_RESETTING;
    }

    if(ipf_ccore_is_down())
    {
        g_ipf_ctx.desc->acpu_wake_ccpu();
    }

    for(i = 0; i < u32Num; i++)
    {
        if(0 == pstUlPara[i].u16Len)
        {
        	bsp_err("%s the %d bdlen is zero drop packet\n", __func__, i);
			g_ipf_ctx.status->bd_len_zero_err++;
            return BSP_ERR_IPF_INVALID_PARA;
        }
    }

	/* æ£€æŸ¥Ccoreæ˜¯å¦ä¸Šç”µ*/
    if(IPF_FORRESET_CONTROL_FORBID <= g_ipf_ctx.modem_status)
    {
    	g_ipf_ctx.status->mdrv_called_not_init++;
        return BSP_ERR_IPF_CCORE_RESETTING;
    }

    g_ipf_ctx.desc->config_bd(u32Num, pstUlPara);
    return IPF_SUCCESS;
}
int mdrv_ipf_config_dlad(IPF_AD_TYPE_E eAdType, unsigned int u32AdNum, IPF_AD_DESC_S * pstAdDesc)
{
	return psam_config_dlad(eAdType, u32AdNum, pstAdDesc);
}

int mdrv_ipf_register_ops(struct mdrv_ipf_ops *ops)
{
	if(ops)
	{
		if(ipf_register_wakeup_dlcb(ops->rx_complete_cb)!= IPF_SUCCESS)
			return IPF_ERROR;
		if(psam_register_adq_empty_dlcb((adq_emtpy_cb_t)ops->adq_empty_cb)!= IPF_SUCCESS)
			return IPF_ERROR;
	} else {
		bsp_err("%s para invalid\n", __func__);
		g_ipf_ctx.status->invalid_para++;
		return IPF_ERROR;
	}
	return IPF_SUCCESS;
}

void mdrv_ipf_get_dlrd(unsigned int* pu32Num, IPF_RD_DESC_S *pstRd)
{
    g_ipf_ctx.desc->get_rd(pu32Num, pstRd);
}

int mdrv_ipf_get_dlad_num (unsigned int* pu32AD0Num, unsigned int* pu32AD1Num)
{
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	/* if ipf suspended, return fail*/
	if(sm->init.status.acore==IPF_PWR_DOWN){
		return -EINVAL;
	}

	return psam_get_dlad_num(pu32AD0Num, pu32AD1Num);
}

unsigned int mdrv_ipf_get_dlrd_num(void)
{
    return g_ipf_ctx.desc->get_dlrd_num();
}

int reset_ipf_psam_from_sys(void)
{
	unsigned int regvalue = 0;
	unsigned int cnt = 1000;
	void* base_addr;

    base_addr = g_ipf_ctx.peribase;
    
	regvalue = readl((unsigned char*)base_addr+ g_ipf_ctx.reset_peri_crg[1]);
	regvalue |= g_ipf_ctx.reset_peri_crg[4];
	writel(regvalue, (unsigned char*)base_addr + g_ipf_ctx.reset_peri_crg[1]);
	regvalue = readl((unsigned char*)base_addr + g_ipf_ctx.reset_peri_crg[2]);
	regvalue |= g_ipf_ctx.reset_peri_crg[4];
	writel(regvalue, (unsigned char*)base_addr + g_ipf_ctx.reset_peri_crg[2]);

    do{
		regvalue = readl((unsigned char*)base_addr + g_ipf_ctx.reset_peri_crg[3]);
		regvalue &= g_ipf_ctx.reset_peri_crg[4];
		cnt--;
	}while((0x0 != regvalue) && cnt);

    if(0==cnt)
    {
        g_ipf_ctx.stax.timeout_stamp = bsp_get_slice_value();
    }
    return 0;
}

void ipf_om_dump_init(void)
{
	g_ipf_ctx.dump_area= bsp_dump_register_field(DUMP_MODEMAP_IPF, "IPF_ACPU", 0, 0, IPF_DUMP_SIZE, 0);
	
	if (!g_ipf_ctx.dump_area) {
		bsp_err("ipf backup mem for dump alloc failed\n");
		return;
	} 

    /* reg the dump callback to om */
	if (-1 == bsp_dump_register_hook("IPF_ACPU", g_ipf_ctx.desc->dump)) {
	    bsp_err("ipf bsp_dump_register_hook failed\n");
	    return;
	}
}

static int ipf_probe(struct platform_device *pdev)
{
	struct resource	*regs;
	int ret;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs)
		return -ENXIO;

    memset_s(&g_ipf_ctx, sizeof(g_ipf_ctx), 0, sizeof(ipf_ctx_t));
	g_ipf_ctx.irq = platform_get_irq(pdev, 0);
	if (unlikely(g_ipf_ctx.irq == 0))
		return -ENXIO;

	g_ipf_ctx.regs = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(g_ipf_ctx.regs))
		return PTR_ERR(g_ipf_ctx.regs);
	

	g_ipf_ctx.dev = &pdev->dev;
	g_ipf_ctx.dma_mask = 0xffffffffULL;
	g_ipf_ctx.dev->dma_mask = &g_ipf_ctx.dma_mask;
	spin_lock_init(&g_ipf_ctx.filter_spinlock);

	g_ipf_ctx.clk = devm_clk_get(g_ipf_ctx.dev, "ipf_clk");
	if (IS_ERR(g_ipf_ctx.clk)) {
		bsp_err("ipf clock not available\n");
		return -ENXIO;
	} else {
		ret = clk_prepare_enable(g_ipf_ctx.clk);
		if (ret) {
			bsp_err("failed to enable ipf clock\n");
			return ret;
		}
	}

    (void)of_property_read_u32_array(pdev->dev.of_node, "rst_crg", g_ipf_ctx.reset_peri_crg, 5);
    g_ipf_ctx.peribase = (void *)ioremap_nocache(g_ipf_ctx.reset_peri_crg[0], PERI_CRG_4K);
    if(!g_ipf_ctx.peribase)
    {
        bsp_err("ipf peribase ioremap_nocache failed\n");
    }
	g_ipf_ctx.limit_addr = (IPF_LIMIT_ADDR_S *)sm->trans_limit;
    g_ipf_ctx.memblock_show = (unsigned long *)sm->memlock;
	memset_s((void*)sm->trans_limit, sizeof(sm->trans_limit), 0x0, IPF_TRANS_LIMIT_SIZE);
    memset_s((void*)sm->memlock, sizeof(sm->memlock), 0x0, IPF_ADDR_MEMBLOCK_SIZE);

	if(ipf_get_limit_addr()){
		g_ipf_ctx.not_get_space++;
		bsp_err("ipf addr limit func disable\n");
	}

	ipf_init();

	g_ipf_ctx.pm = &ipf_dev_pm_ops;
	sm->init.status.acore = IPF_PWR_UP;
    sm->init.status.save = 0;

    ipf_om_dump_init();
    
	return 0;
}
static int ipf_remove(struct platform_device *pdev)
{
    return IPF_SUCCESS;
}

static const struct of_device_id ipf_match[] = {
	{ .compatible = "hisilicon,ipf_balong_app" },
	{},
};

static struct platform_driver ipf_pltfm_driver = {
	.probe		= ipf_probe,
	.remove		= ipf_remove,
	.driver		= {
		.name		= "ipf",
		.of_match_table	= ipf_match,
		.pm = &ipf_dev_pm_ops,
	},
};

__init int ipf_pltfm_driver_init(void)
{
    return platform_driver_register(&ipf_pltfm_driver);
}
module_init(ipf_pltfm_driver_init);
EXPORT_SYMBOL(bsp_ipf_reset_ccore_cb);
EXPORT_SYMBOL(g_ipf_ctx);
EXPORT_SYMBOL(mdrv_ipf_config_dlad);
EXPORT_SYMBOL(mdrv_ipf_get_used_dlad);
EXPORT_SYMBOL(bsp_ipf_config_timeout);
EXPORT_SYMBOL(bsp_ipf_set_control_flag_for_ccore_reset);
EXPORT_SYMBOL(mdrv_ipf_reinit_dlreg);
EXPORT_SYMBOL(mdrv_ipf_get_uldesc_num);
EXPORT_SYMBOL(mdrv_ipf_config_ulbd);
EXPORT_SYMBOL(mdrv_ipf_get_dlad_num);
EXPORT_SYMBOL(mdrv_ipf_get_ulbd_num);
EXPORT_SYMBOL(bsp_ipf_srest);
EXPORT_SYMBOL(ipf_init);
EXPORT_SYMBOL(mdrv_ipf_get_dlrd);
EXPORT_SYMBOL(mdrv_ipf_get_dlrd_num);
EXPORT_SYMBOL(mdrv_ipf_get_ulrd_num);
EXPORT_SYMBOL(mdrv_ipf_register_ops);
EXPORT_SYMBOL(ipf_register_wakeup_dlcb);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:ipf");


/*lint -restore*/

