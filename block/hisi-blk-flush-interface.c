#include "hisi-blk-flush-interface.h"

void __cfi_hisi_blk_flush_work_fn(struct work_struct *work)
{
        hisi_blk_flush_work_fn(work);
}

int __cfi_hisi_blk_poweroff_flush_notifier_call(
	struct notifier_block *powerkey_nb, unsigned long event, void *data)
{
	return hisi_blk_poweroff_flush_notifier_call(powerkey_nb, event, data);
}

int __init __cfi_hisi_blk_flush_init(void)
{
	return hisi_blk_flush_init();
}

void __exit __cfi_hisi_blk_flush_exit(void)
{
        hisi_blk_flush_exit();
}


