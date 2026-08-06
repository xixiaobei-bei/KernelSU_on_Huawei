/*
 * edp_bridge_register.c
 *
 * operate function for edp_bridge_register
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "edp_bridge_register.h"
#include "edp_bridge.h"

LIST_HEAD(mipi2edp_bridge_chip_info_list);
LIST_HEAD(mipi2edp_bridge_product_info_list);

static void bridge_chip_info_clone(struct mipi2edp_bridge_chip_info *src, struct mipi2edp_bridge_chip_info *dst)
{
	dst->chip_addr = src->chip_addr;
	dst->name = src->name;
	dst->regmap_config = src->regmap_config;
	dst->is_chip_onboard = src->is_chip_onboard;
	dst->pre_enable = src->pre_enable;
	dst->enable = src->enable;
	dst->disable = src->disable;
	dst->post_disable = src->post_disable;
	dst->get_edid = src->get_edid;
	dst->transfer = src->transfer;
	dst->set_clk = src->set_clk;
}

static int check_bridge_chip_info_valid(struct mipi2edp_bridge_chip_info *chip_info)
{
	if (!chip_info) {
		printk(KERN_DEBUG "chip_info is nullptr!");
		return -EINVAL;
	}
	if (!chip_info->name || !chip_info->is_chip_onboard || !chip_info->regmap_config
		|| !chip_info->get_edid ) {
		printk(KERN_DEBUG "register bridge chip info is invalid!name:%p, is_chip_onboard:%p, regmap_config:%p,"
			" get_edid:%p", chip_info->name, chip_info->is_chip_onboard, chip_info->regmap_config,
			chip_info->get_edid);
		return -EINVAL;
	}
	return 0;
}

int mipi2edp_bridge_chip_info_register(struct mipi2edp_bridge_chip_info *chip_info)
{
	struct list_head *entry = NULL;
	struct list_head *next = NULL;
	struct mipi2edp_bridge_chip_info *chip_info_it = NULL;

	if (check_bridge_chip_info_valid(chip_info))
		return -EINVAL;

	list_for_each_safe(entry, next, &mipi2edp_bridge_chip_info_list) {
		chip_info_it =  list_entry(entry, struct mipi2edp_bridge_chip_info, list);
		if (chip_info->chip_addr == chip_info_it->chip_addr) {
			printk(KERN_DEBUG "the chip addr:%d has been registed by %s!", chip_info_it->chip_addr,
				chip_info_it->name);
			return -EINVAL;
		}
	}

	list_add(&chip_info->list, &mipi2edp_bridge_chip_info_list);

	return 0;
}

int mipi2edp_bridge_chip_info_get(enum mipi2edp_chip_addr chip_addr, struct mipi2edp_bridge_chip_info *chip_info)
{
	struct list_head *entry = NULL;
	struct list_head *next = NULL;
	struct mipi2edp_bridge_chip_info *chip_info_it = NULL;

	if (!chip_info) {
		HISI_DRM_INFO("chip_info is nullptr!");
		return -EINVAL;
	}

	list_for_each_safe(entry, next, &mipi2edp_bridge_chip_info_list) {
		chip_info_it =  list_entry(entry, struct mipi2edp_bridge_chip_info, list);
		if (chip_addr == chip_info_it->chip_addr) {
			bridge_chip_info_clone(chip_info_it, chip_info);
			HISI_DRM_INFO("confirm chip:%d to %s!", chip_info_it->chip_addr, chip_info_it->name);
			return 0;
		}
	}
	HISI_DRM_INFO("chip_addr:%d mipi2edp bridge chip info has't been registed!", chip_addr);
	return -EINVAL;
}

static int check_bridge_product_info_valid(struct mipi2edp_bridge_product_info *product_info)
{
	if (!product_info) {
		printk(KERN_DEBUG "product_info is nullptr!");
		return -EINVAL;
	}
	if (!product_info->desc || !product_info->bridge_ops || !product_info->bridge_connector_ops
		|| !product_info->bridge_pm_ops) {
		printk(KERN_DEBUG "register bridge product info is invalid!desc:%p, bridge_ops:%p,"
			" bridge_connector_ops:%p, bridge_pm_ops:%p", product_info->desc, product_info->bridge_ops,
			product_info->bridge_connector_ops, product_info->bridge_pm_ops);
		return -EINVAL;
	}
	return 0;
}

static void bridge_product_info_clone(struct mipi2edp_bridge_product_info *src,
	struct mipi2edp_bridge_product_info *dst)
{
	dst->product_type = src->product_type;
	dst->product_series = src->product_series;
	dst->desc = src->desc;
	dst->connector_type = src->connector_type;
	dst->bridge_ops = src->bridge_ops;
	dst->bridge_connector_ops = src->bridge_connector_ops;
	dst->bridge_pm_ops = src->bridge_pm_ops;
}

int mipi2edp_bridge_product_info_register(struct mipi2edp_bridge_product_info *product_info)
{
	struct list_head *entry = NULL;
	struct list_head *next = NULL;
	struct mipi2edp_bridge_product_info *product_info_it = NULL;

	if (check_bridge_product_info_valid(product_info))
		return -EINVAL;

	list_for_each_safe(entry, next, &mipi2edp_bridge_product_info_list) {
		product_info_it =  list_entry(entry, struct mipi2edp_bridge_product_info, list);
		if (product_info->product_type == product_info_it->product_type) {
			printk(KERN_DEBUG "the product:%d has been registed by %s!", product_info_it->product_type,
				product_info_it->desc);
			return -EINVAL;
		}
	}

	list_add(&product_info->list, &mipi2edp_bridge_product_info_list);

	return 0;
}

int mipi2edp_bridge_product_info_get(enum mipi2edp_product_type product_type,
	struct mipi2edp_bridge_product_info *product_info)
{
	struct list_head *entry = NULL;
	struct list_head *next = NULL;
	struct mipi2edp_bridge_product_info *product_info_it = NULL;

	if (!product_info) {
		HISI_DRM_INFO("product_info is nullptr!");
		return -EINVAL;
	}

	list_for_each_safe(entry, next, &mipi2edp_bridge_product_info_list) {
		product_info_it =  list_entry(entry, struct mipi2edp_bridge_product_info, list);
		if (product_type == product_info_it->product_type) {
			bridge_product_info_clone(product_info_it, product_info);
			HISI_DRM_INFO("confirm product:%d to %s!", product_info_it->product_type, product_info_it->desc);
			return 0;
		}
	}
	HISI_DRM_INFO("product_type:%d mipi2edp bridge product info has't been registed!", product_type);
	return -EINVAL;
}
