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
#include <linux/string.h>
#include <linux/of_gpio.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/sched/rt.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
#include <linux/regulator/consumer.h>
#include <huawei_ts_kit.h>
#include <huawei_ts_kit_platform.h>
#include <huawei_ts_kit_api.h>
#if ((defined CONFIG_FB) && (defined CONFIG_HUAWEI_DEVKIT_HISI_3_0))
#include "hisi_fb.h"
#endif

#define MAX_LOTUS_NUM 6
#define DEC_BASE_DATA 10
#define RAWDATA_DEBUG_HEAD_TEXT "*************touch debug data*************\n"
#define NUM_OF_ROW 16
#define FORCEKEY_STR_LEN 50
#define FORCEKEY_DATA_LEN 10
#define TS_CHIP_INFO_SIZE 5
#define REGISTER_STORE_OFFSET 10
#define EASYWAKE_POSITION_LENGHT 9
#define EASYWAKE_POSITION_SIZE 2
#define ROI_RIGHT_SHIFT 8
#define ROI_DATA_STEP 2
#define ANTI_TAG_LENGTH 128
#define ANTI_STR_LENGTH 32
#define TUI_ENABLE_LENGTH 10
#define INFO_DATA_TEMP_LENGTH 16

enum ts_offset_bit {
	TS_OFFSET_0BIT = 0,
	TS_OFFSET_1BIT = 1,
	TS_OFFSET_2BIT = 2,
	TS_OFFSET_3BIT = 3,
	TS_OFFSET_4BIT = 4,
};

volatile bool ts_kit_gesture_func;
static bool g_ts_easy_wakeup_status;
static bool g_stylus_wake_up_status;
extern struct mutex ts_kit_easy_wake_guesure_lock;
extern struct ts_kit_platform_data g_ts_kit_platform_data;

static char *g_anti_false_touch_string[] = {
	ANTI_FALSE_TOUCH_FEATURE_ALL,
	ANTI_FALSE_TOUCH_DRV_STOP_WIDTH,
	ANTI_FALSE_TOUCH_LCD_WIDTH,
	ANTI_FALSE_TOUCH_LCD_HEIGHT,
};

static ssize_t ts_chip_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_chip_info_param *info = &g_ts_kit_platform_data.chip_info;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	cmd->command = TS_GET_CHIP_INFO;
	cmd->cmd_param.prv_params = (void *)info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(cmd);
	else
		error = ts_kit_put_one_cmd(cmd, LONG_SYNC_TIMEOUT);

	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}

	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("read action failed\n");
		error = -EIO;
		goto out;
	}
	TS_LOG_INFO("get_info_flag = %d\n",
		g_ts_kit_platform_data.get_info_flag);
	if (g_ts_kit_platform_data.get_info_flag) {
		error = snprintf(buf,
			CHIP_INFO_LENGTH, "%s\n", info->mod_vendor);
		g_ts_kit_platform_data.get_info_flag = false;
	} else if (g_ts_kit_platform_data.hide_plain_id && (!ts_is_factory())) {
		error = snprintf(buf, CHIP_INFO_LENGTH * 3, "%s\n",
			info->ic_vendor);
	} else {
		error = snprintf(buf,
			CHIP_INFO_LENGTH * TS_CHIP_INFO_SIZE, "%s-%s-%s\n",
			info->ic_vendor, info->mod_vendor, info->fw_vendor);
	}
out:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_chip_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long tmp = 0;
	unsigned int value;
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_kit_platform_data *info = NULL;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (unsigned int)tmp;
	TS_LOG_DEBUG("strict_strtoul value is %u\n", value);

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}
	info = kzalloc(sizeof(struct ts_kit_platform_data), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}
	info->get_info_flag = value;
	cmd->command = TS_SET_INFO_FLAG;
	cmd->cmd_param.prv_params = (void *)info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(cmd);
	else
		error = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);

	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}
	error = count;
out:
	kfree(cmd);
	cmd = NULL;
	kfree(info);
	info = NULL;
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_dsm_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_dsm_debug_info info;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	cmd->command = TS_DSM_DEBUG;
	cmd->cmd_param.prv_params = (void *)&info;
	error = ts_kit_put_one_cmd(cmd, LONG_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		goto out;
	}
	if (info.status == TS_ACTION_SUCCESS)
		error = snprintf(buf, MAX_STR_LEN, "%s\n", "success");
	else
		error = snprintf(buf, MAX_STR_LEN, "%s\n", "failed");

out:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("ts_debug_show done\n");
	return error;
}

static ssize_t ts_calibrate_wakeup_gesture_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_calibrate_info info;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	cmd->command = TS_CALIBRATE_DEVICE_LPWG;
	cmd->cmd_param.prv_params = (void *)&info;
	error = ts_kit_put_one_cmd(cmd, LONG_LONG_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}

	if (info.status == TS_ACTION_SUCCESS)
		error = snprintf(buf, MAX_STR_LEN, "%s\n", "success");
	else
		error = snprintf(buf, MAX_STR_LEN, "%s\n", "failed");

out:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_calibrate_info info;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	cmd->command = TS_CALIBRATE_DEVICE;
	cmd->cmd_param.prv_params = (void *)&info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(cmd);
	else
		error = ts_kit_put_one_cmd(cmd, LONG_LONG_SYNC_TIMEOUT);

	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}

	if (info.status == TS_ACTION_SUCCESS)
		error = snprintf(buf, MAX_STR_LEN, "%s\n", "success");
	else
		error = snprintf(buf, MAX_STR_LEN, "%s\n", "failed");

out:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_reset_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int error;
	struct ts_cmd_node cmd;

	TS_LOG_INFO("%s called\n", __func__);

	memset(&cmd, 0, sizeof(struct ts_cmd_node));
	cmd.command = TS_FORCE_RESET;
	error = ts_kit_put_one_cmd(&cmd, SHORT_SYNC_TIMEOUT);
	if (error)
		TS_LOG_ERR("%s failed:%d\n", __func__, error);
	TS_LOG_INFO("%s done\n", __func__);
	return count;
}

static ssize_t ts_glove_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_glove_info *info = NULL;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	info = &g_ts_kit_platform_data.feature_info.glove_info;

	info->op_action = TS_ACTION_READ;
	cmd->command = TS_GLOVE_SWITCH;
	cmd->cmd_param.prv_params = (void *)info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(cmd);
	else
		error = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);

	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto free_cmd;
	}

	if (info->status == TS_ACTION_SUCCESS)
		error = snprintf(buf, MAX_STR_LEN, "%d\n", info->glove_switch);
	else
		error = -EFAULT;

free_cmd:
	kfree(cmd);
	cmd = NULL;
out:
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_glove_mode_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	u8 value;
	unsigned long tmp = 0;
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_glove_info *info = NULL;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (u8)tmp;
	TS_LOG_DEBUG("strict_strtoul value is %u\n", value);

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	info = &g_ts_kit_platform_data.feature_info.glove_info;
	info->op_action = TS_ACTION_WRITE;
	info->glove_switch = value;
	cmd->command = TS_GLOVE_SWITCH;
	cmd->cmd_param.prv_params = (void *)info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(cmd);
	else
		error = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);

	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}

	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("action failed\n");
		error = -EIO;
		goto out;
	}

	error = count;

out:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static int ts_wakeup_gesture_enable_cmd(u8 switch_value)
{
	int error;
	struct ts_cmd_node cmd;
	struct ts_wakeup_gesture_enable_info *info =
		&g_ts_kit_platform_data.feature_info.wakeup_gesture_enable_info;

	info->op_action = TS_ACTION_WRITE;
	info->switch_value = switch_value;
	cmd.command = TS_WAKEUP_GESTURE_ENABLE;
	cmd.cmd_param.prv_params = (void *)info;

	if (!g_ts_kit_platform_data.chip_data->send_stylus_gesture_switch) {
		if (atomic_read(&g_ts_kit_platform_data.state) == TS_WORK) {
			TS_LOG_ERR(
				"can not enable/disable wakeup_gesture when tp is working in normal mode\n");
			error = -EINVAL;
			goto out;
		}
	}

	error = ts_kit_put_one_cmd(&cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("%s: put cmd error :%d\n", __func__, error);
		error = -EBUSY;
		goto out;
	}
	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("%s action failed\n", __func__);
		error = -EIO;
		goto out;
	}

out:
	return error;
}

int ts_send_roi_cmd(enum ts_action_status read_write_type, int timeout)
{
	int error = NO_ERR;
	struct ts_cmd_node cmd;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	memset(&cmd, 0, sizeof(struct ts_cmd_node));
	TS_LOG_INFO("%s, read_write_type=%d\n", __func__, read_write_type);
	if (pdata->feature_info.roi_info.roi_supported) {
		pdata->feature_info.roi_info.op_action = read_write_type;
		cmd.command = TS_ROI_SWITCH;
		cmd.cmd_param.prv_params =
			(void *)&pdata->feature_info.roi_info;
		if ((pdata->chip_data->is_direct_proc_cmd) &&
			(pdata->chip_data->is_can_device_use_int))
		/*
		 * Set this macro check to make sure
		 * bootup use the put one cmd
		 */
			error = ts_kit_proc_command_directly(&cmd);
		else
			error = ts_kit_put_one_cmd(&cmd, timeout);
		if (error) {
			TS_LOG_ERR("put cmd error :%d\n", error);
			error = -EBUSY;
		}
	}
	return error;
}

static ssize_t ts_touch_window_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int window_enable;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	int error;
	struct ts_window_info info = {0};
	struct ts_cmd_node cmd;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	memset(&cmd, 0, sizeof(struct ts_cmd_node));
	TS_LOG_INFO("%s holster_supported = %d\n", __func__,
		pdata->feature_info.holster_info.holster_supported);
	error = sscanf(buf, "%4d %4d %4d %4d %4d",
		&window_enable, &x0, &y0, &x1, &y1);
	if (error <= 0) {
		TS_LOG_ERR("sscanf error\n");
		error = -EINVAL;
		goto out;
	}
	TS_LOG_INFO("the value is %d, %d-%d, %d-%d\n",
		window_enable, x0, y0, x1, y1);
	if (window_enable && ((x0 < 0) || (y0 < 0) ||
		(x1 <= x0) || (y1 <= y0))) {
		TS_LOG_ERR("value is %d, %d-%d, %d-%d\n",
			window_enable, x0, y0, x1, y1);
		error = -EINVAL;
		goto out;
	}

	info.top_left_x0 = x0;
	info.top_left_y0 = y0;
	info.bottom_right_x1 = x1;
	info.bottom_right_y1 = y1;
	info.window_enable = window_enable;

	cmd.command = TS_TOUCH_WINDOW;
	cmd.cmd_param.prv_params = (void *)&info;
	error = ts_kit_put_one_cmd(&cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}

	if (info.status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("action failed\n");
		error = -EIO;
		goto out;
	}

	error = count;

out:
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_sensitivity_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ts_holster_info *info =
		&g_ts_kit_platform_data.feature_info.holster_info;
	ssize_t ret;

	TS_LOG_INFO("%s holster_supported = %d\n",
		__func__, info->holster_supported);
	if (g_ts_kit_platform_data.chip_data->is_parade_solution &&
		(!info->holster_supported)) {
		TS_LOG_INFO(
			"It's parade_solution, and don't support holster\n");
		return -EINVAL;
	}

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		ret = -EINVAL;
		goto out;
	}

	ret = snprintf(buf, MAX_STR_LEN, "%d\n", info->holster_switch);
out:
	return ret;
}

static ssize_t ts_sensitivity_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	u8 value;
	unsigned long tmp = 0;
	int error;
	struct ts_holster_info *info =
		&g_ts_kit_platform_data.feature_info.holster_info;

	TS_LOG_INFO("%s holster_supported = %d\n",
		__func__, info->holster_supported);
	if (g_ts_kit_platform_data.chip_data->is_parade_solution &&
		(!info->holster_supported)) {
		TS_LOG_INFO(
			"It's parade_solution, and don't support holster\n");
		return -EINVAL;
	}
	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (u8) tmp;
	TS_LOG_INFO("%s, strict_strtoul value is %u\n", __func__, value);

	info->op_action = TS_ACTION_WRITE;
	info->holster_switch = value;

	error = ts_send_holster_cmd();
	if (error) {
		TS_LOG_ERR("ts_send_holster_cmd failed\n");
		error = -ENOMEM;
		goto out;
	}

	error = count;

out:
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_hand_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_hand_info *info = NULL;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}
	info = kzalloc(sizeof(struct ts_hand_info), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	info->op_action = TS_ACTION_READ;
	cmd->command = TS_HAND_DETECT;
	cmd->cmd_param.prv_params = (void *)info;
	error = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto out;
	}

	if (info->status == TS_ACTION_SUCCESS)
		error = snprintf(buf, MAX_STR_LEN, "%d\n", info->hand_value);
	else
		error = -EFAULT;

out:
	kfree(cmd);
	cmd = NULL;
	kfree(info);
	info = NULL;
	TS_LOG_DEBUG("%s:done\n", __func__);
	return error;
}

static ssize_t ts_supported_func_indicater_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}
	/*
	 * supported_func_indicater
	 * bit order: right -> left
	 * bit0: calibrate, bit1: calibrate_wakeup_gesture
	 */
	error = snprintf(buf, MAX_STR_LEN, "%d\n",
		g_ts_kit_platform_data.chip_data->supported_func_indicater);

out:
	TS_LOG_INFO("%s done\n", __func__);
	return error;
}

static ssize_t ts_loglevel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_kit_device_data *dev_data = g_ts_kit_platform_data.chip_data;

	TS_LOG_INFO("%s called\n", __func__);
	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		return -EINVAL;
	}

	error = snprintf(buf, MAX_STR_LEN, "%d -> %d\n",
		g_ts_kit_log_cfg, !g_ts_kit_log_cfg);
	g_ts_kit_log_cfg = !g_ts_kit_log_cfg;

	if (dev_data->ops->chip_debug_switch)
		dev_data->ops->chip_debug_switch(g_ts_kit_log_cfg);

	TS_LOG_INFO("%s done\n", __func__);
	return error;
}

static ssize_t ts_touch_window_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_window_info *info = &(pdata->feature_info.window_info);

	TS_LOG_INFO("%s called\n", __func__);
	TS_LOG_INFO("%s holster_supported = %d\n", __func__,
		pdata->feature_info.holster_info.holster_supported);
	error = snprintf(buf, MAX_STR_LEN, "%d %d %d %d %d\n",
		info->window_enable,
		info->top_left_x0, info->top_left_y0,
		info->bottom_right_x1, info->bottom_right_y1);

	return error;
}

static ssize_t ts_loglevel_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t ts_fw_update_sd_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int error;
	struct ts_cmd_node cmd;

	TS_LOG_INFO("%s called\n", __func__);

	memset(&cmd, 0, sizeof(struct ts_cmd_node));
	cmd.command = TS_FW_UPDATE_SD;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(&cmd);
	else
		error = ts_kit_put_one_cmd(&cmd, LONG_LONG_SYNC_TIMEOUT);

	if (error)
		TS_LOG_ERR("%s failed:%d\n", __func__, error);

	TS_LOG_INFO("%s done\n", __func__);
	return count;
}

static ssize_t ts_register_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	int count = 0;
	char *buf_value = NULL;
	int i;
	int ret;

	TS_LOG_INFO("%s is called\n", __func__);
	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		return error;
	}
	buf_value = buf;
	for (i = 0; i < g_ts_kit_platform_data.chip_data->reg_num; i++) {
		ret = snprintf(buf_value, TS_MAX_REG_VALUE_NUM, "0x%02x",
			g_ts_kit_platform_data.chip_data->reg_values[i]);
		buf_value += ret;
		count += ret; /* 0x%2x,one num need four char to show */
	}
	TS_LOG_DEBUG(
		"show reg value error,maybe no store first,count = %d, buf = %s\n",
		count, buf);
	if (count <= 0) {
		TS_LOG_ERR(
			"show reg value error,maybe no store first,count = %d, buf = %s\n",
			count, buf);
		error = -EINVAL;
		return error;
	}
	TS_LOG_INFO("%s done\n", __func__);
	return count;
}

static ssize_t ts_register_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int addr = 0;
	int bit = 0;
	char *value = NULL;
	u8 value_u8[TS_MAX_REG_VALUE_NUM] = {0};
	int num = 0;
	int flag = 0;
	int error;
	int i;

	struct ts_cmd_node *cmd = NULL;
	struct ts_regs_info *info = NULL;
	char tmp;

	TS_LOG_INFO("%s called\n", __func__);
	if ((dev == NULL) || !g_ts_kit_platform_data.chip_data) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	if (strlen(buf) >= TS_MAX_REG_VALUE_NUM) {
		TS_LOG_ERR("Input too long\n");
		return (ssize_t) count;
	}
	value = kzalloc(TS_MAX_REG_VALUE_NUM, GFP_KERNEL);
	if (!value) {
		TS_LOG_ERR("value kzalloc error\n");
		error = -ENOMEM;
		goto out;
	}

	error = sscanf(buf, "%d 0x%4x %79s %d %d",
		&flag, &addr, value, &num, &bit);
	if (error <= 0) {
		TS_LOG_ERR(
			"sscanf return invaild flag:%d, addr:%d, value:%s, num:%d, bit:%d\n",
			flag, addr, value, num, bit);
		error = -EINVAL;
		goto free_memory;
	}

	if (num > TS_MAX_REG_VALUE_NUM - 1) {
		TS_LOG_ERR("input num is larger than the max!\n");
		error = -EINVAL;
		goto free_memory;
	}

	TS_LOG_INFO(
		"sscanf return data is flag:%d, addr:%d, value:%s, num:%d, bit:%d\n",
		flag, addr, value, num, bit);

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto free_memory;
	}
	info = kzalloc(sizeof(struct ts_regs_info), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto free_memory;
	}

	if (flag == 0)
		info->op_action = TS_ACTION_READ;
	else
		info->op_action = TS_ACTION_WRITE;

	info->addr = addr;
	info->bit = bit;
	info->num = num;
	g_ts_kit_platform_data.chip_data->reg_num = num;
	/* convert char to u8 because sscanf char from buf */
	for (i = 0; (TS_OFFSET_4BIT * i + TS_OFFSET_3BIT) <
			TS_MAX_REG_VALUE_NUM; i++) {
		tmp = value[TS_OFFSET_4BIT * i + TS_OFFSET_2BIT];
		if ((tmp >= '0') && (tmp <= '9'))
			value_u8[TS_OFFSET_2BIT * i] = tmp - '0';
		else if ((_tolower(tmp >= 'a')) && (_tolower(tmp) <= 'f'))
			/* convert hex to dec */
			value_u8[TS_OFFSET_2BIT * i] =
				_tolower(tmp) - 'a' + REGISTER_STORE_OFFSET;

		tmp = value[TS_OFFSET_4BIT * i + TS_OFFSET_3BIT];
		if ((tmp >= '0') && (tmp <= '9'))
			value_u8[TS_OFFSET_2BIT * i + TS_OFFSET_1BIT] =
				tmp - '0';
		else if ((_tolower(tmp) >= 'a') && (_tolower(tmp) <= 'f'))
			value_u8[TS_OFFSET_2BIT * i + TS_OFFSET_1BIT] =
				_tolower(tmp) - 'a' + REGISTER_STORE_OFFSET;
		/* convert input value to reg_values */
		info->values[i] =
			(value_u8[TS_OFFSET_2BIT * i] << TS_OFFSET_4BIT) |
			(value_u8[TS_OFFSET_2BIT * i + TS_OFFSET_1BIT]);
	}

	cmd->command = TS_REGS_STORE;
	cmd->cmd_param.prv_params = (void *)info;
	error = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto free_cmd;
	}
	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("action failed\n");
		error = -EIO;
		goto free_memory;
	}
	if (info->op_action == TS_ACTION_READ)
		memcpy(g_ts_kit_platform_data.chip_data->reg_values,
			info->values, TS_MAX_REG_VALUE_NUM);
	else
		memset(g_ts_kit_platform_data.chip_data->reg_values, 0, num);

	error = (int)count;

free_memory:
	kfree(info);
	info = NULL;
free_cmd:
	kfree(cmd);
	cmd = NULL;
	kfree(value);
	value = NULL;
out:
	TS_LOG_INFO("ts_reg_operate_store done\n");
	return (ssize_t)error;
}

/*
 * using  for gesture wakeup anti-mistouch when  proximity is sheltered,
 * and which will just take effect in screen off
 */
static ssize_t ts_wakeup_gesture_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u8 value;
	unsigned long tmp = 0;
	int error;

	TS_LOG_INFO("%s called\n", __func__);

	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (u8) tmp;
	error = ts_wakeup_gesture_enable_cmd(value);
	if (error) {
		TS_LOG_ERR("ts_wakeup_gesture_enable_cmd failed\n");
		error = -ENOMEM;
		goto out;
	}

	error = count;

out:
	return error;
}

static ssize_t ts_wakeup_gesture_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ts_wakeup_gesture_enable_info *info =
		&g_ts_kit_platform_data.feature_info.wakeup_gesture_enable_info;
	ssize_t ret;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		ret = -EINVAL;
		goto out;
	}

	ret = snprintf(buf, MAX_STR_LEN, "%d\n", info->switch_value);
out:
	return ret;
}

static ssize_t ts_easy_wakeup_gesture_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ts_easy_wakeup_info *info =
		&g_ts_kit_platform_data.chip_data->easy_wakeup_info;
	ssize_t ret;

	TS_LOG_INFO("%s\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		ret = -EINVAL;
		goto out;
	}

	ret = snprintf(buf, MAX_STR_LEN, "0x%04X\n",
		info->easy_wakeup_gesture +
		info->palm_cover_flag * TS_GET_CALCULATE_NUM);
out:
	return ret;
}

static ssize_t ts_easy_wakeup_gesture_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_easy_wakeup_info *info = &(pdata->chip_data->easy_wakeup_info);
	unsigned long value;
	int ret;
	struct ts_cmd_node *cmd = NULL;
	struct ts_palm_info *palm_info = NULL;
	struct ts_pen_info *pen_info = &(pdata->feature_info.pen_info);

	TS_LOG_INFO("ts_easy_wakeup_gesture_store_called\n");

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		ret = -EINVAL;
		goto out;
	}

	if (!pdata->setting_multi_gesture_mode) {
		if ((atomic_read(&(pdata->state)) == TS_SLEEP) ||
			(atomic_read(&(pdata->state)) == TS_WORK_IN_SLEEP)) {
			TS_LOG_ERR(
				"do not echo this node when tp work in sleep or tp is sleep\n");
			ret = -EINVAL;
			goto out;
		}
	}

	ret = kstrtoul(buf, DEC_BASE_DATA, &value);
	if (ret < 0)
		return ret;

	if (value > TS_GESTURE_INVALID_COMMAND)
		return -1;

	if (pdata->setting_multi_gesture_mode) {
		info->easy_wakeup_gesture = (u16) value & TS_GESTURE_COMMAND;
		TS_LOG_INFO("%s:easy_wakeup_gesture = %d\n", __func__,
			info->easy_wakeup_gesture);
		return size;
	}
	info->easy_wakeup_gesture = (u16) value & TS_GESTURE_COMMAND;
	info->palm_cover_flag =
		(u16)(value & TS_GESTURE_PALM_BIT) / TS_GET_CALCULATE_NUM;
	if (info->palm_cover_flag == true)
		info->palm_cover_control = true;
	else
		info->palm_cover_control = false;
	TS_LOG_INFO("easy_wakeup_gesture=0x%x,palm_cover_flag=0x%x\n",
		info->easy_wakeup_gesture, info->palm_cover_flag);
	if (info->easy_wakeup_gesture == false) {
		info->sleep_mode = TS_POWER_OFF_MODE;
		ts_kit_gesture_func = false;
		TS_LOG_INFO("poweroff mode\n");
		if (pen_info->pen_supported != 0) {
			g_ts_easy_wakeup_status = false;
			if (g_stylus_wake_up_status == true) {
				info->sleep_mode = TS_GESTURE_MODE;
				ts_kit_gesture_func = true;
				TS_LOG_INFO("pen gesture mode\n");
			}
		}
	} else {
		info->sleep_mode = TS_GESTURE_MODE;
		ts_kit_gesture_func = true;
		TS_LOG_INFO("gesture mode\n");
		if (pen_info->pen_supported != 0)
			g_ts_easy_wakeup_status = true;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		ret = -ENOMEM;
		goto out;
	}
	palm_info = kzalloc(sizeof(struct ts_palm_info), GFP_KERNEL);
	if (!palm_info) {
		TS_LOG_ERR("malloc failed\n");
		ret = -ENOMEM;
		goto out;
	}
	palm_info->op_action = TS_ACTION_WRITE;
	palm_info->palm_switch = info->palm_cover_control;
	cmd->command = TS_PALM_SWITCH;
	cmd->cmd_param.prv_params = (void *)palm_info;
	ret = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);
	if (ret) {
		TS_LOG_ERR("put cmd error :%d\n", ret);
		ret = -EBUSY;
		goto free_cmd;
	}
	if (palm_info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("action failed\n");
		ret = -EIO;
		goto out;
	}
	ret = size;
out:
	kfree(palm_info);
	palm_info = NULL;
free_cmd:

	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("ts gesture wakeup no done\n");

	return ret;
}

static ssize_t ts_aod_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;
	struct ts_easy_wakeup_info *info =
		&(pdata->chip_data->easy_wakeup_info);
	unsigned long value;
	int ret;

	TS_LOG_INFO("ts_aod_switch_ctrl_store_called\n");

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		return -EINVAL;
		}
	if (!pdata->aod_display_support) {
		TS_LOG_ERR("not support aod\n");
		return -EINVAL;
	}

	ret = kstrtoul(buf, DEC_BASE_DATA, &value);
	if (ret < 0)
		return ret;
	info->aod_touch_switch_ctrl = (u16) value & TS_GESTURE_COMMAND;
	return size;
}

static ssize_t ts_aod_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ts_easy_wakeup_info *info =
		&g_ts_kit_platform_data.chip_data->easy_wakeup_info;
	ssize_t ret;

	TS_LOG_INFO("%s\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		return -EINVAL;
	}
	if (!g_ts_kit_platform_data.aod_display_support) {
		TS_LOG_ERR("not support aod\n");
		return -EINVAL;
	}

	ret = snprintf(buf, MAX_STR_LEN, "%d\n",
		info->aod_touch_switch_ctrl);

	return ret;
}

static ssize_t ts_easy_wakeup_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long value;
	int ret;
	int error;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	TS_LOG_INFO("%s called\n", __func__);

	ret = kstrtoul(buf, DEC_BASE_DATA, &value);
	if (ret < 0)
		return ret;

	if (value > TS_GESTURE_INVALID_CONTROL_NO)
		return -1;

	value = (u8)value & TS_GESTURE_COMMAND;
	if (value == 1) {
		if (pdata->chip_data->ops->chip_wrong_touch !=
			NULL) {
			error = pdata->chip_data->ops->chip_wrong_touch();
			if (error < 0)
				TS_LOG_INFO("chip_wrong_touch error\n");
		} else {
			TS_LOG_INFO("chip_wrong_touch not init\n");
		}
		value = 0;
	}
	TS_LOG_INFO("%s done\n", __func__);
	return size;
}

static ssize_t ts_easy_wakeup_position_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char temp[EASYWAKE_POSITION_LENGHT] = {0};
	int i;
	struct ts_kit_device_data *pchip = g_ts_kit_platform_data.chip_data;

	TS_LOG_INFO("ts_position_show\n");
	if (dev == NULL || pchip == NULL) {
		TS_LOG_ERR("dev is null\n");
		ret = -EINVAL;
		goto out;
	}
	mutex_lock(&ts_kit_easy_wake_guesure_lock);
	for (i = 0; i < MAX_LOTUS_NUM; i++) {
		snprintf(temp,
			(sizeof(u32) * EASYWAKE_POSITION_SIZE + 1),
			"%08x",
			pchip->easy_wakeup_info.easywake_position[i]);
		strncat(buf, temp, (sizeof(u32) * EASYWAKE_POSITION_SIZE + 1));
	}
	strncat(buf, "\n", 1);
	strncat(buf, "\0", 1);
	mutex_unlock(&ts_kit_easy_wake_guesure_lock);
	ret = (strlen(buf) + 1);
out:
	return ret;
}

static ssize_t ts_capacitance_test_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_test_type_info *info = NULL;

	TS_LOG_INFO("ts_touch_test_mode_show called\n");

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}
	info = kzalloc(sizeof(struct ts_test_type_info), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	info->op_action = TS_ACTION_READ;
	cmd->command = TS_TEST_TYPE;
	cmd->cmd_param.prv_params = (void *)info;
	error = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto free_cmd;
	}

	if (info->status == TS_ACTION_SUCCESS)
		error = snprintf(buf, TS_CAP_TEST_TYPE_LEN, "%s\n",
			info->tp_test_type);
	else
		error = -EFAULT;

out:
	kfree(info);
	info = NULL;
free_cmd:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("ts_touch_test_mode_show done\n");
	return error;
}

static ssize_t ts_capacitance_test_type_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int value;
	unsigned long tmp = 0;
	int error;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (unsigned int)tmp;
	TS_LOG_DEBUG("strict_strtoul value is %u\n", value);
	error = count;

out:
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

/*lint -save -e* */
static ssize_t ts_rawdata_debug_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int index;
	int index1;
	int count = 0;
	short row_size;
	int range_size;
	int size;
	int len;
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_diff_data_info *info = NULL;
	char buffer1[FORCEKEY_DATA_LEN] = {0};
	char buffer2[FORCEKEY_STR_LEN] = {0};

	TS_LOG_INFO("%s called\n", __func__);
	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}
	info = kzalloc(sizeof(struct ts_diff_data_info), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	info->used_size = 0;
	info->debug_type = g_ts_kit_platform_data.chip_data->rawdata_debug_type;
	cmd->command = TS_DEBUG_DATA;
	cmd->cmd_param.prv_params = (void *)info;

	error = ts_kit_put_one_cmd(cmd, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto free_cmd;
	}

	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("read action failed\n");
		error = -EIO;
		goto out;
	}
	if (info->debug_type == READ_FORCE_DATA) {
		size = info->buff[0];
		if (size < 0 || size > TS_RAWDATA_BUFF_MAX - 1) {
			TS_LOG_ERR("buff size error :%d\n", size);
			goto out;
		}
		for (index = 1; index < size; index++) {
			len = snprintf(buf + count,
				FORCEKEY_DATA_LEN, "%6d", info->buff[index]);
			count = count + len;
			if (index % NUM_OF_ROW == 0) {
				len = snprintf(buf + count,
					FORCEKEY_DATA_LEN, "\n ");
				count = count + len;
			}
		}
		len = snprintf(buf + count, FORCEKEY_DATA_LEN, "\n ");
		count = count + len;
		error = count;
	} else {
		sprintf(buf, RAWDATA_DEBUG_HEAD_TEXT);
		count = count + strlen(RAWDATA_DEBUG_HEAD_TEXT);

		row_size = info->buff[0];
		if ((row_size <= 0) || (info->used_size) <= 0) {
			TS_LOG_ERR("%s data error! DO NOT surport this mode!",
				__func__);
			goto out;
		}
		range_size = info->buff[1];
		sprintf(buffer2, "rx: %d, tx : %d\n ", row_size, range_size);
		strncat(buf, buffer2, strlen(buffer2));
		count = count + strlen(buffer2);

		TS_LOG_INFO("info->used+size = %d\n", info->used_size);

		for (index = 0; (row_size * index + ROI_DATA_STEP) <
			(info->used_size); index++) {
			for (index1 = 0; index1 < row_size; index1++) {
				sprintf(buffer1, "%d,",
					(info->buff[ROI_DATA_STEP + row_size * index + index1]));
				strncat(buf, buffer1, strlen(buffer1));
				count = count + strlen(buffer1);
			}
			sprintf(buffer1, "\n ");
			strncat(buf, buffer1, strlen(buffer1));
			count = count + strlen(buffer1);
		}

		strcat(buf, "noisedata end\n");
		count = count + sizeof("noisedata end\n");
		error = count;
	}
out:
	kfree(info);
	info = NULL;
free_cmd:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_INFO("%s done\n", __func__);
	return error;
}
/*lint -restore */

static ssize_t ts_rawdata_debug_test_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int value;
	unsigned long tmp = 0;
	int error;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (unsigned int)tmp;
	TS_LOG_INFO("strict_strtoul value is %u\n", value);

	g_ts_kit_platform_data.chip_data->rawdata_debug_type = value;

	error = count;

out:
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_anti_false_touch_param_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct anti_false_touch_param *local_param = NULL;
	struct ts_kit_device_data *chip_data = g_ts_kit_platform_data.chip_data;
	int feature_all;

	TS_LOG_INFO("%s +\n", __func__);
	if (!chip_data) {
		TS_LOG_ERR("%s chip_data is null\n", __func__);
		return snprintf(buf, PAGE_SIZE, "error1\n");
	}

	local_param = &(chip_data->anti_false_touch_param_data);
	if (g_ts_kit_platform_data.aft_param.aft_enable_flag)
		feature_all = 0;
	else
		feature_all = local_param->feature_all;

	return snprintf(buf, PAGE_SIZE,
		"feature_all=%d, lcd_width=%d, lcd_height=%d, drv_stop_width=%d\n",
		feature_all,
		local_param->lcd_width,
		local_param->lcd_height, local_param->drv_stop_width);
}

static void set_value_to_param(char *tag, int value)
{
	struct ts_kit_device_data *dev = g_ts_kit_platform_data.chip_data;
	struct anti_false_touch_param *local_param = NULL;

	if (!tag || !dev) {
		TS_LOG_ERR("%s aft get tag/dev null\n", __func__);
		return;
	}

	local_param = &(dev->anti_false_touch_param_data);

	if (!strcmp(tag, ANTI_FALSE_TOUCH_FEATURE_ALL)) {
		local_param->feature_all = value;
		local_param->feature_reback_bt = value;
	} else if (!strcmp(tag, ANTI_FALSE_TOUCH_LCD_WIDTH)) {
		local_param->lcd_width = value;
	} else if (!strcmp(tag, ANTI_FALSE_TOUCH_LCD_HEIGHT)) {
		local_param->lcd_height = value;
	} else if (!strcmp(tag, ANTI_FALSE_TOUCH_DRV_STOP_WIDTH)) {
		local_param->drv_stop_width = value;
	}
	TS_LOG_INFO("%s: set %s to %d\n", __func__, tag, value);
}

static int anti_false_touch_get_param(const char *buf, char *tag)
{
	char *ptr_begin = NULL;
	char *ptr_end = NULL;
	char tmp_str[ANTI_TAG_LENGTH] = {0};
	char value_str[ANTI_STR_LENGTH] = {0};
	int len;
	unsigned long value = 0;
	int error;

	if (!buf || !tag) {
		TS_LOG_ERR("misoper get error string\n");
		return -EINVAL;
	}
	TS_LOG_DEBUG("%s:%s\n", buf, tag);
	snprintf(tmp_str, sizeof(tmp_str), "%s=", tag);
	ptr_begin = strstr(buf, tmp_str);
	if (ptr_begin != NULL) {
		ptr_begin += strlen(tmp_str);
		if (ptr_begin) {
			ptr_end = strstr(ptr_begin, ",");
			if (ptr_end) {
				len = ptr_end - ptr_begin;
				if (len > 0 && len < (int)sizeof(value_str)) {
					TS_LOG_DEBUG("%s: get %s string %s\n",
						__func__, tag, value_str);
					strncpy(value_str, ptr_begin, len);
					error = kstrtoul(value_str, 0, &value);
					if (error) {
						TS_LOG_ERR(
							"strict_strtoul return invaild :%d\n",
							error);
						return -EINVAL;
					}
					set_value_to_param(tag, (int)value);
				}
			}
		}
	}
	return 0;
}

static ssize_t ts_anti_false_touch_param_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int i;
	int str_num;
	int ret;

	TS_LOG_INFO("%s +\n", __func__);
	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		return -EINVAL;
	}
	str_num = sizeof(g_anti_false_touch_string) / sizeof(char *);
	TS_LOG_DEBUG("str_num:%d, input buf is [%s]\n", str_num, buf);
	for (i = 0; i < str_num; i++) {
		ret = anti_false_touch_get_param(buf,
			g_anti_false_touch_string[i]);
		if (ret < 0)
			TS_LOG_ERR(
				"anti_false_touch_get_param:return error,ret=%d\n",
				ret);
	}
	TS_LOG_INFO("%s -\n", __func__);
	return count;
}

static ssize_t ts_touch_wideth_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	TS_LOG_INFO("%s done\n", __func__);
	return snprintf(buf, MAX_STR_LEN, "%d\n",
			g_ts_kit_platform_data.edge_wideth);
}

static ssize_t ts_touch_wideth_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int value;
	unsigned long tmp = 0;
	int error;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (unsigned int)tmp;
	TS_LOG_INFO("strict_strtoul value is %u\n", value);
	g_ts_kit_platform_data.edge_wideth = value;

	error = count;

out:
	return error;
}

static ssize_t ts_roi_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int value;
	int error;

	error = ts_send_roi_cmd(TS_ACTION_READ, SHORT_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("ts_send_roi_show_cmd failed\n");
		error = -ENOMEM;
		goto out;
	}
	value = g_ts_kit_platform_data.feature_info.roi_info.roi_switch;
	error = snprintf(buf, MAX_STR_LEN, "%d\n", value);
out:
	TS_LOG_INFO("roi_enable_show done\n");
	return error;
}

static ssize_t ts_roi_enable_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	u8 value;
	unsigned long tmp = 0;
	int error;
	struct ts_roi_info *info =
		&g_ts_kit_platform_data.feature_info.roi_info;

	error = kstrtoul(buf, 0, &tmp);
	if (error) {
		TS_LOG_ERR("kstrtoul return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	value = (u8) tmp;
	TS_LOG_DEBUG("kstrtoul value is %u\n", value);

	if (info->roi_switch == value) {
		TS_LOG_INFO(
			"%s, there is no need to send same cmd twice. roi_switch valie is %u",
			__func__, value);
		error = count;
		goto out;
	}

	info->roi_switch = value;
	error = ts_send_roi_cmd(TS_ACTION_WRITE, NO_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("ts_send_roi_store_cmd failed\n");
		error = -ENOMEM;
		goto out;
	}
	error = count;
out:
	TS_LOG_INFO("roi_enable_store done\n");
	return error;
}

static ssize_t ts_roi_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned char *roi_data_p = NULL;
	struct ts_kit_device_data *dev_data = g_ts_kit_platform_data.chip_data;

	if (g_ts_kit_platform_data.feature_info.roi_info.roi_supported &&
		dev_data->ops->chip_roi_rawdata)
		roi_data_p = dev_data->ops->chip_roi_rawdata();

	if (roi_data_p == NULL) {
		TS_LOG_ERR("not define ROI for roi_data_show\n");
		return -ENOMEM;
	}
	/*
	 * roi_data_temp <-- This is the buffer that
	 * has the ROI data you want to send to Qeexo
	 */
	memcpy(buf, roi_data_p + ROI_HEAD_DATA_LENGTH, ROI_DATA_SEND_LENGTH);
	if (dev_data->need_notify_to_roi_algo)
		sysfs_notify(&dev->kobj, NULL, "roi_data");
	return ROI_DATA_SEND_LENGTH;
}

#define ROI_DATA_LEN (ROI_DATA_SEND_LENGTH / 2)
static ssize_t ts_roi_data_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int cnt;
	int count = 0;
	int i;
	int j = 0;
	short roi_data_16[ROI_DATA_LEN];
	unsigned char *roi_data_p = NULL;
	struct ts_kit_device_data *dev_data = g_ts_kit_platform_data.chip_data;

	if (g_ts_kit_platform_data.feature_info.roi_info.roi_supported &&
		dev_data->ops->chip_roi_rawdata)
		roi_data_p = dev_data->ops->chip_roi_rawdata();

	if (roi_data_p == NULL) {
		TS_LOG_ERR("not define ROI for roi_data_show\n");
		return -ENOMEM;
	}
	TS_LOG_DEBUG("ts_roi_data_debug_show CALLED\n");

	for (i = 0; i < ROI_HEAD_DATA_LENGTH; i++) {
		cnt = snprintf(buf, PAGE_SIZE - count, "%2d\n",
			(char)roi_data_p[i]);
		buf += cnt;
		count += cnt;
	}

	i = ROI_HEAD_DATA_LENGTH;
	for (; i < ROI_DATA_READ_LENGTH; i += ROI_DATA_STEP, j++) {
		roi_data_16[j] = roi_data_p[i] |
			(roi_data_p[i + 1] << ROI_RIGHT_SHIFT);
		cnt = snprintf(buf, PAGE_SIZE - count - 1,
			"%4d\t", roi_data_16[j]);
		buf += cnt;
		count += cnt;

		if ((j + 1) % 7 == 0) { /* The knuckle is a 7 by 7 matrix */
			cnt = snprintf(buf, PAGE_SIZE - count - 1, "\n");
			buf += cnt;
			count += cnt;
		}
	}
	snprintf(buf, PAGE_SIZE - count - 1, "\n");
	count++;
	return count;
}

#if defined(CONFIG_TEE_TUI)
static ssize_t ts_tui_report_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, TUI_ENABLE_LENGTH, "%d",
		g_ts_kit_platform_data.chip_data->report_tui_enable);
}

static ssize_t ts_tui_report_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long value = 0;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	ret = kstrtoul(buf, 0, &value);
	if (ret != 0)
		TS_LOG_ERR("%s: ssanf error, ret = %d\n", __func__, ret);

	pdata->chip_data->report_tui_enable = (unsigned int)value;

	if (pdata->chip_data->report_tui_enable == true)
		ts_kit_tui_secos_init();
	else
		ts_kit_tui_secos_exit();

	TS_LOG_INFO("tui_report enable is %d\n",
		pdata->chip_data->report_tui_enable);

	return count;
}
#endif

static ssize_t ts_oem_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_oem_info_param *info = NULL;
	unsigned char str_oem[TS_CHIP_TYPE_MAX_SIZE];
	unsigned char str_tmp[TS_CHIP_TYPE_MAX_SIZE];
	int i;
	int index;

	TS_LOG_INFO("%s: called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("%s: dev is null\n", __func__);
		error = -EINVAL;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("%s: malloc failed\n", __func__);
		error = -ENOMEM;
		goto out;
	}

	info = kzalloc(sizeof(struct ts_oem_info_param), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("%s: malloc failed\n", __func__);
		error = snprintf(buf, TS_CHIP_TYPE_MAX_SIZE, "%d,\n",
			TS_CHIP_READ_OEM_INFO_ERROR);
		goto out;
	}

	info->op_action = TS_ACTION_READ;
	cmd->command = TS_OEM_INFO_SWITCH;
	cmd->cmd_param.prv_params = (void *)info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd) {
		TS_LOG_INFO("%s:use put_one_cmd_thread func\n", __func__);
		error = ts_kit_proc_command_directly(cmd);
	} else {
		error = ts_kit_put_one_cmd(cmd, LONG_SYNC_TIMEOUT);
	}
	if (error) {
		TS_LOG_ERR("%s: put cmd error :%d\n", __func__, error);
		error = snprintf(buf, TS_CHIP_TYPE_MAX_SIZE, "%d,\n",
			TS_CHIP_READ_OEM_INFO_ERROR);
		goto free_cmd;
	}

	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("%s: read action failed\n", __func__);
		error = snprintf(buf, TS_CHIP_TYPE_MAX_SIZE, "%d,\n",
			TS_CHIP_READ_OEM_INFO_ERROR);
		TS_LOG_INFO("%s: return to sys str:%s\n", __func__, buf);
		goto out;
	}
	if (g_ts_kit_platform_data.chip_data->is_new_oem_structure) {
		TS_LOG_INFO("%s: use new oem structure\n", __func__);
		memset(str_oem, 0, sizeof(str_oem));
		if (info->data[1] != 0) {
			for (i = 0; i < info->length; ++i) {
				for (index = 0; index < INFO_DATA_TEMP_LENGTH; index++) {
					memset(str_tmp, 0, sizeof(str_tmp));
					snprintf(str_tmp, sizeof(str_tmp), "%d,",
						info->data[index + i * INFO_DATA_TEMP_LENGTH]);
					strncat(str_oem, str_tmp, strlen(str_tmp));
				}
			}
			TS_LOG_INFO("%s:  str_oem string:%s\n", __func__,
				str_oem);
		}
	} else {
		memset(str_oem, 0, sizeof(str_oem));
		if (info->data[1] != 0) {
			for (i = 0; i < info->data[1]; ++i) {
				for (index = 0; index < INFO_DATA_TEMP_LENGTH; index++) {
					memset(str_tmp, 0, sizeof(str_tmp));
					snprintf(str_tmp, sizeof(str_tmp), "%d,",
						info->data[index + i * INFO_DATA_TEMP_LENGTH]);
					strncat(str_oem, str_tmp, strlen(str_tmp));
				}
			}
			TS_LOG_INFO("%s:  str_oem string:%s\n", __func__,
				str_oem);
		}
	}
	if (strlen(info->data) == 1) {
		error = snprintf(buf, TS_CHIP_TYPE_MAX_SIZE, "%d,\n",
			info->data[0]);
		TS_LOG_INFO("%s: Return read result string:%s to sys file\n",
			__func__, buf);
	} else {
		error = snprintf(buf, TS_CHIP_TYPE_MAX_SIZE, "%s\n", str_oem);
		TS_LOG_INFO("%s: Return read data string:%s to sys file\n",
			__func__, buf);
	}

out:
	kfree(info);
	info = NULL;
free_cmd:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("%s done\n", __func__);
	return error;
}

static ssize_t ts_oem_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_oem_info_param *info = NULL;
	char *cur = NULL;
	char *token = NULL;
	int i;
	long int value;
#if defined(CONFIG_LCD_KIT_DRIVER)
	struct lcd_kit_ops *lcd_ops = lcd_kit_get_ops();
#endif

	TS_LOG_INFO("%s: called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("%s: dev is null\n", __func__);
		error = -EINVAL;
		return error;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("%s: malloc failed\n", __func__);
		error = -ENOMEM;
		return error;
	}

	info = kzalloc(sizeof(struct ts_oem_info_param), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("%s: malloc failed\n", __func__);
		error = -ENOMEM;
		goto out;
	}

	if (strlen(buf) > TS_GAMMA_DATA_MAX_SIZE - 1) {
		TS_LOG_ERR(
			"%s: Store TPIC type size= %lu > MAX input size=%d\n",
			__func__, strlen(buf), TS_CHIP_TYPE_MAX_SIZE);
		error = -EINVAL;
		goto out;
	}

	i = 0;
	cur = (char *)buf;
	token = strsep(&cur, ",");
	while (token) {
		info->data[i++] = 0;
		value = 0;
		if (kstrtol(token, 0, &value) == 0)
			info->data[i++] = value;
		token = strsep(&cur, ",");
	}

	info->op_action = TS_ACTION_WRITE;
	cmd->command = TS_OEM_INFO_SWITCH;
	cmd->cmd_param.prv_params = (void *)info;
	if (g_ts_kit_platform_data.chip_data->is_direct_proc_cmd) {
		TS_LOG_INFO("%s:use put_one_cmd_thread func\n", __func__);
		error = ts_kit_proc_command_directly(cmd);
	} else {
		error = ts_kit_put_one_cmd(cmd, LONG_LONG_SYNC_TIMEOUT);
	}
	if (error) {
		TS_LOG_ERR("%s: put cmd error :%d\n", __func__, error);
		error = -EBUSY;
		goto free_cmd;
	}
	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("%s action failed\n", __func__);
		error = -EIO;
		goto out;
	}
#if defined(CONFIG_LCD_KIT_DRIVER)
	if ((lcd_ops) && (lcd_ops->write_otp_gamma) &&
		g_ts_kit_platform_data.chip_data->support_gammadata_in_tp) {
		error = lcd_ops->write_otp_gamma(info->data);
		if (error < 0) {
			TS_LOG_ERR("%s: get pt station status fail\n",
				__func__);
			goto out;
		}
	}
#endif

	error = count;
out:
	kfree(info);
	info = NULL;
free_cmd:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_DEBUG("%s: done\n", __func__);
	return error;
}

static ssize_t ts_calibration_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_cmd_node *cmd = NULL;
	struct ts_calibration_info_param *info = NULL;
	struct ts_kit_platform_data *pdata = &g_ts_kit_platform_data;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	if (!(pdata->chip_data->should_check_tp_calibration_info)) {
		TS_LOG_ERR("No calibration info\n");
		error = NO_ERR;
		goto out;
	}

	cmd = kzalloc(sizeof(struct ts_cmd_node), GFP_KERNEL);
	if (!cmd) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	info = kzalloc(sizeof(struct ts_calibration_info_param), GFP_KERNEL);
	if (!info) {
		TS_LOG_ERR("malloc failed\n");
		error = -ENOMEM;
		goto out;
	}

	cmd->command = TS_GET_CALIBRATION_INFO;
	cmd->cmd_param.prv_params = (void *)info;

	if (pdata->chip_data->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(cmd);
	else
		error = ts_kit_put_one_cmd(cmd, LONG_SYNC_TIMEOUT);
	if (error) {
		TS_LOG_ERR("put cmd error :%d\n", error);
		error = -EBUSY;
		goto free_cmd;
	}

	if (info->status != TS_ACTION_SUCCESS) {
		TS_LOG_ERR("read action failed\n");
		error = -EIO;
		goto out;
	}
	error = snprintf(buf, PAGE_SIZE, "%d\n", info->calibration_crc);
out:
	kfree(info);
	info = NULL;
free_cmd:
	kfree(cmd);
	cmd = NULL;
	TS_LOG_INFO("%s done\n", __func__);

	return error;
}

static ssize_t ts_capacitance_test_config_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;

	TS_LOG_INFO("%s called\n", __func__);

	if (dev == NULL) {
		TS_LOG_ERR("dev is null\n");
		error = -EINVAL;
		goto out;
	}
	error = snprintf(buf, MAX_STR_LEN, "%d\n",
		g_ts_kit_platform_data.chip_data->capacitance_test_config);

out:
	TS_LOG_INFO("%s done\n", __func__);
	return error;
}

static ssize_t ts_touch_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int value;
	int error = -1;

	TS_LOG_INFO("%s called\n", __func__);
	if (g_ts_kit_platform_data.chip_data == NULL) {
		TS_LOG_ERR("%s error\n", __func__);
		goto out;
	}
	value = g_ts_kit_platform_data.chip_data->touch_switch_flag;
	error = snprintf(buf, MAX_STR_LEN, "%d\n", value);
out:
	TS_LOG_INFO("-\n");
	return error;
}

static ssize_t ts_touch_switch_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int len;
	int error;
	struct ts_cmd_node cmd;
	struct ts_kit_device_data *pdata = g_ts_kit_platform_data.chip_data;

	TS_LOG_INFO("%s called\n", __func__);
	len = strlen(buf);
	if ((pdata == NULL) || (len < 0) || (len > MAX_STR_LEN - 1)) {
		TS_LOG_ERR("%s error, len:%d\n", __func__, len);
		goto out;
	}
	if (((pdata->touch_switch_flag & TS_SWITCH_TYPE_DOZE) !=
		TS_SWITCH_TYPE_DOZE) &&
		((pdata->touch_switch_flag & TS_SWITCH_TYPE_GAME) !=
		TS_SWITCH_TYPE_GAME) &&
		((pdata->touch_switch_flag & TS_SWITCH_TYPE_SCENE) !=
		TS_SWITCH_TYPE_SCENE)) {
		TS_LOG_INFO("tp doze,game,scene switch not support\n");
		goto out;
	}

	memset(pdata->touch_switch_info, 0, MAX_STR_LEN);
	snprintf(pdata->touch_switch_info, MAX_STR_LEN - 1, "%s", buf);

	memset(&cmd, 0, sizeof(struct ts_cmd_node));
	cmd.command = TS_TOUCH_SWITCH;
	cmd.cmd_param.prv_params = (void *)&pdata->touch_switch_info;
	if (pdata->is_direct_proc_cmd)
		error = ts_kit_proc_command_directly(&cmd);
	else
		error = ts_kit_put_one_cmd(&cmd, SHORT_SYNC_TIMEOUT);

	if (error)
		TS_LOG_ERR("put cmd error :%d\n", error);

out:
	TS_LOG_INFO("-\n");
	return count;
}

static ssize_t udfp_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;

	TS_LOG_INFO("%s,0x%x\n", __func__,
		g_ts_kit_platform_data.udfp_enable_flag);
	error = snprintf(buf, MAX_STR_LEN, "%d\n",
		g_ts_kit_platform_data.udfp_enable_flag);

	return error;
}

static ssize_t udfp_enable_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int value = 0;
	int error;

	error = kstrtouint(buf, 0, &value);
	TS_LOG_DEBUG("%s,value  = %d   |error = %d\n", __func__, value, error);
	g_ts_kit_platform_data.udfp_enable_flag = value;
	TS_LOG_INFO("[%s] -> %d\n", __func__,
		g_ts_kit_platform_data.udfp_enable_flag);
	error = count;

	return error;
}

static ssize_t stylus_wakeup_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error;
	struct ts_pen_info *info =
		&g_ts_kit_platform_data.feature_info.pen_info;

	TS_LOG_INFO("%s,0x%x\n", __func__, info->supported_pen_mmitest);
	error = snprintf(buf, MAX_STR_LEN, "%d\n",
		info->supported_pen_mmitest);
	return error;
}

static ssize_t stylus_wakeup_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int parameter = 0;
	int ret;
	unsigned int stylus_wake_up_mask = (1 << GESTURE_STYLUS_WAKE_UP);
	struct ts_easy_wakeup_info *info = NULL;
	struct ts_wakeup_gesture_enable_info *gesture_enabel_info =
		&g_ts_kit_platform_data.feature_info.wakeup_gesture_enable_info;
	struct ts_pen_info *pen_info =
		&g_ts_kit_platform_data.feature_info.pen_info;

	ret = kstrtoint(buf, 0, &parameter);
	TS_LOG_INFO("%s,parameter = %d,error =%d\n", __func__, parameter, ret);

	if (!dev || !g_ts_kit_platform_data.chip_data || ret < 0) {
		TS_LOG_ERR("%s:invalid input\n", __func__);
		goto out;
	}
	if ((atomic_read(&g_ts_kit_platform_data.state) == TS_SLEEP) ||
			(atomic_read(&g_ts_kit_platform_data.state) ==
				TS_WORK_IN_SLEEP)) {
		if ((parameter >= 0) && (parameter < MAX_STATUS))
			/*
			 * save switch status when tp in sleep,
			 * and send to ic when tp resume
			 */
			gesture_enabel_info->switch_value = parameter;

		TS_LOG_ERR(
			"do not echo this node when tp work in sleep or tp is sleep\n");
		goto out;
	}

	info = &g_ts_kit_platform_data.chip_data->easy_wakeup_info;
	if (parameter == STYLUS_WAKEUP_NORMAL_STATUS) {
		info->easy_wakeup_gesture |= stylus_wake_up_mask;
		info->sleep_mode = TS_GESTURE_MODE;
		ts_kit_gesture_func = true;
		TS_LOG_INFO("gesture mode :0x%x\n", info->easy_wakeup_gesture);
		if (pen_info->pen_supported != 0)
			g_stylus_wake_up_status = true;
	} else if ((parameter == STYLUS_WAKEUP_DISABLE) ||
		(parameter == STYLUS_WAKEUP_LOW_FREQENCY)) {
		info->easy_wakeup_gesture &= (~stylus_wake_up_mask);
		info->sleep_mode = TS_POWER_OFF_MODE;
		ts_kit_gesture_func = false;
		TS_LOG_INFO("poweroff mode:0x%x\n", info->easy_wakeup_gesture);
		if (pen_info->pen_supported != 0) {
			g_stylus_wake_up_status = false;
			if (g_ts_easy_wakeup_status == true) {
				info->sleep_mode = TS_GESTURE_MODE;
				ts_kit_gesture_func = true;
				TS_LOG_INFO("double gesture mode\n");
			}
		}
	} else if (parameter == STYLUS_WAKEUP_TESTMODE) {
		TS_LOG_INFO("enter test mode\n");
	} else {
		TS_LOG_INFO("invalid  input of STYLUS_WAKEUP_CTRL\n");
		goto out;
	}
	ret = ts_wakeup_gesture_enable_cmd(parameter);
out:
	return count;
}

#define NEED_SCREEN_OFF 1
static ssize_t ts_factory_extra_cmd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int type = 0;
	int data = 0;
	struct ts_factory_extra_cmd *factory_extra_cmd =
		&g_ts_kit_platform_data.factory_extra_cmd;
#if ((defined CONFIG_FB) && (defined CONFIG_HUAWEI_DEVKIT_HISI_3_0))
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];
#endif

	if (dev == NULL) {
		TS_LOG_ERR("%s: dev is null\n", __func__);
		return -EINVAL;
	}
	if ((factory_extra_cmd->support_factory_mode_extra_cmd == 0) ||
		(atomic_read(&g_ts_kit_platform_data.state) == TS_SLEEP) ||
		(atomic_read(&g_ts_kit_platform_data.state) ==
			TS_WORK_IN_SLEEP)) {
		TS_LOG_INFO("%s: not support or suspended\n", __func__);
		return -EINVAL;
	}
	ret = sscanf(buf, "%3u,%4d", &type, &data);
	if (ret <= 0) {
		TS_LOG_INFO("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	TS_LOG_INFO("%s: type:%u, data:%d\n",
		__func__, type, data);
	switch (type) {
	case SUSPEND_NEED_CAP_TEST:
		if (ts_is_factory()) {
			TS_LOG_INFO("%s in factory mode\n", __func__);
#if ((defined CONFIG_FB) && (defined CONFIG_HUAWEI_DEVKIT_HISI_3_0))
			if (data == NEED_SCREEN_OFF) {
				factory_extra_cmd->always_poweron_in_screenoff =
					data;
				factory_extra_cmd->factory_mode_current_backlight =
					hisifd->bl_level;
				TS_LOG_INFO("%s lcd power off current_bl = %d\n",
					__func__,
					factory_extra_cmd->factory_mode_current_backlight);
				hisi_fb_blank_sub(FB_BLANK_POWERDOWN,
					hisifd->fbi);
			} else {
				hisi_fb_blank_sub(FB_BLANK_UNBLANK,
					hisifd->fbi);
				/* triggers refresh and brightness setting */
				hisi_fb_frame_refresh(hisifd, "esd");
				msleep(100); /* delay 100ms */
				hisifb_set_backlight(hisifd,
					factory_extra_cmd->factory_mode_current_backlight,
					false);
				TS_LOG_INFO("%s lcd power on current_bl = %d\n",
					__func__,
					factory_extra_cmd->factory_mode_current_backlight);
				factory_extra_cmd->always_poweron_in_screenoff =
					data;
			}
#endif
		}
		break;
	default:
		return -EINVAL;
	}
	return count;
}

static DEVICE_ATTR(touch_chip_info, 0640,
	ts_chip_info_show, ts_chip_info_store);
static DEVICE_ATTR(calibrate, 0440, ts_calibrate_show, NULL);
static DEVICE_ATTR(calibrate_wakeup_gesture, 0400,
	ts_calibrate_wakeup_gesture_show, NULL);
static DEVICE_ATTR(touch_glove, 0600, ts_glove_mode_show,
	ts_glove_mode_store);
static DEVICE_ATTR(touch_sensitivity, 0600, ts_sensitivity_show,
	ts_sensitivity_store);
static DEVICE_ATTR(hand_detect, 0400, ts_hand_detect_show, NULL);
static DEVICE_ATTR(loglevel, 0600, ts_loglevel_show,
	ts_loglevel_store);
static DEVICE_ATTR(supported_func_indicater, 0440,
	ts_supported_func_indicater_show, NULL);
static DEVICE_ATTR(touch_window, 0600, ts_touch_window_show,
	ts_touch_window_store);
static DEVICE_ATTR(fw_update_sd, 0200, NULL, ts_fw_update_sd_store);
static DEVICE_ATTR(reset, 0200, NULL, ts_reset_store);
static DEVICE_ATTR(easy_wakeup_gesture, 0600,
	ts_easy_wakeup_gesture_show, ts_easy_wakeup_gesture_store);
static DEVICE_ATTR(aod_touch_switch_ctrl, 0600,
	ts_aod_switch_ctrl_show, ts_aod_switch_ctrl_store);
static DEVICE_ATTR(wakeup_gesture_enable, 0600,
	ts_wakeup_gesture_enable_show, ts_wakeup_gesture_enable_store);
static DEVICE_ATTR(touch_dsm_debug, 0444,
	ts_dsm_debug_show, NULL);
static DEVICE_ATTR(easy_wakeup_control, 0200, NULL,
	ts_easy_wakeup_control_store);
static DEVICE_ATTR(easy_wakeup_position, 0400, ts_easy_wakeup_position_show,
	NULL);
static DEVICE_ATTR(touch_register_operation, 0660, ts_register_show,
	ts_register_store);
static DEVICE_ATTR(tp_capacitance_test_type, 0600,
	ts_capacitance_test_type_show, ts_capacitance_test_type_store);
static DEVICE_ATTR(touch_rawdata_debug, 0660,
	ts_rawdata_debug_test_show, ts_rawdata_debug_test_store);
static DEVICE_ATTR(anti_false_touch_param,
	0660,
	ts_anti_false_touch_param_show, ts_anti_false_touch_param_store);
static DEVICE_ATTR(touch_wideth, 0660,
	ts_touch_wideth_show, ts_touch_wideth_store);
static DEVICE_ATTR(roi_enable, 0660,
	ts_roi_enable_show, ts_roi_enable_store);
static DEVICE_ATTR(roi_data, 0440, ts_roi_data_show, NULL);
static DEVICE_ATTR(roi_data_debug, 0440, ts_roi_data_debug_show,
	NULL);
#if defined(CONFIG_TEE_TUI)
static DEVICE_ATTR(touch_tui_enable, 0600, ts_tui_report_show,
	ts_tui_report_store);
#endif
static DEVICE_ATTR(touch_oem_info, 0644,
	ts_oem_info_show, ts_oem_info_store);
static DEVICE_ATTR(calibration_info, 0644,
	ts_calibration_info_show, NULL);
static DEVICE_ATTR(tp_capacitance_test_config, 0400,
	ts_capacitance_test_config_show, NULL);
static DEVICE_ATTR(touch_switch, 0660,
	ts_touch_switch_show, ts_touch_switch_store);
static DEVICE_ATTR(udfp_enable, 0660,
	udfp_enable_show, udfp_enable_store);
static DEVICE_ATTR(stylus_wakeup_ctrl, 0660,
	stylus_wakeup_ctrl_show, stylus_wakeup_ctrl_store);
static DEVICE_ATTR(factory_extra_cmd, 0660,
	NULL, ts_factory_extra_cmd_store);

static struct attribute *ts_attributes[] = {
	&dev_attr_touch_oem_info.attr,
	&dev_attr_touch_chip_info.attr,
	&dev_attr_calibration_info.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_wakeup_gesture.attr,
	&dev_attr_touch_glove.attr,
	&dev_attr_touch_sensitivity.attr,
	&dev_attr_hand_detect.attr,
	&dev_attr_loglevel.attr,
	&dev_attr_supported_func_indicater.attr,
	&dev_attr_touch_window.attr,
	&dev_attr_fw_update_sd.attr,
	&dev_attr_reset.attr,
	&dev_attr_easy_wakeup_gesture.attr,
	&dev_attr_wakeup_gesture_enable.attr,
	&dev_attr_touch_dsm_debug.attr,
	&dev_attr_easy_wakeup_control.attr,
	&dev_attr_aod_touch_switch_ctrl.attr,
	&dev_attr_easy_wakeup_position.attr,
	&dev_attr_touch_register_operation.attr,
	&dev_attr_roi_enable.attr,
	&dev_attr_roi_data.attr,
	&dev_attr_roi_data_debug.attr,
	&dev_attr_tp_capacitance_test_type.attr,
	&dev_attr_tp_capacitance_test_config.attr,
	&dev_attr_touch_rawdata_debug.attr,
	&dev_attr_anti_false_touch_param.attr,
	&dev_attr_touch_wideth.attr,
#if defined(CONFIG_TEE_TUI)
	&dev_attr_touch_tui_enable.attr,
#endif
	&dev_attr_touch_switch.attr,
	&dev_attr_udfp_enable.attr,
	&dev_attr_stylus_wakeup_ctrl.attr,
	&dev_attr_factory_extra_cmd.attr,
	NULL
};

const struct attribute_group ts_attr_group = {
	.attrs = ts_attributes,
};
