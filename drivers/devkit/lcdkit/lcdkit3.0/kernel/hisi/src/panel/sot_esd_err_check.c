/*
 * sot_esd_err_check.c
 *
 * sot_esd_err_check lcd adapt file
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

void lcd_kit_sot_err_dual_ic_check(void *hld)
{
	int i, ret;
	uint32_t dsi0_sot_err_cnt = 0;
	uint32_t dsi1_sot_err_cnt = 0;
	uint8_t dsi0_value[MAX_REG_READ_COUNT] = { 0 };
	uint8_t dsi1_value[MAX_REG_READ_COUNT] = { 0 };

	ret = lcd_kit_dual_dsi_cmds_rx(hld, dsi0_value, dsi1_value,
		MAX_REG_READ_COUNT - 1, &disp_info->sot_err_check.check_cmds);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return;
	}
	for (i = 0; i < disp_info->sot_err_check.check_value.cnt; i++) {
		LCD_KIT_INFO("check_value[%d]:0x%02x!\n",
			i, disp_info->sot_err_check.check_value.buf[i]);
		if (dsi0_value[0] == disp_info->sot_err_check.check_value.buf[i])
			dsi0_sot_err_cnt++;
		if (dsi1_value[0] == disp_info->sot_err_check.check_value.buf[i])
			dsi1_sot_err_cnt++;
	}
	disp_info->sot_err_check.dsi0_sot_err_cnt += dsi0_sot_err_cnt;
	disp_info->sot_err_check.dsi1_sot_err_cnt += dsi1_sot_err_cnt;
	LCD_KIT_INFO("dsi0_value = 0x%02x, dsi1_value = 0x%02x, dsi0_err_cnt = %u, dsi1_err_cnt = %u\n",
		dsi0_value[0],
		dsi1_value[0],
		disp_info->sot_err_check.dsi0_sot_err_cnt,
		disp_info->sot_err_check.dsi1_sot_err_cnt);
}

int lcd_kit_sot_err_check(void *hld)
{
	int i, ret;
	uint32_t dsi0_sot_err_cnt = 0;
	uint8_t dsi0_value[MAX_REG_READ_COUNT] = { 0 };
	struct hisi_fb_data_type *hisifd = NULL;

	if (!hld)
		return LCD_KIT_FAIL;
	if (!disp_info->sot_err_check.support)
		return LCD_KIT_OK;
	if (!disp_info->sot_err_check.check_cmds.cmds)
		return LCD_KIT_OK;

	hisifd = (struct hisi_fb_data_type *)hld;
	if (is_dual_mipi_panel(hisifd)) {
		lcd_kit_sot_err_dual_ic_check(hld);
		return LCD_KIT_OK;
	}
	ret = lcd_kit_dsi_cmds_rx(hld, dsi0_value, MAX_REG_READ_COUNT - 1,
		&disp_info->sot_err_check.check_cmds);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return ret;
	}
	for (i = 0; i < disp_info->sot_err_check.check_value.cnt; i++) {
		LCD_KIT_INFO("check_value[%d]:0x%02x!\n",
			i, disp_info->sot_err_check.check_value.buf[i]);
		if (dsi0_value[0] == disp_info->sot_err_check.check_value.buf[i])
			dsi0_sot_err_cnt++;
	}
	disp_info->sot_err_check.dsi0_sot_err_cnt += dsi0_sot_err_cnt;
	LCD_KIT_INFO("dsi0_value = 0x%02x, dsi0_err_cnt = %u\n",
		dsi0_value[0], disp_info->sot_err_check.dsi0_sot_err_cnt);
	return LCD_KIT_OK;
}

static int lcd_kit_sot_esd_err_check(
	struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;

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
	(void)lcd_kit_sot_err_check((void *)hisifd);
	if (is_dual_mipi_panel(hisifd))
		ret = lcd_kit_dual_mipi_esd_check(hisifd);
	else if (common_ops->esd_handle)
		ret = common_ops->esd_handle(hisifd);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("esd check fail\n");
		return ret;
	}
	return ret;
}

static struct lcd_kit_panel_ops g_sot_esd_err_check = {
	.lcd_esd_check = lcd_kit_sot_esd_err_check,
};

int lcd_kit_sot_esd_err_check_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&g_sot_esd_err_check);
	if (ret) {
		LCD_KIT_ERR("register failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

