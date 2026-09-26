/*
 * Copyright (c) 2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "dw_mmc_clk_for_calibration.h"

static void __iomem *clk_pericrg_base = NULL;
static void __iomem *clk_sd_reg_base = NULL;
static void __iomem *clk_gpio_sd_clk_base = NULL;
static void __iomem *mmc0_crg_base = NULL;

static void init_ctrl_base(void)
{
	struct device_node *np = NULL;

	if (!clk_pericrg_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,crgctrl");
		if (!np) {
			pr_err("can't find crgctrl!\n");
			return;
		}

		clk_pericrg_base = of_iomap(np, 0);
		if (!clk_pericrg_base) {
			pr_err("crgctrl iomap error!\n");
			return;
		}
	}

	if (!clk_sd_reg_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,sd_reg_base");
		if (!np) {
			pr_err("can't find sd_reg_base!\n");
			return;
		}

		clk_sd_reg_base = of_iomap(np, 0);
		if (!clk_sd_reg_base) {
			pr_err("sd_reg_base iomap error!\n");
			return;
		}
	}

	if (!clk_gpio_sd_clk_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,gpio_sd_clk_base");
		if (!np) {
			pr_err("can't find gpio_sd_clk_base!\n");
			return;
		}

		clk_gpio_sd_clk_base = of_iomap(np, 0);
		if (!clk_gpio_sd_clk_base) {
			pr_err("gpio_sd_clk_base iomap error!\n");
			return;
		}
	}

	if (!mmc0_crg_base) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,mmc0crg");
		if (!np) {
			pr_err("can't find mmc0_crg!\n");
			return;
		}

		mmc0_crg_base = of_iomap(np, 0);
		if (!mmc0_crg_base) {
			pr_err("mmc0sysctrl iomap error!\n");
			return;
		}
	}
}

void output_clk_for_hifi_calibration(int input)
{
	init_ctrl_base();

	if (!clk_pericrg_base || !clk_sd_reg_base || !clk_gpio_sd_clk_base)
		return;

	/* ppll2 1920M */
	writel(SOURCE_CLK, clk_pericrg_base + SOURCE_CLK_BASE);
	/* sc_gt_clk_sd enable */
	writel(SC_GT_CLK_SD_ENABLE,
	       clk_pericrg_base + SC_GT_CLK_SD_ENABLE_BASE);
	/* gt_clk_sd enable */
	writel(GT_CLK_SD_ENABLE,
	       clk_pericrg_base + GT_CLK_SD_ENABLE_BASE);
	/* sc_gt_clk_sd_sys enable */
	writel(SC_GT_CLK_SD_SYS_ENABLE,
	       clk_pericrg_base + SC_GT_CLK_SD_SYS_ENABLE_BASE);

	if (mmc0_crg_base) {
		/* gt_hclk_sd enable */
		writel(GT_HCLK_SD_ENABLE_CS,
		       mmc0_crg_base + GT_HCLK_SD_ENABLE_BASE);
		/* ip_hrst_sd ip_rst_sd rst */
		writel(MMC_BIT_RST_SD_TAURUS, mmc0_crg_base + MMC0_CRG_SD_HRST);
		/* ip_hrst_sd ip_rst_sd urst */
		writel(BIT_URST_SD_TAURUS, mmc0_crg_base + MMC0_CRG_SD_HURST);
	} else {
		/* gt_hclk_sd enable */
		writel(GT_HCLK_SD_ENABLE,
		       clk_pericrg_base + GT_HCLK_SD_ENABLE_BASE);
		/* ip_hrst_sd ip_rst_sd rst */
		writel(IP_HRST_SD_IP_RST_SD_RST,
		       clk_pericrg_base + IP_HRST_SD_IP_RST_SD_RST_BASE);
		/* ip_hrst_sd ip_rst_sd urst */
		writel(IP_HRST_SD_IP_RST_SD_URST,
		       clk_pericrg_base +
			       IP_HRST_SD_IP_RST_SD_URST_BASE);
	}

	/* rem SD host clk enable 192M */
	writel(GPIO_OP1, clk_sd_reg_base + GPIO_BASE);
	writel(GPIO_OP2, clk_sd_reg_base + GPIO_BASE);
	writel(GPIO_OP3, clk_sd_reg_base + GPIO_BASE);
	writel(CLKENA_OP, clk_sd_reg_base + CLKENA_BASE);
	writel(CMD_OP, clk_sd_reg_base + CMD_BASE);

	/* GPIO Fun1,input 1 means enable,input other means disable */
	if (input == 1)
		writel(GPIO_FUN_VALUE,
		       clk_gpio_sd_clk_base + GPIO_FUN_BASE);
	else
		writel(GPIO_FUN_VALUE_DIS,
		       clk_gpio_sd_clk_base + GPIO_FUN_BASE);

	/* gpio drive */
	writel(GPIO_IOMG_VALUE,
	       clk_gpio_sd_clk_base + GPIO_IOMG_BASE);

	pr_err("output_clk_for_hifi_calibration ok!\n");
}
