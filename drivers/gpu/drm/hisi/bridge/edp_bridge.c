/*
 * edp_bridge.c
 *
 * i2c driver for mipi2edp bridge
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
/*lint -e548 -e574 -e578*/
#include <securec.h>
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <linux/component.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/semaphore.h>
#include <linux/backlight.h>
#include "linux/mfd/hisi_pmic.h"
#include "edp_bridge.h"
#include <linux/mfd/hisi_pmic.h>
#include <linux/clk-provider.h>
#include <linux/clk/clk-conf.h>

#define  GPIO_ID_HIGH 0x01
#define  GPIO_ID_LOW  0x0

static struct pinctrl_data pinctrl;

static struct pinctrl_cmd_desc bridge_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &pinctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &pinctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &pinctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc bridge_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &pinctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static const u8 edid_header[EDID_HEADER_LEN] = {0x00, 0xff, 0xff, 0xff,
			   0xff, 0xff, 0xff, 0x00};

int check_edid(char *edid, unsigned int len)
{
	int i;
	if (!edid) {
		HISI_DRM_ERR("Edid The pointer is NULL.\n");
		return -1;
	}

	if (len < EDID_LEN) {
		HISI_DRM_ERR("len input err");
		return -1;
	}

	/*Verify Header*/
	for (i = 0; i < EDID_HEADER_LEN; i++) {
		if (edid[i] != edid_header[i]) {
			HISI_DRM_ERR("Don't match EDID header\n");
			return -1;
		}
	}
	HISI_DRM_INFO("match EDID header SUCCESS!\n");
	return 0;
}

int mipi2edp_get_edid(struct mipi2edp *pdata)
{
	if (pdata->bridge_chip_info.get_edid) {
		mutex_lock(&pdata->lock);
		pdata->bridge_chip_info.get_edid(pdata->edid_array, EDID_LEN);
		mutex_unlock(&pdata->lock);
	}
	return 0;
}

int read_dts_value_u32(struct device_node *dn, const char *propname, u32 *out_value)
{
	int ret = of_property_read_u32(dn, propname, out_value);
	if (ret < 0) {
		HISI_DRM_ERR("get %s from dts failed!", propname);
	} else {
		HISI_DRM_INFO("dts info %s = 0x%x !", propname, *out_value);
	}
	return ret;
}

/* Connector helper functions */
static int mipi2edp_connector_get_modes(
	struct drm_connector *connector)
{
	struct mipi2edp *pdata = NULL;
	HISI_DRM_INFO("+");

	if (!connector) {
		HISI_DRM_ERR("connector is nullptr!");
		return -EINVAL;
	}
	pdata = container_of(connector, struct mipi2edp, connector);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_connector_ops->drm_connector_get_mode)
		return pdata->bridge_product_info.bridge_connector_ops->drm_connector_get_mode(pdata);

	HISI_DRM_INFO("- drm_connector_get_mode is not implement on product:%s!", pdata->bridge_product_info.desc);

	return 0;
}

static enum drm_mode_status mipi2edp_connector_mode_valid(
	struct drm_connector *connector,
	struct drm_display_mode *mode)
{
	struct mipi2edp *pdata=NULL;
	HISI_DRM_INFO("+");
	if (!connector) {
		HISI_DRM_ERR("connector is nullptr!");
		return -EINVAL;
	}
	pdata = container_of(connector, struct mipi2edp, connector);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_connector_ops->drm_connector_mode_valid)
		return pdata->bridge_product_info.bridge_connector_ops->drm_connector_mode_valid(connector, mode);

	HISI_DRM_INFO("- drm_connector_mode_valid is not implement on product:%s!", pdata->bridge_product_info.desc);
	return MODE_OK;
}

static struct drm_connector_helper_funcs mipi2edp_connector_helper_funcs = {
	.get_modes = mipi2edp_connector_get_modes,
	.mode_valid = mipi2edp_connector_mode_valid,
};

static enum drm_connector_status mipi2edp_connector_detect(
	struct drm_connector *connector,
	bool force)
{
	struct mipi2edp *pdata = NULL;
	enum drm_connector_status status = connector_status_disconnected;
	HISI_DRM_INFO("+");

	if (!connector) {
		HISI_DRM_ERR("connector is nullptr!");
		return -EINVAL;
	}
	pdata = container_of(connector, struct mipi2edp, connector);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_connector_ops->drm_connector_detect) {
		status = pdata->bridge_product_info.bridge_connector_ops->drm_connector_detect(pdata, force);
		HISI_DRM_INFO("mipi2edp_cur_status = %d !\n", status);
	} else {
		HISI_DRM_INFO("drm_connector_detect is not implement on product:%s!", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");
	return status;
}

const static struct drm_connector_funcs mipi2edp_connector_funcs = {
	.fill_modes = drm_helper_probe_single_connector_modes,
	.detect = mipi2edp_connector_detect,
	.destroy = drm_connector_cleanup,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

/**********************************************************
 *  Function:       mipi2edp_bridge_pre_enable
 *  Discription:    pre_enable mopi2edp_bridge moudule
 *  Parameters:     struct drm_bridge *bridge
 *  return value:   none
 **********************************************************/
static void mipi2edp_bridge_pre_enable(struct drm_bridge *bridge)
{
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!bridge) {
		HISI_DRM_ERR("bridge is nullptr!");
		return;
	}

	pdata = container_of(bridge, struct mipi2edp, bridge);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return;
	}

	if (pdata->bridge_product_info.bridge_ops->bridge_pre_enable) {
		pdata->bridge_product_info.bridge_ops->bridge_pre_enable(pdata);
	} else {
		HISI_DRM_INFO("bridge_pre_enable is not implement on product:%s!", pdata->bridge_product_info.desc);
	}

	if (pdata->bridge_chip_info.pre_enable) {
		pdata->bridge_chip_info.pre_enable(pdata);
	} else {
		HISI_DRM_INFO("chip pre_enable is not implement on chip:%s!", pdata->bridge_chip_info.name);
	}

	HISI_DRM_INFO("-");
}

static void mipi2edp_bridge_enable(struct drm_bridge *bridge)
{
	int ret = 0;
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!bridge) {
		HISI_DRM_ERR("bridge is nullptr!");
		return;
	}

	pdata = container_of(bridge, struct mipi2edp, bridge);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return;
	}

	HISI_DRM_INFO("+");

	if (pdata->bridge_chip_info.enable) {
		ret = pdata->bridge_chip_info.enable(pdata);
		if (ret)
			HISI_DRM_ERR("chip enable failed on chip:%s", pdata->bridge_chip_info.name);
	} else {
		HISI_DRM_INFO("chip enable is not implement on chip:%s!", pdata->bridge_chip_info.name);
	}

	if (pdata->bridge_product_info.bridge_ops->bridge_enable) {
		pdata->bridge_product_info.bridge_ops->bridge_enable(pdata);
	} else {
		HISI_DRM_INFO("product bridge_enable is not implement on product:%s!", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");
}

static void mipi2edp_bridge_disable(struct drm_bridge *bridge)
{
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!bridge) {
		HISI_DRM_ERR("bridge is nullptr!");
		return;
	}

	pdata = container_of(bridge, struct mipi2edp, bridge);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return;
	}

	if (pdata->bridge_product_info.bridge_ops->bridge_disable) {
		pdata->bridge_product_info.bridge_ops->bridge_disable(pdata);
	} else {
		HISI_DRM_INFO("product bridge_disable is not implement on product:%s!", pdata->bridge_product_info.desc);
	}

	/* disable  stream */
	if (pdata->bridge_chip_info.disable) {
		pdata->bridge_chip_info.disable(pdata);
	} else {
		HISI_DRM_INFO("chip disable is not implement on chip:%s!", pdata->bridge_chip_info.name);
	}

	HISI_DRM_INFO("-");
}

static void mipi2edp_bridge_post_disable(struct drm_bridge *bridge)
{
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!bridge) {
		HISI_DRM_ERR("bridge is nullptr!");
		return;
	}

	pdata = container_of(bridge, struct mipi2edp, bridge);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return;
	}

	if (pdata->bridge_product_info.bridge_ops->bridge_post_disable) {
		pdata->bridge_product_info.bridge_ops->bridge_post_disable(pdata);
	} else {
		HISI_DRM_INFO("product bridge_post_disable is not implement on product:%s!", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");
}

static void mipi2edp_bridge_mode_set(
	struct drm_bridge *bridge,
	struct drm_display_mode *mode,
	struct drm_display_mode *adjusted_mode)
{
	struct mipi2edp *pdata = NULL;
	UNUSED(mode);
	HISI_DRM_INFO("+");
	if (!bridge) {
		HISI_DRM_ERR("bridge is nullptr!");
		return;
	}

	if (!adjusted_mode) {
		HISI_DRM_ERR("adjusted_mode is nullptr!");
		return;
	}

	pdata = container_of(bridge, struct mipi2edp, bridge);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return;
	}

	if (pdata->bridge_product_info.bridge_ops->bridge_mode_set) {
		pdata->bridge_product_info.bridge_ops->bridge_mode_set(pdata, mode, adjusted_mode);
	} else {
		HISI_DRM_INFO("product bridge_mode_set is not implement on product:%s!", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");
}

static int mipi2edp_bridge_attach(struct drm_bridge *bridge)
{
	int ret = 0;
	struct mipi2edp *pdata = NULL;
	struct drm_encoder *encoder = NULL;
	struct hisi_dsi *dsi = NULL;
	u32 allowed_scalers = 0;

	HISI_DRM_INFO("+");

	if (!bridge) {
		HISI_DRM_ERR("bridge is nullptr!");
		return -EINVAL;
	}

	pdata = container_of(bridge, struct mipi2edp, bridge);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	encoder = bridge->encoder;
	if (!encoder) {
		HISI_DRM_ERR("encoder is nullptr!");
		return -EINVAL;
	}

	ret = drm_connector_init(bridge->dev, &pdata->connector, &mipi2edp_connector_funcs,
		pdata->bridge_product_info.connector_type);

	if (ret) {
		HISI_DRM_ERR("Failed to initialize connector with drm, ret=%d!", ret);
		return ret;
	}

	drm_connector_helper_add(&pdata->connector, &mipi2edp_connector_helper_funcs);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	drm_mode_connector_attach_encoder(&pdata->connector, bridge->encoder);
#else
	drm_connector_attach_encoder(&pdata->connector, bridge->encoder);
#endif
	allowed_scalers |= BIT(DRM_MODE_SCALE_ASPECT);
	allowed_scalers |= BIT(DRM_MODE_SCALE_FULLSCREEN);
	drm_connector_attach_scaling_mode_property(&pdata->connector, allowed_scalers);

	dsi = encoder_to_dsi(encoder);
	dsi->client.lanes = 4;
	dsi->client.format = MIPI_DSI_FMT_RGB888;
	dsi->client.mode_flags = MIPI_DSI_MODE_VIDEO|MIPI_DSI_CLOCK_NON_CONTINUOUS;
	dsi->client.phy_clock = 0;
	dsi->panel_info = &(pdata->panel_info);

	if (pdata->bridge_product_info.bridge_ops->bridge_attach) {
		ret = pdata->bridge_product_info.bridge_ops->bridge_attach(pdata);
		if (ret) {
			HISI_DRM_ERR("product bridge_attach failed on product desc:%s", pdata->bridge_product_info.desc);
			return ret;
		}
	} else {
		HISI_DRM_INFO("product bridge_attach is not implement on product:%s!", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");

	return ret;
}

static const struct drm_bridge_funcs mipi2edp_bridge_funcs = {
	.attach = mipi2edp_bridge_attach,
	.pre_enable = mipi2edp_bridge_pre_enable,
	.enable = mipi2edp_bridge_enable,
	.disable = mipi2edp_bridge_disable,
	.post_disable = mipi2edp_bridge_post_disable,
	.mode_set = mipi2edp_bridge_mode_set,
};

static int mipi2edp_bridge_runtime_resume(struct device *dev)
{
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!dev) {
		HISI_DRM_ERR("device is nullptr!");
		return -EINVAL;
	}

	pdata = dev_get_drvdata(dev);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_pm_ops->pm_runtime_resume) {
		pdata->bridge_product_info.bridge_pm_ops->pm_runtime_resume(pdata);
	} else {
		HISI_DRM_INFO("product pm_runtime_resume not implement on product:%s", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");

	return 0;
}

static int mipi2edp_bridge_runtime_suspend(struct device *dev)
{
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!dev) {
		HISI_DRM_ERR("device is nullptr!");
		return -EINVAL;
	}

	pdata = dev_get_drvdata(dev);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_pm_ops->pm_runtime_suspend) {
		pdata->bridge_product_info.bridge_pm_ops->pm_runtime_suspend(pdata);
	} else {
		HISI_DRM_INFO("product pm_runtime_suspend not implement on product:%s", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");

	return 0;
}

static int mipi2edp_bridge_system_suspend(struct device *dev)
{
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!dev) {
		HISI_DRM_ERR("device is nullptr!");
		return -EINVAL;
	}

	pdata = dev_get_drvdata(dev);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_pm_ops->pm_system_suspend) {
		pdata->bridge_product_info.bridge_pm_ops->pm_system_suspend(pdata);
	} else {
		HISI_DRM_INFO("product pm_system_suspend not implement on product:%s", pdata->bridge_product_info.desc);
	}

	/* disable  stream */
	if (pdata->bridge_chip_info.disable) {
		pdata->bridge_chip_info.disable(pdata);
	} else {
		HISI_DRM_INFO("chip disable not implement on chip:%s", pdata->bridge_chip_info.name);
	}

	HISI_DRM_INFO("-");

	return 0;
}

static int mipi2edp_bridge_system_resume(struct device *dev)
{
	struct mipi2edp *pdata = NULL;

	HISI_DRM_INFO("+");

	if (!dev) {
		HISI_DRM_ERR("device is nullptr!");
		return -EINVAL;
	}

	pdata = dev_get_drvdata(dev);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_pm_ops->pm_system_resume) {
		pdata->bridge_product_info.bridge_pm_ops->pm_system_resume(pdata);
	} else {
		HISI_DRM_INFO("product pm_system_resume not implement on product:%s", pdata->bridge_product_info.desc);
	}

	if (pdata->bridge_chip_info.pre_enable) {
		pdata->bridge_chip_info.pre_enable(pdata);
	} else {
		HISI_DRM_INFO("chip pre_enable not implement on chip:%s", pdata->bridge_chip_info.name);
	}

	if (pdata->bridge_chip_info.enable) {
		if (pdata->bridge_chip_info.enable(pdata))
			HISI_DRM_INFO("chip enable failed on chip:%s", pdata->bridge_chip_info.name);
	} else {
		HISI_DRM_INFO("chip enable not implement on chip:%s", pdata->bridge_chip_info.name);
	}

	HISI_DRM_INFO("-");

	return 0;
}

static const struct dev_pm_ops mipi2edp_bridge_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(mipi2edp_bridge_system_suspend, mipi2edp_bridge_system_resume)
	SET_RUNTIME_PM_OPS(mipi2edp_bridge_runtime_suspend, mipi2edp_bridge_runtime_resume, NULL)
};

static int mipi2edp_get_brightness(struct backlight_device *bl)
{
	return bl->props.brightness;
}

static int mipi2edp_backlight_update(struct backlight_device *bl)
{
	struct mipi2edp *pdata = bl_get_data(bl);
	int brightness = bl->props.brightness;

	HISI_DRM_INFO("update brightness :%d", brightness);
	hisi_blpwm_set_backlight(&(pdata->panel_info), (uint32_t)brightness);

	HISI_DRM_INFO("+");
	return 0;
}

static const struct backlight_ops mipi2edp_backlight_ops = {
	.get_brightness = mipi2edp_get_brightness,
	.update_status = mipi2edp_backlight_update,
};

void mipi2edp_panel_common_info_init(struct hisi_panel_info *pinfo)
{
	pinfo->type = PANEL_MIPI_VIDEO;
	pinfo->mipi.non_continue_en = 0;
	pinfo->mipi.lane_nums = DSI_4_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;
	pinfo->mipi.vc = 0;
	pinfo->mipi.max_tx_esc_clk = 10 * 1000000;
	pinfo->mipi.phy_mode = DPHY_MODE;
	pinfo->mipi.dsi_version = DSI_1_1_VERSION;
	pinfo->mipi.dsi_timing_support = 1;
	pinfo->ifbc_type = IFBC_TYPE_NONE;
	pinfo->pxl_clk_rate_div = 1;
}

static int mipi2edp_probe_init(struct i2c_client *client, struct device **dev,
	struct mipi2edp **pdata)
{
	int ret = 0;
	struct i2c_adapter *adapter = NULL;

	adapter = client->adapter;
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		HISI_DRM_ERR("failed to i2c_check_functionality!");
		return -EOPNOTSUPP;
	}
	*dev = &client->dev;
	*pdata = devm_kzalloc(*dev, sizeof(struct mipi2edp), GFP_KERNEL);
	if (!(*pdata)) {
		HISI_DRM_ERR("failed to alloc i2c!");
		return -ENOMEM;
	}

	dev_set_drvdata(*dev, *pdata);
	(*pdata)->dev = *dev;

	(*pdata)->client = client;
	i2c_set_clientdata(client, *pdata);

	return 0;
}

static int check_mipi2edp_onboard(struct mipi2edp *pdata)
{
	/* assume the chip is enaable in UEFI.
	 * check whether the mipi2edp bridge chip is onboard
	 */
	int ret = 0;

	if (pdata->bridge_chip_info.is_chip_onboard) {
		ret = pdata->bridge_chip_info.is_chip_onboard(pdata);
		if (ret < 0)
			HISI_DRM_ERR("failed to find the bridge chip!");
	}
	return ret;
}

static int mipi2edp_init_data_from_dts(struct i2c_client *client, struct mipi2edp *pdata)
{
	struct device_node *dn = NULL;
	int ret = 0;

	dn = client->dev.of_node;

	ret = read_dts_value_u32(dn, "product_type", &pdata->product_type);
	if (ret < 0)
		return ret;

	ret = read_dts_value_u32(dn, "dsi_bit_clk", &pdata->dsi_bit_clk);
	if (ret < 0)
		return ret;

	return ret;
}

static int mipi2edp_data_init(struct i2c_client *client, struct mipi2edp *pdata)
{
	int ret = 0;
	int i;

	mutex_init(&pdata->lock);

	ret = mipi2edp_bridge_chip_info_get(client->addr, &pdata->bridge_chip_info);
	if (ret)
		return ret;

	pdata->regmap = devm_regmap_init_i2c(client, pdata->bridge_chip_info.regmap_config);
	if (IS_ERR(pdata->regmap)) {
		HISI_DRM_ERR("failed to regmap iit i2c!");
		return -ENOMEM;
	}

	ret = mipi2edp_init_data_from_dts(client, pdata);
	if (ret)
		return ret;

	ret = mipi2edp_bridge_product_info_get(pdata->product_type, &pdata->bridge_product_info);
	if (ret)
		return ret;


	pdata->pinctrl = (struct pinctrl_data *)&pinctrl;
	ret = memset_s((void *)pdata->pinctrl, sizeof(struct pinctrl_data), 0, sizeof(struct pinctrl_data));
	if (ret)
		HISI_DRM_ERR("memset for pinctrl failed!");

	HISI_DRM_INFO("%s", pdata->bridge_chip_info.name);
	pdata->aux.name = pdata->bridge_chip_info.name;
	pdata->aux.dev = pdata->dev;
	pdata->aux.transfer = pdata->bridge_chip_info.transfer;

	for (i = 0; i < EDID_LEN; i++)
		pdata->edid_array[i] = 0;
	return ret;
}

static int bridge_pinctrl_init(struct i2c_client *client)
{
	int ret = 0;

	/* bridge pinctrl init */
	ret = pinctrl_cmds_tx(&client->dev, bridge_pinctrl_init_cmds, ARRAY_SIZE(bridge_pinctrl_init_cmds));
	if (ret != 0) {
		HISI_DRM_ERR("Init bridge pinctrl failed, ret=%d!", ret);
		return ret;
	}

	ret = pinctrl_cmds_tx(&client->dev, bridge_pinctrl_normal_cmds, ARRAY_SIZE(bridge_pinctrl_normal_cmds));
	if (ret) {
		HISI_DRM_ERR("failed to pinctrl_cmds_tx, ret=%d!", ret);
		return ret;
	}
	return ret;
}

static int get_mipi2edp_clk(struct mipi2edp *pdata, struct device *dev)
{
	/* get mipi2edp clk resource */
	int ret = 0;

	pdata->mipi2edp_clk = devm_clk_get(dev, "clk_nfc");
	if (IS_ERR(pdata->mipi2edp_clk)) {
		HISI_DRM_ERR("mipi2edp_clk not found!");
		ret = -ENXIO;
	}
	return ret;
}

int set_bl_props(struct mipi2edp *pdata, struct device *dev)
{
	int ret;

	pdata->bl = backlight_device_register("mipi2edp-backlight", dev, pdata, &mipi2edp_backlight_ops, NULL);
	if (IS_ERR(pdata->bl)) {
		HISI_DRM_ERR("failed to register backlight\n");
		ret = PTR_ERR(pdata->bl);
		pdata->bl = NULL;
		return -1;
	}
	pdata->bl->props.max_brightness = DEFAULT_MAX_BRIGHTNESS;
	pdata->bl->props.brightness = DEFAULT_MAX_BRIGHTNESS/2;

	return 0;
}

void aux_register(struct mipi2edp *pdata)
{
	if (pdata && pdata->aux.transfer)
		drm_dp_aux_register(&pdata->aux);
}

static int get_func_for_bridge(struct mipi2edp *pdata, struct device *dev)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	int ret;
#endif
	pdata->bridge.funcs = &mipi2edp_bridge_funcs;
	pdata->bridge.of_node = dev->of_node;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	ret = drm_bridge_add(&pdata->bridge);
	if (ret) {
		HISI_DRM_ERR("failed to add drm bridge on chip:%s!", pdata->bridge_chip_info.name);
		return ret;
	}
#else
	drm_bridge_add(&pdata->bridge);
#endif
	return 0;
}

static int mipi2edp_i2c_probe(
	struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct mipi2edp *pdata = NULL;
	struct device *dev = NULL;
	int ret = -1;

	HISI_DRM_INFO("+");

	ret = mipi2edp_probe_init(client, &dev, &pdata);
	if (ret)
		return ret;

	ret = mipi2edp_data_init(client, pdata);
	if (ret)
		goto err_out;

	ret = check_mipi2edp_onboard(pdata);
	if (ret)
		goto err_out;

	if (pdata->bridge_product_info.bridge_ops->bridge_init_private_data) {
		ret = pdata->bridge_product_info.bridge_ops->bridge_init_private_data(pdata, client);
		if (ret) {
			HISI_DRM_ERR("bridge init private data failed!product desc:%s", pdata->bridge_product_info.desc);
			goto err_out;
		}
	} else {
		HISI_DRM_INFO("product bridge_init_private_data not implement on product:%s", pdata->bridge_product_info.desc);
	}

	ret = bridge_pinctrl_init(client);
	if (ret)
		goto err_out;

	ret = get_mipi2edp_clk(pdata, dev);
	if (ret)
		goto err_out;

	HISI_DRM_INFO("mipi2edp_clk:[%lu]\n", clk_get_rate(pdata->mipi2edp_clk));

	if (pdata->bridge_product_info.bridge_ops->bridge_probe) {
		ret = pdata->bridge_product_info.bridge_ops->bridge_probe(pdata, client);
		if (ret) {
			HISI_DRM_ERR("bridge probe failed!product desc:%s", pdata->bridge_product_info.desc);
			goto err_out;
		}
	} else {
		HISI_DRM_INFO("product bridge_probe not implement on product:%s", pdata->bridge_product_info.desc);
	}

	ret = get_func_for_bridge(pdata, dev);
	if (ret)
		goto err_out;

	pm_runtime_enable(pdata->dev);

	HISI_DRM_INFO("-");

	return ret;

err_out:
	dev_set_drvdata(dev, NULL);
	devm_kfree(&client->dev, pdata);

	return ret;
}

static int mipi2edp_i2c_remove(struct i2c_client *client)
{
	struct mipi2edp *pdata = NULL;
	int ret = 0;

	HISI_DRM_INFO("+");

	if (!client) {
		HISI_DRM_ERR("client is nullptr!");
		return -EINVAL;
	}

	pdata = i2c_get_clientdata(client);
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.bridge_ops->bridge_remove) {
		ret = pdata->bridge_product_info.bridge_ops->bridge_remove(client);
		if (ret) {
			HISI_DRM_ERR("bridge remove failed!product desc:%s", pdata->bridge_product_info.desc);
			return ret;
		}
	} else {
		HISI_DRM_INFO("product bridge_remove not implement on product:%s", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");

	return 0;
}

static void mipi2edp_i2c_shutdown(struct i2c_client *client)
{
	struct device *dev = NULL;
	struct mipi2edp *pdata = NULL;
	int ret = 0;

	HISI_DRM_INFO("+");

	if (client == NULL) {
		HISI_DRM_ERR("client is nullptr!");
		return;
	}
	dev = &client->dev;
	pdata = dev_get_drvdata(dev);

	if (pdata == NULL) {
		HISI_DRM_ERR("pdata is nullptr!");
		return;
	}

	if (pdata->bridge_product_info.bridge_ops->bridge_shutdown) {
		ret = pdata->bridge_product_info.bridge_ops->bridge_shutdown(pdata);
		if (ret)
			HISI_DRM_ERR("bridge remove failed!product desc:%s", pdata->bridge_product_info.desc);
	} else {
		HISI_DRM_INFO("product bridge_shutdown not implement on product:%s", pdata->bridge_product_info.desc);
	}

	HISI_DRM_INFO("-");
}

static const struct i2c_device_id mipi2edp_i2c_ids[] = {
	{ "ti,sn65dsix6", 0 },
	{ "lt,lt9711a", 0 },
	{}
};

static const struct of_device_id mipi2edp_i2c_of_ids[] = {
	{.compatible = "ti,sn65dsix6"},
	{.compatible = "lt,lt9711a"},
	{}
};


static struct i2c_driver mipi2edp_i2c_driver = {
	.driver = {
		.name = "mipi2edp",
		.of_match_table = mipi2edp_i2c_of_ids,
		.pm = &mipi2edp_bridge_pm_ops,
	},
	.id_table = mipi2edp_i2c_ids,
	.probe = mipi2edp_i2c_probe,
	.remove = mipi2edp_i2c_remove,
	.shutdown = mipi2edp_i2c_shutdown,
};

static int __init mipi2edp_init(void)
{
	int ret = 0;

	HISI_DRM_INFO("+");

	ret = i2c_add_driver(&mipi2edp_i2c_driver);
	if (ret) {
		HISI_DRM_ERR("failed to i2c_add_driver, ret=%d!", ret);
		return ret;
	}

	HISI_DRM_INFO("-");

	return ret;
}
module_init(mipi2edp_init);

static void __exit mipi2edp_exit(void)
{
	HISI_DRM_INFO("+.");
	i2c_del_driver(&mipi2edp_i2c_driver);
	HISI_DRM_INFO("-.");
}
module_exit(mipi2edp_exit);

MODULE_ALIAS("huawei mipi2edp bridge module");
MODULE_DESCRIPTION("huawei mipi2edp bridge driver");
MODULE_LICENSE("GPL");

/*lint +e548 +e574 +e578*/
