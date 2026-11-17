/*
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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
#include <linux/clk.h>
#include "../clk.h"

#define CLK_INPUT_MAX_NUM       100
#define hwlog_err(fmt, args...) pr_err("[clk]" fmt, ## args)

#ifdef CONFIG_HISI_CLK_TRACE
#define CLK_NAME_LEN   32
#define CLK_MAGIC_NUM  0x20160322
#define CLK_ERR_NUM    0xFF

#define CLK_DVFS_ADDR_0(Base)  SOC_PMCTRL_PERI_CTRL4_ADDR(Base)
#define CLK_DVFS_ADDR_1(Base)  SOC_PMCTRL_PERI_CTRL5_ADDR(Base)

enum track_clk_type {
	TRACK_ON_OFF = 0,
	TRACK_SET_FREQ,
	TRACK_SET_DVFS,
	TRACK_CLK_MAX
};

enum himntn_trace_switch {
	SWITCH_CLOSE = 0,
	SWITCH_OPEN,
	SWITCH_MAX
};

struct pc_record_info {
	unsigned int    dump_magic;
	unsigned int    buffer_size;
	unsigned char  *buffer_addr;
	unsigned char  *percpu_addr[NR_CPUS];
	unsigned int    percpu_length[NR_CPUS];
};

struct clk_record_info {
	u64   current_time;
	u16   item;
	u16   enable_count;
	u32   current_rate;
	u32   cpu_l;
	u32   cpu_b;
	u32   ppll;
	u32   ddr_freq;
	u32   peri_dvfs_vote0;
	u32   peri_dvfs_vote1;
	char  comm[CLK_NAME_LEN];
};

struct hs_trace_clk_info {
	void __iomem   *pmuctrl;
	struct clk     *clk_cpu_l;
	struct clk     *clk_cpu_b;
	struct clk     *clk_ddr;
};

enum buf_type_en {
	SINGLE_BUFF = 0,
	MULTI_BUFF,
};

static u64 clk_rdr_core_id = RDR_CLK;
static struct pc_record_info *g_clk_track_addr;
static unsigned char g_clk_hook_on;
static enum buf_type_en clk_sel_buf_type = MULTI_BUFF;
static struct hs_trace_clk_info hs_trace_info;
#endif

static struct dentry *clock;
static struct dentry *test_all_clocks;
static struct dentry *test_one_clock;

static struct clk_debug_data g_clk_debug_data[MAX_CLOCK_TYPE];

static DEFINE_MUTEX(clock_list_lock);

#define to_clk_gate(_hw) container_of(_hw, struct clk_gate, hw)
#define to_clk_mux(_hw) container_of(_hw, struct clk_mux, hw)

#define CLK_STATUS_NUM 4
enum {
	CLK_STATUS_NOREG,
	CLK_STATUS_OK,
	CLK_STATUS_ERR,
	CLK_STATUS_NULL,
};
static const char *g_clk_status[CLK_STATUS_NUM] = { "NOREG", "OK", "ERR", "NULL" };

struct clk_core *__clk_core_get_parent(struct clk_core *clk)
{
	if (!clk)
		return NULL;

	/* Create a per-user clk and change callers to call clk_put */
	return clk->parent ? clk->parent : NULL;
}

const char *__clk_core_get_name(struct clk_core *clk)
{
	return clk ? clk->name : NULL;
}

unsigned int __clk_core_get_enable_count(struct clk_core *clk)
{
	return clk ? clk->enable_count : 0;
}

struct clk_core *find_clock(const char *clk_name)
{
	struct clk_core *clk = NULL;

	if (!clk_name)
		return NULL;

	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("[%s] clk is err or nul!\n", __func__);
			goto out;
		}
		if (strcmp(clk->name, clk_name) == 0) {
			pr_info("[%s] find clock: %s!\n", __func__, clk_name);
			goto out;
		}
	}
	clk = NULL; /* No Clock found */
out:
	mutex_unlock(&clock_list_lock);
	return clk;
}

/* clk_debug_data is malloced only once and freed by system */
static int create_clk_debug_data(enum clk_type_id clk_type)
{
	g_clk_debug_data[clk_type].size = 0;
	g_clk_debug_data[clk_type].clks = kzalloc(sizeof(struct clk_core *) * MAX_CLK_NUMS, GFP_KERNEL);
	if (!g_clk_debug_data[clk_type].clks)
		return -1;

	return 0;
}

void debug_clk_add(struct clk *clk, enum clk_type_id clk_type)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(clk) || clk_type >= MAX_CLOCK_TYPE) {
		pr_err("[%s] clk is wrong (error or null) or clk type is not exit\n", __func__);
		return;
	}

	if (!g_clk_debug_data[clk_type].clks)
		ret = create_clk_debug_data(clk_type);

	if (ret) {
		pr_err("%s: could not allocate clock lookup table\n", __func__);
		return;
	}

	if (g_clk_debug_data[clk_type].size >= MAX_CLK_NUMS) {
		pr_err("[%s] lookup table-%d is full\n", __func__, clk_type);
		return;
	}

	g_clk_debug_data[clk_type].clks[g_clk_debug_data[clk_type].size] = clk->core;
	g_clk_debug_data[clk_type].size++;
}

int get_clock_type(struct clk_core *clk)
{
	int i, j;
	struct clk_core *tmp_clk_core = NULL;

	if (IS_ERR_OR_NULL(clk)) {
		pr_err("[%s] clk is error or null that clk_type can not find\n", __func__);
		return -1;
	}

	for (i = 0; i < MAX_CLOCK_TYPE; i++) {
		for (j = 0; j < g_clk_debug_data[i].size; j++) {
			tmp_clk_core = g_clk_debug_data[i].clks[j];
			if (tmp_clk_core == clk)
				return i;
		}
	}
	pr_err("[%s] %s not found in lookup table\n", __func__, clk->name);

	return -1;
}


void clk_base_addr_print(struct seq_file *s)
{
	if (!s)
		return;
	seq_puts(s, "------------------------------------------------------\n");
#if defined SOC_ACPU_PERI_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "PERICRG",
		SOC_ACPU_PERI_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_SCTRL_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "SCTRL", SOC_ACPU_SCTRL_BASE_ADDR);
#endif
#if defined SOC_ACPU_PMC_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "PMCTRL", SOC_ACPU_PMC_BASE_ADDR);
#endif
#if defined SOC_ACPU_PCTRL_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "PCTRL", SOC_ACPU_PCTRL_BASE_ADDR);
#endif
#if defined SOC_ACPU_MEDIA1_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MEDIA1CRG",
		SOC_ACPU_MEDIA1_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_MEDIA2_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MEDIA2CRG",
		SOC_ACPU_MEDIA2_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MMC1CRG",
		SOC_ACPU_MMC1_SYS_CTRL_BASE_ADDR);
#endif
#if defined SOC_ACPU_MMC0_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "MMC0CRG",
		SOC_ACPU_MMC0_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_HSDT_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "HSDTCRG", SOC_ACPU_HSDT_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_HSDT1_CRG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "HSDT1CRG", SOC_ACPU_HSDT1_CRG_BASE_ADDR);
#endif
#if defined SOC_ACPU_IOMCU_CONFIG_BASE_ADDR
	seq_printf(s, " %-15s [0x%X]\n", "IOMCU",
		SOC_ACPU_IOMCU_CONFIG_BASE_ADDR);
#endif
	seq_puts(s, "------------------------------------------------------\n");
}

void clk_tree_dump_reg(struct seq_file *s, struct clk_core *clk_core)
{
	int ret = 0;

	if (!clk_core || !s)
		return;

	mutex_lock(&clock_list_lock);
	if (clk_core->ops->dump_reg)
		ret = clk_core->ops->dump_reg(clk_core->hw, NULL, 0, s);

	if (ret) {
		if (clk_core->name)
			hwlog_err("%s dump reg return fail!\n", clk_core->name);
	}
	mutex_unlock(&clock_list_lock);
}

static const char *clk_enreg_check(struct clk_core *clk)
{
	int val;

	if (IS_ERR(clk))
		return g_clk_status[CLK_STATUS_ERR];

	if (!clk->ops->is_enabled)
		return g_clk_status[CLK_STATUS_NOREG];

	val = clk->ops->is_enabled(clk->hw);
	if (val == 2) /* regulator not exist */
		return g_clk_status[CLK_STATUS_NOREG];

	if ((val && clk->enable_count) || ((val == 0) && (clk->enable_count == 0)))
		return g_clk_status[CLK_STATUS_OK];
	else
		return g_clk_status[CLK_STATUS_ERR];
}

static const char *clk_selreg_check(struct clk_core *clk)
{
	int val;

	if (IS_ERR(clk))
		return g_clk_status[CLK_STATUS_ERR];

	if (!clk->ops->check_selreg)
		return g_clk_status[CLK_STATUS_NOREG];

	val = clk->ops->check_selreg(clk->hw);
	if (val == 3) /* clock reg addr is NULL */
		return g_clk_status[CLK_STATUS_NULL];

	return val ? g_clk_status[CLK_STATUS_OK] : g_clk_status[CLK_STATUS_ERR];
}

static const char *clk_divreg_check(struct clk_core *clk)
{
	int val;

	if (IS_ERR(clk))
		return g_clk_status[CLK_STATUS_ERR];

	if (!clk->ops->check_divreg)
		return g_clk_status[CLK_STATUS_NOREG];

	val = clk->ops->check_divreg(clk->hw);

	return val ? g_clk_status[CLK_STATUS_OK] : g_clk_status[CLK_STATUS_ERR];
}

int clock_get_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	struct clk_core *clock_core = NULL;
	struct clk *clock_get = NULL;
	const char *status = NULL;

	seq_printf(s, "%18s%-3s%5s%-3s%10s\n", "clock", "", "status", "", "rate");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test clk_get() interface. */
	list_for_each_entry(clk, &clocks, node) {
		clock_get = clk_get(NULL, clk->name);
		if (IS_ERR(clock_get)) {
			pr_err("%s clock_get failed!\n", clk->name);
			continue;
		}
		clock_core = clock_get->core;
		if (!clock_core) {
			pr_err("%s get failed!\n", clk->name);
			continue;
		}

		if (clock_core->enable_count)
			status = "on";
		else
			status = "off";

		seq_printf(s, "%18s%-3s%5s%-3s%10lu\n", clock_core->name,
			"", status, "", clock_core->rate);
		clk_put(clock_get);
	}

	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_lookup_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	struct clk_core *clock_core = NULL;
	struct clk *clock_get = NULL;
	const char *status = NULL;

	seq_printf(s, "%14s%-3s%5s%-3s%10s\n", "clock", "", "status", "", "rate");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/*
	 * Output all clocks in the clocks list,
	 * test __clk_lookup() interface.
	 */
	list_for_each_entry(clk, &clocks, node) {
		clock_get = __clk_lookup(clk->name);
		if (!clock_get) {
			pr_err("%s clock_get failed!\n", clk->name);
			goto out;
		}
		clock_core = clock_get->core;
		if (!clock_core) {
			pr_err("%s get failed!\n", clk->name);
			goto out;
		}

		if (clock_core->enable_count)
			status = "on";
		else
			status = "off";

		seq_printf(s, "%20s%-3s%5s%-3s%10lu\n",
			clock_core->name, "", status, "", clock_core->rate);
	}
out:
	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_enable_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	int ret = 0;

	pr_err("%20s%-3s%5s%-3s%5s\n", "clock", "", "refcnt_pre", "", "refcnt_now");
	pr_err("---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/* Output all clocks in the clocks list, test clk_enable() interface. */
	list_for_each_entry(clk, &clocks, node) {
		if (!clk->ops->enable)
			continue;

		pr_err("%20s%-3s%5u", clk->name, "", clk->enable_count);

		ret = clk_prepare_enable(clk->hw->clk);
		if (ret) {
			pr_err("%s enable failed!\n", clk->name);
			goto out;
		}

		pr_err("%-3s%5u\n", "", clk->enable_count);
	}
out:
	mutex_unlock(&clock_list_lock);
	return ret;
}

int clock_disable_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;

	pr_err("%20s%-3s%5s%-3s%5s\n", "clock", "", "refcnt_pre", "", "refcnt_now");
	pr_err("---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/*
	 * Output all clocks in the clocks list,
	 * test clk_disable() interface.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (!clk->ops->enable)
			continue;

		pr_err("%20s%-3s%5u", clk->name, "", clk->enable_count);

		clk_disable_unprepare(clk->hw->clk);

		pr_err("%-3s%5u\n", "", clk->enable_count);
	}

	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_getparent_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	int i;

	seq_printf(s, "%20s%-5s%8s\n", "clock", "", "parent");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/*
	 * Output all clocks in the clocks list,
	 * test clk_get_parent() interface.
	 */
	list_for_each_entry(clk, &clocks, node) {
		if (clk->num_parents == 0) {
			seq_printf(s, "%20s%-5s%8s\n", clk->name, "", "");
		} else {
			seq_printf(s, "%20s", clk->name);

			for (i = 0; i < clk->num_parents; i++)
				seq_printf(s, "%-5s%20s", "",
					clk->parent_names[i]);

			seq_printf(s, "\n");
		}
	}
	mutex_unlock(&clock_list_lock);
	return 0;
}

int clock_getrate_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;

	seq_printf(s, "%20s%-5s%10s\n", "clock", "", "rate");
	seq_printf(s, "---------------------------------------------------------------\n\n");

	mutex_lock(&clock_list_lock);

	/*
	 * Output all clocks in the clocks list,
	 * test clk_get_rate() interface.
	 */
	list_for_each_entry(clk, &clocks, node)
		seq_printf(s, "%20s%-5s%10lu\n", clk->name, "", clk->rate);

	mutex_unlock(&clock_list_lock);

	return 0;
}

/* judge register status of this clock. */
static const char *clk_register_status(struct clk_core *clk)
{
	const char *state = NULL;

	if (clk_core_is_enabled(clk))
		state = "on";
	else
		state = "off";

	return state;
}

static void base_info_print(struct seq_file *s)
{
	seq_printf(s, "%17s%-6s%4s%-2s%6s%-2s%6s%-2s%6s%-2s%7s%-2s%5s%-2s%16s%-2s%4s%-2s%5s%-3s%5s\n",
		"clock", "", "status", "", "enreg", "", "selreg", "", "divreg", "", "refcnt", "", "rate",
		"", "parent", "", "status", "", "refcnt", "", "rate");
	seq_printf(s, "-------------------------------------------------------------------------------------\n\n");
}

static void base_clk_info_print(struct seq_file *s, struct clk_core *clk, struct clk_core *parent)
{
	seq_printf(s, "%20s%-2s%4s%-2s%6s%-2s%6s%-2s%6s%-2s%3u%-2s%10lu%-2s%20s%-2s%4s%-2s%3u%-3s%10lu\n",
		__clk_core_get_name(clk), "", clk_register_status(clk), "",
		clk_enreg_check(clk), "", clk_selreg_check(clk),
		"", clk_divreg_check(clk), "", __clk_core_get_enable_count(clk), "",
		clk_core_get_rate(clk), "", __clk_core_get_name(parent), "",
		clk_register_status(parent), "",
		__clk_core_get_enable_count(parent), "", clk_core_get_rate(parent));
}

static void clk_info_print(struct seq_file *s, struct clk_core *clk)
{
	seq_printf(s, "%18s%-2s%4s%-2s%6s%-2s%6s%-2s%6s%-2s%3u%-2s%10lu%-2s%18s%-2s%4s%-2s%3s%-3s%5s%-2s%15s\n",
		__clk_core_get_name(clk), "", clk_register_status(clk), "", clk_enreg_check(clk), "",
		clk_selreg_check(clk), "", clk_divreg_check(clk), "", __clk_core_get_enable_count(clk), "",
		clk_core_get_rate(clk), "", "", "", "", "", "", "", "", "", "");
}


int clock_tree_show(struct seq_file *s, void *data)
{
	struct clk_core *clk = NULL;
	struct clk_core *parent = NULL;

	mutex_lock(&clock_list_lock);
	/* Output gating clock and fixed rate clock in the clocks list */
	base_info_print(s);

	list_for_each_entry(clk, &clocks, node) {
		if (clk->ops->check_divreg ||
		    clk->ops->check_selreg ||
		    clk->ops->round_rate)
			continue;

		parent = __clk_core_get_parent(clk);
		if (!parent)
			clk_info_print(s, clk);
		else
			base_clk_info_print(s, clk, parent);
	}

	/* Output multiplexer clock in the clocks list */
	seq_printf(s, "---------------------------------------------------------------------------------------\n");
	seq_printf(s, "-----------------------------------[mux clock]-----------------------------------------\n");
	seq_printf(s, "---------------------------------------------------------------------------------------\n");

	base_info_print(s);
	list_for_each_entry(clk, &clocks, node) {
		if (!(clk->ops->set_parent))
			continue;

		parent = __clk_core_get_parent(clk);
		base_clk_info_print(s, clk, parent);
	}

	/*
	 * Output adjustable divider clock / fixed multiplier
	 * and divider clock in the clocks list
	 */
	seq_printf(s, "---------------------------------------------------------------------------------------\n");
	seq_printf(s, "-----------------------------[div clock] [fixed-factor clock]--------------------------\n");
	seq_printf(s, "---------------------------------------------------------------------------------------\n");

	base_info_print(s);
	list_for_each_entry(clk, &clocks, node) {
		if (!(clk->ops->set_rate))
			continue;

		parent = __clk_core_get_parent(clk);
		base_clk_info_print(s, clk, parent);
	}

	mutex_unlock(&clock_list_lock);

	return 0;
}

MODULE_FUNCS_DEFINE(clock_get);
MODULE_FUNCS_DEFINE(clock_tree);
MODULE_FUNCS_DEFINE(clock_lookup);
MODULE_FUNCS_DEFINE(clock_enable);
MODULE_FUNCS_DEFINE(clock_disable);
MODULE_FUNCS_DEFINE(clock_getrate);
MODULE_FUNCS_DEFINE(clock_getparent);

static ssize_t check_and_copy_user_name(const char __user *ubuf, size_t cnt,
	char **clock_name)
{
	if (cnt == 0 || !ubuf) {
		pr_err("Input string is NULL\n");
		return -EINVAL;
	}

	if (cnt > CLK_INPUT_MAX_NUM) {
		pr_err("Input string is too long\n");
		return -EINVAL;
	}
	*clock_name = kzalloc(sizeof(char) * cnt, GFP_KERNEL);
	if (!(*clock_name)) {
		pr_err("Cannot allocate clk_name\n");
		return -EINVAL;
	}

	/* copy clock name from user space. */
	if (copy_from_user((*clock_name), ubuf, cnt - 1)) {
		kfree((*clock_name));
		return -EINVAL;
	}
	(*clock_name)[cnt - 1] = '\0';

	return 0;
}

ssize_t clock_enable_store(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int ret;
	int err;

	err = check_and_copy_user_name(ubuf, cnt, &clk_name);
	if (err)
		return err;

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, prepare and enable it.
	 */
	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (strncmp(clk->name, clk_name, cnt) == 0) {
			pr_info("[old] enable_refcnt = %u\n",
				clk->enable_count);

			ret = clk_prepare_enable(clk->hw->clk);
			pr_err("[RetValue] clk_prepare_enable ret = %d\n", ret);
			if (ret) {
				err = -EINVAL;
				goto out;
			}

			pr_info("[new] enable_refcnt = %u\n",
				clk->enable_count);
			err = cnt;
			goto out;
		}
	}

	pr_err("clk name error!\n");
	err = -EINVAL;
out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

ssize_t clock_disable_store(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int err;

	err = check_and_copy_user_name(ubuf, cnt, &clk_name);
	if (err)
		return err;

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, disable and unprepare it.
	 */
	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (strncmp(clk->name, clk_name, cnt) == 0) {
			if (!__clk_core_get_enable_count(clk)) {
				pr_info("[%s] clk is disabled !\n", __func__);
				err = cnt;
				goto out;
			}
			pr_info("[old] enable_refcnt = %u\n",
				clk->enable_count);
			clk_disable_unprepare(clk->hw->clk);
			pr_info("[new] enable_refcnt = %u\n",
				clk->enable_count);
			err = cnt;
			goto out;
		}
	}
	pr_err("clk name error!\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

static void print_parent_names(struct clk_core *clk)
{
	int i;

	if (clk->num_parents)
		for (i = 0; i < clk->num_parents; i++)
			pr_info("[%d] %s\n", i, clk->parent_names[i]);
	else
		pr_info("null\n");
}

ssize_t clock_getparent_store(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int err;

	err = check_and_copy_user_name(ubuf, cnt, &clk_name);
	if (err)
		return err;

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, get parent of this clock.
	 */
	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (strncmp(clk->name, clk_name, cnt) == 0) {
			pr_info("[%s] ", clk->name);
			print_parent_names(clk);

			pr_info("\n------[current_parent]------\n");
			pr_info("%s\n\n\n", clk->parent->name);
			err = cnt;
			goto out;
		}
	}

	pr_err("clk name error!\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}


ssize_t clock_getrate_store(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	int err;

	err = check_and_copy_user_name(ubuf, cnt, &clk_name);
	if (err)
		return err;

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, get rate of this clock.
	 */
	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (strncmp(clk->name, clk_name, cnt) == 0) {
			pr_info("\n[%s]  %lu\n\n\n", clk->name,
				clk_get_rate(clk->hw->clk));
			err = cnt;
			goto out;
		}
	}

	pr_err("clk name error!\n\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

ssize_t clock_setrate_store(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *str = NULL;
	char *clk_name = NULL;
	unsigned long rate;
	int ret = 0;
	int err;

	err = check_and_copy_user_name(ubuf, cnt, &clk_name);
	if (err)
		return err;


	/* get clock rate */
	str = clk_name;
	str = strchr(str, ' ');
	if (!str) {
		err = -EINVAL;
		goto out;
	} else {
		*str = '\0';
	}
	str++;

	rate = simple_strtoul(str, NULL, 10);
	if (!rate) {
		pr_err("please input clk rate!\n");
		err = -EINVAL;
		goto out;
	}
	pr_info("rate = %lu\n\n", rate);

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, set rate of this clock.
	 */
	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (strncmp(clk->name, clk_name, cnt) == 0) {
			ret = clk_set_rate(clk->hw->clk, rate);
			pr_err("[RetValue] clk_set_rate ret = %d\n", ret);
			if (ret) {
				err = -EINVAL;
				goto out;
			}

			pr_info("[%s]  %lu\n\n", clk->name, clk->rate);
			err = cnt;
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error. */
	pr_err("clk name error!\n\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

static ssize_t __clock_setparent(struct clk_core *clk, const char *parent_name,
	const size_t cnt)
{
	int i, ret;
	struct clk_core *parent = NULL;

	for (i = 0; i < clk->num_parents; i++) {
		if (strncmp(parent_name, clk->parent_names[i], cnt) == 0) {
			parent = clk->parents[i];
			if (!parent) {
				struct clk *tclk;

				tclk = __clk_lookup(parent_name);
				parent = tclk ? tclk->core : NULL;
			}
			break;
		}
	}

	if (parent) {
		pr_info("[%s]  set %s\n\n", __func__, parent->name);
		ret = clk_set_parent(clk->hw->clk, parent->hw->clk);
		pr_err("[RetValue] clk_set_parent ret = %d\n", ret);
		if (ret)
			return -EINVAL;

		pr_info("[%s]  %s\n\n", clk->name, parent->name);
		pr_info("clk set parent ok!\n\n");
	} else {
		pr_err("no parent find!");
	}

	return (ssize_t)cnt;
}

ssize_t clock_setparent_store(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *input_string = NULL;
	char *clk_name = NULL;
	const char *parent_name = NULL;
	int err;

	err = check_and_copy_user_name(ubuf, cnt, &clk_name);
	if (err)
		return err;

	/* get clock parent name. */
	input_string = clk_name;
	input_string = strchr(input_string, ' ');
	if (!input_string) {
		err = -EINVAL;
		goto out;
	} else {
		*input_string = '\0';
	}

	input_string++;

	parent_name = input_string;
	if (!parent_name) {
		pr_err("please input clk parent name!\n");
		err = -EINVAL;
		goto out;
	}

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, set parent of this clock.
	 */
	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (strncmp(clk->name, clk_name, cnt) == 0) {
			err = __clock_setparent(clk, parent_name, cnt);
			goto out;
		}
	}
	pr_err("clk name error!\n");
	err = -EINVAL;

out:
	kfree(clk_name);
	mutex_unlock(&clock_list_lock);
	return err;
}

static void __iomem *clock_get_reg(struct clk_core *clk)
{
	void __iomem *ret = NULL;
	struct clk_gate *gate = NULL;
	struct clk_mux *mux =  NULL;

	if (clk->ops->get_reg) {
		/* hi3xxx_periclk,hixxx_divclk */
		ret = clk->ops->get_reg(clk->hw);
	} else if (clk->ops->enable) {
		gate = to_clk_gate(clk->hw); /* andgt clock */
		ret = gate->reg;
	} else if (clk->ops->set_parent) { /* mux clock */
		mux = to_clk_mux(clk->hw);
		ret = mux->reg;
	} else {
		pr_err("the clock %s is fixed or fiexd-factor\n",
			clk->name);
	}

	return ret;
}

ssize_t clock_getreg_store(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	struct clk_core *clk = filp->private_data;
	char *clk_name = NULL;
	struct clk *clock_get = NULL;
	struct clk_core *clock_core = NULL;
	void __iomem *ret = NULL;
	int err;

	err = check_and_copy_user_name(ubuf, cnt, &clk_name);
	if (err)
		return err;

	/*
	 * Check if we have such a clock in the clocks list.
	 * if exist, get reg of this clock.
	 */
	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node) {
		if (strncmp(clk->name, clk_name, cnt) == 0) {
			clock_get = clk_get(NULL, clk->name);
			if (IS_ERR(clock_get)) {
				pr_err("%s clock_get failed!\n", clk->name);
				err = -EINVAL;
				goto out;
			}
			clock_core = clock_get->core;
			if (!clock_core) {
				pr_err("%s get failed!\n", clk->name);
				err = -EINVAL;
				goto out_put;
			}
			ret = clock_get_reg(clock_core);
			err = cnt;
			goto out_put;
		}
	}
	pr_err("clk name error!\n\n");
	err = -EINVAL;

out_put:
	clk_put(clock_get);
out:
	mutex_unlock(&clock_list_lock);
	kfree(clk_name);
	return err;
}

MODULE_SHOW_DEFINE(clock_tree);
MODULE_SHOW_DEFINE(clock_get);
MODULE_SHOW_DEFINE(clock_lookup);
MODULE_SHOW_DEFINE(clock_enable);
MODULE_SHOW_DEFINE(clock_disable);
MODULE_SHOW_DEFINE(clock_getrate);
MODULE_SHOW_DEFINE(clock_getparent);

static int clock_store_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

#define MODULE_STORE_DEFINE(func_name)                                  \
	static const struct file_operations func_name##_fops = {        \
	.open           = clock_store_open,                             \
	.write          = func_name##_store,                            \
}

MODULE_STORE_DEFINE(clock_enable);
MODULE_STORE_DEFINE(clock_disable);
MODULE_STORE_DEFINE(clock_getrate);
MODULE_STORE_DEFINE(clock_setrate);
MODULE_STORE_DEFINE(clock_getparent);
MODULE_STORE_DEFINE(clock_setparent);
MODULE_STORE_DEFINE(clock_getreg);

/* ***************** find all leaf clocks begin ******************** */
static bool find_clock_by_clkname(const char **clock_names,
	unsigned int clock_num, const char *clkname)
{
	unsigned int i;

	for (i = 0; i < clock_num; i++)
		if (strcmp(clkname, clock_names[i]) == 0)
			return true;

	return false;
}

static void __find_all_leaf_clocks(const char **clock_names,
	unsigned int *p_leaf_clock_num)
{
	struct clk_core *clk = NULL;
	struct clk_core *child = NULL;
	unsigned int clock_num = 0;
	unsigned int clock_idx = 0;
	unsigned int child_num;
	unsigned int leaf_clock_num = 0;

	mutex_lock(&clock_list_lock);
	list_for_each_entry(clk, &clocks, node)
		clock_names[clock_num++] = clk->name;

	list_for_each_entry(clk, &clocks, node) {
		child_num = 0;
		hlist_for_each_entry(child, &clk->children, child_node) {
			if (find_clock_by_clkname(clock_names, clock_num,
				child->name))
				child_num++;
		}

		if (!child_num) {
			const char *temp = clock_names[leaf_clock_num];

			clock_names[leaf_clock_num] = clock_names[clock_idx];
			clock_names[clock_idx] = temp;
			leaf_clock_num++;
		}
		clock_idx++;
	}
	mutex_unlock(&clock_list_lock);

	*p_leaf_clock_num = leaf_clock_num;
}

static void find_all_leaf_clocks(struct seq_file *s)
{
	const unsigned int MAX_CLOCK_NUM = 1024; /* clock numbers */
	const char **clock_names = NULL;
	unsigned int clock_idx = 0;
	unsigned int leaf_clock_num = 0;

	if (IS_ERR_OR_NULL(s))
		return;

	clock_names = kzalloc(sizeof(char *) * MAX_CLOCK_NUM, GFP_KERNEL);
	if (!clock_names) {
		pr_err("[%s] failed to alloc clock_names!\n", __func__);
		return;
	}

	__find_all_leaf_clocks(clock_names, &leaf_clock_num);

	for (clock_idx = 0; clock_idx < leaf_clock_num; clock_idx++)
		seq_printf(s, "%s\n", clock_names[clock_idx]);

	kfree(clock_names);
	seq_printf(s, "---------------------\nTotally %u leaf clocks found!\n",
		leaf_clock_num);
}

int clock_getleaf_show(struct seq_file *s, void *data)
{
	find_all_leaf_clocks(s);
	return 0;
}
MODULE_FUNCS_DEFINE(clock_getleaf);
MODULE_SHOW_DEFINE(clock_getleaf);
/* ***************** find all leaf clocks end ******************** */

static int clk_rate_fops_get(void *data, u64 *rate)
{
	struct clk_core *clk = data;
	*rate = clk->rate;

	return 0;
};

static int clk_rate_fops_set(void *data, u64 rate)
{
	struct clk_core *clk = data;
	int ret;

	ret = clk_prepare_enable(clk->hw->clk);
	if (ret)
		return ret;

	ret = clk_set_rate(clk->hw->clk, rate);
	if (ret < 0)
		pr_err("%s clk_set_rate fail, ret=%d\n", __func__, ret);

	clk_disable_unprepare(clk->hw->clk);
	return ret;
};

DEFINE_SIMPLE_ATTRIBUTE(clk_rate_fops, clk_rate_fops_get, clk_rate_fops_set, "%llu\n");

void __clk_statcheck(struct clk_core *clk)
{
	if (WARN_ON(clk_core_is_enabled(clk) == false))
		pr_err("%s stat exception! cnt is %u\n", clk->name,
			clk->enable_count);
}

#define MAX_LEVEL_NUM 16
void clk_reg_summary_show_one(struct seq_file *s, struct clk_core *c, int level)
{
	if (!c) {
		pr_err("%s c is null\n", __func__);
		return;
	}

	if (level > MAX_LEVEL_NUM)
		return;

	/* 50: length for level, 3: space for level */
	seq_printf(s, "%*s%-*s %5d  %11lu", level * 3 + 1, "", 50 - level * 3,
		c->name, c->enable_count, clk_core_get_rate(c));
	clk_tree_dump_reg(s, c);
	seq_printf(s, "\n");
}

void clk_reg_summary_show_subtree(struct seq_file *s, struct clk_core *c,
	int level)
{
	struct clk_core *child = NULL;

	if (!c)
		return;

	clk_reg_summary_show_one(s, c, level);

	hlist_for_each_entry(child, &c->children, child_node)
		clk_reg_summary_show_subtree(s, child, level + 1);
}

int clk_reg_summary_show(struct seq_file *s, void *data)
{
	struct clk_core *c = NULL;
	struct hlist_head **lists = (struct hlist_head **)s->private;

	clk_base_addr_print(s);
	for (; *lists; lists++)
		hlist_for_each_entry(c, *lists, child_node)
			clk_reg_summary_show_subtree(s, c, 0);
	return 0;
}

static int clk_reg_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_reg_summary_show, inode->i_private);
}

static const struct file_operations clk_reg_summary_fops = {
	.open           = clk_reg_summary_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static void create_test_all_clocks_file(struct dentry *pdentry)
{
	debugfs_create_file("clk_get", PRIV_AUTH, pdentry, NULL,
		&clock_get_show_fops);
	debugfs_create_file("clk_lookup", PRIV_AUTH, pdentry, NULL,
		&clock_lookup_show_fops);
	debugfs_create_file("clk_enable", PRIV_AUTH, pdentry, NULL,
		&clock_enable_show_fops);
	debugfs_create_file("clk_disable", PRIV_AUTH, pdentry, NULL,
		&clock_disable_show_fops);
	debugfs_create_file("clk_getparent", PRIV_AUTH, pdentry, NULL,
		&clock_getparent_show_fops);
	debugfs_create_file("clk_getrate", PRIV_AUTH, pdentry, NULL,
		&clock_getrate_show_fops);
	debugfs_create_file("clock_getleaf", PRIV_AUTH, pdentry, NULL,
		&clock_getleaf_show_fops);
}

static void create_test_one_clock_file(struct dentry *pdentry)
{
	debugfs_create_file("enable", PRIV_MODE, pdentry, NULL,
		&clock_enable_fops);
	debugfs_create_file("disable", PRIV_MODE, pdentry, NULL,
		&clock_disable_fops);
	debugfs_create_file("get_parent", PRIV_MODE, pdentry, NULL,
		&clock_getparent_fops);
	debugfs_create_file("set_parent", PRIV_MODE, pdentry, NULL,
		&clock_setparent_fops);
	debugfs_create_file("get_rate", PRIV_MODE, pdentry, NULL,
		&clock_getrate_fops);
	debugfs_create_file("set_rate", PRIV_MODE, pdentry, NULL,
		&clock_setrate_fops);
	debugfs_create_file("get_reg", PRIV_MODE, pdentry, NULL,
		&clock_getreg_fops);
	create_clk_test_debugfs(pdentry);
}

int plat_clk_debug_init(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	struct dentry *pdentry = NULL;

	clock = debugfs_create_dir("clock", NULL);
	if (!clock)
		return -ENOMEM;

	test_one_clock = debugfs_create_dir("test_one_clock", clock);
	if (!test_one_clock)
		return -ENOMEM;

	test_all_clocks = debugfs_create_dir("test_all_clocks", clock);
	if (!test_all_clocks)
		return -ENOMEM;

	debugfs_create_file("clock_tree", S_IRUGO, clock, NULL,
		&clock_tree_show_fops);

	pdentry = debugfs_create_file("clk_reg_summary", S_IRUGO, clock,
		(void *)clk_all_lists_debug, &clk_reg_summary_fops);
	if (!pdentry)
		return -ENOMEM;

	pdentry = test_all_clocks;
	create_test_all_clocks_file(pdentry);

	pdentry = test_one_clock;
	create_test_one_clock_file(pdentry);

#endif
	return 0;
}

void clk_list_add(struct clk_core *clk)
{
	mutex_lock(&clock_list_lock);
	list_add(&clk->node, &clocks);
	mutex_unlock(&clock_list_lock);
}

struct dentry *debugfs_create_clkfs(struct clk_core *clk)
{
	if (!clk)
		return NULL;
#ifdef CONFIG_HISI_DEBUG_FS
	return debugfs_create_file("clk_rate", S_IWUSR | S_IRUGO, clk->dentry,
		clk, &clk_rate_fops);
#else
	return NULL;
#endif
}

#ifdef CONFIG_HISI_CLK_TRACE
static void __track_clk(struct clk *clk, enum track_clk_type track_item,
	u32 freq)
{
	struct clk_record_info info;
	u8 cpu;
	const char *clk_name = NULL;

	if (!g_clk_hook_on)
		return;
	if (track_item >= TRACK_CLK_MAX) {
		pr_err("[%s], track_type [%d] is invalid!\n", __func__,
			track_item);
		return;
	}

	cpu = (u8) raw_smp_processor_id();
	if (clk_sel_buf_type == SINGLE_BUFF)
		cpu = 0;

	info.current_time    = hisi_getcurtime();
	info.item            = track_item;
	info.enable_count    = __clk_get_enable_count(clk);
	info.current_rate    = __clk_get_rate(clk);
	info.cpu_l           = __clk_get_rate(hs_trace_info.clk_cpu_l);
	info.cpu_b           = __clk_get_rate(hs_trace_info.clk_cpu_b);
	if (__clk_get_source(clk) < 0)
		info.ppll    = CLK_ERR_NUM;
	else
		info.ppll    = __clk_get_source(clk);
	info.ddr_freq        = __clk_get_rate(hs_trace_info.clk_ddr);
	info.peri_dvfs_vote0 = readl(CLK_DVFS_ADDR_0(hs_trace_info.pmuctrl));
	info.peri_dvfs_vote1 = readl(CLK_DVFS_ADDR_1(hs_trace_info.pmuctrl));

	clk_name = __clk_get_name(clk);
	if (!clk_name)
		return;
	if (memset_s(info.comm, CLK_NAME_LEN, 0, sizeof(info.comm)) != EOK) {
		pr_err("%s memset_s failed!\n", __func__);
		return;
	}
	if (strncpy_s(info.comm, CLK_NAME_LEN, clk_name,
		strlen(clk_name)) != EOK) {
		pr_err("%s strncpy_s failed!\n", __func__);
		return;
	}
	info.comm[CLK_NAME_LEN - 1] = '\0';
	pr_debug("######%s!\n", info.comm);
	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)g_clk_track_addr->percpu_addr[cpu],
		(u8 *)&info);
}

void print_clk_trace(void)
{
	int ret;

	for (int cpu = 0; cpu < BITS_PER_BYTE; cpu++) {
		pr_info("cpu[%d]\n", cpu);
		do {
			struct clk_record_info info;

			ret = hisiap_ringbuffer_read((struct hisiap_ringbuffer_s *)g_clk_track_addr->percpu_addr[cpu], (u8 *)&info, sizeof(struct clk_record_info));
			if (ret) {
				pr_err("read return -1, cpu = %d\n", cpu);
				break;
			}
			if (strncmp(info.comm, "clk_spi3", strlen("clk_spi3")) != EOK)
				pr_err("%llx %lx %d %d %s\n", info.current_time, info.item, info.enable_count, info.current_rate, info.comm);
		} while (1);
	}
}

void track_clk_enable(struct clk *clk)
{
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	__track_clk(clk, TRACK_ON_OFF, 0);
}
void track_clk_set_freq(struct clk *clk, u32 freq)
{
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	__track_clk(clk, TRACK_SET_FREQ, freq);
}
void track_clk_set_dvfs(struct clk *clk, u32 freq)
{
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	__track_clk(clk, TRACK_SET_DVFS, freq);
}
static void track_clk_reset(u32 modid, u32 etype, u64 coreid)
{
}

static void track_clk_dump(u32 modid, u32 etype, u64 coreid, char *pathname,
	pfn_cb_dump_done pfn_cb)
{
	if (pfn_cb)
		pfn_cb(modid, coreid);

	pr_info("%s dump!\n", __func__);
}

static int track_clk_rdr_register(struct rdr_register_module_result *result)
{
	struct rdr_module_ops_pub s_module_ops;
	int ret = -1;

	pr_info("%s start!\n", __func__);
	if (!result) {
		pr_err("%s para null!\n", __func__);
		return ret;
	}
	s_module_ops.ops_dump  = track_clk_dump;
	s_module_ops.ops_reset = track_clk_reset;
	ret = rdr_register_module_ops(clk_rdr_core_id, &s_module_ops, result);
	pr_info("%s end!\n", __func__);
	return ret;
}

#define ALIGN8(size) ((size / BITS_PER_BYTE) * BITS_PER_BYTE)
static int clk_percpu_buffer_init(u8 *addr, u32 size, u32 fieldcnt,
	u32 magic_number, u32 ratio[][BITS_PER_BYTE], u32 max_cpu_nums,
	enum buf_type_en buf_type)
{
	u32 i;
	int ret;
	u32 cpu_num = num_possible_cpus();
	if (cpu_num > max_cpu_nums) {
		pr_err("[%s] cpu number error!\n", __func__);
		return -1;
	}

	if (buf_type == SINGLE_BUFF)
		cpu_num = 1;
	pr_info("[%s], num_online_cpus [%d]!\n", __func__, num_online_cpus());

	if (IS_ERR_OR_NULL(addr)) {
		pr_err("[%s], buffer_addr [0x%pK], buffer_size [0x%x]\n",
			__func__, addr, size);
		return -1;
	}

	/* set pc info for parse */
	g_clk_track_addr              = (struct pc_record_info *)addr;
	g_clk_track_addr->buffer_addr = addr;
	g_clk_track_addr->buffer_size = size - sizeof(struct pc_record_info);
	g_clk_track_addr->dump_magic  = magic_number;

	/* set per cpu buffer */
	for (i = 0; i < cpu_num; i++) {
		pr_info("[%s], ratio[%u][%d] = [%u]\n", __func__,
			(cpu_num - 1), i, ratio[cpu_num - 1][i]);

		/* 16: elelments for per line */
		g_clk_track_addr->percpu_length[i] =
			g_clk_track_addr->buffer_size / 16 * ratio[cpu_num - 1][i];
		g_clk_track_addr->percpu_length[i] =
			ALIGN8(g_clk_track_addr->percpu_length[i]);

		if (i == 0) {
			g_clk_track_addr->percpu_addr[0] =
				g_clk_track_addr->buffer_addr + sizeof(struct pc_record_info);
		} else {
			g_clk_track_addr->percpu_addr[i] =
			g_clk_track_addr->percpu_addr[i - 1] +
			g_clk_track_addr->percpu_length[i - 1];
		}

		pr_info("[%s], [%d]: percpu_addr [0x%pK], percpu_length [0x%x], fieldcnt [%u]\n",
			__func__, i, g_clk_track_addr->percpu_addr[i],
			g_clk_track_addr->percpu_length[i], fieldcnt);

		ret = hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)g_clk_track_addr->percpu_addr[i],
			g_clk_track_addr->percpu_length[i],
			fieldcnt, "clk");
		if (ret) {
			pr_err("[%s], cpu [%d] ringbuffer init failed!\n",
				__func__, i);
			return ret;
		}
	}
	return 0;
}

static int clk_buffer_init(u8 *addr, u32 size, enum buf_type_en buf_type)
{
	unsigned int record_ratio[BITS_PER_BYTE][BITS_PER_BYTE] = {
		{ 16, 0, 0, 0, 0, 0, 0, 0 },
		{  8, 8, 0, 0, 0, 0, 0, 0 },
		{  6, 5, 5, 0, 0, 0, 0, 0 },
		{  4, 4, 4, 4, 0, 0, 0, 0 },
		{  4, 4, 4, 3, 1, 0, 0, 0 },
		{  4, 4, 3, 3, 1, 1, 0, 0 },
		{  4, 3, 3, 3, 1, 1, 1, 0 },
		{  3, 3, 3, 3, 1, 1, 1, 1 }
	};

	return clk_percpu_buffer_init(addr, size, sizeof(struct clk_record_info),
		CLK_MAGIC_NUM, record_ratio, BITS_PER_BYTE, buf_type);
}

static int get_hs_trace_info(struct device_node *np,
	struct hs_trace_clk_info *trace_info)
{
	trace_info->pmuctrl = of_iomap(np, 0);
	if (!trace_info->pmuctrl) {
		pr_err("[%s]failed to iomap!\n", __func__);
		return -1;
	}

	trace_info->clk_cpu_l = __clk_lookup("cpu-cluster.0");
	if (IS_ERR(trace_info->clk_cpu_l)) {
		pr_err("_clk_get: clk_cpu_l not found!\n");
		return -1;
	}

	trace_info->clk_cpu_b = __clk_lookup("cpu-cluster.1");
	if (IS_ERR(trace_info->clk_cpu_b)) {
		pr_err("_clk_get: clk_cpu_b not found!\n");
		return -1;
	}

	trace_info->clk_ddr = __clk_lookup("clk_ddrc_freq");
	if (IS_ERR(trace_info->clk_ddr)) {
		pr_err("_clk_get: clk_ddr not found!\n");
		return -1;
	}

	return 0;
}

static int __init track_clk_record_init(void)
{
	int ret;
	struct rdr_register_module_result clk_rdr_info;
	unsigned char *vir_addr = NULL;
	struct device_node *np = NULL;

	/* alloc rdr memory and init */
	ret = track_clk_rdr_register(&clk_rdr_info);
	if (ret)
		return ret;

	if (clk_rdr_info.log_len == 0) {
		pr_err("%s clk_rdr_len is 0x0!\n", __func__);
		return 0;
	}
	vir_addr = (unsigned char *)hisi_bbox_map((phys_addr_t)clk_rdr_info.log_addr,
		clk_rdr_info.log_len);

	pr_info("%s log_addr is 0x%llx, log_len is 0x%x!\n", __func__,
		clk_rdr_info.log_addr, clk_rdr_info.log_len);
	if (!vir_addr) {
		pr_err("%s vir_addr err!\n", __func__);
		return -EINVAL;
	}
	/* clean mem 0 */
	ret = memset_s(vir_addr, clk_rdr_info.log_len, 0, clk_rdr_info.log_len);
	if (ret != EOK) {
		pr_err("%s memset_s failed!\n", __func__);
		return -EINVAL;
	}

	ret = clk_buffer_init(vir_addr, clk_rdr_info.log_len, clk_sel_buf_type);
	if (ret) {
		pr_err("%s buffer init err!\n", __func__);
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (!np) {
		pr_err("[%s] fail to find pmctrl node!\n", __func__);
		return -EINVAL;
	}

	ret = get_hs_trace_info(np, &hs_trace_info);
	if (ret)
		return -EINVAL;

	if (check_himntn(HIMNTN_TRACE_CLK_REGULATOR))
		g_clk_hook_on = SWITCH_OPEN;

	pr_err("%s: hook_on = %u,rdr_phy_addr = 0x%llx, rdr_len = 0x%x, rdr_virt_add = 0x%pK\n",
		__func__, g_clk_hook_on, clk_rdr_info.log_addr,
		clk_rdr_info.log_len, vir_addr);
	return 0;
}
module_init(track_clk_record_init);

static void __exit track_clk_record_exit(void)
{
}
module_exit(track_clk_record_exit);
MODULE_LICENSE("GPL");

#endif
