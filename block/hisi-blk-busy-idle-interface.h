#ifndef __HISI_BLK_BUSY_IDLE_INTERFACE_H__
#define __HISI_BLK_BUSY_IDLE_INTERFACE_H__
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/types.h>

extern void __cfi_hisi_blk_busy_idle_handler_latency_check_timer_expire(unsigned long data);
extern void hisi_blk_busy_idle_handler_latency_check_timer_expire(unsigned long data);
extern int __cfi_hisi_blk_busy_idle_notify_handler(struct notifier_block *nb, unsigned long val, void *v);
extern int hisi_blk_busy_idle_notify_handler(struct notifier_block *nb, unsigned long val, void *v);
extern void __cfi_hisi_blk_idle_notify_work(struct work_struct *work);
extern void hisi_blk_idle_notify_work(struct work_struct *work);
extern ssize_t __cfi_hisi_queue_busy_idle_enable_store(struct request_queue *q, const char *page, size_t count);
extern ssize_t hisi_queue_busy_idle_enable_store(struct request_queue *q, const char *page, size_t count);
extern ssize_t __cfi_hisi_queue_busy_idle_statistic_reset_store(struct request_queue *q, const char *page, size_t count);
extern ssize_t hisi_queue_busy_idle_statistic_reset_store(struct request_queue *q, const char *page, size_t count);
extern ssize_t __cfi_hisi_queue_busy_idle_statistic_show(struct request_queue *q, char *page);
extern ssize_t hisi_queue_busy_idle_statistic_show(struct request_queue *q, char *page);
extern ssize_t __cfi_hisi_queue_hw_idle_enable_show(struct request_queue *q, char *page);
extern ssize_t hisi_queue_hw_idle_enable_show(struct request_queue *q, char *page);
extern ssize_t __cfi_hisi_queue_idle_state_show(struct request_queue *q, char *page);
extern ssize_t hisi_queue_idle_state_show(struct request_queue *q, char *page);
extern void __cfi_hisi_blk_busy_idle_end_rq(struct request *rq, blk_status_t error);
extern void hisi_blk_busy_idle_end_rq(struct request *rq, blk_status_t error);
#endif /* __HISI_BLK_BUSY_IDLE_INTERFACE_H__ */
