/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: ming feng <ming_feng@ilitek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "ilitek.h"

#define ABS_MT_TOUCH_MAJOR_MAX 255
#define ABS_MT_TOUCH_MINOR_MAX 255
#define ILITEK_DOZE_MAX_INPUT_SEPARATE_NUM 2
#define REAL_PROJECT_ID_LEN 10

struct ilitek_ts_data *ilits;
extern int hostprocessing_get_project_id(char *out);

void ili_tp_reset(void)
{
#ifdef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	int error = 0;
#endif

	TS_LOG_INFO("edge delay = %d\n", ilits->rst_edge_delay);

	/* Need accurate power sequence, do not change it to msleep */
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	gpio_direction_output(ilits->tp_rst, 1);
	mdelay(1);
	gpio_set_value(ilits->tp_rst, 0);
	mdelay(5);
	gpio_set_value(ilits->tp_rst, 1);
	mdelay(ilits->rst_edge_delay);
#else
	error = pinctrl_select_state(ilits->pctrl, ilits->pinctrl_state_reset_high);
	mdelay(1);
	error = pinctrl_select_state(ilits->pctrl, ilits->pinctrl_state_reset_low);
	mdelay(5);
	error = pinctrl_select_state(ilits->pctrl, ilits->pinctrl_state_reset_high);
	mdelay(ilits->rst_edge_delay);
#endif
}

int ilitek_tddi_upgrade_get_fw_name(void)
{
	struct ts_kit_device_data *p_dev_data = ilits->ts_dev_data;

	int ret = 0;

	if (!strnlen(p_dev_data->ts_platform_data->product_name, MAX_STR_LEN - 1)
		|| !strnlen(p_dev_data->chip_name, MAX_STR_LEN - 1)
		|| !strnlen(ilits->project_id, ILITEK_PROJECT_ID_LEN)
		|| !strnlen(p_dev_data->module_name, MAX_STR_LEN - 1)) {
		TS_LOG_ERR("fw file name is not detected");
		return -EINVAL;
	}

	if (p_dev_data->hide_fw_name) {
		ret = snprintf(ilits->fw_name, ILITEK_FILE_NAME_LEN, "ts/%s.img", ilits->project_id);
		if (ret >= ILITEK_FILE_NAME_LEN) {
			TS_LOG_ERR("fw name buffer out of range, ret=%d\n", ret);
			return -ENOMEM;
		}
	} else {
		snprintf(ilits->fw_name, ILITEK_FILE_NAME_LEN, "ts/%s_%s_%s_%s.img",
			p_dev_data->ts_platform_data->product_name,
			p_dev_data->chip_name,
			ilits->project_id,
			p_dev_data->module_name);
	}

	TS_LOG_INFO("firmware file name = %s", ilits->fw_name);

	return 0;
}

static int ilitek_tddi_pinctrl_init(void)
{
	int ret = 0;

	ilits->pctrl = devm_pinctrl_get(&ilits->pdev->dev);
	if (IS_ERR_OR_NULL(ilits->pctrl)) {
		TS_LOG_ERR("get pinctrl failed\n");
		return -EINVAL;
	}
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ilits->pins_default =
		pinctrl_lookup_state(ilits->pctrl, ILITEK_DTS_PIN_DEFAULT);
	if (IS_ERR(ilits->pins_default)) {
		TS_LOG_ERR("failed to pinctrl lookup state default\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

	ilits->pins_idle = pinctrl_lookup_state(ilits->pctrl, ILITEK_DTS_PIN_IDLE);
	if (IS_ERR(ilits->pins_idle)) {
		TS_LOG_ERR("failed to pinctrl lookup state idle\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
#else
	 ilits->pinctrl_state_reset_high = pinctrl_lookup_state(ilits->pctrl, PINCTRL_STATE_RESET_HIGH);
	if (IS_ERR_OR_NULL(ilits->pinctrl_state_reset_high)) {
		TS_LOG_ERR("Can not lookup %s pinstate\n", PINCTRL_STATE_RESET_HIGH);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	ilits->pinctrl_state_reset_low = pinctrl_lookup_state(ilits->pctrl, PINCTRL_STATE_RESET_LOW);
	if (IS_ERR_OR_NULL(ilits->pinctrl_state_reset_low)) {
		TS_LOG_ERR("-Can not lookup %s pinstate\n", PINCTRL_STATE_RESET_LOW);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	ilits->pinctrl_state_as_int = pinctrl_lookup_state(ilits->pctrl, PINCTRL_STATE_AS_INT);
	if (IS_ERR_OR_NULL(ilits->pinctrl_state_as_int)) {
		TS_LOG_ERR("-Can not lookup %s pinstate \n", PINCTRL_STATE_AS_INT);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	ilits->pinctrl_state_int_high = pinctrl_lookup_state(ilits->pctrl, PINCTRL_STATE_INT_HIGH);
	if (IS_ERR_OR_NULL(ilits->pinctrl_state_int_high)) {
		TS_LOG_ERR("-Can not lookup %s pinstate\n", PINCTRL_STATE_INT_HIGH);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	ilits->pinctrl_state_int_low = pinctrl_lookup_state(ilits->pctrl, PINCTRL_STATE_INT_LOW);
	if (IS_ERR_OR_NULL(ilits->pinctrl_state_int_low)) {
		TS_LOG_ERR("-Can not lookup %s pinstate \n",
			PINCTRL_STATE_INT_LOW);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}

	ret = pinctrl_select_state(ilits->pctrl, ilits->pinctrl_state_as_int);
	if (ret < 0) {
		TS_LOG_ERR("set gpio as int failed\n");
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
#endif
	TS_LOG_INFO("pinctrl init succeeded\n");

	return 0;

err_pinctrl_put:
	if (ilits->pctrl) {
		devm_pinctrl_put(ilits->pctrl);
		ilits->pctrl = NULL;
	}
	return ret;
}
static int ilitek_pinctrl_release(void)
{
	if (ilits->pctrl) {
		devm_pinctrl_put(ilits->pctrl);
		ilits->pctrl = NULL;
		ilits->pins_default = NULL;
		ilits->pins_idle = NULL;
#if defined(CONFIG_HUAWEI_DEVKIT_MTK_3_0)
	ilits->pinctrl_state_reset_high = NULL;
	ilits->pinctrl_state_int_high = NULL;
	ilits->pinctrl_state_int_low = NULL;
	ilits->pinctrl_state_reset_low = NULL;
	ilits->pinctrl_state_as_int = NULL;
#endif

	}

	TS_LOG_INFO("pinctrl release succeeded\n");

	return 0;
}
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
static int ilitek_pinctrl_select_default(void)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(ilits->pctrl) ||
		IS_ERR_OR_NULL(ilits->pins_default)) {
		TS_LOG_ERR("pctrl or pins_default is NULL\n");
		return -EINVAL;
	}

	ret = pinctrl_select_state(ilits->pctrl, ilits->pins_default);
	if (ret) {
		TS_LOG_ERR("pinctrl select default failed\n");
		return -EINVAL;
	}

	TS_LOG_INFO("pinctrl select default succeeded\n");

	return 0;
}

static int ilitek_pinctrl_select_idle(void)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(ilits->pctrl) ||
		IS_ERR_OR_NULL(ilits->pins_idle)) {
		TS_LOG_ERR("pctrl or pins_idle is NULL\n");
		return -EINVAL;
	}

	ret = pinctrl_select_state(ilits->pctrl, ilits->pins_idle);
	if (ret) {
		TS_LOG_ERR("pinctrl select idle failed\n");
		return -EINVAL;
	}

	TS_LOG_INFO("pinctrl select idle succeeded\n");

	return 0;
}
#endif
static int ilitek_bus_communicate_check(void)
{

	mutex_init(&ilits->touch_mutex);
	mutex_init(&ilits->debug_mutex);
	mutex_init(&ilits->debug_read_mutex);
	mutex_init(&ilits->wrong_touch_lock);
	init_waitqueue_head(&(ilits->inq));
	spin_lock_init(&ilits->irq_spin);
	init_completion(&ilits->esd_done);

	atomic_set(&ilits->irq_stat, DISABLE);
	atomic_set(&ilits->ice_stat, DISABLE);
	atomic_set(&ilits->tp_reset, END);
	atomic_set(&ilits->fw_stat, END);
	atomic_set(&ilits->mp_stat, DISABLE);
	atomic_set(&ilits->tp_sleep, END);
	atomic_set(&ilits->cmd_int_check, DISABLE);
	atomic_set(&ilits->esd_stat, END);

	ili_ic_init();

	/* Must do hw reset once in first time for work normally if tp reset is available */
#if !TDDI_RST_BIND
	if (ili_reset_ctrl(ilits->reset) < 0)
		TS_LOG_ERR("TP Reset failed during init\n");
#endif


	/*
	 * This status of ice enable will be reset until process of fw upgrade runs.
	 * it might cause unknown problems if we disable ice mode without any
	 * codes inside touch ic.
	 */
	if (ili_ice_mode_ctrl(ENABLE, OFF) < 0)
		TS_LOG_ERR("Failed enable ice during communicate check\n");

	if (ili_ic_dummy_check() < 0) {
		TS_LOG_ERR("Not found ilitek chip\n");
		return -ENODEV;
	}
	return 0;
}

static int ilitek_tddi_chip_detect(struct ts_kit_platform_data *p_data)
{
	int ret = 0;

	if (err_alloc_mem(p_data) ||
		err_alloc_mem(p_data->ts_dev)) {
		TS_LOG_ERR("platform_data or ts_dev invaild\n");
		ret = -EINVAL;
		goto exit;
	}

	ilits->ts_dev_data->ts_platform_data = p_data;
	ilits->pdev = p_data->ts_dev;
	ilits->pdev->dev.of_node = ilits->ts_dev_data->cnode;

	ilits->ts_dev_data->is_i2c_one_byte = false;
	ilits->ts_dev_data->is_new_oem_structure = false;
	ilits->ts_dev_data->is_parade_solution = false;
	ret = ilitek_tddi_prase_ic_config_dts(ilits->pdev->dev.of_node);
	if (ret) {
		TS_LOG_ERR("parse ic config in dts failed\n");
		goto err_free;
	}
	ret = ilitek_bus_init();
	if (ret < 0) {
		TS_LOG_ERR("init bus failed\n");
	goto err_bus_free;
	}

	ret = ilitek_tddi_pinctrl_init();
	if (ret) {
		TS_LOG_ERR(" pinctrl operation error\n");
	goto err_chip_free;
	}
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ret = ilitek_pinctrl_select_default();
	if (ret)
		TS_LOG_ERR("have no pinctrl operation\n");

	ilits->tp_rst = p_data->reset_gpio;
#endif

	ret = ilitek_bus_communicate_check();
	if (ret) {
		TS_LOG_ERR("find ilitek device failed\n");
		goto err_chip_free;
	} else {
		strncpy(ilits->ts_dev_data->chip_name, TDDI_DEV_ID, MAX_STR_LEN);
	}
	init_completion(&ilits->roi_completion);

	TS_LOG_INFO("driver version = %s\n", DRIVER_VERSION);
	TS_LOG_INFO("chip detect succeeded\n");
	return 0;
err_chip_free:
	ilitek_pinctrl_release();
err_bus_free:
	ilitek_bus_release();
err_free:
	kfree(ilits->ts_dev_data);
	ilits->ts_dev_data = NULL;
	kfree(ilits);
	ilits = NULL;
exit:
	TS_LOG_ERR("chip detect failed\n");
	return ret;
}

int read_projectid_from_lcd(char *project_id)
{
	int retval;

	retval = hostprocessing_get_project_id(project_id);
	if (retval)
		TS_LOG_ERR("%s, fail get project_id\n", __func__);

	TS_LOG_INFO("kit_project_id=%s\n", project_id);
	strncpy(ilits->ts_dev_data->project_id, project_id,
		REAL_PROJECT_ID_LEN - 1);

	return retval;
}

static int ilitek_tddi_init_chip(void)
{
	int ret = 0;

	ret = read_projectid_from_lcd(ilits->project_id);
	if (ret)
		TS_LOG_ERR("read_projectid_from_lcd failed\n");
	ret = ilitek_parse_dts(ilits->ts_dev_data->cnode);
	if (ret)
		TS_LOG_ERR("parse config in dts failed\n");
	ret = ilitek_parse_ic_special_dts(ilits->pdev->dev.of_node);
	if (ret)
		TS_LOG_ERR("parse ic special config in dts failed\n");
	ilitek_tddi_wq_init();
	ilits->demo_debug_info[0] = ili_demo_debug_info_id0;
	ilits->tp_data_format = DATA_FORMAT_DEMO;
	ilits->boot = false;
	ilits->open_threshold_status = false;
	if (ili_ic_get_info() < 0)
		TS_LOG_ERR("Chip info is incorrect\n");

	ili_update_tp_module_info();

	ili_node_init();

	ili_fw_read_flash_info();

	return NO_ERR;
}
static int ilitek_tddi_get_brightness_info(void)
{
	int error = NO_ERR;

	return error;
}
static int ilitek_tddi_input_config(struct input_dev *input_dev)
{
	u16 x_min = 0, x_max = 0, y_min = 0, y_max = 0, finger_nums = 0;

	ilits->input = input_dev;

	TS_LOG_INFO("input_config:max_x = %d, max_y = %d\n", ilits->max_x, ilits->max_y);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(BTN_TOOL_FINGER, input_dev->keybit);
	set_bit(TS_PALM_COVERED, input_dev->keybit);
	set_bit(TS_DOUBLE_CLICK, input_dev->keybit);
	set_bit(TS_LETTER_C, input_dev->keybit);
	set_bit(TS_LETTER_E, input_dev->keybit);
	set_bit(TS_LETTER_M, input_dev->keybit);
	set_bit(TS_LETTER_W, input_dev->keybit);
	set_bit(TS_KEY_IRON, input_dev->keybit);

#ifdef INPUT_PROP_DIRECT
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
#endif

	input_set_abs_params(input_dev, ABS_X,
		0, ilits->max_x - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_Y,
		0, ilits->max_y - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
		ilits->max_x - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
		ilits->max_y - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, ABS_MT_TOUCH_MAJOR_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0, ABS_MT_TOUCH_MINOR_MAX, 0, 0);

#ifdef TYPE_B_PROTOCOL
#ifdef KERNEL_ABOVE_3_7
	/* input_mt_init_slots now has a "flags" parameter */
	input_mt_init_slots(input_dev, 10, INPUT_MT_DIRECT);
#else
	input_mt_init_slots(input_dev, 10);
#endif
#endif

	return 0;
}
static int ilitek_tddi_irq_top_half(struct ts_cmd_node *cmd)
{
	cmd->command = TS_INT_PROCESS;
	return 0;
}
int ilitek_tddi_irq_bottom_half(struct ts_cmd_node *in_cmd, struct ts_cmd_node *out_cmd)
{
	int ret = 0;
	struct ts_fingers *info = NULL;

	if (atomic_read(&ilits->cmd_int_check) == ENABLE) {
		atomic_set(&ilits->cmd_int_check, DISABLE);
		TS_LOG_DEBUG("CMD INT detected, ignore\n");
		wake_up(&(ilits->inq));
		ret = -EINVAL;
		goto out;
	}
	if (ilits->prox_near) {
		TS_LOG_INFO("Proximity event, ignore interrupt!\n");
		ret = -EINVAL;
		goto out;
	}
	TS_LOG_DEBUG("report: %d, rst: %d, fw: %d, switch: %d, mp: %d, sleep: %d, esd: %d\n",
			ilits->report,
			atomic_read(&ilits->tp_reset),
			atomic_read(&ilits->fw_stat),
			atomic_read(&ilits->tp_sw_mode),
			atomic_read(&ilits->mp_stat),
			atomic_read(&ilits->tp_sleep),
			atomic_read(&ilits->esd_stat));

	if (!ilits->report || atomic_read(&ilits->tp_reset) ||
		atomic_read(&ilits->fw_stat) || atomic_read(&ilits->tp_sw_mode) ||
		atomic_read(&ilits->mp_stat) || atomic_read(&ilits->tp_sleep) ||
		atomic_read(&ilits->esd_stat)) {
		TS_LOG_DEBUG("ignore interrupt !\n");
		ret = -EINVAL;
		goto out;
	}
	if (mutex_is_locked(&ilits->touch_mutex)) {
		TS_LOG_DEBUG("touch is locked, ignore\n");
		out_cmd->command = TS_INVAILD_CMD;
		ret = -EINVAL;
		goto out;
	}

	info = &out_cmd->cmd_param.pub_params.algo_param.info;
	out_cmd->command = TS_INPUT_ALGO;
	out_cmd->cmd_param.pub_params.algo_param.algo_order =
			ilits->ts_dev_data->algo_id;
	mutex_lock(&ilits->touch_mutex);
	ret = ilitek_tddi_read_report_data(info);
	mutex_unlock(&ilits->touch_mutex);

out:
	if (ret)
		out_cmd->command = TS_INVAILD_CMD;
	return ret;
}
int ilitek_tddi_fw_update_boot(char *file_name)
{
	int ret = 0;

	/* 1. get firmware name */
	ret = ilitek_tddi_upgrade_get_fw_name();
	if (ret) {
		TS_LOG_ERR("get firmware name failed");
		return ret;
	}
	mutex_lock(&ilits->touch_mutex);
	ret = ili_fw_upgrade_handler(NULL);
	mutex_unlock(&ilits->touch_mutex);
	if (ret < 0) {
		TS_LOG_ERR("fw upgrade fail\n");
		return ret;
	}
	return 0;
}

int ilitek_tddi_fw_update_sd(void)
{
	int ret = 0;

	snprintf(ilits->fw_name, ILITEK_FILE_NAME_LEN, "%s",ILITEK_UPGRADE_FW_SD_NAME);

	TS_LOG_INFO("firmware file name = %s", ilits->fw_name);
	mutex_lock(&ilits->touch_mutex);
	ilits->force_fw_update = ENABLE;
	ret = ili_fw_upgrade_handler(NULL);
	ilits->force_fw_update = DISABLE;
	mutex_unlock(&ilits->touch_mutex);
	if (ret < 0) {
		TS_LOG_ERR("fw upgrade fail\n");
		return ret;
	}
	return 0;
}

static int ilitek_tddi_chip_get_info(struct ts_chip_info_param *info)
{
	struct ts_kit_device_data *p_dev_data = ilits->ts_dev_data;

	if (IS_ERR_OR_NULL(info)) {
		TS_LOG_ERR("info invaild\n");
		return -EINVAL;
	}

	if (p_dev_data->ts_platform_data->hide_plain_id)
		sprintf(info->ic_vendor, "%s", ilits->project_id);
	else
		sprintf(info->ic_vendor, "%s%x-%s", TDDI_DEV_ID,
			ilits->chip->pid, ilits->project_id);

	sprintf(info->mod_vendor, "%s", ilits->ts_dev_data->module_name);
	sprintf(info->fw_vendor, "0x%x", ilits->chip->fw_ver);

	return 0;
}

static int ilitek_tddi_chip_get_capacitance_test_type(struct ts_test_type_info *info)
{
	struct ts_kit_device_data *ts_dev_data = ilits->ts_dev_data;

	if (IS_ERR_OR_NULL(ts_dev_data) ||
		IS_ERR_OR_NULL(info)) {
		TS_LOG_ERR("ts_dev_data or info invaild\n");
		return -EINVAL;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		memcpy(info->tp_test_type, ts_dev_data->tp_test_type, TS_CAP_TEST_TYPE_LEN);
		TS_LOG_INFO("test_type = %s\n", info->tp_test_type);
		break;
	case TS_ACTION_WRITE:
		break;
	default:
		TS_LOG_ERR("invalid op action: %d\n", info->op_action);
		return -EINVAL;
	}

	return 0;
}

static int ilitek_tddi_set_info_flag(struct ts_kit_platform_data *info)
{
	return NO_ERR;
}
static int ilitek_tddi_suspend(void)
{
	struct ts_kit_device_data *ts_dev_data = ilits->ts_dev_data;
	struct ts_kit_platform_data *ts_platform_data = ts_dev_data->ts_platform_data;
	struct ts_easy_wakeup_info *info = &ts_dev_data->easy_wakeup_info;



	switch (ts_dev_data->easy_wakeup_info.sleep_mode) {
	case TS_POWER_OFF_MODE:
		ilits->gesture = false;
		if (ili_sleep_handler(TP_DEEP_SLEEP) < 0)
			TS_LOG_ERR("TP sleep failed\n");
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
		ilitek_pinctrl_select_idle();
#endif
		break;
	case TS_GESTURE_MODE:
		ilits->gesture = true;
		if (ili_sleep_handler(TP_SUSPEND) < 0)
			TS_LOG_ERR("TP suspend failed\n");
			mutex_lock(&ilits->wrong_touch_lock);
			ts_dev_data->easy_wakeup_info.off_motion_on = true;
			mutex_unlock(&ilits->wrong_touch_lock);
		info->easy_wakeup_flag = true;
		break;
	default:
		if (ili_sleep_handler(TP_DEEP_SLEEP) < 0)
			TS_LOG_ERR("TP sleep failed\n");
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
		ilitek_pinctrl_select_idle();
#endif
		break;
	}
	if (ilits->gesture == false)
		gpio_direction_output(ts_platform_data->cs_gpio, 0);
	return 0;
}


static int ilitek_tddi_before_suspend(void)
{
	TS_LOG_INFO("before suspend start\n");
	return 0;

}

/* lcdkit do tp and lcd hw reset before tp resume,
 * so don't send cmd(sleep out, sens start) in resume
 * or do soft reset in after_resume
 */
static int ilitek_tddi_resume(void)
{
	struct ts_kit_device_data *ts_dev_data = ilits->ts_dev_data;
	struct ts_kit_platform_data *ts_platform_data = ts_dev_data->ts_platform_data;
	struct ts_easy_wakeup_info *info = &ts_dev_data->easy_wakeup_info;

	TS_LOG_INFO("resume start\n");

	if (ilits->gesture == false)
		gpio_direction_output(ts_platform_data->cs_gpio, 1);
	switch (ts_dev_data->easy_wakeup_info.sleep_mode) {
	case TS_POWER_OFF_MODE:
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
		ilitek_pinctrl_select_default();
#endif
	if (ili_sleep_handler(TP_RESUME) < 0)
		TS_LOG_ERR("TP resume failed\n");
		break;
	case TS_GESTURE_MODE:
		if (true == ts_platform_data->feature_info.wakeup_gesture_enable_info.switch_value) {
			if (ili_sleep_handler(TP_RESUME) < 0)
				TS_LOG_ERR("TP resume failed\n");
			info->easy_wakeup_flag = false;
		}
		break;
	default:
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
		ilitek_pinctrl_select_default();
#endif
	if (ili_sleep_handler(TP_RESUME) < 0)
		TS_LOG_ERR("TP resume failed\n");
		break;
	}

	TS_LOG_INFO("resume end\n");

	return 0;
}

/*  do some thing after power on. */
static int ilitek_tddi_after_resume(void *feature_info)
{
	TS_LOG_INFO("after resume start\n");
	return 0;
}

static int ilitek_tddi_wakeup_gesture_enable_switch(
	struct ts_wakeup_gesture_enable_info *info)
{
	return 0;
}
static int ilitek_tddi_get_raw_data(struct ts_rawdata_info *info, struct ts_cmd_node *out_cmd)
{
	unsigned long timer_start = 0, timer_end = 0;
	u8 result_str[TS_RAWDATA_RESULT_MAX] = {0};
	bool esd_en = ilits->wq_esd_ctrl, bat_en = ilits->wq_bat_ctrl;
	int ret = 0;

	timer_start = jiffies;
	mutex_lock(&ilits->touch_mutex);

	/* Create the directory for mp_test result */
	if ((dev_mkdir(CSV_LCM_ON_PATH, S_IRUGO | S_IWUSR)) != 0)
		TS_LOG_ERR("Failed to create directory for mp_test\n");

	if (esd_en)
		ili_wq_ctrl(WQ_ESD, DISABLE);
	if (bat_en)
		ili_wq_ctrl(WQ_BAT, DISABLE);

	TS_LOG_INFO("mp test start\n");
	memset(result_str, 0, sizeof(result_str));
	ilits->offset = 2;
	ret = ili_mp_test_handler(info, result_str, ON);
	TS_LOG_INFO("MP TEST %s, Error code = %d\n", (ret < 0) ? "FAIL" : "PASS", ret);
	timer_end = jiffies;
	TS_LOG_INFO("mp test result: %s\n", info->result);
	TS_LOG_INFO("mp test end, use time: %dms\n", jiffies_to_msecs(timer_end - timer_start));
	mutex_unlock(&ilits->touch_mutex);
	return 0;
}

static int ilitek_tddi_get_debug_data(struct ts_diff_data_info *info,
	struct ts_cmd_node *out_cmd)
{
	return 0;
}
static int ilitek_tddi_glove_switch(struct ts_glove_info *info)
{
	int ret = 0;

	if (!info) {
		TS_LOG_ERR("%s:info is null\n", __func__);
		ret = -ENOMEM;
		return ret;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:

		break;
	case TS_ACTION_WRITE:

		break;
	default:
		break;
	}

	return 0;
}
static void ilitek_tddi_shutdown(void)
{

}
static int ilitek_tddi_holster_switch(struct ts_holster_info  *info)
{
	return 0;
}

int ili_ic_get_roi_staus(void)
{
	int ret = 0;
	u8 cmd[3] = {0};
	u8 buf[10] = {0};
	u32 status = 0;
	u8 read_len = P5_X_READ_ROI_STATUS_LEN;

	cmd[0] = P5_X_READ_ROI_CTRL;
	cmd[1] = P5_X_ROI_CMD;
	cmd[2] = P5_X_READ_ROI_STUAUS;
	if (ilits->wrapper(cmd, sizeof(cmd), NULL, 0, OFF, OFF) < 0) {
		TS_LOG_INFO("Write protocol ver pre cmd failed\n");
		ret = -EINVAL;
		goto out;
	}
	msleep(1);
	if (ilits->wrapper(NULL, 0, buf, read_len, OFF, OFF)) {
		TS_LOG_ERR("Write fw version cmd failed\n");
		ret = -EINVAL;
		goto out;
	}
	TS_LOG_INFO("buf = %d,%d,%d\n", buf[0], buf[1], buf[2]);

	if (buf[0] != P5_X_ROI_CMD) {
		TS_LOG_INFO("Invalid ROI status\n");
		ret = -EINVAL;
		goto out;
	}
	status = buf[2];
	return status;
out:

	return ret;
}

static int ilitek_get_roi_switch(u8 *roi_switch)
{
	if (!roi_switch)
		return NULL;

	*roi_switch = ili_ic_get_roi_staus();

	TS_LOG_INFO("get roi status = %d\n", *roi_switch);

	return 0;
}
static int ilitek_set_roi_switch(bool roi_enable)
{
	int ret = 0;

	ret = ili_ic_func_ctrl("roi_set", roi_enable ? CMD_ENABLE : CMD_DISABLE);
	if (ret) {
		TS_LOG_INFO("roi control failed, enable = %d, ret = %d\n", roi_enable, ret);
		return ret;
	}

	TS_LOG_INFO("set roi status = %d\n", roi_enable);

	return 0;
}

static int ilitek_tddi_roi_switch(struct ts_roi_info *info)
{
	int i = 0;
	int ret = 0;

	if (IS_ERR_OR_NULL(info)) {
		TS_LOG_ERR("info invaild\n");
		return -EINVAL;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		ret = ilitek_get_roi_switch(&info->roi_switch);
		if (ret) {
			TS_LOG_INFO("get roi switch failed, ret = %d\n", ret);
			return ret;
		}
		break;
	case TS_ACTION_WRITE:
		ret = ilitek_set_roi_switch(!!info->roi_switch);
		if (ret) {
			TS_LOG_INFO("set roi switch failed, ret = %d\n", ret);
			return ret;
		}
		ilits->roi_switch_backup = info->roi_switch;
		if (!info->roi_switch) {
			for (i = 0; i < ROI_DATA_READ_LENGTH; i++)
				ilits->roi_data[i] = 0;
		}
		break;
	default:
		break;
	}

	return 0;
}


static struct ts_roi_info *ilitek_get_roi_info(
	struct ts_kit_device_data *dev_data)
{
	return &dev_data->ts_platform_data->feature_info.roi_info;
}

static unsigned char *chip_roi_rawdata(void)
{
	unsigned char *rawdata_ptr = NULL;
	struct ts_roi_info *roi_info = NULL;

	roi_info = ilitek_get_roi_info(ilits->ts_dev_data);
	if (!ilits->ts_dev_data->ts_platform_data->feature_info.roi_info.roi_supported || !ilits->roi_data || !(roi_info->roi_switch)) {
		TS_LOG_ERR("%s : Not supported roi mode or rawadta is null.\n", __func__);
		return NULL;
	}

	if (wait_for_completion_interruptible_timeout(&ilits->roi_completion, msecs_to_jiffies(30))) {
		mutex_lock(&ilits->roi_mutex);
		if (ilits->roi_data_ready) {
			rawdata_ptr = (unsigned char *)ilits->roi_data;
		} else {
			TS_LOG_ERR("%s : roi is not enable or roi data not ready\n", __func__);
		}
		mutex_unlock(&ilits->roi_mutex);
	} else {
		/*
		 * Never wait again if data
		 * refreshing gets timeout.
		 */
		memset(ilits->roi_data, 0,
				sizeof(ilits->roi_data));
		TS_LOG_ERR("%s : wait roi_data_done timeout!\n", __func__);
	}

	return rawdata_ptr;
}

void input_kit_read_roi_data(void)
{
	if (ilits->ts_dev_data->ts_platform_data->feature_info.roi_info.roi_supported) {

		reinit_completion(&ilits->roi_completion);
		ilitek_read_roi_data();
	}
}

static int ilitek_tddi_palm_switch(struct ts_palm_info *info)
{
	return 0;
}

static int ilitek_tddi_regs_operate(struct ts_regs_info *info)
{
	return 0;
}

static int ilitek_tddi_calibrate(void)
{
	return 0;
}

static int ilitek_tddi_calibrate_wakeup_gesture(void)
{
	return 0;
}

static int ilitek_tddi_esdcheck_func(void)
{
	return 0;
}

#if defined(HUAWEI_CHARGER_FB)
static int ilitek_charger_switch(struct ts_charger_info *info)
{
	return 0;
}
#endif

static int ilitek_tddi_reset_device(void)
{
	int ret = 0;

	mutex_lock(&ilits->touch_mutex);
	ret = ili_fw_upgrade_handler(NULL);
	mutex_unlock(&ilits->touch_mutex);
	if (ret < 0) {
		TS_LOG_ERR("fw upgrade fail\n");
		return ret;
	}
	return 0;
}
#if defined(HUAWEI_CHARGER_FB)
static int ilitek_tddi_charger_switch(struct ts_charger_info *info)
{

	if (info == NULL) {
		TS_LOG_ERR("pointer info is NULL\n");
		return -ENOMEM;
	}
	TS_LOG_INFO("tddi_charger_switch called! info->charger_switch=%d, charger_switch_addr=0x%x\n",
			info->charger_switch, info->charger_switch_addr);
	if (info->charger_switch == USB_CHARGER_OUT) { /*usb plug out*/

	} else if (info->charger_switch == USB_CHARGER_IN) { /*usb plug in*/

	} else {
		TS_LOG_ERR("unknown USB status, info->charger_switch=%d\n",
			info->charger_switch);
	}
	return NO_ERR;
}
#endif

static int ilitek_tddi_wrong_touch(void)
{
	struct ts_kit_device_data *ts_dev_data = ilits->ts_dev_data;

	mutex_lock(&ilits->wrong_touch_lock);
	ts_dev_data->easy_wakeup_info.off_motion_on  = true;
	mutex_unlock(&ilits->wrong_touch_lock);

	TS_LOG_INFO("wrong touch switch done\n");

	return 0;
}

static void game_switch(bool enable)
{
	ilits->game_switch = enable;
	mutex_lock(&ilits->touch_mutex);
	if (ilits->game_switch)
		ili_ic_func_ctrl("lock_point", DISABLE);
	else
		ili_ic_func_ctrl("lock_point", ENABLE);
	mutex_unlock(&ilits->touch_mutex);
}

static void soper_switch(unsigned int soper)
{
	switch (soper) {
	case TS_SWITCH_GAME_ENABLE:
		TS_LOG_INFO("%s: enter game_mode\n", __func__);
		game_switch(1);
		break;
	case TS_SWITCH_GAME_DISABLE:
		TS_LOG_INFO("%s: exit game_mode\n", __func__);
		game_switch(0);
		break;
	default:
		TS_LOG_ERR("%s: soper unknown:%d, invalid\n",
			__func__, soper);
		break;
	}
}

static void chip_tddi_chip_touch_switch(void)
{
	char in_data[MAX_STR_LEN] = {0};
	unsigned int stype;
	unsigned int soper;
	unsigned int time;
	unsigned int i;
	unsigned int cnt = 0;
	struct ts_kit_device_data *ts_dev_data = ilits->ts_dev_data;
	struct ts_kit_platform_data *ts_platform_data = NULL;

	TS_LOG_INFO("%s enter\n", __func__);
	if (!ts_dev_data) {
		TS_LOG_ERR("chip_touch_switch, error chip data\n");
		goto out;
	}
	ts_platform_data = ts_dev_data->ts_platform_data;

	memcpy(in_data, ts_dev_data->touch_switch_info, MAX_STR_LEN - 1);
	for (i = 0; (i < strlen(in_data)) && (in_data[i] != '\n'); i++) {
		if (in_data[i] == ',') {
			cnt++;
		} else if (!isdigit(in_data[i])) {
			TS_LOG_ERR("%s: input format error!!\n", __func__);
			goto out;
		}
	}
	if (cnt != ILITEK_DOZE_MAX_INPUT_SEPARATE_NUM) {
		TS_LOG_ERR("%s: [separation_cnt=%d]!!\n", __func__, cnt);
		goto out;
	}

	if (sscanf(in_data, "%u,%u,%u", &stype, &soper, &time) <= 0) {
		TS_LOG_ERR("%s: sscanf error\n", __func__);
		goto out;
	}

	TS_LOG_INFO("stype=%u,soper=%u,time=%u\n", stype, soper, time);

	switch (stype) {
	case TS_SWITCH_TYPE_GAME:
		if (TS_SWITCH_TYPE_GAME !=
			(ts_dev_data->touch_switch_flag & TS_SWITCH_TYPE_GAME)) {
			TS_LOG_ERR("%s, game mode not support\n", __func__);
			goto out;
		}
		break;
	case TS_SWITCH_SCENE_5:
		if (TS_SWITCH_TYPE_SCENE !=
			(ilits->ts_dev_data->touch_switch_flag & TS_SWITCH_TYPE_SCENE)) {
			TS_LOG_ERR("%s, scene mode not support\n", __func__);
			goto out;
		}
		soper_switch(soper);
		break;
	default:
		TS_LOG_ERR("%s: stype unknown:%u, invalid\n",
			__func__, stype);
		break;
	}

out:
	return;
}
struct ts_device_ops ilitek_tddi_ops = {
	.chip_detect = ilitek_tddi_chip_detect,
	.chip_init = ilitek_tddi_init_chip,
	.chip_get_brightness_info = ilitek_tddi_get_brightness_info,
	.chip_input_config = ilitek_tddi_input_config,
	.chip_irq_top_half = ilitek_tddi_irq_top_half,
	.chip_irq_bottom_half = ilitek_tddi_irq_bottom_half,
	.chip_fw_update_boot = ilitek_tddi_fw_update_boot,
	.chip_fw_update_sd = ilitek_tddi_fw_update_sd,
	.chip_get_info = ilitek_tddi_chip_get_info,
	.chip_get_capacitance_test_type = ilitek_tddi_chip_get_capacitance_test_type,
	.chip_set_info_flag = ilitek_tddi_set_info_flag,
	.chip_before_suspend = ilitek_tddi_before_suspend,
	.chip_suspend = ilitek_tddi_suspend,
	.chip_resume = ilitek_tddi_resume,
	.chip_after_resume = ilitek_tddi_after_resume,
	.chip_wakeup_gesture_enable_switch = ilitek_tddi_wakeup_gesture_enable_switch,
	.chip_get_rawdata = ilitek_tddi_get_raw_data,
	.chip_special_rawdata_proc_printf = ilitek_tddi_rawdata_print,
	.chip_get_debug_data = ilitek_tddi_get_debug_data,
	.chip_glove_switch = ilitek_tddi_glove_switch,
	.chip_shutdown = ilitek_tddi_shutdown,
	.chip_holster_switch = ilitek_tddi_holster_switch,
	.chip_roi_switch = ilitek_tddi_roi_switch,
	.chip_roi_rawdata = chip_roi_rawdata,
	.chip_palm_switch = ilitek_tddi_palm_switch,
	.chip_regs_operate = ilitek_tddi_regs_operate,
	.chip_calibrate = ilitek_tddi_calibrate,
	.chip_calibrate_wakeup_gesture = ilitek_tddi_calibrate_wakeup_gesture,
	.chip_reset = ilitek_tddi_reset_device,
	.chip_check_status = ilitek_tddi_esdcheck_func,
	.chip_charger_switch = ilitek_tddi_charger_switch,
	.chip_wrong_touch = ilitek_tddi_wrong_touch,
	.chip_touch_switch = chip_tddi_chip_touch_switch,
};
static int __init ilitek_tddi_module_init(void)
{
	int ret = 0;
	bool found = false;
	struct device_node *child = NULL;
	struct device_node *root = NULL;

	TS_LOG_INFO("chip_module_init: called\n");
	ilits = kzalloc(sizeof(struct ilitek_ts_data), GFP_KERNEL);
	if (err_alloc_mem(ilits)) {
		TS_LOG_ERR("%s: Failed to allocate ilits memory\n", __func__);
		ret = -ENOMEM;
		goto err_out;
	}
	ilits->ts_dev_data = kzalloc(sizeof(*ilits->ts_dev_data), GFP_KERNEL);
	if (err_alloc_mem(ilits->ts_dev_data)) {
		TS_LOG_ERR("%s: alloc ts_kit_device_data failed\n", __func__);
		ret =  -ENOMEM;
		goto err_ts_free;
	}
	root = of_find_compatible_node(NULL, NULL, ILITEK_DTS_HW_TS_NODE);
	if (err_alloc_mem(root)) {
		TS_LOG_ERR("%s: find_compatible_node failed\n", __func__);
		ret = -EINVAL;
		goto err_dev_free;
	}

	for_each_child_of_node(root, child) {
		if (of_device_is_compatible(child, ILITEK_DTS_NODE)) {
			found = true;
			break;
		}
	}

	if (!found) {
		TS_LOG_ERR("%s: device tree node not found, name = %s\n",
			__func__, ILITEK_DTS_NODE);
		ret = -EINVAL;
		goto err_dev_free;
	}
	ilits->ts_dev_data->cnode = child;
	ilits->ts_dev_data->ops = &ilitek_tddi_ops;
	ilits->ts_dev_data->is_in_cell = 1;
	TS_LOG_INFO("found child node !\n");

	ret = huawei_ts_chip_register(ilits->ts_dev_data);
	if (ret) {
		TS_LOG_ERR("%s: chip register fail, ret = %d\n", __func__,ret);
		goto err_dev_free;
	}

	TS_LOG_INFO("module init succeeded\n");

	return 0;

err_dev_free:
	kfree(ilits->ts_dev_data);
	ilits->ts_dev_data = NULL;
err_ts_free:
	kfree(ilits);
	ilits = NULL;
err_out:
	TS_LOG_ERR("%s: module init failed\n", __func__);
	return ret;
}
static void __exit ilitek_tddi_module_exit(void)
{
	ili_dev_remove();
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	ilitek_pinctrl_select_default();
#endif
	ilitek_pinctrl_release();

	kfree(ilits->ts_dev_data);
	ilits->ts_dev_data = NULL;
	kfree(ilits);
	ilits = NULL;

	TS_LOG_INFO("module exit succeeded\n");
}
module_init(ilitek_tddi_module_init);
module_exit(ilitek_tddi_module_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_LICENSE("GPL");
