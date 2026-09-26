/*
 * lcd_kit_sysfs.c
 *
 * lcdkit sysfs function for lcdkit
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

#include "lcd_kit_sysfs.h"
#include "lcd_kit_dbg.h"

static ssize_t lcd_kit_model_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.model_lock);
	if (sysfs_ops->model_show)
		ret = sysfs_ops->model_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.model_lock);
	return ret;
}

static ssize_t lcd_kit_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.type_lock);
	if (sysfs_ops->type_show)
		ret = sysfs_ops->type_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.type_lock);
	return ret;
}

static ssize_t lcd_kit_panel_info_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.panel_info_lock);
	if (sysfs_ops->panel_info_show)
		ret = sysfs_ops->panel_info_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.panel_info_lock);
	return ret;
}

static ssize_t	lcd_kit_lcd_voltage_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.vol_enable_lock);
	if (sysfs_ops->voltage_enable_store)
		sysfs_ops->voltage_enable_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.vol_enable_lock);
	return count;
}

static ssize_t lcd_kit_amoled_acl_ctrl_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.amoled_acl_lock);
	if (sysfs_ops->amoled_acl_ctrl_show)
		ret = sysfs_ops->amoled_acl_ctrl_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.amoled_acl_lock);
	return ret;
}

static ssize_t lcd_kit_amoled_acl_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.amoled_acl_lock);
	if (sysfs_ops->amoled_acl_ctrl_store)
		sysfs_ops->amoled_acl_ctrl_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.amoled_acl_lock);
	return count;
}

static ssize_t lcd_kit_amoled_vr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.amoled_vr_lock);
	if (sysfs_ops->amoled_vr_mode_show)
		ret = sysfs_ops->amoled_vr_mode_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.amoled_vr_lock);
	return ret;
}

static ssize_t lcd_kit_amoled_vr_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.amoled_vr_lock);
	if (sysfs_ops->amoled_vr_mode_store)
		sysfs_ops->amoled_vr_mode_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.amoled_vr_lock);
	return count;
}

static ssize_t lcd_kit_support_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.support_mode_lock);
	if (sysfs_ops->effect_color_mode_show)
		ret = sysfs_ops->effect_color_mode_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.support_mode_lock);
	return ret;
}

static ssize_t lcd_kit_support_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.support_mode_lock);
	if (sysfs_ops->effect_color_mode_store)
		sysfs_ops->effect_color_mode_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.support_mode_lock);
	return count;
}

static ssize_t lcd_kit_gamma_dynamic_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.gamma_dynamic_lock);
	if (sysfs_ops->gamma_dynamic_store)
		sysfs_ops->gamma_dynamic_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.gamma_dynamic_lock);
	return count;
}

static ssize_t lcd_kit_frame_count_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.frame_count_lock);
	if (sysfs_ops->frame_count_show)
		ret = sysfs_ops->frame_count_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.frame_count_lock);
	return ret;
}

static ssize_t lcd_kit_frame_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.frame_update_lock);
	if (sysfs_ops->frame_update_show)
		ret = sysfs_ops->frame_update_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.frame_update_lock);
	return ret;
}

static ssize_t	lcd_kit_frame_update_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.frame_update_lock);
	if (sysfs_ops->frame_update_store)
		sysfs_ops->frame_update_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.frame_update_lock);
	return count;
}

static ssize_t lcd_kit_mipi_dsi_clk_upt_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.mipi_dsi_clk_lock);
	if (sysfs_ops->mipi_dsi_clk_upt_show)
		ret = sysfs_ops->mipi_dsi_clk_upt_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.mipi_dsi_clk_lock);
	return ret;
}

static ssize_t lcd_kit_mipi_dsi_clk_upt_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.mipi_dsi_clk_lock);
	if (sysfs_ops->mipi_dsi_clk_upt_store)
		sysfs_ops->mipi_dsi_clk_upt_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.mipi_dsi_clk_lock);
	return count;
}

static ssize_t lcd_kit_fps_scence_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (sysfs_ops->fps_scence_show)
		ret = sysfs_ops->fps_scence_show(dev, attr, buf);
	return ret;
}

static ssize_t lcd_kit_fps_order_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.fps_order_lock);
	if (sysfs_ops->fps_order_show)
		ret = sysfs_ops->fps_order_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.fps_order_lock);
	return ret;
}

static ssize_t lcd_kit_fps_scence_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (sysfs_ops->fps_scence_store)
		sysfs_ops->fps_scence_store(dev, attr, buf, count);
	return count;
}

static ssize_t lcd_kit_alpm_function_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.alpm_function_lock);
	if (sysfs_ops->alpm_function_show)
		ret = sysfs_ops->alpm_function_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.alpm_function_lock);
	return ret;
}

static ssize_t lcd_kit_alpm_function_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.alpm_function_lock);
	if (sysfs_ops->alpm_function_store)
		sysfs_ops->alpm_function_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.alpm_function_lock);
	return count;
}

static ssize_t lcd_kit_alpm_setting_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = lcd_kit_get_sysfs_ops();

	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}

	mutex_lock(&COMMON_LOCK->sysfs_lock.alpm_setting_lock);
	if (sysfs_ops->alpm_function_show)
		ret = sysfs_ops->alpm_setting_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.alpm_setting_lock);

	return ret;
}

static ssize_t lcd_kit_alpm_setting_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = lcd_kit_get_sysfs_ops();

	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.alpm_setting_lock);
	if (sysfs_ops->alpm_setting_store)
		count = sysfs_ops->alpm_setting_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.alpm_setting_lock);
	return count;
}

static ssize_t lcd_kit_ddic_alpha_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.ddic_alpha_lock);
	if (sysfs_ops->ddic_local_hbm_store)
		sysfs_ops->ddic_local_hbm_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.ddic_alpha_lock);
	return count;
}

static ssize_t lcd_kit_func_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.func_switch_lock);
	if (sysfs_ops->func_switch_show)
		ret = sysfs_ops->func_switch_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.func_switch_lock);
	return ret;
}

static ssize_t lcd_kit_func_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.func_switch_lock);
	if (sysfs_ops->func_switch_store)
		sysfs_ops->func_switch_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.func_switch_lock);
	return count;
}

static ssize_t lcd_kit_reg_read_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.reg_read_lock);
	if (sysfs_ops->reg_read_show)
		ret = sysfs_ops->reg_read_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.reg_read_lock);
	return ret;
}
static ssize_t lcd_kit_reg_read_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.reg_read_lock);
	if (sysfs_ops->reg_read_store)
		sysfs_ops->reg_read_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.reg_read_lock);
	return count;
}

static ssize_t lcd_kit_ddic_oem_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.ddic_oem_info_lock);
	if (sysfs_ops->ddic_oem_info_show)
		ret = sysfs_ops->ddic_oem_info_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.ddic_oem_info_lock);
	return ret;
}

static ssize_t lcd_kit_ddic_oem_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.ddic_oem_info_lock);
	if (sysfs_ops->ddic_oem_info_store)
		sysfs_ops->ddic_oem_info_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.ddic_oem_info_lock);
	return count;
}

static ssize_t lcd_kit_bl_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.bl_mode_lock);
	if (sysfs_ops->bl_mode_store)
		sysfs_ops->bl_mode_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.bl_mode_lock);
	return count;
}

static ssize_t lcd_kit_bl_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.bl_mode_lock);
	if (sysfs_ops->bl_mode_show)
		ret = sysfs_ops->bl_mode_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.bl_mode_lock);
	return ret;
}

static ssize_t lcd_kit_support_bl_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.support_bl_mode_lock);
	if (sysfs_ops->support_bl_mode_store)
		sysfs_ops->support_bl_mode_store(dev, attr, buf, count);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.support_bl_mode_lock);
	return count;
}

static ssize_t lcd_kit_support_bl_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.support_bl_mode_lock);
	if (sysfs_ops->support_bl_mode_show)
		ret = sysfs_ops->support_bl_mode_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.support_bl_mode_lock);
	return ret;
}

static ssize_t lcd_kit_effect_bl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (mutex_trylock(&COMMON_LOCK->sysfs_lock.effect_bl_mode_lock)) {
		if (sysfs_ops->effect_bl_show)
			ret = sysfs_ops->effect_bl_show(dev, attr, buf);
		mutex_unlock(&COMMON_LOCK->sysfs_lock.effect_bl_mode_lock);
		return ret;
	} else {
		// Try lock failed
		LCD_KIT_INFO("try lock failed\n");
		return LCD_KIT_OK;
	}
}

static ssize_t lcd_kit_effect_bl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (mutex_trylock(&COMMON_LOCK->sysfs_lock.effect_bl_mode_lock)) {
		if (sysfs_ops->effect_bl_store)
			ret = sysfs_ops->effect_bl_store(dev, attr, buf, count);
		mutex_unlock(&COMMON_LOCK->sysfs_lock.effect_bl_mode_lock);
		return ret;
	} else {
		// Try lock failed
		// This delta has been skipped, and hal may resent the command.
		LCD_KIT_INFO("try lock failed\n");
		if (buf != NULL)
			LCD_KIT_INFO("buf=%s\n", buf);
		return 0;
	}
}

static ssize_t lcd_kit_ddic_lv_detect_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (sysfs_ops == NULL) {
		LCD_KIT_ERR("sysfs_ops is NULL\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.ddic_lv_detect_lock);
	if (sysfs_ops->ddic_lv_detect_test_show)
		ret = sysfs_ops->ddic_lv_detect_test_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.ddic_lv_detect_lock);
	return ret;
}

static ssize_t lcd_kit_ddic_lv_detect_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (sysfs_ops == NULL) {
		LCD_KIT_ERR("sysfs_ops is NULL\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.ddic_lv_detect_lock);
	if (sysfs_ops->ddic_lv_detect_test_store) {
		ret = sysfs_ops->ddic_lv_detect_test_store(dev, attr,
			buf, count);
		if (ret)
			LCD_KIT_ERR("ddic lv detect test store error\n");
	}
	mutex_unlock(&COMMON_LOCK->sysfs_lock.ddic_lv_detect_lock);
	return count;
}

static ssize_t lcd_kit_hbm_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (!sysfs_ops->hbm_mode_show) {
		LCD_KIT_ERR("hbm_mode_show is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.hbm_mode_lock);
	ret = sysfs_ops->hbm_mode_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.hbm_mode_lock);
	return ret;
}

static ssize_t lcd_kit_hbm_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}

	if (!sysfs_ops->hbm_mode_store) {
		LCD_KIT_ERR("hbm_mode_store is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.hbm_mode_lock);
	if (sysfs_ops->hbm_mode_store(dev, attr, buf, count)) {
		LCD_KIT_ERR("hbm_mode_store error!\n");
		mutex_unlock(&COMMON_LOCK->sysfs_lock.hbm_mode_lock);
		return LCD_KIT_FAIL;
	}
	mutex_unlock(&COMMON_LOCK->sysfs_lock.hbm_mode_lock);
	return count;
}

static ssize_t lcd_kit_panel_sncode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (sysfs_ops == NULL) {
		LCD_KIT_ERR("sysfs_ops is NULL\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.panel_sn_code_lock);
	if (sysfs_ops->panel_sncode_show)
		ret = sysfs_ops->panel_sncode_show(dev, attr, buf);
	mutex_unlock(&COMMON_LOCK->sysfs_lock.panel_sn_code_lock);
	return ret;
}

static ssize_t lcd_kit_panel_sncode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}

	if (!sysfs_ops->panel_sncode_store) {
		LCD_KIT_ERR("panel_sncode_store is NULL!\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.panel_sn_code_lock);
	if (sysfs_ops->panel_sncode_store(dev, attr, buf, count)) {
		LCD_KIT_ERR("panel_sncode_store error!\n");
		mutex_unlock(&COMMON_LOCK->sysfs_lock.panel_sn_code_lock);
		return LCD_KIT_FAIL;
	}
	mutex_unlock(&COMMON_LOCK->sysfs_lock.panel_sn_code_lock);
	return count;
}

static ssize_t lcd_kit_idle_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = lcd_kit_get_sysfs_ops();

	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (!sysfs_ops->idle_mode_store) {
		LCD_KIT_ERR("idle_mode_store is NULL\n");
		return LCD_KIT_FAIL;
	}
	return sysfs_ops->idle_mode_store(dev, attr, buf, count);
}

static ssize_t lcd_kit_pre_camera_position_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&COMMON_LOCK->sysfs_lock.pre_camera_position_lock);
	if (sysfs_ops->pre_camera_position_show) {
		mutex_unlock(&COMMON_LOCK->sysfs_lock.pre_camera_position_lock);
		return sysfs_ops->pre_camera_position_show(dev, attr, buf);
	}
	mutex_unlock(&COMMON_LOCK->sysfs_lock.pre_camera_position_lock);
	return LCD_KIT_OK;
}

static ssize_t lcd_kit_panel_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (sysfs_ops->panel_version_show)
		ret = sysfs_ops->panel_version_show(dev, attr, buf);
	return ret;
}

static DEVICE_ATTR(lcd_model, 0644, lcd_kit_model_show, NULL);
static DEVICE_ATTR(lcd_display_type, 0644, lcd_kit_type_show, NULL);
static DEVICE_ATTR(panel_info, 0644, lcd_kit_panel_info_show, NULL);
static DEVICE_ATTR(lcd_voltage_enable, 0200, NULL,
	lcd_kit_lcd_voltage_enable_store);
static DEVICE_ATTR(amoled_acl, 0644, lcd_kit_amoled_acl_ctrl_show,
	lcd_kit_amoled_acl_ctrl_store);
static DEVICE_ATTR(amoled_vr_mode, 0644, lcd_kit_amoled_vr_mode_show,
	lcd_kit_amoled_vr_mode_store);
static DEVICE_ATTR(lcd_support_mode, 0644, lcd_kit_support_mode_show,
	lcd_kit_support_mode_store);
static DEVICE_ATTR(gamma_dynamic, 0644, NULL, lcd_kit_gamma_dynamic_store);
static DEVICE_ATTR(frame_count, 0444, lcd_kit_frame_count_show, NULL);
static DEVICE_ATTR(frame_update, 0644, lcd_kit_frame_update_show,
	lcd_kit_frame_update_store);
static DEVICE_ATTR(mipi_dsi_bit_clk_upt, 0644, lcd_kit_mipi_dsi_clk_upt_show,
	lcd_kit_mipi_dsi_clk_upt_store);
static DEVICE_ATTR(lcd_fps_scence, 0644, lcd_kit_fps_scence_show,
	lcd_kit_fps_scence_store);
static DEVICE_ATTR(alpm_function, 0644, lcd_kit_alpm_function_show,
	lcd_kit_alpm_function_store);
static DEVICE_ATTR(alpm_setting, 0644, lcd_kit_alpm_setting_show,
	lcd_kit_alpm_setting_store);
static DEVICE_ATTR(lcd_func_switch, 0644, lcd_kit_func_switch_show,
	lcd_kit_func_switch_store);
static DEVICE_ATTR(lcd_reg_read, 0600, lcd_kit_reg_read_show,
	lcd_kit_reg_read_store);
static DEVICE_ATTR(ddic_oem_info, 0644, lcd_kit_ddic_oem_info_show,
	lcd_kit_ddic_oem_info_store);
static DEVICE_ATTR(lcd_bl_mode, 0644, lcd_kit_bl_mode_show,
	lcd_kit_bl_mode_store);
static DEVICE_ATTR(lcd_bl_support_mode, 0644, lcd_kit_support_bl_mode_show,
	lcd_kit_support_bl_mode_store);
static DEVICE_ATTR(effect_bl, 0644, lcd_kit_effect_bl_show,
	lcd_kit_effect_bl_store);
static DEVICE_ATTR(ddic_lv_detect, 0644,
	lcd_kit_ddic_lv_detect_test_show,
	lcd_kit_ddic_lv_detect_test_store);
static DEVICE_ATTR(lcd_hbm_mode, 0644,
	lcd_kit_hbm_mode_show,
	lcd_kit_hbm_mode_store);

static DEVICE_ATTR(ddic_alpha, 0644, NULL, lcd_kit_ddic_alpha_store);
static DEVICE_ATTR(lcd_fps_order, 0644, lcd_kit_fps_order_show,
	NULL);
static DEVICE_ATTR(panel_sncode, 0644,
	lcd_kit_panel_sncode_show,
	lcd_kit_panel_sncode_store);
static DEVICE_ATTR(display_idle_mode, 0644,
	NULL, lcd_kit_idle_mode_store);
/*lint -e1564*/
static DEVICE_ATTR(pre_camera_position, 0644,
	lcd_kit_pre_camera_position_show, NULL);
static DEVICE_ATTR(panel_version, 0644, lcd_kit_panel_version_show, NULL);
/*lint +e1564*/

static struct attribute *lcd_kit_sysfs_attrs[LCD_KIT_SYSFS_MAX] = {NULL};
struct attribute *lcd_kit_conf[] = {
	&dev_attr_lcd_model.attr,
	&dev_attr_lcd_display_type.attr,
	&dev_attr_panel_info.attr,
	&dev_attr_lcd_voltage_enable.attr,
	&dev_attr_amoled_acl.attr,
	&dev_attr_amoled_vr_mode.attr,
	&dev_attr_lcd_support_mode.attr,
	&dev_attr_gamma_dynamic.attr,
	&dev_attr_frame_count.attr,
	&dev_attr_frame_update.attr,
	&dev_attr_mipi_dsi_bit_clk_upt.attr,
	&dev_attr_lcd_fps_scence.attr,
	&dev_attr_alpm_function.attr,
	&dev_attr_alpm_setting.attr,
	&dev_attr_lcd_func_switch.attr,
	&dev_attr_lcd_reg_read.attr,
	&dev_attr_ddic_oem_info.attr,
	&dev_attr_lcd_bl_mode.attr,
	&dev_attr_lcd_bl_support_mode.attr,
	&dev_attr_effect_bl.attr,
	&dev_attr_ddic_lv_detect.attr,
	&dev_attr_lcd_hbm_mode.attr,
	&dev_attr_ddic_alpha.attr,
	&dev_attr_lcd_fps_order.attr,
	&dev_attr_panel_sncode.attr,
	&dev_attr_display_idle_mode.attr,
	&dev_attr_pre_camera_position.attr,
	&dev_attr_panel_version.attr,
};

struct attribute_group lcd_kit_sysfs_attr_group = {
	.attrs = lcd_kit_sysfs_attrs,
};

static struct lcd_kit_sysfs_ops *g_lcd_kit_sysfs_ops;
int lcd_kit_sysfs_ops_register(struct lcd_kit_sysfs_ops *ops)
{
	if (g_lcd_kit_sysfs_ops) {
		LCD_KIT_ERR("g_lcd_kit_sysfs_ops has already registered!\n");
		return LCD_KIT_FAIL;
	}
	g_lcd_kit_sysfs_ops = ops;
	LCD_KIT_INFO("g_lcd_kit_sysfs_ops register success!\n");
	return LCD_KIT_OK;
}

int lcd_kit_sysfs_ops_unregister(struct lcd_kit_sysfs_ops *ops)
{
	if (g_lcd_kit_sysfs_ops == ops) {
		g_lcd_kit_sysfs_ops = NULL;
		LCD_KIT_INFO("g_lcd_kit_sysfs_ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("g_lcd_kit_sysfs_ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_sysfs_ops *lcd_kit_get_sysfs_ops(void)
{
	return g_lcd_kit_sysfs_ops;
}

static int lcd_kit_check_support(int index)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		LCD_KIT_ERR("sysfs_ops is null\n");
		return LCD_KIT_OK;
	}
	if (sysfs_ops->check_support)
		return sysfs_ops->check_support(index);
	LCD_KIT_INFO("not register config function\n");
	return LCD_KIT_OK;
}

int lcd_kit_create_sysfs(struct kobject *obj)
{
	int rc;
	int i;
	int count = 0;

	for (i = 0; i < (int)(ARRAY_SIZE(lcd_kit_conf)); i++) {
		if (i >= (LCD_KIT_SYSFS_MAX - 1)) {
			LCD_KIT_ERR("dev attr number exceed sysfs max\n");
			return LCD_KIT_FAIL;
		}
		if (lcd_kit_check_support(i) && count < LCD_KIT_SYSFS_MAX)
			lcd_kit_sysfs_attrs[count++] = lcd_kit_conf[i];  //lint !e661
	}
	rc = sysfs_create_group(obj, &lcd_kit_sysfs_attr_group);
	if (rc)
		LCD_KIT_ERR("sysfs group creation failed, rc=%d\n", rc);
	return rc;
}

void lcd_kit_remove_sysfs(struct kobject *obj)
{
	sysfs_remove_group(obj, &lcd_kit_sysfs_attr_group);
}
