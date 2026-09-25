/*
 * support definition interface for dw_mmc_hi3xxx_helper
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DW_MMC_HI3XXX_HELPER_
#define _DW_MMC_HI3XXX_HELPER_

#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/of.h>

#ifdef CONFIG_HUAWEI_DSM
#define LOG_BUF_SIZE 1024
#endif

#define EMMC_PATTERN_ADDRESS (384 * 2)

#define RANGE_MIN 0
#define RANGE_MAX 1
#define DW_MMC_SET_WRITE_FLAG 0x80000000
#define DW_MMC_TUNING_MOVE_OFFSET 2
#define DW_MMC_TUNING_MOVE_RIGHT 1
#define DW_MMC_TUNING_MOVE_LEFT (-1)
#define DW_MMC_SAMPLE_OFFSET 1
#define DW_MMC_HS200_TUNING_BLKSZ_8_BIT 128
#define DW_MMC_HS200_TUNING_BLKSZ_4_BIT 64
#define DW_MMC_TUNING_BLKSZ 64
#define DW_MMC_READ_SINGLE_TUNING_BLKSZ 512
#define DW_MMC_SDIO_RW_TUNING_BLKSZ 4

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
static int hs_timing_config_aries_cs_sd_ppll3[TUNING_INIT_CONFIG_NUM] = {
	1200000000, 6, 4, 3, 13, 0, 171000000 };
static int hs_timing_config_aries_cs_sdio_ppll3[TUNING_INIT_CONFIG_NUM] = {
	1200000000, 6, 4, 3, 13, 0, 171000000 };
static int hs_timing_config_cancer_cs_sd_ppll2[TUNING_INIT_CONFIG_NUM] = {
	960000000, 4, 4, 2, 9, 0, 192000000 };
static int hs_timing_config_cancer_cs_sdio_ppll2[TUNING_INIT_CONFIG_NUM] = {
	960000000, 4, 4, 2, 9, 0, 192000000 };
static int hs_timing_config_libra_sd_ppll2[TUNING_INIT_CONFIG_NUM] = {
	960000000, 4, 3, 2, 9, 0, 192000000 };
static int hs_timing_config_libra_sdio_ppll2[TUNING_INIT_CONFIG_NUM] = {
	960000000, 4, 3, 2, 9, 0, 192000000 };
static int hs_timing_config_taurus_sd_ppll2[TUNING_INIT_CONFIG_NUM] = {
	960000000, 4, 2, 3, 9, 0, 192000000 };
static int hs_timing_config_taurus_sdio_ppll2[TUNING_INIT_CONFIG_NUM] = {
	1572000000, 7, 7, 6, 15, 0, 196500000 };
static int hs_timing_960_clock_source_capricorn[TUNING_INIT_CONFIG_NUM] = {
	960000000, 4, 3, 2, 9, 0, 192000000 };
#else
#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
static int hs_timing_960_clock_source_capricorn[TUNING_INIT_CONFIG_NUM] = {
	960000000, 4, 3, 2, 9, 0, 192000000 };
static int hs_timing_1920_clock_source_capricorn[TUNING_INIT_CONFIG_NUM] = {
	1920000000, 9, 7, 4, 19, 0, 192000000 };
#endif
#endif

#ifdef CONFIG_SDIO_HI_CLOCK_RETUNING
#define TUNING_FLAG_ALL_PASS 0x1
#define TUNING_ERR_ALL_PHASE_PASS (-2)
#endif

static const u8 tuning_blk_pattern_4bit[] = {
	0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
	0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
	0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
	0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
	0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
	0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
	0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
	0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const u8 tuning_blk_pattern_8bit[] = {
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

#define SCTRL_SCPERSTATUS16_ADDR           0x35C
#define SCTRL_SCPERSTATUS17_ADDR           0x358
#define SCTRL_SCCLKDIV10_ADDR              0x280
#define SCTRL_SCCLKDIV11_ADDR              0x284
#define SCTRL_SCMMC_AUTODIV_ADDR           0x288
#define SCTRL_SCCLKDIV0_ADDR               0x250
#define SCTRL_SCPPLLCTRL0_ADDR             0x100
#define SCTRL_SCPPLLCTRL1_ADDR             0x104
#define SCTRL_SCPPLLSSCCTRL_ADDR           0x108
#define SCTRL_SCPPLLSTAT_ADDR              0x10C
#define SCTRL_SCPERSTAT2_ADDR              0x19C
#define PCTRL_PERI_STAT2_ADDR              0x09C
#define BUCK0_VSET_ADDR                    0x48
#define BUCK0_VSET_ECO_ADDR                0x49
#define LDO30_VSET_ADDR                    0x71

extern void __iomem *sys_base;
extern void __iomem *pctrl_base;
#endif /* DW_MMC_HI3XXX_HELPER_ */
