/*
 * lm3644.c
 *
 * camera driver source file
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
#include <securec.h>
#include "hwcam_intf.h"
#include "../hwdriver_ic.h"
#include "cam_log.h"

#define INVALID_GPIO         999
#define INVALID_I2C_INDEX   (-1)
#define INVALID_IC_POSITION (-1)
#define LM3644_GPIO_ENABLE   1
#define LM3644_GPIO_DISABLE  0

#define DRIVER_IC_ENABLE_PIN  10
#define BUCKBOOST_ENABLE_PIN  8
#define DRV_IC1_NAME "drv_ic_lm3644_1"
#define DRV_IC2_NAME "drv_ic_lm3644_2"

#define intf_to_driveric(i) container_of(i, struct driveric_t, intf)

enum drv_ic_lm3644_pin_type_t {
	POWER_ENABLE = 0,
	RESET = 1,
	LM3644_MAX_GPIOPIN,
};

enum drv_ic_lm3644_pin_status_t {
	FREED = 0,
	REQUESTED,
};

enum powertype_t {
	DISABLE = 0,
	ENABLE = 1,
};

struct drv_ic_lm3644_pin_t {
	unsigned int pin_id;
	enum drv_ic_lm3644_pin_status_t pin_status;
};

struct drv_ic_lm3644_private_data_t {
	struct drv_ic_lm3644_pin_t pin[LM3644_MAX_GPIOPIN];
};

static int lm3644_get_dt_data(const struct hwdriveric_intf_t *intf,
	struct device_node *dev_node)
{
	struct driveric_t *drv_ic = NULL;
	struct drv_ic_lm3644_private_data_t *pdata = NULL;
	unsigned int pin_tmp[LM3644_MAX_GPIOPIN] = {0};
	int rc;
	int i;
	unsigned int position, i2c_index;

	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}
	if (!dev_node) {
		cam_err("%s invalid params dev_node", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);
	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (pdata) {
		drv_ic->pdata = (void *)pdata;
	} else {
		cam_err("fail to alloc _drv_ic_lm3644_private_data_t,out of memory");
		return -1;
	}

	rc = of_property_read_u32(dev_node, "huawei,position",
		(u32 *)&position);
	if (rc < 0) {
		cam_err("%s get dt position failed", __func__);
		drv_ic->position = INVALID_IC_POSITION;
		goto get_dt_error_handler;
	} else {
		drv_ic->position = position;
		cam_info("%s position %u", __func__, drv_ic->position);
	}

	rc = of_property_read_u32(dev_node, "huawei,i2c_index",
		(u32 *)&i2c_index);
	if (rc < 0) {
		cam_err("%s get dt i2c_index failed", __func__);
		drv_ic->i2c_index = INVALID_I2C_INDEX;
		goto get_dt_error_handler;
	} else {
		drv_ic->i2c_index = i2c_index;
		cam_info("%s i2c_index %u", __func__, drv_ic->i2c_index);
	}

	rc = of_property_read_u32_array(dev_node, "huawei,gpio_pin",
		pin_tmp, LM3644_MAX_GPIOPIN);
	if (rc < 0) {
		cam_err("%s get dt gpio-pin failed", __func__);
		for (i = 0; i < LM3644_MAX_GPIOPIN; i++) {
			pdata->pin[i].pin_id = INVALID_GPIO;
			pdata->pin[i].pin_status = FREED;
		}
		goto get_dt_error_handler;
	} else {
		for (i = 0; i < LM3644_MAX_GPIOPIN; i++) {
			pdata->pin[i].pin_id = pin_tmp[i];
			pdata->pin[i].pin_status = FREED;
			cam_info("%s gpio-pin[%d] = %u", __func__,
				i, pdata->pin[i].pin_id);
		}
	}

	return rc;
get_dt_error_handler:
	kfree(drv_ic->pdata);
	drv_ic->pdata = NULL;
	return rc;
}

static int drv_ic_lm3644_set_pin(
	struct drv_ic_lm3644_private_data_t *pdata,
	int state)
{
	int rc;
	int i;

	if (!pdata) {
		cam_err("%s invalid params pdata", __func__);
		return -EINVAL;
	}

	for (i = 0; i < LM3644_MAX_GPIOPIN; i++) {
		cam_debug("%s pin_id=%d, state=%d", __func__,
			pdata->pin[i].pin_id, state);

		if (pdata->pin[i].pin_id == INVALID_GPIO) {
			cam_err("%s gpio pin is err rc=%d", __func__,
				pdata->pin[i].pin_id);
			return -1;
		}
		rc = gpio_request(pdata->pin[i].pin_id, NULL);
		if (rc < 0) {
			cam_err("%s, gpio-request %d", __func__,
				pdata->pin[i].pin_id);
			return -1;
		}

		pdata->pin[i].pin_status = REQUESTED;
		rc = gpio_direction_output(pdata->pin[i].pin_id, state);
		if (rc < 0)
			cam_err("%s gpio output is err rc=%d", __func__, rc);

		gpio_free(pdata->pin[i].pin_id);
	}

	return rc;
}

static int lm3644_init(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *drv_ic = NULL;

	cam_info("%s init enter", __func__);

	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);
	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}
	if (!drv_ic->pdata) {
		cam_err("%s invalid params drv_ic->pdata", __func__);
		return -EINVAL;
	}

	cam_info("%s init success", __func__);
	return 0;
}

static int lm3644_power_on(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *drv_ic = NULL;
	struct drv_ic_lm3644_private_data_t *pdata = NULL;
	int rc;

	cam_info("%s power on enter", __func__);
	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);
	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}
	if (!drv_ic->pdata) {
		cam_err("%s invalid params drv_ic->pdata", __func__);
		return -EINVAL;
	}

	pdata = (struct drv_ic_lm3644_private_data_t *)drv_ic->pdata;

	rc = drv_ic_lm3644_set_pin(pdata, ENABLE);

	cam_info("%s power on end,rc=%d", __func__, rc);
	return rc;
}

static int lm3644_power_off(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *drv_ic = NULL;
	struct drv_ic_lm3644_private_data_t *pdata = NULL;
	int rc;

	cam_info("%s power off enter", __func__);
	if (!intf) {
		cam_err("%s invalid params intf", __func__);
		return -EINVAL;
	}

	drv_ic = intf_to_driveric(intf);
	if (!drv_ic) {
		cam_err("%s invalid params drv_ic", __func__);
		return -EINVAL;
	}
	if (!drv_ic->pdata) {
		cam_err("%s invalid params drv_ic->pdata", __func__);
		return -EINVAL;
	}

	pdata = (struct drv_ic_lm3644_private_data_t *)drv_ic->pdata;

	rc = drv_ic_lm3644_set_pin(pdata, DISABLE);

	cam_info("%s power off end", __func__);
	return rc;
}

char const *lm3644_get_name(const struct hwdriveric_intf_t *intf)
{
	struct driveric_t *lm3644 = NULL;
	char *name = NULL;

	lm3644 = intf_to_driveric(intf);

	if (!lm3644) {
		cam_err("%s. lm3644 is NULL", __func__);
		return NULL;
	}
	if (lm3644->position == 0)
		name = DRV_IC1_NAME;
	else
		name = DRV_IC2_NAME;

	cam_info("%s position = %d get name = %s", __func__,
		lm3644->position, name);
	return name;
}

static struct hwdriveric_vtbl_t g_vtbl_lm3644 = {
	.get_name = lm3644_get_name,
	.power_on = lm3644_power_on,
	.power_off = lm3644_power_off,
	.driveric_get_dt_data = lm3644_get_dt_data,
	.init = lm3644_init,
};

static struct driveric_t g_lm3644 = {
	.name  = "drv_ic_LM3644",
	.intf  = { .vtbl = &g_vtbl_lm3644, },
	.pdata = NULL,
};

static const struct of_device_id g_lm3644_dt_match[] = {
	{
		.compatible = "huawei,drv_ic_LM3644_1",
		.data = &g_lm3644.intf,
	},
	{
		.compatible = "huawei,drv_ic_LM3644_2",
		.data = &g_lm3644.intf,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, g_lm3644_dt_match);

static struct platform_driver g_lm3644_driver = {
	.driver = {
		.name = "huawei,drv_ic_LM3644",
		.owner = THIS_MODULE,
		.of_match_table = g_lm3644_dt_match,
	},
};

static int32_t lm3644_platform_probe(
	struct platform_device *pdev)
{
	struct driveric_t *drv_ic = NULL;
	char *name1 = "drv_ic_LM3644_1";
	char *name2 = "drv_ic_LM3644_2";
	char *name = NULL;
	unsigned int position = 0;
	int rc;
	int ret;

	cam_notice("%s enter", __func__);

	rc = of_property_read_u32(pdev->dev.of_node,
		"huawei,position", (u32 *)&position);
	if (rc != 0) {
		cam_err("%s fail to read huawei,position", __func__);
		return rc;
	}
	cam_info("%s to read huawei,position %d ", __func__, position);

	if (position == 0)
		name = name1;
	else
		name = name2;

	drv_ic = kzalloc(sizeof(*drv_ic), GFP_KERNEL);
	if (drv_ic) {
		drv_ic->position = position;
		ret = snprintf_s(drv_ic->name, sizeof(drv_ic->name),
			sizeof(drv_ic->name) - 1, "%s", name);
		if (ret < 0) {
			cam_err("snprintf_s failed %s()%d", __func__, __LINE__);
			kfree(drv_ic);
			return -EINVAL;
		}
		drv_ic->intf.vtbl = &g_vtbl_lm3644;
		if (hwdriveric_register(pdev, &(drv_ic->intf),
			&(drv_ic->notify)) != 0) {
			cam_err("fail to do hwdriveric_register");
			kfree(drv_ic);
			return -1;
		}
	} else {
		cam_err("fail to alloc struct driveric_t,out of memory");
		return -1;
	}
	return rc;
}

static int __init lm3644_init_module(void)
{
	int ret = 0;

	cam_notice("%s enter", __func__);

	ret = platform_driver_probe(&g_lm3644_driver,
		lm3644_platform_probe);

	return ret;
}

static void __exit lm3644_exit_module(void)
{
	platform_driver_unregister(&g_lm3644_driver);
}

module_init(lm3644_init_module);
module_exit(lm3644_exit_module);
MODULE_DESCRIPTION("lm3644");
MODULE_LICENSE("GPL v2");

