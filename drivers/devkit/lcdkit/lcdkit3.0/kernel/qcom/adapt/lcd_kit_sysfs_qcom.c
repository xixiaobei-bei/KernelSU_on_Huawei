/*
 * lcd_kit_sysfs_qcom.c
 *
 * lcdkit sysfs function for lcd driver qcom platform
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "lcd_kit_drm_panel.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_sysfs_qcom.h"
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
#define PANEL_MAX 10
#define NIT_LENGTH 3

/* oem info */
static int oem_info_type = INVALID_TYPE;
static unsigned long g_display_nit = 0;
static int lcd_get_2d_barcode(char *oem_data);
static int lcd_get_project_id(char *oem_data);
static struct oem_info_cmd oem_read_cmds[] = {
	{ PROJECT_ID_TYPE, lcd_get_project_id },
	{ BARCODE_2D_TYPE, lcd_get_2d_barcode }
};
extern unsigned int lcm_get_panel_state(void);
static ssize_t lcd_model_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

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

static int __init early_parse_nit_cmdline(char *arg)
{
	int len;

	if (arg == NULL) {
		LCD_KIT_ERR("nit is null\n");
		return -EINVAL;
	}

	len = strlen(arg);
	if (len <= NIT_LENGTH)
		strict_strtoul(arg, 0, &g_display_nit);

	return LCD_KIT_OK;
}

early_param("display_nit", early_parse_nit_cmdline);

static ssize_t lcd_panel_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char panel_type[PANEL_MAX] = {0};
	struct lcd_kit_bl_ops *bl_ops = NULL;
	char *bl_type = " ";

	if (common_info->panel_type == LCD_TYPE)
		strncpy(panel_type, "LCD", strlen("LCD"));
	else if (common_info->panel_type == AMOLED_TYPE)
		strncpy(panel_type, "AMOLED", strlen("AMOLED"));
	else
		strncpy(panel_type, "INVALID", strlen("INVALID"));

	bl_ops = lcd_kit_get_bl_ops();
	if ((bl_ops != NULL) && (bl_ops->name != NULL))
		bl_type = bl_ops->name;

	ret = snprintf(buf, PAGE_SIZE, "blmax:%u,blmin:%u,blmax_nit_actual:%d,blmax_nit_standard:%d,lcdtype:%s,bl_type:%s\n",
		common_info->bl_level_max, common_info->bl_level_min,
		g_display_nit, common_info->bl_max_nit,
		panel_type, bl_type);

	return ret;
}

static ssize_t lcd_fps_scence_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char str[LCD_REG_LENGTH_MAX] = {0};
	char tmp[MAX_BUF] = {0};
	int i;
	int fps_rate;
	struct qcom_panel_info *pinfo = NULL;

	if (disp_info->fps.support) {
		ret = snprintf(str, sizeof(str), "current_fps:%d;default_fps:%d",
			disp_info->fps.current_fps, disp_info->fps.default_fps);
		strncat(str, ";support_fps_list:", strlen(";support_fps_list:"));
		for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
			fps_rate = disp_info->fps.panel_support_fps_list.buf[i];
			if (i > 0)
				strncat(str, ",", strlen(","));
			ret += snprintf(tmp, sizeof(tmp), "%d", fps_rate);
			strncat(str, tmp, strlen(tmp));
		}
	} else {
		pinfo = lcm_get_panel_info();

		ret = snprintf(str, sizeof(str), "lcd_fps=%d",
			disp_info->fps.default_fps);
	}
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	LCD_KIT_INFO("%s\n", str);
	return ret;
}

static ssize_t lcd_fps_scence_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t lcd_fps_order_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char str[LCD_REG_LENGTH_MAX] = {0};
	char tmp[MAX_BUF] = {0};
	int i;
	int fps_rate;

	if (!buf || !attr) {
		LCD_KIT_ERR("lcd_fps_order_show buf NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!disp_info->fps.support) {
		ret = snprintf(buf, PAGE_SIZE, "0\n");
		return ret;
	} else {
		ret = snprintf(str, sizeof(str), "1,%d,%d", disp_info->fps.default_fps,
			disp_info->fps.panel_support_fps_list.cnt);
		for (i = 0; i < disp_info->fps.panel_support_fps_list.cnt; i++) {
			fps_rate = disp_info->fps.panel_support_fps_list.buf[i];
			if (i == 0)
				strncat(str, ",", strlen(","));
			else
				strncat(str, ";", strlen(";"));
			ret += snprintf(tmp, sizeof(tmp), "%d:%d", fps_rate, ORDER_DELAY);
			strncat(str, tmp, strlen(tmp));
		}
	}
	LCD_KIT_INFO("%s\n", str);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	return ret;
}

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

static ssize_t lcd_alpm_setting_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int mode = 0;

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
	return count;
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
	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx is NULL\n");
		return LCD_KIT_FAIL;
	}
	val = simple_strtoul(buf, NULL, 0);
	if (!FACT_INFO->inversion.support) {
		LCD_KIT_ERR("not support inversion\n");
		return LCD_KIT_OK;
	}
	switch (val) {
	case COLUMN_MODE:
		ret = adapt_ops->mipi_tx(panel_info->display,
			&FACT_INFO->inversion.column_cmds);
		LCD_KIT_DEBUG("ret = %d", ret);
		break;
	case DOT_MODE:
		ret = adapt_ops->mipi_tx(panel_info->display,
			&FACT_INFO->inversion.dot_cmds);
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
	panel_state = lcm_get_panel_state();
	if (!panel_state) {
		LCD_KIT_ERR("panel is power off!\n");
		return ret;
	}
	if (FACT_INFO->check_reg.support) {
		expect_ptr = (char *)FACT_INFO->check_reg.value.buf;
		ret = adapt_ops->mipi_rx(panel_info->display, read_value,
			MAX_REG_READ_COUNT, &FACT_INFO->check_reg.cmds);
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
	struct qcom_panel_info *pinfo = NULL;

	pinfo = lcm_get_panel_info();
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

	length = sizeof(oem_read_cmds) / sizeof(oem_read_cmds[0]);
	for (i = 0; i < length; i++) {
		if (oem_info_type == oem_read_cmds[i].type) {
			LCD_KIT_INFO("cmd = %d\n", oem_info_type);
			if (oem_read_cmds[i].func)
				(*oem_read_cmds[i].func)(oem_info_data);
		}
	}

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

static ssize_t lcd_kit_cabc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	if (common_ops->get_cabc_mode)
		ret = common_ops->get_cabc_mode(buf);
	return ret;
}

static ssize_t lcd_kit_cabc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = LCD_KIT_OK;
	unsigned long mode = 0;
	struct qcom_panel_info *panel_info = NULL;

	panel_info = lcm_get_panel_info();
	if (panel_info == NULL) {
		LCD_KIT_ERR("panel_info is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = strict_strtoul(buf, 0, &mode);
	if (ret) {
		LCD_KIT_ERR("invalid data!\n");
		return ret;
	}

	if (common_ops->set_cabc_mode) {
		mutex_lock(&COMMON_LOCK->mipi_lock);
		common_ops->set_cabc_mode(panel_info->display, mode);
		mutex_unlock(&COMMON_LOCK->mipi_lock);
	}
	return ret;
}

static ssize_t lcd_panel_sncode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	ssize_t ret;
	struct qcom_panel_info *pinfo = NULL;
	char str_oem[OEM_INFO_SIZE_MAX] = {0};
	char str_tmp[OEM_INFO_SIZE_MAX] = {0};

	if (buf == NULL) {
		LCD_KIT_ERR("hisifd or buf is null\n");
		return LCD_KIT_FAIL;
	}

	pinfo = lcm_get_panel_info();
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

static int lcd_check_support(int index)
{
	switch (index) {
	case LCD_MODEL_INDEX:
	case LCD_TYPE_INDEX:
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
	case LCD_CABC_MODE:
		return common_info->cabc.support;
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
	.panel_sncode_show = lcd_panel_sncode_show,
	.lcd_cabc_mode_show = lcd_kit_cabc_show,
	.lcd_cabc_mode_store = lcd_kit_cabc_store,
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
