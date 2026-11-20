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
#ifndef __PLAT_CLK_GATE_H_
#define __PLAT_CLK_GATE_H_

#include <linux/clk.h>
#include <securec.h>
#include "dvfs/peri_volt_poll.h"
#include "clk.h"

#ifndef CONFIG_HISI_CLK_ALWAYS_ON
#define hi3xxx_CLK_GATE_DISABLE_OFFSET		0x4
#endif

#define hi3xxx_CLK_GATE_STATUS_OFFSET		0x8
#define hi3xxx_RST_DISABLE_REG_OFFSET		0x4

#define CLOCK_GATE_SYNC_MAX		10 /* us */

struct hi3xxx_periclk {
	struct clk_hw hw;
	void __iomem *enable; /* enable register */
	u32 ebits; /* bits in enable/disable register */
	const char *friend;
	spinlock_t *lock;
	u32 peri_dvfs_sensitive; /* 0:non,1:direct avs,rate(HZ):sensitive rate */
	u32 freq_table[DVFS_MAX_FREQ_NUM];
	u32 volt_table[DVFS_MAX_VOLT_NUM];
	u32 sensitive_level;
	u32 perivolt_poll_id;
	u32 sensitive_pll;
	u32 always_on;
	u32 sync_time;
};
#endif
