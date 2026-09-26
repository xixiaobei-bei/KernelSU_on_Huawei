#ifndef _DW_MMC_CLK_FOR_CALIBRATION_H
#define _DW_MMC_CLK_FOR_CALIBRATION_H
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
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/of.h>
#include <linux/of_address.h>

/* crg base offset */
#define SOURCE_CLK_BASE 0xb8
#define GT_HCLK_SD_ENABLE_BASE 0x0
#define SC_GT_CLK_SD_ENABLE_BASE 0xf4
#define GT_CLK_SD_ENABLE_BASE 0x40
#define SC_GT_CLK_SD_SYS_ENABLE_BASE 0xf4
#define IP_HRST_SD_IP_RST_SD_RST_BASE 0x90
#define IP_HRST_SD_IP_RST_SD_URST_BASE 0x94
#define MMC0_CRG_SD_HRST 0x20
#define MMC0_CRG_SD_HURST 0x24
#define MMC_BIT_RST_SD_TAURUS (0x1 << 0)
#define BIT_URST_SD_TAURUS (0x1 << 0)
#define GT_HCLK_SD_ENABLE_CS 0x1

/* crg value */
#define SOURCE_CLK 0x7F0060
#define GT_HCLK_SD_ENABLE 0x40000000
#define SC_GT_CLK_SD_ENABLE 0x80008
#define GT_CLK_SD_ENABLE 0x20000
#define SC_GT_CLK_SD_SYS_ENABLE 0x200020
#define IP_HRST_SD_IP_RST_SD_RST 0x1040000
#define IP_HRST_SD_IP_RST_SD_URST 0x1040000


/* sd host base offset */
#define GPIO_BASE 0x58
#define CLKENA_BASE 0x10
#define CMD_BASE 0x2c

/* sd host value */
#define GPIO_OP1 0xFE0F00
#define GPIO_OP2 0xFE0900
#define GPIO_OP3 0xFF0900
#define CLKENA_OP 0x1
#define CMD_OP 0xa0202000

/* gpio bse offset */
#define GPIO_FUN_BASE 0x0
#define GPIO_IOMG_BASE 0x800
/* gpio value */
#define GPIO_FUN_VALUE 0x1
#define GPIO_FUN_VALUE_DIS 0x0
#define GPIO_IOMG_VALUE 0x50

extern void output_clk_for_hifi_calibration(int input);
#endif