/*
 * lcd_kit_utils.c
 *
 * lcdkit utils function for lcd driver
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

#include "lcd_kit_utils.h"
#include <linux/time.h>
#include "lcd_kit_common.h"
#include "lcd_kit_drm_panel.h"
#include "lcd_kit_power.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_core.h"
#include "lcd_kit_sysfs_qcom.h"
#ifdef CONFIG_APP_INFO
#include <misc/app_info.h>
#endif
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif

#define MAX_DELAY_TIME 200
#define LCDKIT_DEFAULT_PANEL_NAME "AUO_OTM1901A 5.2' VIDEO TFT 1080 x 1920 DEFAULT"

bool lcd_kit_support(void)
{
	struct device_node *lcdkit_np = NULL;
	const char *support_type = NULL;
	ssize_t ret;

	lcdkit_np = of_find_compatible_node(NULL, NULL,
		DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!lcdkit_np) {
		LCD_KIT_ERR("NOT FOUND device node!\n");
		return false;
	}
	ret = of_property_read_string(lcdkit_np, "support_lcd_type",
		&support_type);
	if (ret) {
		LCD_KIT_ERR("failed to get support_type\n");
		return false;
	}
	if (!strncmp(support_type, "LCD_KIT", strlen("LCD_KIT"))) {
		LCD_KIT_INFO("lcd_kit is support!\n");
		return true;
	}
	LCD_KIT_INFO("lcd_kit is not support!\n");
	return false;
}

static int lcd_kit_check_project_id(void)
{
	int i;

	for (i = 0; i < PROJECTID_LEN; i++) {
		if (isalnum((disp_info->project_id.id)[i]) == 0 &&
			isdigit((disp_info->project_id.id)[i]) == 0)
			return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

int lcd_kit_read_project_id(void)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct qcom_panel_info *panel_info = NULL;

	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		LCD_KIT_ERR("panel_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_rx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info) {
		LCD_KIT_ERR("disp_info is null\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->project_id.support) {
		memset(disp_info->project_id.id, 0,
			sizeof(disp_info->project_id.id));
		ret = adapt_ops->mipi_rx(panel_info->display, disp_info->project_id.id,
			LCD_DDIC_INFO_LEN - 1, &disp_info->project_id.cmds);
		if (ret) {
			LCD_KIT_ERR("read project id fail\n");
			return LCD_KIT_FAIL;
		}
		if (lcd_kit_check_project_id() == LCD_KIT_OK) {
			LCD_KIT_ERR("project id check fail\n");
			return LCD_KIT_FAIL;
		}

		LCD_KIT_INFO("read project id is %s\n",
			disp_info->project_id.id);
		return LCD_KIT_OK;
	} else {
		LCD_KIT_ERR("project id is not support!\n");
		return LCD_KIT_FAIL;
	}
}

void lcd_kit_disp_on_check_delay(void)
{
	long delta_time_bl_to_panel_on;
	unsigned int delay_margin;
	struct timeval tv = {0};
	int max_delay_margin = MAX_DELAY_TIME;

	if (disp_info == NULL) {
		LCD_KIT_INFO("disp_info is NULL\n");
		return;
	}
	memset(&tv, 0, sizeof(struct timeval));
	do_gettimeofday(&tv);
	/* change s to us */
	delta_time_bl_to_panel_on = (tv.tv_sec - disp_info->quickly_sleep_out.panel_on_record_tv.tv_sec) *
		1000000 + tv.tv_usec - disp_info->quickly_sleep_out.panel_on_record_tv.tv_usec;
	/* change us to ms */
	delta_time_bl_to_panel_on /= 1000;
	if (delta_time_bl_to_panel_on >= disp_info->quickly_sleep_out.interval) {
		LCD_KIT_INFO("%lu > %d, no need delay\n",
			delta_time_bl_to_panel_on,
			disp_info->quickly_sleep_out.interval);
		disp_info->quickly_sleep_out.panel_on_tag = false;
		return;
	}
	delay_margin = disp_info->quickly_sleep_out.interval -
		delta_time_bl_to_panel_on;
	if (delay_margin > max_delay_margin) {
		LCD_KIT_INFO("something maybe error");
		disp_info->quickly_sleep_out.panel_on_tag = false;
		return;
	}
	msleep(delay_margin);
	LCD_KIT_INFO("backlight on delay %dms\n", delay_margin);
	disp_info->quickly_sleep_out.panel_on_tag = false;
}

void lcd_kit_disp_on_record_time(void)
{
	if (disp_info == NULL) {
		LCD_KIT_INFO("disp_info is NULL\n");
		return;
	}
	do_gettimeofday(&disp_info->quickly_sleep_out.panel_on_record_tv);
	LCD_KIT_INFO("display on at %lu seconds %lu mil seconds\n",
		disp_info->quickly_sleep_out.panel_on_record_tv.tv_sec,
		disp_info->quickly_sleep_out.panel_on_record_tv.tv_usec);
	disp_info->quickly_sleep_out.panel_on_tag = true;
}

void lcd_kit_pinfo_init(struct device_node *np, struct qcom_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-type",
		&pinfo->bl_set_type, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-min",
		&pinfo->bl_min, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-max",
		&pinfo->bl_max, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-def",
		&pinfo->bl_default, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-cmd-type",
		&pinfo->type, 0);

	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-lcm-type",
		&pinfo->panel_lcm_type, 0);
	/* project id */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,project-id-support",
		&disp_info->project_id.support, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-gpio-offset",
		&pinfo->gpio_offset, 0);
	if (disp_info->project_id.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,project-id-cmds",
			"lcd-kit,project-id-cmds-state",
			&disp_info->project_id.cmds);

		disp_info->project_id.default_project_id = (char*)of_get_property(np,
			"lcd-kit,default-project-id", NULL);
	}

	/* oem information */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,oem-info-support",
		&disp_info->oeminfo.support, 0);

	if (disp_info->oeminfo.support) {
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,oem-barcode-2d-support",
			&disp_info->oeminfo.barcode_2d.support, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,oem-barcode-2d-num-offset",
			&disp_info->oeminfo.barcode_2d.number_offset, 0);
		if (disp_info->oeminfo.barcode_2d.support) {
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,barcode-2d-cmds",
				"lcd-kit,barcode-2d-cmds-state",
				&disp_info->oeminfo.barcode_2d.cmds);
			disp_info->oeminfo.barcode_2d.flags = false;
		}
	}
}

void lcd_kit_parse_effect(struct device_node *np)
{
	/* rgbw */
	lcd_kit_parse_u32(np, "lcd-kit,rgbw-support",
		&disp_info->rgbw.support, 0);
	if (disp_info->rgbw.support) {
		lcd_kit_parse_u32(np, "lcd-kit,rgbw-bl-max",
			&disp_info->rgbw.rgbw_bl_max, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode1-cmds",
			"lcd-kit,rgbw-mode1-cmds-state",
			&disp_info->rgbw.mode1_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode2-cmds",
			"lcd-kit,rgbw-mode2-cmds-state",
			&disp_info->rgbw.mode2_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode3-cmds",
			"lcd-kit,rgbw-mode3-cmds-state",
			&disp_info->rgbw.mode3_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode4-cmds",
			"lcd-kit,rgbw-mode4-cmds-state",
			&disp_info->rgbw.mode4_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-backlight-cmds",
			"lcd-kit,rgbw-backlight-cmds-state",
			&disp_info->rgbw.backlight_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-pixel-gain-limit-cmds",
			"lcd-kit,rgbw-pixel-gain-limit-cmds-state",
			&disp_info->rgbw.pixel_gain_limit_cmds);
	}
}

void lcd_kit_parse_alpm(struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-enter-aod-cmds",
		"lcd-kit,panel-enter-aod-cmds-state",
		&disp_info->alpm.off_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-exit-aod-cmds",
		"lcd-kit,panel-exit-aod-cmds-state",
		&disp_info->alpm.exit_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-aod-high-brightness-cmds",
		"lcd-kit,panel-aod-high-brightness-cmds-state",
		&disp_info->alpm.high_light_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-aod-low-brightness-cmds",
		"lcd-kit,panel-aod-low-brightness-cmds-state",
		&disp_info->alpm.low_light_cmds);
}

void lcd_kit_parse_util(struct device_node *np)
{
	if (np == NULL) {
		LCD_KIT_ERR("np is null\n");
		return;
	}
	/* quickly sleep out */
	lcd_kit_parse_u32(np, "lcd-kit,quickly-sleep-out-support",
		&disp_info->quickly_sleep_out.support, 0);
	if (disp_info->quickly_sleep_out.support)
		lcd_kit_parse_u32(np,
			"lcd-kit,quickly-sleep-out-interval",
			&disp_info->quickly_sleep_out.interval, 0);

	/* alpm */
	lcd_kit_parse_u32(np, "lcd-kit,ap-aod-support",
		&disp_info->alpm.support, 0);
	if (disp_info->alpm.support)
		lcd_kit_parse_alpm(np);
	/* fps */
	lcd_kit_parse_u32(np, "lcd-kit,fps-support",
		&disp_info->fps.support, 0);
	/* default fps */
	lcd_kit_parse_u32(np, "lcd-kit,default-fps",
		&disp_info->fps.default_fps, 0);
	if (disp_info->fps.support) {
		disp_info->fps.panel_support_fps_list.cnt = SEQ_NUM;
		lcd_kit_parse_array_data(np, "lcd-kit,panel-support-fps-list",
			&disp_info->fps.panel_support_fps_list);
	}
}

void lcd_kit_parse_dt(struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return;
	}
	/* parse effect info */
	lcd_kit_parse_effect(np);
	/* parse normal function */
	lcd_kit_parse_util(np);
}

int lcd_kit_get_bl_max_nit_from_dts(void)
{
	int ret;
	struct device_node *np = NULL;

	if (common_info->blmaxnit.get_blmaxnit_type == GET_BLMAXNIT_FROM_DDIC) {
		np = of_find_compatible_node(NULL, NULL,
			DTS_COMP_LCD_KIT_PANEL_TYPE);
		if (!np) {
			LCD_KIT_ERR("NOT FOUND device node %s!\n",
				DTS_COMP_LCD_KIT_PANEL_TYPE);
			ret = -1;
			return ret;
		}
		OF_PROPERTY_READ_U32_RETURN(np, "panel_ddic_max_brightness",
			&common_info->actual_bl_max_nit);
		LCD_KIT_INFO("max nit is %d\n", common_info->actual_bl_max_nit);
	}
	return LCD_KIT_OK;
}

static int lcd_kit_get_project_id(char *buff)
{
	if (buff == NULL) {
		LCD_KIT_ERR("buff is null\n");
		return LCD_KIT_FAIL;
	}

	if (!disp_info) {
		LCD_KIT_ERR("disp_info is null\n");
		return LCD_KIT_FAIL;
	}

	/* use read project id */
	if (disp_info->project_id.support &&
		(strlen(disp_info->project_id.id) > 0)) {
		strncpy(buff, disp_info->project_id.id,
			strlen(disp_info->project_id.id));
		LCD_KIT_INFO("use read project id is %s\n",
			disp_info->project_id.id);
		return LCD_KIT_OK;
	}

	/* use default project id */
	if (disp_info->project_id.support &&
		disp_info->project_id.default_project_id) {
		strncpy(buff, disp_info->project_id.default_project_id,
			PROJECTID_LEN);
		LCD_KIT_INFO("use default project id:%s\n",
			disp_info->project_id.default_project_id);
		return LCD_KIT_OK;
	}

	LCD_KIT_ERR("not support get project id\n");
	return LCD_KIT_FAIL;
}

int lcd_kit_get_online_status(void)
{
	int status = LCD_ONLINE;

	if (!strncmp(disp_info->compatible, LCD_KIT_DEFAULT_PANEL,
		strlen(disp_info->compatible)))
		/* panel is online */
		status = LCD_OFFLINE;
	LCD_KIT_INFO("status = %d\n", status);
	return status;
}

static int lcd_get_2d_barcode(char *buff)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct qcom_panel_info *panel_info = NULL;

	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		LCD_KIT_ERR("panel_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_rx == NULL) {
		LCD_KIT_ERR("mipi_rx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!buff) {
		LCD_KIT_ERR("buff is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (!disp_info) {
		LCD_KIT_ERR("disp_info is null\n");
		return LCD_KIT_FAIL;
	}

	if (!disp_info->oeminfo.support) {
		LCD_KIT_ERR("oem info is not support\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_rx(panel_info->display, buff,
		LCD_KIT_SN_CODE_LENGTH, &disp_info->oeminfo.barcode_2d.cmds);
	if (ret) {
		LCD_KIT_ERR("get 2d barcode fail\n");
		return LCD_KIT_FAIL;
	} else {
		LCD_KIT_INFO("get 2d barcode success! %s\n", buff);
		return LCD_KIT_OK;
	}
}

int lcd_kit_rgbw_set_mode(struct dsi_display *display, int mode)
{
	int ret = LCD_KIT_OK;
	static int old_rgbw_mode;
	int rgbw_mode;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (display == NULL) {
		LCD_KIT_INFO("display is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info == NULL) {
		LCD_KIT_ERR("disp_info is null\n");
		return LCD_KIT_FAIL;
	}
	rgbw_mode = disp_info->ddic_rgbw_param.ddic_rgbw_mode;
	if (rgbw_mode != old_rgbw_mode) {
		switch (mode) {
		case RGBW_SET1_MODE:
			ret = adapt_ops->mipi_tx(display, &disp_info->rgbw.mode1_cmds);
			break;
		case RGBW_SET2_MODE:
			ret = adapt_ops->mipi_tx(display, &disp_info->rgbw.mode2_cmds);
			break;
		case RGBW_SET3_MODE:
			ret = adapt_ops->mipi_tx(display, &disp_info->rgbw.mode3_cmds);
			break;
		case RGBW_SET4_MODE:
			ret = adapt_ops->mipi_tx(display, &disp_info->rgbw.mode4_cmds);
			break;
		default:
			LCD_KIT_ERR("mode err: %d\n", disp_info->ddic_rgbw_param.ddic_rgbw_mode);
			ret = LCD_KIT_FAIL;
			break;
		}
	}
	LCD_KIT_DEBUG("rgbw_mode=%d,rgbw_mode_old=%d!\n", rgbw_mode, old_rgbw_mode);
	old_rgbw_mode = rgbw_mode;
	return ret;
}

int lcd_kit_rgbw_set_backlight(struct dsi_display *display, int bl_level)
{
	int ret;
	unsigned int level;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (display == NULL) {
		LCD_KIT_INFO("display is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (bl_level < 0)
		bl_level = 0;
	level = (unsigned int)bl_level;
	/* change bl level to dsi cmds */
	disp_info->rgbw.backlight_cmds.cmds[0].payload[1] = (level >> 8) & 0xff;
	disp_info->rgbw.backlight_cmds.cmds[0].payload[2] = level & 0xff;
	ret = adapt_ops->mipi_tx(display, &disp_info->rgbw.backlight_cmds);
	return ret;
}

static int lcd_kit_rgbw_pix_gain(struct dsi_display *display)
{
	unsigned int pix_gain;
	static unsigned int pix_gain_old;
	int rgbw_mode;
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (display == NULL) {
		LCD_KIT_INFO("display is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->rgbw.pixel_gain_limit_cmds.cmds == NULL) {
		LCD_KIT_INFO("RGBW not support pixel_gain_limit\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	rgbw_mode = disp_info->ddic_rgbw_param.ddic_rgbw_mode;
	pix_gain = (unsigned int)disp_info->ddic_rgbw_param.pixel_gain_limit;
	if ((pix_gain != pix_gain_old) && (rgbw_mode == RGBW_SET4_MODE)) {
		disp_info->rgbw.pixel_gain_limit_cmds.cmds[0].payload[1] = pix_gain;
		ret = adapt_ops->mipi_tx(display, &disp_info->rgbw.pixel_gain_limit_cmds);
		LCD_KIT_DEBUG("RGBW pixel_gain=%u,pix_gain_old=%u\n",
			pix_gain, pix_gain_old);
		pix_gain_old = pix_gain;
	}
	return ret;
}

int lcd_kit_rgbw_set_handle(void)
{
	int ret;
	static int old_rgbw_backlight;
	int rgbw_backlight;
	int rgbw_bl_level;
	struct qcom_panel_info *panel_info = NULL;

	if (disp_info == NULL) {
		LCD_KIT_ERR("disp_info is null\n");
		return LCD_KIT_FAIL;
	}
	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		LCD_KIT_ERR("panel_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* set mode */
	ret = lcd_kit_rgbw_set_mode(panel_info->display,
		disp_info->ddic_rgbw_param.ddic_rgbw_mode);
	if (ret < 0) {
		LCD_KIT_ERR("RGBW set mode fail\n");
		return LCD_KIT_FAIL;
	}

	/* set backlight */
	rgbw_backlight = disp_info->ddic_rgbw_param.ddic_rgbw_backlight;
	if (disp_info->rgbw.backlight_cmds.cmds &&
		(panel_info->bl_current != 0) &&
		(rgbw_backlight != old_rgbw_backlight)) {
		rgbw_bl_level = rgbw_backlight * disp_info->rgbw.rgbw_bl_max /
			panel_info->bl_max;
		ret = lcd_kit_rgbw_set_backlight(panel_info->display, rgbw_bl_level);
		if (ret < 0) {
			LCD_KIT_ERR("RGBW set backlight fail\n");
			return LCD_KIT_FAIL;
		}
	}
	old_rgbw_backlight = rgbw_backlight;

	/* set gain */
	ret = lcd_kit_rgbw_pix_gain(panel_info->display);
	if (ret) {
		LCD_KIT_INFO("RGBW set pix_gain fail\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

int lcd_kit_get_status_by_type(int type, int *status)
{
	int ret;

	if (status == NULL) {
		LCD_KIT_ERR("status is null\n");
		return LCD_KIT_FAIL;
	}
	switch (type) {
	case LCD_ONLINE_TYPE:
		*status = lcd_kit_get_online_status();
		ret = LCD_KIT_OK;
		break;
	case PT_STATION_TYPE:
#ifdef LCD_FACTORY_MODE
		*status = lcd_kit_get_pt_station_status();
#endif
		ret = LCD_KIT_OK;
		break;
	default:
		LCD_KIT_ERR("not support type\n");
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}

void lcd_kit_set_bl_cmd(uint32_t level)
{
	if (common_info->backlight.order != BL_BIG_ENDIAN &&
		common_info->backlight.order != BL_LITTLE_ENDIAN) {
		LCD_KIT_ERR("not support order\n");
		return;
	}
	if (common_info->backlight.order == BL_BIG_ENDIAN) {
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
			return;
		}
		/* change bl level to dsi cmds */
		common_info->backlight.bl_cmd.cmds[0].payload[1] =
			(level >> 8) & 0xFF;
		common_info->backlight.bl_cmd.cmds[0].payload[2] = level & 0xFF;
		return;
	}
	if (common_info->backlight.bl_max <= 0xFF) {
		common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
		return;
	}
	/* change bl level to dsi cmds */
	common_info->backlight.bl_cmd.cmds[0].payload[1] = level & 0xFF;
	common_info->backlight.bl_cmd.cmds[0].payload[2] = (level >> 8) & 0xFF;
}

static int set_mipi_bl_level(struct hbm_type_cfg hbm_source, uint32_t level)
{
	lcd_kit_set_bl_cmd(level);

	return LCD_KIT_OK;
}

int lcd_kit_mipi_set_backlight(struct hbm_type_cfg hbm_source, uint32_t level)
{
	if (set_mipi_bl_level(hbm_source, level) == LCD_KIT_FAIL)
		return LCD_KIT_FAIL;
	return LCD_KIT_OK;
}

static int lcd_create_sysfs(struct kobject *obj)
{
	int rc = LCD_KIT_FAIL;

	if (obj == NULL) {
		LCD_KIT_ERR("create sysfs fail, obj is NULL\n");
		return rc;
	}

	rc = lcd_kit_create_sysfs(obj);
	if (rc) {
		LCD_KIT_ERR("create sysfs fail\n");
		return rc;
	}
#ifdef LCD_FACTORY_MODE
	rc = lcd_create_fact_sysfs(obj);
	if (rc) {
		LCD_KIT_ERR("create fact sysfs fail\n");
		return rc;
	}
#endif
	return rc;
}

static int lcd_kit_proximity_power_off(void)
{
	LCD_KIT_INFO("[Proximity_feature] lcd_kit_proximity_power_off enter!\n");
	if (!common_info->thp_proximity.support) {
		LCD_KIT_INFO("[Proximity_feature] thp_proximity not support exit!\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_kit_get_pt_mode()) {
		LCD_KIT_INFO("[Proximity_feature] pt test mode exit!\n");
		return LCD_KIT_FAIL;
	}
	down(&disp_info->thp_second_poweroff_sem);
	if (common_info->thp_proximity.panel_power_state == POWER_ON) {
		LCD_KIT_INFO("[Proximity_feature] power state is on exit!\n");
		up(&disp_info->thp_second_poweroff_sem);
		return LCD_KIT_FAIL;
	}
	if (common_info->thp_proximity.panel_power_state == POWER_TS_SUSPEND) {
		LCD_KIT_INFO("[Proximity_feature] power off suspend state exit!\n");
		up(&disp_info->thp_second_poweroff_sem);
		return LCD_KIT_OK;
	}
	if (common_info->thp_proximity.work_status == TP_PROXMITY_DISABLE) {
		LCD_KIT_INFO("[Proximity_feature] thp_proximity has been disabled exit!\n");
		up(&disp_info->thp_second_poweroff_sem);
		return LCD_KIT_FAIL;
	}
	common_info->thp_proximity.work_status = TP_PROXMITY_DISABLE;
	if (common_ops->panel_only_power_off)
		common_ops->panel_only_power_off(NULL);
	up(&disp_info->thp_second_poweroff_sem);
	LCD_KIT_INFO("[Proximity_feature] lcd_kit_proximity_power_off exit!\n");
	return LCD_KIT_OK;
}

static int lcd_kit_get_sn_code(void)
{
	int ret;
	struct qcom_panel_info *pinfo = NULL;
	struct lcd_kit_panel_ops *panel_ops = NULL;
	char read_value[OEM_INFO_SIZE_MAX + 1] = {0};
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_rx == NULL) {
		LCD_KIT_ERR("mipi_rx is NULL\n");
		return LCD_KIT_FAIL;
	}
	pinfo = lcm_get_panel_info();
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	if (common_info && common_info->sn_code.support) {
		panel_ops = lcd_kit_panel_get_ops();
		if (panel_ops && panel_ops->lcd_get_2d_barcode) {
			ret = panel_ops->lcd_get_2d_barcode(read_value);
			if (ret != 0) {
				LCD_KIT_ERR("get sn_code error!\n");
				return LCD_KIT_FAIL;
			}
			memcpy(pinfo->sn_code, read_value + 2, LCD_KIT_SN_CODE_LENGTH);
			pinfo->sn_code_length = LCD_KIT_SN_CODE_LENGTH;
			return LCD_KIT_OK;
		}
		if (disp_info && disp_info->oeminfo.barcode_2d.support) {
			ret = adapt_ops->mipi_rx(pinfo->display, read_value,
				LCD_KIT_SN_CODE_LENGTH,
				&disp_info->oeminfo.barcode_2d.cmds);
			if (ret != 0) {
				LCD_KIT_ERR("get sn_code error!\n");
				return LCD_KIT_FAIL;
			}
			memcpy(pinfo->sn_code, read_value, LCD_KIT_SN_CODE_LENGTH);
			pinfo->sn_code_length = LCD_KIT_SN_CODE_LENGTH;
			return LCD_KIT_OK;
		}
	}
	return LCD_KIT_OK;
}

int lcd_panel_sncode_store(void)
{
	int ret;
	if (disp_info->oeminfo.barcode_2d.flags == true) {
		LCD_KIT_ERR("panel_sncode got it!\n");
		return LCD_KIT_OK;
	}

	/* get sn code */
	ret = lcd_kit_get_sn_code();
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("get sn code failed!\n");
		return LCD_KIT_FAIL;
	}
	disp_info->oeminfo.barcode_2d.flags = true;
	return LCD_KIT_OK;
}

struct lcd_kit_ops g_lcd_ops = {
	.lcd_kit_support = lcd_kit_support,
	.get_project_id = lcd_kit_get_project_id,
	.create_sysfs = lcd_create_sysfs,
	.read_project_id = lcd_kit_read_project_id,
	.get_2d_barcode = lcd_get_2d_barcode,
	.get_status_by_type = lcd_kit_get_status_by_type,
	.proximity_power_off = lcd_kit_proximity_power_off,
#ifdef LCD_FACTORY_MODE
	.get_pt_station_status = lcd_kit_get_pt_station_status,
#endif
	.get_sn_code = lcd_kit_get_sn_code,
};

#ifdef CONFIG_APP_INFO
static void lcd_kit_app_info_set()
{
	int ret;
	static const char *panel_model = NULL;
	static const char *info_node = "lcd type";

	panel_model = common_info->panel_name;
	if (panel_model == NULL) {
		LCD_KIT_ERR("panel_name is NULL\n");
		panel_model = "default lcd";
	} else {
		if (!strcmp(panel_model, LCDKIT_DEFAULT_PANEL_NAME))
			panel_model = "default lcd";
	}
	LCD_KIT_INFO("Panel Name = %s\n", panel_model);
	ret = app_info_set(info_node, panel_model);
	if (ret)
		LCD_KIT_ERR("app_info_set:panel dt parse failed\n");
}
#endif

int lcd_kit_utils_init(struct device_node *np, struct qcom_panel_info *pinfo)
{
	/* init sem */
	sema_init(&disp_info->lcd_kit_sem, 1);
	sema_init(&disp_info->thp_second_poweroff_sem, 1);
	/* init mipi lock */
	mutex_init(&disp_info->mipi_lock);
	/* parse display dts */
	lcd_kit_parse_dt(np);
#ifdef LCD_FACTORY_MODE
	lcd_kit_fact_init(np);
#endif
	/*init qcom pinfo*/
	lcd_kit_pinfo_init(np, pinfo);
	lcd_kit_ops_register(&g_lcd_ops);
#ifdef CONFIG_APP_INFO
	lcd_kit_app_info_set();
#endif
	return LCD_KIT_OK;
}
