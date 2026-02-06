/*
 * imx576.c
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
#include <securec.h>
#include "hwsensor.h"
#include "sensor_commom.h"
#include <linux/pinctrl/consumer.h>
#include "../pmic/hw_pmic.h"

#define intf_to_sensor(i) container_of((i), sensor_t, intf)
#define sensor_to_pdev(s) container_of((s).dev, struct platform_device, dev)

static hwsensor_vtbl_t g_imx576_vtbl;
static bool g_power_on_status; /* false: power off, true:power on */
static struct platform_device *g_pdev;
struct mutex g_imx576_power_lock;

/* imx576 cs udp project */
static struct sensor_power_setting g_imx576_power_setting[] = {
	/* MCAM1 AVDD 2.8V */
	{
		.seq_type = SENSOR_AVDD,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 DVDD 1.1V */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM IOVDD 1.80V */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
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
	{
		.seq_type = SENSOR_RST2,
		.config_val = SENSOR_GPIO_HIGH,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};

/* imx576 A&B project */
static struct sensor_power_setting g_imx576_power_setting_ab[] = {
	/* SENSOR IOVDD 1.8V VOUT21 */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 AVDD 2.8V */
	{
		.seq_type = SENSOR_AVDD,
		.data = (void *)"front-sensor-avdd",
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 DVDD 1.1V */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_V1P1V,
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

/* ES UDP power up seq */
static struct sensor_power_setting g_imx576_power_setting_udp[] = {
	/* disable main camera reset */
	{
		.seq_type = SENSOR_SUSPEND,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* disable sub camera reset */
	{
		.seq_type = SENSOR_SUSPEND2,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM IOVDD 1.80V */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
	/* MCAM1 AVDD 2.8V */
	{
		.seq_type = SENSOR_AVDD,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 DVDD 1.2V */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P2V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* VCM [2.60v] */
	{
		.seq_type = SENSOR_VCM_AVDD,
		.config_val = LDO_VOLTAGE_V2P8V,
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

/* Emily power up seq */
static struct sensor_power_setting g_imx576_power_setting_emily[] = {
	/* MCAM1 AVDD 2.8V */
	{
		.seq_type = SENSOR_AVDD,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 DVDD 1.1V */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM IOVDD 1.80V */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
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

/* Neo power up seq */
static struct sensor_power_setting g_imx576_power_setting_neo[] = {
	/* MCAM1 AVDD 2.8V */
	{
		.seq_type = SENSOR_AVDD,
		.config_val = LDO_VOLTAGE_V2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 DVDD 1.1V */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM IOVDD 1.80V */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
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
	{
		.seq_type = SENSOR_RST2,
		.config_val = SENSOR_GPIO_HIGH,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
	},
};


/* hma power up seq */
static struct sensor_power_setting g_imx576_power_setting_hma[] = {
	/* MCAM1 AVDD 2.8V */
	{
		.seq_type = SENSOR_AVDD1_EN,
		.config_val = SENSOR_GPIO_LOW,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 DVDD 1.1V */
	{
		.seq_type = SENSOR_DVDD,
		.config_val = LDO_VOLTAGE_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM IOVDD 1.80V */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
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


/* hma power up seq */
static struct sensor_power_setting g_imx576_power_setting_laya[] = {
	/* MCAM1 AVDD 2.8V */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_LDO_1,
		.config_val = PMIC_2P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM1 DVDD 1.1V */
	{
		.seq_type = SENSOR_PMIC,
		.seq_val = VOUT_BUCK_1,
		.config_val = PMIC_1P1V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 0,
	},
	/* MCAM IOVDD 1.80V */
	{
		.seq_type = SENSOR_IOVDD,
		.config_val = LDO_VOLTAGE_1P8V,
		.sensor_index = SENSOR_INDEX_INVALID,
		.delay = 1,
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

atomic_t volatile g_imx576_powered = ATOMIC_INIT(0);
static sensor_t g_imx576 = {
	.intf = { .vtbl = &g_imx576_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx576_power_setting),
		.power_setting = g_imx576_power_setting,
	},
	.p_atpowercnt = &g_imx576_powered,
};

static sensor_t g_imx576_ab = {
	.intf = { .vtbl = &g_imx576_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx576_power_setting_ab),
		.power_setting = g_imx576_power_setting_ab,
	},
	.p_atpowercnt = &g_imx576_powered,
};

static sensor_t g_imx576_udp = {
	.intf = { .vtbl = &g_imx576_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx576_power_setting_udp),
		.power_setting = g_imx576_power_setting_udp,
	},
	.p_atpowercnt = &g_imx576_powered,
};

static sensor_t g_imx576_emily = {
	.intf = { .vtbl = &g_imx576_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx576_power_setting_emily),
		.power_setting = g_imx576_power_setting_emily,
	},
	.p_atpowercnt = &g_imx576_powered,
};

static sensor_t g_imx576_neo = {
	.intf = { .vtbl = &g_imx576_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx576_power_setting_neo),
		.power_setting = g_imx576_power_setting_neo,
	},
	.p_atpowercnt = &g_imx576_powered,
};

static sensor_t g_imx576_hma = {
	.intf = { .vtbl = &g_imx576_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx576_power_setting_hma),
		.power_setting = g_imx576_power_setting_hma,
	},
	.p_atpowercnt = &g_imx576_powered,
};

static sensor_t g_imx576_laya = {
	.intf = { .vtbl = &g_imx576_vtbl, },
	.power_setting_array = {
		.size = ARRAY_SIZE(g_imx576_power_setting_laya),
		.power_setting = g_imx576_power_setting_laya,
	},
	.p_atpowercnt = &g_imx576_powered,
};

static const struct of_device_id g_imx576_dt_match[] = {
	{
		.compatible = "huawei,imx576",
		.data = &g_imx576.intf,
	},
	{
		.compatible = "huawei,imx576_ab",
		.data = &g_imx576_ab.intf,
	},
	{
		.compatible = "huawei,imx576_udp",
		.data = &g_imx576_udp.intf,
	},
	{
		.compatible = "huawei,imx576_emily",
		.data = &g_imx576_emily.intf,
	},
	{
		.compatible = "huawei,imx576_neo",
		.data = &g_imx576_neo.intf,
	},
	{
		.compatible = "huawei,imx576_hma",
		.data = &g_imx576_hma.intf,
	},
	{
		.compatible = "huawei,imx576_laya",
		.data = &g_imx576_laya.intf,
	},
	{
	}, /* terminate list */
};
MODULE_DEVICE_TABLE(of, g_imx576_dt_match);

static struct platform_driver g_imx576_driver = {
	.driver = {
		.name = "huawei,imx576",
		.owner = THIS_MODULE,
		.of_match_table = g_imx576_dt_match,
	},
};

char const *imx576_get_name(hwsensor_intf_t *si)
{
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is NULL", __func__);
		return NULL;
	}
	sensor = intf_to_sensor(si);
	return sensor->board_info->name;
}

int imx576_power_up(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is NULL", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	cam_info("enter %s. index = %d name = %s", __func__,
		sensor->board_info->sensor_index, sensor->board_info->name);

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

int imx576_power_down(hwsensor_intf_t *si)
{
	int ret;
	sensor_t *sensor = NULL;

	if (!si) {
		cam_err("%s. si is NULL", __func__);
		return -EINVAL;
	}

	sensor = intf_to_sensor(si);
	cam_info("enter %s. index = %d name = %s", __func__,
		sensor->board_info->sensor_index, sensor->board_info->name);
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

int imx576_csi_enable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

int imx576_csi_disable(hwsensor_intf_t *si)
{
	(void)si;
	return 0;
}

static int imx576_match_id(hwsensor_intf_t *si, void *data)
{
	sensor_t *sensor = intf_to_sensor(si);
	struct sensor_cfg_data *cdata = (struct sensor_cfg_data *)data;
	char sensor_name[] = {"IMX576_SUNNY"};
	int ret;

	cam_info("%s enter", __func__);

	cdata->data = SENSOR_INDEX_INVALID;

	if (!strncmp(sensor->board_info->name,
		"IMX576_SUNNY", strlen("IMX576_SUNNY"))) {
		ret = strncpy_s(cdata->cfg.name,
			DEVICE_NAME_SIZE - 1,
			sensor_name,
			strlen(sensor_name) + 1);
		if (ret != 0) {
			cam_err("%s. strncpy failed", __func__);
			return -EINVAL;
		}
		cdata->data = sensor->board_info->sensor_index;
	} else {
		ret = strncpy_s(cdata->cfg.name,
			DEVICE_NAME_SIZE - 1,
			sensor->board_info->name,
			strlen(sensor->board_info->name) + 1);
		if (ret != 0) {
			cam_err("%s.strncpy failed", __func__);
			return -EINVAL;
		}
		cdata->data = sensor->board_info->sensor_index;
	}

	if (cdata->data != SENSOR_INDEX_INVALID)
		cam_info("%s, cdata->cfg.name = %s", __func__, cdata->cfg.name);

	cam_info("%s TODO.  cdata->data=%d", __func__, cdata->data);
	return 0;
}

int imx576_config(hwsensor_intf_t *si, void  *argp)
{
	struct sensor_cfg_data *data = NULL;
	int ret = 0;

	if (!si || !argp) {
		cam_err("%s : si or argp is null", __func__);
		return -EINVAL;
	}

	data = (struct sensor_cfg_data *)argp;
	cam_debug("imx576 cfgtype = %d", data->cfgtype);
	switch (data->cfgtype) {
	case SEN_CONFIG_POWER_ON:
		mutex_lock(&g_imx576_power_lock);
		if (g_power_on_status == false) {
			ret = si->vtbl->power_up(si);
			if (ret == 0)
				g_power_on_status = true;
			else
				cam_err("%s. power up fail", __func__);
		}
		mutex_unlock(&g_imx576_power_lock);
		break;
	case SEN_CONFIG_POWER_OFF:
		mutex_lock(&g_imx576_power_lock);
		if (g_power_on_status == true) {
			ret = si->vtbl->power_down(si);
			if (ret != 0)
				cam_err("%s. power_down fail", __func__);
			g_power_on_status = false;
		}
		mutex_unlock(&g_imx576_power_lock);

		break;
	case SEN_CONFIG_WRITE_REG:
	case SEN_CONFIG_READ_REG:
	case SEN_CONFIG_WRITE_REG_SETTINGS:
	case SEN_CONFIG_READ_REG_SETTINGS:
	case SEN_CONFIG_ENABLE_CSI:
	case SEN_CONFIG_DISABLE_CSI:
		break;
	case SEN_CONFIG_MATCH_ID:
		ret = si->vtbl->match_id(si, argp);
		break;
	default:
		cam_warn("%s cfgtype %d is unknown", __func__, data->cfgtype);
		break;
	}

	return ret;
}

static hwsensor_vtbl_t g_imx576_vtbl = {
	.get_name = imx576_get_name,
	.config = imx576_config,
	.power_up = imx576_power_up,
	.power_down = imx576_power_down,
	.match_id = imx576_match_id,
	.csi_enable = imx576_csi_enable,
	.csi_disable = imx576_csi_disable,
};

static int32_t imx576_platform_probe(struct platform_device *pdev)
{
	int rc;
	const struct of_device_id *id = NULL;
	hwsensor_intf_t *intf = NULL;
	sensor_t *sensor = NULL;
	struct device_node *np = NULL;

	cam_notice("enter %s", __func__);

	if (!pdev) {
		cam_err("%s pdev is NULL", __func__);
		return -EINVAL;
	}

	np = pdev->dev.of_node;
	if (!np) {
		cam_err("%s of_node is NULL", __func__);
		return -ENODEV;
	}

	id = of_match_node(g_imx576_dt_match, np);
	if (!id) {
		cam_err("%s none id matched", __func__);
		return -ENODEV;
	}

	intf = (hwsensor_intf_t *)id->data;
	sensor = intf_to_sensor(intf);

	rc = hw_sensor_get_dt_data(pdev, sensor);
	if (rc < 0) {
		cam_err("%s no dt data rc %d", __func__, rc);
		return -ENODEV;
	}
	sensor->dev = &pdev->dev;
	mutex_init(&g_imx576_power_lock);
	rc = hwsensor_register(pdev, intf);
	if (rc < 0) {
		cam_err("%s hwsensor_register failed rc %d\n",
			__func__, rc);
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

	return rc;
}

static int __init imx576_init_module(void)
{
	cam_info("Enter: %s", __func__);
	return platform_driver_probe(&g_imx576_driver,
		imx576_platform_probe);
}

static void __exit imx576_exit_module(void)
{
	if (g_pdev) {
		hwsensor_unregister(g_pdev);
		g_pdev = NULL;
	}

	platform_driver_unregister(&g_imx576_driver);
}

module_init(imx576_init_module);
module_exit(imx576_exit_module);
MODULE_DESCRIPTION("imx576");
MODULE_LICENSE("GPL v2");
