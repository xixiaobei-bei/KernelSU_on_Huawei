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

#include <huawei_ts_kit.h>
#include <huawei_ts_kit_api.h>
#include <huawei_ts_kit_platform.h>
#include <huawei_ts_kit_misc.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/input/mt.h>

#include "trace-events-touch.h"

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
	void __user *argp = (void __user *)arg;
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

static long ts_ioctl_get_aft_param_info(unsigned long arg)
{
	void __user *argp = (void __user *)arg;

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
	struct ts_normal_fingers n_data;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	void __user *argp = (void __user *)arg;
	struct ts_fingers *finger = NULL;
	struct input_dev *input_dev = g_ts_kit_platform_data.input_dev;
	int finger_num;
	int id = 0;
	unsigned short last_fingers_status = 0;
	unsigned short filtered_fingers = 0;
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
		ret = copy_from_user(&n_data, argp, sizeof(n_data));
		copy_normal_to_fold_fingers(&data, &n_data);
	}
	if (ret) {
		TS_LOG_ERR("%s: Failed to copy_from_user()\n", __func__);
		return -EFAULT;
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
	TS_LOG_DEBUG("%s:finger_num = %d\n", __func__, finger_num);

	for (id = 0; id < finger_num; id++) {
		if (finger->fingers[id].status & TS_FINGER_PRESS) {
			current_fingers_status |= BIT_MASK(id);
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
	int id;
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
		dump_fingers_info_debug(fingers->fingers);
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
		up(&pdata->fingers_aft_send);
		TS_LOG_DEBUG("[MUTI_AFT] ts_algo_calibrate hal aglo process too slow\n");
		aft_filtered_fingers = atomic_read(
					&pdata->last_aft_filtered_fingers);
		for (id = 0; id < pdata->max_fingers; id++) {
			if ((fingers->fingers[id].status & TS_FINGER_PRESS)) {
				 /* update input event from chip report */
				input_flag |= (1 << id);
				if (aft_filtered_fingers & (1 << id)) {
					/* release last filtered finger */
					fingers->fingers[id].status =
						TS_FINGER_RELEASE;
				}
			}
		}
		aft_filtered_fingers &= input_flag;
		atomic_set(&pdata->last_input_fingers_status, input_flag);
		atomic_set(&pdata->last_aft_filtered_fingers,
			aft_filtered_fingers);
		atomic_set(&pdata->aft_in_slow_status, 1);
		TS_LOG_DEBUG("aft_filtered_fingers=%x, input_flag=%x\n",
			aft_filtered_fingers, input_flag);
	} else {
		TS_LOG_DEBUG("[MUTI_AFT] ts_algo_calibrate no wait\n");
	}

	return -EINVAL;
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
	int ret = 0;
	void __user *argp = (void __user *)arg;
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
				TS_LOG_ERR("%s:[DIFF_DATA]Failed to copy_to_user()\n",
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

static long aft_get_info_misc_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	long ret;

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
	int error = NO_ERR;
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
	void __user *argp = (void __user *)arg;
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
	void __user *argp = (void __user *)arg;
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
	void __user *argp = (void __user *)arg;
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
	easy_wakeup_info->easywake_position[0] = (data.x_position << 16) |
						data.y_position;
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
	void __user *argp = (void __user *)arg;
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

static long aft_set_info_misc_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	long ret;

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
	default:
		TS_LOG_ERR("cmd unknown: %x\n", cmd);
		ret = -EINVAL;
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

	if (!pdata->aft_param.aft_enable_flag) {
		TS_LOG_INFO("%s: aft not enable\n", __func__);
		return 0;
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
	atomic_set(&g_ts_kit_platform_data.last_input_fingers_status, 0);
	atomic_set(&g_ts_kit_platform_data.last_aft_filtered_fingers, 0);
	return error;
}

void ts_kit_misc_destroy(void)
{
	misc_deregister(&g_aft_get_info_misc_device);
	misc_deregister(&g_aft_set_info_misc_device);
}
