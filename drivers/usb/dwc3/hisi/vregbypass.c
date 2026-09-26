/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: utilityies for usb.
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/hisi/bl31_smc.h>

static unsigned long write_vregbypass_efuse(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("product is eng, do not set vregbypass\n");
	return 0;
#else
	register uint64_t r0 __asm__("x0") = (uint64_t)FID_USB_SET_VREGBYPASS_EFUSE;
	register uint64_t r1 __asm__("x1") = 0;

	__asm__ volatile("smc #0"
			: /* Output registers, also used as inputs ('+' constraint). */
			"+r"(r0), "+r"(r1));
	return r0;
#endif
}

void usb20phy_vregbypass_init(void)
{
	unsigned long smc_ret;

	smc_ret = write_vregbypass_efuse();
	pr_info("%s: write vregbypass return 0x%llx\n", __func__, smc_ret);
}
