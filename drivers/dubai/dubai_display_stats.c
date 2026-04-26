#include "dubai_display_stats.h"

#include <linux/slab.h>

#include <chipset_common/dubai/dubai_ioctl.h>

#include "utils/dubai_utils.h"

#define IOC_BRIGHTNESS_ENABLE DUBAI_DISPLAY_DIR_IOC(W, 1, bool)
#define IOC_BRIGHTNESS_GET DUBAI_DISPLAY_DIR_IOC(R, 2, uint32_t)
#define IOC_DISPLAY_HIST_POLICY_GET DUBAI_DISPLAY_DIR_IOC(R, 3, int)
#define IOC_DISPLAY_HIST_LEN_GET DUBAI_DISPLAY_DIR_IOC(R, 4, size_t)
#define IOC_DISPLAY_HIST_ENABLE DUBAI_DISPLAY_DIR_IOC(W, 5, bool)
#define IOC_DISPLAY_HIST_DATA_GET DUBAI_DISPLAY_DIR_IOC(R, 6, struct dev_transmit_t)

#define CALCULATE_HIST_DELTA(a, b) (((a) < (b)) ? ((b) - (a)) : 0)

#define MAX_BRIGHTNESS			10000
#define DISP_HIST_TIME_PERIOD	100

struct dubai_brightness_info {
	atomic_t enable;
	uint64_t last_time;
	uint32_t last_brightness;
	uint64_t sum_time;
	uint64_t sum_brightness_time;
};

static int dubai_set_brightness_enable(void __user *argp);
static int dubai_get_brightness_info(void __user *argp);

static struct dubai_brightness_info dubai_backlight;
static DEFINE_MUTEX(brightness_lock);

static void dubai_display_wq_handler(struct work_struct *work);
static DECLARE_DELAYED_WORK(display_hist_work, dubai_display_wq_handler);
static void dubai_schedule_display_hist_work(void);
static int dubai_get_display_hist_policy(void __user *argp);
static int dubai_get_display_hist_len(void __user *argp);
static int dubai_set_display_hist_enable(void __user *argp);
static int dubai_get_display_hist_data(void __user *argp);
static int dubai_get_display_hist_delta(hist_data *delta, int size);
static void dubai_calculate_delta(const hist_data *last, const hist_data *curr, hist_data *delta, int size);

static bool g_display_hist_enable = false;
static bool g_display_panel_state = true;
static struct dubai_display_hist_ops *display_hist_op = NULL;

long dubai_ioctl_display(unsigned int cmd, void __user *argp)
{
	int rc = 0;

	switch (cmd) {
	case IOC_BRIGHTNESS_ENABLE:
		rc = dubai_set_brightness_enable(argp);
		break;
	case IOC_BRIGHTNESS_GET:
		rc = dubai_get_brightness_info(argp);
		break;
	case IOC_DISPLAY_HIST_POLICY_GET:
		rc = dubai_get_display_hist_policy(argp);
		break;
	case IOC_DISPLAY_HIST_LEN_GET:
		rc = dubai_get_display_hist_len(argp);
		break;
	case IOC_DISPLAY_HIST_ENABLE:
		rc = dubai_set_display_hist_enable(argp);
		break;
	case IOC_DISPLAY_HIST_DATA_GET:
		rc = dubai_get_display_hist_data(argp);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int dubai_set_brightness_enable(void __user *argp)
{
	int ret;
	bool enable = false;

	ret = get_enable_value(argp, &enable);
	if (ret == 0) {
		atomic_set(&dubai_backlight.enable, enable ? 1 : 0);
		dubai_info("Dubai brightness enable: %d", enable ? 1 : 0);
	}

	return ret;
}

static int dubai_set_brightness_locked(uint32_t brightness)
{
	uint64_t current_time;
	uint64_t diff_time;

	if (brightness > MAX_BRIGHTNESS) {
		dubai_err("Need valid data! brightness= %d", brightness);
		return -EINVAL;
	}

	current_time = div_u64(ktime_get_ns(), NSEC_PER_MSEC);

	if (dubai_backlight.last_brightness > 0) {
		diff_time = current_time - dubai_backlight.last_time;
		dubai_backlight.sum_time += diff_time;
		dubai_backlight.sum_brightness_time += dubai_backlight.last_brightness * diff_time;
	}

	dubai_backlight.last_time = current_time;
	dubai_backlight.last_brightness = brightness;

	return 0;
}

void dubai_update_brightness(uint32_t brightness)
{
	int ret;

	if (!atomic_read(&dubai_backlight.enable))
		return;

	mutex_lock(&brightness_lock);
	ret = dubai_set_brightness_locked(brightness);
	if (ret < 0)
		dubai_err("Update brightness= %d error!", brightness);
	mutex_unlock(&brightness_lock);
}
EXPORT_SYMBOL(dubai_update_brightness);

static int dubai_get_brightness_info(void __user *argp)
{
	int ret;
	uint32_t brightness;

	mutex_lock(&brightness_lock);
	ret = dubai_set_brightness_locked(dubai_backlight.last_brightness);
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

void dubai_display_stats_init(void)
{
	atomic_set(&dubai_backlight.enable, 0);
	dubai_backlight.last_time = 0;
	dubai_backlight.last_brightness = 0;
	dubai_backlight.sum_time = 0;
	dubai_backlight.sum_brightness_time = 0;
}

int dubai_display_register_ops(struct dubai_display_hist_ops *op)
{
	if (!display_hist_op)
		display_hist_op = op;
	return 0;
}

int dubai_display_unregister_ops(void)
{
	if (display_hist_op)
		display_hist_op = NULL;
	return 0;
}

void dubai_display_wq_handler(struct work_struct *work)
{
	if (work == NULL) {
		dubai_err("display histogram stats work is NULL");
		return;
	}

	if (!display_hist_op || !display_hist_op->update) {
		dubai_info("Display histogram not supported");
		return;
	}

	if (!g_display_hist_enable) {
		dubai_err("display histogram stats is disable");
		return;
	}

	if (!g_display_panel_state) {
		dubai_err("display panel state is closed");
		return;
	}

	if (display_hist_op->update() < 0)
		dubai_info("display histogram update failed");
	dubai_schedule_display_hist_work();
}

static void dubai_schedule_display_hist_work()
{
	schedule_delayed_work(&display_hist_work, msecs_to_jiffies(DISP_HIST_TIME_PERIOD));
}

static int dubai_get_display_hist_policy(void __user *argp)
{
	int policy;

	if (!display_hist_op || !display_hist_op->get_policy) {
		dubai_info("Display histogram not supported");
		return -EOPNOTSUPP;
	}
	policy = (int)(display_hist_op->get_policy());
	if (copy_to_user(argp, &policy, sizeof(int)))
		return -EFAULT;

	return 0;
}

static int dubai_get_display_hist_len(void __user *argp)
{
	size_t len;

	if (!display_hist_op || !display_hist_op->get_len) {
		dubai_info("Display histogram not supported");
		return -EOPNOTSUPP;
	}

	len = display_hist_op->get_len();
	if (copy_to_user(argp, &len, sizeof(len)))
		return -EFAULT;

	return 0;
}

static int dubai_set_display_hist_enable(void __user *argp)
{
	bool enable = false;
	int ret = -1;

	if (!display_hist_op || !display_hist_op->enable) {
		dubai_info("Display histogram not supported");
		return -EOPNOTSUPP;
	}

	if (get_enable_value(argp, &enable))
		return -EFAULT;
	if (g_display_hist_enable == enable) {
		return 0;
	}
	ret = display_hist_op->enable(enable);
	if (ret < 0)
		return -EFAULT;
	g_display_hist_enable = enable;
	if (g_display_hist_enable)
		dubai_schedule_display_hist_work();

	return 0;
}

void dubai_notify_display_state(bool state)
{
	if (!g_display_hist_enable)
		return;

	if (state) {
		dubai_schedule_display_hist_work();
	} else {
		cancel_delayed_work_sync(&display_hist_work);
	}
	g_display_panel_state = state;
}
EXPORT_SYMBOL(dubai_notify_display_state); //lint !e580

static int dubai_get_display_hist_data(void __user *argp)
{
	int data_size, ret;
	struct dev_transmit_t *transmit = NULL;

	if (!display_hist_op || !display_hist_op->get_len || !display_hist_op->get) {
		dubai_info("Display histogram not supported");
		return -EOPNOTSUPP;
	}
	data_size = sizeof(hist_data) * display_hist_op->get_len();
	transmit = dubai_alloc_transmit(data_size);
	if (transmit == NULL)
		return -ENOMEM;
	ret = dubai_get_display_hist_delta((hist_data *)(transmit->data), data_size);
	if (ret < 0) {
		dubai_err("Get display histogram delta fail, ret:%d", ret);
		goto out;
	}
	if (copy_to_user(argp, transmit, dubai_get_transmit_size(transmit)))
		ret = -EFAULT;
out:
	dubai_free_transmit(transmit);
	return ret;
}

static int dubai_get_display_hist_delta(hist_data *delta, int size)
{
	int ret;
	hist_data *curr;
	static hist_data *display_hist_last = NULL;

	curr = kzalloc(size, GFP_KERNEL);
	if (curr == NULL)
		return -ENOMEM;
	ret = display_hist_op->get(curr, size);
	if (ret < 0) {
		dubai_err("Failed to get display histogram data");
		kfree(curr);
		return -EFAULT;
	}
	if (display_hist_last != NULL) {
		dubai_calculate_delta(display_hist_last, curr, delta, size);
		kfree(display_hist_last);
	}
	display_hist_last = curr;

	return 0;
}

static void dubai_calculate_delta(const hist_data *last, const hist_data *curr, hist_data *delta, int size)
{
#ifdef CONFIG_HUAWEI_DUBAI_RGB_STATS
	int i;
	int num = size / sizeof(hist_data);
	for (i = 0; i < num; i++) {
		delta[i].red = CALCULATE_HIST_DELTA(last[i].red, curr[i].red);
		delta[i].green = CALCULATE_HIST_DELTA(last[i].green, curr[i].green);
		delta[i].blue = CALCULATE_HIST_DELTA(last[i].blue, curr[i].blue);
	}
#endif
}
