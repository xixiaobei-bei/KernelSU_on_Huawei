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
#include "clk-mclk.h"

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
#define CLK_DVFS_IPC_CMD			0xC
static u32 g_count_num_dvfs = 0;
#endif

static int hi3xxx_mclk_prepare(struct clk_hw *hw)
{
	struct hi3xxx_mclk *mclk = container_of(hw, struct hi3xxx_mclk, hw);
#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	s32 ret = 0;
#endif
	mclk->ref_cnt++;

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	/* notify m3 when the ref_cnt of mclk is 1 */
	if (mclk->gate_abandon_enable)
		return 0;

	if (mclk->en_cmd[1] == CLK_DVFS_IPC_CMD) {
		g_count_num_dvfs++;
		if (g_count_num_dvfs == 1) {
			ret = clkmbox_send_msg_sync(&mclk->en_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to enable clk, ret = %d!\n", __func__, ret);
		}
	} else {
		if (mclk->ref_cnt == 1) {
			ret = clkmbox_send_msg_sync(&mclk->en_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to enable clk, ret = %d!\n", __func__, ret);
		}
	}
#endif

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	return ret;
#else
	return 0;
#endif
}

static void hi3xxx_mclk_unprepare(struct clk_hw *hw)
{
	struct hi3xxx_mclk *mclk = container_of(hw, struct hi3xxx_mclk, hw);
#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	s32 ret;
#endif
	mclk->ref_cnt--;

#ifdef CONFIG_HISI_CLK_MAILBOX_SUPPORT
	/* notify m3 when the ref_cnt of gps_clk is 0 */
	if (mclk->always_on)
		return;
	if (mclk->dis_cmd[1] == CLK_DVFS_IPC_CMD) {
		g_count_num_dvfs--;
		if (!g_count_num_dvfs) {
			ret = clkmbox_send_msg_sync(&mclk->dis_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to disable clk, ret = %d!\n", __func__, ret);
		}
	} else {
		if (!mclk->ref_cnt) {
			ret = clkmbox_send_msg_sync(&mclk->dis_cmd[0], LPM3_CMD_LEN);
			if (ret)
				pr_err("[%s] fail to disable clk, ret = %d!\n", __func__, ret);
		}
	}
#endif
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hi3xxx_dump_mclk(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	if ((buf == NULL) && (s != NULL))
		seq_printf(s, "    %-15s    %-15s", "NONE", "ipc-clk");

	return 0;
}
#endif

static struct clk_ops hi3xxx_mclk_ops = {
	.prepare	= hi3xxx_mclk_prepare,
	.unprepare	= hi3xxx_mclk_unprepare,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = hi3xxx_dump_mclk,
#endif
};

static struct clk *__clk_register_mclk(const struct mailbox_clock *mailbox_clk,
	struct clock_data *data)
{
	struct hi3xxx_mclk *mclk = NULL;
	struct clk_init_data init;
	struct clk *clk = NULL;
	unsigned int i;
	struct hs_clk *hs_clk = get_hs_clk_info();

	mclk = kzalloc(sizeof(*mclk), GFP_KERNEL);
	if (!mclk) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		goto err_mclk;
	}

	init.name = mailbox_clk->name;
	init.ops = &hi3xxx_mclk_ops;
	init.flags = CLK_SET_RATE_PARENT;
	init.parent_names = &(mailbox_clk->parent_name);
	init.num_parents = 1;

	for (i = 0; i < LPM3_CMD_LEN; i++) {
		mclk->en_cmd[i] = mailbox_clk->en_cmd[i];
		mclk->dis_cmd[i] = mailbox_clk->dis_cmd[i];
	}
	mclk->always_on = mailbox_clk->always_on;
	mclk->gate_abandon_enable = mailbox_clk->gate_abandon_enable;
	/* initialize the reference count */
	mclk->ref_cnt = 0;
	mclk->lock = &hs_clk->lock;
	mclk->hw.init = &init;

	clk = clk_register(NULL, &mclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, mailbox_clk->name);
		goto err_init;
	}

	return clk;
err_init:
	kfree(mclk);
err_mclk:
	return clk;
}

void plat_clk_register_mclk(const struct mailbox_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i, ret;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_mclk(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}

#ifdef CONFIG_HISI_CLK_DEBUG
		debug_clk_add(clk, CLOCK_IPC);
#endif

		if (clks[i].alias) {
			ret = clk_register_clkdev(clk, clks[i].alias, NULL);
			if (ret)
				pr_err("%s: failed to register clock lookup for %s",
					__func__, clks[i].alias);
		}

		clk_log_dbg("clks id %u, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);
		if (clks[i].id < data->clk_data.clk_num)
			data->clk_data.clks[clks[i].id] = clk;
		else
			pr_err("[QYZ]%s: clks id %u >= nums %u, clks name = %s!\n",
				__func__, clks[i].id, data->clk_data.clk_num, clks[i].name);
	}
}
EXPORT_SYMBOL_GPL(plat_clk_register_mclk);

