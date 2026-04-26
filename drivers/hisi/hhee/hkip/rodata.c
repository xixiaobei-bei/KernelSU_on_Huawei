/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 *  Description: hkip ro data proctect
 *  Create : 2018/4/26
 */

#include <asm/sections.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/hisi/hkip.h>

u32 hkip_hvc_dispatch(u32 fid, unsigned long x1, unsigned long x2,
		      unsigned long x3, unsigned long x4, unsigned long x5,
		      unsigned long x6)
{
	static DEFINE_SPINLOCK(lock);
	unsigned long flags;

	if (hhee_is_present()) {
		/*
		 * Perform HVC normally during early boot
		 * (before assembler alternatives are applied)
		 */
		struct arm_smccc_res res;

		arm_smccc_hvc(fid, x1, x2, x3, x4, x5, x6, 0, &res);
		return res.a0;
	}

	switch (fid) {
	case HKIP_HVC_ROWM_SET_BIT: {
		u8 *bits = (u8 *)(uintptr_t)x1;

		bits += (x2 / 8);
		x2 %= 8;

		spin_lock_irqsave(&lock, flags);
		if (x3)
			*bits |= 1u << x2;
		else
			*bits &= ~(1u << x2);
		spin_unlock_irqrestore(&lock, flags);
		return 0;
	}
	default:
		break;
	}
	return ENOTSUPP;
}

MODULE_DESCRIPTION("HKIP kernel data protection");
MODULE_LICENSE("GPL");
