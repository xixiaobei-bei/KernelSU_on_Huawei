#ifndef __HISI_BLK_FLUSH_INTERFACE_H__
#define __HISI_BLK_FLUSH_INTERFACE_H__
#include <linux/workqueue.h>
#include <linux/notifier.h>

extern void __cfi_hisi_blk_flush_work_fn(struct work_struct *work);
extern void hisi_blk_flush_work_fn(struct work_struct *work);
extern int __cfi_hisi_blk_poweroff_flush_notifier_call(
	struct notifier_block *powerkey_nb, unsigned long event, void *data);
extern int hisi_blk_poweroff_flush_notifier_call(
	struct notifier_block *powerkey_nb, unsigned long event, void *data);
extern int __init __cfi_hisi_blk_flush_init(void);
extern int __init hisi_blk_flush_init(void);
extern void __exit __cfi_hisi_blk_flush_exit(void);
extern void __exit hisi_blk_flush_exit(void);
#endif /* __HISI_BLK_FLUSH_INTERFACE_H__ */

