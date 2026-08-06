/*
 * Copyright (c) 2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "dw_mmc_clk_for_calibration.h"

static void __iomem *clk_pericrg_base = NULL;
static void __iomem *clk_sd_reg_base = NULL;
static void __iomem *clk_gpio_sd_clk_base = NULL;



static int init_ctrl_base(void)
{
	struct device_node *np = NULL;

	if (!clk_pericrg_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,crgctrl");
		if (!np) {
			pr_err("can't find crgctrl!\n");
			return -1;
		}

		clk_pericrg_base = of_iomap(np, 0);
		if (!clk_pericrg_base) {
			pr_err("crgctrl iomap error!\n");
			return -1;
		}
	}

	if (!clk_sd_reg_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,sd_reg_base");
		if (!np) {
			pr_err("can't find sd_reg_base!\n");
			return -1;
		}

		clk_sd_reg_base = of_iomap(np, 0);
		if (!clk_sd_reg_base) {
			pr_err("sd_reg_base iomap error!\n");
			return -1;
		}
	}

	if (!clk_gpio_sd_clk_base) {
		np = of_find_compatible_node(NULL, NULL, "Hisilicon,gpio_sd_clk_base");
		if (!np) {
			pr_err("can't find gpio_sd_clk_base!\n");
			return -1;
		}

		clk_gpio_sd_clk_base = of_iomap(np, 0);
		if (!clk_gpio_sd_clk_base) {
			pr_err("gpio_sd_clk_base iomap error!\n");
			return -1;
		}
	}

	return 0;
}

void output_clk_for_hifi_calibration(int input)
{
	int res = 0;

	res = init_ctrl_base();

	if(res)
		return;

	/*ppll2 1920M*/
	writel(cancer_source_clk, clk_pericrg_base + cancer_source_clk_base);
	/*gt_hclk_sd enable*/
	writel(cancer_gt_hclk_sd_enable, clk_pericrg_base + cancer_gt_hclk_sd_enable_base);
	/*sc_gt_clk_sd enable*/
	writel(cancer_sc_gt_clk_sd_enable, clk_pericrg_base + cancer_sc_gt_clk_sd_enable_base);
	/*gt_clk_sd enable*/
	writel(cancer_gt_clk_sd_enable, clk_pericrg_base + cancer_gt_clk_sd_enable_base);
	/*sc_gt_clk_sd_sys enable*/
	writel(cancer_sc_gt_clk_sd_sys_enable, clk_pericrg_base + cancer_sc_gt_clk_sd_sys_enable_base);
	/*ip_hrst_sd ip_rst_sd rst*/
	writel(cancer_ip_hrst_sd_ip_rst_sd_rst, clk_pericrg_base + cancer_ip_hrst_sd_ip_rst_sd_rst_base);
	/*ip_hrst_sd ip_rst_sd urst*/
	writel(cancer_ip_hrst_sd_ip_rst_sd_urst, clk_pericrg_base + cancer_ip_hrst_sd_ip_rst_sd_urst_base);

	/*rem SD host clk enable 192M*/
	writel(cancer_gpio_op1, clk_sd_reg_base + cancer_gpio_base);
	writel(cancer_gpio_op2, clk_sd_reg_base + cancer_gpio_base);
	writel(cancer_gpio_op3, clk_sd_reg_base + cancer_gpio_base);
	writel(cancer_clkena_op, clk_sd_reg_base + cancer_clkena_base);
	writel(cancer_cmd_op, clk_sd_reg_base + cancer_cmd_base);

	/*GPIO Fun1,input 1 means enable,input other means disable*/
	if(1 == input)
		writel(cancer_gpio_fun_value, clk_gpio_sd_clk_base + cancer_gpio_fun_base);
	else
		writel(cancer_gpio_fun_value_dis, clk_gpio_sd_clk_base + cancer_gpio_fun_base);

	/*drive*/
	writel(cancer_gpio_iomg_value, clk_gpio_sd_clk_base + cancer_gpio_iomg_base);

	pr_err("output_clk_for_hifi_calibration ok!\n");
}
