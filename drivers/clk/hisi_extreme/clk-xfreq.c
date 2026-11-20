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
#include "clk-xfreq.h"

static struct device_node *of_get_clk_cpu_node(int cluster)
{
	struct device_node *np = NULL;
	struct device_node *parent = NULL;
	const u32 *mpidr = NULL;

	parent = of_find_node_by_path("/cpus");
	if (parent == NULL) {
		pr_err("failed to find OF /cpus\n");
		return NULL;
	}

	/*
	 * Get first cluster node ; big or little custer core0 must
	 * contain reg and operating-points node
	 * according to DT description, cluster is the 8-15 bit of reg +1
	 */
	for_each_child_of_node(parent, np) {
		mpidr = of_get_property(np, "reg", NULL);
		if (mpidr == NULL) {
			pr_err("%s missing reg property\n", np->full_name);
			of_node_put(np);
			np = NULL;
			break;
		} else if (((be32_to_cpup(mpidr + 1) >> 8) & 0xff) == cluster) {
			if (of_get_property(np, "operating-points-v2", NULL)) {
				pr_debug("cluster%d suppoet operating-points-v2\n", cluster);
				of_node_put(np);
				break;
			} else if (of_get_property(np, "operating-points", NULL)) {
				pr_debug("cluster%d suppoet operating-points-v1\n", cluster);
				of_node_put(np);
				break;
			}
			of_node_put(np);
			pr_err("cluster%d can not find opp v1&v2\n", cluster);
			np = NULL;

			break;
		}
	}

	of_node_put(parent);

	return np;
}

static struct device_node *of_get_xfreq_node(const char *xfreq)
{
	struct device_node *np = NULL;

	if (!strcmp(xfreq, "ddrfreq"))
		np = of_find_compatible_node(NULL, NULL, "hisilicon,ddr_devfreq");
	else if (!strcmp(xfreq, "gpufreq"))
		np = of_find_compatible_node(NULL, NULL, "arm,mali-midgard");
	else if (!strcmp(xfreq, "sysctrl"))
		np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	return np;
}

static int __xfreq_clk_table_opp_np(struct device_node *np,
	struct hi3xxx_xfreq_clk *xfreqclk, int *ret)
{
	struct device_node *opp_np = NULL;
	struct device_node *opp_np_chd = NULL;
	int count = 0;
	int k = 0;
	unsigned long long rate = 0;
	unsigned int volt = 0;

	opp_np = of_parse_phandle(np, "operating-points-v2", 0);
	if (opp_np != NULL) {
		for_each_available_child_of_node(opp_np, opp_np_chd) {
			count++;
			*ret = of_property_read_u64(opp_np_chd, "opp-hz", &rate);
			if (*ret) {
				pr_err("%s: Failed to read opp-hz, %d\n", __func__, *ret);
				return -ENODEV;
			}
			*ret = of_property_read_u32(opp_np_chd, "opp-microvolt", &volt);
			if (*ret) {
				pr_err("%s: Failed to read  opp-microvolt, %d\n", __func__, *ret);
				return -ENODEV;
			}
			if (k >= MAX_FREQ_NUM)
				return -EINVAL;
			xfreqclk->freq[k] = (unsigned int)(rate / FREQ_CONVERSION);
			xfreqclk->volt[k] = volt;
			k++;
		}
		/* There should be one of more OPP defined */
		if (WARN_ON(!count)) {
			*ret = -ENOENT;
			return -ENOENT;
		}
		return -EINVAL;
	}

	return 0;
}

static int xfreq_clk_table_init(struct device_node *np, struct hi3xxx_xfreq_clk *xfreqclk)
{
	const struct property *prop = NULL;
	const __be32 *val = NULL;
	int k = 0;
	int ret = 0;
	unsigned int volt, freq;
	int nr, flag;

	flag = __xfreq_clk_table_opp_np(np, xfreqclk, &ret);
	if (flag)
		return ret;

	prop = of_find_property(np, "operating-points", NULL);
	if (prop == NULL)
		return -ENODEV;
	if (!prop->value)
		return -ENODATA;

	/*
	 * Each OPP is a set of tuples consisting of frequency and
	 * voltage like <freq-kHz vol-uV>.
	 * 2:freq-kHz, vol-uV
	 */
	nr = prop->length / sizeof(u32);
	if ((nr % 2) || (nr / 2) > MAX_FREQ_NUM)
		return -EINVAL;

	xfreqclk->table_length = nr / 2;
	val = prop->value;
	while (nr) {
		freq = be32_to_cpup(val++);
		volt = be32_to_cpup(val++);
		xfreqclk->freq[k] = freq;
		xfreqclk->volt[k] = volt;
		pr_debug("[%s]: the OPP k %d,freq %u, volt %u\n", __func__, k, freq, volt);
		nr -= 2;
		k++;
	}

	return 0;
}

#define FREQ_INDEX_MASK		0xF
static unsigned int hi3xxx_xfreq_clk_get_freq_idx(struct hi3xxx_xfreq_clk *xfreq_clk)
{
	unsigned int sys_bak_reg;

	if (xfreq_clk->sysreg == NULL)
		return 0;

	sys_bak_reg = readl(xfreq_clk->sysreg);

	/*
	 * sysctrl SCBAKDATA4
	 * bit 0-3	LITTLE Cluster
	 * bit 4-7	BIG Cluster
	 * bit 8-11	DDR
	 * bit 12-15	GPU
	 */
	switch (xfreq_clk->id) {
	case LITTLE_CLUSRET_BIT:
		break;
	case BIG_CLUSTER_BIT:
		sys_bak_reg >>= 4;
		break;
	case GPU_BIT:
		sys_bak_reg >>= 12;
		break;
	case DDR_BIT:
		sys_bak_reg >>= 8;
		break;
	default:
		return 0;
	}

	sys_bak_reg &= FREQ_INDEX_MASK;

	if (sys_bak_reg >= xfreq_clk->table_length)
		sys_bak_reg = 0;

	return sys_bak_reg;
}

static unsigned long hi3xxx_xfreq_clk_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct hi3xxx_xfreq_clk *xfreq_clk = container_of(hw, struct hi3xxx_xfreq_clk, hw);
	unsigned int rate, freq_index, mul_ret;

	switch (xfreq_clk->id) {
	/* DDR get freq */
	case DDR_FREQ_ID:
		freq_index = hi3xxx_xfreq_clk_get_freq_idx(xfreq_clk);
		mul_ret = mul_valid_cal(xfreq_clk->freq[freq_index], FREQ_CONVERSION);
		if (!mul_ret)
			return 0;
		rate = xfreq_clk->freq[freq_index] * FREQ_CONVERSION;
		pr_debug("[%s]3 idx=%u rate=%u\n", __func__, freq_index, rate);
		break;
	/* DDR set min */
	default:
		rate = xfreq_clk->rate;
	}
	return rate;
}

static long hi3xxx_xfreq_clk_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *prate)
{
	return rate;
}

static int hi3xxx_xfreq_clk_determine_rate(struct clk_hw *hw, struct clk_rate_request *req)
{
	return 0;
}

static int hi3xxx_xfreq_clk_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	struct hi3xxx_xfreq_clk *xfreq_clk =
		container_of(hw, struct hi3xxx_xfreq_clk, hw);
	/* 1000000:freq conversion HZ_TO_MHZ */
	unsigned long new_rate = rate / 1000000;
	int ret = 0;

	pr_debug("[%s] set rate = %luMHZ\n", __func__, new_rate);
	if (xfreq_clk->vote_method == DDR_HW_VOTE) {
		switch (xfreq_clk->id) {
		case DDRC_MIN_CLK_ID:
		case DDRC_MAX_CLK_ID:
		case DMSS_MIN_CLK_ID:
			new_rate = new_rate | xfreq_clk->ddr_mask;
			writel(new_rate, xfreq_clk->pmreg);
			break;
		default:
			pr_err("[%s]dev_id is error!\n", __func__);
		}
	} else {
		xfreq_clk->set_rate_cmd[1] = new_rate;
#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
		ret = clkmbox_send_msg(xfreq_clk->set_rate_cmd, LPM3_CMD_LEN);
		if (ret < 0) {
			pr_err("[%s]core id:%u fail to send msg to LPM3!\n", __func__, xfreq_clk->id);

			return -EINVAL;
		}
#endif
	}
	xfreq_clk->rate = rate;

	return ret;
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_dump_xfreq_clk(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	if ((buf == NULL) && (s != NULL))
		seq_printf(s, "    %-15s    %-15s", "NONE", "ddr-cpu-clk");

	return 0;
}
#endif

static const struct clk_ops hi3xxx_xfreq_clk_ops = {
	.recalc_rate = hi3xxx_xfreq_clk_recalc_rate,
	.determine_rate = hi3xxx_xfreq_clk_determine_rate,
	.round_rate = hi3xxx_xfreq_clk_round_rate,
	.set_rate = hi3xxx_xfreq_clk_set_rate,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = hi3xxx_dump_xfreq_clk,
#endif
};

static int __hi3xxx_xfreq_lpm3_cmd_setup(const struct xfreq_clock *xfreq,
	struct hi3xxx_xfreq_clk *xfreqclk)
{
	int i;

	for (i = 0; i < LPM3_CMD_LEN; i++) {
		xfreqclk->set_rate_cmd[i] = xfreq->set_rate_cmd[i];
		xfreqclk->get_rate_cmd[i] = xfreq->get_rate_cmd[i];
	}

	return 0;
}

static int __hi3xxx_xfreq_xfreqclk_setup(const struct xfreq_clock *xfreq,
	struct hi3xxx_xfreq_clk *xfreqclk)
{
	struct device_node *xfreq_np = NULL;

	xfreq_np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	xfreqclk->sysreg = of_iomap(xfreq_np, 0);
	xfreq_np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	xfreqclk->pmreg = of_iomap(xfreq_np, 0);

	if (xfreq->ddr_mask)
		xfreqclk->ddr_mask = xfreq->ddr_mask;
	else
		xfreqclk->ddr_mask = DDR_FREQ_MASK;

	xfreqclk->id = xfreq->clock_id;
	xfreqclk->vote_method = xfreq->vote_method;

	if (xfreqclk->sysreg == NULL) {
		pr_err("[%s] iomap fail!\n", __func__);
		return -EINVAL;
	}

	if (xfreqclk->pmreg == NULL) {
		pr_err("[%s]pmreg iomap fail!\n", __func__);
		iounmap(xfreqclk->sysreg);
		xfreqclk->sysreg = NULL;
		return -EINVAL;
	}
	xfreqclk->sysreg += xfreq->scbakdata;
	xfreqclk->pmreg += xfreq->scbakdata;

	return 0;
}

static void __hi3xxx_xfreqclk_sort(struct hi3xxx_xfreq_clk *xfreqclk)
{
	int i, k, temp;

	for (i = 0; i < MAX_FREQ_NUM - 1; i++) {
		for (k = MAX_FREQ_NUM - 1; k > i; k--) {
			if (xfreqclk->freq[k] < xfreqclk->freq[k - 1]) {
				temp = xfreqclk->freq[k];
				xfreqclk->freq[k] =
				xfreqclk->freq[k - 1];
				xfreqclk->freq[k - 1] = temp;
			}
		}
	}
}

static int __hi3xxx_xfreqclk_set(struct hi3xxx_xfreq_clk *xfreqclk, u32 device_id)
{
	struct device_node *xfreq_np = NULL;
	unsigned int freq_index, mul_ret;
	int i, k, ret;

	switch (device_id) {
	case CPU_CLUSTER_0:
	case CPU_CLUSTER_1:
		xfreq_np = of_get_clk_cpu_node(device_id);
		if (!xfreq_clk_table_init(xfreq_np, xfreqclk)) {
			freq_index = hi3xxx_xfreq_clk_get_freq_idx(xfreqclk);
			mul_ret = mul_valid_cal(xfreqclk->freq[freq_index], FREQ_CONVERSION);
			if (!mul_ret)
				return -EINVAL;
			xfreqclk->rate = xfreqclk->freq[freq_index] * FREQ_CONVERSION;
		}
		return CLK_SET_OK;
	case CLK_G3D:
		xfreq_np = of_get_xfreq_node("gpufreq");
		if (!xfreq_clk_table_init(xfreq_np, xfreqclk)) {
			freq_index = hi3xxx_xfreq_clk_get_freq_idx(xfreqclk);
			mul_ret = mul_valid_cal(xfreqclk->freq[freq_index], FREQ_CONVERSION);
			if (!mul_ret)
				return -EINVAL;
			xfreqclk->rate = xfreqclk->freq[freq_index] * FREQ_CONVERSION;
		}
		return CLK_SET_OK;
	case CLK_DDRC_FREQ:
	case CLK_DDRC_MAX:
	case CLK_DDRC_MIN:
		xfreq_np = of_get_xfreq_node("ddrfreq");
		ret = xfreq_clk_table_init(xfreq_np, xfreqclk);
		if (ret)
			return ret;
		/* sort lowtohigh */
		__hi3xxx_xfreqclk_sort(xfreqclk);

		k = 0;
		for (i = 0; i < MAX_FREQ_NUM; i++) {
			pr_debug("1xfreqclk->freq[i]=%u\n", xfreqclk->freq[i]);
			if (xfreqclk->freq[i] == 0) {
				k++;
				continue;
			}
			xfreqclk->freq[i - k] = xfreqclk->freq[i];
		}
		if (k > 0) {
			for (i = MAX_FREQ_NUM - 1; i > MAX_FREQ_NUM - k - 1; i--)
				xfreqclk->freq[i] = 0;
		}
		for (i = 0; i < MAX_FREQ_NUM; i++)
			pr_debug("2xfreqclk->freq[i]=%u\n", xfreqclk->freq[i]);
		return CLK_SET_OK;
	case CLK_DMSS_MIN:
		return CLK_SET_OK;
	default:
		pr_err("[%s]dev_id is error!\n", __func__);
	}

	return 0;
}

/* xfreq_clk is used for cpufreq & devfreq */
static struct clk *__clk_register_xfreq(const struct xfreq_clock *xfreq,
	struct clock_data *data)
{
	struct clk_init_data init;
	struct hi3xxx_xfreq_clk *xfreqclk = NULL;
	struct clk *clk = NULL;
	int ret;

	xfreqclk = kzalloc(sizeof(*xfreqclk), GFP_KERNEL);
	if (xfreqclk == NULL)
		return clk;

	init.name = xfreq->name;
	init.ops = &hi3xxx_xfreq_clk_ops;
	init.parent_names = NULL;
	init.num_parents = 0;
	init.flags = CLK_IS_ROOT | CLK_GET_RATE_NOCACHE;

	xfreqclk->hw.init = &init;
	ret = __hi3xxx_xfreq_xfreqclk_setup(xfreq, xfreqclk);
	if (ret)
		goto err_init;

	ret = __hi3xxx_xfreq_lpm3_cmd_setup(xfreq, xfreqclk);
	if (ret)
		goto err_init;

	ret = __hi3xxx_xfreqclk_set(xfreqclk, xfreqclk->id);
	if (!ret)
		goto err_init;

	clk = clk_register(NULL, &xfreqclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register xfreqclk %s!\n",
				__func__, xfreq->name);
		goto err_init;
	}

	return clk;
err_init:
	kfree(xfreqclk);
	return clk;
}

void plat_clk_register_xfreq(const struct xfreq_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_xfreq(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}

#ifdef CONFIG_HISI_CLK_DEBUG
		debug_clk_add(clk, CLOCK_ROOT);
#endif

		clk_log_dbg("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);

		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
}

