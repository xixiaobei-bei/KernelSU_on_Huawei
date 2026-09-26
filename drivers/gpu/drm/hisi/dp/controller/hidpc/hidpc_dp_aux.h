/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef __HIDPC_DP_AUX_H__
#define __HIDPC_DP_AUX_H__

#include <linux/kernel.h>

struct dp_ctrl;

/*
 * aux read/write
 */
int dptx_aux_rw(struct dp_ctrl *dptx, bool rw, bool i2c, bool mot,
	bool addr_only, uint32_t addr, uint8_t *bytes, uint32_t len);

#endif // HIDPC_DP_AUX

