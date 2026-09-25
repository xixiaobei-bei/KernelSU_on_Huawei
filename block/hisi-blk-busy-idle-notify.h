#ifndef __HISI_BLK_BUSY_IDLE_NOTIFY__
#define __HISI_BLK_BUSY_IDLE_NOTIFY__
#include <linux/blkdev.h>

/* Internal structure! */
struct blk_busy_idle_nb {
	/* list into subscribed_event_list */
	struct list_head subscribed_event_node;
	/* copied from subscribed event node */
	/* for internal maintenance */
	struct blk_dev_lld *subscriber_source;
	/* for internal maintenance */
	struct timer_list busy_idle_handler_latency_check_timer;
	/* would be initialized by block layer rather than subscriber module*/
	struct notifier_block busy_idle_nb;
	/* Copy of subscriber's event_node */
	struct blk_busy_idle_event_node event_node;
	/* provided by subscriber module */
	/* for internal maintenance */
	enum blk_io_state last_state;
	/*If no special requirement, pls keep it zero */
	unsigned long nb_flag;
	/* for internal maintenance */
	unsigned long continue_trigger_io_count;
};

ssize_t hisi_queue_hw_idle_enable_show(struct request_queue *q, char *page);
ssize_t hisi_queue_idle_state_show(struct request_queue *q, char *page);

#endif /* __HISI_BLK_BUSY_IDLE_NOTIFY__ */
