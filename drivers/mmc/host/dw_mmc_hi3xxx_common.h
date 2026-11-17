/*
 * support definition common interface for dw_mmc_hi3xxx
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#ifndef _DW_MMC_HI3XXX_COMMON_
#define _DW_MMC_HI3XXX_COMMON_

#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>

#include "dw_mmc_zodiac.h"

#define CCLK_FRQ 0
#define CCLK_DIV 1
#define DRV_PHASE 2
#define SAM_DLY 3
#define SAM_PHASE_MID 2
#define SAM_PHASE_MAX 4
#define SAM_PHASE_MIN 5
#define BUS_HZ 6

#define IS_CS_TIMING_CONFIG 0x1
#define DW_MMC_MIN_SZ_ARRAY_READ 2
#define DW_MCI_HS_DO_RESET 1
#define DW_MCI_HS_DO_UNRESET 0

#ifdef CONFIG_SD_TIMEOUT_RESET
#define VOLT_HOLD_CLK_08V 0x1
#define VOLT_TO_1S 0x1
#endif

extern int hs_timing_config[][TUNING_INIT_TIMING_MODE][TUNING_INIT_CONFIG_NUM];
extern struct dw_mci *sdio_host;

struct dw_mci *dw_mci_get_1135_host(void);
void dw_mci_hs_set_clk_helper(struct dw_mci *host, struct mmc_ios *ios);
void dw_mci_hs_set_timing(
	struct dw_mci *host, int id, int timing, int sam_phase, int clk_div);
int dw_mci_hs_tuning_find_condition_helper(struct dw_mci *host, int timing);
int dw_mci_hs_tuning_move_helper(struct dw_mci *host, int timing, int start);
int dw_mci_priv_execute_tuning_helper(struct dw_mci_slot *slot, u32 opcode,
	struct dw_mci_tuning_data *tuning_data);
void dw_mci_hi3xxx_reset_restore(struct dw_mci *host);
int dw_mci_hs_set_controller(struct dw_mci *host, bool set);
void dw_mci_hs_set_rst_m(struct dw_mci *host, bool set);
bool dw_mci_wait_host_busy(struct dw_mci *host, bool need_reset);
#endif /* DW_MMC_HI3XXX_COMMON_ */
