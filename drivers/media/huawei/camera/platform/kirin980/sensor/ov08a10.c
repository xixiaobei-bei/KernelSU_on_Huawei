/*
 * ov08a10.c
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

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include "hwsensor.h"
#include "sensor_commom.h"
#include "../pmic/hw_pmic.h"

#define intf_to_sensor(i) container_of(i, sensor_t, intf)
#define sensor_to_pdev(s) container_of((s).dev, struct platform_device, dev)
#define POWER_DELAY_0        0 /* delay 0 ms */
#define POWER_DELAY_1        1 /* delay 1 ms */
#define CTL_RESET_HOLD       0
#define CTL_RESET_RELEASE    1

static bool g_power_on_status; /* false: power off, true:power on */
struct mutex g_ov08a10_power_lock;

static char const *ov08a10_get_name(hwsensor_intf_t *si);
static int ov08a10_config(hwsensor_intf_t *si, void  *argp);
static int ov08a10_power_up(hwsensor_intf_t *si);
static int ov08a10_power_down(hwsensor_intf_t *si);
static int ov08a10_match_id(hwsensor_intf_t *si, void *data);
static int ov08a10_csi_enable(hwsensor_intf_t *si);
static int ov08a10_csi_disable(hwsensor_intf_t *si);
extern int hw_sensor_ldo_config(
	ldo_index_t ldo,
	hwsensor_board_info_t *sensor_info,
	struct sensor_power_setting *power_setting,
	int state);

static hwsensor_vtbl_t g_ov08a10_vtbl = {
	.get_name = ov08a10_get_name,
	.config = ov08a10_config,
	.power_up = ov08a10_power_up,
	.power_down = ov08a10_power_down,
	.match_id = ov08a10_match_id,
	.csi_enable = ov08a10_csi_enable,
	.csi_disable = ov08a10_csi_disable,
};

static struct platform_device *g_pdev;
static sensor_t *g_sensor;

struct sensor_power_setting g_buck2_power_up = {
	.seq_type = SENSOR_MISP_VDD,
	.config_val = LDO_VOLTAGE_V1P25V,
	.sensor_index = SENSOR_INDEX_INVALID,
	.delay = 0,
};

struct sensor_power_setting g_hw_ov08a10_power_up_setting[] = {
	{
		.seq_type = SENSOR_RXDPHY_CLK,
		.delay = 1,
	},
	/* TCAM DVDD 1.2V */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P205V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},

	/* SENSOR IOVDD 1.8V VOUT21 */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* AFVDD GPIO 255 ENABLE */
	{
		.seq_type = SENSOR_VCM_PWDN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_AVDD1_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_2,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM_DRVVDD_2V8 */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_4,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_MCLK,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_RST,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},

};

atomic_t volatile g_ov08a10_powered = ATOMIC_INIT(0);
sensor_t g_ov08a10 = {
	.intf = { .vtbl = &g_ov08a10_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_hw_ov08a10_power_up_setting),
		.power_setting = g_hw_ov08a10_power_up_setting,
	},
	.p_atpowercnt = &g_ov08a10_powered,
};

const struct of_device_id g_ov08a10_dt_match[] = {
	{
		.compatible = "huawei,ov08a10",
		.data = &g_ov08a10.intf,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, g_ov08a10_dt_match);

struct platform_driver g_ov08a10_driver = {
	.driver = {
		.name = "huawei,ov08a10",
		.owner = THIS_MODULE,
		.of_match_table = g_ov08a10_dt_match,
	},
};

static char const *ov08a10_get_name(hwsensor_intf_t *si)
{
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is NULL", __func__);
		return NULL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info || !sensor->board_info->name) {
		cam_err("%s. sensor or board_info->name is NULL", __func__);
		return NULL;
	}
	return sensor->board_info->name;
}

static int ov08a10_power_up(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is NULL", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info || !sensor->board_info->name) {
		cam_err("%s. sensor or board_info->name is NULL", __func__);
		return -EINVAL;
	}
	cam_info("enter %s. index = %d name = %s", __func__,
		sensor->board_info->sensor_index,
		sensor->board_info->name);

	if (hw_is_fpga_board())
		ret = do_sensor_power_on(sensor->board_info->sensor_index,
			sensor->board_info->name);
	else
		ret = hw_sensor_power_up(sensor);

	if (ret == 0)
		cam_info("%s. power up sensor success", __func__);
	else
		cam_err("%s. power up sensor fail", __func__);

	return ret;
}

static int ov08a10_power_down(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is NULL", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info || !sensor->board_info->name) {
		cam_err("%s. sensor or board_info->name is NULL", __func__);
		return -EINVAL;
	}
	cam_info("enter %s. index = %d name = %s", __func__,
		sensor->board_info->sensor_index,
		sensor->board_info->name);
	if (hw_is_fpga_board())
		ret = do_sensor_power_off(sensor->board_info->sensor_index,
			sensor->board_info->name);
	else
		ret = hw_sensor_power_down(sensor);

	if (ret == 0)
		cam_info("%s. power down sensor success", __func__);
	else
		cam_err("%s. power down sensor fail", __func__);

	return ret;
}

static int ov08a10_csi_enable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

static int ov08a10_csi_disable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

static int ov08a10_do_hw_reset(hwsensor_intf_t *si, int ctl, int id)
{
	char *state = NULL;
	sensor_t *sensor = intf_to_sensor(si);
	hwsensor_board_info_t *b_info = NULL;
	int ret;

	b_info = sensor->board_info;
	if (!b_info) {
		cam_warn("%s invalid sensor board info", __func__);
		return 0;
	}

	ret  = gpio_request(b_info->gpios[RESETB].gpio, "ov08a10reset-0");
	if (ret) {
		cam_err("%s requeset reset pin failed", __func__);
		return ret;
	}

	if (ctl == CTL_RESET_HOLD) {
		state = "hold";
		ret  = gpio_direction_output(b_info->gpios[RESETB].gpio, 0);
	} else {
		state = "release";
		ret  = gpio_direction_output(b_info->gpios[RESETB].gpio, 1);
		udelay(2000); /* delay 2000us */
	}
	gpio_free(b_info->gpios[RESETB].gpio);

	if (ret)
		cam_err("%s set reset pin failed", __func__);
	else
		cam_info("%s: set reset state=%s", __func__, state);
	return ret;
}

static int ov08a10_match_id(hwsensor_intf_t *si, void *data)
{
	sensor_t *sensor = NULL;
	struct sensor_cfg_data *cdata = NULL;

	cam_info("%s enter", __func__);

	if ((!si) || (!data)) {
		cam_err("%s. si or data is NULL", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	if (!sensor || !sensor->board_info || !sensor->board_info->name) {
		cam_err("%s. sensor or board_info is NULL", __func__);
		return -EINVAL;
	}
	cdata  = (struct sensor_cfg_data *)data;
	cdata->data = sensor->board_info->sensor_index;
	cam_info("%s name:%s", __func__, sensor->board_info->name);

	return 0;
}

static int ov08a10_config(hwsensor_intf_t *si, void  *argp)
{
	struct sensor_cfg_data *data = NULL;
	int ret = 0;

	if (!si || !argp || !si->vtbl) {
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}

	data = (struct sensor_cfg_data *)argp;
	cam_debug("ov08a10 cfgtype = %d", data->cfgtype);
	switch (data->cfgtype) {
	case SEN_CONFIG_POWER_ON:
		mutex_lock(&g_ov08a10_power_lock);
		if (g_power_on_status == false) {
			ret = si->vtbl->power_up(si);
			if (ret == 0)
				g_power_on_status = true;
			else
				cam_err("%s. power up fail", __func__);
		}
		mutex_unlock(&g_ov08a10_power_lock);
		break;
	case SEN_CONFIG_POWER_OFF:
		mutex_lock(&g_ov08a10_power_lock);
		if (g_power_on_status == true) {
			ret = si->vtbl->power_down(si);
			if (ret != 0)
				cam_err("%s. power_down fail", __func__);
			g_power_on_status = false;
		}
		mutex_unlock(&g_ov08a10_power_lock);
	break;
	case SEN_CONFIG_MATCH_ID:
		ret = si->vtbl->match_id(si, argp);
		break;
	case SEN_CONFIG_RESET_HOLD:
		ret = ov08a10_do_hw_reset(si, CTL_RESET_HOLD, data->mode);
		break;
	case SEN_CONFIG_RESET_RELEASE:
		ret = ov08a10_do_hw_reset(si, CTL_RESET_RELEASE, data->mode);
		break;
	default:
		cam_debug("%s cfgtype %d is error", __func__, data->cfgtype);
		break;
	}
	return ret;
}

int32_t ov08a10_platform_probe(struct platform_device *pdev)
{
	int rc = 0;
	const struct of_device_id *id = NULL;
	hwsensor_intf_t *intf = NULL;
	sensor_t *sensor = NULL;
	struct device_node *np = NULL;
	struct regulator *regulator_buck = NULL;

	cam_notice("enter %s", __func__);

	if (!pdev) {
		cam_err("%s pdev is NULL", __func__);
		return -EINVAL;
	}

	mutex_init(&g_ov08a10_power_lock);
	np = pdev->dev.of_node;
	if (!np) {
		cam_err("%s of_node is NULL", __func__);
		return -ENODEV;
	}

	id = of_match_node(g_ov08a10_dt_match, np);
	if (!id) {
		cam_err("%s none id matched", __func__);
		return -ENODEV;
	}

	intf = (hwsensor_intf_t *)id->data;
	if (!intf) {
		cam_err("%s intf is NULL", __func__);
		return -ENODEV;
	}

	sensor = intf_to_sensor(intf);
	if (!sensor) {
		cam_err("%s sensor is NULL rc %d", __func__, rc);
		return -ENODEV;
	}
	rc = hw_sensor_get_dt_data(pdev, sensor);
	if (rc < 0) {
		cam_err("%s no dt data rc %d", __func__, rc);
		return -ENODEV;
	}

	sensor->dev = &pdev->dev;
	rc = hwsensor_register(pdev, intf);
	if (rc < 0) {
		cam_err("%s hwsensor_register failed rc %d\n", __func__, rc);
		return -ENODEV;
	}
	g_pdev = pdev;

	rc = rpmsg_sensor_register(pdev, (void *)sensor);
	if (rc < 0) {
		hwsensor_unregister(g_pdev);
		g_pdev = NULL;
		cam_err("%s rpmsg_sensor_register failed rc %d\n",
			__func__, rc);
		return -ENODEV;
	}
	g_sensor = sensor;
	regulator_buck = devm_regulator_get(&pdev->dev, "buck2");
	if (IS_ERR(regulator_buck)) {
		rc = -ENODEV;
		cam_err("Get buck2 failed,ret:%d", rc);
		return rc;
	}
	hw_sensor_ldo_config(LDO_MISP, sensor->board_info,
		&g_buck2_power_up, POWER_ON);

	return rc;
}

int __init ov08a10_init_module(void)
{
	cam_notice("enter %s", __func__);
	return platform_driver_probe(&g_ov08a10_driver,
			ov08a10_platform_probe);
}

void __exit ov08a10_exit_module(void)
{
	if (g_sensor) {
		rpmsg_sensor_unregister((void *)g_sensor);
		g_sensor = NULL;
	}

	if (g_pdev) {
		hwsensor_unregister(g_pdev);
		g_pdev = NULL;
	}
	platform_driver_unregister(&g_ov08a10_driver);
}

module_init(ov08a10_init_module);
module_exit(ov08a10_exit_module);
MODULE_DESCRIPTION("ov08a10");
MODULE_LICENSE("GPL v2");
