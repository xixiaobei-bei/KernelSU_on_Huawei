/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "clk-debug.h"
#include "../clk.h"
#include "../clk-dvfs.h"
#include "../clk-pmu.h"
#include "../plat-clk-div.h"
#include "../clk-fast-dvfs.h"
#include "../plat-clk-gate.h"
#include "../clk-ppll.h"

#define REG_VADDR_MAP(phyAddr)          ioremap(phyAddr, sizeof(unsigned long))
#define REG_VADDR_UNMAP(virAddr)        iounmap(virAddr)

#define clklog_err(fmt, args...)  \
	pr_err("[clk_test] [%s]" fmt, __func__, ## args)
#define clklog_info(fmt, args...) \
	pr_info("[clk_test] [%s]" fmt, __func__, ## args)
#define clklog_debug(fmt, args...) do { } while (0)
#define seq_clklog(s, fmt, args...)                              \
	do {                                                     \
		if (s)                                           \
			seq_printf(s, fmt, ## args);             \
		else                                             \
			pr_err("[clk_test] " fmt, ## args); \
	} while (0)

/* check kernel data state matched to register */
#define MATCH                            0
#define PARTIAL_MATCH                   (-2)
#define MISMATCH                        (-1)

struct clock_check_param {
	struct seq_file *s;
	struct device_node *np;
	struct clk_core *clk;
	int level;
	unsigned int parent_idx;
};

#define CLOCK_REG_GROUPS   4
struct clock_print_info {
	const char *clock_name;
	const char *clock_type;
	unsigned long rate;
	int clock_reg_groups;
	const char *crg_region[CLOCK_REG_GROUPS];
	int region_stat[CLOCK_REG_GROUPS];
	unsigned int reg[CLOCK_REG_GROUPS];
	unsigned char h_bit[CLOCK_REG_GROUPS];
	unsigned char l_bit[CLOCK_REG_GROUPS];
	unsigned int reg_val[CLOCK_REG_GROUPS];
	unsigned int enable_cnt;
	unsigned int prepare_cnt;
	int check_state;
	int parent_idx;
};

#define CLOCK_TYPE_NAME_LEN 64
#define CLOCK_TYPE_ALIAS_LEN 32
struct clock_type {
	char name[CLOCK_TYPE_NAME_LEN];
	int (*check_func)(struct clock_check_param *);
	char alias[CLOCK_TYPE_ALIAS_LEN];
};

static int check_fixed_rate(struct clock_check_param *param);
static int check_general_gate(struct clock_check_param *param);
static int check_himask_gate(struct clock_check_param *param);
static int check_mux_clock(struct clock_check_param *param);
static int check_div_clock(struct clock_check_param *param);
static int check_fixed_div_clock(struct clock_check_param *param);
static int check_pll_clock(struct clock_check_param *param);
static int check_pmu_clock(struct clock_check_param *param);
static int check_ipc_clock(struct clock_check_param *param);
static int check_root_clock(struct clock_check_param *param);
static int check_dvfs_clock(struct clock_check_param *param);
static int check_fast_clock(struct clock_check_param *param);

#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
static int peri_power_up_ok(void);
#endif
#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
static int sctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_PMC_BASE_ADDR)
static int pmctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
static int pctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
static int media1_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
static int media2_power_up_ok(void);
#endif
#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
static int iomcu_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
static int mmc1sysctrl_power_up_ok(void);
#endif
#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
static int mmc0_power_up_ok(void);
#endif
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR) || defined(SOC_ACPU_HSDT0_CRG_BASE_ADDR)
static int hsdt_power_up_ok(void);
#endif
#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
static int hsdt1_power_up_ok(void);
#endif
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
static int pmuspmi_power_up_ok(void);
#endif

const struct clock_type clock_types[] = {
	{ "fixed-clock",                check_fixed_rate,       "Fixed-Rate"   },
	{ "hisilicon,hi3xxx-clk-gate",  check_general_gate,     "Gate"         },
	{ "hisilicon,clk-gate",         check_himask_gate,      "Himask-Gate"  },
	{ "hisilicon,hi3xxx-clk-mux",   check_mux_clock,        "Mux(SW)"      },
	{ "hisilicon,hi3xxx-clk-div",   check_div_clock,        "Div"          },
	{ "fixed-factor-clock",         check_fixed_div_clock,  "Fixed-Factor" },
	{ "hisilicon,kirin-ppll-ctrl",  check_pll_clock,        "PPLL"         },
	{ "hisilicon,clk-pmu-gate",     check_pmu_clock,        "PMU-Gate"     },
	{ "hisilicon,interactive-clk",  check_ipc_clock,        "IPC-Gate"     },
	{ "hisilicon,hi3xxx-xfreq-clk", check_root_clock,       "Root"         },
	{ "hisilicon,clkdev-dvfs",      check_dvfs_clock,       "DVFS"         },
	{ "hisilicon,hi3xxx-fast-dvfs", check_fast_clock,       "FAST-DVFS"    },
};

/* CRG Regions Types */
enum {
#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	CLOCK_CRG_PERI,
#endif
#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
	CLOCK_CRG_SCTRL,
#endif
#if defined(SOC_ACPU_PMC_BASE_ADDR)
	CLOCK_CRG_PMCTRL,
#endif
#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
	CLOCK_CRG_PCTRL,
#endif
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
	CLOCK_CRG_MEDIA1,
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
	CLOCK_CRG_MEDIA2,
#endif
#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
	CLOCK_CRG_IOMCU,
#endif
#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
	CLOCK_CRG_MMC1,
#endif
#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
	CLOCK_CRG_MMC0,
#endif
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR) || defined(SOC_ACPU_HSDT0_CRG_BASE_ADDR)
	CLOCK_CRG_HSDT,
#endif
#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
	CLOCK_CRG_HSDT1,
#endif
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
	CLOCK_CRG_PMU,
#endif
};

#define CRG_NAME_LEN 16
struct crg_region {
	unsigned int base_addr;
	int (*region_ok)(void);
	char crg_name[CRG_NAME_LEN];
};

const struct crg_region crg_regions[] = {
#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	{ SOC_ACPU_PERI_CRG_BASE_ADDR,      peri_power_up_ok,        "PERI"   },
#endif
#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
	{ SOC_ACPU_SCTRL_BASE_ADDR,         sctrl_power_up_ok,       "SCTRL"  },
#endif
#if defined(SOC_ACPU_PMC_BASE_ADDR)
	{ SOC_ACPU_PMC_BASE_ADDR,           pmctrl_power_up_ok,      "PMCTRL" },
#endif
#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
	{ SOC_ACPU_PCTRL_BASE_ADDR,         pctrl_power_up_ok,       "PCTRL"  },
#endif
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
	{ SOC_ACPU_MEDIA1_CRG_BASE_ADDR,    media1_power_up_ok,      "MEDIA1" },
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
	{ SOC_ACPU_MEDIA2_CRG_BASE_ADDR,    media2_power_up_ok,      "MEDIA2" },
#endif
#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
	{ SOC_ACPU_IOMCU_CONFIG_BASE_ADDR,  iomcu_power_up_ok,       "IOMCU"  },
#endif
#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
	{ SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR, mmc1sysctrl_power_up_ok, "MMC1"   },
#endif
#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
	{ SOC_ACPU_MMC0_CRG_BASE_ADDR,      mmc0_power_up_ok,        "MMC0"   },
#endif
#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR)
	{ SOC_ACPU_HSDT_CRG_BASE_ADDR,      hsdt_power_up_ok,        "HSDT"   },
#elif defined(SOC_ACPU_HSDT0_CRG_BASE_ADDR)
	{ SOC_ACPU_HSDT0_CRG_BASE_ADDR,     hsdt_power_up_ok,        "HSDT"  },
#endif
#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
	{ SOC_ACPU_HSDT1_CRG_BASE_ADDR,     hsdt1_power_up_ok,       "HSDT1"  },
#endif
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
	{ SOC_ACPU_SPMI_BASE_ADDR,          pmuspmi_power_up_ok,     "PMU"    },
#endif
};
const int crg_region_size = ARRAY_SIZE(crg_regions);

static int read_reg_u32(unsigned int hwaddr, unsigned int *value)
{
	void __iomem *reg = NULL;

	reg = REG_VADDR_MAP(hwaddr); /*lint !e446*/
	if (!IS_ERR_OR_NULL(reg)) {
		if (value)
			*value = readl(reg);
		REG_VADDR_UNMAP(reg);
	} else {
		clklog_err("ioremap 0x%x error!\n", hwaddr);
		return -ENOMEM;
	}

	return 0;
}

static inline int update_line_offset(int offset, int count)
{
	if (count > 0)
		return offset + count;
	return offset;
}

static const char *get_clock_matched_state(int check_state)
{
	if (check_state == MATCH)
		return "[Y]";
	else if (check_state == PARTIAL_MATCH)
		return "[P]";
	else
		return "[N]";
}

#define CLOCK_PARENT_IDX_OFFSET  0x0000ffff
#define MAX_STR_BUF_SIZE              256
#define LINE_LEFT(offset)                      \
	(((MAX_STR_BUF_SIZE - (offset)) > 1) ? \
	(MAX_STR_BUF_SIZE - (offset)) : 1)
#define MAX_BIT_SIZE 32
#define MAX_LEVEL_NUM 16
static void print_clock_info(struct seq_file *s,
	const struct clock_print_info *info, int level)
{
	char line[MAX_STR_BUF_SIZE] = {0};
	int offset = 0;
	int ret, i;

	if (level > MAX_LEVEL_NUM)
		return;
	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%*s%-*s", level * 3 + 1,
		(info->parent_idx >= CLOCK_PARENT_IDX_OFFSET ? "*" : ""),
		50 - level * 3, info->clock_name); /* 50: length, 3: space */
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%-13s", info->clock_type);
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-10lu", info->rate);
	offset = update_line_offset(offset, ret);

	if (info->clock_reg_groups > 0) {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-6s%-5s", info->crg_region[0],
			(info->region_stat[0] ? "" : "[OFF]"));
		offset = update_line_offset(offset, ret);

		if (info->h_bit[0] >= MAX_BIT_SIZE ||
		    info->l_bit[0] >= MAX_BIT_SIZE) {
			ret = snprintf_s(line + offset,
				LINE_LEFT(offset), LINE_LEFT(offset) - 1,
				" %-17s %-10s", "NA", "NA");
			offset = update_line_offset(offset, ret);
		} else {
			ret = snprintf_s(line + offset, LINE_LEFT(offset),
				LINE_LEFT(offset) - 1, " 0x%08X[%2u:%-2u]",
				info->reg[0], info->h_bit[0], info->l_bit[0]);
			offset = update_line_offset(offset, ret);

			ret = snprintf_s(line + offset,
				LINE_LEFT(offset), LINE_LEFT(offset) - 1,
				" 0x%08X", info->reg_val[0]);
			offset = update_line_offset(offset, ret);
		}
	} else {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-11s %-17s %-10s",
			"NA", "NA", "NA");
		offset = update_line_offset(offset, ret);
	}

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, " %-6u", info->enable_cnt);
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, " %-5u", info->prepare_cnt);
	offset = update_line_offset(offset, ret);

	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, " %-3s\n",
		get_clock_matched_state(info->check_state));
	seq_clklog(s, "%s", line);

	for (i = 1; i < info->clock_reg_groups; i++) {
		offset = 0;
		(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%75s", "");
		offset = update_line_offset(offset, ret);

		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " %-6s%-5s", info->crg_region[i],
			(info->region_stat[i] ? "" : "[OFF]"));
		offset = update_line_offset(offset, ret);

		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " 0x%08X[%2u:%-2u]",
			info->reg[i], info->h_bit[i], info->l_bit[i]);
		offset = update_line_offset(offset, ret);

		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, " 0x%08X\n", info->reg_val[i]);
		if (ret == -1)
			pr_err("%s info->reg_val[i] snprintf_s failed!\n",
				__func__);

		seq_clklog(s, "%s", line);
	}
}

#if defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
static int peri_power_up_ok(void)
{
	/* Peri should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_SCTRL_BASE_ADDR)
static int sctrl_power_up_ok(void)
{
	/* sctrl always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_PMC_BASE_ADDR)
static int pmctrl_power_up_ok(void)
{
	/* pmctrl should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_PCTRL_BASE_ADDR)
static int pctrl_power_up_ok(void)
{
	/* pctrl should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
static int media1_power_up_ok(void)
{
	int ret = 0;
#if defined(SOC_CRGPERIPH_PERRSTSTAT5_ADDR) && \
	defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	unsigned int value;
	unsigned int hwaddr;

	hwaddr = SOC_CRGPERIPH_PERRSTSTAT5_ADDR(SOC_ACPU_PERI_CRG_BASE_ADDR);
	ret = read_reg_u32(hwaddr, &value);
	if (ret)
		return ret;
#if defined(SOC_CRGPERIPH_PERRSTSTAT5_ip_rst_media_crg_START)
	ret = !(value & BIT(SOC_CRGPERIPH_PERRSTSTAT5_ip_rst_media_crg_START));
#endif
#endif
	return ret;
}
#endif

#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
static int media2_power_up_ok(void)
{
	int ret = 0;
#if defined(SOC_CRGPERIPH_PERRSTSTAT4_ADDR) && \
	defined(SOC_ACPU_PERI_CRG_BASE_ADDR)
	unsigned int value;
	unsigned int hwaddr;

	hwaddr = SOC_CRGPERIPH_PERRSTSTAT4_ADDR(SOC_ACPU_PERI_CRG_BASE_ADDR);
	ret = read_reg_u32(hwaddr, &value);
	if (ret)
		return ret;
#if defined(SOC_CRGPERIPH_PERRSTSTAT4_ip_rst_media2_crg_START)
	ret = !(value & BIT(SOC_CRGPERIPH_PERRSTSTAT4_ip_rst_media2_crg_START));
#endif
#endif
	return ret;
}
#endif

#if defined(SOC_ACPU_IOMCU_CONFIG_BASE_ADDR)
static int iomcu_power_up_ok(void)
{
	/* iomcu is always on while system is normal */
	return 1;
}
#endif

#if defined(SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR)
static int mmc1sysctrl_power_up_ok(void)
{
	/* MMC1 should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_MMC0_CRG_BASE_ADDR)
static int mmc0_power_up_ok(void)
{
	/* MMC0 should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_HSDT_CRG_BASE_ADDR) || defined(SOC_ACPU_HSDT0_CRG_BASE_ADDR)
static int hsdt_power_up_ok(void)
{
	/* HSDT should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_HSDT1_CRG_BASE_ADDR)
static int hsdt1_power_up_ok(void)
{
	/* HSDT1 should be considered always on */
	return 1;
}
#endif

#if defined(SOC_ACPU_SPMI_BASE_ADDR)
static int pmuspmi_power_up_ok(void)
{
	/* PMU/SPMI always on */
	return 1;
}
#endif

static int get_base_hwaddr(struct device_node *np)
{
	struct of_phandle_args clkspec;
	int rc, i;
	u64 hwaddr;
	const unsigned int *in_addr = NULL;
	int region_idx = -1;

	rc = of_parse_phandle_with_args(np, "clocks", "#clock-cells",
		0, &clkspec);
	if (rc)
		return rc;

	in_addr = of_get_address(clkspec.np, 0, NULL, NULL);
	if (!in_addr) {
		clklog_err("node %s of_get_address get I/O addr err!\n",
			clkspec.np->name);
		return -EINVAL;
	}
	hwaddr = of_translate_address(clkspec.np, in_addr);
	if (hwaddr == OF_BAD_ADDR)
		clklog_err("node %s of_translate_address err!\n",
			clkspec.np->name);

	of_node_put(clkspec.np);

	for (i = 0; i < crg_region_size; i++) {
		if (crg_regions[i].base_addr == (unsigned int)hwaddr) {
			region_idx = i;
			break;
		}
	}

	return region_idx;
}

#define CHECK_REGION_IDX(region_idx) \
	(((region_idx) < 0) || ((region_idx) >= crg_region_size))

static int check_fixed_rate(struct clock_check_param *param)
{
	unsigned long fixed_rate = 0;
	unsigned long rate;
	int ret = MISMATCH;
	struct clock_print_info pinfo;
	struct clk_fixed_rate *fixed_clk = NULL;

	fixed_clk = container_of(param->clk->hw, struct clk_fixed_rate, hw);
	fixed_rate = fixed_clk->fixed_rate;

	rate = clk_get_rate(param->clk->hw->clk);
	if (rate == fixed_rate)
		ret = MATCH;

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_FIXED].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int __check_gate_state_matched(unsigned int reg_value,
	unsigned int gate_bitmask, unsigned int enable_count, bool always_on)
{
	int matched = MISMATCH;
	bool gate_onoff = ((reg_value & gate_bitmask) == gate_bitmask);
	bool enable_onoff = (enable_count > 0);

	if (gate_onoff && enable_onoff) {
		matched = MATCH;
	} else if (gate_onoff && !enable_onoff) {
		if (always_on)
			matched = MATCH;
		else
			matched = PARTIAL_MATCH;
	} else if (!gate_onoff && enable_onoff) {
		matched = MISMATCH;
	} else {
		matched = MATCH;
	}

	return matched;
}

static void __print_dvfs_extra_info(struct clock_check_param *param,
	bool is_dvfs_clock)
{
	int ret;
	int offset = 0;
	int device_id = -1;
	struct hi3xxx_periclk *pclk = NULL;
	struct peri_dvfs_clk *dclk = NULL;
#if defined(SOC_ACPU_PMC_BASE_ADDR)
	unsigned int hwaddr_pmctrl;
#endif
	unsigned int ctrl4_data = 0;
	unsigned int ctrl5_data = 0;
	char line[MAX_STR_BUF_SIZE] = {0};

	if (!is_dvfs_clock) {
		pclk = container_of(param->clk->hw, struct hi3xxx_periclk, hw);
		device_id = pclk->perivolt_poll_id;
	} else {
		dclk = container_of(param->clk->hw, struct peri_dvfs_clk, hw);
		device_id = dclk->id;
	}

#if defined(SOC_ACPU_PMC_BASE_ADDR)
	hwaddr_pmctrl = crg_regions[CLOCK_CRG_PMCTRL].base_addr;
	(void)read_reg_u32(SOC_PMCTRL_PERI_CTRL4_ADDR(hwaddr_pmctrl),
		&ctrl4_data);
	(void)read_reg_u32(SOC_PMCTRL_PERI_CTRL5_ADDR(hwaddr_pmctrl),
		&ctrl5_data);
#endif

	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%76s", "");
	offset = update_line_offset(offset, ret);
	ret = snprintf_s(line + offset,
		LINE_LEFT(offset), LINE_LEFT(offset) - 1,
		"%-12s%-6dCTRL4:0x%08X CTRL5:0x%08X", "#DVFS-ID:",
		device_id, ctrl4_data, ctrl5_data);
	if (ret == -1)
		pr_err("%s device_id, ctrl4_data, ctrl5_data snprintf_s failed!\n",
			__func__);
	if (line[0] != '\0')
		seq_clklog(param->s, "%s\n", line);
}

static void __print_avs_extra_info(struct clock_check_param *param)
{
	int ret;
	int offset = 0;
	int avs_poll_id;
	struct peri_dvfs_clk *dclk = NULL;
#if defined(SOC_ACPU_SCTRL_BASE_ADDR) && defined(SOC_SCTRL_SCBAKDATA24_ADDR)
	unsigned int hwaddr_sctrl;
#endif
	unsigned int avs_data = 0;
	char line[MAX_STR_BUF_SIZE] = {0};

	dclk = container_of(param->clk->hw, struct peri_dvfs_clk, hw);
	avs_poll_id = dclk->avs_poll_id;
	if (avs_poll_id < 0)
		return;

#if defined(SOC_ACPU_SCTRL_BASE_ADDR) && defined(SOC_SCTRL_SCBAKDATA24_ADDR)
	hwaddr_sctrl = crg_regions[CLOCK_CRG_SCTRL].base_addr;
	(void)read_reg_u32(SOC_SCTRL_SCBAKDATA24_ADDR(hwaddr_sctrl),
		&avs_data);
#endif

	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%76s", "");
	offset = update_line_offset(offset, ret);
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1,
		"%-12s%-6dAVS:  0x%08X", "#AVS-ID:", avs_poll_id, avs_data);
	if (ret == -1)
		pr_err("%s snprintf_s failed!\n", __func__);
	if (line[0] != '\0')
		seq_clklog(param->s, "%s\n", line);
}

static void printf_dvfs_info(struct clock_check_param *param, u32 *freq_table,
	u32 *volt_table, u32 sensitive_level)
{
	int ret;
	unsigned int i;
	char line[MAX_STR_BUF_SIZE] = {0};
	int offset = 0;

	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%76s%-12s", "", "#DVFS-Freq:");
	offset = update_line_offset(offset, ret);
	for (i = 0; i < sensitive_level; i++) {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "%-6uKHz ", freq_table[i]);
		offset = update_line_offset(offset, ret);
	}
	seq_clklog(param->s, "%s\n", line);

	offset = 0;
	(void)memset_s(line, MAX_STR_BUF_SIZE, '\0', sizeof(line));
	ret = snprintf_s(line + offset, LINE_LEFT(offset),
		LINE_LEFT(offset) - 1, "%76s%-12s", "", "#DVFS-Volt:");
	offset = update_line_offset(offset, ret);
	for (i = 0; i <= sensitive_level; i++) {
		ret = snprintf_s(line + offset, LINE_LEFT(offset),
			LINE_LEFT(offset) - 1, "VOLT_%-5u", volt_table[i]);
		offset = update_line_offset(offset, ret);
	}
	seq_clklog(param->s, "%s\n", line);
}

static void print_dvfs_avs_extra_info(struct clock_check_param *param,
	bool is_dvfs_clock)
{
	u32 freq_table[DVFS_MAX_FREQ_NUM] = {0};
	u32 volt_table[DVFS_MAX_FREQ_NUM + 1] = {0};
	u32 sensitive_level;
	struct hi3xxx_periclk *pclk = NULL;
	struct peri_dvfs_clk *dclk = NULL;
	unsigned int i;

	if (!IS_ENABLED(CONFIG_HISI_PERIDVFS))
		return;

	if (!is_dvfs_clock) {
		pclk = container_of(param->clk->hw, struct hi3xxx_periclk, hw);
		if (!pclk->peri_dvfs_sensitive)
			return;
		sensitive_level = pclk->sensitive_level;
		for (i = 0; i < DVFS_MAX_FREQ_NUM; i++) {
			freq_table[i] = pclk->freq_table[i];
			volt_table[i] = pclk->volt_table[i];
		}
		volt_table[i] = pclk->volt_table[i];
	} else {
		dclk = container_of(param->clk->hw, struct peri_dvfs_clk, hw);
		sensitive_level = dclk->sensitive_level;
		for (i = 0; i < DVFS_MAX_FREQ_NUM; i++) {
			freq_table[i] = (u32)dclk->sensitive_freq[i];
			volt_table[i] = (u32)dclk->sensitive_volt[i];
		}
		volt_table[i] = (u32)dclk->sensitive_volt[i];
	}

	__print_dvfs_extra_info(param, is_dvfs_clock);
	if (is_dvfs_clock)
		__print_avs_extra_info(param);

	printf_dvfs_info(param, freq_table, volt_table, sensitive_level);
}

#define REG_STATE_OFFSET 0x8
#define calcu_offset(config_value) (((uintptr_t)(config_value)) & 0xFFF)
static int check_general_gate(struct clock_check_param *param)
{
	struct hi3xxx_periclk *pclk = NULL;
	int matched = MISMATCH;
	unsigned int value = 0;
	unsigned long offset;
	int region_idx, region_stat;
	struct clock_print_info pinfo = {0};

	pclk = container_of(param->clk->hw, struct hi3xxx_periclk, hw);

	pinfo.rate = clk_get_rate(param->clk->hw->clk);
	region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n", param->clk->name);
		return -EINVAL;
	}

	offset =  calcu_offset(pclk->enable);
	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
			crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	/* crg region powered up */
	if (region_stat && pclk->ebits) {
		value = readl(pclk->enable + REG_STATE_OFFSET);
		matched = __check_gate_state_matched(value, pclk->ebits,
			param->clk->enable_count, pclk->always_on);
	} else {
		/* fake gate clock Or crg region power down */
		matched = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_GENERAL_GATE].alias;
	pinfo.clock_reg_groups   = 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = crg_regions[region_idx].base_addr + offset + REG_STATE_OFFSET;
	pinfo.h_bit[0]           = fls(pclk->ebits) - 1;
	pinfo.l_bit[0]           = ffs(pclk->ebits) - 1;
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.parent_idx         = param->parent_idx;
	pinfo.check_state        = matched;
	print_clock_info(param->s, &pinfo, param->level);
	print_dvfs_avs_extra_info(param, false);

	return (matched == MISMATCH ? MISMATCH : MATCH);
}

#define is_scgt_always_on(flags) (flags & CLK_GATE_ALWAYS_ON_MASK ? 1 : 0)
static int check_himask_gate(struct clock_check_param *param)
{
	struct clk_gate *scgt_clk = NULL;
	int region_idx, region_stat;
	int matched = MISMATCH;
	unsigned int value = 0;
	unsigned int offset;
	struct clock_print_info pinfo;

	scgt_clk = container_of(param->clk->hw, struct clk_gate, hw);

	pinfo.rate = clk_get_rate(param->clk->hw->clk);
	region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n", param->clk->name);
		return -EINVAL;
	}

	offset =  calcu_offset(scgt_clk->reg);

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
			crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	/* crg region powered up */
	if (region_stat) {
		value = readl(scgt_clk->reg);
		matched = __check_gate_state_matched(value, BIT(scgt_clk->bit_idx),
			param->clk->enable_count, is_scgt_always_on(scgt_clk->flags));
	} else {
		/* crg region power down */
		matched = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_HIMASK_GATE].alias;
	pinfo.clock_reg_groups   = 1;
	pinfo.h_bit[0]           = scgt_clk->bit_idx;
	pinfo.l_bit[0]           = scgt_clk->bit_idx;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = crg_regions[region_idx].base_addr + offset;
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.parent_idx         = param->parent_idx;
	pinfo.check_state        = matched;
	print_clock_info(param->s, &pinfo, param->level);

	return (matched == MISMATCH ? MISMATCH : MATCH);
}

static int __check_mux_state_matched(unsigned int reg_value, u8 shift, u8 mask,
	unsigned int parent_idx)
{
	int matched = MISMATCH;
	unsigned int mux_bitmask = mask << shift;

	if ((int)parent_idx >= CLOCK_PARENT_IDX_OFFSET)
		parent_idx -= CLOCK_PARENT_IDX_OFFSET;

	if ((reg_value & mux_bitmask) == (parent_idx << shift))
		matched = MATCH;

	return matched;
}

static int check_mux_clock(struct clock_check_param *param)
{
	struct clk_mux *mux_clk = NULL;
	int ret, region_idx, region_stat;
	unsigned int value = 0;
	unsigned int offset;
	struct clock_print_info pinfo;

	mux_clk = container_of(param->clk->hw, struct clk_mux, hw);

	pinfo.rate = clk_get_rate(param->clk->hw->clk);
	region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n", param->clk->name);
		return -EINVAL;
	}

	offset = calcu_offset(mux_clk->reg);
	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
			crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	/* crg region powered up */
	if (region_stat) {
		value = readl(mux_clk->reg);
		ret = __check_mux_state_matched(value, mux_clk->shift, mux_clk->mask,
			param->parent_idx);
	} else {
		/* crg region power down */
		ret = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_MUX].alias;
	pinfo.clock_reg_groups   = 1;
	pinfo.h_bit[0]           = fls(mux_clk->mask << mux_clk->shift) - 1;
	pinfo.l_bit[0]           = mux_clk->shift;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = crg_regions[region_idx].base_addr + offset;
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.parent_idx         = param->parent_idx;
	pinfo.check_state        = ret;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int __check_div_state_matched(unsigned int reg_value,
	unsigned int div_bitmask, unsigned long prate, unsigned long rate)
{
	int matched = MISMATCH;
	unsigned int low_bit = (unsigned int)ffs(div_bitmask);
	if (low_bit == 0)
		return -EINVAL;
	low_bit -= 1;

	if (!rate)
		return -EINVAL;

	if ((reg_value & div_bitmask) == ((prate / rate - 1) << low_bit))
		matched = MATCH;

	return matched;
}

#define div_mask(mbits) (mbits >> DIV_MASK_OFFSET)
static int check_div_clock(struct clock_check_param *param)
{
	unsigned long prate;
	struct hi3xxx_divclk *div_clk = NULL;
	struct clk_core *pclk = NULL;
	int ret, region_idx, region_stat;
	unsigned int value = 0;
	struct clock_print_info pinfo;

	div_clk = container_of(param->clk->hw, struct hi3xxx_divclk, hw);

	pclk = __clk_core_get_parent(param->clk);
	if (IS_ERR_OR_NULL(pclk)) {
		clklog_err("failed to get clk[%s] parent clock!\n", param->clk->name);
		return -EINVAL;
	}
	pinfo.rate = clk_get_rate(param->clk->hw->clk);
	prate = clk_get_rate(pclk->hw->clk);

	region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
			crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	/* crg region powered up */
	if (region_stat) {
		value = readl(div_clk->reg);
		ret = __check_div_state_matched(value, div_mask(div_clk->mbits), prate, pinfo.rate);
	} else {
		/* crg region power down */
		ret = MATCH;
	}

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_DIV].alias;
	pinfo.clock_reg_groups   = 1;
	pinfo.h_bit[0]           = fls(div_mask(div_clk->mbits)) - 1;
	pinfo.l_bit[0]           = ffs(div_mask(div_clk->mbits)) - 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.reg[0]             = crg_regions[region_idx].base_addr + calcu_offset(div_clk->reg);
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.parent_idx         = param->parent_idx;
	pinfo.check_state        = ret;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int check_fixed_div_clock(struct clock_check_param *param)
{
	unsigned long rate, prate;
	struct clk_core *pclk = NULL;
	struct clk_fixed_factor *fixed_div_clk = NULL;
	int ret = MISMATCH;
	struct clock_print_info pinfo;

	fixed_div_clk = container_of(param->clk->hw, struct clk_fixed_factor, hw);

	pclk = __clk_core_get_parent(param->clk);
	if (IS_ERR_OR_NULL(pclk)) {
		clklog_err("cannot get clk[%s] parent clock!\n",
			param->clk->name);
		return -EINVAL;
	}
	prate = clk_get_rate(pclk->hw->clk);
	rate = clk_get_rate(param->clk->hw->clk);
	if (!rate) {
		clklog_err("get clock[%s] rate is 0!\n", param->clk->name);
		return -EINVAL;
	}

	if (fixed_div_clk->div != 0 && rate == (prate * fixed_div_clk->mult / fixed_div_clk->div))
		ret = MATCH;

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_FIXED_DIV].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

/* check PPLL configs */
#define CHECK_PLL_CONFIG(param, ret, pll_lock_state) do {                           \
	(void)read_reg_u32(ppll_cfg.hwaddr_ctrl + ppll_cfg.st_addr[0], &ppll_cfg.pll_st); \
	(void)read_reg_u32(ppll_cfg.hwaddr + ppll_cfg.en_addr[0], &ppll_cfg.pll_en); \
	(void)read_reg_u32(ppll_cfg.hwaddr + ppll_cfg.gt_addr[0], &ppll_cfg.pll_gt); \
	(void)read_reg_u32(ppll_cfg.hwaddr + ppll_cfg.bp_addr[0], &ppll_cfg.pll_bp); \
	if ((ppll_cfg.pll_en & BIT(ppll_cfg.en_addr[1])) &&                    \
	    (ppll_cfg.pll_gt & BIT(ppll_cfg.gt_addr[1])) &&                    \
	   !(ppll_cfg.pll_bp & BIT(ppll_cfg.bp_addr[1])) &&                    \
	    pll_lock_state) {                                         \
		if ((param)->clk->enable_count > 0)                   \
			(ret) = MATCH;                                \
		else                                                  \
			(ret) = PARTIAL_MATCH;                        \
	} else {                                                      \
		if ((param)->clk->enable_count > 0)                   \
			(ret) = MISMATCH;                             \
		else                                                  \
			(ret) = MATCH;                                \
	}                                                             \
} while (0)

#define __REG_ADDR_OFFSET 2
struct ppll_check_param {
	u32 en_addr[__REG_ADDR_OFFSET];
	u32 gt_addr[__REG_ADDR_OFFSET];
	u32 bp_addr[__REG_ADDR_OFFSET];
	u32 st_addr[__REG_ADDR_OFFSET];
	unsigned int hwaddr;
	unsigned int hwaddr_ctrl;
	unsigned int pll_en;
	unsigned int pll_gt;
	unsigned int pll_bp;
	unsigned int pll_st;
	int region_idx;
	int pll_st_region_idx;
};

#ifndef CONFIG_HISI_PLL_VOTE_SEC
static void __read_ppll_clock_config(struct clock_check_param *param,
	struct ppll_check_param *cfg)
{
	struct hi3xxx_ppll_clk *pll_clk = NULL;

	pll_clk = container_of(param->clk->hw, struct hi3xxx_ppll_clk, hw);
	cfg->en_addr[0] = pll_clk->en_ctrl[0];
	cfg->en_addr[1] = pll_clk->en_ctrl[1];

	cfg->gt_addr[0] = pll_clk->gt_ctrl[0];
	cfg->gt_addr[1] = pll_clk->gt_ctrl[1];

	cfg->bp_addr[0] = pll_clk->bypass_ctrl[0];
	cfg->bp_addr[1] = pll_clk->bypass_ctrl[1];

	cfg->st_addr[0] = pll_clk->st_ctrl[0];
	cfg->st_addr[1] = pll_clk->st_ctrl[1];
}

static void __fill_pll_pinfo(struct clock_print_info *pinfo,
	struct ppll_check_param *cfg)
{
	pinfo->clock_reg_groups = CLOCK_REG_GROUPS;

	/* the first group regs */
	pinfo->crg_region[0]  = crg_regions[cfg->region_idx].crg_name;
	pinfo->region_stat[0] = 1;
	pinfo->reg[0]         = cfg->hwaddr + cfg->en_addr[0];
	pinfo->h_bit[0]       = cfg->en_addr[1];
	pinfo->l_bit[0]       = cfg->en_addr[1];
	pinfo->reg_val[0]     = cfg->pll_en;

	/* the second group regs */
	pinfo->crg_region[1]  = crg_regions[cfg->region_idx].crg_name;
	pinfo->region_stat[1] = 1;
	pinfo->reg[1]         = cfg->hwaddr + cfg->bp_addr[0];
	pinfo->h_bit[1]       = cfg->bp_addr[1];
	pinfo->l_bit[1]       = cfg->bp_addr[1];
	pinfo->reg_val[1]     = cfg->pll_bp;

	/* the third group regs */
	pinfo->crg_region[2]  = crg_regions[cfg->region_idx].crg_name;
	pinfo->region_stat[2] = 1;
	pinfo->reg[2]         = cfg->hwaddr + cfg->gt_addr[0];
	pinfo->h_bit[2]       = cfg->gt_addr[1];
	pinfo->l_bit[2]       = cfg->gt_addr[1];
	pinfo->reg_val[2]     = cfg->pll_gt;

	/* the fourth group regs */
	pinfo->crg_region[3]  = crg_regions[cfg->pll_st_region_idx].crg_name;
	pinfo->region_stat[3] = 1;
	pinfo->reg[3]         = cfg->hwaddr_ctrl + cfg->st_addr[0];
	pinfo->h_bit[3]       = cfg->st_addr[1];
	pinfo->l_bit[3]       = cfg->st_addr[1];
	pinfo->reg_val[3]     = cfg->pll_st;
}
#endif

static int check_kirin_ppll_clock(struct clock_check_param *param)
{
#ifndef CONFIG_HISI_PLL_VOTE_SEC
	struct ppll_check_param ppll_cfg = {{0}};
	int ret = MISMATCH;
	struct clock_print_info pinfo = {0};
	struct hi3xxx_ppll_clk *pll_clk = NULL;
	u32 reg_value, pll_lock_state;

	memset(&ppll_cfg, 0, sizeof(struct ppll_check_param));
	pinfo.clock_reg_groups = 0;
	__read_ppll_clock_config(param, &ppll_cfg);

	ppll_cfg.region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(ppll_cfg.region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n", param->clk->name);
		return -EINVAL;
	}

	pll_clk = container_of(param->clk->hw, struct hi3xxx_ppll_clk, hw);
	if (pll_clk->pll_id == AUPLL)
		ppll_cfg.pll_st_region_idx = 1;
	if (CHECK_REGION_IDX(ppll_cfg.pll_st_region_idx)) {
		clklog_err("get crg index error!\n");
		return -EINVAL;
	}
	ppll_cfg.hwaddr_ctrl = crg_regions[ppll_cfg.pll_st_region_idx].base_addr;
	ppll_cfg.hwaddr = crg_regions[ppll_cfg.region_idx].base_addr;

	if (pll_clk->pll_id != PPLL0) {
		reg_value = readl(pll_clk->endisable_addr + pll_clk->st_ctrl[0]);
		pll_lock_state = (reg_value & BIT(pll_clk->st_ctrl[1])) >> pll_clk->st_ctrl[1];
		CHECK_PLL_CONFIG(param, ret, pll_lock_state);
	} else {
		ret = MATCH;
	}

	pinfo.clock_reg_groups = CLOCK_REG_GROUPS;
	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_PPLL].alias;
	pinfo.rate               = clk_get_rate(param->clk->hw->clk);
	if (pinfo.clock_reg_groups == CLOCK_REG_GROUPS)
		__fill_pll_pinfo(&pinfo, &ppll_cfg);
	else if (pinfo.clock_reg_groups == 0)
		ret = MATCH;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return (ret == MISMATCH ? MISMATCH : MATCH);
#else
	return MATCH;
#endif
}

static void __fill_pll_info(struct hi3xxx_ppll_clk *pll_clk,
	struct clock_print_info *ppinfo, unsigned int hwaddr)
{
	u32 pll_id = pll_clk->pll_id;

	switch (pll_id) {
	case SCPLL:
		ppinfo->clock_reg_groups = 3; /* register groups */
		ppinfo->reg[0] = hwaddr + pll_clk->en_ctrl[0];
		ppinfo->reg_val[0] = (unsigned int)readl(pll_clk->addr + pll_clk->en_ctrl[0]);
		ppinfo->reg[1] = hwaddr + pll_clk->st_ctrl[0];
		ppinfo->reg_val[1] = (unsigned int)readl(pll_clk->addr + pll_clk->st_ctrl[0]);
		ppinfo->reg[2] = hwaddr + pll_clk->gt_ctrl[0];
		ppinfo->reg_val[2] = (unsigned int)readl(pll_clk->addr + pll_clk->gt_ctrl[0]);
		break;
	case PCIE0PLL:
	case PCIE1PLL:
		ppinfo->clock_reg_groups = 2; /* register groups */
		ppinfo->reg[0] = hwaddr + pll_clk->en_ctrl[0];
		ppinfo->reg_val[0] = (unsigned int)readl(pll_clk->addr + pll_clk->en_ctrl[0]);
		ppinfo->reg[1] = hwaddr + pll_clk->st_ctrl[0];
		ppinfo->reg_val[1] = (unsigned int)readl(pll_clk->addr + pll_clk->st_ctrl[0]);
		break;
	default:
		break;
	}
}

static int check_pll_clock(struct clock_check_param *param)
{
	unsigned int hwaddr;
	int region_idx, region_stat;
	struct hi3xxx_ppll_clk *pll_clk = NULL;
	struct clock_print_info pinfo = {0};

	pll_clk = container_of(param->clk->hw, struct hi3xxx_ppll_clk, hw);

	if (!pll_clk->en_method)
		return check_kirin_ppll_clock(param);

	region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n", param->clk->name);
		return -EINVAL;
	}

	region_stat = crg_regions[region_idx].region_ok();
	if (region_stat < 0) {
		clklog_err("check region %s err, err_code is %d\n",
			crg_regions[region_idx].crg_name, region_stat);
		return region_stat;
	}

	hwaddr = crg_regions[region_idx].base_addr;
	if (region_stat)
		__fill_pll_info(pll_clk, &pinfo, hwaddr);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_PPLL].alias;
	pinfo.rate               = clk_get_rate(param->clk->hw->clk);

	/* the first group regs */
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = region_stat;
	pinfo.h_bit[0]           = pll_clk->en_ctrl[1];
	pinfo.l_bit[0]           = pll_clk->en_ctrl[1];

	/* the second group regs */
	pinfo.crg_region[1]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[1]     = region_stat;
	pinfo.h_bit[1]           = pll_clk->st_ctrl[1];
	pinfo.l_bit[1]           = pll_clk->st_ctrl[1];

	/* the third group regs */
	pinfo.crg_region[2]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[2]     = region_stat;
	pinfo.h_bit[2]           = pll_clk->gt_ctrl[1];
	pinfo.l_bit[2]           = pll_clk->gt_ctrl[1];

	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = MATCH;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return MATCH;
}

static int check_pmu_clock(struct clock_check_param *param)
{
#if defined(SOC_ACPU_SPMI_BASE_ADDR)
	unsigned long rate;
	struct hi3xxx_clkpmu *pmu_clk = NULL;
	int matched = MISMATCH;
	int region_idx;
	unsigned int value;
	struct clock_print_info pinfo;

	pmu_clk = container_of(param->clk->hw, struct hi3xxx_clkpmu, hw);

	rate = clk_get_rate(param->clk->hw->clk);

	region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(region_idx)) {
		clklog_err("get clk[%s] [SPMI/PMU: 0x%x] base addr wrong!\n",
			param->clk->name,
			crg_regions[CLOCK_CRG_PMU].base_addr);
		region_idx = CLOCK_CRG_PMU;
	}

	value = (*pmu_clk->clk_pmic_read)(pmu_clk->pmu_clk_enable);
	matched = __check_gate_state_matched(value, pmu_clk->ebits,
		param->clk->enable_count, false);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_PMU].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 1;
	pinfo.crg_region[0]      = crg_regions[region_idx].crg_name;
	pinfo.region_stat[0]     = 1;
	pinfo.reg[0]             = pmu_clk->pmu_clk_enable;
	pinfo.h_bit[0]           = fls(pmu_clk->ebits) - 1;
	pinfo.l_bit[0]           = ffs(pmu_clk->ebits) - 1;
	pinfo.reg_val[0]         = value;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = matched;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return (matched == MISMATCH ? MISMATCH : MATCH);
#else
	return MATCH;
#endif
}

static int check_ipc_clock(struct clock_check_param *param)
{
	int ret = MATCH;
	unsigned long rate;
	struct clock_print_info pinfo;

	rate = clk_get_rate(param->clk->hw->clk);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_IPC].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int check_root_clock(struct clock_check_param *param)
{
	int ret = MATCH;
	unsigned long rate;
	struct clock_print_info pinfo;

	rate = clk_get_rate(param->clk->hw->clk);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_ROOT].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return ret;
}

static int clock_check(struct seq_file *s, struct clk_core *clk,
	unsigned int level, int parent_idx)
{
	int clock_type;
	struct clock_check_param param;

	param.s = s;
	param.clk = clk;
	param.level = level;
	param.parent_idx = parent_idx;
	param.np = of_find_node_by_name(NULL, clk->name);
	if (IS_ERR_OR_NULL(param.np)) {
		clklog_err("node %s doesn't find!\n", clk->name);
		return -EINVAL;
	}

	clock_type = get_clock_type(clk);
	if ((clock_type < 0) || (clock_type >= MAX_CLOCK_TYPE)) {
		clklog_err("no matched clock_type, clock_type = %d!\n",
			clock_type);
		return -EINVAL;
	}

	if (clock_types[clock_type].check_func)
		return clock_types[clock_type].check_func(&param);

	return -ENODEV;
}

static int find_clock_parent_idx(struct clk_core *clk)
{
	int i;
	struct clk_core *parent = NULL;

	if (IS_ERR_OR_NULL(clk))
		return -1;

	for (i = 0; i < clk->num_parents; i++) {
		parent = __clk_core_get_parent(clk);
		if (parent == NULL) {
			clklog_err("clock struct err!\n");
			return -EINVAL;
		}
		if (strcmp(parent->name, clk->parent_names[i]) == 0)
			return i;
	}

	return 0;
}

static int find_all_clock_sources(struct seq_file *s, struct clk_core *clk,
	unsigned int level, int parent_idx)
{
	int ret = 0;
	int i;
	struct clk_core *parent = NULL;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	/* Consistency check and print clk state */
	ret += clock_check(s, clk, level, parent_idx);

	/* recursive finding all source clock sources */
	for (i = 0; i < clk->num_parents; i++) {
		struct clk *tclk = NULL;
		int pp_idx = 0;

		parent = __clk_core_get_parent(clk);
		if (parent == NULL) {
			clklog_err("clock struct err!\n");
			return -EINVAL;
		}

		if (strcmp(parent->name, clk->parent_names[i]) == 0) {
			if (parent_idx >= CLOCK_PARENT_IDX_OFFSET)
				pp_idx = CLOCK_PARENT_IDX_OFFSET;
		} else if (strcmp("clk_invalid", clk->parent_names[i]) == 0) {
			continue; /* ignore clk_invalid */
		} else {
			tclk = __clk_lookup(clk->parent_names[i]);
			parent = (tclk != NULL) ? tclk->core : NULL;
		}

		pp_idx += find_clock_parent_idx(parent);
		ret += find_all_clock_sources(s, parent, level + 1, pp_idx);
	}

	return ret;
}

static int __find_single_clock_sources(struct seq_file *s,
	struct clk_core *clk, unsigned int level)
{
	int i;

	/* Consistency check and print clk state */
	if (clk->num_parents <= 0)
		return clock_check(s, clk, level, -1);

	/* traversal all parent clocks */
	for (i = 0; i < clk->num_parents; i++) {
		struct clk_core *pclk = __clk_core_get_parent(clk);

		/* Consistency check and print clk state */
		if (!IS_ERR_OR_NULL(pclk) &&
			strcmp(pclk->name, clk->parent_names[i]) == 0)
			return clock_check(s, clk, level, i);
	}
	return 0;
}

static int find_single_clock_sources(struct seq_file *s, struct clk_core *clk,
	unsigned int level)
{
	int ret = 0;

	while (!IS_ERR_OR_NULL(clk)) {
		ret += __find_single_clock_sources(s, clk, level);

		/* upward to clock source */
		clk = __clk_core_get_parent(clk);
		level++;
	}

	return ret;
}

static int __find_clock_path(struct seq_file *s, struct clk_core *clk,
	bool leading, bool recur)
{
	const char *ONE_LINE_STR =
		"---------------------------------------------";
	int ret;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	/* leading of table */
	if (leading) {
		seq_clklog(s, "%14s%47s%8s%13s%9s%24s%8s%6s%6s\n",
			"clock name", "clock type", "rate", "region",
			"reg", "reg value", "en_cnt", "p_cnt", "state");
		seq_clklog(s, "%s%s%s\n", ONE_LINE_STR, ONE_LINE_STR,
			ONE_LINE_STR);
	}

	if (recur)
		ret = find_all_clock_sources(s, clk, 0,
			find_clock_parent_idx(clk) +
			CLOCK_PARENT_IDX_OFFSET);
	else
		ret = find_single_clock_sources(s, clk, 0);

	/* last of table */
	if (leading) {
		seq_clklog(s, "%s%s%s\n", ONE_LINE_STR, ONE_LINE_STR,
			ONE_LINE_STR);
		seq_clklog(s, "[+  CLK: %s +][+ RET: %d +][+ RESULT: %s +]\n",
			clk->name, ret, (ret == 0 ? "Success" : "Failure"));
	}

	return ret;
}

static int find_clock_path(struct seq_file *s, const char *clock_name,
	bool leading, bool recur)
{
	struct clk_core *clk = NULL;

	if (IS_ERR_OR_NULL(clock_name))
		return -EINVAL;

	clk = find_clock(clock_name);
	if (IS_ERR_OR_NULL(clk)) {
		clklog_err("clock %s doesn't find!\n", clock_name);
		return -EINVAL;
	}

	return __find_clock_path(s, clk, leading, recur);
}

static int check_dvfs_clock(struct clock_check_param *param)
{
	int ret = MATCH;
	unsigned long rate;
	struct clock_print_info pinfo;
	struct peri_dvfs_clk *dclk = NULL;
	const char *clk_friend = NULL;

	rate = clk_get_rate(param->clk->hw->clk);

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_DVFS].alias;
	pinfo.rate               = rate;
	pinfo.clock_reg_groups   = 0;
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = ret;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);
	print_dvfs_avs_extra_info(param, true);

	dclk = container_of(param->clk->hw, struct peri_dvfs_clk, hw);
	clk_friend = dclk->link;

	return find_clock_path(param->s, clk_friend, false,
		(param->parent_idx >= CLOCK_PARENT_IDX_OFFSET));
}

#define FAST_CLOCK_REG_GROUPS 4
struct fast_clock_param {
	void __iomem *base;
	unsigned int hwaddr;
	u32 clksw_offset[SW_DIV_CFG_CNT];
	u32 clkdiv_offset[SW_DIV_CFG_CNT];
	u32 clkgt_cfg[GATE_CFG_CNT];
	u32 clkgate_cfg[GATE_CFG_CNT];
	u32 pll_profile[PLL_CNT];
	unsigned long rate;
	unsigned int sw_value;
	unsigned int div_value;
	unsigned int scgt_value;
	unsigned int gate_value;
	int region_idx;
	int region_stat;
	bool always_on;
};

static void __fill_fclk_info(struct clock_print_info *pinfo,
	struct fast_clock_param *fclk_param)
{
	pinfo->clock_reg_groups = FAST_CLOCK_REG_GROUPS;

	/* the first group regs */
	pinfo->crg_region[0]  = crg_regions[fclk_param->region_idx].crg_name;
	pinfo->region_stat[0] = fclk_param->region_stat;
	pinfo->reg[0]         = fclk_param->hwaddr + fclk_param->clksw_offset[CFG_OFFSET];
	pinfo->h_bit[0]       = fls(fclk_param->clksw_offset[CFG_MASK]) - 1;
	pinfo->l_bit[0]       = ffs(fclk_param->clksw_offset[CFG_MASK]) - 1;
	pinfo->reg_val[0]     = fclk_param->sw_value;

	/* the second group regs */
	pinfo->crg_region[1]  = crg_regions[fclk_param->region_idx].crg_name;
	pinfo->region_stat[1] = fclk_param->region_stat;
	pinfo->reg[1]         = fclk_param->hwaddr + fclk_param->clkdiv_offset[CFG_OFFSET];
	pinfo->h_bit[1]       = fls(fclk_param->clkdiv_offset[CFG_MASK]) - 1;
	pinfo->l_bit[1]       = ffs(fclk_param->clkdiv_offset[CFG_MASK]) - 1;
	pinfo->reg_val[1]     = fclk_param->div_value;

	/* the third group regs */
	pinfo->crg_region[2]  = crg_regions[fclk_param->region_idx].crg_name;
	pinfo->region_stat[2] = fclk_param->region_stat;
	pinfo->reg[2]         = fclk_param->hwaddr + fclk_param->clkgt_cfg[CFG_OFFSET];
	pinfo->h_bit[2]       = fclk_param->clkgt_cfg[CFG_MASK];
	pinfo->l_bit[2]       = fclk_param->clkgt_cfg[CFG_MASK];
	pinfo->reg_val[2]     = fclk_param->scgt_value;

	/* the fourth group regs */
	pinfo->crg_region[3]  = crg_regions[fclk_param->region_idx].crg_name;
	pinfo->region_stat[3] = fclk_param->region_stat;
	pinfo->reg[3]         = fclk_param->hwaddr + fclk_param->clkgate_cfg[CFG_OFFSET] + REG_STATE_OFFSET;
	pinfo->h_bit[3]       = fclk_param->clkgate_cfg[CFG_MASK];
	pinfo->l_bit[3]       = fclk_param->clkgate_cfg[CFG_MASK];
	pinfo->reg_val[3]     = fclk_param->gate_value;
}

static void fill_fclk_param(struct clock_check_param *param,
	struct fast_clock_param *fclk_param)
{
	int i;
	struct hi3xxx_fastclk *fclk = NULL;

	fclk = container_of(param->clk->hw, struct hi3xxx_fastclk, hw);
	for (i = 0; i < SW_DIV_CFG_CNT; i++) {
		fclk_param->clksw_offset[i] = fclk->clksw_offset[i];
		fclk_param->clkdiv_offset[i] = fclk->clkdiv_offset[i];
	}
	for (i = 0; i < GATE_CFG_CNT; i++) {
		fclk_param->clkgt_cfg[i] = fclk->clkgt_cfg[i];
		fclk_param->clkgate_cfg[i] = fclk->clkgate_cfg[i];
	}
	for (i = 0; i < PLL_CNT; i++)
		fclk_param->pll_profile[i] = fclk->pll_profile[i];

	fclk_param->always_on = (fclk->always_on ? true : false);
}

static int check_fast_clock_rate(struct clock_check_param *param,
	struct fast_clock_param *fclk_param)
{
	void __iomem *div_addr = NULL;
	void __iomem *sw_addr = NULL;
	unsigned long rate;
	unsigned int sw_value, div_value, val;
	int i;

	div_addr = fclk_param->base + fclk_param->clkdiv_offset[CFG_OFFSET];
	fclk_param->div_value = (unsigned int)readl(div_addr);
	div_value = fclk_param->div_value;
	div_value &= fclk_param->clkdiv_offset[CFG_MASK];
	div_value = div_value >> fclk_param->clkdiv_offset[SHIFT];
	div_value++;

	sw_addr = fclk_param->base + fclk_param->clksw_offset[CFG_OFFSET];
	fclk_param->sw_value = (unsigned int)readl(sw_addr);
	sw_value = fclk_param->sw_value;
	sw_value &= fclk_param->clksw_offset[CFG_MASK];
	sw_value = sw_value >> fclk_param->clksw_offset[SHIFT];

	val = sw_value >> 1;
	for (i = 0; val != 0; i++)
		val >>= 1;
	if (i >= PLL_CNT) {
		clklog_err(" %s: sw value is illegal,  sw_value = 0x%x, i = %d!\n",
			param->clk->name, sw_value, i);
		return MISMATCH;
	}

	/* div_value will not be 0 */
	rate = (((unsigned long)fclk_param->pll_profile[i]) * FREQ_CONVERSION) / div_value;
	if (rate != fclk_param->rate) {
		clklog_err("%s: rate is not matched, rate_get = %lu, rate = %lu\n",
			param->clk->name, fclk_param->rate, rate);
		return MISMATCH;
	}

	return MATCH;
}

static int fast_clock_state_check(struct clock_check_param *param,
	struct fast_clock_param *fclk_param)
{
	int match, rate_matched, scgt_matched, gate_matched;

	rate_matched = check_fast_clock_rate(param, fclk_param);
	fclk_param->scgt_value = (unsigned int)readl(fclk_param->base +
		fclk_param->clkgt_cfg[CFG_OFFSET]);
	scgt_matched = __check_gate_state_matched(fclk_param->scgt_value,
		BIT(fclk_param->clkgt_cfg[CFG_MASK]),
		param->clk->enable_count, fclk_param->always_on);
	fclk_param->gate_value = (unsigned int)readl(fclk_param->base +
		(fclk_param->clkgate_cfg[CFG_OFFSET] + REG_STATE_OFFSET));
	gate_matched = __check_gate_state_matched(fclk_param->gate_value,
		BIT(fclk_param->clkgate_cfg[CFG_MASK]),
		param->clk->enable_count, fclk_param->always_on);

	match = rate_matched + scgt_matched + gate_matched;
	return match;
}

static int check_fast_clock(struct clock_check_param *param)
{
	int matched = MISMATCH;
	struct clock_print_info pinfo;
	struct hi3xxx_fastclk *fclk = NULL;
	struct fast_clock_param fclk_param = {0};

	fclk = container_of(param->clk->hw, struct hi3xxx_fastclk, hw);
	fclk_param.rate = clk_get_rate(fclk->hw.clk);
	fclk_param.base = fclk->base_addr;
	fill_fclk_param(param, &fclk_param);

	fclk_param.region_idx = get_base_hwaddr(param->np);
	if (CHECK_REGION_IDX(fclk_param.region_idx)) {
		clklog_err("Failed to get clk[%s] base addr!\n",
			param->clk->name);
		return -EINVAL;
	}
	fclk_param.hwaddr = crg_regions[fclk_param.region_idx].base_addr;
	fclk_param.region_stat = crg_regions[fclk_param.region_idx].region_ok();
	if (fclk_param.region_stat  < 0) {
		clklog_err("check region %s err, err_code is %d\n",
			crg_regions[fclk_param.region_idx].crg_name, fclk_param.region_stat);
		return fclk_param.region_stat;
	}

	/* crg region powered up */
	if (fclk_param.region_stat)
		matched = fast_clock_state_check(param, &fclk_param);
	else
		matched = MATCH; /* crg region power down */

	pinfo.clock_name         = param->clk->name;
	pinfo.clock_type         = clock_types[CLOCK_FAST_CLOCK].alias;
	pinfo.rate               = fclk_param.rate;
	pinfo.clock_reg_groups   = FAST_CLOCK_REG_GROUPS;
	__fill_fclk_info(&pinfo, &fclk_param);
	pinfo.enable_cnt         = param->clk->enable_count;
	pinfo.prepare_cnt        = param->clk->prepare_count;
	pinfo.check_state        = matched;
	pinfo.parent_idx         = param->parent_idx;
	print_clock_info(param->s, &pinfo, param->level);

	return (matched == MISMATCH ? MISMATCH : MATCH);
}

/* ***************** clock test plan begin ******************** */
#define MAX_CLOCK_RATE_NUM     8
struct clock_test_plan {
	char clock_name[MAX_CLK_NAME_LEN];
	unsigned long rates[MAX_CLOCK_RATE_NUM + 1];
};
static const char *clock_test_plan_help =
	"\n[Help Tips]:\nPlease use test_plan debugfs as follows:\n"
	"1) echo <clkname> [rate1] [rate2]...> /d/clock/test_one_clock/test_plan\n"
	"                       {NOTE: only support max to 8 clock rates!}\n"
	"2) cat /d/clock/test_one_clock/test_plan\n\n";
static DEFINE_MUTEX(clock_plan_lock);
static struct clock_test_plan *clock_plan;

#define CHECK_FIND_CLOCK_PATH(_s, _rc)  do {                          \
	if (_rc) {                                                    \
		ret = 1;                                              \
		seq_clklog((_s), "clock[%s] path check err! rc=%d\n", \
			plan->clock_name, (_rc));                     \
	}                                                             \
} while (0)

static int test_clock_plan(struct seq_file *s, struct clock_test_plan *plan)
{
	struct clk_core *clk = NULL;
	int ret;
	int rc;
	unsigned int i;
	unsigned long old_rate;

	clk = find_clock(plan->clock_name);
	if (IS_ERR_OR_NULL(clk)) {
		seq_clklog(s, "find clock: %s err!\n", plan->clock_name);
		return -EINVAL;
	}

	old_rate = clk_get_rate(clk->hw->clk);
	seq_clklog(s, "clock[%s] get old_rate[%lu]\n", plan->clock_name,
		old_rate);

	seq_clklog(s, "clock[%s] prepare and enable\n", plan->clock_name);
	ret = clk_prepare_enable(clk->hw->clk);
	if (ret) {
		seq_clklog(s, "clock[%s] prepare and enable err! ret=%d\n",
			plan->clock_name, ret);
		return ret;
	}
	rc = find_clock_path(s, plan->clock_name, true, false);
	CHECK_FIND_CLOCK_PATH(s, rc);

	for (i = 0; i <= MAX_CLOCK_RATE_NUM; i++) {
		if (!plan->rates[i]) {
			/* if last rate is equal to old_rate, ignore */
			if (i > 0 && plan->rates[i - 1] != old_rate)
				plan->rates[i] = old_rate;
			break;
		}
	}
	for (i = 0; i <= MAX_CLOCK_RATE_NUM && plan->rates[i]; i++) {
		seq_clklog(s, "********CLOCK[%s] RATE[%lu] TESTING ********\n",
			plan->clock_name, plan->rates[i]);
		rc = clk_set_rate(clk->hw->clk, plan->rates[i]);
		if (rc < 0) {
			ret = 1;
			seq_clklog(s, "clock[%s] set rate %lu err! rc = %d\n",
				plan->clock_name, plan->rates[i], rc);
		}
		rc = find_clock_path(s, plan->clock_name, true, false);
		CHECK_FIND_CLOCK_PATH(s, rc);
	}

	seq_clklog(s, "clock[%s] disable and unprepare:\n", plan->clock_name);
	clk_disable_unprepare(clk->hw->clk);
	rc = find_clock_path(s, plan->clock_name, true, false);
	CHECK_FIND_CLOCK_PATH(s, rc);

	return ret;
}

static int clk_testplan_show(struct seq_file *s, void *data)
{
	int ret;

	mutex_lock(&clock_plan_lock);
	if (IS_ERR_OR_NULL(clock_plan)) {
		seq_clklog(s, "No clock test plan buffered!\n");
		seq_clklog(s, "%s", clock_test_plan_help);
		goto out;
	}
	ret = test_clock_plan(s, clock_plan);
	if (ret < 0)
		goto out;

	if (s->size > 0 && s->size == s->count) {
		clklog_info("resize the seq_file buffer size (%u -> %u)!\n",
			(u32)s->size, (u32)s->size * 2);
		goto end;
	}

out:
	kfree(clock_plan);
	clock_plan = NULL;

end:
	mutex_unlock(&clock_plan_lock);

	return 0;
}

int clk_testplan_single_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_testplan_show, inode->i_private);
}

static int __analysis_clock_test_plan_string(char *str, int ret_val)
{
	char *t_str = NULL;
	int i;
	unsigned long rate;

	for (i = 0; str != NULL && *str && i <= MAX_CLOCK_RATE_NUM; i++) {
		t_str = strchr(str, ' ');
		if (t_str != NULL) {
			*t_str = '\0';
			t_str++;
		}

		if (i == 0) {
			if (strcpy_s(clock_plan->clock_name, MAX_CLK_NAME_LEN,
				str) != EOK) {
				clklog_err("clock name %s is too long!\n", str);
				return -EINVAL;
			}
			clklog_info("clock name: %s\n", clock_plan->clock_name);
		} else {
			/* at most 10 digits */
			rate = simple_strtoul(str, NULL, 10);
			if (!rate) {
				clklog_err("please input valid clk rate!\n");
				return -EINVAL;
			}
			clock_plan->rates[i - 1] = rate;
			clklog_info("clock rate-%d: %lu\n", i,
				clock_plan->rates[i - 1]);
		}
		str = t_str;
	}

	return ret_val;
}

#define MAX_STRING_LEN 128
ssize_t clk_testplan_write(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	char *clkname_freq_str = NULL;
	int ret = cnt;

	if (cnt == 0 || !ubuf) {
		clklog_err("Input string is NULL\n");
		return -EINVAL;
	}

	if (cnt >= MAX_STRING_LEN) {
		clklog_err("Input string is too long\n");
		return -EINVAL;
	}

	mutex_lock(&clock_plan_lock);
	if (!clock_plan) {
		clock_plan = kzalloc(sizeof(struct clock_test_plan),
			GFP_KERNEL);
		if (!clock_plan) {
			ret = -EINVAL;
			goto out;
		}
	} else {
		clklog_err("clock %s does not finish test_plan!\n",
			clock_plan->clock_name);
		goto out;
	}

	clkname_freq_str = kzalloc(cnt * sizeof(char), GFP_KERNEL);
	if (!clkname_freq_str) {
		ret = -EINVAL;
		goto out;
	}

	/* copy clock name from user space(the last symbol is newline). */
	if (copy_from_user(clkname_freq_str, ubuf, cnt - 1)) {
		clklog_err("Input string is too long\n");
		ret = -EINVAL;
		goto out;
	}
	clkname_freq_str[cnt - 1] = '\0';
	ret = __analysis_clock_test_plan_string(clkname_freq_str, cnt);

out:
	mutex_unlock(&clock_plan_lock);
	if (clkname_freq_str)
		kfree(clkname_freq_str);
	return ret;
}

const struct file_operations clk_testplan_fops = {
	.open    = clk_testplan_single_open,
	.read    = seq_read,
	.write   = clk_testplan_write,
	.llseek  = seq_lseek,
	.release = single_release,
};

/* ***************** clock test plan end ******************** */

/* ***************** get clock path begin ******************** */
static DEFINE_MUTEX(leaf_clock_name_lock);
static const int MAX_CLKNAME_LEN = 128;
static char *leaf_clock_name;
static const char *clock_get_path_help =
	"\nHelp Tips:\nPlease use get_path debugfs as follows:\n"
	"1) echo {clock name} [string] > /d/clock/test_one_clock/get_path\n"
	"2) cat /d/clock/test_one_clock/get_path\n\n";

static int clk_getpath_show(struct seq_file *s, void *data)
{
	int rc;
	bool recur = false;
	char *t_str = NULL;

	mutex_lock(&leaf_clock_name_lock);
	if (!leaf_clock_name) {
		seq_clklog(s, "No clock name buffered!\n");
		seq_clklog(s, "%s", clock_get_path_help);
		goto out;
	}

	t_str = strchr(leaf_clock_name, ' ');
	if (t_str != NULL) {
		*t_str = '\0';
		recur = true;
	}
	rc = find_clock_path(s, leaf_clock_name, true, recur);
	if (rc < 0)
		clklog_err("clock path check fail: rc = %d!", rc);

	if (s->size > 0 && s->size == s->count) {
		clklog_info("resize the seq_file buffer size (%u -> %u)!\n",
			(u32)s->size, (u32)s->size * 2); /* 2: times */
		/* restore leaf_clock_name string */
		if (t_str != NULL)
			*t_str = ' ';
		goto end;
	}

out:
	kfree(leaf_clock_name);
	leaf_clock_name = NULL;

end:
	mutex_unlock(&leaf_clock_name_lock);
	return 0;
}

int clk_getpath_single_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_getpath_show, inode->i_private);
}

ssize_t clk_getpath_write(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	int ret = cnt;

	if (cnt == 0 || !ubuf) {
		clklog_err("Input string is NULL\n");
		return -EINVAL;
	}

	if (cnt >= MAX_CLKNAME_LEN) {
		clklog_err("Input string is too long\n");
		return -EINVAL;
	}

	mutex_lock(&leaf_clock_name_lock);
	if (leaf_clock_name == NULL) {
		leaf_clock_name = kzalloc(sizeof(char) * MAX_CLKNAME_LEN,
			GFP_KERNEL);
		if (leaf_clock_name == NULL) {
			ret = -EINVAL;
			goto out;
		}
	} else {
		clklog_err("clock %s does not finish get_path!\n",
			leaf_clock_name);
		goto out;
	}

	/* copy clock name from user space(the last symbol is newline). */
	if (copy_from_user(leaf_clock_name, ubuf, cnt - 1)) {
		clklog_err("Input string is too long\n");
		ret = -EINVAL;
		goto out;
	}
	clklog_info("clock buffer: %s\n", leaf_clock_name);
out:
	mutex_unlock(&leaf_clock_name_lock);
	return ret;
}

const struct file_operations clk_getpath_fops = {
	.open    = clk_getpath_single_open,
	.read    = seq_read,
	.write   = clk_getpath_write,
	.llseek  = seq_lseek,
	.release = single_release,
};

/* ***************** get clock path end ******************** */

void create_clk_test_debugfs(struct dentry *pdentry)
{
	if (!pdentry)
		return;
#ifdef CONFIG_HISI_DEBUG_FS
	debugfs_create_file("get_path", PRIV_AUTH, pdentry, NULL,
		&clk_getpath_fops);
	debugfs_create_file("clk_testplan", PRIV_AUTH, pdentry, NULL,
		&clk_testplan_fops);
#endif
}
