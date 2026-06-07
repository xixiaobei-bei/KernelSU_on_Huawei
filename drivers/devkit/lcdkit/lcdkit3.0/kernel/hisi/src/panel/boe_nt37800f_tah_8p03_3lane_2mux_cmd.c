/*
 * boe_nt37800f_tah_8p03_3lane_2mux_cmd.c
 *
 * boe_nt37800f lcd adapt file
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

#define NORMAL_MODE	0x9c
#define PARTIAL_MODE	0xb4
#define READ_REG0A_LEN	1
#define MIPI_ERROR_CNT	0x10
#define REPORT_ERR	0x0327
#define READ_REGAB_LEN	2

static int check_mipi_err_reg(struct hisi_fb_data_type *hisifd)
{
	char reg_05[] = { 0x05 }; /* 0x05 is register address */
	struct dsi_cmd_desc read_05_cmds[] = {
		{ DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US, /* 10--delay 10 us */
			sizeof(reg_05), reg_05 },
	};
	uint32_t read_value = 0;
	uint32_t read_value1 = 0;
	int ret;
	int ret1;

	ret = mipi_dsi_lread_reg(&read_value, read_05_cmds,
		READ_REG0A_LEN, hisifd->mipi_dsi0_base);
	ret1 = mipi_dsi_lread_reg(&read_value1, read_05_cmds,
		READ_REG0A_LEN, hisifd->mipi_dsi1_base);
	if ((ret) || (ret1)) {
		LCD_KIT_ERR("05 reg read dsi0 ret %d, dsi1 ret %d\n", ret, ret1);
		return LCD_KIT_FAIL;
	}
	if ((read_value > MIPI_ERROR_CNT) || (read_value1 > MIPI_ERROR_CNT)) {
		LCD_KIT_ERR("dsi0:reg 0x05 = 0x%x, threshold = 0x%x\n",
			read_value, MIPI_ERROR_CNT);
		LCD_KIT_ERR("dsi1:reg 0x05 = 0x%x, threshold = 0x%x\n",
			read_value1, MIPI_ERROR_CNT);
		hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("dsi0:reg 0x05 = 0x%x, threshold = 0x%x\n",
		read_value, MIPI_ERROR_CNT);
	LCD_KIT_INFO("dsi1:reg 0x05 = 0x%x, threshold = 0x%x\n",
		read_value1, MIPI_ERROR_CNT);
	return LCD_KIT_OK;
}

static int check_power_status(struct hisi_fb_data_type *hisifd)
{
	char reg_0a[] = { 0x0a }; // 0x0a is register address
	struct dsi_cmd_desc read_0a_cmds[] = {
		{ DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US, // 10--delay 10 us
			sizeof(reg_0a), reg_0a },
	};
	uint32_t read_value = 0;
	int ret;

	ret = mipi_dsi_lread_reg(&read_value, read_0a_cmds,
		READ_REG0A_LEN, hisifd->mipi_dsi0_base);
	if (ret) {
		LCD_KIT_ERR("Read dsi0 error number: %d\n", ret);
		return LCD_KIT_FAIL;
	}
	if ((read_value != NORMAL_MODE) &&
		(read_value != PARTIAL_MODE)) {
		LCD_KIT_ERR("dsi0:reg 0x0a = 0x%x\n", read_value);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("dsi0:reg 0x0a = 0x%x\n", read_value);
	if (lcd_is_dual_mipi()) {
		ret = mipi_dsi_lread_reg(&read_value, read_0a_cmds,
			READ_REG0A_LEN, hisifd->mipi_dsi1_base);
		if (ret) {
			LCD_KIT_ERR("Read dsi1 error number: %d\n", ret);
			return LCD_KIT_FAIL;
		}
		if ((read_value != NORMAL_MODE) &&
			(read_value != PARTIAL_MODE)) {
			LCD_KIT_ERR("dsi1:reg 0x0a = 0x%x\n", read_value);
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("dsi1:reg 0x0a = 0x%x\n", read_value);
	}
	return ret;
}

static int check_error_report(struct hisi_fb_data_type *hisifd)
{
	char reg_ab[] = { 0xab }; // 0xab is register address
	struct dsi_cmd_desc read_ab_cmds[] = {
		{ DTYPE_DCS_READ, 0, 10, WAIT_TYPE_US, // 10---delay 10us
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
	if (lcd_is_dual_mipi()) {
		ret = mipi_dsi_lread_reg(&dsi1_value, read_ab_cmds,
			READ_REGAB_LEN, hisifd->mipi_dsi1_base);
		if (ret) {
			LCD_KIT_ERR("Read dsi1 error number: %d\n", ret);
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("dsi1:reg 0xab = 0x%x\n", dsi1_value);
	}
	if (dsi0_value & REPORT_ERR) {
		LCD_KIT_ERR("dsi0:reg 0xab = 0x%x\n", dsi0_value);
		return LCD_KIT_FAIL;
	}
	if (lcd_is_dual_mipi()) {
		if (dsi1_value & REPORT_ERR) {
			LCD_KIT_ERR("dsi1:reg 0xab = 0x%x\n", dsi1_value);
			return LCD_KIT_FAIL;
		}
	}
	return ret;
}

static int boe_nt37800f_esd_check(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;

	if (!common_info->esd.support) {
		LCD_KIT_INFO("not support esd\n");
		return ret;
	}
	if (common_info->esd.status == ESD_STOP) {
		LCD_KIT_INFO("bypass esd check\n");
		return LCD_KIT_OK;
	}
	lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_LP_MODE);
	/* check 0x0a */
	ret = check_power_status(hisifd);
	if (ret) {
		LCD_KIT_ERR("check power status fail\n");
		goto fail;
	}
	/* check 0xab */
	ret = check_error_report(hisifd);
	if (ret) {
		LCD_KIT_ERR("check error report fail\n");
		goto fail;
	}
	/* check 0x05 */
	ret = check_mipi_err_reg(hisifd);
	if (ret) {
		LCD_KIT_ERR("check mipi error fail\n");
		goto fail;
	}
fail:
	lcd_kit_set_mipi_link(hisifd, LCD_KIT_DSI_HS_MODE);
	return ret;
}

static struct lcd_kit_panel_ops g_boe_nt37800f = {
	.lcd_esd_check = boe_nt37800f_esd_check,
};

int boe_nt37800f_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&g_boe_nt37800f);
	if (ret) {
		LCD_KIT_ERR("register failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
