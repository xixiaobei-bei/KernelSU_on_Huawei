// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/printk.h>
#include <linux/atomic.h>
#include <linux/completion.h>

#include "timers.h"

static void work_handler(struct work_struct *work)
{
	struct d2d_timer *timer = NULL;
	ktime_t reset           = 0;
	s64 usecs_reset         = 0;
	bool need_restart       = true;

	timer = container_of(work, struct d2d_timer, work.work);

	if (atomic_read_acquire(&timer->destroying)) {
		complete(&timer->completion);
		return;
	}

	/* check if need to reset timer */
	reset = atomic64_xchg(&timer->reset_time, 0);
	if (reset) {
		ktime_t now = ktime_get();
		ktime_t passed = now - reset;
		u32 period_usecs = timer->delay_usecs;
		ktime_t last = ktime_set(0, period_usecs * 1000) - passed;

		timer->retry_num = 0;
		timer->delay_usecs = timer->default_delay_usecs;
		if (last > 0) {
			usecs_reset = ktime_to_us(last);
			queue_delayed_work(timer->wq, &timer->work,
					   usecs_to_jiffies(usecs_reset));
			return;
		}
	}

	/*
	 * Let's avoid possible problems by pairing this load with store in
	 * enable/disable methods
	 */
	if (atomic_read_acquire(&timer->enabled))
		need_restart = timer->func(timer->arg);

	if (need_restart)
		queue_delayed_work(timer->wq, &timer->work,
				   usecs_to_jiffies(timer->delay_usecs));
	else
		complete(&timer->completion);
}

struct workqueue_struct *d2d_timer_workqueue_create(char *wq_name)
{
	return alloc_workqueue("%s", WQ_MEM_RECLAIM, 1, wq_name);
}

void d2d_timer_workqueue_destroy(struct workqueue_struct *wq)
{
	flush_workqueue(wq);
	destroy_workqueue(wq);
}

void d2d_timer_create(struct workqueue_struct *wq, struct d2d_timer *timer,
		      u32 usecs, d2d_timer_action action, void *arg)
{
	ktime_t now = ktime_get();

	atomic64_set_release(&timer->reset_time, now);
	timer->func = action;
	timer->arg = arg;
	timer->delay_usecs = usecs;
	timer->default_delay_usecs = usecs;
	timer->retry_num = 0;
	atomic_set_release(&timer->enabled, 0);
	init_completion(&timer->completion);
	atomic_set_release(&timer->destroying, 0);

	timer->wq = wq;
	INIT_DELAYED_WORK(&timer->work, work_handler);
	queue_delayed_work(timer->wq,
			   &timer->work,
			   usecs_to_jiffies(timer->delay_usecs));
}

void d2d_timer_destroy(struct d2d_timer *timer)
{
	atomic_set_release(&timer->destroying, 1);
	wait_for_completion(&timer->completion);
}

void d2d_timer_enable(struct d2d_timer *timer)
{
	atomic_set_release(&timer->enabled, 1);
}

void d2d_timer_disable(struct d2d_timer *timer)
{
	atomic_set_release(&timer->enabled, 0);
}

void d2d_timer_reset(struct d2d_timer *timer)
{
	ktime_t now = ktime_get();

	atomic64_set_release(&timer->reset_time, now);
}
