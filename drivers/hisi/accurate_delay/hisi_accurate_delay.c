#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/arch_timer.h>

static unsigned long syscount_100us = 0;

void accurate_delay_100us(unsigned long us_100)
{
	unsigned long now;
	unsigned long target;

	local_irq_disable();

	now = arch_counter_get_cntvct();
	target = now + us_100*syscount_100us;
	while(now < target)
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
