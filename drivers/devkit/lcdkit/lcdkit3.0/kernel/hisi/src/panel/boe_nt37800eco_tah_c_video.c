/*
 * boe_nt37800eco_tah_c_video.c
 *
 * boe_nt37800eco lcd adapt file
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#define READ_REG0A_LEN	1
#define REPORT_ERR_ECO	0x0325
#define READ_REGAB_LEN	2
#define READ_DTYPE_VD	0x1000006


static int check_error_report_eco(struct hisi_fb_data_type *hisifd)
{
	char reg_ab[] = { 0xab }; // 0xab is register address
	struct dsi_cmd_desc read_ab_cmds[] = {
		{ READ_DTYPE_VD, 0, 10, WAIT_TYPE_US, // 10---delay 10us
			sizeof(reg_ab), reg_ab },
	};
	uint32_t dsi0_value = 0;
	uint32_t dsi1_value = 0;
	int ret;

	ret = mipi_dsi_lread_reg(&dsi0_value, read_ab_cmds,
		READ_REGAB_LEN, hisifd->mipi_dsi0_base);
	if (ret) {
		LCD_KIT_ERR("Read dsi0 error number: %d\n", ret);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("dsi0:reg 0xab = 0x%x\n", dsi0_value);
	if (is_dual_mipi_panel(hisifd)) {
		ret = mipi_dsi_lread_reg(&dsi1_value, read_ab_cmds,
			READ_REGAB_LEN, hisifd->mipi_dsi1_base);
		if (ret) {
			LCD_KIT_ERR("Read dsi1 error number: %d\n", ret);
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("dsi1:reg 0xab = 0x%x\n", dsi1_value);
	}
	if (dsi0_value & REPORT_ERR_ECO) {
		LCD_KIT_ERR("dsi0:reg 0xab = 0x%x\n", dsi0_value);
		hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
		return LCD_KIT_FAIL;
	}
	if (is_dual_mipi_panel(hisifd)) {
		if (dsi1_value & REPORT_ERR_ECO) {
			LCD_KIT_ERR("dsi1:reg 0xab = 0x%x\n", dsi1_value);
			hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
			return LCD_KIT_FAIL;
		}
	}
	return ret;
}

static int boe_nt37800eco_esd_check(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;

	if (!common_info) {
		LCD_KIT_ERR("common_info is null\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->esd.support) {
		LCD_KIT_INFO("not support esd\n");
		return ret;
	}
	if (common_info->esd.status == ESD_STOP) {
		LCD_KIT_INFO("bypass esd check\n");
		return LCD_KIT_OK;
	}
	lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	/* check 0xab */
	ret = check_error_report_eco(hisifd);
	if (ret)
		LCD_KIT_ERR("check error report fail\n");
	lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	return ret;
}

static struct lcd_kit_panel_ops g_boe_nt37800eco = {
	.lcd_esd_check = boe_nt37800eco_esd_check,
};

int boe_nt37800eco_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&g_boe_nt37800eco);
	if (ret) {
		LCD_KIT_ERR("register failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

