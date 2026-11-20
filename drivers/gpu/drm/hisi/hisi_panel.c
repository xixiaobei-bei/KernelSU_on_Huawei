/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_dsi_reg.h"
#include "hisi_dpe.h"
#include "hisi_drm_drv.h"
#include "hisi_panel.h"

struct mipi_ifbc_division g_mipi_ifbc_division[MIPI_DPHY_NUM][IFBC_TYPE_MAX] = {
	/* single mipi */
	{
		/* none */
		{XRES_DIV_1, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* orise2x */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		/* orise3x */
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_1, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		/* himax2x */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_2, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		/* rsp2x */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_3, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		/* rsp3x  [NOTE]reality: xres_div = 1.5, yres_div = 2,
		 * amended in "mipi_ifbc_get_rect" function
		 */
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_4, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		/* vesa2x_1pipe */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		/* vesa3x_1pipe */
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		/* vesa2x_2pipe */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		/* vesa3x_2pipe */
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		/* vesa3.75x_2pipe */
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3}
	},

	/* dual mipi */
	{
		/* none */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_1, PXL0_DSI_GT_EN_3},
		/* orise2x */
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		/* orise3x */
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_1, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, PXL0_DSI_GT_EN_3},
		/* himax2x */
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_2, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		/* rsp2x */
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_3, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		/* rsp3x */
		{XRES_DIV_3, YRES_DIV_2, IFBC_COMP_MODE_4, PXL0_DIV2_GT_EN_CLOSE,
		PXL0_DIV4_GT_EN_OPEN, PXL0_DIVCFG_5, PXL0_DSI_GT_EN_3},
		/* vesa2x_1pipe */
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		/* vesa3x_1pipe */
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, PXL0_DSI_GT_EN_3},
		/* vesa2x_2pipe */
		{XRES_DIV_4, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_3, PXL0_DSI_GT_EN_3},
		/* vesa3x_2pipe */
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, 3},
		/* vesa3.75x_2pipe */
		{XRES_DIV_6, YRES_DIV_1, IFBC_COMP_MODE_6, PXL0_DIV2_GT_EN_OPEN,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, 3}
	}
};

bool is_mipi_video_panel(struct hisi_panel_info *pinfo)
{
	if (pinfo == NULL) {
		HISI_DRM_ERR("pinfo is nullptr!");
		return false;
	}

	if (pinfo->type & (PANEL_MIPI_VIDEO |
		PANEL_DUAL_MIPI_VIDEO | PANEL_RGB2MIPI)) {
		return true;
	}

	return false;
}

bool is_mipi_cmd_panel(struct hisi_panel_info *pinfo)
{
	if (pinfo == NULL) {
		HISI_DRM_ERR("pinfo is nullptr!");
		return false;
	}

	if (pinfo->type & (PANEL_MIPI_CMD | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

static int check_dtype_gpio(struct gpio_desc *cm)
{
	if (cm->dtype == DTYPE_GPIO_INPUT) {
		if (gpio_direction_input(*(cm->gpio)) != 0) {
			HISI_DRM_ERR("failed to gpio_direction_input, lable=%s, gpio=%d!\n",
				cm->label, *(cm->gpio));
			return -1;
		}
	} else if (cm->dtype == DTYPE_GPIO_OUTPUT) {
		if (gpio_direction_output(*(cm->gpio), cm->value) != 0) {
			HISI_DRM_ERR("failed to gpio_direction_output, label%s, gpio=%d!\n",
				cm->label, *(cm->gpio));
			return -1;
		}
	} else if (cm->dtype == DTYPE_GPIO_REQUEST) {
		if (gpio_request(*(cm->gpio), cm->label) != 0) {
			HISI_DRM_ERR("failed to gpio_request, lable=%s, gpio=%d!\n",
				cm->label, *(cm->gpio));
			return -1;
		}
	} else if (cm->dtype == DTYPE_GPIO_FREE) {
		gpio_free(*(cm->gpio));
	} else {
		HISI_DRM_ERR("dtype=%x NOT supported\n", cm->dtype);
		return -1;
	}
	return 0;
}

int gpio_cmds_tx(struct gpio_desc *cmds, int cnt)
{
	int ret;
	struct gpio_desc *cm = NULL;
	int i = 0;

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if ((cm == NULL) || (cm->label == NULL)) {
			HISI_DRM_ERR("cm or cm->label is null! index=%d\n", i);
			ret = -1;
			goto error;
		}

		if (gpio_is_valid(*(cm->gpio)) == 0) {
			HISI_DRM_ERR("gpio invalid, dtype=%d, lable=%s, gpio=%d!\n",
				cm->dtype, cm->label, *(cm->gpio));
			ret = -1;
			goto error;
		}

		ret = check_dtype_gpio(cm);
		if (ret)
			goto error;

		if (cm->wait) {
			if (cm->waittype == WAIT_TYPE_US)
				udelay(cm->wait);
			else if (cm->waittype == WAIT_TYPE_MS)
				mdelay(cm->wait);
			else
				mdelay(cm->wait * 1000);
		}
		cm++;
	}

	return 0;

error:
	return ret;
}

static int get_dtype_pinctrl(struct device *dev,
	struct pinctrl_cmd_desc *cm, int i)
{
	if (!cm->pctrl_data) {
		HISI_DRM_ERR("pctrl_data is NULL\n", i);
		return -EINVAL;
	}

	cm->pctrl_data->p = devm_pinctrl_get(dev);
	if (IS_ERR(cm->pctrl_data->p)) {
		HISI_DRM_ERR("failed to get p, index=%d!\n", i);
		return -1;
	}
	return 0;
}

static int get_dtype_pinctrl_state(struct pinctrl_cmd_desc *cm, int i)
{
	if (!cm->pctrl_data) {
		HISI_DRM_ERR("pctrl_data is NULL\n");
		return -1;
	}

	if (cm->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
		cm->pctrl_data->pinctrl_def = pinctrl_lookup_state(cm->pctrl_data->p,
			PINCTRL_STATE_DEFAULT);
		if (IS_ERR(cm->pctrl_data->pinctrl_def)) {
			HISI_DRM_ERR("failed to get pinctrl_def, index=%d!\n", i);
			return -1;
		}
	} else if (cm->mode == DTYPE_PINCTRL_STATE_IDLE) {
		cm->pctrl_data->pinctrl_idle = pinctrl_lookup_state(cm->pctrl_data->p,
			PINCTRL_STATE_IDLE);
		if (IS_ERR(cm->pctrl_data->pinctrl_idle)) {
			HISI_DRM_ERR("failed to get pinctrl_idle, index=%d!\n", i);
			return -1;
		}
	} else {
		HISI_DRM_ERR("unknown pinctrl type to get!\n");
		return -1;
	}
	return 0;
}

static int set_dtype_pinctrl(struct pinctrl_cmd_desc *cm)
{
	int ret;

	if (!cm->pctrl_data) {
		HISI_DRM_ERR("pctrl_data is NULL\n");
		return -1;
	}

	if (cm->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
		if (cm->pctrl_data->p && cm->pctrl_data->pinctrl_def) {
			ret = pinctrl_select_state(cm->pctrl_data->p,
				cm->pctrl_data->pinctrl_def);
			if (ret) {
				HISI_DRM_ERR("could not set this pin to default state!\n");
				ret = -1;
				return ret;
			}
		}
	} else if (cm->mode == DTYPE_PINCTRL_STATE_IDLE) {
		if (cm->pctrl_data->p && cm->pctrl_data->pinctrl_idle) {
			ret = pinctrl_select_state(cm->pctrl_data->p,
				cm->pctrl_data->pinctrl_idle);
			if (ret) {
				HISI_DRM_ERR("could not set this pin to idle state!\n");
				ret = -1;
				return ret;
			}
		}
	} else {
		ret = -1;
		HISI_DRM_ERR("unknown pinctrl type to set!\n");
		return ret;
	}
	return 0;
}

int pinctrl_cmds_tx(struct device *dev,
	struct pinctrl_cmd_desc *cmds, int cnt)
{
	int ret = 0;

	int i = 0;
	struct pinctrl_cmd_desc *cm = NULL;

	if (!dev) {
		HISI_DRM_ERR("pdev is NULL");
		return -EINVAL;
	}

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (cm == NULL) {
			HISI_DRM_ERR("cm is null! index=%d\n", i);
			continue;
		}

		if (cm->dtype == DTYPE_PINCTRL_GET) {
			ret = get_dtype_pinctrl(dev, cm, i);
			if (ret)
				goto err;
		} else if (cm->dtype == DTYPE_PINCTRL_STATE_GET) {
			ret = get_dtype_pinctrl_state(cm, i);
			if (ret)
				goto err;
		} else if (cm->dtype == DTYPE_PINCTRL_SET) {
			ret = set_dtype_pinctrl(cm);
			if (ret)
				goto err;
		} else if (cm->dtype == DTYPE_PINCTRL_PUT) {
			if (cm->pctrl_data->p)
				pinctrl_put(cm->pctrl_data->p);
		} else {
			HISI_DRM_ERR("not supported command type!\n");
			ret = -1;
			goto err;
		}

		cm++;
	}

	return 0;

err:
	return ret;
}

