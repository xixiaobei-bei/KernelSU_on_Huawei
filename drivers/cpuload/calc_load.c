/*
 * Copyright (C) 2016 The Huawei Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/hisi/hisi_cpufreq_dt.h>

#include <linux/sysctl.h>
#include <linux/tick.h>
#include <linux/kernel_stat.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
#include <linux/cputime.h>
#endif

#include <linux/cpumask.h>
#include <linux/cpuset.h>

#include <cpu_netlink/cpu_netlink.h>
#include <log/log_usertype.h>
#include "../../kernel/sched/sched.h"

#define IF_PROC_LOAD   5

#define HIGH_LOAD_VALUE 98
#define CPU_LOAD_TIMER_RATE 5
#define CPU_HIGH_LOAD_THRESHOLD 4
#define CPU_LOW_LOAD_THRESHOLD 0
#define DETECT_PERIOD 1000000

#define CPU_LOAD_BIG_HIGHCYCLE   6
#define CPU_LOAD_BIG_LOWCYCLE    3

#define CPUS_PROC_DURING 4000000
#define CPUS_PROC_PERIOD 400000
#define PID_LIST_MAX  20
#define HIGHLOAD_MAX_PIDS      16
#define HIGHLOAD_MAX_TIDS      8

#define HIGH_LOAD_PERCENT       100
#define HIGH_LOAD_CPUSET_ROOT   95 // default 95
#define HIGH_LOAD_CPUSET_BG     95 // default 95

#define PERCENT_HUNDRED   100
#define NORMAL_LOAD_CPUSET_ROOT   60
#define NORMAL_LOAD_CPUSET_BG     60

#define PROC_STATIC_MAX          1024
#define PROC_STATIC_CPUSET_ROOT  4
#define PROC_STATIC_CPUSET_BG    5
#define PROC_STATIC_CLUSTER_BIG  5

#define MAX_LAST_RQCLOCK         5000000

#define USER_BETA_INIT 1024

enum {
	LOW_LOAD = 1,
	HIGH_LOAD = 2,
};

enum {
	ALL = 0,
	CPUSET_ROOT = 1,
	CPUSET_BG = 2,
	CLUSTER_BIG = 3,
	HIGHLOAD_MAX_TYPE
};

enum {
	LOAD_SWITCH_DEFAULT = 0,
	LOAD_SWITCH_ROOT = 1,
	LOAD_SWITCH_BG = 2,
	LOAD_SWITCH_BIGCORE = 3,
};

struct high_load_data {
	int cmd;
	int pid[HIGHLOAD_MAX_PIDS];
};

struct action_ctl {
	u64 bits_high;
	u64 bits_type;
};

static int check_intervals;
static int check_procstatic;
static int high_load_cnt[HIGHLOAD_MAX_TYPE] = { 0 };
static int normal_load_cnt[HIGHLOAD_MAX_TYPE] = { 0 };
static int last_status[HIGHLOAD_MAX_TYPE] = { 0 };
static int cycle_big_highcnt[HIGHLOAD_MAX_TYPE] = { 0 };
static int cycle_big_normalcnt[HIGHLOAD_MAX_TYPE] = { 0 };
static int cycle_big_cycles[HIGHLOAD_MAX_TYPE] = { 0 };

static long high_load_switch;
static struct delayed_work high_load_work;
static struct delayed_work cpus_procstatic_work;
static unsigned long cpumask_bg = 0x0000000c;
static unsigned long cpumask_root = 0x000000ff;
static unsigned long cpumask_big = 0x000000c0;

static struct cpufreq_policy *policy;
static unsigned int *freqs_weight;
static u64 *freqs_time;
static u64 *freqs_time_last;
static int freqs_len;
static unsigned int fg_freqs_threshold = 75;

static struct action_ctl action_ctl_bits = { 0 };

struct pidstat_node {
	struct rb_node node;
	pid_t key_pid;
	int count;
};

struct pidstat_mgr {
	int index_curr;
	int max_count;
	struct pidstat_node *head;
	struct rb_root rb_root;
	struct high_load_data data;
};

struct pidstat_mgr g_pidstat_mgt[HIGHLOAD_MAX_TYPE] = { 0 };

void cpuset_bg_cpumask(unsigned long bits)
{
	pr_info("cpuload:bg cpumask bits:0x%x", bits);
	cpumask_bg = bits;
}

static bool is_beta_user(void)
{
	static unsigned int user_type = USER_BETA_INIT;

	if (user_type == USER_BETA_INIT)
		user_type = get_logusertype_flag();

	return user_type == BETA_USER;
}


static inline void set_action_ctl(int type, bool high_load)
{
	action_ctl_bits.bits_type |= (1 << type);

	if (high_load)
		action_ctl_bits.bits_high |= (1 << type);
}

static void send_to_user_high(int type, int size, int *data)
{
	int i = 0;
	struct high_load_data high_data;

	memset(&high_data, 0, sizeof(high_data));
	high_data.cmd = (type << 1) - 1;

	for (i = 0; i < size; i++)
		high_data.pid[i] = data[i];

	send_to_user(IF_PROC_LOAD, sizeof(high_data) / sizeof(int),
		(int *)&high_data);
}

static void send_to_user_low(int type)
{
	struct high_load_data high_data;

	memset(&high_data, 0, sizeof(high_data));
	high_data.cmd = (type << 1);
	high_data.pid[0] = 0;
	send_to_user(IF_PROC_LOAD, sizeof(high_data) / sizeof(int),
		(int *)&high_data);
}

static int pidstat_init(void)
{
	int i = 0;

	for (i = 0; i < HIGHLOAD_MAX_TYPE; i++) {
		g_pidstat_mgt[i].index_curr = 0;
		g_pidstat_mgt[i].max_count =
			(CPUS_PROC_DURING / CPUS_PROC_PERIOD) * CONFIG_NR_CPUS;
		g_pidstat_mgt[i].head =
			kmalloc_array(g_pidstat_mgt[i].max_count,
				sizeof(*(g_pidstat_mgt[i].head)), GFP_KERNEL);

		if (g_pidstat_mgt[i].head == NULL)
			return -ENOMEM;

		g_pidstat_mgt[i].rb_root = RB_ROOT;
		memset(g_pidstat_mgt[i].head, 0,
			g_pidstat_mgt[i].max_count *
			sizeof(*(g_pidstat_mgt[i].head)));
		memset(&g_pidstat_mgt[i].data, 0,
			sizeof(g_pidstat_mgt[i].data));
	}

	pr_info("cpuload: pidstat init ok!");
	return 0;
}

static struct pidstat_node *pidstat_getnode(int type)
{
	if (g_pidstat_mgt[type].index_curr >=
		(g_pidstat_mgt[type].max_count - 1))
		return NULL;

	g_pidstat_mgt[type].index_curr++;
	return g_pidstat_mgt[type].head + g_pidstat_mgt[type].index_curr - 1;
}

static void pidstat_reset(int type)
{
	if (type >= HIGHLOAD_MAX_TYPE)
		return;

	memset(g_pidstat_mgt[type].head,
		0,
		g_pidstat_mgt[type].index_curr *
		sizeof(*(g_pidstat_mgt[type].head)));
	memset(&g_pidstat_mgt[type].data, 0, sizeof(g_pidstat_mgt[type].data));
	g_pidstat_mgt[type].index_curr = 0;
	g_pidstat_mgt[type].rb_root = RB_ROOT;
}

static void pidstat_clear(void)
{
	int i = 0;

	for (i = 0; i < HIGHLOAD_MAX_TYPE; i++) {
		if (g_pidstat_mgt[i].head != NULL)
			kfree(g_pidstat_mgt[i].head);

		memset(&g_pidstat_mgt[i], 0, sizeof(g_pidstat_mgt[i]));
	}
}

static void pidstat_search_insert(pid_t key_pid, int type)
{
	struct rb_node **curr = &(g_pidstat_mgt[type].rb_root.rb_node);
	struct rb_node *parent = NULL;
	struct pidstat_node *this;
	pid_t ret_pid;
	struct pidstat_node *new_node;

	while (*curr) {
		this = container_of(*curr, struct pidstat_node, node);

		parent = *curr;
		ret_pid = key_pid - this->key_pid;

		if (ret_pid < 0) {
			curr = &((*curr)->rb_left);
		} else if (ret_pid > 0) {
			curr = &((*curr)->rb_right);
		} else {
			this->count++;
			return;
		}
	}

	/* add new node and rebalance tree. */
	new_node = pidstat_getnode(type);

	if (new_node == NULL)
		return;

	new_node->key_pid = key_pid;
	new_node->count = 1;
	rb_link_node(&new_node->node, parent, curr);
	rb_insert_color(&new_node->node, &g_pidstat_mgt[type].rb_root);
}

int send_to_user_netlink(int data)
{
	int dt[] = { data };

	return send_to_user(CPU_HIGH_LOAD, 1, dt);
}

/*lint -save -e501 -e64 -e507 -e644 -e64 -e409*/
static u64 get_idle_time(int cpu)
{
	u64 idle, idle_time = -1ULL;

	if (cpu_online(cpu))
		idle_time = get_cpu_idle_time_us(cpu, NULL);

	if (idle_time == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.idle */
		idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		idle = idle_time * NSEC_PER_USEC;

#else
		idle = usecs_to_cputime64(idle_time);
#endif
	return idle;
}

static u64 get_iowait_time(int cpu)
{
	u64 iowait, iowait_time = -1ULL;

	if (cpu_online(cpu))
		iowait_time = get_cpu_iowait_time_us(cpu, NULL);

	if (iowait_time == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.iowait */
		iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		iowait = iowait_time * NSEC_PER_USEC;

#else
		iowait = usecs_to_cputime64(iowait_time);
#endif
	return iowait;
}

static u64 cpu_total_time[CONFIG_NR_CPUS];
static u64 cpu_busy_time[CONFIG_NR_CPUS];
static int cpu_online[CONFIG_NR_CPUS];

static void get_cpu_time(void)
{
	int i = 0;
	u64 user, nice, system, idle, iowait, irq, softirq, steal;
	u64 guest, guest_nice;

	memset(cpu_online, 0, sizeof(cpu_online));
	for_each_online_cpu(i) {
		user = kcpustat_cpu(i).cpustat[CPUTIME_USER];
		nice = kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		system = kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		idle = get_idle_time(i);
		iowait = get_iowait_time(i);
		irq = kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		softirq = kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
		steal = kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
		guest = kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
		guest_nice = kcpustat_cpu(i).cpustat[CPUTIME_GUEST_NICE];
		cpu_total_time[i] =
			user + nice + system + idle + iowait + irq + softirq +
			steal + guest + guest_nice;
		cpu_busy_time[i] = user + nice + system;
		cpu_online[i] = 1;
	}
}

/*
 *
 * This function recoreds the cpustat information
 */
static void get_cpu_load(u64 *total_time, u64 *busy_time,
			 unsigned long cpumask)
{
	int i = 0;

	for (i = 0; i < CONFIG_NR_CPUS; i++) {
		if (!cpu_online[i])
			continue;

		if (((1 << i) & cpumask) == 0)
			continue;

		*total_time += cpu_total_time[i];
		*busy_time += cpu_busy_time[i];
	}
}

/*lint -restore*/

/*
 * This function gets called by the timer code, with HZ frequency.
 * We call it with interrupts disabled.
 */

bool high_load_tick(void);
bool high_load_tick_mask(unsigned long bits, int type,
			 int threhold_up, int threhold_down);

static unsigned long get_mask_bytype(int type)
{
	switch (type) {
	case CPUSET_ROOT:
		return cpumask_root;

	case CPUSET_BG:
		return cpumask_bg;

	case CLUSTER_BIG:
		return cpumask_big;

	default:
		return 0;
	}
}

static int get_threhold_bytype(int type)
{
	switch (type) {
	case CPUSET_ROOT:
		return PROC_STATIC_CPUSET_ROOT;

	case CPUSET_BG:
		return PROC_STATIC_CPUSET_BG;

	case CLUSTER_BIG:
		return PROC_STATIC_CLUSTER_BIG;

	default:
		return PROC_STATIC_MAX;
	}
}

static void high_load_tickfn(struct work_struct *work);
static void cpus_procstatic_tickfn(struct work_struct *work);
static void high_freqs_load_tick(void);

static void high_load_count_reset(void)
{
	check_intervals = 0;
	high_load_cnt[ALL] = 0;
	high_load_cnt[CPUSET_BG] = 0;
	high_load_cnt[CPUSET_ROOT] = 0;
	high_load_cnt[CLUSTER_BIG] = 0;
	normal_load_cnt[CPUSET_BG] = 0;
	normal_load_cnt[CPUSET_ROOT] = 0;
	normal_load_cnt[CLUSTER_BIG] = 0;
}

static void high_load_tickfn(struct work_struct *work)
{
	++check_intervals;

	if (check_intervals >= CPU_LOAD_TIMER_RATE) {
		action_ctl_bits.bits_type = 0;
		action_ctl_bits.bits_high = 0;
	}

	get_cpu_time();

	if ((high_load_switch & (1 << LOAD_SWITCH_DEFAULT)) != 0) {
		if (high_load_tick())
			goto HIGHLOAD_END;
	}

	if (!is_beta_user())
		goto HIGHLOAD_END;

	if ((high_load_switch & (1 << LOAD_SWITCH_ROOT)) != 0)
		high_load_tick_mask(0xf0, CPUSET_ROOT,
			HIGH_LOAD_CPUSET_ROOT, NORMAL_LOAD_CPUSET_ROOT);
	if ((high_load_switch & (1 << LOAD_SWITCH_BG)) != 0)
		high_load_tick_mask(0x0c, CPUSET_BG,
			HIGH_LOAD_CPUSET_BG, NORMAL_LOAD_CPUSET_BG);

	if (check_intervals >= CPU_LOAD_TIMER_RATE) {
#ifdef CONFIG_HISI_FREQ_STATS_COUNTING_IDLE
		if (((high_load_switch & (1 << LOAD_SWITCH_BIGCORE)) != 0) &&
			is_beta_user())
			high_freqs_load_tick();
#endif
		if (action_ctl_bits.bits_type)
			schedule_delayed_work_on(0, &cpus_procstatic_work,
				usecs_to_jiffies(CPUS_PROC_PERIOD));
	}

HIGHLOAD_END:
	if (check_intervals >= CPU_LOAD_TIMER_RATE)
		high_load_count_reset();

	schedule_delayed_work(&high_load_work, usecs_to_jiffies(DETECT_PERIOD));
}

static pid_t threhold_pid_list[HIGHLOAD_MAX_TYPE][PID_LIST_MAX] = { 0 };
static int threhold_pid_size[HIGHLOAD_MAX_TYPE] = { 0 };

static void cpus_procstatic_low(void)
{
	int type;
	int i;

	for (type = 1; type < HIGHLOAD_MAX_TYPE; type++) {
		if (!(action_ctl_bits.bits_type & (1 << type)))
			continue;

		if (action_ctl_bits.bits_high & (1 << type))
			continue;

		pr_info("cpuload: delete size:%d type:%d",
			threhold_pid_size[type], type);
		send_to_user_low(type);
		for (i = 0; i < threhold_pid_size[type]; i++)
			threhold_pid_list[type][i] = 0;

		pidstat_reset(type);
		threhold_pid_size[type] = 0;
	}
}

static void cpus_procstatic_high(int type)
{
	int i;
	struct pidstat_node *curr;
	int index;

	for (i = 0, curr = g_pidstat_mgt[type].head;
		i < g_pidstat_mgt[type].index_curr;
		i++, curr++) {
		if (curr->count > get_threhold_bytype(type)) {
			pr_info("cpuload: key_pid:%d,count:%d,type:%d",
				curr->key_pid, curr->count, type);

			if (threhold_pid_size[type] < PID_LIST_MAX) {
				index = threhold_pid_size[type];
				threhold_pid_list[type][index]
					= curr->key_pid;
				threhold_pid_size[type]++;
			}
		}
	}

	pr_info("cpuload: threhold_pid_size:%d,type:%d",
		threhold_pid_size[type], type);
	if (threhold_pid_size[type])
		send_to_user_high(type, threhold_pid_size[type],
				  threhold_pid_list[type]);
	pidstat_reset(type);
	for (i = 0; i < threhold_pid_size[type]; i++)
		threhold_pid_list[type][i] = 0;
	threhold_pid_size[type] = 0;
}

static ssize_t enable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (buf == NULL) {
		pr_err("high_load_show buf is NULL");
		return -EINVAL;
	}

	return snprintf(buf, PAGE_SIZE, "%ld\n", high_load_switch);
}

static ssize_t enable_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	long value;
	int ret;

	if (buf == NULL) {
		pr_err("high_load_store buf is NULL");
		return -EINVAL;
	}

	ret = strict_strtol(buf, 10, &value);

	if (ret != 0 || value < 0)
		return -EINVAL;

	if (value != 0) {
		if (!high_load_switch)
			schedule_delayed_work(&high_load_work,
				usecs_to_jiffies(DETECT_PERIOD));
	} else {
		if (high_load_switch) {
			high_load_count_reset();
			cancel_delayed_work_sync(&high_load_work);
		}
	}

	high_load_switch = value;
	return count;
}

static ssize_t threshold_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (buf == NULL) {
		pr_err("threshold_show buf is NULL");
		return -EINVAL;
	}

	return snprintf(buf, PAGE_SIZE, "%ld\n", fg_freqs_threshold);
}

static ssize_t threshold_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	long value;
	int ret;

	if (buf == NULL) {
		pr_err("threshold_store buf is NULL");
		return -EINVAL;
	}

	ret = strict_strtol(buf, 10, &value);

	if (ret != 0 || value < 0 || value > 100)
		return -EINVAL;

	fg_freqs_threshold = value;
	return count;
}

static struct kobj_attribute high_load_attribute =
__ATTR(enable, 0600, enable_show, enable_store);
static struct kobj_attribute threshold_attribute =
__ATTR(threshold, 0600, threshold_show, threshold_store);
static struct kobject *high_load_kobj;

bool high_load_tick(void)
{
	bool ret = false;
	static int last_report_reason = LOW_LOAD;
	static u64 last_total_time;
	static u64 last_busy_time;
	u64 total_time = 0;
	u64 busy_time = 0;
	u64 total_delta_time;

	get_cpu_load(&total_time, &busy_time, 0xffffffff);
	total_delta_time = total_time - last_total_time;

	if (total_delta_time != 0) {
		if ((busy_time - last_busy_time) * HIGH_LOAD_PERCENT >=
			HIGH_LOAD_VALUE * total_delta_time)
			high_load_cnt[ALL]++;
	}

	if (check_intervals >= CPU_LOAD_TIMER_RATE) {
		if (high_load_cnt[ALL] >= CPU_HIGH_LOAD_THRESHOLD
			&& last_report_reason != HIGH_LOAD) {
			send_to_user_netlink(HIGH_LOAD);
			last_report_reason = HIGH_LOAD;
			ret = true;
			pr_info("cpuload: cpuload HIGH_LOAD!");
		} else if (high_load_cnt[ALL] == CPU_LOW_LOAD_THRESHOLD
			   && last_report_reason != LOW_LOAD) {
			send_to_user_netlink(LOW_LOAD);
			last_report_reason = LOW_LOAD;
			ret = true;
			pr_info("cpuload: cpuload LOW_LOAD!");
		}
	}

	last_total_time = total_time;
	last_busy_time = busy_time;
	return ret;
}

static void cycle_big_count_reset(int type)
{
	cycle_big_highcnt[type] = 0;
	cycle_big_normalcnt[type] = 0;
	cycle_big_cycles[type] = 0;
}

static bool cycle_big_highcycle(int type)
{
	bool ret = false;

	if (last_status[type] == HIGH_LOAD) {
		if (cycle_big_highcnt[type] >
			(CPU_LOAD_BIG_HIGHCYCLE * CPU_HIGH_LOAD_THRESHOLD)) {
			pr_info("cpuload: cycle big HIGH_LOAD!");
			set_action_ctl(type, true);
			ret = true;
		}
	}

	cycle_big_count_reset(type);
	return ret;
}

static bool cycle_big_lowcycle(int type)
{
	if (last_status[type] == HIGH_LOAD) {
		if (cycle_big_normalcnt[type] >
			(CPU_LOAD_BIG_LOWCYCLE * CPU_HIGH_LOAD_THRESHOLD)) {
			pr_info("cpuload: cycle big LOW_LOAD!");
			last_status[type] = LOW_LOAD;
			set_action_ctl(type, false);
			cycle_big_count_reset(type);
			return true;
		}
	}

	return false;
}

bool high_load_tick_mask(unsigned long bits, int type,
			 int threhold_up, int threhold_down)
{
	static u64 last_total_time[HIGHLOAD_MAX_TYPE] = { 0 };
	static u64 last_busy_time[HIGHLOAD_MAX_TYPE] = { 0 };
	u64 total_time = 0;
	u64 busy_time = 0;
	u64 total_delta_time;
	u64 busy_precent = 0;

	get_cpu_load(&total_time, &busy_time, bits);

	total_delta_time = total_time - last_total_time[type];
	if (total_delta_time != 0) {
		busy_precent =
			(busy_time - last_busy_time[type]) * HIGH_LOAD_PERCENT;
		if (busy_precent >= (threhold_up * total_delta_time)) {
			high_load_cnt[type]++;
			cycle_big_highcnt[type]++;
		}

		if (busy_precent < (threhold_down * total_delta_time)) {
			normal_load_cnt[type]++;
			cycle_big_normalcnt[type]++;
		}
	}

	last_total_time[type] = total_time;
	last_busy_time[type] = busy_time;

	if (check_intervals < CPU_LOAD_TIMER_RATE)
		return false;

	cycle_big_cycles[type]++;
	if (high_load_cnt[type] >= CPU_HIGH_LOAD_THRESHOLD &&
		last_status[type] != HIGH_LOAD) {
		pr_info("cpuload: cpuload mask HIGH_LOAD");
		last_status[type] = HIGH_LOAD;
		set_action_ctl(type, true);
		cycle_big_count_reset(type);
		return true;
	}

	if ((cycle_big_cycles[type] % CPU_LOAD_BIG_LOWCYCLE) == 0) {
		if (cycle_big_lowcycle(type))
			return true;
	}

	if (cycle_big_cycles[type] >= CPU_LOAD_BIG_HIGHCYCLE) {
		if (cycle_big_highcycle(type))
			return true;
	}

	return false;
}

static void high_freqs_load_tick(void)
{
	u64 delta_time;
	u64 delta_freqs_time;
	ktime_t now;
	static ktime_t last;
	int i;
	int ret_err;

	if ((!policy) || (!freqs_weight))
		return;

	now = ktime_get();
	delta_time = ktime_us_delta(now, last);
	last = now;

#ifdef CONFIG_HISI_FREQ_STATS_COUNTING_IDLE
	ret_err = hisi_time_in_freq_get(CONFIG_NR_CPUS - 1,
		freqs_time, freqs_len);

	if (ret_err)
		return;

	delta_freqs_time = 0;
	for (i = 0; i < freqs_len; i++) {
		if (freqs_weight[i] < fg_freqs_threshold)
			continue;
		delta_freqs_time += (freqs_time[i] - freqs_time_last[i]) *
			freqs_weight[i];
		freqs_time_last[i] = freqs_time[i];
	}

	if (delta_freqs_time > (delta_time * fg_freqs_threshold)) {
		pr_info("cpuload: high freqs load");
		set_action_ctl(CLUSTER_BIG, true);
		last_status[CLUSTER_BIG] = HIGH_LOAD;
	} else if (last_status[CLUSTER_BIG] != LOW_LOAD) {
		set_action_ctl(CLUSTER_BIG, false);
		last_status[CLUSTER_BIG] = LOW_LOAD;
	}
#endif
}

static void get_current_task_mask(int type)
{
	int cpu = 0;
	pid_t curr_pid;
	pid_t local_pid = current->pid;
	unsigned long cpumask = get_mask_bytype(type);

	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
		struct rq *rq_cur = NULL;
		struct task_struct *p = NULL;

		if (((1 << cpu) & cpumask) == 0)
			continue;

		rq_cur = cpu_rq(cpu);

		if (!rq_cur) {
			pr_info("cpuload: cpu:%d rq_cur is NULL!", cpu);
			continue;
		}

		p = rq_cur->curr;

		if (!p) {
			pr_info("cpuload: cpu:%d p is NULL!", cpu);
			continue;
		}

		get_task_struct(p);
		curr_pid = p->tgid;
		put_task_struct(p);

		if (curr_pid == 0)
			continue;

		if (curr_pid == local_pid)
			continue;

		if (curr_pid != 0)
			pidstat_search_insert(curr_pid, type);
	}
}

static void get_current_thread_mask(int type)
{
	int cpu = 0;
	pid_t curr_pid;
	pid_t local_pid = current->pid;
	unsigned long cpumask = get_mask_bytype(type);

	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
		struct rq *rq_cur = NULL;
		struct task_struct *p = NULL;

		if (((1 << cpu) & cpumask) == 0)
			continue;

		rq_cur = cpu_rq(cpu);

		if (!rq_cur) {
			pr_info("cpuload: cpu:%d rq_cur is NULL!", cpu);
			continue;
		}

		p = rq_cur->curr;

		if (!p) {
			pr_info("cpuload: cpu:%d p is NULL!", cpu);
			continue;
		}

		get_task_struct(p);
#ifdef CONFIG_HW_QOS_THREAD
		if (atomic_read(&p->thread_qos.dynamic_qos) ==
		    VALUE_QOS_CRITICAL) {
			put_task_struct(p);
			continue;
		}
#endif
		curr_pid = p->pid;
		put_task_struct(p);

		if (curr_pid == 0)
			continue;

		if (curr_pid == local_pid)
			continue;

		if (curr_pid != 0)
			pidstat_search_insert(curr_pid, type);
	}
}

static void cpus_procstatic_tickfn(struct work_struct *work)
{
	int type;
	// low load
	if (check_procstatic == 0)
		cpus_procstatic_low();

	check_procstatic += CPUS_PROC_PERIOD;

	// high load
	for (type = 1; type < HIGHLOAD_MAX_TYPE; type++) {
		if (!(action_ctl_bits.bits_type & (1 << type)))
			continue;

		if (!(action_ctl_bits.bits_high & (1 << type)))
			continue;

		// get thread or task statistic
		if (type == CLUSTER_BIG)
			get_current_thread_mask(type);
		else
			get_current_task_mask(type);

		if (check_procstatic >= CPUS_PROC_DURING)
			cpus_procstatic_high(type);
	}

	if (check_procstatic >= CPUS_PROC_DURING) {
		check_procstatic = 0;
	} else {
		schedule_delayed_work_on(0, &cpus_procstatic_work,
			usecs_to_jiffies(CPUS_PROC_PERIOD));
	}
}

static void init_freqs_data(const struct cpufreq_policy *policy)
{
	struct cpufreq_frequency_table *pos = NULL;
	unsigned int len = 0;
	unsigned int last;
	int i;
	int mem_size;

	cpufreq_for_each_valid_entry(pos, policy->freq_table)
		len++;

	if (len == 0)
		return;
	freqs_len = len;

	last = policy->freq_table[len - 1].frequency / PERCENT_HUNDRED;
	if (last == 0)
		return;

	// request memory for three arrays:
	// freqs_weight, freqs_time, freqs_time_last
	mem_size = len * (sizeof(unsigned int) + sizeof(u64) + sizeof(u64));
	freqs_weight = kzalloc(mem_size, GFP_KERNEL);
	if (!freqs_weight)
		return;
	freqs_time = (u64 *)(freqs_weight + len);
	freqs_time_last = freqs_time + len;

	i = 0;
	cpufreq_for_each_valid_entry(pos, policy->freq_table)
		freqs_weight[i++] = (pos->frequency / last) + 1;
	freqs_weight[len - 1] = PERCENT_HUNDRED;
}

static int __init calc_load_init(void)
{
	int ret = -1;
	int i = 0;

	check_intervals = 0;
	check_procstatic = 0;

	high_load_kobj = kobject_create_and_add("highload", kernel_kobj);

	if (!high_load_kobj)
		goto err_create_kobject;

	ret = sysfs_create_file(high_load_kobj, &high_load_attribute.attr);

	if (ret)
		goto err_create_sysfs;

	ret = sysfs_create_file(high_load_kobj, &threshold_attribute.attr);

	if (ret)
		goto err_create_threshold;

	ret = pidstat_init();

	if (ret != 0)
		goto err_create_pidstat;

	policy = cpufreq_cpu_get(CONFIG_NR_CPUS - 1);
	if (policy)
		init_freqs_data(policy);

	INIT_DEFERRABLE_WORK(&high_load_work, high_load_tickfn);
	INIT_DEFERRABLE_WORK(&cpus_procstatic_work, cpus_procstatic_tickfn);

	for (i = 0; i < HIGHLOAD_MAX_TYPE; i++)
		last_status[i] = LOW_LOAD;

	return 0;
err_create_pidstat:
	sysfs_remove_file(high_load_kobj, &threshold_attribute.attr);
err_create_threshold:
	sysfs_remove_file(high_load_kobj, &high_load_attribute.attr);
err_create_sysfs:
	kobject_put(high_load_kobj);
	high_load_kobj = NULL;
err_create_kobject:
	return ret;
}

static void __exit calc_load_exit(void)
{
	if (high_load_kobj) {
		sysfs_remove_file(high_load_kobj, &high_load_attribute.attr);
		sysfs_remove_file(high_load_kobj, &threshold_attribute.attr);
		kobject_put(high_load_kobj);
		high_load_kobj = NULL;
	}

	kfree(freqs_weight);
	freqs_weight = NULL;
	freqs_time = NULL;
	freqs_time_last = NULL;
	freqs_len = 0;
	pidstat_clear();
}

module_init(calc_load_init);
module_exit(calc_load_exit);
