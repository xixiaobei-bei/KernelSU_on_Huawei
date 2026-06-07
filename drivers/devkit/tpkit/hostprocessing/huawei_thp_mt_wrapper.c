/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#if defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif

#ifdef CONFIG_INPUTHUB_20
#include "contexthub_recovery.h"
#endif

#ifdef CONFIG_HUAWEI_PS_SENSOR
#include "ps_sensor.h"
#endif

#if defined(CONFIG_HUAWEI_TS_KIT_3_0)
#include "../3_0/trace-events-touch.h"
#else
#define trace_touch(x...)
#endif

#define DEVICE_NAME "input_mt_wrapper"

static struct thp_mt_wrapper_data *g_thp_mt_wrapper;

void thp_inputkey_report(unsigned int gesture_wakeup_value)
{
	input_report_key(g_thp_mt_wrapper->input_dev, gesture_wakeup_value, 1);
	input_sync(g_thp_mt_wrapper->input_dev);
	input_report_key(g_thp_mt_wrapper->input_dev, gesture_wakeup_value, 0);
	input_sync(g_thp_mt_wrapper->input_dev);
	THP_LOG_INFO("%s ->done\n", __func__);
}

void thp_input_pen_report(unsigned int pen_event_value)
{
	input_report_key(g_thp_mt_wrapper->pen_dev, pen_event_value, 1);
	input_sync(g_thp_mt_wrapper->pen_dev);
	input_report_key(g_thp_mt_wrapper->pen_dev, pen_event_value, 0);
	input_sync(g_thp_mt_wrapper->pen_dev);
	THP_LOG_INFO("%s:done\n", __func__);
}

int thp_mt_wrapper_ioctl_get_events(unsigned long event)
{
	int t = 0;
	int __user *events = (int *)event;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!events)) {
		THP_LOG_INFO("%s: input null\n", __func__);
		return -ENODEV;
	}

	THP_LOG_INFO("%d: cd->event_flag\n", cd->event_flag);
	if (cd->event_flag) {
		if (copy_to_user(events, &cd->event, sizeof(cd->event))) {
			THP_LOG_ERR("%s:copy events failed\n", __func__);
			return -EFAULT;
		}

		cd->event_flag = false;
	} else {
		cd->thp_event_waitq_flag = WAITQ_WAIT;
		t = wait_event_interruptible(cd->thp_event_waitq,
			(cd->thp_event_waitq_flag == WAITQ_WAKEUP));
		THP_LOG_INFO("%s: set wait finish :%d\n", __func__, t);
	}

	return 0;
}

static long thp_mt_wrapper_ioctl_set_coordinate(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;
	struct thp_mt_wrapper_ioctl_touch_data data;
	u8 i;

	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_IN, "thp");
	if (arg == 0) {
		THP_LOG_ERR("%s:arg is null\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&data, argp,
			sizeof(struct thp_mt_wrapper_ioctl_touch_data))) {
		THP_LOG_ERR("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_OUT, "thp");

	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_IN, "thp");
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
	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_OUT, "thp");
	return 0;
}

void thp_clean_fingers(void)
{
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;
	struct thp_mt_wrapper_ioctl_touch_data data;

	memset(&data, 0, sizeof(data));

	input_mt_sync(input_dev);
	input_sync(input_dev);

	input_report_key(input_dev, BTN_TOUCH, 0);
	input_sync(input_dev);
}

static int thp_mt_wrapper_open(struct inode *inode, struct file *filp)
{
	THP_LOG_INFO("%s:called\n", __func__);
	return 0;
}

static int thp_mt_wrapper_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int thp_mt_wrapper_ioctl_read_status(unsigned long arg)
{
	int __user *status = (int *)arg;
	u32 thp_status = thp_get_status_all();

	THP_LOG_INFO("%s:status=%d\n", __func__, thp_status);

	if (!status) {
		THP_LOG_ERR("%s:input null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(status, &thp_status, sizeof(u32))) {
		THP_LOG_ERR("%s:copy status failed\n", __func__);
		return -EFAULT;
	}

	if (atomic_read(&g_thp_mt_wrapper->status_updated) != 0)
		atomic_dec(&g_thp_mt_wrapper->status_updated);

	return 0;
}

static int thp_mt_ioctl_read_input_config(unsigned long arg)
{
	struct thp_input_dev_config __user *config =
			(struct thp_input_dev_config *)arg;
	struct thp_input_dev_config *input_config =
			&g_thp_mt_wrapper->input_dev_config;

	if (!config) {
		THP_LOG_ERR("%s:input null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(config, input_config,
			sizeof(struct thp_input_dev_config))) {
		THP_LOG_ERR("%s:copy input config failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_mt_wrapper_ioctl_read_scene_info(unsigned long arg)
{
	struct thp_scene_info __user *config = (struct thp_scene_info *)arg;
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_scene_info *scene_info = NULL;

	if (!cd) {
		THP_LOG_ERR("%s:thp_core_data is NULL\n", __func__);
		return -EINVAL;
	}
	scene_info = &(cd->scene_info);

	THP_LOG_INFO("%s:%d,%d,%d\n", __func__,
		scene_info->type, scene_info->status, scene_info->parameter);

	if (copy_to_user(config, scene_info, sizeof(struct thp_scene_info))) {
		THP_LOG_ERR("%s:copy scene_info failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_mt_wrapper_ioctl_get_window_info(unsigned long arg)
{
	struct thp_window_info __user *window_info =
				(struct thp_window_info *)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!window_info)) {
		THP_LOG_ERR("%s:args error\n", __func__);
		return -EINVAL;
	}

	THP_LOG_INFO("%s:x0=%d,y0=%d,x1=%d,y1=%d\n", __func__,
		cd->window.x0, cd->window.y0, cd->window.x1, cd->window.y1);

	if (copy_to_user(window_info, &cd->window,
		sizeof(struct thp_window_info))) {
		THP_LOG_ERR("%s:copy window_info failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_mt_wrapper_ioctl_get_projectid(unsigned long arg)
{
	char __user *project_id = (char __user *)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!project_id)) {
		THP_LOG_ERR("%s:args error\n", __func__);
		return -EINVAL;
	}

	THP_LOG_INFO("%s:project id:%s\n", __func__, cd->project_id);

	if (copy_to_user(project_id, cd->project_id, sizeof(cd->project_id))) {
		THP_LOG_ERR("%s:copy project_id failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int thp_mt_wrapper_ioctl_set_roi_data(unsigned long arg)
{
	short __user *roi_data = (short __user *)arg;
	struct thp_core_data *cd = thp_get_core_data();

	if ((!cd) || (!roi_data)) {
		THP_LOG_ERR("%s:args error\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(cd->roi_data, roi_data, sizeof(cd->roi_data))) {
		THP_LOG_ERR("%s:copy roi data failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_mt_wrapper_ioctl_set_events(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	void __user *argp = (void __user *)arg;
	int val;

	if (arg == 0) {
		THP_LOG_ERR("%s:arg is null\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&val, argp,
			sizeof(int))) {
		THP_LOG_ERR("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	THP_LOG_INFO("thp_send, write: %d\n", val);
	cd->event_flag = true;
	cd->event = val;
	if (cd->event_flag) {
		cd->thp_event_waitq_flag = WAITQ_WAKEUP;
		wake_up_interruptible(&cd->thp_event_waitq);
		THP_LOG_INFO("%d: wake_up\n", cd->event);
	}

	return 0;
}

static int thp_mt_ioctl_report_keyevent(unsigned long arg)
{
	int report_value[PROX_VALUE_LEN] = {0};
	struct input_dev *input_dev = g_thp_mt_wrapper->input_dev;
	void __user *argp = (void __user *)arg;
	enum input_mt_wrapper_keyevent keyevent;

	if (arg == 0) {
		THP_LOG_ERR("%s:arg is null\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&keyevent, argp,
			sizeof(enum input_mt_wrapper_keyevent))) {
		THP_LOG_ERR("Failed to copy_from_user()\n");
		return -EFAULT;
	}

	if (keyevent == INPUT_MT_WRAPPER_KEYEVENT_ESD) {
		input_report_key(input_dev, KEY_F26, 1);
		input_sync(input_dev);
		input_report_key(input_dev, KEY_F26, 0);
		input_sync(input_dev);
	} else if (keyevent == INPUT_MT_WRAPPER_KEYEVENT_APPROACH) {
		THP_LOG_INFO("[Proximity_feature] %s: report [near] event!\n",
			__func__);
		report_value[0] = APPROCH_EVENT_VALUE;
#if (defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR)
		thp_prox_event_report(report_value, PROX_EVENT_LEN);
#endif
	} else if (keyevent == INPUT_MT_WRAPPER_KEYEVENT_AWAY) {
		THP_LOG_INFO("[Proximity_feature] %s: report [far] event!\n",
			__func__);
		report_value[0] = AWAY_EVENT_VALUE;
#if (defined CONFIG_INPUTHUB_20) || (defined CONFIG_HUAWEI_PS_SENSOR)
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
		THP_LOG_INFO("%s: input null\n", __func__);
		return -ENODEV;
	}

	THP_LOG_INFO("%s: cd->platform_type %d\n", __func__, cd->platform_type);

	if (copy_to_user(platform_type, &cd->platform_type,
					sizeof(cd->platform_type))) {
		THP_LOG_ERR("%s:copy platform_type failed\n", __func__);
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
		THP_LOG_ERR("%s:input is invalid\n", __func__);
		return -EINVAL;
	}
	input_dev = g_thp_mt_wrapper->extra_key_dev;
	THP_LOG_INFO("%s Ring-Vibrate : key: %d, value: %d\n",
		__func__, key_info->key, key_info->action);
	if ((key_info->key != KEY_VOLUME_UP) &&
		(key_info->key != KEY_VOLUME_DOWN) &&
		(key_info->key != KEY_POWER) &&
		(key_info->key != KEY_VOLUME_MUTE) &&
		(key_info->key != KEY_VOLUME_TRIG)) {
		THP_LOG_ERR("%s:key is invalid\n", __func__);
		return -EINVAL;
	}
	if ((key_info->action != THP_KEY_UP) &&
		(key_info->action != THP_KEY_DOWN)) {
		THP_LOG_ERR("%s:action is invalid\n", __func__);
		return -EINVAL;
	}

	input_report_key(input_dev, key_info->key,
		key_info->action);
	input_sync(input_dev);
	return 0;
}

static int thp_mt_ioctl_report_system_keyevent(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct thp_key_info key_info;

	if (arg == 0) {
		THP_LOG_ERR("%s:arg is null\n", __func__);
		return -EINVAL;
	}
	memset(&key_info, 0, sizeof(key_info));
	if (copy_from_user(&key_info, argp, sizeof(key_info))) {
		THP_LOG_ERR("Failed to copy_from_user()\n");
		return -EFAULT;
	}
	return thp_report_system_event(&key_info);
}

#ifdef CONFIG_HUAWEI_SHB_THP
int thp_send_volumn_to_drv(const char * head)
{
	struct thp_volumn_info *rx = (struct thp_volumn_info *)head;
	struct thp_key_info key_info;
	struct thp_core_data *cd = thp_get_core_data();

	if ((rx == NULL) || (cd == NULL)) {
		THP_LOG_ERR("%s:rx or cd is null\n", __func__);
		return -EINVAL;
	}
	if (!atomic_read(&cd->register_flag)) {
		THP_LOG_ERR("%s: thp have not be registered\n", __func__);
		return -ENODEV;
	}
	__pm_wakeup_event(&cd->thp_wake_lock, jiffies_to_msecs(HZ));
	THP_LOG_INFO("thp_send_volumn_to_drv, key:%ud, action:%ud\n",
		rx->data[0], rx->data[1]);
	key_info.key = rx->data[0];
	key_info.action = rx->data[1];
	return thp_report_system_event(&key_info);
}

static int thp_event_info_dispatch(struct thp_shb_info info)
{
	int ret;
	unsigned int cmd_type = info.cmd_type;
	uint8_t cmd = ST_CMD_TYPE_MAX;

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
		THP_LOG_ERR("%s: thp_shb_info is null\n", __func__);
		ret = -EFAULT;
	}
	return ret;
}

static int thp_mt_ioctl_cmd_shb_event(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int ret;
	struct thp_shb_info data;
	char *cmd_data = NULL;

	if (arg == 0) {
		THP_LOG_ERR("%s:arg is null.\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&data, argp, sizeof(struct thp_shb_info))) {
		THP_LOG_ERR("%s:copy info failed\n", __func__);
		return -EFAULT;
	}
	if ((data.cmd_len > MAX_THP_CMD_INFO_LEN) || (data.cmd_len == 0)) {
		THP_LOG_ERR("%s:cmd_len:%u is illegal\n", __func__, data.cmd_len);
		return 0;
	}
	cmd_data = kzalloc(data.cmd_len, GFP_KERNEL);
	if (cmd_data == NULL) {
		THP_LOG_ERR("%s:cmd buffer kzalloc failed\n", __func__);
		return -EFAULT;
	}
	if (copy_from_user(cmd_data, data.cmd_addr, data.cmd_len)) {
		THP_LOG_ERR("%s:copy cmd data failed\n", __func__);
		kfree(cmd_data);
		return -EFAULT;
	}
	data.cmd_addr = cmd_data;
	ret = thp_event_info_dispatch(data);
	if (ret < 0) {
		THP_LOG_ERR("%s:thp event info dispatch failed\n", __func__);
	}
	kfree(cmd_data);
	return 0;
}
#endif

static long thp_ioctl_get_volume_side(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	void __user *status = (void __user *)(uintptr_t)arg;

	if (cd == NULL) {
		THP_LOG_ERR("%s: thp cord data null\n", __func__);
		return -EINVAL;
	}
	if (status == NULL) {
		THP_LOG_ERR("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(status, (void *)&cd->volume_side_status,
		sizeof(cd->volume_side_status))) {
		THP_LOG_ERR("%s: get volume side failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static long thp_ioctl_get_power_switch(unsigned long arg)
{
	struct thp_core_data *cd = thp_get_core_data();
	void __user *status = (void __user *)(uintptr_t)arg;

	if ((cd == NULL) || (status == NULL)) {
		THP_LOG_ERR("%s: thp cord data null\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(status, (void *)&cd->power_switch,
		sizeof(cd->power_switch))) {
		THP_LOG_ERR("%s: get power_switch failed\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static void thp_report_pen_event(struct input_dev *input, struct thp_tool tool,
	int pressure, int tool_type, int tool_value)
{
	if (input == NULL) {
		THP_LOG_ERR("%s: input null ptr\n", __func__);
		return;
	}

	THP_LOG_DEBUG("%s:tool.tip_status:%d, tool_type:%d, tool_value:%d\n",
		__func__, tool.tip_status, tool_type, tool_value);
	input_report_abs(input, ABS_X, tool.x);
	input_report_abs(input, ABS_Y, tool.y);
	input_report_abs(input, ABS_PRESSURE, pressure);

	input_report_key(input, BTN_TOUCH, tool.tip_status);
	input_report_key(input, tool_type, tool_value);
	input_sync(input);
}

static int thp_mt_wrapper_ioctl_report_pen(unsigned long arg)
{
	struct thp_mt_wrapper_ioctl_pen_data pens;
	struct input_dev *input = g_thp_mt_wrapper->pen_dev;
	int i;
	int key_value;
	void __user *argp = (void __user *)(uintptr_t)arg;

	if (arg == 0) {
		THP_LOG_ERR("%s:arg is null\n", __func__);
		return -EINVAL;
	}

	memset(&pens, 0, sizeof(pens));
	if (copy_from_user(&pens, argp, sizeof(pens))) {
		THP_LOG_ERR("Failed to copy_from_user\n");
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
			THP_LOG_ERR("pen index is %d\n", i);
			input_report_key(input, pens.buttons[i].key,
				key_value);
		}
	}

	/* pen or rubber report point */
	thp_report_pen_event(input, pens.tool, pens.tool.pressure,
		pens.tool.tool_type, pens.tool.pen_inrange_status);
	return 0;
}

static long thp_mt_wrapper_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	long ret;

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
#ifdef CONFIG_HUAWEI_SHB_THP
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
	default:
		THP_LOG_ERR("cmd unknown\n");
		ret = -EINVAL;
	}

	return ret;
}

int thp_mt_wrapper_wakeup_poll(void)
{
	if (!g_thp_mt_wrapper) {
		THP_LOG_ERR("%s: wrapper not init\n", __func__);
		return -ENODEV;
	}
	atomic_inc(&g_thp_mt_wrapper->status_updated);
	wake_up_interruptible(&g_thp_mt_wrapper->wait);
	return 0;
}

static unsigned int thp_mt_wrapper_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	THP_LOG_DEBUG("%s:poll call in\n", __func__);
	poll_wait(file, &g_thp_mt_wrapper->wait, wait);
	if (atomic_read(&g_thp_mt_wrapper->status_updated) > 0)
		mask |= POLLIN | POLLRDNORM;

	THP_LOG_DEBUG("%s:poll call out, mask = 0x%x\n", __func__, mask);
	return mask;
}

static const struct file_operations g_thp_mt_wrapper_fops = {
	.owner = THIS_MODULE,
	.open = thp_mt_wrapper_open,
	.release = thp_mt_wrapper_release,
	.unlocked_ioctl = thp_mt_wrapper_ioctl,
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
static int thp_parse_input_config(struct thp_input_dev_config *config)
{
	int rc;
	struct device_node *thp_dev_node = NULL;

	thp_dev_node = of_find_compatible_node(NULL, NULL,
					THP_INPUT_DEV_COMPATIBLE);
	if (!thp_dev_node) {
		THP_LOG_INFO("%s:not found node, use defatle config\n",
					__func__);
		goto use_defaule;
	}

	rc = of_property_read_u32(thp_dev_node, "abs_max_x",
						&config->abs_max_x);
	if (rc) {
		THP_LOG_ERR("%s:abs_max_x not config, use deault\n", __func__);
		config->abs_max_x = THP_MT_WRAPPER_MAX_X;
	}

	rc = of_property_read_u32(thp_dev_node, "abs_max_y",
						&config->abs_max_y);
	if (rc) {
		THP_LOG_ERR("%s:abs_max_y not config, use deault\n", __func__);
		config->abs_max_y = THP_MT_WRAPPER_MAX_Y;
	}

	rc = of_property_read_u32(thp_dev_node, "abs_max_z",
						&config->abs_max_z);
	if (rc) {
		THP_LOG_ERR("%s:abs_max_z not config, use deault\n", __func__);
		config->abs_max_z = THP_MT_WRAPPER_MAX_Z;
	}

	rc = of_property_read_u32(thp_dev_node, "tracking_id_max",
						&config->tracking_id_max);
	if (rc) {
		THP_LOG_ERR("%s:tracking_id_max not config, use deault\n",
				__func__);
		config->tracking_id_max = THP_MT_WRAPPER_MAX_FINGERS;
	}

	rc = of_property_read_u32(thp_dev_node, "major_max",
						&config->major_max);
	if (rc) {
		THP_LOG_ERR("%s:major_max not config, use deault\n", __func__);
		config->major_max = THP_MT_WRAPPER_MAX_MAJOR;
	}

	rc = of_property_read_u32(thp_dev_node, "minor_max",
						&config->minor_max);
	if (rc) {
		THP_LOG_ERR("%s:minor_max not config, use deault\n", __func__);
		config->minor_max = THP_MT_WRAPPER_MAX_MINOR;
	}

	rc = of_property_read_u32(thp_dev_node, "orientation_min",
						&config->orientation_min);
	if (rc) {
		THP_LOG_ERR("%s:orientation_min not config, use deault\n",
				__func__);
		config->orientation_min = THP_MT_WRAPPER_MIN_ORIENTATION;
	}

	rc = of_property_read_u32(thp_dev_node, "orientation_max",
					&config->orientation_max);
	if (rc) {
		THP_LOG_ERR("%s:orientation_max not config, use deault\n",
				__func__);
		config->orientation_max = THP_MT_WRAPPER_MAX_ORIENTATION;
	}

	rc = of_property_read_u32(thp_dev_node, "tool_type_max",
					&config->tool_type_max);
	if (rc) {
		THP_LOG_ERR("%s:tool_type_max not config, use deault\n",
				__func__);
		config->tool_type_max = THP_MT_WRAPPER_TOOL_TYPE_MAX;
	}

	return 0;

use_defaule:
	set_default_input_config(config);
	return 0;
}

static int thp_parse_pen_input_config(struct thp_input_pen_dev_config *config)
{
	int rc = -EINVAL;
	struct device_node *thp_dev_node = NULL;

	if (config == NULL) {
		THP_LOG_ERR("%s: config is null\n", __func__);
		goto err;
	}
	thp_dev_node = of_find_compatible_node(NULL, NULL,
		THP_PEN_INPUT_DEV_COMPATIBLE);
	if (!thp_dev_node) {
		THP_LOG_INFO("%s:thp_dev_node not found\n", __func__);
		goto err;
	}

	rc = of_property_read_u32(thp_dev_node, "max_x",
		&config->max_x);
	if (rc) {
		THP_LOG_ERR("%s:max_x not config\n", __func__);
		goto err;
	}

	rc = of_property_read_u32(thp_dev_node, "max_y",
		&config->max_y);
	if (rc) {
		THP_LOG_ERR("%s:max_y not config\n", __func__);
		goto err;
	}

	rc = of_property_read_u32(thp_dev_node, "max_pressure",
		&config->pressure);
	if (rc) {
		THP_LOG_ERR("%s:pressure not config\n", __func__);
		config->pressure = THP_MT_WRAPPER_MAX_Z;
	}

	rc = of_property_read_u32(thp_dev_node, "max_tilt_x",
		&config->max_tilt_x);
	if (rc) {
		THP_LOG_ERR("%s:max_tilt_x not config\n", __func__);
		config->max_tilt_x = THP_PEN_WRAPPER_TILT_MAX_X;
	}

	rc = of_property_read_u32(thp_dev_node, "max_tilt_y",
		&config->max_tilt_y);
	if (rc) {
		THP_LOG_ERR("%s:max_tilt_y not config\n", __func__);
		config->max_tilt_y = THP_PEN_WRAPPER_TILT_MAX_X;
	}
err:
	return rc;
}

static int thp_set_pen_input_config(struct input_dev *pen_dev)
{
	if (pen_dev == NULL) {
		THP_LOG_ERR("%s:input null ptr\n", __func__);
		return -EINVAL;
	}

	pen_dev->evbit[0] |= BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	__set_bit(ABS_X, pen_dev->absbit);
	__set_bit(ABS_Y, pen_dev->absbit);
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
	__set_bit(TS_STYLUS_WAKEUP_TO_MEMO, pen_dev->keybit);
	__set_bit(TS_STYLUS_WAKEUP_SCREEN_ON, pen_dev->keybit);
	return 0;
}

static int thp_set_extra_key_input_config(
	struct input_dev *extra_key_dev)
{
	if (extra_key_dev == NULL) {
		THP_LOG_ERR("%s:input null ptr\n", __func__);
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
		THP_LOG_ERR("%s:failed to allocate memory\n", __func__);
		rc = -ENOMEM;
		goto err_out;
	}

	pen_dev->name = TS_PEN_DEV_NAME;
	g_thp_mt_wrapper->pen_dev = pen_dev;
	pen_config = &g_thp_mt_wrapper->input_pen_dev_config;
	rc = thp_parse_pen_input_config(pen_config);
	if (rc)
		THP_LOG_ERR("%s: parse pen input config failed: %d\n",
			__func__, rc);

	rc = thp_set_pen_input_config(pen_dev);
	if (rc) {
		THP_LOG_ERR("%s:set input config failed : %d\n",
			__func__, rc);
		goto err_free_dev;
	}
	rc = input_register_device(pen_dev);
	if (rc) {
		THP_LOG_ERR("%s:input dev register failed : %d\n",
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
		THP_LOG_ERR("%s:failed to allocate memory\n", __func__);
		rc = -ENOMEM;
		goto err_out;
	}

	extra_key->name = TS_EXTRA_KEY_DEV_NAME;
	g_thp_mt_wrapper->extra_key_dev = extra_key;

	rc = thp_set_extra_key_input_config(extra_key);
	if (rc) {
		THP_LOG_ERR("%s:set input config failed : %d\n",
			__func__, rc);
		goto err_free_dev;
	}
	rc = input_register_device(extra_key);
	if (rc) {
		THP_LOG_ERR("%s:input dev register failed : %d\n",
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
	static struct thp_mt_wrapper_data *mt_wrapper = NULL;
	struct thp_core_data *cd = thp_get_core_data();
	int rc;

	if (g_thp_mt_wrapper) {
		THP_LOG_ERR("%s:thp_mt_wrapper have inited, exit\n", __func__);
		return 0;
	}

	mt_wrapper = kzalloc(sizeof(struct thp_mt_wrapper_data), GFP_KERNEL);
	if (!mt_wrapper) {
		THP_LOG_ERR("%s:out of memory\n", __func__);
		return -ENOMEM;
	}
	init_waitqueue_head(&mt_wrapper->wait);

	input_dev = input_allocate_device();
	if (!input_dev) {
		THP_LOG_ERR("%s:Unable to allocated input device\n", __func__);
		kfree(mt_wrapper);
		return -ENODEV;
	}

	input_dev->name = THP_INPUT_DEVICE_NAME;

	rc = thp_parse_input_config(&mt_wrapper->input_dev_config);
	if (rc)
		THP_LOG_ERR("%s: parse config fail\n", __func__);

	__set_bit(EV_SYN, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	__set_bit(BTN_TOOL_FINGER, input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	__set_bit(KEY_F26, input_dev->keybit);
	__set_bit(TS_DOUBLE_CLICK, input_dev->keybit);
	__set_bit(KEY_VOLUME_UP, input_dev->keybit);
	__set_bit(KEY_VOLUME_DOWN, input_dev->keybit);
	__set_bit(KEY_POWER, input_dev->keybit);
	__set_bit(TS_SINGLE_CLICK, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, 0,
			mt_wrapper->input_dev_config.abs_max_x - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0,
			mt_wrapper->input_dev_config.abs_max_y - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE,
			0, mt_wrapper->input_dev_config.abs_max_z, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
			0, mt_wrapper->input_dev_config.abs_max_x - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
			0, mt_wrapper->input_dev_config.abs_max_y - 1, 0, 0);
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
		THP_LOG_ERR("%s:failed to register input device\n", __func__);
		goto input_dev_reg_err;
	}

	rc = misc_register(&g_thp_mt_wrapper_misc_device);
	if (rc) {
		THP_LOG_ERR("%s:failed to register misc device\n", __func__);
		goto misc_dev_reg_err;
	}

	mt_wrapper->input_dev = input_dev;
	g_thp_mt_wrapper = mt_wrapper;
	if (cd->pen_supported) {
		rc = thp_input_pen_device_register();
		if (rc)
			THP_LOG_ERR("%s:pen register failed\n", __func__);
	}
	if (cd->support_extra_key_event_input) {
		rc = thp_input_extra_key_register();
		if (rc)
			THP_LOG_ERR("%s:ring key register failed\n", __func__);
	}
	atomic_set(&g_thp_mt_wrapper->status_updated, 0);
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

