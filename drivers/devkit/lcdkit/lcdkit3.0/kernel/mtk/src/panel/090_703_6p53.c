/*
 * panel_090_703_6p53.c
 *
 * 090_703_6p53 lcd driver
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

#define AREA_SETTING_REG 0x81

static int panel_090_703_6p53_set_area(uint32_t new_vert_start)
{
	int i = 0;
	uint32_t len = 0;
	uint32_t start = 0;
	uint32_t end = 0;
	struct lcd_kit_dsi_cmd_desc *cmds = NULL;
	struct lcd_kit_dsi_panel_cmds *off_cmds = &disp_info->alpm.off_cmds;

	for (i = 0; i < off_cmds->cmd_cnt; i++) {
		cmds = off_cmds->cmds + i;
		if (cmds->payload[0] == AREA_SETTING_REG)
			break;
	}
	if (i == off_cmds->cmd_cnt) {
		LCD_KIT_ERR("not find reg 0x%x\n", AREA_SETTING_REG);
		return LCD_KIT_FAIL;
	}
	/*
	 * payload[4], vertical_start[11:4]
	 * payload[5], vertical_start[3:0] | vertical_end[11:8]
	 * payload[6], vertical_end[7:0]
	 */
	start = (cmds->payload[4] << 4) | (cmds->payload[5] >> 4);
	end = ((cmds->payload[5] & 0xF) << 8) | cmds->payload[6];
	if (end < start) {
		LCD_KIT_ERR("end less than start\n");
		return LCD_KIT_FAIL;
	}
	len = end - start;
	start = new_vert_start;
	cmds->payload[4] &= 0x00;
	cmds->payload[4] = (start >> 4) & 0xFF;
	cmds->payload[5] &= 0x0F;
	cmds->payload[5] |= (start & 0xF) << 4;
	end = (start & 0xFFF) + len;
	cmds->payload[5] &= 0xF0;
	cmds->payload[5] |= (end >> 8) & 0xF;
	cmds->payload[6] &= 0x00;
	cmds->payload[6] = end & 0xFF;

	return LCD_KIT_OK;
}

static struct lcd_kit_panel_ops panel_090_703_6p53_ops = {
	.aod_set_area = panel_090_703_6p53_set_area,
};

int panel_090_703_6p53_probe(void)
{
	int ret = lcd_kit_panel_ops_register(&panel_090_703_6p53_ops);
	if (ret < 0) {
		LCD_KIT_ERR("failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
