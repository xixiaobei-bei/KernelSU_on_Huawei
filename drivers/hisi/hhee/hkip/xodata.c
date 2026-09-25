/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Hisi XO interface
 * Creator : security-ap
 * Create : 2019/9/2
 */

#include <asm/sections.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hisi/hisi_hkip.h>

int hkip_register_xo(const uintptr_t base, size_t size)
{
	unsigned long addr = (unsigned long)base;
	BUG_ON((addr | size) & ~PAGE_MASK);

	if (hkip_hvc3(HKIP_HVC_XO_REGISTER, addr, size))
		return -ENOTSUPP;
	return 0;
}

MODULE_DESCRIPTION("Huawei kernel xom protection");
MODULE_AUTHOR("wang.kui1@huawei.com");
MODULE_LICENSE("GPL");
