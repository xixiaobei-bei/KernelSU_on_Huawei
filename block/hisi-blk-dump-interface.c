#include "hisi-blk-dump-interface.h"

int __cfi_hisi_blk_dump_panic_notify(
	struct notifier_block *nb, unsigned long event, void *buf)
{
	return hisi_blk_dump_panic_notify(nb, event, buf);
}

