/*
 * clk-kirin-common.h
 *
 * Hisilicon clock common definition
 *
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
#ifndef __LINUX_CLK_KIRIN_COMMON_H_
#define __LINUX_CLK_KIRIN_COMMON_H_

#include <linux/clk-provider.h>
#include <linux/compiler_types.h>
#include <linux/raid/pq.h>
#include <linux/spinlock_types.h>
#include <linux/of.h>

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
	LITTLE_CLUSRET = 0,
	BIG_CLUSTER,
	GPU,
	DDR,
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

#define LPM3_CMD_LEN			2
#define DVFS_MAX_VOLT			3
#define DVFS_MAX_FREQ_NUM		3
#define DVFS_MAX_VOLT_NUM		4
#define DDR_HW_VOTE			1
#define PERI_AVS_LOOP_MAX		400
#define DDRC_MIN_CLK_ID			4
#define DDRC_MAX_CLK_ID			5
#define DMSS_MIN_CLK_ID			6

struct hi3xxx_periclk {
	struct clk_hw	hw;
	void __iomem	*enable; /* enable register */
	void __iomem	*reset;  /* reset register */
	u32		ebits;   /* bits in enable/disable register */
	u32		rbits;   /* bits in reset/unreset register */
	void __iomem	*sctrl;  /* sysctrl addr */
	void __iomem	*pmctrl; /* pmctrl addr */
	const char	*friend;
	spinlock_t *lock;
	u32		flags;
	struct hwspinlock *clk_hwlock;
	u32		peri_dvfs_sensitive; /* 0:non,1:direct avs,rate(HZ):sensitive rate */
	u32		freq_table[DVFS_MAX_FREQ_NUM];
	u32		volt_table[DVFS_MAX_VOLT_NUM];
	u32		sensitive_level;
	u32		perivolt_poll_id;
	u32		sensitive_pll;
	u32		always_on;
	u32		sync_time;
	u32		clock_id;
	int		pmu_clk_enable;
	unsigned int	(*clk_pmic_read)(int clk_enable);
	void		(*clk_pmic_write)(int clk_enable, int val);
};

struct hi3xxx_divclk {
	struct clk_hw hw;
	void __iomem *reg; /* divider register */
	u8 shift;
	u8 width;
	u32 mbits;         /* mask bits in divider register */
	const struct clk_div_table *table;
#ifdef CONFIG_HISI_CLK_WAIT_DONE
	void __iomem *div_done;
	u32 done_bit;
#endif
	spinlock_t *lock;
};

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
#define AVS_ENABLE_PLL			1
#define AVS_DISABLE_PLL			0
/* AVS DEFINE END */

#ifndef CLOCK_MDEBUG_LEVEL
#define CLOCK_MDEBUG_LEVEL		0
#endif

#ifndef CLK_LOG_TAG
#define CLK_LOG_TAG	"clock"
#endif

#if CLOCK_MDEBUG_LEVEL
#define clk_log_dbg(fmt, ...) \
	pr_err("[%s]%s: " fmt "\n", CLK_LOG_TAG, __func__, ##__VA_ARGS__)
#else
#define clk_log_dbg(fmt, ...)
#endif

#ifdef CONFIG_HISI_CLK_DEBUG
char *hs_base_addr_transfer(unsigned int long base_addr);
#endif


#ifdef CONFIG_HISI_CLK
int is_fpga(void);
#endif

extern int clk_core_prepare(struct clk_core *core);
extern void clk_core_unprepare(struct clk_core *core);
extern int clk_set_rate_nolock(struct clk *clk, unsigned long rate);
extern int __clk_enable(struct clk *clk);
extern void __clk_disable(struct clk *clk);

/* ********** DEFINE END **************** */
void __iomem *hs_clk_base(u32 ctrl);
void __iomem __init *hs_clk_get_base(struct device_node *np);
const char *__hi3xxx_clk_get_parent_name(struct device_node *np);

struct clk_init_data *hisi_clk_init_data_alloc(const char *clk_name);

u32 hisi_clk_crg_type_get(struct device_node *np, const u32 default_crg_type);
unsigned int mul_valid_cal(unsigned long freq, unsigned long freq_conversion);
struct hs_clk *get_hs_clk_info(void);

#endif
