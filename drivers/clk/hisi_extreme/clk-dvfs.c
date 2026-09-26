/*
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#include "clk-dvfs.h"

#ifdef CONFIG_HISI_CLK_LOW_TEMPERATURE_JUDGE_BY_VOLT
/* for get temprature */
static int clk_peri_get_temprature(struct peri_volt_poll *pvp)
{
	unsigned int temprature;
	int ret = 0;

	if (hwspin_lock_timeout((struct hwspinlock *)pvp->priv, HWLOCK_TIMEOUT)) {
		pr_err("pvp hwspinlock timout!\n");
		return -ENOENT;
	}
	temprature = readl(pvp->addr_0);
	temprature &= PMCTRL_PERI_CTRL4_TEMPERATURE_MASK;
	temprature = temprature >> PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT;
	if (temprature != NORMAL_TEMPRATURE)
		ret = -1;

	hwspin_unlock((struct hwspinlock *)pvp->priv);

	return ret;
}
#endif

#ifdef CONFIG_HISI_PERIDVFS
#if defined(CONFIG_HISI_HW_PERI_DVS)
static int clk_ip_avs_poll(struct peri_dvfs_clk *dfclk, bool flag)
{
	struct peri_volt_poll *pvp = NULL;
	int ret;

	if (!dfclk)
		return -EINVAL;

	pvp = peri_volt_poll_get(dfclk->id, NULL);
	if (!pvp) {
		pr_err("[%s] get pvp dev_id %u get failed!\n",
			__func__, dfclk->id);
		return -EINVAL;
	}

	ret = peri_set_avs(pvp, flag);
	if (ret < 0) {
		pr_err("[%s] pvp dev_id %u valid set failed!\n",
			__func__, dfclk->id);
		return ret;
	}
	return 0;
}

static int wait_avs_complete(struct peri_dvfs_clk *dfclk)
{
	if (!dfclk)
		return -EINVAL;

	/* nothing to do */
	return 0;
}
#else
static int clk_ip_avs_poll(struct peri_dvfs_clk *dfclk, bool flag)
{
	struct peri_volt_poll *pvp = NULL;
	int ret = 0;
	u32 val;

	/* Base AVS IP No need to poll AVS */
	if (dfclk->avs_poll_id <= 0)
		return ret;
	if (dfclk->avs_poll_id >= AVS_MAX_ID) {
		pr_err("[%s]avs id illegal!\n", __func__);
		return -EINVAL;
	}
	pvp = peri_volt_poll_get(dfclk->id, NULL);
	if (pvp == NULL) {
		pr_err("[%s]pvp get failed, dev_id = %u!\n", __func__, dfclk->id);
		return -EINVAL;
	}
	/*
	 * Because wrong volt will be set when low temp turn to normal,
	 * Low temp also need avs poll.
	 */
	if (hwspin_lock_timeout((struct hwspinlock *)pvp->priv, HWLOCK_TIMEOUT)) {
		pr_err("[%s]pvp hwspinlock timout!\n", __func__);
		return -ENOENT;
	}
	val = readl(dfclk->reg_base + SC_SCBAKDATA24_ADDR);
	if (flag == AVS_ENABLE_PLL)
		val = val | BIT((unsigned int)dfclk->avs_poll_id);
	else
		val = val & (~BIT((unsigned int)dfclk->avs_poll_id));

	writel(val, dfclk->reg_base + SC_SCBAKDATA24_ADDR);

	clk_log_dbg("val = 0x%x, avs id = %d, clk name = %s, SCDATA24 = 0x%x, flag = %d!\n",
		val, dfclk->avs_poll_id, dfclk->hw.core->name,
		readl(dfclk->reg_base + SC_SCBAKDATA24_ADDR), flag);
	hwspin_unlock((struct hwspinlock *)pvp->priv);

	return ret;
}

static int wait_avs_complete(struct peri_dvfs_clk *dfclk)
{
	struct peri_volt_poll *pvp = NULL;
	int loop = PERI_AVS_LOOP_MAX;
	unsigned int val;
	int ret = 0;

	/* Base AVS IP No need to wait AVS */
	if (dfclk->avs_poll_id <= 0)
		return ret;
	pvp = peri_volt_poll_get(dfclk->id, NULL);
	if (pvp == NULL) {
		pr_err("[%s]pvp get failed, dev_id = %u!\n", __func__, dfclk->id);
		return -EINVAL;
	}

	if (peri_get_temperature(pvp) != NORMAL_TEMPRATURE)
		return ret;

	do {
		val = readl(dfclk->reg_base + SC_SCBAKDATA24_ADDR);
		val = (val >> AVS_BITMASK_FLAG) & 0x1;
		if (!val) {
			loop--;
			/* AVS complete timeout is about 80us * 400 ~ 120us * 400 */
			usleep_range(80, 120);
		}
	} while (!val && loop > 0);

	if (!val) {
		pr_err("[%s]:clk prepare wait for avs bitmask timeout, loop = %d, clk name = %s,"
			"pmctrl 0x350 = 0x%x, 0x354 = 0x%x, SCData24 = 0x%x!\n",
			__func__, loop, dfclk->hw.core->name, readl(pvp->addr_0), readl(pvp->addr),
			readl(dfclk->reg_base + SC_SCBAKDATA24_ADDR));
		ret = -EINVAL;
	}

	return ret;
}
#endif /* CONFIG_HW_PERI_DVS */

static int peri_temperature(struct peri_dvfs_clk *pclk)
{
	struct peri_volt_poll *pvp = NULL;
	int ret = 0;

	if (pclk->low_temperature_property != LOW_TEMPERATURE_PROPERTY)
		return 0;

	pvp = peri_volt_poll_get(pclk->id, NULL);
	if (pvp == NULL) {
		pr_err("[%s]pvp get failed, dev_id = %u!\n", __func__, pclk->id);
		return -EINVAL;
	}

	if (peri_get_temperature(pvp) != NORMAL_TEMPRATURE)
		ret = -1;

	return ret;
}

static int peri_dvfs_set_rate_nolock(struct clk *friend_clk,
	unsigned long divider_rate, unsigned long rate, unsigned int dev_id)
{
	int ret;

	/* rate div 2 or 3 for SW switch high freq problem */
	ret = clk_set_rate_nolock(friend_clk, divider_rate);
	if (ret < 0)
		pr_err("[%s]set half rate failed in set rate, ret = %d, poll id = %u, divider_rate = %lu!\n",
			__func__, ret, dev_id, divider_rate);
	ret = clk_set_rate_nolock(friend_clk, rate);
	if (ret < 0)
		pr_err("[%s] fail to updata rate, ret = %d!\n", __func__, ret);

	return ret;
}

static int peri_dvfs_set_rate_lock(struct clk *friend_clk, unsigned long divider_rate,
	unsigned long rate, unsigned int dev_id)
{
	int ret;

	/* rate div 2 or 3 for SW switch high freq problem */
	ret = clk_set_rate(friend_clk, divider_rate);
	if (ret < 0)
		pr_err("[%s]set half rate failed in set rate, ret = %d, poll id = %u, divider_rate = %lu!\n",
			__func__, ret, dev_id, divider_rate);
	ret = clk_set_rate(friend_clk, rate);
	if (ret < 0)
		pr_err("[%s] set linkage failed, ret = %d!\n", __func__, ret);

	return ret;
}

/* func: async dvfs func */
static int dvfs_block_func(struct peri_volt_poll *pvp, u32 volt)
{
	unsigned int volt_get;
	int loop = PERI_AVS_LOOP_MAX;
	int ret = 0;

	do {
		volt_get = peri_get_volt(pvp);
		if (volt_get > DVFS_MAX_VOLT) {
			pr_err("[%s]get volt illegal volt = %u!\n", __func__, volt_get);
			return -EINVAL;
		}
		if (volt_get < volt) {
			loop--;
			/* AVS complete timeout is about 150us * 400 ~ 300us * 400 */
			usleep_range(150, 300);
		}
	} while (volt_get < volt && loop > 0);
	if (volt_get < volt) {
		pr_err("[%s]schedule up volt failed, volt_get = %u, target_volt = %u,"
			"loop = %d, pmctrl 0x350 = 0x%x, 0x354 = 0x%x!\n",
			__func__, volt_get, volt, loop, readl(pvp->addr_0), readl(pvp->addr));
		ret = -EINVAL;
	}

	return ret;
}

static int __updata_freq_set_volt(struct peri_dvfs_switch_up *sw)
{
	struct peri_volt_poll *pvp = NULL;
	struct clk *friend_clk = NULL;
	unsigned long freq_old, target_freq, target_volt, divider_rate;
	int ret;

	mutex_lock(&dvfs_lock);
	pvp = sw->pvp;
	friend_clk = sw->linkage;
	target_freq = sw->target_freq;
	target_volt = sw->target_volt;
	divider_rate = sw->divider_rate;
	mutex_unlock(&dvfs_lock);

	freq_old = clk_get_rate(friend_clk);
	/*
	 * rasing rate first set volt, then set rate and
	 * droping rate first set rate, then set volt!
	 */
	if (target_freq > freq_old) {
		ret = peri_set_volt(pvp, target_volt);
		if (ret < 0) {
			pr_err("[%s]set volt failed ret = %d!\n", __func__, ret);
			return ret;
		}
		ret = dvfs_block_func(pvp, target_volt);
		if (ret != 0)
			return ret;
		/* rate div 2 or 3 for SW switch high freq problem */
		ret = peri_dvfs_set_rate_lock(friend_clk, divider_rate,
			target_freq, pvp->dev_id);
		if (ret < 0)
			pr_err("[%s]fail to updata rate int rasing rate, ret = %d!\n",
				__func__, ret);
	} else {
		ret = peri_dvfs_set_rate_lock(friend_clk, divider_rate,
			target_freq, pvp->dev_id);
		if (ret < 0) {
			pr_err("[%s]fail to updata rate in droping rate, ret = %d!\n",
				__func__, ret);
			return ret;
		}
		ret = peri_set_volt(pvp, target_volt);
		if (ret < 0) {
			pr_err("[%s]set volt failed ret = %d in droping volt!\n",
				__func__, ret);
			ret = clk_set_rate(friend_clk, freq_old);
			if (ret < 0)
				pr_err("[%s] fail to reback async, ret = %d!\n",
					__func__, ret);
		}
	}

	return 0;
}

static void updata_freq_volt_up_fn(struct work_struct *work)
{
	struct peri_dvfs_switch_up *sw =
		container_of(work, struct peri_dvfs_switch_up, updata);
	struct clk *ppll_ap = NULL;
	const char *enable_pll_name = NULL;
	int ret = 0;

	mutex_lock(&dvfs_lock);
	enable_pll_name = sw->enable_pll_name;
	mutex_unlock(&dvfs_lock);

	if (enable_pll_name != NULL) {
		ppll_ap = clk_get(NULL, enable_pll_name);
		if (IS_ERR_OR_NULL(ppll_ap)) {
			pr_err("[%s]cannot get ppll ap clock!", __func__);
			return;
		}

		ret = clk_prepare_enable(ppll_ap);
		if (ret != 0) {
			pr_err("[%s]prepare enable ppll clock fail!\n", __func__);
			goto err_dvfs;
		}
	}
	ret = __updata_freq_set_volt(sw);
	if (ret)
		goto err_clk_unprepare;

err_clk_unprepare:
	if (enable_pll_name)
		clk_disable_unprepare(ppll_ap);

err_dvfs:
	if (enable_pll_name)
		clk_put(ppll_ap);
}
#endif

static long peri_dvfs_clk_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *prate)
{
	return rate;
}

static int peri_dvfs_clk_determine_rate(struct clk_hw *hw, struct clk_rate_request *req)
{
#ifdef CONFIG_HISI_PERIDVFS
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	unsigned int mul_ret;

#ifdef CONFIG_HISI_CLK_LOW_TEMPERATURE_JUDGE_BY_VOLT
	struct peri_volt_poll *pvp = NULL;
	/* for low temperature dvfs */
	if (__clk_is_enabled(hw->clk) == false)
		return 0;

	/* Don't deal with USB */
	if (dfclk->id == USB_POLL_ID)
		return 0;
	pvp = peri_volt_poll_get(dfclk->id, NULL);
	if (pvp == NULL) {
		pr_err("[%s]pvp get failed!\n", __func__);
		return -EINVAL;
	}
	if (clk_peri_get_temprature(pvp)) {
		/* 1:freq level index */
		mul_ret = mul_valid_cal(dfclk->sensitive_freq[1], FREQ_CONVERSION);
		if (!mul_ret)
			return -EINVAL;
		if (req->rate > (dfclk->sensitive_freq[1] * FREQ_CONVERSION)) {
			pr_err("[%s]: cur_freq-%lu > low_temperature_freq-%lu!\n",
				__func__, req->rate, dfclk->sensitive_freq[1]);
			return -ELOW_TEMPERATURE;
		}
	}
#endif

	if (peri_temperature(dfclk)) {
		mul_ret = mul_valid_cal(dfclk->low_temperature_freq, FREQ_CONVERSION);
		if (!mul_ret)
			return -EINVAL;
		if (req->rate > (dfclk->low_temperature_freq * FREQ_CONVERSION)) {
			pr_err("[%s]: clk name =%s, cur_freq-%lu > low_temperature_freq-%lu!\n",
				__func__, hw->core->name, req->rate,
				dfclk->low_temperature_freq);
			return -ELOW_TEMPERATURE;
		}
	}
#endif

	return 0;
}

/* func: get cur freq */
static unsigned long peri_dvfs_clk_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	struct clk *clk_friend = NULL;
	const char *clk_name = NULL;
	u32 rate;

	clk_friend = __clk_lookup(dfclk->link);
	if (IS_ERR_OR_NULL(clk_friend)) {
		pr_err("[%s] %s get failed!\n", __func__, dfclk->link);
		return 0;
	}
	clk_name = __clk_get_name(hw->clk);
	if (IS_ERR_OR_NULL(clk_name)) {
		pr_err("[%s] clk name get failed!\n", __func__);
		return 0;
	}
	rate = clk_get_rate(clk_friend);

	return rate;
}

#ifdef CONFIG_HISI_PERIDVFS
static int __peri_dvfs_divider_rate_set(struct peri_dvfs_clk *dfclk, struct clk *friend_clk,
	unsigned long *divider_rate)
{
	unsigned long freq_old;

	freq_old = clk_get_rate(friend_clk);
	if (!freq_old) {
		pr_err("[%s]soft rate: must not be 0, please check!\n", __func__);
	} else {
		if (dfclk->divider)
			*divider_rate = freq_old / dfclk->divider;
		if (dfclk->divider && (freq_old % dfclk->divider))
			/* Example:For 184.444M to 185M */
			*divider_rate = *divider_rate + FREQ_OFFSET_ADD;
	}

	return freq_old;
}
static int __peri_dvfs_no_wait_avs_set(struct peri_dvfs_clk *dfclk, struct clk *friend_clk,
	struct peri_volt_poll *pvp, unsigned long rate, unsigned int volt)
{
	unsigned long divider_rate = rate;
	unsigned long freq_old;
	int ret;

	freq_old = __peri_dvfs_divider_rate_set(dfclk, friend_clk, &divider_rate);
	ret = peri_dvfs_set_rate_nolock(friend_clk, divider_rate, rate, pvp->dev_id);
	if (ret < 0)
		return ret;
	ret = peri_set_volt(pvp, volt);
	if (ret < 0) {
		pr_err("[%s]set volt failed ret = %d!\n", __func__, ret);
		ret = clk_set_rate_nolock(friend_clk, freq_old);
		if (ret < 0)
			pr_err("[%s] fail to reback, ret = %d!\n", __func__, ret);
		return ret;
	}

	return ret;
}

static int __peri_dvfs_block_mode_set(struct peri_dvfs_clk *dfclk, struct clk *friend_clk,
	struct peri_volt_poll *pvp, unsigned long rate, unsigned int volt)
{
	unsigned long divider_rate = rate;
	unsigned long freq_old;
	int ret;

	freq_old = __peri_dvfs_divider_rate_set(dfclk, friend_clk, &divider_rate);
	ret = peri_set_volt(pvp, volt);
	if (ret < 0) {
		pr_err("[%s]set volt failed ret = %d, old rate = %lu!\n", __func__, ret, freq_old);
		return ret;
	}
	ret = dvfs_block_func(pvp, volt);
	if (ret != 0)
		return -EINVAL;
	ret = wait_avs_complete(dfclk);
	if (ret < 0) {
		pr_err("[%s]:wait avs fail, ret = %d!\n", __func__, ret);
		return ret;
	}
	ret = peri_dvfs_set_rate_nolock(friend_clk, divider_rate, rate, pvp->dev_id);
	if (ret < 0)
		return ret;

	return 0;
}

static int peri_dvfs_set(struct peri_dvfs_clk *dfclk, unsigned long rate,
	unsigned int volt)
{
	struct peri_volt_poll *pvp = NULL;
	struct clk *friend_clk = NULL;
	const char *enable_pll_name = NULL;
	unsigned long divider_rate = rate;
	unsigned long freq_old;
	int ret = 0;

	friend_clk = __clk_lookup(dfclk->link);
	if (IS_ERR_OR_NULL(friend_clk)) {
		pr_err("[%s] %s get failed!\n", __func__, dfclk->link);
		return -ENODEV;
	}
	enable_pll_name = dfclk->enable_pll_name;
	pvp = peri_volt_poll_get(dfclk->id, NULL);
	if (pvp == NULL) {
		pr_err("[%s]pvp get failed!\n", __func__);
		return -EINVAL;
	}

	freq_old = __peri_dvfs_divider_rate_set(dfclk, friend_clk, &divider_rate);

	/*
	 * UNBLOCK_MODE:
	 *      schedule a work to set freq and volt, it relyed on the system schedule;
	 * BLOCK_MODE:
	 *      raise frequency: raise voltage, set frequency
	 *      reduce frequency: reduce frequency, set voltage
	 */
	if (dfclk->block_mode == HS_UNBLOCK_MODE) {
		/* rasing or dropping rate and volt */
		mutex_lock(&dvfs_lock);
		dfclk->sw_up.target_freq = rate;
		dfclk->sw_up.target_volt = volt;
		dfclk->sw_up.pvp = pvp;
		dfclk->sw_up.linkage = friend_clk;
		dfclk->sw_up.divider_rate = divider_rate;
		dfclk->sw_up.enable_pll_name = enable_pll_name;
		mutex_unlock(&dvfs_lock);
		schedule_work(&dfclk->sw_up.updata);
	} else {
		/* block mode */
		if (rate > freq_old) {
			ret = __peri_dvfs_block_mode_set(dfclk, friend_clk, pvp, rate, volt);
			if (ret)
				return ret;
		} else {
			/* Profile down case no need wait AVS and Profile voltage OK */
			ret = __peri_dvfs_no_wait_avs_set(dfclk, friend_clk, pvp, rate, volt);
			if (ret)
				return ret;
		}
	}
	if (dfclk->avs_poll_id > 0)
		clk_log_dbg("IP %s Old rate = %lu, Current rate= %lu!\n",
			dfclk->hw.core->name, freq_old, rate);

	return ret;
}
#endif

#ifdef CONFIG_HISI_PERIDVFS
static int __peri_dvfs_clk_set(struct peri_dvfs_clk *dfclk,
	struct clk *friend_clk, unsigned long rate)
{
	unsigned int level = dfclk->sensitive_level;
	unsigned int i, mul_ret;
	int ret;

	for (i = 0; i < level; i++) {
		mul_ret = mul_valid_cal(dfclk->sensitive_freq[i], FREQ_CONVERSION);
		if (!mul_ret)
			return -EINVAL;
		if (rate > dfclk->sensitive_freq[i] * FREQ_CONVERSION)
			continue;
		ret = peri_dvfs_set(dfclk, rate, dfclk->sensitive_volt[i]);
		if (ret < 0) {
			pr_err("[%s]pvp set volt failed ret = %d!\n", __func__, ret);
			return ret;
		}
		return 0;
	}
	if (i == level) {
		ret = peri_dvfs_set(dfclk, rate, dfclk->sensitive_volt[i]);
		if (ret < 0) {
			pr_err("[%s]pvp set volt failed ret = %d!\n", __func__, ret);
			return ret;
		}
	}

	return 0;
}
#endif

/* func: dvfs set rate main func */
static int peri_dvfs_clk_set_rate(struct clk_hw *hw, unsigned long rate,
	unsigned long parent_rate)
{
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	struct clk *friend_clk = NULL;
	int ret;
#ifdef CONFIG_HISI_PERIDVFS
	unsigned int mul_ret;
#endif

	friend_clk = __clk_lookup(dfclk->link);
	if (IS_ERR_OR_NULL(friend_clk)) {
		pr_err("[%s] %s get failed!\n", __func__, dfclk->link);
		return -ENODEV;
	}
#ifndef CONFIG_HISI_PERIDVFS
	ret = clk_set_rate_nolock(friend_clk, rate);
	if (ret < 0)
		pr_err("[%s] fail to set rate, ret = %d, %d!\n",
			__func__, ret, __LINE__);
#else
	if (peri_temperature(dfclk)) {
		mul_ret = mul_valid_cal(dfclk->low_temperature_freq, FREQ_CONVERSION);
		if (!mul_ret)
			return -EINVAL;
		if (rate > (dfclk->low_temperature_freq * FREQ_CONVERSION)) {
			pr_err("[%s]: cur_rate-%lu > low_temperature_freq-%lu!\n",
				__func__, rate, dfclk->low_temperature_freq);
			return -ELOW_TEMPERATURE;
		}
	}
	if (friend_clk->core->enable_count == 0) {
		ret = clk_set_rate_nolock(friend_clk, rate);
		if (ret < 0)
			pr_err("[%s] fail to set rate, ret = %d, %d!\n",
				__func__, ret, __LINE__);
		goto now;
	}

	ret = __peri_dvfs_clk_set(dfclk, friend_clk, rate);
	if (ret)
		return ret;

now:
#endif
	dfclk->rate = rate;
	return ret;
}

#ifdef CONFIG_HISI_PERIDVFS
static int peri_volt_change(u32 id, u32 volt)
{
	struct peri_volt_poll *pvp = NULL;
	int ret;

	pvp = peri_volt_poll_get(id, NULL);
	if (pvp == NULL) {
		pr_err("[%s]pvp get failed!\n", __func__);
		return -EINVAL;
	}
	ret = peri_set_volt(pvp, volt);
	if (ret < 0) {
		pr_err("[%s]set volt failed ret = %d!\n", __func__, ret);
		return ret;
	}
	ret = dvfs_block_func(pvp, volt);

	return ret;
}

static int __peri_dvfs_volt_change(struct peri_dvfs_clk *pclk, unsigned long cur_rate,
	unsigned int level)
{
	int ret = 0;
	unsigned int i;
	unsigned int mul_ret;

	if (pclk->sensitive_freq[0] > 0) {
		for (i = 0; i < level; i++) {
			mul_ret = mul_valid_cal(pclk->sensitive_freq[i], FREQ_CONVERSION);
			if (!mul_ret)
				return -EINVAL;
			if (cur_rate <= pclk->sensitive_freq[i] * FREQ_CONVERSION) {
				ret = peri_volt_change(pclk->id, pclk->sensitive_volt[i]);
				return ret;
			}
		}
		if (i == level)
			ret = peri_volt_change(pclk->id, pclk->sensitive_volt[i]);
	} else if (pclk->sensitive_freq[0] == 0) {
		ret = peri_volt_change(pclk->id, pclk->sensitive_volt[level]);
	} else {
		pr_err("[%s]soft level: freq must not be less than 0,please check!\n", __func__);
	}

	return ret;
}

static int peri_dvfs_prepare(struct peri_dvfs_clk *pclk)
{
	unsigned long cur_rate;
	unsigned int level = pclk->sensitive_level;
	unsigned int mul_ret;
	int ret;

	cur_rate = clk_get_rate(pclk->hw.clk);
	if (!cur_rate)
		pr_err("[%s]soft rate: must not be 0,please check!\n", __func__);
	if (peri_temperature(pclk)) {
		mul_ret = mul_valid_cal(pclk->low_temperature_freq, FREQ_CONVERSION);
		if (!mul_ret)
			return -EINVAL;
		if (cur_rate > (pclk->low_temperature_freq * FREQ_CONVERSION)) {
			pr_err("[%s]: cur_freq-%lu > low_temperature_freq-%lu!\n",
				__func__, cur_rate, pclk->low_temperature_freq);
			return -ELOW_TEMPERATURE;
		}
	}

	ret = __peri_dvfs_volt_change(pclk, cur_rate, level);
	if (ret < 0)
		pr_err("[%s]pvp set volt failed ret = %d!\n", __func__, ret);
	if (pclk->avs_poll_id > 0)
		clk_log_dbg("IP %s will be enable, current rate = %lu!\n",
			pclk->hw.core->name, cur_rate);

	return ret;
}

static void peri_dvfs_unprepare(struct peri_dvfs_clk *pclk)
{
	int ret;

	/* Clock close case no need wait AVS and Profile voltage OK */
	ret = peri_volt_change(pclk->id, PERI_VOLT_0);
	if (ret < 0)
		pr_err("[%s]peri volt change failed ret = %d!\n", __func__, ret);
}
#endif

static int peri_dvfs_clk_prepare(struct clk_hw *hw)
{
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	struct clk *friend_clk = NULL;
	int ret;
#ifdef CONFIG_HISI_PERIDVFS
	int avs_ret;
#endif

	friend_clk = __clk_lookup(dfclk->link);
	if (IS_ERR_OR_NULL(friend_clk)) {
		pr_err("[%s] %s get failed!\n", __func__, dfclk->link);
		return -ENODEV;
	}

	ret = clk_core_prepare(friend_clk->core);
	if (ret) {
		pr_err("[%s], friend clock prepare faild!\n", __func__);
		return ret;
	}
#ifdef CONFIG_HISI_PERIDVFS
	/* Avs ip poll open clock */
	ret = clk_ip_avs_poll(dfclk, AVS_ENABLE_PLL);
	if (ret < 0) {
		pr_err("[%s]clk avs ip poll fail,ret = %d!\n", __func__, ret);
		clk_core_unprepare(friend_clk->core);
		return ret;
	}
	ret = peri_dvfs_prepare(dfclk);
	if (ret < 0) {
		pr_err("[%s]:clock prepare dvfs faild, ret = %d!\n", __func__, ret);
		clk_core_unprepare(friend_clk->core);
		avs_ret = clk_ip_avs_poll(dfclk, AVS_DISABLE_PLL);
		if (avs_ret < 0)
			pr_err("[%s]clk avs ip poll unprepare fail,ret = %d!\n",
				__func__, avs_ret);
		return ret;
	}

	ret = wait_avs_complete(dfclk);
	if (ret < 0)
		pr_err("[%s]:wait avs fail, ret = %d!\n", __func__, ret);
#endif

	return ret;
}

/* func: dvfs clk enable func */
static int peri_dvfs_clk_enable(struct clk_hw *hw)
{
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	struct clk *friend_clk = NULL;
	int ret;

	friend_clk = __clk_lookup(dfclk->link);
	if (IS_ERR_OR_NULL(friend_clk)) {
		pr_err("[%s] %s get failed!\n", __func__, dfclk->link);
		return -ENODEV;
	}
	ret = __clk_enable(friend_clk);
	if (ret) {
		pr_err("[%s], friend clock enable faild!", __func__);
		return ret;
	}

	return ret;
}

/* func: dvfs clk disable func */
static void peri_dvfs_clk_disable(struct clk_hw *hw)
{
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	struct clk *friend_clk = NULL;

	friend_clk = __clk_lookup(dfclk->link);
	if (IS_ERR_OR_NULL(friend_clk))
		pr_err("[%s] %s get failed!\n", __func__, dfclk->link);
	__clk_disable(friend_clk);
}

static void peri_dvfs_clk_unprepare(struct clk_hw *hw)
{
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	struct clk *friend_clk = NULL;
#ifdef CONFIG_HISI_PERIDVFS
	int ret;
#endif

	friend_clk = __clk_lookup(dfclk->link);
	if (IS_ERR_OR_NULL(friend_clk)) {
		pr_err("[%s] %s get failed!\n", __func__, dfclk->link);
		return;
	}

	clk_core_unprepare(friend_clk->core);

#ifdef CONFIG_HISI_PERIDVFS
	/* Avs poll ip clock close */
	ret = clk_ip_avs_poll(dfclk, AVS_DISABLE_PLL);
	if (ret < 0)
		pr_err("[%s]clk avs ip poll unprepare fail,ret = %d!\n", __func__, ret);
	peri_dvfs_unprepare(dfclk);
	/* Close clock no need to wait AVS OK */
#endif
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int peri_dvfs_dump_clk(struct clk_hw *hw, char *buf, int buf_length, struct seq_file *s)
{
	struct peri_dvfs_clk *dfclk = container_of(hw, struct peri_dvfs_clk, hw);
	unsigned int index;

	if ((buf == NULL) && (s != NULL)) {
		seq_printf(s, "    %-15s    %-15s", "NONE", "dvfs-clk");
		for (index = 0; index < DVFS_MAX_FREQ_NUM; index++)
			seq_printf(s, "    %11lu", dfclk->sensitive_freq[index] * FREQ_CONVERSION);
	}

	return 0;
}
#endif

static const struct clk_ops peri_dvfs_clk_ops = {
	.recalc_rate = peri_dvfs_clk_recalc_rate,
	.set_rate = peri_dvfs_clk_set_rate,
	.determine_rate = peri_dvfs_clk_determine_rate,
	.round_rate = peri_dvfs_clk_round_rate,
	.prepare = peri_dvfs_clk_prepare,
	.unprepare = peri_dvfs_clk_unprepare,
	.enable = peri_dvfs_clk_enable,
	.disable = peri_dvfs_clk_disable,
#ifdef CONFIG_HISI_CLK_DEBUG
	.dump_reg = peri_dvfs_dump_clk,
#endif
};

static void __peri_dvfs_setup(const struct dvfs_clock *dvfs,
	struct peri_dvfs_clk *devfreq_clk)
{
	int i;

	devfreq_clk->sensitive_level = dvfs->sensitive_level;

	for (i = 0; i < DVFS_MAX_FREQ_NUM; i++) {
		devfreq_clk->sensitive_freq[i] = dvfs->sensitive_freq[i];
		devfreq_clk->sensitive_volt[i] = dvfs->sensitive_volt[i];
	}
	devfreq_clk->sensitive_volt[i] = dvfs->sensitive_volt[i];
}

static void __peri_devfreq_clk_set(const struct dvfs_clock *dvfs,
	struct peri_dvfs_clk *devfreq_clk)
{
	devfreq_clk->id = dvfs->devfreq_id;
	devfreq_clk->avs_poll_id = dvfs->avs_poll_id;
	devfreq_clk->block_mode = dvfs->block_mode;
	devfreq_clk->link = dvfs->link;
	devfreq_clk->divider = 0;
	devfreq_clk->enable_pll_name = NULL;
	devfreq_clk->rate = 0;
}

static void __peri_dvfs_low_temperature_property_read(const struct dvfs_clock *dvfs,
	struct peri_dvfs_clk *devfreq_clk)
{
	if (dvfs->low_temperature_property) {
		devfreq_clk->low_temperature_property = 1;
		devfreq_clk->low_temperature_freq = dvfs->low_temperature_freq;
	} else {
		devfreq_clk->low_temperature_property = 0;
		devfreq_clk->low_temperature_freq = 0;
	}
}

static struct clk *__clk_register_dvfs_clk(const struct dvfs_clock *dvfs,
	struct clock_data *data)
{
	struct clk *clk = NULL;
	struct clk_init_data init;
	struct peri_dvfs_clk *devfreq_clk = NULL;

	devfreq_clk = kzalloc(sizeof(*devfreq_clk), GFP_KERNEL);
	if (devfreq_clk == NULL)
		return clk;

	init.name = dvfs->name;
	init.ops = &peri_dvfs_clk_ops;
	init.parent_names = NULL;
	init.num_parents = 0;
	init.flags = CLK_IS_ROOT | CLK_GET_RATE_NOCACHE;

	__peri_dvfs_low_temperature_property_read(dvfs, devfreq_clk);

	devfreq_clk->hw.init = &init;
	devfreq_clk->reg_base = data->base;

	__peri_devfreq_clk_set(dvfs, devfreq_clk);

	__peri_dvfs_setup(dvfs, devfreq_clk);

	clk = clk_register(NULL, &devfreq_clk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register devfreq_clk %s!\n",
			__func__, dvfs->name);
		goto err_init;
	}

#ifdef CONFIG_HISI_PERIDVFS
	INIT_WORK(&devfreq_clk->sw_up.updata, updata_freq_volt_up_fn);
#endif

	return clk;
err_init:
	kfree(devfreq_clk);
	return clk;
}

void plat_clk_register_dvfs_clk(const struct dvfs_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_dvfs_clk(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			continue;
		}

#ifdef CONFIG_HISI_CLK_DEBUG
		debug_clk_add(clk, CLOCK_DVFS);
#endif

		clk_log_dbg("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);

		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
}

