/*
 * hisi_accurate_delay.c
 *
 * hisi accurate delay
 *
 * Copyright (C) 2018-2020 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/arch_timer.h>

static unsigned long syscount_100us;

void accurate_delay_100us(unsigned long us_100)
{
	unsigned long now;
	unsigned long target;

	local_irq_disable();

	now = arch_counter_get_cntvct();
	target = now + us_100*syscount_100us;
	while (now < target)
		now = arch_counter_get_cntvct();

	local_irq_enable();

	return;
}

static int hisi_accurate_delay_init(void)
{
	syscount_100us = (unsigned long)(arch_timer_get_cntfrq() / 10000);

	return 0;
}
arch_initcall(hisi_accurate_delay_init);

MODULE_AUTHOR("Hisilicon Co. Ltd");
MODULE_DESCRIPTION("Hisi Accurate Delay Lib");
MODULE_LICENSE("GPL");
