/*
 * boe_r66451.c
 *
 * boe_r66451 lcd driver
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

#define R66458_BARCODE_2D_LEN  44

static int boe_r66458_2d_code(char *oem_data,
	struct hisi_fb_data_type *hisifd)
{
	char read_value[OEM_INFO_SIZE_MAX + 1] = {0};
	int ret = 0;
	int i;
	int j;

	if (disp_info->oeminfo.barcode_2d.support) {
		ret = lcd_kit_dsi_cmds_rx(hisifd, read_value, OEM_INFO_SIZE_MAX,
			&disp_info->oeminfo.barcode_2d.cmds);
		oem_data[0] = BARCODE_2D_TYPE;
		oem_data[1] = BARCODE_BLOCK_NUM;
		/* oem data store from 3rd byte */
		for (i = 0, j = 2; i < R66458_BARCODE_2D_LEN; i = i + 2, j++)
			oem_data[j] = ((read_value[i] << 4) & 0xF0) |
				(read_value[i + 1] & 0x0F);
	}
	return ret;
}

static struct lcd_kit_panel_ops boe_r66458 = {
	.lcd_get_2d_barcode = boe_r66458_2d_code,
};

int boe_r66458_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&boe_r66458);
	if (ret) {
		LCD_KIT_ERR("failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

