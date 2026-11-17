#include "dubai_misc_stats.h"

#include <linux/hashtable.h>
#include <linux/kallsyms.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/time.h>

#include <chipset_common/dubai/dubai_ioctl.h>

#include "utils/buffered_log_sender.h"
#include "utils/dubai_utils.h"

#define IOC_LOG_STATS_ENABLE DUBAI_MISC_DIR_IOC(W, 1, bool)
#define IOC_KWORKER_INFO_REQUEST DUBAI_MISC_DIR_IOC(W, 2, long long)
#define IOC_UEVENT_INFO_REQUEST DUBAI_MISC_DIR_IOC(W, 3, long long)
#define IOC_BINDER_STATS_ENABLE DUBAI_MISC_DIR_IOC(W, 4, bool)
#define IOC_BINDER_STATS_REQUEST DUBAI_MISC_DIR_IOC(W, 5, long long)

#define KWORKER_HASH_BITS					10
#define MAX_SYMBOL_LEN						48
#define MAX_DEVPATH_LEN						128
#define BINDER_STATS_HASH_BITS				10
#define MAX_BINDER_UID						100000

struct dubai_kworker_info {
	long long count;
	long long time;
	char symbol[MAX_SYMBOL_LEN];
} __packed;

struct dubai_kworker_transmit {
	long long timestamp;
	int count;
	char data[0];
} __packed;

struct dubai_kworker_entry {
	unsigned long address;
	struct dubai_kworker_info info;
	struct hlist_node hash;
};

struct dubai_uevent_info {
	char devpath[MAX_DEVPATH_LEN];
	int actions[KOBJ_MAX];
} __packed;

struct dubai_uevent_transmit {
	long long timestamp;
	int action_count;
	int count;
	char data[0];
} __packed;

struct dubai_uevent_entry {
	struct list_head list;
	struct dubai_uevent_info uevent;
};

struct dubai_binder_proc {
	uid_t uid;
	pid_t pid;
	char cmdline[CMDLINE_LEN];
} __packed;

struct dubai_binder_client_entry {
	struct dubai_binder_proc proc;
	int count;
	struct list_head node;
};

struct dubai_binder_stats_entry {
	struct dubai_binder_proc proc;
	int count;
	struct list_head client;
	struct list_head died;
	struct hlist_node hash;
};

struct dubai_binder_stats_info {
	struct dubai_binder_proc service;
	struct dubai_binder_proc client;
	int count;
} __packed;

struct dubai_binder_stats_transmit {
	long long timestamp;
	int count;
	char data[0];
} __packed;

static int dubai_log_stats_enable(void __user *argp);
static int dubai_get_kworker_info(void __user *argp);
static int dubai_get_uevent_info(void __user *argp);
static int dubai_get_binder_stats(void __user *argp);
static int dubai_binder_stats_enable(void __user *argp);

static atomic_t kworker_count;
static atomic_t uevent_count;
static atomic_t log_stats_enable;
static atomic_t binder_stats_enable;
static atomic_t binder_client_count;

static DECLARE_HASHTABLE(kworker_hash_table, KWORKER_HASH_BITS);
static DEFINE_MUTEX(kworker_lock);
static LIST_HEAD(uevent_list);
static DEFINE_MUTEX(uevent_lock);
static DECLARE_HASHTABLE(binder_stats_hash_table, BINDER_STATS_HASH_BITS);
static LIST_HEAD(binder_stats_died_list);
static DEFINE_MUTEX(binder_stats_hash_lock);

long dubai_ioctl_misc(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_LOG_STATS_ENABLE:
		rc = dubai_log_stats_enable(argp);
		break;
	case IOC_KWORKER_INFO_REQUEST:
		rc = dubai_get_kworker_info(argp);
		break;
	case IOC_UEVENT_INFO_REQUEST:
		rc = dubai_get_uevent_info(argp);
		break;
	case IOC_BINDER_STATS_ENABLE:
		rc = dubai_binder_stats_enable(argp);
		break;
	case IOC_BINDER_STATS_REQUEST:
		rc = dubai_get_binder_stats(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static struct dubai_kworker_entry *dubai_find_kworker_entry(unsigned long address)
{
	struct dubai_kworker_entry *entry = NULL;

	hash_for_each_possible(kworker_hash_table, entry, hash, address) {
		if (entry->address == address)
			return entry;
	}

	entry = kzalloc(sizeof(struct dubai_kworker_entry), GFP_ATOMIC);
	if (!entry) {
		dubai_err("Failed to allocate memory");
		return NULL;
	}
	entry->address = address;
	atomic_inc(&kworker_count);
	hash_add(kworker_hash_table, &entry->hash, address);

	return entry;
}

void dubai_log_kworker(unsigned long address, unsigned long long enter_time)
{
	unsigned long long exit_time;
	struct dubai_kworker_entry *entry = NULL;

	if (!atomic_read(&log_stats_enable))
		return;

	exit_time = ktime_get_ns();

	if (!mutex_trylock(&kworker_lock))
		return;

	entry = dubai_find_kworker_entry(address);
	if (!entry) {
		dubai_err("Failed to find kworker entry");
		goto out;
	}

	entry->info.count++;
	entry->info.time += exit_time - enter_time;

out:
	mutex_unlock(&kworker_lock);
}
EXPORT_SYMBOL(dubai_log_kworker);

static void dubai_get_kworker_stats(struct buffered_log_entry *log_entry, long long timestamp, int max_count)
{
	unsigned char *data = NULL;
	unsigned long bkt;
	struct dubai_kworker_entry *entry = NULL;
	struct hlist_node *tmp = NULL;
	struct dubai_kworker_transmit *transmit = NULL;

	transmit = (struct dubai_kworker_transmit *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->count = 0;
	data = transmit->data;

	mutex_lock(&kworker_lock);
	hash_for_each_safe(kworker_hash_table, bkt, tmp, entry, hash) {
		if (entry->info.count == 0 || entry->info.time == 0) {
			hash_del(&entry->hash);
			kfree(entry);
			atomic_dec(&kworker_count);
			continue;
		}

		if (strlen(entry->info.symbol) == 0) {
			char buffer[KSYM_SYMBOL_LEN] = {0};

			sprint_symbol_no_offset(buffer, entry->address);
			buffer[KSYM_SYMBOL_LEN - 1] = '\0';
			strncpy(entry->info.symbol, buffer, MAX_SYMBOL_LEN - 1);
		}

		if (transmit->count < max_count) {
			memcpy(data, &entry->info, sizeof(struct dubai_kworker_info));
			data += sizeof(struct dubai_kworker_info);
			transmit->count++;
		}
		entry->info.count = 0;
		entry->info.time = 0;
	}
	mutex_unlock(&kworker_lock);
}

static int dubai_get_kworker_info(void __user *argp)
{
	int ret, count;
	long long timestamp, size;
	struct buffered_log_entry *log_entry = NULL;
	struct dubai_kworker_transmit *transmit = NULL;

	if (!atomic_read(&log_stats_enable))
		return -EPERM;

	count = atomic_read(&kworker_count);
	if (count < 0)
		return -EINVAL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		dubai_err("Failed to get timestamp");
		return ret;
	}

	size = cal_log_entry_len(struct dubai_kworker_transmit, struct dubai_kworker_info, count);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_KWORKER);
	if (log_entry == NULL) {
		dubai_err("Failed to create buffered log entry");
		return -ENOMEM;
	}
	dubai_get_kworker_stats(log_entry, timestamp, count);
	transmit = (struct dubai_kworker_transmit *)log_entry->data;
	log_entry->length = cal_log_entry_len(struct dubai_kworker_transmit, struct dubai_kworker_info, transmit->count);
	ret = send_buffered_log(log_entry);
	if (ret < 0)
		dubai_err("Failed to send kworker log entry");
	free_buffered_log_entry(log_entry);

	return ret;
}

static struct dubai_uevent_entry *dubai_find_uevent_entry(const char *devpath)
{
	struct dubai_uevent_entry *entry = NULL;

	list_for_each_entry(entry, &uevent_list, list) {
		if (!strncmp(devpath, entry->uevent.devpath, MAX_DEVPATH_LEN - 1))
			return entry;
	}

	entry = kzalloc(sizeof(struct dubai_uevent_entry), GFP_ATOMIC);
	if (!entry) {
		dubai_err("Failed to allocate memory");
		return NULL;
	}
	strncpy(entry->uevent.devpath, devpath, MAX_DEVPATH_LEN - 1);
	atomic_inc(&uevent_count);
	list_add_tail(&entry->list, &uevent_list);

	return entry;
}

void dubai_log_uevent(const char *devpath, unsigned int action)
{
	struct dubai_uevent_entry *entry = NULL;

	if (!atomic_read(&log_stats_enable)
		|| !devpath
		|| (action >= KOBJ_MAX))
		return;

	mutex_lock(&uevent_lock);
	entry = dubai_find_uevent_entry(devpath);
	if (!entry) {
		dubai_err("Failed to find uevent entry");
		goto out;
	}
	(entry->uevent.actions[action])++;

out:
	mutex_unlock(&uevent_lock);
}
EXPORT_SYMBOL(dubai_log_uevent);

static void dubai_get_uevent_stats(struct buffered_log_entry *log_entry, long long timestamp, int max_count)
{
	unsigned char *data = NULL;
	struct dubai_uevent_entry *entry = NULL, *tmp = NULL;
	struct dubai_uevent_transmit *transmit = NULL;
	int i, total;

	transmit = (struct dubai_uevent_transmit *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->action_count = KOBJ_MAX;
	transmit->count = 0;
	data = transmit->data;

	mutex_lock(&uevent_lock);
	list_for_each_entry_safe(entry, tmp, &uevent_list, list) {
		total = 0;
		for (i = 0; i < KOBJ_MAX; i++)
			total += entry->uevent.actions[i];

		if (total == 0) {
			list_del_init(&entry->list);
			kfree(entry);
			atomic_dec(&uevent_count);
			continue;
		}
		if (transmit->count < max_count) {
			memcpy(data, &entry->uevent, sizeof(struct dubai_uevent_info));
			data += sizeof(struct dubai_uevent_info);
			transmit->count++;
		}
		memset(&(entry->uevent.actions), 0, KOBJ_MAX * sizeof(int));
	}
	mutex_unlock(&uevent_lock);
}

static int dubai_get_uevent_info(void __user *argp)
{
	int ret, count;
	long long timestamp, size;
	struct buffered_log_entry *log_entry = NULL;
	struct dubai_uevent_transmit *transmit = NULL;

	if (!atomic_read(&log_stats_enable))
		return -EPERM;

	count = atomic_read(&uevent_count);
	if (count < 0)
		return -EINVAL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		dubai_err("Failed to get timestamp");
		return ret;
	}

	size = cal_log_entry_len(struct dubai_uevent_transmit, struct dubai_uevent_info, count);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_UEVENT);
	if (log_entry == NULL) {
		dubai_err("Failed to create buffered log entry");
		return -ENOMEM;
	}
	dubai_get_uevent_stats(log_entry, timestamp, count);
	transmit = (struct dubai_uevent_transmit *)log_entry->data;
	log_entry->length = cal_log_entry_len(struct dubai_uevent_transmit, struct dubai_uevent_info, transmit->count);
	ret = send_buffered_log(log_entry);
	if (ret < 0)
		dubai_err("Failed to send uevent log entry");
	free_buffered_log_entry(log_entry);

	return ret;
}

static int dubai_log_stats_enable(void __user *argp)
{
	int ret;
	bool enable = false;

	ret = get_enable_value(argp, &enable);
	if (!ret) {
		atomic_set(&log_stats_enable, enable ? 1 : 0);
		dubai_info("Dubai log stats enable: %d", enable ? 1 : 0);
	}

	return ret;
}

static void dubai_init_binder_client(struct list_head *head)
{
	struct dubai_binder_client_entry *client = NULL, *tmp = NULL;

	if (!head) {
		dubai_err("Invalid param");
		return;
	}

	list_for_each_entry_safe(client, tmp, head, node) {
		list_del_init(&client->node);
		kfree(client);
	}
}

static void dubai_init_binder_stats(void)
{
	struct hlist_node *tmp = NULL;
	unsigned long bkt;
	struct dubai_binder_stats_entry *stats = NULL, *temp = NULL;

	mutex_lock(&binder_stats_hash_lock);
	hash_for_each_safe(binder_stats_hash_table, bkt, tmp, stats, hash) {
		dubai_init_binder_client(&stats->client);
		hash_del(&stats->hash);
		kfree(stats);
	}
	list_for_each_entry_safe(stats, temp, &binder_stats_died_list, died) {
		dubai_init_binder_client(&stats->client);
		list_del_init(&stats->died);
		kfree(stats);
	}
	mutex_unlock(&binder_stats_hash_lock);
	atomic_set(&binder_client_count, 0);
}

static struct dubai_binder_client_entry *dubai_check_binder_client_entry(
	const struct list_head *head, const struct dubai_binder_proc *proc)
{
	struct dubai_binder_client_entry *entry = NULL;

	if (!head || !proc) {
		dubai_err("Invalid param");
		return NULL;
	}

	list_for_each_entry(entry, head, node) {
		if (((entry->proc.pid == proc->pid) && (entry->proc.uid == proc->uid))
			|| !strncmp(entry->proc.cmdline, proc->cmdline, CMDLINE_LEN - 1))
			return entry;
	}

	return NULL;
}

static struct dubai_binder_client_entry *dubai_find_binder_client_entry(struct dubai_binder_stats_entry *stats,
	const struct dubai_binder_proc *proc)
{
	struct dubai_binder_client_entry *entry = NULL;

	if (!stats || !proc) {
		dubai_err("Invalid param");
		return NULL;
	}

	entry = dubai_check_binder_client_entry(&stats->client, proc);
	if (!entry) {
		entry = kzalloc(sizeof(struct dubai_binder_client_entry), GFP_ATOMIC);
		if (!entry) {
			dubai_err("Failed to allocate binder client entry");
			return NULL;
		}
		memset(&entry->proc, 0, sizeof(struct dubai_binder_proc));
		entry->count = 0;
		list_add_tail(&entry->node, &stats->client);
		stats->count++;
		atomic_inc(&binder_client_count);
	}

	return entry;
}

static struct dubai_binder_stats_entry *dubai_check_binder_stats_entry(const struct dubai_binder_proc *proc)
{
	struct dubai_binder_stats_entry *entry = NULL;

	if (!proc) {
		dubai_err("Invalid param");
		return NULL;
	}

	hash_for_each_possible(binder_stats_hash_table, entry, hash, proc->pid) {
		if (entry->proc.pid == proc->pid) {
			if (entry->proc.uid != proc->uid) {
				entry->count = 0;
				dubai_init_binder_client(&entry->client);
				list_del_init(&entry->client);
			}
			return entry;
		}
	}

	return NULL;
}

static struct dubai_binder_stats_entry *dubai_find_binder_stats_entry(struct dubai_binder_proc *proc)
{
	struct dubai_binder_stats_entry *entry = NULL;

	if (!proc) {
		dubai_err("Invalid param");
		return NULL;
	}

	entry = dubai_check_binder_stats_entry(proc);
	if (!entry) {
		entry = kzalloc(sizeof(struct dubai_binder_stats_entry), GFP_ATOMIC);
		if (!entry) {
			dubai_err("Failed to allocate binder stats entry");
			return NULL;
		}
		memset(&entry->proc, 0, sizeof(struct dubai_binder_proc));
		entry->count = 0;
		INIT_LIST_HEAD(&entry->client);
		hash_add(binder_stats_hash_table, &entry->hash, proc->pid);
	}

	return entry;
}

static int dubai_get_binder_proc_cmdline(struct dubai_binder_proc *service, struct dubai_binder_proc *client)
{
	struct dubai_binder_stats_entry *stats = NULL;
	struct dubai_binder_client_entry *entry = NULL;
	int rc;
	char service_cmdline[CMDLINE_LEN] = {0};
	char client_cmdline[CMDLINE_LEN] = {0};

	if (!service || !client) {
		dubai_err("Invalid param");
		return -1;
	}

	if (!mutex_trylock(&binder_stats_hash_lock))
		return -1;

	stats = dubai_check_binder_stats_entry(service);
	if (!stats) {
		mutex_unlock(&binder_stats_hash_lock);
		rc = dubai_get_cmdline_by_uid_pid(service->uid, service->pid, service_cmdline, CMDLINE_LEN);
		if (rc < 0) {
			dubai_err("Failed to get service cmdline: %d, %d", service->uid, service->pid);
			return -1;
		}
		rc = dubai_get_cmdline_by_uid_pid(client->uid, client->pid, client_cmdline, CMDLINE_LEN);
		if (rc < 0) {
			dubai_err("Failed to get client cmdline: %d, %d", client->uid, client->pid);
			return -1;
		}
		strncpy(service->cmdline, service_cmdline, CMDLINE_LEN - 1);
		strncpy(client->cmdline, client_cmdline, CMDLINE_LEN - 1);
	} else {
		strncpy(service->cmdline, stats->proc.cmdline, CMDLINE_LEN - 1);
		entry = dubai_check_binder_client_entry(&stats->client, client);
		if (!entry) {
			mutex_unlock(&binder_stats_hash_lock);
			memset(client_cmdline, 0, CMDLINE_LEN);
			rc = dubai_get_cmdline_by_uid_pid(client->uid, client->pid, client_cmdline, CMDLINE_LEN);
			if (rc < 0) {
				dubai_err("Failed to get client cmdline: %d, %d", client->uid, client->pid);
				return -1;
			}
			strncpy(client->cmdline, client_cmdline, CMDLINE_LEN - 1);
		} else {
			strncpy(client->cmdline, entry->proc.cmdline, CMDLINE_LEN - 1);
			mutex_unlock(&binder_stats_hash_lock);
		}
	}

	return 0;
}

static void dubai_add_binder_stats(struct dubai_binder_proc *service, struct dubai_binder_proc *client)
{
	struct dubai_binder_stats_entry *stats = NULL;
	struct dubai_binder_client_entry *entry = NULL;

	if (!service || !client) {
		dubai_err("Invalid param");
		return;
	}

	if (!mutex_trylock(&binder_stats_hash_lock))
		return;

	stats = dubai_find_binder_stats_entry(service);
	if (!stats) {
		dubai_err("Failed to find binder stats entry");
		goto out;
	}
	stats->proc = *service;
	entry = dubai_find_binder_client_entry(stats, client);
	if (!entry) {
		dubai_err("Failed to add binder client entry");
		goto out;
	}
	entry->proc = *client;
	entry->count++;

out:
	mutex_unlock(&binder_stats_hash_lock);
}

static void dubai_get_binder_client(struct dubai_binder_stats_entry *stats,
	struct dubai_binder_stats_transmit *transmit, int count)
{
	struct dubai_binder_client_entry *client = NULL, *temp = NULL;
	struct dubai_binder_stats_info info;
	unsigned char *data = NULL;

	if (!stats || !transmit) {
		dubai_err("Invalid param");
		return;
	}

	data = transmit->data + transmit->count * sizeof(struct dubai_binder_stats_info);
	info.service = stats->proc;
	list_for_each_entry_safe(client, temp, &stats->client, node) {
		info.client = client->proc;
		info.count = client->count;
		if (transmit->count < count) {
			memcpy(data, &info, sizeof(struct dubai_binder_stats_info));
			data += sizeof(struct dubai_binder_stats_info);
			transmit->count++;
		}

		list_del_init(&client->node);
		kfree(client);
	}
}

static void dubai_get_binder_stats_info(struct dubai_binder_stats_transmit *transmit, int count)
{
	unsigned long bkt;
	struct dubai_binder_stats_entry *stats = NULL;
	struct hlist_node *tmp = NULL;

	if (!transmit) {
		dubai_err("Invalid param");
		return;
	}

	hash_for_each_safe(binder_stats_hash_table, bkt, tmp, stats, hash) {
		if (stats->count == 0) {
			dubai_init_binder_client(&stats->client);
			hash_del(&stats->hash);
			kfree(stats);
			continue;
		}

		dubai_get_binder_client(stats, transmit, count);
		stats->count = 0;
		INIT_LIST_HEAD(&stats->client);
	}
}

static void dubai_get_binder_stats_died(struct dubai_binder_stats_transmit *transmit, int count)
{
	struct dubai_binder_stats_entry *tmp = NULL, *stats = NULL;

	if (!transmit) {
		dubai_err("Invalid param");
		return;
	}

	list_for_each_entry_safe(stats, tmp, &binder_stats_died_list, died) {
		dubai_get_binder_client(stats, transmit, count);
		list_del_init(&stats->died);
		kfree(stats);
	}
}

static int dubai_get_binder_stats(void __user *argp)
{
	int ret = 0, count = 0;
	long long timestamp, size = 0;
	struct buffered_log_entry *log_entry = NULL;
	struct dubai_binder_stats_transmit *transmit = NULL;

	if (!atomic_read(&binder_stats_enable))
		return -EPERM;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		dubai_err("Failed to get timestamp");
		return ret;
	}

	count = atomic_read(&binder_client_count);
	size = cal_log_entry_len(struct dubai_binder_stats_transmit, struct dubai_binder_stats_info, count);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_BINDER_STATS);
	if (!log_entry) {
		dubai_init_binder_stats();
		dubai_err("Failed to create buffered log entry");
		return -ENOMEM;
	}
	transmit = (struct dubai_binder_stats_transmit *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->count = 0;

	mutex_lock(&binder_stats_hash_lock);
	dubai_get_binder_stats_died(transmit, count);
	dubai_get_binder_stats_info(transmit, count);
	mutex_unlock(&binder_stats_hash_lock);
	atomic_set(&binder_client_count, 0);

	if (transmit->count > 0) {
		log_entry->length = cal_log_entry_len(struct dubai_binder_stats_transmit,
			struct dubai_binder_stats_info, transmit->count);
		ret = send_buffered_log(log_entry);
		if (ret < 0)
			dubai_err("Failed to send binder stats log entry: %d", ret);
	}
	free_buffered_log_entry(log_entry);

	return ret;
}

void dubai_log_binder_stats(int reply, uid_t c_uid, int c_pid, uid_t s_uid, int s_pid)
{
	struct dubai_binder_proc client, service;

	if (reply || !atomic_read(&binder_stats_enable))
		return;

	if ((c_uid > MAX_BINDER_UID) || (s_uid > MAX_BINDER_UID))
		return;

	memset(&service, 0, sizeof(struct dubai_binder_proc));
	service.uid = s_uid;
	service.pid = s_pid;
	memset(&client, 0, sizeof(struct dubai_binder_proc));
	client.uid = c_uid;
	client.pid = c_pid;

	if (!dubai_get_binder_proc_cmdline(&service, &client))
		dubai_add_binder_stats(&service, &client);
}
EXPORT_SYMBOL(dubai_log_binder_stats);

static int dubai_binder_stats_enable(void __user *argp)
{
	int ret;
	bool enable = false;

	ret = get_enable_value(argp, &enable);
	if (!ret) {
		dubai_info("Set binder stats enable: %d", enable ? 1 : 0);
		atomic_set(&binder_stats_enable, enable ? 1 : 0);
		if (!enable)
			dubai_init_binder_stats();
	}

	return ret;
}

static void dubai_process_binder_died(pid_t pid)
{
	unsigned long bkt;
	struct hlist_node *tmp = NULL;
	struct dubai_binder_stats_entry *stats = NULL;

	if (!atomic_read(&binder_stats_enable))
		return;

	mutex_lock(&binder_stats_hash_lock);
	hash_for_each_safe(binder_stats_hash_table, bkt, tmp, stats, hash) {
		if (stats->proc.pid == pid) {
			hash_del(&stats->hash);
			list_add_tail(&stats->died, &binder_stats_died_list);
			break;
		}
	}
	mutex_unlock(&binder_stats_hash_lock);
}

static int dubai_stats_process_notifier(struct notifier_block __always_unused *self,
	unsigned long __always_unused cmd, void *v)
{
	struct task_struct *task = v;

	if (task && (task->tgid == task->pid))
		dubai_process_binder_died(task->tgid);

	return NOTIFY_OK;
}

static struct notifier_block process_notifier_block = {
	.notifier_call	= dubai_stats_process_notifier,
};

void dubai_misc_stats_init(void)
{
	int ret;

	atomic_set(&binder_stats_enable, 0);
	atomic_set(&binder_client_count, 0);
	hash_init(binder_stats_hash_table);
	atomic_set(&log_stats_enable, 0);
	hash_init(kworker_hash_table);
	atomic_set(&kworker_count, 0);
	atomic_set(&uevent_count, 0);
	ret = profile_event_register(PROFILE_TASK_EXIT, &process_notifier_block);
	if (ret)
		dubai_err("Failed to register task exit notifier");
}

void dubai_misc_stats_exit(void)
{
	struct dubai_kworker_entry *kworker = NULL;
	struct hlist_node *tmp = NULL;
	struct dubai_uevent_entry *uevent = NULL, *temp = NULL;
	unsigned long bkt;

	profile_event_unregister(PROFILE_TASK_EXIT, &process_notifier_block);
	dubai_init_binder_stats();

	mutex_lock(&kworker_lock);
	hash_for_each_safe(kworker_hash_table, bkt, tmp, kworker, hash) {
		hash_del(&kworker->hash);
		kfree(kworker);
	}
	atomic_set(&kworker_count, 0);
	mutex_unlock(&kworker_lock);

	mutex_lock(&uevent_lock);
	list_for_each_entry_safe(uevent, temp, &uevent_list, list) {
		list_del_init(&uevent->list);
		kfree(uevent);
	}
	atomic_set(&uevent_count, 0);
	mutex_unlock(&uevent_lock);
}
