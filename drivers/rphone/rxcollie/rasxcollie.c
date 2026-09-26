#include "../rasbase/rasbase.h"
#include "../rasbase/rasprobe.h"
#include "../rasbase/rasproc.h"
#include <chipset_common/hwxcollie/hw_xcollie.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/statfs.h>
#include <linux/module.h>
#include <linux/path.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/timekeeping.h>
#include <linux/delay.h>

static struct task_struct *rxcollie_task;
static int ovt_time;
static int sleep_time;
static int test_loop;

#define rxcollie_timespec_cmp(a, b, op) \
	((a)->tv_sec == (b)->tv_sec ? \
	(a)->tv_nsec op(b)->tv_nsec : \
	(a)->tv_sec op(b)->tv_sec)

#define HW_SLEEP_THREASHOLD_NANOSECOND  (100 * 1000 * 1000)
#define HW_TIMERRING_CHECK_INTVAL       1
#define THOUSAND                        1000
#define MILLION                         (THOUSAND * THOUSAND)
#define BILLION                         (MILLION * THOUSAND)
#define TEN_SECOND                      10

typedef int (*replace_fun_start)(const char *name, int timeout,
	int (*func)(void *), void *arg, int flag);
typedef void (*replace_fun_end)(int id);

static replace_fun_start hwxcollie_start;
static replace_fun_end hwxcollie_end;

static void rxcollie_sub_timespec(struct timespec *minuend,
	struct timespec *subtrahend, struct timespec *x)
{
	x->tv_sec = minuend->tv_sec - subtrahend->tv_sec;
	if (minuend->tv_nsec >= subtrahend->tv_nsec) {
		x->tv_nsec = minuend->tv_nsec - subtrahend->tv_nsec;
	} else {
		x->tv_nsec = BILLION - subtrahend->tv_nsec + minuend->tv_nsec;
		x->tv_sec--;
	}
}

static int rxcollie_callback(void *arg)
{
	pr_info("rxcollie: inject callback %s\n", (char *)arg);

	return 0;
}

static int rxcollie_clock_target_check(struct timespec *ts,
	struct timespec *cur,
	struct timespec *delta)
{
	struct timespec min = { 0, HW_SLEEP_THREASHOLD_NANOSECOND };

	*cur = current_kernel_time();
	if (rxcollie_timespec_cmp(cur, ts, >))
		return -1;

	rxcollie_sub_timespec(ts, cur, delta);
	if (rxcollie_timespec_cmp(delta, &min, <))
		return 1;
	return 0;
}

static void rxcollie_sleep(unsigned long second)
{
	struct timespec to;
	struct timespec cur;
	struct timespec delta;
	unsigned int msecs;

	to = current_kernel_time();
	to.tv_sec += second;

	while (1) {
		if (rxcollie_clock_target_check(&to, &cur, &delta))
			break;

		msecs = (delta.tv_sec % TEN_SECOND) * THOUSAND +
			delta.tv_nsec / MILLION;

		if (msecs > (TEN_SECOND*THOUSAND)) {
			pr_err("hwxcollie: xcollie_sleep ovt %u\n", msecs);
			break;
		}
		msleep(msecs);
	}
}

static int rxcollie_thread_handle(void *arg)
{
	int id = 0;
	char hahaha[] = "haahhahaa";

	pr_info("rxcollie: inject thread start run\n");

	while (1) {

		if (ovt_time == 0)
			continue;

		pr_info("rxcollie: inject thread start\n");

		/* Set overtime Value */
		id = hwxcollie_start(__func__,
				(int)(ovt_time * HW_TIMERRING_CHECK_INTVAL),
				rxcollie_callback,
				(void *)hahaha,
				HW_XCOLLIE_FLAG_DEFAULT);

		ovt_time = 0;

		/* Set Sleep time Value */
		rxcollie_sleep((int)(sleep_time * HW_TIMERRING_CHECK_INTVAL));

		hwxcollie_end(id);

		pr_info("rxcollie: inject thread end\n");
	}

	return 0;
}

module_param(ovt_time, int, 0644);
module_param(sleep_time, int, 0644);

static int rxcollie_init(void)
{
	ras_debugset(1);

	test_loop = 0;

	hwxcollie_start = (replace_fun_start)kallsyms_lookup_name(
		"__hw_xcollie_start_ex");
	hwxcollie_end = (replace_fun_end)kallsyms_lookup_name(
		"hw_xcollie_end");

	rxcollie_task = kthread_run(rxcollie_thread_handle,
			NULL, "rxcollietask");

	return 0;
}
static void rxcollie_exit(void)
{
	(void)kthread_stop(rxcollie_task);
}

module_init(rxcollie_init);
module_exit(rxcollie_exit);
MODULE_DESCRIPTION("hwxcollie faults inject.");
MODULE_LICENSE("GPL");
MODULE_VERSION("V001R001C151-1.0");
