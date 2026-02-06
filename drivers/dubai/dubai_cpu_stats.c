#include "dubai_cpu_stats.h"

#include <linux/init.h>
#include <linux/mm.h>
#include <linux/net.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/sched/stat.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/sched/signal.h>
#endif

#include <chipset_common/dubai/dubai_ioctl.h>

#include "cpu_stats/dubai_cpu_stats_common.h"
#include "utils/buffered_log_sender.h"
#include "utils/dubai_hashtable.h"
#include "utils/dubai_utils.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
typedef u64 dubai_cputime_t;
#else
typedef cputime_t dubai_cputime_t;
#endif

#define IOC_PROC_CPUTIME_ENABLE DUBAI_CPU_DIR_IOC(W, 1, bool)
#define IOC_PROC_CPUTIME_REQUEST DUBAI_CPU_DIR_IOC(W, 2, long long)
#define IOC_TASK_CPUPOWER_SUPPORT DUBAI_CPU_DIR_IOC(R, 3, bool)
#define IOC_PROC_DECOMPOSE_SET DUBAI_CPU_DIR_IOC(W, 4, struct dev_transmit_t)
#define IOC_CPU_CONFIGS_GET DUBAI_CPU_DIR_IOC(R, 5, struct dubai_cpu_config_info *)

#define PROC_HASH_BITS			15
#define THREAD_HASH_BITS		10
#define BASE_COUNT				1500
#define MAX_PROC_AVAIL_COUNT	1024
#define MAX_THREAD_AVAIL_COUNT	2048
#define NAME_LEN				(PREFIX_LEN + TASK_COMM_LEN)
#define RCU_LOCK_BREAK_TIMEOUT	(HZ / 10)

#define dubai_check_entry_state(entry, expected)	((entry)->state == (expected))
#define dubai_check_update_entry_state(entry, expected, updated) \
	({ \
		bool ret = dubai_check_entry_state(entry, expected); \
		if (!ret) \
			(entry)->state = (updated); \
		ret; \
	})

enum {
	CMDLINE_NEED_TO_GET = 0,
	CMDLINE_PROCESS,
	CMDLINE_THREAD,
};

enum {
	PROCESS_STATE_DEAD = 0,
	PROCESS_STATE_ACTIVE,
};

enum {
	DUBAI_STATE_DEAD,
	DUBAI_STATE_ALIVE,
	DUBAI_STATE_UNSETTLED,
};

struct dubai_cputime {
	uid_t uid;
	pid_t pid;
	unsigned long long time;
	unsigned long long power;
	unsigned char cmdline;
	char name[NAME_LEN];
} __packed;

struct dubai_thread_entry {
	pid_t pid;
	dubai_cputime_t time;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	unsigned long long power;
#endif
	int8_t state;
	char name[NAME_LEN];
	struct hlist_node hash;
};

struct dubai_proc_entry {
	pid_t tgid;
	uid_t uid;
	dubai_cputime_t dead_time;
	dubai_cputime_t active_time;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	unsigned long long dead_power;
	unsigned long long active_power;
#endif
	int8_t state;
	bool decomposed;
	bool cmdline;
	char name[NAME_LEN];
	struct dubai_dynamic_hashtable *threads;
	struct hlist_node hash;
};

struct dubai_cputime_transmit {
	long long timestamp;
	int type;
	int count;
	unsigned char value[0];
} __packed;

static int dubai_proc_cputime_enable(void __user *argp);
static int dubai_get_task_cpupower_enable(void __user *argp);
static int dubai_get_proc_cputime(void __user *argp);

static DECLARE_DUBAI_HASHTABLE(proc_hash_table, PROC_HASH_BITS);
static DEFINE_MUTEX(dubai_proc_lock);
static atomic_t proc_cputime_enable;
static atomic_t dead_count;
static atomic_t active_count;
static pid_t dubaid_tgid;
static int proc_entry_avail_cnt;
static int thread_entry_avail_cnt;
static struct dubai_proc_entry *proc_entry_avail_pool[MAX_PROC_AVAIL_COUNT];
static struct dubai_thread_entry *thread_entry_avail_pool[MAX_THREAD_AVAIL_COUNT];
static bool update_active_succ = true;

#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
static const atomic_t task_power_enable = ATOMIC_INIT(1);
#else
static const atomic_t task_power_enable = ATOMIC_INIT(0);
#endif

long dubai_ioctl_cpu(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_PROC_CPUTIME_ENABLE:
		rc = dubai_proc_cputime_enable(argp);
		break;
	case IOC_PROC_CPUTIME_REQUEST:
		rc = dubai_get_proc_cputime(argp);
		break;
	case IOC_TASK_CPUPOWER_SUPPORT:
		rc = dubai_get_task_cpupower_enable(argp);
		break;
	case IOC_PROC_DECOMPOSE_SET:
		rc = dubai_set_proc_decompose(argp);
		break;
	case IOC_CPU_CONFIGS_GET:
		rc = dubai_get_cpu_configs(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static inline unsigned long long dubai_cputime_to_msecs(dubai_cputime_t time)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	return ((unsigned long long)ktime_to_ms(time));
#else
	return ((unsigned long long)jiffies_to_msecs(cputime_to_jiffies(time)));
#endif
}

static inline bool dubai_is_proc_entry_decomposed(struct dubai_proc_entry *entry)
{
	return (entry->decomposed && entry->threads);
}

static void dubai_copy_name(char *to, const char *from, size_t len)
{
	char *p = NULL;

	if (strlen(from) <= len) {
		strncpy(to, from, len);
		return;
	}

	p = strrchr(from, '/');
	if (p != NULL && (*(p + 1) != '\0'))
		strncpy(to, p + 1, len);
	else
		strncpy(to, from, len);
}

// Create log entry to store cpu stats
static struct buffered_log_entry *dubai_create_log_entry(long long timestamp, int count, int type)
{
	long long size = 0;
	struct buffered_log_entry *entry = NULL;
	struct dubai_cputime_transmit *transmit = NULL;

	if (unlikely(count < 0)) {
		dubai_err("Invalid parameter, count=%d", count);
		return NULL;
	}
	/*
	 * allocate more space(BASE_COUNT)
	 * size = dubai_cputime_transmit size + dubai_cputime size * count
	 */
	count += BASE_COUNT;
	size = cal_log_entry_len(struct dubai_cputime_transmit, struct dubai_cputime, count);
	entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_PROC_CPUTIME);
	if (entry == NULL) {
		dubai_err("Failed to create buffered log entry");
		return NULL;
	}

	transmit = (struct dubai_cputime_transmit *)(entry->data);
	transmit->timestamp = timestamp;
	transmit->type = type;
	transmit->count = count;

	return entry;
}

void dubai_send_log_entry(struct buffered_log_entry *entry)
{
	int ret;
	struct dubai_cputime_transmit *transmit = NULL;

	transmit = (struct dubai_cputime_transmit *)(entry->data);
	entry->length = cal_log_entry_len(struct dubai_cputime_transmit, struct dubai_cputime, transmit->count);
	ret = send_buffered_log(entry);
	if (ret < 0)
		dubai_err("Failed to send process log entry, type: %d", transmit->type);
}

static void dubai_proc_entry_copy(unsigned char *value, const struct dubai_proc_entry *entry)
{
	struct dubai_cputime stat;

	memset(&stat, 0, sizeof(struct dubai_cputime));
	stat.uid = entry->uid;
	stat.pid = entry->tgid;
	stat.time = dubai_cputime_to_msecs(entry->active_time + entry->dead_time);
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	stat.power = entry->active_power + entry->dead_power;
#endif
	stat.cmdline = entry->cmdline ? CMDLINE_PROCESS : CMDLINE_NEED_TO_GET;
	dubai_copy_name(stat.name, entry->name, NAME_LEN - 1);

	memcpy(value, &stat, sizeof(struct dubai_cputime));
}

static void dubai_thread_entry_copy(unsigned char *value, uid_t uid, const struct dubai_thread_entry *thread)
{
	struct dubai_cputime stat;

	memset(&stat, 0, sizeof(struct dubai_cputime));
	stat.uid = uid;
	stat.pid = thread->pid;
	stat.time = dubai_cputime_to_msecs(thread->time);
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	stat.power = thread->power;
#endif
	stat.cmdline = CMDLINE_THREAD;
	dubai_copy_name(stat.name, thread->name, NAME_LEN - 1);

	memcpy(value, &stat, sizeof(struct dubai_cputime));
}

static struct dubai_proc_entry *dubai_get_avail_proc_entry(void)
{
	struct dubai_proc_entry *entry = NULL;

	if ((proc_entry_avail_cnt > 0) && (proc_entry_avail_cnt <= MAX_PROC_AVAIL_COUNT)) {
		entry = proc_entry_avail_pool[proc_entry_avail_cnt - 1];
		proc_entry_avail_pool[proc_entry_avail_cnt - 1] = NULL;
		proc_entry_avail_cnt--;
	}

	return entry;
}

static struct dubai_thread_entry *dubai_get_avail_thread_entry(void)
{
	struct dubai_thread_entry *entry = NULL;

	if ((thread_entry_avail_cnt > 0) && (thread_entry_avail_cnt <= MAX_THREAD_AVAIL_COUNT)) {
		entry = thread_entry_avail_pool[thread_entry_avail_cnt - 1];
		thread_entry_avail_pool[thread_entry_avail_cnt - 1] = NULL;
		thread_entry_avail_cnt--;
	}

	return entry;
}

static void dubai_free_proc_entry(struct dubai_proc_entry *entry)
{
	hash_del(&entry->hash);
	dubai_free_dynamic_hashtable(entry->threads);
	if (proc_entry_avail_cnt == MAX_PROC_AVAIL_COUNT) {
		kfree(entry);
		return;
	}
	memset(entry, 0, sizeof(struct dubai_proc_entry));
	proc_entry_avail_pool[proc_entry_avail_cnt] = entry;
	proc_entry_avail_cnt++;
}

static void dubai_free_thread_entry(struct dubai_thread_entry *entry)
{
	hash_del(&entry->hash);
	if (thread_entry_avail_cnt == MAX_THREAD_AVAIL_COUNT) {
		kfree(entry);
		return;
	}
	memset(entry, 0, sizeof(struct dubai_thread_entry));
	thread_entry_avail_pool[thread_entry_avail_cnt] = entry;
	thread_entry_avail_cnt++;
}

static struct dubai_thread_entry *dubai_find_or_register_thread_entry_locked(pid_t pid,
		const char *comm, struct dubai_proc_entry *entry)
{
	struct dubai_thread_entry *thread = NULL;

	if (!dubai_is_proc_entry_decomposed(entry))
		return NULL;

	dubai_dynamic_hash_for_each_possible(entry->threads, thread, hash, pid) {
		if (thread->pid == pid)
			goto update_thread_name;
	}

	thread = dubai_get_avail_thread_entry();
	if (!thread) {
		thread = kzalloc(sizeof(struct dubai_thread_entry), GFP_ATOMIC);
		if (!thread) {
			dubai_err("Failed to allocate memory");
			return NULL;
		}
	}
	thread->pid = pid;
	thread->state = DUBAI_STATE_ALIVE;
	dubai_dynamic_hash_add(entry->threads, &thread->hash, pid);
	atomic_inc(&active_count);

update_thread_name:
	snprintf(thread->name, NAME_LEN - 1, "%s_%s", entry->name, comm);

	return thread;
}

static struct dubai_proc_entry *dubai_find_proc_entry_locked(pid_t tgid)
{
	struct dubai_proc_entry *entry = NULL;

	dubai_hash_for_each_possible(proc_hash_table, entry, hash, tgid) {
		if (entry->tgid == tgid)
			return entry;
	}

	return NULL;
}

static struct dubai_proc_entry *dubai_register_proc_entry_locked(uid_t uid, pid_t tgid, bool cmdline, const char *name)
{
	struct dubai_proc_entry *entry = NULL;

	entry = dubai_get_avail_proc_entry();
	if (!entry) {
		entry = kzalloc(sizeof(struct dubai_proc_entry), GFP_ATOMIC);
		if (!entry) {
			dubai_err("Failed to allocate memory");
			return NULL;
		}
	}
	entry->tgid = tgid;
	entry->uid = uid;
	entry->state = DUBAI_STATE_ALIVE;
	entry->decomposed = false;
	entry->cmdline = cmdline;
	strncpy(entry->name, name, NAME_LEN - 1);
	entry->threads = NULL;
	dubai_hash_add(proc_hash_table, &entry->hash, tgid);
	atomic_inc(&active_count);

	return entry;
}

static struct dubai_proc_entry *dubai_find_or_register_proc_entry_locked(pid_t tgid, struct task_struct *task)
{
	uid_t uid;
	const char *name = NULL;
	struct dubai_proc_entry *entry = NULL;

	name = dubai_get_task_normalized_name(task);
	uid = from_kuid_munged(current_user_ns(), task_uid(task));
	entry = dubai_find_proc_entry_locked(tgid);
	if (entry == NULL)
		return dubai_register_proc_entry_locked(uid, tgid, dubai_is_kthread(task), name);

	// always update uid and name because they are mutable sometimes.
	entry->uid = uid;
	if (!entry->cmdline && strncmp(entry->name, name, NAME_LEN - 1))
		strncpy(entry->name, name, NAME_LEN - 1);

	return entry;
}

void dubai_set_proc_entry_decompose(pid_t tgid, struct task_struct *task, const char *name)
{
	struct dubai_proc_entry *entry = NULL;

	mutex_lock(&dubai_proc_lock);
	entry = dubai_find_or_register_proc_entry_locked(tgid, task);
	if (!entry)
		goto out;
	if (!entry->threads)
		entry->threads = dubai_alloc_dynamic_hashtable(THREAD_HASH_BITS);
	if (entry->threads) {
		entry->decomposed = true;
		entry->cmdline = true;
		strncpy(entry->name, name, NAME_LEN - 1);
		dubai_info("Set proc entry decomposed, tgid: %d, %s", entry->tgid, entry->name);
	}

out:
	mutex_unlock(&dubai_proc_lock);
}

/*
 * To avoid extending the RCU grace period for an unbounded amount of time,
 * periodically exit the critical section and enter a new one.
 * For preemptible RCU it is sufficient to call rcu_read_unlock in order
 * to exit the grace period. For classic RCU, a reschedule is required.
 */
static bool dubai_rcu_lock_break(struct task_struct *g, struct task_struct *t)
{
	bool can_cont = true;

	get_task_struct(g);
	get_task_struct(t);
	rcu_read_unlock();
	cond_resched();
	rcu_read_lock();
	can_cont = pid_alive(g) && pid_alive(t);
	put_task_struct(t);
	put_task_struct(g);

	return can_cont;
}

static int dubai_update_stats_locked(void)
{
	u64 cal_time;
	pid_t tgid;
	struct dubai_proc_entry *entry = NULL;
	struct dubai_thread_entry *thread = NULL;
	struct task_struct *task = NULL, *temp = NULL;
	dubai_cputime_t utime, stime, time;
	unsigned long last_break;
	int ret = 0, proc_cnt = 0;

	update_active_succ = true;
	rcu_read_lock();
	cal_time = ktime_get_ns();
	last_break = jiffies;
	// update active time from alive task
	for_each_process_thread(temp, task) {
		/*
		 * check whether task is alive or not
		 * if not, do not record this task's cpu time
		 */
		if (!dubai_is_task_alive(task))
			continue;

		tgid = dubai_get_task_normalized_tgid(task);
		if (!entry || (entry->tgid != tgid)) {
			proc_cnt++;
			if (((proc_cnt % 10) == 0) &&
				time_is_before_jiffies(last_break + RCU_LOCK_BREAK_TIMEOUT)) {
				dubai_info("Timeout with rcu_read_lock");
				if (!dubai_rcu_lock_break(temp, task)) {
					dubai_err("Cannot continue to update cpu stats");
					update_active_succ = false;
					goto unlock;
				}
				last_break = jiffies;
			}
			entry = dubai_find_or_register_proc_entry_locked(tgid, task);
		}
		if (!entry) {
			dubai_err("Failed to find the entry for process %d", tgid);
			ret = -ENOMEM;
			goto unlock;
		}
		task_cputime_adjusted(task, &utime, &stime);
		time = utime + stime;
		if (!dubai_check_entry_state(entry, DUBAI_STATE_DEAD)) {
			entry->state = DUBAI_STATE_ALIVE;
			entry->active_time += time;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
			entry->active_power += task->cpu_power;
#endif
		}
		thread = dubai_find_or_register_thread_entry_locked(task->pid, task->comm, entry);
		if ((thread != NULL) && !dubai_check_entry_state(thread, DUBAI_STATE_DEAD)) {
			thread->state = DUBAI_STATE_ALIVE;
			thread->time = time;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
			thread->power = task->cpu_power;
#endif
		}
	};
unlock:
	dubai_info("Updating cpu stats with rcu_read_lock takes %lldms", (ktime_get_ns() - cal_time) / NSEC_PER_MSEC);
	rcu_read_unlock();

	return ret;
}

/*
 * initialize hash list
 * report dead process and delete hash node
 */
static void dubai_get_clear_dead_stats_locked(struct buffered_log_entry *log_entry)
{
	bool decomposed = false;
	int max;
	unsigned char *value = NULL;
	unsigned long bkt, tbkt;
	struct dubai_proc_entry *entry = NULL;
	struct hlist_node *tmp = NULL, *ttmp = NULL;
	struct dubai_thread_entry *thread = NULL;
	struct dubai_cputime_transmit *transmit = (struct dubai_cputime_transmit *)(log_entry->data);

	value = transmit->value;
	max = transmit->count;
	transmit->count = 0;

	dubai_hash_for_each_safe(proc_hash_table, bkt, tmp, entry, hash) {
		entry->active_time = 0;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
		entry->active_power = 0;
#endif
		decomposed = dubai_is_proc_entry_decomposed(entry);
		if (decomposed) {
			dubai_dynamic_hash_for_each_safe(entry->threads, tbkt, ttmp, thread, hash) {
				if (!dubai_check_update_entry_state(thread, DUBAI_STATE_DEAD, DUBAI_STATE_UNSETTLED))
					continue;
				if ((transmit->count < max) && (thread->time > 0)) {
					dubai_thread_entry_copy(value, entry->uid, thread);
					value += sizeof(struct dubai_cputime);
					transmit->count++;
				}
				dubai_free_thread_entry(thread);
			}
		}

		if (!dubai_check_update_entry_state(entry, DUBAI_STATE_DEAD, DUBAI_STATE_UNSETTLED))
			continue;
		if (!decomposed && (transmit->count < max) && (entry->dead_time > 0)) {
			dubai_proc_entry_copy(value, entry);
			value += sizeof(struct dubai_cputime);
			transmit->count++;
		}
		if (!decomposed || (dubai_dynamic_hash_empty(entry->threads)))
			dubai_free_proc_entry(entry);
	}
}

static void dubai_get_active_stats_locked(struct buffered_log_entry *log_entry)
{
	int max_count;
	unsigned long bkt, tbkt;
	struct dubai_proc_entry *entry = NULL;
	struct dubai_thread_entry *thread = NULL;
	struct dubai_cputime_transmit *transmit = (struct dubai_cputime_transmit *)(log_entry->data);
	unsigned char *value = transmit->value;

	max_count = transmit->count;
	transmit->count = 0;
	dubai_hash_for_each(proc_hash_table, bkt, entry, hash) {
		if (unlikely(dubai_check_entry_state(entry, DUBAI_STATE_UNSETTLED)))
			dubai_err("Proc stay unsettled: uid=%u, tgid=%d, name=%s",
				entry->uid, entry->tgid, entry->name);

		if ((update_active_succ &&
			!dubai_check_update_entry_state(entry, DUBAI_STATE_ALIVE, DUBAI_STATE_DEAD)) ||
			(entry->active_time == 0))
			continue;

		if (transmit->count >= max_count)
			break;

		if (!dubai_is_proc_entry_decomposed(entry)) {
			dubai_proc_entry_copy(value, entry);
			value += sizeof(struct dubai_cputime);
			transmit->count++;
			continue;
		}

		dubai_dynamic_hash_for_each(entry->threads, tbkt, thread, hash) {
			if (unlikely(dubai_check_entry_state(thread, DUBAI_STATE_UNSETTLED)))
				dubai_err("Thread stay unsettled: uid=%u, tgid=%d, pid=%d, name=%s",
					entry->uid, entry->tgid, thread->pid, thread->name);

			if ((update_active_succ &&
				!dubai_check_update_entry_state(thread, DUBAI_STATE_ALIVE, DUBAI_STATE_DEAD)) ||
				(thread->time == 0))
				continue;

			if (transmit->count >= max_count)
				break;

			dubai_thread_entry_copy(value, entry->uid, thread);
			value += sizeof(struct dubai_cputime);
			transmit->count++;
		}
	}
}

static int dubai_get_stats(long long timestamp)
{
	int ret;
	struct buffered_log_entry *dead_entry = NULL;
	struct buffered_log_entry *active_entry = NULL;
	int dead_cnt = atomic_read(&dead_count);
	int total_cnt = dead_cnt + atomic_read(&active_count);

	dead_entry = dubai_create_log_entry(timestamp, dead_cnt, PROCESS_STATE_DEAD);
	if (dead_entry == NULL) {
		dubai_err("Failed to create log entry for dead processes");
		return -ENOMEM;
	}
	// use total_cnt for active process
	active_entry = dubai_create_log_entry(timestamp, total_cnt, PROCESS_STATE_ACTIVE);
	if (active_entry == NULL) {
		dubai_err("Failed to create log entry for active processes");
		free_buffered_log_entry(dead_entry);
		return -ENOMEM;
	}

	dubai_info("Get cpu stats enter");
	mutex_lock(&dubai_proc_lock);
	dubai_get_clear_dead_stats_locked(dead_entry);
	atomic_set(&dead_count, 0);

	ret = dubai_update_stats_locked();
	if (ret < 0) {
		dubai_err("Failed to update cpu stats");
		mutex_unlock(&dubai_proc_lock);
		goto exit;
	}
	dubai_get_active_stats_locked(active_entry);
	mutex_unlock(&dubai_proc_lock);
	dubai_send_log_entry(dead_entry);
	dubai_send_log_entry(active_entry);

exit:
	free_buffered_log_entry(dead_entry);
	free_buffered_log_entry(active_entry);
	dubai_info("Get cpu stats exit, active count: %d, nr_threads: %d",
		atomic_read(&active_count), nr_threads);

	return ret;
}

/*
 * if there are dead processes in the list,
 * we should clear these dead processes
 * in case of pid reused
 */
static int dubai_get_proc_cputime(void __user *argp)
{
	int ret = 0;
	long long timestamp;

	if (!atomic_read(&proc_cputime_enable))
		return -EPERM;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		dubai_err("Failed to get timestamp");
		goto exit;
	}
	dubai_info("Get cpu stats: %lld", timestamp);

	ret = dubai_get_stats(timestamp);
	if (ret < 0) {
		dubai_err("Failed to clear dead process and update list");
		goto exit;
	}

exit:
	return ret;
}

static void dubai_proc_cputime_reset(void)
{
	int i;
	unsigned long bkt, tbkt;
	struct dubai_proc_entry *entry = NULL;
	struct dubai_thread_entry *thread = NULL;
	struct hlist_node *tmp = NULL, *ttmp = NULL;

	mutex_lock(&dubai_proc_lock);
	dubai_hash_for_each_safe(proc_hash_table, bkt, tmp, entry, hash) {
		if (dubai_is_proc_entry_decomposed(entry)) {
			dubai_dynamic_hash_for_each_safe(entry->threads, tbkt, ttmp, thread, hash)
				dubai_free_thread_entry(thread);
		}
		dubai_free_proc_entry(entry);
	}
	for (i = 0; i < MAX_PROC_AVAIL_COUNT; i++) {
		if (proc_entry_avail_pool[i])
			kfree(proc_entry_avail_pool[i]);
	}
	for (i = 0; i < MAX_THREAD_AVAIL_COUNT; i++) {
		if (thread_entry_avail_pool[i])
			kfree(thread_entry_avail_pool[i]);
	}
	atomic_set(&dead_count, 0);
	atomic_set(&active_count, 0);
	atomic_set(&proc_cputime_enable, 0);
	dubai_hash_init(proc_hash_table);
	dubaid_tgid = -1;
	proc_entry_avail_cnt = 0;
	thread_entry_avail_cnt = 0;
	memset(proc_entry_avail_pool, 0, sizeof(proc_entry_avail_pool));
	memset(thread_entry_avail_pool, 0, sizeof(thread_entry_avail_pool));
	mutex_unlock(&dubai_proc_lock);
	dubai_info("Dubai cpu process stats reset");
}

static bool is_same_process(const struct dubai_proc_entry *entry, struct task_struct *task)
{
	uid_t uid;

	if (!dubai_check_entry_state(entry, DUBAI_STATE_DEAD))
		return true;

	// pid has been reused by another task
	if (unlikely(entry->tgid == task->pid) || dubai_is_task_alive(task->group_leader))
		return false;

	uid = from_kuid_munged(current_user_ns(), task_uid(task));
	if (uid != entry->uid)
		return false;

	return true;
}

static void dubai_update_proc_dead(struct dubai_proc_entry *entry, struct task_struct *task, dubai_cputime_t time)
{
	if (!is_same_process(entry, task))
		return;

	entry->dead_time += time;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	if (task->cpu_power != ULLONG_MAX)
		entry->dead_power += task->cpu_power;
#endif

	// process has died
	if ((entry->tgid == task->pid) && !dubai_check_entry_state(entry, DUBAI_STATE_DEAD)) {
		entry->state = DUBAI_STATE_DEAD;
		atomic_dec(&active_count);
		atomic_inc(&dead_count);
	}
}

static void dubai_update_thread_dead(struct dubai_thread_entry *thread, struct task_struct *task, dubai_cputime_t time)
{
	if (dubai_check_entry_state(thread, DUBAI_STATE_DEAD))
		return;

	thread->time = time;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	if (task->cpu_power != ULLONG_MAX)
		thread->power = task->cpu_power;
#endif
	thread->state = DUBAI_STATE_DEAD;
	atomic_dec(&active_count);
	atomic_inc(&dead_count);
}

static int dubai_process_notifier(struct notifier_block __always_unused *self,
	unsigned long __always_unused cmd, void *v)
{
	struct task_struct *task = v;
	pid_t tgid;
	struct dubai_proc_entry *entry = NULL;
	struct dubai_thread_entry *thread = NULL;
	char cmdline[CMDLINE_LEN] = {0};
	dubai_cputime_t utime, stime, time;
	bool group_dying = false, decomposed = false;

	if (task == NULL || !atomic_read(&proc_cputime_enable))
		return NOTIFY_OK;

	group_dying = dubai_thread_group_dying(task);
	tgid = dubai_get_task_normalized_tgid(task);
	if (!dubai_is_kthread(task) && group_dying) {
		if (dubai_get_cmdline(task, cmdline, CMDLINE_LEN) < 0)
			cmdline[0] = '\0';
	}

	mutex_lock(&dubai_proc_lock);
	if (group_dying || dubai_is_task_alive(task->group_leader)) {
		entry = dubai_find_or_register_proc_entry_locked(tgid, task);
	} else {
		entry = dubai_find_proc_entry_locked(tgid);
	}
	if (entry == NULL)
		goto exit;
	if (strlen(cmdline) > 0) {
		dubai_copy_name(entry->name, cmdline, NAME_LEN - 1);
		entry->cmdline = true;
	}
	decomposed = dubai_is_proc_entry_decomposed(entry);
	task_cputime_adjusted(task, &utime, &stime);
	time = utime + stime;
	dubai_update_proc_dead(entry, task, time);
	thread = dubai_find_or_register_thread_entry_locked(task->pid, task->comm, entry);
	if (thread)
		dubai_update_thread_dead(thread, task, time);

exit:
	mutex_unlock(&dubai_proc_lock);
	if (group_dying && !dubai_is_kernel_tgid(tgid)) {
		if (unlikely(decomposed))
			dubai_remove_proc_decompose(tgid);
		if (unlikely(tgid == dubaid_tgid))
			dubai_proc_cputime_reset();
	}

	return NOTIFY_OK;
}

static struct notifier_block process_notifier_block = {
	.notifier_call	= dubai_process_notifier,
	.priority = INT_MAX,
};


static int dubai_proc_cputime_enable(void __user *argp)
{
	int ret;
	bool enable = false;

	ret = get_enable_value(argp, &enable);
	if (ret == 0) {
		dubaid_tgid = current->tgid;
		atomic_set(&proc_cputime_enable, enable ? 1 : 0);
		dubai_info("Dubai cpu process stats enable: %d", enable ? 1 : 0);
	}

	return ret;
}

static int dubai_get_task_cpupower_enable(void __user *argp)
{
	bool enable;

	enable = !!atomic_read(&task_power_enable);
	if (copy_to_user(argp, &enable, sizeof(bool)))
		return -EFAULT;

	return 0;
}

void dubai_proc_cputime_init(void)
{
	int ret;

	dubai_proc_cputime_reset();
	ret = profile_event_register(PROFILE_TASK_EXIT, &process_notifier_block);
	if (ret)
		dubai_err("Failed to register task exit notifier");
	dubai_cpu_configs_init();
}

void dubai_proc_cputime_exit(void)
{
	profile_event_unregister(PROFILE_TASK_EXIT, &process_notifier_block);
	dubai_proc_cputime_reset();
	dubai_cpu_configs_exit();
}
