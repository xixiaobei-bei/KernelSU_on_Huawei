/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef __DP_PHY_CONFIG_INTERFACE_H__
#define __DP_PHY_CONFIG_INTERFACE_H__

#include <linux/kernel.h>

struct dp_ctrl;

struct phylane_mapping {
	int plug_type;
	int lane;
	int phylane;
};

struct usb31phy_mapping {
	uint32_t sw_level;
	uint32_t pe_level;
	bool need_config;
	uint32_t tx_vboot_addr_val;
	uint32_t u161n02_addr_val;
	uint32_t u161n03_addr_val;
};

void dptx_phy_layer_init(struct dp_ctrl *dptx);

#endif /* DP_PHY_CONFIG_INTERFACE_H */
