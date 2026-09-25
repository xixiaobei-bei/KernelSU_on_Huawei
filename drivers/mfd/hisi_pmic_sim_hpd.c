/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: pmic sim hpd irq process
 * Author: Hisilicon
 * Create: 2019-11-18
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include "hisi_pmic_sim_hpd.h"
#include <linux/mfd/hisi_pmic.h>

#if defined(HISI_PMIC_SIM_HPD_EN)

static void hisi_pmic_ldo_en_clear(u32 ldo_en_reg)
{
	unsigned int reg_tmp;
	unsigned int value = LDO_DISABLE;
	unsigned int mask = LDO_ONOFF_MASK;

	reg_tmp = hisi_pmic_reg_read(ldo_en_reg);
	reg_tmp &= ~mask;
	reg_tmp |= value;
	hisi_pmic_reg_write(ldo_en_reg, reg_tmp);
}

void hisi_pmic_sim_hpd_proc(void)
{
	unsigned int pending;
	unsigned int sim_ctr0, sim_ctr1;

	pending = hisi_pmic_reg_read(SIM_IRQ_ADDR);

	if (!(pending & SIM_HPD_PENDING))
		return;

	sim_ctr0 = hisi_pmic_reg_read(SIM_CTRL0_ADDR);
	sim_ctr1 = hisi_pmic_reg_read(SIM_CTRL1_ADDR);

	if (pending & SIM0_HPD_R_MASK) {
		if (sim_ctr0 & SIM0_HPD_R_LDO11_PD) {
			hisi_pmic_ldo_en_clear(LDO11_ONOFF_ADDR);

			if (sim_ctr0 & SIM0_HPD_LDO12_PD)
				hisi_pmic_ldo_en_clear(LDO12_ONOFF_ADDR);
		}

		if (sim_ctr1 & SIM0_HPD_R_LDO16_PD)
			hisi_pmic_ldo_en_clear(LDO16_ONOFF_ADDR);
	}

	if (pending & SIM0_HPD_F_MASK) {
		if (sim_ctr0 & SIM0_HPD_F_LDO11_PD) {
			hisi_pmic_ldo_en_clear(LDO11_ONOFF_ADDR);

			if (sim_ctr0 & SIM0_HPD_LDO12_PD)
				hisi_pmic_ldo_en_clear(LDO12_ONOFF_ADDR);
		}

		if (sim_ctr1 & SIM0_HPD_F_LDO16_PD)
			hisi_pmic_ldo_en_clear(LDO16_ONOFF_ADDR);
	}

	if (pending & SIM1_HPD_R_MASK) {
		if (sim_ctr0 & SIM1_HPD_R_LDO12_PD)
			hisi_pmic_ldo_en_clear(LDO12_ONOFF_ADDR);

		if (sim_ctr1 & SIM1_HPD_R_LDO16_PD)
			hisi_pmic_ldo_en_clear(LDO16_ONOFF_ADDR);
	}

	if (pending & SIM1_HPD_F_MASK) {
		if (sim_ctr0 & SIM1_HPD_F_LDO12_PD)
			hisi_pmic_ldo_en_clear(LDO12_ONOFF_ADDR);

		if (sim_ctr1 & SIM1_HPD_F_LDO16_PD)
			hisi_pmic_ldo_en_clear(LDO16_ONOFF_ADDR);
	}
}
#endif

#if defined(HISI_SUB_PMIC_SIM_HPD_EN)

static void hisi_sub_pmic_ldo_en_clear(u32 ldo_en_reg)
{
	unsigned int reg_tmp;
	unsigned int value = LDO_DISABLE;
	unsigned int mask = LDO_ONOFF_MASK;

	reg_tmp = hisi_sub_pmic_reg_read(ldo_en_reg);
	reg_tmp &= ~mask;
	reg_tmp |= value;
	hisi_sub_pmic_reg_write(ldo_en_reg, reg_tmp);
}

static void hisi_sub_pmic_level_bbrp_clear(u32 level_bbrp_reg)
{
	unsigned int reg_tmp;
	unsigned int value = SIM_ENABLE;
	unsigned int mask = SIM_ONOFF_MASK;

	reg_tmp = hisi_sub_pmic_reg_read(level_bbrp_reg);
	reg_tmp &= ~mask;
	reg_tmp |= value;
	hisi_sub_pmic_reg_write(level_bbrp_reg, reg_tmp);
}

static void hisi_sub_pmic_sim_ldo_clear(u32 ldo_en_reg, u32 level_bbrp_reg)
{
	hisi_sub_pmic_level_bbrp_clear(level_bbrp_reg);
	hisi_sub_pmic_ldo_en_clear(ldo_en_reg);
}

void hisi_sub_pmic_sim_hpd_proc(void)
{
	unsigned int pending;
	unsigned int sub_sim_ctr0;

	pending = hisi_sub_pmic_reg_read(SUB_SIM_IRQ_ADDR);

	if (!(pending & SUB_SIM_HPD_PENDING))
		return;

	sub_sim_ctr0 = hisi_sub_pmic_reg_read(SUB_SIM_CTRL0_ADDR);

	if (pending & SUB_SIM_HPD_R_MASK) {
		if (sub_sim_ctr0 & SUB_SIM_HPD_R_PD_EN) {
			if (sub_sim_ctr0 & SUB_SIM_HPD_PD_LDO53)
				hisi_sub_pmic_sim_ldo_clear(LDO53_ONOFF_ADDR, SUB_SIM_CFG_0_ADDR);

			if (sub_sim_ctr0 & SUB_SIM_HPD_PD_LDO54)
				hisi_sub_pmic_sim_ldo_clear(LDO54_ONOFF_ADDR, SUB_SIM_CFG_2_ADDR);
		}
	}

	if (pending & SUB_SIM_HPD_F_MASK) {
		if (sub_sim_ctr0 & SUB_SIM_HPD_F_PD_EN) {
			if (sub_sim_ctr0 & SUB_SIM_HPD_PD_LDO53)
				hisi_sub_pmic_sim_ldo_clear(LDO53_ONOFF_ADDR, SUB_SIM_CFG_0_ADDR);

			if (sub_sim_ctr0 & SUB_SIM_HPD_PD_LDO54)
				hisi_sub_pmic_sim_ldo_clear(LDO54_ONOFF_ADDR, SUB_SIM_CFG_2_ADDR);
		}
	}
}
#endif
