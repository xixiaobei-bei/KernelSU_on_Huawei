/*
 * lcd_kit_panel.h
 *
 * lcdkit panel function for lcd driver head file
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

#ifndef _LCD_KIT_PANEL_H_
#define _LCD_KIT_PANEL_H_

#include "lcd_kit_common.h"
#include "hisi_fb.h"

/*
 * panel compatible
 */
#define PANEL_LG_NT36772A      "lg_nt36772a_hma_6p53_1080plus_cmd"
#define PANEL_LG_NT36772A_V2   "lg_nt36772a_hma_v2_6p53_1080plus_cmd"

#define PANEL_BOE_HX83112E           "boe_hx83112e_hma_6p53_1080plus_cmd"
#define PANEL_SAMSUNG_EA8076_ELLE    "samsung_ea8076_elle_6p11_1080plus_cmd"
#define PANEL_SAMSUNG_EA8076_ELLE_V2 "samsung_ea8076_elle_v2_6p11_1080plus_cmd"
#define PANEL_SAMSUNG_EA8074         "samsung_ea8074_ever_7p21_1080plus_cmd"
#define PANEL_SAMSUNG_EA8074_VN1     "samsung_ea8074_ever_vn1_7p21_1080plus_cmd"
#define PANEL_BOE_R66451             "boe_r66451_laya"
#define PANEL_SAMSUNG_EA8076         "samsung_ea8076"
#define PANEL_SAMSUNG_EA8076_6P53    "samsung_ea8076_6p53_1080p_cmd"
#define PANEL_SAMSUNG_EA8074_PANDA   "samsung_ea8074_panda_7p21_cmd"
#define PANEL_BOE_NT37800F_TAH   "boe-nt37800f-tah-8p03-3lane-2mux-cmd"
#define PANEL_BOE_NT37800ECO_TAH_C   "boe-nt37800eco-tah-8p03-3lane-2mux-cmd"
#define PANEL_SAMSUNG_EA8079         "samsung_ea8079_taurus"
#define PANEL_SM_S6E3HA8_LION        "sm_s6e3ha8_lion"
#define PANEL_SM_S6E3HC2_ELSA        "sm_s6e3hc2_elsa"
#define PANEL_090_901_6P756          "090_901_6p756"
#define PANEL_BOE_R66458_6P63        "boe_r66458_6p53"
#define PANEL_BOE_NT37700P_6P57      "boe_nt37700p_6p57"
#define PANEL_SAMSUNG_8FC1           "samsung_8fc1_6p3_1080p_video"
#define PANEL_191_206_8P01           "191_206_8p01"
#define PANEL_190_206_6P45           "190_206_6p45"
#define PANEL_190_C00_6P45           "190_c00_6p45"
#define PANEL_191_C00_8P01           "191_c00_8p01"
#define PANEL_350_C00_12P6           "350_c00_12p6"

/*
 * struct
 */
struct lcd_kit_panel_ops {
	int (*lcd_kit_read_project_id)(void);
	int (*lcd_kit_rgbw_set_mode)(struct hisi_fb_data_type *hisifd, int mode);
	int (*lcd_get_2d_barcode)(char *oem_data, struct hisi_fb_data_type *hisifd);
	int (*lcd_kit_set_backlight_by_type)(struct platform_device *pdev,
		int backlight_type, struct timer_list *backlight_second_timer);
	int (*lcd_set_vss_by_thermal)(void *hld);
	int (*lcd_esd_check)(struct hisi_fb_data_type *hisifd);
};

struct lcd_kit_panel_map {
	char *compatible;
	int (*callback)(void);
};

/*
 * function declare
 */
struct lcd_kit_panel_ops *lcd_kit_panel_get_ops(void);
int lcd_kit_panel_init(void);
int lcd_kit_panel_ops_register(struct lcd_kit_panel_ops *ops);
int lcd_kit_panel_ops_unregister(struct lcd_kit_panel_ops *ops);
int lcd_kit_esd_mipi_err_check(void *hld);
#endif
