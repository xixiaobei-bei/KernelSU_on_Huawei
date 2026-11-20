
/*
 * lcd_kit_sysfs_hs.c
 *
 * lcdkit sys node function for lcd driver
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

#include "lcd_kit_sysfs_hs.h"
#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_bl.h"
#include <linux/hisi/hw_cmdline_parse.h>
#include "lcd_kit_power.h"
#include "lcd_kit_disp.h"
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif
#include "lcd_kit_ext_disp.h"

/* oem info */
static int oem_info_type = LCD_KIT_FAIL;
static unsigned int fold_panel_id;
static int lcd_get_2d_barcode(char *oem_data,
	struct hisi_fb_data_type *hisifd);
static int lcd_get_project_id(char *oem_data,
	struct hisi_fb_data_type *hisifd);
static int lcd_get_brightness_colorpoint(char *oem_data,
	struct hisi_fb_data_type *hisifd);
static int lcd_set_brightness_xcc_rgbw(char *oem_data,
	struct hisi_fb_data_type *hisifd);

static struct oem_info_cmd oem_read_cmds[] = {
	{ PROJECT_ID_TYPE, lcd_get_project_id },
	{ BARCODE_2D_TYPE, lcd_get_2d_barcode },
	{ BRIGHTNESS_COLOROFWHITE_TYPE, lcd_get_brightness_colorpoint },
};

static struct oem_info_cmd oem_write_cmds[] = {
	{ BRIGHTNESS_COLOROFWHITE_TYPE, lcd_set_brightness_xcc_rgbw },
};

static ssize_t lcd_model_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if ((!runmode_is_factory()) && (common_info->panel_model != NULL))
		return snprintf(buf, PAGE_SIZE, "%s\n", common_info->panel_model);

	if (common_ops->get_panel_name)
		ret = common_ops->get_panel_name(buf);
	return ret;
}

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("NULL_PTR ERROR!\n");
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", is_mipi_cmd_panel(hisifd) ? 1 : 0);
}

static ssize_t lcd_panel_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_panel_info)
		ret = common_ops->get_panel_info(buf);
	return ret;
}

static ssize_t lcd_fps_scence_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	char str[LCD_REG_LENGTH_MAX] = {0};
	char tmp[MAX_BUF] = {0};
	int i, fps_rate;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd || !buf || !hisifd->panel_power_on)
		return LCD_KIT_FAIL;
	fps_rate = hisifd->panel_info.fps;
	if (!lcd_is_support_dfr()) {
		ret = snprintf(buf, PAGE_SIZE, "lcd_fps = %d \n", fps_rate);
		return ret;
	}
	if (fps_rate == LCD_FPS_60_HIGH)
		fps_rate = LCD_FPS_60;
	if (disp_info->fps.fps_need_high_60 != FPS_CONFIG_EN)
		ret = snprintf(str, sizeof(str), "current_fps:%d;default_fps:%d",
			fps_rate, LCD_FPS_60);
	else
		ret = snprintf(str, sizeof(str), "current_fps:%d;default_fps:%dn",
			fps_rate, LCD_FPS_60);
	if (disp_info->fps.panel_support_fps_list.buf == NULL ||
		disp_info->fps.panel_support_fps_list.cnt == 0) {
		LCD_KIT_INFO("%s\n", str);
		ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
		return ret;
	}
	strncat(str, ";support_fps_list:", strlen(";support_fps_list:"));
	for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
		if (i > 0)
			strncat(str, ",", strlen(","));
		fps_rate = disp_info->fps.panel_support_fps_list.buf[i];
		ret += snprintf(tmp, sizeof(tmp), "%d", fps_rate);
		strncat(str, tmp, strlen(tmp));
	}
	LCD_KIT_INFO("%s\n", str);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	return ret;
}

static int lcd_fps_get_scence(int fps, int *scence)
{
	switch (fps) {
	case LCD_FPS_30:
		*scence = LCD_FPS_SCENCE_30;
		break;
	case LCD_FPS_45:
		*scence = LCD_FPS_SCENCE_45;
		break;
	case LCD_FPS_60P:
		if (disp_info->fps.fps_need_high_60 != FPS_CONFIG_EN)
			*scence = LCD_FPS_SCENCE_60P;
		else
			*scence = LCD_FPS_SCENCE_60;
		break;
	case LCD_FPS_60_HIGH:
		*scence = LCD_FPS_SCENCE_60P;
		break;
	case LCD_FPS_90:
		*scence = LCD_FPS_SCENCE_90;
		break;
	case LCD_FPS_120:
		*scence = LCD_FPS_SCENCE_120;
		break;
	case LCD_FPS_180:
		*scence = LCD_FPS_SCENCE_180;
		break;
	default:
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static ssize_t lcd_fps_order_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char str[LCD_REG_LENGTH_MAX] = {0};
	char tmp[MAX_BUF] = {0};
	int i;
	int fps_index;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf || !attr) {
		LCD_KIT_ERR("lcd_fps_order_show buf NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info->fps.support || !disp_info->fps.fps_switch_support) {
		ret = snprintf(buf, PAGE_SIZE, "0\n");
		return ret;
	} else {
		if (disp_info->fps.fps_switch_order.cnt < FPS_ELEM_NUM ||
			(disp_info->fps.fps_switch_order.cnt % FPS_ELEM_NUM) != 0) {
			ret = snprintf(buf, PAGE_SIZE, "0\n");
			return ret;
		}
		if (disp_info->fps.fps_need_high_60 != FPS_CONFIG_EN)
			ret = snprintf(str, sizeof(str), "1,%d,%d", LCD_FPS_SCENCE_60P,
				disp_info->fps.fps_switch_order.cnt / FPS_ELEM_NUM);
		else
			ret = snprintf(str, sizeof(str), "1,%d,%d", LCD_FPS_SCENCE_60,
				disp_info->fps.fps_switch_order.cnt / FPS_ELEM_NUM);
		for (i = 0; i < disp_info->fps.fps_switch_order.cnt;) {
			ret = lcd_fps_get_scence(disp_info->fps.fps_switch_order.buf[i],
				&fps_index);
			if (ret)
				break;
			if (i == 0)
				strncat(str, ",", strlen(","));
			else
				strncat(str, ";", strlen(";"));
			ret += snprintf(tmp, sizeof(tmp), "%d:%d", fps_index,
				disp_info->fps.fps_switch_order.buf[i + 1]);
			strncat(str, tmp, strlen(tmp));
			i += FPS_ELEM_NUM;
		}
	}
	LCD_KIT_INFO("%s\n", str);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	return ret;
}

static ssize_t lcd_fps_scence_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (!pdata) {
		LCD_KIT_ERR("lcd fps scence store pdata NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd fps scence store buf NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	val = simple_strtoul(buf, NULL, 0);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("fb%d, panel power off!\n", hisifd->index);
		return LCD_KIT_FAIL;
	}
	if (disp_info->fps.support) {
		if (pdata->lcd_fps_scence_handle)
			pdata->lcd_fps_scence_handle(hisifd->pdev, val);
	}
	return count;
}

static ssize_t lcd_alpm_function_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd alpm function show buf is null!\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->alpm.support)
		ret = snprintf(buf, PAGE_SIZE, "aod_function = %d\n",
			hisifd->aod_function);
	return ret;
}

static ssize_t lcd_alpm_function_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd alpm function store buf null Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->alpm.support) {
		ret = sscanf(buf, "%u", &hisifd->aod_function);
		if (!ret) {
			LCD_KIT_ERR("sscanf return invaild:%zd\n", ret);
			return LCD_KIT_FAIL;
		}
	}
	return count;
}

static ssize_t lcd_alpm_setting_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct hisi_fb_data_type *hisifd = NULL;
	unsigned int mode = 0;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd alpm setting store buf is null!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	ret = sscanf(buf, "%u", &mode);
	if (!ret) {
		LCD_KIT_ERR("sscanf return invaild:%zd\n", ret);
		return LCD_KIT_FAIL;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (disp_info->alpm.support) {
			hisifb_activate_vsync(hisifd);
			lcd_kit_alpm_setting(hisifd, mode);
			hisifb_deactivate_vsync(hisifd);
		}
	}
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t lcd_amoled_acl_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_acl_mode)
		ret = common_ops->get_acl_mode(buf);
	return ret;
}

static ssize_t lcd_amoled_acl_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;
	unsigned long val = 0;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd amoled acl ctrl store buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (common_ops->set_acl_mode) {
			hisifb_activate_vsync(hisifd);
			ret = common_ops->set_acl_mode(hisifd, val);
			hisifb_deactivate_vsync(hisifd);
		}
	}
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t lcd_amoled_vr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_vr_mode)
		ret = common_ops->get_vr_mode(buf);
	return ret;
}

static ssize_t lcd_amoled_vr_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;
	unsigned long val = 0;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (common_ops->set_vr_mode) {
			hisifb_activate_vsync(hisifd);
			ret = common_ops->set_vr_mode(hisifd, val);
			hisifb_deactivate_vsync(hisifd);
		}
	}
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t lcd_effect_color_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_effect_color_mode)
		ret = common_ops->get_effect_color_mode(buf);
	return ret;
}

static ssize_t lcd_effect_color_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val = 0;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}
	if (common_ops->set_effect_color_mode)
		ret = common_ops->set_effect_color_mode(val);
	return count;
}

static ssize_t lcd_reg_read_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = LCD_KIT_OK;
	char lcd_reg_buf[LCD_REG_LENGTH_MAX] = {0};

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (disp_info->gamma_cal.support) {
			hisifb_activate_vsync(hisifd);
			lcd_kit_read_gamma(hisifd, lcd_reg_buf,
				LCD_REG_LENGTH_MAX);
			hisifb_deactivate_vsync(hisifd);
			LCD_KIT_INFO("%s\n", lcd_reg_buf);
			ret = snprintf(buf, sizeof(lcd_reg_buf), "%s\n",
				lcd_reg_buf);
		}
	}
	up(&hisifd->blank_sem);
	return ret;
}

static ssize_t lcd_reg_read_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
#define MAX_REG_LEN 100
	unsigned int reg_value[MAX_REG_LEN];
	char *cur = NULL;
	char *token = NULL;
	int i = 0;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->gamma_cal.support) {
		/* parse buf */
		cur = (char *)buf;
		token = strsep(&cur, ",");
		while (token) {
			reg_value[i++] = simple_strtol(token, NULL, 0);
			token = strsep(&cur, ",");
			if (i >= MAX_REG_LEN) {
				LCD_KIT_INFO("count is too long\n");
				return LCD_KIT_FAIL;
			}
		}
		disp_info->gamma_cal.addr = reg_value[0];
		disp_info->gamma_cal.length = reg_value[1];
	}
	return count;
}

static ssize_t lcd_gamma_dynamic_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->gamma_cal.support) {
		if (count != GM_IGM_LEN) {
			HISI_FB_ERR("gamma count err!count=%d\n", (int)count);
			return LCD_KIT_FAIL;
		}
		hisifb_update_dynamic_gamma(hisifd, buf, count);
	}
	return count;
}

static ssize_t lcd_frame_count_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd frame count buf is null!\n");
		return LCD_KIT_FAIL;
	}
	return snprintf(buf, PAGE_SIZE, "%u\n", hisifd->frame_count);
}

static ssize_t lcd_frame_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	return snprintf(buf, PAGE_SIZE, "%u\n", hisifd->vsync_ctrl.vsync_infinite);
}

static ssize_t lcd_frame_update_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	unsigned long val;
	static uint32_t esd_enable;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	val = (int)simple_strtoul(buf, NULL, 0);
	down(&hisifd->blank_sem);
	g_enable_dirty_region_updt = (val > 0) ? 0 : 1;
	hisifd->frame_update_flag = (val > 0) ? 1 : 0;
	hisifb_set_vsync_activate_state(hisifd, (val > 0) ? true : false);
	if (!is_mipi_cmd_panel(hisifd))
		goto err_out;
	if (!hisifd->panel_power_on) {
		HISI_FB_DEBUG("fb%d, panel power off!\n", hisifd->index);
		goto err_out;
	}
	hisifb_activate_vsync(hisifd);
	if (val == 1) {
		esd_enable = hisifd->panel_info.esd_enable;
		hisifd->panel_info.esd_enable = 0;
		mdelay(50);
	}
	ldi_frame_update(hisifd, (val > 0) ? true : false);
	if (val == 0) {
		hisifd->vactive0_start_flag = 1;
		mdelay(50);
		hisifd->panel_info.esd_enable = esd_enable;
		esd_enable = 0;
	}
	hisifb_deactivate_vsync(hisifd);
err_out:
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t lcd_mipi_clk_upt_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("mipi clk upt show hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (!pdata) {
		LCD_KIT_ERR("mipi clk upt show pdata is null!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd mipi clk upt show buf is null!\n");
		return LCD_KIT_FAIL;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (pdata->mipi_dsi_bit_clk_upt_show) {
			hisifb_activate_vsync(hisifd);
			ret = pdata->mipi_dsi_bit_clk_upt_show(hisifd->pdev, buf);
			hisifb_deactivate_vsync(hisifd);
		}
	}
	up(&hisifd->blank_sem);
	return ret;
}

static ssize_t lcd_mipi_clk_upt_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (!pdata) {
		LCD_KIT_ERR("pdata is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (pdata->mipi_dsi_bit_clk_upt_store) {
			if (lcd_is_support_dfr() && lcd_is_high_fps_state(hisifd)) {
				LCD_KIT_INFO("high fps state not support dsi clk update\n");
				up(&hisifd->blank_sem);
				return LCD_KIT_OK;
			}
			hisifb_activate_vsync(hisifd);
			ret = pdata->mipi_dsi_bit_clk_upt_store(hisifd->pdev, buf, count);
			hisifb_deactivate_vsync(hisifd);
		}
	}
	up(&hisifd->blank_sem);
	return ret;
}

static ssize_t lcd_func_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd || !buf) {
		LCD_KIT_ERR("hisifd or buf is null\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	ret = snprintf(buf, PAGE_SIZE,
			"sbl=%d\n"
			"xcc_support=%d\n"
			"dsi_bit_clk_upt=%d\n"
			"dirty_region_upt=%d\n"
			"fps_updt_support=%d\n"
			"ifbc_type=%d\n"
			"esd_enable=%d\n"
			"blpwm_input_ena=%d\n"
			"lane_nums=%d\n"
			"panel_effect_support=%d\n"
			"color_temp_rectify_support=%d\n"
			"ddic_rgbw_support=%d\n"
			"hiace=%d\n"
			"effect_enable=%d\n"
			"effect_debug=%d\n"
			"fps_support=%d\n"
			"bl_delay_enable=%d\n"
			"panel_on_change_status=%d\n",
			pinfo->sbl_support,
			pinfo->xcc_support,
			pinfo->dsi_bit_clk_upt_support,
			pinfo->dirty_region_updt_support,
			pinfo->fps_updt_support,
			pinfo->ifbc_type,
			pinfo->esd_enable,
			pinfo->blpwm_input_ena,
			pinfo->mipi.lane_nums + 1,
			pinfo->panel_effect_support,
			pinfo->color_temp_rectify_support,
			pinfo->rgbw_support,
			pinfo->hiace_support,
			g_enable_effect,
			g_debug_effect,
			disp_info->fps.support,
			pinfo->delay_set_bl_thr_support,
			common_info->panel_cmd_backup.change_flag);
	return ret;
}

static ssize_t lcd_func_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	char command[MAX_BUF] = {0};

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		HISI_FB_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	if (!sscanf(buf, "%s", command)) {
		LCD_KIT_INFO("bad command(%s)\n", command);
		return count;
	}
	lcd_kit_parse_switch_cmd(hisifd, command);
	return count;
}

static int lcd_get_project_id(char *oem_data, struct hisi_fb_data_type *hisifd)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	char project_id[PROJECT_ID_LENGTH + 1] = {0};
	int ret = 0;
	int i = 0;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_ops->get_project_id)
		ret = lcd_ops->get_project_id(project_id);
	oem_data[0] = PROJECT_ID_TYPE;
	oem_data[1] = OEM_INFO_BLOCK_NUM;
	for (i = 0; i < PROJECT_ID_LENGTH; i++)
		oem_data[i + 2] = project_id[i]; /* project id store behind 3th array */
	return ret;
}

static int lcd_get_2d_barcode(char *oem_data, struct hisi_fb_data_type *hisifd)
{
	char read_value[OEM_INFO_SIZE_MAX + 1];
	int ret = LCD_KIT_OK;
	struct lcd_kit_panel_ops *panel_ops = NULL;

	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_get_2d_barcode)
		return panel_ops->lcd_get_2d_barcode(oem_data, hisifd);

	memset(read_value, 0, OEM_INFO_SIZE_MAX + 1);

	if (disp_info->oeminfo.barcode_2d.support) {
		ret = lcd_kit_dsi_cmds_rx(hisifd, read_value, OEM_INFO_SIZE_MAX,
			&disp_info->oeminfo.barcode_2d.cmds);
		oem_data[0] = BARCODE_2D_TYPE;
		oem_data[1] = disp_info->oeminfo.barcode_2d.block_num;
		strncat(oem_data, read_value, strlen(read_value));
	}

	return ret;
}

static int lcd_get_brightness_colorpoint(char *oem_data,
	struct hisi_fb_data_type *hisifd)
{
	char read_value[OEM_INFO_SIZE_MAX + 1] = {0};
	struct lcd_kit_brightness_color_oeminfo *bri_col_info = NULL;
	uint32_t value_temp;

	if (disp_info->oeminfo.brightness_color_uniform.support) {
		lcd_kit_dsi_cmds_rx(hisifd, read_value, OEM_INFO_SIZE_MAX,
			&disp_info->oeminfo.brightness_color_uniform.brightness_color_cmds);
		oem_data[0] = BRIGHTNESS_COLOROFWHITE_TYPE;
		oem_data[1] = OEM_INFO_BLOCK_NUM * 2; // it means the second block num
		bri_col_info = lcd_kit_get_brightness_color_oeminfo();
		if (!bri_col_info) {
			LCD_KIT_ERR("bri_col_info is null\n");
			return LCD_KIT_FAIL;
		}
		/* brightness data */
		oem_data[2] = 0x00;
		oem_data[3] = 0x00;
		oem_data[4] = read_value[0];
		oem_data[5] = read_value[1];

		/* Color point of White x(2Bytes) */
		oem_data[6] = 0x00;
		oem_data[7] = 0x00;
		oem_data[8] = read_value[2];
		oem_data[9] = read_value[3];

		/* Color point of White y(2Bytes) */
		oem_data[10] = 0x00;
		oem_data[11] = 0x00;
		oem_data[12] = read_value[4];
		oem_data[13] = read_value[5];

		/* modulesn:Serial number of inspection system(4Bytes) */
		value_temp = bri_col_info->panel_id.modulesn;
		oem_data[14] = (uint8_t)((value_temp >> 24) & 0xFF);
		oem_data[15] = (uint8_t)((value_temp >> 16) & 0xFF);
		oem_data[16] = (uint8_t)((value_temp >> 8) & 0xFF);
		oem_data[17] = (uint8_t)(value_temp & 0xFF);

		/* equipid(1Bytes) */
		value_temp = bri_col_info->panel_id.equipid;
		oem_data[18] = (uint8_t)((value_temp >> 24) & 0xFF);
		oem_data[19] = (uint8_t)((value_temp >> 16) & 0xFF);
		oem_data[20] = (uint8_t)((value_temp >> 8) & 0xFF);
		oem_data[21] = (uint8_t)(value_temp & 0xFF);

		/* modulemanufactdate:year(1Bytes)-month(1Bytes)-date(1Bytes) */
		value_temp = bri_col_info->panel_id.modulemanufactdate;
		oem_data[22] = (uint8_t)((value_temp >> 24) & 0xFF);
		oem_data[23] = (uint8_t)((value_temp >> 16) & 0xFF);
		oem_data[24] = (uint8_t)((value_temp >> 8) & 0xFF);
		oem_data[25] = (uint8_t)(value_temp & 0xFF);

		/* vendorid(1Bytes) */
		value_temp = bri_col_info->panel_id.vendorid;
		oem_data[26] = (uint8_t)((value_temp >> 24) & 0xFF);
		oem_data[27] = (uint8_t)((value_temp >> 16) & 0xFF);
		oem_data[28] = (uint8_t)((value_temp >> 8) & 0xFF);
		oem_data[29] = (uint8_t)(value_temp & 0xFF);
	}
	return LCD_KIT_OK;
}

static int lcd_set_brightness_xcc_rgbw(char *oem_data,
	struct hisi_fb_data_type *hisifd)
{
#define MAX_BUF_LEN 128
#define MAX_CNT 9
	uint32_t i;
	uint32_t j;
	uint32_t ret;
	uint32_t *tmp = NULL;
	uint32_t count = MAX_CNT;
	struct lcd_kit_brightness_color_oeminfo *oeminfo = NULL;
	char buf[MAX_BUF_LEN] = {0};

	if ((!hisifd) || (!oem_data)) {
		LCD_KIT_ERR("NULL pointer\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < oem_data[1]; i++)
		LCD_KIT_INFO("oem_data[%d] = %d\n", i, oem_data[i]);
	oeminfo = lcd_kit_get_brightness_color_oeminfo();
	tmp = (uint32_t *)oeminfo;
	for (i = 0, j = 2; i < sizeof(struct lcd_kit_brightness_color_oeminfo) / sizeof(uint32_t); i++) {
		tmp[i] = (uint32_t)((((uint32_t)oem_data[j]) << 24) |
			(((uint32_t)oem_data[(long)j + 1]) << 16) |
			(((uint32_t)oem_data[(long)j + 2]) << 8) |
			((uint32_t)oem_data[(long)j + 3]));
		j = j + 4;
	}
	snprintf(buf, sizeof(buf), "%d,%d,%d,%d,%d,%d,%d,%d,%d",
			oeminfo->color_params.mxcc_matrix[0][0],
			oeminfo->color_params.mxcc_matrix[0][1],
			oeminfo->color_params.mxcc_matrix[0][2],
			oeminfo->color_params.mxcc_matrix[1][0],
			oeminfo->color_params.mxcc_matrix[1][1],
			oeminfo->color_params.mxcc_matrix[1][2],
			oeminfo->color_params.mxcc_matrix[2][0],
			oeminfo->color_params.mxcc_matrix[2][1],
			oeminfo->color_params.mxcc_matrix[2][2]);
	LCD_KIT_INFO("buf:%s\n", buf);
	ret = lcd_kit_realtime_set_xcc(hisifd, buf, count);
	if (ret != 0)
		LCD_KIT_ERR("realtime set lcd xcc failed!\n");
	return ret;
}

static ssize_t lcd_oem_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	int i;
	struct hisi_fb_data_type *hisifd = NULL;
	char oem_info_data[OEM_INFO_SIZE_MAX];
	char str_oem[OEM_INFO_SIZE_MAX + 1] = {0};
	char str_tmp[OEM_INFO_SIZE_MAX + 1] = {0};

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info->oeminfo.support) {
		LCD_KIT_ERR("oem info is not support\n");
		return LCD_KIT_FAIL;
	}
	if (oem_info_type == -1) {
		LCD_KIT_ERR("first write ddic_oem_info, then read\n");
		return LCD_KIT_FAIL;
	}
	memset(oem_info_data, 0, OEM_INFO_SIZE_MAX);
	/* execute cmd func */
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		hisifb_activate_vsync(hisifd);
		for (i = 0; i < ARRAY_SIZE(oem_read_cmds); i++) {
			if (oem_info_type != oem_read_cmds[i].type)
				continue;
			LCD_KIT_INFO("cmd = %d\n", oem_info_type);
			if (oem_read_cmds[i].func)
				(*oem_read_cmds[i].func)(oem_info_data, hisifd);
		}
		hisifb_deactivate_vsync(hisifd);
	}
	up(&hisifd->blank_sem);
	/* parse data */
	memset(str_oem, 0, sizeof(str_oem));
	for (i = 0; i < oem_info_data[1]; i++) {
		memset(str_tmp, 0, sizeof(str_tmp));
		snprintf(str_tmp, sizeof(str_tmp), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
		oem_info_data[0 + i * BARCODE_BLOCK_LEN],
		oem_info_data[1 + i * BARCODE_BLOCK_LEN],
		oem_info_data[2 + i * BARCODE_BLOCK_LEN],
		oem_info_data[3 + i * BARCODE_BLOCK_LEN],
		oem_info_data[4 + i * BARCODE_BLOCK_LEN],
		oem_info_data[5 + i * BARCODE_BLOCK_LEN],
		oem_info_data[6 + i * BARCODE_BLOCK_LEN],
		oem_info_data[7 + i * BARCODE_BLOCK_LEN],
		oem_info_data[8 + i * BARCODE_BLOCK_LEN],
		oem_info_data[9 + i * BARCODE_BLOCK_LEN],
		oem_info_data[10 + i * BARCODE_BLOCK_LEN],
		oem_info_data[11 + i * BARCODE_BLOCK_LEN],
		oem_info_data[12 + i * BARCODE_BLOCK_LEN],
		oem_info_data[13 + i * BARCODE_BLOCK_LEN],
		oem_info_data[14 + i * BARCODE_BLOCK_LEN],
		oem_info_data[15 + i * BARCODE_BLOCK_LEN]);
		strncat(str_oem, str_tmp, strlen(str_tmp));
	}
	LCD_KIT_INFO("str_oem = %s\n", str_oem);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str_oem);
	return ret;
}

static ssize_t lcd_oem_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	char *cur = NULL;
	char *token = NULL;
	char oem_info[OEM_INFO_SIZE_MAX] = {0};
	int i = 0;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info->oeminfo.support) {
		LCD_KIT_ERR("oem info is not support\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) < OEM_INFO_SIZE_MAX) {
		cur = (char *)buf;
		token = strsep(&cur, ",");
		while (token) {
			oem_info[i++] = (unsigned char)simple_strtol(token, NULL, 0);
			token = strsep(&cur, ",");
		}
	} else {
		memcpy(oem_info, "INVALID", strlen("INVALID") + 1);
		LCD_KIT_INFO("invalid cmd\n");
	}
	LCD_KIT_INFO("write Type=0x%x , data len=%d\n", oem_info[0], oem_info[1]);
	oem_info_type = oem_info[0];
	/* if the data length is 0, then just store the type */
	if (oem_info[1] == 0) {
		LCD_KIT_INFO("Just store type:0x%x and then finished\n", oem_info[0]);
		return count;
	}
	/* execute cmd func */
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		hisifb_activate_vsync(hisifd);
		for (i = 0; i < ARRAY_SIZE(oem_write_cmds); i++) {
			if (oem_info_type == oem_write_cmds[i].type)
				(*oem_write_cmds[i].func)(oem_info, hisifd);
		}
		hisifb_deactivate_vsync(hisifd);
	}
	up(&hisifd->blank_sem);
	LCD_KIT_INFO("oem_info = %s\n", oem_info);
	return count;
}

static ssize_t lcd_effect_bl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	if (!dev) {
		LCD_KIT_ERR("effect_bl_show dev is null!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("effect_bl_show buf is null!\n");
		return LCD_KIT_FAIL;
	}
	ret = hisifb_display_effect_bl_ctrl_show(dev_get_drvdata(dev), buf);
	return ret;
}

static ssize_t lcd_effect_bl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;

	if (!dev) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}

	ret = hisifb_display_effect_bl_ctrl_store(dev_get_drvdata(dev), buf, count);
	return ret;
}
#ifdef LCD_FACTORY_MODE
static int lcd_judge_ddic_lv_detect(struct hisi_fb_data_type *hisifd,
	unsigned int pic_index, int loop)
{
	int i;
	unsigned char expect_value;
	int ret;
	unsigned int *detect_value = NULL;
	unsigned char read_value[MAX_REG_READ_COUNT] = {0};
	struct lcd_kit_array_data congfig_data;

	congfig_data = FACT_INFO->ddic_lv_detect.value[pic_index];
	detect_value = congfig_data.buf;
	if (FACT_INFO->ddic_lv_detect.rd_cmds[pic_index].cmds == NULL) {
		LCD_KIT_INFO("read ddic lv detect cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* delay 2s, or ddic reg value invalid */
	ssleep(2);
	ret = lcd_kit_dsi_cmds_rx(hisifd, read_value, MAX_REG_READ_COUNT - 1,
		&FACT_INFO->ddic_lv_detect.rd_cmds[pic_index]);
	if (ret) {
		LCD_KIT_INFO("read ddic lv detect cmd error\n");
		return ret;
	}
	for (i = 0; i < congfig_data.cnt; i++) {
		/* obtains the value of the first byte */
		expect_value = detect_value[i] & 0xFF;
		LCD_KIT_INFO("pic: %u, read_val:%d = 0x%x, expect_val = 0x%x\n",
			pic_index, i, read_value[i], expect_value);
		if ((i < VAL_NUM) && (loop < DETECT_LOOPS))
			FACT_INFO->ddic_lv_detect.reg_val[loop][pic_index][i] =
				read_value[i];
		if (read_value[i] != expect_value) {
			FACT_INFO->ddic_lv_detect.err_flag[pic_index]++;
			LCD_KIT_ERR("pic: %u, read_val:%d = 0x%x, but expect_val = 0x%x\n",
				pic_index, i, read_value[i], expect_value);
			return LCD_KIT_FAIL;
		}
	}
	return ret;
}

static int lcd_ddic_lv_detect_test(struct hisi_fb_data_type *hisifd)
{
	int i;
	int ret;
	static int count;
	static int loop_num;
	int err_record = 0;
	unsigned int pic_index;
	unsigned int *error_flag = NULL;

	pic_index = FACT_INFO->ddic_lv_detect.pic_index;
	error_flag = FACT_INFO->ddic_lv_detect.err_flag;
	if (pic_index >= DETECT_NUM) {
		LCD_KIT_ERR("pic number not support\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_judge_ddic_lv_detect(hisifd, pic_index, loop_num);
	if (ret)
		LCD_KIT_ERR("ddic lv detect judge fail\n");
	count++;
	if (count >= DETECT_NUM) {
		loop_num++;
		/* set initial value */
		count = 0;
	}
	LCD_KIT_INFO("count = %d, loop_num = %d\n", count, loop_num);
	if (loop_num >= DETECT_LOOPS) {
		for (i = 0; i < DETECT_NUM; i++) {
			LCD_KIT_INFO("pic : %d, err_num = %d\n",
				i, error_flag[i]);
			if (error_flag[i] > ERR_THRESHOLD)
				err_record++;
			/* set initial value */
			FACT_INFO->ddic_lv_detect.err_flag[i] = 0;
		}
		if (err_record >= i)
			lcd_kit_ddic_lv_detect_dmd_report(
				FACT_INFO->ddic_lv_detect.reg_val);
		/* set initial value */
		loop_num = 0;
	}
	return ret;
}

static int lcd_ddic_lv_detect_set(struct hisi_fb_data_type *hisifd,
	unsigned int pic_index)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *enter_cmds = NULL;

	pic_index = pic_index - DET_START;
	enter_cmds = FACT_INFO->ddic_lv_detect.enter_cmds;
	if ((pic_index >= DET1_INDEX) && (pic_index < DETECT_NUM)) {
		/* disable esd */
		lcd_esd_enable(hisifd, 0);
		if (enter_cmds[pic_index].cmds == NULL) {
			LCD_KIT_ERR("send ddic lv detect cmd is NULL\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_dsi_cmds_tx(hisifd, &enter_cmds[pic_index]);
		if (ret) {
			LCD_KIT_ERR("send ddic lv detect cmd error\n");
			return ret;
		}
		LCD_KIT_INFO("set picture : %u\n", pic_index);
		FACT_INFO->ddic_lv_detect.pic_index = pic_index;
	} else {
		LCD_KIT_INFO("set picture : %u unknown\n", pic_index);
		FACT_INFO->ddic_lv_detect.pic_index = INVALID_INDEX;
	}
	return ret;
}

static ssize_t lcd_ddic_lv_detect_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int result;
	ssize_t ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->ddic_lv_detect.support) {
		down(&hisifd->blank_sem);
		if (hisifd->panel_power_on) {
			hisifb_activate_vsync(hisifd);
			result = lcd_ddic_lv_detect_test(hisifd);
			hisifb_deactivate_vsync(hisifd);
			ret = snprintf(buf, PAGE_SIZE, "%d\n", result);
		}
		up(&hisifd->blank_sem);
		lcd_kit_recovery_display(hisifd);
		/* enable esd */
		lcd_esd_enable(hisifd, 1);
	}
	return ret;
}

static ssize_t lcd_ddic_lv_detect_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int index = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!FACT_INFO->ddic_lv_detect.support) {
		LCD_KIT_ERR("ddic lv detect is not support\n");
		return LCD_KIT_FAIL;
	}
	if (dev == NULL) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = dev_get_hisifd(dev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	/* decimal parsing */
	ret = kstrtouint(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("strict_strtoul fail\n");
		return ret;
	}
	LCD_KIT_INFO("picture index=%u\n", index);
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		hisifb_activate_vsync(hisifd);
		ret = lcd_ddic_lv_detect_set(hisifd, index);
		hisifb_deactivate_vsync(hisifd);
	}
	up(&hisifd->blank_sem);
	return ret;
}
#endif

static ssize_t lcd_hbm_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!buf) {
		LCD_KIT_ERR("buf is NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("common_info->hbm_mode = %u\n",
		 common_info->hbm_mode);

	return snprintf(buf, PAGE_SIZE, "%u\n", common_info->hbm_mode);
}

static ssize_t lcd_hbm_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hisi_fb_panel_data *pdata = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!buf || !dev) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (!pdata) {
		LCD_KIT_ERR("pdata is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!pdata->lcd_set_backlight_by_type_func) {
		LCD_KIT_ERR("lcd_set_backlight_by_type_func is NULL\n");
		return LCD_KIT_FAIL;
	}
	common_info->hbm_mode = simple_strtoul(buf, NULL, 0);

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("fb %d, panel power off!\n", hisifd->index);
		up(&hisifd->blank_sem);
		return LCD_KIT_FAIL;
	}

	hisifb_activate_vsync(hisifd);
	if (!common_info->hbm_mode)
		pdata->lcd_set_backlight_by_type_func(hisifd->pdev,
			BACKLIGHT_LOW_LEVEL);
	else
		pdata->lcd_set_backlight_by_type_func(hisifd->pdev,
			BACKLIGHT_HIGH_LEVEL);
	hisifb_deactivate_vsync(hisifd);

	up(&hisifd->blank_sem);
	return count;
}

static ssize_t lcd_local_hbm_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!dev) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}

	if (!common_info->hbm.local_hbm_support) {
		LCD_KIT_ERR("local hbm is not support!\n");
		return LCD_KIT_FAIL;
	}

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("fb %d, panel power off!\n", hisifd->index);
		up(&hisifd->blank_sem);
		return LCD_KIT_FAIL;
	}

	hisifb_activate_vsync(hisifd);
	ret = lcd_kit_local_hbm(hisifd, buf, count);
	hisifb_deactivate_vsync(hisifd);
	up(&hisifd->blank_sem);
	return ret;
}

static ssize_t lcd_panel_sncode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	ssize_t ret;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char str_oem[OEM_INFO_SIZE_MAX] = {0};
	char str_tmp[OEM_INFO_SIZE_MAX] = {0};

	hisifd = dev_get_hisifd(dev);
	if (!hisifd || !buf) {
		LCD_KIT_ERR("hisifd or buf is null\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (lcd_kit_get_product_type() == LCD_FOLDER_TYPE)
		pinfo = lcd_kit_get_pinfo(fold_panel_id);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}

	memset(str_oem, 0, sizeof(str_oem));
	for(i = 0; i < sizeof(pinfo->sn_code); i++) {
		memset(str_tmp, 0, sizeof(str_tmp));
		ret = snprintf(str_tmp, sizeof(str_tmp), "%d,",pinfo->sn_code[i]);
		if (ret < 0) {
			LCD_KIT_ERR("snprintf fail\n");
			return LCD_KIT_FAIL;
		}
		strncat(str_oem, str_tmp, strlen(str_tmp));
	}
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str_oem);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf fail\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}
static ssize_t lcd_panel_sncode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned int val = 0;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtouint(buf, 10, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}

	fold_panel_id = val;
	LCD_KIT_INFO("panel id is %u\n", val);
	return ret;
}
static ssize_t lcd_pre_camera_position_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t len;

	if (!buf) {
		LCD_KIT_ERR("buf is NULL Pointer\n");
		return LCD_KIT_FAIL;
	}

	len = snprintf(buf, PAGE_SIZE, "end_y:%u\n",
		common_info->p_cam_position.end_y);
	if (len < 0) {
		LCD_KIT_ERR("snprintf fail\n");
		return LCD_KIT_FAIL;
	}
	return len;
}

static ssize_t lcd_panel_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd || !buf) {
		LCD_KIT_ERR("hisifd or buf is null\n");
		return LCD_KIT_FAIL;
	}

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}

	if (!disp_info->panel_version.support) {
		ret = snprintf(buf, PAGE_SIZE, "VER:NA\n");
		return ret;
	}
	ret = snprintf(buf, PAGE_SIZE, "%s\n", pinfo->lcd_panel_version);
	return ret;
}

static int lcd_check_support(int index)
{
	if (runmode_is_factory())
		return SYSFS_SUPPORT;

	switch (index) {
	case LCD_MODEL_INDEX:
		return SYSFS_SUPPORT;
	case LCD_TYPE_INDEX:
		return SYSFS_SUPPORT;
	case PANEL_INFO_INDEX:
		return SYSFS_SUPPORT;
	case VOLTAGE_ENABLE_INDEX:
		return SYSFS_NOT_SUPPORT;
	case ACL_INDEX:
		return common_info->acl.support;
	case VR_INDEX:
		return common_info->vr.support;
	case SUPPORT_MODE_INDEX:
		return common_info->effect_color.support;
	case GAMMA_DYNAMIC_INDEX:
		return disp_info->gamma_cal.support;
	case FRAME_COUNT_INDEX:
	case FRAME_UPDATE_INDEX:
	case MIPI_DSI_CLK_UPT_INDEX:
	case FPS_SCENCE_INDEX:
	case FPS_ORDER_INDEX:
		return SYSFS_SUPPORT;
	case ALPM_FUNCTION_INDEX:
		return disp_info->alpm.support;
	case ALPM_SETTING_INDEX:
		return disp_info->alpm.support;
	case FUNC_SWITCH_INDEX:
		return SYSFS_SUPPORT;
	case REG_READ_INDEX:
		return disp_info->gamma_cal.support;
	case DDIC_OEM_INDEX:
		return disp_info->oeminfo.support;
	case BL_MODE_INDEX:
		return SYSFS_NOT_SUPPORT;
	case BL_SUPPORT_MODE_INDEX:
		return SYSFS_NOT_SUPPORT;
	case EFFECT_BL_INDEX:
		return SYSFS_SUPPORT;
	case HBM_MODE_INDEX:
	case DDIC_ALPHA_INDEX:
		return SYSFS_SUPPORT;
	case PANEL_SNCODE_INDEX:
		return common_info->sn_code.support;
	case PRE_CAMERA_POSITION:
		return common_info->p_cam_position.support;
	case PANEL_VERSION_INDEX:
		return disp_info->panel_version.support;
	default:
		return SYSFS_NOT_SUPPORT;
	}
}

struct lcd_kit_sysfs_ops g_lcd_sysfs_ops = {
	.check_support = lcd_check_support,
	.model_show = lcd_model_show,
	.type_show = lcd_type_show,
	.panel_info_show = lcd_panel_info_show,
	.amoled_acl_ctrl_show = lcd_amoled_acl_ctrl_show,
	.amoled_acl_ctrl_store = lcd_amoled_acl_ctrl_store,
	.amoled_vr_mode_show = lcd_amoled_vr_mode_show,
	.amoled_vr_mode_store = lcd_amoled_vr_mode_store,
	.effect_color_mode_show = lcd_effect_color_mode_show,
	.effect_color_mode_store = lcd_effect_color_mode_store,
	.reg_read_show = lcd_reg_read_show,
	.reg_read_store = lcd_reg_read_store,
	.gamma_dynamic_store = lcd_gamma_dynamic_store,
	.frame_count_show = lcd_frame_count_show,
	.frame_update_show = lcd_frame_update_show,
	.frame_update_store = lcd_frame_update_store,
	.mipi_dsi_clk_upt_show = lcd_mipi_clk_upt_show,
	.mipi_dsi_clk_upt_store = lcd_mipi_clk_upt_store,
	.fps_scence_show = lcd_fps_scence_show,
	.fps_scence_store = lcd_fps_scence_store,
	.fps_order_show = lcd_fps_order_show,
	.alpm_function_show = lcd_alpm_function_show,
	.alpm_function_store = lcd_alpm_function_store,
	.alpm_setting_store = lcd_alpm_setting_store,
	.func_switch_show = lcd_func_switch_show,
	.func_switch_store = lcd_func_switch_store,
	.ddic_oem_info_show = lcd_oem_info_show,
	.ddic_oem_info_store = lcd_oem_info_store,
	.effect_bl_show = lcd_effect_bl_show,
	.effect_bl_store = lcd_effect_bl_store,
#ifdef LCD_FACTORY_MODE
	.ddic_lv_detect_test_show = lcd_ddic_lv_detect_test_show,
	.ddic_lv_detect_test_store = lcd_ddic_lv_detect_test_store,
#endif
	.hbm_mode_show = lcd_hbm_mode_show,
	.hbm_mode_store = lcd_hbm_mode_store,
	.ddic_local_hbm_store = lcd_local_hbm_store,
	.panel_sncode_show = lcd_panel_sncode_show,
	.panel_sncode_store = lcd_panel_sncode_store,
	.pre_camera_position_show = lcd_pre_camera_position_show,
	.panel_version_show = lcd_panel_version_show,
};

int lcd_kit_sysfs_init(void)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;

	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_sysfs_ops_register(&g_lcd_sysfs_ops);
	ret = lcd_kit_create_sysfs(&hisifd->fbi->dev->kobj);
	if (ret)
		LCD_KIT_ERR("create fs node fail\n");
	return LCD_KIT_OK;
}
