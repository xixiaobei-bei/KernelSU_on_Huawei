/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the tedpus of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>

#include <linux/device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "dpu_utils.h"
#include "dpu_config.h"

struct dpu_config g_dpu_config;

static void _config_get_ip_base(struct device_node *np, char __iomem *ip_base_addrs[], uint32_t ip_count)
{
	uint32_t i;

	dpu_pr_info("+++");

	for (i = 0; i < ip_count; i++) {
		ip_base_addrs[i] = of_iomap(np, i);
		if (!ip_base_addrs[i])
			dpu_pr_err("get ip %u base addr fail", i);

		dpu_pr_info("ip %u, ip_base_addrs:0x%llx", i, ip_base_addrs[i]);
	}
}

static uint32_t _config_get_lbuf_size(struct device_node *np)
{
	uint32_t lbuf_size = 0;

	of_property_read_u32(np, "linebuf_size", &lbuf_size);
	dpu_pr_info("lbuf_size = %u K", lbuf_size / SIZE_1K);

	return lbuf_size;
}

int dpu_init_config(struct platform_device *device)
{
	struct device_node *np = NULL;

	dpu_pr_info("+++");

	if (!device)
		return -1;

	np = device->dev.of_node;
	if (!np) {
		dpu_pr_err("platfodpu device of node is null");
		return -1;
	}

	_config_get_ip_base(np, g_dpu_config.ip_base_addrs, DISP_IP_MAX);

	_config_get_lbuf_size(np);

	return 0;
}
