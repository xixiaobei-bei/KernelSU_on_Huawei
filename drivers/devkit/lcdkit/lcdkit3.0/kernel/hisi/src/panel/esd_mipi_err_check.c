/*
 * esd_mipi_err_check.c
 *
 * esd_mipi_err_check lcd adapt file
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

int lcd_kit_esd_mipi_err_check(void *hld)
{
	int ret;
	uint8_t dsi0_value[MAX_REG_READ_COUNT] = { 0 };
	uint8_t dsi1_value[MAX_REG_READ_COUNT] = { 0 };
	struct hisi_fb_data_type *hisifd = NULL;

	if (!hld)
		return LCD_KIT_FAIL;
	if (!common_info->esd.mipi_err_cmds.cmds)
		return LCD_KIT_OK;
	if (common_info->esd.mipi_err_cnt == 0)
		return LCD_KIT_OK;

	hisifd = (struct hisi_fb_data_type *)hld;
	if (is_dual_mipi_panel(hisifd)) {
		ret = lcd_kit_dual_dsi_cmds_rx(hld, dsi0_value, dsi1_value,
			MAX_REG_READ_COUNT - 1, &common_info->esd.mipi_err_cmds);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_INFO("mipi_rx fail\n");
			return ret;
		}
		if ((dsi0_value[0] > common_info->esd.mipi_err_cnt) ||
			(dsi1_value[0] > common_info->esd.mipi_err_cnt)) {
			LCD_KIT_ERR("dsi0_err_cnt = 0x%x, threshold = 0x%x\n",
				dsi0_value[0], common_info->esd.mipi_err_cnt);
			LCD_KIT_ERR("dsi1_err_cnt = 0x%x, threshold = 0x%x\n",
				dsi1_value[0], common_info->esd.mipi_err_cnt);
			hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("dsi0_err_cnt = 0x%x, dsi1_err_cnt = 0x%x, threshold = 0x%x\n",
			dsi0_value[0], dsi1_value[0], common_info->esd.mipi_err_cnt);
		return LCD_KIT_OK;
	}
	ret = lcd_kit_dsi_cmds_rx(hld, dsi0_value, MAX_REG_READ_COUNT - 1,
		&common_info->esd.mipi_err_cmds);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return ret;
	}
	if (dsi0_value[0] > common_info->esd.mipi_err_cnt) {
		LCD_KIT_ERR("dsi0_err_cnt = 0x%x, threshold = 0x%x\n",
			dsi0_value[0], common_info->esd.mipi_err_cnt);
		hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("mipi_err_cnt = 0x%x, threshold = 0x%x\n",
		dsi0_value[0], common_info->esd.mipi_err_cnt);
	return LCD_KIT_OK;
}

static int lcd_kit_esd_mipi_check(
	struct hisi_fb_data_type *hisifd)
{
	int ret;

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
	if (common_ops->esd_handle) {
		ret = common_ops->esd_handle(hisifd);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_ERR("esd check fail\n");
			return ret;
		}
	}
	ret = lcd_kit_esd_mipi_err_check((void *)hisifd);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("esd check fail\n");
		return ret;
	}
	return ret;
}

static struct lcd_kit_panel_ops g_esd_mipi_err_check = {
	.lcd_esd_check = lcd_kit_esd_mipi_check,
};

int lcd_kit_esd_mipi_err_check_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&g_esd_mipi_err_check);
	if (ret) {
		LCD_KIT_ERR("register failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

