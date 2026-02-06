/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __CLK_PPLL_H_
#define __CLK_PPLL_H_

#include <soc_crgperiph_interface.h>
#include <soc_pmctrl_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_acpu_baseaddr_interface.h>
#include "clk.h"

enum {
	PPLL0 = 0,
	PPLL1,
	PPLL2,
	PPLL3,
	PPLL4,
	PPLL5,
	PPLL6 = 0x6,
	PPLL7 = 0x7,
	SCPLL = 0x8,
	PPLL2_B = 0x9,
	FNPLL1 = 0xA,
	FNPLL4 = 0xB,
	AUPLL = 0xC,
	PCIE0PLL = 0xD,
	PCIE1PLL = 0xE,
	PPLLMAX,
};

/* ********** PLL DEFINE **************** */
#define AP_PPLL_STABLE_TIME			1000
#define PLL_MASK_OFFSET				16

#ifdef CONFIG_HISI_PLL_LOCK_RETRY
#define MAX_RETRY_NUM 5
#define MAX_CHECK_TIME 60
#define PLL_LOCK_STATUS 0xFFFFF

struct pll_lock_info {
	u32 pll_en_count[PPLLMAX];
	u32 pll_retry_count[PPLLMAX][MAX_RETRY_NUM];
};
#endif

/* PPLL */
struct hi3xxx_ppll_clk {
	struct clk_hw hw;
	u32 ref_cnt; /* reference count */
	u32 pll_id;
	u32 en_ctrl[PLL_REG_NUM];
	u32 gt_ctrl[PLL_REG_NUM];
	u32 bypass_ctrl[PLL_REG_NUM];
	u32 st_ctrl[PLL_REG_NUM];
#ifdef CONFIG_HISI_CLK_DEBUG
	u32 en_method;
#endif
	void __iomem *addr; /* base addr */
	void __iomem *endisable_addr;
	spinlock_t *lock;
};
#endif
