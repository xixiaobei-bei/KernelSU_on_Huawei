/*
 * hw_vote.c
 *
 * Device driver for regulators in IC
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd
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

#include <linux/io.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <hw_vote.h>
#include "hw_vote_private.h"
#include <securec.h>

#define hv_debug(fmt, arg...)	pr_err("[hw vote]"fmt, ##arg)
#define COMPATIBLE_NODE		"hisi,freq-hw-vote"

#define UNIT			1000 /* KHz <--> MHz */
#define DTSI_REG_U32_NUM	(sizeof(struct dtsi_reg_info) / sizeof(unsigned int))

#define BASE_NUM_MAX	3

/* this must match dtsi & struct hv_reg_cfg */
struct dtsi_reg_info {
	unsigned int offset;
	unsigned int rd_mask;
	unsigned int wr_mask;
};

/* The list of all hardware vote channel info */
struct hv_channel *g_hv_channel_array;
static void __iomem *g_hv_base[BASE_NUM_MAX];
unsigned int g_hv_channel_size;
static DEFINE_MUTEX(hv_array_mutex);

void hw_vote_mutex_lock(void)
{
	mutex_lock(&hv_array_mutex);
}

void hw_vote_mutex_unlock(void)
{
	mutex_unlock(&hv_array_mutex);
}

static int hv_reg_write(const struct hv_reg_cfg *cfg, unsigned int val)
{
	int shift, width;
	unsigned int mask;

	if (cfg == NULL)
		return -EINVAL;

	if (cfg->reg == NULL)
		return -EINVAL;

	shift = ffs(cfg->rd_mask);
	if (shift <= 0)
		return -EPERM;

	shift -= 1;
	width = fls(cfg->rd_mask) - shift;
	mask  = (1 << (unsigned int)width) - 1;

	if (val > mask)
		val = mask;

	writel(cfg->wr_mask | ((val & mask) << (unsigned int)shift), cfg->reg);

	return 0;
}

int hv_reg_read(const struct hv_reg_cfg *cfg, unsigned int *val)
{
	int shift;

	if (cfg == NULL)
		return -EINVAL;

	if (cfg->reg == NULL)
		return -EINVAL;

	shift = ffs(cfg->rd_mask);
	if (shift <= 0)
		return -EINVAL;

	shift -= 1;
	*val = ((unsigned int)readl(cfg->reg) & cfg->rd_mask) >> (unsigned int)shift;

	return 0;
}

/*
 * Name         : check_channel_para
 *
 * Synopsis     : int check_channel_para(struct hvdev *hvdev)
 *
 * Arguments    : struct hvdev  *hvdev
 *
 * Description  : you must check hvdev before use this function
 *
 * Returns      : error flag
 */
static int check_channel_para(struct hvdev *hvdev)
{
	if (IS_ERR_OR_NULL(hvdev->parent))
		return -EINVAL;

	if (hvdev->parent->ratio == 0)
		return -EINVAL;

	return 0;
}

int hv_set_freq(struct hvdev *hvdev, unsigned int freq_khz)
{
	int ret;

	if (IS_ERR_OR_NULL(hvdev)) {
		pr_err("%s: not register hw vote!\n", __func__);
		return -ENODEV;
	}

	ret = check_channel_para(hvdev);
	if (ret != 0) {
		pr_err("%s: parent para error\n", __func__);
		goto out;
	}

	ret = hv_reg_write(&hvdev->vote, freq_khz / UNIT / hvdev->parent->ratio);
	if (ret != 0) {
		pr_err("%s: write fail %d\n", __func__, ret);
		goto out;
	}
	hvdev->last_set = freq_khz;

out:
	return ret;
}

static int check_get_para(struct hvdev *hvdev, unsigned int *freq_khz)
{
	if (IS_ERR_OR_NULL(hvdev) || IS_ERR_OR_NULL(freq_khz))
		return -ENODEV;

	return 0;
}

int hv_get_freq(struct hvdev *hvdev, unsigned int *freq_khz)
{
	int ret;
	unsigned int val = 0;

	ret = check_get_para(hvdev, freq_khz);
	if (ret != 0) {
		pr_err("%s: user para error!\n", __func__);
		goto out;
	}

	ret = check_channel_para(hvdev);
	if (ret != 0) {
		pr_err("%s: parent para error\n", __func__);
		goto out;
	}

	ret = hv_reg_read(&hvdev->vote, &val);
	if (ret != 0) {
		pr_err("%s: read fail %d!\n", __func__, ret);
		goto out;
	}
	*freq_khz = val * UNIT * hvdev->parent->ratio;

out:
	return ret;
}

int hv_get_result(struct hvdev *hvdev, unsigned int *freq_khz)
{
	int ret;
	unsigned int val = 0;
	struct hv_channel *parent = NULL;

	ret = check_get_para(hvdev, freq_khz);
	if (ret != 0) {
		pr_err("%s: user para error!\n", __func__);
		goto out;
	}

	ret = check_channel_para(hvdev);
	if (ret != 0) {
		pr_err("%s: parent para error\n", __func__);
		goto out;
	}

	parent = hvdev->parent;
	ret = hv_reg_read(&parent->result, &val);
	if (ret != 0) {
		pr_err("%s: read fail %d!\n", __func__, ret);
		goto out;
	}
	*freq_khz = val * UNIT * parent->ratio;

out:
	return ret;
}

int hv_get_last(struct hvdev *hvdev, unsigned int *freq_khz)
{
	int ret;

	ret = check_get_para(hvdev, freq_khz);
	if (ret != 0) {
		pr_err("%s: user para error!\n", __func__);
		return ret;
	}

	*freq_khz = hvdev->last_set;

	return 0;
}

static struct hvdev *find_hvdev_by_name(const struct hv_channel *channel,
					const char *vsrc)
{
	struct hvdev *hvdev = channel->hvdev_head;
	unsigned int id;

	for (id = 0; id < channel->hvdev_num; id++) {
		if (strncmp(hvdev[id].name, vsrc, VSRC_NAME_MAX) == 0 &&
		    hvdev[id].dev == NULL)
			return &hvdev[id];
	}

	return NULL;
}

struct hvdev *hvdev_register(struct device *dev,
				  const char *ch_name,
				  const char *vsrc)
{
	struct hvdev *hvdev = NULL;
	unsigned int val = 0;
	unsigned int id;
	int ret;

	if (IS_ERR_OR_NULL(dev) || ch_name == NULL || vsrc == NULL) {
		pr_err("%s: register device fail!\n", __func__);
		goto out;
	}

	if (g_hv_channel_size == 0 || g_hv_channel_array == NULL) {
		pr_err("%s: hw vote not init\n", __func__);
		goto out;
	}

	hw_vote_mutex_lock();
	for (id = 0; id < g_hv_channel_size; id++) {
		if (strncmp(g_hv_channel_array[id].name, ch_name, CHANNEL_NAME_MAX) == 0) {
			hvdev = find_hvdev_by_name(&g_hv_channel_array[id], vsrc);
			break;
		}
	}

	if (hvdev != NULL) {
		ret = hv_get_result(hvdev, &val);
		if (ret != 0)
			pr_err("%s: init get result fail!\n", __func__);

		hvdev->dev = dev;
		hvdev->last_set = val;
	} else {
		pr_err("%s: not find unused vote\n", __func__);
	}

	hw_vote_mutex_unlock();

out:
	return hvdev;
}

int hvdev_remove(struct hvdev *hvdev)
{
	if (IS_ERR_OR_NULL(hvdev)) {
		pr_err("%s: remove device fail!\n", __func__);
		return -ENODEV;
	}

	hw_vote_mutex_lock();
	hvdev->last_set = 0;
	hvdev->dev = NULL;
	hw_vote_mutex_unlock();

	return 0;
}

static int get_member_data(struct device *dev,
			   struct device_node *np,
			   struct hv_channel *channel)
{
	struct hvdev *hvdev_array = NULL;
	struct device_node *child = NULL;
	struct dtsi_reg_info reg_info;
	int count, ret;
	int id = 0;

	count = of_get_child_count(np);
	if (count == 0 || IS_ERR_OR_NULL(channel)) {
		dev_err(dev, "channel is null,or bad child nodes %d\n", count);
		ret = -EINVAL;
		goto out;
	}

	hvdev_array = devm_kzalloc(dev, sizeof(struct hvdev) * count, GFP_KERNEL);
	if (hvdev_array == NULL) {
		dev_err(dev, "alloc hvdev_array fail\n");
		ret = -ENOMEM;
		goto out;
	}

	while ((child = of_get_next_child(np, child)) != NULL) {
		ret = of_property_read_u32_array(child, "vote_reg",
						 (unsigned int *)(&reg_info),
						 DTSI_REG_U32_NUM);
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s vote_reg fail%d!\n",
				__func__, child->name, ret);
			ret = -EINVAL;
			goto free_out;
		}

		if (id < count) {
			hvdev_array[id].name = child->name;
			hvdev_array[id].parent = channel;
			hvdev_array[id].vote.reg = channel->base + reg_info.offset;
			hvdev_array[id].vote.rd_mask = reg_info.rd_mask;
			hvdev_array[id].vote.wr_mask = reg_info.wr_mask;
			hvdev_array[id].dev = NULL;
			hvdev_array[id].last_set = 0;
		}
		id++;
	}
	channel->hvdev_num = id;
	channel->hvdev_head = hvdev_array;

	return 0;

free_out:
	devm_kfree(dev, hvdev_array);
	channel->hvdev_head = NULL;
out:
	return ret;
}

static void __iomem *get_child_base(struct device *dev,
				    struct device_node *np)
{
	int ret;
	unsigned int index = 0;

	ret = of_property_read_u32(np, "base_index", &index);
	if (ret != 0)
		return g_hv_base[0];

	if (index >= BASE_NUM_MAX) {
		dev_err(dev, "wrong base index%u\n", index);
		return NULL;
	}

	return g_hv_base[index];
}

static int hv_init_iomap(struct device *dev)
{
	struct device_node *root = dev->of_node;
	int ret = 0;
	int i;

	ret = memset_s(g_hv_base, sizeof(g_hv_base), 0, sizeof(g_hv_base));
	if (ret != EOK) {
		dev_err(dev, "base buf clear fail %d\n", ret);
		return -ENODEV;
	}

	for (i = 0; i < BASE_NUM_MAX; i++) {
		g_hv_base[i] = of_iomap(root, i);
		if (g_hv_base[i] == NULL)
			break;
	}

	return 0;
}

static void hv_free_iomap(void)
{
	int i;

	for (i = 0; i < BASE_NUM_MAX; i++) {
		if (g_hv_base[i] != NULL) {
			iounmap(g_hv_base[i]);
			g_hv_base[i] = NULL;
		}
	}
}

static int hv_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *root = dev->of_node;
	struct device_node *child = NULL;
	struct dtsi_reg_info reg_info;
	void __iomem *child_base = NULL;
	unsigned int ratio = 1;
	int count;
	int id = 0;
	int ret;

	if (root == NULL) {
		dev_err(dev, "[%s]dts[%s] node not found\n",
			__func__, COMPATIBLE_NODE);
		return -ENODEV;
	}

	count = of_get_child_count(root);
	if (count <= 0) {
		dev_err(dev, "bad channel nodes %d\n", count);
		return -EINVAL;
	}

	ret = hv_init_iomap(dev);
	if (ret != 0)
		return ret;

	hw_vote_mutex_lock();

	g_hv_channel_array = devm_kzalloc(dev,
					  sizeof(struct hv_channel) * count,
					  GFP_KERNEL);
	if (g_hv_channel_array == NULL) {
		dev_err(dev, "alloc g_hv_channel_array fail\n");
		ret = -ENOMEM;
		goto out_unlock;
	}
	g_hv_channel_size = count;

	for_each_child_of_node(root, child) {
		child_base = get_child_base(dev, child);
		if (child_base == NULL)
			continue;

		ret = of_property_read_u32_array(child, "result_reg",
						 (unsigned int *)(&reg_info),
						 DTSI_REG_U32_NUM);
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s result_reg fail%d!\n",
				__func__, child->name, ret);
			ret = -EINVAL;
			goto out_unlock;
		}

		ret = of_property_read_u32(child, "ratio", &ratio);
		if (ret != 0) {
			dev_err(dev, "[%s]parse %s ratio fail%d!\n",
				__func__, child->name, ret);
			ret = -EINVAL;
			goto out_unlock;
		}

		if (id < count) {
			g_hv_channel_array[id].name = child->name;
			g_hv_channel_array[id].ratio = ratio;
			g_hv_channel_array[id].base = child_base;
			g_hv_channel_array[id].result.reg = child_base + reg_info.offset;
			g_hv_channel_array[id].result.rd_mask = reg_info.rd_mask;
			g_hv_channel_array[id].result.wr_mask = reg_info.wr_mask;
			g_hv_channel_array[id].hvdev_num = 0;
			g_hv_channel_array[id].hvdev_head = NULL;

			ret = get_member_data(dev, child, &g_hv_channel_array[id]);
			if (ret != 0) {
				dev_err(dev, "[%s][%s] get vote src fail\n",
					__func__, child->name);
				ret = -EINVAL;
				goto out_unlock;
			}
		}
		id++;
	}

	hv_debug("%s: g_hv_channel_size %d\n", __func__, id);

	if (id != g_hv_channel_size) {
		dev_err(dev, "[%s]channel info error\n", __func__);
		ret = -EINVAL;
		goto out_unlock;
	}

	hw_vote_mutex_unlock();

	return 0;
out_unlock:
	g_hv_channel_array = NULL;
	g_hv_channel_size = 0;
	hw_vote_mutex_unlock();
	hv_free_iomap();

	return ret;
}

static int hv_remove(struct platform_device *pdev)
{
	unsigned int i;

	hw_vote_mutex_lock();

	hv_free_iomap();

	for (i = 0; i < g_hv_channel_size; i++)
		g_hv_channel_array[i].base = NULL;

	g_hv_channel_array = NULL;
	g_hv_channel_size  = 0;
	hw_vote_mutex_unlock();
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id hv_of_match[] = {
	{.compatible = "hisi,freq-hw-vote",},
	{},
};

MODULE_DEVICE_TABLE(of, hv_of_match);
#endif

static struct platform_driver hv_driver = {
	.probe  = hv_probe,
	.remove = hv_remove,
	.driver = {
		.name = "HISI-Hardware-Vote",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hv_of_match),
	},
};

static int __init hv_init(void)
{
	return platform_driver_register(&hv_driver);
}
core_initcall(hv_init);

static void __exit hv_exit(void)
{
	platform_driver_unregister(&hv_driver);
}
module_exit(hv_exit);

MODULE_DESCRIPTION("vote frequency by hardware state machine");
MODULE_LICENSE("GPL v2");
