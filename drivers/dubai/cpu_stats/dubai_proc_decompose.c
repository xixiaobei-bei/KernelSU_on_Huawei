#include <linux/sched/signal.h>
#include <linux/slab.h>

#include "dubai_cpu_stats_common.h"
#include "../utils/dubai_utils.h"

#define DECOMPOSE_COUNT_MAX		10
#define DECOMPOSE_RETYR_MAX		100
#define DECOMPOSE_RETRY_PERIOD	20

struct dubai_decompose_info {
	uid_t uid;
	char comm[TASK_COMM_LEN];
	char prefix[PREFIX_LEN];
} __packed;

struct dubai_proc_decompose {
	pid_t tgid;
	struct dubai_decompose_info decompose;
	struct list_head node;
};

static DEFINE_MUTEX(dubai_proc_decompose_lock);
static void dubai_decompose_work_fn(struct work_struct *work);
static DECLARE_DELAYED_WORK(dubai_decompose_work, dubai_decompose_work_fn);
static LIST_HEAD(dubai_proc_decompose_list);
static int decompose_count;
static int decompose_count_target;
static int decompose_check_retry;

static inline bool dubai_check_proc_compose_count(void)
{
	return decompose_count == decompose_count_target;
}

static struct dubai_proc_decompose *dubai_find_decompose_entry_locked(pid_t tgid)
{
	struct dubai_proc_decompose *entry = NULL;

	list_for_each_entry(entry, &dubai_proc_decompose_list, node) {
		if (entry->tgid == tgid)
			return entry;
	}

	return NULL;
}

static struct task_struct *dubai_find_proc(uid_t t_uid, const char *t_comm)
{
	uid_t uid;
	struct task_struct *t = NULL, *task = NULL;
	bool finding_kthread = false;

	finding_kthread = !strncmp(t_comm, KERNEL_NAME, TASK_COMM_LEN - 1);
	rcu_read_lock();
	for_each_process(t) {
		if (!dubai_is_task_alive(t))
			continue;

		if (dubai_is_kthread(t)) {
			if (!finding_kthread)
				continue;
			task = t;
			break;
		}
		uid = from_kuid_munged(current_user_ns(), task_uid(t));
		if (uid == t_uid && !strncmp(t->comm, t_comm, TASK_COMM_LEN - 1)) {
			task = t;
			break;
		}
	}
	if (task)
		get_task_struct(task);
	rcu_read_unlock();

	return task;
}

static bool dubai_check_proc_decompose_locked(void)
{
	struct dubai_proc_decompose *entry = NULL;
	struct task_struct *task = NULL;

	list_for_each_entry(entry, &dubai_proc_decompose_list, node) {
		if (entry->tgid >= 0)
			continue;

		task = dubai_find_proc(entry->decompose.uid, entry->decompose.comm);
		if (task) {
			entry->tgid = dubai_get_task_normalized_tgid(task);
			dubai_set_proc_entry_decompose(entry->tgid, task, entry->decompose.prefix);
			put_task_struct(task);
			decompose_count++;
			dubai_info("Add decompose proc: %d, %s", entry->tgid, entry->decompose.prefix);
		}
	}

	return dubai_check_proc_compose_count();
}

static void dubai_schedule_decompose_work_locked(void)
{
	unsigned long delay;

	if (decompose_check_retry > DECOMPOSE_RETYR_MAX)
		return;
	decompose_check_retry++;
	delay = ((decompose_check_retry % DECOMPOSE_RETRY_PERIOD) != 0) ? (HZ / 2) : (5 * HZ);
	schedule_delayed_work(&dubai_decompose_work, delay);
}

static void dubai_decompose_work_fn(struct work_struct *work)
{
	mutex_lock(&dubai_proc_decompose_lock);
	if (!dubai_check_proc_decompose_locked()) {
		dubai_schedule_decompose_work_locked();
	} else {
		decompose_check_retry = 0;
		dubai_info("Succeed to get processes to decompose, count: %d", decompose_count);
	}
	mutex_unlock(&dubai_proc_decompose_lock);
}

void dubai_remove_proc_decompose(pid_t tgid)
{
	struct dubai_proc_decompose *entry = NULL;

	mutex_lock(&dubai_proc_decompose_lock);
	entry = dubai_find_decompose_entry_locked(tgid);
	if (entry != NULL) {
		dubai_info("Remove decompose proc: %d, %s", tgid, entry->decompose.prefix);
		entry->tgid = -1;
		decompose_count--;
		dubai_schedule_decompose_work_locked();
	}
	mutex_unlock(&dubai_proc_decompose_lock);
}

static void dubai_clear_proc_decompose_locked(void)
{
	struct dubai_proc_decompose *entry = NULL, *tmp = NULL;

	list_for_each_entry_safe(entry, tmp, &dubai_proc_decompose_list, node) {
		list_del_init(&entry->node);
		kfree(entry);
	}
	decompose_count = 0;
	decompose_count_target = 0;
	decompose_check_retry = 0;
}

static int dubai_set_proc_decompose_list(const struct dubai_decompose_info *info, int info_cnt)
{
	int ret = 0, cnt;
	struct dubai_proc_decompose *entry = NULL;

	mutex_lock(&dubai_proc_decompose_lock);
	dubai_clear_proc_decompose_locked();
	for (cnt = 0; cnt < info_cnt; cnt++) {
		entry = kzalloc(sizeof(struct dubai_proc_decompose), GFP_KERNEL);
		if (entry == NULL) {
			ret = -ENOMEM;
			goto exit;
		}
		memcpy(&entry->decompose, info, sizeof(struct dubai_decompose_info));
		entry->decompose.comm[TASK_COMM_LEN - 1] = '\0';
		entry->decompose.prefix[PREFIX_LEN - 1] = '\0';
		entry->tgid = -1;
		list_add_tail(&entry->node, &dubai_proc_decompose_list);
		info++;
	}

exit:
	decompose_count_target = cnt;
	if ((cnt > 0) && !dubai_check_proc_decompose_locked())
		dubai_schedule_decompose_work_locked();
	mutex_unlock(&dubai_proc_decompose_lock);

	return ret;
}

int dubai_set_proc_decompose(const void __user *argp)
{
	int ret = 0, size, count, length, remain;
	struct dev_transmit_t *transmit = NULL;

	if (copy_from_user(&length, argp, sizeof(int)))
		return -EFAULT;

	count = length / (int)(sizeof(struct dubai_decompose_info));
	remain = length % (int)(sizeof(struct dubai_decompose_info));
	if ((count <= 0) || (count > DECOMPOSE_COUNT_MAX) || (remain != 0)) {
		dubai_err("Invalid length, length: %d, count: %d, remain: %d", length, count, remain);
		return -EINVAL;
	}

	size = length + sizeof(struct dev_transmit_t);
	transmit = kzalloc(size, GFP_KERNEL);
	if (transmit == NULL)
		return -ENOMEM;

	if (copy_from_user(transmit, argp, size)) {
		ret = -EFAULT;
		goto exit;
	}

	ret = dubai_set_proc_decompose_list((const struct dubai_decompose_info *)(transmit->data), count);
	if (ret < 0)
		dubai_err("Failed to set process decompose list");

exit:
	kfree(transmit);

	return ret;
}
