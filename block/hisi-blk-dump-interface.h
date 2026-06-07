#ifndef __HISI_BLK_DUMP_INTERFACE_H__
#define __HISI_BLK_DUMP_INTERFACE_H__
#include <linux/kernel.h>
#include <linux/blkdev.h>

extern int __cfi_hisi_blk_dump_panic_notify(
	struct notifier_block *nb, unsigned long event, void *buf);
extern int hisi_blk_dump_panic_notify(
	struct notifier_block *nb, unsigned long event, void *buf);

#endif /* __HISI_BLK_DUMP_INTERFACE_H__ */
