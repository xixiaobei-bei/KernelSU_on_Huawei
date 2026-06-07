#include "hisi-blk-busy-idle-interface.h"

void __cfi_hisi_blk_busy_idle_handler_latency_check_timer_expire(unsigned long data)
{
        hisi_blk_busy_idle_handler_latency_check_timer_expire(data);
}

int __cfi_hisi_blk_busy_idle_notify_handler(
	struct notifier_block *nb, unsigned long val, void *v)
{
	return hisi_blk_busy_idle_notify_handler(nb, val, v);
}

void __cfi_hisi_blk_idle_notify_work(struct work_struct *work)
{
        hisi_blk_idle_notify_work(work);
}

ssize_t __cfi_hisi_queue_busy_idle_enable_store(
	struct request_queue *q, const char *page, size_t count)
{
	return hisi_queue_busy_idle_enable_store(q, page, count);
}

ssize_t __cfi_hisi_queue_busy_idle_statistic_reset_store(
	struct request_queue *q, const char *page, size_t count)
{
	return hisi_queue_busy_idle_statistic_reset_store(q, page, count);
}

ssize_t __cfi_hisi_queue_busy_idle_statistic_show(struct request_queue *q, char *page)
{
	return hisi_queue_busy_idle_statistic_show(q, page);
}

ssize_t __cfi_hisi_queue_hw_idle_enable_show(struct request_queue *q, char *page)
{
	return hisi_queue_hw_idle_enable_show(q, page);
}

ssize_t __cfi_hisi_queue_idle_state_show(struct request_queue *q, char *page)
{
	return hisi_queue_idle_state_show(q, page);
}

void __cfi_hisi_blk_busy_idle_end_rq(struct request *rq, blk_status_t error)
{
        hisi_blk_busy_idle_end_rq(rq, error);
}


