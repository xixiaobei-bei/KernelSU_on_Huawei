/*
 * hi_cdc_ctrl.h
 *
 * codec controller driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO., Ltd.All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HI_CDC_CTRL_H__
#define __HI_CDC_CTRL_H__

#include <linux/kernel.h>

enum hi_cdcctrl_supply {
	CDC_SUP_MAIN,
	CDC_SUP_ANLG,
	CDC_SUP_MAX
};

enum hi_cdcctrl_clk {
	CDC_MCLK,
	CDC_32K,
	CDC_CLK_MAX
};

enum bustype_select {
	BUSTYPE_SELECT_NULL = 0,
	BUSTYPE_SELECT_SLIMBUS = 1,
	BUSTYPE_SELECT_SSI = 2,
	BUSTYPE_SELECT_BUTT
};

enum {
	DA_COMBINE_CODEC_TYPE_V2,
	DA_COMBINE_CODEC_TYPE_V3,
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	DA_COMBINE_CODEC_TYPE_V5,
#endif
	DA_COMBINE_CODEC_TYPE_BUTT
};

enum ssi_check_type {
	SSI_STATE_CHECK_DISABLE,
	SSI_STATE_CHECK_ENABLE,
	SSI_STATE_CHECK_BUTT
};

struct hi_cdc_ctrl {
	struct device *dev;
	enum bustype_select bus_sel;
	bool pm_runtime_support;
	bool ssi_check_enable;
	uint32_t slimbusclk_cdc_drv;
	uint32_t slimbusdata_cdc_drv;
	bool need_reset_in_kernel;
	struct pinctrl *pctrl;
};

void hi_cdcctrl_dump(struct hi_cdc_ctrl *cdc_ctrl);

unsigned int hi_cdcctrl_reg_read(struct hi_cdc_ctrl *cdc_ctrl, unsigned int reg_addr);

int hi_cdcctrl_reg_write(struct hi_cdc_ctrl *cdc_ctrl,
	unsigned int reg_addr, unsigned int reg_val);

void hi_cdcctrl_reg_update_bits(struct hi_cdc_ctrl *cdc_ctrl, unsigned int reg,
	unsigned int mask, unsigned int value);

int hi_cdcctrl_hw_reset(const struct hi_cdc_ctrl *cdc_ctrl);

int hi_cdcctrl_get_irq(const struct hi_cdc_ctrl *cdc_ctrl);

int hi_cdcctrl_enable_supply(struct hi_cdc_ctrl *cdc_ctrl,
	enum hi_cdcctrl_supply sup_type, bool enable);

int hi_cdcctrl_enable_clk(struct hi_cdc_ctrl *cdc_ctrl,
	enum hi_cdcctrl_clk clk_type, bool enable);

void hi_cdcctrl_pm_get(void);

void hi_cdcctrl_pm_put(void);

unsigned int hi_cdcctrl_get_pmu_mclk_status(void);

#endif /* __HI_CDC_CTRL_H__ */

