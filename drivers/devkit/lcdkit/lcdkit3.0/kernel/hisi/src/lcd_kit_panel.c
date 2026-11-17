/*
 * lcd_kit_panel.c
 *
 * lcdkit panel function for lcd driver
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

#include "lcd_kit_panel.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_ext_disp.h"
#include "lcd_kit_sysfs_hs.h"
#include "panel/lg_nt36772a.c"
#include "panel/boe_hx83112e.c"
#include "panel/samsung_ea8076.c"
#include "panel/boe_r66451.c"
#include "panel/boe_nt37800f_tah_8p03_3lane_2mux_cmd.c"
#include "panel/boe_nt37800eco_tah_c_video.c"
#include "panel/boe_r66458.c"
#include "panel/090_901_6p756.c"
#include "panel/samsung_s6e3hc2.c"
#include "panel/boe_nt37700p_6p57.c"
#include "panel/esd_mipi_err_check.c"
#include "panel/dual_mipi_panel.c"
#include "panel/sot_esd_err_check.c"

static struct lcd_kit_panel_map panel_map[] = {
	{ PANEL_LG_NT36772A, lg_nt36772a_probe },
	{ PANEL_LG_NT36772A_V2, lg_nt36772a_probe },
	{ PANEL_BOE_HX83112E, boe_hx83112e_probe },
	{ PANEL_SAMSUNG_EA8076_ELLE, samsung_ea8076_probe },
	{ PANEL_SAMSUNG_EA8076_ELLE_V2, samsung_ea8076_probe },
	{ PANEL_SAMSUNG_EA8074, samsung_ea8076_probe },
	{ PANEL_SAMSUNG_EA8074_VN1, samsung_ea8076_probe },
	{ PANEL_BOE_R66451, boe_r66451_probe },
	{ PANEL_SAMSUNG_EA8076, samsung_ea8076_probe },
	{ PANEL_SAMSUNG_EA8076_6P53, samsung_ea8076_probe },
	{ PANEL_SAMSUNG_EA8074_PANDA, samsung_ea8076_probe },
	{ PANEL_BOE_NT37800F_TAH, boe_nt37800f_probe },
	{ PANEL_BOE_NT37800ECO_TAH_C, boe_nt37800eco_probe },
	{ PANEL_SAMSUNG_EA8079, samsung_ea8076_probe },
	{ PANEL_SM_S6E3HA8_LION, samsung_ea8076_probe },
	{ PANEL_BOE_R66458_6P63, boe_r66458_probe },
	{ PANEL_SM_S6E3HC2_ELSA, samsung_s6e3hc2_probe },
	{ PANEL_BOE_NT37700P_6P57, boe_nt37700p_probe },
	{ PANEL_090_901_6P756, _090_901_probe },
	{ PANEL_SAMSUNG_8FC1, samsung_ea8076_probe },
	{ PANEL_190_206_6P45, lcd_kit_esd_mipi_err_check_probe },
	{ PANEL_191_206_8P01, lcd_kit_dual_mipi_panel_probe },
	{ PANEL_350_C00_12P6, lcd_kit_dual_mipi_panel_probe },
	{ PANEL_190_C00_6P45, lcd_kit_sot_esd_err_check_probe },
	{ PANEL_191_C00_8P01, lcd_kit_sot_esd_err_check_probe },
};

struct lcd_kit_panel_ops *g_lcd_kit_panel_ops[LCD_ACTIVE_PANEL_BUTT];
int lcd_kit_panel_ops_register(struct lcd_kit_panel_ops *ops)
{
	if (!g_lcd_kit_panel_ops[LCD_ACTIVE_PANEL]) {
		g_lcd_kit_panel_ops[LCD_ACTIVE_PANEL] = ops;
		LCD_KIT_INFO("ops register success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops have been registered!\n");
	return LCD_KIT_FAIL;
}
int lcd_kit_panel_ops_unregister(struct lcd_kit_panel_ops *ops)
{
	if (g_lcd_kit_panel_ops[LCD_ACTIVE_PANEL] == ops) {
		g_lcd_kit_panel_ops[LCD_ACTIVE_PANEL] = NULL;
		LCD_KIT_INFO("ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_panel_ops *lcd_kit_panel_get_ops(void)
{
	return g_lcd_kit_panel_ops[LCD_ACTIVE_PANEL];
}

int lcd_kit_panel_init(void)
{
	int ret;
	int i;

	if (!disp_info->compatible) {
		LCD_KIT_ERR("compatible is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < ARRAY_SIZE(panel_map); i++) {
		if (!strncmp(disp_info->compatible, panel_map[i].compatible,
			strlen(disp_info->compatible))) {
			ret = panel_map[i].callback();
			if (ret) {
				LCD_KIT_ERR("ops init fail\n");
				return LCD_KIT_FAIL;
			}
			break;
		}
	}
	/* init ok */
	if (i >= ARRAY_SIZE(panel_map))
		LCD_KIT_INFO("not find adapter ops\n");
	return LCD_KIT_OK;
}
