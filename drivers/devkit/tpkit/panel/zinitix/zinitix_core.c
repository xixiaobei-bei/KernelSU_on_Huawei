/*
 * zinitix_core.c
 *
 * zinitix driver core
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "zinitix_core.h"
#include "zinitix_dts.h"
#include <linux/printk.h>
#include "huawei_ts_kit.h"
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif
#include "securec.h"
#include <huawei_platform/log/hwlog_kernel.h>
#include "ext_sensorhub_api.h"

#define TP_RESET_COMMAND_ID 0x98
#define AP_MCU_SERVICE_ID 0x01
#define ERROR_RESET 10

#ifndef NULL
#define NULL 0
#endif

/* global values */
struct zinitix_platform_data *g_zinitix_pdata;
struct ts_kit_device_data *g_zinitix_dev_data;
struct zinitix_test_params *g_zinitix_params;
int g_zinitix_err_count = 0;

/* function declarations */
static int zinitix_init_chip(void);
static int zinitix_before_suspend(void);
static int zinitix_suspend(void);
static int zinitix_resume(void);
static void zinitix_shutdown(void);
static int zinitix_hardware_reset(void);
static int zinitix_calibrate(void);
#if defined(HUAWEI_CHARGER_FB)
static int zinitix_charger_switch(struct ts_charger_info *info);
#endif
static int zinitix_input_config(struct input_dev *input_dev);
static int zinitix_regs_operate(struct ts_regs_info *info);
static int zinitix_chip_detect(struct ts_kit_platform_data *data);
static int zinitix_irq_top_half(struct ts_cmd_node *cmd);
static int zinitix_get_brightness_info(void);
static int zinitix_fw_update_boot(char *file_name);
static int zinitix_fw_update_sd(void);
static int zinitix_chip_get_info(struct ts_chip_info_param *info);
static int zinitix_set_info_flag(struct ts_kit_platform_data *info);
static int zinitix_after_resume(void *feature_info);
static int zinitix_irq_bottom_half(struct ts_cmd_node *in_cmd,
		struct ts_cmd_node *out_cmd);
static int zinitix_power_on(void);
static void zinitix_power_off(void);
static int zinitix_power_resume(void);
static int zinitix_get_raw_data(struct ts_rawdata_info *info,
		struct ts_cmd_node *out_cmd);
static int zinitix_get_debug_data(struct ts_diff_data_info *info,
		struct ts_cmd_node *out_cmd);
static int zinitix_holster_switch(struct ts_holster_info *info);
static void zinitix_chip_touch_switch(void);
static int zinitix_get_capacitance_test_type(struct ts_test_type_info *info);
static int zinitix_roi_switch(struct ts_roi_info *info);
static u8 *zinitix_roi_rawdata(void);
static int zinitix_palm_switch(struct ts_palm_info *info);
static int zinitix_calibrate_wakeup_gesture(void);
static int zinitix_wrong_touch(void);
static int zinitix_status_check(void);
static void zinitix_clear_globals(void);

struct ts_device_ops ts_zinitix_ops = {
	.chip_detect = zinitix_chip_detect,
	.chip_init = zinitix_init_chip,
	.chip_get_brightness_info = zinitix_get_brightness_info,
	.chip_input_config = zinitix_input_config,
	.chip_irq_top_half = zinitix_irq_top_half,
	.chip_irq_bottom_half = zinitix_irq_bottom_half,
	.chip_fw_update_boot = zinitix_fw_update_boot,
	.chip_fw_update_sd = zinitix_fw_update_sd,
	.chip_get_info = zinitix_chip_get_info,
	.chip_get_capacitance_test_type = zinitix_get_capacitance_test_type,
	.chip_set_info_flag = zinitix_set_info_flag,
	.chip_before_suspend = zinitix_before_suspend,
	.chip_suspend = zinitix_suspend,
	.chip_resume = zinitix_resume,
	.chip_after_resume = zinitix_after_resume,
	.chip_get_rawdata = zinitix_get_raw_data,
	.chip_get_debug_data = zinitix_get_debug_data,
	.chip_shutdown = zinitix_shutdown,
	.chip_holster_switch = zinitix_holster_switch,
	.chip_roi_switch = zinitix_roi_switch,
	.chip_roi_rawdata = zinitix_roi_rawdata,
	.chip_palm_switch = zinitix_palm_switch,
	.chip_regs_operate = zinitix_regs_operate,
	.chip_calibrate_wakeup_gesture = zinitix_calibrate_wakeup_gesture,
	.chip_check_status = zinitix_status_check,
#if defined(HUAWEI_CHARGER_FB)
	.chip_charger_switch = zinitix_charger_switch,
#endif
#ifdef HUAWEI_TOUCHSCREEN_TEST
	.chip_test = test_dbg_cmd_test,
#endif
	.chip_wrong_touch = zinitix_wrong_touch,
	.chip_touch_switch = zinitix_chip_touch_switch,
	.chip_clear_memory = zinitix_clear_globals,
};

inline struct ts_kit_device_data *zinitix_get_device_data(void)
{
	return g_zinitix_dev_data;
}

inline struct zinitix_platform_data *zinitix_get_platform_data(void)
{
	return g_zinitix_pdata;
}

void zinitix_clear_globals(void)
{
	kfree(g_zinitix_pdata);
	g_zinitix_pdata = NULL;
	kfree(g_zinitix_dev_data);
	g_zinitix_dev_data = NULL;
}

int zinitix_read(u16 *addrs, u16 addr_size, u8 *values, u16 values_size)
{
	int ret;
	struct ts_bus_info *bops = NULL;

	if (values == NULL) {
		TS_LOG_ERR("%s:fail, null pointer\n", __func__);
		return -EINVAL;
	}

	bops = g_zinitix_dev_data->ts_platform_data->bops;

	ret = bops->bus_read((u8 *)addrs, addr_size, values, values_size);
	if (ret) {
		if (addrs != NULL) {
			TS_LOG_ERR("%s:fail, addrs:0x%x\n", __func__,
					*(u16 *)addrs);
		} else {
			TS_LOG_ERR("%s:fail\n", __func__);
		}
	}

	return ret;
}

int zinitix_write(u16 *values, u16 values_size)
{
	int ret;
	struct ts_bus_info *bops = NULL;

	if (values == NULL) {
		TS_LOG_ERR("%s:fail, null pointer\n", __func__);
		return -EINVAL;
	}

	bops = g_zinitix_dev_data->ts_platform_data->bops;
	ret = bops->bus_write((u8 *)values, values_size);
	if (ret)
		TS_LOG_ERR("%s:fail, addrs:0x%x\n", __func__, *(u16 *)values);

	return ret;
}

int ext_reset_command(void)
{
	int ret;
	struct command send_tp_reset = {0};
	unsigned char buff = 0;
	send_tp_reset.service_id = AP_MCU_SERVICE_ID;
	send_tp_reset.command_id = TP_RESET_COMMAND_ID;
	send_tp_reset.send_buffer = &buff;
	send_tp_reset.send_buffer_len = 1;

	if (!gpio_get_value(g_zinitix_pdata->i2c_switch)) {
		TS_LOG_INFO("%s:i2c not at AP!\n", __func__);
		return -EINVAL;
	}

	ret = send_command(UPGRADE_CHANNEL, &send_tp_reset, false, NULL);
	TS_LOG_INFO("%s: send_command ret %d\n", __func__, ret);
	if (ret < 0)
		TS_LOG_INFO("%s: send reset cmd failed\n", __func__);

	return ret;
}

void zinitix_set_rate()
{
	u16 cmd[2] = { 0, 0 }; // cmd len is 2
	int ret;

	cmd[0] = 0x0186; // rate reg
	cmd[1] = 120; // rate
	ret = zinitix_write(&cmd[0], sizeof(cmd));
	if (ret < 0)
		TS_LOG_ERR("%s:set rate fail, ret=%d\n", __func__, ret);
}

int zinitix_gpio_reset(void)
{
	int ret;
	int reset_gpio;

	if (g_zinitix_dev_data == NULL) {
		TS_LOG_ERR("%s:fail, null pointer\n", __func__);
		return -EINVAL;
	}
#ifdef TARGET_HUAWEI_WEARABLES
	return ext_reset_command();
#endif
	reset_gpio = g_zinitix_dev_data->ts_platform_data->reset_gpio;

	ret = gpio_direction_output(reset_gpio, ZINITIX_GPIO_HIGH);
	if (ret) {
		TS_LOG_ERR("%s:gpio direction output to 1 fail, ret=%d\n",
				__func__, ret);
		return ret;
	}
	msleep(ZINITIX_GPIO_RESET_HIGH_DELAY);
	ret = gpio_get_value(g_zinitix_dev_data->ts_platform_data->reset_gpio);
	TS_LOG_INFO("%s: 2 reset gpio_get_value %d\n", __func__, ret);

	ret = gpio_direction_output(reset_gpio, ZINITIX_GPIO_LOW);
	if (ret) {
		TS_LOG_ERR("%s:gpio direction output to 0 fail, ret=%d\n",
				__func__, ret);
		return ret;
	}
	msleep(ZINITIX_GPIO_RESET_LOW_DELAY);
	ret = gpio_get_value(g_zinitix_dev_data->ts_platform_data->reset_gpio);
	TS_LOG_INFO("%s: reset gpio_get_value %d\n", __func__, ret);

	ret = gpio_direction_output(reset_gpio, ZINITIX_GPIO_HIGH);
	if (ret) {
		TS_LOG_ERR("%s:gpio direction output to 1 fail, ret=%d\n",
				__func__, ret);
		return ret;
	}
	msleep(ZINITIX_GPIO_RESET_RELEASE_DELAY);

	ret = gpio_get_value(g_zinitix_dev_data->ts_platform_data->reset_gpio);
	TS_LOG_INFO("%s: reset gpio_get_value %d\n", __func__, ret);
	return 0;
}

static int zinitix_gpio_request(void)
{
	int ret = 0;

	TS_LOG_INFO("%s:enter\n", __func__);
	if ((g_zinitix_dev_data == NULL) || (g_zinitix_pdata == NULL)) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return -EINVAL;
	}

	if (g_zinitix_pdata->self_ctrl_power != ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s:power controlled by LCD\n", __func__);
		return 0;
	}

	if (g_zinitix_dev_data->vci_gpio_type == ZINITIX_GPIO_POWER) {
		ret = gpio_request(g_zinitix_dev_data->vci_gpio_ctrl,
				"zinitix_vci_gpio_ctrl");
		if (ret) {
			TS_LOG_ERR("%s:unable to request vci_gpio_ctrl!\n",
					__func__);
			return ret;
		}
		TS_LOG_INFO("%s:vci_gpio_ctrl request succ\n", __func__);
	}

	if (g_zinitix_dev_data->vddio_gpio_type == ZINITIX_GPIO_POWER) {
		ret = gpio_request(g_zinitix_dev_data->vddio_gpio_ctrl,
				"zinitix_vddio_gpio_ctrl");
		if (ret) {
			TS_LOG_ERR("%s:unable to request vddio_gpio_ctrl!\n",
					__func__);
			goto ts_vddio_out;
		}
		TS_LOG_INFO("%s:vddio_gpio_ctrl request succ\n", __func__);
	}
	return 0;
ts_vddio_out:
	if (g_zinitix_dev_data->vci_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vci_gpio_ctrl))
			gpio_free(g_zinitix_dev_data->vci_gpio_ctrl);
	}

	return ret;
}

static void zinitix_gpio_free(void)
{
	TS_LOG_INFO("%s:enter\n", __func__);
	if ((g_zinitix_dev_data == NULL) || (g_zinitix_pdata == NULL)) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return;
	}

	if (g_zinitix_pdata->self_ctrl_power != ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s:power controlled by LCD\n", __func__);
		return;
	}

	if (g_zinitix_dev_data->vci_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vci_gpio_ctrl))
			gpio_free(g_zinitix_dev_data->vci_gpio_ctrl);

		TS_LOG_INFO("%s:vci_gpio_ctrl free succ\n", __func__);
	}

	if (g_zinitix_dev_data->vddio_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vddio_gpio_ctrl))
			gpio_free(g_zinitix_dev_data->vddio_gpio_ctrl);
		TS_LOG_INFO("%s:vddio_gpio_ctrl free succ\n", __func__);
	}
}

int wakeup_nvm(int i)
{
	int ret;
	u16 cmd[2] = { 0, 0 }; // cmd len is 2
	u16 wake_nvm;

	/* ic vendor cmd enable */
	cmd[0] = ZINITIX_REG_VENDOR_EN;
	cmd[1] = ZINITIX_CMD_REG_ENABLE;
	ret = zinitix_write(&cmd[0], sizeof(cmd));
	if (ret < 0) {
		TS_LOG_ERR("%s:enable ic vendor command fail, ret=%d, i=%d\n",
			__func__, ret, i);
		return -EINVAL;
	}
	TS_LOG_INFO("%s:enable ic vendor command success\n",
		__func__);
	return 0;
}

static int zitinix_ic_enable(void)
{
	int i = 0;
	int ret = NO_ERR;
	/*length of cmd*/
	int cmd_len = 2;
	u16 cmd[2] = { 0, 0 };

	TS_LOG_INFO("%s:ic enable start\n", __func__);
	for (i = 0; i < ZINITIX_DETECT_I2C_RETRY_TIMES; i++) {
		/* wakeup nvm */
		if (g_zinitix_pdata->chip_id == ZINITIX_ZTW523_CHIP_ID) {
			ret = wakeup_nvm(i);
		} else {
			cmd[0] = ZINITIX_REG_VENDOR_EN;
			cmd[1] = ZINITIX_CMD_REG_ENABLE;
			ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
		}
		mdelay(ZINITIX_GENERAL_DELAY_1MS);
		if (ret < 0)
			continue;

		/* clear int */
		cmd[0] = ZINITIX_CMD_INT_CLEAR;
		ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
		if (ret < 0) {
			TS_LOG_ERR("%s:int clear fail, ret=%d, i=%d\n",
					__func__, ret, i);
			continue;
		}
		TS_LOG_INFO("%s:int clear success\n", __func__);
		mdelay(ZINITIX_GENERAL_DELAY_1MS);

		/* nvm init */
		cmd[0] = ZINITIX_REG_NVM_CTRL;
		cmd[1] = ZINITIX_CMD_REG_ENABLE;

		ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
		if (ret < 0) {
			TS_LOG_ERR("%s:enable ic nvm init fail, ret=%d, i=%d\n",
					__func__, ret, i);
			continue;
		}
		TS_LOG_INFO("%s:enable ic nvm init success\n", __func__);
		mdelay(ZINITIX_NVM_INIT_DELAY);

		/* start to run ic program */
		cmd[0] = ZINITIX_REG_PROG_CTRL;
		cmd[1] = ZINITIX_CMD_REG_ENABLE;
		ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
		if (ret < 0) {
			TS_LOG_ERR("%s:start to run program on ic fail,\
					ret=%d, i=%d\n", __func__, ret, i);
			continue;
		}
		TS_LOG_INFO("%s:start to run program on ic success\n", __func__);

		/* wait for checksum cal */
		msleep(ZINITIX_FIRMWARE_ON_DELAY);
		break;
	}
	TS_LOG_INFO("%s:ic enable success\n", __func__);

	return ret;
}

int set_support_number(struct zinitix_platform_data *zinitix_pdata)
{
	/*length of cmd*/
	int cmd_len = 2;
	u16 cmd[2] = { 0, 0 };
	int ret;
	/* set support number */
	zinitix_pdata->key_num = 0;
	cmd[0] = ZINITIX_BUTTON_SUPPORTED_NUM;
	cmd[1] = zinitix_pdata->key_num;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set button number fail, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* set supported finger number */
	zinitix_pdata->multi_fingers = ZINITIX_MAX_SUPPORTED_FINGER_NUM;
	cmd[0] = ZINITIX_SUPPORTED_FINGER_NUM_REG;
	cmd[1] = zinitix_pdata->multi_fingers;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set supported finger number failed , ret=%d\n",
				__func__, ret);
		return ret;
	}
	return ret;
}

int get_dnd_count(struct zinitix_platform_data *zinitix_pdata)
{
	int ret;
	u16 cmd[2] = { 0, 0 };
	/* get dnd n count */
	cmd[0] = ZINITIX_DND_N_COUNT;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->dnd_n_cnt, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read dnd n count failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* get dnd u count */
	cmd[0] = ZINITIX_DND_U_COUNT;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->dnd_u_cnt, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read dnd u count failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* get afe frequency */
	cmd[0] = ZINITIX_AFE_FREQUENCY;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->afe_frequency, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read afe frequency failed, ret=%d\n",
				__func__, ret);
		return ret;
	}
	return ret;
}

int get_xy_node_info(struct zinitix_platform_data *zinitix_pdata)
{
	int ret;
	/*length of cmd*/
	u16 cmd[2] = { 0, 0 };
	/* get vendor id */
	cmd[0] = ZINITIX_IC_VENDOR_ID;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->vendor_id, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read vendor id failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* get x node num */
	cmd[0] = ZINITIX_TOTAL_NUMBER_OF_X;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->channel_x_num, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read x node num failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* get y node num */
	cmd[0] = ZINITIX_TOTAL_NUMBER_OF_Y;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->channel_y_num, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read y node num failed, ret=%d\n",
				__func__, ret);
		return ret;
	}
	return ret;
}

int get_version_data(struct zinitix_platform_data *zinitix_pdata)
{
	int ret;
	u16 cmd[2] = { 0, 0 };

	cmd[0] = ZINITIX_FW_VER_REG;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->fw_version, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read firmware major version failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* get chip firmware minor version */
	cmd[0] = ZINITIX_MINOR_FW_VERSION;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->fw_minor_version, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read firmware minor version failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* get reg data version */
	cmd[0] = ZINITIX_DATA_VER_REG;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->reg_data_version, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read firmware data version failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* get reg data version */
	cmd[0] = ZINITIX_HW_ID_REG;
	ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&zinitix_pdata->hw_id, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read firmware data version failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	ret = snprintf(zinitix_pdata->project_id, ZINITIX_PROJECT_ID_LEN - 1,
			"%04x", zinitix_pdata->hw_id);
	if (ret >= (ZINITIX_PROJECT_ID_LEN - 1)) {
		TS_LOG_INFO("%s:%s, ret=%d, size=%u\n", __func__,
				"compatible_name out of range", ret,
				(ZINITIX_PROJECT_ID_LEN - 1));
		return -EINVAL;
	}
	zinitix_pdata->project_id[ret] = '\0';
	return ret;
}

/* set xy resolution */
int set_xy_resolution(struct zinitix_platform_data *zinitix_pdata)
{
	int ret;
	/*length of cmd*/
	int cmd_len = 2;
	u16 cmd[2] = { 0, 0 };

	cmd[0] = ZINITIX_X_RESOLUTION;
	cmd[1] = zinitix_pdata->zinitix_device_data->x_max_mt;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set x resolution fail, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* set y resolution */
	cmd[0] = ZINITIX_Y_RESOLUTION;
	cmd[1] = zinitix_pdata->zinitix_device_data->y_max_mt;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set y resolution fail, ret=%d\n",
				__func__, ret);
		return ret;
	}
	return ret;
}

int set_irq_mode()
{
	int cmd_len = 2;
	u16 cmd[2] = { 0, 0 };
	int ret;

	cmd[0] = ZINITIX_INT_ENABLE_FLAG;
	cmd[1] = ZINITIX_CMD_INT_VALUE;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	msleep(ZINITIX_GENERAL_DELAY_1MS);
	return ret;
}

int set_init_touch_mode(struct zinitix_platform_data *zinitix_pdata)
{
	/*length of cmd*/
	int cmd_len = 2;
	u16 cmd[2] = { 0, 0 };
	int ret;
	/* set initial touch mode */
	zinitix_pdata->touch_mode = ZINITIX_POINT_MODE;
	cmd[0] = ZINITIX_INITIAL_TOUCH_MODE_REG;
	cmd[1] = zinitix_pdata->touch_mode;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set initial touch mode failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	cmd[0] = ZINITIX_TOUCH_MODE_REG;
	cmd[1] = ZINITIX_POINT_MODE;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set touch mode failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	ret = set_irq_mode();
	if (ret < 0) {
		TS_LOG_ERR("%s:set int and status reg fail, ret=%d\n",
				__func__, ret);
		return ret;
	}

#ifdef TARGET_HUAWEI_WEARABLES
	/* enable wake up gesture detection when init */
	cmd[0] = ZINITIX_WAKE_UP_GENSTRURE;
	cmd[1] = 0;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set gesture reg fail, ret=%d\n",
				__func__, ret);
		return ret;
	}
#endif /* TARGET_HUAWEI_WEARABLES */
	return ret;
}

int clear_int_and_status(struct zinitix_platform_data *zinitix_pdata)
{
	/*length of cmd*/
	u16 cmd[2] = { 0, 0 };
	int ret = 0;
	int i;

	cmd[0] = ZINITIX_CLEAR_INT_CMD;
	for (i = 0; i < ZINITIX_GENERAL_RETRY_TIMES; i++) {
		ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0)
			break;

		udelay(ZINITIX_CHIP_EN_DELAY);
	}

	if (i >= ZINITIX_GENERAL_RETRY_TIMES) {
		TS_LOG_ERR("%s:clear int and status failed, ret=%d\n",
				__func__, ret);
		return -EINVAL;
	}
	return ret;
}

int touch_info_init(struct zinitix_platform_data *zinitix_pdata)
{
	int ret;

	ret = get_xy_node_info(zinitix_pdata);
	if (ret < 0)
		return ret;

	ret = get_dnd_count(zinitix_pdata);
	if (ret < 0)
		return ret;

	/* get chip firmware version */
	ret = get_version_data(zinitix_pdata);
	if (ret < 0)
		return ret;

	/* initialize */
	ret = set_xy_resolution(zinitix_pdata);
	if (ret < 0)
		return ret;

	ret = set_support_number(zinitix_pdata);
	if (ret < 0)
		return ret;

	/* set touch mode */
	ret = set_init_touch_mode(zinitix_pdata);
	if (ret < 0)
		return ret;

	/* clear int and status */
	ret = clear_int_and_status(zinitix_pdata);
	if (ret < 0)
		return ret;

	return ret;
}

static int zinitix_touch_init(struct zinitix_platform_data *zinitix_pdata)
{
	int ret;
	/* length of cmd */
	u16 cmd[2] = { 0, 0 };

	if (zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:zinitix_pdata is null pointer\n", __func__);
		return -EINVAL;
	}

	/* software reset */
	cmd[0] = ZINITIX_SWRESET_CMD;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:software reset fail, ret=%d\n", __func__, ret);
		return ret;
	}
	zinitix_pdata->multi_fingers = ZINITIX_MAX_SUPPORTED_FINGER_NUM;
	touch_info_init(zinitix_pdata);

	TS_LOG_INFO("%s:vendor id=0x%x, x node num=%d, y node num=%d, \
				dnd_n_cnt=%d, dnd_u_cnt =%d, afe_freq =%d, \
				fw_ver=%d, fw_min_ver=%d, reg_data_ver=%d, \
				hw_id=%d, max fingers num=%d, touch_mode=%d\n",
			__func__,
			zinitix_pdata->vendor_id,
			zinitix_pdata->channel_x_num,
			zinitix_pdata->channel_y_num,
			zinitix_pdata->dnd_n_cnt,
			zinitix_pdata->dnd_u_cnt,
			zinitix_pdata->afe_frequency,
			zinitix_pdata->fw_version,
			zinitix_pdata->fw_minor_version,
			zinitix_pdata->reg_data_version,
			zinitix_pdata->hw_id,
			zinitix_pdata->multi_fingers,
			zinitix_pdata->touch_mode);

	TS_LOG_INFO("%s:touch init success\n", __func__);
	return NO_ERR;
}

int zinitix_hardware_reset(void)
{
	int ret;

	TS_LOG_INFO("%s:start.\n", __func__);
	if (g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return -EINVAL;
	}

	ret = zinitix_gpio_reset();
	if (ret) {
		TS_LOG_ERR("%s:gpio reset failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	ret = zitinix_ic_enable();
	if (ret < 0) {
		TS_LOG_ERR("%s:enable chip failed, ret=%d\n", __func__, ret);
		return ret;
	}

	ret = zinitix_touch_init(g_zinitix_pdata);
	if (ret < 0) {
		TS_LOG_ERR("%s:touch init error, ret=%d\n", __func__, ret);
		return ret;
	}

	TS_LOG_INFO("%s:end.\n", __func__);
	return 0;
}

int set_valid_data(struct zinitix_touch_point_info *touch_data,
		struct ts_event *event_data)
{
	int i;
	u16 x = 0;
	u16 y = 0;
	int eventdata = 255;
	int ret = 0;

	u16 x_max_mat = g_zinitix_pdata->zinitix_device_data->x_max_mt;
	u16 y_max_mat = g_zinitix_pdata->zinitix_device_data->y_max_mt;

	for (i = 0; i < g_zinitix_pdata->multi_fingers; i++) {
		/* valid data */
		if (zinitix_bit_test((*touch_data).coord[i].sub_status,
					ZINITIX_SUB_BIT_PT_EXIST)) {
			x = (*touch_data).coord[i].x;
			y = (*touch_data).coord[i].y;
			if (x > x_max_mat || y > y_max_mat)	{
				TS_LOG_ERR("%s:invalid coord,x=%d, y=%d, x_max=%d, y_max=%d\n",
						__func__, x, y, x_max_mat, y_max_mat);
#ifdef TARGET_HUAWEI_WEARABLES
				g_zinitix_err_count++;
				if (g_zinitix_err_count > ERROR_RESET)
					ext_reset_command();
#endif
				return -EINVAL;
			}
			g_zinitix_err_count = 0;
			event_data->finger_id[i] = i;
			event_data->touch_point++;
			event_data->touch_point_num++;
			event_data->touch_event[i] = 0;
			event_data->pressure[i] = eventdata;
			event_data->area[i] = eventdata;
			if (g_zinitix_pdata->zinitix_device_data->rawdata_arrange_swap ==
					false) {
				event_data->position_x[i] = x;
				event_data->position_y[i] = y;
			} else {
				event_data->position_x[i] = y;
				event_data->position_y[i] = x;
			}
		} else {
			/* 0 down 1 up ,no 2 contact */
			event_data->touch_event[i] = 1;
			event_data->pressure[i] = 0;
			event_data->area[i] = 0;
		}

		TS_LOG_DEBUG("%s:touch data:(i=%d,id=%d,x=(0x%04x),y=(0x%04x),\
			w=(0x%04x)),point_num=%d,event=%d\n", __func__, i,
			event_data->finger_id[i], event_data->position_x[i],
			event_data->position_y[i], event_data->pressure[i],
			event_data->touch_point, event_data->touch_event[i]);
	}
	return ret;
}

#ifdef TARGET_HUAWEI_WEARABLES
static int zinitix_read_gesture(struct ts_event *event_data)
{
	u16 cmd;
	int ret;
	u16 gesture_flag = 0;
	/* length of cmd: 2 */
	int cmd_len = ZINITIX_REG_ADDRESS_SIZE;
	u16 cmd_data[ZINITIX_REG_ADDRESS_SIZE];
	enum ts_sleep_mode sleep_mode;
	int easy_wakeup_flag;

	if (event_data == NULL)
		/* return and read touch finger data */
		return -EINVAL;
	sleep_mode = g_zinitix_dev_data->easy_wakeup_info.sleep_mode;
	easy_wakeup_flag = g_zinitix_dev_data->easy_wakeup_info.easy_wakeup_flag;

	if ((sleep_mode != TS_GESTURE_MODE) || (!easy_wakeup_flag) ||
			(!g_zinitix_pdata->suspend_state)) {
		/* return and read touch finger data */
		return -EINVAL;
	}

	/* if in sleep mode, the double click gesture will resume TP */
	TS_LOG_INFO("%s:gesture wakeup\n", __func__);
	cmd = ZINITIX_WAKE_UP_GENSTRURE;
	ret = zinitix_read(&cmd, ZINITIX_REG_ADDRESS_SIZE,
		(u8 *)&gesture_flag, sizeof(gesture_flag));
	if (ret < 0) {
		TS_LOG_ERR("%s:read gesture reg fail, ret=%d\n", __func__, ret);
		goto exit;
	}
	/* gesture wake up TP in sleep mode */
	TS_LOG_INFO("%s:gesture_flag %d\r\n", __func__, gesture_flag);

	cmd_data[0] = ZINITIX_REG_3DA;
	cmd_data[1] = gesture_flag;
	ret = zinitix_write(&cmd_data[0],
		ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set gesture reg 0x3da fail, ret=%d\n", __func__, ret);
		goto exit;
	}

	/* 1 double click, 2 single click */
	if ((gesture_flag == 1) || (gesture_flag == 2)) {
		if (g_zinitix_pdata->waking_up == 1) {
			TS_LOG_ERR("%s:is waking up, exit.\n", __func__);
			goto exit;
		}
		g_zinitix_pdata->waking_up = 1;
		event_data->gesture_wakeup_value = KEY_WAKEUP;
		HWDUBAI_LOGE("DUBAI_TAG_SCREEN_ON_EVENT", "event=SingleTouch");
		cmd_data[0] = ZINITIX_WAKE_UP_GENSTRURE;
		cmd_data[1] = 0;
		ret = zinitix_write(&cmd_data[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
		if (ret < 0) {
			TS_LOG_ERR("%s:set gesture reg fail, ret=%d\n", __func__, ret);
			goto exit;
		}
	}
exit:
	/* in suspend state, return and do not read touch finger data */
	return NO_ERR;
}
#endif /* TARGET_HUAWEI_WEARABLES */

static int zinitix_read_touch_data(struct ts_event *event_data)
{
	struct zinitix_touch_point_info touch_data = { 0 };
	u16 cmd = ZINITIX_POINT_STATUS_REG;
	int ret;
	int ret1;

	if (event_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:para is null pointer\n", __func__);
		return -EINVAL;
	}
	memset(event_data, 0, sizeof(struct ts_event));

#ifdef TARGET_HUAWEI_WEARABLES
	/* if read gesture ok, return and do not read touch finger data */
	ret = zinitix_read_gesture(event_data);
	if (ret == NO_ERR)
		goto read_data_exit;
#endif /* TARGET_HUAWEI_WEARABLES */

	/* check range */
	if ((g_zinitix_pdata->multi_fingers > ZINITIX_MAX_SUPPORTED_FINGER_NUM)
			|| (g_zinitix_pdata->multi_fingers == 0)) {
		TS_LOG_ERR("%s:multi_fingers=%d is out of range(0,%d].\n",
				__func__,
				g_zinitix_pdata->multi_fingers,
				ZINITIX_MAX_SUPPORTED_FINGER_NUM);
		return -EINVAL;
	}

	memset((u8 *)&touch_data, 0, sizeof(touch_data));

	ret = zinitix_read(&cmd, ZINITIX_REG_ADDRESS_SIZE, (u8 *)&touch_data,
			sizeof(touch_data));
	if (ret < 0) {
		TS_LOG_ERR("%s:read touch data failed, ret=%d.\n",
				__func__, ret);
		ret = -EINVAL;
		goto read_data_exit;
	}

#ifdef TARGET_HUAWEI_WEARABLES
	/* if palm cover event, will screen off */
	if (zinitix_bit_test(touch_data.status, ZINITIX_BIT_PALM)) {
		TS_LOG_ERR("%s:palm 0x%x %d\n",
			__func__, touch_data.status,
			event_data->gesture_wakeup_value);
		event_data->gesture_wakeup_value = KEY_SLEEP;
		ret = NO_ERR;
		goto read_data_exit;
	}
#endif /* TARGET_HUAWEI_WEARABLES */

	if (zinitix_bit_test(touch_data.status, ZINITIX_BIT_MUST_ZERO)
			|| touch_data.status == 0) {
		TS_LOG_INFO("%s:invalid touch data status(0x%04x)\n",
				__func__, touch_data.status);
		ret = -EINVAL;
		goto read_data_exit;
	}

	ret = set_valid_data(&touch_data, event_data);
	if (ret < 0)
		TS_LOG_ERR("%s:set valid data failed, ret=%d.\n",
				__func__, ret);

read_data_exit:
	cmd = ZINITIX_CLEAR_INT_CMD;

	ret1 = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
	if (ret1 == 0)
		TS_LOG_DEBUG("%s:send clear int cmd success.\n",
				__func__);

	return ret;
}

/*
 * why we need this function ?
 * when ts kit found info->cur_finger_number is 0,
 * ts kit will report BTN_TOUCH UP, otherwize,
 * report BTN_TOUCH DOWN.
 * the recovery mode will use BTN_TOUCH UP
 * to judge if the finger have release, so we need this event.
 *
 * what risk at here ?
 * we set cur_finger_number to 0, so the ts kit will not report
 * this poit, ts kit will report a BTN_TOUCH UP instead of this
 * point. when close SITRONIX_REPORT_BTN_TOUCH, the last two data package
 * in input log is like the following two packages, we can open
 * SITRONIX_REPORT_BTN_TOUCH, if only the package two is report, we can't
 * open SITRONIX_REPORT_BTN_TOUCH.
 *
 *     [package 1]
 *         EV_ABS       ABS_MT_PRESSURE      00000009
 *         EV_ABS       ABS_MT_POSITION_X    00000434
 *         EV_ABS       ABS_MT_POSITION_Y    0000075e
 *         EV_ABS       ABS_MT_TRACKING_ID   00000000
 *         EV_SYN       SYN_MT_REPORT        00000000
 *         EV_SYN       SYN_REPORT           00000000
 *
 *     [package 2]
 *         EV_ABS       ABS_MT_PRESSURE      00000000
 *         EV_ABS       ABS_MT_POSITION_X    00000434
 *         EV_ABS       ABS_MT_POSITION_Y    0000075e
 *         EV_ABS       ABS_MT_TRACKING_ID   00000000
 *         EV_SYN       SYN_MT_REPORT        00000000
 *         EV_SYN       SYN_REPORT           00000000
 *
 * notice: the different between package 1 and package 2 is:
 * the value of [package 1]->ABS_MT_POSITION_X is the same as
 * [package 2]->ABS_MT_POSITION_X, and so do ABS_MT_POSITION_Y,
 * but the ABS_MT_PRESSURE of [package 1] is not 0, and the
 * ABS_MT_PRESSURE of [package 2] is 0
 */
static void zinitix_set_finger_number(struct ts_fingers *info,
		struct ts_event *touch_data)
{
	struct zinitix_platform_data *pdata = NULL;
	struct ts_event *last_data = NULL;

	if ((info == NULL) || (touch_data == NULL)) {
		TS_LOG_ERR("%s:para is null pointer\n", __func__);
		return;
	}

	pdata = zinitix_get_platform_data();
	last_data = &pdata->touch_data;

	/* if this point is the release point, set finger number to 0 */
	info->cur_finger_number = touch_data->touch_point_num;
	memcpy(last_data, touch_data, sizeof(struct ts_event));
}

static int zinitix_irq_top_half(struct ts_cmd_node *cmd)
{
	TS_LOG_DEBUG("%s:start\n", __func__);
	if (cmd == NULL) {
		TS_LOG_ERR("%s:para is null pointer\n", __func__);
		return -EINVAL;
	}
	if (g_zinitix_dev_data->ts_platform_data->chip_data->need_drop_point) {
		TS_LOG_INFO("%s:enable irq need to drop a point\n", __func__);
		g_zinitix_dev_data->ts_platform_data->chip_data->need_drop_point = 0;
		return -EINVAL;
	}

	TS_LOG_DEBUG("zinitix irq top half before TS_INT_PROCESS\n");
	cmd->command = TS_INT_PROCESS;
	TS_LOG_DEBUG("zinitix irq top half called\n");
	return NO_ERR;
}

void irq_set_info(struct ts_event st_touch_data, struct ts_fingers *info)
{
	int x = 0;
	int y = 0;
	int z = 0;
	int wx = 0;
	int wy = 0;
	int i;

	for (i = 0; i < st_touch_data.touch_point; i++) {
		x = st_touch_data.position_x[i];
		y = st_touch_data.position_y[i];
		wx = st_touch_data.area[i];
		wy = st_touch_data.area[i];
		z = st_touch_data.pressure[i];

		info->fingers[st_touch_data.finger_id[i]].status = TP_FINGER;
		info->fingers[st_touch_data.finger_id[i]].x = x;
		info->fingers[st_touch_data.finger_id[i]].y = y;
		info->fingers[st_touch_data.finger_id[i]].major = wx;
		info->fingers[st_touch_data.finger_id[i]].minor = wy;
		info->fingers[st_touch_data.finger_id[i]].pressure = z;

		TS_LOG_DEBUG("%s:%d:x = %d; y = %d; wx = %d; wy = %d\n",
				__func__,
				st_touch_data.finger_id[i], x, y, wx, wy);
	}

#ifdef TARGET_HUAWEI_WEARABLES
	info->gesture_wakeup_value = st_touch_data.gesture_wakeup_value;
#endif /* TARGET_HUAWEI_WEARABLES */
}

static int zinitix_irq_bottom_half(struct ts_cmd_node *in_cmd,
		struct ts_cmd_node *out_cmd)
{
	int ret;
	struct ts_event st_touch_data;
	struct algo_param *algo_p = NULL;
	struct ts_fingers *info = NULL;

	TS_LOG_DEBUG("%s:start\n", __func__);

	if ((in_cmd == NULL) || (out_cmd == NULL)
			|| (g_zinitix_dev_data == NULL)
			|| (g_zinitix_pdata == NULL)) {
		TS_LOG_ERR("%s:para is null pointer\n", __func__);
		return -EINVAL;
	}
	if (!gpio_get_value(g_zinitix_pdata->i2c_switch)) {
		TS_LOG_INFO("%s:i2c not at AP!\n", __func__);
		return -EINVAL;
	}

	if (g_zinitix_dev_data->need_wd_check_status == true)
		g_zinitix_pdata->irq_processing = true;

	out_cmd->command = TS_INVAILD_CMD;

	ret = zinitix_read_touch_data(&st_touch_data);
	if (ret < 0) {
		TS_LOG_DEBUG("%s:get touch data failed\n", __func__);
		goto irq_exit;
	}

	algo_p = &out_cmd->cmd_param.pub_params.algo_param;
	info = &algo_p->info;

	out_cmd->command = TS_INPUT_ALGO;
	algo_p->algo_order = g_zinitix_dev_data->algo_id;
	TS_LOG_DEBUG("%s:algo_order:%d\n", __func__, algo_p->algo_order);

	irq_set_info(st_touch_data, info);

	zinitix_set_finger_number(info, &st_touch_data);

irq_exit:
	if (g_zinitix_dev_data->need_wd_check_status == true)
		g_zinitix_pdata->irq_processing = false;

	return NO_ERR;
}

int send_clear_int_cmd(void)
{
	u8 retry;
	int ret = 0;
	/*length of cmd*/
	u16 cmd[2] = { 0, 0 };

	cmd[0] = ZINITIX_CLEAR_INT_CMD;
	for (retry = 0; retry < ZINITIX_GENERAL_RETRY_TIMES; retry++) {
		ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0) {
			TS_LOG_INFO("%s:send clear int cmd success.\n",
					__func__);
			break;
		}
		msleep(ZINITIX_GENERAL_DELAY_10MS);
	}
	return ret;
}

int software_reset(u16 dnd_n_cnt, u16 dnd_u_cnt, u16 afe_freq, u16 value)
{
	/*length of cmd*/
	int cmd_len = 2;
	u16 cmd[2] = { 0, 0 }; // cmd len 2
	int ret;

	if (value != ZINITIX_POINT_MODE) {
		cmd[0] = ZINITIX_DND_N_COUNT;
		cmd[1] = dnd_n_cnt;
		int write_len = ZINITIX_REG_ADDRESS_SIZE * cmd_len;

		ret = zinitix_write(&cmd[0], write_len);
		if (ret < 0) {
			TS_LOG_ERR("%s:set dnd n count failed , ret=%d\n",
				__func__, ret);
			return ret;
		}
		cmd[0] = ZINITIX_DND_U_COUNT;
		cmd[1] = dnd_u_cnt;
		ret = zinitix_write(&cmd[0], write_len);
		if (ret < 0) {
			TS_LOG_ERR("%s:set dnd u count failed , ret=%d\n",
				__func__, ret);
			return ret;
		}
		cmd[0] = ZINITIX_AFE_FREQUENCY;
		cmd[1] = afe_freq;
		ret = zinitix_write(&cmd[0], write_len);
		if (ret < 0) {
			TS_LOG_ERR("%s:set afe frequency failed , ret=%d\n",
				__func__, ret);
			return ret;
		}
		mdelay(20); // delay 20ms

		cmd[0] = ZINITIX_DELAY_RAW_FOR_HOST;
		cmd[1] = ZINITIX_RAWDATA_DELAY_FOR_HOST;
		ret = zinitix_write(&cmd[0], write_len);
		if (ret < 0) {
			TS_LOG_ERR("%s:set raw data delay for host failed, ret=%d\n",
				__func__, ret);
			return ret;
		}
	}

	/* set touch mode */
	cmd[0] = ZINITIX_TOUCH_MODE_REG;
	cmd[1] = value;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE * cmd_len);
	if (ret < 0) {
		TS_LOG_ERR("%s:set touch mode failed, ret=%d\n",
			__func__, ret);
		return ret;
	}

	cmd[0] = ZINITIX_SWRESET_CMD;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:software reset fail, ret=%d\n", __func__, ret);
		return ret;
	}
	msleep(ZINITIX_GENERAL_DELAY_400MS);
	ret = send_clear_int_cmd();
	if (ret < 0) {
		TS_LOG_ERR("%s:clear int cmd failed, ret=%d\n", __func__, ret);
		return -EINVAL;
	}
	msleep(ZINITIX_GENERAL_DELAY_20MS);
	return ret;
}

static int zinitix_set_touchmode(u16 value)
{
	u16 dnd_n_cnt = ZINITIX_PDND_N_COUNT;
	u16 dnd_u_cnt = ZINITIX_PDND_U_COUNT;
	u16 afe_freq = ZINITIX_PDND_FREQUENCY;

	int ret;
	/*length of cmd*/
	int cmd_len = 2;
	u16 cmd[2] = { 0, 0 };

	if (g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:null pointer\n", __func__);
		return -EINVAL;
	}

	if ((value != ZINITIX_POINT_MODE) &&
		(value != ZINITIX_PDND_MODE) &&
		(value != ZINITIX_SHORT_MODE)) {
		TS_LOG_ERR("%s:incorrect touch mode,value=%d\n",
				__func__,
				value);
		return -EINVAL;
	}

	/* software reset */
	ret = software_reset(dnd_n_cnt, dnd_u_cnt, afe_freq, value);
	if (ret < 0)
		return ret;

	TS_LOG_INFO("%s:set touch mode=%d, dnd_n_cnt=%d, dnd_u_cnt=%d, \
				afe_freq=%d, success\n",
			__func__,
			value,
			dnd_n_cnt,
			dnd_u_cnt,
			afe_freq);

	return NO_ERR;
}

int wait_for_irq(int read_num, u32 pkg_size, int *raw_data)
{
	u32 wait_times = 0;
	int index = 0;
	int temp_num = 0;
	s16 raw[ZINITIX_SENSOR_NUM_MAX]; /* raw is s16: -32768~32767 */
	u16 cmd;
	int ret = 0;

	memset(raw, 0, ZINITIX_REG_VALUE_SIZE * ZINITIX_SENSOR_NUM_MAX);
	while (gpio_get_value(g_zinitix_dev_data->ts_platform_data->irq_gpio)) {
		wait_times++;
		if (wait_times >= ZINITIX_WAIT_IRQ_TIMES) {
			TS_LOG_ERR("%s:no irq received\n", __func__);
			return -EINVAL;
		}
		msleep(1);
	}

	while (read_num > 0) {
		temp_num = ZINITIX_RAWDATA_BUFFER_SIZE;
		if (read_num < ZINITIX_RAWDATA_BUFFER_SIZE)
			temp_num = read_num;

		/* get raw data */
		cmd = ZINITIX_RAWDATA_REG + index;
		ret = zinitix_read(&cmd, sizeof(cmd), ((u8 *)raw) +
				index * ZINITIX_RAWDATA_BUFFER_SIZE, temp_num);
		if (ret < 0) {
			TS_LOG_ERR("%s:read raw data failed, ret=%d\n",
					__func__, ret);
			return -EINVAL;
		}

		index++;
		read_num -= ZINITIX_RAWDATA_BUFFER_SIZE;
	}
	for (index = 0; index < pkg_size; index++) {
		/* raw is s16, the valid value is >= 0 */
		if (raw[index] < 0)
			raw[index] = 0;
		raw_data[index] = (int)raw[index];
	}

	cmd = ZINITIX_CLEAR_INT_CMD;
	for (index = 0; index < ZINITIX_GENERAL_RETRY_TIMES; index++) {
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret < 0)
			TS_LOG_INFO("%s:send clear int cmd failed,ret=%d\n", __func__, ret);
		msleep(ZINITIX_GENERAL_DELAY_1MS);
	}

	return 0;
}

static int zinitix_read_raw_data(int *raw_data, size_t size)
{
	int ret;
	u32 pkg_size;
	int read_num;
	u32 wait_times = 0;
	u8 index = 0;
	u16 cmd;

	if ((raw_data == NULL) || (g_zinitix_pdata == NULL) ||
			(g_zinitix_dev_data == NULL)) {
		TS_LOG_ERR("%s:raw_data is null pointer\n", __func__);
		return -EINVAL;
	}

	if ((g_zinitix_pdata->channel_x_num > ZINITIX_TX_NUM_MAX) ||
		(g_zinitix_pdata->channel_y_num > ZINITIX_RX_NUM_MAX)) {
		TS_LOG_ERR("%s:sensor number error,tx_num=%d,rx_num=%d\n",
				 __func__,
				g_zinitix_pdata->channel_x_num,
				g_zinitix_pdata->channel_y_num);
		return -EINVAL;
	}

	pkg_size = g_zinitix_pdata->channel_x_num * g_zinitix_pdata->channel_y_num;
	if (pkg_size > size) {
		TS_LOG_ERR("%s:size %d is not enough,need=%d\n",
				__func__, size, pkg_size);
		return -EINVAL;
	}

	read_num = ZINITIX_REG_VALUE_SIZE * pkg_size;

	cmd = ZINITIX_CLEAR_INT_CMD;
	for (index = 0; index < ZINITIX_GENERAL_RETRY_TIMES; index++) {
		while (gpio_get_value(g_zinitix_dev_data->ts_platform_data->irq_gpio)) {
			wait_times++;
			if (wait_times >= ZINITIX_WAIT_IRQ_TIMES) {
				TS_LOG_ERR("%s:no irq received\n", __func__);
				return -EINVAL;
			}
			msleep(1);
		}

		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0)
			TS_LOG_INFO("%s:send clear int cmd sucess.\n",
					__func__);
			msleep(ZINITIX_GENERAL_DELAY_10MS);
	}
	/* wait for irq */
	ret = wait_for_irq(read_num, pkg_size, raw_data);
	if (ret)
		return -EINVAL;

	TS_LOG_INFO("%s:get raw data success,x node=%d, y node=%d\n",
				__func__,
				g_zinitix_pdata->channel_x_num,
				g_zinitix_pdata->channel_x_num);
	return 0;
}

static int zinitix_init_test_prm(struct zinitix_platform_data *pdata,
		struct zinitix_test_params *params,
		struct ts_rawdata_info *info)
{
	int ret;

	TS_LOG_INFO("%s: set param data called\n", __func__);
	if ((!pdata) || !(params) || !(info)) {
		TS_LOG_INFO("%s: parameters invalid!\n", __func__);
		return -EINVAL;
	}

	ret = zinitix_parse_cap_test_config(pdata, params);
	if (ret < 0) {
		TS_LOG_ERR("%s: analysis tp test data failed\n", __func__);
		return ret;
	}

	TS_LOG_INFO("%s: set param data success\n", __func__);

	return 0;
}

int raw_data_test(struct zinitix_test_params *params,
		struct ts_rawdata_info *info, int raw_data_size)
{
	int ret = 0;
	int i = 0;
	int rawdata_buff_index = 2;

	if (params->point_by_point_judge == RAW_POINT_BY_POINT_JUDGE) {
		TS_LOG_INFO("%s: test %d\n", __func__,
				params->point_by_point_judge);
		/* compare raw data with shreshold */
		for (i = 0; i < raw_data_size; i++) {
			if ((info->buff[i + rawdata_buff_index] <
					params->threshold.raw_data_min_array[i]) ||
					(info->buff[i + rawdata_buff_index] >
					params->threshold.raw_data_max_array[i])) {
				TS_LOG_ERR("%s:%d: raw data[%d]=%d ,out of range[%d,%d]\n",
						__func__,
						__LINE__,
						i,
						info->buff[i + rawdata_buff_index],
						params->threshold.raw_data_min_array[i],
						params->threshold.raw_data_max_array[i]);
#if defined(CONFIG_HUAWEI_DSM)
				ts_dmd_report(DSM_TP_RAWDATA_ERROR_NO,
						"%s:%d: raw data[%d]=%d,out of range[%d,%d]\n",
						__func__,
						__LINE__,
						i,
						info->buff[i + rawdata_buff_index],
						params->threshold.raw_data_min_array[i],
						params->threshold.raw_data_max_array[i]);
#endif
				return -EINVAL;
			}
		}
	} else {
		/* compare raw data with shreshold */
		for (i = 0; i < raw_data_size; i++) {
			if ((info->buff[i + rawdata_buff_index] <
					params->threshold.raw_data_min)	||
					(info->buff[i + rawdata_buff_index] >
					params->threshold.raw_data_max)) {
				TS_LOG_ERR("%s:%d: raw data[%d]=%d ,out of range[%d,%d]\n",
						__func__,
						__LINE__,
						i,
						info->buff[i + rawdata_buff_index],
						params->threshold.raw_data_min,
						params->threshold.raw_data_max);
#if defined(CONFIG_HUAWEI_DSM)
				ts_dmd_report(DSM_TP_RAWDATA_ERROR_NO,
						"%s:%d: raw data[%d]=%d,out of range[%d,%d]\n",
						__func__,
						__LINE__,
						i,
						info->buff[i + rawdata_buff_index],
						params->threshold.raw_data_min_array[i],
						params->threshold.raw_data_max_array[i]);
#endif
				return -EINVAL;
			}
		}
	}
	return ret;
}

static int zinitix_raw_data_test(struct zinitix_test_params *params,
		struct ts_rawdata_info *info)
{
	int ret;
	int raw_data_size;
	int rawdata_buff_index = 2;
	int rawdata_index = 2;

	TS_LOG_INFO("%s: test item raw data\n", __func__);
	if (!params || !info || !g_zinitix_pdata ||
			!(g_zinitix_pdata->channel_x_num) ||
			!(g_zinitix_pdata->channel_y_num)) {
		TS_LOG_ERR("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	raw_data_size = g_zinitix_pdata->channel_x_num * g_zinitix_pdata->channel_y_num;
	if ((raw_data_size > (TS_RAWDATA_BUFF_MAX - rawdata_buff_index)) ||
			(raw_data_size <= 0)) {
		TS_LOG_ERR("%s: raw data size %d is out of range(0, %d]\n",
				__func__,
				raw_data_size,
				TS_RAWDATA_BUFF_MAX - rawdata_buff_index);
		return -EINVAL;
	}

	/* get raw data,reserved index 0 and 1 for saved channel x y */
	ret = zinitix_read_raw_data(&(info->buff[rawdata_index]), raw_data_size);
	if (ret < 0) {
		TS_LOG_ERR("%s: get raw data failed\n", __func__);
		return -EINVAL;
	}
	info->buff[0] = g_zinitix_pdata->channel_x_num;
	info->buff[1] = g_zinitix_pdata->channel_y_num;
	info->used_size = raw_data_size + rawdata_buff_index;

	ret = raw_data_test(params, info, raw_data_size);
	if (ret)
		return -EINVAL;

	return 0;
}

static int init_parameters(u8 *cap_test_num, struct ts_rawdata_info *info)
{
	int ret = 0;
	int free_mem = -2;
	int restore_point_mode = -1;

	(*cap_test_num)++;
	if (!g_zinitix_params) {
		g_zinitix_params = kzalloc(sizeof(struct zinitix_test_params),
				GFP_KERNEL);
		if (!g_zinitix_params) {
			TS_LOG_ERR("%s:alloc mem for params fail\n", __func__);
			snprintf(info->result, TS_RAWDATA_RESULT_MAX,
					"%dF-software reason", (*cap_test_num));
			ret = 0;
			return free_mem;
		}
		memset(g_zinitix_params, 0, sizeof(struct zinitix_test_params));
	}

	(*cap_test_num)++;
	if (g_zinitix_pdata->open_threshold_status == true) {
		ret = zinitix_init_test_prm(g_zinitix_pdata, g_zinitix_params, info);
		if (ret < 0) {
			TS_LOG_ERR("%s:get param from dts fail, ret=%d",
					__func__, ret);
			ret = 0;
			snprintf(info->result, TS_RAWDATA_RESULT_MAX,
					"%dF-software reason", (*cap_test_num));
			return free_mem;
		}

		if (g_zinitix_pdata->only_open_once_captest_threshold)
			g_zinitix_pdata->open_threshold_status = false;
	}

	/* set factory mode */
	(*cap_test_num)++;
	ret = zinitix_set_touchmode(ZINITIX_PDND_MODE);
	if (ret < 0) {
		TS_LOG_ERR("%s:set pdnd mode failed, ret=%d, test=%d\n",
				__func__, ret, (*cap_test_num));
		snprintf(info->result, TS_RAWDATA_RESULT_MAX,
				"%dF-software reason", (*cap_test_num));
		return restore_point_mode;
	}

	(*cap_test_num)++;
	ret = zinitix_raw_data_test(g_zinitix_params, info);
	if (ret < 0) {
		TS_LOG_ERR("%s:test raw data failed, ret=%d, test=%d\n",
				__func__, ret, (*cap_test_num));
		snprintf(info->result, TS_RAWDATA_RESULT_MAX,
				"%dF-panel reason", (*cap_test_num));
		return restore_point_mode;
	}

	TS_LOG_INFO("%s:tp raw test pass\n", __func__);
	snprintf(info->result, TS_RAWDATA_RESULT_MAX,
			"%dP-tp raw data test", (*cap_test_num));
	return ret;
}

static int get_short_data(struct ts_rawdata_info *info, u8 *buff, int skip_cnt)
{
	int ret;
	u16 cmd;
	u32 index;
	u32 wait_times = 0;
	int irq_gpio;

	irq_gpio = g_zinitix_dev_data->ts_platform_data->irq_gpio;
	for (index = 0; index < skip_cnt; index++) {
		while (gpio_get_value(irq_gpio)) {
			wait_times++;
			if (wait_times >= ZINITIX_WAIT_IRQ_TIMES) {
				TS_LOG_ERR("%s:no irq received\n", __func__);
				return -EINVAL;
			}
			msleep(ZINITIX_GENERAL_DELAY_1MS);
		}

		cmd = ZINITIX_CLEAR_INT_CMD;
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0)
			TS_LOG_INFO("%s:send clear int cmd suc\n", __func__);

		msleep(ZINITIX_GENERAL_DELAY_1MS);
	}

	while (gpio_get_value(irq_gpio)) {
		wait_times++;
		if (wait_times >= ZINITIX_WAIT_IRQ_TIMES) {
			TS_LOG_ERR("%s:no irq received\n", __func__);
			return -EINVAL;
		}
		msleep(ZINITIX_GENERAL_DELAY_1MS);
	}

	/* get raw data */
	cmd = ZINITIX_RAWDATA_REG;
	ret = zinitix_read(&cmd, sizeof(cmd), (char *)buff,
		ZINITIX_SHORT_DATA_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:failed to read short data, ret=%d\n",
			__func__, ret);
		return -EINVAL;
	}

	cmd = ZINITIX_CLEAR_INT_CMD;
	ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
	if (ret == 0)
		TS_LOG_INFO("%s:send clear int cmd suc\n", __func__);

	return 0;
}

static int zinitix_short_test(u8 *cap_test_num, struct ts_rawdata_info *info)
{
	int i;
	int ret;
	int restore_point_mode = -1; /* -1: need restore point mode */
	u16 buff[ZINITIX_SHORT_DATA_NUM] = { 0 };

	(*cap_test_num)++;
	/* set short test mode, num = 5 */
	ret = zinitix_set_touchmode(ZINITIX_SHORT_MODE);
	if (ret < 0) {
		TS_LOG_ERR("%s:set short mode failed, ret=%d, test=%d\n",
				__func__, ret, *cap_test_num);
		snprintf(info->result, TS_RAWDATA_RESULT_MAX,
				"%dF-software reason", *cap_test_num);
		return restore_point_mode;
	}

	(*cap_test_num)++;
	/* get short test data, num = 6 */
	ret = get_short_data(info, (u8 *)buff, ZINITIX_SHORT_TEST_SKIP);
	if (ret < 0) {
		TS_LOG_ERR("%s:set short mode failed, ret=%d, test=%d\n",
				__func__, ret, *cap_test_num);
		snprintf(info->result, TS_RAWDATA_RESULT_MAX,
				"%dF-software reason", *cap_test_num);
		return restore_point_mode;
	}

	(*cap_test_num)++;
	for (i = 0; i < ZINITIX_SHORT_DATA_NUM; i++) {
		TS_LOG_DEBUG("%s: short[%d]=%d\n", __func__, i, buff[i]);
		if (buff[i] != ZINITIX_NORMAL_SHORT_VALUE) {
			TS_LOG_ERR("%s:err short val %d %d, ret=%d, test=%d\n",
				__func__, i, buff[i], ret, *cap_test_num);
			snprintf(info->result, TS_RAWDATA_RESULT_MAX,
				"%dF-software reason", *cap_test_num);
			return restore_point_mode;
		}
	}

	TS_LOG_INFO("%s:tp short test pass\n", __func__);
	snprintf(info->result, TS_RAWDATA_RESULT_MAX,
			"%dP-tp raw data test", *cap_test_num);
	return ret;
}

static int zinitix_get_raw_data(struct ts_rawdata_info *info,
		struct ts_cmd_node *out_cmd)
{
	int ret;
	u8 cap_test_num = 0;
	u8 index = 0;
	u16 cmd = ZINITIX_CLEAR_INT_CMD;
	int free_mem = -2;
	int restore_point_mode = -1;

	TS_LOG_INFO("%s:zinitix get rawdata called\n", __func__);
	if (info == NULL) {
		TS_LOG_ERR("%s:info is null pointer\n", __func__);
		return 0;
	}

	if ((g_zinitix_pdata == NULL) || (out_cmd == NULL)) {
		TS_LOG_ERR("%s:null pointer, test=%d\n",
				__func__, cap_test_num);
		snprintf(info->result, TS_RAWDATA_RESULT_MAX,
				"%dF-software reason", cap_test_num);
		return 0;
	}

	/* tp raw data test: cm capacitance data test */
	ret = init_parameters(&cap_test_num, info);
	if (ret == free_mem)
		goto free_mem;
	else if (ret == restore_point_mode)
		goto restore_point_mode;

	/* tp short test */
	ret = zinitix_short_test(&cap_test_num, info);
	if (ret == restore_point_mode)
		goto restore_point_mode;

restore_point_mode:
	/* set point mode */
	cap_test_num++;
	ret = zinitix_set_touchmode(ZINITIX_POINT_MODE);
	if (ret < 0) {
		TS_LOG_ERR("%s:set point mode failed, ret=%d, test=%d\n",
				__func__, ret, cap_test_num);
		snprintf(info->result, TS_RAWDATA_RESULT_MAX,
				"%dF-software reason", cap_test_num);
	}

	for (index = 0; index < ZINITIX_GENERAL_RETRY_TIMES; index++) {
		msleep(ZINITIX_GENERAL_DELAY_100MS);
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret < 0)
			TS_LOG_INFO("%s:send clear int cmd failed,ret=%d\n",
					__func__, ret);
	}
	g_zinitix_dev_data->ts_platform_data->chip_data->need_drop_point = 1;
	return 0;

free_mem:
	kfree(g_zinitix_params);
	g_zinitix_params = NULL;
	g_zinitix_pdata->open_threshold_status = true;
	return ret;
}

int switch_info_debug_type(struct ts_diff_data_info *info, int raw_data_size)
{
	int rawdata_buff_index = 2;
	int ret = 0;

	switch (info->debug_type) {
	case READ_RAW_DATA:
		TS_LOG_INFO("%s:read raw data\n", __func__);
		/* get raw data,reserved index 0 and 1 for saved channel x y */
		ret = zinitix_read_raw_data(&(info->buff[rawdata_buff_index]),
				raw_data_size);
		if (ret < 0) {
			TS_LOG_ERR("%s: get raw data failed\n", __func__);
			return -EINVAL;
		}
		break;
	default:
		TS_LOG_ERR("%s:debug_type mis match\n", __func__);
		return -EINVAL;
	}

	if (ret == 0) {
		info->used_size = raw_data_size;
		info->buff[0] = g_zinitix_pdata->channel_x_num;
		info->buff[1] = g_zinitix_pdata->channel_y_num;
		info->used_size += rawdata_buff_index;
	}
	return ret;
}

static int zinitix_get_debug_data(struct ts_diff_data_info *info,
		struct ts_cmd_node *out_cmd)
{
	int ret;
	int raw_data_size;
	u8 index = 0;
	u16 cmd = ZINITIX_CLEAR_INT_CMD;
	int rawdata_buff_index = 2;

	TS_LOG_INFO("%s: get debug raw data\n", __func__);
	if (!g_zinitix_pdata || !info) {
		TS_LOG_ERR("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	ret = zinitix_set_touchmode(ZINITIX_PDND_MODE);
	if (ret < 0) {
		TS_LOG_ERR("%s:set pdnd mode failed, ret=%d\n",
				__func__, ret);
		return 0;
	}

	raw_data_size = g_zinitix_pdata->channel_x_num *
		g_zinitix_pdata->channel_y_num;
	if ((raw_data_size > (TS_RAWDATA_BUFF_MAX - rawdata_buff_index)) ||
			(raw_data_size <= 0)) {
		TS_LOG_ERR("%s: raw data size %d is out of range(0, %d]\n",
				__func__,
				raw_data_size,
				TS_RAWDATA_BUFF_MAX - rawdata_buff_index);
		goto restore_point_mode;
	}

	ret = switch_info_debug_type(info, raw_data_size);

restore_point_mode:
	ret = zinitix_set_touchmode(ZINITIX_POINT_MODE);
	if (ret < 0)
		TS_LOG_ERR("%s:set pdnd mode failed, ret=%d\n",
				__func__, ret);

	for (index = 0; index < ZINITIX_GENERAL_RETRY_TIMES; index++) {
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret < 0)
			TS_LOG_INFO("%s:send clear int cmd failed,ret=%d\n",
					__func__, ret);
	}

	return 0;
}

static bool zinitix_check_fw_version(const struct firmware *fw)
{
	u16 new_fw_main_ver;
	u16 new_fw_minor_ver;
	u16 new_fw_data_ver;

	if (fw == NULL || fw->data == NULL) {
		TS_LOG_ERR("%s:fw request fail\n", __func__);
		return false;
	}

	if (g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:null pointer found\n", __func__);
		return false;
	}

	/* new fw version */
	new_fw_main_ver = ((u16)fw->data[ZINITIX_FW_MAIN_VER_OFFSET]
			| ((u16)fw->data[ZINITIX_FW_MAIN_VER_OFFSET + 1]
				<< ZINITIX_BITS_PER_BYTE));

	new_fw_minor_ver = ((u16)fw->data[ZINITIX_FW_MINOR_VER_OFFSET]
			| ((u16)fw->data[ZINITIX_FW_MINOR_VER_OFFSET + 1]
				<< ZINITIX_BITS_PER_BYTE));

	new_fw_data_ver = ((u16)fw->data[ZINITIX_FW_DATA_VER_OFFSET]
			| ((u16)fw->data[ZINITIX_FW_DATA_VER_OFFSET + 1]
				<< ZINITIX_BITS_PER_BYTE));

	TS_LOG_ERR("%s: curr FW_VER=%x,FW_MIN_VER=%x,FW_DATA_VER=%x.\n",
			__func__,
			g_zinitix_pdata->fw_version,
			g_zinitix_pdata->fw_minor_version,
			g_zinitix_pdata->reg_data_version);
	TS_LOG_ERR("%s:new FW_VER=%x,FW_MIN_VER=%x,FW_DATA_VER=%x.\n",
			__func__,
			new_fw_main_ver,
			new_fw_minor_ver,
			new_fw_data_ver);

	if ((g_zinitix_pdata->fw_version == new_fw_main_ver) &&
		(g_zinitix_pdata->fw_minor_version == new_fw_minor_ver) &&
		(g_zinitix_pdata->reg_data_version == new_fw_data_ver)) {
		TS_LOG_ERR("%s:no need to upgrade.\n", __func__);
		return false;
	}

	TS_LOG_ERR("%s:need to upgrade.\n", __func__);
	return true;
}

static bool zinitix_check_need_upgrade(const struct firmware *fw)
{
	u16 buffer = 0;
	u16 fw_chip_id;
	u16 cmd;
	int ret;

	if (fw == NULL || fw->data == NULL) {
		TS_LOG_ERR("%s:fw request fail\n", __func__);
		return false;
	}

	if (g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:null pointer found\n", __func__);
		return false;
	}
	cmd = ZINITIX_IC_VENDOR_ID;
	ret = zinitix_read(&cmd, ZINITIX_REG_ADDRESS_SIZE, (u8 *)&buffer,
			ZINITIX_REG_VALUE_SIZE);
	if (ret < 0 || buffer != ZINITIX_VENDOR_ID_DEFAULT) {
		TS_LOG_ERR("%s: get IC vendor ID failed!\n", __func__);
		if ((g_zinitix_pdata->chip_id == ZINITIX_ZTW622_CHIP_ID) ||
			(g_zinitix_pdata->chip_id == ZINITIX_ZTW523_CHIP_ID))
			return true;
		return false;
	}

	fw_chip_id = ((u16)fw->data[ZINITIX_FW_CHIP_ID_OFFSET]
			| ((u16)fw->data[ZINITIX_FW_CHIP_ID_OFFSET + 1]
				<< ZINITIX_BITS_PER_BYTE));
	if (g_zinitix_pdata->update_fw_force != true) {
		if (fw_chip_id != g_zinitix_pdata->chip_id) {
			TS_LOG_ERR("%s: incorrect chid id: fw chip id(0x%04x),ic chid id(0x%04x)\n",
					__func__,
					fw_chip_id,
					g_zinitix_pdata->chip_id);
			return false;
		}
	}

	if (zinitix_check_fw_version(fw)
			|| g_zinitix_pdata->update_fw_force) {
		TS_LOG_ERR("%s:need to upgrade firmware\n", __func__);
		return true;
	}

	return false;
}

static void zinitix_get_fw_name(char *file_name)
{
	char *firmware_form = ".bin";
	u8 vendor_name_len;
	int ret;
	char chip_id_name[ZINITIX_CHIP_ID_LEN] = {0};

	if ((file_name == NULL) || (g_zinitix_pdata == NULL)) {
		TS_LOG_ERR("%s:null pointer found\n", __func__);
		return;
	}

	ret = snprintf_s(chip_id_name, ZINITIX_CHIP_ID_LEN, ZINITIX_CHIP_ID_LEN - 1,
		"%04x", g_zinitix_pdata->chip_id);
	if (ret < 0) {
		TS_LOG_ERR("%s:get chip_id_name failed!\n", __func__);
		return;
	}

	vendor_name_len = (u8)strlen(g_zinitix_pdata->vendor_name);
	if (vendor_name_len == 0)
		ret = strncat_s(file_name, ZINITIX_FILE_NAME_LEN, chip_id_name,
				strlen(chip_id_name) + 1);
	else
		ret = strncat_s(file_name, ZINITIX_FILE_NAME_LEN, g_zinitix_pdata->vendor_name,
				(vendor_name_len + 1));

	if (ret != EOK)
		TS_LOG_ERR("%s : strncat_s failed. ret = %d\n",
			__func__, ret);

	strncat(file_name, firmware_form, strlen(firmware_form) + 1);
	TS_LOG_INFO("%s:firmware name: %s\n", __func__, file_name);
}

int read_flash(int need_write_bytes_tmp, u8 *temp_buf)
{
	int ret = 0;
	u16 copy_bytes_onetime = 0;
	u16 flash_addr = 0;
#if defined(CONFIG_HUAWEI_DSM)
	int flash_sector_index = 8;
#endif
	u16 cmd[(ZINITIX_REG_ADDRESS_SIZE + ZINITIX_FLASH_SECTOR_SIZE)
			/ ZINITIX_REG_VALUE_SIZE] = { 0 };
	u16 total_sectors = (ZINITIX_FLASH_PAGE_SIZE / ZINITIX_FLASH_SECTOR_SIZE);
	u16 flash_sector;

	while (need_write_bytes_tmp > 0) {
		for (flash_sector = 0; flash_sector < total_sectors; flash_sector++) {
			cmd[0] = ZINITIX_READ_FLASH;
			/* out of range */
			copy_bytes_onetime = ZINITIX_FLASH_SECTOR_SIZE;
			if (need_write_bytes_tmp < ZINITIX_FLASH_SECTOR_SIZE)
				copy_bytes_onetime = need_write_bytes_tmp;

			ret = zinitix_read(&cmd[0], ZINITIX_REG_ADDRESS_SIZE,
					(u8 *)&temp_buf[flash_addr], copy_bytes_onetime);
			if (ret < 0) {
				TS_LOG_ERR("%s:read flash sector %d failed\n",
						__func__, flash_sector);
#if defined(CONFIG_HUAWEI_DSM)
				g_zinitix_dev_data->ts_platform_data->dsm_info
					.constraints_update_status =
					(((int)(flash_sector)) << flash_sector_index) +
					ZINITIX_READ_FLASH_ERROR;
#endif
				return -EINVAL;
			}
			flash_addr += ZINITIX_FLASH_SECTOR_SIZE;
			need_write_bytes_tmp -= ZINITIX_FLASH_SECTOR_SIZE;
			if (need_write_bytes_tmp <= 0)
				break;
		}
	}
	return ret;
}

int zinitix_read_firmware(const struct firmware *fw, int need_write_bytes)
{
	u8 *temp_buf = NULL;
	int ret;
	int need_write_bytes_tmp = need_write_bytes;

	TS_LOG_INFO("%s:start to read flash:\n", __func__);
	temp_buf = kzalloc(need_write_bytes, GFP_KERNEL);
	if (temp_buf == NULL) {
		TS_LOG_ERR("%s:alloc mem for device data fail\n", __func__);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_ALLOC_MEM_ERROR;
#endif
		kfree(temp_buf);
		temp_buf = NULL;
		return -ENOMEM;
	}
	memset((u8 *)temp_buf, 0, need_write_bytes_tmp);
	ret = read_flash(need_write_bytes, temp_buf);
	if (ret < 0) {
		kfree(temp_buf);
		temp_buf = NULL;
		return ret;
	}

	/* verified firmware */
	if (memcmp((u8 *)&temp_buf[0], (u8 *)&(fw->data[0]),
				need_write_bytes) != 0) {
		TS_LOG_ERR("%s:verified firmware failed\n", __func__);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_VERIFIED_FW_ERROR;
#endif
		kfree(temp_buf);
		temp_buf = NULL;
		return -EINVAL;
	}
	kfree(temp_buf);
	temp_buf = NULL;
	return 0;
}

int flash_nvm_set(void)
{
	int ret;
	/*length of cmd*/
	int cmd_len = 2;
	u16 cmd[(ZINITIX_REG_ADDRESS_SIZE + ZINITIX_FLASH_SECTOR_SIZE)
			/ ZINITIX_REG_VALUE_SIZE] = { 0 };
	/* nvm vpp off */
	cmd[0] = ZINITIX_REG_NVM_VPP_CTRL;
	cmd[1] = ZINITIX_CMD_REG_DISABLE;
	ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:disable ic nvm vpp fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_DISABLE_NVM_VPP_ERROR;
#endif
		return ret;
	}

	/* nvm wp disable */
	cmd[0] = ZINITIX_REG_NVM_WP_CTRL;
	cmd[1] = ZINITIX_CMD_REG_DISABLE;
	ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:disable ic nvm wp fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_DISABLE_NVM_WP_ERROR;
#endif
		return ret;
	}

	/* flash init */
	cmd[0] = ZINITIX_INIT_FLASH;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:init flash fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_INIT_FLASH_ERROR;
#endif
		return ret;
	}
	return ret;
}

int write_firmware_to_flash(const struct firmware *fw)
{
	u16 write_bytes_onetime;
	u16 flash_addr;
	int need_write_bytes_tmp;
	u16 total_sectors;
	int flash_sector;
	int ret = 0;
#if defined(CONFIG_HUAWEI_DSM)
	int flash_sector_index = 8;
#endif
	u16 copy_bytes_onetime = 0;
	u16 cmd[(ZINITIX_REG_ADDRESS_SIZE + ZINITIX_FLASH_SECTOR_SIZE)
			/ ZINITIX_REG_VALUE_SIZE] = { 0 };
	int need_write_bytes = fw->size;

	TS_LOG_INFO("%s:start to write flash:\n", __func__);
	if (need_write_bytes > g_zinitix_pdata->flash_size)
		need_write_bytes = g_zinitix_pdata->flash_size;

	write_bytes_onetime = (u16)(ZINITIX_REG_ADDRESS_SIZE +
			ZINITIX_FLASH_SECTOR_SIZE);
	total_sectors = (ZINITIX_FLASH_PAGE_SIZE / ZINITIX_FLASH_SECTOR_SIZE);

	need_write_bytes_tmp = need_write_bytes;
	flash_addr = 0;
	while (need_write_bytes_tmp > 0) {
		for (flash_sector = 0; flash_sector < total_sectors; flash_sector++) {
			memset((u8 *)cmd, 0xff, write_bytes_onetime);
			cmd[0] = ZINITIX_WRITE_FLASH;
			/* out of range */
			copy_bytes_onetime = ZINITIX_FLASH_SECTOR_SIZE;
			if (need_write_bytes_tmp < ZINITIX_FLASH_SECTOR_SIZE)
				copy_bytes_onetime = need_write_bytes_tmp;

			memcpy((u8 *)&cmd[1], (u8 *)&fw->data[flash_addr],
					copy_bytes_onetime);
			ret = zinitix_write(&cmd[0], write_bytes_onetime);
			if (ret < 0) {
				TS_LOG_ERR("%s:write flash sector %d failed\n",
						__func__, flash_sector);
#if defined(CONFIG_HUAWEI_DSM)
				g_zinitix_dev_data->ts_platform_data->dsm_info
					.constraints_update_status = (((int)(flash_sector))
					<< flash_sector_index) + ZINITIX_WRITE_FLASH_ERROR;
#endif
				return ret;
			}
			flash_addr += ZINITIX_FLASH_SECTOR_SIZE;
			need_write_bytes_tmp -= ZINITIX_FLASH_SECTOR_SIZE;
			udelay(ZINITIX_GENERAL_DELAY_100US);
			if (need_write_bytes_tmp <= 0)
				break;
		}
		/* for fuzing delay */
		mdelay(ZINITIX_GENERAL_DELAY_20MS);
	}
	ret = flash_nvm_set();
	return ret;
}

int nvm_set(void)
{
	u16 cmd[(ZINITIX_REG_ADDRESS_SIZE + ZINITIX_FLASH_SECTOR_SIZE)
			/ ZINITIX_REG_VALUE_SIZE] = { 0 };
	int cmd_len = 2;
	int ret;
	/* nvm vpp on */
	cmd[0] = ZINITIX_REG_NVM_VPP_CTRL;
	cmd[1] = ZINITIX_CMD_REG_ENABLE;
	ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:enable ic nvm vpp fail,ret=%d\n", __func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status =
			ZINITIX_ENABLE_NVM_VPP_ERROR;
#endif
		return ret;
	}

	/* nvm wp enable */
	cmd[0] = ZINITIX_REG_NVM_WP_CTRL;
	cmd[1] = ZINITIX_CMD_REG_ENABLE;
	ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:enable ic nvm wp fail, ret=%d\n", __func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_ENABLE_NVM_WP_ERROR;
#endif
		return ret;
	}
	return ret;
}

int int_nvm_flash_init(void)
{
	u16 cmd[(ZINITIX_REG_ADDRESS_SIZE + ZINITIX_FLASH_SECTOR_SIZE)
		/ ZINITIX_REG_VALUE_SIZE] = { 0 };
	int ret;
	/* length of cmd */
	int cmd_len = 2;
	/* clear int */
	cmd[0] = ZINITIX_CMD_INT_CLEAR;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:int clear fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status
			= ZINITIX_CMD_INT_CLEAR_ERROR;
#endif
		return ret;
	}
	udelay(ZINITIX_CHIP_EN_DELAY);

	/* nvm init */
	cmd[0] = ZINITIX_REG_NVM_CTRL;
	cmd[1] = ZINITIX_CMD_REG_ENABLE;
	ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:enable ic nvm init fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status
			= ZINITIX_ENABLE_NVM_INIT_ERROR;
#endif
		return ret;
	}
	mdelay(ZINITIX_NVM_INIT_DELAY);

	ret = nvm_set();
	if (ret < 0)
		return ret;
	/* flash init */
	cmd[0] = ZINITIX_INIT_FLASH;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:init flash fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_INIT_FLASH_ERROR;
#endif
		return ret;
	}
	cmd[0] = ZINITIX_ERASE_FLASH;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0)
		TS_LOG_ERR("%s:erase flash fail, ret=%d\n", __func__, ret);
	mdelay(ZINITIX_GENERAL_DELAY_100MS);
	return ret;
}

int nvm_fumode_power_ic_set(void)
{
	u16 cmd[(ZINITIX_REG_ADDRESS_SIZE + ZINITIX_FLASH_SECTOR_SIZE)
			/ ZINITIX_REG_VALUE_SIZE] = { 0 };
	int ret;
	/*length of cmd*/
	int cmd_len = 2;

	/* enter fu mode */
	cmd[0] = ZINITIX_FU_MODE_ENTER;
	ret = zinitix_write(&cmd[0], ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:enter fu mode fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data
			->dsm_info.constraints_update_status
			= ZINITIX_ENTER_FU_MODE_ERROR;
#endif
		return ret;
	}
	mdelay(ZINITIX_GENERAL_DELAY_10MS);

	/* power reset */
	zinitix_power_off();
	ret = zinitix_power_on();
	if (ret < 0) {
		TS_LOG_ERR("%s:power on device failed\n", __func__);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status = ZINITIX_POWER_ON_ERROR;
#endif
		return ret;
	}
	mdelay(ZINITIX_GENERAL_DELAY_10MS);

	/* ic vendor cmd enable */
	cmd[0] = ZINITIX_REG_VENDOR_EN;
	cmd[1] = ZINITIX_CMD_REG_ENABLE;
	ret = zinitix_write(&cmd[0], cmd_len * ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:enable ic vendor command fail, ret=%d\n",
				__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data->dsm_info
			.constraints_update_status
			= ZINITIX_IC_VENDOR_COMMAND_ERROR;
#endif
		return ret;
	}
	udelay(ZINITIX_CHIP_EN_DELAY);
	ret = int_nvm_flash_init();
	return ret;
}

static int zinitix_flash_fw_upgrade(const struct firmware *fw)
{
	int ret;
	int need_write_bytes = fw->size;
	u16 cmd;

	TS_LOG_INFO("%s:write flash start:\n", __func__);
	if (fw == NULL || fw->data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:fw request fail\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s:write flash length:0x%X\n", __func__, fw->size);
	if (fw->size == 0) {
		TS_LOG_INFO("%s:no need to write flash\n", __func__);
		return 0;
	}

	ret = nvm_fumode_power_ic_set();
	if (ret < 0)
		return ret;


	/* write firmware to flash */
	ret = write_firmware_to_flash(fw);
	if (ret < 0)
		return ret;
	if (need_write_bytes > g_zinitix_pdata->flash_size)
		need_write_bytes = g_zinitix_pdata->flash_size;
	ret = zinitix_read_firmware(fw, need_write_bytes);
	if (ret < 0)
		return ret;

	cmd = ZINITIX_SDRAM_RESET;
	zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
	msleep(ZINITIX_GENERAL_DELAY_10MS);
	ret = zinitix_power_resume();
	if (ret < 0) {
		TS_LOG_ERR("%s:power on device sequence failed\n",
				__func__);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data
			->dsm_info.constraints_update_status
			= ZINITIX_POWER_ON_SEQUENCE_ERROR;
#endif
		return ret;
	}
	TS_LOG_INFO("%s:write firmware success\n", __func__);
	return ret;
}

void zinitix_fw_update(const struct firmware *fw_entry)
{
	int ret = 0;
	int retry_count;

	g_zinitix_pdata->firmware_updating = true;
	for (retry_count = 0; retry_count < ZINITIX_FW_UPGRADE_RETRY_TIMES; retry_count++) {
		if (zinitix_check_need_upgrade(fw_entry)) {
			TS_LOG_INFO("%s:zinitix flash fw upgrade start\n",
					__func__);
			ret = zinitix_flash_fw_upgrade(fw_entry);
			if (ret < 0)
				TS_LOG_ERR("%s:zinitix firmware upgrade failed\n",
						__func__);
			else
				TS_LOG_ERR("%s:zinitix firmware	upgrade success\n",
						__func__);

		} else {
			TS_LOG_INFO("%s:zinitix needn't upgrade firmware\n",
					__func__);
		}

		if (ret == 0) {
			TS_LOG_INFO("%s:upgrade firmware end,retry=%d\n",
					__func__, retry_count);
			break;
		}
	}

	g_zinitix_pdata->firmware_updating = false;
	g_zinitix_pdata->is_finish_update = true;
}

static int zinitix_firmware_update(char *firmware_name, size_t name_size)
{
	int ret;
	const struct firmware *fw_entry = NULL;
	char app_info[TS_MAX_REG_VALUE_NUM] = { 0 };
	int offset;
#ifdef CONFIG_APP_INFO
	int ret1;
#endif

	if ((firmware_name == NULL) || (g_zinitix_pdata == NULL) || name_size == 0) {
		TS_LOG_ERR("%s:null pointer found\n", __func__);
		ret = -EINVAL;
		goto set_app_info;
	}

	TS_LOG_INFO("%s:request_firmware name: %s\n",
			__func__, firmware_name);
	__pm_stay_awake(&g_zinitix_pdata->zinitix_device_data
			->ts_platform_data->ts_wake_lock);
	ret = request_firmware(&fw_entry, firmware_name,
			&g_zinitix_pdata->zinitix_device_data
			->ts_platform_data->ts_dev->dev);
	if ((ret < 0) || (fw_entry == NULL) || (fw_entry->data == NULL)) {
		TS_LOG_ERR("%s %d: fail to request firmware %s, ret = %d\n",
				__func__, __LINE__, firmware_name, ret);
#if defined(CONFIG_HUAWEI_DSM)
		g_zinitix_dev_data->ts_platform_data
			->dsm_info.constraints_update_status
			= ZINITIX_REQUEST_FW_ERROR;
#endif
		ret = -EINVAL;
		goto err_request_firmware;
	}

	zinitix_fw_update(fw_entry);

err_request_firmware:
	if (fw_entry != NULL)
		release_firmware(fw_entry);
	__pm_relax(&g_zinitix_pdata->zinitix_device_data
			->ts_platform_data->ts_wake_lock);
set_app_info:
	offset = snprintf(app_info, TS_MAX_REG_VALUE_NUM - 1,
			"%s %s ", g_zinitix_pdata->vendor_name,
			ZINITIX_CHIP_NAME);
	/* get fw min_ver */
	offset += snprintf((u8 *)&(app_info[offset]),
			(TS_MAX_REG_VALUE_NUM - offset - 1), "%02x %02x %02x",
			g_zinitix_pdata->fw_version,
			g_zinitix_pdata->fw_minor_version,
			g_zinitix_pdata->reg_data_version);
#ifdef CONFIG_APP_INFO
	ret1 = app_info_set("touch_panel", app_info);
	if (ret1)
		TS_LOG_ERR("%s:set app info failed, ret=%d\n",
				__func__, ret);
#endif

	return ret;
}

int zinitix_fw_update_boot(char *file_name)
{
	int ret = NO_ERR;
	char firmware_name[ZINITIX_FW_FILE_NAME_MAX_LEN] = "";

	TS_LOG_INFO("%s: fw boot upgrade enter!\n", __func__);

	if (file_name == NULL) {
		TS_LOG_ERR("%s:null pointer\n", __func__);
		return -EINVAL;
	}


	zinitix_get_fw_name(file_name);
	TS_LOG_INFO("%s:zinitix start to request firmware  %s\n",
			__func__, file_name);
	ret = snprintf_s(firmware_name, ZINITIX_FW_FILE_NAME_MAX_LEN,
		ZINITIX_FW_FILE_NAME_MAX_LEN - 1, "ts/%s", file_name);
	if (ret < 0) {
		TS_LOG_ERR("%s:get firmware_name failed\n", __func__);
		return ret;
	}

	ret = zinitix_firmware_update(firmware_name, ZINITIX_FW_FILE_NAME_MAX_LEN);
	if (ret < 0)
		TS_LOG_ERR("%s:zinitix fw boot upgrade failed\n", __func__);
	TS_LOG_INFO("%s: end!\n", __func__);

	return ret;
}

int zinitix_fw_update_sd(void)
{
	int ret;
	char firmware_name[ZINITIX_FW_FILE_NAME_MAX_LEN] = "";

	if (g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:null pointer\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s: enter!\n", __func__);
	snprintf(firmware_name, sizeof(firmware_name), "%s",
			ZINITIX_FW_MANUAL_UPDATE_FILE_NAME);

	TS_LOG_INFO("%s:zinitix start to request firmware  %s\n",
			__func__, firmware_name);
	g_zinitix_pdata->update_fw_force = true;
	ret = zinitix_firmware_update(firmware_name, ZINITIX_FW_FILE_NAME_MAX_LEN);
	if (ret < 0)
		TS_LOG_ERR("%s:zinitix fw sd upgrade failed\n", __func__);
	g_zinitix_pdata->update_fw_force = false;
	TS_LOG_INFO("%s: end!\n", __func__);

	return ret;
}

static int zinitix_chip_get_info(struct ts_chip_info_param *info)
{
	size_t ic_vendor_size;
	size_t fw_vendor_size = 0;
	u8 offset = 0;
	int chip_info_num = 2;

	if ((info == NULL) || (g_zinitix_pdata == NULL)) {
		TS_LOG_ERR("%s:null pointer\n", __func__);
		return -EINVAL;
	}

	ic_vendor_size = CHIP_INFO_LENGTH * chip_info_num;
	strlcpy(info->ic_vendor, ZINITIX_CHIP_NAME, ic_vendor_size);
	strncat(info->ic_vendor, "-", 1);
	strncat(info->ic_vendor, g_zinitix_pdata->project_id,
			strlen(g_zinitix_pdata->project_id));
	strlcpy(info->mod_vendor, g_zinitix_pdata
			->vendor_name, CHIP_INFO_LENGTH);

	/* get fw version */
	offset += snprintf((u8 *)&(info->fw_vendor[offset]),
			(fw_vendor_size - offset - 1), "%02x %02x %02x",
			g_zinitix_pdata->fw_version,
			g_zinitix_pdata->fw_minor_version,
			g_zinitix_pdata->reg_data_version);
	if (fw_vendor_size <= offset) {
		TS_LOG_ERR("%s:get fw ver failed, offset=%d,fw_vendor_size=%d\n",
				__func__, offset, fw_vendor_size);
		return offset;
	}

	return NO_ERR;
}

int check_status(void)
{
	int ret = 1;

	if (g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:para is null pointer\n", __func__);
		return 0;
	}

	/* 1. not init ok , can't check esd  */
	if (g_zinitix_pdata->driver_init_ok == false) {
		TS_LOG_INFO("%s:[ESD]: not check esd, return immediately!\n",
				__func__);
		return 0;
	}

	/* 2. esdcheck is interrupt, then return */
	if (g_zinitix_pdata->irq_processing == true) {
		TS_LOG_INFO("%s:[ESD]: In interrupt state, return immediately!\n",
				__func__);
		return 0;
	}

	/* 3. check power state, if suspend, no need check esd */
	if (g_zinitix_pdata->suspend_state == true) {
		TS_LOG_INFO("%s:[ESD]: In suspend, return immediately!\n",
				__func__);
		/* because in suspend state, adb can be used,
		 *  when upgrade FW, will active ESD check(active = 1)
		 *  But in suspend, then will don't queue_delayed_work,
		 * when resume, don't check ESD again
		 */
		return 0;
	}

	/* 4. In boot upgrade mode , can't check esd */
	if (g_zinitix_pdata->firmware_updating == true) {
		TS_LOG_INFO("%s:[ESD]: In boot upgrade state, return immediately!\n",
				__func__);
		return 0;
	}

	return ret;
}

int read_vendor_id()
{
	u16 cmd;
	int ret;
	static u8 i2c_err = 0;
	u16 buffer = 0;
	int esd_check_reset = -1;
	static u8 vendorid_check_cnt = 0;

	cmd = ZINITIX_IC_VENDOR_ID;
	ret = zinitix_read(&cmd, ZINITIX_REG_ADDRESS_SIZE, (u8 *)&buffer,
			ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:[ESD]:read vendor id failed, ret=%d\n",
				__func__, ret);
		i2c_err++;
		if (i2c_err >= ZINITIX_ESD_CHECK_TIMES) {
			TS_LOG_ERR("%s:[ESD]:i2c abnormal. i2c_err=%d\n",
					__func__, i2c_err);
			i2c_err = 0;
			return esd_check_reset;
		}
		TS_LOG_ERR("%s: i2c_err=%d\n", __func__, i2c_err);
		return 0;
	}
	i2c_err = 0;

	/* if vendor id incorrect in ZINITIX_ESD_CHECK_TIMES times,reset it */
	if (buffer != ZINITIX_VENDOR_ID_DEFAULT) {
		vendorid_check_cnt++;
		if (vendorid_check_cnt >= ZINITIX_ESD_CHECK_TIMES) {
			TS_LOG_ERR("%s:[ESD]:IC vendor id incorrent. cnt=%d\n",
					__func__, vendorid_check_cnt);
			vendorid_check_cnt = 0;
			return esd_check_reset;
		}
		return esd_check_reset;
	}
	vendorid_check_cnt = 0;
	return 0;
}

static int zinitix_status_check(void)
{
	if (g_zinitix_pdata->is_finish_update == false) {
		TS_LOG_ERR("%s:[ESD]:update firmware, esd need not work", __func__);
		return;
	}

	int ret;

	TS_LOG_DEBUG("%s: Enter!\n", __func__);

	ret = check_status();
	if (ret == 0)
		return ret;

	/* 5. read vendor id */
	ret = read_vendor_id();
	if (ret < 0)
		goto esd_check_reset;

	TS_LOG_INFO("%s:[ESD]: esd check ok\n", __func__);
	return 0;

esd_check_reset:
	ret = zinitix_hardware_reset();
	if (ret)
		TS_LOG_ERR("%s:[ESD]:reset ic failed, ret=%d\n", __func__, ret);

	return ret;
}

static int zinitix_set_info_flag(struct ts_kit_platform_data *info)
{
	return NO_ERR;
}

static int zinitix_holster_switch(struct ts_holster_info *info)
{
	return NO_ERR;
}

static int zinitix_before_suspend(void)
{
	return NO_ERR;
}

static int zinitix_get_brightness_info(void)
{
	return NO_ERR;
}

static int zinitix_roi_switch(struct ts_roi_info *info)
{
	return NO_ERR;
}

static u8 *zinitix_roi_rawdata(void)
{
#ifdef ROI
	return NULL;
#else
	return NULL;
#endif
}

static int zinitix_palm_switch(struct ts_palm_info *info)
{
	return NO_ERR;
}

static int zinitix_calibrate_wakeup_gesture(void)
{
	return NO_ERR;
}

static void zinitix_chip_touch_switch(void)
{
}

static int zinitix_wrong_touch(void)
{
	return NO_ERR;
}

static int zinitix_get_capacitance_test_type(struct ts_test_type_info *info)
{
	int ret = 0;

	if (!info || !g_zinitix_dev_data) {
		TS_LOG_ERR("%s:info is Null\n", __func__);
		return -ENOMEM;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		memcpy(info->tp_test_type,
				g_zinitix_dev_data->tp_test_type,
				TS_CAP_TEST_TYPE_LEN);
		TS_LOG_INFO("%s:test_type=%s\n", __func__,
				info->tp_test_type);
		break;
	case TS_ACTION_WRITE:
		break;
	default:
		TS_LOG_ERR("%s:invalid op action:%d\n",
				__func__, info->op_action);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int zinitix_sleep_in(void)
{
	int ret;
	u16 cmd;
	u8 retry = 0;

	TS_LOG_INFO("%s:begin.\n", __func__);

	/* send clear int cmd */
	cmd = ZINITIX_CLEAR_INT_CMD;
	for (retry = 0; retry < ZINITIX_GENERAL_RETRY_TIMES; retry++) {
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0) {
			TS_LOG_INFO("%s:send clear int cmd success.\n",
					__func__);
			break;
		}
		msleep(ZINITIX_GENERAL_DELAY_10MS);
	}

	/* send sleep in cmd */
	cmd = ZINITIX_SLEEP_CMD;
	for (retry = 0; retry < ZINITIX_GENERAL_RETRY_TIMES; retry++) {
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0) {
			TS_LOG_INFO("%s:go to sleep in mode success.\n",
					__func__);
			break;
		}
		msleep(ZINITIX_GENERAL_DELAY_10MS);
	}

	if (retry >= ZINITIX_GENERAL_RETRY_TIMES) {
		TS_LOG_ERR("%s:go to sleep in mode failed, retry=%d\n",
				__func__, retry);
		return -EINVAL;
	}

	TS_LOG_INFO("%s:end.\n", __func__);
	return NO_ERR;
}

static int zinitix_sleep_out(void)
{
	int ret = NO_ERR;
	u16 cmd;
	u8 retry = 0;

	TS_LOG_INFO("%s:begin.\n", __func__);
	cmd = ZINITIX_SDRAM_RESET;
	for (retry = 0; retry < ZINITIX_GENERAL_RETRY_TIMES; retry++) {
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0) {
			TS_LOG_INFO("%s:get out of sleep in mode success.\n", __func__);
			break;
		}
		msleep(ZINITIX_GENERAL_DELAY_10MS);
	}
	g_zinitix_dev_data->ts_platform_data->chip_data->need_drop_point = 1;

	if (retry >= ZINITIX_GENERAL_RETRY_TIMES) {
		TS_LOG_ERR("%s:get out of sleep in mode failed, retry=%d\n",
				__func__, retry);
		return -EINVAL;
	}

	/* software reset */
	msleep(ZINITIX_GENERAL_DELAY_50MS);
	ret = zitinix_ic_enable();
	if (ret < 0) {
		TS_LOG_ERR("%s:enable chip failed, ret=%d\n", __func__, ret);
		return ret;
	}

	ret = set_irq_mode();
	if (ret < 0) {
		TS_LOG_ERR("%s:set int and status reg fail, ret=%d\n", __func__, ret);
		return ret;
	}
	// set rate 120
	zinitix_set_rate();

	cmd = ZINITIX_SWRESET_CMD;
	ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:software reset fail, ret=%d\n", __func__, ret);
		return ret;
	}

	/* send clear int cmd */
	msleep(ZINITIX_GENERAL_DELAY_20MS);
	cmd = ZINITIX_CLEAR_INT_CMD;
	for (retry = 0; retry < ZINITIX_GENERAL_RETRY_TIMES; retry++) {
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0) {
			TS_LOG_INFO("%s:send clear int cmd success.\n", __func__);
			break;
		}
		msleep(ZINITIX_GENERAL_DELAY_10MS);
	}
	g_zinitix_pdata->waking_up = 0;

	TS_LOG_INFO("%s:end.\n", __func__);
	return NO_ERR;
}

#ifdef TARGET_HUAWEI_WEARABLES
static int zinitix_idle_in(void)
{
	int ret;
	u16 cmd;
	u8 retry;

	TS_LOG_INFO("%s:begin.\n", __func__);
	if (!gpio_get_value(g_zinitix_pdata->i2c_switch)) {
		TS_LOG_INFO("%s:i2c not at AP,no need to idle.\n", __func__);
		return NO_ERR;
	}

	/* clear int and status */
	ret = clear_int_and_status(g_zinitix_pdata);
	if (ret < 0) {
		TS_LOG_ERR("%s:clear int and status failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	/* send sleep in cmd */
	cmd = ZINITIX_IDLE_CMD;
	for (retry = 0; retry < ZINITIX_GENERAL_RETRY_TIMES; retry++) {
		ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		if (ret == 0) {
			TS_LOG_INFO("%s:go to idle mode success.\n",
					__func__);
			break;
		}
		msleep(ZINITIX_GENERAL_DELAY_10MS);
	}

	if (retry >= ZINITIX_GENERAL_RETRY_TIMES) {
		TS_LOG_ERR("%s:go to idle mode failed, retry=%d\n",
				__func__, retry);
		return -EINVAL;
	}

	TS_LOG_INFO("%s:end.\n", __func__);
	return NO_ERR;
}
#endif /* TARGET_HUAWEI_WEARABLES */

static int zinitix_power_resume(void)
{
	int ret;

	TS_LOG_INFO("%s:begin.\n", __func__);
	ret = zinitix_power_on();
	if (ret != NO_ERR) {
		TS_LOG_ERR("%s:power on failed\n", __func__);
		return ret;
	}

	ret = zitinix_ic_enable();
	if (ret < 0) {
		TS_LOG_ERR("%s:enable chip failed, ret=%d\n", __func__, ret);
		return ret;
	}

	ret = zinitix_touch_init(g_zinitix_pdata);
	if (ret < 0) {
		TS_LOG_ERR("%s:touch init error, ret=%d\n", __func__, ret);
		return ret;
	}

	TS_LOG_INFO("%s:end.\n", __func__);
	return NO_ERR;
}

int zinitix_suspend_info(void)
{
	int ret = 0;
	int reset_gpio = g_zinitix_dev_data->ts_platform_data->reset_gpio;

	if (g_zinitix_dev_data->ts_platform_data->feature_info
			.wakeup_gesture_enable_info.switch_value == true) {
		switch (g_zinitix_dev_data->easy_wakeup_info.sleep_mode) {
		case TS_POWER_OFF_MODE:
			if (g_zinitix_pdata->self_ctrl_power ==
					ZINITIX_SELF_CTRL_POWER &&
					!g_tskit_pt_station_flag &&
					g_zinitix_pdata->power_down_ctrl) {
				gpio_direction_output(reset_gpio, ZINITIX_GPIO_LOW);
				udelay(ZINITIX_RESET_KEEP_LOW_TIME);
				zinitix_power_off();
			} else {
				/* goto sleep in mode */
				ret = zinitix_sleep_in();
				if (ret != NO_ERR)
					TS_LOG_ERR("%s, %d: enter sleep mode failed\n",
						__func__, __LINE__);
			}
			break;
		case TS_GESTURE_MODE:
#ifdef TARGET_HUAWEI_WEARABLES
			/* goto idle in mode */
			ret = zinitix_idle_in();
			if (ret != NO_ERR)
				TS_LOG_ERR("%s, %d: enter idle mode fail\n",
					__func__, __LINE__);
#endif
			break;
		default:
			break;
		}
	} else {
		if (g_zinitix_pdata->self_ctrl_power ==
				ZINITIX_SELF_CTRL_POWER &&
				!g_tskit_pt_station_flag &&
				g_zinitix_pdata->power_down_ctrl) {
			reset_gpio = g_zinitix_dev_data
						 ->ts_platform_data->reset_gpio;
			gpio_direction_output(reset_gpio, 0);
			udelay(ZINITIX_RESET_KEEP_LOW_TIME);
			zinitix_power_off();
		} else {
			ret = zinitix_sleep_in();
			if (ret != NO_ERR)
				TS_LOG_ERR("%s, %d: enter sleep mode failed\n",
						__func__, __LINE__);
		}
	}
	return ret;
}

static int zinitix_suspend(void)
{
	int ret;

	if (!g_zinitix_pdata || !g_zinitix_dev_data) {
		TS_LOG_ERR("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	if (g_zinitix_pdata->suspend_state == true) {
		TS_LOG_INFO("%s:Already suspended. Skipped\n", __func__);
		return NO_ERR;
	}

	if (g_zinitix_pdata->firmware_updating == true) {
		TS_LOG_INFO("%s:in firmware upgrading. Skipped\n", __func__);
		return NO_ERR;
	}

	TS_LOG_INFO("%s:suspend enter\n", __func__);

	ret = zinitix_suspend_info();

	if (ret != NO_ERR) {
		TS_LOG_ERR("%s: suspend info set failed\n", __func__);
		return ret;
	}

	g_zinitix_pdata->suspend_state = true;

	if ((g_zinitix_dev_data->easy_wakeup_info.sleep_mode !=
		TS_GESTURE_MODE) && (g_zinitix_pdata->ts_pinctrl)) {
		ret = pinctrl_select_state(g_zinitix_pdata->ts_pinctrl,
			g_zinitix_pdata->pinctrl_state_suspend);
		if (ret < 0)
			TS_LOG_ERR("%s:set pinctrl suspend fail\n", __func__);
		TS_LOG_INFO("%s: pinctrl_select_state suspend\n", __func__);
	}

	TS_LOG_INFO("%s:suspend end\n", __func__);

	return NO_ERR;
}

int zinitix_resume_info_set(int reset_gpio)
{
	int ret = 0;

	if (g_zinitix_dev_data->ts_platform_data
			->feature_info.wakeup_gesture_enable_info.switch_value
			== true) {
		switch (g_zinitix_dev_data->easy_wakeup_info.sleep_mode) {
		case TS_POWER_OFF_MODE:
			if (g_zinitix_pdata->self_ctrl_power ==
					ZINITIX_SELF_CTRL_POWER &&
					!g_tskit_pt_station_flag &&
					g_zinitix_pdata->power_down_ctrl) {
/* In suspend function reset pull down, Wakeup reset not need pull down */
				ret = zinitix_power_resume();
				if (ret != NO_ERR)
					TS_LOG_ERR("%s, %d: chip resume	failed\n",
						__func__, __LINE__);
				udelay(ZINITIX_RESET_KEEP_LOW_TIME);
				gpio_direction_output(reset_gpio, ZINITIX_GPIO_HIGH);
			} else {
				/* exit sleep mode */
				ret = zinitix_sleep_out();
				if (ret != NO_ERR)
					TS_LOG_ERR("%s, %d: set sleep out failed\n",
						__func__, __LINE__);
			}
			break;
		case TS_GESTURE_MODE:
			ret = zinitix_sleep_out();
			if (ret != NO_ERR)
				TS_LOG_ERR("%s: have error\n", __func__);
			break;
		default:
			TS_LOG_ERR("%s: no resume mode\n", __func__);
			return -EINVAL;
		}
	} else {
		if (g_zinitix_pdata->self_ctrl_power
				== ZINITIX_SELF_CTRL_POWER
				&& !g_tskit_pt_station_flag
				&& g_zinitix_pdata->power_down_ctrl) {
/* In suspend function reset pull down, Wakeup reset not need pull down */
			ret = zinitix_power_resume();
			if (ret != NO_ERR)
				TS_LOG_ERR("%s, %d: chip resume failed\n",
						__func__, __LINE__);
			udelay(ZINITIX_RESET_KEEP_LOW_TIME);
			gpio_direction_output(reset_gpio, ZINITIX_GPIO_HIGH);
		} else {
			/* exit sleep mode */
			ret = zinitix_sleep_out();
			if (ret != NO_ERR)
				TS_LOG_ERR("%s, %d: set sleep out failed\n",
						__func__, __LINE__);
		}
	}

	return ret;
}

static int zinitix_resume(void)
{
	int ret;
	int reset_gpio;

	if (!g_zinitix_pdata || !g_zinitix_dev_data) {
		TS_LOG_ERR("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	if (g_zinitix_pdata->suspend_state == false) {
		TS_LOG_INFO("%s:Already resumed. Skipped\n", __func__);
		return NO_ERR;
	}

	if (g_zinitix_pdata->firmware_updating == true) {
		TS_LOG_INFO("%s:in firmware upgrading. Skipped\n", __func__);
		return NO_ERR;
	}

	TS_LOG_INFO("%s:resume enter\n", __func__);
	reset_gpio = g_zinitix_dev_data->ts_platform_data->reset_gpio;
	g_zinitix_pdata->suspend_state = false;

	ret = zinitix_resume_info_set(reset_gpio);
	if (ret != NO_ERR) {
		TS_LOG_ERR("%s: resume info set failed and reset.\n",
			__func__);
		zinitix_hardware_reset();
		return ret;
	}

	if ((g_zinitix_dev_data->easy_wakeup_info.sleep_mode
			!= TS_GESTURE_MODE) && (g_zinitix_pdata->ts_pinctrl)) {
		ret = pinctrl_select_state(g_zinitix_pdata->ts_pinctrl,
				g_zinitix_pdata->pinctrl_state_active);
		if (ret < 0)
			TS_LOG_ERR("%s: set pinctrl active fail\n",
					__func__);
		TS_LOG_INFO("%s: pinctrl_select_state active\n",
				__func__);
	}

	TS_LOG_INFO("%s:resume end\n", __func__);
	return ret;
}

static int zinitix_after_resume(void *feature_info)
{
	return 0;
}

static int zinitix_regs_operate(struct ts_regs_info *info)
{
	u8 value_tmp;
	int ret = NO_ERR;
	u8 value[TS_MAX_REG_VALUE_NUM] = { 0 };
	u16 cmd = 0;
	int i = 0;
	int info_index = 8;

	if (!info || !g_zinitix_dev_data) {
		TS_LOG_ERR("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s:addr %d, op_action %d, bit %d, num %d\n",
			__func__,
			info->addr,
			info->op_action,
			info->bit,
			info->num);

	value_tmp = info->values[0];
	for (i = 0; i < info->num; i++)
		TS_LOG_INFO("%s:value[%d]=%d\n", __func__, i, info->values[i]);
	switch (info->op_action) {
	case TS_ACTION_READ:
		cmd = (u16)info->addr;
		ret = zinitix_read(&cmd, sizeof(cmd), value, info->num);
		if (ret < 0) {
			TS_LOG_ERR("%s:TS_ACTION_READ error,addr %d\n",
					__func__, info->addr);
			return -EINVAL;
		}
		if ((info->num == 1) && (info->bit < info_index)) {
			g_zinitix_dev_data->ts_platform_data->chip_data
			->reg_values[0] = (value[0]	>> info->bit) & 0x01;
		} else {
			for (i = 0; i < info->num; i++)
				g_zinitix_dev_data->ts_platform_data->chip_data->reg_values[i] = value[i];
		}
		break;
	default:
		TS_LOG_ERR("%s:reg operate default invalid action %d\n",
				__func__, info->op_action);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int zinitix_input_config(struct input_dev *input_dev)
{
	int abs_param = 255;

	if (!input_dev || !g_zinitix_pdata) {
		TS_LOG_ERR("%s:info is Null\n", __func__);
		return -EINVAL;
	}

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_TOOL_TYPE, input_dev->absbit);
	set_bit(ABS_MT_BLOB_ID, input_dev->absbit);
	set_bit(ABS_MT_TRACKING_ID, input_dev->absbit);
#ifdef TARGET_HUAWEI_WEARABLES
	input_set_capability(input_dev, EV_KEY, KEY_SLEEP);
	input_set_capability(input_dev, EV_KEY, KEY_WAKEUP);
#endif /* TARGET_HUAWEI_WEARABLES */

	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, abs_param, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, abs_param, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
			g_zinitix_pdata->multi_fingers, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, abs_param, 0, 0);
	if (g_zinitix_pdata->zinitix_device_data
			->rawdata_arrange_swap == false) {
		input_set_abs_params(input_dev, ABS_MT_POSITION_X,
				g_zinitix_pdata->zinitix_device_data->x_max,
				g_zinitix_pdata->zinitix_device_data->x_max_mt,
				0, 0);
		input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
				g_zinitix_pdata->zinitix_device_data->y_max,
				g_zinitix_pdata->zinitix_device_data->y_max_mt,
				0, 0);
	} else {
		input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
				g_zinitix_pdata->zinitix_device_data->x_max,
				g_zinitix_pdata->zinitix_device_data->x_max_mt,
				0, 0);
		input_set_abs_params(input_dev, ABS_MT_POSITION_X,
				g_zinitix_pdata->zinitix_device_data->y_max,
				g_zinitix_pdata->zinitix_device_data->y_max_mt,
				0, 0);
	}

	return NO_ERR;
}

#if defined(HUAWEI_CHARGER_FB)
static int zinitix_charger_switch(struct ts_charger_info *info)
{
	return NO_ERR;
}
#endif

static int zinitix_pinctrl_init(void)
{
	int ret = 0;

	if (!g_zinitix_pdata || !g_zinitix_dev_data) {
		TS_LOG_ERR("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	g_zinitix_pdata->ts_pinctrl = devm_pinctrl_get(&g_zinitix_dev_data
			->ts_platform_data->ts_dev->dev);
	if (IS_ERR(g_zinitix_pdata->ts_pinctrl)) {
		TS_LOG_ERR("%s:failed to devm pinctrl get\n", __func__);
		ret = -EINVAL;
		return ret;
	}

	g_zinitix_pdata->pinctrl_state_active =
		pinctrl_lookup_state(g_zinitix_pdata->ts_pinctrl,
				PINCTRL_STATE_ACTIVE);
	if (IS_ERR(g_zinitix_pdata->pinctrl_state_active)) {
		TS_LOG_ERR("%s:failed to pinctrl lookup state active\n",
				__func__);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	TS_LOG_INFO("%s: pinctrl_lookup_state %s\n", __func__,
			PINCTRL_STATE_ACTIVE);

	g_zinitix_pdata->pinctrl_state_suspend =
		pinctrl_lookup_state(g_zinitix_pdata->ts_pinctrl,
				PINCTRL_STATE_SUSPEND);
	if (IS_ERR(g_zinitix_pdata->pinctrl_state_suspend)) {
		TS_LOG_ERR("%s:failed to pinctrl lookup state suspend\n",
				__func__);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	TS_LOG_INFO("%s: pinctrl_lookup_state %s\n", __func__,
			PINCTRL_STATE_SUSPEND);

	g_zinitix_pdata->pinctrl_state_release =
		pinctrl_lookup_state(g_zinitix_pdata->ts_pinctrl,
				PINCTRL_STATE_RELEASE);
	if (IS_ERR(g_zinitix_pdata->pinctrl_state_release)) {
		TS_LOG_ERR("%s:failed to pinctrl lookup state release\n",
				__func__);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	TS_LOG_INFO("%s: pinctrl_lookup_state %s\n", __func__,
			PINCTRL_STATE_RELEASE);

	ret = pinctrl_select_state(g_zinitix_pdata->ts_pinctrl,
			g_zinitix_pdata->pinctrl_state_active);
	if (ret) {
		TS_LOG_ERR("%s:failed to set pinctr active\n", __func__);
		ret = -EINVAL;
		goto err_pinctrl_put;
	}
	TS_LOG_INFO("%s: pinctrl_select_state active\n", __func__);
	ret = gpio_get_value(g_zinitix_dev_data->ts_platform_data->irq_gpio);
	TS_LOG_INFO("%s: gpio_get_value %d\n", __func__, ret);

	return 0;

err_pinctrl_put:
	devm_pinctrl_put(g_zinitix_pdata->ts_pinctrl);
	return ret;
}

static void zinitix_pinctrl_release(void)
{
	TS_LOG_INFO("%s:called\n", __func__);
	if (!g_zinitix_pdata) {
		TS_LOG_ERR("%s: parameters invalid !\n", __func__);
		return;
	}

	if (!IS_ERR(g_zinitix_pdata->ts_pinctrl)) {
		pinctrl_select_state(g_zinitix_pdata->ts_pinctrl,
				g_zinitix_pdata->pinctrl_state_release);
		TS_LOG_ERR("%s: pinctrl_select_state release\n", __func__);
		devm_pinctrl_put(g_zinitix_pdata->ts_pinctrl);
	}
	g_zinitix_pdata->ts_pinctrl = NULL;
}

static int zinitix_init_chip(void)
{
	int ret;
	struct zinitix_platform_data *zinitix_pdata = g_zinitix_pdata;
	struct ts_kit_device_data *zinitix_dev_data = NULL;
	struct ts_kit_platform_data *ts_platform_data = NULL;

	if (zinitix_pdata == NULL || g_zinitix_dev_data == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return -EINVAL;
	}
	zinitix_dev_data = g_zinitix_dev_data;

	ret = zinitix_parse_dts(zinitix_dev_data->cnode, zinitix_pdata);
	if (ret) {
		TS_LOG_ERR("%s:parse dts fail, ret=%d\n", __func__, ret);
		return ret;
	}

	ts_platform_data = zinitix_dev_data->ts_platform_data;
	zinitix_pdata->zinitix_platform_dev = ts_platform_data->ts_dev;

#ifndef TARGET_HUAWEI_WEARABLES
	zinitix_dev_data->easy_wakeup_info.sleep_mode = TS_POWER_OFF_MODE;
	zinitix_dev_data->rawdata_arrange_swap = false;
	zinitix_dev_data->easy_wakeup_info.easy_wakeup_gesture = false;
	zinitix_dev_data->easy_wakeup_info.easy_wakeup_flag = false;
	zinitix_dev_data->easy_wakeup_info.palm_cover_flag = false;
	zinitix_dev_data->easy_wakeup_info.palm_cover_control = false;
#else
	/* enable easy wakeup gesture and palm cover report */
	zinitix_dev_data->easy_wakeup_info.sleep_mode = TS_GESTURE_MODE;
	zinitix_dev_data->easy_wakeup_info.easy_wakeup_flag = true;
	zinitix_dev_data->easy_wakeup_info.palm_cover_flag = true;
	zinitix_dev_data->easy_wakeup_info.palm_cover_control = true;
	ts_platform_data->feature_info.wakeup_gesture_enable_info.switch_value = true;
#endif
	zinitix_dev_data->easy_wakeup_info.off_motion_on = false;
	g_zinitix_dev_data->ts_platform_data->chip_data->need_drop_point = 0;
	ts_platform_data->feature_info.holster_info.holster_switch = false;

	ret = zinitix_touch_init(zinitix_pdata);
	if (ret < 0) {
		TS_LOG_ERR("%s:touch init error, ret=%d\n", __func__, ret);
		return ret;
	}

	ret = zinitix_get_vendor_name(zinitix_pdata);
	if (ret < 0) {
		TS_LOG_ERR("%s: get vendor name failed\n", __func__);
		return ret;
	}

	g_zinitix_pdata->driver_init_ok = true;
	TS_LOG_ERR("%s:init chip success.\n", __func__);
	ret = gpio_get_value(g_zinitix_dev_data->ts_platform_data->irq_gpio);
	TS_LOG_INFO("%s: gpio_get_value %d\n", __func__, ret);
	return NO_ERR;
}

int vci_vddio_enable(void)
{
	int ret = 0;

	if (g_zinitix_dev_data->vddio_regulator_type == ZINITIX_LDO_POWER) {
		ret = regulator_enable(g_zinitix_pdata->vddd);
		if (ret < 0) {
			TS_LOG_ERR("%s:failed to enable zinitix vddd, rc = %d\n",
					__func__, ret);
			goto enable_vddd_failed;
		}
		TS_LOG_INFO("%s:vddd enable success\n", __func__);
	}
	if (g_zinitix_dev_data->vddio_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vddio_gpio_ctrl)) {
			gpio_direction_output(g_zinitix_dev_data->vddio_gpio_ctrl,
					ZINITIX_GPIO_HIGH);
			TS_LOG_INFO("%s:vddd switch gpio on\n", __func__);
		}
	}

	if (g_zinitix_dev_data->vci_regulator_type == ZINITIX_LDO_POWER) {
		ret = regulator_enable(g_zinitix_pdata->vdda);
		if (ret < 0) {
			TS_LOG_ERR("%s:failed to enable zinitix vdda,rc = %d\n",
					__func__, ret);
			goto enable_vdda_failed;
		}
		TS_LOG_INFO("%s:vdda enable success\n", __func__);
	}
	if (g_zinitix_dev_data->vci_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vci_gpio_ctrl)) {
			gpio_direction_output(g_zinitix_dev_data->vci_gpio_ctrl,
					ZINITIX_GPIO_HIGH);
			TS_LOG_INFO("%s:vdda switch gpio on\n", __func__);
		}
	}
	return ret;
enable_vdda_failed:
	if (g_zinitix_dev_data->vci_regulator_type == ZINITIX_LDO_POWER)
		regulator_disable(g_zinitix_pdata->vdda);

	if (g_zinitix_dev_data->vddio_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vddio_gpio_ctrl))
			gpio_direction_output(g_zinitix_dev_data->vddio_gpio_ctrl,
					ZINITIX_GPIO_LOW);
	}
enable_vddd_failed:
	return ret;
}

static int zinitix_power_on(void)
{
	int ret;
	int sleep_time = 10;

	TS_LOG_ERR("%s:enter\n", __func__);
	if (g_zinitix_dev_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return -EINVAL;
	}
	if (g_zinitix_pdata->self_ctrl_power != ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s:power controlled by LCD\n", __func__);
		return 0;
	}
	if (g_zinitix_pdata->power_on == true) {
		TS_LOG_INFO("%s:device is powered on\n", __func__);
		return 0;
	}
	/* power on sequence */
	if (gpio_is_valid(g_zinitix_dev_data->ts_platform_data->reset_gpio))
		gpio_direction_output(g_zinitix_dev_data->ts_platform_data
				->reset_gpio, ZINITIX_GPIO_LOW);
	msleep(1);

	ret = vci_vddio_enable();
	if (ret) {
		TS_LOG_ERR("%s:vci/vddio enable failed\n", __func__);
		return ret;
	}

	ret = gpio_get_value(g_zinitix_dev_data->ts_platform_data->reset_gpio);
	TS_LOG_INFO("%s: gpio_get_value %d\n", __func__, ret);
	msleep(sleep_time);
	ret = gpio_is_valid(g_zinitix_dev_data->ts_platform_data->reset_gpio);
	TS_LOG_INFO("%s: reset_gpio %d valid %d\n", __func__,
			g_zinitix_dev_data->ts_platform_data->reset_gpio, ret);
	if (gpio_is_valid(g_zinitix_dev_data->ts_platform_data->reset_gpio)) {
		gpio_direction_output(g_zinitix_dev_data
				->ts_platform_data->reset_gpio,
				ZINITIX_GPIO_HIGH);
		ret = gpio_get_value(g_zinitix_dev_data
				->ts_platform_data->reset_gpio);
		TS_LOG_INFO("%s: valid gpio_get_value %d\n", __func__, ret);
	}
	if (gpio_is_valid(g_zinitix_dev_data->ts_platform_data->irq_gpio))
		gpio_direction_input(g_zinitix_dev_data->ts_platform_data->irq_gpio);

	ret = gpio_get_value(g_zinitix_dev_data->ts_platform_data->reset_gpio);
	TS_LOG_INFO("%s: gpio_get_value %d\n", __func__, ret);

	g_zinitix_pdata->power_on = true;
	TS_LOG_ERR("%s:power on success\n", __func__);
	return 0;
}

static void zinitix_power_off(void)
{
	int ret = 0;

	TS_LOG_ERR("%s:enter\n", __func__);
	if (g_zinitix_dev_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return;
	}

	if (g_zinitix_pdata->self_ctrl_power != ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s:power controlled by LCD\n", __func__);
		return;
	}

	if (g_zinitix_pdata->power_on == false) {
		TS_LOG_INFO("%s:device is powered off\n", __func__);
		return;
	}

	/* power on sequence */
	if (gpio_is_valid(g_zinitix_dev_data->ts_platform_data->reset_gpio))
		gpio_direction_output(g_zinitix_dev_data->ts_platform_data
				->reset_gpio, ZINITIX_GPIO_LOW);
	msleep(1);

	if (g_zinitix_dev_data->vci_regulator_type == ZINITIX_LDO_POWER) {
		ret = regulator_disable(g_zinitix_pdata->vdda);
		if (ret < 0) {
			TS_LOG_ERR("%s:failed to disable zinitix vdda, rc = %d\n",
					__func__, ret);
			return;
		}
		TS_LOG_INFO("%s:vdda disable success\n", __func__);
	} else if (g_zinitix_dev_data->vci_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vci_gpio_ctrl)) {
			gpio_direction_output(g_zinitix_dev_data->vci_gpio_ctrl,
					ZINITIX_GPIO_LOW);
			TS_LOG_INFO("%s:vdda switch gpio off\n", __func__);
		}
	}

	if (g_zinitix_dev_data->vddio_regulator_type == ZINITIX_LDO_POWER) {
		ret = regulator_disable(g_zinitix_pdata->vddd);
		if (ret < 0) {
			TS_LOG_ERR("%s:failed to disable zinitix vddd,rc = %d\n",
					__func__, ret);
			return;
		}
		TS_LOG_INFO("%s:vddd disable success\n", __func__);
	}
	if (g_zinitix_dev_data->vddio_gpio_type == ZINITIX_GPIO_POWER) {
		if (gpio_is_valid(g_zinitix_dev_data->vddio_gpio_ctrl)) {
			gpio_direction_output(g_zinitix_dev_data
					->vddio_gpio_ctrl, ZINITIX_GPIO_LOW);
			TS_LOG_INFO("%s:vddd switch gpio off\n", __func__);
		}
	}
	g_zinitix_pdata->power_on = false;
	TS_LOG_ERR("%s:power off success\n", __func__);
}

static int zinitix_regulator_get(void)
{
	if (g_zinitix_dev_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s:enter,vci_regulator_type=%d,vddio_regulator_type=%d\n",
			__func__,
			g_zinitix_dev_data->vci_regulator_type,
			g_zinitix_dev_data->vddio_regulator_type);

	if (g_zinitix_pdata->self_ctrl_power != ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s, power control by LCD, nothing to do\n",
				__func__);
		return 0;
	}

	TS_LOG_INFO("%s:power control by touch ic\n", __func__);
	/* get vci */
	if (g_zinitix_dev_data->vci_regulator_type == ZINITIX_LDO_POWER) {
		g_zinitix_pdata->vdda = regulator_get(&g_zinitix_dev_data
				->ts_platform_data->ts_dev->dev, "zts-vdd");
		if (IS_ERR(g_zinitix_pdata->vdda)) {
			TS_LOG_ERR("%s, regulator tp vdda not used\n",
					__func__);
			goto ts_vci_out;
		}
		TS_LOG_INFO("%s:regulator tp vdda get success\n", __func__);
	}

	/* get vddio */
	if (g_zinitix_dev_data->vddio_regulator_type == ZINITIX_LDO_POWER) {
		g_zinitix_pdata->vddd = regulator_get(&g_zinitix_dev_data
				->ts_platform_data->ts_dev->dev, "zts-vddio");
		if (IS_ERR(g_zinitix_pdata->vddd)) {
			TS_LOG_ERR("%s, regulator tp vddd not used\n",
					__func__);
			goto ts_vddio_out;
		}
		TS_LOG_INFO("%s:regulator tp vddd get success\n", __func__);
	}

	return 0;
ts_vddio_out:
	regulator_put(g_zinitix_pdata->vdda);
ts_vci_out:
	return -EINVAL;
}

static int zinitix_regulator_set(void)
{
	int ret = 0;

	if (g_zinitix_dev_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s:enter,vci_regulator_type=%d,vddio_regulator_type=%d\n",
			__func__,
			g_zinitix_dev_data->vci_regulator_type,
			g_zinitix_dev_data->vddio_regulator_type);

	if (g_zinitix_pdata->self_ctrl_power != ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s, power control by LCD, nothing to do\n",
				__func__);
		return 0;
	}

	TS_LOG_INFO("%s:power control by touch ic\n", __func__);
	/* set vci */
	if (g_zinitix_dev_data->vci_regulator_type == ZINITIX_LDO_POWER) {
		/* set voltage for vdda */
		ret = regulator_set_voltage(g_zinitix_pdata->vdda,
				g_zinitix_dev_data->regulator_ctr.vci_value,
				g_zinitix_dev_data->regulator_ctr.vci_value);
		if (ret < 0) {
			TS_LOG_ERR("%s:set vci voltage failed\n", __func__);
			return ret;
		}
	}

	/* set vddio */
	if (g_zinitix_dev_data->vddio_regulator_type == ZINITIX_LDO_POWER) {
		/* set voltage for vddd */
		ret = regulator_set_voltage(g_zinitix_pdata->vddd,
				g_zinitix_dev_data->regulator_ctr.vddio_value,
				g_zinitix_dev_data->regulator_ctr.vddio_value);
		if (ret < 0) {
			TS_LOG_ERR("%s:set vddio voltage failed\n", __func__);
			return ret;
		}
	}

	return 0;
}

static void zinitix_regulator_put(void)
{
	if (g_zinitix_dev_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return;
	}

	TS_LOG_INFO("%s:called,vci_regulator_type=%d,vddio_regulator_type=%d\n",
			__func__,
			g_zinitix_dev_data->vci_regulator_type,
			g_zinitix_dev_data->vddio_regulator_type);

	if (g_zinitix_pdata->self_ctrl_power == ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s, power control by touch ic\n", __func__);
		/* get vci */
		if ((g_zinitix_dev_data->vci_regulator_type ==
				ZINITIX_LDO_POWER) &&
				(!IS_ERR(g_zinitix_pdata->vdda))) {
			regulator_put(g_zinitix_pdata->vdda);
			TS_LOG_INFO("%s, reg_vddio put success\n", __func__);
		}

		if ((g_zinitix_dev_data->vddio_regulator_type ==
				ZINITIX_LDO_POWER) &&
				(!IS_ERR(g_zinitix_pdata->vddd))) {
			regulator_put(g_zinitix_pdata->vddd);
			TS_LOG_INFO("%s, reg_vddio put success\n", __func__);
		}
	} else {
		TS_LOG_INFO("%s, power control by LCD, nothing to do\n",
				__func__);
	}
}

static int zinitix_power_init(void)
{
	int ret = 0;

	TS_LOG_ERR("%s:power on start\n", __func__);
	if (g_zinitix_dev_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:param invalid\n", __func__);
		return -EINVAL;
	}

	if (g_zinitix_pdata->self_ctrl_power != ZINITIX_SELF_CTRL_POWER) {
		TS_LOG_INFO("%s, power control by LCD, nothing to do\n",
				__func__);
		return ret;
	}

	ret = zinitix_regulator_get();
	if (ret < 0) {
		TS_LOG_ERR("%s:get regulator failed, ret=%d\n",
				__func__, ret);
		goto exit_power_config;
	}
	TS_LOG_INFO("%s:get regulator success\n", __func__);

	ret = zinitix_gpio_request();
	if (ret < 0) {
		TS_LOG_ERR("%s:gpio reauest fail, ret=%d\n", __func__, ret);
		goto exit_regulator_put;
	}
	TS_LOG_INFO("%s:gpio request success\n", __func__);

	ret = zinitix_pinctrl_init();
	if (ret < 0) {
		TS_LOG_ERR("%s: zinitix_pinctrl_init error\n", __func__);
		goto exit_gpio_free;
	}

	ret = zinitix_regulator_set();
	if (ret < 0) {
		TS_LOG_ERR("%s:regulator set fail, ret=%d\n",
				__func__, ret);
		goto exit_gpio_free;
	}
	TS_LOG_INFO("%s:regulator set success\n", __func__);

	/* power on */
	ret = zinitix_power_on();
	if (ret < 0) {
		TS_LOG_ERR("%s: zinitix power on failed,ret=%d\n",
				__func__, ret);
		goto exit_gpio_free;
	}

	TS_LOG_ERR("%s:power on success\n", __func__);
	return 0;
exit_gpio_free:
	zinitix_pinctrl_release();
	zinitix_gpio_free();
exit_regulator_put:
	zinitix_regulator_put();
exit_power_config:
	return -EINVAL;
}

int zinitix_get_chip_id(void)
{
	int ret;
	u16 cmd;
	u16 value = 0;
	u16 enable_cmd[2] = { 0, 0 }; // enable ic cmd len is 2

	TS_LOG_INFO("%s:get chip id start\n", __func__);
	/* read chip id */
	cmd = ZINITIX_REG_CHIP_ID;
	ret = zinitix_read(&cmd, sizeof(cmd), (u8 *)&value, sizeof(value));
	if (value != ZINITIX_ZTW523_CHIP_ID && value != ZINITIX_ZTW622_CHIP_ID) {
		cmd = ZINITIX_SDRAM_RESET;
		zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
		msleep(ZINITIX_GENERAL_DELAY_10MS);
		enable_cmd[0] = ZINITIX_REG_VENDOR_EN;
		enable_cmd[1] = ZINITIX_CMD_REG_ENABLE;
		zinitix_write(&enable_cmd[0], sizeof(enable_cmd));
		cmd = ZINITIX_REG_CHIP_ID;
		ret = zinitix_read(&cmd, sizeof(cmd), (u8 *)&value, sizeof(value));
	}
	if (ret < 0) {
		TS_LOG_ERR("%s:chip id read fail, ret=%d\n",
				__func__, ret);
		return ret;
	}
	TS_LOG_INFO("%s:chip id read success, chip id:0x%X\n",
			__func__, value);
	g_zinitix_pdata->chip_id = value;
	TS_LOG_INFO("%s:get chip id success\n", __func__);
	/* set flash size */
	switch (g_zinitix_pdata->chip_id) {
	case ZINITIX_ZTW522_CHIP_ID:
	case ZINITIX_ZT7538_CHIP_ID:
		g_zinitix_pdata->flash_size = ZINITIX_ZTW522_FLASH_SIZE;
		break;
	case ZINITIX_ZTW523_CHIP_ID:
	case ZINITIX_ZT7548_CHIP_ID:
	case ZINITIX_ZTW622_CHIP_ID:
		g_zinitix_pdata->flash_size = ZINITIX_ZTW523_FLASH_SIZE;
		break;
	default:
		g_zinitix_pdata->flash_size = ZINITIX_ZT7554_FLASH_SIZE;
		break;
	}
	return ret;
}

static void zinitix_shutdown(void)
{
	TS_LOG_INFO("%s enter\n", __func__);
	zinitix_pinctrl_release();
	zinitix_power_off();
	zinitix_gpio_free();
	zinitix_regulator_put();

	TS_LOG_INFO("%s exit\n", __func__);
}

int g_zinitix_dev_data_init(struct ts_kit_platform_data *pdata)
{
	int ret = 0;

	if (g_zinitix_dev_data == NULL || g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s, param invalid\n", __func__);
		ret = -EINVAL;
		return ret;
	}
	g_zinitix_dev_data->ts_platform_data = pdata;
	TS_LOG_ERR("%s reset gpio = %d\n", __func__,
			g_zinitix_dev_data->ts_platform_data->reset_gpio);

	if ((!pdata) && (!pdata->ts_dev)) {
		TS_LOG_ERR("%s device, ts_kit_platform_data *data or data->ts_dev is NULL\n",
				__func__);
		ret = -ENOMEM;
		return ret;
	}

	g_zinitix_dev_data->is_i2c_one_byte = 0;
	g_zinitix_dev_data->is_new_oem_structure = 0;
	g_zinitix_dev_data->is_parade_solution = 0;
	g_zinitix_dev_data->ts_platform_data->ts_dev = pdata->ts_dev;
	g_zinitix_dev_data->ts_platform_data->ts_dev->dev.of_node
		= g_zinitix_dev_data->cnode;
	g_zinitix_pdata->open_threshold_status = true;
	g_zinitix_pdata->firmware_updating = false;
	g_zinitix_pdata->is_finish_update = false;
	g_zinitix_pdata->power_on = false;
	g_zinitix_pdata->irq_is_disable = false;
	g_zinitix_pdata->irq_processing = false;
	g_zinitix_pdata->suspend_state = false;
	g_zinitix_pdata->driver_init_ok = false;
	g_zinitix_pdata->flash_size = ZINITIX_ZTW523_FLASH_SIZE;
	g_zinitix_pdata->waking_up = 0;
	return ret;
}

int check_ic_firmware(void)
{
	u16 cmd;
	int ret;
	u16 fw_check_sum = 0;

	cmd = ZINITIX_CHECKSUM_RESULT;

	ret = zinitix_read(&cmd, ZINITIX_REG_ADDRESS_SIZE,
			(u8 *)&fw_check_sum, ZINITIX_REG_VALUE_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:read firmware checksum failed, ret=%d\n",
				__func__, ret);
		ret = -EINVAL;
		return ret;
	}

	/* firmware or cfg in flash is incomplete */
	if (fw_check_sum != ZINITIX_FW_CHECK_SUM) {
#if defined(CONFIG_HUAWEI_DSM)
		ts_dmd_report(DSM_TP_FW_CRC_ERROR_NO, " fw may be corrupted\n");
#endif

		ret = zinitix_fw_update_sd();
		if (ret < 0) {
			TS_LOG_ERR("%s:fw update forcely failed,ret=%d\n",
					__func__, ret);
#if defined(CONFIG_HUAWEI_DSM)
			ts_dmd_report(DSM_TP_FWUPDATE_ERROR_NO,
					"fw update result: failed.updata_status is:%d\n",
					g_zinitix_dev_data->ts_platform_data
					->chip_data->ts_platform_data
					->dsm_info.constraints_update_status);
#endif
			ret = -EINVAL;
		}
	}
	return ret;
}

static void zinitix_software_reset()
{
	int ret;
	u16 cmd;

	cmd = ZINITIX_SWRESET_CMD;
	ret = zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
	if (ret < 0) {
		TS_LOG_ERR("%s:software reset fail, ret=%d\n", __func__, ret);
		return;
	}
	/* send clear int cmd */
	msleep(ZINITIX_GENERAL_DELAY_100MS);
	cmd = ZINITIX_CLEAR_INT_CMD;
	zinitix_write(&cmd, ZINITIX_REG_ADDRESS_SIZE);
	msleep(ZINITIX_GENERAL_DELAY_20MS);
}

static int zinitix_chip_detect(struct ts_kit_platform_data *pdata)
{
	int ret;

	ret = g_zinitix_dev_data_init(pdata);
	if (ret)
		goto exit;

	ret = zinitix_parse_ic_config_dts(g_zinitix_dev_data->cnode,
			g_zinitix_dev_data);
	if (ret) {
		TS_LOG_ERR("%s:parse ic config dts fail, ret=%d\n",
				__func__, ret);
		goto exit;
	}
	/* i2c communication, get the i2c reg address */
	pdata->client->addr = g_zinitix_dev_data->slave_addr;
	g_zinitix_pdata->zinitix_device_data = g_zinitix_dev_data;
	ret = zinitix_power_init();
	if (ret < 0) {
		TS_LOG_ERR("%s: zts power init failed\n", __func__);
		goto exit;
	}
	ret = zinitix_get_chip_id();
	if (ret < 0) {
		TS_LOG_ERR("%s: get chip id failed\n", __func__);
		goto exit_power_off;
	}
	if (g_zinitix_pdata->chip_id == ZINITIX_ZTW523_CHIP_ID) {
		ret = zitinix_ic_enable();
		if (ret < 0) {
			TS_LOG_ERR("%s:not find zinitix device, ret=%d\n",
				__func__, ret);
			goto exit_power_off;
		} else {
			TS_LOG_INFO("%s:find zinitix device\n", __func__);
			strlcpy(g_zinitix_dev_data->chip_name,
				ZINITIX_CHIP_NAME, MAX_STR_LEN);
		}
	} else {
		strlcpy(g_zinitix_dev_data->chip_name, ZINITIX_CHIP_NAME, MAX_STR_LEN);
	}
	// set rate 120
	zinitix_set_rate();
	zinitix_software_reset();
	TS_LOG_ERR("%s:zinitix chip detect success\n", __func__);
	return 0;
exit_power_off:
	zinitix_shutdown();
exit:
	TS_LOG_INFO("%s:zinitix chip detect fail\n", __func__);
	return ret;
}

static int __init zinitix_core_module_init(void)
{
	int ret = NO_ERR;
	bool found = false;
	struct device_node *child = NULL;
	struct device_node *root = NULL;

	TS_LOG_INFO("%s: called\n", __func__);

	g_zinitix_pdata = kzalloc(sizeof(struct zinitix_platform_data),
			GFP_KERNEL);
	if (g_zinitix_pdata == NULL) {
		TS_LOG_ERR("%s:alloc mem for device data fail\n",
				__func__);
		ret = -ENOMEM;
		goto error_exit;
	}

	g_zinitix_dev_data =
		kzalloc(sizeof(struct ts_kit_device_data), GFP_KERNEL);
	if (g_zinitix_dev_data == NULL) {
		TS_LOG_ERR("%s:alloc mem for device data fail\n",
				__func__);
		ret = -ENOMEM;
		goto error_exit;
	}

	root = of_find_compatible_node(NULL, NULL, HUAWEI_TS_KIT);
	if (!root) {
		TS_LOG_ERR("%s:find_compatible_node error\n", __func__);
		ret = -EINVAL;
		goto error_exit;
	}

	for_each_child_of_node(root, child)	{
		if (of_device_is_compatible(child, ZINITIX_CHIP_NAME)) {
			found = true;
			break;
		}
	}

	if (!found) {
		TS_LOG_ERR("%s:device tree node not found, name=%s\n",
				__func__, ZINITIX_CHIP_NAME);
		ret = -EINVAL;
		goto error_exit;
	}

	g_zinitix_dev_data->cnode = child;
	g_zinitix_dev_data->ops = &ts_zinitix_ops;
	ret = huawei_ts_chip_register(g_zinitix_dev_data);
	if (ret) {
		TS_LOG_ERR("%s:chip register fail, ret=%d\n",
				__func__, ret);
		goto error_exit;
	}

	TS_LOG_INFO("%s:success\n", __func__);
	return 0;

error_exit:
	zinitix_clear_globals();
	TS_LOG_INFO("%s:fail\n", __func__);
	return ret;
}

static void __exit zinitix_ts_module_exit(void)
{
	zinitix_clear_globals();
}

late_initcall(zinitix_core_module_init);
module_exit(zinitix_ts_module_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
