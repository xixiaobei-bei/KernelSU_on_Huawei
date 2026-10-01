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
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <bsp_vic.h>
#include <securec.h>
#include "psam_balong.h"
#include "n_bsp_psam.h"
#include "bsp_trace.h"
#include "n_bsp_ipf.h"

#define PSAM_ADQ_RESERVE    3
#define PSAM_VIC_LEVEL		6
struct psam_device *g_psam_device;
struct psam_device g_psam_ctx;
static unsigned long long d_psam_mask;
static void * p_ctrl_base_addr = NULL;
static unsigned int idle_p_ctrl[3] = {0,};

static const struct of_device_id psam_match[] = {
	{ .compatible = "hisilicon,psam_balong_app" },
	{},
};

#define PSAM_IS_32BIT	(g_psam_device->psam_ver < PSAM_VERSION_160)

extern struct psam_hal_handle psam32_hal;
extern struct psam_hal_handle psam64_hal;


struct psam_hal_handle* psam_get_hal(unsigned int version_id)
{
	if(version_id < PSAM_VERSION_160)
		return &psam32_hal;
	else
		return &psam64_hal;
}



MODULE_DEVICE_TABLE(of, psam_match);


int bsp_psam_idle(void)
{
	HI_PSAM_SRST_T srst;

	srst.u32 = psam_readl(HI_PSAM_SRST_OFFSET);
	
	return srst.bits.psam_idle;
}

int bsp_psam_get_used_dlad(IPF_AD_TYPE_E ad_type, unsigned int * ad_num, IPF_AD_DESC_S * pst_ad_desc)
{
    unsigned int time_out = 10;
    unsigned int free_ad_num = 0;
    unsigned int adq_wptr;
    unsigned int adq_rptr;
    if((NULL == ad_num)||(NULL == pst_ad_desc)){
        bsp_err("\r input para ERROR!NULL == ad_num or NULL == pst_ad_desc\n");
        return BSP_ERR_IPF_INVALID_PARA;
    }

    if(bsp_get_init_status() == PSAM_ACORE_INIT_SUC){
        bsp_set_init_status(0);
    }
    
    do{
        if(bsp_psam_idle()){
            break;
        }
        msleep(20);
    }while(--time_out);
    if (!time_out){
        bsp_err("\r After 20ms psam dl channel still on, unable to free AD \n");
        return IPF_ERROR;
    }

	if(IPF_AD_0 == ad_type){
	    adq_wptr = readl(g_psam_device->regs + HI_PSAM_ADQ0_WPTR_OFFSET);
		adq_rptr = readl(g_psam_device->regs + HI_PSAM_ADQ0_RPTR_OFFSET);
	    while(adq_rptr != adq_wptr){
			g_psam_device->desc_hd->ad_h2s(pst_ad_desc, g_psam_device->desc[ad_type], adq_rptr);
	        adq_rptr = ((adq_rptr + 1) < IPF_DLAD0_DESC_SIZE)? (adq_rptr + 1) : 0;
	        pst_ad_desc++;
	        free_ad_num++;
	    }
	}
	else if(IPF_AD_1 == ad_type){
	    adq_wptr = readl(g_psam_device->regs + HI_PSAM_ADQ1_WPTR_OFFSET);
		adq_rptr = readl(g_psam_device->regs + HI_PSAM_ADQ1_RPTR_OFFSET);
	    while(adq_rptr != adq_wptr){
	        g_psam_device->desc_hd->ad_h2s(pst_ad_desc, g_psam_device->desc[ad_type], adq_rptr);
	        adq_rptr = ((adq_rptr + 1) < IPF_DLAD1_DESC_SIZE)? (adq_rptr + 1) : 0;
	        pst_ad_desc++;
	        free_ad_num++;
	    }
	}
	else{
	    bsp_err("\r para ERROR! u32AdType >= IPF_AD_MAX\n");
	    return BSP_ERR_IPF_INVALID_PARA;
	}
	
	g_psam_device->debug.adq_free_cnt += free_ad_num;
    *ad_num = free_ad_num;
    return 0;
}
int psam_srset(void)
{
    HI_PSAM_SRST_T psam_srst;
	unsigned int count = 10000;
	
    psam_srst.u32 = readl(g_psam_device->regs + HI_PSAM_SRST_OFFSET);
    psam_srst.bits.psam_srst = 1;
	writel(psam_srst.u32, g_psam_device->regs + HI_PSAM_SRST_OFFSET);
	
	while(psam_srst.bits.psam_srst){
		psam_srst.u32 = readl(g_psam_device->regs + HI_PSAM_SRST_OFFSET);
		if(count == 0){
			return -EINVAL;
		}
		count--;
	}
	return 0;
}

int psam_reinit_regs(void)
{
    HI_PSAM_INT0_MASK_T mask;
    HI_PSAM_ADQ_CTRL_T ctrl;

	if(g_psam_device ==  NULL)
		return -EINVAL;

	g_psam_device->hal->set_ad_base();
	mask.u32 = 0;
    mask.bits.adq0_epty_mask0 = 1;
    mask.bits.adq1_epty_mask0 = 1;
	writel(mask.u32, g_psam_device->regs + HI_PSAM_INT0_MASK_OFFSET);
	writel(0xffffffff, g_psam_device->regs + HI_PSAM_INT0_STAT_OFFSET);

	ctrl.u32 = 0;
    ctrl.bits.adq_en = 0x3;
    ctrl.bits.adq0_size_sel = IPF_DL_ADQ_LEN_EXP;
    ctrl.bits.adq1_size_sel = IPF_DL_ADQ_LEN_EXP;
    ctrl.bits.adq_plen_th = 404;
	writel(ctrl.u32, g_psam_device->regs + HI_PSAM_ADQ_CTRL_OFFSET);

	(void)mask;
	(void)ctrl;
	return 0;
}


int psam_register_adq_empty_dlcb(adq_emtpy_cb_t adq_emtpy_cb)
{
    g_psam_device->adq_emtpy_cb = adq_emtpy_cb;
	return 0;
}

int psam_get_dlad_num(unsigned int* ad0_num, unsigned int* ad1_num)
{
    unsigned int wptr = 0;
    unsigned int rptr = 0;
    unsigned int*  ad[2];
    unsigned int tmp;
    int i;

    if(!g_psam_device->debug.init_ok) {
        return -EINVAL;
    }

    if(!ad0_num || !ad1_num) {
        return -EINVAL;
    }
    
    ad[0] = ad0_num;
    ad[1] = ad1_num;
    
    for (i = 0; i < 2; i++) {
        wptr = readl(g_psam_device->regs + HI_PSAM_ADQ0_WPTR_OFFSET+0x10*(i));
        rptr = readl(g_psam_device->regs + HI_PSAM_ADQ0_RPTR_OFFSET+0x10*(i));

        tmp = (g_psam_device->desc_num[i] + rptr -(wptr + 1)) % g_psam_device->desc_num[i];
        *ad[i] = (tmp>PSAM_ADQ_RESERVE)?(tmp-PSAM_ADQ_RESERVE):0;
    }
    return 0;
}


int psam_config_dlad(unsigned int type, unsigned int num, IPF_AD_DESC_S *desc)
{
	if(bsp_get_init_status()!= PSAM_ACORE_INIT_SUC){
		return -EINVAL;
	}

	if(!desc) {
		g_psam_device->debug.dlad_desc_null[type]++;
		return -EINVAL;
	}

	if(!g_psam_device->debug.init_ok) {
		return -EINVAL;
	}

	if(num > g_psam_device->desc_num[type])	{
		g_psam_device->debug.cfg_dlad_num_err++;
		return -EINVAL;
	}

	g_psam_device->hal->config_ad((IPF_AD_TYPE_E)type, num, desc);
    g_psam_device->ads_up = 1;
	return 0;
}

static irqreturn_t psam_interrupt(int irq, void *dev_id)
{
	HI_PSAM_INT0_STAT_T int_stat;
	(void)irq;
	(void)dev_id;
	int_stat.u32 = readl(g_psam_device->regs + HI_PSAM_INT_STAT_OFFSET(0));
	writel(int_stat.u32, g_psam_device->regs + HI_PSAM_INT_STAT_OFFSET(0));

	if(int_stat.bits.adq0_epty_int0){
		g_psam_device->debug.adq0_epty_int1++;
		if (g_psam_device->adq_emtpy_cb)
			g_psam_device->adq_emtpy_cb(IPF_EMPTY_ADQ0);
	}

	if(int_stat.bits.adq1_epty_int0){
		g_psam_device->debug.adq1_epty_int1++;
		if (g_psam_device->adq_emtpy_cb)
			g_psam_device->adq_emtpy_cb(IPF_EMPTY_ADQ1);
	}

	if(int_stat.bits.lbdq_epty_int0){
		g_psam_device->debug.lbdq_empty++;
	}

//	if(int_stat.bits.adq_p1_err_int0){
//		g_psam_device->debug.adq_p1_err++;
//	}

	if(int_stat.bits.rd0_wbuf_overflow_int0){
		g_psam_device->debug.rd0_wbuf_overflow++;
	}

	if(int_stat.bits.rd1_wbuf_overflow_int0){
		g_psam_device->debug.rd1_wbuf_overflow++;
	}

	if(int_stat.bits.rd2_wbuf_overflow_int0){
		g_psam_device->debug.rd2_wbuf_overflow++;
	}

	if(int_stat.bits.adq0_upoverflow_int0){
		g_psam_device->debug.adq0_upoverflow++;
	}

	if(int_stat.bits.adq1_upoverflow_int0){
		g_psam_device->debug.adq1_upoverflow++;
	}

    if(int_stat.bits.lbdq_upoverflow_int0){
            g_psam_device->debug.lbdq_upoverflow++;
    }

    if(int_stat.bits.rd0_wbuf_full_int0){
		g_psam_device->debug.rd0_wbuf_full++;
	}

	if(int_stat.bits.rd1_wbuf_full_int0){
		g_psam_device->debug.rd1_wbuf_full++;
	}

    if(int_stat.bits.rd2_wbuf_full_int0){
            g_psam_device->debug.rd2_wbuf_full++;
    }

    if(int_stat.bits.adq0_full_int0){
		g_psam_device->debug.adq0_full++;
	}

	if(int_stat.bits.adq1_full_int0){
		g_psam_device->debug.adq1_full++;
	}

    if(int_stat.bits.lbdq_full_int0){
            g_psam_device->debug.lbdq_full++;
    }

	return IRQ_HANDLED;
}


int psam_is_idle(void)
{
	HI_PSAM_SRST_T srst;
	HI_PSAM_ADQ0_STAT_T adq0_stat;
	HI_PSAM_ADQ1_STAT_T adq1_stat;
	
	srst.u32 = psam_readl(HI_PSAM_SRST_OFFSET);
	if(!srst.bits.psam_idle){
		g_psam_device->debug.not_idle++;
		return 0;
	}

    if(!g_psam_device->ads_up){
        return 1;
    }

	adq0_stat.u32 = psam_readl(HI_PSAM_ADQ0_STAT_OFFSET);
	if(adq0_stat.bits.adq0_epty){
		g_psam_device->debug.adq0_empty++;
		return 0;
	}

	adq1_stat.u32 = psam_readl(HI_PSAM_ADQ1_STAT_OFFSET);
	if(adq1_stat.bits.adq1_epty){
		g_psam_device->debug.adq1_empty++;
		return 0;
	}

	return 1;
}

void psam_save_reg(unsigned int *base)
{    
    g_psam_device->hal->save_reg(base);
}

void psam_restore_reg(unsigned int *base)
{
	g_psam_device->hal->restore_reg(base);
    return;
}

struct psam_pm_cb psam_pm = {
    .is_idle = psam_is_idle,
    .save_reg = psam_save_reg,
    .restore_reg = psam_restore_reg,
};

struct ipf_desc_handler_s* tmp_desc_hd = 0;

struct psam_pm_cb* bsp_psam_set_ipf_para(struct tagpsam_ipf_reg *para)
{
	if(para && para->desc_hd)
	{
		if(g_psam_device)
			g_psam_device->desc_hd = para->desc_hd;
		else
	    	tmp_desc_hd = para->desc_hd;
	}
	else
	{
		bsp_err("get cipher para is NULL!\n");
		return NULL;
	}

	return &psam_pm;
}

static unsigned int psam_get_version(void)
{
	unsigned int ver = 0;
	ver = psam_readl(HI_PSAM_VERSION_OFFSET);
	switch(ver){
	case PSAM_VER_100A:
		g_psam_device->psam_ver = PSAM_VERSION_100a;
		break;
	case PSAM_VER_110A:
		g_psam_device->psam_ver = PSAM_VERSION_110a;
		break;
	case PSAM_VER_120A:
		g_psam_device->psam_ver = PSAM_VERSION_120a;
		break;
	case PSAM_VER_150A:
		g_psam_device->psam_ver = PSAM_VERSION_150a;
		break;
	case PSAM_VER_160:
		g_psam_device->psam_ver = PSAM_VERSION_160;
		break;
	default:
		g_psam_device->psam_ver = PSAM_VERSION_0000;
		break;
	}
    return g_psam_device->psam_ver;
}

static int psam_probe(struct platform_device *pdev)
{
    struct resource *regs;
    int i;
    int ret;
    u64 mask = (u64)-1;

    memset_s(&g_psam_ctx, sizeof(g_psam_ctx), 0, sizeof(struct psam_device));
    g_psam_device = &g_psam_ctx;

    regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!regs)
        return -ENXIO;

    g_psam_device->irq = platform_get_irq(pdev, 0);
    if (g_psam_device->irq < 0)
        return g_psam_device->irq;

    g_psam_device->irq_flags = IRQF_NO_SUSPEND;
    g_psam_device->dev = &pdev->dev;

    d_psam_mask = (g_psam_device->psam_ver<PSAM_VERSION_160)?0xffffffffULL:0xffffffffffffffff;
    g_psam_device->dev->dma_mask = &d_psam_mask;
    g_psam_device->regs = devm_ioremap_resource(&pdev->dev, regs);
    if (IS_ERR(g_psam_device->regs))
        return PTR_ERR(g_psam_device->regs);
    g_psam_device->clk = devm_clk_get(g_psam_device->dev, "psam_aclk");
    if (IS_ERR(g_psam_device->clk)) {
        bsp_err("psam clock not available\n");
        return -EPERM;
    } else {
        ret = clk_prepare_enable(g_psam_device->clk);
        if (ret) {
            bsp_err("failed to enable psam clock\n");
            return ret;
        }
    }

    psam_get_version();
    g_psam_device->hal = psam_get_hal(g_psam_device->psam_ver);
    g_psam_device->desc_num[0] = PSAM_DLAD0_DESC_NUM;
    g_psam_device->desc_num[1] = PSAM_DLAD1_DESC_NUM;
    g_psam_device->pm = &psam_pm;

    dma_set_mask_and_coherent(g_psam_device->dev, mask);

    for (i = 0; i < PSAM_ADQ_NUM; i++) {
        g_psam_device->desc[i] = dmam_alloc_coherent(g_psam_device->dev,
            g_psam_device->desc_num[i] * sizeof(ipf64_ad_s), &g_psam_device->dma[i], GFP_KERNEL);
        if (g_psam_device->desc[i] == NULL)
            return -ENOMEM;
    }
    g_psam_device->hal->set_ad_base();
    g_psam_device->adq0_info.vir_addr = (unsigned int)(unsigned long)(g_psam_device->desc[0]);
    g_psam_device->adq1_info.vir_addr = (unsigned int)(unsigned long)(g_psam_device->desc[1]);

    ret = devm_request_irq(g_psam_device->dev, g_psam_device->irq, psam_interrupt,
                   g_psam_device->irq_flags, "psam", g_psam_device);
    if(ret){
        bsp_err("failed to request psam irq\n");
        return ret;
    }

    platform_set_drvdata(pdev, g_psam_device);

    bsp_ipf_set_debug_para(&(g_psam_device->ipf_deb));
    /*only dallas platform need to juduge idle by p_ctrl.*/
    (void)of_property_read_u32_array(pdev->dev.of_node, "idle_ctrl", idle_p_ctrl, 3);
    if(idle_p_ctrl[0]){
        p_ctrl_base_addr=(void *)ioremap_nocache(idle_p_ctrl[0], P_CTRL_4K);
    }

    if(tmp_desc_hd){
        g_psam_device->desc_hd = tmp_desc_hd;
    }
    
    g_psam_device->debug.init_ok = 1;
    bsp_err("psam init ok!\n");

    return 0;
}

static int psam_remove(struct platform_device *pdev)
{
	struct psam_device *psam_dev = NULL;

	psam_dev = platform_get_drvdata(pdev);
	if (!psam_dev)
		return -ENOMEM;

	platform_set_drvdata(pdev, NULL);

	clk_disable_unprepare(psam_dev->clk);

	return 0;
}

static struct platform_driver psam_pltfm_driver = {
	.probe		= psam_probe,
	.remove		= psam_remove,
	.driver		= {
		.name		= "psam",
		.of_match_table	= psam_match,
	},
};

__init int psam_pltfm_driver_init(void)
{
    return platform_driver_register(&psam_pltfm_driver);
}
module_init(psam_pltfm_driver_init);
EXPORT_SYMBOL(g_psam_device);
EXPORT_SYMBOL(psam_srset);
EXPORT_SYMBOL(psam_reinit_regs);
EXPORT_SYMBOL(psam_register_adq_empty_dlcb);
MODULE_DESCRIPTION("Balong Specific Psam Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:psam");
