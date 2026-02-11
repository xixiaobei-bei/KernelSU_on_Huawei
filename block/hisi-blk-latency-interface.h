#ifndef __HISI_BLK_LATENCY_INTERFACE_H__
#define __HISI_BLK_LATENCY_INTERFACE_H__

extern void hisi_blk_latency_check_timer_expire(unsigned long data);
extern void __hisi_blk_latency_check_timer_expire(unsigned long data);
extern ssize_t hisi_queue_io_latency_warning_threshold_store(
	struct request_queue *q, const char *page, size_t count);
extern ssize_t __hisi_queue_io_latency_warning_threshold_store(
	struct request_queue *q, const char *page, size_t count);
extern ssize_t hisi_queue_io_latency_statistic_enable_store(
	struct request_queue *q, const char *page, size_t count);
extern ssize_t __hisi_queue_io_latency_statistic_enable_store(
	struct request_queue *q, const char *page, size_t count);
extern ssize_t hisi_queue_io_latency_statistic_show(
	struct request_queue *q, char *page);
extern ssize_t __hisi_queue_io_latency_statistic_show(
	struct request_queue *q, char *page);
extern ssize_t hisi_queue_io_hw_latency_statistic_show(
	struct request_queue *q, char *page);
extern ssize_t __hisi_queue_io_hw_latency_statistic_show(
	struct request_queue *q, char *page);
extern ssize_t hisi_queue_io_sw_latency_statistic_show(
	struct request_queue *q, char *page);
extern ssize_t __hisi_queue_io_sw_latency_statistic_show(
	struct request_queue *q, char *page);

#endif /* __HISI_BLK_LATENCY_INTERFACE_H__ */
