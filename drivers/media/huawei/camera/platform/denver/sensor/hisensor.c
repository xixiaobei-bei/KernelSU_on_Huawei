/*
 * hisensor.c
 * Description:  Hisilicon Kirin camera driver source file
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/io.h>
#include <securec.h>

#include "hwsensor.h"
#include "sensor_commom.h"

#define intf_to_sensor(i) container_of(i, sensor_t, intf)
#define GPIO_LOW_STATE 0
#define GPIO_HIGH_STATE 1
#define CTL_RESET_HOLD 0
#define CTL_RESET_RELEASE 1

#define LPM3_GPU_BUCK_ADDR 0xFFF0A45C
#define LPM3_GPU_BUCK_MAP_BYTE 8

unsigned int *lpm3;

extern struct hw_csi_pad hw_csi_pad;

int hisensor_config(hwsensor_intf_t *si, void *argp);

struct mutex g_hisensor_power_lock;

char const *hisensor_get_name(hwsensor_intf_t *si)
{
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s si is null", __func__);
		return NULL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info) {
		cam_err("%s sensor or board_info->name is NULL", __func__);
		return NULL;
	}

	return sensor->board_info->name;
}
static int hisensor_do_hw_fpccheck(hwsensor_intf_t *si, void *data)
{
	int status = -1;
	int ret;
	sensor_t *sensor = NULL;
	hwsensor_board_info_t *b_info = NULL;
	struct sensor_cfg_data *cdata = NULL;
	unsigned int gpio_num = 0;

	if (!si || !data) {
		cam_err("%s si or data is NULL", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info || !sensor->board_info->name) {
		cam_err("%s sensor or board_info->name is NULL", __func__);
		return -EINVAL;
	}
	b_info = sensor->board_info;

	cam_info("%s enter", __func__);
	cdata = (struct sensor_cfg_data *)data;
	gpio_num = b_info->gpios[FSIN].gpio;
	cam_info("%s check gpio[%d]", __func__, gpio_num);
	if (gpio_num <= 0) {
		cam_err("%s gpio get failed", __func__);
		return status;
	}
	ret = gpio_request(gpio_num, NULL);
	if (ret < 0) {
		cam_err("%s request gpio[%d] failed", __func__, gpio_num);
		return ret;
	}
	ret = gpio_direction_input(gpio_num);
	if (ret < 0) {
		cam_err("%s gpio_direction_input failed", __func__);
		return ret;
	}
	status = gpio_get_value(gpio_num);
	cdata->data = status;
	if (status == GPIO_LOW_STATE) {
		cam_info("%s gpio[%d] status is %d, camera fpc is ok",
			__func__, gpio_num, status);
	} else if (status == GPIO_HIGH_STATE) {
		cam_err("%s gpio[%d] status is %d, camera fpc is broken",
			__func__, gpio_num, status);
	} else {
		cam_err("%s gpio[%d] status is %d, camera gpio_get_value is wrong",
			__func__, gpio_num, status);
	}
	gpio_free(gpio_num);
	return status;
}

int hisensor_power_up(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s si is null", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info || !sensor->board_info->name) {
		cam_err("%s sensor or board_info->name is NULL", __func__);
		return -EINVAL;
	}

	cam_info("enter %s index = %d name = %s", __func__,
		sensor->board_info->sensor_index, sensor->board_info->name);

	if (hw_is_fpga_board())
		ret = do_sensor_power_on(sensor->board_info->sensor_index,
			sensor->board_info->name);
	else
		ret = hw_sensor_power_up(sensor);

	if (ret == 0)
		cam_info("%s power up sensor success", __func__);
	else
		cam_err("%s power up sensor fail", __func__);

	return ret;
}

int hisensor_power_down(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s si is null", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info || !sensor->board_info->name) {
		cam_err("%s sensor or board_info->name is NULL", __func__);
		return -EINVAL;
	}

	cam_info("enter %s index = %d name = %s", __func__,
		sensor->board_info->sensor_index, sensor->board_info->name);

	if (hw_is_fpga_board())
		ret = do_sensor_power_off(sensor->board_info->sensor_index,
			sensor->board_info->name);
	else
		ret = hw_sensor_power_down(sensor);

	if (ret == 0)
		cam_info("%s power down sensor success", __func__);
	else
		cam_err("%s power down sensor fail", __func__);

	return ret;
}

int hisensor_csi_enable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

int hisensor_csi_disable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

int get_ext_name(sensor_t *sensor, struct sensor_cfg_data *cdata)
{
	int i;
	int j;
	int volt;
	int max;
	int min;

	if (!sensor || !cdata) {
		cam_err("%s si or data is NULL", __func__);
		return -EINVAL;
	}

	if (sensor->board_info->ext_type == EXT_INFO_NO_ADC) {
		cam_info("%s no adc channel, use ext_name config in overlay:%s",
			__func__, sensor->board_info->ext_name[0]);
		strncpy_s(cdata->info.extend_name, DEVICE_NAME_SIZE - 1,
			sensor->board_info->ext_name[0],
			strlen(sensor->board_info->ext_name[0])+1);
	}

	if (sensor->board_info->ext_type == EXT_INFO_ADC) {
		volt = hisi_adc_get_value(sensor->board_info->adc_channel);
		if (volt < 0) { // negative means get adc fail
			cam_err("get volt fail\n");
			return -EINVAL;
		}
		cam_info("get adc value = %d\n", volt);
		// each ext_name has two adc threshold
		for (i = 0, j = 0; (i < (sensor->board_info->ext_num - 1)) &&
			(j < EXT_NAME_NUM); i += 2, j++) {
			max = sensor->board_info->adc_threshold[i];
			min = sensor->board_info->adc_threshold[(unsigned int)(i + 1)];
			if ((volt < max) && (volt > min)) {
				cam_info("%s adc ext_name: %s\n", __func__,
					sensor->board_info->ext_name[j]);
				strncpy_s(cdata->info.extend_name,
					DEVICE_NAME_SIZE - 1,
					sensor->board_info->ext_name[j],
					strlen(sensor->board_info->ext_name[j]) + 1);
			}
		}
	}

	return 0;
}

static int hisensor_match_id(hwsensor_intf_t *si, void *data)
{
	sensor_t *sensor = NULL;
	struct sensor_cfg_data *cdata = NULL;

	if (!si || !data) {
		cam_err("%s si or data is NULL", __func__);
		return -EINVAL;
	}

	cam_info("%s enter", __func__);
	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info) {
		cam_err("%s sensor or board_info is NULL", __func__);
		return -EINVAL;
	}

	cdata = (struct sensor_cfg_data *)data;
	cdata->data = sensor->board_info->sensor_index;
	memset_s(cdata->info.extend_name, DEVICE_NAME_SIZE,
		0, DEVICE_NAME_SIZE);

	if (sensor->board_info->ext_type != 0)
		get_ext_name(sensor, cdata);

	return 0;
}

static hwsensor_vtbl_t g_hisensor_vtbl = {
	.get_name = hisensor_get_name,
	.config = hisensor_config,
	.power_up = hisensor_power_up,
	.power_down = hisensor_power_down,
	.match_id = hisensor_match_id,
	.csi_enable = hisensor_csi_enable,
	.csi_disable = hisensor_csi_disable,
};

enum camera_metadata_enum_android_hw_dual_primary_mode {
	ANDROID_HW_DUAL_PRIMARY_FIRST = 0,
	ANDROID_HW_DUAL_PRIMARY_SECOND = 2,
	ANDROID_HW_DUAL_PRIMARY_BOTH = 3,
};

#define RESET_TYPE_NONE 0
#define RESET_TYPE_PRIMARY 1
#define RESET_TYPE_SECOND 2
#define RESET_TYPE_BOTH 3

static void hisensor_deliver_camera_status(struct work_struct *work)
{
	rpc_info_t *rpc_info = NULL;

	if (!work) {
		cam_err("%s work is null", __func__);
		return;
	}

	rpc_info = container_of(work, rpc_info_t, rpc_work);
	if (!rpc_info) {
		cam_err("%s rpc_info is null", __func__);
		return;
	}
}

static int hisensor_do_pwdn(hwsensor_board_info_t *b_info, int ctl)
{
	int ret = gpio_request(b_info->gpios[PWDN].gpio, "pwdn-0");
	if (ret) {
		cam_err("%s requeset pwdn pin failed", __func__);
		return ret;
	}
	if (ctl == CTL_RESET_HOLD) { // stop streaming
		ret  = gpio_direction_output(b_info->gpios[PWDN].gpio,
			b_info->high_impedance_pwdn_val[HOLD_GPIO_VALUE]);
		hw_camdrv_msleep(b_info->high_impedance_pwdn_val[HOLD_GPIO_DELAY]);
		cam_info("%s set pwdn %u", __func__,
			b_info->high_impedance_pwdn_val[HOLD_GPIO_VALUE]);
	} else {
		ret  = gpio_direction_output(b_info->gpios[PWDN].gpio,
			b_info->high_impedance_pwdn_val[RELEASE_GPIO_VALUE]);
		hw_camdrv_msleep(b_info->high_impedance_pwdn_val[RELEASE_GPIO_DELAY]);
		cam_info("%s set pwdn %u", __func__,
			b_info->high_impedance_pwdn_val[RELEASE_GPIO_VALUE]);
	}
	gpio_free(b_info->gpios[PWDN].gpio);
	return ret;
}

int hisensor_do_reset_primary(hwsensor_board_info_t *b_info, int ctl)
{
	int ret;

	ret  = gpio_request(b_info->gpios[RESETB].gpio, "reset-0");
	if (ret) {
		cam_err("%s requeset reset pin failed", __func__);
		return ret;
	}

	if (ctl == CTL_RESET_HOLD) {
		ret  = gpio_direction_output(b_info->gpios[RESETB].gpio,
			CTL_RESET_HOLD);
	} else {
		ret  = gpio_direction_output(b_info->gpios[RESETB].gpio,
			CTL_RESET_RELEASE);
		hw_camdrv_msleep(2);
	}
	gpio_free(b_info->gpios[RESETB].gpio);
	return ret;
}

int hisensor_do_reset_second(hwsensor_board_info_t *b_info, int ctl, int id)
{
	int ret;

	ret = gpio_request(b_info->gpios[RESETB2].gpio, "reset-1");
	if (ret) {
		cam_err("%s requeset reset2 pin failed", __func__);
		return ret;
	}

	if (ctl == CTL_RESET_HOLD) {
		if ((id == ANDROID_HW_DUAL_PRIMARY_SECOND) ||
			(id == ANDROID_HW_DUAL_PRIMARY_BOTH)) {
			ret = gpio_direction_output(b_info->gpios[RESETB2].gpio,
				CTL_RESET_HOLD);
			cam_info("RESETB2 = CTL_RESET_HOLD");
		}
	} else if (ctl == CTL_RESET_RELEASE) {
		if ((id == ANDROID_HW_DUAL_PRIMARY_SECOND) ||
			(id == ANDROID_HW_DUAL_PRIMARY_BOTH)) {
			ret = gpio_direction_output(b_info->gpios[RESETB2].gpio,
				CTL_RESET_RELEASE);
			cam_info("RESETB2 = CTL_RESET_RELEASE");
			hw_camdrv_msleep(2);
		}
	}

	gpio_free(b_info->gpios[RESETB2].gpio);
	return ret;
}

int hisensor_do_reset_both(hwsensor_board_info_t *b_info, int ctl, int id)
{
	int ret;

	ret = gpio_request(b_info->gpios[RESETB].gpio, "reset-0");
	if (ret) {
		cam_err("%s:%d requeset reset pin failed", __func__, __LINE__);
		return ret;
	}

	ret = gpio_request(b_info->gpios[RESETB2].gpio, "reset-1");
	if (ret) {
		cam_err("%s:%d requeset reset pin failed",
			__func__, __LINE__);
		gpio_free(b_info->gpios[RESETB].gpio);
		return ret;
	}

	if (ctl == CTL_RESET_HOLD) {
		ret += gpio_direction_output(b_info->gpios[RESETB].gpio,
			CTL_RESET_HOLD);
		ret += gpio_direction_output(b_info->gpios[RESETB2].gpio,
			CTL_RESET_HOLD);
		cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_HOLD");
		udelay(2000);
	} else if (ctl == CTL_RESET_RELEASE) {
		if (id == ANDROID_HW_DUAL_PRIMARY_FIRST) {
			ret += gpio_direction_output(b_info->gpios[RESETB].gpio,
				CTL_RESET_RELEASE);
			ret += gpio_direction_output(b_info->gpios[RESETB2].gpio,
				CTL_RESET_HOLD);
			cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_HOLD");
		} else if (id == ANDROID_HW_DUAL_PRIMARY_BOTH) {
			ret += gpio_direction_output(b_info->gpios[RESETB].gpio,
				CTL_RESET_RELEASE);
			ret += gpio_direction_output(b_info->gpios[RESETB2].gpio,
				CTL_RESET_RELEASE);
			cam_info("RESETB = CTL_RESET_RELEASE, RESETB2 = CTL_RESET_RELEASE");
		} else if (id == ANDROID_HW_DUAL_PRIMARY_SECOND) {
			ret += gpio_direction_output(b_info->gpios[RESETB2].gpio,
				CTL_RESET_RELEASE);
			ret += gpio_direction_output(b_info->gpios[RESETB].gpio,
				CTL_RESET_HOLD);
			cam_info("RESETB = CTL_RESET_HOLD, RESETB2 = CTL_RESET_RELEASE");
		}
	}

	gpio_free(b_info->gpios[RESETB].gpio);
	gpio_free(b_info->gpios[RESETB2].gpio);
	return ret;
}

static int hisensor_do_hw_reset(hwsensor_intf_t *si, int ctl, int id)
{
	sensor_t *sensor = intf_to_sensor(si);
	hwsensor_board_info_t *b_info = NULL;
	int ret = 0;

	if (!sensor) {
		cam_err("%s sensor is NULL", __func__);
		return -EINVAL;
	}

	b_info = sensor->board_info;
	if (!b_info) {
		cam_warn("%s invalid sensor board info", __func__);
		return 0;
	}
	cam_info("reset_type = %d ctl %d id %d", b_info->reset_type, ctl, id);

	// camera radio power ctl for radio frequency interference
	if (b_info->need_rpc == 1) {
		if (ctl == CTL_RESET_RELEASE)
			b_info->rpc_info.camera_status = 1; // set rpc
		else
			b_info->rpc_info.camera_status = 0;

		if (b_info->rpc_info.rpc_work_queue)
			queue_work(b_info->rpc_info.rpc_work_queue,
				&(b_info->rpc_info.rpc_work));
	}
	// operate sensor pwdn; 1 for need, 0 for no need
	if (b_info->high_impedance_pwdn == 1)
		ret = hisensor_do_pwdn(b_info, ctl);
	if (b_info->reset_type == RESET_TYPE_PRIMARY)
		ret = hisensor_do_reset_primary(b_info, ctl);
	else if (b_info->reset_type == RESET_TYPE_SECOND)
		ret = hisensor_do_reset_second(b_info, ctl, id);
	else if (b_info->reset_type == RESET_TYPE_BOTH)
		ret = hisensor_do_reset_both(b_info, ctl, id);
	else
		return 0;

	if (ret)
		cam_err("%s set reset pin failed", __func__);
	else
		cam_info("%s: set reset state = %d, mode = %d",
			__func__, ctl, id);

	return ret;
}

static int hisensor_do_hw_mipisw(hwsensor_intf_t *si)
{
	sensor_t *sensor = NULL;
	hwsensor_board_info_t *b_info = NULL;
	int ret;
	int index;
	struct sensor_power_setting_array *power_setting_array = NULL;
	struct sensor_power_setting *power_setting = NULL;

	if (!si) {
		cam_err("%s si is null\n", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor) {
		cam_err("%s sensor is null\n", __func__);
		return -EINVAL;
	}

	b_info = sensor->board_info;
	if (!b_info) {
		cam_warn("%s invalid sensor board info", __func__);
		return 0;
	}

	cam_info("%s enter", __func__);
	power_setting_array = &sensor->power_setting_array; // SENSOR_MIPI_SW

	for (index = 0; index < power_setting_array->size; index++) {
		power_setting = &power_setting_array->power_setting[index];
		if (power_setting->seq_type == SENSOR_MIPI_SW)
			break;
	}

	ret  = gpio_request(b_info->gpios[MIPI_SW].gpio, NULL);
	if (ret) {
		cam_err("%s requeset reset pin failed", __func__);
		return ret;
	}

	if (!power_setting) {
		cam_warn("%s invalid sensor board info", __func__);
		return 0;
	}
	ret  = gpio_direction_output(b_info->gpios[MIPI_SW].gpio,
		(power_setting->config_val + 1) % 2);
	cam_info("mipi_sw = gpio[%d], config_val = %d, index = %d",
		b_info->gpios[MIPI_SW].gpio, power_setting->config_val, index);

	gpio_free(b_info->gpios[MIPI_SW].gpio);

	return ret;
}

int hisensor_config(hwsensor_intf_t *si, void *argp)
{
	int ret = 0;
	struct sensor_cfg_data *data = NULL;
	sensor_t *sensor = NULL;

	if (!si || !argp || !si->vtbl) {
		cam_err("%s si or argp is null\n", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor) {
		cam_err("%s sensor is null\n", __func__);
		return -EINVAL;
	}

	data = (struct sensor_cfg_data *)argp;
	cam_info("hisensor cfgtype = %d", data->cfgtype);

	switch (data->cfgtype) {
	case SEN_CONFIG_POWER_ON:
		mutex_lock(&g_hisensor_power_lock);
		if (sensor->state == HWSENSRO_POWER_DOWN) {
			if (!si->vtbl->power_up) {
				cam_err("%s si->vtbl->power_up is null",
					__func__);
				mutex_unlock(&g_hisensor_power_lock);
				return -EINVAL;
			}
			ret = si->vtbl->power_up(si);
			if (ret == 0)
				sensor->state = HWSENSOR_POWER_UP;
			else
				cam_err("%s power up fail", __func__);
		}
		mutex_unlock(&g_hisensor_power_lock);
		break;

	case SEN_CONFIG_POWER_OFF:
		mutex_lock(&g_hisensor_power_lock);
		if (sensor->state == HWSENSOR_POWER_UP) {
			if (!si->vtbl->power_down) {
				cam_err("%s si->vtbl->power_down is null",
					__func__);
				mutex_unlock(&g_hisensor_power_lock);
				return -EINVAL;
			}
			ret = si->vtbl->power_down(si);
			if (ret != 0)
				cam_err("%s power_down fail", __func__);
			sensor->state = HWSENSRO_POWER_DOWN;
		}
		mutex_unlock(&g_hisensor_power_lock);
		break;

	case SEN_CONFIG_WRITE_REG:
		break;

	case SEN_CONFIG_READ_REG:
		break;

	case SEN_CONFIG_WRITE_REG_SETTINGS:
		break;

	case SEN_CONFIG_READ_REG_SETTINGS:
		break;

	case SEN_CONFIG_ENABLE_CSI:
		break;

	case SEN_CONFIG_DISABLE_CSI:
		break;

	case SEN_CONFIG_MATCH_ID:
		if (!si->vtbl->match_id) {
			cam_err("%s si->vtbl->match_id is null",
				__func__);
			ret = -EINVAL;
		} else {
			ret = si->vtbl->match_id(si, argp);
		}
	break;

	case SEN_CONFIG_RESET_HOLD:
		ret = hisensor_do_hw_reset(si,
			CTL_RESET_HOLD, data->mode);
		break;

	case SEN_CONFIG_RESET_RELEASE:
		ret = hisensor_do_hw_reset(si,
			CTL_RESET_RELEASE, data->mode);
		break;

	case SEN_CONFIG_MIPI_SWITCH:
		ret = hisensor_do_hw_mipisw(si);
		break;

	case SEN_CONFIG_FPC_CHECK:
		ret = hisensor_do_hw_fpccheck(si, argp);
		break;
	default:
		cam_err("%s cfgtype = %d is error",
			__func__, data->cfgtype);
		break;
	}

	cam_debug("%s exit %d", __func__, ret);
	return ret;
}

static int32_t hisensor_platform_probe(struct platform_device *pdev)
{
	int rc;
	sensor_t *s_ctrl = NULL;
	atomic_t *hisensor_powered = NULL;
	hwsensor_board_info_t *b_info = NULL;

	cam_info("%s enter", __func__);

	if (!pdev) {
		cam_err("%s:%d pdev is null\n", __func__, __LINE__);
		return -EINVAL;
	}

	s_ctrl = kzalloc(sizeof(sensor_t), GFP_KERNEL);
	if (!s_ctrl) {
		cam_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		return -ENOMEM;
	}

	rc = hw_sensor_get_dt_data(pdev, s_ctrl);
	if (rc < 0) {
		cam_err("%s:%d no dt data rc %d", __func__, __LINE__, rc);
		rc = -ENODEV;
		goto hisensor_probe_fail;
	}

	rc = hw_sensor_get_dt_power_setting_data(pdev, s_ctrl);
	if (rc < 0) {
		cam_err("%s:%d no dt power setting data rc %d",
			__func__, __LINE__, rc);
		rc = -ENODEV;
		goto hisensor_probe_fail;
	}

	mutex_init(&g_hisensor_power_lock);
	hisensor_powered = kzalloc(sizeof(atomic_t), GFP_KERNEL);
	if (!hisensor_powered) {
		cam_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		goto hisensor_probe_fail;
	}

	s_ctrl->intf.vtbl = &g_hisensor_vtbl;
	s_ctrl->p_atpowercnt = hisensor_powered;
	s_ctrl->dev = &pdev->dev;
	rc = hwsensor_register(pdev, &(s_ctrl->intf));
	if (rc != 0) {
		cam_err("%s:%d hwsensor_register fail\n", __func__, __LINE__);
		goto hisensor_probe_fail;
	}

	rc = rpmsg_sensor_register(pdev, (void *)s_ctrl);
	if (rc != 0) {
		cam_err("%s:%d rpmsg_sensor_register fail\n",
			__func__, __LINE__);
		hwsensor_unregister(pdev);
		goto hisensor_probe_fail;
	}

	b_info = s_ctrl->board_info;
	if (b_info && (b_info->need_rpc == 1)) {
		b_info->rpc_info.rpc_work_queue =
			create_singlethread_workqueue("camera_radio_power_ctl");
		if (!b_info->rpc_info.rpc_work_queue) {
			cam_err("%s - create workqueue error", __func__);
		} else {
			INIT_WORK(&(b_info->rpc_info.rpc_work),
				hisensor_deliver_camera_status);
			cam_info("%s - create workqueue success", __func__);
		}
	}

	/*
	 * this code just for the bug at elle,
	 * this config is lpm3_gpu_buck in dts
	 */
	if (s_ctrl->board_info->lpm3_gpu_buck == 1) {
		if (!lpm3) {
			lpm3 = ioremap(LPM3_GPU_BUCK_ADDR,
				LPM3_GPU_BUCK_MAP_BYTE);
			if (!lpm3)
				cam_err("%s: ioremap failed", __func__);
			else
				cam_info("%s: ioremap success", __func__);
		} else {
			cam_info("%s: lpm3 is not NULL, continue", __func__);
		}
	} else {
		cam_info("%s: do not set lpm3_gpu_buck", __func__);
	}

	return rc;

hisensor_probe_fail:
	if (s_ctrl->power_setting_array.power_setting)
		kfree(s_ctrl->power_setting_array.power_setting);

	if (s_ctrl->power_down_setting_array.power_setting)
		kfree(s_ctrl->power_down_setting_array.power_setting);

	if (s_ctrl->board_info) {
		kfree(s_ctrl->board_info);
		s_ctrl->board_info = NULL;
	}
	if (s_ctrl->p_atpowercnt) {
		kfree((atomic_t *)s_ctrl->p_atpowercnt);
		s_ctrl->p_atpowercnt = NULL;
	}
	if (s_ctrl) {
		kfree(s_ctrl);
		s_ctrl = NULL;
	}

	return rc;
}

static int32_t hisensor_platform_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	sensor_t *s_ctrl = NULL;
	hwsensor_board_info_t *b_info = NULL;

	if (!sd) {
		cam_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	s_ctrl = (sensor_t *)v4l2_get_subdevdata(sd);
	if (!s_ctrl) {
		cam_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	if (s_ctrl->board_info && (s_ctrl->board_info->lpm3_gpu_buck == 1)) {
		if (lpm3) {
			iounmap(lpm3);
			lpm3 = NULL;
			cam_info("%s: iounmap success", __func__);
		} else {
			cam_info("%s: lpm3 is NULL, do not iounmap", __func__);
		}
	} else {
		cam_info("%s: do not lpm3 iounmap", __func__);
	}

	b_info = s_ctrl->board_info;
	if (b_info && (b_info->need_rpc == 1)) {
		if ((b_info->need_rpc == 1) &&
			b_info->rpc_info.rpc_work_queue) {
			destroy_workqueue(b_info->rpc_info.rpc_work_queue);
			b_info->rpc_info.rpc_work_queue = NULL;
			cam_info("%s - destroy workqueue success", __func__);
		}
	}

	rpmsg_sensor_unregister((void *)s_ctrl);

	hwsensor_unregister(pdev);

	if (s_ctrl->power_setting_array.power_setting)
		kfree(s_ctrl->power_setting_array.power_setting);

	if (s_ctrl->power_down_setting_array.power_setting)
		kfree(s_ctrl->power_down_setting_array.power_setting);

	if (s_ctrl->p_atpowercnt) {
		kfree((atomic_t *)s_ctrl->p_atpowercnt);
		s_ctrl->p_atpowercnt = NULL;
	}
	if (s_ctrl->board_info)
		kfree(s_ctrl->board_info);

	kfree(s_ctrl);

	return 0;
}

static const struct of_device_id g_hisensor_dt_match[] = {
	{
		.compatible = "huawei,sensor",
	}, {
	},
};

MODULE_DEVICE_TABLE(of, g_hisensor_dt_match);

static struct platform_driver g_hisensor_platform_driver = {
	.driver = {
		.name = "huawei,sensor",
		.owner = THIS_MODULE,
		.of_match_table = g_hisensor_dt_match,
	},

	.probe = hisensor_platform_probe,
	.remove = hisensor_platform_remove,
};

static int __init hisensor_init_module(void)
{
	cam_info("enter %s", __func__);
	return platform_driver_register(&g_hisensor_platform_driver);
}

static void __exit hisensor_exit_module(void)
{
	platform_driver_unregister(&g_hisensor_platform_driver);
}

module_init(hisensor_init_module);
module_exit(hisensor_exit_module);
MODULE_DESCRIPTION("hisensor");
MODULE_LICENSE("GPL v2");

