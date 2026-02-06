/*
 * cc_water_check.c
 *
 * use cc irq to detect water and do action.
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <linux/device.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <chipset_common/hwpower/hardware_monitor/water_detect.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG cc_water_check
HWLOG_REGIST();

/* used for cc abnormal change handler(cc interrupt storm), also used as water check */
#define CC_CHANGE_COUNTER_THRESHOLD      50
#define CC_CHANGE_INTERVAL               2000 /* ms */
#define CC_CHANGE_MSEC                   1000
#define CC_CHANGE_BUF_SIZE               10
#define CC_DMD_COUNTER_THRESHOLD         1
#define CC_DMD_INTERVAL                  (24 * 60 * 60) /* s */
#define CC_DMD_BUF_SIZE                  1024
#define CC_MOISTURE_FLAG_RESTORE_DELAY   120000 /* ms */

enum event_change_type {
	ECI_CC_CHANGE,
	ECI_VBUS_CHANGE,
	ECI_END,
};

enum cc_mode {
	CC_MODE_DRP,
	CC_MODE_UFP,
};

/* used to handle abnormal cc change and report dmd */
struct event_check_info {
	enum event_change_type event_id;
	bool first_enter; /* used to control if first cc change happend */
	int counter; /* used to count cc interrupt */
	int dmd_counter; /* used to count dmd report time */
	int change_data[CC_CHANGE_BUF_SIZE]; /* cc interrupt distributed data struct */
	int dmd_data[CC_CHANGE_BUF_SIZE]; /* dmd data struct */
	struct timespec64 last; /* last trigger time of cc interrupt  */
	struct timespec64 dmd_last; /* last trigger time of dmd report */
};

struct cc_water_check_device {
	bool moisture_status;
	bool moisture_happened;
	unsigned int cur_mode;
	unsigned int abnormal_detection;
	unsigned int abnormal_interval;
	unsigned int abnormal_dmd_report_enable;
	unsigned int  moisture_detection_enable;
	unsigned int  moisture_status_report;
	struct work_struct fb_work;
	struct delayed_work cc_work;
	struct device *dev;
	struct notifier_block wc_nb;
	struct notifier_block fb_nb;
	struct event_check_info eci[ECI_END];
};

struct time_calc {
	unsigned int tm_diff;
	unsigned int tm_diff_idx;
	struct timespec64 interval;
	struct timespec64 now;
	struct timespec64 sum;
};

static struct cc_water_check_device *g_cc_water_check_di;

/* in notifier begin */
static struct blocking_notifier_head g_in_notifier;
static BLOCKING_NOTIFIER_HEAD(g_in_notifier);

int cc_water_check_register_client(struct notifier_block *nb)
{
	int ret;

	ret = blocking_notifier_chain_register(&g_in_notifier, nb);
	if (ret < 0)
		hwlog_err("failed to register notifier\n");

	return ret;
}
EXPORT_SYMBOL_GPL(cc_water_check_register_client);

int cc_water_check_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&g_in_notifier, nb);
}
EXPORT_SYMBOL_GPL(cc_water_check_unregister_client);

int cc_water_check_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&g_in_notifier, val, v);
}
EXPORT_SYMBOL_GPL(cc_water_check_notifier_call_chain);
/* in notifier end */

bool cc_water_check_get_status(void)
{
	if (g_cc_water_check_di)
		return g_cc_water_check_di->moisture_status;

	return false;
}
EXPORT_SYMBOL_GPL(cc_water_check_get_status);

static void cc_water_check_set_cc_mode(struct cc_water_check_device *di, int mode)
{
	if (di->cur_mode == mode)
		return;

	di->cur_mode = mode;
	hwlog_info("cur_mode = %d, new mode = %d\n", di->cur_mode, mode);
}

static void cc_water_check_flag_restore_work(struct work_struct *work)
{
	struct cc_water_check_device *di = container_of(work,
		struct cc_water_check_device, cc_work.work);

	hwlog_err("moisture_happened = %d, moisture_status = %d\n",
		di->moisture_happened, di->moisture_status);
	if (!di->moisture_happened)
		di->moisture_status = false;
}

static void cc_water_check_fb_unblank_work(struct work_struct *work)
{
	unsigned int flag = WD_NON_STBY_DRY;
	struct cc_water_check_device *di = container_of(work,
		struct cc_water_check_device, fb_work);

	cc_water_check_set_cc_mode(di, CC_MODE_DRP);
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_UEVENT, &flag);

	if (di->moisture_status_report == 0)
		return;

	di->moisture_happened = false;
	schedule_delayed_work(&di->cc_work,
		msecs_to_jiffies(CC_MOISTURE_FLAG_RESTORE_DELAY));
}

static int cc_water_check_handle_fb_event(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct fb_event *fb_event = data;
	int *blank = NULL;

	if (!fb_event)
		return NOTIFY_DONE;

	blank = fb_event->data;
	if (!blank)
		return NOTIFY_DONE;

	if ((event == FB_EVENT_BLANK) && (*blank == FB_BLANK_UNBLANK))
		schedule_work(&g_cc_water_check_di->fb_work);

	return NOTIFY_DONE;
}

static void cc_water_check_update(struct cc_water_check_device *di, int event)
{
	struct time_calc tc;
	struct event_check_info *eci = &di->eci[event];

	tc.interval.tv_sec = 0;
	tc.interval.tv_nsec = di->abnormal_interval * NSEC_PER_MSEC;
	tc.now = current_kernel_time64();
	eci->last = tc.now;

	if (eci->first_enter) {
		eci->first_enter = false;
		return;
	}

	tc.sum = timespec64_add_safe(eci->last, tc.interval);
	if (tc.sum.tv_sec == TIME_T_MAX) {
		eci->counter = 0;
		hwlog_err("time overflow happend\n");
		return;
	}

	if (timespec64_compare(&tc.sum, &tc.now) < 0) {
		eci->counter = 0;
		memset(eci->change_data, 0, sizeof(eci->change_data));
		return;
	}

	tc.tm_diff = (tc.now.tv_sec - eci->last.tv_sec) * CC_CHANGE_MSEC +
		(tc.now.tv_nsec - eci->last.tv_nsec) / NSEC_PER_MSEC;
	tc.tm_diff_idx = tc.tm_diff / (CC_CHANGE_INTERVAL / CC_CHANGE_BUF_SIZE);

	if (tc.tm_diff_idx >= CC_CHANGE_BUF_SIZE)
		tc.tm_diff_idx = CC_CHANGE_BUF_SIZE - 1;

	++eci->counter;
	++eci->change_data[tc.tm_diff_idx];
	hwlog_info("cc counter = %d\n", eci->counter);
}

static void cc_water_check_action(struct cc_water_check_device *di, int event)
{
	int i;
	unsigned int flag = WD_NON_STBY_MOIST;
	struct event_check_info *eci = &di->eci[event];

	if (eci->counter < CC_CHANGE_COUNTER_THRESHOLD)
		return;

	if (di->moisture_status_report) {
		di->moisture_happened = true;
		di->moisture_status = true;
	}

	for (i = 0; i < CC_CHANGE_BUF_SIZE; i++)
		eci->dmd_data[i] += eci->change_data[i];

	eci->counter = 0;
	++eci->dmd_counter;
	memset(eci->change_data, 0, sizeof(eci->change_data));

	if (di->moisture_detection_enable) {
		power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_UEVENT, &flag);
		hwlog_err("moisture detected and reported\n");
	}

	cc_water_check_set_cc_mode(di, CC_MODE_UFP);
	hwlog_err("cc counter hit\n");
}

static void cc_water_check_dmd_action(struct cc_water_check_device *di, int event)
{
	char dsm_buf[CC_DMD_BUF_SIZE] = {0};
	int i;
	struct time_calc tc;
	struct event_check_info *eci = &di->eci[event];

	if (!di->abnormal_dmd_report_enable)
		return;

	if (eci->dmd_counter < CC_DMD_COUNTER_THRESHOLD)
		return;

	eci->dmd_counter = 0;
	tc.interval.tv_sec = CC_DMD_INTERVAL;
	tc.interval.tv_nsec = 0;
	tc.now = current_kernel_time64();
	tc.sum = timespec64_add_safe(eci->dmd_last, tc.interval);

	if (tc.sum.tv_sec == TIME_T_MAX) {
		hwlog_err("time overflow happend\n");
		return;
	}

	if (timespec64_compare(&tc.sum, &tc.now) >= 0) {
		hwlog_err("water check dmd report within 24 hour\n");
		return;
	}

	snprintf(dsm_buf, CC_DMD_BUF_SIZE - 1, "cc abnormal is triggered:");
	for (i = 0; i < CC_CHANGE_BUF_SIZE; i++)
		snprintf(dsm_buf + strlen(dsm_buf), CC_DMD_BUF_SIZE - 1, " %d",
			eci->dmd_data[i]);

	snprintf(dsm_buf + strlen(dsm_buf), CC_DMD_BUF_SIZE - 1, "\n");
	power_dsm_report_dmd(POWER_DSM_BATTERY, ERROR_NO_WATER_CHECK_IN_USB, dsm_buf);
	memset(eci->dmd_data, 0, sizeof(eci->dmd_data));
	eci->dmd_last = tc.now;
}

static int cc_water_check_handle_cc_event(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct cc_water_check_device *di = g_cc_water_check_di;

	if (event > ECI_END)
		return NOTIFY_DONE;

	cc_water_check_update(di, event);
	cc_water_check_action(di, event);
	cc_water_check_dmd_action(di, event);

	return NOTIFY_DONE;
}

static void cc_water_check_parse_dts(struct cc_water_check_device *di,
	struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_detection", &di->abnormal_detection, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_interval", &di->abnormal_interval, CC_CHANGE_INTERVAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"moisture_detection_enable", &di->moisture_detection_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"moisture_status_report", &di->moisture_status_report, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_abnormal_dmd_report_enable", &di->abnormal_dmd_report_enable, 1);
}

static int cc_water_check_probe(struct platform_device *pdev)
{
	struct cc_water_check_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	g_cc_water_check_di = di;

	di->eci[ECI_CC_CHANGE].event_id = ECI_CC_CHANGE;
	di->eci[ECI_CC_CHANGE].first_enter = true;
	di->eci[ECI_VBUS_CHANGE].event_id = ECI_VBUS_CHANGE;
	di->eci[ECI_VBUS_CHANGE].first_enter = true;

	cc_water_check_parse_dts(di, np);
	INIT_WORK(&di->fb_work, cc_water_check_fb_unblank_work);
	INIT_DELAYED_WORK(&di->cc_work, cc_water_check_flag_restore_work);

	di->wc_nb.notifier_call = cc_water_check_handle_cc_event;
	cc_water_check_register_client(&di->wc_nb);

	if (di->abnormal_detection) {
		di->fb_nb.notifier_call = cc_water_check_handle_fb_event;
		fb_register_client(&di->fb_nb);
	}

	return 0;
}

static const struct of_device_id cc_water_check_match_table[] = {
	{
		.compatible = "cc_water_check",
		.data = NULL,
	},
	{},
};

static struct platform_driver cc_water_check_driver = {
	.probe = cc_water_check_probe,
	.driver = {
		.name = "cc_water_check",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cc_water_check_match_table),
	},
};

static int __init cc_water_check_init(void)
{
	return platform_driver_register(&cc_water_check_driver);
}

static void __exit cc_water_check_exit(void)
{
	platform_driver_unregister(&cc_water_check_driver);
}

late_initcall(cc_water_check_init);
module_exit(cc_water_check_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cc_water_check driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
