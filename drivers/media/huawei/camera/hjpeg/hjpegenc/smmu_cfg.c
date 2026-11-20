/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: provide interface for config smmu.
 * Author: lixiuhua
 * Create: 2012-12-22
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/iommu.h>
#include <asm/io.h>

#include "smmu_cfg.h"
#include "hjpeg_common.h"
#include "cam_log.h"

static void do_cfg_smmu_global_bypass(void __iomem *smmu_base_addr);
static void do_cfg_smmu_smrx_ns_offset(hjpeg_hw_ctl_t *hw_ctl,
	void __iomem *smmu_base_addr);
static void do_cfg_smmu_fama_bps_msb_ns(void *pgd,
	void __iomem *smmu_base_addr);
static void do_cfg_smmu_intclr_ns(void __iomem *smmu_base_addr);
static void do_cfg_smmu_intmask_ns(void __iomem *smmu_base_addr);
static void do_cfg_smmu_rwerr_addr(hjpeg_hw_ctl_t *hw_ctl,
	void __iomem *smmu_base_addr);
static void do_cfg_smmu_cb_ttbr0(void *pgd, void __iomem *smmu_base_addr);
static void do_cfg_smmu_cb_ttbcr(void __iomem *smmu_base_addr);
static void do_cfg_smmu_fama_ctrl1_ns(void *pgd, void __iomem *smmu_base_addr);
static void do_cfg_smmu_fama_ctrl0_ns(void __iomem *smmu_base_addr);

static int do_cfg_isp_smmu(hjpeg_hw_ctl_t *hw_ctl)
{
	u32 smmu_smrx_ns_offset;
	bool bypass = false;
	void __iomem *smmu_base_addr = hw_ctl->smmu_viraddr;

	cam_debug("%s enter", __FUNCTION__);

	if (IS_ERR_OR_NULL(smmu_base_addr)) {
		cam_err("%s %d base addr error", __FUNCTION__, __LINE__);
		return -1;
	}

	bypass = (hw_ctl->smmu_bypass == BYPASS_YES);
	if (bypass) {
		/* disable SMMU only for JPGENC stream id */
		smmu_smrx_ns_offset = SMMU_SMRX_NS(hw_ctl->stream_id[0]);
		set_reg_val((void __iomem*)((char*)smmu_base_addr +
			smmu_smrx_ns_offset),
			get_reg_val((void __iomem*)((char*)smmu_base_addr +
			smmu_smrx_ns_offset)) | 0x1);

		smmu_smrx_ns_offset = SMMU_SMRX_NS(hw_ctl->stream_id[1]);
		set_reg_val((void __iomem*)((char*)smmu_base_addr +
			smmu_smrx_ns_offset),
			get_reg_val((void __iomem*)((char*)smmu_base_addr +
			smmu_smrx_ns_offset)) | 0x1);

		/* 2: array para */
		smmu_smrx_ns_offset = SMMU_SMRX_NS(hw_ctl->stream_id[2]);
		set_reg_val((void __iomem*)((char*)smmu_base_addr +
			smmu_smrx_ns_offset),
			get_reg_val((void __iomem*)((char*)smmu_base_addr +
			smmu_smrx_ns_offset)) | 0x1);
	}
	return 0;
}

static int do_cfg_smmu(hjpeg_hw_ctl_t *hw_ctl, void *pgd)
{
	bool bypass = false;
	void __iomem *smmu_base_addr = hw_ctl->smmu_viraddr;

	cam_info("%s enter", __FUNCTION__);

	if (IS_ERR_OR_NULL(smmu_base_addr)) {
		cam_err("%s %d base addr error", __FUNCTION__, __LINE__);
		return -1;
	}
	/* enable SMMU for global */
	do_cfg_smmu_global_bypass(smmu_base_addr);

	bypass = hw_ctl->smmu_bypass == BYPASS_YES;
	if (bypass) {
		/* disable SMMU only for JPGENC stream id */
		do_cfg_smmu_smrx_ns_offset(hw_ctl, smmu_base_addr);
		do_cfg_smmu_fama_bps_msb_ns(pgd, smmu_base_addr);
		return 0;
	}

	/* config smmu */
	do_cfg_smmu_intclr_ns(smmu_base_addr);
	do_cfg_smmu_intmask_ns(smmu_base_addr);

	/* RWERRADDR */
	do_cfg_smmu_rwerr_addr(hw_ctl, smmu_base_addr);

	/* SMMU Context Config */
	do_cfg_smmu_cb_ttbr0(pgd, smmu_base_addr);
	do_cfg_smmu_cb_ttbcr(smmu_base_addr);

	/* FAMA configuration */
	do_cfg_smmu_fama_ctrl1_ns(pgd, smmu_base_addr);
	do_cfg_smmu_fama_ctrl0_ns(smmu_base_addr);

	return 0;
}

static void do_cfg_smmu_global_bypass(void __iomem *smmu_base_addr)
{
	u_smmu_smmu_scr smmu_scr;

	smmu_scr.reg32 = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_GLOBAL_BYPASS));

	smmu_scr.bits.glb_bypass = 0x0;

	if (is_hjpeg_qos_update() != 0)
		smmu_scr.bits.ptw_pf = 0x5;
	else
		smmu_scr.bits.ptw_pf = 0x1;

	smmu_scr.bits.ptw_mid = 0x1d;
	cam_notice("%s: set reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_GLOBAL_BYPASS, smmu_scr.reg32);
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_GLOBAL_BYPASS), smmu_scr.reg32);
}

static void do_cfg_smmu_smrx_ns_offset(hjpeg_hw_ctl_t *hw_ctl,
	void __iomem *smmu_base_addr)
{
	u32 smmu_smrx_ns_offset;

	smmu_smrx_ns_offset = SMMU_SMRX_NS(hw_ctl->stream_id[0]);
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		smmu_smrx_ns_offset),
		get_reg_val((void __iomem*)((char*)smmu_base_addr +
		smmu_smrx_ns_offset)) | 0x1);

	smmu_smrx_ns_offset = SMMU_SMRX_NS(hw_ctl->stream_id[1]);
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		smmu_smrx_ns_offset),
		get_reg_val((void __iomem*)((char*)smmu_base_addr +
		smmu_smrx_ns_offset)) | 0x1);

	/* 2: array para */
	smmu_smrx_ns_offset = SMMU_SMRX_NS(hw_ctl->stream_id[2]);
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		smmu_smrx_ns_offset),
		get_reg_val((void __iomem*)((char*)smmu_base_addr +
		smmu_smrx_ns_offset)) | 0x1);
}

static void do_cfg_smmu_fama_bps_msb_ns(void *pgd, void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	phy_pgd_t *phy_pgd = (phy_pgd_t *)pgd;

	tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_FAMA_CTRL1_NS));
	cam_notice("%s: SMMU_FAMA_CTRL1_NS = 0x%x, phy_pgd_fama_bps_msb_ns = 0x%x",
		__FUNCTION__, tmpVal, phy_pgd->phy_pgd_fama_bps_msb_ns);
	REG_SET_FIELD(tmpVal, FAMA_CTRL0_NS_fama_bps_msb_ns,
		phy_pgd->phy_pgd_fama_bps_msb_ns);
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_FAMA_CTRL1_NS), tmpVal);
	cam_notice("%s: set reg SMMU_FAMA_CTRL1_NS = 0x%x",
		__FUNCTION__, tmpVal);
}

static void do_cfg_smmu_intclr_ns(void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_INTCLR_NS));
	cam_notice("%s: get reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_INTCLR_NS, tmpVal);
	tmpVal = 0xFF;
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_INTCLR_NS), tmpVal);
	cam_notice("%s: set reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_INTCLR_NS, tmpVal);
}

static void do_cfg_smmu_intmask_ns(void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_INTMASK_NS));
	cam_notice("%s: get reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_INTMASK_NS, tmpVal);
	tmpVal = 0x0;
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_INTMASK_NS), tmpVal);
	cam_notice("%s: set reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_INTMASK_NS, tmpVal);
}

static void do_cfg_smmu_rwerr_addr(hjpeg_hw_ctl_t *hw_ctl,
	void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	uint64_t smmu_rwerraddr_phys;
	u_smmu_smmu_addr_msb smmu_addr_msb;

	if (hw_ctl->jpg_smmu_rwerraddr_virt) {
		smmu_rwerraddr_phys =
			virt_to_phys(hw_ctl->jpg_smmu_rwerraddr_virt);
		tmpVal = smmu_rwerraddr_phys & 0xFFFFFFFF;
		cam_debug("%s: set smmu err low addr (0x%x)",
			__FUNCTION__, tmpVal);
		set_reg_val((void __iomem*)((char*)smmu_base_addr +
			SMMU_ERR_RDADDR), tmpVal);
		set_reg_val((void __iomem*)((char*)smmu_base_addr +
			SMMU_ERR_WRADDR), tmpVal);
		smmu_addr_msb.reg32 = get_reg_val((void __iomem*)((char*)
			smmu_base_addr + SMMU_ADDR_MSB));
		tmpVal = (smmu_rwerraddr_phys >> 32) & 0x7F; /* 32: bit move */
		cam_debug("%s: set smmu err high addr (0x%x)",
			__FUNCTION__, tmpVal);
		smmu_addr_msb.bits.msb_errrd = tmpVal;
		smmu_addr_msb.bits.msb_errwr = tmpVal;
		set_reg_val((void __iomem*)((char*)smmu_base_addr +
			SMMU_ADDR_MSB), smmu_addr_msb.reg32);
	} else {
		cam_warn("%s: jpg_smmu_rwerraddr_virt is NULL", __FUNCTION__);
	}
}

static void do_cfg_smmu_cb_ttbr0(void *pgd, void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	phy_pgd_t *phy_pgd = (phy_pgd_t*)pgd;

	tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_CB_TTBR0));
	cam_notice("%s: get reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_CB_TTBR0, tmpVal);
	tmpVal = phy_pgd->phy_pgd_base;
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_CB_TTBR0), tmpVal);
	cam_notice("%s: set reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_CB_TTBR0, tmpVal);
}

static void do_cfg_smmu_cb_ttbcr(void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_CB_TTBCR));
	cam_notice("%s: get reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_CB_TTBCR, tmpVal);
	tmpVal |= 0x1;
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_CB_TTBCR), tmpVal);
	cam_notice("%s: set reg 0x%x = 0x%x",
		__FUNCTION__, SMMU_CB_TTBCR, tmpVal);
}

static void do_cfg_smmu_fama_ctrl1_ns(void *pgd, void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	phy_pgd_t *phy_pgd = (phy_pgd_t*)pgd;

	tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_FAMA_CTRL1_NS));
	cam_notice("%s: SMMU_FAMA_CTRL1_NS = 0x%x, phy_pgd_fama_ptw_msb = 0x%x",
		__FUNCTION__, tmpVal, phy_pgd->phy_pgd_fama_ptw_msb);
	REG_SET_FIELD(tmpVal, FAMA_CTRL1_NS_fama_ptw_msb_ns,
		phy_pgd->phy_pgd_fama_ptw_msb);
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_FAMA_CTRL1_NS), tmpVal);
	cam_notice("%s: set reg SMMU_FAMA_CTRL1_NS = 0x%x",
		__FUNCTION__, tmpVal);
}

static void do_cfg_smmu_fama_ctrl0_ns(void __iomem *smmu_base_addr)
{
	u32 tmpVal;
	tmpVal = get_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_FAMA_CTRL0_NS));
	cam_notice("%s: SMMU_FAMA_CTRL0_NS = 0x%x", __FUNCTION__, tmpVal);
	REG_SET_FIELD(tmpVal, FAMA_CTRL0_NS_fama_chn_sel_ns, 1);
	set_reg_val((void __iomem*)((char*)smmu_base_addr +
		SMMU_FAMA_CTRL0_NS), tmpVal);
	cam_notice("%s: set reg SMMU_FAMA_CTRL0_NS = 0x%x",
		__FUNCTION__, tmpVal);
}

static int do_cfg_smmuv3_opentbu(void __iomem *tbu_base_addr)
{
	u_smmuv3_smmu_tbu_cr tmp_tbu_cr;
	u_smmuv3_smmu_tbu_crack tmp_tbu_crack;
	struct timeval start_time, end_time;
	bool btimeout_flag = false;

	tmp_tbu_cr.reg32 = get_reg_val((void __iomem*)((char*)tbu_base_addr +
		SMMUV3_TBU_CR_OFFSET));
	tmp_tbu_cr.bits.max_tok_trans = IPP_MAX_TOK_TRANS_NUM - 1;
	tmp_tbu_cr.bits.tbu_en_req = 0x1; /* request tbu and tcu connection */
	set_reg_val((void __iomem*)((char*)tbu_base_addr +
		SMMUV3_TBU_CR_OFFSET), tmp_tbu_cr.reg32);

	/*
	 * wait tbu and tcu connect success,
	 * until tbu_en_ack = 0x1 and timeout < 1000us
	 */
	do_gettimeofday(&start_time);
	do {
		tmp_tbu_crack.reg32 = get_reg_val((void __iomem*)((char*)
			tbu_base_addr + SMMUV3_TBU_CRACK_OFFSET));
		do_gettimeofday(&end_time);
		btimeout_flag = (TBU_EN_ACK_TIMECOST(start_time, end_time) >
			TBU_EN_ACK_TIMEOUT);
	} while (!btimeout_flag && (tmp_tbu_crack.bits.tbu_en_ack != 0x1));

	cam_info("%s TBU Crack: 0x%08x, timoutFlag: %u",
		__FUNCTION__, tmp_tbu_crack.reg32, btimeout_flag);
	if (btimeout_flag && (tmp_tbu_crack.bits.tbu_en_ack != 0x1)) {
		cam_err("%s: tbu and tcu connect [timeout] failed",
			__FUNCTION__);
		return SMMUV3_FAIL;
	}

	if (tmp_tbu_crack.bits.tbu_connected != 0x1) {
		cam_err("%s: tbu and tcu connect failed", __FUNCTION__);
		return SMMUV3_FAIL;
	}

	if (tmp_tbu_crack.bits.tok_trans_gnt < tmp_tbu_cr.bits.max_tok_trans) {
		cam_err("%s: tok_grans_gnt[%u] < tbuCr.max_tok_trans[%u]",
			__FUNCTION__, tmp_tbu_crack.bits.tok_trans_gnt,
			tmp_tbu_cr.bits.max_tok_trans);
		return SMMUV3_FAIL;
	}

	return SMMUV3_SUCCESS;
}

static void do_cfg_smmuv3_tbu_swid(bool bypass, hjpeg_hw_ctl_t *hw_ctl)
{
	void __iomem *tbu_base_addr = NULL;
	u_smmuv3_smmu_tbu_swid_cfg tmp_tbu_swid_cfg;
	void __iomem *tbu_swidcfg_addr = NULL;
	int i;
	uint32_t swid = hw_ctl->secadapt_prop.start_swid;

	tbu_base_addr = hw_ctl->smmu_viraddr;
	if (!tbu_base_addr) {
		cam_err("%s base addr is null", __FUNCTION__);
		return;
	}

	/* open tbu prefetch */
	for (i = 0; i < hw_ctl->secadapt_prop.swid_num; i++) {
		tbu_swidcfg_addr = tbu_base_addr + (swid++) *
			SECADAPT_SWID_CFG_NS_PER_OFFSET +
			SMMUV3_SWID_CFG_OFFSET;
		tmp_tbu_swid_cfg.reg32 = get_reg_val(tbu_swidcfg_addr);
		tmp_tbu_swid_cfg.bits.pref_en = 0x1;
		set_reg_val(tbu_swidcfg_addr, tmp_tbu_swid_cfg.reg32);
	}
	cam_info("%s setval 0x%08x ok", __FUNCTION__, tmp_tbu_swid_cfg.reg32);
}

static void do_cfg_smmuv3_irpt_init(void __iomem *tbu_base_addr)
{
	/* clear all probe left non-secure interrupt */
	set_reg_val((void __iomem*)((char*)tbu_base_addr +
		SMMUV3_TBU_IRPT_CLR_NS_OFFSET), 0xFFFFFFFF);
	/* open all non-secure inpterrupt source */
	set_reg_val((void __iomem*)((char*)tbu_base_addr +
		SMMUV3_TBU_IRPT_MASK_NS_OFFSET), 0x0);
}

static int do_cfg_secadapt_sid(hjpeg_hw_ctl_t *hw_ctl)
{
	int i;
	void __iomem *swid_cfg_ns_addr = NULL;
	u_sec_adpt_swid_cfg_ns_union sec_adapt_swid_cfg_ns;
	uint32_t swid = hw_ctl->secadapt_prop.start_swid;
	void __iomem *sec_adapt_base_addr = hw_ctl->secadapt_viraddr;
	if (IS_ERR_OR_NULL(sec_adapt_base_addr)) {
		cam_err("%s %d base addr error", __FUNCTION__, __LINE__);
		return SMMUV3_FAIL;
	}

	for (i = 0; i < hw_ctl->secadapt_prop.swid_num; i++) {
		sec_adapt_swid_cfg_ns.value = 0x0;
		swid_cfg_ns_addr = sec_adapt_base_addr +
			SECADAPT_SWID_CFG_NS_OFFSET +
			(swid++) * SECADAPT_SWID_CFG_NS_PER_OFFSET;
		sec_adapt_swid_cfg_ns.value = get_reg_val(swid_cfg_ns_addr);

		if (hw_ctl->smmu_bypass == BYPASS_YES) {
			sec_adapt_swid_cfg_ns.reg.sid = SMMUBYPASS_SID;
			sec_adapt_swid_cfg_ns.reg.ssidv_ns =
				SMMUBYPASS_SSIDV_NS;
		} else {
			sec_adapt_swid_cfg_ns.reg.sid =
				hw_ctl->secadapt_prop.sid;
			sec_adapt_swid_cfg_ns.reg.ssidv_ns = 1; /* ssid valid */
			sec_adapt_swid_cfg_ns.reg.ssid_ns =
				hw_ctl->secadapt_prop.ssid_ns;
		}
		set_reg_val(swid_cfg_ns_addr, sec_adapt_swid_cfg_ns.value);
	}

	cam_info("%s config secadapt sid val 0x%08x end",
		__FUNCTION__, sec_adapt_swid_cfg_ns.value);
	return SMMUV3_SUCCESS;
}

/* config smmuv3 tbu */
static int do_cfg_smmuv3(hjpeg_hw_ctl_t *hw_ctl)
{
	int ret_code;
	void __iomem *smmuv3_tbu_base_addr = NULL;
	cam_info("%s enter", __FUNCTION__);

	smmuv3_tbu_base_addr = hw_ctl->smmu_viraddr;
	if (IS_ERR_OR_NULL(smmuv3_tbu_base_addr)) {
		cam_err("%s %d base addr error", __FUNCTION__, __LINE__);
		return SMMUV3_FAIL;
	}

	ret_code = do_cfg_secadapt_sid(hw_ctl);
	if (ret_code == SMMUV3_FAIL) {
		cam_err("%s %d open tbu error", __FUNCTION__, __LINE__);
		return SMMUV3_FAIL;
	}

	ret_code = do_cfg_smmuv3_opentbu(smmuv3_tbu_base_addr);
	if (ret_code == SMMUV3_FAIL) {
		cam_err("%s %d open tbu error", __FUNCTION__, __LINE__);
		return SMMUV3_FAIL;
	}

	do_cfg_smmuv3_tbu_swid((hw_ctl->smmu_bypass == BYPASS_YES), hw_ctl);
	do_cfg_smmuv3_irpt_init(smmuv3_tbu_base_addr);

	cam_info("%s config smmu ok", __FUNCTION__);
	return SMMUV3_SUCCESS;
}

int do_cfg_smmuv3_tbuclose(hjpeg_hw_ctl_t *hw_ctl)
{
	struct timeval start_time;
	struct timeval end_time;
	bool btimeout_flag = false;

	u_smmuv3_smmu_tbu_cr tmp_tbu_cr;
	u_smmuv3_smmu_tbu_crack tmp_tbu_crack;
	void __iomem *smmuv3_tbu_base_addr = NULL;

	cam_info("%s enter", __FUNCTION__);
	if (!hw_ctl) {
		cam_err("%s %d hw_ctrl NULL", __FUNCTION__, __LINE__);
		return SMMUV3_FAIL;
	}

	smmuv3_tbu_base_addr = hw_ctl->smmu_viraddr;

	if (IS_ERR_OR_NULL(smmuv3_tbu_base_addr)) {
		cam_err("%s %d base addr error", __FUNCTION__, __LINE__);
		return SMMUV3_FAIL;
	}

	tmp_tbu_cr.reg32 = get_reg_val((void __iomem*)((char*)
		smmuv3_tbu_base_addr + SMMUV3_TBU_CR_OFFSET));
	tmp_tbu_cr.bits.tbu_en_req = 0x0; /* request tbu and tcu connection */
	set_reg_val((void __iomem*)((char*)smmuv3_tbu_base_addr +
		SMMUV3_TBU_CR_OFFSET), tmp_tbu_cr.reg32);

	/*
	 * wait tbu and tcu disconnect success,
	 * until tbu_en_ack = 0x1 and timeout < 1000us
	 */
	do_gettimeofday(&start_time);
	do {
		tmp_tbu_crack.reg32 = get_reg_val((void __iomem*)((char*)
			smmuv3_tbu_base_addr + SMMUV3_TBU_CRACK_OFFSET));
		do_gettimeofday(&end_time);
		btimeout_flag = (TBU_EN_ACK_TIMECOST(start_time, end_time) >
			TBU_EN_ACK_TIMEOUT);
	} while (!btimeout_flag && (tmp_tbu_crack.bits.tbu_en_ack != 0x1));

	cam_debug("%s: timeoutflag: %u, tmp_tbu_crack.reg32: 0x%08x",
		__FUNCTION__, btimeout_flag, tmp_tbu_crack.reg32);

	if (btimeout_flag && tmp_tbu_crack.bits.tbu_en_ack != 0x1) {
		cam_err("%s: tbu and tcu disconnect [timeout %u] [tbuCrack.reg32: 0x%08x]failed",
			__FUNCTION__, btimeout_flag, tmp_tbu_crack.reg32);
		return SMMUV3_FAIL;
	}

	if (tmp_tbu_crack.bits.tbu_connected != 0x0) {
		cam_err("%s: tbu and tcu disconnect failed![tbuCrack: 0x%08x]",
			__FUNCTION__, tmp_tbu_crack.reg32);
		return SMMUV3_FAIL;
	}
	cam_info("%s tbu close OK", __FUNCTION__);
	return SMMUV3_SUCCESS;
}

static do_cfg_smmuv3_ippcomm(hjpeg_hw_ctl_t *hw_ctl)
{
	int ret;
	cam_info("%s enter", __FUNCTION__);

	ret = s_hjpeg_ipp_comm->enable_smmu(s_hjpeg_ipp_comm);
	if (ret != 0) {
		cam_err("%s: enable smmu failed", __FUNCTION__);
		return ret;
	}

	ret = s_hjpeg_ipp_comm->set_pref(s_hjpeg_ipp_comm,
		hw_ctl->secadapt_prop.start_swid,
		hw_ctl->secadapt_prop.swid_num);
	if (ret != 0) {
		cam_err("%s set smmu pref failed", __FUNCTION__);
		return ret;
	}

	ret = s_hjpeg_ipp_comm->setsid_smmu(s_hjpeg_ipp_comm,
		hw_ctl->secadapt_prop.start_swid,
		hw_ctl->secadapt_prop.swid_num,
		hw_ctl->secadapt_prop.sid,
		hw_ctl->secadapt_prop.ssid_ns);
	if (ret != 0) {
		cam_err("%s: set smmu sid failed", __FUNCTION__);
		return ret;
	}

	dump_smmu_reg(hw_ctl);

	return SMMUV3_SUCCESS;
}

void dump_smmu_reg(hjpeg_hw_ctl_t *hw_ctl)
{
	void __iomem *viraddr = NULL;
	uint32_t offset;
	int i;
	int swid;

	if (!hw_ctl) {
		cam_err("%s %d hw_ctrl NULL", __FUNCTION__, __LINE__);
		return;
	}

	if (hw_ctl->smmu_type != ST_SMMUV3)
		return;

	if (IS_ERR_OR_NULL(hw_ctl->secadapt_viraddr) ||
		IS_ERR_OR_NULL(hw_ctl->smmu_viraddr)) {
		cam_err("%s %d base addr error", __FUNCTION__, __LINE__);
		return;
	}

	swid = hw_ctl->secadapt_prop.start_swid;
	viraddr = hw_ctl->secadapt_viraddr;
	for (i = 0; i < hw_ctl->secadapt_prop.swid_num; i++) {
		offset = SECADAPT_SWID_CFG_NS_OFFSET + (swid++) *
			SECADAPT_SWID_CFG_NS_PER_OFFSET;
		cam_info("%s sec_adapt addr reg 0x%x = 0x%x",
			__FUNCTION__, offset,
			get_reg_val((void __iomem*)((char*)viraddr +
			offset)));
	}

	viraddr = hw_ctl->smmu_viraddr;
	cam_info("%s tbu addr reg 0x%x = 0x%x", __FUNCTION__,
		SMMUV3_TBU_CR_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		SMMUV3_TBU_CR_OFFSET)));
	cam_info("%s tbu addr reg 0x%x = 0x%x", __FUNCTION__,
		SMMUV3_TBU_CRACK_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		SMMUV3_TBU_CRACK_OFFSET)));

	swid = hw_ctl->secadapt_prop.start_swid;
	for (i = 0; i < hw_ctl->secadapt_prop.swid_num; i++) {
		offset = SMMUV3_SWID_CFG_OFFSET + (swid++) *
			SECADAPT_SWID_CFG_NS_PER_OFFSET;
		cam_info("%s tbu swid cfg addr reg 0x%x = 0x%x", __FUNCTION__,
			offset,
			get_reg_val((void __iomem*)((char*)viraddr +
			offset)));
	}
}

/* smmuv3 station, when single ip debug use pc_drv */
int hjpeg_smmu_config(hjpeg_hw_ctl_t *hw_ctl, void *pgd)
{
	if (!hw_ctl || !pgd) {
		cam_err("%s %d invalid parameter", __FUNCTION__, __LINE__);
		return -1;
	}

	switch (hw_ctl->smmu_type) {
	case ST_ISP_SMMU:
		return do_cfg_isp_smmu(hw_ctl);
	case ST_SMMU:
		return do_cfg_smmu(hw_ctl, pgd);
	case ST_IPP_SMMU:
		cam_debug("%s %d using ipp smmu enable", __func__, __LINE__);
		return do_ippsmmu_enable(0);
	case ST_SMMUV3:
		if (hw_ctl->power_controller == PC_IPPCOMM)
			return do_cfg_smmuv3_ippcomm(hw_ctl);
		else if (hw_ctl->power_controller == PC_DRV)
			return do_cfg_smmuv3(hw_ctl);
		/* fall-through */
	default:
		break;
	}
	cam_err("%s %d error smmu type %d", __FUNCTION__, __LINE__,
		hw_ctl->smmu_type);
	return -1;
}

int hjpeg_smmu_deconfig(hjpeg_hw_ctl_t *hw_ctl, void *pgd)
{
	if (!hw_ctl || !pgd) {
		cam_err("%s %d invalid parameter", __func__, __LINE__);
		return -1;
	}

	if (hw_ctl->smmu_type == ST_IPP_SMMU) {
		cam_debug("%s %d using ipp smmu disable", __func__, __LINE__);
		return do_ippsmmu_disable(0);
	}
	return 0;
}
