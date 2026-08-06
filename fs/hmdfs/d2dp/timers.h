/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D timers API
 */

#ifndef D2D_TIMERS_H
#define D2D_TIMERS_H

#include <linux/atomic.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/completion.h>

/**
 * d2d_timer_action - timer callback for D2D Protocol
 * @arg: pointer to the function argument structure
 *
 * The callback function with will be executed periodically in a workqueue
 *
 * Returns:
 * true  - the timer should be restarted
 * false - the timer should not be restarted (in case of errors)
 */
typedef bool (*d2d_timer_action)(void *arg);

struct d2d_timer {
	struct delayed_work work;
	d2d_timer_action func;
	void *arg;
	u8 retry_num;
	u32 default_delay_usecs;
	u32 delay_usecs;
	struct workqueue_struct *wq;
	atomic_t enabled;
	atomic64_t reset_time;
	atomic_t destroying;
	struct completion completion;
};

struct workqueue_struct *d2d_timer_workqueue_create(char *wq_name);
void d2d_timer_workqueue_destroy(struct workqueue_struct *wq);

void d2d_timer_create(struct workqueue_struct *wq, struct d2d_timer *timer,
		      u32 usecs, d2d_timer_action action, void *arg);
void d2d_timer_destroy(struct d2d_timer *timer);

void d2d_timer_enable(struct d2d_timer *timer);
void d2d_timer_disable(struct d2d_timer *timer);
void d2d_timer_reset(struct d2d_timer *timer);

#endif /* D2D_TIMERS_H */
