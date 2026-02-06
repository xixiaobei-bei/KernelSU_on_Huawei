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

#include <huawei_ts_kit.h>
#include <huawei_ts_kit_api.h>
#include <huawei_ts_kit_platform.h>
#include <huawei_ts_kit_misc.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <huawei_ts_kit_algo.h>

#include "trace-events-touch.h"
#include <linux/errno.h>

static unsigned int daemon_max_fingers_supproted;

/* copy 20 fingers info to 10 fingers */
static void copy_fold_to_normal_fingers(struct ts_normal_fingers *dst,
	const struct ts_fingers *src)
{
	if ((dst == NULL) || (src == NULL)) {
		TS_LOG_ERR("%s:input null ptr\n", __func__);
		return;
	}
	memcpy(dst->fingers, src->fingers, sizeof(dst->fingers));
	dst->add_release_flag = src->add_release_flag;
	dst->cur_finger_number = src->cur_finger_number;
	dst->gesture_wakeup_value = src->gesture_wakeup_value;
	dst->special_button_flag = src->special_button_flag;
	dst->special_button_key = src->special_button_key;
}

/* copy 10 fingers info to 20 fingers */
static void copy_normal_to_fold_fingers(struct ts_fingers *dst,
	const struct ts_normal_fingers *src)
{
	if ((dst == NULL) || (src == NULL)) {
		TS_LOG_ERR("%s:input null ptr\n", __func__);
		return;
	}
	memcpy(dst->fingers, src->fingers, sizeof(src->fingers));
	dst->add_release_flag = src->add_release_flag;
	dst->cur_finger_number = src->cur_finger_number;
	dst->gesture_wakeup_value = src->gesture_wakeup_value;
	dst->special_button_flag = src->special_button_flag;
	dst->special_button_key = src->special_button_key;
}

static long ts_ioctl_get_fingers_info(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_normal_fingers n_fingers;

	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	trace_touch(TOUCH_TRACE_ALGO_GET_DATA, TOUCH_TRACE_FUNC_IN,
		"with aft");
	/* wait event */
	atomic_set(&pdata->fingers_waitq_flag, AFT_WAITQ_WAIT);
	if (down_interruptible(&pdata->fingers_aft_send))
		TS_LOG_INFO(" Failed to down_interruptible()\n");

	if (atomic_read(&pdata->fingers_waitq_flag) == AFT_WAITQ_WAIT) {
		atomic_set(&pdata->fingers_waitq_flag, AFT_WAITQ_IGNORE);
		return -EINVAL;
	}
	if (atomic_read(&pdata->fingers_waitq_flag) == AFT_WAITQ_WAKEUP) {
		if (pdata->chip_data->fold_fingers_supported &&
			daemon_max_fingers_supproted) {
			TS_LOG_DEBUG("%s:copy 20fingers to user\n", __func__);
			ret = copy_to_user(argp, &pdata->fingers_send_aft_info,
				sizeof(pdata->fingers_send_aft_info));
		} else {
			copy_fold_to_normal_fingers(&n_fingers,
				&pdata->fingers_send_aft_info);
			ret = copy_to_user(argp, &n_fingers, sizeof(n_fingers));
		}
		if (ret) {
			TS_LOG_ERR("%s: Failed to copy_from_user()\n",
				__func__);
			return -EFAULT;
		}
	}
	trace_touch(TOUCH_TRACE_ALGO_GET_DATA, TOUCH_TRACE_FUNC_OUT,
		"with aft");
	return ret;
}

static long ts_ioctl_get_finger_pen_info(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("%s: Not support 3.x protocol\n", __func__);
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	trace_touch(TOUCH_TRACE_ALGO_GET_DATA, TOUCH_TRACE_FUNC_IN,
		"with aft");
	/* wait event */
	atomic_set(&pdata->finger_pen_waitq_flag, AFT_WAITQ_WAIT);
	if (down_interruptible(&pdata->finger_pen_aft_send))
		TS_LOG_INFO(" Failed to down_interruptible()\n");

	if (atomic_read(&pdata->finger_pen_waitq_flag) == AFT_WAITQ_WAIT) {
		atomic_set(&pdata->finger_pen_waitq_flag, AFT_WAITQ_IGNORE);
		return -EINVAL;
	}
	if (atomic_read(&pdata->finger_pen_waitq_flag) == AFT_WAITQ_WAKEUP) {
		if (copy_to_user(argp, &pdata->finger_pen_send_aft_info,
			sizeof(pdata->finger_pen_send_aft_info))) {
			TS_LOG_ERR("%s: Failed to copy_from_user()\n",
				__func__);
			return -EFAULT;
		}
	}
	trace_touch(TOUCH_TRACE_ALGO_GET_DATA, TOUCH_TRACE_FUNC_OUT,
		"with aft");
	return ret;
}

static long ts_ioctl_get_stylus3_connect_status(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	int ret;

	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	ret = down_interruptible(&pdata->stylus3_status_flag);
	if (ret) {
		TS_LOG_INFO("get_stylus3_connect_status down Failed\n");
		return -EFAULT;
	}
	if (copy_to_user(argp, &pdata->current_stylus3_status,
		sizeof(pdata->current_stylus3_status))) {
		TS_LOG_ERR("%s: Failed to copy_from_user()\n",
			__func__);
		return -EFAULT;
	}
	return 0;
}

static long ts_ioctl_get_callback_events(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	int ret;
	if ((pdata->feature_info.pen_info.pen_change_protocol !=
		SUPPORT_CHANGE_PEN_PROTOCOL) &&
		((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE))) {
		TS_LOG_ERR("%s: Not support stylus3\n", __func__);
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	ret = down_interruptible(&pdata->stylus3_callback_flag);
	if (ret) {
		TS_LOG_INFO("%s, Failed to down_interruptible()\n", __func__);
	} else {
		if (copy_to_user(argp, &pdata->stylus3_callback_event,
			sizeof(pdata->stylus3_callback_event))) {
			TS_LOG_ERR("%s: Failed to copy_to_user()\n", __func__);
			return -EFAULT;
		}
		TS_LOG_INFO("%s, eventClass=%d, eventCode=%d, extraInfo=%s\n",
			__func__, pdata->stylus3_callback_event.event_class,
			pdata->stylus3_callback_event.event_code,
			pdata->stylus3_callback_event.extra_info);
	}
	atomic_set(&pdata->callback_event_flag, false);
	return 0;
}

static long ts_ioctl_get_stylus3_shift_freq_info(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("%s: Not support stylus3\n", __func__);
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	if (down_interruptible(&pdata->stylus3_shift_freq_flag))
		TS_LOG_INFO(" Failed to down_interruptible()\n");
	if (copy_to_user(argp, &pdata->stylus3_shift_freq_info,
		sizeof(pdata->stylus3_shift_freq_info))) {
		TS_LOG_ERR("%s: Failed to copy_to_user()\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static long ts_ioctl_get_aft_param_info(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;

	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	if (copy_to_user(argp, &g_ts_kit_platform_data.aft_param,
		sizeof(struct ts_aft_algo_param))) {
		TS_LOG_ERR("%s: Failed to copy_to_user()\n", __func__);
		return -EFAULT;
	}
	return 0;
}

static long ts_ioctl_set_coordinates(unsigned long arg)
{
	struct ts_fingers data;
	struct ts_normal_fingers *n_data = NULL;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_fingers *finger = NULL;
	struct input_dev *input_dev = g_ts_kit_platform_data.input_dev;
	int finger_num;
	unsigned int id;
	unsigned short last_fingers_status;
	unsigned short filtered_fingers;
	unsigned short current_fingers_status = 0;
	int ret;

	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_IN,
		"with aft");

	if (!input_dev) {
		TS_LOG_ERR("The command node or input device is not exist!\n");
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	if (pdata->chip_data->fold_fingers_supported &&
		daemon_max_fingers_supproted) {
		TS_LOG_DEBUG("%s:copy 20fingers from user\n", __func__);
		ret = copy_from_user(&data, argp, sizeof(data));
	} else {
		n_data = kzalloc(sizeof(*n_data), GFP_KERNEL);
		if (!n_data) {
			TS_LOG_ERR("%s: no enough memory for finger data\n",
				__func__);
			return -ENOMEM;
		}
		ret = copy_from_user(n_data, argp, sizeof(*n_data));
		copy_normal_to_fold_fingers(&data, n_data);
	}
	if (ret) {
		TS_LOG_ERR("%s: Failed to copy_from_user()\n", __func__);
		ret = -EFAULT;
		goto out;
	}
	finger = &data;
	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_OUT,
		"with aft");

	/*
	 * If aft missed return event back
	 * drop these event which before current time
	 */
	if (atomic_read(&pdata->aft_in_slow_status) &&
			!finger->add_release_flag) {
		TS_LOG_INFO("aft_in_slow_statusted, daemon dont't report\n");
		goto out;
	}

	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_IN, "with aft");
	ts_check_touch_window(finger);
	finger_num = ts_count_fingers(finger);
	TS_LOG_DEBUG("%s:finger_num down = %d\n", __func__, finger_num);
	for (id = 0; id < finger_num; id++) {
		if (finger->fingers[id].status & TS_FINGER_PRESS) {
			current_fingers_status |= (unsigned int)BIT_MASK(id);
			input_report_abs(input_dev, ABS_MT_PRESSURE,
					finger->fingers[id].pressure);
			input_report_abs(input_dev, ABS_MT_POSITION_X,
					finger->fingers[id].x);
			input_report_abs(input_dev, ABS_MT_POSITION_Y,
					finger->fingers[id].y);
			if (pdata->fp_tp_enable) {
				input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
						finger->fingers[id].major);
				input_report_abs(input_dev, ABS_MT_TOUCH_MINOR,
						finger->fingers[id].minor);
			}
			input_report_abs(input_dev, ABS_MT_TRACKING_ID, id);
			input_mt_sync(input_dev);
		}
	}
	/* all fingers up, report the last empty finger */
	if (finger_num == 0)
		input_mt_sync(input_dev);

	input_report_key(input_dev, BTN_TOUCH, finger_num);
	input_sync(input_dev);
	ts_film_touchplus(finger, finger_num, input_dev);
	if (((pdata->chip_data->easy_wakeup_info.sleep_mode ==
		TS_GESTURE_MODE) ||
		(pdata->chip_data->easy_wakeup_info.palm_cover_flag == true)) &&
		(pdata->feature_info.holster_info.holster_switch == 0)) {
		input_report_key(input_dev, finger->gesture_wakeup_value, 1);
		input_sync(input_dev);
		input_report_key(input_dev, finger->gesture_wakeup_value, 0);
		input_sync(input_dev);
	}
	if ((pdata->aft_param.aft_enable_flag) && (finger->add_release_flag)) {
		finger->add_release_flag = 0;
		input_report_key(input_dev, BTN_TOUCH, 0);
		input_mt_sync(input_dev);
		input_sync(input_dev);
	}
	last_fingers_status = atomic_read(&pdata->last_input_fingers_status);
	filtered_fingers = (last_fingers_status ^ current_fingers_status) &
		last_fingers_status;
	atomic_set(&pdata->last_aft_filtered_fingers, filtered_fingers);
	atomic_set(&g_ts_kit_data_report_over, 1);
	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_OUT, "with aft");

out:
	kfree(n_data);
	atomic_set(&pdata->aft_in_slow_status, 0);
	return 0;
}

static void ts_report_pen_event(struct input_dev *input,
	struct ts_tool tool, int pressure, int tool_type, int tool_value)
{
	input_report_abs(input, ABS_X, tool.x);
	input_report_abs(input, ABS_Y, tool.y);
	input_report_abs(input, ABS_PRESSURE, pressure);

	/* check if the pen support tilt event */
	if ((tool.tilt_x != 0) || (tool.tilt_y != 0)) {
		input_report_abs(input, ABS_TILT_X, tool.tilt_x);
		input_report_abs(input, ABS_TILT_Y, tool.tilt_y);
	}

	if ((tool.tool_type != WACOM_RUBBER_TO_PEN) &&
		(tool.tool_type != WACOM_PEN_TO_RUBBER))
		input_report_key(input, BTN_TOUCH, tool.tip_status);

	input_report_key(input, tool_type, tool_value);
	input_sync(input);
}

static long ts_ioctl_set_pens(unsigned long arg)
{
	struct ts_pens data;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_pens *pens = NULL;
	struct input_dev *input = g_ts_kit_platform_data.pen_dev;
	int key_value;
	int id;

	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_IN,
		"with aft");

	if (!input) {
		TS_LOG_ERR("The command node or input device is not exist!\n");
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	if (copy_from_user(&data, argp, sizeof(*pens))) {
		TS_LOG_ERR("%s: Failed to copy_from_user()\n", __func__);
		return -EFAULT;
	}
	pens = &data;
	trace_touch(TOUCH_TRACE_ALGO_SET_EVENT, TOUCH_TRACE_FUNC_OUT,
		"with aft");

	/*
	 * If aft missed return event back
	 * drop these event which before current time
	 */
	if (atomic_read(&pdata->aft_in_slow_status)) {
		TS_LOG_INFO("aft_in_slow_statusted, daemon dont't report\n");
		goto out;
	}

	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_IN, "with aft");

	/* report pen basic single button */
	for (id = 0; id < TS_MAX_PEN_BUTTON; id++) {
		if (pens->buttons[id].status == 0)
			continue;
		else if (pens->buttons[id].status == TS_PEN_BUTTON_PRESS)
			key_value = 1;
		else
			key_value = 0;

		if (pens->buttons[id].key != 0) {
			TS_LOG_ERR("type is %d, button is %d, value is %d\n",
				id, pens->buttons[id].key, key_value);
			input_report_key(input, pens->buttons[id].key,
				key_value);
		}
	}

	/*
	 * report tool change hover -> leave -> in
	 * when hover or leave, the pressure must be 0;
	 * when hover, tool value will report 1, means inrange;
	 * when leave, tool value will report 0, means outrange.
	 */
	if (pens->tool.tool_type == WACOM_RUBBER_TO_PEN) {
		/* rubber hover */
		ts_report_pen_event(input, pens->tool, 0, BTN_TOOL_RUBBER, 1);
		/* rubber leave */
		ts_report_pen_event(input, pens->tool, 0, BTN_TOOL_RUBBER, 0);
		/* pen in */
		pens->tool.tool_type = BTN_TOOL_PEN;
	} else if (pens->tool.tool_type == WACOM_PEN_TO_RUBBER) {
		/* pen hover */
		ts_report_pen_event(input, pens->tool, 0, BTN_TOOL_PEN, 1);
		/* pen leave */
		ts_report_pen_event(input, pens->tool, 0, BTN_TOOL_PEN, 0);
		/* rubber in */
		pens->tool.tool_type = BTN_TOOL_RUBBER;
	}
	/* pen or rubber report point */
	ts_report_pen_event(input, pens->tool, pens->tool.pressure,
		pens->tool.tool_type,
		pens->tool.pen_inrange_status);
	TS_LOG_DEBUG("%s, pen_pressure=%d\n", __func__, pens->tool.pressure);
#if defined(CONFIG_HUAWEI_DSM)
	if (pdata->feature_info.pen_info.supported_pen_alg) {
		if (ts_pen_open_confirm(pens) != 0) {
			input_report_key(input, TS_STYLUS_WAKEUP_SCREEN_ON, 1);
			input_sync(input);
			input_report_key(input, TS_STYLUS_WAKEUP_SCREEN_ON, 0);
			input_sync(input);
			TS_LOG_INFO("%s: report TS_STYLUS_WAKEUP_SCREEN_ON\n",
				__func__);
		}
	}
#endif
	atomic_set(&pdata->last_aft_filtered_pen_pressure, pens->tool.pressure);
	trace_touch(TOUCH_TRACE_PEN_REPORT, TOUCH_TRACE_FUNC_OUT, "report pen");
out:
	atomic_set(&pdata->aft_in_slow_status, 0);
	return 0;
}

static void dump_diff_data_debug(void)
{
	int i = 0;
	unsigned char *diff_data = g_ts_kit_platform_data.chip_data->diff_data;

	if (!g_ts_kit_log_cfg)
		return;

	TS_LOG_INFO("%s: [DIFF_DATA] in\n", __func__);
	for (; i < g_ts_kit_platform_data.chip_data->diff_data_len; i++)
		TS_LOG_INFO("%d", diff_data[i]);
	TS_LOG_INFO("%s: [DIFF_DATA] out\n", __func__);
}


int copy_fingers_to_aft_daemon(struct ts_kit_platform_data *pdata,
	struct ts_fingers *fingers)
{
	unsigned int id;
	unsigned short aft_filtered_fingers;
	unsigned short input_flag = 0;

	if (!pdata->aft_param.aft_enable_flag) {
		TS_LOG_DEBUG("%s:aft not enable\n", __func__);
		return -EINVAL;
	}

	if (atomic_read(&pdata->fingers_waitq_flag) == AFT_WAITQ_WAIT) {
		trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_IN,
			"with aft");
		memcpy(&pdata->fingers_send_aft_info, fingers,
			sizeof(struct ts_fingers));
		dump_fingers_info_debug(fingers->fingers,
			g_ts_kit_platform_data.max_fingers);
		/* clear slow status if daemon not busy */
		atomic_set(&pdata->aft_in_slow_status, 0);
		atomic_set(&pdata->fingers_waitq_flag, AFT_WAITQ_WAKEUP);
		up(&pdata->fingers_aft_send);
		trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_OUT,
			"with aft");
		ts_work_after_input(); /* read roi data for some ic */
		TS_LOG_DEBUG("%s:copy to daemon buffer done\n", __func__);
		if (pdata->chip_data->diff_data_report_supported) {
			atomic_set(&(pdata->diff_data_status),
				DIFF_DATA_WAKEUP);
			up(&(pdata->diff_data_report_flag));
			dump_diff_data_debug();
		}
		return 0;
	} else if (atomic_read(&pdata->fingers_waitq_flag) != AFT_WAITQ_IDLE) {
		/*
		 * when aft in slow status
		 * we set the flag aft_in_slow_status as soon as possible
		 * That will reduce the conflict probability of
		 * daemon report touch and driver report touch
		 */
		if (pdata->use_aft_slow_status)
			atomic_set(&pdata->aft_in_slow_status, 1);
		up(&pdata->fingers_aft_send);
		TS_LOG_DEBUG(
			"[MUTI_AFT] ts_algo_calibrate hal aglo process too slow\n");
		aft_filtered_fingers = atomic_read(
					&pdata->last_aft_filtered_fingers);
		for (id = 0; id < pdata->max_fingers; id++) {
			if ((fingers->fingers[id].status & TS_FINGER_PRESS)) {
				 /* update input event from chip report */
				input_flag |= (1U << id);
				if (aft_filtered_fingers & (1U << id))
					/* release last filtered finger */
					fingers->fingers[id].status =
						TS_FINGER_RELEASE;
			}
		}
		aft_filtered_fingers &= input_flag;
		atomic_set(&pdata->last_input_fingers_status, input_flag);
		atomic_set(&pdata->last_aft_filtered_fingers,
			aft_filtered_fingers);
		if (!pdata->use_aft_slow_status)
			atomic_set(&pdata->aft_in_slow_status, 1);
		TS_LOG_DEBUG("aft_filtered_fingers=%x, input_flag=%x\n",
			aft_filtered_fingers, input_flag);
	} else {
		TS_LOG_DEBUG("[MUTI_AFT] ts_algo_calibrate no wait\n");
	}

	return -EINVAL;
}

int copy_finger_pen_to_aft_daemon(struct ts_kit_platform_data *pdata,
	struct ts_finger_pen_info *finger_pen)
{
	u32 id;
	unsigned short aft_filtered_fingers;
	unsigned short input_flag = 0;
	u32 finger_pen_flag = 0;

	if (pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE) {
		TS_LOG_DEBUG("%s:aft_pen not enable\n", __func__);
		return -EINVAL;
	}
	if (atomic_read(&pdata->finger_pen_waitq_flag) == AFT_WAITQ_WAIT) {
		trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_IN,
			"with aft");
		memcpy(&pdata->finger_pen_send_aft_info, finger_pen,
			sizeof(pdata->finger_pen_send_aft_info));
		finger_pen_flag =
			pdata->finger_pen_send_aft_info.finger_pen_flag;
		if (finger_pen_flag == TS_ONLY_FINGER ||
			(finger_pen_flag == TS_FINGER_PEN))
			dump_fingers_info_debug(
				(struct ts_finger *)&(finger_pen->report_finger_info),
				g_ts_kit_platform_data.max_fingers);
		/* clear slow status if daemon not busy */
		atomic_set(&pdata->aft_in_slow_status, 0);
		atomic_set(&pdata->finger_pen_waitq_flag, AFT_WAITQ_WAKEUP);
		up(&pdata->finger_pen_aft_send);
		trace_touch(TOUCH_TRACE_DATA2ALGO, TOUCH_TRACE_FUNC_OUT,
			"with aft");
		ts_work_after_input(); /* read roi data for some ic */
		TS_LOG_DEBUG("%s:copy to daemon buffer done\n", __func__);
		if (pdata->chip_data->diff_data_report_supported) {
			atomic_set(&(pdata->diff_data_status),
				DIFF_DATA_WAKEUP);
			up(&(pdata->diff_data_report_flag));
			dump_diff_data_debug();
		}
		return 0;
	} else if (atomic_read(&pdata->finger_pen_waitq_flag) !=
		AFT_WAITQ_IDLE) {
		up(&pdata->finger_pen_aft_send);
		TS_LOG_DEBUG(
			"[MUTI_AFT] ts_algo_calibrate hal aglo process too slow\n");
		if (finger_pen_flag == TS_ONLY_FINGER ||
			(finger_pen_flag == TS_FINGER_PEN)) {
			aft_filtered_fingers = atomic_read(
					&pdata->last_aft_filtered_fingers);
			for (id = 0; id < TS_MAX_FINGER; id++) {
				if ((finger_pen->report_finger_info.fingers[id].status & TS_FINGER_PRESS)) {
					/* update input event from chip report */
					input_flag |= (1 << id);
					if (aft_filtered_fingers & (1 << id))
						/* release last filtered finger */
						finger_pen->report_finger_info.fingers[id].status =
							TS_FINGER_RELEASE;
				}
			}
			aft_filtered_fingers &= input_flag;
			atomic_set(&pdata->last_input_fingers_status,
				input_flag);
			atomic_set(&pdata->last_aft_filtered_fingers,
				aft_filtered_fingers);
			TS_LOG_DEBUG("aft_filtered_fingers=%x, input_flag=%x\n",
			aft_filtered_fingers, input_flag);
			if (finger_pen_flag == TS_FINGER_PEN)
				finger_pen->report_pen_info.tool.pressure =
					atomic_read(&pdata->last_aft_filtered_pen_pressure);
		} else {
			finger_pen->report_pen_info.tool.pressure =
				atomic_read(&pdata->last_aft_filtered_pen_pressure);
			TS_LOG_DEBUG("last_aft_filtered_pen_pressure:%d\n",
				finger_pen->report_pen_info.tool.pressure);
		}
		atomic_set(&pdata->aft_in_slow_status, 1);
	} else {
		TS_LOG_DEBUG("[MUTI_AFT] ts_finger_pen_algo_calibrate no wait\n");
	}

	return -EINVAL;
}

void copy_stylus3_shift_freq_to_aft_daemon(
	struct shift_freq_info *shift_freq_info)
{
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("not support stylus3");
		return;
	}
	memcpy(&pdata->stylus3_shift_freq_info, shift_freq_info,
		sizeof(pdata->stylus3_shift_freq_info));
	up(&pdata->stylus3_shift_freq_flag);
}

static int aft_get_info_misc_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int aft_get_info_misc_release(struct inode *inode, struct file *filp)
{
	return 0;
}


static int ts_ioctl_get_diff_data_info(unsigned long arg)
{
	int ret;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;

	if (arg == 0) {
		TS_LOG_ERR("%s:[DIFF_DATA]arg == 0\n", __func__);
		return -EINVAL;
	}
	trace_touch(TOUCH_TRACE_GET_ROI_OR_DIFF_DATA, TOUCH_TRACE_FUNC_IN,
		"read_diff");
	if ((dev->diff_data_report_supported) && (dev->diff_data) &&
			(dev->diff_data_len > 0) &&
			(dev->diff_data_len < DIFF_DATA_MAX_LEN)) {
		TS_LOG_DEBUG("%s:[DIFF_DATA] in\n", __func__);

		if (atomic_read(&(g_ts_kit_platform_data.diff_data_status)) !=
							DIFF_DATA_WAKEUP) {
			TS_LOG_DEBUG("%s:[DIFF_DATA] set status to wait\n",
					__func__);
			atomic_set(&(g_ts_kit_platform_data.diff_data_status),
					DIFF_DATA_WAIT);
		}

		ret = down_interruptible(
			&g_ts_kit_platform_data.diff_data_report_flag);
		if (ret)
			TS_LOG_ERR("%s down_interruptible fail\n", __func__);

		if (atomic_read(&(g_ts_kit_platform_data.diff_data_status)) ==
				DIFF_DATA_WAKEUP) {
			if (copy_to_user(argp, dev->diff_data,
					dev->diff_data_len)) {
				TS_LOG_ERR(
					"%s:[DIFF_DATA]Failed to copy_to_user()\n",
					__func__);
				return -EFAULT;
			}
			TS_LOG_DEBUG("%s:[DIFF_DATA] success\n", __func__);
		} else {
			TS_LOG_INFO("%s:[DIFF_DATA]sensibility data is error\n",
				__func__);
			return -EFAULT;
		}
	}
	trace_touch(TOUCH_TRACE_GET_ROI_OR_DIFF_DATA, TOUCH_TRACE_FUNC_IN,
		"read_diff");
	return 0;
}

static int ts_ioctl_get_stylus3_type(unsigned long arg)
{
	int ret = NO_ERR;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	TS_LOG_INFO("%s: enter\n", __func__);
	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("not support 3.x protocol\n");
		return -EINVAL;
	}
	if (!pdata->chip_data->send_stylus_type_to_fw) {
		TS_LOG_ERR("%s: unsupported send stylus type to fw\n",
			__func__);
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("%s: arg == 0\n", __func__);
		return -EINVAL;
	}
	if (copy_to_user(argp, &pdata->current_stylus3_type_info,
		sizeof(pdata->current_stylus3_type_info))) {
		TS_LOG_ERR("%s: copy stylus type info to user failed\n",
			__func__);
		return -EFAULT;
	}

	return ret;
}

static long aft_get_info_misc_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg)
{
	long ret = -EINVAL;

	switch (cmd) {
	case INPUT_AFT_IOCTL_CMD_GET_TS_FINGERS_INFO:
		daemon_max_fingers_supproted = 0; /* support 10 fingers */
		ret = ts_ioctl_get_fingers_info(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_GET_FOLD_TS_FINGERS_INFO:
		daemon_max_fingers_supproted = 1; /* support 20 fingers */
		ret = ts_ioctl_get_fingers_info(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_GET_ALGO_PARAM_INFO:
		ret = ts_ioctl_get_aft_param_info(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_GET_DIFF_DATA_INFO:
		ret = ts_ioctl_get_diff_data_info(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_GET_TS_FINGER_PEN_INFO:
		ret = ts_ioctl_get_finger_pen_info(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_GET_STYLUS3_CONNECT_STATUS:
		if ((g_ts_kit_platform_data.pen_aft_enable_flag ==
			SUPPORT_PEN_AFT) &&
			(g_ts_kit_platform_data.aft_param.aft_enable_flag ==
			TS_PEN_AFT_ENABLE))
			ret = ts_ioctl_get_stylus3_connect_status(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_GET_CALLBACK_EVENTS:
		ret = ts_ioctl_get_callback_events(arg);
		break;
	case IOCTL_CMD_READ_STYLUS3_SHIFT_FREQ_INFO:
		ret = ts_ioctl_get_stylus3_shift_freq_info(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_GET_BTPEN_TYPE_TO_TPIC:
		ret = ts_ioctl_get_stylus3_type(arg);
		break;
	default:
		TS_LOG_ERR("cmd unknown.%x\n", (cmd));
		ret = -EINVAL;
	}

	return ret;
}

static const struct file_operations g_aft_get_info_misc_fops = {
	.owner = THIS_MODULE,
	.open = aft_get_info_misc_open,
	.release = aft_get_info_misc_release,
	.unlocked_ioctl = aft_get_info_misc_ioctl,
};

static struct miscdevice g_aft_get_info_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_AFT_GET_INFO,
	.fops = &g_aft_get_info_misc_fops,
};

static int aft_set_info_misc_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int aft_set_info_misc_release(struct inode *inode, struct file *filp)
{
	return 0;
}

int ts_send_sensibility_cmd(unsigned int sensibility_cfg)
{
	int error;
	struct ts_cmd_node cmd;

	TS_LOG_DEBUG("%s:in\n", __func__);
	memset(&cmd, 0, sizeof(struct ts_cmd_node));
	cmd.command = TS_SET_SENSIBILITY;
	cmd.cmd_param.pub_params.sensibility_cfg = sensibility_cfg;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(&cmd);
	else
		error = ts_kit_put_one_cmd(&cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}

out:
	TS_LOG_DEBUG("%s: out\n", __func__);
	return error;
}

static int ts_ioctl_set_sensibility_cfg(unsigned long arg)
{
	int ret;
	void __user *argp = (void __user *)(uintptr_t)arg;
	unsigned int sensibility_cfg = 0;

	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}

	if (copy_from_user(&sensibility_cfg, argp, sizeof(sensibility_cfg))) {
		TS_LOG_ERR("%s: Failed to copy_from_user()\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s:sensibility_cfg = %d\n", __func__, sensibility_cfg);
	ret = ts_send_sensibility_cmd(sensibility_cfg);
	if (ret != 0)
		TS_LOG_ERR("ts_send_sensibility_cmd failed\n");
	return 0;
}

static void ts_report_keyevent(unsigned int key_value)
{
	struct input_dev *keyevent_input_dev = NULL;

	if (!g_ts_kit_platform_data.chip_data->support_extra_key_event_input) {
		TS_LOG_ERR("%s no support key event input\n", __func__);
		return;
	}
	if (g_ts_kit_platform_data.keyevent_dev == NULL) {
		TS_LOG_ERR("%s keyevent_dev NULL\n", __func__);
		return;
	}
	keyevent_input_dev = g_ts_kit_platform_data.keyevent_dev;
	switch (key_value) {
	case KEY_FLIP:
	case TS_DOUBLE_CLICK:
		/* 1 report key_event DOWN */
		input_report_key(keyevent_input_dev, key_value, 1);
		input_sync(keyevent_input_dev);
		/* 0 report key_event UP */
		input_report_key(keyevent_input_dev, key_value, 0);
		input_sync(keyevent_input_dev);
		break;
	default:
		TS_LOG_ERR("key_value is invalid %x\n", key_value);
		break;
	}
}
static int ts_ioctl_set_key_value(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	unsigned int key_value;

	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}

	if (copy_from_user(&key_value, argp, sizeof(unsigned int))) {
		TS_LOG_ERR("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s:key_value = %d\n", __func__, key_value);
	ts_report_keyevent(key_value);
	return 0;
}

static int ts_ioctl_set_double_click(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct double_click_data data;
	struct ts_easy_wakeup_info *easy_wakeup_info = NULL;

	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	if (!g_ts_kit_platform_data.chip_data->support_gesture_mode) {
		TS_LOG_ERR("%s: not surport gesture mode\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&data, argp, sizeof(data))) {
		TS_LOG_ERR("%s: Failed to copy_from_user()\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s:key_code = %d\n", __func__, data.key_code);
	easy_wakeup_info = &g_ts_kit_platform_data.chip_data->easy_wakeup_info;
	easy_wakeup_info->easywake_position[0] =
		(data.x_position << EASYWAKE_POSITION_SHIFT) | data.y_position;
	ts_report_keyevent(data.key_code);
	return 0;
}

static int ts_send_tp_ic_cmd(struct tp_ic_command ic_cmd)
{
	int error;
	struct ts_cmd_node cmd;

	TS_LOG_DEBUG("%s:in\n", __func__);
	memset(&cmd, 0, sizeof(cmd));
	cmd.command = TS_SET_TP_IC_CMD;
	cmd.cmd_param.pub_params.ic_cmd.type = ic_cmd.type;
	cmd.cmd_param.pub_params.ic_cmd.length = ic_cmd.length;
	cmd.cmd_param.pub_params.ic_cmd.data = ic_cmd.data;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(&cmd);
	else
		error = ts_kit_put_one_cmd(&cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
	}
	TS_LOG_DEBUG("%s: out\n", __func__);
	return error;
}

static int ts_ioctl_set_tp_ic_command(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct tp_ic_command ic_cmd = {0};
	unsigned char *data = g_ts_kit_platform_data.chip_data->ic_cmd_data;
	int ret;

	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}

	if (copy_from_user(&ic_cmd, argp, sizeof(ic_cmd))) {
		TS_LOG_ERR("%s: Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	if (ic_cmd.length > TP_IC_CMD_MAX_LEN) {
		TS_LOG_ERR("%s:invalid length\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(data, ic_cmd.data, ic_cmd.length)) {
		TS_LOG_ERR("%s: Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	ic_cmd.data = data;
	ret = ts_send_tp_ic_cmd(ic_cmd);
	if (ret < 0)
		TS_LOG_INFO("%s:set status error\n", __func__);
	return 0;
}

static int ts_ioctl_set_stylus3_connect_status(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	unsigned int stylus3_status;

	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}

	if (copy_from_user(&stylus3_status, argp, sizeof(stylus3_status))) {
		TS_LOG_ERR("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s:stylus3_status = %u\n", __func__, stylus3_status);
	atomic_set(&g_ts_kit_platform_data.current_stylus3_status,
		stylus3_status);
	up(&g_ts_kit_platform_data.stylus3_status_flag);
	if (dev->send_bt_status_to_fw) {
		if (ops->chip_stylus3_connect_state)
			ops->chip_stylus3_connect_state(stylus3_status);
	}
	return 0;
}

void ts_update_stylu3_shif_freq_flag(unsigned int status, bool set_flag)
{
	unsigned int last_status;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("not support stylus3");
		return;
	}

	last_status = atomic_read(&(pdata->current_stylus3_status));
	if (set_flag)
		last_status |= status;
	else
		last_status &= ~status;

	TS_LOG_INFO("%s, last_status=%x\n", __func__, last_status);
	atomic_set(&(pdata->current_stylus3_status), last_status);
	up(&(pdata->stylus3_status_flag));
}

static int set_stylus3_change_protocol(unsigned long arg)
{
	unsigned int stylus_status;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	if (arg == 0) {
		TS_LOG_ERR("do not change\n");
		return -EINVAL;
	}
	if (copy_from_user(&stylus_status, argp, sizeof(stylus_status))) {
		TS_LOG_ERR("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	/* bt close,do not need handle */
	if (stylus_status == 0) {
		TS_LOG_ERR("do not change pen protocol\n");
		return NO_ERR;
	}
	/* change pen protocol to 2.2 */
	g_ts_kit_platform_data.stylus3_callback_event.event_class =
	SUPPORT_PEN_PROTOCOL_CLASS;
	g_ts_kit_platform_data.stylus3_callback_event.event_code =
	SUPPORT_PEN_PROTOCOL_CODE;
	TS_LOG_INFO("%s: to pen\n", __func__);
	atomic_set(&pdata->callback_event_flag, true);
	up(&(pdata->stylus3_callback_flag));
	return NO_ERR;
}

static int ts_ioctl_set_callback_events(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("not support stylus3");
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}
	if (atomic_read(&pdata->callback_event_flag) != false) {
		TS_LOG_ERR("%s, ret=%d,callback event not handle, need retry\n",
			__func__, EBUSY);
		return -EBUSY;
	}
	if (copy_from_user(&g_ts_kit_platform_data.stylus3_callback_event,
		argp, sizeof(g_ts_kit_platform_data.stylus3_callback_event))) {
		TS_LOG_ERR("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s, eventClass=%d, eventCode=%d, extraInfo=%s\n", __func__,
		g_ts_kit_platform_data.stylus3_callback_event.event_class,
		g_ts_kit_platform_data.stylus3_callback_event.event_code,
		g_ts_kit_platform_data.stylus3_callback_event.extra_info);
	atomic_set(&pdata->callback_event_flag, true);
	up(&g_ts_kit_platform_data.stylus3_callback_flag);
	TS_LOG_INFO("%s, wake up callback_event\n", __func__);
	return 0;
}

static int ts_ioctl_set_stylus3_ack_freq(unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	int ret;

	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("not support stylus3\n");
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("arg == 0\n");
		return -EINVAL;
	}

	if (copy_from_user(&g_ts_kit_platform_data.ack_freq,
		argp, sizeof(g_ts_kit_platform_data.ack_freq))) {
		TS_LOG_ERR("%s Failed to copy_from_user\n", __func__);
		return -EFAULT;
	}
	TS_LOG_INFO("%s daemon set ack ok\n", __func__);
	if (ops->chip_write_hop_ack) {
		ret = ops->chip_write_hop_ack();
		return ret;
	}
	return 0;
}

static int ts_ioctl_set_stylus3_type(unsigned long arg)
{
	int ret = NO_ERR;
	u32 stylus3_type_info;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	TS_LOG_INFO("%s: enter\n", __func__);
	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("not support 3.x protocol\n");
		return -EINVAL;
	}
	if (!pdata->chip_data->send_stylus_type_to_fw) {
		TS_LOG_ERR("%s: unsupported send stylus type to fw\n",
			__func__);
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("%s: arg == 0\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&stylus3_type_info, argp,
		sizeof(stylus3_type_info))) {
		TS_LOG_ERR("%s: copy stylus type info from user failed\n",
			__func__);
		return -EFAULT;
	}
	pdata->current_stylus3_type_info = stylus3_type_info;
	TS_LOG_INFO("%s: current_stylus3_type_info = 0x%x\n", __func__,
		pdata->current_stylus3_type_info);
	if (ops->chip_stylus3_type_info_handle) {
		if (ops->chip_stylus3_type_info_handle(pdata->current_stylus3_type_info)) {
			TS_LOG_ERR("%s: send stylus type info to ic failed\n",
				__func__);
			return -EFAULT;
		}
	}

	return ret;
}

static int ts_ioctl_set_stylus3_plam_suppression(unsigned long arg)
{
	int ret = NO_ERR;
	unsigned int suppression_type;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	TS_LOG_INFO("%s: enter\n", __func__);
	if ((pdata->pen_aft_enable_flag != SUPPORT_PEN_AFT) ||
		(pdata->aft_param.aft_enable_flag != TS_PEN_AFT_ENABLE)) {
		TS_LOG_ERR("not support 3.x protocol\n");
		return -EINVAL;
	}
	if (!pdata->chip_data->support_stylus3_plam_suppression) {
		TS_LOG_ERR("%s: unsupported stylus3 plam suppression\n",
			__func__);
		return -EINVAL;
	}
	if (arg == 0) {
		TS_LOG_ERR("%s: arg == 0\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(&suppression_type, argp, sizeof(suppression_type))) {
		TS_LOG_ERR("%s: copy stylus3 type info from user failed\n",
			__func__);
		return -EFAULT;
	}
	if (ops->chip_send_supression_cmd_to_fw)
		ops->chip_send_supression_cmd_to_fw(suppression_type);
	TS_LOG_INFO("%s: over\n", __func__);

	return ret;
}

static long aft_set_info_misc_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg)
{
	long ret = -EINVAL;

	switch (cmd) {
	case INPUT_AFT_IOCTL_CMD_SET_COORDINATES:
		daemon_max_fingers_supproted = 0; /* support 10 fingers */
		ret = ts_ioctl_set_coordinates(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_SET_FOLD_COORDINATES:
		daemon_max_fingers_supproted = 1; /* support 20 fingers */
		ret = ts_ioctl_set_coordinates(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_SET_SENSIBILITY_CFG:
		ret = ts_ioctl_set_sensibility_cfg(arg);
		break;
	case INPUT_IOCTL_CMD_SET_FLIP_KEY:
		ret = ts_ioctl_set_key_value(arg);
		break;
	case INPUT_IOCTL_CMD_SET_DOUBLE_CLICK:
		ret = ts_ioctl_set_double_click(arg);
		break;
	case INPUT_IOCTL_CMD_SET_TP_IC_COMMAND:
		ret = ts_ioctl_set_tp_ic_command(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_SET_PENS:
		ret = ts_ioctl_set_pens(arg);
		break;
	case INPUT_AFT_IOCTRL_CMD_SET_STYLUS3_CONNECT_STATUS:
		if (g_ts_kit_platform_data.feature_info.pen_info.pen_change_protocol ==
			SUPPORT_CHANGE_PEN_PROTOCOL) {
			return set_stylus3_change_protocol(arg);
		}
		if ((g_ts_kit_platform_data.pen_aft_enable_flag ==
			SUPPORT_PEN_AFT) &&
			(g_ts_kit_platform_data.aft_param.aft_enable_flag ==
			TS_PEN_AFT_ENABLE))
			ret = ts_ioctl_set_stylus3_connect_status(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_SET_CALLBACK_EVENTS:
		ret = ts_ioctl_set_callback_events(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_SET_STYLUS3_ACK_FREQ:
		ret = ts_ioctl_set_stylus3_ack_freq(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_SET_BTPEN_TYPE_TO_TPIC:
		ret = ts_ioctl_set_stylus3_type(arg);
		break;
	case INPUT_AFT_IOCTL_CMD_SET_STYLUS3_PLAM_SUPPRESSION_STATUS:
		ret = ts_ioctl_set_stylus3_plam_suppression(arg);
		break;
	default:
		TS_LOG_ERR("cmd unknown: %x\n", cmd);
	}
	return ret;
}

static const struct file_operations g_aft_set_info_misc_fops = {
	.owner = THIS_MODULE,
	.open = aft_set_info_misc_open,
	.release = aft_set_info_misc_release,
	.unlocked_ioctl = aft_set_info_misc_ioctl,
};

static struct miscdevice g_aft_set_info_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_AFT_SET_INFO,
	.fops = &g_aft_set_info_misc_fops,
};

int ts_kit_misc_init(struct ts_kit_platform_data *pdata)
{
	int error;
	char set_node_name[MULTI_PANEL_NODE_BUF_LEN] = {0};
	char get_node_name[MULTI_PANEL_NODE_BUF_LEN] = {0};

	if (!pdata->aft_param.aft_enable_flag) {
		TS_LOG_INFO("%s: aft not enable\n", __func__);
		return 0;
	}
	if (pdata->multi_panel_index != SINGLE_TOUCH_PANEL) {
		error = snprintf(get_node_name, MULTI_PANEL_NODE_BUF_LEN,
			"%s%d", DEVICE_AFT_GET_INFO,
			g_ts_kit_platform_data.multi_panel_index);
		if (error < 0) {
			TS_LOG_ERR("%s: snprintf err\n", __func__);
			return -EINVAL;
		}
		g_aft_get_info_misc_device.name = (const char *)get_node_name;
		TS_LOG_INFO("%s: aft get info name = %s\n", __func__,
			g_aft_get_info_misc_device.name);

		error = snprintf(set_node_name, MULTI_PANEL_NODE_BUF_LEN,
			"%s%d", DEVICE_AFT_SET_INFO,
			g_ts_kit_platform_data.multi_panel_index);
		if (error < 0) {
			TS_LOG_ERR("%s: snprintf err\n", __func__);
			return -EINVAL;
		}
		g_aft_set_info_misc_device.name = (const char *)set_node_name;
		TS_LOG_INFO("%s: aft set info name = %s\n", __func__,
			g_aft_set_info_misc_device.name);
	}

	error = misc_register(&g_aft_get_info_misc_device);
	if (error) {
		TS_LOG_ERR("Failed to register aft_get_info misc device\n");
		return -ENODEV;
	}

	error = misc_register(&g_aft_set_info_misc_device);
	if (error) {
		TS_LOG_ERR("Failed to register aft_set_info misc device\n");
		return -ENODEV;
	}

	sema_init(&pdata->fingers_aft_send, 0);
	atomic_set(&pdata->fingers_waitq_flag, AFT_WAITQ_IDLE);
	sema_init(&pdata->diff_data_report_flag, 0);
	atomic_set(&pdata->diff_data_status, DIFF_DATA_IDLE);
	atomic_set(&pdata->aft_in_slow_status, 0);
	atomic_set(&pdata->last_input_fingers_status, 0);
	atomic_set(&pdata->last_aft_filtered_fingers, 0);
	if ((pdata->pen_aft_enable_flag == SUPPORT_PEN_AFT) &&
		(pdata->aft_param.aft_enable_flag == TS_PEN_AFT_ENABLE)) {
		sema_init(&pdata->finger_pen_aft_send, 0);
		atomic_set(&pdata->finger_pen_waitq_flag, AFT_WAITQ_IDLE);
		atomic_set(&pdata->last_aft_filtered_pen_pressure, 0);
		sema_init(&pdata->stylus3_status_flag, STATUS_FLAG_INIT_VALUE);
		atomic_set(&pdata->current_stylus3_status, 0);
		sema_init(&pdata->stylus3_shift_freq_flag,
			STATUS_FLAG_INIT_VALUE);
		sema_init(&pdata->stylus3_callback_flag,
			STATUS_FLAG_INIT_VALUE);
		sema_init(&pdata->stylus3_ack_freq_flag,
			STATUS_FLAG_INIT_VALUE);
		atomic_set(&pdata->callback_event_flag, false);
	}
	if (pdata->feature_info.pen_info.pen_change_protocol ==
		SUPPORT_CHANGE_PEN_PROTOCOL) {
		atomic_set(&pdata->callback_event_flag, false);
		sema_init(&pdata->stylus3_callback_flag,
		STATUS_FLAG_INIT_VALUE);
	}
	return error;
}

void ts_kit_misc_destroy(void)
{
	misc_deregister(&g_aft_get_info_misc_device);
	misc_deregister(&g_aft_set_info_misc_device);
}
