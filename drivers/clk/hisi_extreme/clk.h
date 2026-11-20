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

#ifndef __CLK_H
#define __CLK_H
#include <linux/kernel.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "clk-mailbox.h"
#ifdef CONFIG_HISI_CLK_DEBUG
#include "debug/clk-debug.h"
#endif

#ifndef CLOCK_MDEBUG_LEVEL
#define CLOCK_MDEBUG_LEVEL	0
#endif
#ifndef CLK_LOG_TAG
#define CLK_LOG_TAG		"clock"
#endif

#if CLOCK_MDEBUG_LEVEL
#define clk_log_dbg(fmt, ...) \
	pr_err("[%s]%s: " fmt, CLK_LOG_TAG, __func__, ##__VA_ARGS__)
#else
#define clk_log_dbg(fmt, ...)
#endif

enum {
	HS_PMCTRL,
	HS_SYSCTRL,
	HS_CRGCTRL,
	HS_PMUCTRL,
	HS_PCTRL,
	HS_MEDIACRG,
	HS_IOMCUCRG,
	HS_MEDIA1CRG,
	HS_MEDIA2CRG,
	HS_MMC1CRG,
	HS_HSDTCRG,
	HS_MMC0CRG,
	HS_HSDT1CRG,
	HS_MAX_BASECRG,
};

enum {
	LITTLE_CLUSRET_BIT = 0,
	BIG_CLUSTER_BIT,
	GPU_BIT,
	DDR_BIT,
};

enum {
	CPU_CLUSTER_0 = 0,
	CPU_CLUSTER_1,
	CLK_G3D,
	CLK_DDRC_FREQ,
	CLK_DDRC_MAX,
	CLK_DDRC_MIN,
	CLK_DMSS_MIN,
};

enum {
	HS_UNBLOCK_MODE,
	HS_BLOCK_MODE,
};

#define NORMAL_ON		0
#define ALWAYS_ON		1

#define CLK_GATE_ALWAYS_ON_MASK		0x4

#define LPM3_CMD_LEN		2
#define PLL_REG_NUM		2
#define DVFS_MAX_VOLT		3
#define DVFS_MAX_FREQ_NUM		3
#define DVFS_MAX_VOLT_NUM		4
#define PERI_AVS_LOOP_MAX		400
#define INVALID_HWSPINLOCK_ID		0xFF

#define CLKGATE_COUNT		2

/* fast dvfs clk define */
#define PROFILE_CNT		5
#define PLL_CNT		4
#define GATE_CFG_CNT		2
#define SW_DIV_CFG_CNT		3

#ifndef CONFIG_HISI_CLK_ALWAYS_ON
#define CLK_GATE_DISABLE_OFFSET		0x4
#endif
#define CLK_GATE_STATUS_OFFSET		0x8
struct hs_clk {
	void __iomem *pmctrl;
	void __iomem *sctrl;
	void __iomem *crgctrl;
	void __iomem *pmuctrl;
	void __iomem *pctrl;
	void __iomem *mediacrg;
	void __iomem *iomcucrg;
	void __iomem *media1crg;
	void __iomem *media2crg;
	void __iomem *mmc1crg;
	void __iomem *hsdtcrg;
	void __iomem *mmc0crg;
	void __iomem *hsdt1crg;
	spinlock_t lock;
};

/* AVS DEFINE BEGIN */
enum {
	AVS_ICS = 1,
	AVS_ICS2,
	AVS_ISP,
	/* DSS AVS 4 no need */
	AVS_VENC = 5,
	AVS_VDEC,
	AVS_IVP,
	AVS_MAX_ID,
};
#define AVS_BITMASK_FLAG		28
#define SC_SCBAKDATA24_ADDR		0x46C
#define AVS_ENABLE_PLL		1
#define AVS_DISABLE_PLL		0
/* AVS DEFINE END */

struct clock_data {
	struct clk_onecell_data	clk_data;
	void __iomem	*base;
};

struct fixed_rate_clock {
	unsigned int	id;
	char *name;
	const char *parent_name;
	unsigned long	flags;
	unsigned long	fixed_rate;
	const char *alias;
};

struct fixed_factor_clock {
	unsigned int	id;
	char *name;
	const char *parent_name;
	unsigned long	mult;
	unsigned long	div;
	unsigned long	flags;
	const char *alias;
};

struct pll_clock {
	u32	id;
	const char *name;
	const char *parent_name;
	u32	pll_id;
	u32	en_ctrl[PLL_REG_NUM];
	u32	gt_ctrl[PLL_REG_NUM];
	u32	bypass_ctrl[PLL_REG_NUM];
	u32	st_ctrl[PLL_REG_NUM]; /* PLL state reg_offset, lock bit */
	u32	pll_st_addr; /* PLL state reg_base */
	u32	pll_en_method; /* 0: PERICRG Vote; 1: PLL_CTLR Vote */
	const char *alias;
};

struct mux_clock {
	unsigned int	id;
	const char *name;
	const char * const *parent_names;
	u8	num_parents;
	unsigned long	offset;
	u32	mux_mask;
	u8	mux_flags;
	const char *alias;
};

struct div_clock {
	unsigned int	id;
	const char *name;
	const char *parent_name;
	unsigned long	offset;
	u32	div_mask;
	unsigned int	max_div;
	unsigned int	min_div;
	unsigned int	multiple;
#ifdef CONFIG_HISI_CLK_WAIT_DONE
	unsigned long	div_done;
	unsigned int	done_bit;
#endif
	const char	*alias;
};

struct gate_clock {
	unsigned int	id;
	const char *name;
	const char *parent_name;
	unsigned int	offset;
	u32	bit_mask;
	u32	always_on;
	const char	*friend;
	u32	peri_dvfs_sensitive;
	u32	freq_table[DVFS_MAX_FREQ_NUM];
	u32	volt_table[DVFS_MAX_FREQ_NUM+1];
	u32	sensitive_level;
	u32	perivolt_poll_id;
	u32	sync_time;
	const char *alias;
};

struct scgt_clock {
	unsigned int	id;
	const char *name;
	const char *parent_name;
	unsigned long	offset;
	u8	bit_idx;
	u8	gate_flags;
	const char *alias;
};

struct xfreq_clock {
	u32	id;
	const char *name;
	u32	clock_id;
	u32	vote_method;
	u32	ddr_mask;
	u32	scbakdata;
	u32	set_rate_cmd[LPM3_CMD_LEN];
	u32	get_rate_cmd[LPM3_CMD_LEN];
	const char *alias;
};

struct mailbox_clock {
	u32	id;
	const char *name;
	const char *parent_name;
	u32	always_on;
	u32	en_cmd[LPM3_CMD_LEN];
	u32	dis_cmd[LPM3_CMD_LEN];
	u32	gate_abandon_enable;
	const char *alias;
};

struct pmu_clock {
	u32	id;
	const char *name;
	const char *parent_name;
	u32	offset; /* bits in enable/disable register */
	u32	bit_mask;
	u32	hwlock_id;
	u32	always_on;
	const char *alias;
};

struct dvfs_clock {
	u32	id;
	const char *name;
	const char *link;
	u32	devfreq_id;
	int	avs_poll_id; /* the default value of those no avs feature clk is -1 */
	u32	sensitive_level;
	u32	block_mode;
	unsigned long	sensitive_freq[DVFS_MAX_FREQ_NUM];
	unsigned int	sensitive_volt[DVFS_MAX_VOLT_NUM];
	unsigned long	low_temperature_freq;
	unsigned int	low_temperature_property;
	const char *alias;
};

struct fast_dvfs_clock {
	u32	id;
	const char *name;
	u32	clksw_offset[SW_DIV_CFG_CNT]; /* offset mask start_bit */
	u32	clkdiv_offset[SW_DIV_CFG_CNT]; /* offset mask start_bit */
	u32	clkgt_cfg[GATE_CFG_CNT]; /* offset value */
	u32	clkgate_cfg[GATE_CFG_CNT]; /* offset value */
	u32	pll_profile[PLL_CNT];
	u32	pll_name_id[PROFILE_CNT];
	u32	p_value[PROFILE_CNT]; /* profile value */
	u32	pll_profile_id[PROFILE_CNT]; /* pll profile id */
	u32	p_sw_cfg[PROFILE_CNT]; /* profile sw cfg */
	u32	div_val[PROFILE_CNT]; /* profile div val */
	u32	p_div_cfg[PROFILE_CNT]; /* profile div cfg */
	u32	always_on;
	const char *alias;
};

struct clk_test {
	const char *clk_name;
	unsigned long	rate;
};

struct clock_data *clk_init(struct device_node *np, int nr_clks);
void clk_data_init(struct clk *clk, const char *alias,
	u32 id, struct clock_data *data);

void plat_clk_register_fixed_rate(const struct fixed_rate_clock *clks,
		int nums, struct clock_data *data);
void plat_clk_register_fixed_factor(const struct fixed_factor_clock *clks,
		int nums, struct clock_data *data);
void plat_clk_register_mux(const struct mux_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_clkpmu(const struct pmu_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_divider(const struct div_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_xfreq(const struct xfreq_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_pll(const struct pll_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_gate(const struct gate_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_scgt(const struct scgt_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_dvfs_clk(const struct dvfs_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_fast_dvfs_clk(const struct fast_dvfs_clock *clks,
	int nums, struct clock_data *data);
void plat_clk_register_mclk(const struct mailbox_clock *clks,
	int nums, struct clock_data *data);

extern int clk_core_prepare(struct clk_core *core);
extern void clk_core_unprepare(struct clk_core *core);
extern int clk_set_rate_nolock(struct clk *clk, unsigned long rate);
extern int __clk_enable(struct clk *clk);
extern void __clk_disable(struct clk *clk);
void __iomem *hs_clk_base(u32 ctrl);

#ifdef CONFIG_HISI_CLK
int is_fpga(void);
#endif

#ifdef CONFIG_HISI_CLK_DEBUG
int is_no_media(void);
char *hs_base_addr_transfer(long unsigned int base_addr);
#endif

unsigned int mul_valid_cal(unsigned long freq, unsigned long freq_conversion);

void show_root_orphan_list(void);
struct hs_clk *get_hs_clk_info(void);

#endif /* __CLK_H */
