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
#ifndef __PLAT_CLK_DIV_H_
#define __PLAT_CLK_DIV_H_

#include <linux/clk.h>
#include <securec.h>
#include "clk.h"

#define WIDTH_TO_MASK(width)		((1 << (width)) - 1)
#define DIV_MASK_OFFSET		16

/*
 * The reverse of DIV_ROUND_UP: The maximum number which
 * divided by m is r
 */
#define MULT_ROUND_UP(r, m)                    ((r) * (m) + (m) - 1)

struct hi3xxx_divclk {
	struct clk_hw hw;
	void __iomem *reg; /* divider register */
	u8 shift;
	u8 width;
	u32 mbits; /* mask bits in divider register */
	const struct clk_div_table *table;
#ifdef CONFIG_HISI_CLK_WAIT_DONE
	void __iomem *div_done;
	u32 done_bit;
#endif
	spinlock_t *lock;
};
#endif
