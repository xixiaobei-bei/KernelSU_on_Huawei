/*
 * Copyright (c) 2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "dw_mmc_hisi.h"
#include <linux/bootdevice.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/hisi/kirin_partition.h>
#include <linux/uaccess.h>
#include <linux/printk.h>
#include <linux/bootdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/of_address.h>
#include <linux/pm_runtime.h>
#include <linux/clk-provider.h>
#include <linux/hisi/util.h>
#include <linux/hwspinlock.h>
#include "dw_mmc.h"
#include "dw_mmc-pltfm.h"

 /*crg base offset*/
#define cancer_source_clk_base 0xb8
#define cancer_gt_hclk_sd_enable_base 0x0
#define cancer_sc_gt_clk_sd_enable_base 0xf4
#define cancer_gt_clk_sd_enable_base 0x40
#define cancer_sc_gt_clk_sd_sys_enable_base 0xf4
#define cancer_ip_hrst_sd_ip_rst_sd_rst_base 0x90
#define cancer_ip_hrst_sd_ip_rst_sd_urst_base 0x94

/*crg value*/
#define cancer_source_clk 0x7F0060
#define cancer_gt_hclk_sd_enable 0x40000000
#define cancer_sc_gt_clk_sd_enable 0x80008
#define cancer_gt_clk_sd_enable 0x20000
#define cancer_sc_gt_clk_sd_sys_enable 0x200020
#define cancer_ip_hrst_sd_ip_rst_sd_rst 0x1040000
#define cancer_ip_hrst_sd_ip_rst_sd_urst 0x1040000


/*sd host base offset*/
#define cancer_gpio_base 0x58
#define cancer_clkena_base 0x10
#define cancer_cmd_base 0x2c

/*sd host value*/
#define cancer_gpio_op1 0xFE0F00
#define cancer_gpio_op2 0xFE0900
#define cancer_gpio_op3 0xFF0900
#define cancer_clkena_op 0x1
#define cancer_cmd_op 0xa0202000

/*gpio bse offset*/
#define cancer_gpio_fun_base 0x0
#define cancer_gpio_iomg_base 0x800
/*gpio value*/
#define cancer_gpio_fun_value 0x1
#define cancer_gpio_fun_value_dis 0x0
#define cancer_gpio_iomg_value 0x50
 
extern void output_clk_for_hifi_calibration(int input);