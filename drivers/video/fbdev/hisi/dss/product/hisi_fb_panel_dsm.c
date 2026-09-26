/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"
#include "hisi_fb_panel.h"
#include "panel/mipi_lcd_utils.h"
#if defined(CONFIG_DEVICE_ATTRS)
#include "product/attrs/hisi_panel_attrs.h"
#endif

#if defined(CONFIG_HUAWEI_DSM)
void panel_check_status_and_report_by_dsm(struct lcd_reg_read_t *lcd_status_reg, int cnt, char __iomem *mipi_dsi0_base)
{
	u32 read_value = 0, expected_value = 0, read_mask = 0;
	u8 reg_addr = 0;
	char *reg_name = NULL;
	int dsm_error_found = 0, dsm_client_ready = 0;
	u32 pkg_status = 0, try_times = 100;
	int i = 0;
	struct dsi_cmd_desc packet_size_set_cmd = {DTYPE_MAX_PKTSIZE, 0, 10, WAIT_TYPE_US, 1, NULL};
	bool for_debug = false;

	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient))
		dsm_client_ready = 1;

	lcd_status_reg->recovery = 0;
	mipi_dsi_check_0lane_is_ready(mipi_dsi0_base);
	mipi_dsi_max_return_packet_size(&packet_size_set_cmd, mipi_dsi0_base);

	for (i = 0; i < cnt; i++) {
		/* Each iteration read a registers */
		reg_addr = lcd_status_reg[i].reg_addr;
		reg_name = lcd_status_reg[i].reg_name;
		expected_value = lcd_status_reg[i].expected_value;
		read_mask = lcd_status_reg[i].read_mask;
		for_debug = lcd_status_reg[i].for_debug;

		/* Do not read debug register to avoid lcd light delay */
		if (for_debug)
			continue;

		/* Send MIPI read command, and wait the return value no longer than 100*30us */
		try_times = 100;
		outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, reg_addr << 8 | 0x06);
		udelay(20);
		do {
			pkg_status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status & 0x10))
				break;

			udelay(30);
		} while (--try_times);

		/* Read the return value, report error if read timeout or read value is not expected */
		read_value = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
		if (read_mask != 0)
			read_value &= read_mask;
		if (!try_times) {
			DPU_FB_ERR("Read %s timeout!\n", reg_name);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "Read %s timeout!\n", reg_name);
				dsm_error_found++;
			}
			break;
		} else if (read_value != expected_value && read_mask != 0) {
			DPU_FB_ERR("ERROR: read %s = 0x%X, but expect: 0x%X\n", reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect: 0x%X\n",
					reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		} else if (read_value == expected_value && read_mask == 0) {
			/* In this case read_mask == 0, read value equal to expected_value is a LCD error */
			DPU_FB_ERR("ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n",
				reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n",
					reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		}

		DPU_FB_INFO("Read %s = 0x%x\n", reg_name, read_value);
	}

	/* Report error to Device Status Monitor */
	if (dsm_error_found > 0) {
		for (i = 0; i < cnt; i++) {
			reg_addr = lcd_status_reg[i].reg_addr;
			for_debug = lcd_status_reg[i].for_debug;
			expected_value = lcd_status_reg[i].expected_value;

			/* Do not read status registers because already read them just now */
			if (!for_debug)
				continue;

			/* Send MIPI read command, and wait the return value no longer than 100*30us */
			try_times = 100;
			outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, reg_addr << 8 | 0x06);
			udelay(20);
			do {
				pkg_status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
				if (!(pkg_status & 0x10))
					break;

				udelay(30);
			} while (--try_times);

			/* dump the register which you want to read for debug */
			read_value = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);

			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "Read 0x%X = 0x%x expect_value = 0x%x\n",
					reg_addr, read_value, expected_value);
			}
		}
		dsm_client_notify(lcd_dclient, DSM_LCD_STATUS_ERROR_NO);

		lcd_status_reg->recovery = 1;
	}

	if (dsm_client_ready)
		dsm_client_unocuppy(lcd_dclient);
	else
		DPU_FB_INFO("dsm lcd_dclient ocuppy failed!\n");
}

void panel_status_report_by_dsm(struct lcd_reg_read_t *lcd_status_reg, int cnt,
	char __iomem *mipi_dsi0_base, int report_cnt)
{
	u32 read_value = 0, expected_value = 0, read_mask = 0;
	u8 reg_addr = 0;
	char *reg_name = NULL;
	int dsm_error_found = 0, dsm_client_ready = 0;
	u32 pkg_status = 0, try_times = 100;
	int i = 0;
	struct dsi_cmd_desc packet_size_set_cmd = {DTYPE_MAX_PKTSIZE, 0, 10, WAIT_TYPE_US, 1, NULL};
	static int dmd_report_count;

	if (!dsm_client_ocuppy(lcd_dclient))
		dsm_client_ready = 1;

	mipi_dsi_check_0lane_is_ready(mipi_dsi0_base);
	mipi_dsi_max_return_packet_size(&packet_size_set_cmd, mipi_dsi0_base);

	for (i = 0; i < cnt; i++) {
		/* Each iteration read a registers */
		reg_addr = lcd_status_reg[i].reg_addr;
		reg_name = lcd_status_reg[i].reg_name;
		expected_value = lcd_status_reg[i].expected_value;
		read_mask = lcd_status_reg[i].read_mask;

		/* Send MIPI read command, and wait the return value no longer than 100*30us */
		try_times = 100;
		outp32(mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, reg_addr << 8 | 0x06);
		udelay(20);
		do {
			pkg_status = inp32(mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
			if (!(pkg_status & 0x10))
				break;
			udelay(30);
		} while (--try_times);

		/* Read the return value, report error if read timeout or read value is not expected */
		read_value = inp32(mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
		if (read_mask != 0)
			read_value &= read_mask;
		if (!try_times) {
			DPU_FB_ERR("Read %s timeout!\n", reg_name);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "Read %s timeout!\n", reg_name);
				dsm_error_found++;
			}
			break;
		} else if (read_value != expected_value && read_mask != 0) {
			DPU_FB_ERR("ERROR: read %s = 0x%X, but expect: 0x%X\n", reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect: 0x%X\n",
					reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		} else if (read_value == expected_value && read_mask == 0) {
			/* In this case read_mask == 0, read value equal to expected_value is a LCD error */
			DPU_FB_ERR("ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n",
				reg_name, read_value, expected_value);
			if (dsm_client_ready) {
				dsm_client_record(lcd_dclient, "ERROR: read %s = 0x%X, but expect NOT equal to: 0x%X\n",
					reg_name, read_value, expected_value);
				dsm_error_found++;
			}
			break;
		}

		DPU_FB_INFO("Read %s = 0x%x\n", reg_name, read_value);
	}

	/* Report error to Device Status Monitor */
	if (dsm_error_found > 0) {
		dmd_report_count++;
		if (dmd_report_count == report_cnt) {
			dsm_client_notify(lcd_dclient, DSM_LCD_STATUS_ERROR_NO);
			dmd_report_count = 0;
		}
	}
	if (dsm_client_ready)
		dsm_client_unocuppy(lcd_dclient);
	else
		DPU_FB_INFO("dsm lcd_dclient ocuppy failed!\n");
}
#endif

