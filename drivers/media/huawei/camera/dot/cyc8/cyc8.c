/*
 * cyc8.c
 *
 * Description: cyc8 source file
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

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"
#include "../hwdot.h"
#include "cam_log.h"
#include "../../platform/sensor_commom.h"
#include "huawei_platform/sensor/hw_comm_pmic.h"

#define INVALID_GPIO      999
#define INVALID_I2C_INDEX (-1)
#define CYC8_GPIO_ENABLE  1
#define CYC8_GPIO_DISABLE 0

#define intf_to_dot(i) container_of(i, dot_t, intf)

enum dot_cyc8_pin_type_t {
	POWER_ENABLE = 0,
	MAX_PIN,
};

enum dot_cyc8_pin_status_t {
	FREED = 0,
	REQUESTED,
};

struct dot_cyc8_private_data_t {
	unsigned int pin[MAX_PIN];
};

static dot_t g_cyc8;
static struct platform_device *g_pdev;
static struct dot_cyc8_private_data_t g_cyc8_pdata;

static int cyc8_get_dt_data(const hwdot_intf_t *intf,
	struct device_node *dev_node)
{
	dot_t *dot = NULL;
	struct dot_cyc8_private_data_t *pdata = NULL;
	unsigned int pin_tmp[MAX_PIN] = {0};
	int rc;
	int i;
	unsigned int i2c_index = 0;

	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	if (!dev_node) {
		cam_err("%s invalid params dev_node", __func__);
		return -EINVAL;
	}

	dot = intf_to_dot(intf);

	if (!dot) {
		cam_err("%s invalid params dot", __func__);
		return -EINVAL;
	}

	if (!dot->pdata) {
		cam_err("%s invalid params dot->pata", __func__);
		return -EINVAL;
	}

	pdata = (struct dot_cyc8_private_data_t *)dot->pdata;

	rc = of_property_read_u32(dev_node, "huawei,i2c_index",
		(u32 *)&i2c_index);
	if (rc < 0) {
		cam_err("%s get dt i2c_index failed", __func__);
		dot->i2c_index = INVALID_I2C_INDEX;
		goto get_dt_error_handler;
	} else {
		dot->i2c_index = i2c_index;
		cam_info("%s i2c_index %u", __func__, dot->i2c_index);
	}

	rc = of_property_read_u32_array(dev_node, "huawei,gpio_pin",
		pin_tmp, MAX_PIN);
	if (rc < 0) {
		cam_err("%s get dt dot-pin failed", __func__);
		for (i = 0; i < MAX_PIN; i++)
			pdata->pin[i] = INVALID_GPIO;
		goto get_dt_error_handler;
	} else {
		for (i = 0; i < MAX_PIN; i++) {
			pdata->pin[i] = pin_tmp[i];
			cam_info("%s dot-pin[%d] = %u", __func__, i,
				pdata->pin[i]);
		}
	}

	return rc;

get_dt_error_handler:

	return rc;
}

int cyc8_get_thermal(const hwdot_intf_t *i, void *data)
{
	int rc;
	dot_thermal_data *dotdata = (dot_thermal_data *)data;

	if (!i || !data) {
		cam_err("%s cyc8 null pointer", __func__);
		return -1;
	}

	rc = hw_sensor_get_thermal("dot", &(dotdata->data));
	if (rc == -1) {
		cam_err("%s cyc8 get thermal fail", __func__);
		return -1;
	}

	return rc;
}


static int cyc8_init(const hwdot_intf_t *intf)
{
	dot_t *dot = NULL;

	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	dot = intf_to_dot(intf);

	if (!dot) {
		cam_err("%s invalid params dot", __func__);
		return -EINVAL;
	}

	if (!dot->pdata) {
		cam_err("%s invalid params dot->pdata", __func__);
		return -EINVAL;
	}

	return 0;
}

void cyc8_notify_error(uint32_t id);

int cyc8_power_on(const hwdot_intf_t *intf)
{
	cam_info("%s enter", __func__);
	return 0;
}

int cyc8_power_off(const hwdot_intf_t *intf)
{
	cam_info("%s enter", __func__);
	return 0;
}

char const *cyc8_get_name(const hwdot_intf_t *intf)
{
	dot_t *cyc8 = NULL;

	if (!intf) {
		cam_err("%s. intf is NULL", __func__);
		return NULL;
	}

	cyc8 = intf_to_dot(intf);

	if (!cyc8) {
		cam_err("%s. cyc8 is NULL", __func__);
		return NULL;
	}

	return cyc8->name;
}

void cyc8_notify_error(uint32_t id)
{
	hwdot_event_t cyc8_ev;

	cyc8_ev.kind = HWDOT_INFO_ERROR;
	cyc8_ev.data.error.id = id;
	cam_info("%s id = %x", __func__, id);
	hwdot_intf_notify_error(g_cyc8.notify, &cyc8_ev);
}


static ssize_t cyc8_thermal_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;
	int tmper = 0;

	rc = hw_sensor_get_thermal("dot", &tmper);
	if (rc == -1) {
		cam_err("%s cyc8 get thermal fail", __func__);
		return -1;
	}

	rc = scnprintf(buf, PAGE_SIZE, "%d", tmper);

	return rc;
}
extern int register_camerafs_attr(struct device_attribute *attr);
static struct device_attribute g_dev_attr_cyc8thermal =
	__ATTR(cyc8thermal, 0440, cyc8_thermal_show, NULL);

int cyc8_register_attribute(const hwdot_intf_t *intf, struct device *dev)
{
	int rc;

	if (!intf) {
		cam_err("%s intf is NULL", __func__);
		return -1;
	}

	rc = device_create_file(dev, &g_dev_attr_cyc8thermal);
	if (rc < 0) {
		cam_err("%s failed to creat cyc8 thermal attribute", __func__);
		goto err_out;
	}

	return 0;
err_out:
	return rc;
}

static hwdot_vtbl_t g_vtbl_cyc8 = {
	.get_name = cyc8_get_name,
	.power_on = cyc8_power_on,
	.power_off = cyc8_power_off,
	.dot_get_dt_data = cyc8_get_dt_data,
	.init = cyc8_init,
	.dot_register_attribute = cyc8_register_attribute,
	.get_thermal = cyc8_get_thermal,
};

static dot_t g_cyc8 = {
	.name = "cyc8",
	.intf = {
		.vtbl = &g_vtbl_cyc8,
	},
	.pdata = (void *)&g_cyc8_pdata,
};

static const struct of_device_id g_cyc8_dt_match[] = {
	{
		.compatible = "huawei,dot_cyc8",
		.data = &g_cyc8.intf,
	},
	{
	},
};

MODULE_DEVICE_TABLE(of, g_cyc8_dt_match);

static struct platform_driver g_cyc8_driver = {
	.driver = {
		.name = "huawei,cyc8",
		.owner = THIS_MODULE,
		.of_match_table = g_cyc8_dt_match,
	},
};

static int32_t cyc8_platform_probe(struct platform_device *pdev)
{
	cam_notice("%s enter", __func__);
	register_camerafs_attr(&g_dev_attr_cyc8thermal);
	g_pdev = pdev;
	return hwdot_register(pdev, &g_cyc8.intf, &g_cyc8.notify);
}

static int __init cyc8_init_module(void)
{
	int ret = 0;

	cam_notice("%s enter", __func__);

	ret = platform_driver_probe(&g_cyc8_driver,
		cyc8_platform_probe);

	return ret;
}

static void __exit cyc8_exit_module(void)
{
	platform_driver_unregister(&g_cyc8_driver);
	if (g_pdev) {
		hwdot_unregister(g_pdev);
		g_pdev = NULL;
	}
}

module_init(cyc8_init_module);
module_exit(cyc8_exit_module);
MODULE_DESCRIPTION("cyc8");
MODULE_LICENSE("GPL v2");
