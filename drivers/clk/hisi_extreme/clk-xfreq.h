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
#ifndef __CLK_XFREQ_H_
#define __CLK_XFREQ_H_
#include <linux/version.h>
#include <linux/clk.h>
#include <securec.h>
#include "clk-mailbox.h"
#include "clk.h"

#define MAX_FREQ_NUM			16
#define DDR_HW_VOTE				1
#define DDRC_MIN_CLK_ID			4
#define DDRC_MAX_CLK_ID			5
#define DMSS_MIN_CLK_ID			6
#define DDR_FREQ_MASK			0x00FF0000

#define FREQ_CONVERSION			1000
#define DDR_FREQ_ID			3
#define CLK_SET_OK			1

struct hi3xxx_xfreq_clk {
	struct clk_hw hw;
	void __iomem *sysreg;
	void __iomem *pmreg;
	u32 id;
	u32 vote_method;
	u32 ddr_mask;
	u32 set_rate_cmd[LPM3_CMD_LEN];
	u32 get_rate_cmd[LPM3_CMD_LEN];
	u32 freq[MAX_FREQ_NUM];
	u32 volt[MAX_FREQ_NUM];
	u32 table_length;
	u32 rate;
};
#endif
