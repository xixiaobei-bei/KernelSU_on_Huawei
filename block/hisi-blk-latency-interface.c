#include <linux/kernel.h>
#include <linux/blkdev.h>
#include "hisi-blk-latency-interface.h"

void __hisi_blk_latency_check_timer_expire(unsigned long data)
{
        hisi_blk_latency_check_timer_expire(data);
}

ssize_t __hisi_queue_io_latency_warning_threshold_store(
	struct request_queue *q, const char *page, size_t count)
{
	return hisi_queue_io_latency_warning_threshold_store(q, page, count);
}

ssize_t __hisi_queue_io_latency_statistic_enable_store(
	struct request_queue *q, const char *page, size_t count)
{
	return hisi_queue_io_latency_statistic_enable_store(q, page, count);
}

ssize_t __hisi_queue_io_latency_statistic_show(
	struct request_queue *q, char *page)
{
	return hisi_queue_io_latency_statistic_show(q, page);
}

ssize_t __hisi_queue_io_hw_latency_statistic_show(
	struct request_queue *q, char *page)
{
	return hisi_queue_io_hw_latency_statistic_show(q, page);
}

ssize_t __hisi_queue_io_sw_latency_statistic_show(
	struct request_queue *q, char *page)
{
	return hisi_queue_io_sw_latency_statistic_show(q, page);
}


