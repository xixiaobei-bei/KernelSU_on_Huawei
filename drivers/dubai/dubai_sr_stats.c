#include "dubai_sr_stats.h"

#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/wakeup_reason.h>
#include <net/ip.h>
#include <uapi/linux/udp.h>

#include <chipset_common/dubai/dubai_ioctl.h>
#include <huawei_platform/log/hwlog_kernel.h>

#include "utils/buffered_log_sender.h"
#include "utils/dubai_utils.h"

#define IOC_LASTING_WAKELOCK_REQUEST DUBAI_SR_DIR_IOC(W, 1, long long)

#define MAX_WS_NAME_LEN						64
#define MAX_WS_NAME_COUNT					128
#define WAKEUP_SOURCE_SIZE					32
#define APP_WAKEUP_DELAY					(HZ / 2)

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

static int dubai_get_ws_lasting_name(void __user *argp);

static struct dubai_rtc_timer_info rtc_timer_info;
static char suspend_abort_reason[MAX_SUSPEND_ABORT_LEN];
static struct dubai_wakeup_stats_ops *wakeup_stats_op = NULL;
static unsigned char last_wakeup_source[WAKEUP_SOURCE_SIZE] = {0};
static unsigned long last_app_wakeup_time = 0;
static unsigned long last_wakeup_time = 0;

long dubai_ioctl_sr(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_LASTING_WAKELOCK_REQUEST:
		rc = dubai_get_ws_lasting_name(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

void dubai_log_packet_wakeup_stats(const char *tag, const char *key, int value)
{
	if (!dubai_is_beta_user() || !tag || !key)
		return;

	HWDUBAI_LOGE(tag, "%s=%d", key, value);
}
EXPORT_SYMBOL(dubai_log_packet_wakeup_stats);

static void dubai_parse_wakeup_skb(const struct sk_buff *skb, int *protocol, int *port)
{
	const struct iphdr *iph = NULL;
	struct udphdr hdr;
	struct udphdr *hp = NULL;

	if (!skb || !protocol || !port)
		return;

	iph = ip_hdr(skb);
	if (!iph)
		return;

	*protocol = iph->protocol;
	if ((iph->protocol == IPPROTO_TCP) || (iph->protocol == IPPROTO_UDP)) {
		hp = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(hdr), &hdr);
		if (!hp)
			return;
		*port = (int)ntohs(hp->dest);
	}
}

void dubai_wakeup_notify(const char *source)
{
	if (!dubai_is_beta_user() || !source)
		return;

	memset(last_wakeup_source, 0, WAKEUP_SOURCE_SIZE * sizeof(unsigned char));
	strncpy(last_wakeup_source, source, WAKEUP_SOURCE_SIZE - 1);
	last_wakeup_time = jiffies;
}
EXPORT_SYMBOL(dubai_wakeup_notify); //lint !e580

void dubai_send_app_wakeup(const struct sk_buff *skb, unsigned int hook, unsigned int pf, int uid, int pid)
{
	int protocol = -1;
	int port = -1;

	if (!dubai_is_beta_user())
		return;

	if (!skb || (pf != NFPROTO_IPV4) || (uid < 0) || (pid < 0))
		return;

	if ((last_wakeup_time <= 0)
		|| (last_wakeup_time == last_app_wakeup_time)
		|| time_is_before_jiffies(last_wakeup_time + APP_WAKEUP_DELAY))
		return;

	last_app_wakeup_time = last_wakeup_time;

	dubai_parse_wakeup_skb(skb, &protocol, &port);
	if ((protocol < 0) || (port < 0))
		return;

	HWDUBAI_LOGE("DUBAI_TAG_APP_WAKEUP", "uid=%d pid=%d protocol=%d port=%d source=%s",
		uid, pid, protocol, port, last_wakeup_source);
}
EXPORT_SYMBOL(dubai_send_app_wakeup); //lint !e580

/*
 * Caution: It's dangerous to use HWDUBAI_LOG in this function,
 * because it's in the SR process, and the HWDUBAI_LOG will wake up the kworker thread that will open irq
 */
void dubai_update_suspend_abort_reason(const char *reason)
{
	size_t suspend_abort_reason_len, new_reason_len;

	if (!reason)
		return;

	suspend_abort_reason_len = strlen(suspend_abort_reason);
	new_reason_len = strlen(reason);
	if ((suspend_abort_reason_len + new_reason_len) >= (MAX_SUSPEND_ABORT_LEN - 1))
		return;

	strncat(suspend_abort_reason, reason, new_reason_len);
	suspend_abort_reason[suspend_abort_reason_len + new_reason_len] = ' ';
	suspend_abort_reason[MAX_SUSPEND_ABORT_LEN - 1] = '\0';
}
EXPORT_SYMBOL(dubai_update_suspend_abort_reason); //lint !e580

void dubai_set_rtc_timer(const char *name, int pid)
{
	if (!name || (pid <= 0)) {
		dubai_err("Invalid parameter, pid is %d", pid);
		return;
	}

	memcpy(rtc_timer_info.name, name, TASK_COMM_LEN - 1);
	rtc_timer_info.pid = pid;
}
EXPORT_SYMBOL(dubai_set_rtc_timer); //lint !e580

static int dubai_get_ws_lasting_name(void __user * argp)
{
	int ret = 0;
	long long timestamp, size = 0;
	char *data = NULL;
	struct dubai_ws_lasting_transmit *transmit = NULL;
	struct buffered_log_entry *log_entry = NULL;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		dubai_err("Failed to get timestamp");
		return ret;
	}

	size = cal_log_entry_len(struct dubai_ws_lasting_transmit, struct dubai_ws_lasting_name, MAX_WS_NAME_COUNT);
	log_entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_WS_LASTING_NAME);
	if (!log_entry) {
		dubai_err("Failed to create buffered log entry");
		ret = -ENOMEM;
		return ret;
	}
	transmit = (struct dubai_ws_lasting_transmit *)log_entry->data;
	transmit->timestamp = timestamp;
	transmit->count = MAX_WS_NAME_COUNT;
	data = transmit->data;

	ret = wakeup_source_getlastingname(data, MAX_WS_NAME_LEN, MAX_WS_NAME_COUNT);
	if ((ret <= 0) || (ret > MAX_WS_NAME_COUNT)) {
		dubai_err("Fail to call wakeup_source_getlastingname.");
		ret = -ENOMEM;
		goto exit;
	}

	transmit->count = ret;
	log_entry->length = cal_log_entry_len(struct dubai_ws_lasting_transmit,
		struct dubai_ws_lasting_name, transmit->count);
	ret = send_buffered_log(log_entry);
	if (ret < 0)
		dubai_err("Failed to send wakeup source log entry");

exit:
	free_buffered_log_entry(log_entry);
	return ret;
}

static void dubai_send_wakeup_stats(void)
{
	int i;
	struct dubai_wakeup_info *wakeup_info = NULL;

	if (!wakeup_stats_op || !wakeup_stats_op->get_stats) {
		dubai_info("Wakeup stats not supported");
		return;
	}

	wakeup_info = kzalloc(DUBAI_WAKEUP_NUM_MAX * sizeof(struct dubai_wakeup_info), GFP_KERNEL);
	if (!wakeup_info) {
		dubai_err("Failed to alloc");
		return;
	}

	if (!wakeup_stats_op->get_stats(wakeup_info, DUBAI_WAKEUP_NUM_MAX)) {
		for (i = 0; i < DUBAI_WAKEUP_NUM_MAX; i++) {
			if (wakeup_info[i].type == DUBAI_WAKEUP_TYPE_INVALID)
				continue;

			if ((wakeup_info[i].type == DUBAI_WAKEUP_TYPE_RTC) && (rtc_timer_info.pid > 0))
				HWDUBAI_LOGE("DUBAI_TAG_RTC_WAKEUP", "name=%s pid=%d", rtc_timer_info.name, rtc_timer_info.pid);
			HWDUBAI_LOGE(wakeup_info[i].tag, wakeup_info[i].msg);
		}
	} else {
		dubai_err("Failed to get stats");
	}
	kfree(wakeup_info);
	memset(&rtc_timer_info, 0, sizeof(struct dubai_rtc_timer_info));
}

static void dubai_send_suspend_abort_reason(void)
{
	if (strlen(suspend_abort_reason))
		HWDUBAI_LOGE("DUBAI_TAG_SUSPEND_ABORT", "reason=%s", suspend_abort_reason);
	memset(suspend_abort_reason, 0, MAX_SUSPEND_ABORT_LEN);
}

static int dubai_pm_notify(struct notifier_block __always_unused *nb, unsigned long mode, void __always_unused *data)
{
	switch (mode) {
	case PM_SUSPEND_PREPARE:
		HWDUBAI_LOGE("DUBAI_TAG_KERNEL_SUSPEND", "");
		break;
	case PM_POST_SUSPEND:
		dubai_send_suspend_abort_reason();
		dubai_send_wakeup_stats();
		HWDUBAI_LOGE("DUBAI_TAG_KERNEL_RESUME", "");
		break;
	default:
		break;
	}

	if (wakeup_stats_op && wakeup_stats_op->suspend_notify)
		wakeup_stats_op->suspend_notify(mode);
	return 0;
}

static struct notifier_block dubai_pm_nb = {
	.notifier_call = dubai_pm_notify,
};

int dubai_wakeup_register_ops(struct dubai_wakeup_stats_ops *op)
{
	if (!wakeup_stats_op)
		wakeup_stats_op = op;
	return 0;
}

int dubai_wakeup_unregister_ops(void)
{
	if (wakeup_stats_op)
		wakeup_stats_op = NULL;
	return 0;
}

void dubai_sr_stats_init(void)
{
	int ret;

	ret = register_pm_notifier(&dubai_pm_nb);
	if (ret)
		dubai_err("Failed to register pm notifier");
}

void dubai_sr_stats_exit(void)
{
	unregister_pm_notifier(&dubai_pm_nb);
}
