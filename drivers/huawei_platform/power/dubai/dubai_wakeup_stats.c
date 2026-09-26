#include <linux/suspend.h>

#include <chipset_common/dubai/dubai_plat.h>
#include <huawei_platform/power/dubai/dubai_wakeup_stats.h>

#define WAKEUP_SOURCE_SIZE						32

static atomic_t wakeup_enable = ATOMIC_INIT(0);
static bool wakeup_state = false;
static size_t wakeup_num = 0;
static struct dubai_wakeup_info wakeup_stats[DUBAI_WAKEUP_NUM_MAX];
static const char *rtc_wakeup_source[] = {"RTC0", "rtc0"};

static void dubai_reset_wakeup_stats(void)
{
	wakeup_state = false;
	wakeup_num = 0;
	memset(wakeup_stats, 0, DUBAI_WAKEUP_NUM_MAX * sizeof(struct dubai_wakeup_info));
}

static bool is_rtc_wakeup(const char *irq)
{
	size_t i;

	for (i = 0; i < sizeof(rtc_wakeup_source) / sizeof(const char *); i++) {
		if (strstr(irq, rtc_wakeup_source[i]))
			return true;
	}
	return false;
}

static void set_wakeup_info(enum dubai_wakeup_type type, const char *tag, const char *msg)
{
	if (wakeup_num >= DUBAI_WAKEUP_NUM_MAX) {
		dubai_err("Invalid wakeup num: %d", wakeup_num);
		return;
	}

	strncpy(wakeup_stats[wakeup_num].tag, tag, DUBAI_WAKEUP_TAG_LENGTH - 1);
	strncpy(wakeup_stats[wakeup_num].msg, msg, DUBAI_WAKEUP_MSG_LENGTH - 1);
	wakeup_stats[wakeup_num].type = type;
	wakeup_num++;
}

/*
 * Caution: It's dangerous to use HWDUBAI_LOG in this function,
 * because it's in the SR process, and the HWDUBAI_LOG will wake up the kworker thread that will open irq
 */
void dubai_log_wakeup_info(const char *tag, const char *fmt, ...)
{
	va_list args;
	char msg[DUBAI_WAKEUP_MSG_LENGTH] = {0};

	if (atomic_read(&wakeup_enable) == 0) {
		return;
	}

	if (!tag || (strlen(tag) >= DUBAI_WAKEUP_TAG_LENGTH)) {
		dubai_err("Invalid parameter");
		return;
	}

	va_start(args, fmt);
	vscnprintf(msg, DUBAI_WAKEUP_MSG_LENGTH, fmt, args);
	set_wakeup_info(DUBAI_WAKEUP_TYPE_OTHERS, tag, msg);
	va_end(args);
}
EXPORT_SYMBOL(dubai_log_wakeup_info); //lint !e580

/*
 * Caution: It's dangerous to use HWDUBAI_LOG in this function,
 * because it's in the SR process, and the HWDUBAI_LOG will wake up the kworker thread that will open irq
 */
void dubai_log_irq_wakeup(enum dubai_irq_wakeup_type type, const char *source, int gpio)
{
	char wakeup_source[WAKEUP_SOURCE_SIZE] = {0};
	char msg[DUBAI_WAKEUP_MSG_LENGTH] = {0};
	enum dubai_wakeup_type wakeup_type = DUBAI_WAKEUP_TYPE_INVALID;

	if (atomic_read(&wakeup_enable) == 0) {
		return;
	}

	if (!source) {
		dubai_err("Invalid parameter");
		return;
	}

	switch (type) {
	case DUBAI_IRQ_WAKEUP_TYPE_AP:
		if (gpio >= 0) {
			snprintf(wakeup_source, WAKEUP_SOURCE_SIZE - 1, "GPIO%d", gpio);
			wakeup_type = DUBAI_WAKEUP_TYPE_GPIO;
		} else if (is_rtc_wakeup(source)) {
			snprintf(wakeup_source, WAKEUP_SOURCE_SIZE - 1, "RTC0");
			wakeup_type = DUBAI_WAKEUP_TYPE_RTC;
		} else {
			snprintf(wakeup_source, WAKEUP_SOURCE_SIZE - 1, "%s", source);
			wakeup_type = DUBAI_WAKEUP_TYPE_OTHERS;
		}

		if (!wakeup_state) {
			dubai_wakeup_notify(wakeup_source);
			wakeup_state = true;
		}
		break;
	case DUBAI_IRQ_WAKEUP_TYPE_LPMCU:
		if (wakeup_state)
			return;

		snprintf(wakeup_source, WAKEUP_SOURCE_SIZE - 1, "%s", source);
		wakeup_type = DUBAI_WAKEUP_TYPE_OTHERS;
		break;
	default:
		return;
	}

	snprintf(msg, DUBAI_WAKEUP_MSG_LENGTH - 1, "name=%s", wakeup_source);
	set_wakeup_info(wakeup_type, "DUBAI_TAG_AP_WAKE_IRQ", msg);
}
EXPORT_SYMBOL(dubai_log_irq_wakeup); //lint !e580

static int dubai_get_wakeup_info(struct dubai_wakeup_info *data, size_t num)
{
	if (!data || (num < wakeup_num) || (num > DUBAI_WAKEUP_NUM_MAX)) {
		dubai_err("Invalid parameter");
		return -1;
	}

	if (wakeup_num > 0)
		memcpy(data, wakeup_stats, wakeup_num * sizeof(struct dubai_wakeup_info));
	return 0;
}

static int dubai_suspend_notify(unsigned long mode)
{
	switch (mode) {
	case PM_SUSPEND_PREPARE:
		dubai_reset_wakeup_stats();
		atomic_set(&wakeup_enable, 1);
		break;
	case PM_POST_SUSPEND:
		atomic_set(&wakeup_enable, 0);
		break;
	default:
		break;
	}
	return 0;
}

static struct dubai_wakeup_stats_ops wakeup_ops = {
	.suspend_notify = dubai_suspend_notify,
	.get_stats = dubai_get_wakeup_info,
};

int dubai_wakeup_stats_init(void)
{
	dubai_register_module_ops(DUBAI_MODULE_WAKEUP, &wakeup_ops);
	return 0;
}

void dubai_wakeup_stats_exit(void)
{
	dubai_unregister_module_ops(DUBAI_MODULE_WAKEUP);
	dubai_reset_wakeup_stats();
}
