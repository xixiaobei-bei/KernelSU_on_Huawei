 /*
 * lcd_kit_sysfs_mtk.c
 *
 * lcdkit sysfs function for lcd driver mtk platform
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

#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_sysfs_mtk.h"
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#include "mtk_panel_ext.h"
#else
#include "lcm_drv.h"
#endif
#include <linux/kernel.h>
#include "lcd_kit_adapt.h"
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif
/* marco define */
#ifndef strict_strtoul
#define strict_strtoul kstrtoul
#endif

#ifndef strict_strtol
#define strict_strtol kstrtol
#endif

static int g_alpm_setting_ret = LCD_KIT_FAIL;

/* oem info */
static int oem_info_type = INVALID_TYPE;
static int lcd_get_2d_barcode(char *oem_data);
static int lcd_get_project_id(char *oem_data);

static struct oem_info_cmd oem_read_cmds[] = {
	{ PROJECT_ID_TYPE, lcd_get_project_id },
	{ BARCODE_2D_TYPE, lcd_get_2d_barcode }
};

extern int hisi_adc_get_value(int adc_channel);
extern int is_mipi_cmd_panel(void);
extern bool runmode_is_factory(void);
extern int lcd_kit_dsi_cmds_extern_tx(struct lcd_kit_dsi_panel_cmds *cmds);
extern unsigned int lcm_get_panel_state(void);
static ssize_t lcd_model_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops == NULL) {
		LCD_KIT_ERR("common_ops NULL\n");
		return LCD_KIT_FAIL;
	}
	if (common_ops->get_panel_name)
		ret = common_ops->get_panel_name(buf);
	return ret;
}

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!buf) {
		LCD_KIT_ERR("NULL_PTR ERROR!\n");
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", is_mipi_cmd_panel() ? 1 : 0);
}

static ssize_t lcd_panel_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
#define PANEL_MAX 10
	int ret;
	char panel_type[PANEL_MAX] = {0};

	if (common_info->panel_type == LCD_TYPE)
		strncpy(panel_type, "LCD", strlen("LCD"));
	else if (common_info->panel_type == AMOLED_TYPE)
		strncpy(panel_type, "AMOLED", strlen("AMOLED"));
	else
		strncpy(panel_type, "INVALID", strlen("INVALID"));
	ret = snprintf(buf, PAGE_SIZE, "blmax:%u,blmin:%u,blmax_nit_actual:%d,blmax_nit_standard:%d,lcdtype:%s,\n",
		common_info->bl_level_max, common_info->bl_level_min,
		common_info->actual_bl_max_nit, common_info->bl_max_nit,
		panel_type);
	return ret;
}

static ssize_t lcd_fps_scence_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char str[LCD_REG_LENGTH_MAX] = {0};
	char tmp[MAX_BUF] = {0};
	int i;
	int index;
	struct lcd_kit_array_data *timing = NULL;
	struct mtk_panel_info *pinfo = NULL;

	if (disp_info->fps.support) {
		ret = snprintf(str, sizeof(str), "current_fps:%d;default_fps:%d",
			disp_info->fps.current_fps, disp_info->fps.default_fps);
		strncat(str, ";support_fps_list:", strlen(";support_fps_list:"));
		for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
			index = disp_info->fps.panel_support_fps_list.buf[i];
			timing = &disp_info->fps.fps_dsi_timming[index];
			if (i > 0)
				strncat(str, ",", strlen(","));
			ret += snprintf(tmp, sizeof(tmp), "%d", timing->buf[FPS_VRE_INDEX]);
			strncat(str, tmp, strlen(tmp));
		}
		ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	} else {
		pinfo = lcd_kit_get_mkt_panel_info();
		if (pinfo == NULL) {
			LCD_KIT_ERR("pinfo is null\n");
			return LCD_KIT_FAIL;
		}
		ret = snprintf(str, sizeof(str), "current_fps:%d;default_fps:%d;support_fps_list:%d",
			pinfo->vrefresh, pinfo->vrefresh, pinfo->vrefresh);
	}

	LCD_KIT_INFO("%s\n", str);
	return ret;
}

static ssize_t lcd_fps_scence_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;
	int ret = LCD_KIT_FAIL;

	if (!buf) {
		LCD_KIT_ERR("lcd fps scence store buf NULL Pointer!\n");
		return (ssize_t)ret;
	}

	val = simple_strtoul(buf, NULL, 0);
	LCD_KIT_INFO("val = %lu", val);

	switch (val) {
	case LCD_FPS_APP_SET_60:
		disp_info->fps.current_fps = 60;
		ret = lcd_kit_dsi_cmds_extern_tx(&disp_info->fps.fps_to_cmds[LCD_FPS_SCENCE_H60]);
		break;
	case LCD_FPS_APP_SET_90:
		disp_info->fps.current_fps = 90;
		ret = lcd_kit_dsi_cmds_extern_tx(&disp_info->fps.fps_to_cmds[LCD_FPS_SCENCE_90]);
		break;
	case LCD_FPS_APP_SET_120:
		disp_info->fps.current_fps = 120;
		ret = lcd_kit_dsi_cmds_extern_tx(&disp_info->fps.fps_to_cmds[LCD_FPS_SCENCE_120]);
		break;
	default:
		LCD_KIT_ERR("fps mode %d not found\n", val);
		break;
	}

	if (ret < 0)
		LCD_KIT_ERR("fps mode switch to %d failed\n", val);

	return (ssize_t)ret;
}

static int lcd_fps_get_scence(int fps)
{
	int ret_fps;

	switch (fps) {
	case LCD_FPS_SCENCE_H60:
		ret_fps = LCD_FPS_60_INDEX;
		break;
	case LCD_FPS_SCENCE_90:
		ret_fps = LCD_FPS_90_INDEX;
		break;
	case LCD_FPS_SCENCE_120:
		ret_fps = LCD_FPS_120_INDEX;
		break;
	default:
		ret_fps = LCD_FPS_60_INDEX;
		break;
	}
	return ret_fps;
}

static ssize_t lcd_fps_order_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char str[LCD_REG_LENGTH_MAX] = {0};
	char tmp[MAX_BUF] = {0};
	int i;
	int fps_index;

	if (!buf || !attr) {
		LCD_KIT_ERR("lcd_fps_order_show buf NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info->fps.support) {
		ret = snprintf(buf, PAGE_SIZE, "0\n");
		return ret;
	} else {
		ret = snprintf(str, sizeof(str), "1,%d,%d", 0,
			disp_info->fps.panel_support_fps_list.cnt);
		for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
			fps_index = lcd_fps_get_scence(disp_info->fps.panel_support_fps_list.buf[i]);
			if (i == 0)
				strncat(str, ",", strlen(","));
			else
				strncat(str, ";", strlen(";"));
			ret += snprintf(tmp, sizeof(tmp), "%d:%d", fps_index, ORDER_DELAY);
			strncat(str, tmp, strlen(tmp));
		}
	}
	LCD_KIT_INFO("%s\n", str);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	return ret;
}

#ifdef LCD_FACTORY_MODE
static ssize_t lcd_amoled_gpio_pcd_errflag(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_gpio_pcd_errflag_check();
	ret = snprintf(buf, PAGE_SIZE, "%d\n", ret);
	return ret;
}
#endif

#ifdef LCD_FACTORY_MODE
static ssize_t lcd_amoled_cmds_pcd_errflag(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int check_result;
	struct mtk_panel_info *pinfo = NULL;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	pinfo = lcd_kit_get_mkt_panel_info();
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}

	if (pinfo->panel_state) {
		if (disp_info->pcd_errflag.pcd_support ||
			disp_info->pcd_errflag.errflag_support) {
			check_result = lcd_kit_check_pcd_errflag_check_fac();
			ret = snprintf(buf, PAGE_SIZE, "%d\n", check_result);
			LCD_KIT_INFO("pcd_errflag, the check_result = %d\n",
				check_result);
		}
	}
	return ret;
}
#endif

#ifdef LCD_FACTORY_MODE
int lcd_kit_pcd_detect(uint32_t val)
{
	int ret;
	static uint32_t pcd_mode;

	if (!FACT_INFO->pcd_errflag_check.pcd_errflag_check_support) {
		LCD_KIT_ERR("pcd errflag not support!\n");
		return LCD_KIT_OK;
	}

	if (pcd_mode == val) {
		LCD_KIT_ERR("pcd detect already\n");
		return LCD_KIT_OK;
	}

	pcd_mode = val;
	if (pcd_mode == LCD_KIT_PCD_DETECT_OPEN)
		ret = lcd_kit_dsi_cmds_extern_tx(
			&FACT_INFO->pcd_errflag_check.pcd_detect_open_cmds);
	else if (pcd_mode == LCD_KIT_PCD_DETECT_CLOSE)
		ret = lcd_kit_dsi_cmds_extern_tx(
			&FACT_INFO->pcd_errflag_check.pcd_detect_close_cmds);
	return ret;
}
#endif

#ifdef LCD_FACTORY_MODE
static ssize_t lcd_amoled_pcd_errflag_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->pcd_errflag.pcd_support ||
		disp_info->pcd_errflag.errflag_support)
		ret = lcd_amoled_cmds_pcd_errflag(dev, attr, buf);
	else
		ret = lcd_amoled_gpio_pcd_errflag(dev, attr, buf);
	return ret;
}
#endif

#ifdef LCD_FACTORY_MODE
static ssize_t lcd_amoled_pcd_errflag_store(struct device *dev,
	struct device_attribute *attr, const char *buf)
{
	int ret;
	unsigned long val = 0;
	struct mtk_panel_info *pinfo = NULL;

	pinfo = lcd_kit_get_mkt_panel_info();
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}

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
	return lcd_kit_pcd_detect(val);
}
#endif

static ssize_t lcd_alpm_function_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_KIT_OK;
	return ret;
}

static ssize_t lcd_alpm_function_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_alpm_setting_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!buf) {
		LCD_KIT_ERR("NULL_PTR ERROR!\n");
		return -EINVAL;
	}
	return (ssize_t)snprintf(buf, PAGE_SIZE, "%d\n", g_alpm_setting_ret);
}

static ssize_t lcd_alpm_setting_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int mode = 0;

	g_alpm_setting_ret = LCD_KIT_FAIL;
	if (!buf) {
		LCD_KIT_ERR("lcd alpm setting store buf is null!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	if (!sscanf(buf, "%u", &mode)) {
		LCD_KIT_ERR("sscanf return invaild\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->alpm.support && (lcd_kit_alpm_setting(mode) < 0))
		return LCD_KIT_FAIL;

	g_alpm_setting_ret = LCD_KIT_OK;
	return LCD_KIT_OK;
}
#ifdef LCD_FACTORY_MODE
static ssize_t lcd_inversion_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return lcd_kit_inversion_get_mode(buf);
}

static ssize_t lcd_inversion_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;
	int ret = LCD_KIT_OK;

	val = simple_strtoul(buf, NULL, 0);
	if (!FACT_INFO->inversion.support) {
		LCD_KIT_ERR("not support inversion\n");
		return count;
	}
	switch (val) {
	case COLUMN_MODE:
		ret = lcd_kit_dsi_cmds_extern_tx(&FACT_INFO->inversion.column_cmds);
		LCD_KIT_DEBUG("ret = %d", ret);
		break;
	case DOT_MODE:
		ret = lcd_kit_dsi_cmds_extern_tx(&FACT_INFO->inversion.dot_cmds);
		LCD_KIT_DEBUG("ret = %d", ret);
		break;
	default:
		return LCD_KIT_FAIL;
	}
	FACT_INFO->inversion.mode = (int)val;
	LCD_KIT_INFO("inversion.support = %d, inversion.mode = %d\n",
		FACT_INFO->inversion.support, FACT_INFO->inversion.mode);
	return LCD_KIT_OK;
}

static ssize_t lcd_check_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	uint8_t read_value[MAX_REG_READ_COUNT] = {0};
	int i;
	char *expect_ptr = NULL;
	unsigned int panel_state;

	panel_state = lcm_get_panel_state();
	if (!panel_state) {
		LCD_KIT_ERR("panel is power off!\n");
		return ret;
	}
	if (FACT_INFO->check_reg.support) {
		expect_ptr = (char *)FACT_INFO->check_reg.value.buf;
		lcd_kit_dsi_cmds_extern_rx(read_value,
			&FACT_INFO->check_reg.cmds, MAX_REG_READ_COUNT);
		for (i = 0; i < FACT_INFO->check_reg.cmds.cmd_cnt; i++) {
			if ((char)read_value[i] != expect_ptr[i]) {
				ret = -1;
				LCD_KIT_ERR("read_value[%u] = 0x%x, but expect_ptr[%u] = 0x%x!\n",
					i, read_value[i], i, expect_ptr[i]);
				break;
			}
			LCD_KIT_INFO("read_value[%u] = 0x%x same with expect value!\n",
					 i, read_value[i]);
		}
		if (ret == 0)
			ret = snprintf(buf, PAGE_SIZE, "OK\n");
		else
			ret = snprintf(buf, PAGE_SIZE, "FAIL\n");
		LCD_KIT_INFO("checkreg result:%s\n", buf);
	}
	return ret;
}

static ssize_t lcd_sleep_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return lcd_kit_get_sleep_mode(buf);
}

static ssize_t lcd_sleep_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val = 0;
	struct mtk_panel_info *pinfo = NULL;

	pinfo = lcd_kit_get_mkt_panel_info();
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}

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

	ret = lcd_kit_set_sleep_mode(val);
	if (ret)
		LCD_KIT_ERR("set sleep mode fail\n");
	return ret;
}
#endif
static ssize_t lcd_amoled_acl_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_ops == NULL) {
		LCD_KIT_ERR("common_ops NULL\n");
		return LCD_KIT_FAIL;
	}
	if (common_ops->get_acl_mode)
		ret = common_ops->get_acl_mode(buf);
	return ret;
}

static ssize_t lcd_amoled_acl_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_amoled_vr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t lcd_amoled_vr_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_effect_color_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return LCD_KIT_OK;
}

static ssize_t lcd_effect_color_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}
#ifdef LCD_FACTORY_MODE
static ssize_t lcd_test_config_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	ret = lcd_kit_get_test_config(buf);
	if (ret)
		LCD_KIT_ERR("not find test item\n");
	return ret;
}

static ssize_t lcd_test_config_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (lcd_kit_set_test_config(buf) < 0)
		LCD_KIT_ERR("set_test_config failed\n");
	return count;
}
#endif
static ssize_t lcd_reg_read_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return LCD_KIT_OK;
}

static ssize_t lcd_reg_read_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_gamma_dynamic_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_frame_count_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t lcd_frame_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t lcd_frame_update_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_mipi_clk_upt_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return LCD_KIT_OK;
}

static ssize_t lcd_mipi_clk_upt_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return LCD_KIT_OK;
}

static ssize_t lcd_func_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return LCD_KIT_OK;
}

static ssize_t lcd_func_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static int lcd_get_project_id(char *oem_data)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	char project_id[PROJECT_ID_LENGTH] = {0};
	int ret = 0;
	int i;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->project_id.support) {
		if (lcd_ops->get_project_id)
			ret = lcd_ops->get_project_id(project_id);
		oem_data[0] = PROJECT_ID_TYPE;
		oem_data[1] = OEM_INFO_BLOCK_NUM;
		for (i = 0; i < PROJECT_ID_LENGTH; i++)
			/* 0 type 1 block number after 2 for data */
			oem_data[i + 2] = project_id[i];
	}
	return ret;
}

static int lcd_get_2d_barcode(char *oem_data)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	char read_value[OEM_INFO_SIZE_MAX] = {0};
	int ret = 0;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->oeminfo.barcode_2d.support) {
		if (lcd_ops->get_2d_barcode)
			ret = lcd_ops->get_2d_barcode(read_value);
		oem_data[0] = BARCODE_2D_TYPE;
		oem_data[1] = BARCODE_BLOCK_NUM +
			disp_info->oeminfo.barcode_2d.number_offset;
		strncat(oem_data, read_value, strlen(read_value));
	}
	return ret;
}

static ssize_t lcd_oem_info_show(struct device* dev,
	struct device_attribute* attr, char *buf)
{
	int i;
	int ret;
	int length;
	char oem_info_data[OEM_INFO_SIZE_MAX] = {0};
	char str_oem[OEM_INFO_SIZE_MAX + 1] = {0};
	char str_tmp[OEM_INFO_SIZE_MAX + 1] = {0};
	unsigned int panel_state;

	panel_state = lcm_get_panel_state();
	if (!panel_state) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}

	if (!disp_info->oeminfo.support) {
		LCD_KIT_ERR("oem info is not support\n");
		return LCD_KIT_FAIL;
	}

	if (oem_info_type == INVALID_TYPE) {
		LCD_KIT_ERR("first write ddic_oem_info, then read\n");
		return LCD_KIT_FAIL;
	}

	disp_info->bl_is_shield_backlight = true;
	length = sizeof(oem_read_cmds) / sizeof(oem_read_cmds[0]);
	for (i = 0; i < length; i++) {
		if (oem_info_type == oem_read_cmds[i].type) {
			LCD_KIT_INFO("cmd = %d\n", oem_info_type);
			if (oem_read_cmds[i].func)
				(*oem_read_cmds[i].func)(oem_info_data);
		}
	}
	disp_info->bl_is_shield_backlight = false;

	/* parse data */
	memset(str_oem, 0, sizeof(str_oem));
	for (i = 0; i < oem_info_data[1]; i++) {
		memset(str_tmp, 0, sizeof(str_tmp));
		snprintf(str_tmp, sizeof(str_tmp),
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
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
	struct device_attribute *attr, const char *buff, size_t count)
{
	char *cur = NULL;
	char *token = NULL;
	char oem_info[OEM_INFO_SIZE_MAX] = {0};
	unsigned int panel_state;
	int i = 0;

	if (!buff) {
		LCD_KIT_ERR("buff is NULL\n");
		return count;
	}

	panel_state = lcm_get_panel_state();
	if (!panel_state) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}

	if (!disp_info->oeminfo.support) {
		LCD_KIT_ERR("oem info is not support\n");
		return LCD_KIT_FAIL;
	}

	if (strlen(buff) < OEM_INFO_SIZE_MAX) {
		cur = (char *)buff;
		token = strsep(&cur, ",");
		while (token) {
			oem_info[i++] = (unsigned char)simple_strtol(token, NULL, 0);
			token = strsep(&cur, ",");
		}
	} else {
		memcpy(oem_info, "INVALID", strlen("INVALID") + 1);
		LCD_KIT_ERR("invalid cmd\n");
	}

	LCD_KIT_INFO("write Type=0x%x , data len=%d\n", oem_info[0], oem_info[DATA_INDEX]);

	oem_info_type = oem_info[0];
	/* if the data length is 0, then just store the type */
	if (oem_info[DATA_INDEX] == 0) {
		LCD_KIT_INFO("Just store type:0x%x and then finished\n", oem_info[0]);
		return count;
	}

	LCD_KIT_INFO("oem_info = %s\n", oem_info);
	return count;
}

static ssize_t lcd_panel_sncode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	ssize_t ret;
	struct mtk_panel_info *pinfo = NULL;
	char str_oem[OEM_INFO_SIZE_MAX] = {0};
	char str_tmp[OEM_INFO_SIZE_MAX] = {0};

	if (buf == NULL) {
		LCD_KIT_ERR("hisifd or buf is null\n");
		return LCD_KIT_FAIL;
	}
#ifdef CONFIG_DRM_MEDIATEK
	pinfo = lcm_get_panel_info();
#else
	pinfo = (struct mtk_panel_info *)lcdkit_mtk_common_panel.panel_info;
#endif
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	for(i = 0; i < sizeof(pinfo->sn_code); i++) {
		memset(str_tmp, 0, sizeof(str_tmp));
		ret = snprintf(str_tmp, sizeof(str_tmp), "%d,", pinfo->sn_code[i]);
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

static ssize_t lcd_idle_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	val = simple_strtoul(buf, NULL, 0);
	if (val) {
#ifdef CONFIG_DRM_MEDIATEK
		mtk_drm_set_idlemgr_ex(1);
#endif
		LCD_KIT_INFO("enable idle mode\n");
	} else {
#ifdef CONFIG_DRM_MEDIATEK
		mtk_drm_set_idlemgr_ex(0);
#endif
		LCD_KIT_INFO("disable idle mode\n");
	}
	return count;
}

#ifdef LCD_FACTORY_MODE
static int lcd_judge_ddic_lv_detect(unsigned int pic_index, int loop)
{
	int i;
	unsigned char expect_value;
	int ret;
	unsigned int *detect_value = NULL;
	unsigned char read_value[MAX_REG_READ_COUNT] = {0};
	struct lcd_kit_array_data congfig_data;

	congfig_data = FACT_INFO->ddic_lv_detect.value[pic_index];
	detect_value = congfig_data.buf;
	if (!FACT_INFO->ddic_lv_detect.rd_cmds[pic_index].cmds) {
		LCD_KIT_INFO("read ddic lv detect cmd is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* delay 2s, or ddic reg value invalid */
	ssleep(2);
	ret = lcd_kit_dsi_cmds_extern_rx(read_value,
		&FACT_INFO->ddic_lv_detect.rd_cmds[pic_index],
		MAX_REG_READ_COUNT - 1);
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
			LCD_KIT_ERR("pic: %u, read_val:%d = 0x%x, "
				"but expect_val = 0x%x\n",
				pic_index, i, read_value[i], expect_value);
			return LCD_KIT_FAIL;
		}
	}
	return ret;
}

static int lcd_ddic_lv_detect_test(void)
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
	ret = lcd_judge_ddic_lv_detect(pic_index, loop_num);
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

static int lcd_ddic_lv_detect_set(unsigned int pic_index)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_dsi_panel_cmds *enter_cmds = NULL;

	pic_index = pic_index - DET_START;
	enter_cmds = FACT_INFO->ddic_lv_detect.enter_cmds;
	if ((pic_index >= DET1_INDEX) && (pic_index < DETECT_NUM)) {
		/* disable esd */
		lcd_esd_enable(DISABLE);
		if (!enter_cmds[pic_index].cmds) {
			LCD_KIT_ERR("send ddic lv detect cmd is NULL\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_dsi_cmds_extern_tx(&enter_cmds[pic_index]);
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
	struct mtk_panel_info *pinfo = NULL;

	pinfo = lcd_kit_get_mkt_panel_info();
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null!\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->ddic_lv_detect.support) {
		down(&disp_info->blank_sem);
		if (pinfo->panel_state) {
			result = lcd_ddic_lv_detect_test();
			ret = snprintf(buf, PAGE_SIZE, "%d\n", result);
		}
		up(&disp_info->blank_sem);
		lcd_kit_recovery_display();
		/* enable esd */
		lcd_esd_enable(ENABLE);
	}
	return ret;
}

static ssize_t lcd_ddic_lv_detect_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int index = 0;
	struct mtk_panel_info *pinfo = NULL;

	pinfo = lcd_kit_get_mkt_panel_info();
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	if (!FACT_INFO->ddic_lv_detect.support) {
		LCD_KIT_ERR("ddic lv detect is not support\n");
		return LCD_KIT_FAIL;
	}
	if (!dev) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	/* decimal parsing */
	ret = kstrtouint(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("strict_strtoul fail\n");
		return ret;
	}
	LCD_KIT_INFO("picture index=%u\n", index);
	down(&disp_info->blank_sem);
	if (pinfo->panel_state)
		ret = lcd_ddic_lv_detect_set(index);
	up(&disp_info->blank_sem);
	return ret;
}

static int lcd_hor_line_test(void)
{
	int ret = LCD_KIT_OK;
	struct mtk_panel_info *pinfo = NULL;
	int count = HOR_LINE_TEST_TIMES;

	pinfo = lcd_kit_get_mkt_panel_info();
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("horizontal line test start\n");
	LCD_KIT_INFO("g_fact_info.hor_line.duration = %d\n",
			FACT_INFO->hor_line.duration);
	/* disable esd check */
	lcd_esd_enable(DISABLE);
	while (count--) {
		/* hardware reset */
		lcd_hardware_reset();
		mdelay(30);
		/* test avdd */
		down(&disp_info->blank_sem);
		if (!pinfo->panel_state) {
			LCD_KIT_ERR("panel is power off\n");
			up(&disp_info->blank_sem);
			return LCD_KIT_FAIL;
		}
		if (FACT_INFO->hor_line.hl_cmds.cmds) {
			ret = lcd_kit_dsi_cmds_extern_tx(
				&FACT_INFO->hor_line.hl_cmds);
			if (ret)
				LCD_KIT_ERR("send avdd cmd error\n");
		}
		up(&disp_info->blank_sem);
		msleep(FACT_INFO->hor_line.duration * MILLISEC_TIME);
		/* recovery display */
		lcd_kit_recovery_display();
	}
	/* enable esd */
	lcd_esd_enable(ENABLE);
	LCD_KIT_INFO("horizontal line test end\n");
	return ret;
}

static ssize_t lcd_general_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	if (FACT_INFO->hor_line.support)
		ret = lcd_hor_line_test();

	if (ret == LCD_KIT_OK)
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	else
		ret = snprintf(buf, PAGE_SIZE, "FAIL\n");

	return ret;
}

static void lcd_vtc_line_set_bias_voltage()
{
	struct lcd_kit_bias_ops *bias_ops = NULL;
	int ret;

	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not get bias_ops\n");
		return;
	}
	/* set bias voltage */
	if ((bias_ops->set_vtc_bias_voltage) &&
		(FACT_INFO->vtc_line.vtc_vsp > 0) &&
		(FACT_INFO->vtc_line.vtc_vsn > 0)) {
		ret = bias_ops->set_vtc_bias_voltage(
			FACT_INFO->vtc_line.vtc_vsp,
			FACT_INFO->vtc_line.vtc_vsn, true);
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("set vtc bias voltage error\n");
		if (bias_ops->set_bias_is_need_disable())
			lcd_kit_recovery_display();
	}
}

static void lcd_vtc_line_resume_bias_voltage()
{
	struct lcd_kit_bias_ops *bias_ops = NULL;
	int ret;

	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not get bias_ops\n");
		return;
	}
	/* set bias voltage */
	if ((bias_ops->set_vtc_bias_voltage) &&
		(bias_ops->set_bias_is_need_disable())) {
		/* buf[2]:set bias voltage value */
		ret = bias_ops->set_vtc_bias_voltage(power_hdl->lcd_vsp.buf[2],
			power_hdl->lcd_vsn.buf[2], false);
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("set vtc bias voltage error\n");
	}
}

static int lcd_vtc_line_test(unsigned long pic_index)
{
	int ret = LCD_KIT_OK;

	switch (pic_index) {
	case PIC1_INDEX:
		/* disable esd */
		lcd_esd_enable(DISABLE);
		/* lcd panel set bias */
		lcd_vtc_line_set_bias_voltage();
		/* hardware reset */
		if (!FACT_INFO->vtc_line.vtc_no_reset)
			lcd_hardware_reset();
		mdelay(20);
		if (FACT_INFO->vtc_line.vtc_cmds.cmds) {
			ret = lcd_kit_dsi_cmds_extern_tx(
				&FACT_INFO->vtc_line.vtc_cmds);
			if (ret)
				LCD_KIT_ERR("send vtc cmd error\n");
		}
		break;
	case PIC2_INDEX:
	case PIC3_INDEX:
	case PIC4_INDEX:
		LCD_KIT_INFO("picture:%lu display\n", pic_index);
		break;
	case PIC5_INDEX:
		/* lcd panel resume bias */
		lcd_vtc_line_resume_bias_voltage();
		lcd_kit_recovery_display();
		/* enable esd */
		lcd_esd_enable(ENABLE);
		break;
	default:
		LCD_KIT_ERR("pic number not support\n");
		break;
	}
	return ret;
}

static ssize_t lcd_vertical_line_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	if (FACT_INFO->hor_line.support)
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	return ret;
}

static ssize_t lcd_vertical_line_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long index = 0;

	if (!dev) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtoul(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("strict_strtoul fail!\n");
		return ret;
	}
	LCD_KIT_INFO("index=%ld\n", index);
	if (FACT_INFO->vtc_line.support) {
		ret = lcd_vtc_line_test(index);
		if (ret)
			LCD_KIT_ERR("vtc line test fail\n");
	}
	return count;
}

static ssize_t lcd_oneside_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (FACT_INFO->oneside_mode.support ||
		FACT_INFO->color_aod_det.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n",
			FACT_INFO->oneside_mode.mode);

	return ret;
}

static int lcd_kit_oneside_setting(uint32_t mode)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("oneside driver mode is %u\n", mode);
	FACT_INFO->oneside_mode.mode = (int)mode;
	switch (mode) {
	case ONESIDE_DRIVER_LEFT:
		ret = lcd_kit_dsi_cmds_extern_tx(
			&FACT_INFO->oneside_mode.left_cmds);
		break;
	case ONESIDE_DRIVER_RIGHT:
		ret = lcd_kit_dsi_cmds_extern_tx(
			&FACT_INFO->oneside_mode.right_cmds);
		break;
	case ONESIDE_MODE_EXIT:
		ret = lcd_kit_dsi_cmds_extern_tx(
			&FACT_INFO->oneside_mode.exit_cmds);
		break;
	default:
		break;
	}
	return ret;
}

static int lcd_kit_aod_detect_setting(uint32_t mode)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("color aod detect mode is %u\n", mode);
	FACT_INFO->oneside_mode.mode = (int)mode;
	if (mode == COLOR_AOD_DETECT_ENTER)
		ret = lcd_kit_dsi_cmds_extern_tx(
			&FACT_INFO->color_aod_det.enter_cmds);
	else if (mode == COLOR_AOD_DETECT_EXIT)
		ret = lcd_kit_dsi_cmds_extern_tx(
			&FACT_INFO->color_aod_det.exit_cmds);
	else
		LCD_KIT_ERR("color aod detect mode not support\n");
	return ret;
}

static ssize_t lcd_oneside_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	unsigned int mode = 0;
	struct mtk_panel_info *pinfo = NULL;

	pinfo = lcd_kit_get_mkt_panel_info();
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	if (!pinfo->panel_state) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("oneside setting store buf null!\n");
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
	if (FACT_INFO->oneside_mode.support)
		lcd_kit_oneside_setting(mode);
	if (FACT_INFO->color_aod_det.support)
		lcd_kit_aod_detect_setting(mode);
	return count;
}
#endif

static int lcd_check_support(int index)
{
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
		return disp_info->vr_support;
	case FRAME_UPDATE_INDEX:
		return disp_info->vr_support;
	case MIPI_DSI_CLK_UPT_INDEX:
	case FPS_SCENCE_INDEX:
		return SYSFS_SUPPORT;
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
	case PANEL_SNCODE_INDEX:
		return common_info->sn_code.support;
	case DISPLAY_IDLE_MODE:
		return disp_info->lcd_kit_idle_mode_support;
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
	.alpm_setting_show = lcd_alpm_setting_show,
	.alpm_setting_store = lcd_alpm_setting_store,
	.func_switch_show = lcd_func_switch_show,
	.func_switch_store = lcd_func_switch_store,
	.ddic_oem_info_show = lcd_oem_info_show,
	.ddic_oem_info_store = lcd_oem_info_store,
	.panel_sncode_show = lcd_panel_sncode_show,
	.idle_mode_store = lcd_idle_mode_store,
#ifdef LCD_FACTORY_MODE
	.ddic_lv_detect_test_show = lcd_ddic_lv_detect_test_show,
	.ddic_lv_detect_test_store = lcd_ddic_lv_detect_test_store,
#endif
};
#ifdef LCD_FACTORY_MODE
struct lcd_fact_ops g_fact_ops = {
	.inversion_mode_show = lcd_inversion_mode_show,
	.inversion_mode_store = lcd_inversion_mode_store,
	.check_reg_show = lcd_check_reg_show,
	.sleep_ctrl_show = lcd_sleep_ctrl_show,
	.sleep_ctrl_store = lcd_sleep_ctrl_store,
	.test_config_show = lcd_test_config_show,
	.test_config_store = lcd_test_config_store,
	.general_test_show = lcd_general_test_show,
	.vtc_line_test_show = lcd_vertical_line_test_show,
	.vtc_line_test_store = lcd_vertical_line_test_store,
	.oneside_mode_show = lcd_oneside_mode_show,
	.oneside_mode_store = lcd_oneside_mode_store,
	.pcd_errflag_check_show = lcd_amoled_pcd_errflag_show,
	.pcd_errflag_check_store = lcd_amoled_pcd_errflag_store,
};
#endif

int lcd_kit_sysfs_init(void)
{
#ifdef LCD_FACTORY_MODE
	lcd_fact_ops_register(&g_fact_ops);
#endif
	lcd_kit_sysfs_ops_register(&g_lcd_sysfs_ops);
	return LCD_KIT_OK;
}
