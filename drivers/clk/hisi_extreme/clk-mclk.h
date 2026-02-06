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
#ifndef __CLK_MCLK_H_
#define __CLK_MCLK_H_

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <securec.h>
#include <linux/hwspinlock.h>
#include <linux/mfd/hisi_pmic.h>
#include "clk-mailbox.h"
#include "clk.h"

#ifdef CONFIG_HISI_CLK_DEBUG
#include "debug/clk-debug.h"
#endif

struct hi3xxx_mclk {
	struct clk_hw	hw;
	u32		ref_cnt; /* reference count */
	u32		en_cmd[LPM3_CMD_LEN];
	u32		dis_cmd[LPM3_CMD_LEN];
	u32		always_on;
	u32		gate_abandon_enable;
	spinlock_t *lock;
};

#endif