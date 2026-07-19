/*
 * Device driver for regulators in Hisi IC
 *
 * Copyright (c) 2013 Linaro Ltd.
 * Copyright (c) 2011 Hisilicon.
 *
 * Guodong Xu <guodong.xu@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/io.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <hisi_hw_vote.h>



#define HV_DEBUG(fmt, arg...)	pr_err("[hw vote]"fmt, ##arg)

#define MODULE_NAME		"hisi_hw_vote"
#define COMPATIBLE_NODE		"hisi,freq-hw-vote"

#define CHANNEL_NAME_MAX	(30)
#define VSRC_NAME_MAX		(30)

#define MHz			(1000000)
#define UNIT			(1000)/*KHz <--> MHz*/
#define OWNER_DEV_NAME(dev)	(IS_ERR_OR_NULL(dev) ? "unregister" : dev_name(dev))
#define HVDEV_NAME(hvdev)	(IS_ERR_OR_NULL(hvdev->name) ? "unknown" : hvdev->name)


/* The list of all hardware vote channel info */
static struct hv_channel *hv_channel_array = NULL;
static unsigned int hv_channel_size = 0;
static DEFINE_MUTEX(hv_array_mutex);

/*this must match dtsi & struct hv_reg_cfg*/
struct dtsi_reg_info {
	u32 offset;
	u32 rd_mask;
	u32 wr_mask;
};
#define DTSI_REG_U32_NUM	(sizeof(struct dtsi_reg_info) / sizeof(u32))




int hv_reg_write(struct hv_reg_cfg cfg, u32 val)
{
	int shift, width;
	u32 mask;

	shift = ffs(cfg.rd_mask);
	if (shift <= 0) {
		return -EPERM;
	}
	shift -= 1;
	width = fls(cfg.rd_mask) - shift;
	mask  = (1 << (u32)width) - 1;

	if (val > mask) {
		val = mask;
	}
	writel(cfg.wr_mask | ((val & mask) << (u32)shift), cfg.reg);

	return 0;
}

int hv_reg_read(struct hv_reg_cfg cfg, u32 *val)
{
	int shift;

	shift = ffs(cfg.rd_mask);
	if (shift <= 0) {
		return -EINVAL;
	}
	shift -= 1;
	*val = ((u32)readl(cfg.reg) & cfg.rd_mask) >> (u32)shift;

	return 0;
}

/*
 * Name         : check_channel_para
 *
 * Synopsis     : int check_channel_para(struct hvdev *hvdev)
 *
 * Arguments    : struct hvdev  *hvdev :
 *
 * Description  : you must check hvdev before use this function
 *
 * Returns      : error flag
 */
int check_channel_para(struct hvdev *hvdev)
{
	if (IS_ERR_OR_NULL(hvdev->parent))
		return -EINVAL;

	if (0 == hvdev->parent->ratio)
		return -EINVAL;

	return 0;
}

int hisi_hv_set_freq(struct hvdev *hvdev, unsigned int freq_khz)
{
	int ret;

	if (IS_ERR_OR_NULL(hvdev)) {
		pr_err("%s: not register hw vote!\n", __func__);
		return -ENODEV;
	}

	ret = check_channel_para(hvdev);
	if (ret) {
		pr_err("%s: parent para error\n", __func__);
		goto out;
	}

	ret = hv_reg_write(hvdev->vote, ((freq_khz / UNIT) / hvdev->parent->ratio));
	if (ret) {
		pr_err("%s: write fail(%d)\n", __func__, ret);
		goto out;
	}
	hvdev->last_set = freq_khz;

out:
	return ret;
}

int check_get_para(struct hvdev *hvdev, unsigned int *freq_khz)
{
	if (IS_ERR_OR_NULL(hvdev) || IS_ERR_OR_NULL(freq_khz)) {
		return -ENODEV;
	}

	return 0;
}

int hisi_hv_get_freq(struct hvdev *hvdev, unsigned int *freq_khz)
{
	int ret;
	u32 val = 0;

	ret = check_get_para(hvdev, freq_khz);
	if (ret) {
		pr_err("%s: user para error!\n", __func__);
		goto out;
	}

	ret = check_channel_para(hvdev);
	if (ret) {
		pr_err("%s: parent para error\n", __func__);
		goto out;
	}

	ret = hv_reg_read(hvdev->vote, &val);
	if (ret) {
		pr_err("%s: read fail(%d)!\n", __func__, ret);
		goto out;
	}
	*freq_khz = val * UNIT * hvdev->parent->ratio;

out:
	return ret;
}

int hisi_hv_get_result(struct hvdev *hvdev, unsigned int *freq_khz)
{
	int ret;
	unsigned int val = 0;

	ret = check_get_para(hvdev, freq_khz);
	if (ret) {
		pr_err("%s: user para error!\n", __func__);
		goto out;
	}

	ret = check_channel_para(hvdev);
	if (ret) {
		pr_err("%s: parent para error\n", __func__);
		goto out;
	}

	ret = hv_reg_read(hvdev->parent->result, &val);
	if (ret) {
		pr_err("%s: read fail(%d)!\n", __func__, ret);
		goto out;
	}
	*freq_khz = val * UNIT * hvdev->parent->ratio;

out:
	return ret;
}

int hisi_hv_get_last(struct hvdev *hvdev, unsigned int *freq_khz)
{
	int ret;

	ret = check_get_para(hvdev, freq_khz);
	if (ret) {
		pr_err("%s: user para error!\n", __func__);
		return ret;
	}

	*freq_khz = hvdev->last_set;

	return 0;
}

struct hvdev *find_hvdev_by_name(struct hv_channel channel, const char *vsrc)
{
	struct hvdev *hvdev = channel.hvdev_head;
	unsigned int id;

	for (id = 0; id < channel.hvdev_num; id++) {
		if (!strncmp(hvdev[id].name, vsrc, VSRC_NAME_MAX) && !(hvdev[id].dev)) {
			return &(hvdev[id]);
		}
	}

	return NULL;
}

struct hvdev *hisi_hvdev_register(struct device *dev, const char *ch_name, const char *vsrc)
{
	struct hvdev *hvdev = NULL;
	unsigned int val = 0;
	unsigned int id;
	int ret;


	if (IS_ERR_OR_NULL(dev) || !ch_name) {
		pr_err("%s: register device fail!\n", __func__);
		goto out;
	}

	if ((0 == hv_channel_size) || (NULL == hv_channel_array)) {
		pr_err("%s: hw vote not init\n", __func__);
		goto out;
	}

	mutex_lock(&hv_array_mutex);
	for (id = 0; id < hv_channel_size; id++) {
		if (!strncmp(hv_channel_array[id].name, ch_name, CHANNEL_NAME_MAX)) {
			hvdev = find_hvdev_by_name(hv_channel_array[id], vsrc);
			break;
		}
	}

	if (hvdev) {
		ret = hisi_hv_get_result(hvdev, &val);
		if (0 != ret){
			pr_err("%s: init get result fail!\n", __func__);
		}
		hvdev->dev      = dev;
		hvdev->last_set = val;
	} else {
		pr_err("%s: not find unused vote\n", __func__);
	}

	mutex_unlock(&hv_array_mutex);

out:
	return hvdev;
}

int hisi_hvdev_remove(struct hvdev *hvdev)
{
	if (IS_ERR_OR_NULL(hvdev)) {
		pr_err("%s: remove device fail!\n", __func__);
		return -ENODEV;
	}

	mutex_lock(&hv_array_mutex);
	hvdev->last_set = 0;
	hvdev->dev      = NULL;
	mutex_unlock(&hv_array_mutex);

	return 0;
}

/*lint -save -e429 */
static int get_member_data(struct device *dev, struct device_node *np, struct hv_channel *channel)
{
	struct hvdev *hvdev_array;
	struct device_node *child;
	struct dtsi_reg_info reg_info;
	int count, id = 0;
	int ret = 0;

	count = of_get_child_count(np);
	of_node_put(np);
	if ((0 == count) || IS_ERR_OR_NULL(channel)) {
		dev_err(dev, "channel is null,or bad child nodes(%d)\n", count);
		ret = -EINVAL;
		goto out;
	}

	hvdev_array = devm_kzalloc(dev, sizeof(struct hvdev) * count, GFP_KERNEL);
	if (!hvdev_array) {
		dev_err(dev, "alloc hvdev_array fail\n");
		ret = -ENOMEM;
		goto out;
	}

	child = NULL;
	while (NULL != (child = of_get_next_child(np, child))) {
		ret = of_property_read_u32_array(child, "vote_reg", (u32*)(&reg_info), DTSI_REG_U32_NUM);
		if (ret) {
			dev_err(dev, "[%s]parse %s vote_reg fail%d!\n", __func__, child->name, ret);
			ret = -EINVAL;
			goto free_out;
		}
		hvdev_array[id].name         = child->name;
		hvdev_array[id].parent       = channel;
		hvdev_array[id].vote.reg     = channel->base + reg_info.offset;
		hvdev_array[id].vote.rd_mask = reg_info.rd_mask;
		hvdev_array[id].vote.wr_mask = reg_info.wr_mask;
		hvdev_array[id].dev          = NULL;
		hvdev_array[id].last_set     = 0;

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
/*lint -restore */

static int hisi_hv_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *root = dev->of_node;
	struct device_node *child;
	struct dtsi_reg_info reg_info;
	void __iomem *base;
	u32 ratio = 1;
	int count;
	unsigned int id = 0;
	int ret = 0;


	if (!root) {
		dev_err(dev, "[%s]dts[%s] node not found\n", __func__, COMPATIBLE_NODE);
		ret = -ENODEV;
		goto out;
	}

	base = of_iomap(root, 0);
	if (!base) {
		dev_err(dev, "[%s] base iomap fail\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	count = of_get_child_count(root);
	of_node_put(root);
	if (count <= 0) {
		dev_err(dev, "bad channel nodes(%d)\n", count);
		ret = -EINVAL;
		goto out;
	}

	mutex_lock(&hv_array_mutex);

	hv_channel_array = devm_kzalloc(dev, sizeof(struct hv_channel) * count, GFP_KERNEL);
	if (!hv_channel_array) {
		dev_err(dev, "alloc hv_channel_array fail\n");
		ret = -ENOMEM;
		goto out_unlock;
	}
	hv_channel_size = count;

	child = NULL;
	while (NULL != (child = of_get_next_child(root, child))) {
		ret = of_property_read_u32_array(child, "result_reg", (u32 *)(&reg_info), DTSI_REG_U32_NUM);
		if (ret) {
			dev_err(dev, "[%s]parse %s result_reg fail%d!\n", __func__, child->name, ret);
			ret = -EINVAL;
			goto out_unlock;
		}

		ret = of_property_read_u32(child, "ratio", &ratio);
		if (ret) {
			dev_err(dev, "[%s]parse %s ratio fail%d!\n", __func__, child->name, ret);
			ret = -EINVAL;
			goto out_unlock;
		}

		hv_channel_array[id].name           = child->name;
		hv_channel_array[id].ratio          = ratio;
		hv_channel_array[id].base           = base;
		hv_channel_array[id].result.reg     = base + reg_info.offset;
		hv_channel_array[id].result.rd_mask = reg_info.rd_mask;
		hv_channel_array[id].result.wr_mask = reg_info.wr_mask;
		hv_channel_array[id].hvdev_num      = 0;
		hv_channel_array[id].hvdev_head     = NULL;

		ret = get_member_data(dev, child, &(hv_channel_array[id]));
		if (ret) {
			dev_err(dev, "[%s][%s] get vote src fail\n", __func__, child->name);
			ret = -EINVAL;
			goto out_unlock;
		}

		id++;
	}

	HV_DEBUG("%s: hv_channel_size %d\n", __func__, id);

	if (id != hv_channel_size) {
		dev_err(dev, "[%s]channel info error\n", __func__);
		ret = -EINVAL;
		goto out_unlock;
	}

	mutex_unlock(&hv_array_mutex);

	return 0;
out_unlock:
	hv_channel_array = NULL;
	hv_channel_size  = 0;
	mutex_unlock(&hv_array_mutex);
out:
	return ret;
}

static int hisi_hv_remove(struct platform_device *pdev)
{
	hv_channel_array = NULL;
	hv_channel_size  = 0;
	return 0;
}

static const struct of_device_id hisi_hv_of_match[] = {
	{.compatible = "hisi,freq-hw-vote",},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_hv_of_match);

static struct platform_driver hisi_hv_driver = {
	.probe  = hisi_hv_probe,
	.remove = hisi_hv_remove,
	.driver = {
			.name = "HISI-Hardware-Vote",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(hisi_hv_of_match),
		},
};

static int __init hisi_hv_init(void)
{
	return platform_driver_register(&hisi_hv_driver);
}
core_initcall(hisi_hv_init);

static void __exit hisi_hv_exit(void)
{
	platform_driver_unregister(&hisi_hv_driver);
	return;
}
module_exit(hisi_hv_exit);

MODULE_DESCRIPTION("hisi,vote frequency by hardware state machine");
MODULE_LICENSE("GPL V2");


