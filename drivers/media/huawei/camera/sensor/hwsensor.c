/*
 * hwsensor.c
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 *
 * Hisilicon K3 SOC camera driver source file
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
 */

#include <linux/compiler.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <media/huawei/camera.h>
#include <media/v4l2-subdev.h>
#include <media/huawei/sensor_cfg.h>
#include <linux/hisi/hw_cmdline_parse.h>
#include <linux/uaccess.h>

#include "hwcam_intf.h"
#include "hwsensor.h"
#include "sensor_commom.h"
#include "cam_log.h"
#include "securec.h"

struct hwsensor_t {
	struct v4l2_subdev subdev;
	struct platform_device *pdev;

	hwsensor_intf_t *intf;
	int cam_dev_num;
	struct mutex lock;
	cam_data_table_t *cfg;
};

#define SENSOR_POWER_DOWN 0
#define SENSOR_POWER_ON 1
#define GPIO_LOW_STATE 0
#define GPIO_HIGH_STATE 1
#define TOF_SWING_SENSOR_ID 21
#define dev_to_video_device(i) container_of((i), struct video_device, dev)
#define to_hwsensor_t(i) container_of((i), struct hwsensor_t, lock)
#define sd_to_sensor(sd) container_of(sd, struct hwsensor_t, subdev)
#define intf_to_sensor(i) container_of(i, sensor_t, intf)

struct hwsensor_t *g_s;

static int hw_sensor_subdev_internal_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct hwsensor_t *s = sd_to_sensor(sd);
	int rc = 0;
	struct sensor_cfg_data cdata = {0};

	cdata.cfgtype = SEN_CONFIG_POWER_OFF;

	if (!s) {
		cam_err("%s get s_strl error", __func__);
		return -1;
	}
	if (!s->intf || !s->intf->vtbl ||
		!s->intf->vtbl->config ||
		!s->intf->vtbl->csi_disable)
		return rc;

	rc = s->intf->vtbl->csi_disable(s->intf);
	rc += s->intf->vtbl->config(s->intf, (void *)(&cdata));

	cam_notice(" enter %s, return value %d", __func__, rc);
	return rc;
}

static int hwsensor_init(struct hwsensor_t *s_ctrl)
{
	return 0;
}

static int hwsensor_subdev_open(
	struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct hwsensor_t *s = NULL;

	s = sd_to_sensor(sd);
	hwsensor_init(s);
	return 0;
}

static int hwsensor_subdev_close(
	struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct hwsensor_t *s = NULL;
	cam_data_table_t *cfg = NULL;

	hw_sensor_subdev_internal_close(sd, fh);

	s = sd_to_sensor(sd);
	swap(s->cfg, cfg);

	return 0;
}

static long hwsensor_subdev_get_info(struct hwsensor_t *s, hwsensor_info_t *info)
{
	int i;
	int rc;
	sensor_t *sensor = NULL;

	if (!s || !info) {
		CAM_CFG_ERR("s or info is null");
		return -1;
	}

	sensor = intf_to_sensor(s->intf);
	rc = memset_s(info, sizeof(hwsensor_info_t),
		0, sizeof(hwsensor_info_t));
	if (rc != 0)
		cam_err("%s memset_s return fail\n", __func__);

	rc = strncpy_s(info->name, DEVICE_NAME_SIZE - 1,
		hwsensor_intf_get_name(s->intf),
		strlen(hwsensor_intf_get_name(s->intf)) + 1);
	if (rc != 0) {
		cam_err("%s sensor device name copy error\n", __func__);
		return -1;
	}
	info->vcm_enable = sensor->board_info->vcm_enable;

	if (info->vcm_enable) {
		rc = strncpy_s(info->vcm_name, DEVICE_NAME_SIZE - 1,
			sensor->board_info->vcm_name,
			strlen(sensor->board_info->vcm_name) + 1);
		if (rc != 0) {
			cam_err("%s vcm name copy error\n", __func__);
			return -1;
		}
	}

	if (sensor->board_info->sensor_spec &&
		*(sensor->board_info->sensor_spec) != '\0') {
		if (strncpy_s(info->sensor_spec, DEVICE_NAME_SIZE - 1,
			sensor->board_info->sensor_spec,
			strlen(sensor->board_info->sensor_spec) + 1) != 0)
			cam_err("%s sensor_spec copy error\n", __func__);
	}

	info->dev_id = s->cam_dev_num;
	info->mount_position =
		(hwsensor_position_kind_t)sensor->board_info->sensor_index;
	info->extisp_type = sensor->board_info->extisp_type;
	info->module_type = sensor->board_info->module_type;
	info->flash_pos_type = sensor->board_info->flash_pos_type;
	for (i = 0; i < CSI_NUM; i++) {
		info->csi_id[i] = sensor->board_info->csi_id[i];
		info->i2c_id[i] = sensor->board_info->i2c_id[i];
	}
	info->phyinfo_count = 0;
	if (sensor->board_info->phyinfo_valid > 0) {
		info->phyinfo_count = sensor->board_info->phyinfo_valid;
		if (memcpy_s(&info->phyinfo, sizeof(info->phyinfo),
			&sensor->board_info->phyinfo,
			sizeof(info->phyinfo)) != 0) {
			cam_err("%s camera phyinfo copy fail\n", __func__);
			return -1;
		}
	}

	if (sensor->board_info->bus_type) {
		rc = strncpy_s(info->bustype, DEVICE_BUS_TYPE_SIZE - 1,
			sensor->board_info->bus_type,
			strlen(sensor->board_info->bus_type) + 1);
		if (rc != 0) {
			cam_err("%s bus_type copy error\n", __func__);
			return -1;
		}
	} else {
		cam_info("%s maybe not support bus_type\n", __func__);
	}
	info->ao_i2c_index = sensor->board_info->ao_i2c_id;
	info->i2c_pad_type = sensor->board_info->i2c_pad_type;
	info->matchid_fail_retry_flag =
		sensor->board_info->matchid_fail_retry_flag;
	cam_info("%s bus_type %s, ao_i2c_id:%d, i2c_pad_type:%d matchid_fail_retry_flag:%d\n",
		__func__, info->bustype, info->ao_i2c_index,
		info->i2c_pad_type, info->matchid_fail_retry_flag);

#pragma GCC visibility push(default)
	CAM_CFG_ERR("%s, info_count = %d\n, for print, not err.\n"
		"is_master_sensor[0] = %d, is_master_sensor[1] = %d\n"
		"phy_id[0] = %d, phy_id[1] = %d\n"
		"phy_mode[0] = %d, phy_mode[1] = %d\n"
		"phy_freq_mode[0] = %d, phy_freq_mode[1] = %d\n"
		"phy_freq[0] = %d, phy_freq[1] = %d\n"
		"phy_work_mode[0] = %d, phy_work_mode[1] = %d",
		__func__, info->phyinfo_count,
		info->phyinfo.is_master_sensor[0],
		info->phyinfo.is_master_sensor[1],
		(int)info->phyinfo.phy_id[0], (int)info->phyinfo.phy_id[1],
		info->phyinfo.phy_mode[0], info->phyinfo.phy_mode[1],
		info->phyinfo.phy_freq_mode[0], info->phyinfo.phy_freq_mode[1],
		info->phyinfo.phy_freq[0], info->phyinfo.phy_freq[1],
		info->phyinfo.phy_work_mode[0], info->phyinfo.phy_work_mode[1]);
#pragma GCC visibility pop
	return 0;
}

static int tof_swing_position_switch(sensor_t *hisensor)
{
	int status = -1;
	int tof_gpio_num = 0;

	if (!hisensor) {
		cam_err("%s get hisensor error", __func__);
		return -1;
	}

	tof_gpio_num = hisensor->board_info->tof_gpio_num;
	if (tof_gpio_num == 0) {
		cam_err("%s tof_gpio_num get failed", __func__);
		return -1;
	}

	if (hw_sensor_power_up(hisensor) != 0) {
		cam_err("%s hw_sensor_power_up failed", __func__);
		goto swing_switch_fail;
	}

	if (gpio_request(tof_gpio_num, NULL) != 0) {
		cam_err("%s request gpio[%d] failed",
			__func__, tof_gpio_num);
		goto swing_switch_fail;
	}

	if (gpio_direction_input(tof_gpio_num) != 0) {
		cam_err("%s gpio_direction_input failed", __func__);
		goto swing_switch_fail;
	}

	status = gpio_get_value(tof_gpio_num);
	if (status == GPIO_LOW_STATE) {
		cam_info("%s.gpio[%d] status is %d, get tof swing camera1",
			__func__, tof_gpio_num, status);
	} else if (status == GPIO_HIGH_STATE) {
		cam_info("%s.gpio[%d] status is %d, get tof swing camera2",
			__func__, tof_gpio_num, status);
	} else {
		cam_err("%s.gpio[%d] status is %d, camera gpio_get_value is wrong",
			__func__, tof_gpio_num, status);
		goto swing_switch_fail;
	}

	if (hw_sensor_power_down(hisensor) != 0) {
		cam_err("%s hw_sensor_power_down failed", __func__);
		goto swing_switch_fail;
	}
	gpio_free(tof_gpio_num);

	return status;

swing_switch_fail:
	gpio_free(tof_gpio_num);
	hw_sensor_power_down(hisensor);
	return status;
}

int tof_swing_info_store(struct hwsensor_t *s)
{
	if (!s) {
		cam_err("%s hwsensor_t is NULL \n", __func__);
		return -1;
	}

	g_s = s;
	cam_info("tof_swing_info_store");

	return 0;
}

long tof_swing_sensor_ioctl(struct file *file,
	unsigned int cmd, unsigned long args)
{
	int ret = 0;
	hwsensor_intf_t *si = NULL;
	if (!file) {
		cam_err("%s file is NULL", __func__);
		return -EINVAL;
	}
	if (!g_s){
		cam_err("%s g_s is NULL", __func__);
		return -EINVAL;
	}

	if (!g_s->intf) {
		cam_err("%s g_s->intf is NULL", __func__);
		return -EINVAL;
	}

	cam_info("%s enter, cmd: %x", __func__, cmd);
	si = g_s->intf;

	switch (cmd) {
	case HWSENSOR_IOCTL_GET_INFO: {
		hwsensor_info_t hw_info = {0};
		ret = hwsensor_subdev_get_info(g_s, &hw_info);
		if (ret != 0) {
			cam_err("%s bus_type copy error", __func__);
			return -EINVAL;
		}
		if (copy_to_user((char *)((uintptr_t)args), &hw_info, sizeof(hwsensor_info_t))) {
			cam_err("%s failed to copy to user", __func__);
			return -EINVAL;
		}
	}
	break;
	case HWSENSOR_IOCTL_SENSOR_CFG: {
		struct sensor_cfg_data data = {0};
		if (copy_from_user((void *)&data,
			(void *)((uintptr_t)args), sizeof(struct sensor_cfg_data))) {
			cam_err("%s copy_from_user error", __func__);
			return -EINVAL;
		}
		ret = si->vtbl->config(si, (void *)&data);
	}
	break;
	default:
		cam_err("%s unknown cmd : %d", __func__, cmd);
		return -1;
	}

	return ret;
}

static long hwsensor_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	long rc = -EINVAL;
	struct hwsensor_t *s = NULL;

	if (!sd) {
		CAM_CFG_ERR("sd is NULL\n");
		return -EINVAL;
	}

	s = sd_to_sensor(sd);
	cam_debug("hwsensor cmd = %x", cmd);

	switch (cmd) {
	case HWSENSOR_IOCTL_GET_INFO:
		rc = hwsensor_subdev_get_info(s, arg);
		break;
	case HWSENSOR_IOCTL_SENSOR_CFG:
		if (s->intf->vtbl->config)
			rc = s->intf->vtbl->config(s->intf, arg);
		break;
	case HWSENSOR_IOCTL_OTP_CFG:
		if (s->intf->vtbl->otp_config)
			rc = s->intf->vtbl->otp_config(s->intf, arg);
		break;
	case HWSENSOR_IOCTL_GET_THERMAL:
		if (s->intf->vtbl->get_thermal)
			rc = s->intf->vtbl->get_thermal(s->intf, arg);
		break;
	default:
		CAM_CFG_ERR("invalid IOCTL CMD = %d\n", cmd);
		break;
	}
	return rc;
}

#ifdef CONFIG_COMPAT
static long hwsensor_usercopy(
	struct v4l2_subdev *sd,
	unsigned int cmd,
	void *kp,
	void __user *up)
{
	long rc = -EFAULT;

	rc = memset_s(kp, _IOC_SIZE(cmd), 0, _IOC_SIZE(cmd));
	if (rc != 0) {
		cam_err("%s memset_s return fail\n", __func__);
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(kp, up, _IOC_SIZE(cmd))) {
			cam_err("%s: copy in arg failed!\n", __func__);
			return -EFAULT;
		}
	}

	rc = hwsensor_subdev_ioctl(sd, cmd, kp);
	if (rc < 0){
		cam_err("%s subdev_ioctl failed!\n", __func__);
		return -EFAULT;
	}

	switch (_IOC_DIR(cmd)) {
	case _IOC_READ:
	case (_IOC_WRITE | _IOC_READ):
		if (copy_to_user(up, kp, _IOC_SIZE(cmd))) {
			cam_err("%s: copy back arg failed!\n", __func__);
			return -EFAULT;
		}
		break;
	default:
		break;
	}

	return rc;
}

static long hwsensor_subdev_compat_ioctl32(
	struct v4l2_subdev *sd,
	unsigned int cmd,
	unsigned long arg)
{
	long rc = -EFAULT;
	switch (cmd) {
	case HWSENSOR_IOCTL_GET_INFO:
	{
		hwsensor_info_t data;
		if (_IOC_SIZE(cmd) > sizeof(data)) {
			cam_err("%s: cmd size too large!\n", __func__);
			return -EINVAL;
		}

		rc = hwsensor_usercopy(sd, cmd, (void *)&data, (void __user *)arg);
		if (rc < 0)
			cam_err("%s: HWSENSOR_IOCTL_GET_INFO hwsensor_usercopy fail\n", __func__);
	}
	break;
	case HWSENSOR_IOCTL_SENSOR_CFG:
	{
		struct sensor_cfg_data data;
		if (_IOC_SIZE(cmd) > sizeof(data)) {
			cam_err("%s: cmd size too large!\n", __func__);
			return -EINVAL;
		}

		rc = hwsensor_usercopy(sd, cmd, (void *)&data, (void __user *)arg);
		if (rc < 0)
			cam_err("%s: HWSENSOR_IOCTL_SENSOR_CFG hwsensor_usercopy fail\n", __func__);
	}
	break;
	case HWSENSOR_IOCTL_OTP_CFG:
	{
		hwsensor_config_otp_t data;
		if (_IOC_SIZE(cmd) > sizeof(data)) {
			cam_err("%s: cmd size too large!\n", __func__);
			return -EINVAL;
		}

		rc = hwsensor_usercopy(sd, cmd, (void *)&data, (void __user *)arg);
		if (rc < 0)
			cam_err("%s: HWSENSOR_IOCTL_OTP_CFG hwsensor_usercopy fail\n", __func__);
	}
	break;
	case HWSENSOR_IOCTL_GET_THERMAL:
	{
		sensor_thermal_data data;
		if (_IOC_SIZE(cmd) > sizeof(data)) {
			cam_err("%s: cmd size too large!\n", __func__);
			return -EINVAL;
		}

		rc = hwsensor_usercopy(sd, cmd, (void *)&data, (void __user *)arg);
		if (rc < 0)
			cam_err("%s: HWSENSOR_IOCTL_GET_THERMAL hwsensor_usercopy fail\n", __func__);
	}
	break;
	default:
		CAM_CFG_ERR("invalid IOCTL CMD:%d! \n", cmd);
		break;
	}

	return rc;
}
#endif

static int hwsensor_power(struct v4l2_subdev *sd, int on)
{
	return 0;
}

static struct v4l2_subdev_internal_ops s_subdev_internal_ops_hwsensor = {
	.open = hwsensor_subdev_open,
	.close = hwsensor_subdev_close,
};

static struct v4l2_subdev_core_ops s_subdev_core_ops_hwsensor = {
	.ioctl = hwsensor_subdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = hwsensor_subdev_compat_ioctl32,
#endif
	.s_power = hwsensor_power,
};

static struct v4l2_subdev_video_ops s_subdev_video_ops_hwsensor = {};

static struct v4l2_subdev_ops s_subdev_ops_hwsensor = {
	.core = &s_subdev_core_ops_hwsensor,
	.video = &s_subdev_video_ops_hwsensor,
};

static ssize_t hw_sensor_powerctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	cam_info("enter %s", __func__);
	return 1;
}

static ssize_t hw_sensor_powerctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct video_device *devnode = NULL;
	struct mutex *lock = NULL;
	struct hwsensor_t *sensor = NULL;
	hwsensor_intf_t *intf = NULL;
	int state;

	if (!dev || !attr || !buf) {
		cam_err("%s dev or attr or buf is NULL", __func__);
		return -EINVAL;
	}
	state = simple_strtol(buf, NULL, 10);
	cam_info("enter %s, state %d", __func__, state);
	devnode = dev_to_video_device(dev);
	if (!devnode) {
		cam_err("%s devnode is NULL", __func__);
		return -EINVAL;
	}
	lock = devnode->lock;
	if (!lock) {
		cam_err("%s lock is NULL", __func__);
		return -EINVAL;
	}
	sensor = to_hwsensor_t(lock);
	if (!sensor) {
		cam_err("%s sensor is NULL", __func__);
		return -EINVAL;
	}
	intf = sensor->intf;
	if (!intf) {
		cam_err("%s intf is NULL", __func__);
		return -EINVAL;
	}
	if (intf->vtbl) {
		int rc;

		if (state == SENSOR_POWER_ON && intf->vtbl->power_up) {
			rc = intf->vtbl->power_up(intf);
			cam_info("%s sensor power up, rc = %d", __func__, rc);
		} else if (state == SENSOR_POWER_DOWN &&
			intf->vtbl->power_down) {
			rc = intf->vtbl->power_down(intf);
			cam_info("%s sensor power down, rc = %d", __func__, rc);
		}
	}
	return count;
}

static struct device_attribute hw_sensor_powerctrl =
	__ATTR(sensor_power_ctrl, 0664, hw_sensor_powerctrl_show,
	hw_sensor_powerctrl_store);

int hw_camera_register_attribute(hwsensor_intf_t *intf, struct device *dev)
{
	int ret;

	cam_info("enter %s", __func__);
	if (!intf || !dev) {
		cam_err("%s intf or dev is NULL", __func__);
		return -EINVAL;
	}
	ret = device_create_file(dev, &hw_sensor_powerctrl);
	if (ret < 0) {
		cam_err("%s failed to create power ctrl attribute", __func__);
		return ret;
	}
	return 0;
}

int hwsensor_register(struct platform_device *pdev, hwsensor_intf_t *si)
{
	int rc = 0;
	struct v4l2_subdev *subdev = NULL;
	struct hwsensor_t *sensor = NULL;
	sensor_t *hisensor = NULL;
	int tof_gpio_state;

	if (!pdev || !si) {
		cam_err("%s pdev or si is NULL", __func__);
		return -EINVAL;
	}

	hisensor = intf_to_sensor(si);
	if (!hisensor) {
		cam_err("%s hisensor is NULL", __func__);
		return -EINVAL;
	}

	sensor = (struct hwsensor_t *)kzalloc(sizeof(struct hwsensor_t), GFP_KERNEL);
	if (!sensor) {
		rc = -ENOMEM;
		goto register_fail;
	}

	subdev = &sensor->subdev;
	mutex_init(&sensor->lock);

	v4l2_subdev_init(subdev, &s_subdev_ops_hwsensor);
	subdev->internal_ops = &s_subdev_internal_ops_hwsensor;
	snprintf_s(subdev->name, sizeof(subdev->name),
		sizeof(subdev->name) - 1,
		"%s", hwsensor_intf_get_name(si));
	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	v4l2_set_subdevdata(subdev, hisensor);
	platform_set_drvdata(pdev, subdev);

	init_subdev_media_entity(subdev, CAM_SUBDEV_SENSOR);
	cam_cfgdev_register_subdev(subdev, CAM_SUBDEV_SENSOR);
	subdev->devnode->lock = &sensor->lock;

	cam_dev_create(&pdev->dev, &sensor->cam_dev_num);
	sensor->intf = si;
	sensor->pdev = pdev;
	sensor->cfg = NULL;

	if (runmode_is_factory()) { // just for factory
		rc = hw_camera_register_attribute(si, &subdev->devnode->dev);
		if (rc < 0) {
			cam_err("%s failed to register camera attribute node",
				__func__);
			return rc;
		}
	}
	if (si->vtbl->sensor_register_attribute)
		rc = si->vtbl->sensor_register_attribute(si, &subdev->devnode->dev);
	if (hisensor->board_info->sensor_index == TOF_SWING_SENSOR_ID) {
		tof_gpio_state = tof_swing_position_switch(hisensor);
		if ((tof_gpio_state == GPIO_HIGH_STATE &&
			hisensor->board_info->tof_swing_num == TOF_SWING_NUM_2) ||
			(tof_gpio_state == GPIO_LOW_STATE &&
			hisensor->board_info->tof_swing_num == TOF_SWING_NUM_1)) {
			cam_info("%s get swing tof sensor dev, on_position sensor is %s",
				__func__, hisensor->board_info->name);
			rc = tof_swing_info_store(sensor);
		}
	}

register_fail:
	return rc;
}

void hwsensor_unregister(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev = platform_get_drvdata(pdev);
	struct hwsensor_t *sensor = sd_to_sensor(subdev);

	media_entity_cleanup(&subdev->entity);
	cam_cfgdev_unregister_subdev(subdev);

	kzfree(sensor);
}
