

/* ---- includes ---- */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <uapi/linux/sched/types.h>
#include <hwbootfail/chipsets/common/bootfail_timer.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/core/boot_detector.h>

/* ---- local macroes ---- */
#define BOOT_TIMER_COUNT_STEP 5000
#define HUAWEI_VENDOR_FLAG "/eng"

/* ---- local prototypes  ---- */
struct boot_timer_info {
	unsigned long short_timer_last_start_time;
	unsigned int count_step_value;
	struct task_struct *boot_time_task;
	struct timer_list boot_timer;
	struct completion boot_complete;
	bool boot_timer_count_enable;
	bool boot_timer_count_task_should_run;
	u32 long_timer_timeout_value;
	u32 short_timer_timeout_value;
};

struct action_timer {
	struct list_head  act_list;
	char action_name[ACTION_NAME_LEN];
	u32 action_timer_timeout_value;
	u32 action_timer_timing_count;
	unsigned long last_start_time;
	pid_t action_pid;
	char action_comm[TASK_COMM_LEN];
};

/* ---- local variables ---- */
static struct boot_timer_info *timer_info;
static struct mutex boot_timer_mutex;
static LIST_HEAD(action_timer_active_list);
static LIST_HEAD(action_timer_pause_list);
static u32 short_timer_timing_count; /* unit: msec */

/* ---- local function prototypes ---- */
static void check_boot_time(struct boot_timer_info *ptimer_info);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void wakeup_boot_timer_count_task(unsigned long data);
#else
static void wakeup_boot_timer_count_task(struct timer_list *bf_timer);
#endif
static __ref int boot_timer_count_kthread(void *arg);
static void __free_action_timer_list(struct list_head *atlist);
static inline struct action_timer *__get_action_timer(const char *act_name,
	struct list_head *atlist);

/* ---- local function definitions ---- */
static void process_boot_timer_timeout(void)
{
	struct bootfail_proc_param *pparam = NULL;
	enum bootfail_errorcode errorcode;
	char *detail_info = "";

	/* for dump trace of init Process. */
	dump_init_log();

	pparam = vmalloc(sizeof(*pparam));
	if (pparam == NULL) {
		print_err("vmalloc failed!\n");
		return;
	}
	memset(pparam, 0, sizeof(*pparam));
	(void)get_boot_stage((enum boot_stage *)&pparam->binfo.stage);
	if (is_kernel_stage(pparam->binfo.stage)) {
		pparam->binfo.bootfail_errno = KERNEL_SYSTEM_FREEZE;
		detail_info = "Boot timeout in kernel stage";
	} else if (is_native_stage(pparam->binfo.stage)) {
		pparam->binfo.bootfail_errno = NATIVE_SYSTEM_FREEZE;
		detail_info = "Boot timeout in native stage";
	} else if (is_framework_stage(pparam->binfo.stage)) {
		pparam->binfo.bootfail_errno = FRAMEWORK_SYSTEM_FREEZE;
		detail_info = "Boot timeout in framework stage";
	} else {
		print_err("Invalid boot stage: 0x%08x when "
			"boot timer timeout!\n", pparam->binfo.stage);
		vfree(pparam);
		return;
	}
	strncpy(pparam->detail_info, detail_info,
		min(sizeof(pparam->detail_info) - 1, strlen(detail_info))); //lint !e666
	pparam->binfo.rtc_time = get_sys_rtc_time();
	pparam->binfo.bootup_time = get_bootup_time();
	get_data_part_info(pparam);
	errorcode = boot_fail_error(pparam);
	if (errorcode != BF_OK)
		print_err("boot_fail_error fail:%d!\n", errorcode);
	vfree(pparam);
}

static void process_action_timer_timeout(const char *timer_name)
{
	struct bootfail_proc_param *pparam = NULL;
	enum bootfail_errorcode errorcode;

	if (unlikely(timer_name == NULL))
		return;

	/* for dump trace of init Process. */
	dump_init_log();

	pparam = vmalloc(sizeof(*pparam));
	if (pparam == NULL) {
		print_err("vmalloc failed!\n");
		return;
	}
	memset(pparam, 0, sizeof(*pparam));
	(void)get_boot_stage((enum boot_stage *)&pparam->binfo.stage);
	if (is_native_stage(pparam->binfo.stage)) {
		pparam->binfo.bootfail_errno = NATIVE_SERVICE_FREEZE;
	} else if (is_framework_stage(pparam->binfo.stage)) {
		pparam->binfo.bootfail_errno = FRAMEWORK_SERVICE_FREEZE;
	} else {
		print_err("Invalid boot stage: 0x%08x when "
			"action timer timeout!\n", pparam->binfo.stage);
		vfree(pparam);
		return;
	}
	strncpy(pparam->detail_info, timer_name,
		min(sizeof(pparam->detail_info) - 1, strlen(timer_name))); //lint !e666
	pparam->binfo.rtc_time = get_sys_rtc_time();
	pparam->binfo.bootup_time = get_bootup_time();
	get_data_part_info(pparam);
	errorcode = boot_fail_error(pparam);
	if (errorcode != BF_OK)
		print_err("boot_fail_error fail:%d!\n", errorcode);
	vfree(pparam);
}

static void check_timer_expired(struct action_timer  *atimer,
		struct boot_timer_info *ptimer_info)
{
	if (!atimer || !ptimer_info)
		return;

	atimer->action_timer_timing_count += jiffies_to_msecs(jiffies -
		atimer->last_start_time);
	atimer->last_start_time = jiffies;

	if (atimer->action_timer_timing_count >=
		atimer->action_timer_timeout_value) {
		print_info("hwboot_timer: %s expired! boot fail\n",
			atimer->action_name);
		ptimer_info->boot_timer_count_task_should_run = false;
		mutex_unlock(&boot_timer_mutex);
		if (is_dir_existed(HUAWEI_VENDOR_FLAG))
			process_action_timer_timeout(atimer->action_name);
	}
}

static void check_boot_time(struct boot_timer_info *ptimer_info)
{
	static u32 long_timer_timing_count;
	struct action_timer *atimer = NULL;
	u32 short_value = ptimer_info->short_timer_timeout_value;
	u32 long_value = ptimer_info->long_timer_timeout_value;

	long_timer_timing_count += ptimer_info->count_step_value;
	if (ptimer_info->boot_timer_count_enable) {
		unsigned long jif_time = jiffies -
			ptimer_info->short_timer_last_start_time;
		print_info("hwboot_timer:update timer [long and short]\n");
		short_timer_timing_count += jiffies_to_msecs(jif_time);
		ptimer_info->short_timer_last_start_time = jiffies;
	} else {
		print_info("hwboot_timer:update timer [long]\n");
	}

	if ((short_timer_timing_count >= short_value) ||
		(long_timer_timing_count >= long_value)) {
		print_info("hwboot_timer: %u minutes timer expired! boot fail!\n",
				(long_timer_timing_count >= long_value) ?
				(long_value / (unsigned int)(1000 * 60)) :
				(short_value / (unsigned int)(1000 * 60)));
		ptimer_info->boot_timer_count_task_should_run = false;
		mutex_unlock(&boot_timer_mutex);
		if (is_dir_existed(HUAWEI_VENDOR_FLAG))
			process_boot_timer_timeout();
		else
			set_boot_stage(BOOT_SUCC_STAGE);
	}

	list_for_each_entry(atimer, &action_timer_active_list, act_list) {
		print_info("hwboot_timer:update timer [%s]!\n",
			atimer->action_name);
		check_timer_expired(atimer, ptimer_info);
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void wakeup_boot_timer_count_task(unsigned long data)
{
	struct boot_timer_info *ptimer_info =
		(struct boot_timer_info *)(uintptr_t)data;

	complete(&ptimer_info->boot_complete);
}
#else
static void wakeup_boot_timer_count_task(struct timer_list *bf_timer)
{
	struct boot_timer_info *ptimer_info = NULL;
	if (bf_timer == NULL) {
		print_err("please attention bf_timer err\n");
		return;
	}
	ptimer_info = container_of(bf_timer, struct boot_timer_info, boot_timer);
	complete(&ptimer_info->boot_complete);
}
#endif

static __ref int boot_timer_count_kthread(void *arg)
{
	struct boot_timer_info *ptimer_info = arg;
	unsigned long delay_time = 0;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	if (!arg)
		return 0;

	sched_setscheduler(current, SCHED_FIFO, &param);
	while (!kthread_should_stop()) {
		while (wait_for_completion_interruptible(
				&ptimer_info->boot_complete) != 0)
			;
		reinit_completion(&ptimer_info->boot_complete);
		mutex_lock(&boot_timer_mutex);
		delay_time = msecs_to_jiffies(ptimer_info->count_step_value);
		if (ptimer_info->boot_timer_count_task_should_run) {
			check_boot_time(ptimer_info);
			mod_timer(&ptimer_info->boot_timer, jiffies + delay_time);
		} else {
			print_info("hwboot_timer: time thread will stop now:[boot_short_timer:%d]!\n",
				short_timer_timing_count);
			__free_action_timer_list(&action_timer_active_list);
			__free_action_timer_list(&action_timer_pause_list);
			timer_info = NULL;
			del_timer_sync(&ptimer_info->boot_timer);
			vfree(ptimer_info);
			mutex_unlock(&boot_timer_mutex);
			break;
		}
		mutex_unlock(&boot_timer_mutex);
	}

	return 0;
}

/*
 * function: int get_boot_timer_state(int *state)
 * brief: get state of the boot timer.
 *
 * param: state [out], the state of the boot timer.
 *
 * return: 0 - success, -1 - failed.
 *
 * note:
 *   1. this fuction only need be initialized in kernel.
 *   2. if *state == 0, boot timer disabled, if *state == 1, boot timer enbaled
 */
int get_boot_timer_state(int *state)
{
	int ret = -1;

	if (unlikely(!state))
		return -1;

	mutex_lock(&boot_timer_mutex);
	if (likely(timer_info)) {
		*state = (timer_info->boot_timer_count_enable) ? 1 : 0;
		ret = 0;
	} else {
		*state = 0;
	}
	mutex_unlock(&boot_timer_mutex);

	return ret;
}

/*
 * function: int suspend_boot_timer(void)
 * brief: suspend the boot timer.
 *
 * param: none.
 *
 * return: 0 - success, -1 - failed
 *
 * note:
 */
int suspend_boot_timer(void)
{
	int ret = -1;

	print_info("disable short timer!\n");
	mutex_lock(&boot_timer_mutex);
	if (likely(timer_info) &&
		timer_info->boot_timer_count_enable) {
		unsigned long short_time =
			timer_info->short_timer_last_start_time;

		timer_info->boot_timer_count_enable = false;
		short_timer_timing_count +=
			jiffies_to_msecs(jiffies - short_time);
		ret = 0;
	}
	mutex_unlock(&boot_timer_mutex);

	return ret;
}

/*
 * function: int resume_boot_timer(void)
 * brief: resume the boot timer.
 *
 * param: none.
 *
 * return: 0 - success, -1 - failed.
 *
 * note:
 */
int resume_boot_timer(void)
{
	int ret = -1;

	print_info("enable short timer!\n");
	mutex_lock(&boot_timer_mutex);
	if (likely(timer_info) && !timer_info->boot_timer_count_enable) {
		timer_info->boot_timer_count_enable = true;
		timer_info->short_timer_last_start_time = jiffies;
		ret = 0;
	}
	mutex_unlock(&boot_timer_mutex);

	return ret;
}

static void __free_action_timer_list(struct list_head *atlist)
{
	struct action_timer  *atimer = NULL;
	struct list_head *pos = NULL;
	struct list_head *next = NULL;

	if (!atlist)
		return;

	list_for_each_safe(pos, next, atlist) {
		atimer = list_entry(pos, struct action_timer, act_list);
		print_info("WARNING:[%s:%d]%s act_timer in list(activetime:%d ms, timeout:%d ms)\n",
			atimer->action_comm,
			atimer->action_pid,
			atimer->action_name,
			atimer->action_timer_timing_count,
			atimer->action_timer_timeout_value);
		list_del(&atimer->act_list);
		vfree(atimer);
	}
}

static struct action_timer *__get_action_timer(const char *act_name,
	struct list_head *atlist)
{
	struct action_timer  *atimer = NULL;

	if (!act_name || !atlist)
		return NULL;

	list_for_each_entry(atimer, atlist, act_list) {
		if (!strncmp(act_name, atimer->action_name,
				sizeof(atimer->action_name)))
			return atimer;
	}
	return NULL;
}

/*
 * function: int action_timer_start(char *act_name,
 *                           unsigned int timeout_value)
 * brief:
 *   1. creat a new action timer and start the timer
 *   2. stop the short boot timer
 *
 * param: none.
 *
 * return: 0 - success, -1 - failed.
 *
 * note:
 */
int action_timer_start(char *act_name, unsigned int timeout_value)
{
	int ret = -1;
	struct action_timer *atimer = NULL;
	char task_comm_tmp[TASK_COMM_LEN] = {0};
	pid_t task_pid_tmp;

	if (!act_name) {
		print_invalid_params("act_name.\n");
		return ret;
	}

	if (timeout_value > get_action_timer_timeout_value()) {
		timeout_value = get_action_timer_timeout_value();
		print_invalid_params("!!WARN timeout value [%u] too large!\n",
			timeout_value);
	}

	/* get current task info */
	get_task_comm(task_comm_tmp, current); //lint !e514
	task_pid_tmp = task_pid_nr(current);
	print_info("[%s:%d]%s timer start-->\n",
		task_comm_tmp, task_pid_tmp, act_name);
	mutex_lock(&boot_timer_mutex);

	/* 0.check if the timer already start */
	atimer = __get_action_timer(act_name, &action_timer_active_list);
	if (!atimer)
		atimer = __get_action_timer(act_name, &action_timer_pause_list);
	if (atimer) {
		print_info("[%s:%d]%s timer already start!\n",
			atimer->action_comm,
			atimer->action_pid, act_name);
		goto __out;
	}

	/* 1.init & start action timer */
	atimer = vmalloc(sizeof(*atimer));
	if (!atimer)
		goto __out;

	(void)memset(atimer, 0, sizeof(*atimer));
	strncpy(atimer->action_name, act_name,
		min(sizeof(atimer->action_name) - 1,
		strlen(act_name))); //lint !e666
	atimer->action_name[ACTION_NAME_LEN - 1] = '\0';
	atimer->action_timer_timeout_value = timeout_value;
	atimer->last_start_time = jiffies;
	atimer->action_timer_timing_count = 0;
	atimer->action_pid = task_pid_tmp;
	strncpy(atimer->action_comm, task_comm_tmp,
		min(sizeof(atimer->action_comm) - 1,
		strlen(task_comm_tmp))); //lint !e666
	atimer->action_comm[TASK_COMM_LEN - 1] = '\0';
	list_add_tail(&atimer->act_list, &action_timer_active_list);

	/* 2.suspend short boot timer */
	if (likely(timer_info) &&
		timer_info->boot_timer_count_enable) {
		unsigned long short_time =
			timer_info->short_timer_last_start_time;
		/* stop short boot timer */
		timer_info->boot_timer_count_enable = false;
		short_timer_timing_count +=
			jiffies_to_msecs(jiffies - short_time);
		ret = 0;
	}

__out:
	mutex_unlock(&boot_timer_mutex);
	return ret;
}

static void set_short_time(void)
{
	if (list_empty(&action_timer_active_list) &&
		!timer_info->boot_timer_count_enable) {
		/* resume short boot timer */
		timer_info->boot_timer_count_enable = true;
		timer_info->short_timer_last_start_time = jiffies;
	}
}

/*
 * function: int action_timer_stop(char *act_name)
 * brief:
 *   1. stop & destroy the action timer
 *   2. resume the short boot timer if the action timer list is empty
 *
 * param: none.
 *
 * return: 0 - success, -1 - failed.
 *
 * note:
 */
int action_timer_stop(char *act_name)
{
	struct action_timer  *atimer = NULL;
	int ret = -1;
	char task_comm_tmp[TASK_COMM_LEN] = {0};
	pid_t task_pid_tmp;

	if (!act_name) {
		print_invalid_params("act_name.\n");
		return ret;
	}

	/* get current task info */
	get_task_comm(task_comm_tmp, current); //lint !e514
	task_pid_tmp = task_pid_nr(current);

	print_info("[%s:%d]%s timer stop-->\n",
			task_comm_tmp, task_pid_tmp, act_name);
	mutex_lock(&boot_timer_mutex);

	atimer = __get_action_timer(act_name, &action_timer_active_list);
	if (!atimer)
		atimer = __get_action_timer(act_name,
			&action_timer_pause_list);

	if (atimer != NULL) {
		print_info("have find out \"%s\" timer,so stop it!(activetime:%u ms, timeout:%u ms)\n",
			act_name,
			atimer->action_timer_timing_count,
			atimer->action_timer_timeout_value);
		if (atimer->action_pid != task_pid_tmp)
			print_err("W: pid mismatch stop[%s:%d]: start[%s:%d]",
				task_comm_tmp, task_pid_tmp,
				atimer->action_comm,
				atimer->action_pid);
		list_del(&atimer->act_list);
		vfree(atimer);
	}

	/* resume the short boot timer if needed */
	if (likely(timer_info))	{
		set_short_time();
		ret = 0;
	}
	mutex_unlock(&boot_timer_mutex);

	return ret;
}

int action_timer_pause(char *act_name)
{
	struct action_timer  *atimer = NULL;
	int ret = -1;
	char task_comm_tmp[TASK_COMM_LEN];
	pid_t task_pid_tmp;

	if (!act_name) {
		print_invalid_params("act_name.\n");
		return ret;
	}

	/* get current task info */
	get_task_comm(task_comm_tmp, current); //lint !e514
	task_pid_tmp = task_pid_nr(current);
	print_info("[%s:%d]%s timer pause-->\n",
		task_comm_tmp, task_pid_tmp, act_name);
	mutex_lock(&boot_timer_mutex);
	atimer = __get_action_timer(act_name, &action_timer_active_list);
	if (!atimer) {
		print_info("\"%s\" timer is not in running quene!\n",
				act_name);
		goto __out;
	}
	print_info("\"%s\" timer is running, so pause it!\n", act_name);

	if (atimer->action_pid != task_pid_tmp)
		print_err("WARNING: pid mismatch pause[%s:%d]: start[%s:%d]!!",
			task_comm_tmp, task_pid_tmp,
			atimer->action_comm,
			atimer->action_pid);

	/* pasue the action timer at2 */
	atimer->action_timer_timing_count +=
			jiffies_to_msecs(jiffies - atimer->last_start_time);
	list_move_tail(&atimer->act_list, &action_timer_pause_list);
	if (likely(timer_info)) {
		set_short_time();
		ret = 0;
	}

__out:
	mutex_unlock(&boot_timer_mutex);
	return ret;
}

int action_timer_resume(char *act_name)
{
	struct action_timer  *atimer = NULL;
	int ret = -1;
	char task_comm_tmp[TASK_COMM_LEN] = {0};
	pid_t task_pid_tmp;

	if (!act_name) {
		print_invalid_params("act_name.\n");
		return ret;
	}

	/* get current task info */
	get_task_comm(task_comm_tmp, current); //lint !e514
	task_pid_tmp = task_pid_nr(current);

	print_info("[%s:%d]%s timer resume-->\n",
			task_comm_tmp, task_pid_tmp, act_name);
	mutex_lock(&boot_timer_mutex);
	atimer = __get_action_timer(act_name, &action_timer_pause_list);
	if (!atimer) {
		print_info("\"%s\" timer not in pause quene\n", act_name);
		goto __out;
	}
	print_info("\"%s\" timer is paused, resume it\n", act_name);
	if (atimer->action_pid != task_pid_tmp)
		print_err("W: pid mismatch resume[%s:%d]: start[%s:%d]!!",
			task_comm_tmp, task_pid_tmp,
			atimer->action_comm, atimer->action_pid);

	/* resume the action timer */
	atimer->last_start_time = jiffies;
	list_move_tail(&atimer->act_list, &action_timer_active_list);
	if (likely(timer_info)) {
		if (!list_empty(&action_timer_active_list) &&
			timer_info->boot_timer_count_enable) {
			unsigned long short_time =
				timer_info->short_timer_last_start_time;

			timer_info->boot_timer_count_enable = false;
			short_timer_timing_count +=
				jiffies_to_msecs(jiffies - short_time);
		}
		ret = 0;
	}

__out:
	mutex_unlock(&boot_timer_mutex);
	return ret;
}

/*
 * function: int set_boot_timer_timeout_value(unsigned int timeout_value)
 * brief: set timeout value of the boot timer.
 *
 * param: timeout_value [in], timeout value to be set, unit:msec.
 *
 * return: 0 - success, -1 - failed.
 *
 * note:
 *   1. this fuction only need be initialized in kernel.
 */
int set_boot_timer_timeout_value(unsigned int timeout_value)
{
	int ret = -1;

	print_info("update timeout value to: %d!\n", timeout_value);
	mutex_lock(&boot_timer_mutex);
	if (likely(timer_info)) {
		timer_info->short_timer_timeout_value = timeout_value;
		ret = 0;
	}
	mutex_unlock(&boot_timer_mutex);

	return ret;
}

/*
 * function: int get_boot_timer_timeout_value(unsigned int *ptimeout_value)
 * brief: get timeout value of the boot timer.
 *
 * param: ptimeout_value [out], timeout value of the timer, unit:msec.
 *
 * return: 0 - success, -1 - failed.
 *
 * note:
 *   1. this fuction only need be initialized in kernel.
 */
int get_boot_timer_timeout_value(unsigned int *ptimeout_value)
{
	int ret = -1;

	if (unlikely(!ptimeout_value))
		return -1;

	mutex_lock(&boot_timer_mutex);
	if (likely(timer_info)) {
		*ptimeout_value = timer_info->short_timer_timeout_value;
		ret = 0;
	}
	mutex_unlock(&boot_timer_mutex);

	return ret;
}

unsigned int get_elapsed_time(void)
{
	return short_timer_timing_count / 1000;
}

/*
 * function: int stop_boot_timer(void)
 * brief: stop the boot timer.
 *
 * param: none.
 *
 * return: 0 - success, -1 - failed.
 *
 * note: this fuction only need be initialized in kernel.
 */
int stop_boot_timer(void)
{
	int ret = -1;

	print_info("stop timer!\n");
	mutex_lock(&boot_timer_mutex);
	if (likely(timer_info)) {
		timer_info->boot_timer_count_task_should_run = false;
		ret = 0;
	}
	mutex_unlock(&boot_timer_mutex);

	return ret;
}

/*
 * function: int start_boot_timer(void)
 * brief: init the boot timer
 *
 * param: none.
 *
 * return: 0 - success, -1 - failed.
 *
 * note: this fuction only need be initialized in kernel.
 */
int start_boot_timer(void)
{
	int ret = 0;
	unsigned long delay_time = 0;
	struct boot_timer_info *ptimer_info = NULL;

	ptimer_info = vmalloc(sizeof(*ptimer_info));
	if (!ptimer_info) {
		print_err("vmalloc failed!\n");
		return -EIO;
	}

	timer_info = ptimer_info;
	ptimer_info->boot_time_task =
		kthread_create(boot_timer_count_kthread,
			ptimer_info, "hwboot_time");
	if (IS_ERR(ptimer_info->boot_time_task)) {
		ret = PTR_ERR(ptimer_info->boot_time_task);
		goto err;
	}

	ptimer_info->count_step_value = BOOT_TIMER_COUNT_STEP;
	ptimer_info->long_timer_timeout_value =
		get_long_timer_timeout_value();
	ptimer_info->short_timer_timeout_value =
		get_short_timer_timeout_value();
	ptimer_info->boot_timer_count_enable = true;
	ptimer_info->boot_timer_count_task_should_run = true;
	mutex_init(&boot_timer_mutex);
	delay_time = msecs_to_jiffies(ptimer_info->count_step_value);
	init_completion(&ptimer_info->boot_complete);
	wake_up_process(ptimer_info->boot_time_task);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	init_timer(&ptimer_info->boot_timer);
	ptimer_info->boot_timer.data =
		(unsigned long)(uintptr_t)ptimer_info;
	ptimer_info->boot_timer.function =
		wakeup_boot_timer_count_task;
	ptimer_info->boot_timer.expires = jiffies + delay_time;
	ptimer_info->short_timer_last_start_time = jiffies;
#else
	timer_setup(&ptimer_info->boot_timer, wakeup_boot_timer_count_task, 0);
	ptimer_info->boot_timer.expires = jiffies + delay_time;
	ptimer_info->short_timer_last_start_time = jiffies;
#endif
	add_timer(&ptimer_info->boot_timer);
	print_info("boot timer start!\n");
	return 0;

err:
	vfree(ptimer_info);
	return ret;
}
