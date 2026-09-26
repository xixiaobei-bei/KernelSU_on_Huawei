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
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/hwspinlock.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include "../clk-mailbox.h"
#include "peri-volt-internal.h"
#ifdef CONFIG_HISI_PERI_VOLT_HW_VOTE
#include <linux/hisi/hw_vote.h>
#endif

enum {
	HS_PMCTRL,
};

enum {
	PERI_VOLT_0 = 0,
	PERI_VOLT_1,
	PERI_VOLT_2,
	PERI_VOLT_3,
};

#define SC_SCBAKDATA24_ADDR			0x46C
#define AVS_BITMASK_FLAG			28
#define PERIVOLT_POLL_HWLOCK			19
#define HWLOCK_TIMEOUT				1000
#define PMCTRL_PERI_CTRL4_VDD_MASK		0x30000000
#define PMCTRL_PERI_CTRL4_TEMPERATURE_MASK	0xC000000
#define PMCTRL_PERI_CTRL4_VDD_SHIFT		28
#define PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT	26
#define PMCTRL_PERI_CTRL4_ON_OFF_MASK		0xC0000000
#define USB_POLL_ID				28
#define LPM3_CMD_CLOCK_EN			0x000D0002

#define FAST_AVS_ID				13
#define NORMAL_TEMPRATURE			0
static struct hwspinlock *peri_poll_hwlock;
static int g_count;

struct perivolt {
	void __iomem *pmctrl;
	spinlock_t lock;
};
static struct perivolt perivolt = {
	.lock = __SPIN_LOCK_UNLOCKED(perivolt.lock),
};

static void __iomem *perivolt_get_base(struct device_node *np);

#ifdef CONFIG_HISI_PERI_VOLT_HW_VOTE
#define PERI_DVFS_HW_VOTE_CH_NAME	"peri-volt"
#define PERI_DVFS_HW_VOTE_CH_SRC	"vote-src-1"
#define PERI_DVFS_HW_VOTE_VOTE_FLAG	"peri_dvfs_hw_vote_flag"
#define PERI_DVFS_HW_VOTE_RATIO		1000
#define PERI_DVFS_DVFS_ID_SHIFT		8
#define PERI_DVFS_AVS_ID_SHIFT		4
#define PERI_DVFS_HW_VOTE_MASK		0x7FFF
static struct hvdev *peri_dvfs_hvdev;
static unsigned int get_perivolt_hv_value(unsigned int dvfs_id, unsigned int avs_id,
	unsigned int volt, unsigned int last_hv_val)
{
	unsigned int hv_val;

	/*
	 * field_name   bit_range   val_range
	 * volt         [ 3:0]       0~15
	 * avs_id       [ 7:4]       0~15
	 * dvfs_id      [14:8]      0~127
	 */
	hv_val = ((dvfs_id << PERI_DVFS_DVFS_ID_SHIFT) |
			(avs_id << PERI_DVFS_AVS_ID_SHIFT) | volt);
	/* make sure that hv_value NOT exceed to 0x7FFF */
	hv_val = (hv_val & PERI_DVFS_HW_VOTE_MASK);
	/*
	 * Make sure that every hw vote can trigger interrupt to lpm
	 * in order to shorten "AVS STATUS" waiting time,
	 * and then improve pm and dvfs performance(such as NPU).
	 */
	if (hv_val * PERI_DVFS_HW_VOTE_RATIO == last_hv_val)
		/* make the adjacent same hw votes differ on BIT-3 */
		hv_val = hv_val ^ BIT(PERI_DVFS_AVS_ID_SHIFT - 1);

	/* hv_set_freq interface requirement: should multiply ratio(=1000) */
	hv_val = (hv_val * PERI_DVFS_HW_VOTE_RATIO);

	return hv_val;
}

static int peri_dvfs_hw_vote(struct peri_volt_poll *pvp,
	unsigned int target_volt)
{
	unsigned int last_hv_val = 0;
	unsigned int hv_val;
	unsigned int new_hv_val = 0;
	unsigned int hv_result = 0;
	int ret;

	if (IS_ERR_OR_NULL(peri_dvfs_hvdev)) {
		pr_err("[%s] peri dvfs hw vote device is nul!\n", __func__);
		return -EINVAL;
	}

	/* get old hw vote */
	ret = hv_get_freq(peri_dvfs_hvdev, &last_hv_val);
	if (ret)
		pr_err("[%s] get last peri_hw_vote volt fail!\n", __func__);

	/* peri_volt hw vote */
	hv_val = get_perivolt_hv_value(pvp->dev_id, pvp->perivolt_avs_ip,
		target_volt, last_hv_val);
	ret = hv_set_freq(peri_dvfs_hvdev, hv_val);
	if (ret) {
		pr_err("[%s] set peri_hw_vote volt fail!\n", __func__);
		return ret;
	}

	/* get hw vote readback */
	ret = hv_get_freq(peri_dvfs_hvdev, &new_hv_val);
	if (ret)
		pr_err("[%s] get last peri_hw_vote volt fail!\n", __func__);

	/* get hw vote finally result */
	ret = hv_get_result(peri_dvfs_hvdev, &hv_result);
	if (ret)
		pr_err("[%s] get last peri_hw_vote volt fail!\n", __func__);

	return ret;
}
#endif

#ifdef CONFIG_HISI_PERI_FAST_AVS
static int peri_fast_avs(struct peri_volt_poll *pvp, unsigned int target_volt)
{
	int ret = 0;
	u32 val;
	u32 fast_avs_cmd[LPM3_CMD_LEN] = { LPM3_CMD_CLOCK_EN, FAST_AVS_ID };

	if (target_volt > PERI_VOLT_0) {
		val = ((unsigned int)readl(pvp->addr_0) & PMCTRL_PERI_CTRL4_VDD_MASK);
		val = val >> PMCTRL_PERI_CTRL4_VDD_SHIFT;
		if (val < target_volt) {
			ret = clkmbox_send_msg(fast_avs_cmd, LPM3_CMD_LEN);
			if (ret < 0)
				pr_err("[%s]fail to send fast avs msg to LPM3!\n", __func__);
		}
	}

	return ret;
}
#endif

#ifdef CONFIG_HISI_PERI_AVS_IPC_NOTIFY
static int peri_avs_ipc_notify_m3(struct peri_volt_poll *pvp, unsigned int target_volt)
{
	int ret = 0;
	u32 val;
	u32 avs_ipc_cmd[LPM3_CMD_LEN] = { LPM3_CMD_CLOCK_EN, FAST_AVS_ID };

	/* send ipc for avs ip do not care scene */
	if (pvp->perivolt_avs_ip) {
		ret = clkmbox_send_msg(avs_ipc_cmd, LPM3_CMD_LEN);
		if (ret < 0)
			pr_err("[%s]fail to notify LPM3 for avs ip !\n", __func__);
		return ret;
	}

	/*
	 * volt up: send ipc immediately
	 * volt down:volt will be change 5ms later by ddr dvfs
	 */
	if (target_volt > PERI_VOLT_0) {
		val = ((unsigned int)readl(pvp->addr_0) & PMCTRL_PERI_CTRL4_VDD_MASK);
		val = val >> PMCTRL_PERI_CTRL4_VDD_SHIFT;
		if (val < target_volt) {
			ret = clkmbox_send_msg(avs_ipc_cmd, LPM3_CMD_LEN);
			if (ret < 0)
				pr_err("[%s]fail to notify LPM3 for non-avs ip!\n", __func__);
		}
	}

	return ret;
}

#ifdef CONFIG_HISI_PERI_VOLT_HW_VOTE
static int peri_avs_hv_notify_m3(struct peri_volt_poll *pvp, unsigned int target_volt)
{
	int ret;

	/* DVFS(AVS) hw vote */
	ret = peri_dvfs_hw_vote(pvp, target_volt);
	if (ret) {
		pr_err("[%s] peri dvfs hw volt fail!\n", __func__);
		return ret;
	}

	return ret;
}

#endif /* CONFIG_PERI_VOLT_HW_VOTE */

static int peri_avs_notify_m3(struct peri_volt_poll *pvp, unsigned int target_volt)
{
	int ret;

#ifdef CONFIG_HISI_PERI_VOLT_HW_VOTE
	if (!IS_ERR_OR_NULL(peri_dvfs_hvdev)) {
		ret = peri_avs_hv_notify_m3(pvp, target_volt);
		return ret;
	}
#endif
	ret = peri_avs_ipc_notify_m3(pvp, target_volt);

	return ret;
}
#endif /* CONFIG_PERI_AVS_IPC_NOTIFY */

static int peri_set_volt(struct peri_volt_poll *pvp, unsigned int volt)
{
	unsigned int val;
	int ret = 0;

#ifdef CONFIG_HISI_CLK_LOW_TEMPERATURE_JUDGE_BY_VOLT
	int temprature = 0;
#endif

	if (hwspin_lock_timeout((struct hwspinlock *)pvp->priv, HWLOCK_TIMEOUT)) {
		pr_err("pvp hwspinlock timout!\n");
		return -ENOENT;
	}
#ifdef CONFIG_HISI_CLK_LOW_TEMPERATURE_JUDGE_BY_VOLT
	/*
	 * read pmctrl 0x350 bit 27:26,value of bit 27:26 is
	 * not 0: low temprature
	 * 0: normal temprature
	 */
	if (volt == PERI_VOLT_3) {
		temprature = readl(pvp->addr_0);
		temprature &= PMCTRL_PERI_CTRL4_TEMPERATURE_MASK;
		temprature = temprature >> PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT;
		if (temprature != NORMAL_TEMPRATURE) {
			if (pvp->dev_id != USB_POLL_ID) {
				hwspin_unlock((struct hwspinlock *)pvp->priv);
				pr_err("low temprature and set volt fail!\n");
				return -EPERM;
			}
			/* low temprature usb set 0.75V */
			volt = PERI_VOLT_2;
		}
	}
#endif

	if (volt <= PERI_VOLT_3) {
		val = readl(pvp->addr);
		val &= (~(pvp->bitsmask));
		val |= (volt << pvp->bitsshift);
		writel(val, pvp->addr);
	} else {
		hwspin_unlock((struct hwspinlock *)pvp->priv);
		return -EINVAL;
	}

#ifdef CONFIG_HISI_PERI_FAST_AVS
	/* fast avs */
	if (!((unsigned int)readl(pvp->addr_0) & PMCTRL_PERI_CTRL4_ON_OFF_MASK))
		ret = peri_fast_avs(pvp, volt);
#endif

#ifdef CONFIG_HISI_PERI_AVS_IPC_NOTIFY
	if (pvp->perivolt_avs_ip) {
		/* clear avs status */
		val = readl(pvp->sysreg_base + SC_SCBAKDATA24_ADDR);
		val = val & (~BIT(AVS_BITMASK_FLAG));
		writel(val, pvp->sysreg_base + SC_SCBAKDATA24_ADDR);
	}
	/* avs send ipc to notify lpm3 */
	ret = peri_avs_notify_m3(pvp, volt);
#endif
	hwspin_unlock((struct hwspinlock *)pvp->priv);

	return ret;
}

static int peri_get_temprature(struct peri_volt_poll *pvp)
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

static unsigned int peri_get_volt(struct peri_volt_poll *pvp)
{
	unsigned int ret;

	if (hwspin_lock_timeout((struct hwspinlock *)pvp->priv, HWLOCK_TIMEOUT)) {
		pr_err("pvp hwspinlock timout!\n");
		return (unsigned int)-ENOENT;
	}
	ret = readl(pvp->addr_0);
	ret &= PMCTRL_PERI_CTRL4_VDD_MASK;
	ret = ret >> PMCTRL_PERI_CTRL4_VDD_SHIFT;
	WARN_ON(ret > PERI_VOLT_3);

	hwspin_unlock((struct hwspinlock *)pvp->priv);

	return ret;
}

static int peri_set_avs(struct peri_volt_poll *pvp, unsigned int avs)
{
	unsigned int val;

	if (IS_ERR_OR_NULL(pvp))
		return -EINVAL;
	if (pvp->perivolt_avs_ip <= 0)
		return 0;
	if (hwspin_lock_timeout((struct hwspinlock *)pvp->priv, HWLOCK_TIMEOUT)) {
		pr_err("pvp hwspinlock timout!\n");
		return -ENOENT;
	}
	val = readl(pvp->sysreg_base + SC_SCBAKDATA24_ADDR);
	if (avs)
		val = val | BIT(pvp->perivolt_avs_ip);
	else
		val = val & (~BIT(pvp->perivolt_avs_ip));
	writel(val, pvp->sysreg_base + SC_SCBAKDATA24_ADDR);
	hwspin_unlock((struct hwspinlock *)pvp->priv);

	return 0;
}

static int peri_wait_avs_update(struct peri_volt_poll *pvp)
{
	/* peri avs loop max */
	int loop = 400;
	unsigned int val;

	if (IS_ERR_OR_NULL(pvp))
		return -EINVAL;

	if (pvp->perivolt_avs_ip <= 0)
		return 0;

	do {
		val = readl(pvp->sysreg_base + SC_SCBAKDATA24_ADDR);
		val = val & BIT(AVS_BITMASK_FLAG);
		if (!val) {
			loop--;
			/* AVS complete timeout is about 80us * 400 ~ 120us * 400 */
			usleep_range(80, 120);
		} else {
			return 0;
		}
	} while (!val && loop > 0);

	pr_err("[%s] %s peri avs status update failed!\n", __func__, pvp->name);

	return -EINVAL;
}

static void peri_dvfs_init(struct peri_volt_poll *pvp, unsigned int endis)
{
	if (g_count)
		return;

	g_count++;
}

static struct peri_volt_ops peri_volt_ops = {
	.set_volt = peri_set_volt,
	.get_volt = peri_get_volt,
	.recalc_volt = peri_get_volt,
	.set_avs = peri_set_avs,
	.wait_avs_update = peri_wait_avs_update,
	.get_temperature = peri_get_temprature,
	.init = peri_dvfs_init,
};

static int __peri_volt_pvolt_setup(struct device_node *np, struct peri_volt_poll *pvolt,
	void __iomem *reg_base)
{
	/* 2:offset and efficient bit */
	u32 pdata[2] = {0};
	u32 ctrl4_reg = 0;
	u32 perivolt_id = 0;
	u32 perivolt_avs_ip = 0;

	if (of_property_read_u32_array(np, "perivolt-poll-reg", &pdata[0], 2)) {
		pr_err("[%s] %s node doesn't have perivolt-poll-reg property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32(np, "perivolt-poll-lpmcu", &ctrl4_reg)) {
		pr_err("[%s] %s node doesn't have perivolt-poll-lpmcu property!\n",
			__func__, np->name);
		return -ENODEV;
	}
	if (of_property_read_u32_array(np, "perivolt-poll-id", &perivolt_id, 1))
		pr_err("[%s] %s node doesn't have perivolt-poll-id property\n",
			__func__, np->name);
	if (of_property_read_u32(np, "perivolt-avs-ip", &perivolt_avs_ip))
		perivolt_avs_ip = 0;

	pvolt->dev_id = perivolt_id;
	pvolt->perivolt_avs_ip = perivolt_avs_ip;
	pvolt->addr = reg_base + pdata[0];
	pvolt->addr_0 = reg_base + ctrl4_reg;
	pvolt->bitsmask = pdata[1];
	pvolt->bitsshift = ffs(pdata[1]) - 1;
	pvolt->stat = 0;
	pvolt->volt = PERI_VOLT_0;
	pvolt->flags = PERI_GET_VOLT_NOCACHE;

	return 0;
}

static void peri_volt_poll_init(struct device_node *np)
{
	struct peri_volt_poll *pvolt = NULL;
	struct device_node *dev_np = NULL;
	const char *poll_name = NULL;
	void __iomem *reg_base = NULL;
	int ret;

	reg_base = perivolt_get_base(np);
	if (reg_base == NULL) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}
	if (of_property_read_string(np, "perivolt-output-names", &poll_name)) {
		pr_err("[%s] %s node doesn't have output-names property!\n",
			__func__, np->name);
		return;
	}
	pvolt = kzalloc(sizeof(*pvolt), GFP_KERNEL);
	if (pvolt == NULL)
		return;
	pvolt->name = kstrdup(poll_name, GFP_KERNEL);
	if (pvolt->name == NULL)
		goto err_pvp;

	dev_np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	pvolt->sysreg_base = of_iomap(dev_np, 0);
	if (pvolt->sysreg_base == NULL) {
		pr_err("[%s] fail to iomap!\n", __func__);
		goto err_pvp_name;
	}

	pvolt->ops = &peri_volt_ops;
	pvolt->priv = peri_poll_hwlock;

	ret = __peri_volt_pvolt_setup(np, pvolt, reg_base);
	if (ret)
		goto err_iomap;

	ret = perivolt_register(NULL, pvolt);
	if (ret) {
		pr_err("[%s] fail to reigister pvp %s!\n", __func__, poll_name);
		goto err_iomap;
	}
	pr_info("[%s] peri dvfs node:%s\n", __func__, poll_name);
	goto err_pvp_name;

err_iomap:
	/*
	 * if pvolt register succ, sysreg_base value pass to
	 * another peri_volt_poll, no need to release.
	 */
	iounmap(pvolt->sysreg_base);
err_pvp_name:
	kfree(pvolt->name);
err_pvp:
	kfree(pvolt);
}

PERIVOLT_OF_DECLARE(perivolt, "hisilicon,soc-peri-volt", peri_volt_poll_init);

static const struct of_device_id perivolt_of_match[] = {
	{ .compatible = "hisilicon,soc-peri-dvfs", .data = (void *)HS_PMCTRL, },
	{},
};

static void __iomem *perivolt_get_base(struct device_node *np)
{
	struct device_node *parent = NULL;
	const struct of_device_id *match = NULL;
	void __iomem *ret = NULL;

	parent = of_get_parent(np);
	if (parent == NULL) {
		pr_err("[%s] node %s doesn't have parent node!\n", __func__, np->name);
		return NULL;
	}
	match = of_match_node(perivolt_of_match, parent);
	if (match == NULL) {
		pr_err("[%s] parent node %s doesn't match!\n", __func__, parent->name);
		return NULL;
	}
	switch ((uintptr_t) match->data) {
	case HS_PMCTRL:
		if (perivolt.pmctrl == NULL) {
			ret = of_iomap(parent, 0);
			WARN_ON(!ret);
			perivolt.pmctrl = ret;
		} else {
			ret = perivolt.pmctrl;
		}
		break;
	default:
		pr_err("[%s] cannot find the match node!\n", __func__);
		ret = NULL;
	}

	return ret;
}

static const struct of_device_id __perivolt_of_table_sentinel __used __section(__perivolt_of_table_end);

static int peri_volt_poll_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct device_node *parent = pdev->dev.of_node;
	const struct of_device_id *matches = NULL;
	of_perivolt_init_cb_t perivolt_init_cb = NULL;

#ifdef CONFIG_HISI_PERI_VOLT_HW_VOTE
	if (of_property_read_bool(parent, PERI_DVFS_HW_VOTE_VOTE_FLAG)) {
		peri_dvfs_hvdev = hvdev_register(&pdev->dev,
			PERI_DVFS_HW_VOTE_CH_NAME,
			PERI_DVFS_HW_VOTE_CH_SRC);
		if (IS_ERR_OR_NULL(peri_dvfs_hvdev)) {
			pr_err("[%s] pvp request hw vote device failed!\n", __func__);
			return -ENODEV;
		}
	}
#endif

	peri_poll_hwlock = hwspin_lock_request_specific(PERIVOLT_POLL_HWLOCK);
	if (peri_poll_hwlock == NULL) {
		pr_err("pvp request hwspin lock failed !\n");
		return -ENODEV;
	}
	g_count = 0;
	matches = __perivolt_of_table;

	for_each_child_of_node(parent, np) {
		const struct of_device_id *match = of_match_node(matches, np);

		perivolt_init_cb = match->data;
		perivolt_init_cb(np);
	}

	platform_set_drvdata(pdev, NULL);
	pr_info("[%s] sucess!\n", __func__);

	return 0;
}

static int peri_volt_poll_remove(struct platform_device *pdev)
{
#ifdef CONFIG_HISI_PERI_VOLT_HW_VOTE
	if (!IS_ERR_OR_NULL(peri_dvfs_hvdev)
	    && hvdev_remove(peri_dvfs_hvdev) != 0)
		pr_err("[%s] remove hw vote device failed!\n", __func__);

#endif

	return 0;
}

static const struct of_device_id pvp_of_match[] = {
	{.compatible = "hisilicon,soc-peri-dvfs"},
	{},
};

MODULE_DEVICE_TABLE(of, pvp_of_match);

static struct platform_driver peri_volt_poll_driver = {
	.probe = peri_volt_poll_probe,
	.remove = peri_volt_poll_remove,
	.driver = {
		.name = "perivolt",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(pvp_of_match),
		},
};

static int __init plat_peri_volt_poll_init(void)
{
	return platform_driver_register(&peri_volt_poll_driver);
}

fs_initcall(plat_peri_volt_poll_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("clock driver");
