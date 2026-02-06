/*
 * laptop_bridge.c
 *
 * operate function for laptop product
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <securec.h>
#include <linux/init.h>
#include <linux/pm_runtime.h>
#include "edp_bridge.h"

struct laptop_common_data {
	int chip_reset;
	int lcd_vdd_enable_gpio;
	int bl_config_enable_gpio;
};

struct laptop_u_private_data {
	struct laptop_common_data* com_data;

	struct regulator *ec_1v2_en;
	struct regulator *ec_dsi_vccio_on;
};

struct laptop_v_private_data {
	struct laptop_common_data* com_data;

	int soc_gpio_1v2_en;    /* SoC control LCD 1v2 enabel */
	int soc_gpio_1v8_en;	/* SoC control LCD 1v8 enabel */
};

struct laptop_common_data laptop_com_data;
struct laptop_u_private_data laptop_u_priv_data = {
	.com_data = &laptop_com_data
};

static struct laptop_v_private_data laptop_v_priv_data = {
	.com_data = &laptop_com_data
};

static struct gpio_desc laptop_gpio_init_cmds[] = {
	/* init enable gpio, set high to enable bridge */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "chip_reset", &laptop_com_data.chip_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "chip_reset", &laptop_com_data.chip_reset, 1},
	/* lcd vdd enable gpio, set high to enable vdd */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "lcd_vdd_enable_gpio", &laptop_com_data.lcd_vdd_enable_gpio, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "lcd_vdd_enable_gpio", &laptop_com_data.lcd_vdd_enable_gpio, 1},
	/* bl vdd enable gpio, set high to enable bl */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "bl_config_enable_gpio", &laptop_com_data.bl_config_enable_gpio, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "bl_config_enable_gpio", &laptop_com_data.bl_config_enable_gpio, 1},
};

static struct gpio_desc laptop_v_gpio_init_cmds[] = {
	/* init 1v2 gpio, set high to enable 1v2 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "soc_gpio_1v2_en", &laptop_v_priv_data.soc_gpio_1v2_en, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "soc_gpio_1v2_en", &laptop_v_priv_data.soc_gpio_1v2_en, 1},
	/* init 1v8 gpio, set high to enable 1v8 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, "soc_gpio_1v8_en", &laptop_v_priv_data.soc_gpio_1v8_en, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "soc_gpio_1v8_en", &laptop_v_priv_data.soc_gpio_1v8_en, 1},
};

static struct gpio_desc laptop_chip_reset_disable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "chip_reset", &laptop_com_data.chip_reset, 0},
};

static struct gpio_desc laptop_chip_reset_enable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "chip_reset", &laptop_com_data.chip_reset, 1},
};

static struct gpio_desc laptop_lcd_vdd_enable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "lcd_vdd_enable_gpio", &laptop_com_data.lcd_vdd_enable_gpio, 1},
};

static struct gpio_desc laptop_lcd_vdd_disable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, "lcd_vdd_enable_gpio", &laptop_com_data.lcd_vdd_enable_gpio, 0},
};

static struct gpio_desc laptop_bl_config_enable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, "bl_config_enable_gpio", &laptop_com_data.bl_config_enable_gpio, 1},
};

static struct gpio_desc laptop_bl_config_disable[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 60, "bl_config_enable_gpio", &laptop_com_data.bl_config_enable_gpio, 0},
};

static struct gpio_desc laptop_v_power_on_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "soc_gpio_1v2_en", &laptop_v_priv_data.soc_gpio_1v2_en, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "soc_gpio_1v8_en", &laptop_v_priv_data.soc_gpio_1v8_en, 1},
};

static struct gpio_desc laptop_v_power_off_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "soc_gpio_1v8_en", &laptop_v_priv_data.soc_gpio_1v8_en, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, "soc_gpio_1v2_en", &laptop_v_priv_data.soc_gpio_1v2_en, 0},
};

static const struct drm_display_mode default_mode_lcd = {
	.type = DRM_MODE_TYPE_DEFAULT,
	.clock = 206016,
	.hdisplay = 2160,
	.hsync_start = 2160 + 48,
	.hsync_end = 2160 + 48 + 80,
	.htotal = 2160 + 48 + 80 + 32,
	.vdisplay = 1440,
	.vsync_start = 1440 + 3,
	.vsync_end = 1440 + 3 + 27,
	.vtotal = 1440 + 3 + 27 + 10,
	.vrefresh = 60,
};

static int laptop_init_common_data(struct device_node *dn, struct laptop_common_data *com_data)
{
	int ret = 0;

	ret = read_dts_value_u32(dn, "chip_reset", &com_data->chip_reset);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "lcd_vdd_enable_gpio", &com_data->lcd_vdd_enable_gpio);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "bl_config_enable_gpio", &com_data->bl_config_enable_gpio);
	if (ret < 0)
		return ret;

	return ret;
}

static void laptop_panel_info_init(struct hisi_panel_info *pinfo, int dsi_bit_clk)
{
	mipi2edp_panel_common_info_init(pinfo);

	pinfo->mipi.dsi_bit_clk = dsi_bit_clk;
	pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
	pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
	pinfo->mipi.hsa = 24;
	pinfo->mipi.hbp = 60;
	pinfo->mipi.hline_time = 1742;
	pinfo->mipi.vsa = 10;
	pinfo->mipi.vbp = 27;
	pinfo->mipi.vfp = 3;
	pinfo->mipi.dpi_hsize = 1622;
	pinfo->mipi.vactive_line = 1440;
	pinfo->blpwm_input_ena = 0;
	/**
	* Brightness min value can't cause screen completely black!
	* so the default min value set to 20 percent of max!
	*/
	pinfo->bl_min = DEFAULT_MAX_BRIGHTNESS/90;
	pinfo->bl_max = DEFAULT_MAX_BRIGHTNESS;
	pinfo->bl_ic_ctrl_mode = 10;
	/* set pwm frequecy to 4KHz */
	pinfo->blpwm_out_div_value = 0x37;
}

static void set_blpwm_for_bl_chip(struct mipi2edp *pdata, int enable)
{
	struct hisi_panel_info *pinfo = &(pdata->panel_info);

	HISI_DRM_INFO("+");
	if (enable) {
		/* enable blpwm for backlight chip */
		pinfo->blpwm_input_ena = 0;
		pinfo->bl_min = DEFAULT_MAX_BRIGHTNESS / 90;
		pinfo->bl_max = DEFAULT_MAX_BRIGHTNESS;
		pinfo->bl_ic_ctrl_mode = 10;
		/* set pwm frequecy to 4KHz */
		pinfo->blpwm_out_div_value = 0x37;
		hisi_blpwm_on(pinfo);
		hisi_blpwm_set_backlight(pinfo, pdata->bl->props.brightness);
	} else {
		/* enable blpwm for backlight chip */
		hisi_blpwm_off(pinfo);
	}
	HISI_DRM_INFO("-");
}

/**********************************************************
 *  Function:       set_power_for_bridge
 *  Discription:    enable its regulator moudule
 *  Parameters:     struct mipi2edp *pdata,int enable
 *  return value:   0-sucess or others-fail
 **********************************************************/
static int set_power_for_bridge(struct mipi2edp *pdata, int enable)
{
	int error = 0;
	struct laptop_u_private_data *priv_data = (struct laptop_u_private_data *)pdata->private_data;

	if (!priv_data) {
		HISI_DRM_ERR("private_data is nullptr!");
		return 0;
	}

	if (enable) {
		error = regulator_enable(priv_data->ec_1v2_en);
		if (error) {
			dev_err(&pdata->client->dev, "failed to enable ec_1v2_en regulator: %d\n", error);
			return -EINVAL;
		}
		mdelay(2);
		error = regulator_enable(priv_data->ec_dsi_vccio_on);
		if (error) {
			dev_err(&pdata->client->dev, "failed to enable ec_dsi_vccio_on regulator: %d\n", error);
			return -EINVAL;
		}
		mdelay(2);
	} else {
		error = regulator_disable(priv_data->ec_dsi_vccio_on);
		if (error) {
			dev_err(&pdata->client->dev, "failed to disable ec_dsi_vccio_on regulator: %d\n", error);
			return -EINVAL;
		}
		mdelay(20);
		error = regulator_disable(priv_data->ec_1v2_en);
		if (error) {
			dev_err(&pdata->client->dev, "failed to disable ec_1v2_en regulator: %d\n", error);
			return -EINVAL;
		}
		mdelay(20);
	}
	return 0;
}

/************************************kelvin u private impl******************************************/
static int laptop_u_init_private_data_impl(struct mipi2edp *pdata, struct i2c_client *client,
	struct laptop_u_private_data* priv_data)
{
	int ret = 0;

	ret = laptop_init_common_data(client->dev.of_node, priv_data->com_data);
	if (ret) {
		HISI_DRM_ERR("laptop_init_common_data failed!");
		return ret;
	}

	/* get ec_1v2_en regulator*/
	priv_data->ec_1v2_en = devm_regulator_get_optional(&client->dev, "EC_1V2_EN");
	if (IS_ERR(priv_data->ec_1v2_en)) {
		ret = (PTR_ERR(priv_data->ec_1v2_en) == -EPROBE_DEFER) ? -EPROBE_DEFER : -EINVAL;
		HISI_DRM_ERR("Failed to get 'ec_1v2_en' regulator: %ld\n", PTR_ERR(priv_data->ec_1v2_en));
		return ret;
	}

	/* get ec_dsi_vccio_on regulator*/
	priv_data->ec_dsi_vccio_on = devm_regulator_get_optional(&client->dev, "EC_DSI_VCCIO_ON");
	if (IS_ERR(priv_data->ec_dsi_vccio_on)) {
		ret = (PTR_ERR(priv_data->ec_dsi_vccio_on) == -EPROBE_DEFER) ? -EPROBE_DEFER : -EINVAL;
		HISI_DRM_ERR("Failed to get 'vccio' regulator: %ld\n", PTR_ERR(priv_data->ec_dsi_vccio_on));
		return ret;
	}

	return 0;
}

static int laptop_u_init_private_data(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret = 0;

	ret = laptop_u_init_private_data_impl(pdata, client, &laptop_u_priv_data);
	if (ret)
		return ret;

	pdata->private_data = &laptop_u_priv_data;

	return ret;
}

static int laptop_u_bridge_probe(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = &(pdata->panel_info);

	gpio_cmds_tx(laptop_gpio_init_cmds, ARRAY_SIZE(laptop_gpio_init_cmds));

	ret = set_bl_props(pdata, pdata->dev);
	if (ret)
		return ret;

	aux_register(pdata);

	laptop_panel_info_init(pinfo, pdata->dsi_bit_clk);

	hisi_blpwm_on(pinfo);
	hisi_blpwm_set_backlight(pinfo, pdata->bl->props.brightness);

	return 0;
}

static int laptop_bridge_remove(struct i2c_client *client)
{
	return 0;
}

static int laptop_u_bridge_shutdown(struct mipi2edp *pdata)
{
	set_blpwm_for_bl_chip(pdata, 0);
	mdelay(20);

	gpio_cmds_tx(laptop_bl_config_disable, ARRAY_SIZE(laptop_bl_config_disable));

	/* disable  stream */
	if (pdata->bridge_chip_info.disable) {
		pdata->bridge_chip_info.disable(pdata);
	} else {
		HISI_DRM_INFO("chip disable is not implement on chip:%s!", pdata->bridge_chip_info.name);
	}

	/* reset the 9711 bridge chip */
	gpio_cmds_tx(laptop_chip_reset_disable, ARRAY_SIZE(laptop_chip_reset_disable));
	mdelay(20);

	/* disable lcdvdd */
	gpio_cmds_tx(laptop_lcd_vdd_disable, ARRAY_SIZE(laptop_lcd_vdd_disable));

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 0);

	set_power_for_bridge(pdata, 0);

	return 0;
}

static void laptop_u_bridge_pre_enable(struct mipi2edp *pdata)
{
	pm_runtime_get_sync(pdata->dev);

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 1);

	mdelay(15);

	gpio_cmds_tx(laptop_chip_reset_enable, ARRAY_SIZE(laptop_chip_reset_enable));
	mdelay(2);
	gpio_cmds_tx(laptop_lcd_vdd_enable, ARRAY_SIZE(laptop_lcd_vdd_enable));
	mdelay(2);
}

static void laptop_bridge_enable(struct mipi2edp *pdata)
{
	mdelay(2);
	gpio_cmds_tx(laptop_bl_config_enable, ARRAY_SIZE(laptop_bl_config_enable));

	/* enable backlight */
	set_blpwm_for_bl_chip(pdata, 1);
}

static void laptop_bridge_disable(struct mipi2edp *pdata)
{
	/* set backlight pwm 0 */
	set_blpwm_for_bl_chip(pdata,0);
	mdelay(20);

	gpio_cmds_tx(laptop_bl_config_disable, ARRAY_SIZE(laptop_bl_config_disable));
}

static void laptop_bridge_post_disable(struct mipi2edp *pdata)
{
	pm_runtime_put_sync(pdata->dev);
	mdelay(500);
}

static enum drm_connector_status laptop_drm_connector_detect(struct mipi2edp *pdata, bool force)
{
	return connector_status_connected;
}

static int laptop_drm_connector_get_mode(struct mipi2edp *pdata)
{
	struct drm_display_mode *mode = NULL;

	mode = drm_mode_duplicate(pdata->connector.dev, &default_mode_lcd);
	if (mode == NULL) {
		HISI_DRM_ERR("failed to add mode %ux%ux@%u!", default_mode_lcd.hdisplay,
			default_mode_lcd.vdisplay, default_mode_lcd.vrefresh);
		return -ENOMEM;
	}

	mode->type = DRM_MODE_TYPE_PREFERRED;
	drm_mode_set_name(mode);

	drm_mode_probed_add(&pdata->connector, mode);

	return 1;
}

static int laptop_bridge_runtime_suspend(struct mipi2edp *pdata)
{
	int status = 0;

	gpio_cmds_tx(laptop_lcd_vdd_disable, ARRAY_SIZE(laptop_lcd_vdd_disable));
	mdelay(5);
	gpio_cmds_tx(laptop_chip_reset_disable, ARRAY_SIZE(laptop_chip_reset_disable));
	mdelay(5);

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 0);
	status = set_power_for_bridge(pdata, 0);
	if (status)
		pr_err("set eDP_bridge power down failed !\n");
	return 0;
}

static int laptop_bridge_runtime_resume(struct mipi2edp *pdata)
{
	int status = 0;

	status = set_power_for_bridge(pdata, 1);
	if (status)
		pr_err("set eDP_bridge power down failed !\n");
	return 0;
}

static struct bridge_product_funcs laptop_u_bridge_funcs = {
	.bridge_init_private_data = laptop_u_init_private_data,
	.bridge_probe = laptop_u_bridge_probe,
	.bridge_remove = laptop_bridge_remove,
	.bridge_shutdown = laptop_u_bridge_shutdown,
	.bridge_attach = NULL,  // kelvin u no need to impl
	.bridge_pre_enable = laptop_u_bridge_pre_enable,
	.bridge_enable = laptop_bridge_enable,
	.bridge_disable = laptop_bridge_disable,
	.bridge_post_disable = laptop_bridge_post_disable,
	.bridge_mode_set = NULL,  // kelvin no need to impl
	.get_edid = mipi2edp_get_edid
};

static struct bridge_connector_help_funcs laptop_u_connector_help_funcs = {
	.drm_connector_detect = laptop_drm_connector_detect,
	.drm_connector_get_mode = laptop_drm_connector_get_mode,
	.drm_connector_mode_valid = NULL  // kelvin u no need to impl
};

static struct bridge_pm_func laptop_u_bridge_pm_func = {
	.pm_runtime_suspend = laptop_bridge_runtime_suspend,
	.pm_runtime_resume = laptop_bridge_runtime_resume,
	.pm_system_suspend = NULL,  // kelvin u no need to impl
	.pm_system_resume = NULL  // kelvin u no need to impl
};

static struct mipi2edp_bridge_product_info laptop_u_product_info = {
	.product_type = PRODUCT_TYPE_LAPTOP,
	.product_series = PRODUCT_SERIES_LAPTOP,
	.desc = "laptop product U",
	.connector_type = DRM_MODE_CONNECTOR_eDP,
	.bridge_ops = &laptop_u_bridge_funcs,
	.bridge_connector_ops = &laptop_u_connector_help_funcs,
	.bridge_pm_ops = &laptop_u_bridge_pm_func
};

/************************************kelvin v private impl******************************************/
static int laptop_v_init_private_data_impl(struct mipi2edp *pdata, struct i2c_client *client,
	struct laptop_v_private_data* priv_data)
{
	int ret;
	struct device_node *dn = client->dev.of_node;

	ret = laptop_init_common_data(client->dev.of_node, priv_data->com_data);
	if (ret) {
		HISI_DRM_ERR("laptop_init_common_data failed!");
		return ret;
	}

	ret = read_dts_value_u32(dn, "soc_gpio_1v2_en", &priv_data->soc_gpio_1v2_en);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "soc_gpio_1v8_en", &priv_data->soc_gpio_1v8_en);
	if (ret < 0)
		return ret;

	return 0;
}

static int laptop_v_init_private_data(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret;

	ret = laptop_v_init_private_data_impl(pdata, client, &laptop_v_priv_data);
	if (ret)
		return ret;
	pdata->private_data = &laptop_v_priv_data;

	return ret;
}

static int laptop_v_bridge_probe(struct mipi2edp *pdata, struct i2c_client *client)
{
	int ret;
	struct hisi_panel_info *pinfo = &(pdata->panel_info);

	gpio_cmds_tx(laptop_gpio_init_cmds, ARRAY_SIZE(laptop_gpio_init_cmds));
	gpio_cmds_tx(laptop_v_gpio_init_cmds, ARRAY_SIZE(laptop_v_gpio_init_cmds));

	ret = set_bl_props(pdata, pdata->dev);
	if (ret)
		return ret;

	aux_register(pdata);

	laptop_panel_info_init(pinfo, pdata->dsi_bit_clk);

	hisi_blpwm_on(pinfo);
	hisi_blpwm_set_backlight(pinfo, pdata->bl->props.brightness);

	return 0;
}

static int laptop_v_bridge_shutdown(struct mipi2edp *pdata)
{
	set_blpwm_for_bl_chip(pdata, 0);
	mdelay(20);
	gpio_cmds_tx(laptop_bl_config_disable, ARRAY_SIZE(laptop_bl_config_disable));

	/* disable  stream */
	if (pdata->bridge_chip_info.disable) {
		pdata->bridge_chip_info.disable(pdata);
	} else {
		HISI_DRM_INFO("chip disable is not implement on chip:%s!", pdata->bridge_chip_info.name);
	}

	/* disable lcdvdd */
	gpio_cmds_tx(laptop_lcd_vdd_disable, ARRAY_SIZE(laptop_lcd_vdd_disable));
	mdelay(5);
	gpio_cmds_tx(laptop_chip_reset_disable, ARRAY_SIZE(laptop_chip_reset_disable));
	mdelay(5);
	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 0);
	gpio_cmds_tx(laptop_v_power_off_cmds, ARRAY_SIZE(laptop_v_power_off_cmds));

	return 0;
}

static void laptop_v_bridge_pre_enable(struct mipi2edp *pdata)
{
	static int count = 0;
	if (count == 0) {
		laptop_v_bridge_shutdown(pdata);
		mdelay(500);
		count = 1;
		HISI_DRM_INFO(" LCD enable and bridge reset disable");
	}

	pm_runtime_get_sync(pdata->dev);

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 1);

	mdelay(5);
	gpio_cmds_tx(laptop_chip_reset_enable, ARRAY_SIZE(laptop_chip_reset_enable));
	mdelay(15);
	gpio_cmds_tx(laptop_lcd_vdd_enable, ARRAY_SIZE(laptop_lcd_vdd_enable));
}

static int laptop_v_bridge_runtime_suspend(struct mipi2edp *pdata)
{

	gpio_cmds_tx(laptop_lcd_vdd_disable, ARRAY_SIZE(laptop_lcd_vdd_disable));
	gpio_cmds_tx(laptop_chip_reset_disable, ARRAY_SIZE(laptop_chip_reset_disable));

	if (pdata->bridge_chip_info.set_clk)
		pdata->bridge_chip_info.set_clk(pdata->mipi2edp_clk, 0);

	gpio_cmds_tx(laptop_v_power_off_cmds, ARRAY_SIZE(laptop_v_power_off_cmds));

	return 0;
}

static int laptop_v_bridge_runtime_resume(struct mipi2edp *pdata)
{
	gpio_cmds_tx(laptop_v_power_on_cmds, ARRAY_SIZE(laptop_v_power_on_cmds));
	return 0;
}

static struct bridge_product_funcs laptop_v_bridge_funcs = {
	.bridge_init_private_data = laptop_v_init_private_data,
	.bridge_probe = laptop_v_bridge_probe,
	.bridge_remove = laptop_bridge_remove,
	.bridge_shutdown = laptop_v_bridge_shutdown,
	.bridge_attach = NULL,  // kelvin v no need to impl
	.bridge_pre_enable = laptop_v_bridge_pre_enable,
	.bridge_enable = laptop_bridge_enable,
	.bridge_disable = laptop_bridge_disable,
	.bridge_post_disable = laptop_bridge_post_disable,
	.bridge_mode_set = NULL,
	.get_edid = mipi2edp_get_edid
};

static struct bridge_connector_help_funcs laptop_v_connector_help_funcs = {
	.drm_connector_detect = laptop_drm_connector_detect,
	.drm_connector_get_mode = laptop_drm_connector_get_mode,
	.drm_connector_mode_valid = NULL  // kelvin v no need to impl
};

static struct bridge_pm_func laptop_v_bridge_pm_func = {
	.pm_runtime_suspend = laptop_v_bridge_runtime_suspend,
	.pm_runtime_resume = laptop_v_bridge_runtime_resume,
	.pm_system_suspend = NULL,  // kelvin v no need to impl
	.pm_system_resume = NULL  // kelvin v no need to impl
};

static struct mipi2edp_bridge_product_info laptop_v_product_info = {
	.product_type = PRODUCT_TYPE_LAPTOP_KV,
	.product_series = PRODUCT_SERIES_LAPTOP,
	.desc = "laptop product V",
	.connector_type = DRM_MODE_CONNECTOR_eDP,
	.bridge_ops = &laptop_v_bridge_funcs,
	.bridge_connector_ops = &laptop_v_connector_help_funcs,
	.bridge_pm_ops = &laptop_v_bridge_pm_func
};

static int __init laptop_bridge_register(void)
{
	int ret;

	printk(KERN_DEBUG "laptop_bridge_register +");

	ret = mipi2edp_bridge_product_info_register(&laptop_u_product_info);
	if (ret)
		return ret;

	ret = mipi2edp_bridge_product_info_register(&laptop_v_product_info);
	if (ret)
		return ret;

	printk(KERN_DEBUG "laptop_bridge_register -");

	return ret;
}

fs_initcall(laptop_bridge_register);
