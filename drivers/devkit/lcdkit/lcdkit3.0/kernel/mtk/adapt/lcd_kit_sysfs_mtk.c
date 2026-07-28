/* Copyright (c) 2017-2018, Huawei terminal Tech. Co., Ltd. All rights reserved.
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
#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_sysfs_mtk.h"
#include "lcm_drv.h"
#include <linux/kernel.h>
#include "lcm_drv.h"
/* marco define*/
#ifndef strict_strtoul
#define strict_strtoul kstrtoul
#endif

#ifndef strict_strtol
#define strict_strtol kstrtol
#endif

extern struct LCM_DRIVER lcdkit_mtk_common_panel;
/*extern declare*/
extern int hisi_adc_get_value(int adc_channel);

static ssize_t lcd_model_show(struct device* dev,
										struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_panel_name) {
		ret = common_ops->get_panel_name(buf);
	}
	return ret;
}

static ssize_t lcd_panel_info_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_panel_info) {
		ret = common_ops->get_panel_info(buf);
	}
	return ret;
}

static ssize_t lcd_fps_scence_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	ssize_t ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_fps_scence_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_alpm_function_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	ssize_t ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_alpm_function_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_alpm_setting_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_inversion_mode_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	ssize_t ret = LCD_KIT_OK;

	if (common_ops->inversion_get_mode) {
		ret = common_ops->inversion_get_mode(buf);
	}
	return ret;
}

static ssize_t lcd_inversion_mode_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_scan_mode_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->scan_get_mode) {
		ret = common_ops->scan_get_mode(buf);
	}
	return ret;
}

static ssize_t lcd_scan_mode_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_check_reg_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_gram_check_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_gram_check_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_sleep_ctrl_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_sleep_mode) {
		ret = common_ops->get_sleep_mode(buf);
	}
	return ret;
}

static ssize_t lcd_sleep_ctrl_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	int ret = LCD_KIT_OK;
	unsigned long val = 0;
    struct mtk_panel_info *pinfo = (struct mtk_panel_info *)lcdkit_mtk_common_panel.panel_info;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}
	if (!pinfo->panel_state) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}
    
	if (common_ops->get_sleep_mode) {
		ret = common_ops->set_sleep_mode(val);
	}	
    return count;
}

static ssize_t lcd_hkadc_debug_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_hkadc_debug_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_amoled_acl_ctrl_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops->get_acl_mode) {
		ret = common_ops->get_acl_mode(buf);
	}
	return ret;
}

static ssize_t lcd_amoled_acl_ctrl_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_amoled_vr_mode_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	return 0;
}

static ssize_t lcd_amoled_vr_mode_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_effect_color_mode_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_effect_color_mode_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_test_config_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;

	return snprintf(buf, PAGE_SIZE, "%d", ret);
}

static ssize_t lcd_test_config_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_reg_read_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_reg_read_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_gamma_dynamic_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_frame_count_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
return 0;
}

static ssize_t lcd_frame_update_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
    return 0;

}

static ssize_t lcd_frame_update_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
	return count;
}

static ssize_t lcd_mipi_clk_upt_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
 	ssize_t ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_mipi_clk_upt_store(struct device* dev, struct device_attribute* attr,
		const char* buf, size_t count)
{
 	ssize_t ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_func_switch_show(struct device* dev,
										struct device_attribute* attr, char* buf)
{
 	ssize_t ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_func_switch_store(struct device* dev,
		struct device_attribute* attr, const char* buf, size_t count)
{
 	return count;
}

static ssize_t lcd_lv_detect_show(struct device* dev,
									   struct device_attribute* attr, char* buf)
{
    return 0;
}

static ssize_t lcd_current_detect_show(struct device* dev,
		struct device_attribute* attr, char* buf)
{
	int ret = LCD_KIT_OK;
	if (!buf) {
		LCD_KIT_ERR("buf is null point!\n");
		return LCD_KIT_FAIL;
	}
	return snprintf(buf, PAGE_SIZE, "%d", ret); 
}



static int lcd_check_support(int index)
{
	switch (index) {
		case LCD_MODEL_INDEX:
			return SYSFS_SUPPORT;
		case PANEL_INFO_INDEX:
			return SYSFS_SUPPORT;
		case INVERSION_INDEX:
			return common_info->inversion.support;
		case SCAN_INDEX:
			return common_info->scan.support;
		case CHECK_REG_INDEX:
			return common_info->check_reg.support;
		case CHECKSUM_INDEX:
			return disp_info->checksum.support;
		case SLEEP_CTRL_INDEX:
			return common_info->pt.support;
		case HKADC_INDEX:
			return disp_info->hkadc.support;
		case VOLTAGE_ENABLE_INDEX:
			return SYSFS_NOT_SUPPORT;
		case PCD_ERRFLAG_INDEX:
			return disp_info->pcd_errflag_check_support;
		case ACL_INDEX:
			return common_info->acl.support;
		case VR_INDEX:
			return common_info->vr.support;
		case SUPPORT_MODE_INDEX:
			return common_info->effect_color.support;
		case GAMMA_DYNAMIC_INDEX:
			return disp_info->gamma_cal.support;
		case FRAME_COUNT_INDEX:
			return disp_info->vr_support;
		case FRAME_UPDATE_INDEX:
			return disp_info->vr_support;
		case MIPI_DSI_CLK_UPT_INDEX:
			//return hisifd->panel_info.dsi_bit_clk_upt_support;
		case FPS_SCENCE_INDEX:
			return disp_info->fps.support;
		case ALPM_FUNCTION_INDEX:
			return disp_info->alpm.support;
		case ALPM_SETTING_INDEX:
			return disp_info->alpm.support;
		case FUNC_SWITCH_INDEX:
			return SYSFS_SUPPORT;
		case TEST_CONFIG_INDEX:
			return SYSFS_SUPPORT;
		case LV_DETECT_INDEX:
			return disp_info->lv_det.support;
		case CURRENT_DETECT_INDEX:
			return disp_info->current_det.support;
		case REG_READ_INDEX:
			return disp_info->gamma_cal.support;
		case DDIC_OEM_INDEX:
			return disp_info->oeminfo.support;
		case BL_MODE_INDEX:
			return SYSFS_NOT_SUPPORT;
		case BL_SUPPORT_MODE_INDEX:
			return SYSFS_NOT_SUPPORT;
		case LDO_CHECK_INDEX:
			return disp_info->ldo_check.support;
		default:
			return SYSFS_NOT_SUPPORT;
	}
}

struct lcd_kit_sysfs_ops g_lcd_sysfs_ops = {
	.check_support = lcd_check_support,
	.model_show = lcd_model_show,
	.panel_info_show = lcd_panel_info_show,
	.inversion_mode_show = lcd_inversion_mode_show,
	.inversion_mode_store = lcd_inversion_mode_store,
	.scan_mode_show = lcd_scan_mode_show,
	.scan_mode_store = lcd_scan_mode_store,
	.check_reg_show = lcd_check_reg_show,
	.gram_check_show = lcd_gram_check_show,
	.gram_check_store = lcd_gram_check_store,
	.sleep_ctrl_show = lcd_sleep_ctrl_show,
	.sleep_ctrl_store = lcd_sleep_ctrl_store,
	.hkadc_debug_show = lcd_hkadc_debug_show,
	.hkadc_debug_store = lcd_hkadc_debug_store,
	.amoled_acl_ctrl_show = lcd_amoled_acl_ctrl_show,
	.amoled_acl_ctrl_store = lcd_amoled_acl_ctrl_store,
	.amoled_vr_mode_show = lcd_amoled_vr_mode_show,
	.amoled_vr_mode_store = lcd_amoled_vr_mode_store,
	.effect_color_mode_show = lcd_effect_color_mode_show,
	.effect_color_mode_store = lcd_effect_color_mode_store,
	.test_config_show = lcd_test_config_show,
	.test_config_store = lcd_test_config_store,
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
	.alpm_function_show = lcd_alpm_function_show,
	.alpm_function_store = lcd_alpm_function_store,
	.alpm_setting_store = lcd_alpm_setting_store,
	.func_switch_show = lcd_func_switch_show,
	.func_switch_store = lcd_func_switch_store,
	.lv_detect_show = lcd_lv_detect_show,
	.current_detect_show = lcd_current_detect_show,
};

int lcd_kit_sysfs_init(void)
{
	lcd_kit_sysfs_ops_register(&g_lcd_sysfs_ops);
	return LCD_KIT_OK;
}
