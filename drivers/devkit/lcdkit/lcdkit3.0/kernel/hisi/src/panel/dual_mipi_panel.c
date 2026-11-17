/*
 * dual_mipi_panel.c
 *
 * dual_mipi_panel lcd adapt file
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
#define MAX_ESD_READ_COUNT 10
#define LCD_ESD_DETECT_NOMAL 0
#define LCD_ESD_DETECT_EXT 1

static int32_t lcd_kit_esd_gpio_detect(void)
{
	int32_t gpio_value;
	int32_t ret;
	u32 gpio_num;
	gpio_num = common_info->esd.gpio_detect_num;

	ret = gpio_request(gpio_num, "esd_detect_gpio");
	if (ret != 0) {
		LCD_KIT_ERR("esd_detect_gpio[%u] request fail!\n", gpio_num);
		return LCD_KIT_FAIL;
	}
	ret = gpio_direction_input(gpio_num);
	if (ret != 0) {
		gpio_free(gpio_num);
		LCD_KIT_ERR("esd_detect_gpio[%u] direction set fail!\n", gpio_num);
		return LCD_KIT_FAIL;
	}
	gpio_value = gpio_get_value(gpio_num);
	gpio_free(gpio_num);
	LCD_KIT_INFO("gpio_num:%u value:%d\n", gpio_num, gpio_value);

	if (gpio_value != common_info->esd.gpio_normal_value)
		return LCD_KIT_FAIL;
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_detect_event(void)
{
	int ret = LCD_KIT_OK;
	int i;
	struct ts_kit_ops *ts_ops = NULL;

	if (!lcd_kit_esd_gpio_detect())
		return LCD_KIT_OK;
	if (!common_info->esd.tp_esd_event)
		return LCD_KIT_FAIL;
	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->send_esd_event) {
		LCD_KIT_ERR("ts_ops or send_esd_event is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < common_info->esd.tp_report_detect_times; i++) {
		if (!lcd_kit_esd_gpio_detect())
			return LCD_KIT_FAIL;
	}
	ret = ts_ops->send_esd_event(!common_info->esd.gpio_normal_value);
	if (ret)
		LCD_KIT_ERR("ts_ops->send_esd_event faile\n");
	return LCD_KIT_FAIL;
}

static int lcd_kit_dual_mipi_esd_handle(
	struct hisi_fb_data_type *hisifd, int detect_type)
{
	int ret = LCD_KIT_OK;
	int esd_cnt = 0;
	uint8_t dsi0_value[MAX_ESD_READ_COUNT] = { 0 };
	uint8_t dsi1_value[MAX_ESD_READ_COUNT] = { 0 };
	uint8_t i;
	uint8_t expect_value;
	uint8_t judge_type;
	u32 *esd_value = NULL;

	if (common_info->esd.gpio_detect_support) {
		if (lcd_kit_gpio_detect_event())
			return LCD_KIT_FAIL;
	}

	if (detect_type == LCD_ESD_DETECT_NOMAL) {
		ret = lcd_kit_dual_dsi_cmds_rx(hisifd, dsi0_value, dsi1_value,
			MAX_ESD_READ_COUNT, &common_info->esd.cmds);
		esd_value = common_info->esd.value.buf;
		esd_cnt = common_info->esd.value.cnt;
	} else if (detect_type == LCD_ESD_DETECT_EXT) {
		ret = lcd_kit_dual_dsi_cmds_rx(hisifd, dsi0_value, dsi1_value,
			MAX_ESD_READ_COUNT, &common_info->esd.ext_cmds);
		esd_value = common_info->esd.ext_value.buf;
		esd_cnt = common_info->esd.ext_value.cnt;
	}

	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return ret;
	}

	if (!esd_value) {
		LCD_KIT_ERR("esd_value is null\n");
		return LCD_KIT_OK;
	}

	for (i = 0; i < esd_cnt; i++) {
		if (dsi0_value[i] != dsi1_value[i]) {
			LCD_KIT_INFO("dsi0_value[%u] = 0x%x, dsi1_value[%u] = 0x%x",
				i, dsi0_value[i], i, dsi1_value[i]);
			ret = LCD_KIT_FAIL;
			break;
		}
		judge_type = (esd_value[i] >> 8) & 0xFF;
		expect_value = esd_value[i] & 0xFF;
		if (lcd_kit_judge_esd(judge_type,
			dsi0_value[i], expect_value)) {
			LCD_KIT_ERR("judge_type[%u], read_val[%u] = 0x%x, but exp_val = 0x%x!\n",
				judge_type, i, dsi0_value[i], expect_value);
			ret = LCD_KIT_FAIL;
			break;
		}
		LCD_KIT_INFO("judge_type[%d], esd_val[%u] = 0x%x, read_val = 0x%x, exp_val = 0x%x",
			judge_type, i, esd_value[i], dsi0_value[i], expect_value);
	}
	return ret;
}

int lcd_kit_dual_mipi_esd_check(
	struct hisi_fb_data_type *hisifd)
{
	int ret;
	static int detect_type = LCD_ESD_DETECT_NOMAL;

	if (!common_info) {
		LCD_KIT_ERR("common_info is null\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->esd.support) {
		LCD_KIT_INFO("not support esd\n");
		return LCD_KIT_OK;
	}
	if (common_info->esd.status == ESD_STOP) {
		LCD_KIT_INFO("bypass esd check\n");
		return LCD_KIT_OK;
	}
	if (!common_info->esd.ext_cmds.cmds)
		detect_type = LCD_ESD_DETECT_NOMAL;
	ret = lcd_kit_dual_mipi_esd_handle(hisifd, detect_type);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("check error report fail\n");
		return ret;
	}
	ret = lcd_kit_esd_mipi_err_check((void *)hisifd);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("check error report fail\n");
		return ret;
	}
	detect_type = (detect_type == LCD_ESD_DETECT_NOMAL) ? \
		LCD_ESD_DETECT_EXT : LCD_ESD_DETECT_NOMAL;
	return ret;
}

static struct lcd_kit_panel_ops g_dual_mipi_panel = {
	.lcd_esd_check = lcd_kit_dual_mipi_esd_check,
};

int lcd_kit_dual_mipi_panel_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&g_dual_mipi_panel);
	if (ret) {
		LCD_KIT_ERR("register failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

