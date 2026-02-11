/*
 * DUBAI kernel statistics.
 *
 * Copyright (C) 2017 Huawei Device Co.,Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hashtable.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/time.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/profile.h>
#include <linux/pm_wakeup.h>
#include <net/ip.h>
#include <uapi/linux/udp.h>
#ifdef CONFIG_LOG_EXCEPTION
#include <log/log_usertype.h>
#endif
#ifdef CONFIG_INPUTHUB_20
#include <huawei_platform/inputhub/sensorhub.h>
#endif
#include <huawei_platform/log/hwlog_kernel.h>
#include <chipset_common/dubai/dubai.h>
#include <chipset_common/dubai/dubai_common.h>

#define KWORKER_HASH_BITS				(10)
#define MAX_SYMBOL_LEN					(48)
#define MAX_DEVPATH_LEN					(128)
#define PRINT_MAX_LEN					(40)
#define MAX_BRIGHTNESS					(10000)
#define BINDER_STATS_HASH_BITS			(10)
#define SENSOR_TIME_BUFF_LEN			(128)
#define PHYSICAL_SENSOR_TYPE_MAX		(40)
#define MAX_WS_NAME_LEN					(64)
#define MAX_WS_NAME_COUNT				(128)
#define WAKEUP_TAG_SIZE					(48)
#define WAKEUP_MSG_SIZE					(128)
#define WAKEUP_SOURCE_SIZE				(32)
#define KERNEL_WAKEUP_TAG				"DUBAI_TAG_KERNEL_WAKEUP"

#ifdef SENSORHUB_DUBAI_INTERFACE
extern int iomcu_dubai_log_fetch(uint32_t event_type, void* data, uint32_t length);
#endif

#define LOG_ENTRY_SIZE(head, info, count) \
	sizeof(head) \
		+ (long long)(count) * sizeof(info)

#pragma GCC diagnostic ignored "-Wunused-variable"
enum {
	DUBAI_EVENT_NULL = 0,
	DUBAI_EVENT_AOD_PICKUP = 3,
	DUBAI_EVENT_AOD_PICKUP_NO_FINGERDOWN =4,
	DUBAI_EVENT_AOD_TIME_STATISTICS = 6,
	DUBAI_EVENT_FINGERPRINT_ICON_COUNT = 7,
	DUBAI_EVENT_ALL_SENSOR_STATISTICS = 8,
	DUBAI_EVENT_ALL_SENSOR_TIME = 9,
	DUBAI_EVENT_SWING = 10,
	DUBAI_EVENT_END
};

enum {
	TINY_CORE_MODEL_FD = 0,
	TINY_CORE_MODEL_SD,
	TINY_CORE_MODEL_FR,
	TINY_CORE_MODEL_AD,
	TINY_CORE_MODEL_FP,
	TINY_CORE_MODEL_HD,
	TINY_CORE_MODEL_WD,
	TINY_CORE_MODEL_WE,
	TINY_CORE_MODEL_WC,
	TINY_CORE_MODEL_MAX,
};

struct dubai_brightness_info {
	atomic_t enable;
	uint64_t last_time;
	uint32_t last_brightness;
	uint64_t sum_time;
	uint64_t sum_brightness_time;
};

struct sensor_time {
	uint32_t type;
	uint32_t time;
} __packed;

struct swing_data {
	uint64_t active_time;
	uint64_t software_standby_time;
	uint64_t als_time;
	uint64_t fill_light_time;
	uint32_t capture_cnt;
	uint32_t fill_light_cnt;
	uint32_t tiny_core_model_cnt[TINY_CORE_MODEL_MAX];
} __packed;

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
	char name[NAME_LEN];
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

struct dubai_ws_lasting_name {
	char name[MAX_WS_NAME_LEN];
} __packed;

struct dubai_ws_lasting_transmit {
	long long timestamp;
	int count;
	char data[0];
} __packed;

struct dubai_rtc_timer_info {
	char name[TASK_COMM_LEN];
	pid_t pid;
} __packed;

#ifdef CONFIG_INPUTHUB_20
struct dubai_sensorhub_type_info {
	int sensorhub_type;
	int data[SENSORHUB_TAG_NUM_MAX];
} __packed;

struct dubai_sensorhub_type_list {
	long long timestamp;
	int size;
	int tag_count;
	struct dubai_sensorhub_type_info data[0];
} __packed;
#endif

struct dubai_wakeup_entry {
	char tag[WAKEUP_TAG_SIZE];
	char msg[WAKEUP_MSG_SIZE];
	struct list_head list;
};

static atomic_t kworker_count;
static atomic_t uevent_count;
static atomic_t log_stats_enable;
static struct dubai_brightness_info dubai_backlight;
static atomic_t binder_stats_enable;
static atomic_t binder_client_count;
static struct dubai_rtc_timer_info dubai_rtc_timer;

static unsigned char last_wakeup_source[WAKEUP_SOURCE_SIZE];
static unsigned long last_app_wakeup_time;
static unsigned long last_wakeup_time;
static int last_wakeup_gpio;

static DECLARE_HASHTABLE(kworker_hash_table, KWORKER_HASH_BITS);
static DEFINE_MUTEX(kworker_lock);
static LIST_HEAD(uevent_list);
static DEFINE_MUTEX(uevent_lock);
static DEFINE_MUTEX(brightness_lock);

static DECLARE_HASHTABLE(binder_stats_hash_table, BINDER_STATS_HASH_BITS);
static LIST_HEAD(binder_stats_died_list);
static DEFINE_MUTEX(binder_stats_hash_lock);
static LIST_HEAD(wakeup_list);

static inline bool dubai_is_beta_user()
{
#ifdef CONFIG_LOG_EXCEPTION
	return (get_logusertype_flag() == BETA_USER || get_logusertype_flag() == OVERSEA_USER);
#else
	return false;
#endif
}

void dubai_log_packet_wakeup_stats(const char *tag, const char *key, int value)
{
	if (!dubai_is_beta_user() || !tag || !key) {
		return;
	}

	HWDUBAI_LOGE(tag, "%s=%d", key, value);
}
EXPORT_SYMBOL(dubai_log_packet_wakeup_stats);

static struct dubai_kworker_entry *dubai_find_kworker_entry(unsigned long address)
{
	struct dubai_kworker_entry *entry = NULL;

	hash_for_each_possible(kworker_hash_table, entry, hash, address) {
		if (entry->address == address)
			return entry;
	}

	entry = kzalloc(sizeof(struct dubai_kworker_entry), GFP_ATOMIC);
	if (entry == NULL) {
		DUBAI_LOGE("Failed to allocate memory");
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
	if (entry == NULL) {
		DUBAI_LOGE("Failed to find kworker entry");
		goto out;
	}

	entry->info.count++;
	entry->info.time += exit_time - enter_time;

out:
	mutex_unlock(&kworker_lock);
}
EXPORT_SYMBOL(dubai_log_kworker);

int dubai_get_kworker_info(void __user *argp)
{
	int ret = 0, count = 0;
	long long timestamp, size = 0;
	unsigned char *data = NULL;
	unsigned long bkt;
	struct dubai_kworker_entry *entry = NULL;
	struct hlist_node *tmp = NULL;
	struct buffered_log_entry *log_entry = NULL;
	struct dubai_kworker_transmit *transmit = NULL;

	if (!atomic_read(&log_stats_enable))
		return -EPERM;

	count = atomic_read(&kworker_count);
	if (count < 0)
		return -EINVAL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to get timestamp");
		return ret;
	}

	size = LOG_ENTRY_SIZE(struct dubai_kworker_transmit, struct dubai_kworker_info, count);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_KWORKER);
	if (log_entry == NULL) {
		DUBAI_LOGE("Failed to create buffered log entry");
		return -ENOMEM;
	}
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
			strncpy(entry->info.symbol, buffer, MAX_SYMBOL_LEN - 1);/* unsafe_function_ignore: strncpy */
		}

		if (transmit->count < count) {
			memcpy(data, &entry->info, sizeof(struct dubai_kworker_info));/* unsafe_function_ignore: memcpy */
			data += sizeof(struct dubai_kworker_info);
			transmit->count++;
		}
		entry->info.count = 0;
		entry->info.time = 0;
	}
	mutex_unlock(&kworker_lock);

	log_entry->length = LOG_ENTRY_SIZE(struct dubai_kworker_transmit,
							struct dubai_kworker_info, transmit->count);
	ret = send_buffered_log(log_entry);
	if (ret < 0)
		DUBAI_LOGE("Failed to send kworker log entry");
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
	if (entry == NULL) {
		DUBAI_LOGE("Failed to allocate memory");
		return NULL;
	}
	strncpy(entry->uevent.devpath, devpath, MAX_DEVPATH_LEN - 1);/* unsafe_function_ignore: strncpy */
	atomic_inc(&uevent_count);
	list_add_tail(&entry->list, &uevent_list);

	return entry;
}

void dubai_log_uevent(const char *devpath, unsigned int action) {
	struct dubai_uevent_entry *entry = NULL;

	if (!atomic_read(&log_stats_enable)
		|| devpath == NULL
		|| action >= KOBJ_MAX)
		return;

	mutex_lock(&uevent_lock);
	entry = dubai_find_uevent_entry(devpath);
	if (entry == NULL) {
		DUBAI_LOGE("Failed to find uevent entry");
		goto out;
	}
	(entry->uevent.actions[action])++;

out:
	mutex_unlock(&uevent_lock);
}
EXPORT_SYMBOL(dubai_log_uevent);

int dubai_get_uevent_info(void __user *argp)
{
	int ret = 0, count = 0;
	long long timestamp, size = 0;
	unsigned char *data = NULL;
	struct dubai_uevent_entry *entry = NULL, *tmp = NULL;
	struct buffered_log_entry *log_entry = NULL;
	struct dubai_uevent_transmit *transmit = NULL;

	if (!atomic_read(&log_stats_enable))
		return -EPERM;

	count = atomic_read(&uevent_count);
	if (count < 0)
		return -EINVAL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to get timestamp");
		return ret;
	}

	size = LOG_ENTRY_SIZE(struct dubai_uevent_transmit, struct dubai_uevent_info, count);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_UEVENT);
	if (log_entry == NULL) {
		DUBAI_LOGE("Failed to create buffered log entry");
		return -ENOMEM;
	}
	transmit = (struct dubai_uevent_transmit *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->action_count = KOBJ_MAX;
	transmit->count = 0;
	data = transmit->data;

	mutex_lock(&uevent_lock);
	list_for_each_entry_safe(entry, tmp, &uevent_list, list) {
		int i, total;

		for (i = 0, total = 0; i < KOBJ_MAX; i++) {
			total += entry->uevent.actions[i];
		}

		if (total == 0) {
			list_del_init(&entry->list);
			kfree(entry);
			atomic_dec(&uevent_count);
			continue;
		}
		if (transmit->count < count) {
			memcpy(data, &entry->uevent, sizeof(struct dubai_uevent_info));/* unsafe_function_ignore: memcpy */
			data += sizeof(struct dubai_uevent_info);
			transmit->count++;
		}
		memset(&(entry->uevent.actions), 0, KOBJ_MAX * sizeof(int));/* unsafe_function_ignore: memset */
	}
	mutex_unlock(&uevent_lock);

	log_entry->length = LOG_ENTRY_SIZE(struct dubai_uevent_transmit,
							struct dubai_uevent_info, transmit->count);
	ret = send_buffered_log(log_entry);
	if (ret < 0)
		DUBAI_LOGE("Failed to send uevent log entry");
	free_buffered_log_entry(log_entry);

	return ret;
}

int dubai_log_stats_enable(void __user *argp)
{
	int ret;
	bool enable = false;

	ret = get_enable_value(argp, &enable);
	if (ret == 0) {
		atomic_set(&log_stats_enable, enable ? 1 : 0);
		DUBAI_LOGI("Dubai log stats enable: %d", enable ? 1 : 0);
	}

	return ret;
}

static void dubai_parse_skb(const struct sk_buff *skb, int *protocol, int *port)
{
	const struct iphdr *iph = NULL;
	struct udphdr hdr;
	struct udphdr *hp = NULL;

	if (!skb || !protocol || !port) {
		return;
	}

	iph = ip_hdr(skb);
	if (!iph) {
		return;
	}

	*protocol = iph->protocol;
	if (iph->protocol == IPPROTO_TCP || iph->protocol == IPPROTO_UDP) {
		hp = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(hdr), &hdr);
		if (!hp) {
			return;
		}
		*port = (int)ntohs(hp->dest);
	}
}

void dubai_send_app_wakeup(const struct sk_buff *skb, unsigned int hook, unsigned int pf, int uid, int pid)
{
	int protocol = -1;
	int port = -1;
	const struct iphdr *iph = NULL;
	struct udphdr hdr;
	struct udphdr *hp = NULL;

	if (!dubai_is_beta_user()) {
		return;
	}

	if (!skb || pf != NFPROTO_IPV4 || uid < 0 || pid < 0) {
		return;
	}

	if (last_wakeup_time <= 0
		|| last_wakeup_time == last_app_wakeup_time
		|| (hisi_getcurtime() / NSEC_PER_MSEC - last_wakeup_time) > 500) {
		/* time limit is 500ms */
		return;
	}
	last_app_wakeup_time = last_wakeup_time;

	dubai_parse_skb(skb, &protocol, &port);
	if (protocol < 0 || port < 0) {
		return;
	}

	HWDUBAI_LOGE("DUBAI_TAG_APP_WAKEUP", "uid=%d pid=%d protocol=%d port=%d source=%s gpio=%d",
		uid, pid, protocol, port, last_wakeup_source, last_wakeup_gpio);
}
EXPORT_SYMBOL(dubai_send_app_wakeup);

static void dubai_set_wakeup_time(const char *source, int gpio)
{
	if (!dubai_is_beta_user() || !source) {
		return;
	}

	memset(last_wakeup_source, 0, sizeof(unsigned char)); /* unsafe_function_ignore: memset */
	strncpy(last_wakeup_source, source, WAKEUP_SOURCE_SIZE - 1); /* unsafe_function_ignore: strncpy */
	last_wakeup_gpio = gpio;
	last_wakeup_time = hisi_getcurtime() / NSEC_PER_MSEC;
}

/*
 * Caution: It's dangerous to use HWDUBAI_LOG in this function,
 * because it's in the SR process, and the HWDUBAI_LOG will wake up the kworker thread that will open irq
 */
void dubai_update_wakeup_info(const char *tag, const char *fmt, ...)
{
	va_list args;
	struct dubai_wakeup_entry *entry = NULL;
	const char *source = NULL;
	int gpio = -1;

	if (tag == NULL || strlen(tag) >= WAKEUP_TAG_SIZE) {
		DUBAI_LOGE("Invalid parameter");
		return;
	}

	entry = kzalloc(sizeof(struct dubai_wakeup_entry), GFP_ATOMIC);
	if (entry == NULL) {
		DUBAI_LOGE("Failed to allocate memory");
		return;
	}

	strncpy(entry->tag, tag, WAKEUP_TAG_SIZE - 1); /* unsafe_function_ignore: strncpy */
	va_start(args, fmt);
	vscnprintf(entry->msg, WAKEUP_MSG_SIZE, fmt, args);
	list_add_tail(&entry->list, &wakeup_list);
	if (!strcmp(entry->tag, KERNEL_WAKEUP_TAG)) {
		source = va_arg(args, const char *);
		gpio = va_arg(args, int);
		dubai_set_wakeup_time(source, gpio);
	}
	va_end(args);
}
EXPORT_SYMBOL(dubai_update_wakeup_info);

void dubai_set_rtc_timer(const char *name, int pid)
{
	if (name == NULL || pid <= 0) {
		DUBAI_LOGE("Invalid parameter, pid is %d", pid);
		return;
	}

	memcpy(dubai_rtc_timer.name, name, TASK_COMM_LEN - 1);/* unsafe_function_ignore: memcpy */
	dubai_rtc_timer.pid = pid;
}
EXPORT_SYMBOL(dubai_set_rtc_timer);

int dubai_set_brightness_enable(void __user *argp)
{
	int ret;
	bool enable = false;

	ret = get_enable_value(argp, &enable);
	if (ret == 0) {
		atomic_set(&dubai_backlight.enable, enable ? 1 : 0);
		DUBAI_LOGI("Dubai brightness enable: %d", enable ? 1 : 0);
	}

	return ret;
}

static int dubai_lock_set_brightness(uint32_t brightness)
{
	uint64_t current_time;

	if (brightness > MAX_BRIGHTNESS) {
		DUBAI_LOGE("Need valid data! brightness= %d", brightness);
		return -EINVAL;
	}

	current_time = div_u64(ktime_get_ns(), NSEC_PER_MSEC);

	if (dubai_backlight.last_brightness > 0) {
		uint64_t diff_time = current_time - dubai_backlight.last_time;

		dubai_backlight.sum_time += diff_time;
		dubai_backlight.sum_brightness_time += dubai_backlight.last_brightness * diff_time;
	}

	dubai_backlight.last_time = current_time;
	dubai_backlight.last_brightness = brightness;

	return 0;
}

void dubai_update_brightness(uint32_t brightness)
{
	if (!atomic_read(&dubai_backlight.enable))
		return;

	mutex_lock(&brightness_lock);
	dubai_lock_set_brightness(brightness);
	mutex_unlock(&brightness_lock);
}
EXPORT_SYMBOL(dubai_update_brightness);

int dubai_get_brightness_info(void __user *argp)
{
	int ret;
	uint32_t brightness;

	mutex_lock(&brightness_lock);
	ret = dubai_lock_set_brightness(dubai_backlight.last_brightness);
	if (ret == 0) {
		if (dubai_backlight.sum_time > 0 && dubai_backlight.sum_brightness_time > 0)
			brightness = (uint32_t)div64_u64(dubai_backlight.sum_brightness_time, dubai_backlight.sum_time);
		else
			brightness = 0;

		if (copy_to_user(argp, &brightness, sizeof(uint32_t)))
			ret = -EFAULT;
	}

	dubai_backlight.sum_time = 0;
	dubai_backlight.sum_brightness_time = 0;
	mutex_unlock(&brightness_lock);

	return ret;
}

int dubai_get_aod_duration(void __user *argp)
{
#ifdef SENSORHUB_DUBAI_INTERFACE
	int ret;
	uint64_t duration = 0;

	ret = iomcu_dubai_log_fetch(DUBAI_EVENT_AOD_TIME_STATISTICS, &duration, sizeof(duration));
	if (ret < 0) {
		DUBAI_LOGE("Read sensorhub aod data fail!");
		return -EFAULT;
	}

	if (copy_to_user(argp, &duration, sizeof(uint64_t)))
		return -EFAULT;

	return 0;
#else
	return -EOPNOTSUPP;
#endif
}

int dubai_get_fp_icon_stats(void __user *argp)
{
#ifdef SENSORHUB_DUBAI_INTERFACE
	int ret;
	uint32_t fp_icon_stats = 0;

	ret = iomcu_dubai_log_fetch(DUBAI_EVENT_FINGERPRINT_ICON_COUNT, &fp_icon_stats, sizeof(fp_icon_stats));
	if (ret < 0) {
		DUBAI_LOGE("Read sensorhub fp data fail!");
		return -EFAULT;
	}

	if (copy_to_user(argp, &fp_icon_stats, sizeof(fp_icon_stats)))
		return -EFAULT;

	return 0;
#else
	return -EOPNOTSUPP;
#endif
}

#ifdef SENSORHUB_DUBAI_INTERFACE
static int dubai_get_all_sensor_time(void __user *argp, size_t count, size_t packets)
{
	int rc, ret, length;
	size_t i, size;
	uint8_t *sensor_data = NULL;
	int32_t read_len, remain_len;
	struct dev_transmit_t *stat = NULL;

	if (copy_from_user(&length, argp, sizeof(int)))
		return -EFAULT;

	size = sizeof(struct dev_transmit_t) + count * sizeof(struct sensor_time);
	if (length < size)
		return -EINVAL;
	stat = kzalloc(size, GFP_KERNEL);
	if (stat == NULL) {
		return -ENOMEM;
	}

	remain_len = count * sizeof(struct sensor_time);
	sensor_data = (uint8_t *)(stat->data);
	for (i = 0; i < packets && remain_len > 0; i++) {
		read_len = (remain_len < SENSOR_TIME_BUFF_LEN) ? remain_len : SENSOR_TIME_BUFF_LEN;
		ret = iomcu_dubai_log_fetch(DUBAI_EVENT_ALL_SENSOR_TIME, sensor_data, read_len);
		if (ret < 0) {
			DUBAI_LOGE("Read sensorhub sensor time fail!");
			kfree(stat);
			return -EFAULT;
		}
		sensor_data += read_len;
		remain_len -= read_len;
	}
	stat->length = (int)count;

	rc = copy_to_user(argp, stat, size);
	kfree(stat);

	return rc;
}
#endif

int dubai_get_all_sensor_stats(void __user *argp)
{
#ifdef SENSORHUB_DUBAI_INTERFACE
	int ret;
	size_t count, packets;
	uint32_t stats = 0;

	ret = iomcu_dubai_log_fetch(DUBAI_EVENT_ALL_SENSOR_STATISTICS, &stats, sizeof(stats));
	if (ret < 0) {
		DUBAI_LOGE("Read sensorhub sensor statistics fail!");
		return -EFAULT;
	}
	count = (size_t)(stats & 0xFF);
	count = (count <= PHYSICAL_SENSOR_TYPE_MAX) ? count : PHYSICAL_SENSOR_TYPE_MAX;
	packets = (count * sizeof(struct sensor_time) + SENSOR_TIME_BUFF_LEN - 1) / SENSOR_TIME_BUFF_LEN;

	return dubai_get_all_sensor_time(argp, count, packets);
#else
	return -EOPNOTSUPP;
#endif
}

int dubai_get_swing_data(void __user *argp)
{
#ifdef SENSORHUB_DUBAI_INTERFACE
	int rc, ret, length;
	size_t size;
	struct swing_data *swing = NULL;
	struct dev_transmit_t *stat = NULL;

	if (copy_from_user(&length, argp, sizeof(int)))
		return -EFAULT;

	size = sizeof(struct dev_transmit_t) + sizeof(struct swing_data);
	if (length != size)
		return -EINVAL;
	stat = kzalloc(size, GFP_KERNEL);
	if (stat == NULL) {
		return -ENOMEM;
	}

	swing = (struct swing_data *)stat->data;
	ret = iomcu_dubai_log_fetch(DUBAI_EVENT_SWING, swing, sizeof(struct swing_data));
	if (ret < 0) {
		DUBAI_LOGE("Read swing camera data fail!");
		kfree(stat);
		return -EFAULT;
	}

	rc = copy_to_user(argp, stat, size);
	kfree(stat);

	return rc;
#else
	return -EOPNOTSUPP;
#endif
}

static void dubai_init_binder_client(struct list_head *head)
{
	struct dubai_binder_client_entry *client = NULL, *tmp = NULL;

	if (head != NULL) {
		list_for_each_entry_safe(client, tmp, head, node) {
			list_del_init(&client->node);
			kfree(client);
		}
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
	struct list_head *head, struct dubai_binder_proc *proc)
{
	struct dubai_binder_client_entry *entry = NULL;

	if ((head == NULL) || (proc == NULL)) {
		DUBAI_LOGE("Invalid param");
		return NULL;
	}

	list_for_each_entry(entry, head, node) {
		if (((entry->proc.pid == proc->pid) && (entry->proc.uid == proc->uid))
			|| (strncmp(entry->proc.name, proc->name, NAME_LEN - 1) == 0)) {
			return entry;
		}
	}
	return NULL;
}

static struct dubai_binder_client_entry *dubai_find_binder_client_entry(
	struct dubai_binder_stats_entry *stats, struct dubai_binder_proc *proc)
{
	struct dubai_binder_client_entry *entry = NULL;

	if ((stats == NULL) || (proc == NULL)) {
		DUBAI_LOGE("Invalid param");
		return NULL;
	}

	entry = dubai_check_binder_client_entry(&stats->client, proc);
	if (entry == NULL) {
		entry = kzalloc(sizeof(struct dubai_binder_client_entry), GFP_ATOMIC);
		if (entry == NULL) {
			DUBAI_LOGE("Failed to allocate binder client entry");
			return NULL;
		}
		memset(&entry->proc, 0, sizeof(struct dubai_binder_proc));/* unsafe_function_ignore: memset */
		entry->count = 0;
		list_add_tail(&entry->node, &stats->client);
		stats->count++;
		atomic_inc(&binder_client_count);
	}
	return entry;
}


static struct dubai_binder_stats_entry *dubai_check_binder_stats_entry(struct dubai_binder_proc *proc)
{
	struct dubai_binder_stats_entry *entry = NULL;

	if (proc == NULL) {
		DUBAI_LOGE("Invalid param");
		return NULL;
	}

	hash_for_each_possible(binder_stats_hash_table, entry, hash, proc->pid) {//lint !e666
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

	if (proc == NULL) {
		DUBAI_LOGE("Invalid param");
		return NULL;
	}

	entry = dubai_check_binder_stats_entry(proc);
	if (entry == NULL) {
		entry = kzalloc(sizeof(struct dubai_binder_stats_entry), GFP_ATOMIC);
		if (entry == NULL) {
			DUBAI_LOGE("Failed to allocate binder stats entry");
			return NULL;
		}
		memset(&entry->proc, 0, sizeof(struct dubai_binder_proc));/* unsafe_function_ignore: memset */
		entry->count = 0;
		INIT_LIST_HEAD(&entry->client);
		hash_add(binder_stats_hash_table, &entry->hash, proc->pid);//lint !e666
	}
	return entry;
}

static int dubai_get_binder_proc_name(struct dubai_binder_proc *service, struct dubai_binder_proc *client)
{
	struct dubai_binder_stats_entry *stats = NULL;
	struct dubai_binder_client_entry *entry = NULL;
	int rc = 0;
	struct process_name s, c;

	if ((service == NULL) || (client == NULL)) {
		DUBAI_LOGE("Invalid param");
		return -1;
	}

	if (!mutex_trylock(&binder_stats_hash_lock))
		return -1;

	stats = dubai_check_binder_stats_entry(service);
	if (stats == NULL) {
		mutex_unlock(&binder_stats_hash_lock);//lint !e455

		memset(&s, 0, sizeof(struct process_name));/* unsafe_function_ignore: memset */
		s.pid = service->pid;
		memset(&c, 0, sizeof(struct process_name));/* unsafe_function_ignore: memset */
		c.pid = client->pid;
		rc = dubai_get_process_name(&s);
		if (rc <= 0) {
			DUBAI_LOGE("Failed to get service name: %d, %d", service->uid, service->pid);
			return -1;
		}
		rc = dubai_get_process_name(&c);
		if (rc <= 0) {
			DUBAI_LOGE("Failed to get client name: %d, %d", client->uid, client->pid);
			return -1;
		}
		strncpy(service->name, s.name, NAME_LEN - 1);/* unsafe_function_ignore: strncpy */
		strncpy(client->name, c.name, NAME_LEN - 1);/* unsafe_function_ignore: strncpy */
	} else {
		strncpy(service->name, stats->proc.name, NAME_LEN - 1);/* unsafe_function_ignore: strncpy */
		entry = dubai_check_binder_client_entry(&stats->client, client);
		if (entry == NULL) {
			mutex_unlock(&binder_stats_hash_lock);//lint !e455

			memset(&c, 0, sizeof(struct process_name));/* unsafe_function_ignore: memset */
			c.pid = client->pid;
			rc = dubai_get_process_name(&c);
			if (rc <= 0) {
				DUBAI_LOGE("Failed to get client name: %d, %d", client->uid, client->pid);
				return -1;
			}
			strncpy(client->name, c.name, NAME_LEN - 1);/* unsafe_function_ignore: strncpy */
		} else {
			strncpy(client->name, entry->proc.name, NAME_LEN - 1);/* unsafe_function_ignore: strncpy */
			mutex_unlock(&binder_stats_hash_lock);//lint !e455
		}
	}
	return 0;
}

static void dubai_add_binder_stats(struct dubai_binder_proc *service, struct dubai_binder_proc *client)
{
	struct dubai_binder_stats_entry *stats = NULL;
	struct dubai_binder_client_entry *entry = NULL;

	if ((service == NULL) || (client == NULL)) {
		DUBAI_LOGE("Invalid param");
		return;
	}

	if (!mutex_trylock(&binder_stats_hash_lock))
		return;

	stats = dubai_find_binder_stats_entry(service);
	if (stats == NULL) {
		DUBAI_LOGE("Failed to find binder stats entry");
		goto out;
	}
	stats->proc = *service;
	entry = dubai_find_binder_client_entry(stats, client);
	if (entry == NULL) {
		DUBAI_LOGE("Failed to add binder client entry");
		goto out;
	}
	entry->proc = *client;
	entry->count++;

out:
	mutex_unlock(&binder_stats_hash_lock);//lint !e455
}

static void dubai_get_binder_client(
	struct dubai_binder_stats_entry *stats, struct dubai_binder_stats_transmit *transmit, int count)
{
	struct dubai_binder_client_entry *client = NULL, *temp = NULL;
	struct dubai_binder_stats_info info;
	unsigned char *data = NULL;

	if ((stats == NULL) || (transmit == NULL)) {
		DUBAI_LOGE("Invalid param");
		return;
	}

	data = transmit->data + transmit->count * sizeof(struct dubai_binder_stats_info);
	info.service = stats->proc;
	list_for_each_entry_safe(client, temp, &stats->client, node) {
		DUBAI_LOGD("service: %d, %s, %d, %d, client: %d, %s, %d, %d",
			stats->count, stats->proc.name, stats->proc.uid, stats->proc.pid,
			client->count, client->proc.name, client->proc.uid, client->proc.pid);

		info.client = client->proc;
		info.count = client->count;
		if (transmit->count < count) {
			memcpy(data, &info, sizeof(struct dubai_binder_stats_info));/* unsafe_function_ignore: memcpy */
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

	if (transmit == NULL) {
		DUBAI_LOGE("Invalid param");
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

	if (transmit == NULL) {
		DUBAI_LOGE("Invalid param");
		return;
	}

	list_for_each_entry_safe(stats, tmp, &binder_stats_died_list, died) {
		dubai_get_binder_client(stats, transmit, count);
		list_del_init(&stats->died);
		kfree(stats);
	}
}

int dubai_get_binder_stats(void __user *argp)
{
	int ret = 0, count = 0;
	long long timestamp, size = 0;
	struct buffered_log_entry *log_entry = NULL;
	struct dubai_binder_stats_transmit *transmit = NULL;

	if (!atomic_read(&binder_stats_enable))
		return -EPERM;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to get timestamp");
		return ret;
	}

	count = atomic_read(&binder_client_count);
	size = LOG_ENTRY_SIZE(struct dubai_binder_stats_transmit, struct dubai_binder_stats_info, count);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_BINDER_STATS);
	if (log_entry == NULL) {
		dubai_init_binder_stats();
		DUBAI_LOGE("Failed to create buffered log entry");
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
		log_entry->length = LOG_ENTRY_SIZE(struct dubai_binder_stats_transmit, struct dubai_binder_stats_info, transmit->count);
		ret = send_buffered_log(log_entry);
		if (ret < 0)
			DUBAI_LOGE("Failed to send binder stats log entry: %d", ret);
	}
	free_buffered_log_entry(log_entry);

	return ret;
}

void dubai_log_binder_stats(int reply, uid_t c_uid, int c_pid, uid_t s_uid, int s_pid)
{
	struct dubai_binder_proc client, service;

	if (reply || !atomic_read(&binder_stats_enable))
		return;

	if (c_uid > 1000000 || s_uid > 1000000)
		return;

	memset(&service, 0, sizeof(struct dubai_binder_proc));/* unsafe_function_ignore: memset */
	service.uid = s_uid;
	service.pid = s_pid;
	memset(&client, 0, sizeof(struct dubai_binder_proc));/* unsafe_function_ignore: memset */
	client.uid = c_uid;
	client.pid = c_pid;

	if (dubai_get_binder_proc_name(&service, &client) == 0) {
		dubai_add_binder_stats(&service, &client);
	}
	DUBAI_LOGD("SNAME: %s, SUID: %d, SPID: %d, CNAME: %s, CUID: %d, CPID: %d",
		service.name, s_uid, s_pid, client.name, c_uid, c_pid);
}
EXPORT_SYMBOL(dubai_log_binder_stats);

int dubai_binder_stats_enable(void __user *argp)
{
	int ret;
	bool enable = false;

	ret = get_enable_value(argp, &enable);
	if (ret == 0) {
		DUBAI_LOGI("Set binder stats enable: %d", enable ? 1 : 0);
		atomic_set(&binder_stats_enable, enable ? 1 : 0);
		if (!enable) {
			dubai_init_binder_stats();
		}
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
			DUBAI_LOGD("service: %d, %s, %d, %d",
				stats->count, stats->proc.name, stats->proc.uid, stats->proc.pid);

			hash_del(&stats->hash);
			list_add_tail(&stats->died, &binder_stats_died_list);
			break;
		}
	}
	mutex_unlock(&binder_stats_hash_lock);
}

int dubai_get_sensorhub_type_list(void __user * argp)
{
#ifdef CONFIG_INPUTHUB_20
	int i, j, ret = 0, count = 0;
	long long timestamp, size = 0;
	struct dubai_sensorhub_type_list *transmit = NULL;
	struct buffered_log_entry *log_entry = NULL;
	const struct app_link_info *info = NULL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to get timestamp");
		return ret;
	}

	size = LOG_ENTRY_SIZE(struct dubai_sensorhub_type_list, struct dubai_sensorhub_type_info,
		SENSORHUB_TYPE_END - SENSORHUB_TYPE_BEGIN);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_SENSORHUB_TYPE_LIST);
	if (log_entry == NULL) {
		DUBAI_LOGE("Failed to create buffered log entry");
		ret = -ENOMEM;
		return ret;
	}

	transmit = (struct dubai_sensorhub_type_list *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->tag_count = SENSORHUB_TAG_NUM_MAX;
	for (i = SENSORHUB_TYPE_BEGIN; i < SENSORHUB_TYPE_END; i++) {
		info = get_app_link_info(i);
		if (info && info->used_sensor_cnt > 0 && info->used_sensor_cnt <= SENSORHUB_TAG_NUM_MAX) {
			transmit->data[count].sensorhub_type = info->hal_sensor_type;
			for (j = 0; j < info->used_sensor_cnt; j++) {
				transmit->data[count].data[j] = info->used_sensor[j];
			}
			count++;
		}
	}
	if (count == 0) {
		DUBAI_LOGE("No data");
		ret = -EINVAL;
		goto exit;
	}
	transmit->size = count;
	log_entry->length = LOG_ENTRY_SIZE(struct dubai_sensorhub_type_list,
				struct dubai_sensorhub_type_info, transmit->size);
	ret = send_buffered_log(log_entry);
	if (ret < 0) {
		DUBAI_LOGE("Failed to send sensorhub type list entry");
	}

exit:
	free_buffered_log_entry(log_entry);

	return ret;
#else
	return -EOPNOTSUPP;
#endif
}

int dubai_get_ws_lasting_name(void __user * argp)
{
	int ret = 0;
	size_t i;
	long long timestamp, size = 0;
	char *data = NULL;
	struct dubai_ws_lasting_transmit *transmit = NULL;
	struct buffered_log_entry *log_entry = NULL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to get timestamp");
		return ret;
	}

	size = LOG_ENTRY_SIZE(struct dubai_ws_lasting_transmit, struct dubai_ws_lasting_name, MAX_WS_NAME_COUNT);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_WS_LASTING_NAME);
	if (log_entry == NULL) {
		DUBAI_LOGE("Failed to create buffered log entry");
		ret = -ENOMEM;
		return ret;
	}
	transmit = (struct dubai_ws_lasting_transmit *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->count = MAX_WS_NAME_COUNT;
	data = transmit->data;

	ret = wakeup_source_getlastingname(data, MAX_WS_NAME_LEN, MAX_WS_NAME_COUNT);
	if ((ret <= 0) || (ret > MAX_WS_NAME_COUNT)) {
		DUBAI_LOGE("Fail to call wakeup_source_getlastingname.");
		ret = -ENOMEM;
		goto exit;
	}

	transmit->count = ret;
	log_entry->length = LOG_ENTRY_SIZE(struct dubai_ws_lasting_transmit,
				struct dubai_ws_lasting_name, transmit->count);
	ret = send_buffered_log(log_entry);
	if (ret < 0) {
		DUBAI_LOGE("Failed to send wakeup source log entry");
	}

exit:
	free_buffered_log_entry(log_entry);

	return ret;
}

int dubai_get_rss(void __user *argp)
{
	unsigned long rss = get_mm_rss(current->mm);
	long long size = rss * PAGE_SIZE;

	if (copy_to_user(argp, &size, sizeof(long long))) {
		DUBAI_LOGE("Failed to copy_to_user %lld.", size);
		return -EINVAL;
	}

	return 0;
}

static int dubai_stats_process_notifier(struct notifier_block *self, unsigned long cmd, void *v)
{
	struct task_struct *task = v;

	if (task == NULL)
		goto exit;

	if (task->tgid == task->pid)
		dubai_process_binder_died(task->tgid);

exit:
	return NOTIFY_OK;
}

static void dubai_send_wakeup_info(void)
{
	struct dubai_wakeup_entry *entry = NULL, *tmp = NULL;

	list_for_each_entry_safe(entry, tmp, &wakeup_list, list) {
		if (!strcmp(entry->tag, KERNEL_WAKEUP_TAG)
			&& strstr(entry->msg, "irq=RTC0")
			&& dubai_rtc_timer.pid > 0) {
			HWDUBAI_LOGE("DUBAI_TAG_RTC_WAKEUP", "name=%s pid=%d", dubai_rtc_timer.name, dubai_rtc_timer.pid);
		} else {
			HWDUBAI_LOGE(entry->tag, "%s", entry->msg);
		}
		list_del_init(&entry->list);
		kfree(entry);
	}
	memset(&dubai_rtc_timer, 0, sizeof(struct dubai_rtc_timer_info)); /* unsafe_function_ignore: memset */
}

static int dubai_pm_notify(struct notifier_block *nb,
			unsigned long mode, void *data)
{
	switch (mode) {
	case PM_SUSPEND_PREPARE:
		HWDUBAI_LOGE("DUBAI_TAG_KERNEL_SUSPEND", "");
		break;
	case PM_POST_SUSPEND:
		dubai_send_wakeup_info();
		HWDUBAI_LOGE("DUBAI_TAG_KERNEL_RESUME", "");
		break;
	default:
		break;
	}

	return 0;
}

static struct notifier_block dubai_pm_nb = {
	.notifier_call = dubai_pm_notify,
};

static struct notifier_block process_notifier_block = {
	.notifier_call	= dubai_stats_process_notifier,
};

void dubai_stats_init(void)
{
	atomic_set(&binder_stats_enable, 0);
	atomic_set(&binder_client_count, 0);
	hash_init(binder_stats_hash_table);
	atomic_set(&log_stats_enable, 0);
	hash_init(kworker_hash_table);
	atomic_set(&kworker_count, 0);
	atomic_set(&uevent_count, 0);
	atomic_set(&dubai_backlight.enable, 0);
	dubai_backlight.last_time = 0;
	dubai_backlight.last_brightness = 0;
	dubai_backlight.sum_time = 0;
	dubai_backlight.sum_brightness_time = 0;
	memset(&dubai_rtc_timer, 0, sizeof(struct dubai_rtc_timer_info));/* unsafe_function_ignore: memset */
	register_pm_notifier(&dubai_pm_nb);
	profile_event_register(PROFILE_TASK_EXIT, &process_notifier_block);
	DUBAI_LOGI("DUBAI stats initialize success");
}

void dubai_stats_exit(void)
{
	struct dubai_kworker_entry *kworker = NULL;
	struct hlist_node *tmp = NULL;
	struct dubai_uevent_entry *uevent = NULL, *temp = NULL;
	struct dubai_wakeup_entry *wakeup = NULL, *wakeup_temp = NULL;
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
	list_for_each_entry_safe(wakeup, wakeup_temp, &wakeup_list, list) {
		list_del_init(&wakeup->list);
		kfree(wakeup);
	}

	unregister_pm_notifier(&dubai_pm_nb);
}
