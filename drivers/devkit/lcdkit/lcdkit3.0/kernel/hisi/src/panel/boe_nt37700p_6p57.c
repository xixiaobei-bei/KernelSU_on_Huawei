/*
 * boe_nt37700p_6p57.c
 *
 * boe_nt37700p_6p57 lcd adapt file
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
static void dsm_client_record_esd_err(uint32_t err_no, char read_value, char expect_value)
{
#if defined(CONFIG_HUAWEI_DSM)
	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, "lcd esd register status error:read_value[2]=0x%x, expect_value[2]=0x%x\n",
			read_value, expect_value);
		dsm_client_notify(lcd_dclient, err_no);
	}
#endif
}

static int panel_esd_check(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;
	int i;
	char read_value[MAX_REG_READ_COUNT] = {0};
	char expect_value;
	u32 *esd_value = NULL;

	if (!common_info->esd.support) {
		LCD_KIT_INFO("not support esd\n");
		return ret;
	}
	if (common_info->esd.status == ESD_STOP) {
		LCD_KIT_INFO("bypass esd check\n");
		return ret;
	}

	ret = lcd_kit_dsi_cmds_rx(hisifd, read_value, MAX_REG_READ_COUNT,
		&common_info->esd.cmds);
	if (ret)
		LCD_KIT_INFO("mipi_rx fail, error is = %d\n", ret);

	esd_value = common_info->esd.value.buf;
	for (i = 0; i < common_info->esd.value.cnt; i++) {
		expect_value = esd_value[i] & 0xFF;
		if (read_value[i] == expect_value) {
			LCD_KIT_INFO("esd_value[%d] = 0x%x, read_value[%d] = 0x%x, expect_value = 0x%x\n",
				i, esd_value[i], i, read_value[i], expect_value);
			continue;
		}
		LCD_KIT_ERR("read_value[%d] = 0x%x, but expect_value = 0x%x!\n",
			i, read_value[i], expect_value);
		ret = LCD_KIT_FAIL;
		/* i == 2 means check 0xe9 register error */
		if (i == 2) {
			dsm_client_record_esd_err(DSM_LCD_ESD_STATUS_ERROR_NO,
				read_value[i], expect_value);
			ret = LCD_KIT_OK;
		}
		break;
	}
	LCD_KIT_INFO("esd check result:%d\n", ret);
	return ret;
}

static struct lcd_kit_panel_ops g_boe_nt37700p = {
	.lcd_esd_check = panel_esd_check,
};

int boe_nt37700p_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&g_boe_nt37700p);
	if (ret) {
		LCD_KIT_ERR("register failed , error is %d\n", ret);
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
