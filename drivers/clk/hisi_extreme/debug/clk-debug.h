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

#ifndef __CLK_DEBUG_H
#define __CLK_DEBUG_H

#include <linux/version.h>
#include <linux/clk-provider.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/seq_file.h>
#include <linux/clk/clk-conf.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/io.h>
#include <soc_acpu_baseaddr_interface.h>
#ifdef CONFIG_HISI_CLK_TRACE
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <linux/hisi/rdr_pub.h>
#include <soc_pmctrl_interface.h>
#include <linux/of_address.h>
#endif
#include <soc_pmctrl_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_crgperiph_interface.h>
#if defined(SOC_ACPU_MEDIA1_CRG_BASE_ADDR)
#include <soc_media1_crg_interface.h>
#endif
#if defined(SOC_ACPU_MEDIA2_CRG_BASE_ADDR)
#include <soc_media2_crg_interface.h>
#endif
#include <soc_iomcu_interface.h>
#include <pmic_interface.h>
#include <soc_pctrl_interface.h>
#include <linux/of_address.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/delay.h>
#include <securec.h>

/* Clock Types */
enum clk_type_id {
	CLOCK_FIXED = 0,
	CLOCK_GENERAL_GATE,
	CLOCK_HIMASK_GATE,
	CLOCK_MUX,
	CLOCK_DIV,
	CLOCK_FIXED_DIV,
	CLOCK_PPLL,
	CLOCK_PMU,
	CLOCK_IPC,
	CLOCK_ROOT,
	CLOCK_DVFS,
	CLOCK_FAST_CLOCK,
	MAX_CLOCK_TYPE,
};

#define MAX_CLK_NUMS 512
#define MAX_CLK_NAME_LEN 64
struct clk_debug_data {
	struct clk_core **clks;
	int size;
};

#define CLK_ADDR_HIGH_MASK  0xFFFFFFFFFFFFF000
#define CLK_ADDR_LOW_MASK   0xFFF
#define PRIV_MODE	    (S_IWUSR | S_IWGRP)
#define PRIV_AUTH	    (S_IRUSR | S_IWUSR | S_IRGRP)

#define MODULE_FUNCS_DEFINE(func_name)                                   \
static int func_name##_open(struct inode *inode, struct file *file)      \
{                                                                        \
	return single_open(file, func_name##_show, inode->i_private);    \
}

#define MODULE_SHOW_DEFINE(func_name)                                   \
	static const struct file_operations func_name##_show_fops = {   \
	.open           = func_name##_open,                             \
	.read           = seq_read,                                     \
	.llseek         = seq_lseek,                                    \
	.release        = single_release,                               \
}

extern struct hlist_head *clk_all_lists_debug[];
extern struct list_head clocks;

void debug_clk_add(struct clk *clk, enum clk_type_id clk_type);
int get_clock_type(struct clk_core *clk);

extern char *hs_base_addr_transfer(unsigned long int base_addr);
void clk_tree_dump_reg(struct seq_file *s, struct clk_core *clk_core);
void clk_base_addr_print(struct seq_file *s);

extern struct clk *clk_get_parent_by_index(struct clk *clk, u8 index);
extern bool clk_core_is_enabled(struct clk_core *clk);
extern unsigned long clk_core_get_rate(struct clk_core *clk);

int plat_clk_debug_init(void);
void clk_list_add(struct clk_core *clk);
struct dentry *debugfs_create_clkfs(struct clk_core *clk);
void create_clk_test_debugfs(struct dentry *pdentry);
struct clk_core *find_clock(const char *clk_name);
struct clk_core *__clk_core_get_parent(struct clk_core *clk);

#ifdef CONFIG_HISI_CLK_TRACE
extern int __clk_get_source(struct clk *clk);
void track_clk_enable(struct clk *clk);
void track_clk_set_freq(struct clk *clk, u32 freq);
void track_clk_set_dvfs(struct clk *clk, u32 freq);
void print_clk_trace(void);
#else
void print_clk_trace(void) {};
#endif
#endif
