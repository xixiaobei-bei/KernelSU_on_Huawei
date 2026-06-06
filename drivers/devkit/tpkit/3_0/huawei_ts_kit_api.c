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
#include <huawei_ts_kit_platform.h>
#include <huawei_ts_kit_api.h>
#include <huawei_ts_kit_misc.h>
#include "trace-events-touch.h"
#include "huawei_ts_kit_algo.h"
#include <tpkit_platform_adapter.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/sched/rt.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

atomic_t g_ts_kit_data_report_over = ATOMIC_INIT(1);

void ts_proc_bottom_half(struct ts_cmd_node *in_cmd,
		 struct ts_cmd_node *out_cmd)
{
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	TS_LOG_DEBUG("bottom half called\n");

	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_IN, NULL);
	atomic_set(&g_ts_kit_data_report_over, 0);
	/* related event need process, use out cmd to notify */
	if (ops->chip_irq_bottom_half)
		ops->chip_irq_bottom_half(in_cmd, out_cmd);

	trace_touch(TOUCH_TRACE_IRQ_BOTTOM, TOUCH_TRACE_FUNC_OUT, NULL);
}

void ts_work_after_input(void)
{
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;

	trace_touch(TOUCH_TRACE_GET_ROI_OR_DIFF_DATA, TOUCH_TRACE_FUNC_IN,
			"read_diff");
	if (dev->ops->chip_work_after_input)
		dev->ops->chip_work_after_input();

	trace_touch(TOUCH_TRACE_GET_ROI_OR_DIFF_DATA, TOUCH_TRACE_FUNC_OUT,
			"read_diff");
}

void dump_fingers_info_debug(struct ts_finger *fingers)
{
	int i = 0;

	if (!g_ts_kit_log_cfg)
		return;

	for (; i < g_ts_kit_platform_data.max_fingers; i++) {
		if (!fingers[i].status)
			continue;

		TS_LOG_INFO("%s: id=%d status:0x%x\n",
			__func__, i, fingers[i].status);
		TS_LOG_INFO("pressure=%d or=%d maj=%d min=%d\n",
			fingers[i].pressure, fingers[i].orientation,
			fingers[i].major, fingers[i].minor);
		TS_LOG_INFO("wx=%d wy=%d ewx=%d ewy=%d xer=%d yer=%d\n",
			fingers[i].wx, fingers[i].wy,
			fingers[i].ewx, fingers[i].ewy,
			fingers[i].xer, fingers[i].yer);
	}
}

static void ts_finger_events_reformat(struct ts_fingers *finger)
{
	int index;

	dump_fingers_info_debug(finger->fingers);
	for (index = 0; index < g_ts_kit_platform_data.max_fingers; index++) {
		if ((finger->cur_finger_number == 0) ||
			(finger->fingers[index].status == TP_NONE_FINGER))
			finger->fingers[index].status |= TS_FINGER_RELEASE;
		else
			finger->fingers[index].status |= TS_FINGER_PRESS;
	}
}

void ts_algo_calibrate(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int id;
	int algo_size = g_ts_kit_platform_data.chip_data->algo_size;
	u32 algo_order =
		in_cmd->cmd_param.pub_params.algo_param.algo_order;
	struct ts_fingers *in_finger =
		&in_cmd->cmd_param.pub_params.algo_param.info;
	struct ts_fingers *out_finger =
		&out_cmd->cmd_param.pub_params.algo_param.info;
	struct ts_algo_func *algo = NULL;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;

	if (!algo_size) {
		TS_LOG_INFO("no algo handler, direct report\n");
		goto out;
	}
	TS_LOG_DEBUG("%s: inalgo_order = %d\n", __func__, algo_order);
	/*
	 * If 1.aft enabled, hal algo works or 2.dont need algo t1 to filter
	 * glove events which algo_order configed 0, should skip driver algo
	 * and reformat events to add PRESS/RELEASE to finger->status
	 */

	if (g_ts_kit_platform_data.aft_param.aft_enable_flag || (!algo_order)) {
		ts_finger_events_reformat(in_finger);
		goto out;
	}

	TS_LOG_DEBUG("algo algo_order: %d, algo_size :%d\n",
		algo_order, algo_size);

	for (id = 0; id < algo_size; id++) {
		if (algo_order & BIT_MASK(id)) {
			TS_LOG_DEBUG("algo id:%d is setted\n", id);
			/* found the right algo func */
			list_for_each_entry(algo, &dev->algo_head, node) {
				if (algo->algo_index == id) {
					TS_LOG_DEBUG("algo :%s called\n",
						algo->algo_name);
					algo->chip_algo_func(dev,
						in_finger, out_finger);
					memcpy(in_finger, out_finger,
						sizeof(struct ts_fingers));
					break;
				}
			}
		}
	}
out:
	memcpy(out_finger, in_finger, sizeof(struct ts_fingers));
	out_cmd->command = TS_REPORT_INPUT;
	TS_LOG_DEBUG("%s:skip aft\n", __func__);

	/*
	 * copy finger info to aft daemon
	 * if error hanppened, stil goto driver touch report process
	 * else do not report direct and set out_cmd as INVAILD
	 */
	if (!copy_fingers_to_aft_daemon(&g_ts_kit_platform_data, out_finger))
		out_cmd->command = TS_INVAILD_CMD;
}

void ts_check_touch_window(struct ts_fingers *finger)
{
	int id;
	int flag = 0;
	struct ts_window_info *window = NULL;

	if (!finger) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return;
	}
	window = &g_ts_kit_platform_data.feature_info.window_info;
	if (window->window_enable == 0) {
		TS_LOG_DEBUG("no need to part report\n");
		return;
	}
	if (finger->fingers[0].status == TS_FINGER_RELEASE) {
		TS_LOG_DEBUG("no need to part report\n");
		return;
	}

	for (id = 0; id < g_ts_kit_platform_data.max_fingers; id++) {
		if ((finger->fingers[id].status != 0) &&
			(finger->fingers[id].x >= window->top_left_x0) &&
			(finger->fingers[id].x <= window->bottom_right_x1) &&
			(finger->fingers[id].y >= window->top_left_y0) &&
			(finger->fingers[id].y <= window->bottom_right_y1))
			flag = 1;
	}
	if (!flag)
		finger->fingers[0].status = TS_FINGER_RELEASE;
}

void ts_film_touchplus(struct ts_fingers *finger, int finger_num,
		struct input_dev *input_dev)
{
	static int pre_special_button_key = TS_TOUCHPLUS_INVALID;
	int key_max = TS_TOUCHPLUS_KEY2;
	int key_min = TS_TOUCHPLUS_KEY3;
	unsigned char ts_state;

	TS_LOG_DEBUG("ts_film_touchplus called\n");

	/* discard touchplus report in gesture wakeup mode */
	ts_state = atomic_read(&g_ts_kit_platform_data.state);
	if ((ts_state == TS_SLEEP) || (ts_state == TS_WORK_IN_SLEEP))
		return;

	/*
	 * touchplus(LingXiYiZhi)
	 * report, The difference between ABS_report and touchpls key_report
	 * when ABS_report is running, touchpls key will not report
	 * when touchpls key is not in range of touchpls keys
	 * will not report key
	 */
	if ((finger_num != 0) || (finger->special_button_key > key_max) ||
			(finger->special_button_key < key_min)) {
		if (finger->special_button_flag != 0) {
			input_report_key(input_dev,
				finger->special_button_key, 0);
			input_sync(input_dev);
		}
		return;
	}

	/*
	 * touchplus(LingXiYiZhi)
	 * report, store touchpls key data(finger->special_button_key)
	 * when special_button_flag report touchpls key DOWN
	 * store current touchpls key
	 * till the key report UP, then other keys will not report
	 */
	if (finger->special_button_flag == 1) {
		input_report_key(input_dev, finger->special_button_key,
				finger->special_button_flag);
		input_sync(input_dev);
	} else if ((finger->special_button_flag == 0) &&
		(finger->special_button_key == pre_special_button_key)) {
		input_report_key(input_dev, finger->special_button_key,
				finger->special_button_flag);
		input_sync(input_dev);
	} else if ((finger->special_button_flag == 0) &&
		(finger->special_button_key != pre_special_button_key)) {
		input_report_key(input_dev, pre_special_button_key, 0);
		input_sync(input_dev);
	}
	pre_special_button_key = finger->special_button_key;
}

static void ts_report_pen_event(struct input_dev *input,
		struct ts_tool tool, int pressure,
		int tool_type, int tool_value)
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

void ts_report_pen(struct ts_cmd_node *in_cmd)
{
	struct ts_pens *pens = NULL;
	struct input_dev *input = g_ts_kit_platform_data.pen_dev;
	int i;
	int key_value = 0;
	int ret = 0;
	struct ts_kit_platform_data *p_data =
		g_ts_kit_platform_data.chip_data->ts_platform_data;

	if (!in_cmd) {
		TS_LOG_ERR("%s parameters is null!\n", __func__);
		return;
	}
	trace_touch(TOUCH_TRACE_PEN_REPORT, TOUCH_TRACE_FUNC_IN, "report pen");
	pens = &in_cmd->cmd_param.pub_params.report_pen_info;
	if (!pens) {
		TS_LOG_ERR("%s pens is null!\n", __func__);
		return;
	}
	/* report pen basic single button */
	for (i = 0; i < TS_MAX_PEN_BUTTON; i++) {
		if (pens->buttons[i].status == 0)
			continue;
		else if (pens->buttons[i].status == TS_PEN_BUTTON_PRESS)
			key_value = 1;
		else
			key_value = 0;

		if (pens->buttons[i].key != 0) {
			TS_LOG_INFO("%s: index is %d\n", __func__, i);
			input_report_key(input, pens->buttons[i].key,
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
#if defined(CONFIG_HUAWEI_DSM)
	if (p_data->feature_info.pen_info.supported_pen_alg) {
		ret = ts_pen_open_confirm(pens);
		if (ret != 0) {
			ts_dmd_report(DSM_TP_IN_VNOISE_ERROR_NO,
				"DSM_TP_IN_VNOISE_ERROR_NO:report %d\n",
				TS_STYLUS_WAKEUP_SCREEN_ON);
			input_report_key(input, TS_STYLUS_WAKEUP_SCREEN_ON, 1);
			input_sync(input);
			input_report_key(input, TS_STYLUS_WAKEUP_SCREEN_ON, 0);
			input_sync(input);
			TS_LOG_INFO("%s: report TS_STYLUS_WAKEUP_SCREEN_ON\n",
				__func__);
		}
	}
#endif
	trace_touch(TOUCH_TRACE_PEN_REPORT, TOUCH_TRACE_FUNC_OUT, "report pen");
}

int ts_count_fingers(struct ts_fingers *fingers)
{
	int count = 0;
	int id = 0;

	TS_LOG_DEBUG("%s:dump finger:\n", __func__);
	dump_fingers_info_debug(fingers->fingers);
	for (; id < g_ts_kit_platform_data.max_fingers; id++) {
		if (fingers->fingers[id].status & TS_FINGER_PRESS)
			count = id + 1;
	}

	return count;
}

void ts_palm_report(struct ts_cmd_node *in_cmd)
{
	unsigned int button;
	struct input_dev *input_dev = g_ts_kit_platform_data.input_dev;

	if ((!input_dev) || (!in_cmd)) {
		TS_LOG_ERR("The command node or input device is not exist!\n");
		return;
	}
	button = in_cmd->cmd_param.pub_params.ts_key;
	TS_LOG_INFO("%s is called, report %u\n", __func__, button);
	input_report_key(input_dev, button, 1); /* 1 report key_event DOWN */
	input_sync(input_dev);
	input_report_key(input_dev, button, 0); /* 0 report key_event UP */
	input_sync(input_dev);
	atomic_set(&g_ts_kit_data_report_over, 1);
}

void ts_report_key_event(struct ts_cmd_node *in_cmd)
{
	struct ts_key_info *key_info = NULL;
	struct input_dev *input_dev = g_ts_kit_platform_data.input_dev;

	if ((!input_dev) || (!in_cmd)) {
		TS_LOG_ERR("The command node or input device is not exist!\n");
		return;
	}
	key_info = &in_cmd->cmd_param.pub_params.key_info;

	TS_LOG_INFO("ts report key %d, action: %x\n",
		key_info->key_code, key_info->action);

	if (key_info->action & TS_KEY_ACTION_PRESS) {
		input_report_key(input_dev, key_info->key_code, TS_KEY_PRESSED);
		input_sync(input_dev);
	}

	if (key_info->action & (TS_KEY_ACTION_RELEASE)) {
		input_report_key(input_dev, key_info->key_code, TS_KEY_RELEASE);
		input_sync(input_dev);
	}

	atomic_set(&g_ts_kit_data_report_over, 1);
}

void ts_report_input(struct ts_cmd_node *in_cmd)
{
	struct ts_fingers *fingers = NULL;
	struct input_dev *input_dev = g_ts_kit_platform_data.input_dev;
	int finger_count;
	int id = 0;
	struct ts_easy_wakeup_info *easy_wakeup_info = NULL;
	struct ts_feature_info *feature_info = NULL;

	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_IN, "without aft");
	if ((!input_dev) || (!in_cmd)) {
		TS_LOG_ERR("The command node or input device is not exist!\n");
		return;
	}

	easy_wakeup_info = &g_ts_kit_platform_data.chip_data->easy_wakeup_info;
	feature_info = &g_ts_kit_platform_data.feature_info;

	fingers = &in_cmd->cmd_param.pub_params.algo_param.info;
	finger_count = ts_count_fingers(fingers);
	TS_LOG_DEBUG("ts_report_input\n");
	ts_check_touch_window(fingers);

	for (id = 0; id < finger_count; id++) {
		if (fingers->fingers[id].status & TS_FINGER_PRESS) {
			TS_LOG_DEBUG("down: id is %d, pressure = %d\n",
					id, fingers->fingers[id].pressure);
			input_report_abs(input_dev, ABS_MT_PRESSURE,
					fingers->fingers[id].pressure);
			input_report_abs(input_dev, ABS_MT_POSITION_X,
					fingers->fingers[id].x);
			input_report_abs(input_dev, ABS_MT_POSITION_Y,
					fingers->fingers[id].y);
			input_report_abs(input_dev, ABS_MT_TRACKING_ID, id);
			if (g_ts_kit_platform_data.fp_tp_enable) {
				input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR,
						fingers->fingers[id].major);
				input_report_abs(input_dev, ABS_MT_TOUCH_MINOR,
						fingers->fingers[id].minor);
			}
			input_mt_sync(input_dev);
		} else if (fingers->fingers[id].status & TS_FINGER_RELEASE) {
			TS_LOG_DEBUG("up: id is %d, status = %d\n",
				id, fingers->fingers[id].status);
			input_mt_sync(input_dev);
		}
	}

	input_report_key(input_dev, BTN_TOUCH, finger_count);
	input_sync(input_dev);

	ts_film_touchplus(fingers, finger_count, input_dev);
	if (((easy_wakeup_info->sleep_mode == TS_GESTURE_MODE) ||
		(easy_wakeup_info->palm_cover_flag == true)) &&
		(feature_info->holster_info.holster_switch == 0)) {
		input_report_key(input_dev,
			fingers->gesture_wakeup_value, 1);
		input_sync(input_dev);
		input_report_key(input_dev,
			fingers->gesture_wakeup_value, 0);
		input_sync(input_dev);
	}
	TS_LOG_DEBUG("ts_report_input done, finger_count = %d\n",
			finger_count);

	ts_work_after_input(); /* do some delayed works */

	atomic_set(&g_ts_kit_data_report_over, 1);
	trace_touch(TOUCH_TRACE_INPUT, TOUCH_TRACE_FUNC_OUT,
			"without aft");
}

static void send_up_msg_in_resume(void)
{
	struct input_dev *input_dev = g_ts_kit_platform_data.input_dev;

	input_report_key(input_dev, BTN_TOUCH, 0);
	input_mt_sync(input_dev);
	input_sync(input_dev);
	TS_LOG_DEBUG("send_up_msg_in_resume\n");
}

int ts_power_off_control_mode(int irq_id, struct ts_cmd_node *in_cmd)
{
	int error = NO_ERR;
	enum lcd_kit_ts_pm_type pm_type =
		in_cmd->cmd_param.pub_params.pm_type;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_kit_device_data *dev = pdata->chip_data;

	switch (pm_type) {
	case TS_BEFORE_SUSPEND: /* do something before suspend */
		TS_LOG_DEBUG("[performance]%s:before suspend in\n", __func__);
		atomic_set(&pdata->power_state, TS_GOTO_SLEEP);
		if (!((dev->is_parade_solution == 1) &&
			(dev->sleep_in_mode == 0)))
			ts_stop_wd_timer(pdata);
		if (dev->ops->chip_before_suspend)
			error = dev->ops->chip_before_suspend();
		TS_LOG_DEBUG("[performance]%s:before suspend done\n",
			__func__);
		break;
	case TS_SUSPEND_DEVICE: /* device power off or sleep */
		TS_LOG_DEBUG("[performance]%s:suspend in\n", __func__);
		atomic_set(&pdata->state, TS_SLEEP);
		if (pdata->aft_param.aft_enable_flag) {
			TS_LOG_INFO("ts_kit aft suspend\n");
			atomic_set(&pdata->aft_in_slow_status, 0);
			atomic_set(&pdata->last_input_fingers_status, 0);
			atomic_set(&pdata->last_aft_filtered_fingers, 0);
			kobject_uevent(&pdata->input_dev->dev.kobj,
				KOBJ_OFFLINE);
		}
		if (dev->ops->chip_suspend)
			error = dev->ops->chip_suspend();
		atomic_set(&pdata->power_state,
			TS_SLEEP);
		TS_LOG_DEBUG("[performance]%s:suspend done\n", __func__);
		break;
	case TS_IC_SHUT_DOWN:
		TS_LOG_DEBUG("[performance]%s:shutdown in\n", __func__);
		atomic_set(&pdata->power_state,
			TS_UNINIT);
		if (!(dev->is_parade_solution == true))
			disable_irq(irq_id);
		if (dev->ops->chip_shutdown)
			dev->ops->chip_shutdown();
		TS_LOG_DEBUG("[performance]%s:shutdown done\n", __func__);
		break;
	case TS_RESUME_DEVICE: /* device power on or wakeup */
		TS_LOG_DEBUG("[performance]%s:resume in\n", __func__);
		atomic_set(&pdata->power_state,
			TS_GOTO_WORK);
		if (dev->ops->chip_resume) {
			error = dev->ops->chip_resume();
			if (error) {
#if defined(CONFIG_HUAWEI_DSM)
				ts_dmd_report(DSM_TP_WAKEUP_ERROR_NO,
					"try to client record 926004020 for resume error\n");
#endif
			}
		}
		TS_LOG_DEBUG("[performance]%s:resume done\n", __func__);
		break;
	case TS_AFTER_RESUME: /* do something after resume */
		TS_LOG_DEBUG("[performance]%s:after resume in\n", __func__);
		if (dev->ops->chip_after_resume) {
			error = dev->ops->chip_after_resume((void *)&pdata->feature_info);
			if (error) {
#if defined(CONFIG_HUAWEI_DSM)
				ts_dmd_report(DSM_TP_WAKEUP_ERROR_NO,
					"try to client record 926004020 for after resume error\n");
#endif
			}
		}
		TS_LOG_INFO("%s: chip_name:%s,module_name:%s,fw version: %s\n",
			__func__, dev->chip_name,
			dev->module_name, dev->version_name);
		send_up_msg_in_resume();
		if (pdata->aft_param.aft_enable_flag) {
			TS_LOG_INFO("ts_kit aft resume\n");
			kobject_uevent(&pdata->input_dev->dev.kobj,
					KOBJ_ONLINE);
		}
		atomic_set(&pdata->state, TS_WORK);
		if (!((dev->is_parade_solution == 1) &&
				(dev->sleep_in_mode == 0))) {
			enable_irq(irq_id);
			ts_start_wd_timer(pdata);
		}
		atomic_set(&pdata->power_state, TS_WORK);
		TS_LOG_DEBUG("[performance]%s: after resume done\n", __func__);
		break;
	default:
		TS_LOG_ERR("pm_type = %d\n", pm_type);
		error = -EINVAL;
		break;
	}
	return error;
}

int ts_power_off_no_config(int irq_id,
		struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	enum lcd_kit_ts_pm_type pm_type =
		in_cmd->cmd_param.pub_params.pm_type;
	struct ts_feature_info *feature_info =
		&g_ts_kit_platform_data.feature_info;

	switch (pm_type) {
	case TS_BEFORE_SUSPEND:
		if (ops->chip_before_suspend) {
			TS_LOG_ERR("TS_BEFORE_SUSPEND\n");
			error = ops->chip_before_suspend();
		}
		break;
	case TS_SUSPEND_DEVICE:
		if (ops->chip_suspend) {
			TS_LOG_ERR("TS_SUSPEND_DEVICE\n");
			error = ops->chip_suspend();
		}
		break;
	case TS_RESUME_DEVICE:
		if (ops->chip_resume) {
			TS_LOG_ERR("TS_RESUME_DEVICE\n");
			error = ops->chip_resume();
		}
		break;
	case TS_AFTER_RESUME:
		if (ops->chip_after_resume) {
			TS_LOG_ERR("TS_AFTER_RESUME\n");
			error = ops->chip_after_resume((void *)feature_info);
		}
		break;
	default:
		TS_LOG_ERR("pm_type = %d\n", pm_type);
		error = -EINVAL;
		break;
	}
	return error;
}

int ts_power_off_easy_wakeup_mode(int irq_id,
		struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	enum lcd_kit_ts_pm_type pm_type = in_cmd->cmd_param.pub_params.pm_type;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;

	switch (pm_type) {
	case TS_BEFORE_SUSPEND: /* do something before suspend */
		if (!(dev->is_parade_solution == true)) {
			ts_stop_wd_timer(&g_ts_kit_platform_data);
			disable_irq(irq_id);
		}
		if (dev->ops->chip_before_suspend)
			error = dev->ops->chip_before_suspend();
		break;
	/* switch to easy-wakeup mode, and enable interrupts */
	case TS_SUSPEND_DEVICE:
		atomic_set(&g_ts_kit_platform_data.state, TS_WORK_IN_SLEEP);
		if (g_ts_kit_platform_data.aft_param.aft_enable_flag &&
			dev->support_notice_aft_gesture_mode) {
			TS_LOG_INFO("%s ts_kit aft suspend\n", __func__);
			atomic_set(&g_ts_kit_platform_data.aft_in_slow_status,
				0);
			atomic_set(&g_ts_kit_platform_data.
				last_input_fingers_status, 0);
			atomic_set(&g_ts_kit_platform_data.
				last_aft_filtered_fingers, 0);
			kobject_uevent(&g_ts_kit_platform_data.
				input_dev->dev.kobj, KOBJ_OFFLINE);
		}
		if (dev->ops->chip_suspend)
			error = dev->ops->chip_suspend();
		if (!(dev->is_parade_solution == true))
			enable_irq(irq_id);
		if (!dev->send_stylus_gesture_switch) {
			out_cmd->command = TS_WAKEUP_GESTURE_ENABLE;
			out_cmd->cmd_param.prv_params =
				(void *)&g_ts_kit_platform_data.feature_info.wakeup_gesture_enable_info;
		}
		break;
	case TS_IC_SHUT_DOWN:
		if (!(dev->is_parade_solution == true))
			disable_irq(irq_id);
		if (dev->ops->chip_shutdown)
			dev->ops->chip_shutdown();
		break;
	case TS_RESUME_DEVICE: /* exit easy-wakeup mode and restore sth */
		if (!(dev->is_parade_solution == true))
			disable_irq(irq_id);
		if (dev->ops->chip_resume)
			error = dev->ops->chip_resume();
		break;
	case TS_AFTER_RESUME: /* do nothing */
		if (dev->ops->chip_after_resume)
			error = dev->ops->chip_after_resume((void *)&g_ts_kit_platform_data.feature_info);
		TS_LOG_INFO("%s: chip_name:%s,module_name:%s,fw version: %s\n",
				__func__, dev->chip_name,
				dev->module_name, dev->version_name);
		send_up_msg_in_resume();
		if (g_ts_kit_platform_data.aft_param.aft_enable_flag &&
			dev->support_notice_aft_gesture_mode) {
			TS_LOG_INFO("%s ts_kit aft resume\n", __func__);
			kobject_uevent(&g_ts_kit_platform_data.input_dev->
				dev.kobj, KOBJ_ONLINE);
		}
		atomic_set(&g_ts_kit_platform_data.state, TS_WORK);
		if (!(dev->is_parade_solution == true)) {
			enable_irq(irq_id);
			ts_start_wd_timer(&g_ts_kit_platform_data);
		}
		break;
	default:
		TS_LOG_ERR("pm_type = %d\n", pm_type);
		error = -EINVAL;
		break;
	}
	return error;
}

int ts_power_control(int irq_id, struct ts_cmd_node *in_cmd,
		struct ts_cmd_node *out_cmd)
{
	int error;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;
	atomic_t *face_dct_en = &g_ts_kit_platform_data.face_dct_en;

	if (dev->easy_wakeup_info.sleep_mode == TS_GESTURE_MODE) {
		error = ts_power_off_easy_wakeup_mode(irq_id, in_cmd, out_cmd);
		return error;
	} else if (dev->easy_wakeup_info.sleep_mode != TS_POWER_OFF_MODE) {
		TS_LOG_ERR("no such mode!\n");
		error = -EINVAL;
		return error;
	}
	if (g_ts_kit_platform_data.udfp_enable_flag && dev->suspend_no_config) {
		error = ts_power_off_no_config(irq_id, in_cmd, out_cmd);
		return error;
	}
	if (dev->face_dct_support &&
			(atomic_read(face_dct_en) == FACE_DCT_ENABLE)) {
		enable_irq(g_ts_kit_platform_data.irq_id);
		error = ts_power_off_no_config(irq_id, in_cmd, out_cmd);
	} else {
		error = ts_power_off_control_mode(irq_id, in_cmd);
	}
	return error;
}

int ts_touch_window(struct ts_cmd_node *in_cmd)
{
	struct ts_window_info *info = NULL;
	struct ts_window_info *p_window_info = NULL;

	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_window_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	p_window_info = &g_ts_kit_platform_data.feature_info.window_info;
	p_window_info->window_enable = info->window_enable;
	p_window_info->top_left_x0 = info->top_left_x0;
	p_window_info->top_left_y0 = info->top_left_y0;
	p_window_info->bottom_right_x1 = info->bottom_right_x1;
	p_window_info->bottom_right_y1 = info->bottom_right_y1;

	info->status = TS_ACTION_SUCCESS;

	return NO_ERR;
}

int ts_fw_update_boot(struct ts_cmd_node *in_cmd)
{
	char *fw_name = NULL;
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_kit_platform_data *pdata =
		g_ts_kit_platform_data.chip_data->ts_platform_data;
	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	fw_name = in_cmd->cmd_param.pub_params.firmware_info.fw_name;
	if (ops->chip_fw_update_boot) {
		error = ops->chip_fw_update_boot(fw_name);
		if (error) {
#if defined(CONFIG_HUAWEI_DSM)
			ts_dmd_report(DSM_TP_FWUPDATE_ERROR_NO,
				"fw update result: failed status is: %d\n",
				pdata->dsm_info.constraints_UPDATE_status);
#endif
		}
	}

	TS_LOG_INFO("process firmware update boot, return value:%d\n", error);
	return error;
}

int ts_fw_update_sd(struct ts_cmd_node *in_cmd)
{
	int error = NO_ERR;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;

	TS_LOG_INFO("process firmware update sd\n");
	if (!dev->is_parade_solution)
		ts_stop_wd_timer(&g_ts_kit_platform_data);

	if (dev->ops->chip_fw_update_sd)
		error = dev->ops->chip_fw_update_sd();

	if (!dev->is_parade_solution)
		ts_start_wd_timer(&g_ts_kit_platform_data);

	return error;
}

void ts_start_wd_timer(struct ts_kit_platform_data *cd)
{
	int timeout;

	if ((!cd) || (!cd->chip_data)) {
		TS_LOG_ERR("%s :the pointer is null\n", __func__);
		return;
	}

	if (!cd->chip_data->need_wd_check_status) {
		TS_LOG_DEBUG("%s :no need to check the status by watch dog\n",
			__func__);
		return;
	}

	if (!TS_WATCHDOG_TIMEOUT)
		return;

	TS_LOG_DEBUG("start wd\n");
	if (cd->chip_data->check_status_watchdog_timeout) {
		timeout = cd->chip_data->check_status_watchdog_timeout;
		mod_timer(&cd->watchdog_timer,
			jiffies + msecs_to_jiffies(timeout));
	} else {
		mod_timer(&cd->watchdog_timer,
			jiffies + msecs_to_jiffies(TS_WATCHDOG_TIMEOUT));
	}
}
EXPORT_SYMBOL(ts_start_wd_timer);

void ts_stop_wd_timer(struct ts_kit_platform_data *cd)
{

	if ((!cd) || (!cd->chip_data)) {
		TS_LOG_ERR("%s :the pointer is null\n", __func__);
		return;
	}

	if (!cd->chip_data->need_wd_check_status) {
		TS_LOG_DEBUG("%s :no need to check the status by watch dog\n",
			__func__);
		return;
	}

	if (!TS_WATCHDOG_TIMEOUT)
		return;

	TS_LOG_DEBUG("stop wd\n");
	del_timer(&cd->watchdog_timer);
	cancel_work_sync(&cd->watchdog_work);
	del_timer(&cd->watchdog_timer);
}
EXPORT_SYMBOL(ts_stop_wd_timer);

static bool need_process_in_sleep(struct ts_cmd_node *cmd)
{
	bool is_need_process = true;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;
	enum lcd_kit_ts_pm_type pm_type =
		cmd->cmd_param.pub_params.pm_type;

	switch (cmd->command) {
	case TS_POWER_CONTROL:
		if ((pm_type != TS_RESUME_DEVICE) &&
			(pm_type != TS_AFTER_RESUME))
			is_need_process = false;
		break;
	case TS_TOUCH_WINDOW:
		is_need_process = true;
		break;
	case TS_INT_PROCESS:
	case TS_INT_ERR_OCCUR:
		if (dev->is_parade_solution) {
			if ((dev->isbootupdate_finish == false) ||
					(dev->sleep_in_mode == 0))
				is_need_process = true;
			else
				is_need_process = false;
		} else {
			is_need_process = false;
		}
		break;
	case TS_GET_CHIP_INFO:
		is_need_process = true;
		break;
	default:
		is_need_process = false;
		break;
	}

	return is_need_process;
}

static bool need_process_work_in_sleep(struct ts_cmd_node *cmd)
{
	bool is_need_process = true;
	enum lcd_kit_ts_pm_type pm_type =
		cmd->cmd_param.pub_params.pm_type;

	switch (cmd->command) {
	case TS_POWER_CONTROL:
		if ((pm_type != TS_RESUME_DEVICE) &&
			(pm_type != TS_AFTER_RESUME))
			is_need_process = false;
		break;
	case TS_TOUCH_WINDOW:
		is_need_process = true;
		break;
	case TS_OEM_INFO_SWITCH:
		is_need_process = true;
		break;
	case TS_GET_CHIP_INFO:
		is_need_process = true;
		break;
	case TS_TOUCH_SWITCH:
		is_need_process = false;
		break;
	case TS_READ_RAW_DATA:
		is_need_process = false;
		break;
	default:
		is_need_process = true;
		break;
	}

	return is_need_process;
}

bool ts_cmd_need_process(struct ts_cmd_node *cmd)
{
	bool is_need_process = true;
	struct ts_cmd_sync *sync = cmd->sync;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	if (unlikely((atomic_read(&pdata->state) == TS_SLEEP) ||
		(atomic_read(&pdata->state) == TS_WORK_IN_SLEEP))) {
		if (atomic_read(&pdata->state) == TS_SLEEP)
			is_need_process = need_process_in_sleep(cmd);
		else
			is_need_process = need_process_work_in_sleep(cmd);
	}

	if ((!is_need_process) && sync) {
		if (atomic_read(&sync->timeout_flag) == TS_TIMEOUT)
			kfree(sync);
		else
			complete(&sync->done);
	}

	return is_need_process;
}

int ts_kit_power_notify_callback(struct notifier_block *self,
		unsigned long notify_pm_type, void *data)
{
	int *data_in = data;
	unsigned int timeout;
	int ret;

	if (!data) {
		TS_LOG_ERR("%s: data in null ptr\n", __func__);
		return 0;
	}

	timeout = *data_in;
	TS_LOG_INFO("%s called,pm_type=%lu, timeout=%d\n",
		__func__, notify_pm_type, timeout);
	ret = ts_kit_power_control_notify(notify_pm_type, timeout);
	if (ret)
		TS_LOG_ERR("%s:ts_kit_power_control_notify failed\n",
			__func__);
	return ret;
}

int ts_kit_power_control_notify(enum lcd_kit_ts_pm_type pm_type,
		int timeout)
{
	int error;
	struct ts_cmd_node cmd;
	struct ts_kit_device_data *dev = NULL;
	struct ts_feature_info *feature_info = NULL;

	TS_LOG_INFO("%s called, pm_type is %d", __func__, pm_type);
	if (atomic_read(&g_ts_kit_platform_data.state) == TS_UNINIT) {
		TS_LOG_INFO("ts is not init");
		return -EINVAL;
	}

	dev = g_ts_kit_platform_data.chip_data;
	feature_info = &dev->ts_platform_data->feature_info;

#if defined(CONFIG_TEE_TUI)
	if (dev->report_tui_enable && (pm_type == TS_BEFORE_SUSPEND)) {
		dev->tui_set_flag |= 0x1;
		TS_LOG_INFO("TUI is working, later do before suspend\n");
		return NO_ERR;
	}

	if (dev->report_tui_enable && (pm_type == TS_SUSPEND_DEVICE)) {
		dev->tui_set_flag |= 0x2;
		TS_LOG_INFO("TUI is working, later do suspend\n");
		return NO_ERR;
	}
#endif

	if ((dev->easy_wakeup_info.sleep_mode == TS_POWER_OFF_MODE) &&
			(pm_type == TS_BEFORE_SUSPEND)) {
		if (!((dev->is_parade_solution == 1) &&
				(dev->sleep_in_mode == 0))) {
			disable_irq(g_ts_kit_platform_data.irq_id);
			TS_LOG_INFO("%s: device will sleep, disable irq\n",
				__func__);
		}
	}
	cmd.command = TS_POWER_CONTROL;
	cmd.cmd_param.pub_params.pm_type = pm_type;
	error = ts_kit_put_one_cmd(&cmd, timeout);
	if (error) {
		TS_LOG_ERR("%s: put cmd error :%d\n", __func__, error);
		error = -EBUSY;
	}
	if (pm_type == TS_AFTER_RESUME) {
		if (feature_info->pen_info.supported_pen_alg)
			ts_pen_open_confirm_init();
		TS_LOG_INFO("ts_resume_send_roi_cmd\n");
		if ((dev->is_direct_proc_cmd == 0) &&
			(dev->suspend_no_config == 0))
			/* force to write the roi function */
			ts_send_roi_cmd(TS_ACTION_WRITE, NO_SYNC_TIMEOUT);
		if (error)
			TS_LOG_ERR("ts_resume_send_roi_cmd failed\n");
	}
	return error;
}

int ts_read_debug_data(struct ts_cmd_node *in_cmd,
		struct ts_cmd_node *out_cmd, struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_diff_data_info *info = NULL;

	TS_LOG_INFO("read diff data called\n");
	if ((!in_cmd) || (!out_cmd)) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_diff_data_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (ops->chip_get_debug_data)
		error = ops->chip_get_debug_data(info, out_cmd);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_read_debug_data_timeout\n");
		kfree(info);
		return error;
	}

	if (!error) {
		TS_LOG_INFO("read diff data success\n");
		info->status = TS_ACTION_SUCCESS;
		info->time_stamp = ktime_get();
		goto out;
	}

	info->status = TS_ACTION_FAILED;
	TS_LOG_INFO("read diff data failed :%d\n", error);

out:
	return error;
}

int ts_read_rawdata_for_newformat(struct ts_cmd_node *in_cmd,
				struct ts_cmd_node *out_cmd,
				struct ts_cmd_sync *sync)
{
	struct ts_rawdata_info_new *info = NULL;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	int error = NO_ERR;

	TS_LOG_INFO("ts read rawdata for new format called\n");
	if ((!in_cmd) || (!out_cmd)) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}

	info = (struct ts_rawdata_info_new *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (!g_ts_kit_platform_data.chip_data->is_parade_solution)
		ts_stop_wd_timer(&g_ts_kit_platform_data);
	if (ops->chip_get_rawdata)
		error = ops->chip_get_rawdata((struct ts_rawdata_info *)info,
						out_cmd);
	if (!g_ts_kit_platform_data.chip_data->is_parade_solution)
		ts_start_wd_timer(&g_ts_kit_platform_data);
	if (!error) {
		TS_LOG_INFO("read rawdata success\n");
		info->status = TS_ACTION_SUCCESS;
		info->time_stamp = ktime_get();
		goto out;
	}
	info->status = TS_ACTION_FAILED;
	TS_LOG_ERR("read rawdata failed :%d\n", error);
out:
	return error;
}

int ts_read_rawdata(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd,
		struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	int ret = 0;
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;
	struct ts_rawdata_info *info = NULL;

	/**********************************************/
	/* Rawdata rectification, if dts configured   */
	/* with a new mark, take a new process        */
	/**********************************************/
	if (dev->rawdata_newformatflag == TS_RAWDATA_NEWFORMAT) {
		ret = ts_read_rawdata_for_newformat(in_cmd, out_cmd, sync);
		if (ret < 0)
			TS_LOG_ERR("%s ts_read_rawdata_for_newformat failed\n",
					__func__);
		return ret;
	}

	TS_LOG_INFO("ts read rawdata called\n");
	if ((!in_cmd) || (!out_cmd)) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_rawdata_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (!dev->is_parade_solution)
		ts_stop_wd_timer(&g_ts_kit_platform_data);
	if (dev->ops->chip_get_rawdata)
		error = dev->ops->chip_get_rawdata(info, out_cmd);
	if (!dev->is_parade_solution)
		ts_start_wd_timer(&g_ts_kit_platform_data);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_read_rawdata timeout!\n");
		if (dev->trx_delta_test_support) {
			kfree(info->rx_delta_buf);
			info->rx_delta_buf = NULL;
			kfree(info->tx_delta_buf);
			info->tx_delta_buf = NULL;
		}
		if (dev->td43xx_ee_short_test_support) {
			kfree(info->td43xx_rt95_part_one);
			info->td43xx_rt95_part_one = NULL;
			kfree(info->td43xx_rt95_part_two);
			info->td43xx_rt95_part_two = NULL;
		}
		vfree(info);
		info = NULL;

		return error;
	}

	if (!error) {
		TS_LOG_INFO("read rawdata success\n");
		info->status = TS_ACTION_SUCCESS;
		info->time_stamp = ktime_get();
		goto out;
	}

	info->status = TS_ACTION_FAILED;
	TS_LOG_ERR("read rawdata failed :%d\n", error);

out:
	return error;
}

int ts_read_calibration_data(struct ts_cmd_node *in_cmd,
		struct ts_cmd_node *out_cmd, struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_calibration_data_info *info =
		(struct ts_calibration_data_info *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s called\n", __func__);

	if (ops->chip_get_calibration_data)
		error = ops->chip_get_calibration_data(info, out_cmd);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_read_calibration_data_timeout\n");
		kfree(info);
		return error;
	}

	if (!error) {
		TS_LOG_INFO("read calibration data success\n");
		info->status = TS_ACTION_SUCCESS;
		info->time_stamp = ktime_get();
		goto out;
	}

	info->status = TS_ACTION_FAILED;
	TS_LOG_ERR("read calibration data failed :%d\n", error);

out:
	return error;
}

int ts_get_calibration_info(struct ts_cmd_node *in_cmd,
		struct ts_cmd_node *out_cmd,
		struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_calibration_info_param *info =
		(struct ts_calibration_info_param *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("%s called\n", __func__);

	if (ops->chip_get_calibration_info)
		error = ops->chip_get_calibration_info(info, out_cmd);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_get_calibration_info_timeout\n");
		kfree(info);
		return error;
	}

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_oem_info_switch(struct ts_cmd_node *in_cmd,
		struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_oem_info_param *info =
		(struct ts_oem_info_param *)in_cmd->cmd_param.prv_params;

	TS_LOG_INFO("ts chip data switch called\n");
	if (!info) {
		TS_LOG_ERR("%s, info is NULL, exit\n", __func__);
		return RESULT_ERR;
	}

	if (ops->oem_info_switch)
		error = ops->oem_info_switch(info);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_oem_info_switch_timeout\n");
		return error;
	}

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_gamma_info_switch(struct ts_cmd_node *in_cmd,
		struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_oem_info_param *info =
		(struct ts_oem_info_param *)in_cmd->cmd_param.prv_params;

	TS_LOG_INFO("ts chip data switch called\n");
	if (!info) {
		TS_LOG_ERR("%s, info is NULL, exit\n", __func__);
		return RESULT_ERR;
	}

	if (ops->gamma_info_switch)
		error = ops->gamma_info_switch(info);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_oem_info_switch_timeout\n");
		kfree(info);
		return error;
	}

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_get_chip_info(struct ts_cmd_node *in_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_chip_info_param *info =
		(struct ts_chip_info_param *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("get chip info called\n");
	if (!g_ts_kit_platform_data.chip_data->is_parade_solution)
		ts_stop_wd_timer(&g_ts_kit_platform_data);
	if (ops->chip_get_info)
		error = ops->chip_get_info(info);
	if (!g_ts_kit_platform_data.chip_data->is_parade_solution)
		ts_start_wd_timer(&g_ts_kit_platform_data);

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_set_info_flag(struct ts_cmd_node *in_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_kit_platform_data *info = NULL;

	TS_LOG_INFO("ts_set_info_flag called\n");
	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_kit_platform_data *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (ops->chip_set_info_flag)
		error = ops->chip_set_info_flag(info);

	return error;
}

int ts_calibrate(struct ts_cmd_node *in_cmd, struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_calibrate_info *info =
		(struct ts_calibrate_info *)in_cmd->cmd_param.prv_params;

	TS_LOG_DEBUG("process firmware calibrate\n");
	if (!info) {
		TS_LOG_ERR("%s, info is NULL, exit\n", __func__);
		return RESULT_ERR;
	}

	if (ops->chip_calibrate)
		error = ops->chip_calibrate();

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_calibrate_timeout\n");
		return error;
	}

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_calibrate_wakeup_gesture(struct ts_cmd_node *in_cmd,
				struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_calibrate_info *info =
		(struct ts_calibrate_info *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	TS_LOG_DEBUG("process firmware calibrate\n");

	if (ops->chip_calibrate_wakeup_gesture)
		error = ops->chip_calibrate_wakeup_gesture();

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_calibrate_wakeup_gesture_timeout\n");
		return error;
	}

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_dsm_debug(struct ts_cmd_node *in_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_dsm_debug_info *info =
		(struct ts_dsm_debug_info *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("ts dsm debug is called\n");

	if (ops->chip_dsm_debug)
		error = ops->chip_dsm_debug();

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_glove_switch(struct ts_cmd_node *in_cmd)
{
	int error = -EIO;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_glove_info *info = NULL;

	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_glove_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	TS_LOG_DEBUG("glove action :%d, value:%d", info->op_action,
		info->glove_switch);

	if (ops->chip_glove_switch)
		error = ops->chip_glove_switch(info);

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	TS_LOG_DEBUG("glove switch process result: %d\n", error);

	return error;
}

int ts_get_capacitance_test_type(struct ts_cmd_node *in_cmd,
				 struct ts_cmd_sync *sync)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_test_type_info *info = NULL;

	TS_LOG_INFO("get_mmi_test_mode called\n");
	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_test_type_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (ops->chip_get_capacitance_test_type)
		error = ops->chip_get_capacitance_test_type(info);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_get_capacitance_test_type_timeout\n");
		kfree(info);
		return error;
	}

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_chip_detect(struct ts_cmd_node *in_cmd)
{
	int error = NO_ERR;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_device_ops *ops = NULL;

	TS_LOG_INFO("ts_chip_detect called\n");
	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (atomic_read(&pdata->register_flag) == TS_REGISTER_DONE) {
		TS_LOG_INFO("%s : ts register done. ignore\n", __func__);
		return error;
	}
	pdata->chip_data = in_cmd->cmd_param.pub_params.chip_data;
	ops = pdata->chip_data->ops;
	if (ops->chip_detect)
		error = ops->chip_detect(pdata);

	if (!error) {
		atomic_set(&pdata->register_flag, TS_REGISTER_DONE);
		atomic_set(&pdata->state, TS_WORK);
		wake_up_process(pdata->ts_init_task);
#if defined(CONFIG_HUAWEI_DEV_SELFCHECK) || defined(CONFIG_HUAWEI_HW_DEV_DCT)
		/* detect current device successful, set the flag as present */
		set_tp_dev_flag();
#endif
	} else {
		pdata->chip_data = NULL;
	}
	return error;
}

int ts_palm_switch(struct ts_cmd_node *in_cmd, struct ts_cmd_sync *sync)
{
	int error = -EIO;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_palm_info *info =
		(struct ts_palm_info *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}

	TS_LOG_DEBUG("palm action :%d, value:%d",
		info->op_action, info->palm_switch);

	if (ops->chip_palm_switch)
		error = ops->chip_palm_switch(info);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_palm_switch_timeout\n");
		kfree(info);
		return error;
	}

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	TS_LOG_DEBUG("palm switch process result: %d\n", error);

	return error;
}

int ts_hand_detect(struct ts_cmd_node *in_cmd)
{
	int error = -EIO;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_hand_info *info =
		(struct ts_hand_info *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}

	if (ops->chip_hand_detect)
		error = ops->chip_hand_detect(info);

	if (error)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_force_reset(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	TS_LOG_INFO("ts force reset called\n");

	if (ops->chip_reset)
		error = ops->chip_reset();

	if (error) {
		out_cmd->command = TS_ERR_OCCUR;
		goto out;
	}

out:
	return error;
}

int ts_int_err_process(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	if (ops->chip_reset)
		error = ops->chip_reset();

	/* error nest occurred, we define nest level */
	if (error) {
		out_cmd->command = TS_ERR_OCCUR;
		goto out;
	}

out:
	return error;
}

int ts_err_process(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	static int error_count;
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	TS_LOG_INFO("error process\n");

	if (ops->chip_reset)
		error = ops->chip_reset();

	/* error nest occurred, we define nest level */
	if (error) {
		out_cmd->command = TS_ERR_OCCUR;
		goto out;
	}

	error_count = 0;
out:
	return error;
}

int ts_check_status(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	if (!out_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (ops->chip_check_status)
		error = ops->chip_check_status();

	if (error)
		out_cmd->command = TS_ERR_OCCUR;

	ts_start_wd_timer(&g_ts_kit_platform_data);
	return error;
}

int ts_wakeup_gesture_enable_switch(struct ts_cmd_node *in_cmd)
{
	int error = -EIO;
	int state;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_wakeup_gesture_enable_info *info =
		(struct ts_wakeup_gesture_enable_info *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s: write value: %d", __func__, info->switch_value);
	if (g_ts_kit_platform_data.chip_data->send_stylus_gesture_switch) {
		if (ops->chip_wakeup_gesture_enable_switch)
			error = ops->chip_wakeup_gesture_enable_switch(info);
	} else {
		state = atomic_read(&g_ts_kit_platform_data.state);
		if ((state == TS_WORK_IN_SLEEP) &&
			ops->chip_wakeup_gesture_enable_switch)
			error = ops->chip_wakeup_gesture_enable_switch(info);
	}

	info->op_action = TS_ACTION_UNDEF;
	if (error) {
		info->status = TS_ACTION_FAILED;
		TS_LOG_ERR("%s, process error: %d\n", __func__, error);
	} else {
		info->status = TS_ACTION_SUCCESS;
	}

	TS_LOG_DEBUG("%s, process result: %d\n", __func__, error);

	return error;
}

int ts_holster_switch(struct ts_cmd_node *in_cmd)
{
	int error = -EIO;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_holster_info *info = NULL;

	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_holster_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	TS_LOG_DEBUG("Holster action :%d, value:%d", info->op_action,
		info->holster_switch);

	if (ops->chip_holster_switch)
		error = ops->chip_holster_switch(info);

	if (error) {
		info->status = TS_ACTION_FAILED;
		TS_LOG_ERR("holster switch process error: %d\n", error);
	} else {
		info->status = TS_ACTION_SUCCESS;
	}

	TS_LOG_DEBUG("holster switch process result: %d\n", error);

	return error;
}

int ts_roi_switch(struct ts_cmd_node *in_cmd)
{
	int error = -EIO;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_roi_info *info = NULL;

	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_roi_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (ops->chip_roi_switch)
		error = ops->chip_roi_switch(info);

	if (error) {
		info->status = TS_ACTION_FAILED;
		TS_LOG_ERR("roi switch process error: %d\n", error);
	} else {
		info->status = TS_ACTION_SUCCESS;
	}

	TS_LOG_INFO("roi action :%d, value:%d, process result: %d\n",
		info->op_action, info->roi_switch, error);

	return error;
}

int ts_chip_regs_operate(struct ts_cmd_node *in_cmd,
		struct ts_cmd_sync *sync)
{
	int error = -EIO;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_regs_info *info = NULL;

	if (!in_cmd) {
		TS_LOG_ERR("%s : find a null pointer\n", __func__);
		return -EINVAL;
	}
	info = (struct ts_regs_info *)in_cmd->cmd_param.prv_params;
	if (!info) {
		TS_LOG_ERR("%s, find a null pointer\n", __func__);
		return -EINVAL;
	}
	if (ops->chip_regs_operate)
		error = ops->chip_regs_operate(info);

	if ((sync != NULL) &&
		(atomic_read(&sync->timeout_flag) == TS_TIMEOUT)) {
		TS_LOG_ERR("ts_chip_regs_operate_timeout\n");
		kfree(info);
		return error;
	}

	if (error < 0)
		info->status = TS_ACTION_FAILED;
	else
		info->status = TS_ACTION_SUCCESS;

	return error;
}

int ts_test_cmd(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;

	if (ops->chip_test)
		error = ops->chip_test(in_cmd, out_cmd);

	if (error) {
		out_cmd->command = TS_ERR_OCCUR;
		goto out;
	}

out:
	return error;
}

int ts_send_holster_cmd(void)
{
	int error = NO_ERR;
	struct ts_cmd_node cmd;

	TS_LOG_DEBUG("set holster\n");
	memset(&cmd, 0, sizeof(struct ts_cmd_node));
	cmd.command = TS_HOLSTER_SWITCH;
	cmd.cmd_param.prv_params =
		(void *)&g_ts_kit_platform_data.feature_info.holster_info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(&cmd);
	else
		error = ts_kit_put_one_cmd(&cmd, SHORT_SYNC_TIMEOUT);

	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}
	if (g_ts_kit_platform_data.feature_info.holster_info.status !=
			TS_ACTION_SUCCESS) {
		TS_LOG_ERR("action failed\n");
		error = -EIO;
		goto out;
	}

out:
	return error;
}

void ts_kit_charger_switch(struct ts_cmd_node *in_cmd)
{
	int error = NO_ERR;
	struct ts_device_ops *ops = g_ts_kit_platform_data.chip_data->ops;
	struct ts_charger_info *info =
		(struct ts_charger_info *)in_cmd->cmd_param.prv_params;

	if (!info) {
		TS_LOG_ERR("%s, info is NULL, exit\n", __func__);
		return;
	}

	TS_LOG_DEBUG("%s, action :%d, value:%d\n",
		__func__, info->op_action, info->charger_switch);

	if (ops->chip_charger_switch)
		error = ops->chip_charger_switch(info);

	if (error) {
		info->status = TS_ACTION_FAILED;
		TS_LOG_ERR("%s process result: %d\n", __func__, error);
	} else {
		info->status = TS_ACTION_SUCCESS;
	}

	TS_LOG_DEBUG("%s process result: %d\n", __func__, error);
}

int ts_oemdata_type_check_legal(u8 type, u8 len)
{
	int ret = 0;

	switch (type) {
	case TS_NV_STRUCTURE_PROID:
		ret = 1;
		break;
	case TS_NV_STRUCTURE_BAR_CODE:
		if (len == TS_NV_BAR_CODE_LEN)
			ret = 1;
		break;
	case TS_NV_STRUCTURE_BRIGHTNESS:
		if (len == TS_NV_BRIGHTNESS_LEN)
			ret = 1;
		break;
	case TS_NV_STRUCTURE_WHITE_POINT:
		if (len == TS_NV_WHITE_POINT_LEN)
			ret = 1;
		break;
	case TS_NV_STRUCTURE_BRI_WHITE:
		if (len == TS_NV_BRI_WHITE_LEN)
			ret = 1;
		break;
	case TS_NV_STRUCTURE_REPAIR:
		if (len == TS_NV_REPAIR_LEN)
			ret = 1;
		break;
	case TS_NV_STRUCTURE_RESERVED:
	default:
		break;
	}
	TS_LOG_INFO("%s: type:%u, len:%u, ret:%d\n",
		__func__, type, len, ret);
	return ret;
}
EXPORT_SYMBOL(ts_oemdata_type_check_legal);
/* lint -restore */
