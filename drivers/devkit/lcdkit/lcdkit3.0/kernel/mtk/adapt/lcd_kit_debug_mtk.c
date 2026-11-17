/*
 * lcd_kit_debug_mtk.c
 *
 * lcdkit debug function for lcd driver of mtk platform
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include "lcd_kit_dbg.h"
#include "lcd_kit_adapt.h"
#include <base.h>

extern struct delayed_work detect_fs_work;
extern int g_dpd_mode;
extern int g_dpd_enter;

static int dbg_dsi_cmds_rx(uint8_t *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds)
{
	if (!out || !cmds) {
		LCD_KIT_ERR("out is null or cmds is null\n");
		return LCD_KIT_FAIL;
	}
	return lcd_kit_dsi_cmds_extern_rx(out, cmds, out_len);
}

static bool dbg_panel_power_on(void)
{
	LCD_KIT_INFO("power on: %d\n", common_ops->panel_power_on);
	return common_ops->panel_power_on;
}

static struct lcd_kit_dbg_ops mtk_dbg_ops = {
	.dbg_mipi_rx = dbg_dsi_cmds_rx,
	.panel_power_on = dbg_panel_power_on,
};

int lcd_kit_dbg_init(void)
{
	LCD_KIT_INFO("enter\n");
	return lcd_kit_debug_register(&mtk_dbg_ops);
}

int dpd_init(struct device_node *np)
{
	static bool is_first = true;

	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return LCD_KIT_FAIL;
	}
	if (!g_dpd_mode)
		return LCD_KIT_OK;
	if (is_first) {
		is_first = false;
		INIT_DELAYED_WORK(&detect_fs_work, detect_fs_wq_handler);
		/* delay 500ms schedule work */
		schedule_delayed_work(&detect_fs_work, msecs_to_jiffies(500));
	}
	if (file_sys_is_ready()) {
		LCD_KIT_INFO("sysfs is not ready!\n");
		return LCD_KIT_FAIL;
	}
	g_dpd_enter = 1; // lcd dpd is ok, entry lcd dpd mode
	return LCD_KIT_OK;
}
