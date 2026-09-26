/*
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __CLK_DVFS_H__
#define __CLK_DVFS_H__

#include <linux/version.h>
#include <linux/clk.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hwspinlock.h>
#include <soc_crgperiph_interface.h>

#include "clk-mailbox.h"
#include "clk.h"
#ifdef CONFIG_HISI_PERIDVFS
#include "dvfs/peri_volt_poll.h"
#endif

#define MAX_FREQ_NUM	2
#define MAX_TRY_NUM	40
#define MUX_SOURCE_NUM	4
#define DIV_MUX_DATA_LENGTH	3
#define MUX_MAX_BIT	15
#define FOUR_BITS	0xf
#define FREQ_OFFSET_ADD	1000000  /* For 184.444M to 185M */
#define LOW_TEMPERATURE_PROPERTY	1
#define NORMAL_TEMPRATURE	0
#define ELOW_TEMPERATURE	0xE558
#define USB_POLL_ID	28
#define PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT	26
#define PMCTRL_PERI_CTRL4_TEMPERATURE_MASK	0xC000000
#define HWLOCK_TIMEOUT	1000
#define FREQ_CONVERSION	1000

#ifdef CONFIG_HISI_PERIDVFS
static DEFINE_MUTEX(dvfs_lock);
struct peri_dvfs_switch_up {
	struct work_struct updata;
	struct clk *clk;
	struct clk *linkage;
	struct peri_volt_poll *pvp;
	unsigned int target_volt;
	unsigned long target_freq;
	unsigned long divider_rate;
	const char *enable_pll_name;
};
#endif

struct peri_dvfs_clk {
	struct clk_hw hw;
	void __iomem *reg_base; /* sctrl register */
	u32 id;
	int avs_poll_id; /* the default value of those no avs feature clk is -1 */
	unsigned long freq_table[MAX_FREQ_NUM];
	u32 volt[MAX_FREQ_NUM];
	const char *link;
	unsigned long rate;
	unsigned long sensitive_freq[DVFS_MAX_FREQ_NUM];
	unsigned int sensitive_volt[DVFS_MAX_VOLT_NUM];
	unsigned long low_temperature_freq;
	unsigned int low_temperature_property;
	u32 sensitive_level;
	u32 block_mode;
	u32 div;
	u32 div_bits;
	u32 div_bits_offset;
	u32 mux;
	u32 mux_bits;
	u32 recal_mode;
	u32 divider; /* sw clock need to div 2 or 3 when set_rate */
	/* whether need to enable pll before set rate when clk is enabled */
	const char *enable_pll_name;
	u32 mux_bits_offset;
	const char **parent_names;
#ifdef CONFIG_HISI_PERIDVFS
	struct peri_dvfs_switch_up sw_up;
#endif
};

#endif
