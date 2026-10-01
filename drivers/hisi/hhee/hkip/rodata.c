/*
 * Copyright (c) 2017 Huawei Technologies.
 */

#include <asm/sections.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/hisi/hisi_hkip.h>

bool hhee_is_present(void)
{
	return hhee_check_enable() != HHEE_DISABLE;
}

u32 hkip_hvc_dispatch(u32 fid, unsigned long x1, unsigned long x2,
			unsigned long x3, unsigned long x4, unsigned long x5,
			unsigned long x6)
{
	static DEFINE_SPINLOCK(lock);
	unsigned long flags;

	if (hhee_is_present()) {
		/* Perform HVC normally during early boot
		 * (before assembler alternatives are applied) */
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
	}
	return ENOTSUPP;
}

int hkip_register_ro(const void *base, size_t size)
{
	uintptr_t addr = (uintptr_t)base;

	BUG_ON((addr | size) & ~PAGE_MASK);

	if (hkip_hvc3(HKIP_HVC_RO_REGISTER, addr, size))
		return -ENOTSUPP;
	return 0;
}

int hkip_register_ro_mod(const void *base, size_t size)
{
	uintptr_t addr = (uintptr_t)base;

	BUG_ON((addr | size) & ~PAGE_MASK);

	if (hkip_hvc3(HKIP_HVC_RO_MOD_REGISTER, addr, size))
		return -ENOTSUPP;
	return 0;
}
EXPORT_SYMBOL(hkip_register_ro_mod);

int hkip_unregister_ro_mod(const void *base, size_t size)
{
	uintptr_t addr = (uintptr_t)base;

	BUG_ON((addr | size) & ~PAGE_MASK);

	if (hkip_hvc3(HKIP_HVC_RO_MOD_UNREGISTER, addr, size))
		return -ENOTSUPP;
	return 0;
}
EXPORT_SYMBOL(hkip_unregister_ro_mod);

MODULE_DESCRIPTION("Huawei kernel read-only data protection");
MODULE_AUTHOR("Remi Denis-Courmont <remi.denis.courmont@huawei.com>");
MODULE_LICENSE("GPL");
