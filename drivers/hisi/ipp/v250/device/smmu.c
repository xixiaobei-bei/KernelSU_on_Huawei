/*
 * Hisilicon IPP Common Driver
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/hisi/hipp.h>
#include "smmu_drv.h"
#include "smmu_reg_offset.h"
#include "ipp.h"

static int hipp_smmu_init_mask_clr(struct smmu_nontrus_type_r *r, void __iomem *addr, unsigned int *offset)
{
	/* SMMU_RLD_EN2_NS */
	*offset = SMMU_COMN_SMMU_RLD_EN2_NS_REG;
	r->smmu_rld_en2_ns.u32 = readl((volatile void __iomem *)(addr + *offset));
	r->smmu_rld_en2_ns.bits.smr_rld_en2 = 0x1;
	writel(r->smmu_rld_en2_ns.u32, (volatile void __iomem *)(addr + *offset));

	/* SMMU_INTMASK_NS */
	*offset = SMMU_COMN_SMMU_INTMASK_NS_REG;
	r->smmu_intmask_ns.u32 = readl((volatile void __iomem *)(addr + *offset));
	r->smmu_intmask_ns.bits.intns_ptw_ns_msk         = 1;
	r->smmu_intmask_ns.bits.intns_ptw_invalid_msk    = 1;
	r->smmu_intmask_ns.bits.intns_ptw_trans_msk      = 1;
	r->smmu_intmask_ns.bits.intns_tlbmiss_msk        = 1;
	r->smmu_intmask_ns.bits.intns_ext_msk            = 1;
	r->smmu_intmask_ns.bits.intns_permis_msk         = 1;
	writel(r->smmu_intmask_ns.u32, (volatile void __iomem *)(addr + *offset));
	/* SMMU_INTCLR_NS */
	*offset = SMMU_COMN_SMMU_INTCLR_NS_REG;
	r->smmu_intclr_ns.u32 = readl((volatile void __iomem *)(addr + *offset));
	r->smmu_intclr_ns.bits.intns_ptw_ns_clr          = 1;
	r->smmu_intclr_ns.bits.intns_ptw_invalid_clr     = 1;
	r->smmu_intclr_ns.bits.intns_ptw_trans_clr       = 1;
	r->smmu_intclr_ns.bits.intns_tlbmiss_clr         = 1;
	r->smmu_intclr_ns.bits.intns_ext_clr             = 1;
	r->smmu_intclr_ns.bits.intns_permis_clr          = 1;
	writel(r->smmu_intclr_ns.u32, (volatile void __iomem *)(addr + *offset));
	return 0;
}

int hipp_smmu_nontrus_init(void __iomem *smmuaddr, unsigned long long pteaddr)
{
	struct smmu_nontrus_type_r r;
	void __iomem *addr = NULL;
	unsigned int offset = 0;
	int i = 0;
	if (smmuaddr == NULL || pteaddr == 0) {
		e("[%s] Failed : smmuaddr.%pK, pteaddr.0x%llx\n", __func__, smmuaddr, pteaddr);
		return -ENOMEM;
	}

	addr = smmuaddr;
	/* SMMU_SCR */
	offset = SMMU_COMN_SMMU_SCR_REG;
	r.smmu_scr.u32 = readl((volatile void __iomem *)(addr + offset));
	r.smmu_scr.bits.glb_bypass = 0;
	writel(r.smmu_scr.u32, (volatile void __iomem *)(addr + offset));
	/* SMMU_SMRX_NS */
	offset = SMMU_COMN_SMMU_SMRX_NS_0_REG;

	for (i = 0; i < SMRX_NUM; i++) {
		r.smmu_smrx_ns[i].u32 = readl((volatile void __iomem *)(addr + offset + i * 4));
		r.smmu_smrx_ns[i].bits.smr_bypass = 0;
		writel(r.smmu_smrx_ns[i].u32, (volatile void __iomem *)(addr + offset + i * 4));
	}

	/* SMMU_CB_TTBR0 */
	offset = SMMU_COMN_SMMU_CB_TTBR0_REG;
	r.smmu_cb_ttbr0.u32 = readl((volatile void __iomem *)(addr + offset));
	r.smmu_cb_ttbr0.bits.cb_ttbr0 = pteaddr;
	writel(r.smmu_cb_ttbr0.u32, (volatile void __iomem *)(addr + offset));
	/* SMMU_CB_TTBCR */
	offset = SMMU_COMN_SMMU_CB_TTBCR_REG;
	r.smmu_cb_ttbcr.u32 = readl((volatile void __iomem *)(addr + offset));
	r.smmu_cb_ttbcr.bits.cb_ttbcr_des = 0x1;
	writel(r.smmu_cb_ttbcr.u32, (volatile void __iomem *)(addr + offset));
	/* SMMU_FAMA_CTRL0_NS */
	offset = SMMU_COMN_SMMU_FAMA_CTRL0_NS_REG;
	r.smmu_fama_ctrl0_ns.u32 = readl((volatile void __iomem *)(addr + offset));
	r.smmu_fama_ctrl0_ns.bits.fama_chn_sel_ns = 0x1;
	writel(r.smmu_fama_ctrl0_ns.u32, (volatile void __iomem *)(addr + offset));
	/* SMMU_FAMA_CTRL1_NS */
	offset = SMMU_COMN_SMMU_FAMA_CTRL1_NS_REG;
	r.smmu_fama_ctrl1_ns.u32 = readl((volatile void __iomem *)(addr + offset));
	r.smmu_fama_ctrl1_ns.bits.fama_ptw_msb_ns = (pteaddr >> 32) & 0x7F;
	writel(r.smmu_fama_ctrl1_ns.u32, (volatile void __iomem *)(addr + offset));
	/* SMMU_RLD_EN0_NS */
	offset = SMMU_COMN_SMMU_RLD_EN0_NS_REG;
	r.smmu_rld_en0_ns.u32 = readl((volatile void __iomem *)(addr + offset));
	r.smmu_rld_en0_ns.bits.smr_rld_en0 = 0x1;
	writel(r.smmu_rld_en0_ns.u32, (volatile void __iomem *)(addr + offset));
	/* SMMU_RLD_EN1_NS */
	offset = SMMU_COMN_SMMU_RLD_EN1_NS_REG;
	r.smmu_rld_en1_ns.u32 = readl((volatile void __iomem *)(addr + offset));
	r.smmu_rld_en1_ns.bits.smr_rld_en1 = 0x1;
	writel(r.smmu_rld_en1_ns.u32, (volatile void __iomem *)(addr + offset));

	hipp_smmu_init_mask_clr(&r, addr, &offset);
	return 0;
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hisilicon IPP SMMU Driver");
MODULE_AUTHOR("isp");
