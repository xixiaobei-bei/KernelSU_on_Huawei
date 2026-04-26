/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"
#include "hisi_fb_panel.h"
#include "panel/mipi_lcd_utils.h"
#if defined(CONFIG_DEVICE_ATTRS)
#include "product/attrs/hisi_panel_attrs.h"
#endif
#if defined(CONFIG_TEE_TUI)
#include "tui.h"
#endif

/*lint -e574 -e647 -e568 -e685 -e578*/
DEFINE_SEMAPHORE(hisi_fb_dts_resource_sem);

/* none, orise2x, orise3x, himax2x, rsp2x, rsp3x, vesa2x, vesa3x */
mipi_ifbc_division_t g_mipi_ifbc_division[MIPI_DPHY_NUM][IFBC_TYPE_MAX] = {
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
		/* rsp3x  [NOTE]reality: xres_div = 1.5, yres_div = 2, amended in "mipi_ifbc_get_rect" function */
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
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_2, PXL0_DSI_GT_EN_3},
		/* SPR+DSC1.2 need special calulation for compression.
		 * It calucate the div in get_hsize_after_spr_dsc().
		 */
		/* vesa2.66x_pipe depend on SPR */
		{XRES_DIV_1, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* vesa2.66x_pipe depend on SPR */
		{XRES_DIV_1, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* vesa2x_pipe depend on SPR */
		{XRES_DIV_1, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* vesa2x_pipe depend on SPR */
		{XRES_DIV_1, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* vesa3.75x_1pipe */
		{XRES_DIV_3, YRES_DIV_1, IFBC_COMP_MODE_5, PXL0_DIV2_GT_EN_CLOSE,
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
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_5, 3},
		/* SPR+DSC1.2 need special calulation for compression.
		 * It calucate the div in get_hsize_after_spr_dsc().
		 */
		/* vesa2.66x_pipe depend on SPR */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* vesa2.66x_pipe depend on SPR */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* vesa2x_pipe depend on SPR */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1},
		/* vesa2x_pipe depend on SPR */
		{XRES_DIV_2, YRES_DIV_1, IFBC_COMP_MODE_0, PXL0_DIV2_GT_EN_CLOSE,
			PXL0_DIV4_GT_EN_CLOSE, PXL0_DIVCFG_0, PXL0_DSI_GT_EN_1}
	}
};

int hisi_lcd_backlight_on(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	int ret = 0;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +!\n", dpufd->index);

	if (dpufd->panel_info.bl_set_type & BL_SET_BY_PWM)
		ret = hisi_pwm_on(pdev);
	else if (dpufd->panel_info.bl_set_type & BL_SET_BY_BLPWM)
		ret = hisi_blpwm_on(pdev);
	else if (dpufd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM)
		ret = hisi_sh_blpwm_on(pdev);
	else if (dpufd->panel_info.bl_set_type & BL_SET_BY_MIPI)
		;
	else
		DPU_FB_ERR("No such bl_set_type: %d!\n", dpufd->panel_info.bl_set_type);

	DPU_FB_DEBUG("fb%d, -!\n", dpufd->index);

	return ret;
}

int hisi_lcd_backlight_off(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	int ret = 0;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +!\n", dpufd->index);

	if (dpufd->panel_info.bl_set_type & BL_SET_BY_PWM)
		ret = hisi_pwm_off(pdev);
	else if (dpufd->panel_info.bl_set_type & BL_SET_BY_BLPWM)
		ret = hisi_blpwm_off(pdev);
	else if (dpufd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM)
		ret = hisi_sh_blpwm_off(pdev);
	else if (dpufd->panel_info.bl_set_type & BL_SET_BY_MIPI)
		;
	else
		DPU_FB_ERR("No such bl_set_type: %d!\n", dpufd->panel_info.bl_set_type);

	DPU_FB_DEBUG("fb%d, -!\n", dpufd->index);

	return ret;
}

static int gpio_cmds_tx_check_param(struct gpio_desc *cmds, int index)
{
	if ((cmds == NULL) || (cmds->label == NULL)) {
		DPU_FB_ERR("cmds or cmds->label is NULL! index=%d\n", index);
		return -1;
	}

	if (!gpio_is_valid(*(cmds->gpio))) {
		DPU_FB_ERR("gpio invalid, dtype=%d, lable=%s, gpio=%d!\n",
			cmds->dtype, cmds->label, *(cmds->gpio));
		return -1;
	}

	return 0;
}

static void cmds_tx_delay(int wait, int waittype)
{
	if (wait) {
		if (waittype == WAIT_TYPE_US)
			udelay(wait);
		else if (waittype == WAIT_TYPE_MS)
			mdelay(wait);
		else
			mdelay(wait * 1000);  /* ms */
	}
}

int gpio_cmds_tx(struct gpio_desc *cmds, int cnt)
{
	int ret;
	int i;

	struct gpio_desc *cm = cmds;

	for (i = 0; i < cnt; i++) {

		ret = gpio_cmds_tx_check_param(cm, i);
		if (ret)
			goto error;

		if (cm->dtype == DTYPE_GPIO_INPUT) {
			if (gpio_direction_input(*(cm->gpio)) != 0) {
				DPU_FB_ERR("failed to gpio_direction_input, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = -1;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_OUTPUT) {
			if (gpio_direction_output(*(cm->gpio), cm->value) != 0) {
				DPU_FB_ERR("failed to gpio_direction_output, label%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = -1;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_REQUEST) {
			if (gpio_request(*(cm->gpio), cm->label) != 0) {
				DPU_FB_ERR("failed to gpio_request, lable=%s, gpio=%d!\n",
					cm->label, *(cm->gpio));
				ret = -1;
				goto error;
			}
		} else if (cm->dtype == DTYPE_GPIO_FREE) {
			gpio_free(*(cm->gpio));
		} else {
			DPU_FB_ERR("dtype=%x NOT supported\n", cm->dtype);
			ret = -1;
			goto error;
		}

		cmds_tx_delay(cm->wait, cm->waittype);
		cm++;
	}

	return 0;

error:
	return ret;
}

int resource_cmds_tx(struct platform_device *pdev,
	struct resource_desc *cmds, int cnt)
{
	int ret = 0;
	struct resource *res = NULL;
	struct resource_desc *cm = NULL;
	int i;

	if (!pdev) {
		DPU_FB_ERR("pdev is NULL\n");
		return -EINVAL;
	}
	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if ((cm == NULL) || (cm->name == NULL)) {
			DPU_FB_ERR("cm or cm->name is NULL! index=%d\n", i);
			ret = -1;
			goto error;
		}

		res = platform_get_resource_byname(pdev, cm->flag, cm->name);
		if (!res) {
			DPU_FB_ERR("failed to get %s resource!\n", cm->name);
			ret = -1;
			goto error;
		}

		*(cm->value) = res->start;

		cm++;
	}

error:
	return ret;
}

int spi_cmds_tx(struct spi_device *spi, struct spi_cmd_desc *cmds, int cnt)
{
	return 0;
}

int vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int cnt)
{
	struct vcc_desc *cm = NULL;
	int i;

	if (g_fpga_flag == 1)
		return 0;

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		dpu_check_and_return((!cm || !cm->id), -1, ERR, "cm or cm->id is NULL! index=%d\n", i);

		if (cm->dtype == DTYPE_VCC_GET) {
			dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

			*(cm->regulator) = devm_regulator_get(&pdev->dev, cm->id);
			dpu_check_and_return(IS_ERR(*(cm->regulator)), -1,
				ERR, "failed to get %s regulator!\n", cm->id);
		} else if (cm->dtype == DTYPE_VCC_PUT) {
			if (!IS_ERR(*(cm->regulator)))
				devm_regulator_put(*(cm->regulator));
		} else if (cm->dtype == DTYPE_VCC_ENABLE) {
			if (!IS_ERR(*(cm->regulator))) {
				if (regulator_enable(*(cm->regulator)) != 0) {
					DPU_FB_ERR("failed to enable %s regulator!\n", cm->id);
					return -1;
				}
			}
		} else if (cm->dtype == DTYPE_VCC_DISABLE) {
			if (!IS_ERR(*(cm->regulator))) {
				if (regulator_disable(*(cm->regulator)) != 0) {
					DPU_FB_ERR("failed to disable %s regulator!\n", cm->id);
					return -1;
				}
			}
		} else if (cm->dtype == DTYPE_VCC_SET_VOLTAGE) {
			if (!IS_ERR(*(cm->regulator))) {
				if (regulator_set_voltage(*(cm->regulator), cm->min_uV, cm->max_uV) != 0) {
					DPU_FB_ERR("failed to set %s regulator voltage!\n", cm->id);
					return -1;
				}
			}
		} else {
			DPU_FB_ERR("dtype=%x NOT supported\n", cm->dtype);
			return -1;
		}

		cmds_tx_delay(cm->wait, cm->waittype);
		cm++;
	}

	return 0;
}

static int pinctrl_get_state_by_mode(struct pinctrl_cmd_desc *cmds, int index)
{
	if (cmds->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
		cmds->pctrl_data->pinctrl_def = pinctrl_lookup_state(cmds->pctrl_data->p, PINCTRL_STATE_DEFAULT);
		if (IS_ERR(cmds->pctrl_data->pinctrl_def)) {
			DPU_FB_ERR("failed to get pinctrl_def, index=%d!\n", index);
			return -1;
		}
	} else if (cmds->mode == DTYPE_PINCTRL_STATE_IDLE) {
		cmds->pctrl_data->pinctrl_idle = pinctrl_lookup_state(cmds->pctrl_data->p, PINCTRL_STATE_IDLE);
		if (IS_ERR(cmds->pctrl_data->pinctrl_idle)) {
			DPU_FB_ERR("failed to get pinctrl_idle, index=%d!\n", index);
			return -1;
		}
	} else {
		DPU_FB_ERR("unknown pinctrl type to get!\n");
		return -1;
	}

	return 0;
}

static int pinctrl_set_state_by_mode(struct pinctrl_cmd_desc *cmds)
{
	int ret;

	if (cmds->mode == DTYPE_PINCTRL_STATE_DEFAULT) {
		if (cmds->pctrl_data->p && cmds->pctrl_data->pinctrl_def) {
			ret = pinctrl_select_state(cmds->pctrl_data->p, cmds->pctrl_data->pinctrl_def);
			if (ret) {
				DPU_FB_ERR("could not set this pin to default state!\n");
				return ret;
			}
		}
	} else if (cmds->mode == DTYPE_PINCTRL_STATE_IDLE) {
		if (cmds->pctrl_data->p && cmds->pctrl_data->pinctrl_idle) {
			ret = pinctrl_select_state(cmds->pctrl_data->p, cmds->pctrl_data->pinctrl_idle);
			if (ret) {
				DPU_FB_ERR("could not set this pin to idle state!\n");
				return ret;
			}
		}
	} else {
		ret = -1;
		DPU_FB_ERR("unknown pinctrl type to set!\n");
		return ret;
	}

	return 0;
}

int pinctrl_cmds_tx(struct platform_device *pdev, struct pinctrl_cmd_desc *cmds, int cnt)
{
	int ret;
	int i;
	struct pinctrl_cmd_desc *cm = NULL;

	if (g_fpga_flag == 1)
		return 0;

	cm = cmds;

	for (i = 0; i < cnt; i++) {
		if (!cm) {
			DPU_FB_ERR("cm is NULL! index=%d\n", i);
			continue;
		}

		if (cm->dtype == DTYPE_PINCTRL_GET) {
			dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

			cm->pctrl_data->p = devm_pinctrl_get(&pdev->dev);
			if (IS_ERR(cm->pctrl_data->p)) {
				ret = -1;
				DPU_FB_ERR("failed to get p, index=%d!\n", i);
				goto err;
			}
		} else if (cm->dtype == DTYPE_PINCTRL_STATE_GET) {
			ret = pinctrl_get_state_by_mode(cm, i);
			if (ret)
				goto err;
		} else if (cm->dtype == DTYPE_PINCTRL_SET) {
			ret = pinctrl_set_state_by_mode(cm);
			if (ret) {
				ret = -1;
				goto err;
			}
		} else if (cm->dtype == DTYPE_PINCTRL_PUT) {
			if (cm->pctrl_data->p)
				pinctrl_put(cm->pctrl_data->p);
		} else {
			DPU_FB_ERR("not supported command type!\n");
			ret = -1;
			goto err;
		}

		cm++;
	}

	return 0;

err:
	return ret;
}

int panel_next_set_fastboot(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");

	pdata = (struct dpu_fb_panel_data *)pdev->dev.platform_data;
	if (pdata != NULL) {
		next_pdev = pdata->next;
		if (next_pdev != NULL) {
			next_pdata = (struct dpu_fb_panel_data *)next_pdev->dev.platform_data;
			if ((next_pdata) && (next_pdata->set_fastboot))
				ret = next_pdata->set_fastboot(next_pdev);
		}
	}

	return ret;
}

int panel_next_on(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->on))
			ret = next_pdata->on(next_pdev);
	}

	return ret;
}

int panel_next_off(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->off))
			ret = next_pdata->off(next_pdev);
	}

	return ret;
}

int panel_next_lp_ctrl(struct platform_device *pdev, bool lp_enter)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lp_ctrl))
			ret = next_pdata->lp_ctrl(next_pdev, lp_enter);
	}

	return ret;
}

int panel_next_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->remove))
			ret = next_pdata->remove(next_pdev);
	}

	return ret;
}

int panel_next_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->set_backlight))
			ret = next_pdata->set_backlight(next_pdev, bl_level);
	}

	return ret;
}

int panel_next_vsync_ctrl(struct platform_device *pdev, int enable)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->vsync_ctrl))
			ret = next_pdata->vsync_ctrl(next_pdev, enable);
	}

	return ret;
}

int panel_next_lcd_fps_scence_handle(struct platform_device *pdev, uint32_t scence)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_fps_scence_handle))
			ret = next_pdata->lcd_fps_scence_handle(next_pdev, scence);
	}

	return ret;
}

int panel_next_lcd_fps_updt_handle(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->lcd_fps_updt_handle))
			ret = next_pdata->lcd_fps_updt_handle(next_pdev);
	}

	return ret;
}

int panel_next_esd_handle(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "pdev is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->esd_handle))
			ret = next_pdata->esd_handle(next_pdev);
	}

	return ret;
}

int panel_next_set_display_region(struct platform_device *pdev, struct dss_rect *dirty)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return((!pdev || !dirty), -EINVAL, ERR, "pedv or dirty is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "pdata is NULL\n");

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->set_display_region))
			ret = next_pdata->set_display_region(next_pdev, dirty);
	}

	return ret;
}

bool is_lcd_dfr_support(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd)
		return false;

#if defined(CONFIG_HISI_DISPLAY_DFR)
	if (dpufd->panel_info.dfr_support_value != DISPLAY_FPS_DEFAULT)
		return true;
	else
		return false;
#else
	return false;
#endif
}

bool is_ldi_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.type & PANEL_LCDC)
		return true;

	return false;
}

bool is_mipi_cmd_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.type & (PANEL_MIPI_CMD | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_mipi_cmd_panel_ext(struct dpu_panel_info *pinfo)
{
	if (!pinfo) {
		DPU_FB_ERR("pinfo is NULL\n");
		return false;
	}

	if (pinfo->type & (PANEL_MIPI_CMD | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_mipi_video_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.type & (PANEL_MIPI_VIDEO | PANEL_DUAL_MIPI_VIDEO | PANEL_RGB2MIPI))
		return true;

	return false;
}

bool is_dp_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.type & PANEL_DP)
		return true;

	return false;
}

bool is_mipi_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.type & (PANEL_MIPI_VIDEO | PANEL_MIPI_CMD |
		PANEL_DUAL_MIPI_VIDEO | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_hisync_mode(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (is_mipi_video_panel(dpufd) && dpufd->panel_info.hisync_mode)
		return true;

	return false;
}

bool is_video_idle_ctrl_mode(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (is_mipi_video_panel(dpufd) && dpufd->panel_info.video_idle_mode)
		return true;

	return false;
}

bool is_dual_mipi_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.type & (PANEL_DUAL_MIPI_VIDEO | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool need_config_dsi0(struct dpu_fb_data_type *dpufd)
{
	if ((dpufd->index == PRIMARY_PANEL_IDX) &&
		(is_dual_mipi_panel(dpufd) || is_dsi0_pipe_switch_connector(dpufd)))
		return true;

	return false;
}

bool need_config_dsi1(struct dpu_fb_data_type *dpufd)
{
	if (is_dual_mipi_panel(dpufd) ||
		(dpufd->index == EXTERNAL_PANEL_IDX) ||
		(is_dsi1_pipe_switch_connector(dpufd)))
		return true;

	return false;
}

bool is_dual_mipi_panel_ext(struct dpu_panel_info *pinfo)
{
	if (!pinfo) {
		DPU_FB_ERR("pinfo is NULL\n");
		return false;
	}

	if (pinfo->type & (PANEL_DUAL_MIPI_VIDEO | PANEL_DUAL_MIPI_CMD))
		return true;

	return false;
}

bool is_hisi_writeback_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.type & PANEL_WRITEBACK)
		return true;

	return false;
}

bool is_ifbc_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if (dpufd->panel_info.ifbc_type != IFBC_TYPE_NONE)
		return true;

	return false;
}

bool is_ifbc_vesa_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if ((dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA2X_SINGLE) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA3X_SINGLE) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA2X_DUAL) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA3X_DUAL) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA3_75X_DUAL) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA4X_SINGLE_SPR) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA4X_DUAL_SPR) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA3_75X_SINGLE) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA2X_SINGLE_SPR) ||
		(dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA2X_DUAL_SPR) ||
		dpufd->panel_info.dynamic_dsc_support)
		return true;

	return false;
}

bool is_single_slice_dsc_panel(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if ((dpufd->panel_info.ifbc_type == IFBC_MODE_VESA3X_SINGLE) ||
		(dpufd->panel_info.ifbc_type == IFBC_MODE_VESA3_75X_SINGLE)) {
		return true;
	}

	return false;
}

bool is_dsi0_pipe_switch_connector(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if ((dpufd->panel_info.mipi.dsi_te_type == DSI0_TE0_TYPE) &&
		(g_dsi_pipe_switch_connector == PIPE_SWITCH_CONNECT_DSI0))
		return true;

	return false;
}

bool is_dsi1_pipe_switch_connector(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if ((g_dsi_pipe_switch_connector == PIPE_SWITCH_CONNECT_DSI1) ||
		(dpufd->panel_info.mipi.dsi_te_type == DSI1_TE1_TYPE))
		return true;

	return false;
}

ssize_t panel_mode_switch_store(struct dpu_fb_data_type *dpufd,
	const char *buf, size_t count)
{
	struct dpu_panel_info *pinfo = NULL;
	uint8_t mode_switch_to_tmp = MODE_8BIT;
	int str_len = 0;
	int i;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	pinfo = &(dpufd->panel_info);

	for (i = 0; buf[i] != '\0' && buf[i] != '\n'; i++) {
		str_len++;
		if (str_len >= 9) {  /* max input param */
			DPU_FB_ERR("invalid input parameter: n_str = %d, count = %ld\n", str_len, count);
			break;
		}
	}

	if (str_len != 8) {  /* valid input param is 8 bytes */
		DPU_FB_ERR("invalid input parameter: n_str = %d, count = %ld\n", str_len, count);
		return count;
	}

	if (!dpufd->panel_info.panel_mode_swtich_support) {
		DPU_FB_INFO("fb%d, not support!\n", dpufd->index);
		return count;
	}

	if (pinfo->current_mode != pinfo->mode_switch_to) {
		DPU_FB_ERR("last switch action is not over\n");
		return count;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (g_debug_panel_mode_switch == 1) {
		DPU_FB_ERR("panel_mode_switch is closed\n");
		return count;
	}

	if (!strncmp(buf, PANEL_10BIT_VIDEO_MODE_STR, str_len)) {
		mode_switch_to_tmp = MODE_10BIT_VIDEO_3X;
	} else if (!strncmp(buf, PANEL_8BIT_CMD_MODE_STR, str_len)) {
		mode_switch_to_tmp = MODE_8BIT;
	} else {
		DPU_FB_ERR("fb%d, unknown panel mode!\n", dpufd->index);
		return count;
	}

	if (mode_switch_to_tmp != pinfo->mode_switch_to) {
		pinfo->mode_switch_to = mode_switch_to_tmp;
	} else {
		DPU_FB_ERR("current mode or mode_switch_to is already %d !\n", mode_switch_to_tmp);
		return count;
	}

	DPU_FB_INFO("switch panel mode to %s\n", buf);
	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return count;
}

static uint32_t mode_switch_wait_vfp(struct dpu_fb_data_type *dpufd, uint8_t mode_switch_to)
{
	uint32_t wait_time = 0;
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);

	/* Data volume conversion  */
	if (mode_switch_to == MODE_8BIT) {
		wait_time = (pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width) / 3;
		wait_time += pinfo->xres * 30 / 24 / 3;
		wait_time *= pinfo->ldi.v_front_porch;
		wait_time = wait_time / (uint32_t)(pinfo->pxl_clk_rate / 1000000 / 3); /* HZ to MHz */
	}

	DPU_FB_DEBUG("wait_time:%d us\n", wait_time);

	return wait_time;
}

void panel_mode_switch_isr_handler(struct dpu_fb_data_type *dpufd, uint8_t mode_switch_to)
{
	struct dpu_panel_info *pinfo = NULL;
	int ret;
	uint32_t wait_time_us;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	pinfo = &(dpufd->panel_info);

	DPU_FB_DEBUG("+\n");
	DPU_FB_INFO("panel mode switch not support!\n");
	return;
	DPU_FB_DEBUG("LDI_CTRL = 0x%x\n", inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CTRL));

	wait_time_us = mode_switch_wait_vfp(dpufd, mode_switch_to);
	udelay(wait_time_us);

	ret = switch_panel_mode(dpufd, mode_switch_to);

	single_frame_update(dpufd);

	if (ret == 0) {
		DPU_FB_INFO("panel mode successfully switched from %d to %d\n", pinfo->current_mode, mode_switch_to);
		pinfo->current_mode = pinfo->mode_switch_to;
	}

	DPU_FB_DEBUG("-\n");
}

bool mipi_panel_check_reg(struct dpu_fb_data_type *dpufd, uint32_t *read_value, int buf_len)
{
	int ret;
	char __iomem *mipi_dsi_base = NULL;
	/* mipi reg default value */
	char lcd_reg_05[] = {0x05};
	char lcd_reg_0a[] = {0x0a};
	char lcd_reg_0e[] = {0x0e};
	char lcd_reg_0f[] = {0x0f};

	struct dsi_cmd_desc lcd_check_reg[] = {
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_05), lcd_reg_05},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0a), lcd_reg_0a},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0e), lcd_reg_0e},
		{DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US,
			sizeof(lcd_reg_0f), lcd_reg_0f},
	};

	mipi_dsi_base = get_mipi_dsi_base(dpufd);
	ret = mipi_dsi_cmds_rx_with_check_fifo(read_value, lcd_check_reg, buf_len, mipi_dsi_base);
	if (ret) {
		DPU_FB_ERR("Read error number: %d\n", ret);
		return false;
	}

	return true;
}

/*lint -save -e573*/
int mipi_ifbc_get_rect(struct dpu_fb_data_type *dpufd, struct dss_rect *rect)
{
	uint32_t ifbc_type;
	uint32_t mipi_idx;
	uint32_t xres_div;
	uint32_t yres_div;
	struct panel_dsc_info *panel_dsc_info = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return(!rect, -EINVAL, ERR, "rect is NULL\n");

	ifbc_type = dpufd->panel_info.ifbc_type;
	panel_dsc_info = &(dpufd->panel_info.panel_dsc_info);
	dpu_check_and_return((ifbc_type >= IFBC_TYPE_MAX), -EINVAL, ERR, "ifbc_type is invalid\n");

	mipi_idx = is_dual_mipi_panel(dpufd) ? 1 : 0;

	xres_div = g_mipi_ifbc_division[mipi_idx][ifbc_type].xres_div;
	yres_div = g_mipi_ifbc_division[mipi_idx][ifbc_type].yres_div;

	if ((rect->w % xres_div) > 0)
		DPU_FB_INFO("fb%d, xres: %d is not division_h: %d pixel aligned!\n", dpufd->index, rect->w, xres_div);

	if ((rect->h % yres_div) > 0)
		DPU_FB_ERR("fb%d, yres: %d is not division_v: %d pixel aligned!\n", dpufd->index, rect->h, yres_div);

	/* [NOTE] rsp3x && single_mipi CMD mode amended xres_div = 1.5, */
	/* yres_div = 2 ,VIDEO mode amended xres_div = 3, yres_div = 1 */
	if ((mipi_idx == 0) && (ifbc_type == IFBC_TYPE_RSP3X)) {
		rect->w *= 2;
		rect->h /= 2;
	}
	dpu_check_and_return((xres_div == 0), -EINVAL, ERR, "xres_div is 0\n");
	if ((dpufd->panel_info.mode_switch_to == MODE_10BIT_VIDEO_3X)
		&& (dpufd->panel_info.ifbc_type == IFBC_TYPE_VESA3X_DUAL)) {
		rect->w = rect->w * 30 / 24 / xres_div;
	} else {
		if ((rect->w % xres_div) > 0)
			rect->w = ((panel_dsc_info->dsc_info.chunk_size + panel_dsc_info->dsc_insert_byte_num)
			* (panel_dsc_info->dual_dsc_en + 1)) / xres_div;
		else
			rect->w /= xres_div;
	}

	rect->h /= yres_div;

	return 0;
}

/*lint -restore*/
void dpufb_snd_cmd_before_frame(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	if (!dpufd->panel_info.snd_cmd_before_frame_support)
		return;

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	dpu_check_and_no_retval(!pdata, ERR, "pdata is NULL\n");

	if (pdata->snd_cmd_before_frame != NULL)
		pdata->snd_cmd_before_frame(dpufd->pdev);
}

static void hisi_fb_lcdc_rgb_resource_ready(uint32_t bl_type, bool *flag)
{
	if ((g_dts_resouce_ready & DTS_FB_RESOURCE_INIT_READY) && (g_dts_resouce_ready & DTS_SPI_READY)) {
		if (bl_type & (BL_SET_BY_PWM | BL_SET_BY_BLPWM)) {
			if (g_dts_resouce_ready & DTS_PWM_READY)
				*flag = false;
		} else {
			*flag = false;
		}
	}
}

static void hisi_fb_mipi_resource_ready(uint32_t bl_type, bool *flag)
{
	if (g_dts_resouce_ready & DTS_FB_RESOURCE_INIT_READY) {
		if (bl_type & (BL_SET_BY_PWM | BL_SET_BY_BLPWM)) {
			if (g_dts_resouce_ready & DTS_PWM_READY)
				*flag = false;
		} else {
			*flag = false;
		}
	}
}

bool hisi_fb_device_probe_defer(uint32_t panel_type, uint32_t bl_type)
{
	bool flag = true;

	down(&hisi_fb_dts_resource_sem);

	switch (panel_type) {
	case PANEL_NO:
		if (g_dts_resouce_ready & DTS_FB_RESOURCE_INIT_READY)
			flag = false;
		break;
	case PANEL_LCDC:
	case PANEL_MIPI2RGB:
	case PANEL_RGB2MIPI:
		hisi_fb_lcdc_rgb_resource_ready(bl_type, &flag);
		break;
	case PANEL_MIPI_VIDEO:
	case PANEL_MIPI_CMD:
	case PANEL_DUAL_MIPI_VIDEO:
	case PANEL_DUAL_MIPI_CMD:
		hisi_fb_mipi_resource_ready(bl_type, &flag);
		break;
	case PANEL_HDMI:
	case PANEL_DP:
		if (g_dts_resouce_ready & DTS_PANEL_PRIMARY_READY)
			flag = false;
		break;
	case PANEL_OFFLINECOMPOSER:
		if (g_dts_resouce_ready & DTS_PANEL_EXTERNAL_READY)
			flag = false;
		break;
	case PANEL_WRITEBACK:
		if (g_dts_resouce_ready & DTS_PANEL_OFFLINECOMPOSER_READY)
			flag = false;
		break;
	case PANEL_MEDIACOMMON:
		if (g_dts_resouce_ready & DTS_PANEL_OFFLINECOMPOSER_READY)
			flag = false;
		break;
	default:
		DPU_FB_ERR("not support this panel type: %d\n", panel_type);
		break;
	}

	up(&hisi_fb_dts_resource_sem);

	return flag;
}

void hisi_fb_device_set_status0(uint32_t status)
{
	down(&hisi_fb_dts_resource_sem);
	g_dts_resouce_ready |= status;
	up(&hisi_fb_dts_resource_sem);
}

int hisi_fb_device_set_status1(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	down(&hisi_fb_dts_resource_sem);

	switch (dpufd->panel_info.type) {
	case PANEL_LCDC:
	case PANEL_MIPI_VIDEO:
	case PANEL_MIPI_CMD:
	case PANEL_DUAL_MIPI_VIDEO:
	case PANEL_DUAL_MIPI_CMD:
	case PANEL_DP:
	case PANEL_MIPI2RGB:
	case PANEL_RGB2MIPI:
	case PANEL_HDMI:
		if (dpufd->index == PRIMARY_PANEL_IDX)
			g_dts_resouce_ready |= DTS_PANEL_PRIMARY_READY;
		else if (dpufd->index == EXTERNAL_PANEL_IDX)
			g_dts_resouce_ready |= DTS_PANEL_EXTERNAL_READY;
		else
			DPU_FB_ERR("not support fb%d\n", dpufd->index);
		break;
	case PANEL_OFFLINECOMPOSER:
		g_dts_resouce_ready |= DTS_PANEL_OFFLINECOMPOSER_READY;
		break;
	case PANEL_WRITEBACK:
		g_dts_resouce_ready |= DTS_PANEL_WRITEBACK_READY;
		break;
	case PANEL_MEDIACOMMON:
		g_dts_resouce_ready |= DTS_PANEL_MEDIACOMMON_READY;
		break;
	default:
		DPU_FB_ERR("not support this panel type: %d\n", dpufd->panel_info.type);
		ret = -1;
		break;
	}

	up(&hisi_fb_dts_resource_sem);

	return ret;
}

struct platform_device *hisi_fb_device_alloc(struct dpu_fb_panel_data *pdata,
	uint32_t type, uint32_t id)
{
	struct platform_device *this_dev = NULL;
	char dev_name[32] = {0};  /* device name buf len */

	if (!pdata) {
		DPU_FB_ERR("pdata is NULL");
		return NULL;
	}

	switch (type) {
	case PANEL_MIPI_VIDEO:
	case PANEL_MIPI_CMD:
	case PANEL_DUAL_MIPI_VIDEO:
	case PANEL_DUAL_MIPI_CMD:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_MIPIDSI);
		break;
	case PANEL_DP:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_DP);
		break;
	case PANEL_NO:
	case PANEL_LCDC:
	case PANEL_HDMI:
	case PANEL_OFFLINECOMPOSER:
	case PANEL_WRITEBACK:
	case PANEL_MEDIACOMMON:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_DSS_DPE);
		break;
	case PANEL_RGB2MIPI:
		snprintf(dev_name, sizeof(dev_name), DEV_NAME_RGB2MIPI);
		break;
	default:
		DPU_FB_ERR("invalid panel type = %d!\n", type);
		return NULL;
	}

	if (pdata != NULL)
		pdata->next = NULL;

	this_dev = platform_device_alloc(dev_name, (((uint32_t)type << 16) | (uint32_t)id));
	if (this_dev != NULL) {
		if (platform_device_add_data(this_dev, pdata, sizeof(struct dpu_fb_panel_data))) {
			DPU_FB_ERR("failed to platform_device_add_data!\n");
			platform_device_put(this_dev);
			this_dev = NULL;
			return this_dev;
		}
	}

	return this_dev;
}
/*lint +e574 +e647 +e568 +e685 +e578*/

int panel_next_tcon_mode(struct platform_device *pdev, struct dpu_fb_data_type *dpufd,
	struct dpu_panel_info *pinfo)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	dpu_check_and_return(!pdev, -EINVAL, ERR, "platform_device is NULL\n");
	pdata = dev_get_platdata(&pdev->dev);
	dpu_check_and_return(!pdata, -EINVAL, ERR, "dpu_fb_panel_data is NULL\n");
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return(!pinfo, -EINVAL, ERR, "pinfo is NULL\n");
	dpu_check_and_return(!pinfo->cascadeic_support, 0, DEBUG, "Nomal IC, do nothing\n");

	if (pinfo->current_display_region == EN_DISPLAY_REGION_NONE) {
		pinfo->current_display_region = EN_DISPLAY_REGION_AB;
		DPU_FB_INFO("first power on, set default region\n");
		return 0;
	} else if ((pinfo->current_display_region == EN_DISPLAY_REGION_A) ||
			 (pinfo->current_display_region == EN_DISPLAY_REGION_B) ||
			 (pinfo->current_display_region == EN_DISPLAY_REGION_AB) ||
			 (pinfo->current_display_region == EN_DISPLAY_REGION_AB_FOLDED)) {
		DPU_FB_INFO("change DDIC display region to %d\n", pinfo->current_display_region);
	} else {
		pinfo->current_display_region = EN_DISPLAY_REGION_AB;
		DPU_FB_ERR("wrong display region, should not occur %d\n", pinfo->current_display_region);
	}

	next_pdev = pdata->next;
	if (next_pdev) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->set_tcon_mode)) {
			DPU_FB_INFO("send DCS cmd to DDIC\n");
			ret = next_pdata->set_tcon_mode(next_pdev, pinfo->current_display_region);
		}
	}

	dpufb_panel_set_display_region_timestamp(dpufd);

	return ret;
}

static int set_display_region_by_mode(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	struct _panel_region_notify *region_notify)
{
#ifdef CONFIG_LCD_KIT_DRIVER
	uint8_t curr_mode;
	uint8_t pre_mode;

	curr_mode = region_notify->panel_display_region;
	pre_mode = pinfo->current_display_region;
	lcd_switch_region(dpufd, curr_mode, pre_mode);
#endif
	if ((region_notify->notify_mode == EN_MODE_PRE_NOTIFY) ||
		(region_notify->notify_mode == EN_MODE_REAL_SWITCH_NOTIFY)) {
		pinfo->current_display_region = region_notify->panel_display_region;
		DPU_FB_INFO("notify_mode:%d, change CurrentDisplayRegion to %d\n",
			region_notify->notify_mode, pinfo->current_display_region);
	}

	return panel_next_tcon_mode(dpufd->pdev, dpufd, pinfo);
}

/* just for Cyclomatic Complexity, no need to check input param */
static inline int _set_display_region(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	struct _panel_region_notify *region_notify)
{
	int ret = 0;

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		pinfo->current_display_region = region_notify->panel_display_region;
		if (region_notify->notify_mode == EN_MODE_POWER_OFF_SWITCH_NOTIFY)
			DPU_FB_INFO("receive display region change: %d when powe off, set when next power on\n",
				pinfo->current_display_region);
		else
			DPU_FB_WARNING("receive display region change: %d when powe off by %d notify_mode\n",
				pinfo->current_display_region, region_notify->notify_mode);

		up(&dpufd->blank_sem);
		return ret;
	}

	dpufb_activate_vsync(dpufd);

	if (region_notify->notify_mode == EN_MODE_POWER_OFF_SWITCH_NOTIFY) {
		DPU_FB_WARNING("panel already power on, set DDIC display region: %d right now, "
			"check the timing sequene\n",
			pinfo->current_display_region);
	    ret = set_display_region_by_mode(dpufd, pinfo, region_notify);

	} else if (region_notify->notify_mode == EN_MODE_PRE_NOTIFY) {
		/* set DDIC right now */
		if ((region_notify->panel_display_region == EN_DISPLAY_REGION_AB) ||
		   (region_notify->panel_display_region == EN_DISPLAY_REGION_AB_FOLDED)) {
			if (pinfo->current_display_region != region_notify->panel_display_region)
				ret = set_display_region_by_mode(dpufd, pinfo, region_notify);
			else
				DPU_FB_INFO("same DDIC display region: %d, no need pre change\n",
					pinfo->current_display_region);
		} else {
			DPU_FB_ERR("wrong pre notify which region is %d\n", region_notify->panel_display_region);
			ret = -EFAULT;
		}
	} else if (region_notify->notify_mode == EN_MODE_REAL_SWITCH_NOTIFY) {
		/* need check with TUI & AOD, select the biggest one */
		if (pinfo->current_display_region != region_notify->panel_display_region) {
			ret = set_display_region_by_mode(dpufd, pinfo, region_notify);
			dpufb_panel_add_fold_count(dpufd, pinfo->current_display_region);
		} else {
			DPU_FB_INFO("same DDIC display region: %d, no need real change\n",
				pinfo->current_display_region);
		}
	}

	dpufb_deactivate_vsync(dpufd);
	up(&dpufd->blank_sem);

	return ret;
}

int panel_set_display_region(struct dpu_fb_data_type *dpufd, const void __user *argp)
{
	int ret;
	struct _panel_region_notify region_notify;
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return(!argp, -EINVAL, ERR, "argp is NULL\n");

	pinfo = &(dpufd->panel_info);

	ret = (int)copy_from_user(&region_notify, argp, sizeof(struct _panel_region_notify));
	if (ret) {
		DPU_FB_ERR("copy_from_user(param) failed! ret=%d\n", ret);
		return -EFAULT;
	}

	/* set partial update max area */
	ret = _set_display_region(dpufd, pinfo, &region_notify);

	return ret;
}

static int get_innel_panel_fold_mode(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int target_fold_status)
{
	int fold_mode = 0;

	if (!dpufd || !pinfo) {
		DPU_FB_ERR("[folder]dpufd or pinfo is null point\n");
		return -EINVAL;
	}

	if (dpufd->panel_power_status == EN_INNER_OUTER_PANEL_ON) {
		if (target_fold_status == EN_DISPLAY_REGION_UNFOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE0;
		if (target_fold_status == EN_DISPLAY_REGION_FOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE1;
		if (target_fold_status == EN_DISPLAY_REGION_FOLD_UNFOLD_SELFREFRESH)
			fold_mode = EN_DISPLAY_FOLD_MODE10;
	}

	if (dpufd->panel_power_status == EN_INNER_PANEL_ON) {
		if (target_fold_status == EN_DISPLAY_REGION_FOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE2;
		if (target_fold_status == EN_DISPLAY_REGION_FOLD_UNFOLD_SELFREFRESH)
			fold_mode = EN_DISPLAY_FOLD_MODE9;
	}

	if (dpufd->panel_power_status == EN_INNER_OUTER_PANEL_OFF) {
		if (target_fold_status == EN_DISPLAY_REGION_FOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE3;
	}

	return fold_mode;
}

static int get_outer_panel_fold_mode(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int target_fold_status)
{
	int fold_mode = 0;

	if (!dpufd || !pinfo) {
		DPU_FB_ERR("[folder]dpufd or pinfo is null point\n");
		return -EINVAL;
	}

	if (dpufd->panel_power_status == EN_INNER_OUTER_PANEL_ON) {
		if (target_fold_status == EN_DISPLAY_REGION_FOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE4;
		if (target_fold_status == EN_DISPLAY_REGION_UNFOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE5;
		if (target_fold_status == EN_DISPLAY_REGION_UNFOLD_FOLD_SELFREFRESH)
			fold_mode = EN_DISPLAY_FOLD_MODE11;
	}

	if (dpufd->panel_power_status == EN_OUTER_PANEL_ON) {
		if (target_fold_status == EN_DISPLAY_REGION_UNFOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE6;
		if (target_fold_status == EN_DISPLAY_REGION_UNFOLD_FOLD_SELFREFRESH)
			fold_mode = EN_DISPLAY_FOLD_MODE8;
	}

	if (dpufd->panel_power_status == EN_INNER_OUTER_PANEL_OFF) {
		if (target_fold_status == EN_DISPLAY_REGION_UNFOLD)
			fold_mode = EN_DISPLAY_FOLD_MODE7;
	}

	return fold_mode;
}

static int get_panel_fold_mode(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int target_fold_status)
{
	int display_fold_mode = 0;

	if (!dpufd || !pinfo) {
		DPU_FB_ERR("[folder]dpufd or pinfo is null point\n");
		return -EINVAL;
	}

	if (pinfo->disp_panel_id == DISPLAY_INNEL_PANEL_ID)
		display_fold_mode = get_innel_panel_fold_mode(dpufd, pinfo, target_fold_status);

	if (pinfo->disp_panel_id == DISPLAY_OUTER_PANEL_ID)
		display_fold_mode = get_outer_panel_fold_mode(dpufd, pinfo, target_fold_status);

	DPU_FB_INFO("[fold] disp_panel_id = %d[0:inner;1:outer], panel_power_status = %d, display_fold_mode = 0x%x\n",
		pinfo->disp_panel_id, dpufd->panel_power_status, display_fold_mode);
	return display_fold_mode;
}

void dpufd_get_panel_info(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	uint32_t fold_disp_panel_id)
{
	int ret;
	struct dpu_fb_panel_data *pdata = NULL;
	struct platform_device *next_pdev = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *pdev = NULL;

	pdev = dpufd->pdev;
	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		DPU_FB_ERR("[folder] dev_get_platdata is err\n");
		return;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) && (next_pdata->panel_switch)) {
			ret = next_pdata->panel_switch(next_pdev, fold_disp_panel_id);
			if (fold_disp_panel_id == DISPLAY_OUTER_PANEL_ID)
				DPU_FB_INFO("[folder] config panel to outer panel, ret = %d\n", ret);

			if (fold_disp_panel_id == DISPLAY_INNEL_PANEL_ID)
				DPU_FB_INFO("[folder] config panel to inner panel, ret = %d\n", ret);

			if ((ret == 0) && (next_pdata->get_panel_info)) {
				pinfo = next_pdata->get_panel_info(next_pdev, fold_disp_panel_id);
				memcpy(&dpufd->panel_info, pinfo, sizeof(struct dpu_panel_info));
				DPU_FB_INFO("[folder] get_panel_info xres = %d, yres = %d\n",
					dpufd->panel_info.xres, dpufd->panel_info.yres);
			}
		}
	}
}

/*
 * 1) panel connect to inner panel in fastboot:
 *  i) mipi enter ULPS, dss power off, mipi sw to outer panel;
 *  ii) dss power on, get outer panel info, mipi set power off cmd to outer panel,
 *    outer panel power off, dss power off;
 *  iii) mipi sw to inner panel, dss power on, inner panel normal display.
 * 2) panel connect to outer panel in fastboot:
 *  i) mipi enter ULPS, dss power off, mipi sw to inner panel;
 *  ii) dss power on, get inner panel info, mipi set power off cmd to inner panel,
 *    inner panel power off, dss power off;
 *  iii) mipi sw to outer panel, dss power on, outer panel normal display.
 */
static int dpufb_set_panel_blank(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int fold_disp_panel_id, int other_disp_panel_id)
{
	int ret;

	dpufd->panel_info.skip_power_on_off = SKIP_POWER_ON_OFF;
	/* dss off */
	ret = hisi_fb_blank_sub(FB_BLANK_POWERDOWN, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder]dss power off fail, ret = %d\n", ret);
		return ret;
	}

	dpufd_get_panel_info(dpufd, pinfo, other_disp_panel_id);

	dpufd->panel_info.skip_power_on_off = SKIP_POWER_ON_OFF;
	/* dss on */
	ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder]dss power on fail, ret = %d\n", ret);
		return ret;
	}

	dpufd->panel_info.skip_power_on_off = FOLD_POWER_ON_OFF;
	/* dss off */
	ret = hisi_fb_blank_sub(FB_BLANK_POWERDOWN, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder]dss power off fail, ret = %d\n", ret);
		return ret;
	}

	dpufd_get_panel_info(dpufd, pinfo, fold_disp_panel_id);

	dpufd->panel_info.skip_power_on_off = SKIP_POWER_ON_OFF;
	/* dss on */
	ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder]dss power on fail, ret = %d\n", ret);
		return ret;
	}

	return 0;
}


static int change_panel_fold_status(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int display_panel_status)
{
	int ret;
	int vsync_enabled;
	int fold_disp_panel_id = DISPLAY_INNEL_PANEL_ID;
	int other_disp_panel_id = DISPLAY_OUTER_PANEL_ID;

	pinfo = &(dpufd->panel_info);

	down(&dpufd->blank_sem);
	if (dpufd->panel_power_on == false) {
		up(&dpufd->blank_sem);
		DPU_FB_INFO("[folder] panel is power off, not need set display panel status\n");
		return 0;
	}
	up(&dpufd->blank_sem);

	if (display_panel_status == EN_DISPLAY_REGION_UNFOLD) {
		fold_disp_panel_id = DISPLAY_INNEL_PANEL_ID;
		other_disp_panel_id = DISPLAY_OUTER_PANEL_ID;
	}

	if (display_panel_status == EN_DISPLAY_REGION_FOLD) {
		fold_disp_panel_id = DISPLAY_OUTER_PANEL_ID;
		other_disp_panel_id = DISPLAY_INNEL_PANEL_ID;
	}

	vsync_enabled = dpufd->vsync_ctrl.vsync_enabled;

	ret = dpufb_set_panel_blank(dpufd, pinfo, fold_disp_panel_id, other_disp_panel_id);
	if (ret != 0) {
		DPU_FB_ERR("[folder]dpufb set panel blank fail, ret = %d\n", ret);
		return ret;
	}

	if (display_panel_status == EN_DISPLAY_REGION_UNFOLD)
		dpufd->panel_power_status = EN_INNER_PANEL_ON;

	if (display_panel_status == EN_DISPLAY_REGION_FOLD)
		dpufd->panel_power_status = EN_OUTER_PANEL_ON;

	dpufd->panel_info.skip_power_on_off = NORMAL_POWER_ON_OFF;
	dpufd->vsync_ctrl.vsync_enabled = vsync_enabled;

	return ret;
}

/*
 * 1) panel connect to inner panel in kernel:
 *  i) inner panel power off, dss power off;
 *  ii) mipi sw to outer panel, get outer panel info, dss power on, outer panel power on & normal display.
 * 2) panel connect to outer panel in kernel:
 *  i) outer panel power off, dss power off;
 *  ii) mipi switch to inner panel, get inner panel info, dss power on, inner panel power on & normal display.
 */
static int set_panel_fold_status(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int display_panel_status, int target_fold_mode)
{
	int ret;
	int vsync_enabled;
	uint32_t fold_disp_panel_id = DISPLAY_INNEL_PANEL_ID;

	pinfo = &(dpufd->panel_info);

	down(&dpufd->blank_sem);
	if (dpufd->panel_power_on == false) {
		up(&dpufd->blank_sem);
		DPU_FB_INFO("[folder] panel is power off, not need set display panel status\n");
		return 0;
	}
	up(&dpufd->blank_sem);

	if (display_panel_status == EN_DISPLAY_REGION_UNFOLD)
		fold_disp_panel_id = DISPLAY_INNEL_PANEL_ID;

	if (display_panel_status == EN_DISPLAY_REGION_FOLD)
		fold_disp_panel_id = DISPLAY_OUTER_PANEL_ID;

	vsync_enabled = dpufd->vsync_ctrl.vsync_enabled;

	dpufd->panel_info.skip_power_on_off = FOLD_POWER_ON_OFF;
	/* dss off */
	ret = hisi_fb_blank_sub(FB_BLANK_POWERDOWN, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder] dss power off fail, ret = %d\n", ret);
		return ret;
	}

	dpufd_get_panel_info(dpufd, pinfo, fold_disp_panel_id);

	if ((target_fold_mode == EN_DISPLAY_FOLD_MODE1) ||
		(target_fold_mode == EN_DISPLAY_FOLD_MODE5))
		dpufd->panel_info.skip_power_on_off = SKIP_POWER_ON_OFF;
	else
		dpufd->panel_info.skip_power_on_off = FOLD_POWER_ON_OFF_FOR_EFFECT;

	/* dss on */
	ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder] dss power on fail, ret = %d\n", ret);
		return ret;
	}

	if (display_panel_status == EN_DISPLAY_REGION_UNFOLD)
		dpufd->panel_power_status = EN_INNER_PANEL_ON;

	if (display_panel_status == EN_DISPLAY_REGION_FOLD)
		dpufd->panel_power_status = EN_OUTER_PANEL_ON;

	dpufd->panel_info.skip_power_on_off = NORMAL_POWER_ON_OFF;
	dpufd->vsync_ctrl.vsync_enabled = vsync_enabled;

	return ret;
}

static void set_inner_panel_power_on(struct dpu_fb_data_type *dpufd,
	struct dpu_panel_info *pinfo)
{
	int ret;

#if defined(CONFIG_HISI_FB_AOD)
	/* get aod lock after aod stop */
	hisi_fb_aod_blank(dpufd, FB_BLANK_UNBLANK);
#endif
	pinfo = &(dpufd->panel_info);
	dpufd_get_panel_info(dpufd, pinfo, DISPLAY_INNEL_PANEL_ID);

	dpufd->panel_info.skip_power_on_off = NORMAL_POWER_ON_OFF;
	/* dss on */
	ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder] dss inner panel power on fail, ret = %d\n", ret);
		return;
	}

	dpufd->panel_power_status = EN_INNER_PANEL_ON;
}

static void set_fbi_var_info(struct dpu_fb_data_type *dpufd)
{
	struct fb_info *fbi = NULL;
	struct fb_var_screeninfo *var = NULL;

	fbi = dpufd->fbi;
	var = &fbi->var;

	var->xres = dpufd->panel_info.xres - dpufd->panel_info.dummy_pixel_num;
	var->yres = dpufd->panel_info.yres;
	var->xres_virtual = dpufd->panel_info.xres;
	var->yres_virtual = dpufd->panel_info.yres * dpufd->fb_num;

	var->pixclock = dpufd->panel_info.pxl_clk_rate;
	var->left_margin = dpufd->panel_info.ldi.h_back_porch;
	var->right_margin = dpufd->panel_info.ldi.h_front_porch;
	var->upper_margin = dpufd->panel_info.ldi.v_back_porch;
	var->lower_margin = dpufd->panel_info.ldi.v_front_porch;
	var->hsync_len = dpufd->panel_info.ldi.h_pulse_width;
	var->vsync_len = dpufd->panel_info.ldi.v_pulse_width;
	var->width = dpufd->panel_info.width;
	var->height = dpufd->panel_info.height;

	DPU_FB_INFO("fb%d, var->xres = %d, var->yres = %d\n", dpufd->index, var->xres, var->yres);
}

static int set_two_panel_display(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int display_panel_status)
{
	int ret;
	int vsync_enabled;
	uint32_t panel_power_status;
	uint32_t fold_disp_panel_id = DISPLAY_INNEL_PANEL_ID;
	uint32_t other_disp_panel_id = DISPLAY_OUTER_PANEL_ID;

	if (!dpufd || !pinfo) {
		DPU_FB_ERR("[folder] dpufd or pinfo is null point\n");
		return -EINVAL;
	}

	pinfo = &(dpufd->panel_info);

	down(&dpufd->blank_sem);
	if (dpufd->panel_power_on == false) {
		up(&dpufd->blank_sem);
		DPU_FB_INFO("[folder] panel is power off, not need set two panel display\n");
		return 0;
	}
	up(&dpufd->blank_sem);

	if (display_panel_status == EN_DISPLAY_REGION_UNFOLD_FOLD_SELFREFRESH) {
		fold_disp_panel_id = DISPLAY_INNEL_PANEL_ID;
		other_disp_panel_id = DISPLAY_OUTER_PANEL_ID;
	} else if (display_panel_status == EN_DISPLAY_REGION_FOLD_UNFOLD_SELFREFRESH) {
		fold_disp_panel_id = DISPLAY_OUTER_PANEL_ID;
		other_disp_panel_id = DISPLAY_INNEL_PANEL_ID;
	} else {
		DPU_FB_ERR("[folder] display_panel_status = 0x%x is err\n", display_panel_status);
		return -EINVAL;
	}

	vsync_enabled = dpufd->vsync_ctrl.vsync_enabled;

	dpufd->panel_info.skip_power_on_off = SKIP_POWER_ON_OFF;
	panel_power_status = dpufd->panel_power_status;
	/* dss off */
	ret = hisi_fb_blank_sub(FB_BLANK_POWERDOWN, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder]dss power off fail, ret = %d\n", ret);
		return ret;
	}

	dpufd_get_panel_info(dpufd, pinfo, fold_disp_panel_id);
	DPU_FB_INFO("panel_power_status = %d\n", panel_power_status);
	if (panel_power_status == EN_INNER_OUTER_PANEL_ON)
		dpufd->panel_info.skip_power_on_off = SKIP_POWER_ON_OFF;
	else
		dpufd->panel_info.skip_power_on_off = NORMAL_POWER_ON_OFF;

	/* dss on */
	ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, dpufd->fbi);
	if (ret != 0) {
		DPU_FB_ERR("[folder]dss power on fail, ret = %d\n", ret);
		return ret;
	}

	dpufd->panel_power_status = EN_INNER_OUTER_PANEL_ON;
	dpufd->panel_info.skip_power_on_off = NORMAL_POWER_ON_OFF;
	dpufd->vsync_ctrl.vsync_enabled = vsync_enabled;

	return ret;
}

static void wait_for_tui_end(struct dpu_fb_data_type *dpufd)
{
	int try_times = 0;
	const int timeout_count = 2000;

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return;
	if ((dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) == 0)
		return;

	DPU_FB_INFO("fb%d +\n", dpufd->index);

#if defined(CONFIG_TEE_TUI)
	tui_poweroff_work_start();
#endif

	while (dpufd->secure_ctrl.secure_status != DSS_SEC_IDLE) {
		mdelay(1);
		if (++try_times > timeout_count) {
			DPU_FB_ERR("[fold] wait for send tui stop timeout\n");
			break;
		}
	}
}

static int config_panel_display_status(struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
	int target_display_panel_status)
{
	int target_fold_mode;
	int display_panel_status;
	uint32_t fold_disp_panel_id;

#if defined(CONFIG_HISI_FB_AOD)
	wait_for_aod_end(dpufd);
#endif
	wait_for_tui_end(dpufd);

	dpufd->fold_panel_display_change = true;
	target_fold_mode = get_panel_fold_mode(dpufd, pinfo, target_display_panel_status);

	switch (target_fold_mode) {
	case EN_DISPLAY_FOLD_MODE5:
	case EN_DISPLAY_FOLD_MODE6:
		display_panel_status = EN_DISPLAY_REGION_UNFOLD;
		set_panel_fold_status(dpufd, pinfo, display_panel_status, target_fold_mode);
		break;
	case EN_DISPLAY_FOLD_MODE1:
	case EN_DISPLAY_FOLD_MODE2:
		display_panel_status = EN_DISPLAY_REGION_FOLD;
		set_panel_fold_status(dpufd, pinfo, display_panel_status, target_fold_mode);
		break;
	case EN_DISPLAY_FOLD_MODE0:
		display_panel_status = EN_DISPLAY_REGION_UNFOLD;
		change_panel_fold_status(dpufd, pinfo, display_panel_status);
		break;
	case EN_DISPLAY_FOLD_MODE4:
		display_panel_status = EN_DISPLAY_REGION_FOLD;
		change_panel_fold_status(dpufd, pinfo, display_panel_status);
		break;
	case EN_DISPLAY_FOLD_MODE3:
		display_panel_status = EN_DISPLAY_REGION_FOLD;
		fold_disp_panel_id = DISPLAY_OUTER_PANEL_ID;
		dpufd_get_panel_info(dpufd, pinfo, fold_disp_panel_id);
		break;
	case EN_DISPLAY_FOLD_MODE7:
		display_panel_status = EN_DISPLAY_REGION_UNFOLD;
		set_inner_panel_power_on(dpufd, pinfo);
		break;
	case EN_DISPLAY_FOLD_MODE8:
	case EN_DISPLAY_FOLD_MODE11:
		display_panel_status = EN_DISPLAY_REGION_UNFOLD_FOLD_SELFREFRESH;
		set_two_panel_display(dpufd, pinfo, display_panel_status);
		break;
	case EN_DISPLAY_FOLD_MODE9:
	case EN_DISPLAY_FOLD_MODE10:
		display_panel_status = EN_DISPLAY_REGION_FOLD_UNFOLD_SELFREFRESH;
		set_two_panel_display(dpufd, pinfo, display_panel_status);
		break;
	default:
		DPU_FB_INFO("[fold] target_fold_mode = 0x%x is err\n", target_fold_mode);
		set_fbi_var_info(dpufd);
		dpufd->panel_info.esd_enable = 1;
		dpufd->fold_panel_display_change = false;
		return 0;
	}

	set_fbi_var_info(dpufd);
	dpufd->fold_panel_display_change = false;
	DPU_FB_INFO("[fold] target_fold_mode = 0x%x, display_panel_status = 0x%x\n",
		target_fold_mode, display_panel_status);

	return 0;
}

int dpufb_config_panel_esd_status(struct dpu_fb_data_type *dpufd, const void __user *argp)
{
	int ret;
	int enable = 0;

	dpu_check_and_return((!argp), -EINVAL, ERR, "argp NULL Pointer!\n");
	dpu_check_and_return((dpufd->index != PRIMARY_PANEL_IDX), -EINVAL, ERR, "no support\n");

	ret = copy_from_user(&enable, argp, sizeof(enable));
	dpu_check_and_return(ret, ret, ERR, "config panel esd status failed!\n");

	down(&dpufd->brightness_esd_sem);

	enable = (enable) ? 1 : 0;
	dpufd->panel_info.esd_enable = (uint8_t)enable;

	up(&dpufd->brightness_esd_sem);
	return 0;
}

void get_lcd_panel_info(struct dpu_fb_data_type *dpufd, struct dpu_panel_info **pinfo,
	uint32_t panel_id)
{
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");

	pdata = dev_get_platdata(&(dpufd->pdev)->dev);
	dpu_check_and_no_retval(!pdata, ERR, "pdata is NULL!\n");

	if (pdata->next) {
		next_pdata = dev_get_platdata(&(pdata->next)->dev);
		if (next_pdata && (next_pdata->get_panel_info))
			*pinfo = next_pdata->get_panel_info(pdata->next, panel_id);
	}
}

static void set_panel_info(uint32_t panel_id, struct platform_product_info *product_info, struct dpu_panel_info *pinfo_temp)
{
	product_info->panel_id[panel_id] = pinfo_temp->disp_panel_id;
	product_info->xres[panel_id] = pinfo_temp->xres;
	product_info->yres[panel_id] = pinfo_temp->yres;
	product_info->width[panel_id] = pinfo_temp->width;
	product_info->height[panel_id] = pinfo_temp->height;
}

int dpufb_get_lcd_panel_info(struct dpu_fb_data_type *dpufd, struct platform_product_info *product_info)
{
	struct dpu_panel_info *pinfo_temp = NULL;
	dpu_check_and_return((!product_info), -EINVAL, ERR, "product_info NULL Pointer!\n");
	dpu_check_and_return((dpufd->index != PRIMARY_PANEL_IDX), -EINVAL, ERR, "no support\n");

	get_lcd_panel_info(dpufd, &pinfo_temp, DISPLAY_INNEL_PANEL_ID);
	set_panel_info(DISPLAY_INNEL_PANEL_ID, product_info, pinfo_temp);
	get_lcd_panel_info(dpufd, &pinfo_temp, DISPLAY_OUTER_PANEL_ID);
	set_panel_info(DISPLAY_OUTER_PANEL_ID, product_info, pinfo_temp);

	return 0;
}

int set_display_panel_status(struct dpu_fb_data_type *dpufd, const void __user *argp)
{
	int ret;
	int target_display_panel_status = 0;
	struct dpu_panel_info *pinfo = NULL;
	static uint8_t last_display_panel_status;

	if (!dpufd || !argp) {
		DPU_FB_ERR("[folder]dpufd or argp is NULL\n");
		return -EINVAL;
	}

	pinfo = &(dpufd->panel_info);

	if ((pinfo->product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) == 0) {
		DPU_FB_WARNING("[folder]product type = %d is not support two panel display\n", pinfo->product_type);
		return -EINVAL;
	}

	ret = (int)copy_from_user(&target_display_panel_status, argp, sizeof(target_display_panel_status));
	if (ret) {
		DPU_FB_ERR("[folder]copy_from_user(display panel status) failed, ret = %d\n", ret);
		return -EFAULT;
	}

	if ((target_display_panel_status != EN_DISPLAY_REGION_UNFOLD) &&
		(target_display_panel_status != EN_DISPLAY_REGION_FOLD) &&
		(target_display_panel_status != EN_DISPLAY_REGION_UNFOLD_FOLD_SELFREFRESH) &&
		(target_display_panel_status != EN_DISPLAY_REGION_FOLD_UNFOLD_SELFREFRESH)) {
		DPU_FB_ERR("[folder]display_panel_status=%d is err (0x8:inner screen, 0x10:outer screen,"
			"0x20: inner display &outer self-refresh, 0x40:outer display &inner self-refresh)\n",
			target_display_panel_status);
		return -EFAULT;
	}

	if (last_display_panel_status == target_display_panel_status) {
		DPU_FB_INFO("[folder]same display panel status[%d], not need set again\n",
			target_display_panel_status);
		return 0;
	}

	DPU_FB_INFO("[folder]target_display_panel_status = 0x%x (0x8:inner screen, 0x10:outer screen,"
		"0x20: inner display &outer self-refresh, 0x40:outer display &inner self-refresh)\n",
		target_display_panel_status);

	down(&dpufd->esd_panel_change_sem);
	ret = config_panel_display_status(dpufd, pinfo, target_display_panel_status);
	if (ret == 0)
		last_display_panel_status = target_display_panel_status;
	up(&dpufd->esd_panel_change_sem);

	return ret;
}

/* for display time */
static void panel_hiace_start_timestamp(struct dpu_fb_data_type *dpufd, s64 msecs)
{
	struct dpu_panel_info *pinfo = NULL;
	s64 *start_time = NULL;
	s64 *duration_time = NULL;
	uint32_t *time_state = NULL;
	uint8_t i;

	pinfo = &(dpufd->panel_info);
	start_time = &(dpufd->aging_time_info.start_time[0]);
	duration_time = &(dpufd->aging_time_info.duration_time[0]);
	time_state = &(dpufd->aging_time_info.time_state[0]);

	DPU_FB_INFO("+\n");

	if (pinfo->current_display_region == EN_DISPLAY_REGION_AB) {
		for (i = 0; i < EN_AGING_PANEL_NUM; i++) {
			start_time[i] = msecs;
			time_state[i] = EN_TIME_STATE_WORK;
			DPU_FB_INFO("set start_time[%d]=%lld\n", i, msecs);
		}
	} else if (pinfo->current_display_region == EN_DISPLAY_REGION_AB_FOLDED) {
		for (i = 0; i <= EN_AGING_PANEL_SUB; i++) {
			start_time[i] = msecs;
			time_state[i] = EN_TIME_STATE_WORK;
			DPU_FB_INFO("set start_time[%d]=%lld\n", i, msecs);
		}
	} else if (pinfo->current_display_region == EN_DISPLAY_REGION_A) {
		start_time[EN_AGING_PANEL_MAIN] = msecs;
		time_state[EN_AGING_PANEL_MAIN] = EN_TIME_STATE_WORK;
		DPU_FB_INFO("set start_time[0]=%lld\n", msecs);
	} else if (pinfo->current_display_region == EN_DISPLAY_REGION_B) {
		start_time[EN_AGING_PANEL_SUB] = msecs;
		time_state[EN_AGING_PANEL_SUB] = EN_TIME_STATE_WORK;
		DPU_FB_INFO("set start_time[1]=%lld\n", msecs);
	}
}

static void panel_hiace_stop_timestamp(struct dpu_fb_data_type *dpufd, s64 msecs)
{
	struct dpu_panel_info *pinfo = NULL;
	uint8_t i;
	s64 *start_time = NULL;
	s64 *duration_time = NULL;
	uint32_t *time_state = NULL;

	pinfo = &(dpufd->panel_info);
	start_time = &(dpufd->aging_time_info.start_time[0]);
	duration_time = &(dpufd->aging_time_info.duration_time[0]);
	time_state = &(dpufd->aging_time_info.time_state[0]);

	DPU_FB_INFO("+\n");

	for (i = 0; i < EN_AGING_PANEL_NUM; i++) {
		if (time_state[i] == EN_TIME_STATE_WORK) {
			duration_time[i] += msecs - start_time[i];
			time_state[i] = EN_TIME_STATE_IDLE;
			DPU_FB_INFO("set duration_time[%d]=%lld\n", i, duration_time[i]);
		}
	}
}

static void panel_display_region_start_timestamp(struct dpu_fb_data_type *dpufd, uint8_t index, s64 msecs)
{
	s64 *start_time = NULL;
	uint32_t *time_state = NULL;

	start_time = &(dpufd->aging_time_info.start_time[0]);
	time_state = &(dpufd->aging_time_info.time_state[0]);

	DPU_FB_INFO("+\n");

	if (!dpufd->aging_time_info.hiace_enable) {
		/* hiace off, no need start timer */
		DPU_FB_INFO("hiace off, no need start timer\n");
		return;
	}

	if (time_state[index] == EN_TIME_STATE_IDLE) {
		start_time[index] = msecs;
		time_state[index] = EN_TIME_STATE_WORK;
		DPU_FB_INFO("set start_time[%d]=%lld\n", index, msecs);
	}
}

static void panel_display_region_stop_timestamp(struct dpu_fb_data_type *dpufd,
	uint8_t index, s64 msecs)
{
	s64 *start_time = NULL;
	s64 *duration_time = NULL;
	uint32_t *time_state = NULL;

	start_time = &(dpufd->aging_time_info.start_time[0]);
	duration_time = &(dpufd->aging_time_info.duration_time[0]);
	time_state = &(dpufd->aging_time_info.time_state[0]);

	DPU_FB_INFO("+\n");

	if (time_state[index] == EN_TIME_STATE_WORK) {
		duration_time[index] += msecs - start_time[index];
		time_state[index] = EN_TIME_STATE_IDLE;
		DPU_FB_INFO("set duration_time[%d]=%lld\n", index, duration_time[index]);
	}
}

void dpufb_panel_display_time_init(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	DPU_FB_INFO("+\n");

	memset(&dpufd->aging_time_info, 0, sizeof(dpufd->aging_time_info));
	spin_lock_init(&dpufd->aging_time_info.time_lock);
	spin_lock_init(&dpufd->aging_time_info.count_lock);
}

void dpufb_panel_set_hiace_timestamp(struct dpu_fb_data_type *dpufd, bool enable, int mode)
{
	struct dpu_panel_info *pinfo = NULL;
	ktime_t timestamp;
	s64 msecs;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	/* normal panel, no need do anything */
	pinfo = &(dpufd->panel_info);
	if (!pinfo->cascadeic_support && !pinfo->single_deg_support)
		return;

	if (mode == CE_MODE_SINGLE) {
		/* not consider single mode */
		DPU_FB_DEBUG("not consider single mode\n");
		return;
	}

	if (enable && mode == CE_MODE_DISABLE) {
		DPU_FB_ERR("unexpected conditon, pls check\n");
		return;
	}

	DPU_FB_DEBUG("[%d/%d]+\n", enable, mode);

	spin_lock(&dpufd->aging_time_info.time_lock);

	if (dpufd->aging_time_info.hiace_enable == enable) {
		/* no change, do nothing */
		spin_unlock(&dpufd->aging_time_info.time_lock);
		DPU_FB_DEBUG("no change, do nothing\n");
		return;
	}

	timestamp = ktime_get();
	msecs = ktime_to_ms(timestamp);

	dpufd->aging_time_info.hiace_enable = enable;

	if (enable)
		panel_hiace_start_timestamp(dpufd, msecs);
	else
		panel_hiace_stop_timestamp(dpufd, msecs);

	spin_unlock(&dpufd->aging_time_info.time_lock);
}

void dpufb_panel_set_display_region_timestamp(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	ktime_t timestamp;
	s64 msecs;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	pinfo = &(dpufd->panel_info);

	DPU_FB_INFO("+\n");

	spin_lock(&dpufd->aging_time_info.time_lock);

	timestamp = ktime_get();
	msecs = ktime_to_ms(timestamp);

	/* start idle region, nothing to work region */
	if (pinfo->current_display_region == EN_DISPLAY_REGION_AB) {
		panel_display_region_start_timestamp(dpufd, EN_AGING_PANEL_MAIN, msecs);
		panel_display_region_start_timestamp(dpufd, EN_AGING_PANEL_SUB, msecs);
		panel_display_region_start_timestamp(dpufd, EN_AGING_PANEL_SIDE, msecs);
	} else if (pinfo->current_display_region == EN_DISPLAY_REGION_AB_FOLDED) {
		panel_display_region_start_timestamp(dpufd, EN_AGING_PANEL_MAIN, msecs);
		panel_display_region_start_timestamp(dpufd, EN_AGING_PANEL_SUB, msecs);
		panel_display_region_stop_timestamp(dpufd, EN_AGING_PANEL_SIDE, msecs);
	} else if (pinfo->current_display_region == EN_DISPLAY_REGION_A) {
		panel_display_region_start_timestamp(dpufd, EN_AGING_PANEL_MAIN, msecs);
		panel_display_region_stop_timestamp(dpufd, EN_AGING_PANEL_SUB, msecs);
		panel_display_region_stop_timestamp(dpufd, EN_AGING_PANEL_SIDE, msecs);
	} else if (pinfo->current_display_region == EN_DISPLAY_REGION_B) {
		panel_display_region_stop_timestamp(dpufd, EN_AGING_PANEL_MAIN, msecs);
		panel_display_region_start_timestamp(dpufd, EN_AGING_PANEL_SUB, msecs);
		panel_display_region_stop_timestamp(dpufd, EN_AGING_PANEL_SIDE, msecs);
	}

	spin_unlock(&dpufd->aging_time_info.time_lock);
}

void dpufb_panel_get_hiace_display_time(struct dpu_fb_data_type *dpufd, uint32_t *time)
{
	ktime_t timestamp;
	s64 msecs;
	uint8_t i;
	s64 *start_time = NULL;
	s64 *duration_time = NULL;
	uint32_t *time_state = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dpu_check_and_no_retval(!time, ERR, "time ptr is NULL\n");

	DPU_FB_INFO("+\n");

	start_time = &(dpufd->aging_time_info.start_time[0]);
	duration_time = &(dpufd->aging_time_info.duration_time[0]);
	time_state = &(dpufd->aging_time_info.time_state[0]);

	spin_lock(&dpufd->aging_time_info.time_lock);

	timestamp = ktime_get();
	msecs = ktime_to_ms(timestamp);

	for (i = 0; i < EN_AGING_PANEL_NUM; i++) {
		/* 1. restart all timer if needed */
		if (time_state[i] == EN_TIME_STATE_WORK) {
			duration_time[i] += msecs - start_time[i];
			start_time[i] = msecs;
		}

		/* 2. return the duration time */
		*(time + i) = (uint32_t) duration_time[i];

		DPU_FB_INFO("return time=%d from duration_time[%d]=%lld\n", *(time + i), i, duration_time[i]);

		/* 3. reset the duration time */
		duration_time[i] = 0;
	}

	spin_unlock(&dpufd->aging_time_info.time_lock);
}

void dpufb_panel_add_fold_count(struct dpu_fb_data_type *dpufd, uint8_t region)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	spin_lock(&dpufd->aging_time_info.count_lock);

	if (dpufd->aging_time_info.record_region == region) {
		spin_unlock(&dpufd->aging_time_info.count_lock);
		return;
	}

	if (dpufd->aging_time_info.record_region == EN_DISPLAY_REGION_NONE) {  /* init status */
		dpufd->aging_time_info.record_region = region;
	} else if (dpufd->aging_time_info.record_region == EN_DISPLAY_REGION_AB) {  /* full -> half */
		dpufd->aging_time_info.record_region = region;
		dpufd->aging_time_info.fold_count++;
	} else {  /* EN_DISPLAY_REGION_A, EN_DISPLAY_REGION_B, or EN_DISPLAY_REGION_AB_FOLDED */
		if (region == EN_DISPLAY_REGION_AB) {  // half -> full
			dpufd->aging_time_info.record_region = region;
			dpufd->aging_time_info.fold_count++;
		} else {  /* no fold status change, record only */
			dpufd->aging_time_info.record_region = region;
		}
	}

	spin_unlock(&dpufd->aging_time_info.count_lock);
}

uint32_t dpufb_panel_get_fold_count(struct dpu_fb_data_type *dpufd)
{
	uint32_t count;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return 0;
	}

	spin_lock(&dpufd->aging_time_info.count_lock);

	count = dpufd->aging_time_info.fold_count;
	dpufd->aging_time_info.fold_count = 0;

	spin_unlock(&dpufd->aging_time_info.count_lock);
	return count;
}
