/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include "huawei_thp_mt_wrapper.h"
#include "huawei_thp.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/delay.h>
#if defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif

#ifdef CONFIG_HUAWEI_THP_QCOM
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#endif

#ifdef CONFIG_INPUTHUB_20
#include "contexthub_recovery.h"
#endif

#ifdef CONFIG_HUAWEI_PS_SENSOR
#include "ps_sensor.h"
#endif

#ifdef CONFIG_HUAWEI_SENSORS_2_0
#include "sensor_scp.h"
#endif

#if defined(CONFIG_HUAWEI_TS_KIT_3_0)
#include "../3_0/trace-events-touch.h"
#else
#define trace_touch(x...)
#endif

#define DEVICE_NAME "input_mt_wrapper"
#define SECOND 1000000
#define NO_DELAY 0
#define DELAY_LIMIT_FRAME_COUNT 5
#define SUPPORT_PEN_PROTOCOL_CLASS 2
#define SUPPORT_PEN_PROTOCOL_CODE 4

#define DEFAULT_MAGNIFICATION 1
#define MAX_MAGNIFICATION 16

#ifdef CONFIG_HUAWEI_THP_QCOM
#define CMDLINE_RECOVERY_MODE "1"

static unsigned int enter_recovery_flag;

/*
 * parse enter_recovery cmdline which is passed from lk
 * Format : //on the recovery page enter_recovery=1
 */
static int __init early_parse_enterrecovery_cmdline(char *bootmode)
{
	if (bootmode && !strncmp(bootmode, CMDLINE_RECOVERY_MODE, strlen(CMDLINE_RECOVERY_MODE)))
		enter_recovery_flag = true;

	thp_log_info("boot mode cmdline: [%s], enter_recovery_flag [%d]\n",
			bootmode, enter_recovery_flag);
	return 0;
}

early_param("enter_recovery", early_parse_enterrecovery_cmdline);

unsigned int get_boot_into_recovery_flag_qcom(void)
{
	return enter_recovery_flag;
}
#endif

static struct thp_mt_wrapper_data *g_thp_mt_wrapper;

struct thp_vendor_name {
	const char *vendor_id;
	const char *compatible_name;
};

/*
 * use 2 bits vendor_id in project_id to distinguish LCD IC.
 * 09: SDC, 13: BOE
 */
static struct thp_vendor_name thp_input_compatible_table[] = {
	{ "09", "huawei,thp_input_09" },
	{ "13", "huawei,thp_input_13" },
};

#if ((!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK))
unsigned int get_boot_into_recovery_flag(void);
#endif

void thp_inputkey_report(unsigned int gesture_wakeup_value)
{
	input_report_key(g_thp_mt_wrapper->input_dev, gesture_wakeup_value, 1);
	input_sync(g_thp_mt_wrapper->input_dev);
	input_report_key(g_thp_mt_wrapper->input_dev, gesture_wakeup_value, 0);
	input_sync(g_thp_mt_wrapper->input_dev);
	thp_log_info("%s ->done\n", __func__);
}

#ifdef CONFIG_HUAWEI_THP_MTK
void thp_aod_click_report(struct thp_udfp_data udfp_data)
{
	unsigned int x;
	unsigned int y;
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;

	if (input_dev == NULL) {
		thp_log_err("%s: have null ptr\n", __func__);
		return;
	}
	x = udfp_data.tpud_data.tp_x;
	y = udfp_data.tpud_data.tp_y;
	input_report_abs(input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(input_dev);
	input_report_key(input_dev, TS_SINGLE_CLICK, THP_KEY_DOWN);
	input_sync(input_dev);
	input_report_key(input_dev, TS_SINGLE_CLICK, THP_KEY_UP);
	input_sync(input_dev);
	thp_log_info("%s ->done, (%u, %u)\n", __func__, x, y);
}

void thp_udfp_event_to_aod(struct thp_udfp_data udfp_data)
{
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;

	if (input_dev == NULL) {
		thp_log_err("%s: have null ptr\n", __func__);
		return;
	}
	if (udfp_data.tpud_data.udfp_event == TP_EVENT_FINGER_DOWN) {
		thp_log_info("%s FINGER_DOWN\n", __func__);
		input_report_key(input_dev, KEY_FINGER_DOWN, THP_KEY_DOWN);
		input_sync(input_dev);
		input_report_key(input_dev, KEY_FINGER_DOWN, THP_KEY_UP);
		input_sync(input_dev);
	}
	if (udfp_data.tpud_data.udfp_event == TP_EVENT_FINGER_UP) {
		thp_log_info("%s FINGER_UP\n", __func__);
		input_report_key(input_dev, KEY_FINGER_UP, THP_KEY_DOWN);
		input_sync(input_dev);
		input_report_key(input_dev, KEY_FINGER_UP, THP_KEY_UP);
		input_sync(input_dev);
	}
}

#endif

void thp_input_pen_report(unsigned int pen_event_value)
{
	input_report_key(g_thp_mt_wrapper->pen_dev, pen_event_value, 1);
	input_sync(g_thp_mt_wrapper->pen_dev);
	input_report_key(g_thp_mt_wrapper->pen_dev, pen_event_value, 0);
	input_sync(g_thp_mt_wrapper->pen_dev);
	thp_log_info("%s:done\n", __func__);
}

int thp_mt_wrapper_ioctl_get_events(unsigned long event)
{
	int t;
	int __user *events = (int *)(uintptr_t)event;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!events)) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	thp_log_info("%d: cd->event_flag\n", cd->event_flag);
	if (cd->event_flag) {
		if (copy_to_user(events, &cd->event, sizeof(cd->event))) {
			thp_log_err("%s:copy events failed\n", __func__);
			return -EFAULT;
		}

		cd->event_flag = false;
	} else {
		cd->thp_event_waitq_flag = WAITQ_WAIT;
		t = wait_event_interruptible(cd->thp_event_waitq,
			(cd->thp_event_waitq_flag == WAITQ_WAKEUP));
		thp_log_info("%s: set wait finish :%d\n", __func__, t);
	}

	return 0;
}

static void thp_adjustment_coordinate(void)
{
	unsigned int delta_time;
	unsigned int delay_time;
	unsigned int time_interval;
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd->frame_count)
		time_interval = cd->time_interval;
	else if (cd->frame_count < DELAY_LIMIT_FRAME_COUNT)
		time_interval = cd->time_min_interval;
	else
		time_interval = NO_DELAY;
	do_gettimeofday(&cd->report_cur_time);
	delta_time = ((cd->report_cur_time.tv_sec -
		cd->report_pre_time.tv_sec) * SECOND) +
		cd->report_cur_time.tv_usec - cd->report_pre_time.tv_usec;

	if (delta_time < time_interval) {
		delay_time = time_interval - delta_time;
		udelay(delay_time);
		do_gettimeofday(&cd->report_cur_time);
	}
	cd->report_pre_time = cd->report_cur_time;
}

static void thp_coordinate_report(struct thp_mt_wrapper_ioctl_touch_data data)
{
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;
	unsigned int i;

	for (i = 0; i < INPUT_MT_WRAPPER_MAX_FINGERS; i++) {
#ifdef TYPE_B_PROTOCOL
		input_mt_slot(input_dev, i);
		input_mt_report_slot_state(input_dev,
			data.touch[i].tool_type, data.touch[i].valid != 0);
#endif
		if (data.touch[i].valid != 0) {
			input_report_abs(input_dev, ABS_MT_POSITION_X,
						data.touch[i].x);
			input_report_abs(input_dev, ABS_MT_POSITION_Y,
						data.touch[i].y);
			input_report_abs(input_dev, ABS_MT_PRESSURE,
						data.touch[i].pressure);
			input_report_abs(input_dev, ABS_MT_TRACKING_ID,
						data.touch[i].tracking_id);
			input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
						data.touch[i].major);
			input_report_abs(input_dev, ABS_MT_TOUCH_MINOR,
						data.touch[i].minor);
			input_report_abs(input_dev, ABS_MT_ORIENTATION,
						data.touch[i].orientation);
			input_report_abs(input_dev, ABS_MT_TOOL_TYPE,
						data.touch[i].tool_type);
			input_report_abs(input_dev, ABS_MT_BLOB_ID,
						data.touch[i].hand_side);
#ifndef TYPE_B_PROTOCOL
			input_mt_sync(input_dev);
#endif
		}
	}
	/* BTN_TOUCH DOWN */
	if (data.t_num > 0)
		input_report_key(input_dev, BTN_TOUCH, 1);
	/* BTN_TOUCH UP */
	if (data.t_num == 0) {
#ifndef TYPE_B_PROTOCOL
		input_mt_sync(input_dev);
#endif
		input_report_key(input_dev, BTN_TOUCH, 0);
	}
	input_sync(input_dev);
}

static long thp_mt_wrapper_ioctl_set_coordinate(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_mt_wrapper_ioctl_touch_data data;
	struct thp_core_data *cd = thp_get_core_data();

	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_IN, "thp");
	if (arg == 0) {
		thp_log_err("%s:arg is null\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&data, argp, sizeof(data))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_OUT, "thp");

	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_IN, "thp");

	if ((cd->support_interval_adjustment) && (cd->time_adjustment_switch))
		thp_adjustment_coordinate();

	thp_coordinate_report(data);
	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_OUT, "thp");
	return 0;
}

void thp_clean_fingers(void)
{
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;
	struct thp_mt_wrapper_ioctl_touch_data data;

	memset(&data, 0, sizeof(data));

#ifdef CONFIG_HUAWEI_THP_QCOM
	if (power_cmdline_is_powerdown_charging_mode()) {
		thp_log_info("%s not send for qcom powerdown charging mode\n", __func__);
		return;
	}
#endif

	input_mt_sync(input_dev);
	input_sync(input_dev);

	input_report_key(input_dev, BTN_TOUCH, 0);
	input_sync(input_dev);
}

static int thp_mt_wrapper_open(struct inode *inode, struct file *filp)
{
	thp_log_info("%s:called\n", __func__);
	return 0;
}

static int thp_mt_wrapper_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int thp_mt_wrapper_ioctl_read_status(unsigned long arg)
{
	int __user *status = (int *)(uintptr_t)arg;
	u32 thp_status = thp_get_status_all();

	thp_log_info("%s:status=%d\n", __func__, thp_status);

	if (!status) {
		thp_log_err("%s:input null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(status, &thp_status, sizeof(u32))) {
		thp_log_err("%s:copy status failed\n", __func__);
		return -EFAULT;
	}

	if (atomic_read(&g_thp_mt_wrapper->status_updated) != 0)
		atomic_dec(&g_thp_mt_wrapper->status_updated);

	return 0;
}

static int thp_mt_ioctl_read_input_config(unsigned long arg)
{
	struct thp_input_dev_config __user *config =
		(struct thp_input_dev_config *)(uintptr_t)arg;
	struct thp_input_dev_config *input_config =
			&g_thp_mt_wrapper->input_dev_config;

	if (!config) {
		thp_log_err("%s:input null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(config, input_config,
			sizeof(struct thp_input_dev_config))) {
		thp_log_err("%s:copy input config failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_mt_wrapper_ioctl_read_scene_info(unsigned long arg)
{
	struct thp_scene_info __user *config =
		(struct thp_scene_info *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_scene_info *scene_info = NULL;

	if (!cd) {
		thp_log_err("%s:thp_core_data is NULL\n", __func__);
		return -EINVAL;
	}
	scene_info = &(cd->scene_info);

	thp_log_info("%s:%d,%d,%d\n", __func__,
		scene_info->type, scene_info->status, scene_info->parameter);

	if (copy_to_user(config, scene_info, sizeof(struct thp_scene_info))) {
		thp_log_err("%s:copy scene_info failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

int thp_mt_wrapper_esd_event(unsigned int status)
{
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;

	if (!input_dev) {
		thp_log_err("%s:cd or input_dev is null\n", __func__);
		return -EINVAL;
	}
	if (status < 0) {
		thp_log_err("%s:status value is invalid\n", __func__);
		return -EINVAL;
	}
	input_report_key(input_dev, KEY_F26, 1);
	input_sync(input_dev);
	input_report_key(input_dev, KEY_F26, 0);
	input_sync(input_dev);
	thp_log_info("%s:ESD EVENT\n", __func__);

	return 0;
}

static int thp_mt_wrapper_ioctl_get_window_info(unsigned long arg)
{
	struct thp_window_info __user *window_info =
		(struct thp_window_info *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!window_info)) {
		thp_log_err("%s:args error\n", __func__);
		return -EINVAL;
	}

	thp_log_info("%s:x0=%d,y0=%d,x1=%d,y1=%d\n", __func__,
		cd->window.x0, cd->window.y0, cd->window.x1, cd->window.y1);

	if (copy_to_user(window_info, &cd->window,
		sizeof(struct thp_window_info))) {
		thp_log_err("%s:copy window_info failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_mt_wrapper_ioctl_get_projectid(unsigned long arg)
{
	char __user *project_id = (char __user *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!project_id)) {
		thp_log_err("%s:args error\n", __func__);
		return -EINVAL;
	}

	thp_log_info("%s:project id:%s\n", __func__, cd->project_id);

	if (copy_to_user(project_id, cd->project_id, sizeof(cd->project_id))) {
		thp_log_err("%s:copy project_id failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_mt_wrapper_ioctl_set_roi_data(unsigned long arg)
{
	short __user *roi_data = (short __user *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!roi_data)) {
		thp_log_err("%s:args error\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(cd->roi_data, roi_data, sizeof(cd->roi_data))) {
		thp_log_err("%s:copy roi data failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_mt_wrapper_ioctl_set_events(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	void __user *argp = (void __user *)(uintptr_t)arg;
	int val;

	if (arg == 0) {
		thp_log_err("%s:arg is null\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&val, argp,
			sizeof(int))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	thp_log_info("thp_send, write: %d\n", val);
	cd->event_flag = true;
	cd->event = val;
	if (cd->event_flag) {
		cd->thp_event_waitq_flag = WAITQ_WAKEUP;
		wake_up_interruptible(&cd->thp_event_waitq);
		thp_log_info("%d: wake_up\n", cd->event);
	}

	return 0;
}

static int thp_mt_ioctl_report_keyevent(unsigned long arg)
{
	int report_value[PROX_VALUE_LEN] = {0};
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;
	void __user *argp = (void __user *)(uintptr_t)arg;
	enum input_mt_wrapper_keyevent keyevent;

	if (arg == 0) {
		thp_log_err("%s:arg is null\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&keyevent, argp,
			sizeof(enum input_mt_wrapper_keyevent))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}

	if (keyevent == INPUT_MT_WRAPPER_KEYEVENT_ESD) {
		input_report_key(input_dev, KEY_F26, 1);
		input_sync(input_dev);
		input_report_key(input_dev, KEY_F26, 0);
		input_sync(input_dev);
	} else if (keyevent == INPUT_MT_WRAPPER_KEYEVENT_APPROACH) {
		thp_log_info("[Proximity_feature] %s: report [near] event!\n",
			__func__);
		report_value[0] = APPROCH_EVENT_VALUE;
#if ((defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR) || \
	(defined CONFIG_HUAWEI_SENSORS_2_0))
		thp_prox_event_report(report_value, PROX_EVENT_LEN);
#endif
	} else if (keyevent == INPUT_MT_WRAPPER_KEYEVENT_AWAY) {
		thp_log_info("[Proximity_feature] %s: report [far] event!\n",
			__func__);
		report_value[0] = AWAY_EVENT_VALUE;
#if ((defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR) || \
	(defined CONFIG_HUAWEI_SENSORS_2_0))
		thp_prox_event_report(report_value, PROX_EVENT_LEN);
#endif
	}

	return 0;
}

static long thp_mt_wrapper_ioctl_get_platform_type(unsigned long arg)
{
	int __user *platform_type = (int __user *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!platform_type)) {
		thp_log_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	thp_log_info("%s: cd->platform_type %d\n", __func__, cd->platform_type);

	if (copy_to_user(platform_type, &cd->platform_type,
					sizeof(cd->platform_type))) {
		thp_log_err("%s:copy platform_type failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_report_system_event(struct thp_key_info *key_info)
{
	struct input_dev *input_dev = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	if ((cd == NULL) || (!cd->support_extra_key_event_input) ||
		(g_thp_mt_wrapper->extra_key_dev == NULL)) {
		thp_log_err("%s:input is invalid\n", __func__);
		return -EINVAL;
	}
	input_dev = g_thp_mt_wrapper->extra_key_dev;
	thp_log_info("%s Ring-Vibrate : key: %d, value: %d\n",
		__func__, key_info->key, key_info->action);
	if ((key_info->key != KEY_VOLUME_UP) &&
		(key_info->key != KEY_VOLUME_DOWN) &&
		(key_info->key != KEY_POWER) &&
		(key_info->key != KEY_VOLUME_MUTE) &&
		(key_info->key != KEY_VOLUME_TRIG)) {
		thp_log_err("%s:key is invalid\n", __func__);
		return -EINVAL;
	}
	if ((key_info->action != THP_KEY_UP) &&
		(key_info->action != THP_KEY_DOWN)) {
		thp_log_err("%s:action is invalid\n", __func__);
		return -EINVAL;
	}

	input_report_key(input_dev, key_info->key,
		key_info->action);
	input_sync(input_dev);
	return 0;
}

static int thp_mt_ioctl_report_system_keyevent(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_key_info key_info;

	if (arg == 0) {
		thp_log_err("%s:arg is null\n", __func__);
		return -EINVAL;
	}
	memset(&key_info, 0, sizeof(key_info));
	if (copy_from_user(&key_info, argp, sizeof(key_info))) {
		thp_log_err("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	return thp_report_system_event(&key_info);
}

#ifdef CONFIG_HUAWEI_THP_MTK
static int thp_notify_fp_event(struct thp_shb_info info)
{
	int ret;
	struct ud_fp_ops *fp_ops = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd || !cd->use_ap_gesture) {
		thp_log_info("%s: not support ap handle udfp\n", __func__);
		return -EINVAL;
	}
	if (info.cmd_type != THP_FINGER_PRINT_EVENT) {
		thp_log_info("%s: only handle fp event, return\n", __func__);
		return NO_ERR;
	}
	thp_log_info("%s: called\n", __func__);
	fp_ops = fp_get_ops();
	if (!fp_ops || !fp_ops->fp_irq_notify) {
		thp_log_err("%s: point is NULL!\n", __func__);
		return -EINVAL;
	}
	ret = fp_ops->fp_irq_notify((struct tp_to_udfp_data *)info.cmd_addr);
	if (ret)
		thp_log_err("%s: fp_irq_notify fail, ret %d\n", __func__, ret);
	return ret;
}
#endif

#ifdef CONFIG_HUAWEI_SHB_THP
int thp_send_volumn_to_drv(const char *head)
{
	struct thp_volumn_info *rx = (struct thp_volumn_info *)head;
	struct thp_key_info key_info;
	struct thp_core_data *cd = thp_get_core_data();

	if ((rx == NULL) || (cd == NULL)) {
		thp_log_err("%s:rx or cd is null\n", __func__);
		return -EINVAL;
	}
	if (!atomic_read(&cd->register_flag)) {
		thp_log_err("%s: thp have not be registered\n", __func__);
		return -ENODEV;
	}
	__pm_wakeup_event(&cd->thp_wake_lock, jiffies_to_msecs(HZ));
	thp_log_info("%s:key:%ud, action:%ud\n", __func__,
		rx->data[0], rx->data[1]);
	key_info.key = rx->data[0];
	key_info.action = rx->data[1];
	return thp_report_system_event(&key_info);
}

static int thp_event_info_dispatch(struct thp_shb_info info)
{
	int ret;
	unsigned int cmd_type = info.cmd_type;
	uint8_t cmd;

	switch (cmd_type) {
	case THP_FINGER_PRINT_EVENT:
		cmd = ST_CMD_TYPE_FINGERPRINT_EVENT;
		ret = send_thp_ap_event(info.cmd_len, info.cmd_addr, cmd);
		break;
	case THP_RING_EVENT:
		cmd = ST_CMD_TYPE_RING_EVENT;
		ret = send_thp_ap_event(info.cmd_len, info.cmd_addr, cmd);
		break;
	case THP_ALGO_SCREEN_OFF_INFO:
		ret = send_thp_algo_sync_event(info.cmd_len, info.cmd_addr);
		break;
	case THP_AUXILIARY_DATA:
		ret = send_thp_auxiliary_data(info.cmd_len, info.cmd_addr);
		break;
	default:
		thp_log_err("%s: thp_shb_info is null\n", __func__);
		ret = -EFAULT;
	}
	return ret;
}
#endif
#if ((defined CONFIG_HUAWEI_SHB_THP) || (defined CONFIG_HUAWEI_THP_MTK))
static int thp_mt_ioctl_cmd_shb_event(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	int ret;
	struct thp_shb_info data;
	char *cmd_data = NULL;
#ifdef CONFIG_HUAWEI_SHB_THP
	struct thp_core_data *cd = thp_get_core_data();
#endif

	if (arg == 0) {
		thp_log_err("%s:arg is null.\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&data, argp, sizeof(struct thp_shb_info))) {
		thp_log_err("%s:copy info failed\n", __func__);
		return -EFAULT;
	}
	if ((data.cmd_len > MAX_THP_CMD_INFO_LEN) || (data.cmd_len == 0)) {
		thp_log_err("%s:cmd_len:%u is illegal\n", __func__,
			data.cmd_len);
		return 0;
	}
	cmd_data = kzalloc(data.cmd_len, GFP_KERNEL);
	if (cmd_data == NULL) {
		thp_log_err("%s:cmd buffer kzalloc failed\n", __func__);
		return -EFAULT;
	}
	if (copy_from_user(cmd_data, data.cmd_addr, data.cmd_len)) {
		thp_log_err("%s:copy cmd data failed\n", __func__);
		kfree(cmd_data);
		return -EFAULT;
	}
	data.cmd_addr = cmd_data;
#ifdef CONFIG_HUAWEI_SHB_THP
	if (cd->tsa_event_to_udfp) {
		ret = send_tp_ap_event(data.cmd_len, data.cmd_addr,
			ST_CMD_TYPE_FINGERPRINT_EVENT);
		if (ret < 0)
			thp_log_err("%s:tsa_event notify fp err %d\n",
				__func__, ret);
		kfree(cmd_data);
		return 0;
	}
	ret = thp_event_info_dispatch(data);
	if (ret < 0)
		thp_log_err("%s:thp event info dispatch failed\n", __func__);
#endif
#ifdef CONFIG_HUAWEI_THP_MTK
	ret = thp_notify_fp_event(data);
	if (ret < 0)
		thp_log_err("%s:tpud event notify fp err %d\n", __func__, ret);
#endif
	kfree(cmd_data);
	return 0;
}
#endif

static long thp_ioctl_get_volume_side(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	void __user *status = (void __user *)(uintptr_t)arg;

	if (cd == NULL) {
		thp_log_err("%s: thp cord data null\n", __func__);
		return -EINVAL;
	}
	if (status == NULL) {
		thp_log_err("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(status, (void *)&cd->volume_side_status,
		sizeof(cd->volume_side_status))) {
		thp_log_err("%s: get volume side failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_ioctl_get_power_switch(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	void __user *status = (void __user *)(uintptr_t)arg;

	if ((cd == NULL) || (status == NULL)) {
		thp_log_err("%s: thp cord data null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(status, (void *)&cd->power_switch,
		sizeof(cd->power_switch))) {
		thp_log_err("%s: get power_switch failed\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static void thp_report_pen_event(struct input_dev *input,
	struct thp_tool tool, int pressure, int tool_type, int tool_value)
{
	if (input == NULL) {
		thp_log_err("%s: input null ptr\n", __func__);
		return;
	}

	thp_log_debug("%s:tool.tip_status:%d, tool_type:%d, tool_value:%d\n",
		__func__, tool.tip_status, tool_type, tool_value);
	input_report_abs(input, ABS_X, tool.x);
	input_report_abs(input, ABS_Y, tool.y);
	input_report_abs(input, ABS_PRESSURE, pressure);
	input_report_abs(input, ABS_TILT_X, tool.tilt_x);
	input_report_abs(input, ABS_TILT_Y, tool.tilt_y);
	input_report_key(input, BTN_TOUCH, tool.tip_status);
	input_report_key(input, tool_type, tool_value);
	input_sync(input);
}

static int thp_mt_wrapper_ioctl_report_pen(unsigned long arg)
{
	struct thp_mt_wrapper_ioctl_pen_data pens;
	struct input_dev *input = g_thp_mt_wrapper->pen_dev;
	struct thp_core_data *cd = thp_get_core_data();
	int i;
	int key_value;
	void __user *argp = (void __user *)(uintptr_t)arg;

	if ((arg == 0) || (input == NULL) || (cd == NULL)) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}
	if (cd->pen_supported == 0) {
		thp_log_info("%s:not support pen\n", __func__);
		return 0;
	}
	memset(&pens, 0, sizeof(pens));
	if (copy_from_user(&pens, argp, sizeof(pens))) {
		thp_log_err("Failed to copy_from_user\n");
		return -EFAULT;
	}

	/* report pen basic single button */
	for (i = 0; i < TS_MAX_PEN_BUTTON; i++) {
		if (pens.buttons[i].status == 0)
			continue;
		else if (pens.buttons[i].status == TS_PEN_BUTTON_PRESS)
			key_value = 1; /* key down */
		else
			key_value = 0; /* key up */
		if (pens.buttons[i].key != 0) {
			thp_log_err("pen index is %d\n", i);
			input_report_key(input, pens.buttons[i].key,
				key_value);
		}
	}

	/* pen or rubber report point */
	thp_report_pen_event(input, pens.tool, pens.tool.pressure,
		pens.tool.tool_type, pens.tool.pen_inrange_status);
	return 0;
}

static long thp_ioctl_get_stylus3_connect_status(unsigned long arg)
{
	struct thp_core_data *cd = NULL;
	void __user *argp = NULL;
	int ret;

	if (arg == 0) {
		thp_log_err("arg == 0\n");
		return -EINVAL;
	}
	argp = (void __user *)(uintptr_t)arg;
	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}
	ret = wait_for_completion_interruptible(&cd->stylus3_status_flag);
	if (ret) {
		thp_log_err(" Failed to get_connect_status\n");
		return ret;
	}
	if (copy_to_user(argp, &cd->last_stylus3_status,
		sizeof(cd->last_stylus3_status))) {
		thp_log_err("%s: Failed to copy_to_user()\n",
			__func__);
		return -EFAULT;
	}
	return NO_ERR;
}

static int set_stylus3_change_protocol(unsigned long arg)
{
	struct thp_core_data *cd = NULL;
	unsigned int stylus_status;
	void __user *argp = (void __user *)(uintptr_t)arg;

	if (arg == 0) {
		thp_log_err("arg == 0\n");
		return -EINVAL;
	}
	if (copy_from_user(&stylus_status, argp, sizeof(stylus_status))) {
		thp_log_err("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	/* bt close,do not need handle */
	if (stylus_status == 0) {
		thp_log_info("do not change pen protocol\n");
		return NO_ERR;
	}
	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}
	/* change pen protocol to 2.2 */
	cd->stylus3_callback_event.event_class = SUPPORT_PEN_PROTOCOL_CLASS;
	cd->stylus3_callback_event.event_code = SUPPORT_PEN_PROTOCOL_CODE;
	thp_log_info("%s: to pen\n", __func__);
	atomic_set(&cd->callback_event_flag, true);
	complete(&cd->stylus3_callback_flag);
	return NO_ERR;
}

static int thp_ioctl_set_stylus3_plam_suppression(unsigned long arg)
{
	unsigned int suppression_type;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		thp_log_err("%s have null ptr\n", __func__);
		return -EINVAL;
	}
	if ((!cd->pen_supported) || (!cd->pen_mt_enable_flag) ||
		(!cd->support_stylus3_plam_suppression)) {
		thp_log_err("%s unsupported stylus3 plam suppression\n", __func__);
		return  -EINVAL;
	}
	if (arg == 0) {
		thp_log_err("arg == 0\n");
		return -EINVAL;
	}
	if (copy_from_user(&suppression_type, argp, sizeof(unsigned int))) {
		thp_log_err("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	thp_log_info("%s suppression_type = %u\n",
		__func__, suppression_type);
	thp_set_status(THP_STATUS_STYLUS3_PLAM_SUPPRESSION,
		(suppression_type & STYLUS3_PLAM_SUPPRESSION_MASK));
	return 0;
}

static int thp_ioctl_set_stylus3_connect_status(unsigned long arg)
{
	struct thp_core_data *cd = NULL;
	void __user *argp = NULL;
	unsigned int stylus3_status;

	if (arg == 0) {
		thp_log_err("arg == 0\n");
		return -EINVAL;
	}
	argp = (void __user *)(uintptr_t)arg;
	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&stylus3_status, argp, sizeof(unsigned int))) {
		thp_log_err("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	thp_log_info("%s:stylus3_status = %d\n", __func__, stylus3_status);
	thp_set_status(THP_STATUS_STYLUS3,
		(stylus3_status & STYLUS3_CONNECTED_MASK));
	atomic_set(&cd->last_stylus3_status, stylus3_status);
	complete(&cd->stylus3_status_flag);
	if ((cd->send_bt_status_to_fw) &&
		(cd->thp_dev->ops->bt_handler))
		if (cd->thp_dev->ops->bt_handler(cd->thp_dev, false))
			thp_log_err("send bt status to fw fail\n");

	return 0;
}

static long thp_ioctl_get_callback_events(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_core_data *cd = NULL;
	int ret;

	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}
	if (!(cd->pen_supported) || !(cd->pen_mt_enable_flag)) {
		thp_log_err("%s: Not support stylus3\n", __func__);
		return -EINVAL;
	}
	if (arg == 0) {
		thp_log_err("arg == 0\n");
		return -EINVAL;
	}
	ret = wait_for_completion_interruptible(&cd->stylus3_callback_flag);
	if (ret) {
		thp_log_info(" Failed to get stylus3_callback_flag\n");
	} else {
		if (copy_to_user(argp, &cd->stylus3_callback_event,
			sizeof(cd->stylus3_callback_event))) {
			thp_log_err("%s: Failed to copy_to_user()\n", __func__);
			return -EFAULT;
		}
		thp_log_info("%s, eventClass=%d, eventCode=%d, extraInfo=%s\n",
			__func__, cd->stylus3_callback_event.event_class,
			cd->stylus3_callback_event.event_code,
			cd->stylus3_callback_event.extra_info);
	}
	atomic_set(&cd->callback_event_flag, false);
	return NO_ERR;
}

static int thp_ioctl_set_callback_events(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_core_data *cd = NULL;

	cd = thp_get_core_data();
	if (!cd) {
		thp_log_err("%s:have null ptr\n", __func__);
		return -EINVAL;
	}
	if (!(cd->pen_supported) || !(cd->pen_mt_enable_flag)) {
		thp_log_err("%s: Not support stylus3\n", __func__);
		return -EINVAL;
	}
	if (arg == 0) {
		thp_log_err("arg == 0\n");
		return -EINVAL;
	}
	if (atomic_read(&cd->callback_event_flag) != false) {
		thp_log_err("%s,callback event not handle, need retry\n",
			__func__);
		return -EBUSY;
	}
	if (copy_from_user(&cd->stylus3_callback_event,
		argp, sizeof(cd->stylus3_callback_event))) {
		thp_log_err("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	thp_log_info("%s, eventClass=%d, eventCode=%d, extraInfo=%s\n",
		__func__,
		cd->stylus3_callback_event.event_class,
		cd->stylus3_callback_event.event_code,
		cd->stylus3_callback_event.extra_info);
	atomic_set(&cd->callback_event_flag, true);
	complete(&cd->stylus3_callback_flag);
	return NO_ERR;
}

static int daemon_init_protect(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();
	u32 daemon_flag;

	if (!cd->support_daemon_init_protect) {
		thp_log_err("%s: not support daemon init protect\n", __func__);
		return 0;
	}

	if (copy_from_user(&daemon_flag, argp, sizeof(daemon_flag))) {
		thp_log_err("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}

	thp_log_info("%s called,daemon_flag = %u\n", __func__, daemon_flag);
	if (daemon_flag) {
		atomic_set(&(cd->fw_update_protect), 1);
	} else {
		atomic_set(&(cd->fw_update_protect), 0);
		if (atomic_read(&(cd->resend_suspend_after_fw_update)) == 1) {
			thp_log_info("%s: fw update complete, need resend suspend cmd\n",
				__func__);
			atomic_set(&(cd->resend_suspend_after_fw_update), 0);
			thp_set_status(THP_STATUS_POWER, THP_SUSPEND);
			mdelay(5); /* delay 5ms to wait for daemon reading status */
#if defined(CONFIG_LCD_KIT_DRIVER)
			thp_power_control_notify(TS_BEFORE_SUSPEND, 0);
#endif
		}
	}
	return 0;
}

static unsigned int get_recovery_flag(void)
{
	unsigned int recovery_flag = 0;

#if ((!defined CONFIG_HUAWEI_THP_QCOM) && (!defined CONFIG_HUAWEI_THP_MTK))
	recovery_flag = get_boot_into_recovery_flag();
#endif
#ifdef CONFIG_HUAWEI_THP_QCOM
	recovery_flag = get_boot_into_recovery_flag_qcom();
#endif
	return recovery_flag;
}

unsigned int thp_get_finger_resolution_magnification(void)
{
	unsigned int ret;
	unsigned int recovery_flag;
	struct thp_core_data *cd = thp_get_core_data();

	recovery_flag = get_recovery_flag();
	if ((cd->finger_resolution_magnification > MAX_MAGNIFICATION) ||
		(cd->finger_resolution_magnification == 0) ||
		(recovery_flag != 0))
		ret = DEFAULT_MAGNIFICATION;
	else
		ret = cd->finger_resolution_magnification;
	thp_log_info("%s:recovery_flag = %u, ret = %u\n",
		__func__, recovery_flag, ret);
	return ret;
}

static int  thp_mt_ioctl_read_finger_resolution_magnification(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	unsigned int magnification;

	if (argp == NULL) {
		thp_log_err("%s:argp is NULL\n", __func__);
		return -EINVAL;
	}
	magnification = thp_get_finger_resolution_magnification();
	if (copy_to_user(argp, (void *)&magnification, sizeof(magnification))) {
		thp_log_err("%s: Failed to copy_to_user()\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static int thp_daemon_power_reset(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();
	u32 daemon_flag;
#if defined(CONFIG_LCD_KIT_DRIVER)
	int err;
#endif

	if (cd->multi_panel_index == SINGLE_TOUCH_PANEL) {
		thp_log_err("%s: not support daemon power reset\n", __func__);
		return 0;
	}

	if (copy_from_user(&daemon_flag, argp, sizeof(daemon_flag))) {
		thp_log_err("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}

	thp_log_info("%s called,daemon_flag = %u\n", __func__, daemon_flag);
	if ((daemon_flag != MAIN_TOUCH_PANEL) &&
		(daemon_flag != SUB_TOUCH_PANEL)) {
		thp_log_err("%s invalid arg\n", __func__);
		return -EINVAL;
	}
#if defined(CONFIG_LCD_KIT_DRIVER)
	err = thp_multi_power_control_notify(TS_EARLY_SUSPEND,
		SHORT_SYNC_TIMEOUT, daemon_flag);
	if (err)
		thp_log_err("%s: TS_EARLY_SUSPEND fail\n", __func__);
	msleep(200); /* delay 200ms to wait ic suspend done */
	err = thp_multi_power_control_notify(TS_RESUME_DEVICE,
		SHORT_SYNC_TIMEOUT, daemon_flag);
	if (err)
		thp_log_err("%s: TS_EARLY_SUSPEND fail\n", __func__);
#endif
	return 0;
}

static int ioctl_get_app_info(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_app_info app_info;

	memset(&app_info, 0, sizeof(app_info));
	if (copy_from_user(&app_info, argp, sizeof(app_info))) {
		thp_log_err("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	if ((app_info.len > MAX_APP_INFO_LEN) || (app_info.len <= 0) ||
		(app_info.buf == NULL)) {
		thp_log_err("%s invalid data\n", __func__);
		return -EINVAL;
	}
	if (copy_to_user(app_info.buf, cd->app_info, app_info.len)) {
		thp_log_err("%s Failed to copy_to_user\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static long thp_mt_wrapper_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	long ret;
	struct thp_core_data *cd = NULL;

	switch (cmd) {
	case INPUT_MT_WRAPPER_IOCTL_CMD_SET_COORDINATES:
		ret = thp_mt_wrapper_ioctl_set_coordinate(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_CMD_REPORT_PEN:
		ret = thp_mt_wrapper_ioctl_report_pen(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_READ_STATUS:
		ret = thp_mt_wrapper_ioctl_read_status(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_READ_INPUT_CONFIG:
		ret = thp_mt_ioctl_read_input_config(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_READ_FINGER_RESOLUTION_MAGNIFICATION:
		ret = thp_mt_ioctl_read_finger_resolution_magnification(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_READ_SCENE_INFO:
		ret = thp_mt_wrapper_ioctl_read_scene_info(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_GET_WINDOW_INFO:
		ret = thp_mt_wrapper_ioctl_get_window_info(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_GET_PROJECT_ID:
		ret = thp_mt_wrapper_ioctl_get_projectid(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_CMD_SET_EVENTS:
		ret = thp_mt_wrapper_ioctl_set_events(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_CMD_GET_EVENTS:
		ret = thp_mt_wrapper_ioctl_get_events(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_SET_ROI_DATA:
		ret = thp_mt_wrapper_ioctl_set_roi_data(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_CMD_REPORT_KEYEVENT:
		ret = thp_mt_ioctl_report_keyevent(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_REPORT_SYSTEM_KEYEVENT:
		ret = thp_mt_ioctl_report_system_keyevent(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_GET_PLATFORM_TYPE:
		ret = thp_mt_wrapper_ioctl_get_platform_type(arg);
		break;
#if ((defined CONFIG_HUAWEI_SHB_THP) || (defined CONFIG_HUAWEI_THP_MTK))
	case INPUT_MT_WRAPPER_IOCTL_CMD_SHB_EVENT:
		ret = thp_mt_ioctl_cmd_shb_event(arg);
		break;
#endif
	case INPUT_MT_WRAPPER_IOCTL_GET_VOMLUME_SIDE:
		ret = thp_ioctl_get_volume_side(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_GET_POWER_SWITCH:
		ret = thp_ioctl_get_power_switch(arg);
		break;
	case INPUT_MT_WRAPPER_IOCTL_GET_APP_INFO:
		ret = ioctl_get_app_info(arg);
		break;
	case INPUT_MT_IOCTL_CMD_GET_STYLUS3_CONNECT_STATUS:
		cd = thp_get_core_data();
		if (!cd) {
			thp_log_err("%s:have null ptr\n", __func__);
			return -EINVAL;
		}
		if ((cd->pen_supported) && (cd->pen_mt_enable_flag))
			ret = thp_ioctl_get_stylus3_connect_status(arg);
		break;
	case INPUT_MT_IOCTRL_CMD_SET_STYLUS3_CONNECT_STATUS:
		cd = thp_get_core_data();
		if (!cd) {
			thp_log_err("%s:have null ptr\n", __func__);
			return -EINVAL;
		}
		if (cd->pen_change_protocol) {
			ret = set_stylus3_change_protocol(arg);
			return ret;
		}
		if ((cd->pen_supported) && (cd->pen_mt_enable_flag))
			ret = thp_ioctl_set_stylus3_connect_status(arg);
		break;
	case INPUT_MT_IOCTL_CMD_GET_CALLBACK_EVENTS:
		ret = thp_ioctl_get_callback_events(arg);
		break;
	case INPUT_MT_IOCTL_CMD_SET_CALLBACK_EVENTS:
		ret = thp_ioctl_set_callback_events(arg);
		break;
	case INPUT_MT_IOCTL_CMD_SET_DAEMON_INIT_PROTECT:
		ret = daemon_init_protect(arg);
		break;
	case INPUT_MT_IOCTL_CMD_SET_DAEMON_POWER_RESET:
		ret = thp_daemon_power_reset(arg);
		break;
	case INPUT_MT_IOCTL_CMD_SET_STYLUS3_PLAM_SUPPRESSION_STATUS:
		ret = thp_ioctl_set_stylus3_plam_suppression(arg);
		break;
	default:
		thp_log_err("%s: cmd unknown, cmd = 0x%x\n", __func__, cmd);
		ret = -EINVAL;
	}

	return ret;
}

int thp_mt_wrapper_wakeup_poll(void)
{
	if (!g_thp_mt_wrapper) {
		thp_log_err("%s: wrapper not init\n", __func__);
		return -ENODEV;
	}
	atomic_inc(&g_thp_mt_wrapper->status_updated);
	wake_up_interruptible(&g_thp_mt_wrapper->wait);
	return 0;
}

static unsigned int thp_mt_wrapper_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	thp_log_debug("%s:poll call in\n", __func__);
	poll_wait(file, &g_thp_mt_wrapper->wait, wait);
	if (atomic_read(&g_thp_mt_wrapper->status_updated) > 0)
		mask |= POLLIN | POLLRDNORM;

	thp_log_debug("%s:poll call out, mask = 0x%x\n", __func__, mask);
	return mask;
}

static const struct file_operations g_thp_mt_wrapper_fops = {
	.owner = THIS_MODULE,
	.open = thp_mt_wrapper_open,
	.release = thp_mt_wrapper_release,
	.unlocked_ioctl = thp_mt_wrapper_ioctl,
	.compat_ioctl = thp_mt_wrapper_ioctl,
	.poll = thp_mt_wrapper_poll,
};

static struct miscdevice g_thp_mt_wrapper_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &g_thp_mt_wrapper_fops,
};

static void set_default_input_config(struct thp_input_dev_config *input_config)
{
	input_config->abs_max_x = THP_MT_WRAPPER_MAX_X;
	input_config->abs_max_y = THP_MT_WRAPPER_MAX_Y;
	input_config->abs_max_z = THP_MT_WRAPPER_MAX_Z;
	input_config->major_max = THP_MT_WRAPPER_MAX_MAJOR;
	input_config->minor_max = THP_MT_WRAPPER_MAX_MINOR;
	input_config->tool_type_max = THP_MT_WRAPPER_TOOL_TYPE_MAX;
	input_config->tracking_id_max = THP_MT_WRAPPER_MAX_FINGERS;
	input_config->orientation_min = THP_MT_WRAPPER_MIN_ORIENTATION;
	input_config->orientation_max = THP_MT_WRAPPER_MAX_ORIENTATION;
}

static int thp_projectid_to_vender_id(const char *project_id,
	unsigned int project_id_len, char *temp_buf, unsigned int len)
{
	if ((!project_id) || (!temp_buf)) {
		thp_log_err("%s: project id or temp buffer null\n", __func__);
		return -EINVAL;
	}
	if ((strlen(project_id) > project_id_len) ||
		(len < THP_PROJECTID_VENDOR_ID_LEN)) {
		thp_log_err("%s:project_id or temp_buf has a wrong length\n", __func__);
		return -EINVAL;
	}
	strncpy(temp_buf, project_id + THP_PROJECTID_PRODUCT_NAME_LEN +
		THP_PROJECTID_IC_NAME_LEN, THP_PROJECTID_VENDOR_ID_LEN);

	return 0;
}
static int thp_parse_input_config(struct thp_input_dev_config *config)
{
	int rc;
	unsigned int i;
	int ret;
	char temp_buf[THP_PROJECTID_VENDOR_ID_LEN + 1] = {0};
	struct device_node *thp_dev_node = NULL;
	struct thp_core_data *cd = thp_get_core_data();

	cd->finger_resolution_magnification = DEFAULT_MAGNIFICATION;
	if (cd->support_diff_resolution) {
		thp_log_info("%s: use different resolution\n", __func__);
		ret = thp_projectid_to_vender_id(cd->project_id,
			THP_PROJECT_ID_LEN + 1, temp_buf, sizeof(temp_buf));
		if (ret < 0) {
			thp_log_err("%s: get vendor id failed\n", __func__);
			goto use_default;
		}
		for (i = 0; i < ARRAY_SIZE(thp_input_compatible_table); i++) {
			if (!strncmp(thp_input_compatible_table[i].vendor_id,
				(const char *)temp_buf,
				strlen(thp_input_compatible_table[i].vendor_id))) {
				thp_dev_node = of_find_compatible_node(NULL, NULL,
					thp_input_compatible_table[i].compatible_name);
				break;
			}
		}
		/* if no compatible id-name pair in table, use default */
		if (i == ARRAY_SIZE(thp_input_compatible_table)) {
			thp_log_err("%s:vendor id:%s not in id_table\n", __func__, temp_buf);
			thp_dev_node = of_find_compatible_node(NULL, NULL,
				THP_INPUT_DEV_COMPATIBLE);
		}
	} else {
		thp_dev_node = of_find_compatible_node(NULL, NULL,
			THP_INPUT_DEV_COMPATIBLE);
	}
	if (!thp_dev_node) {
		thp_log_info("%s:not found node, use defatle config\n",
					__func__);
		goto use_default;
	}

	rc = of_property_read_u32(thp_dev_node, "abs_max_x",
						&config->abs_max_x);
	if (rc) {
		thp_log_err("%s:abs_max_x not config, use deault\n", __func__);
		config->abs_max_x = THP_MT_WRAPPER_MAX_X;
	}

	rc = of_property_read_u32(thp_dev_node, "abs_max_y",
						&config->abs_max_y);
	if (rc) {
		thp_log_err("%s:abs_max_y not config, use deault\n", __func__);
		config->abs_max_y = THP_MT_WRAPPER_MAX_Y;
	}

	rc = of_property_read_u32(thp_dev_node, "abs_max_z",
						&config->abs_max_z);
	if (rc) {
		thp_log_err("%s:abs_max_z not config, use deault\n", __func__);
		config->abs_max_z = THP_MT_WRAPPER_MAX_Z;
	}

	rc = of_property_read_u32(thp_dev_node, "tracking_id_max",
						&config->tracking_id_max);
	if (rc) {
		thp_log_err("%s:tracking_id_max not config, use deault\n",
				__func__);
		config->tracking_id_max = THP_MT_WRAPPER_MAX_FINGERS;
	}

	rc = of_property_read_u32(thp_dev_node, "major_max",
						&config->major_max);
	if (rc) {
		thp_log_err("%s:major_max not config, use deault\n", __func__);
		config->major_max = THP_MT_WRAPPER_MAX_MAJOR;
	}

	rc = of_property_read_u32(thp_dev_node, "minor_max",
						&config->minor_max);
	if (rc) {
		thp_log_err("%s:minor_max not config, use deault\n", __func__);
		config->minor_max = THP_MT_WRAPPER_MAX_MINOR;
	}

	rc = of_property_read_u32(thp_dev_node, "orientation_min",
						&config->orientation_min);
	if (rc) {
		thp_log_err("%s:orientation_min not config, use deault\n",
				__func__);
		config->orientation_min = THP_MT_WRAPPER_MIN_ORIENTATION;
	}

	rc = of_property_read_u32(thp_dev_node, "orientation_max",
					&config->orientation_max);
	if (rc) {
		thp_log_err("%s:orientation_max not config, use deault\n",
				__func__);
		config->orientation_max = THP_MT_WRAPPER_MAX_ORIENTATION;
	}

	rc = of_property_read_u32(thp_dev_node, "tool_type_max",
					&config->tool_type_max);
	if (rc) {
		thp_log_err("%s:tool_type_max not config, use deault\n",
				__func__);
		config->tool_type_max = THP_MT_WRAPPER_TOOL_TYPE_MAX;
	}
	rc = of_property_read_u32(thp_dev_node,
		"magnification_of_finger_resolution",
		&cd->finger_resolution_magnification);
	if (rc) {
		thp_log_err("%s:use deault magnification_of_finger_resolution :1\n",
			__func__);
		cd->finger_resolution_magnification = DEFAULT_MAGNIFICATION;
	}
	return 0;

use_default:
	set_default_input_config(config);
	return 0;
}

static int thp_parse_pen_input_config(struct thp_input_pen_dev_config *config)
{
	int rc = -EINVAL;
	struct device_node *thp_dev_node = NULL;

	if (config == NULL) {
		thp_log_err("%s: config is null\n", __func__);
		goto err;
	}
	thp_dev_node = of_find_compatible_node(NULL, NULL,
		THP_PEN_INPUT_DEV_COMPATIBLE);
	if (!thp_dev_node) {
		thp_log_info("%s:thp_dev_node not found\n", __func__);
		goto err;
	}

	rc = of_property_read_u32(thp_dev_node, "max_x",
		&config->max_x);
	if (rc) {
		thp_log_err("%s:max_x not config\n", __func__);
		goto err;
	}

	rc = of_property_read_u32(thp_dev_node, "max_y",
		&config->max_y);
	if (rc) {
		thp_log_err("%s:max_y not config\n", __func__);
		goto err;
	}

	rc = of_property_read_u32(thp_dev_node, "max_pressure",
		&config->pressure);
	if (rc) {
		thp_log_err("%s:pressure not config\n", __func__);
		config->pressure = THP_MT_WRAPPER_MAX_Z;
	}

	rc = of_property_read_u32(thp_dev_node, "max_tilt_x",
		&config->max_tilt_x);
	if (rc) {
		thp_log_err("%s:max_tilt_x not config\n", __func__);
		config->max_tilt_x = THP_PEN_WRAPPER_TILT_MAX_X;
	}

	rc = of_property_read_u32(thp_dev_node, "max_tilt_y",
		&config->max_tilt_y);
	if (rc) {
		thp_log_err("%s:max_tilt_y not config\n", __func__);
		config->max_tilt_y = THP_PEN_WRAPPER_TILT_MAX_X;
	}
err:
	return rc;
}

static int thp_set_pen_input_config(struct input_dev *pen_dev)
{
	if (pen_dev == NULL) {
		thp_log_err("%s:input null ptr\n", __func__);
		return -EINVAL;
	}

	pen_dev->evbit[0] |= BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	__set_bit(ABS_X, pen_dev->absbit);
	__set_bit(ABS_Y, pen_dev->absbit);
	__set_bit(ABS_TILT_X, pen_dev->absbit);
	__set_bit(ABS_TILT_Y, pen_dev->absbit);
	__set_bit(BTN_STYLUS, pen_dev->keybit);
	__set_bit(BTN_TOUCH, pen_dev->keybit);
	__set_bit(BTN_TOOL_PEN, pen_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, pen_dev->propbit);
	input_set_abs_params(pen_dev, ABS_X, 0,
		g_thp_mt_wrapper->input_pen_dev_config.max_x, 0, 0);
	input_set_abs_params(pen_dev, ABS_Y, 0,
		g_thp_mt_wrapper->input_pen_dev_config.max_y, 0, 0);
	input_set_abs_params(pen_dev, ABS_PRESSURE, 0,
		g_thp_mt_wrapper->input_pen_dev_config.pressure, 0, 0);
	input_set_abs_params(pen_dev, ABS_TILT_X,
		-1 * g_thp_mt_wrapper->input_pen_dev_config.max_tilt_x,
		g_thp_mt_wrapper->input_pen_dev_config.max_tilt_x, 0, 0);
	input_set_abs_params(pen_dev, ABS_TILT_Y,
		-1 * g_thp_mt_wrapper->input_pen_dev_config.max_tilt_y,
		g_thp_mt_wrapper->input_pen_dev_config.max_tilt_y, 0, 0);
	__set_bit(TS_STYLUS_WAKEUP_TO_MEMO, pen_dev->keybit);
	__set_bit(TS_STYLUS_WAKEUP_SCREEN_ON, pen_dev->keybit);
	return 0;
}

static int thp_set_extra_key_input_config(
	struct input_dev *extra_key_dev)
{
	if (extra_key_dev == NULL) {
		thp_log_err("%s:input null ptr\n", __func__);
		return -EINVAL;
	}
	__set_bit(EV_SYN, extra_key_dev->evbit);
	__set_bit(EV_KEY, extra_key_dev->evbit);
	__set_bit(KEY_VOLUME_UP, extra_key_dev->keybit);
	__set_bit(KEY_VOLUME_DOWN, extra_key_dev->keybit);
	__set_bit(KEY_POWER, extra_key_dev->keybit);
	__set_bit(KEY_VOLUME_MUTE, extra_key_dev->keybit);
	__set_bit(KEY_VOLUME_TRIG, extra_key_dev->keybit);

	return 0;
}

static int thp_input_pen_device_register(void)
{
	int rc;
	struct thp_input_pen_dev_config *pen_config = NULL;
	struct input_dev *pen_dev = input_allocate_device();

	if (pen_dev == NULL) {
		thp_log_err("%s:failed to allocate memory\n", __func__);
		rc = -ENOMEM;
		goto err_out;
	}

	pen_dev->name = TS_PEN_DEV_NAME;
	g_thp_mt_wrapper->pen_dev = pen_dev;
	pen_config = &g_thp_mt_wrapper->input_pen_dev_config;
	rc = thp_parse_pen_input_config(pen_config);
	if (rc)
		thp_log_err("%s: parse pen input config failed: %d\n",
			__func__, rc);

	rc = thp_set_pen_input_config(pen_dev);
	if (rc) {
		thp_log_err("%s:set input config failed : %d\n",
			__func__, rc);
		goto err_free_dev;
	}
	rc = input_register_device(pen_dev);
	if (rc) {
		thp_log_err("%s:input dev register failed : %d\n",
			__func__, rc);
		goto err_free_dev;
	}
	return rc;
err_free_dev:
	input_free_device(pen_dev);
err_out:
	return rc;
}

static int thp_input_extra_key_register(void)
{
	int rc;
	struct input_dev *extra_key = input_allocate_device();

	if (extra_key == NULL) {
		thp_log_err("%s:failed to allocate memory\n", __func__);
		rc = -ENOMEM;
		goto err_out;
	}

	extra_key->name = TS_EXTRA_KEY_DEV_NAME;
	g_thp_mt_wrapper->extra_key_dev = extra_key;

	rc = thp_set_extra_key_input_config(extra_key);
	if (rc) {
		thp_log_err("%s:set input config failed : %d\n",
			__func__, rc);
		goto err_free_dev;
	}
	rc = input_register_device(extra_key);
	if (rc) {
		thp_log_err("%s:input dev register failed : %d\n",
			__func__, rc);
		goto err_free_dev;
	}
	return rc;
err_free_dev:
	input_free_device(extra_key);
err_out:
	return rc;
}

int thp_mt_wrapper_init(void)
{
	struct input_dev *input_dev = NULL;
	static struct thp_mt_wrapper_data *mt_wrapper;
	struct thp_core_data *cd = thp_get_core_data();
	int rc;
	char node[MULTI_PANEL_NODE_BUF_LEN] = {0};
	unsigned int magnification;

	if (g_thp_mt_wrapper) {
		thp_log_err("%s:thp_mt_wrapper have inited, exit\n", __func__);
		return 0;
	}

	mt_wrapper = kzalloc(sizeof(struct thp_mt_wrapper_data), GFP_KERNEL);
	if (!mt_wrapper) {
		thp_log_err("%s:out of memory\n", __func__);
		return -ENOMEM;
	}
	init_waitqueue_head(&mt_wrapper->wait);

	input_dev = input_allocate_device();
	if (!input_dev) {
		thp_log_err("%s:Unable to allocated input device\n", __func__);
		kfree(mt_wrapper);
		return -ENODEV;
	}

	input_dev->name = THP_INPUT_DEVICE_NAME;

	rc = thp_parse_input_config(&mt_wrapper->input_dev_config);
	if (rc)
		thp_log_err("%s: parse config fail\n", __func__);

	__set_bit(EV_SYN, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	__set_bit(BTN_TOOL_FINGER, input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	__set_bit(KEY_F26, input_dev->keybit);
	__set_bit(TS_DOUBLE_CLICK, input_dev->keybit);
	__set_bit(KEY_FINGER_DOWN, input_dev->keybit);
	__set_bit(KEY_FINGER_UP, input_dev->keybit);
	__set_bit(TS_STYLUS_WAKEUP_TO_MEMO, input_dev->keybit);
	__set_bit(KEY_VOLUME_UP, input_dev->keybit);
	__set_bit(KEY_VOLUME_DOWN, input_dev->keybit);
	__set_bit(KEY_POWER, input_dev->keybit);
	__set_bit(TS_SINGLE_CLICK, input_dev->keybit);

	magnification = thp_get_finger_resolution_magnification();
	input_set_abs_params(input_dev, ABS_X, 0,
		(mt_wrapper->input_dev_config.abs_max_x * magnification) - 1,
		0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0,
		(mt_wrapper->input_dev_config.abs_max_y * magnification) - 1,
		0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE,
		0, mt_wrapper->input_dev_config.abs_max_z, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
		(mt_wrapper->input_dev_config.abs_max_x * magnification) - 1,
		0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
		(mt_wrapper->input_dev_config.abs_max_y * magnification) - 1,
		0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE,
			0, mt_wrapper->input_dev_config.abs_max_z, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
			mt_wrapper->input_dev_config.tracking_id_max - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
			0, mt_wrapper->input_dev_config.major_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR,
			0, mt_wrapper->input_dev_config.minor_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_ORIENTATION,
			mt_wrapper->input_dev_config.orientation_min,
			mt_wrapper->input_dev_config.orientation_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_BLOB_ID, 0,
			INPUT_MT_WRAPPER_MAX_FINGERS, 0, 0);
#ifdef TYPE_B_PROTOCOL
	input_mt_init_slots(input_dev, THP_MT_WRAPPER_MAX_FINGERS);
#endif

	rc = input_register_device(input_dev);
	if (rc) {
		thp_log_err("%s:failed to register input device\n", __func__);
		goto input_dev_reg_err;
	}

	if (cd->multi_panel_index != SINGLE_TOUCH_PANEL) {
		rc = snprintf(node, MULTI_PANEL_NODE_BUF_LEN, "%s%d",
			DEVICE_NAME, cd->multi_panel_index);
		if (rc < 0) {
			thp_log_err("%s: snprintf err\n", __func__);
			goto input_dev_reg_err;
		}

		g_thp_mt_wrapper_misc_device.name = (const char *)node;
		thp_log_info("%s misc name is :%s\n", __func__,
			g_thp_mt_wrapper_misc_device.name);
	}
	rc = misc_register(&g_thp_mt_wrapper_misc_device);
	if (rc) {
		thp_log_err("%s:failed to register misc device\n", __func__);
		goto misc_dev_reg_err;
	}

	mt_wrapper->input_dev = input_dev;
	g_thp_mt_wrapper = mt_wrapper;
	if (cd->pen_supported) {
		rc = thp_input_pen_device_register();
		if (rc)
			thp_log_err("%s:pen register failed\n", __func__);
	}
	if (cd->support_extra_key_event_input) {
		rc = thp_input_extra_key_register();
		if (rc)
			thp_log_err("%s:ring key register failed\n", __func__);
	}
	atomic_set(&g_thp_mt_wrapper->status_updated, 0);
	if ((cd->pen_supported) && (cd->pen_mt_enable_flag)) {
		atomic_set(&cd->last_stylus3_status, 0);
		atomic_set(&cd->callback_event_flag, false);
		init_completion(&cd->stylus3_status_flag);
		init_completion(&cd->stylus3_callback_flag);
	}
	return 0;

misc_dev_reg_err:
	input_unregister_device(input_dev);
input_dev_reg_err:
	kfree(mt_wrapper);

	return rc;
}
EXPORT_SYMBOL(thp_mt_wrapper_init);

void thp_mt_wrapper_exit(void)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (!g_thp_mt_wrapper)
		return;

	input_unregister_device(g_thp_mt_wrapper->input_dev);
	if (cd->pen_supported)
		input_unregister_device(g_thp_mt_wrapper->pen_dev);
	misc_deregister(&g_thp_mt_wrapper_misc_device);
}
EXPORT_SYMBOL(thp_mt_wrapper_exit);

