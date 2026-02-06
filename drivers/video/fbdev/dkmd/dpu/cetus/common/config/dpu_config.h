/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef DISP_CONFIG_H
#define DISP_CONFIG_H

#include <linux/platform_device.h>
#include <linux/types.h>

#include "dpu_config_v110.h"

struct dpu_config {
	char __iomem *ip_base_addrs[DISP_IP_MAX];
	uint32_t lbuf_size;
};

extern struct dpu_config g_dpu_config;

int dpu_init_config(struct platform_device *device);


static inline char __iomem *dpu_config_get_ip_base(uint32_t ip)
{
	if (ip >= DISP_IP_MAX)
		return NULL;

	return g_dpu_config.ip_base_addrs[ip];
}

static inline uint32_t dpu_config_get_lbuf_size()
{
	return g_dpu_config.lbuf_size;
}


#endif /* DISP_CONFIG_H */
