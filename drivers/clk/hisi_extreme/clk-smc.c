/*
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include "clk-smc.h"
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>

#define clk_smc_e(fmt, args ...) pr_err("[CLK_SMC]"fmt, ##args)

#define clk_smc_i(fmt, args ...) pr_info("[CLK_SMC]"fmt, ##args)

#define clk_smc_d(fmt, args ...) pr_debug("[CLK_SMC]"fmt, ##args)

static DEFINE_SPINLOCK(clk_smc_lock);

/* atf entrance function support */
noinline int atfd_service_clk_smc(u64 _arg1, u64 _arg2, u64 _arg3, u64 _arg4)
{
	register u64 function_id asm("x0") = CLK_REGISTER_FN_ID;
	register u64 arg0 asm("x1") = _arg1;
	register u64 arg1 asm("x2") = _arg2;
	register u64 arg2 asm("x3") = _arg3;
	register u64 arg3 asm("x4") = _arg4;
	unsigned long flags;

	spin_lock_irqsave(&clk_smc_lock, flags);

	asm volatile(
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			__asmeq("%4", "x4")
			"smc    #0\n"
			: "+r" (function_id)
			: "r" (arg0), "r" (arg1), "r" (arg2), "r" (arg3));

	spin_unlock_irqrestore(&clk_smc_lock, flags);

	return (int)function_id;
}

noinline struct smc_ppll atfd_ppll_clk_smc(struct smc_ppll args)
{
	register uint64_t r0 __asm__("x0") = CLK_REGISTER_FN_ID;
	register uint64_t r1 __asm__("x1") = args.arg1;
	register uint64_t r2 __asm__("x2") = args.arg2;
	register uint64_t r3 __asm__("x3") = args.arg3;
	register uint64_t r4 __asm__("x4") = args.arg4;

	__asm__ volatile(
		"smc    #0\n"
		: /* Output registers, also used as inputs ('+' constraint). */
		"+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));

	return (struct smc_ppll) {
		.smc_fid = r0,
		.arg1 = r1,
		.arg2 = r2,
		.arg3 = r3,
		.arg4 = r4,
	};
}

