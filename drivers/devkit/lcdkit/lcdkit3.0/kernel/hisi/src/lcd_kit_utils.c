/*
 * lcd_kit_utils.c
 *
 * lcdkit utils function for lcd driver
 *
 * Copyright (c) 2018-2021 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_utils.h"
#include <linux/hisi/hw_cmdline_parse.h> // for runmode_is_factory
#include "global_ddr_map.h"
#include "hisi_fb.h"
#ifdef CONFIG_HISI_FB_V600
#include "dsc/dsc_algorithm_manager.h"
#endif
#include "lcd_kit_disp.h"
#include "lcd_kit_ext_disp.h"
#include "lcd_kit_common.h"
#include "lcd_kit_power.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_core.h"
#include "lcd_kit_effect.h"
#include "lcd_kit_sysfs_hs.h"
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif
#include "voltage/ina231.h"
#include <linux/ctype.h>
#include <linux/thermal.h>
#include <hiview_hievent.h>

/* fps gamma */
#define GAMMA_C8H_LEN 135
#define GAMMA_C9H_LEN 180
#define GAMMA_B3H_LEN 45
#define GAMMA_TOTAL_LEN (GAMMA_C8H_LEN + GAMMA_C9H_LEN + GAMMA_B3H_LEN)
#define FLASH_GAMMA_TOTAL_LEN_MAX 452
#define FLASH_GAMMA_SINGLE_PARAM_RETRY_MAX 3
#define FLASH_GAMMA_PARAM_RETRYT_MAX 3
#define FLASH_GAMMA_READ_DONE 0x02
#define FLASH_GAMMA_RX_BUF_SIZE 3
/* pcd add detect number */
static uint32_t error_num_pcd = 0;

static const uint32_t fps_normal = 60;
static const uint32_t fps_medium = 90;
static const int default_gamma_band_level = 3;

struct hisi_fb_data_type *dev_get_hisifd(struct device *dev)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!dev) {
		LCD_KIT_ERR("lcd fps scence store dev NULL Pointer\n");
		return hisifd;
	}
	fbi = dev_get_drvdata(dev);
	if (!fbi) {
		LCD_KIT_ERR("lcd fps scence store fbi NULL Pointer\n");
		return hisifd;
	}
	hisifd = (struct hisi_fb_data_type *)fbi->par;
	return hisifd;
}

bool lcd_kit_support(void)
{
	struct device_node *lcdkit_np = NULL;
	const char *support_type = NULL;
	ssize_t ret;

	lcdkit_np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!lcdkit_np) {
		LCD_KIT_ERR("NOT FOUND device node!\n");
		return false;
	}
	ret = of_property_read_string(lcdkit_np, "support_lcd_type", &support_type);
	if (ret) {
		LCD_KIT_ERR("failed to get support_type\n");
		return false;
	}
	if (!strncmp(support_type, "LCD_KIT", strlen("LCD_KIT"))) {
		LCD_KIT_INFO("lcd_kit is support!\n");
		return true;
	}
	LCD_KIT_INFO("lcd_kit is not support!\n");
	return false;
}

static void lcd_kit_orise2x(struct hisi_panel_info *pinfo)
{
	pinfo->ifbc_cmp_dat_rev0 = 1;
	pinfo->ifbc_cmp_dat_rev1 = 0;
	pinfo->ifbc_auto_sel = 0;
}

static void lcd_kit_vesa3_config(struct hisi_panel_info *pinfo)
{
	/* dsc parameter info */
	pinfo->vesa_dsc.bits_per_component = 8;
	pinfo->vesa_dsc.bits_per_pixel = 8;
	pinfo->vesa_dsc.initial_xmit_delay = 512;
	pinfo->vesa_dsc.first_line_bpg_offset = 12;
	pinfo->vesa_dsc.mux_word_size = 48;
	/* DSC_CTRL */
	pinfo->vesa_dsc.block_pred_enable = 1;
	pinfo->vesa_dsc.linebuf_depth = 9;
	/* RC_PARAM3 */
	pinfo->vesa_dsc.initial_offset = 6144;
	/* FLATNESS_QP_TH */
	pinfo->vesa_dsc.flatness_min_qp = 3;
	pinfo->vesa_dsc.flatness_max_qp = 12;
	/* DSC_PARAM4 */
	pinfo->vesa_dsc.rc_edge_factor = 0x6;
	pinfo->vesa_dsc.rc_model_size = 8192;
	/* DSC_RC_PARAM5: 0x330b0b */
	pinfo->vesa_dsc.rc_tgt_offset_lo = (0x330b0b >> 20) & 0xF;
	pinfo->vesa_dsc.rc_tgt_offset_hi = (0x330b0b >> 16) & 0xF;
	pinfo->vesa_dsc.rc_quant_incr_limit1 = (0x330b0b >> 8) & 0x1F;
	pinfo->vesa_dsc.rc_quant_incr_limit0 = (0x330b0b >> 0) & 0x1F;
	/* DSC_RC_BUF_THRESH0: 0xe1c2a38 */
	pinfo->vesa_dsc.rc_buf_thresh0 = (0xe1c2a38 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh1 = (0xe1c2a38 >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh2 = (0xe1c2a38 >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh3 = (0xe1c2a38 >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH1: 0x46546269 */
	pinfo->vesa_dsc.rc_buf_thresh4 = (0x46546269 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh5 = (0x46546269 >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh6 = (0x46546269 >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh7 = (0x46546269 >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH2: 0x7077797b */
	pinfo->vesa_dsc.rc_buf_thresh8 = (0x7077797b >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh9 = (0x7077797b >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh10 = (0x7077797b >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh11 = (0x7077797b >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH3: 0x7d7e0000 */
	pinfo->vesa_dsc.rc_buf_thresh12 = (0x7d7e0000 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh13 = (0x7d7e0000 >> 16) & 0xFF;
	/* DSC_RC_RANGE_PARAM0: 0x1020100 */
	pinfo->vesa_dsc.range_min_qp0 = (0x1020100 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp0 = (0x1020100 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset0 = (0x1020100 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp1 = (0x1020100 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp1 = (0x1020100 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset1 = (0x1020100 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM1: 0x94009be */
	pinfo->vesa_dsc.range_min_qp2 = (0x94009be >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp2 = (0x94009be >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset2 = (0x94009be >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp3 = (0x94009be >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp3 = (0x94009be >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset3 = (0x94009be >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM2, 0x19fc19fa */
	pinfo->vesa_dsc.range_min_qp4 = (0x19fc19fa >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp4 = (0x19fc19fa >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset4 = (0x19fc19fa >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp5 = (0x19fc19fa >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp5 = (0x19fc19fa >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset5 = (0x19fc19fa >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM3, 0x19f81a38 */
	pinfo->vesa_dsc.range_min_qp6 = (0x19f81a38 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp6 = (0x19f81a38 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset6 = (0x19f81a38 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp7 = (0x19f81a38 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp7 = (0x19f81a38 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset7 = (0x19f81a38 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM4, 0x1a781ab6 */
	pinfo->vesa_dsc.range_min_qp8 = (0x1a781ab6 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp8 = (0x1a781ab6 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset8 = (0x1a781ab6 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp9 = (0x1a781ab6 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp9 = (0x1a781ab6 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset9 = (0x1a781ab6 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM5, 0x2af62b34 */
	pinfo->vesa_dsc.range_min_qp10 = (0x2af62b34 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp10 = (0x2af62b34 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset10 = (0x2af62b34 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp11 = (0x2af62b34 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp11 = (0x2af62b34 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset11 = (0x2af62b34 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM6, 0x2b743b74 */
	pinfo->vesa_dsc.range_min_qp12 = (0x2b743b74 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp12 = (0x2b743b74 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset12 = (0x2b743b74 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp13 = (0x2b743b74 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp13 = (0x2b743b74 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset13 = (0x2b743b74 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM7, 0x6bf40000 */
	pinfo->vesa_dsc.range_min_qp14 = (0x6bf40000 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp14 = (0x6bf40000 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset14 = (0x6bf40000 >> 16) & 0x3F;
}

static void lcd_kit_vesa3_75_single_config(struct hisi_panel_info *pinfo)
{
	/* dsc parameter info */
	pinfo->vesa_dsc.bits_per_component = 10;
	pinfo->vesa_dsc.bits_per_pixel = 8;
	pinfo->vesa_dsc.initial_xmit_delay = 512;
	pinfo->vesa_dsc.first_line_bpg_offset = 12;
	pinfo->vesa_dsc.mux_word_size = 48;
	/* DSC_CTRL */
	pinfo->vesa_dsc.block_pred_enable = 1;
	pinfo->vesa_dsc.linebuf_depth = 11;
	/* RC_PARAM3 */
	pinfo->vesa_dsc.initial_offset = 6144;
	/* FLATNESS_QP_TH */
	pinfo->vesa_dsc.flatness_min_qp = 7;
	pinfo->vesa_dsc.flatness_max_qp = 16;
	/* DSC_PARAM4 */
	pinfo->vesa_dsc.rc_edge_factor = 0x6;
	pinfo->vesa_dsc.rc_model_size = 8192;
	/* DSC_RC_PARAM5: 0x330f0f */
	pinfo->vesa_dsc.rc_tgt_offset_lo = (0x330f0f >> 20) & 0xF;
	pinfo->vesa_dsc.rc_tgt_offset_hi = (0x330f0f >> 16) & 0xF;
	pinfo->vesa_dsc.rc_quant_incr_limit1 = (0x330f0f >> 8) & 0x1F;
	pinfo->vesa_dsc.rc_quant_incr_limit0 = (0x330f0f >> 0) & 0x1F;
	/* DSC_RC_BUF_THRESH0: 0xe1c2a38 */
	pinfo->vesa_dsc.rc_buf_thresh0 = (0xe1c2a38 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh1 = (0xe1c2a38 >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh2 = (0xe1c2a38 >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh3 = (0xe1c2a38 >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH1: 0x46546269 */
	pinfo->vesa_dsc.rc_buf_thresh4 = (0x46546269 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh5 = (0x46546269 >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh6 = (0x46546269 >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh7 = (0x46546269 >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH2: 0x7077797b */
	pinfo->vesa_dsc.rc_buf_thresh8 = (0x7077797b >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh9 = (0x7077797b >> 16) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh10 = (0x7077797b >> 8) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh11 = (0x7077797b >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH3: 0x7d7e0000 */
	pinfo->vesa_dsc.rc_buf_thresh12 = (0x7d7e0000 >> 24) & 0xFF;
	pinfo->vesa_dsc.rc_buf_thresh13 = (0x7d7e0000 >> 16) & 0xFF;
	/* DSC_RC_RANGE_PARAM0: 0x2022200 */
	pinfo->vesa_dsc.range_min_qp0 = (0x2022200 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp0 = (0x2022200 >> 22) & 0x1F;;
	pinfo->vesa_dsc.range_bpg_offset0 = (0x2022200 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp1 = (0x2022200 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp1 = (0x2022200 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset1 = (0x2022200 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM1: 0x2a402abe */
	pinfo->vesa_dsc.range_min_qp2 = (0x2a402abe >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp2 = (0x2a402abe >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset2 = (0x2a402abe >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp3 = (0x2a402abe >> 11) & 0x1F;;
	pinfo->vesa_dsc.range_max_qp3 = (0x2a402abe >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset3 = (0x2a402abe >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM2, 0x3afc3afa */
	pinfo->vesa_dsc.range_min_qp4 = (0x3afc3afa >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp4 = (0x3afc3afa >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset4 = (0x3afc3afa >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp5 = (0x3afc3afa >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp5 = (0x3afc3afa >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset5 = (0x3afc3afa >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM3, 0x3af83b38 */
	pinfo->vesa_dsc.range_min_qp6 = (0x3af83b38 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp6 = (0x3af83b38 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset6 = (0x3af83b38 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp7 = (0x3af83b38 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp7 = (0x3af83b38 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset7 = (0x3af83b38 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM4, 0x3b783bb6 */
	pinfo->vesa_dsc.range_min_qp8 = (0x3b783bb6 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp8 = (0x3b783bb6 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset8 = (0x3b783bb6 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp9 = (0x3b783bb6 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp9 = (0x3b783bb6 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset9 = (0x3b783bb6 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM5, 0x4bf64c34 */
	pinfo->vesa_dsc.range_min_qp10 = (0x4bf64c34 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp10 = (0x4bf64c34 >> 22) & 0x1F;;
	pinfo->vesa_dsc.range_bpg_offset10 = (0x4bf64c34 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp11 = (0x4bf64c34 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp11 = (0x4bf64c34 >> 6) & 0x1F;;
	pinfo->vesa_dsc.range_bpg_offset11 = (0x4bf64c34 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM6, 0x4c745c74 */
	pinfo->vesa_dsc.range_min_qp12 = (0x4c745c74 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp12 = (0x4c745c74 >> 22) & 0x1F;;
	pinfo->vesa_dsc.range_bpg_offset12 = (0x4c745c74 >> 16) & 0x3F;
	pinfo->vesa_dsc.range_min_qp13 = (0x4c745c74 >> 11) & 0x1F;
	pinfo->vesa_dsc.range_max_qp13 = (0x4c745c74 >> 6) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset13 = (0x4c745c74 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM7, 0x8cf40000 */
	pinfo->vesa_dsc.range_min_qp14 = (0x8cf40000 >> 27) & 0x1F;
	pinfo->vesa_dsc.range_max_qp14 = (0x8cf40000 >> 22) & 0x1F;
	pinfo->vesa_dsc.range_bpg_offset14 = (0x8cf40000 >> 16) & 0x3F;
}

static void lcd_kit_vesa3x_single(struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	lcd_kit_vesa3_config(pinfo);
}

static void lcd_kit_vesa3x_dual(struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	lcd_kit_vesa3_config(pinfo);
}

static void lcd_kit_vesa3_75x_single(struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	lcd_kit_vesa3_75_single_config(pinfo);
}

static void lcd_kit_vesa3_75x_dual(struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	lcd_kit_vesa3_75_single_config(pinfo);
}
static void lcd_kit_dsc_config(struct device_node *np,
	struct hisi_panel_info *pinfo)
{
#ifdef CONFIG_HISI_FB_V600
	struct dsc_algorithm_manager *pt = get_dsc_algorithm_manager_instance();
	struct input_dsc_info panel_input_dsc_info;

	if (!pinfo || !pt || !pt->vesa_dsc_info_calc) {
		LCD_KIT_ERR("pt is null\n");
		return;
	}

	memset(&panel_input_dsc_info, 0, sizeof(struct input_dsc_info));

	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-version",
		&panel_input_dsc_info.dsc_version, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-format",
		&panel_input_dsc_info.format, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-bpp",
		&panel_input_dsc_info.dsc_bpp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-bpc",
		&panel_input_dsc_info.dsc_bpc, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-blk-pred-en",
		&panel_input_dsc_info.block_pred_enable, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-line-buff-depth",
		&panel_input_dsc_info.linebuf_depth, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-gen-rc-params",
		&panel_input_dsc_info.gen_rc_params, 0);
	panel_input_dsc_info.pic_width = pinfo->xres;
	panel_input_dsc_info.pic_height = pinfo->yres;
	panel_input_dsc_info.slice_width = pinfo->vesa_dsc.slice_width + 1;
	panel_input_dsc_info.slice_height = pinfo->vesa_dsc.slice_height + 1;
	pt->vesa_dsc_info_calc(&panel_input_dsc_info,
		&(pinfo->panel_dsc_info.dsc_info), NULL);
#endif
}
void lcd_kit_compress_config(struct device_node *np,
	int mode, struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	switch (mode) {
	case IFBC_TYPE_ORISE2X:
		lcd_kit_orise2x(pinfo);
		break;
	case IFBC_TYPE_VESA3X_SINGLE:
		if (!disp_info->calc_mode)
			lcd_kit_vesa3x_single(pinfo);
		else
			lcd_kit_dsc_config(np, pinfo);
		break;
	case IFBC_TYPE_VESA3_75X_DUAL:
		lcd_kit_vesa3_75x_dual(pinfo);
		break;
	case IFBC_TYPE_VESA3_75X_SINGLE:
		lcd_kit_vesa3_75x_single(pinfo);
		break;
	case IFBC_TYPE_VESA3X_DUAL:
		if (!disp_info->calc_mode)
			lcd_kit_vesa3x_dual(pinfo);
		else
			lcd_kit_dsc_config(np, pinfo);
		break;
	case IFBC_TYPE_VESA4X_SINGLE_SPR:
	case IFBC_TYPE_VESA4X_DUAL_SPR:
	case IFBC_TYPE_VESA2X_SINGLE_SPR:
	case IFBC_TYPE_VESA2X_DUAL_SPR:
		lcd_kit_dsc_config(np, pinfo);
		break;
	case IFBC_TYPE_NONE:
		break;
	default:
		LCD_KIT_ERR("not support compress mode:%d\n", mode);
		break;
	}
}

int lcd_kit_lread_reg(void *pdata, uint32_t *out,
	struct lcd_kit_dsi_cmd_desc *cmds, uint32_t len)
{
	int ret;
	char __iomem *dsi_base_addr = NULL;
	struct dsi_cmd_desc lcd_reg_cmd;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = (struct hisi_fb_data_type *)pdata;
	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	dsi_base_addr = lcd_kit_get_dsi_base_addr(hisifd);
	lcd_reg_cmd.dtype = cmds->dtype;
	lcd_reg_cmd.vc = cmds->vc;
	lcd_reg_cmd.wait = cmds->wait;
	lcd_reg_cmd.waittype = cmds->waittype;
	lcd_reg_cmd.dlen = cmds->dlen;
	lcd_reg_cmd.payload = cmds->payload;
	ret = mipi_dsi_lread_reg(out, &lcd_reg_cmd, len, dsi_base_addr);
	if (ret) {
		LCD_KIT_INFO("read error, ret=%d\n", ret);
		return ret;
	}
	return ret;
}

int lcd_kit_rgbw_set_mode(struct hisi_fb_data_type *hisifd, int mode)
{
	int ret = LCD_KIT_OK;
	static int old_rgbw_mode;
	int rgbw_mode = hisifd->de_info.ddic_rgbw_mode;
	struct lcd_kit_panel_ops *panel_ops = NULL;

	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_kit_rgbw_set_mode) {
		ret = panel_ops->lcd_kit_rgbw_set_mode(hisifd, hisifd->de_info.ddic_rgbw_mode);
	} else if (rgbw_mode != old_rgbw_mode) {
		switch (mode) {
		case RGBW_SET1_MODE:
			ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->rgbw.mode1_cmds);
			break;
		case RGBW_SET2_MODE:
			ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->rgbw.mode2_cmds);
			break;
		case RGBW_SET3_MODE:
			ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->rgbw.mode3_cmds);
			break;
		case RGBW_SET4_MODE:
			ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->rgbw.mode4_cmds);
			break;
		default:
			HISI_FB_ERR("mode err: %d\n", hisifd->de_info.ddic_rgbw_mode);
			ret = LCD_KIT_FAIL;
			break;
		}
	}
	LCD_KIT_DEBUG("[RGBW]rgbw_mode=%d,rgbw_mode_old=%d!\n", rgbw_mode, old_rgbw_mode);
	old_rgbw_mode = rgbw_mode;
	return ret;
}

int lcd_kit_rgbw_set_backlight(struct hisi_fb_data_type *hisifd,
	uint32_t bl_level)
{
	int ret;

	/* change bl level to dsi cmds */
	disp_info->rgbw.backlight_cmds.cmds[0].payload[1] = (bl_level >> 8) & 0xff;
	disp_info->rgbw.backlight_cmds.cmds[0].payload[2] = bl_level & 0xff;
	ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->rgbw.backlight_cmds);
	return ret;
}

static int lcd_kit_rgbw_pix_gain(struct hisi_fb_data_type *hisifd)
{
	uint32_t pix_gain;
	static uint32_t pix_gain_old;
	int rgbw_mode;
	int ret = LCD_KIT_OK;

	if (disp_info->rgbw.pixel_gain_limit_cmds.cmds == NULL) {
		LCD_KIT_DEBUG("not support pixel_gain_limit\n");
		return LCD_KIT_OK;
	}
	rgbw_mode = hisifd->de_info.ddic_rgbw_mode;
	pix_gain = (uint32_t)hisifd->de_info.pixel_gain_limit;
	if ((pix_gain != pix_gain_old) && (rgbw_mode == RGBW_SET4_MODE)) {
		disp_info->rgbw.pixel_gain_limit_cmds.cmds[0].payload[1] = pix_gain;
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->rgbw.pixel_gain_limit_cmds);
		LCD_KIT_DEBUG("[RGBW] pixel_gain=%d,pix_gain_old=%d!\n",
			pix_gain, pix_gain_old);
		pix_gain_old = pix_gain;
	}
	return ret;
}

int lcd_kit_rgbw_set_handle(struct hisi_fb_data_type *hisifd)
{
	int ret;
	static int old_rgbw_backlight;
	int rgbw_backlight;
	int rgbw_bl_level;

	/* set mode */
	ret = lcd_kit_rgbw_set_mode(hisifd, hisifd->de_info.ddic_rgbw_mode);
	if (ret) {
		LCD_KIT_ERR("[RGBW]set mode fail\n");
		return LCD_KIT_FAIL;
	}

	/* set backlight */
	rgbw_backlight = hisifd->de_info.ddic_rgbw_backlight;
	if (disp_info->rgbw.backlight_cmds.cmds &&
		(hisifd->bl_level && (hisifd->backlight.bl_level_old != 0)) &&
		(rgbw_backlight != old_rgbw_backlight)) {
		rgbw_bl_level = rgbw_backlight * disp_info->rgbw.rgbw_bl_max /
			hisifd->panel_info.bl_max;
		ret = lcd_kit_rgbw_set_backlight(hisifd, rgbw_bl_level);
		if (ret) {
			LCD_KIT_ERR("[RGBW]set backlight fail\n");
			return LCD_KIT_FAIL;
		}
	}
	old_rgbw_backlight = rgbw_backlight;

	/* set gain */
	ret = lcd_kit_rgbw_pix_gain(hisifd);
	if (ret) {
		LCD_KIT_INFO("[RGBW]set pix_gain fail\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_flash_gamma_checksum(uint8_t gamma[], uint32_t gamma_len, uint32_t length)
{
	uint32_t i;
	uint32_t sum_calc = 0;
	uint32_t sum_read;
	if (gamma_len < length - 1)
		return LCD_KIT_FAIL;
	// checksum value is saved in the last two param of flash gamma
	sum_read = ((uint32_t)gamma[length - 2] << 8) + gamma[length - 1];
	// checksum rule: sum index from 0 to (flash_gamma_total_length - 3)
	for (i = 0; i <= length - 3; i++)
		sum_calc += gamma[i];
	LCD_KIT_INFO("sum_calc = %u, sum_read = %u", sum_calc, sum_read);
	return sum_calc == sum_read ? LCD_KIT_OK : LCD_KIT_FAIL;
}

static int lcd_kit_save_fps_gamma(struct lcd_kit_dsi_panel_cmds *cmd,
	uint8_t *gamma, uint32_t length)
{
	// c8h, c9h and b3h params are stored in cmds of index 1, 2, and 4
	// as a result, cmd_cnt should not be less than 4
	if (cmd == NULL || cmd->cmd_cnt < 4 || length < GAMMA_TOTAL_LEN) {
		LCD_KIT_ERR("cmds is NULL || cmds->cmd_cnt<4 ||"
		" length < GAMMA_TOTAL_LEN!");
		return LCD_KIT_FAIL;
	}
	// the first byte of payload stores target regiter
	memcpy(cmd->cmds[1].payload + 1, gamma, GAMMA_C8H_LEN);
	memcpy(cmd->cmds[2].payload + 1, gamma + GAMMA_C8H_LEN, GAMMA_C9H_LEN);
	memcpy(cmd->cmds[4].payload + 1,
		gamma + GAMMA_C8H_LEN + GAMMA_C9H_LEN, GAMMA_B3H_LEN);
	return LCD_KIT_OK;
}

static int lcd_kit_read_fps_gamma_60hz(void *hld,
	struct lcd_kit_adapt_ops *adapt_ops)
{
	uint8_t gamma_60hz[GAMMA_TOTAL_LEN] = {0};
	// read otp gamma
	if (adapt_ops->mipi_rx(hld, gamma_60hz, GAMMA_TOTAL_LEN - 1,
		&disp_info->fps_gamma.read_otp)) {
		LCD_KIT_ERR("60 hz mipi_rx failed!\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_kit_save_fps_gamma(
		&(disp_info->fps_gamma.write_60hz),
		gamma_60hz, GAMMA_TOTAL_LEN) != LCD_KIT_OK)
		return LCD_KIT_FAIL;
	return LCD_KIT_OK;
}

static int lcd_kit_read_fps_gamma_from_flash_single_param(uint32_t index,
	void *hld, struct lcd_kit_adapt_ops *adapt_ops, uint8_t param[])
{
	uint32_t retry_count;
	// cmds[1].payload[2,3]: gamma address; payload[2]: high 8, [3]: low 8
	disp_info->fps_gamma.read_flash.cmds[1].payload[2] = (index >> 8) & 0xF;
	disp_info->fps_gamma.read_flash.cmds[1].payload[3] = index & 0xFF;
	for (retry_count = 0; retry_count < FLASH_GAMMA_SINGLE_PARAM_RETRY_MAX;
		retry_count++) {
		if (adapt_ops->mipi_rx(hld, param, FLASH_GAMMA_RX_BUF_SIZE - 1,
			&disp_info->fps_gamma.read_flash) == LCD_KIT_OK &&
			param[0] == FLASH_GAMMA_READ_DONE) {
			LCD_KIT_DEBUG("addr 0x%02X%02X, param 0x%02X 0x%02X\n",
				disp_info->fps_gamma.read_flash.cmds[1].payload[2],
				disp_info->fps_gamma.read_flash.cmds[1].payload[3],
				param[0], param[1]);
			return LCD_KIT_OK;
		}
	}
	LCD_KIT_ERR("read failed! address 0x%02X%02X, param 0x%02X 0x%02X\n",
		disp_info->fps_gamma.read_flash.cmds[1].payload[2],
		disp_info->fps_gamma.read_flash.cmds[1].payload[3],
		param[0], param[1]);
	return LCD_KIT_FAIL;
}

static int lcd_kit_read_fps_gamma_from_flash(void *hld,
	struct lcd_kit_adapt_ops *adapt_ops, uint8_t flash_gamma_checksum[],
	uint32_t checksum_len, uint32_t length)
{
	int index;
	uint8_t flash_single_param[FLASH_GAMMA_RX_BUF_SIZE] = {0};
	if (checksum_len <  length - 1)
		return LCD_KIT_FAIL;
	for (index = 0; index <= length - 1; index++) {
		// read params one by one
		if (lcd_kit_read_fps_gamma_from_flash_single_param(index, hld,
			adapt_ops, flash_single_param) != LCD_KIT_OK)
			return LCD_KIT_FAIL;
		// flash_single_param: [0] read done flag, [1] param
		flash_gamma_checksum[index] = flash_single_param[1];
	}
	return LCD_KIT_OK;
}

static bool lcd_kit_is_need_update_check_sum(struct hisi_fb_data_type *hisifd)
{
	if (disp_info->fps_gamma.support == 0) {
		LCD_KIT_DEBUG("no need to update check_sum due to support");
		return false;
	}
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL!");
		return false;
	}
	if (disp_info->panel_version.support == 0) {
		LCD_KIT_ERR("panel_version.support == 0");
		return false;
	}
	if (disp_info->fps_gamma.check_length_update_support == 0) {
		LCD_KIT_ERR("panel_version.support == 0");
		return false;
	}
	if (strcmp(hisifd->panel_info.lcd_panel_version, " VER:V3") == 0) {
		LCD_KIT_INFO("no need to update check_sum due to panel_version");
		return false;
	}
	LCD_KIT_INFO("panel ver is %s\n", hisifd->panel_info.lcd_panel_version);
	return true;
}

static int lcd_kit_read_fps_gamma_90hz(void *hld,
	struct lcd_kit_adapt_ops *adapt_ops)
{
	int count;
	uint8_t flash_gamma_checksum[FLASH_GAMMA_TOTAL_LEN_MAX] = {0};
	if (lcd_kit_is_need_update_check_sum(hld)) {
		// 452 is real gama length for sepcial panel version
		disp_info->fps_gamma.flash_gamma_total_length = 452;
		LCD_KIT_INFO("update check_sum, gama_length is %d",
			disp_info->fps_gamma.flash_gamma_total_length);
	}
	for (count = 0; count < FLASH_GAMMA_PARAM_RETRYT_MAX; count++) {
		// read
		if (lcd_kit_read_fps_gamma_from_flash(hld, adapt_ops,
			flash_gamma_checksum,
			FLASH_GAMMA_TOTAL_LEN_MAX,
			disp_info->fps_gamma.flash_gamma_total_length) !=
			LCD_KIT_OK)
			// read single param failed, return directly
			return LCD_KIT_FAIL;
		// checksum
		if (lcd_kit_flash_gamma_checksum(flash_gamma_checksum,
			FLASH_GAMMA_TOTAL_LEN_MAX,
			disp_info->fps_gamma.flash_gamma_total_length) !=
			LCD_KIT_OK) {
			// checksum failed, retry
			LCD_KIT_ERR("checksum failed %d time!", count);
		} else {
			// succeed, save params
			if (lcd_kit_save_fps_gamma(
				&(disp_info->fps_gamma.write_90hz),
				flash_gamma_checksum, GAMMA_TOTAL_LEN) !=
				LCD_KIT_OK)
				return LCD_KIT_FAIL;
			return LCD_KIT_OK;
		}
	}
	LCD_KIT_ERR("read failed!");
	return LCD_KIT_FAIL;
}

int lcd_kit_read_fps_gamma(void *hld)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	if (!disp_info->fps_gamma.support) {
		LCD_KIT_DEBUG("fps_gamma not support\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_INFO("lcd_kit_read_fps_gamma\n");
	// init check_flag
	disp_info->fps_gamma.check_flag = 0;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	// otp gamma read
	if (lcd_kit_read_fps_gamma_60hz(hld, adapt_ops) != LCD_KIT_OK) {
		LCD_KIT_ERR("60 hz mipi_rx failed!\n");
		return LCD_KIT_FAIL;
	}
	// flash gamma read enter
	ret = adapt_ops->mipi_tx(hld, &disp_info->fps_gamma.read_flash_enter);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("90hz enter mipi_tx failed! ret = %d", ret);
		return LCD_KIT_FAIL;
	}
	// flash gamma read
	if (lcd_kit_read_fps_gamma_90hz(hld, adapt_ops) != LCD_KIT_OK) {
		LCD_KIT_ERR("90hz read failed!\n");
		return LCD_KIT_FAIL;
	}
	// flash gamma read exit
	ret = adapt_ops->mipi_tx(hld, &disp_info->fps_gamma.read_flash_exit);
	if (ret != LCD_KIT_OK)
		LCD_KIT_WARNING("read_flash_exit failed! ret = %d\n", ret);

	// all done, set check_flag
	disp_info->fps_gamma.check_flag = 1;
	return LCD_KIT_OK;
}

static void lcd_kit_print_fps_gamma(struct lcd_kit_dsi_panel_cmds *cmd)
{
	int i;
	if (lcd_kit_msg_level >= MSG_LEVEL_DEBUG) {
		// the first byte of payload stores target regiter
		for (i = 0; i < GAMMA_C8H_LEN; i++)
			LCD_KIT_DEBUG("c8h[%d] = 0x%02X", i,
				*(cmd->cmds[1].payload + 1 + i));
		for (i = 0; i < GAMMA_C9H_LEN; i++)
			LCD_KIT_DEBUG("c9h[%d] = 0x%02X", i,
				*(cmd->cmds[2].payload + 1 + i));
		for (i = 0; i < GAMMA_B3H_LEN; i++)
			LCD_KIT_DEBUG("b3h[%d] = 0x%02X", i,
				*(cmd->cmds[4].payload + 1 + i));
	}
}

static int lcd_kit_write_fps_gamma_by_scene_no_lock(
	struct hisi_fb_data_type *hisifd, const int index)
{
	int ret = LCD_KIT_OK;
	if (!disp_info->fps_gamma.support) {
		LCD_KIT_DEBUG("fps_gamma not support");
		return LCD_KIT_OK;
	}
	if (!disp_info->fps_gamma.check_flag) {
		LCD_KIT_ERR("fps_gamma check_flag failed!");
		return LCD_KIT_FAIL;
	}
	if (index == LCD_FPS_SCENCE_60P) {
		ret = lcd_kit_dsi_cmds_tx_no_lock(hisifd,
			&(disp_info->fps_gamma.write_60hz));
		lcd_kit_print_fps_gamma(&(disp_info->fps_gamma.write_60hz));
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("60Hz write faild! ret = %d\n", ret);
	}
	if (index == LCD_FPS_SCENCE_90) {
		ret = lcd_kit_dsi_cmds_tx_no_lock(hisifd,
			&(disp_info->fps_gamma.write_90hz));
		lcd_kit_print_fps_gamma(&(disp_info->fps_gamma.write_90hz));
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("90Hz write faild! ret = %d\n", ret);
	}
	return ret;
}

int lcd_kit_write_fps_gamma_by_scene(
	struct hisi_fb_data_type *hisifd, const int index)
{
	int ret = LCD_KIT_OK;
	if (!disp_info->fps_gamma.support) {
		LCD_KIT_DEBUG("fps_gamma not support");
		return LCD_KIT_OK;
	}
	if (!disp_info->fps_gamma.check_flag) {
		LCD_KIT_ERR("fps_gamma check_flag failed!");
		return LCD_KIT_FAIL;
	}
	if (index == LCD_FPS_SCENCE_60P) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&(disp_info->fps_gamma.write_60hz));
		lcd_kit_print_fps_gamma(&(disp_info->fps_gamma.write_60hz));
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("60Hz write faild! ret = %d\n", ret);
	}
	if (index == LCD_FPS_SCENCE_90) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&(disp_info->fps_gamma.write_90hz));
		lcd_kit_print_fps_gamma(&(disp_info->fps_gamma.write_90hz));
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("90Hz write faild! ret = %d\n", ret);
	}
	return ret;
}

static bool lcd_kit_get_hbm_status(struct hisi_fb_data_type *hisifd)
{
	int ret = false;

	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return ret;
	}
	if (hisifd->de_info.hbm_level > 0)
		ret = true;

	if (common_info->hbm.hbm_if_fp_is_using == 1)
		ret = true;

	return ret;
}

static bool lcd_kit_get_ic_dim_status(struct hisi_fb_data_type *hisifd)
{
	bool ret = false;

	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return ret;
	}
	if (hisifd->de_info.hbm_dimming)
		ret = true;

	if (common_info->hbm.hbm_if_fp_is_using == 1)
		ret = false;

	return ret;
}

static void lcd_kit_get_dfr_hbm_combined_cmds(struct hisi_fb_data_type *hisifd,
	char fps_scence, struct lcd_kit_dsi_panel_cmds **cmds)
{
	bool hbm_status = lcd_kit_get_hbm_status(hisifd);
	bool hbm_dimming = lcd_kit_get_ic_dim_status(hisifd);

	if (cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_dsi_panel_cmds is null");
		return;
	}
	LCD_KIT_INFO("get dfr hbm cmd, scence:%d, hbm_status:%d, dimming:%d\n",
		fps_scence, hbm_status, hbm_dimming);
	if (fps_scence == LCD_FPS_SCENCE_60P) {
		if (!hbm_status && hbm_dimming)
			*cmds = &common_info->dfr_info.cmds[FPS_60P_NORMAL_DIM];
		else if (hbm_status && !hbm_dimming)
			*cmds = &common_info->dfr_info.cmds[FPS_60P_HBM_NO_DIM];
		else if (hbm_status && hbm_dimming)
			*cmds = &common_info->dfr_info.cmds[FPS_60P_HBM_DIM];
	} else if (fps_scence == LCD_FPS_SCENCE_90) {
		if (!hbm_status && hbm_dimming)
			*cmds = &common_info->dfr_info.cmds[FPS_90_NORMAL_DIM];
		else if (hbm_status && !hbm_dimming)
			*cmds = &common_info->dfr_info.cmds[FPS_90_HBM_NO_DIM];
		else if (hbm_status && hbm_dimming)
			*cmds = &common_info->dfr_info.cmds[FPS_90_HBM_DIM];
	}
}

static bool lcd_kit_is_need_tx_cmd(struct hisi_panel_info *pinfo)
{
	if (disp_info->fps.hfps_switch_no_need_cmd != FPS_UPDATE_ENABLE)
		return true;
	if (pinfo == NULL)
		return true;
	if ((pinfo->fps == LCD_FPS_60_HIGH && pinfo->fps_updt== LCD_FPS_90) ||
		(pinfo->fps == LCD_FPS_90 && pinfo->fps_updt== LCD_FPS_60_HIGH))
		return false;
	return true;
}

static int lcd_kit_updt_panel_fps(struct hisi_fb_data_type *hisifd, char fps_scence)
{
	int ret = 0;
	int index;
	struct hisi_panel_info *pinfo = NULL;
	struct lcd_kit_dsi_panel_cmds *cmds = NULL;
	pinfo = &(hisifd->panel_info);
	if ((fps_scence >= LCD_FPS_SCENCE_MAX) ||
		(fps_scence < LCD_FPS_SCENCE_60)) {
		LCD_KIT_ERR("fps_scence %d is error\n", fps_scence);
		return LCD_KIT_FAIL;
	}
	index = fps_scence;
	cmds = &disp_info->fps.fps_to_cmds[index];
	if (common_info->dfr_info.fps_lock_command_support)
		lcd_kit_get_dfr_hbm_combined_cmds(hisifd, fps_scence, &cmds);
	if (lcd_kit_is_need_tx_cmd(pinfo))
		ret = lcd_kit_dsi_cmds_tx_no_lock(hisifd, cmds);
	else
		LCD_KIT_INFO("fps switch no need send cmd!\n");
	disp_info->fps.last_update_fps = pinfo->fps_updt;
	LCD_KIT_INFO("sending cmds fps:%d\n",disp_info->fps.last_update_fps);
	lcd_kit_write_fps_gamma_by_scene_no_lock(hisifd, index);
	if ((index >= LCD_FPS_SCENCE_MAX) ||
		(index < LCD_FPS_SCENCE_60)) {
		LCD_KIT_ERR("index %d is error\n", index);
		return LCD_KIT_FAIL;
	}
	if (index == LCD_FPS_SCENCE_60) {
		pinfo->fps_updt_support = FPS_UPDATE_DISABLE;
		pinfo->fps_updt_panel_only = FPS_UPDATE_DISABLE;
	}
	if (pinfo->fps_updt_force_update) {
		LCD_KIT_INFO("set fps_updt_force_update = 0\n");
		pinfo->fps_updt_force_update = FPS_UPDATE_DISABLE;
	}
	LCD_KIT_INFO("send cmds updt panel fps result %d, scence:%d, index:%d\n",
		ret, fps_scence, index);
	if (common_info->dfr_info.fps_lock_command_support) {
		common_info->dfr_info.fps_dfr_status = FPS_DFR_STATUS_IDLE;
		wake_up_interruptible(&common_info->dfr_info.hbm_wait);
	}
	return ret;
}

uint8_t g_last_fps_scence = LCD_FPS_SCENCE_NORMAL;

static int lcd_kit_updt_low_fps(struct hisi_fb_data_type *hisifd,
	struct hisi_panel_info *pinfo)
{
	int ret = LCD_KIT_OK;

	if (!pinfo) {
		LCD_KIT_ERR();
		return LCD_KIT_FAIL;
	}
	switch (pinfo->fps_scence) {
	case LCD_FPS_SCENCE_FUNC_DEFAULT_DISABLE:
		ret = lcd_kit_dsi_cmds_tx_no_lock(hisifd,
			&disp_info->fps.fps_to_60_cmds);
		pinfo->fps_updt_support = 0;
		pinfo->fps_updt_panel_only = 0;
		g_last_fps_scence = LCD_FPS_SCENCE_NORMAL;
		break;
	case LCD_FPS_SCENCE_FUNC_DEFAULT_ENABLE:
		ret = lcd_kit_dsi_cmds_tx_no_lock(hisifd,
			&disp_info->fps.dfr_enable_cmds);
		g_last_fps_scence = LCD_FPS_SCENCE_FUNC_DEFAULT_ENABLE;
		break;
	case LCD_FPS_SCENCE_FORCE_30FPS:
		ret = lcd_kit_dsi_cmds_tx_no_lock(hisifd,
			&disp_info->fps.fps_to_30_cmds);
		g_last_fps_scence = LCD_FPS_SCENCE_FORCE_30FPS;
		break;
	default:
		break;
	}
	if (pinfo->fps_updt_force_update) {
		LCD_KIT_INFO("set fps_updt_force_update = 0\n");
		pinfo->fps_updt_force_update = 0;
	}
	return ret;
}

int lcd_kit_updt_fps(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (!pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR();
		return LCD_KIT_FAIL;
	}
	if (lcd_is_support_dfr()) {
		ret = lcd_kit_updt_panel_fps(hisifd, pinfo->fps_scence);
		if (ret)
			LCD_KIT_ERR("lcd_kit_updt_panel_fps error\n");
		return ret;
	}
	if (pinfo->fps_scence == g_last_fps_scence) {
		LCD_KIT_DEBUG("scence is same and needn't send fps cmds\n");
		return ret;
	}
	ret = lcd_kit_updt_low_fps(hisifd, pinfo);
	if (ret)
		LCD_KIT_ERR("lcd_kit_updt_low_fps error\n");
	return ret;
}

void lcd_kit_fps_scence_set(struct hisi_panel_info *pinfo, uint32_t scence)
{
	switch (scence) {
	case LCD_FPS_SCENCE_NORMAL:
		pinfo->fps_updt = LCD_FPS_60;
		break;
	case LCD_FPS_SCENCE_IDLE:
		pinfo->fps_updt = LCD_FPS_30;
		break;
	case LCD_FPS_SCENCE_FORCE_30FPS:
		pinfo->fps_updt_support = 1;
		pinfo->fps_updt_panel_only = 1;
		pinfo->fps_updt = LCD_FPS_30;
		pinfo->fps_updt_force_update = 1;
		pinfo->fps_scence = scence;
		break;
	case LCD_FPS_SCENCE_FUNC_DEFAULT_ENABLE:
		pinfo->fps_updt_support = 1;
		pinfo->fps_updt_panel_only = 0;
		pinfo->fps_updt = LCD_FPS_60;
		pinfo->fps_updt_force_update = 1;
		pinfo->fps_scence = scence;
		break;
	case LCD_FPS_SCENCE_FUNC_DEFAULT_DISABLE:
		pinfo->fps_updt_force_update = 1;
		pinfo->fps_updt = LCD_FPS_60;
		pinfo->fps_scence = scence;
		break;
	default:
		pinfo->fps_updt = LCD_FPS_60;
		break;
	}
}

void lcd_kit_fps_updt_porch(struct hisi_panel_info *pinfo, uint32_t scence)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	switch (scence) {
	case LCD_KIT_FPS_SCENCE_IDLE:
		pinfo->ldi_updt.h_back_porch = disp_info->fps.low_frame_porch.buf[0];
		pinfo->ldi_updt.h_front_porch = disp_info->fps.low_frame_porch.buf[1];
		pinfo->ldi_updt.h_pulse_width = disp_info->fps.low_frame_porch.buf[2];
		pinfo->ldi_updt.v_back_porch = disp_info->fps.low_frame_porch.buf[3];
		pinfo->ldi_updt.v_front_porch = disp_info->fps.low_frame_porch.buf[4];
		pinfo->ldi_updt.v_pulse_width = disp_info->fps.low_frame_porch.buf[5];
		break;
	case LCD_KIT_FPS_SCENCE_EBOOK:
		pinfo->ldi_updt.h_back_porch = disp_info->fps.low_frame_porch.buf[0];
		pinfo->ldi_updt.h_front_porch = disp_info->fps.low_frame_porch.buf[1];
		pinfo->ldi_updt.h_pulse_width = disp_info->fps.low_frame_porch.buf[2];
		pinfo->ldi_updt.v_back_porch = disp_info->fps.low_frame_porch.buf[3];
		pinfo->ldi_updt.v_front_porch = disp_info->fps.low_frame_porch.buf[4];
		pinfo->ldi_updt.v_pulse_width = disp_info->fps.low_frame_porch.buf[5];
		break;
	default:
		pinfo->ldi_updt.h_back_porch = disp_info->fps.normal_frame_porch.buf[0];
		pinfo->ldi_updt.h_front_porch = disp_info->fps.normal_frame_porch.buf[1];
		pinfo->ldi_updt.h_pulse_width = disp_info->fps.normal_frame_porch.buf[2];
		pinfo->ldi_updt.v_back_porch = disp_info->fps.normal_frame_porch.buf[3];
		pinfo->ldi_updt.v_front_porch = disp_info->fps.normal_frame_porch.buf[4];
		pinfo->ldi_updt.v_pulse_width = disp_info->fps.normal_frame_porch.buf[5];
		break;
	}
}


static int lcd_get_dual_cmd_by_type(unsigned char type,
	struct lcd_kit_dsi_panel_cmds **cmds0,
	struct lcd_kit_dsi_panel_cmds **cmds1)
{
	switch (type) {
	case LCD_DEMURA_WRITE_PREPARE:
		*cmds0 = &(disp_info->demura.d0_w_pre_cmds);
		*cmds1 = &(disp_info->demura.d1_w_pre_cmds);
		break;
	case LCD_DEMURA_WRITE_FIRST:
		*cmds0 = &(disp_info->demura.d0_w_fir_cmds);
		*cmds1 = &(disp_info->demura.d1_w_fir_cmds);
		break;
	case LCD_DEMURA_WRITE_CONTINUE:
		*cmds0 = &(disp_info->demura.d0_w_con_cmds);
		*cmds1 = &(disp_info->demura.d1_w_con_cmds);
		break;
	case LCD_DEMURA_WRITE_END:
		*cmds0 = &(disp_info->demura.d0_w_end_cmds);
		*cmds1 = &(disp_info->demura.d1_w_end_cmds);
		break;
	case LCD_DEMURA_WRITE_IRDROP_PREPARE:
		*cmds0 = &(disp_info->demura.d0_w_ird_pre_cmds);
		*cmds1 = &(disp_info->demura.d1_w_ird_pre_cmds);
		break;
	case LCD_DEMURA_WRITE_IRDROP:
		*cmds0 = &(disp_info->demura.d0_w_ird_cmds);
		*cmds1 = &(disp_info->demura.d1_w_ird_cmds);
		break;
	case LCD_DEMURA_WRITE_IRDROP_END:
		*cmds0 = &(disp_info->demura.d0_w_ird_end_cmds);
		*cmds1 = &(disp_info->demura.d1_w_ird_end_cmds);
		break;
	default:
		LCD_KIT_ERR("[DEMURA]invalid type\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_upt_payload_and_send(struct hisi_fb_data_type *hisifd,
	struct lcd_kit_dsi_panel_cmds *cmds0,
	struct lcd_kit_dsi_panel_cmds *cmds1,
	unsigned char type, const demura_set_info_t *info)
{
	int ret;
	char *payload0 = NULL;
	char *payload1 = NULL;
	char *temp_payload0 = NULL;
	char *temp_payload1 = NULL;
	char dlen0;
	char dlen1;

	dlen0 = cmds0->cmds->dlen;
	payload0 = kzalloc((info->len0 + dlen0), GFP_KERNEL);
	if (!payload0) {
		LCD_KIT_ERR("[DEMURA]alloc payload0 fail\n");
		return LCD_KIT_FAIL;
	}
	memcpy(payload0, cmds0->cmds->payload, dlen0);
	memcpy(payload0 + dlen0, info->data0, info->len0);
	temp_payload0 = cmds0->cmds->payload;
	cmds0->cmds->payload = payload0;
	cmds0->cmds->dlen = info->len0 + dlen0;
	dlen1 = cmds1->cmds->dlen;
	payload1 = kzalloc((info->len1 + dlen1), GFP_KERNEL);
	if (!payload1) {
		LCD_KIT_ERR("[DEMURA]alloc payload1 fail\n");
		cmds0->cmds->payload = temp_payload0;
		cmds0->cmds->dlen = dlen0;
		kfree(payload0);
		return LCD_KIT_FAIL;
	}
	memcpy(payload1, cmds1->cmds->payload, dlen1);
	memcpy(payload1 + dlen1, info->data1, info->len1);
	temp_payload1 = cmds1->cmds->payload;
	cmds1->cmds->payload = payload1;
	cmds1->cmds->dlen = info->len1 + dlen1;
	ret = lcd_kit_dsi_diff_cmds_tx(hisifd, cmds0, cmds1);
	if (ret)
		LCD_KIT_ERR("[DEMURA]lcd_kit_dsi_diff_cmds_tx fail\n");
	cmds0->cmds->payload = temp_payload0;
	cmds0->cmds->dlen = dlen0;
	kfree(payload0);
	cmds1->cmds->payload = temp_payload1;
	cmds1->cmds->dlen = dlen1;
	kfree(payload1);
	return ret;
}

int lcd_set_demura_handle(struct hisi_fb_data_type *hisifd,
	unsigned char type, const demura_set_info_t *info)
{
	struct lcd_kit_dsi_panel_cmds *cmds0 = NULL;
	struct lcd_kit_dsi_panel_cmds *cmds1 = NULL;
	int ret;

	if ((!hisifd) || (!info)) {
		LCD_KIT_ERR("[DEMURA]invalid input param\n");
		return LCD_KIT_FAIL;
	}
	if ((info->len0 > WRITE_MAX_LEN) || (info->len1 > WRITE_MAX_LEN)) {
		LCD_KIT_ERR("[DEMURA]invalid len, max = %d\n", WRITE_MAX_LEN);
		return LCD_KIT_FAIL;
	}
	ret = lcd_get_dual_cmd_by_type(type, &cmds0, &cmds1);
	if (ret)
		return ret;
	if ((info->len0 != 0) && (info->len1 != 0)) {
		ret = lcd_upt_payload_and_send(hisifd, cmds0, cmds1,
			type, info);
	} else if ((info->len0 == 0) && (info->len1 == 0)) {
		ret = lcd_kit_dsi_diff_cmds_tx(hisifd, cmds0, cmds1);
	} else {
		LCD_KIT_ERR("[DEMURA]not support type!\n");
		return LCD_KIT_FAIL;
	}
	if (ret)
		LCD_KIT_ERR("[DEMURA]lcd_kit_dsi_diff_cmds_tx fail\n");
	return ret;
}

int lcd_get_demura_handle(struct hisi_fb_data_type *hisifd,
	unsigned char dsi, unsigned char *out, int out_len,
	unsigned char read_type, unsigned char len)
{
	int ret;
	struct lcd_kit_dsi_panel_cmds *cmds = NULL;

	if ((!hisifd) || (!out)) {
		LCD_KIT_ERR("[DEMURA]invalid input param\n");
		return LCD_KIT_FAIL;
	}
	if (len > READ_MAX_LEN) {
		LCD_KIT_ERR("[DEMURA]invalid len, max len %d\n", READ_MAX_LEN);
		return LCD_KIT_FAIL;
	}
	switch (read_type) {
	case LCD_DEMURA_READ_FIRST:
		cmds = &(disp_info->demura.r_fir_cmds);
		break;
	case LCD_DEMURA_READ_CONTINUE:
		cmds = &(disp_info->demura.r_con_cmds);
		break;
	case LCD_DEMURA_READ_CHECKSUM:
		cmds = &(disp_info->demura.rr_chksum_cmds);
		break;
	case LCD_DEMURA_READ_WRITED_CHKSUM:
		cmds = &(disp_info->demura.rw_chksum_cmds);
		break;
	default:
		LCD_KIT_ERR("[DEMURA]invalid type\n");
		return LCD_KIT_FAIL;
	}
	if (dsi == LCD_DSI0)
		ret = lcd_kit_dsi_cmds_rx(hisifd, out, out_len, cmds);
	else if (dsi == LCD_DSI1)
		ret = lcd_kit_dsi1_cmds_rx(hisifd, out, out_len, cmds);
	else
		return LCD_KIT_FAIL;
	if (ret) {
		LCD_KIT_ERR("[DEMURA]first read ret = %d\n", ret);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_ERR("[DEMURA]read success\n");
	return ret;
}

static int lcd_kit_get_fps_para(int scence, int *fps,
	struct lcd_kit_array_data **dsi_timing)
{
	int index;
	struct lcd_kit_array_data *timing = NULL;

	switch (scence) {
	case LCD_FPS_SCENCE_30:
		*fps = LCD_FPS_30;
		break;
	case LCD_FPS_SCENCE_45:
		*fps = LCD_FPS_45;
		break;
	case LCD_FPS_SCENCE_60P:
		if (disp_info->fps.fps_need_high_60 == FPS_CONFIG_EN)
			*fps = LCD_FPS_60_HIGH;
		else
			*fps = LCD_FPS_60;
		break;
	case LCD_FPS_SCENCE_60:
		*fps = LCD_FPS_60;
		break;
	case LCD_FPS_SCENCE_90:
		*fps = LCD_FPS_90;
		break;
	case LCD_FPS_SCENCE_120:
		*fps = LCD_FPS_120;
		break;
	case LCD_FPS_SCENCE_180:
		*fps = LCD_FPS_180;
		break;
	default:
		LCD_KIT_ERR("error scence:%d\n", scence);
		return LCD_KIT_FAIL;
	}
	index = scence;
	timing = &disp_info->fps.fps_dsi_timming[index];
	if (timing->buf == NULL ||
		timing->cnt < FPS_DSI_TIMMING_PARA_NUM)
		return LCD_KIT_FAIL;
	*dsi_timing = timing;
	return LCD_KIT_OK;
}

static void lcd_kit_dsc_switch_proc(int scence, struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;

	pinfo = &(hisifd->panel_info);
	if (is_mipi_cmd_panel(hisifd)) {
		if (!is_ifbc_vesa_panel(hisifd))
			pinfo->mipi_updt.dynamic_dsc_en = FPS_UPDATE_DISABLE;
		else
			pinfo->mipi_updt.dynamic_dsc_en = FPS_UPDATE_ENABLE;
		return;
	}
	if (disp_info->fps.fps_ifbc_type != IFBC_TYPE_NONE) {
		if (scence >= LCD_FPS_SCENCE_60P &&
			scence <= LCD_FPS_SCENCE_180) {
			pinfo->ifbc_type = disp_info->fps.fps_ifbc_type;
			pinfo->mipi_updt.dynamic_dsc_en = FPS_UPDATE_ENABLE;
		} else if (!pinfo->dynamic_dsc_en) {
			pinfo->ifbc_type = IFBC_TYPE_NONE;
			pinfo->mipi_updt.dynamic_dsc_en = FPS_UPDATE_DISABLE;
		}
	}
	LCD_KIT_INFO("fps_ifbc_type:%d,ifbc_type:%d\n",
		disp_info->fps.fps_ifbc_type,
		pinfo->ifbc_type);
}

static bool lcd_kit_fps_check_delay(struct hisi_panel_info *pinfo)
{
	long delta_time;
	struct timeval tv;
	struct timeval afe_tv;
	struct timeval record_tv;

	if (disp_info->fps.fps_can_update_flag)
		return true;
	record_tv.tv_sec = disp_info->quickly_sleep_out.panel_on_record_tv.tv_sec;
	record_tv.tv_usec = disp_info->quickly_sleep_out.panel_on_record_tv.tv_usec;
	/* first high fsp switch */
	if (record_tv.tv_sec == 0 && record_tv.tv_usec == 0 &&
		pinfo->fps == LCD_FPS_60P) {
		if (!lcd_kit_get_thp_afe_status(&afe_tv))
			return false;
		record_tv.tv_sec = afe_tv.tv_sec;
		record_tv.tv_usec = afe_tv.tv_usec;
	}
	memset(&tv, 0, sizeof(struct timeval));
	do_gettimeofday(&tv);
	/* change s to us */
	delta_time = (tv.tv_sec - record_tv.tv_sec) * 1000000 +
		tv.tv_usec - record_tv.tv_usec;
	/* change us to ms */
	delta_time /= 1000;
	if (delta_time < disp_info->fps.fps_check_panel_on_delay)
		return false;
	else
		disp_info->fps.fps_can_update_flag = true;
	return true;
}

static bool lcd_kit_can_switch_fps(struct hisi_panel_info *pinfo)
{
	if (pinfo == NULL)
		return false;
	if (pinfo->fps_updt != pinfo->fps && pinfo->fps_updt != 0) {
		LCD_KIT_INFO("the last fps %d update handle have not finished\n",
			pinfo->fps_updt);
		return false;
	}
	if (disp_info->fps.fps_check_panel_on_delay > 0 &&
		!lcd_kit_fps_check_delay(pinfo)) {
		LCD_KIT_INFO("the delay time after display on is not satisfied\n");
		return false;
	}
	return true;
}

static int lcd_kit_updt_mipi_fps(struct hisi_fb_data_type *hisifd, int scence)
{
	int ret;
	int frm_rate = LCD_FPS_60;
	struct lcd_kit_array_data *fps_timing = NULL;
	struct hisi_panel_info *pinfo = NULL;

	/* waiting hbm max time */
	long timeout = msecs_to_jiffies(10);

	pinfo = &(hisifd->panel_info);
	ret = lcd_kit_get_fps_para(scence, &frm_rate, &fps_timing);
	if (ret) {
		LCD_KIT_ERR("scence:%d is not support\n", scence);
		return LCD_KIT_OK;
	}
	LCD_KIT_INFO("frm_rate:%d, scence:%d\n", frm_rate, scence);
	if (!lcd_kit_can_switch_fps(pinfo))
		return LCD_KIT_FAIL;
	lcd_kit_dsc_switch_proc(scence, hisifd);
	/* enable fps update func */
	if (common_info->dfr_info.fps_lock_command_support && pinfo->fps != frm_rate) {
		ret = wait_event_interruptible_timeout(common_info->dfr_info.fps_wait,
			common_info->dfr_info.hbm_status, timeout);
		common_info->dfr_info.fps_dfr_status = FPS_DFR_STATUS_DOING;
		LCD_KIT_DEBUG("fps %d->%d, ret = %d", pinfo->fps, frm_rate, ret);
		if (ret == 0)
			LCD_KIT_INFO("wait time out and set fps doing\n");
	}
	pinfo->send_dfr_cmd_in_vactive = disp_info->fps.fps_send_dfr_cmd_in_vactive;
	pinfo->dfr_delay_time = disp_info->fps.fps_effective_frame_count;
	pinfo->mipi_updt.frame_rate = frm_rate;
	pinfo->mipi_updt.take_effect_delayed_frm_cnt = 0;
	pinfo->mipi_updt.hsa = fps_timing->buf[FPS_HSA_PARA_INDEX];
	pinfo->mipi_updt.hbp = fps_timing->buf[FPS_HBP_PARA_INDEX];
	pinfo->mipi_updt.dpi_hsize = fps_timing->buf[FPS_HSIZE_PARA_INDEX];
	pinfo->mipi_updt.hline_time = fps_timing->buf[FPS_HLINE_PARA_INDEX];
	pinfo->mipi_updt.width = pinfo->xres;
	pinfo->mipi_updt.vsa = fps_timing->buf[FPS_VSA_PARA_INDEX];
	pinfo->mipi_updt.vbp = fps_timing->buf[FPS_VBP_PARA_INDEX];
	pinfo->mipi_updt.vfp = fps_timing->buf[FPS_VFP_PARA_INDEX];
	pinfo->mipi_updt.vactive_line =  pinfo->yres;
	pinfo->mipi_updt.dsi_bit_clk_upt = fps_timing->buf[FPS_DSI_CLK_PARA_INDEX];
	pinfo->fps_updt_support = FPS_UPDATE_ENABLE;
	pinfo->fps_updt_force_update = FPS_UPDATE_ENABLE;
	pinfo->fps_scence = scence;
	pinfo->fps_updt = frm_rate;
	LCD_KIT_INFO("take_effect_delayed_frm_cnt:%d, hsa:%d hbp:%d dpi_hsize:%d hline_time:%d width:%d vsa:%d vbp:%d vfp:%d vactive_line:%d dsi_bit_clk_upt:%d\n",
		pinfo->mipi_updt.take_effect_delayed_frm_cnt, pinfo->mipi_updt.hsa, pinfo->mipi_updt.hbp,
		pinfo->mipi_updt.dpi_hsize, pinfo->mipi_updt.hline_time, pinfo->mipi_updt.width, pinfo->mipi_updt.vsa,
		pinfo->mipi_updt.vbp, pinfo->mipi_updt.vfp, pinfo->mipi_updt.vactive_line, pinfo->mipi_updt.dsi_bit_clk_upt);
	return LCD_KIT_OK;
}

int lcd_kit_updt_fps_scence(struct platform_device *pdev, uint32_t scence)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (!pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo)
		return LCD_KIT_FAIL;
	if (lcd_is_support_dfr()) {
		mutex_lock(&COMMON_LOCK->mipi_lock);
		ret = lcd_kit_updt_mipi_fps(hisifd, scence);
		mutex_unlock(&COMMON_LOCK->mipi_lock);
		return ret;
	}

	lcd_kit_fps_scence_set(&hisifd->panel_info, scence);
	if (is_mipi_video_panel(hisifd))
		lcd_kit_fps_updt_porch(&hisifd->panel_info, scence);
	return LCD_KIT_OK;
}

void lcd_kit_disp_on_check_delay(void)
{
	long delta_time;
	u32 delay_margin;
	struct timeval tv;
	int max_margin = 200;

	memset(&tv, 0, sizeof(struct timeval));
	do_gettimeofday(&tv);
	LCD_KIT_INFO("set backlight at %lu seconds %lu mil seconds\n",
		tv.tv_sec, tv.tv_usec);
	/* change s to us */
	delta_time = (tv.tv_sec - disp_info->quickly_sleep_out.panel_on_record_tv.tv_sec) * 1000000 +
		tv.tv_usec - disp_info->quickly_sleep_out.panel_on_record_tv.tv_usec;
	/* change us to ms */
	delta_time /= 1000;
	if (delta_time >= disp_info->quickly_sleep_out.interval) {
		LCD_KIT_INFO("%lu > %d, no need delay\n", delta_time,
			disp_info->quickly_sleep_out.interval);
		goto check_delay_end;
	}
	delay_margin = disp_info->quickly_sleep_out.interval - delta_time;
	if (delay_margin > max_margin) {
		LCD_KIT_INFO("something maybe error");
		goto check_delay_end;
	}
	msleep(delay_margin);
check_delay_end:
	disp_info->quickly_sleep_out.panel_on_tag = false;
}

void lcd_kit_disp_on_record_time(void)
{
	do_gettimeofday(&disp_info->quickly_sleep_out.panel_on_record_tv);
	LCD_KIT_INFO("display on at %lu seconds %lu mil seconds\n",
		disp_info->quickly_sleep_out.panel_on_record_tv.tv_sec,
		disp_info->quickly_sleep_out.panel_on_record_tv.tv_usec);
	disp_info->quickly_sleep_out.panel_on_tag = true;
}

int lcd_kit_get_bl_set_type(struct hisi_panel_info *pinfo)
{
	if (pinfo->bl_set_type & BL_SET_BY_PWM)
		return BL_SET_BY_PWM;
	else if (pinfo->bl_set_type & BL_SET_BY_BLPWM)
		return BL_SET_BY_BLPWM;
	else if (pinfo->bl_set_type & BL_SET_BY_MIPI)
		return BL_SET_BY_MIPI;
	else
		return BL_SET_BY_NONE;
}

int lcd_kit_alpm_setting(struct hisi_fb_data_type *hisifd, uint32_t mode)
{
	int ret = LCD_KIT_OK;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("AOD mode is %d\n", mode);
	switch (mode) {
	case ALPM_DISPLAY_OFF:
		hisifd->aod_mode = 1;
		mutex_lock(&COMMON_LOCK->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_STOP;
		mutex_unlock(&COMMON_LOCK->mipi_lock);
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.off_cmds);
		break;
	case ALPM_ON_MIDDLE_LIGHT:
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.high_light_cmds);
		mutex_lock(&COMMON_LOCK->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_RUNNING;
		mutex_unlock(&COMMON_LOCK->mipi_lock);
		break;
	case ALPM_EXIT:
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.exit_cmds);
		mutex_lock(&COMMON_LOCK->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_RUNNING;
		mutex_unlock(&COMMON_LOCK->mipi_lock);
		hisifd->aod_mode = 0;
		break;
	case ALPM_ON_LOW_LIGHT:
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.low_light_cmds);
		mutex_lock(&COMMON_LOCK->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_RUNNING;
		mutex_unlock(&COMMON_LOCK->mipi_lock);
		break;
	default:
		break;
	}
	return ret;
}

uint32_t get_panel_alpha(uint32_t backlight) {
	uint32_t alpha = backlight;

	// later will use alpha from common_info->hbm.alpha_table.buf[]
	return alpha;
}

static int lcd_kit_local_hbm_coordinate(struct hisi_fb_data_type *hisifd,
	struct local_hbm local_hbm_cmd)
{
	int ret = LCD_KIT_OK;
	uint32_t x_coordinate;
	uint32_t y_coordinate;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	x_coordinate = local_hbm_cmd.position.x_coordinate;
	y_coordinate = local_hbm_cmd.position.y_coordinate;

	common_info->hbm.circle_coordinate_cmds.cmds[0].payload[1]
		= (x_coordinate >> 8) & 0xff;
	common_info->hbm.circle_coordinate_cmds.cmds[0].payload[2]
		= x_coordinate & 0xff;
	common_info->hbm.circle_coordinate_cmds.cmds[0].payload[3]
		= (y_coordinate >> 8) & 0xff;
	common_info->hbm.circle_coordinate_cmds.cmds[0].payload[4]
		= y_coordinate & 0xff;
	ret = lcd_kit_dsi_cmds_tx(hisifd,
		&common_info->hbm.circle_coordinate_cmds);

	return ret;
}

static int lcd_kit_local_hbm_radius(struct hisi_fb_data_type *hisifd,
	struct local_hbm local_hbm_cmd)
{
	int ret = LCD_KIT_OK;
	if (!hisifd) {
		HISI_FB_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	switch (local_hbm_cmd.radius) {
	case 1: // small circle
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&common_info->hbm.circle_size_small_cmds);
		break;
	case 2: // medium circle
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&common_info->hbm.circle_size_mid_cmds);
		break;
	case 3: // large circle
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&common_info->hbm.circle_size_large_cmds);
		break;
	default:
		break;
	}

	return ret;
}

static int set_color_by_index(struct hisi_fb_data_type *hisifd, uint32_t gamma_node_index)
{
	int ret;
	uint32_t fps;
	struct hisi_panel_info *pinfo = &(hisifd->panel_info);

	if (!pinfo) {
		LCD_KIT_ERR("pinfo is NULL\n");
		return LCD_KIT_FAIL;
	}
	fps = pinfo->fps;
	ret = lcd_kit_local_hbm_set_color_by_index(hisifd, fps, gamma_node_index);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("lcd_kit_local_hbm_set_color_by_index error, ret:%d\n", ret);
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_get_fps(struct hisi_fb_data_type *hisifd, int *fps)
{
	struct dpu_panel_info *pinfo = NULL;

	if (!fps) {
		DPU_FB_ERR("fps is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!hisifd) {
		DPU_FB_ERR("hisifd is NULL\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		DPU_FB_ERR("pinfo is NULL\n");
		return LCD_KIT_FAIL;
	}
	*fps = pinfo->fps;
	return LCD_KIT_OK;
}

// cmd[0]:mode cmd[1]~cmd[3]:data
int lcd_kit_local_hbm(struct hisi_fb_data_type *hisifd, const char *cmd,
	size_t count)
{
	int ret = LCD_KIT_OK;
	struct local_hbm local_hbm_cmd;
	uint32_t *param = (uint32_t *)&local_hbm_cmd;
	uint32_t fps;

	ret = lcd_kit_get_fps(hisifd, &fps);
	if (ret != LCD_KIT_OK) {
		DPU_FB_ERR("lcd_kit_get_fps error\n");
		return LCD_KIT_FAIL;
	}
	memset(&local_hbm_cmd, 0, sizeof(local_hbm_cmd));
	if (!hisifd || !cmd)
		return LCD_KIT_FAIL;
	/* param is a node array parameter. The value 1 indicates whether
	   the local HBM is enabled, the value 2 indicates the alpha value,
	   and the value 3 indicates the dbv. */
	sscanf(cmd,"%d %d %d %d", &param[0], &param[1], &param[2], &param[3]);
	switch (local_hbm_cmd.mode) {
	case DDIC_CIRCLE_COORINATE:
		ret = lcd_kit_local_hbm_coordinate(hisifd, local_hbm_cmd);
		break;
	case DDIC_CIRCLE_RADIUS:
		ret = lcd_kit_local_hbm_radius(hisifd, local_hbm_cmd);
		break;
	case DDIC_CIRCLE_COLOR:
		ret = set_color_by_index(hisifd, (uint32_t)param[1]);
		break;
	case DDIC_LOCAL_HBM:
		lcd_kit_hbm_gamma_read(hisifd);
		/* param is a node array parameter. The value 1 indicates whether
		  the local HBM is enabled, the value 2 indicates the alpha value,
		  and the value 3 indicates the dbv. */
		ret = lcd_kit_local_hbm_alpha_circle_with_alpha_and_dbv(hisifd,
			fps, (bool)param[1], (uint32_t)param[2],
			(uint32_t)param[3]);
		break;
	default:
		DPU_FB_ERR(
			"Invalid local_hbm_cmd.mode, local_hbm_cmd.mode:%d\n",
			local_hbm_cmd.mode);
		break;
	}
	return ret;
}

int lcd_kit_rgbw_set_bl(struct hisi_fb_data_type *hisifd, uint32_t level)
{
	uint32_t rgbw_level;
	int ret;

	rgbw_level = level * disp_info->rgbw.rgbw_bl_max / hisifd->panel_info.bl_max;
	/* change bl level to dsi cmds */
	disp_info->rgbw.backlight_cmds.cmds[0].payload[1] = (rgbw_level >> 8) & 0xff;
	disp_info->rgbw.backlight_cmds.cmds[0].payload[2] = rgbw_level & 0xff;
	hisifb_activate_vsync(hisifd);
	ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->rgbw.backlight_cmds);
	hisifb_deactivate_vsync(hisifd);
	// delay 2 frame time
	msleep(38);
	return ret;
}

int lcd_kit_blpwm_set_backlight(struct hisi_fb_data_type *hisifd, uint32_t level)
{
	uint32_t bl_level;
	static uint32_t last_bl_level = 255;
	uint32_t ret;
	struct hisi_panel_info *pinfo = NULL;

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	bl_level = (level < hisifd->panel_info.bl_max) ? level : hisifd->panel_info.bl_max;
	if (disp_info->rgbw.support && disp_info->rgbw.backlight_cmds.cmds) {
		if (hisifd->backlight.bl_level_old == 0 && level != 0) {
			ret = lcd_kit_rgbw_set_bl(hisifd, bl_level);
			if (ret)
				LCD_KIT_ERR("set rgbw level fail\n");
		}
	}
	ret = hisi_blpwm_set_bl(hisifd, bl_level);
	if (power_hdl->lcd_backlight.buf[POWER_MODE] == REGULATOR_MODE) {
		/* enable/disable backlight */
		down(&DISP_LOCK->lcd_kit_sem);
		if (bl_level == 0 && last_bl_level != 0)
			lcd_kit_charger_ctrl(LCD_KIT_BL, 0);
		else if (last_bl_level == 0 && bl_level != 0)
			lcd_kit_charger_ctrl(LCD_KIT_BL, 1);
		last_bl_level = bl_level;
		up(&DISP_LOCK->lcd_kit_sem);
	}
	return ret;
}

static void lcd_last_time_clear(uint8_t curr_mode)
{
	struct timeval curr_time;

	do_gettimeofday(&curr_time);
	switch (curr_mode) {
	case EN_DISPLAY_REGION_A:
		disp_info->dbv_stat.last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.hbm_last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.hbmon_last_time[PRIMARY_REGION] = curr_time;
		break;
	case EN_DISPLAY_REGION_B:
		disp_info->dbv_stat.last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.hbm_last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.hbmon_last_time[SLAVE_REGION] = curr_time;
		break;
	case EN_DISPLAY_REGION_AB:
	case EN_DISPLAY_REGION_AB_FOLDED:
		disp_info->dbv_stat.last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.last_time[FOLD_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[FOLD_REGION] = curr_time;
		disp_info->dbv_stat.hbm_last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.hbm_last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.hbm_last_time[FOLD_REGION] = curr_time;
		disp_info->dbv_stat.hbmon_last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.hbmon_last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.hbmon_last_time[FOLD_REGION] = curr_time;
		break;
	default:
		break;
	}
}

static void lcd_set_dbv_stat_primary_region(unsigned int level,
	unsigned int ori_level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;
	do_gettimeofday(&curr_time);
	tmp = disp_info->dbv_stat.last_time[PRIMARY_REGION];

	/* change s to ms */
	delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
		(curr_time.tv_usec - tmp.tv_usec) / 1000;
	disp_info->dbv_stat.dbv[PRIMARY_REGION] += (level * delta);
	disp_info->dbv_stat.ori_dbv[PRIMARY_REGION] += (ori_level * delta);
	do_gettimeofday(&disp_info->dbv_stat.last_time[PRIMARY_REGION]);
	LCD_KIT_DEBUG("dbv[PRIMARY_REGION]:%d, ori_dbv[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.dbv[PRIMARY_REGION],
		disp_info->dbv_stat.ori_dbv[PRIMARY_REGION]);
}

static void lcd_set_dbv_stat_slave_region(unsigned int level,
	unsigned int ori_level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;
	do_gettimeofday(&curr_time);
	tmp = disp_info->dbv_stat.last_time[SLAVE_REGION];

	/* change s to ms */
	delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
		(curr_time.tv_usec - tmp.tv_usec) / 1000;
	disp_info->dbv_stat.dbv[SLAVE_REGION] += (level * delta);
	disp_info->dbv_stat.ori_dbv[SLAVE_REGION] += (ori_level * delta);
	do_gettimeofday(&disp_info->dbv_stat.last_time[SLAVE_REGION]);
	LCD_KIT_DEBUG("dbv[SLAVE_REGION]:%d, ori_dbv[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.dbv[SLAVE_REGION],
		disp_info->dbv_stat.ori_dbv[SLAVE_REGION]);

}

static void lcd_set_dbv_stat_fold_region(unsigned int level,
	unsigned int ori_level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;
	do_gettimeofday(&curr_time);
	tmp = disp_info->dbv_stat.last_time[FOLD_REGION];

	/* change s to ms */
	delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
		(curr_time.tv_usec - tmp.tv_usec) / 1000;
	disp_info->dbv_stat.dbv[FOLD_REGION] += (level * delta);
	disp_info->dbv_stat.ori_dbv[FOLD_REGION] += (ori_level * delta);
	do_gettimeofday(&disp_info->dbv_stat.last_time[FOLD_REGION]);
	LCD_KIT_DEBUG("dbv[FOLD_REGION]:%d, ori_dbv[FOLD_REGION]:%d\n",
		disp_info->dbv_stat.dbv[FOLD_REGION],
		disp_info->dbv_stat.ori_dbv[FOLD_REGION]);
}

static void lcd_set_dbv_stat_max_region(unsigned int level,
	unsigned int ori_level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;
	do_gettimeofday(&curr_time);
	tmp = disp_info->dbv_stat.last_time[PRIMARY_REGION];

	/* change s to ms */
	delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
		(curr_time.tv_usec - tmp.tv_usec) / 1000;
	disp_info->dbv_stat.dbv[PRIMARY_REGION] += (level * delta);
	disp_info->dbv_stat.ori_dbv[PRIMARY_REGION] += (ori_level * delta);
	tmp = disp_info->dbv_stat.last_time[SLAVE_REGION];
	delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
		(curr_time.tv_usec - tmp.tv_usec) / 1000;
	disp_info->dbv_stat.dbv[SLAVE_REGION] += (level * delta);
	disp_info->dbv_stat.ori_dbv[SLAVE_REGION] += (ori_level * delta);
	tmp = disp_info->dbv_stat.last_time[FOLD_REGION];
	delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
		(curr_time.tv_usec - tmp.tv_usec) / 1000;
	disp_info->dbv_stat.dbv[FOLD_REGION] += (level * delta);
	disp_info->dbv_stat.ori_dbv[FOLD_REGION] += (level * delta);
	do_gettimeofday(&disp_info->dbv_stat.last_time[PRIMARY_REGION]);
	do_gettimeofday(&disp_info->dbv_stat.last_time[SLAVE_REGION]);
	do_gettimeofday(&disp_info->dbv_stat.last_time[FOLD_REGION]);
	LCD_KIT_DEBUG("dbv[PRIMARY_REGION]:%d, ori_dbv[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.dbv[PRIMARY_REGION],
		disp_info->dbv_stat.ori_dbv[PRIMARY_REGION]);
	LCD_KIT_DEBUG("dbv[SLAVE_REGION]:%d, ori_dbv[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.dbv[SLAVE_REGION],
		disp_info->dbv_stat.ori_dbv[SLAVE_REGION]);
	LCD_KIT_DEBUG("dbv[FOLD_REGION]:%d, ori_dbv[FOLD_REGION]:%d\n",
		disp_info->dbv_stat.dbv[FOLD_REGION],
		disp_info->dbv_stat.ori_dbv[FOLD_REGION]);
}

static void _lcd_set_dbv_stat(int region, unsigned int level,
	unsigned int ori_level)
{
	LCD_KIT_DEBUG("FOLD dbv_level:%d, ori_level:%d\n", level, ori_level);
	switch (region) {
	case PRIMARY_REGION:
		lcd_set_dbv_stat_primary_region(level, ori_level);
		break;
	case SLAVE_REGION:
		lcd_set_dbv_stat_slave_region(level, ori_level);
		break;
	case FOLD_REGION:
		lcd_set_dbv_stat_fold_region(level, ori_level);
		break;
	case REGION_MAX:
		lcd_set_dbv_stat_max_region(level, ori_level);
		break;
	default:
		break;
	}
}



static void _lcd_set_hbm_stat(struct hisi_fb_data_type *hisifd, int region, unsigned int level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;
	if(level < hisifd->panel_info.min_hbm_dbv)
		level = 0;
	do_gettimeofday(&curr_time);
	switch (region) {
	case PRIMARY_REGION:
		tmp = disp_info->dbv_stat.hbm_last_time[PRIMARY_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbm[PRIMARY_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbm_last_time[PRIMARY_REGION]);
		break;
	case SLAVE_REGION:
		tmp = disp_info->dbv_stat.hbm_last_time[SLAVE_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbm[SLAVE_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbm_last_time[SLAVE_REGION]);
		break;
	case FOLD_REGION:
		tmp = disp_info->dbv_stat.hbm_last_time[FOLD_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbm[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbm_last_time[FOLD_REGION]);
		break;
	case REGION_MAX:
		tmp = disp_info->dbv_stat.hbm_last_time[PRIMARY_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbm[PRIMARY_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.hbm_last_time[SLAVE_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbm[SLAVE_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.hbm_last_time[FOLD_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbm[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbm_last_time[PRIMARY_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.hbm_last_time[SLAVE_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.hbm_last_time[FOLD_REGION]);
		break;
	default:
		break;
	}
}


static void _lcd_set_pwon_stat(int region, unsigned int level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;
	if (level > 0)
		level = 1;
	do_gettimeofday(&curr_time);
	switch (region) {
	case PRIMARY_REGION:
		tmp = disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[PRIMARY_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION]);
		break;
	case SLAVE_REGION:
		tmp = disp_info->dbv_stat.pwon_last_time[SLAVE_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[SLAVE_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.pwon_last_time[SLAVE_REGION]);
		break;
	case FOLD_REGION:
		tmp = disp_info->dbv_stat.pwon_last_time[FOLD_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.pwon_last_time[FOLD_REGION]);
		break;
	case REGION_MAX:
		tmp = disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[PRIMARY_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.pwon_last_time[SLAVE_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[SLAVE_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.pwon_last_time[FOLD_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.pwon_last_time[SLAVE_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.pwon_last_time[FOLD_REGION]);
		break;
	default:
		break;
	}
}

static void _lcd_set_hbmon_stat(struct hisi_fb_data_type *hisifd, int region, unsigned int level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;
	level = (level < hisifd->panel_info.min_hbm_dbv) ? 0 : 1;
	do_gettimeofday(&curr_time);
	switch (region) {
	case PRIMARY_REGION:
		tmp = disp_info->dbv_stat.hbmon_last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbmon[PRIMARY_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbmon_last_time[PRIMARY_REGION]);
		break;
	case SLAVE_REGION:
		tmp = disp_info->dbv_stat.hbmon_last_time[SLAVE_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbmon[SLAVE_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbmon_last_time[SLAVE_REGION]);
		break;
	case FOLD_REGION:
		tmp = disp_info->dbv_stat.hbmon_last_time[FOLD_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbmon[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbmon_last_time[FOLD_REGION]);
		break;
	case REGION_MAX:
		tmp = disp_info->dbv_stat.hbmon_last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbmon[PRIMARY_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.hbmon_last_time[SLAVE_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbmon[SLAVE_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.hbmon_last_time[FOLD_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.hbmon[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.hbmon_last_time[PRIMARY_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.hbmon_last_time[SLAVE_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.hbmon_last_time[FOLD_REGION]);
		break;
	default:
		break;
	}
}


static void lcd_set_dbv_stat(struct hisi_fb_data_type *hisifd,
	uint32_t level)
{
	static uint32_t last_level;
	static uint32_t last_ori_level;
	static int is_first;
	struct hisi_panel_info *pinfo = NULL;
	int region;

	if (!disp_info->dbv_stat.support)
		return;
	pinfo = &(hisifd->panel_info);
	if (!pinfo)
		LCD_KIT_ERR("pinfo is null!\n");
	if (!is_first) {
		lcd_last_time_clear(pinfo->current_display_region);
		last_level = level;
		last_ori_level = hisifd->bl_level;
		is_first = 1;
		return;
	}
	if (lcd_is_power_on(level))
		lcd_last_time_clear(pinfo->current_display_region);
	switch (pinfo->current_display_region) {
	case EN_DISPLAY_REGION_A:
		region = PRIMARY_REGION;
		break;
	case EN_DISPLAY_REGION_B:
		region = SLAVE_REGION;
		break;
	case EN_DISPLAY_REGION_AB:
	case EN_DISPLAY_REGION_AB_FOLDED:
		region = REGION_MAX;
		break;
	default:
		region = REGION_MAX;
		break;
	}
	_lcd_set_pwon_stat(region, last_level);
	_lcd_set_dbv_stat(region, last_level, last_ori_level);
	_lcd_set_hbm_stat(hisifd, region, last_level);
	_lcd_set_hbmon_stat(hisifd, region, last_level);
	last_level = level;
	last_ori_level = hisifd->bl_level;
	LCD_KIT_INFO("disp_info->dbv_stat.dbv[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.dbv[PRIMARY_REGION]);
	LCD_KIT_DEBUG("disp_info->dbv_stat.dbv[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.dbv[SLAVE_REGION]);
	LCD_KIT_DEBUG("disp_info->dbv_stat.dbv[FOLD_REGION]:%d\n",
		disp_info->dbv_stat.dbv[FOLD_REGION]);
}

static void _lcd_get_dbv_stat(struct hisi_fb_data_type *hisifd,
	uint32_t *dbv_stat, uint32_t dbv_len)
{
	struct hisi_panel_info *pinfo = NULL;
	int region;

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null!\n");
		return;
	}
	switch (pinfo->current_display_region) {
	case EN_DISPLAY_REGION_A:
		region = PRIMARY_REGION;
		break;
	case EN_DISPLAY_REGION_B:
		region = SLAVE_REGION;
		break;
	case EN_DISPLAY_REGION_AB:
	case EN_DISPLAY_REGION_AB_FOLDED:
		region = REGION_MAX;
		break;
	default:
		region = REGION_MAX;
		break;
	}
	_lcd_set_dbv_stat(region, hisifd->de_info.actual_bl_level,
		hisifd->bl_level);
	LCD_KIT_INFO("disp_info->dbv_stat.dbv[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.dbv[PRIMARY_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.dbv[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.dbv[SLAVE_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.dbv[FOLD_REGION]:%d\n",
		disp_info->dbv_stat.dbv[FOLD_REGION]);
	memcpy(dbv_stat, disp_info->dbv_stat.dbv, dbv_len);
	memset(disp_info->dbv_stat.dbv, 0,
		sizeof(disp_info->dbv_stat.dbv));
}

static void _lcd_get_hbm_stat(struct hisi_fb_data_type *hisifd,
	uint32_t *hbm_stat, uint32_t hbm_len)
{
	struct hisi_panel_info *pinfo = NULL;
	int region;

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null!\n");
		return;
	}
	switch (pinfo->current_display_region) {
	case EN_DISPLAY_REGION_A:
		region = PRIMARY_REGION;
		break;
	case EN_DISPLAY_REGION_B:
		region = SLAVE_REGION;
		break;
	case EN_DISPLAY_REGION_AB:
	case EN_DISPLAY_REGION_AB_FOLDED:
		region = REGION_MAX;
		break;
	default:
		region = REGION_MAX;
		break;
	}
	_lcd_set_hbm_stat(hisifd, region, hisifd->de_info.actual_bl_level);
	LCD_KIT_INFO("disp_info->dbv_stat.hbm[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.hbm[PRIMARY_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.hbm[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.hbm[SLAVE_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.hbm[FOLD_REGION]:%d\n",
		disp_info->dbv_stat.hbm[FOLD_REGION]);
	memcpy(hbm_stat, disp_info->dbv_stat.hbm, hbm_len);
	memset(disp_info->dbv_stat.hbm, 0,
		sizeof(disp_info->dbv_stat.hbm));
}

static void _lcd_get_pwon_stat(struct hisi_fb_data_type *hisifd,
	uint32_t *pwon_stat, uint32_t pwon_len)
{
	struct hisi_panel_info *pinfo = NULL;
	int region;

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null!\n");
		return;
	}
	switch (pinfo->current_display_region) {
	case EN_DISPLAY_REGION_A:
		region = PRIMARY_REGION;
		break;
	case EN_DISPLAY_REGION_B:
		region = SLAVE_REGION;
		break;
	case EN_DISPLAY_REGION_AB:
	case EN_DISPLAY_REGION_AB_FOLDED:
		region = REGION_MAX;
		break;
	default:
		region = REGION_MAX;
		break;
	}
	_lcd_set_pwon_stat(region, hisifd->de_info.actual_bl_level);
	LCD_KIT_INFO("disp_info->dbv_stat.pwon[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.pwon[PRIMARY_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.pwon[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.pwon[SLAVE_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.pwon[FOLD_REGION]:%d\n",
		disp_info->dbv_stat.pwon[FOLD_REGION]);
	memcpy(pwon_stat, disp_info->dbv_stat.pwon, pwon_len);
	memset(disp_info->dbv_stat.pwon, 0,
		sizeof(disp_info->dbv_stat.pwon));
}

static void _lcd_get_hbmon_stat(struct hisi_fb_data_type *hisifd,
	uint32_t *hbmon_stat, uint32_t hbmon_size)
{
	struct hisi_panel_info *pinfo = NULL;
	int region;

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null!\n");
		return;
	}
	switch (pinfo->current_display_region) {
	case EN_DISPLAY_REGION_A:
		region = PRIMARY_REGION;
		break;
	case EN_DISPLAY_REGION_B:
		region = SLAVE_REGION;
		break;
	case EN_DISPLAY_REGION_AB:
	case EN_DISPLAY_REGION_AB_FOLDED:
		region = REGION_MAX;
		break;
	default:
		region = REGION_MAX;
		break;
	}
	_lcd_set_hbmon_stat(hisifd, region, hisifd->de_info.actual_bl_level);
	LCD_KIT_INFO("disp_info->dbv_stat.pwon[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.hbmon[PRIMARY_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.pwon[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.hbmon[SLAVE_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.pwon[FOLD_REGION]:%d\n",
		disp_info->dbv_stat.hbmon[FOLD_REGION]);
	memcpy(hbmon_stat, disp_info->dbv_stat.hbmon, hbmon_size);
	memset(disp_info->dbv_stat.hbmon, 0,
		sizeof(disp_info->dbv_stat.hbmon));
}

void clear_time_by_region(int region) {
	struct timeval tmp;
	do_gettimeofday(&tmp);
	disp_info->dbv_stat.last_time[region] = tmp;
	disp_info->dbv_stat.hbm_last_time[region] = tmp;
	disp_info->dbv_stat.pwon_last_time[region] = tmp;
	disp_info->dbv_stat.hbmon_last_time[region] = tmp;
}

void lcd_set_dbv_stat_by_region(struct hisi_fb_data_type *hisifd,
	int region)
{
	_lcd_set_dbv_stat(region, hisifd->de_info.actual_bl_level,
		hisifd->bl_level);
	_lcd_set_pwon_stat(region, hisifd->de_info.actual_bl_level);
	_lcd_set_hbm_stat(hisifd, region, hisifd->de_info.actual_bl_level);
	_lcd_set_hbmon_stat(hisifd, region, hisifd->de_info.actual_bl_level);
}

void lcd_switch_region(struct hisi_fb_data_type *hisifd,
	uint8_t curr_mode, uint8_t pre_mode)
{
	if (!disp_info->dbv_stat.support)
		return;
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null!\n");
		return;
	}
	if ((curr_mode == EN_DISPLAY_REGION_A) &&
		(pre_mode == EN_DISPLAY_REGION_B)) {
		clear_time_by_region(PRIMARY_REGION);
		lcd_set_dbv_stat_by_region(hisifd, SLAVE_REGION);
	} else if ((curr_mode == EN_DISPLAY_REGION_A) &&
		((pre_mode == EN_DISPLAY_REGION_AB) ||
		(pre_mode == EN_DISPLAY_REGION_AB_FOLDED))) {
		lcd_set_dbv_stat_by_region(hisifd, SLAVE_REGION);
		lcd_set_dbv_stat_by_region(hisifd, FOLD_REGION);
	} else if ((curr_mode == EN_DISPLAY_REGION_B) &&
		(pre_mode == EN_DISPLAY_REGION_A)) {
		clear_time_by_region(SLAVE_REGION);
		lcd_set_dbv_stat_by_region(hisifd, PRIMARY_REGION);
	} else if ((curr_mode == EN_DISPLAY_REGION_B) &&
		((pre_mode == EN_DISPLAY_REGION_AB) ||
		(pre_mode == EN_DISPLAY_REGION_AB_FOLDED))) {
		lcd_set_dbv_stat_by_region(hisifd, PRIMARY_REGION);
		lcd_set_dbv_stat_by_region(hisifd, FOLD_REGION);
	} else if (((curr_mode == EN_DISPLAY_REGION_AB) ||
		(curr_mode == EN_DISPLAY_REGION_AB_FOLDED)) &&
		(pre_mode == EN_DISPLAY_REGION_A)) {
		clear_time_by_region(SLAVE_REGION);
		clear_time_by_region(FOLD_REGION);
	} else if (((curr_mode == EN_DISPLAY_REGION_AB) ||
		(curr_mode == EN_DISPLAY_REGION_AB_FOLDED)) &&
		(pre_mode == EN_DISPLAY_REGION_B)) {
		clear_time_by_region(PRIMARY_REGION);
		clear_time_by_region(FOLD_REGION);
	}
}

int lcd_get_dbv_stat(struct hisi_fb_data_type *hisifd,
	struct display_engine_duration param)
{
	struct hisi_panel_info *pinfo = NULL;

	if (!disp_info->dbv_stat.support)
		return LCD_KIT_OK;
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!param.dbv_acc) {
		LCD_KIT_ERR("dbv_stat is null!\n");
		return LCD_KIT_FAIL;
	}
	if (!param.screen_on_duration) {
		LCD_KIT_ERR("pwon_stat is null!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null!\n");
		return LCD_KIT_FAIL;
	}
	_lcd_get_dbv_stat(hisifd, param.dbv_acc, param.size_dbv_acc);
	if (param.original_dbv_acc) {
		memcpy(param.original_dbv_acc, disp_info->dbv_stat.ori_dbv,
			param.size_original_dbv_acc);
		memset(disp_info->dbv_stat.ori_dbv, 0,
			sizeof(disp_info->dbv_stat.ori_dbv));
	}
	_lcd_get_hbm_stat(hisifd, param.hbm_acc, param.size_hbm_acc);
	_lcd_get_pwon_stat(hisifd, param.screen_on_duration, param.size_screen_on_duration);
	_lcd_get_hbmon_stat(hisifd, param.hbm_duration, param.size_hbm_duration);

	lcd_last_time_clear(pinfo->current_display_region);
	return LCD_KIT_OK;
}

bool lcd_is_power_on(uint32_t level)
{
	static uint32_t last_level = MAX_BL_LEVEL;
	bool is_pwon = false;

	if (last_level == 0 && level != 0)
		is_pwon = true;
	last_level = level;
	return is_pwon;
}

int lcd_kit_mipi_set_backlight(struct hisi_fb_data_type *hisifd, uint32_t level)
{
	uint32_t bl_level;
	static uint32_t last_bl_level = 255;
	uint32_t ret = LCD_KIT_OK;
	struct hisi_panel_info *pinfo = NULL;

	pinfo = &(hisifd->panel_info);
	bl_level = (level < hisifd->panel_info.bl_max) ? level : hisifd->panel_info.bl_max;
	hisifb_display_effect_fine_tune_backlight(hisifd, (int)bl_level, (int *)&bl_level);
	bl_flicker_detector_collect_device_bl(bl_level);

	if (pinfo->hbm_entry_delay > 0) {
		mutex_lock(&COMMON_LOCK->hbm_lock);
		ret = common_ops->set_mipi_backlight(hisifd, bl_level);
		pinfo->hbm_blcode_ts = ktime_get();
		mutex_unlock(&COMMON_LOCK->hbm_lock);
	} else {
		ret = common_ops->set_mipi_backlight(hisifd, bl_level);
	}
	hisifd->de_info.actual_bl_level = bl_level;
	lcd_set_dbv_stat(hisifd, bl_level);
	if (power_hdl->lcd_backlight.buf[POWER_MODE] == REGULATOR_MODE) {
		/* enable/disable backlight */
		down(&DISP_LOCK->lcd_kit_sem);
		if (bl_level == 0 && last_bl_level != 0) {
			lcd_kit_charger_ctrl(LCD_KIT_BL, 0);
		} else if (last_bl_level == 0 && bl_level != 0) {
			lcd_kit_charger_ctrl(LCD_KIT_BL, 1);
			LCD_KIT_INFO("bl_level = %d!\n", bl_level);
		}
		last_bl_level = bl_level;
		up(&DISP_LOCK->lcd_kit_sem);
	}
	return ret;
}

int lcd_kit_dsi_fifo_is_full(const char __iomem *dsi_base)
{
	uint32_t pkg_status, phy_status;
	int is_timeout = 1;

	/* read status register */
	int  udelay_cnt = 100000; // max delay 100000*1 us
	while (udelay_cnt > 0) {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status & 0x2) != 0x2 && !(phy_status & 0x2)) {
			is_timeout = 0;
			break;
		}
		udelay_cnt--;
		udelay(1);
	};

	if (is_timeout) {
		HISI_FB_ERR("mipi check full fail:\n \
			MIPIDSI_CMD_PKT_STATUS = 0x%x\n \
			MIPIDSI_PHY_STATUS = 0x%x\n \
			MIPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

int lcd_kit_dsi_fifo_is_empty(const char __iomem *dsi_base)
{
	uint32_t pkg_status, phy_status;
	int is_timeout = 1;

	/* read status register */
	int udelay_cnt = 100000; // max delay 100000*1 us
	while (udelay_cnt > 0) {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		/* separate transmission */
		if ((disp_info != NULL) && (disp_info->transmission.separate_support)) {
			if ((pkg_status & 0x1) == 0x1 && !(phy_status & 0x2) &&
				(pkg_status & disp_info->transmission.separate_status_flag)) {
				is_timeout = 0;
				break;
			}
		} else {
			if ((pkg_status & 0x1) == 0x1 && !(phy_status & 0x2)) {
				is_timeout = 0;
				break;
			}
		}
		udelay_cnt--;
		udelay(1);
	};

	if (is_timeout) {
		HISI_FB_ERR("mipi check empty fail:\n \
			MIPIDSI_CMD_PKT_STATUS = 0x%x\n \
			MIPIDSI_PHY_STATUS = 0x%x\n \
			MIPIDSI_INT_ST1_OFFSET = 0x%x\n",
			inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET),
			inp32(dsi_base + MIPIDSI_INT_ST1_OFFSET));
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static void lcd_kit_vesa_para_parse(struct device_node *np,
	struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	lcd_kit_parse_u32(np, "lcd-kit,vesa-slice-width",
		&pinfo->vesa_dsc.slice_width, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-slice-height",
		&pinfo->vesa_dsc.slice_height, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-calc-mode",
		&disp_info->calc_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-native-422",
		&pinfo->vesa_dsc.native_422, 0);
	lcd_kit_parse_u32(np, "lcd-kit,vesa-dsc-version",
		&pinfo->vesa_dsc.dsc_version, 0);
}

bool lcd_is_support_dfr(void)
{
	int index;
	struct lcd_kit_array_data *fps_timing = NULL;

	if (disp_info->fps.support != FPS_CONFIG_EN ||
		disp_info->fps.panel_support_fps_list.buf == NULL ||
		disp_info->fps.panel_support_fps_list.cnt <= MIN_FPS_LIST)
		return false;
	/* hisi interface current only support high fps switch */
	for (index = LCD_FPS_SCENCE_60P; index < LCD_FPS_SCENCE_180; index++) {
		fps_timing = &disp_info->fps.fps_dsi_timming[index];
		if (fps_timing->buf != NULL)
			return true;
	}
	return false;
}

bool lcd_is_high_fps_state(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL || disp_info == NULL)
		return false;
	pinfo = &(hisifd->panel_info);
	if (disp_info->fps.fps_disable_dsi_clk_upt == FPS_UPDATE_ENABLE &&
		(pinfo->fps_updt > LCD_FPS_60 || pinfo->fps > LCD_FPS_60))
		return true;
	return false;
}

static void lcd_kit_fps_init(const struct device_node *np, struct hisi_panel_info *pinfo)
{
	if (lcd_is_support_dfr()) {
		/* For hwc get panel support fps capability, e.g. 0:60fps; 1:90fps; 2:120fps */
		lcd_kit_parse_u32(np, "lcd-kit,fps-dfr-support-value",
			&pinfo->dfr_support_value, 1);
		pinfo->dfr_method = disp_info->fps.fps_switch_mode;
		disp_info->fps.last_update_fps = LCD_FPS_60;
		disp_info->fps.fps_can_update_flag = false;
		if (disp_info->fps.fps_ifbc_type != IFBC_TYPE_NONE &&
			pinfo->ifbc_type == IFBC_TYPE_NONE) {
			pinfo->dynamic_dsc_support = FPS_UPDATE_ENABLE;
			pinfo->dynamic_dsc_ifbc_type = disp_info->fps.fps_ifbc_type;
		}
		if (disp_info->fps.fps_disable_dsi_clk_upt == FPS_UPDATE_ENABLE)
			pinfo->dfr_constraint |= BIT(0);
	}
	pinfo->fps_updt_support = FPS_UPDATE_DISABLE;
	pinfo->fps_scence = FPS_UPDATE_DISABLE;
	LCD_KIT_INFO("dfr_support_value = %d, dynamic_dsc_support = %d\n",
		pinfo->dfr_support_value, pinfo->dynamic_dsc_support);
}

static void lcd_kit_spr_dsc_parse(const struct device_node *np,
	struct hisi_panel_info *pinfo)
{
	struct lcd_kit_array_data spr_lut_table;
	uint8_t mode = pinfo->spr_dsc_mode;

	/* spr and dsc config init */
	if (mode == SPR_DSC_MODE_SPR_ONLY || mode == SPR_DSC_MODE_SPR_AND_DSC) {
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-rgbg2uyvy-8biten",
			&pinfo->spr.spr_rgbg2uyvy_8biten, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-hpartial-mode",
			&pinfo->spr.spr_hpartial_mode, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-partial-mode",
			&pinfo->spr.spr_partial_mode, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-rgbg2uyvy-en",
			&pinfo->spr.spr_rgbg2uyvy_en, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-horzborderdect",
			&pinfo->spr.spr_horzborderdect, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-linebuf-ldmode",
			&pinfo->spr.spr_linebuf_1dmode, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-bordertb-dummymode",
			&pinfo->spr.spr_bordertb_dummymode, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-borderlr-dummymode",
			&pinfo->spr.spr_borderlr_dummymode, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-pattern-mode",
			&pinfo->spr.spr_pattern_mode, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-pattern-en",
			&pinfo->spr.spr_pattern_en, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-subpxl_layout",
			&pinfo->spr.spr_subpxl_layout, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,ck-gt-spr-en",
			&pinfo->spr.ck_gt_spr_en, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-en",
			&pinfo->spr.spr_en, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-coeffsel-even",
			&pinfo->spr.spr_coeffsel_even, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-coeffsel-odd",
			&pinfo->spr.spr_coeffsel_odd, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,pix-panel-arrange-sel",
			&pinfo->spr.pix_panel_arrange_sel, 0);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-r-v0h0-coef",
			pinfo->spr.spr_r_v0h0_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-r-v0h1-coef",
			pinfo->spr.spr_r_v0h1_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-r-v1h0-coef",
			pinfo->spr.spr_r_v1h0_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-r-v1h1-coef",
			pinfo->spr.spr_r_v1h1_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-g-v0h0-coef",
			pinfo->spr.spr_g_v0h0_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-g-v0h1-coef",
			pinfo->spr.spr_g_v0h1_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-g-v1h0-coef",
			pinfo->spr.spr_g_v1h0_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-g-v1h1-coef",
			pinfo->spr.spr_g_v1h1_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-b-v0h0-coef",
			pinfo->spr.spr_b_v0h0_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-b-v0h1-coef",
			pinfo->spr.spr_b_v0h1_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-b-v1h0-coef",
			pinfo->spr.spr_b_v1h0_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-b-v1h1-coef",
			pinfo->spr.spr_b_v1h1_coef, SPR_COLOR_COEF_LEN);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-borderlr-detect-r",
			&pinfo->spr.spr_borderlr_detect_r, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-bordertb-detect-r",
			&pinfo->spr.spr_bordertb_detect_r, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-borderlr-detect-g",
			&pinfo->spr.spr_borderlr_detect_g, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-bordertb-detect-g",
			&pinfo->spr.spr_bordertb_detect_g, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-borderlr-detect-b",
			&pinfo->spr.spr_borderlr_detect_b, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-bordertb-detect-b",
			&pinfo->spr.spr_bordertb_detect_b, 0);
		(void)lcd_kit_parse_u64(np, "lcd-kit,spr-pix-gain",
			&pinfo->spr.spr_pixgain, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-borderl-position",
			&pinfo->spr.spr_borderl_position, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-borderr-position",
			&pinfo->spr.spr_borderr_position, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-bordert-position",
			&pinfo->spr.spr_bordert_position, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-borderb-position",
			&pinfo->spr.spr_borderb_position, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-r-diff",
			&pinfo->spr.spr_r_diff, 0);
		(void)lcd_kit_parse_u64(np, "lcd-kit,spr-r-weight",
			&pinfo->spr.spr_r_weight, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-g-diff",
			&pinfo->spr.spr_g_diff, 0);
		(void)lcd_kit_parse_u64(np, "lcd-kit,spr-g-weight",
			&pinfo->spr.spr_g_weight, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-b-diff",
			&pinfo->spr.spr_b_diff, 0);
		(void)lcd_kit_parse_u64(np, "lcd-kit,spr-b-weight",
			&pinfo->spr.spr_b_weight, 0);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-rgbg2uyvy-coeff",
			pinfo->spr.spr_rgbg2uyvy_coeff, SPR_CSC_COEF_LEN);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-input-reso",
			&pinfo->spr.input_reso, 0);
		spr_lut_table.buf = NULL;
		spr_lut_table.cnt = 0;
		(void)lcd_kit_parse_array_data((struct device_node *)np,
			"lcd-kit,spr-lut-table", &spr_lut_table);
		pinfo->spr.spr_lut_table = spr_lut_table.buf;
		pinfo->spr.spr_lut_table_len = spr_lut_table.cnt;
	}
	if (mode == SPR_DSC_MODE_DSC_ONLY || mode == SPR_DSC_MODE_SPR_AND_DSC) {
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-dsc-enable",
			&pinfo->spr.dsc_enable, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-dsc-slice0-ck-gt-en",
			&pinfo->spr.slice0_ck_gt_en, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-dsc-slice1-ck-gt-en",
			&pinfo->spr.slice1_ck_gt_en, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-rcb-bits",
			&pinfo->spr.rcb_bits, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-dsc-alg-ctrl",
			&pinfo->spr.dsc_alg_ctrl, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-bits-per-component",
			&pinfo->spr.bits_per_component, 0);
		(void)lcd_kit_parse_u8(np, "lcd-kit,spr-dsc-sample",
			&pinfo->spr.dsc_sample, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-bpp-chk",
			&pinfo->spr.bpp_chk, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-pic-reso",
			&pinfo->spr.pic_reso, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-slc-reso",
			&pinfo->spr.slc_reso, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-initial-xmit-delay",
			&pinfo->spr.initial_xmit_delay, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-initial-dec-delay",
			&pinfo->spr.initial_dec_delay, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-initial-scale-value",
			&pinfo->spr.initial_scale_value, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-scale-interval",
			&pinfo->spr.scale_interval, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-first-bpg",
			&pinfo->spr.first_bpg, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-second-bpg",
			&pinfo->spr.second_bpg, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-second-adj",
			&pinfo->spr.second_adj, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-init-finl-ofs",
			&pinfo->spr.init_finl_ofs, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-slc-bpg",
			&pinfo->spr.slc_bpg, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-flat-range",
			&pinfo->spr.flat_range, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-rc-mode-edge",
			&pinfo->spr.rc_mode_edge, 0);
		(void)lcd_kit_parse_u32(np, "lcd-kit,spr-rc-qua-tgt",
			&pinfo->spr.rc_qua_tgt, 0);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-rc-buff-thresh",
			pinfo->spr.rc_buf_thresh, SPR_RC_BUF_THRESH_LEN);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,spr-rc-para",
			pinfo->spr.rc_para, SPR_RC_PARA_LEN);
	}
}

void lcd_kit_pinfo_init(struct device_node *np, struct hisi_panel_info *pinfo)
{
	uint32_t pxl_clk_rate = 0;
	uint32_t left_time_to_te_us = 0;
	uint32_t right_time_to_te_us = 0;
	uint32_t te_interval_us = 0;
	char *barcode = NULL;

	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}

	lcd_kit_parse_u32(np, "lcd-kit,panel-xres",
		&pinfo->xres, 1440); // 1440 is default value
	lcd_kit_parse_u32(np, "lcd-kit,panel-yres",
		&pinfo->yres, 2560); // 2560 is default value
	lcd_kit_parse_u32(np, "lcd-kit,panel-fb-xres",
		&pinfo->fb_xres, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-fb-yres",
		&pinfo->fb_yres, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-width",
		&pinfo->width, 73); // 73 is default value
	lcd_kit_parse_u32(np, "lcd-kit,panel-height",
		&pinfo->height, 130); // 130 is default value
	lcd_kit_parse_u32(np, "lcd-kit,panel-orientation",
		&pinfo->orientation, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bpp", &pinfo->bpp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bgr-fmt",
		&pinfo->bgr_fmt, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-type",
		&pinfo->bl_set_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-min",
		&pinfo->bl_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max",
		&pinfo->bl_max, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-dbv-max",
		&pinfo->dbv_max, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-def",
		&pinfo->bl_default, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-cmd-type",
		&pinfo->type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,need-set-dsi1-te-ctrl",
		&pinfo->need_set_dsi1_te_ctrl, 0);
	lcd_kit_parse_u32(np, "lcd-kit,product-type",
		&pinfo->product_type, 0);
	lcd_kit_parse_u8(np, "lcd-kit,panel-frc-enable",
		&pinfo->frc_enable, 0);
	lcd_kit_parse_u8(np, "lcd-kit,panel-esd-skip-mipi-check",
		&pinfo->esd_skip_mipi_check, 1);
	lcd_kit_parse_u32(np, "lcd-kit,panel-dsc-switch",
		&pinfo->dynamic_dsc_en, FPS_CONFIG_EN);
	lcd_kit_parse_u32(np, "lcd-kit,panel-ifbc-type",
		&pinfo->ifbc_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-ic-ctrl-type",
		&pinfo->bl_ic_ctrl_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,blpwm-div",
		&pinfo->blpwm_out_div_value, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-pxl-clk",
		&pxl_clk_rate, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-pxl-clk-div",
		&pinfo->pxl_clk_rate_div, 1);
	lcd_kit_parse_u32(np, "lcd-kit,panel-vsyn-ctr-type",
		&pinfo->vsync_ctrl_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-xcc-set-in-isr-support",
		&pinfo->xcc_set_in_isr_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-pwm-preci-type",
		&pinfo->blpwm_precision_type, 0);
	lcd_kit_parse_u8(np, "lcd-kit,non-check-ldi-porch",
		&pinfo->non_check_ldi_porch, 0);

	/* effect info */
	lcd_kit_parse_u8(np, "lcd-kit,post-hihdr-support",
		&pinfo->post_hihdr_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,display-p3-support",
		&pinfo->p3_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,hdr-flw-support",
		&pinfo->hdr_flw_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,sbl-support",
		&pinfo->sbl_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,gamma-support",
		&pinfo->gamma_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,gmp-support",
		&pinfo->gmp_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,dbv-curve-mapped-support",
		&pinfo->dbv_curve_mapped_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,color-temp-support",
		&pinfo->color_temperature_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,color-temp-rectify-support",
		&pinfo->color_temp_rectify_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,comform-mode-support",
		&pinfo->comform_mode_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,cinema-mode-support",
		&pinfo->cinema_mode_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,xcc-support",
		&pinfo->xcc_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,hiace-support",
		&pinfo->hiace_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,dither-support",
		&pinfo->dither_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,panel-ce-support",
		&pinfo->panel_effect_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,arsr1p-sharpness-support",
		&pinfo->arsr1p_sharpness_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,prefix-sharp-1D-support",
		&pinfo->prefix_sharpness1D_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,prefix-sharp-2D-support",
		&pinfo->prefix_sharpness2D_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,post-xcc-support",
		&pinfo->post_xcc_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,scaling-ratio-threshold",
		&pinfo->scaling_ratio_threshold, 0);
	if (pinfo->hiace_support) {
		lcd_kit_parse_u32(np, "lcd-kit,iglobal-hist-black-pos",
			&pinfo->hiace_param.iGlobalHistBlackPos, 0);
		lcd_kit_parse_u32(np, "lcd-kit,iglobal-hist-white-pos",
			&pinfo->hiace_param.iGlobalHistWhitePos, 0);
		lcd_kit_parse_u32(np, "lcd-kit,iglobal-hist-black-weight",
			&pinfo->hiace_param.iGlobalHistBlackWeight, 0);
		lcd_kit_parse_u32(np, "lcd-kit,iglobal-hist-white-weight",
			&pinfo->hiace_param.iGlobalHistWhiteWeight, 0);
		lcd_kit_parse_u32(np, "lcd-kit,iglobal-hist-zero-cut-ratio",
			&pinfo->hiace_param.iGlobalHistZeroCutRatio, 0);
		lcd_kit_parse_u32(np, "lcd-kit,iglobal-hist-slope-cut-ratio",
			&pinfo->hiace_param.iGlobalHistSlopeCutRatio, 0);
		lcd_kit_parse_u32(np, "lcd-kit,imax-lcd-luminance",
			&pinfo->hiace_param.iMaxLcdLuminance, 0);
		lcd_kit_parse_u32(np, "lcd-kit,imin-lcd-luminance",
			&pinfo->hiace_param.iMinLcdLuminance, 0);
	}
	lcd_kit_parse_u32(np, "lcd-kit,color-temp-rectify-r",
		&pinfo->color_temp_rectify_R, 0);
	lcd_kit_parse_u32(np, "lcd-kit,color-temp-rectify-g",
		&pinfo->color_temp_rectify_G, 0);
	lcd_kit_parse_u32(np, "lcd-kit,color-temp-rectify-b",
		&pinfo->color_temp_rectify_B, 0);
	lcd_kit_parse_u8(np, "lcd-kit,prefix-ce-support",
		&pinfo->prefix_ce_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,acm-support",
		&pinfo->acm_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,acm-ce-support",
		&pinfo->acm_ce_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,smart-color-mode-support",
		&pinfo->smart_color_mode_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,acm-valid-num",
		&pinfo->acm_valid_num, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_hh0", &pinfo->r0_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_lh0", &pinfo->r0_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_hh1", &pinfo->r1_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_lh1", &pinfo->r1_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_hh2", &pinfo->r2_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_lh2", &pinfo->r2_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_hh3", &pinfo->r3_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_lh3", &pinfo->r3_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_hh4", &pinfo->r4_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_lh4", &pinfo->r4_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_hh5", &pinfo->r5_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_lh5", &pinfo->r5_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_hh6", &pinfo->r6_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_lh6", &pinfo->r6_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_hh0",
		&pinfo->video_r0_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_lh0",
		&pinfo->video_r0_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_hh1",
		&pinfo->video_r1_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_lh1",
		&pinfo->video_r1_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_hh2",
		&pinfo->video_r2_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_lh2",
		&pinfo->video_r2_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_hh3",
		&pinfo->video_r3_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_lh3",
		&pinfo->video_r3_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_hh4",
		&pinfo->video_r4_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_lh4",
		&pinfo->video_r4_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_hh5",
		&pinfo->video_r5_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_lh5",
		&pinfo->video_r5_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_hh6",
		&pinfo->video_r6_hh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,r_video_lh6",
		&pinfo->video_r6_lh, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mask-delay-time-before-fp",
		&pinfo->mask_delay.delay_time_before_fp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mask-delay-time-after-fp",
		&pinfo->mask_delay.delay_time_after_fp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-lock-command-support",
		&pinfo->fps_lock_command_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,left-time-to-te-us",
		&left_time_to_te_us, 0);
	lcd_kit_parse_u32(np, "lcd-kit,right-time-to-te-us",
		&right_time_to_te_us, 0);
	lcd_kit_parse_u32(np, "lcd-kit,te-interval-time-us",
		&te_interval_us, 0);
	lcd_kit_parse_u32(np, "lcd-kit,single-deg-support",
		&pinfo->single_deg_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,min-hbm-dbv",
		&pinfo->min_hbm_dbv, 0);
	lcd_kit_parse_u32(np, "lcd-kit,bl-delay-frame",
		&pinfo->bl_delay_frame, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fphbm-entry-delay-afterBL",
		&pinfo->hbm_entry_delay, 0);
	lcd_kit_parse_u8(np, "lcd-kit,ic-dim-ctrl-enable",
		&pinfo->ic_dim_ctrl_enable, 0);

	lcd_kit_parse_u64(np, "lcd-kit,ldi-pipe-clk-rate-pre-set",
		&pinfo->ldi.pipe_clk_rate_pre_set, 0);
	lcd_kit_parse_u32(np, "lcd-kit,ldi-div-pre-set",
		&pinfo->ldi.div_pre_set, 1);

	lcd_kit_parse_u32(np, "lcd-kit,tiny-porch-ratio-support",
		&pinfo->support_tiny_porch_ratio, 0);

	/* sbl info */
	lcd_kit_parse_u32(np, "lcd-kit,sbl-stren-limit",
		&pinfo->smart_bl.strength_limit, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-cal-a",
		&pinfo->smart_bl.calibration_a, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-cal-b",
		&pinfo->smart_bl.calibration_b, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-cal-c",
		&pinfo->smart_bl.calibration_c, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-cal-d",
		&pinfo->smart_bl.calibration_d, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-tf-ctl",
		&pinfo->smart_bl.t_filter_control, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-bl-min",
		&pinfo->smart_bl.backlight_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-bl-max",
		&pinfo->smart_bl.backlight_max, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-bl-scale",
		&pinfo->smart_bl.backlight_scale, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-am-light-min",
		&pinfo->smart_bl.ambient_light_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-filter-a",
		&pinfo->smart_bl.filter_a, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-filter-b",
		&pinfo->smart_bl.filter_b, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-logo-left",
		&pinfo->smart_bl.logo_left, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-logo-top",
		&pinfo->smart_bl.logo_top, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-variance-intensity-space",
		&pinfo->smart_bl.variance_intensity_space, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-slope-max",
		&pinfo->smart_bl.slope_max, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sbl-slope-min",
		&pinfo->smart_bl.slope_min, 0);

	lcd_kit_parse_u8(np, "lcd-kit,dpi01-set-change",
		&pinfo->dpi01_exchange_flag, 0);
	/* ldi info */
	lcd_kit_parse_u32(np, "lcd-kit,h-back-porch",
		&pinfo->ldi.h_back_porch, 0);
	lcd_kit_parse_u32(np, "lcd-kit,h-front-porch",
		&pinfo->ldi.h_front_porch, 0);
	lcd_kit_parse_u32(np, "lcd-kit,h-pulse-width",
		&pinfo->ldi.h_pulse_width, 0);
	lcd_kit_parse_u32(np, "lcd-kit,v-back-porch",
		&pinfo->ldi.v_back_porch, 0);
	lcd_kit_parse_u32(np, "lcd-kit,v-front-porch",
		&pinfo->ldi.v_front_porch, 0);
	lcd_kit_parse_u32(np, "lcd-kit,v-pulse-width",
		&pinfo->ldi.v_pulse_width, 0);
	lcd_kit_parse_u8(np, "lcd-kit,ldi-hsync-plr",
		&pinfo->ldi.hsync_plr, 0);
	lcd_kit_parse_u8(np, "lcd-kit,ldi-vsync-plr",
		&pinfo->ldi.vsync_plr, 0);
	lcd_kit_parse_u8(np, "lcd-kit,ldi-pixel-clk-plr",
		&pinfo->ldi.pixelclk_plr, 0);
	lcd_kit_parse_u8(np, "lcd-kit,ldi-data-en-plr",
		&pinfo->ldi.data_en_plr, 0);
	lcd_kit_parse_u8(np, "lcd-kit,ldi-dpi0-overlap-size",
		&pinfo->ldi.dpi0_overlap_size, 0);
	lcd_kit_parse_u8(np, "lcd-kit,ldi-dpi1-overlap-size",
		&pinfo->ldi.dpi1_overlap_size, 0);

	/* spr and dsc config init */
	OF_PROPERTY_READ_U8_DEFAULT(np,
		"lcd-kit,spr-dsc-mode",
		&pinfo->spr_dsc_mode, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np,
		"lcd-kit,dummy-pixel",
		&pinfo->dummy_pixel_num, 0);
	if (pinfo->spr_dsc_mode)
		lcd_kit_spr_dsc_parse(np, pinfo);
	/* mipi info */
	lcd_kit_parse_u8(np, "lcd-kit,mipi-lane-nums",
		&pinfo->mipi.lane_nums, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-te-type",
		&pinfo->mipi.dsi_te_type, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-color-mode",
		&pinfo->mipi.color_mode, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-vc",
		&pinfo->mipi.vc, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-burst-mode",
		&pinfo->mipi.burst_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-bit-clk",
		&pinfo->mipi.dsi_bit_clk, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-max-tx-esc-clk",
		&pinfo->mipi.max_tx_esc_clk, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-bit-clk-val1",
		&pinfo->mipi.dsi_bit_clk_val1, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-bit-clk-val2",
		&pinfo->mipi.dsi_bit_clk_val2, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-bit-clk-val3",
		&pinfo->mipi.dsi_bit_clk_val3, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-bit-clk-val4",
		&pinfo->mipi.dsi_bit_clk_val4, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-dsi-bit-clk-val5",
		&pinfo->mipi.dsi_bit_clk_val5, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-non-continue-enable",
		&pinfo->mipi.non_continue_en, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-post-adjust",
		&pinfo->mipi.clk_post_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-pre-adjust",
		&pinfo->mipi.clk_pre_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-prepare-adjust",
		&pinfo->mipi.clk_t_hs_prepare_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-lpx-adjust",
		&pinfo->mipi.clk_t_lpx_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-t-lpx-adjust",
		&pinfo->mipi.data_t_lpx_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-trail-adjust",
		&pinfo->mipi.clk_t_hs_trial_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-exit-adjust",
		&pinfo->mipi.clk_t_hs_exit_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-clk-t-hs-zero-adjust",
		&pinfo->mipi.clk_t_hs_zero_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-t-hs-zero-adjust",
		&pinfo->mipi.data_t_hs_zero_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-data-t-hs-trail-adjust",
		&pinfo->mipi.data_t_hs_trial_adjust, 0);
	lcd_kit_parse_u32(np,
		"lcd-kit,mipi-data-lane-lp2hs-time-adjust",
		&pinfo->mipi.data_lane_lp2hs_time_adjust, 0);
	lcd_kit_parse_u32(np,
		"lcd-kit,mipi-data-t-hs-prepare-adjust",
		&pinfo->mipi.data_t_hs_prepare_adjust, 0);
	lcd_kit_parse_u32(np,
		"lcd-kit,mipi-cphy-need-adjust-flag",
		(uint32_t *)&pinfo->mipi.mipi_cphy_adjust.need_adjust_cphy_para, 0);
	if (pinfo->mipi.mipi_cphy_adjust.need_adjust_cphy_para) {
		lcd_kit_parse_u32(np,
			"lcd-kit,mipi-cphy-data-t-post-adjust",
			(uint32_t *)&pinfo->mipi.mipi_cphy_adjust.cphy_data_t_post_adjust, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,mipi-cphy-data-t-pre-adjust",
			(uint32_t *)&pinfo->mipi.mipi_cphy_adjust.cphy_data_t_prepare_adjust, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,mipi-cphy-data-t-pre-begin-adjust",
			(uint32_t *)&pinfo->mipi.mipi_cphy_adjust.cphy_data_t_prebegin_adjust, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,mipi-cphy-data-t-lpx-adjust",
			(uint32_t *)&pinfo->mipi.mipi_cphy_adjust.cphy_data_t_lpx_adjust, 0);
	}
	lcd_kit_parse_u32(np, "lcd-kit,mipi-rg-vrefsel-vcm-adjust",
		&pinfo->mipi.rg_vrefsel_vcm_adjust, 0);
	lcd_kit_parse_u32(np, "lcd-kit,need-adjust-dsi-vcm",
		&pinfo->need_adjust_dsi_vcm, 0);
	if (pinfo->need_adjust_dsi_vcm) {
		lcd_kit_parse_u32(np, "lcd-kit,mipi-rg-lptx-sri-adjust",
			&pinfo->mipi.rg_lptx_sri_adjust, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-rg-vrefsel-lptx-adjust",
			&pinfo->mipi.rg_vrefsel_lptx_adjust, 0);
	}
	lcd_kit_parse_u32(np, "lcd-kit,support-de-emphasis",
		&pinfo->mipi.support_de_emphasis, 0);
	if (pinfo->mipi.support_de_emphasis) {
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,de-emphasis-reg",
			pinfo->mipi.de_emphasis_reg, DE_EMPHASIS_REG_NUM);
		(void)lcd_kit_parse_u32_array(np, "lcd-kit,de-emphasis-value",
			pinfo->mipi.de_emphasis_value, DE_EMPHASIS_REG_NUM);
	}
	lcd_kit_parse_u32(np, "lcd-kit,mipi-phy-mode",
		&pinfo->mipi.phy_mode, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-lp11-flag",
		&pinfo->mipi.lp11_flag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-hs-wr-to-time",
		&pinfo->mipi.hs_wr_to_time, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-phy-update",
		&pinfo->mipi.phy_m_n_count_update, 0);
	lcd_kit_parse_u8(np, "lcd-kit,min-phy-timing-flag",
		&pinfo->mipi.mininum_phy_timing_flag, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-dsi-upt-support",
		&pinfo->dsi_bit_clk_upt_support, 0);
	lcd_kit_parse_u8(np, "lcd-kit,video-idle-mode-support",
		&pinfo->video_idle_mode, 0);
	lcd_kit_parse_u8(np, "lcd-kit,mipi-dsi-timing-support",
		&pinfo->mipi.dsi_timing_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-dsc-switch",
		&pinfo->mipi.dynamic_dsc_en, FPS_CONFIG_EN);
	if (pinfo->mipi.dsi_timing_support) {
		lcd_kit_parse_u32(np, "lcd-kit,mipi-h-sync-area",
			&pinfo->mipi.hsa, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-h-back-porch",
			&pinfo->mipi.hbp, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-h-line-time",
			&pinfo->mipi.hline_time, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-dpi-h-size",
			&pinfo->mipi.dpi_hsize, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-v-sync-area",
			&pinfo->mipi.vsa, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-v-back-porch",
			&pinfo->mipi.vbp, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-v-front-porch",
			&pinfo->mipi.vfp, 0);
		lcd_kit_parse_u32(np, "lcd-kit,mipi-ignore-hporch",
			&pinfo->mipi.ignore_hporch, 0);
	}

	lcd_kit_parse_u8(np, "lcd-kit,backlight-delay-adjust-support",
		&pinfo->delay_set_bl_thr_support, 0);

	if (pinfo->delay_set_bl_thr_support)
		lcd_kit_parse_u8(np, "lcd-kit,backlight-delay-adjust",
			&pinfo->delay_set_bl_thr, 2);

	lcd_kit_parse_u8(np, "lcd-kit,panel-mipi-no-round",
		&pinfo->mipi_no_round, 0);
	/* dirty region update */
	if (common_info->dirty_region.support) {
		pinfo->dirty_region_updt_support =
			common_info->dirty_region.support;
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-left-align",
			&pinfo->dirty_region_info.left_align);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-right-align",
			&pinfo->dirty_region_info.right_align);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-top-align",
			&pinfo->dirty_region_info.top_align);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-bott-align",
			&pinfo->dirty_region_info.bottom_align);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-width-align",
			&pinfo->dirty_region_info.w_align);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-height-align",
			&pinfo->dirty_region_info.h_align);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-width-min",
			&pinfo->dirty_region_info.w_min);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-height-min",
			&pinfo->dirty_region_info.h_min);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-top-start",
			&pinfo->dirty_region_info.top_start);
		lcd_kit_parse_dirty_info(np,
			"lcd-kit,dirty-bott-start",
			&pinfo->dirty_region_info.bottom_start);
	}
	lcd_kit_parse_u32(np, "lcd-kit,dirty-alsc-enable",
		&pinfo->dirty_region_info.alsc.alsc_en, 0);
	if (pinfo->dirty_region_info.alsc.alsc_en) {
		lcd_kit_parse_u32(np, "lcd-kit,dirty-alsc-addr",
			&pinfo->dirty_region_info.alsc.alsc_addr, 0);
		lcd_kit_parse_u32(np, "lcd-kit,dirty-alsc-size",
			&pinfo->dirty_region_info.alsc.alsc_size, 0);
		pinfo->dirty_region_info.alsc.pic_size =
			((pinfo->xres - 1) << ALSC_PIC_WIDTH_SHIFT) | (pinfo->yres - 1);
	}
	pinfo->pxl_clk_rate = (uint64_t)pxl_clk_rate;
	pinfo->left_time_to_te_us = (uint64_t)left_time_to_te_us;
	pinfo->right_time_to_te_us = (uint64_t)right_time_to_te_us;
	pinfo->te_interval_us = (uint64_t)te_interval_us;
	if (pinfo->single_deg_support != 0)
		pinfo->current_display_region = EN_DISPLAY_REGION_A;
	/* bl pwm */
	lcd_kit_parse_u32(np, "lcd-kit,blpwm-disable",
		&pinfo->blpwm_input_disable, 0);
	if ((pinfo->bl_set_type == BL_SET_BY_BLPWM) &&
		(pinfo->blpwm_input_disable == 0))
		pinfo->blpwm_input_ena = 1;
	pinfo->cascadeic_support = disp_info->cascade_ic.support;
	pinfo->rgbw_support = disp_info->rgbw.support;
	pinfo->lcd_uninit_step_support = 1;
	// change MHZ to HZ
	pinfo->pxl_clk_rate = pinfo->pxl_clk_rate * 1000000UL;
	pinfo->mipi.dsi_bit_clk_upt = pinfo->mipi.dsi_bit_clk;
	// change MHZ to HZ
	pinfo->mipi.max_tx_esc_clk = pinfo->mipi.max_tx_esc_clk * 1000000;
	pinfo->panel_name = common_info->panel_name;
	pinfo->board_version = disp_info->board_version;

	pinfo->local_hbm_support = common_info->hbm.local_hbm_support;

	lcd_kit_parse_u8(np, "lcd-kit,aod-esd-mutex-support",
		&pinfo->aod_esd_flag, 0);

	/* esd */
	if (common_info->esd.support) {
		pinfo->esd_enable = 1;
		lcd_kit_parse_u32(np, "lcd-kit,esd-recovery-max-count",
			&pinfo->esd_recovery_max_count, 10);
		/* esd_check_max_count set 3 times as default */
		lcd_kit_parse_u32(np, "lcd-kit,esd-check-max-count",
			&pinfo->esd_check_max_count, 3);
		lcd_kit_parse_u8(np, "lcd-kit,esd-timing-ctrl",
			&pinfo->esd_timing_ctrl, 0);
		lcd_kit_parse_u32(np, "lcd-kit,esd-check-time-period",
			&pinfo->esd_check_time_period, 0);
	}
	lcd_kit_fps_init(np, pinfo);
	/* aod */
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,ramless-aod",
		&pinfo->ramless_aod, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,update-core-clk-support",
		&pinfo->update_core_clk_support, 0);
	/* read barcode from dts */
	if (pinfo->product_type == LCD_FOLDER_TYPE) {
		lcd_kit_parse_u32(np, "lcd-kit,barcode-length",
			&pinfo->sn_code_length, 0);
		lcd_kit_parse_string(np, "lcd-kit,barcode-value",
			(const char **)&barcode);
		if ((pinfo->sn_code_length != 0) &&
			(pinfo->sn_code_length <= sizeof(pinfo->sn_code)) &&
			(barcode != NULL))
			memcpy(pinfo->sn_code, barcode, pinfo->sn_code_length);
	}
	/* support_ddr_bw_adjust */
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,support-ddr-bw-adjust",
		&pinfo->support_ddr_bw_adjust, 0);
}

static int lcd_kit_panel_version_read(struct hisi_fb_data_type *hisifd)
{
	int ret;
	if (disp_info->panel_version.enter_cmds.cmds != NULL) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->panel_version.enter_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	ret = lcd_kit_dsi_cmds_rx(hisifd,
		(uint8_t *)disp_info->panel_version.read_value,
		VERSION_VALUE_NUM_MAX - 1,
		&disp_info->panel_version.cmds);
	if (ret)
		return LCD_KIT_FAIL;
	if (disp_info->panel_version.exit_cmds.cmds != NULL) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->panel_version.exit_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	if (disp_info->panel_version.secend_enter_cmds.cmds != NULL) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->panel_version.secend_enter_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	if (disp_info->panel_version.secend_cmds.cmds != NULL) {
		ret = lcd_kit_dsi_cmds_rx(hisifd,
			(uint8_t *)disp_info->panel_version.read_value +
				(int)disp_info->panel_version.value_number -
				disp_info->panel_version.secend_value_number,
			VERSION_VALUE_NUM_MAX - 1,
			&disp_info->panel_version.secend_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	if (disp_info->panel_version.secend_exit_cmds.cmds != NULL) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->panel_version.secend_exit_cmds);
		if (ret)
			return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

int lcd_kit_panel_version_init(struct hisi_fb_data_type *hisifd)
{
	int i, j;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_kit_panel_version_read(hisifd) == LCD_KIT_FAIL) {
		LCD_KIT_ERR("read panel version cmd fail\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < (int)disp_info->panel_version.version_number; i++) {
		for (j = 0; j < (int)disp_info->panel_version.value_number; j++) {
			LCD_KIT_INFO("read_value[%d]:0x%x\n", j,
				disp_info->panel_version.read_value[j]);
			LCD_KIT_INFO("expected_value[%d].buf[%d]:0x%x\n", i, j,
				disp_info->panel_version.value.arry_data[i].buf[j]);
			if (disp_info->panel_version.read_value[j] !=
				disp_info->panel_version.value.arry_data[i].buf[j])
				break;

			if (j == ((int)disp_info->panel_version.value_number - 1)) {
				memcpy(hisifd->panel_info.lcd_panel_version, " VER:", strlen(" VER:") + 1);
				strncat(hisifd->panel_info.lcd_panel_version,
					disp_info->panel_version.lcd_version_name[i],
					strlen(disp_info->panel_version.lcd_version_name[i]));
				LCD_KIT_INFO("Panel version is %s\n",
					hisifd->panel_info.lcd_panel_version);
				return LCD_KIT_OK;
			}
		}
	}

	if (i == disp_info->panel_version.version_number) {
		LCD_KIT_INFO("panel_version not find\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_FAIL;
}

void lcd_kit_parse_effect(struct device_node *np)
{
	int ret;

	/* gamma calibration */
	lcd_kit_parse_u32(np, "lcd-kit,gamma-cal-support",
		&disp_info->gamma_cal.support, 0);
	if (disp_info->gamma_cal.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,gamma-cal-cmds",
			"lcd-kit,gamma-cal-cmds-state",
			&disp_info->gamma_cal.cmds);
	/* brightness and color uniform */
	lcd_kit_parse_u32(np,
		"lcd-kit,brightness-color-uniform-support",
		&disp_info->oeminfo.brightness_color_uniform.support, 0);
	if (disp_info->oeminfo.brightness_color_uniform.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,brightness-color-cmds",
			"lcd-kit,brightness-color-cmds-state",
			&disp_info->oeminfo.brightness_color_uniform.brightness_color_cmds);
	/* oem information */
	lcd_kit_parse_u32(np, "lcd-kit,oem-info-support",
		&disp_info->oeminfo.support, 0);
	if (disp_info->oeminfo.support) {
		lcd_kit_parse_u32(np, "lcd-kit,oem-barcode-2d-support",
			&disp_info->oeminfo.barcode_2d.support, 0);
		if (disp_info->oeminfo.barcode_2d.support) {
			lcd_kit_parse_u32(np,
				"lcd-kit,oem-barcode-2d-block-num",
				&disp_info->oeminfo.barcode_2d.block_num, 3);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,barcode-2d-cmds",
				"lcd-kit,barcode-2d-cmds-state",
				&disp_info->oeminfo.barcode_2d.cmds);
		}
	}
	/* rgbw */
	lcd_kit_parse_u32(np, "lcd-kit,rgbw-support",
		&disp_info->rgbw.support, 0);
	if (disp_info->rgbw.support) {
		lcd_kit_parse_u32(np, "lcd-kit,rgbw-bl-max",
			&disp_info->rgbw.rgbw_bl_max, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode1-cmds",
			"lcd-kit,rgbw-mode1-cmds-state",
			&disp_info->rgbw.mode1_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode2-cmds",
			"lcd-kit,rgbw-mode2-cmds-state",
			&disp_info->rgbw.mode2_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode3-cmds",
			"lcd-kit,rgbw-mode3-cmds-state",
			&disp_info->rgbw.mode3_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-mode4-cmds",
			"lcd-kit,rgbw-mode4-cmds-state",
			&disp_info->rgbw.mode4_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-backlight-cmds",
			"lcd-kit,rgbw-backlight-cmds-state",
			&disp_info->rgbw.backlight_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-saturation-cmds",
			"lcd-kit,rgbw-saturation-cmds-state",
			&disp_info->rgbw.saturation_ctrl_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-frame-gain-limit-cmds",
			"lcd-kit,rgbw-frame-gain-limit-cmds-state",
			&disp_info->rgbw.frame_gain_limit_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-frame-gain-speed-cmds",
			"lcd-kit,rgbw-frame-gain-speed-cmds-state",
			&disp_info->rgbw.frame_gain_speed_cmds);
		lcd_kit_parse_dcs_cmds(np,
			"lcd-kit,rgbw-color-distor-allowance-cmds",
			"lcd-kit,rgbw-color-distor-allowance-cmds-state",
			&disp_info->rgbw.color_distor_allowance_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-pixel-gain-limit-cmds",
			"lcd-kit,rgbw-pixel-gain-limit-cmds-state",
			&disp_info->rgbw.pixel_gain_limit_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-pixel-gain-speed-cmds",
			"lcd-kit,rgbw-pixel-gain-speed-cmds-state",
			&disp_info->rgbw.pixel_gain_speed_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,rgbw-pwm-gain-cmds",
			"lcd-kit,rgbw-pwm-gain-cmds-state",
			&disp_info->rgbw.pwm_gain_cmds);
	}
	/* demura */
	ret = lcd_kit_parse_u32(np, "lcd-kit,demura-support",
		&disp_info->demura.support, 0);
	if (ret) {
		LCD_KIT_INFO("of_property_read_u32:demura-support not find\n");
		disp_info->demura.support = NOT_SUPPORT;
	}
	if (disp_info->demura.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-read-fir-cmds",
			"lcd-kit,demura-read-fir-cmds-state",
			&disp_info->demura.r_fir_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-read-con-cmds",
			"lcd-kit,demura-read-con-cmds-state",
			&disp_info->demura.r_con_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-read-chksum-cmds",
			"lcd-kit,demura-read-chksum-cmds-state",
			&disp_info->demura.rr_chksum_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-rw-chksum-cmds",
			"lcd-kit,demura-rw-chksum-cmds-state",
			&disp_info->demura.rw_chksum_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi0-pre-cmds",
			"lcd-kit,demura-dsi0-pre-cmds-state",
			&disp_info->demura.d0_w_pre_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi1-pre-cmds",
			"lcd-kit,demura-dsi1-pre-cmds-state",
			&disp_info->demura.d1_w_pre_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi0-w-fir-cmds",
			"lcd-kit,demura-dsi0-w-fir-cmds-state",
			&disp_info->demura.d0_w_fir_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi1-w-fir-cmds",
			"lcd-kit,demura-dsi1-w-fir-cmds-state",
			&disp_info->demura.d1_w_fir_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi0-w-con-cmds",
			"lcd-kit,demura-dsi1-w-con-cmds-state",
			&disp_info->demura.d0_w_con_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi1-w-con-cmds",
			"lcd-kit,demura-dsi1-w-con-cmds-state",
			&disp_info->demura.d1_w_con_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi0-w-end-cmds",
			"lcd-kit,demura-dsi0-w-end-cmds-state",
			&disp_info->demura.d0_w_end_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi1-w-end-cmds",
			"lcd-kit,demura-dsi1-w-end-cmds-state",
			&disp_info->demura.d1_w_end_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi0-w-irdrop-cmds",
			"lcd-kit,demura-dsi0-w-irdrop-cmds-state",
			&disp_info->demura.d0_w_ird_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-dsi1-w-irdrop-cmds",
			"lcd-kit,demura-dsi1-w-irdrop-cmds-state",
			&disp_info->demura.d1_w_ird_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-d0-w-ird-pre-cmds",
			"lcd-kit,demura-d0-w-ird-pre-cmds-state",
			&disp_info->demura.d0_w_ird_pre_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-d1-w-ird-pre-cmds",
			"lcd-kit,demura-d1-w-ird-pre-cmds-state",
			&disp_info->demura.d1_w_ird_pre_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-d0-w-ird-end-cmds",
			"lcd-kit,demura-d0-w-ird-end-cmds-state",
			&disp_info->demura.d0_w_ird_end_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,demura-d1-w-ird-end-cmds",
			"lcd-kit,demura-d1-w-ird-end-cmds-state",
			&disp_info->demura.d1_w_ird_end_cmds);
	}
}

static void lcd_kit_parse_fps_gamma(struct device_node *np)
{
	/* fps gamma code */
	lcd_kit_parse_u32(np, "lcd-kit,fps-gamma-support",
		&disp_info->fps_gamma.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-gamma-length-support",
		&disp_info->fps_gamma.check_length_update_support, 0);

	if (disp_info->fps_gamma.support) {
		lcd_kit_parse_u32(np, "lcd-kit,flash-gamma-total-length",
			&disp_info->fps_gamma.flash_gamma_total_length,
			FLASH_GAMMA_TOTAL_LEN_MAX);
		if (disp_info->fps_gamma.flash_gamma_total_length <=
			GAMMA_TOTAL_LEN ||
			disp_info->fps_gamma.flash_gamma_total_length >
			FLASH_GAMMA_TOTAL_LEN_MAX) {
				LCD_KIT_ERR("invalid flash length!");
				disp_info->fps_gamma.support = 0;
				return;
		}
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,otp-gamma-read-cmds",
			"lcd-kit,otp-gamma-read-cmds-state",
			&disp_info->fps_gamma.read_otp);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,flash-gamma-read-enter"
			"-cmds", "lcd-kit,flash-gamma-read-enter-cmds-state",
			&disp_info->fps_gamma.read_flash_enter);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,flash-gamma-read-cmds",
			"lcd-kit,flash-gamma-read-cmds-state",
			&disp_info->fps_gamma.read_flash);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,flash-gamma-read-exit-cmds",
			"lcd-kit,flash-gamma-read-exit-cmds-state",
			&disp_info->fps_gamma.read_flash_exit);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,gamma-write-cmds",
			"lcd-kit,gamma-write-cmds-state",
			&disp_info->fps_gamma.write_60hz);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,gamma-write-cmds",
			"lcd-kit,gamma-write-cmds-state",
			&disp_info->fps_gamma.write_90hz);
		lcd_kit_parse_u32(np, "lcd-kit,fps-gamma-swap-support",
			&disp_info->fps_gamma.swap_support, 0);
	}
}

static void lcd_kit_parse_frame_odd_even(struct device_node *node)
{
	/* odd even frame code */
	lcd_kit_parse_u32(node, "lcd-kit,frame-odd-even-support",
		&disp_info->frame_odd_even.support, 0);
	if (disp_info->frame_odd_even.support)
		lcd_kit_parse_dcs_cmds(node, "lcd-kit,odd-even-read-cmds",
			"lcd-kit,odd-even-read-cmds-state",
			&disp_info->frame_odd_even.read);
}

void lcd_kit_parse_fps(struct device_node *np)
{
	int index;

	lcd_kit_parse_u32(np, "lcd-kit,fps-switch-test-support",
		&disp_info->fps.fps_switch_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-effect-delay-frame-cnt",
		&disp_info->fps.fps_effective_frame_count, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-send-dfr-cmd-in-vactive",
		&disp_info->fps.fps_send_dfr_cmd_in_vactive, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-switch-mode",
		&disp_info->fps.fps_switch_mode, 1);
	lcd_kit_parse_u32(np, "lcd-kit,fps-need-high-60-mode",
		&disp_info->fps.fps_need_high_60, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-need-extend-cmd",
		&disp_info->fps.fps_need_extend_cmds, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-ifbc-type",
		&disp_info->fps.fps_ifbc_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-disable-dsi-clk-upt",
		&disp_info->fps.fps_disable_dsi_clk_upt, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-high-mode-switch-no-tx-cmd",
		&disp_info->fps.hfps_switch_no_need_cmd, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-check-after-power-on-delay",
		&disp_info->fps.fps_check_panel_on_delay, 0);
	index = LCD_FPS_SCENCE_60P;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds",
		"lcd-kit,fps-to-60-cmds-state",
		&disp_info->fps.fps_to_cmds[index]);
	lcd_kit_parse_array_data(np, "lcd-kit,fps-60-dsi-timming",
		&disp_info->fps.fps_dsi_timming[index]);
	index = LCD_FPS_SCENCE_60;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-normal-60-cmds",
		"lcd-kit,fps-to-normal-60-cmds-state",
		&disp_info->fps.fps_to_cmds[index]);
	lcd_kit_parse_array_data(np, "lcd-kit,fps-normal-60-dsi-timming",
		&disp_info->fps.fps_dsi_timming[index]);
	index = LCD_FPS_SCENCE_90;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds",
		"lcd-kit,fps-to-90-cmds-state", &disp_info->fps.fps_to_cmds[index]);
	lcd_kit_parse_array_data(np, "lcd-kit,fps-90-dsi-timming",
		&disp_info->fps.fps_dsi_timming[index]);
	index = LCD_FPS_SCENCE_120;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-120-cmds",
		"lcd-kit,fps-to-120-cmds-state", &disp_info->fps.fps_to_cmds[index]);
	lcd_kit_parse_array_data(np, "lcd-kit,fps-120-dsi-timming",
		&disp_info->fps.fps_dsi_timming[index]);
	lcd_kit_parse_array_data(np, "lcd-kit,fps-switch-order",
		&disp_info->fps.fps_switch_order);
	lcd_kit_parse_array_data(np, "lcd-kit,panel-support-fps-list",
		&disp_info->fps.panel_support_fps_list);
	lcd_kit_parse_u32(np, "lcd-kit,fps-need_update_on",
		&disp_info->fps.fps_need_update_on, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fps-update-to90",
		&disp_info->fps.fps_update_to_90, 0);
	// parse fps gamma if needed
	lcd_kit_parse_fps_gamma(np);
	// parse frame odd even if needed
	lcd_kit_parse_frame_odd_even(np);
}

static void lcd_kit_parse_amoled_power_ic_check(struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,amoled-power-ic-check-support",
		&disp_info->amoled_power_ic_check.support, 0);
	if (disp_info->amoled_power_ic_check.support) {
		lcd_kit_parse_u32(np, "lcd-kit,amoled-power-ic-check-gpio",
			&disp_info->amoled_power_ic_check.gpio, 0);
		lcd_kit_parse_u32(np, "lcd-kit,amoled-power-ic-check-expect-value",
			&disp_info->amoled_power_ic_check.expect_value, 0);
	}
}

static void lcd_kit_parse_panel_irc(struct device_node *np)
{
	/* panel irc code */
	lcd_kit_parse_u32(np, "lcd-kit,irc-support",
		&disp_info->panel_irc.support, 0);
	if (disp_info->panel_irc.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,irc-on-cmds",
			"lcd-kit,irc-on-cmds-state",
			&disp_info->panel_irc.on);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,irc-off-cmds",
			"lcd-kit,irc-off-cmds-state",
			&disp_info->panel_irc.off);
	}
}

static void lcd_kit_parse_sot_err_check(struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,check-sot-err-support",
		&disp_info->sot_err_check.support, 0);
	if (!disp_info->sot_err_check.support)
		return;

	lcd_kit_parse_dcs_cmds(np, "lcd-kit,check-sot-err-cmds",
		"lcd-kit,check-sot-err-cmds-state",
		&disp_info->sot_err_check.check_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,sot-err-repair11-cmds",
		"lcd-kit,sot-err-repair-cmds-state",
		&disp_info->sot_err_check.repair11_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,sot-err-repair12-cmds",
		"lcd-kit,sot-err-repair-cmds-state",
		&disp_info->sot_err_check.repair12_cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,sot-err-value",
		&disp_info->sot_err_check.check_value);
	disp_info->sot_err_check.dsi0_repair_flag = SOT_REPAIR_NONE;
	disp_info->sot_err_check.dsi1_repair_flag = SOT_REPAIR_NONE;
	disp_info->sot_err_check.dsi0_sot_err_cnt = 0;
	disp_info->sot_err_check.dsi1_sot_err_cnt = 0;
}

void lcd_kit_parse_util(struct device_node *np)
{
	char *name[VERSION_NUM_MAX] = {NULL};
	int i, ret;

	/* alpm */
	lcd_kit_parse_u32(np, "lcd-kit,alpm-support", &disp_info->alpm.support, 0);
	if (disp_info->alpm.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,alpm-low-light-cmds",
			"lcd-kit,alpm-low-light-cmds-state",
			&disp_info->alpm.low_light_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,alpm-off-cmds",
			"lcd-kit,alpm-off-cmds-state",
			&disp_info->alpm.off_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,alpm-exit-cmds",
			"lcd-kit,alpm-exit-cmds-state",
			&disp_info->alpm.exit_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,alpm-high-light-cmds",
			"lcd-kit,alpm-high-light-cmds-state",
			&disp_info->alpm.high_light_cmds);
	}
	/* quickly sleep out */
	lcd_kit_parse_u32(np, "lcd-kit,quickly-sleep-out-support",
		&disp_info->quickly_sleep_out.support, 0);
	if (disp_info->quickly_sleep_out.support)
		lcd_kit_parse_u32(np,
			"lcd-kit,quickly-sleep-out-interval",
			&disp_info->quickly_sleep_out.interval, 0);
	/* fps */
	lcd_kit_parse_u32(np, "lcd-kit,fps-support",
		&disp_info->fps.support, 0);
	if (disp_info->fps.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-dfr-disable-cmds",
			"lcd-kit,fps-dfr-disable-cmds-state",
			&disp_info->fps.dfr_disable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-dfr-enable-cmds",
			"lcd-kit,fps-dfr-enable-cmds-state",
			&disp_info->fps.dfr_enable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-30-cmds",
			"lcd-kit,fps-to-30-cmds-state",
			&disp_info->fps.fps_to_30_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds",
			"lcd-kit,fps-to-60-cmds-state",
			&disp_info->fps.fps_to_60_cmds);
		lcd_kit_parse_fps(np);
	}

	lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-support",
				&disp_info->pwrkey_press.support, 0);
	if (disp_info->pwrkey_press.support) {
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-addr",
				&disp_info->pwrkey_press.rst_addr, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-pwroff-flag",
				&disp_info->pwrkey_press.power_off_flag, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-esd-status",
				&disp_info->pwrkey_press.esd_status, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time1",
				&disp_info->pwrkey_press.configtime1, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time2",
				&disp_info->pwrkey_press.configtime2, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time3",
				&disp_info->pwrkey_press.configtime3, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pwrkey-press-rst-time4",
				&disp_info->pwrkey_press.configtime4, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,quick-power-off-cmds",
				"lcd-kit,quick-power-off-cmds-state",
				&disp_info->pwrkey_press.cmds);
	}
	lcd_kit_parse_u32(np, "lcd-kit,pwr-off-time-optimize-support",
		&disp_info->pwr_off_optimize_info.support, 0);
	if (disp_info->pwr_off_optimize_info.support)
		lcd_kit_parse_u32(np, "lcd-kit,pwr-off-time-work-delay",
			&disp_info->pwr_off_optimize_info.delay_time, 0);
	lcd_kit_parse_u32(np, "lcd-kit,cascade-ic-support",
				&disp_info->cascade_ic.support, 0);
	if (disp_info->cascade_ic.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cascade-ic-region-a-cmds",
				"lcd-kit,cascade-ic-region-a-cmds-state",
				&disp_info->cascade_ic.region_a_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cascade-ic-region-b-cmds",
				"lcd-kit,cascade-ic-region-b-cmds-state",
				&disp_info->cascade_ic.region_b_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cascade-ic-region-ab-cmds",
				"lcd-kit,cascade-ic-region-ab-cmds-state",
				&disp_info->cascade_ic.region_ab_cmds);
		lcd_kit_parse_dcs_cmds(np,
				"lcd-kit,cascade-ic-region-ab-fold-cmds",
				"lcd-kit,cascade-ic-region-ab-fold-cmds-state",
				&disp_info->cascade_ic.region_ab_fold_cmds);
	}
	/* panel version */
	lcd_kit_parse_u32(np, "lcd-kit,panel-version-support",
		&disp_info->panel_version.support, 0);
	if (disp_info->panel_version.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-version-enter-cmds",
			"lcd-kit,panel-version-enter-cmds-state",
			&disp_info->panel_version.enter_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-version-cmds",
			"lcd-kit,panel-version-cmds-state",
			&disp_info->panel_version.cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-version-exit-cmds",
			"lcd-kit,panel-version-exit-cmds-state",
			&disp_info->panel_version.exit_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-secend-version-enter-cmds",
			"lcd-kit,panel-secend-version-enter-cmds-state",
			&disp_info->panel_version.secend_enter_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-secend-version-cmds",
			"lcd-kit,panel-secend-version-cmds-state",
			&disp_info->panel_version.secend_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-secend-version-exit-cmds",
			"lcd-kit,panel-secend-version-exit-cmds-state",
			&disp_info->panel_version.secend_exit_cmds);
		disp_info->panel_version.value_number =
			disp_info->panel_version.cmds.cmds->dlen -
			disp_info->panel_version.cmds.cmds->payload[1];
		if(disp_info->panel_version.secend_cmds.cmds != NULL) {
			disp_info->panel_version.secend_value_number =
				disp_info->panel_version.secend_cmds.cmds->dlen -
				disp_info->panel_version.secend_cmds.cmds->payload[1];
			disp_info->panel_version.value_number += disp_info->panel_version.secend_value_number;
			LCD_KIT_INFO("value_num=%d secend_value_number = %d\n",
				disp_info->panel_version.value_number,
				disp_info->panel_version.secend_value_number);
		}
		lcd_kit_parse_arrays_data(np, "lcd-kit,panel-version-value",
			&disp_info->panel_version.value,
			disp_info->panel_version.value_number);
		disp_info->panel_version.version_number =
			disp_info->panel_version.value.cnt;

		LCD_KIT_INFO("Panel version value_num=%d version_num = %d\n",
			disp_info->panel_version.value_number,
			disp_info->panel_version.version_number);
		if (disp_info->panel_version.version_number > 0) {
			ret = lcd_kit_parse_string_array(np,
				"lcd-kit,panel-version",
				(const char **)&name[0],
				disp_info->panel_version.version_number);
			if (ret < 0)
				LCD_KIT_INFO("Panel version parse fail\n");
			for (i = 0; i < (int)disp_info->panel_version.version_number; i++) {
				strncpy(disp_info->panel_version.lcd_version_name[i],
					name[i], LCD_PANEL_VERSION_SIZE - 1);
				LCD_KIT_INFO("Panel version name[%d] = %s\n",
					i, name[i]);
			}
		}
	}
	/* gamma otp */
	lcd_kit_parse_u32(np, "lcd-kit,otp-gamma-support",
		&disp_info->otp_gamma.support, 0);
	if (disp_info->otp_gamma.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,elvss-cmds",
			"lcd-kit,elvss-cmds-state",
			&disp_info->otp_gamma.elvss_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,otp-gamma-cmds",
			"lcd-kit,otp-gamma-cmds-state",
			&disp_info->otp_gamma.gamma_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,otp-gray-cmds",
			"lcd-kit,otp-gray-cmds-state",
			&disp_info->otp_gamma.gray_cmds);
	}
	/* separate transmission */
	lcd_kit_parse_u32(np, "lcd-kit,separate-transmission-support",
		&disp_info->transmission.separate_support, 0);
	if (disp_info->transmission.separate_support)
		lcd_kit_parse_u32(np, "lcd-kit,separate-transmission-status-flag",
			&disp_info->transmission.separate_status_flag, 0x8000);
	/* pcd errflag */
	lcd_kit_parse_u32(np, "lcd-kit,pcd-errflag-check-support",
		&disp_info->pcd_errflag.pcd_errflag_check_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,gpio-pcd",
		&disp_info->pcd_errflag.gpio_pcd, 0);
	lcd_kit_parse_u32(np, "lcd-kit,gpio-errflag",
		&disp_info->pcd_errflag.gpio_errflag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,pcd-cmds-support",
		&disp_info->pcd_errflag.pcd_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,pcd-power-on-support",
		&disp_info->pcd_errflag.power_on_support, 1);
	if (disp_info->pcd_errflag.pcd_support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,start-pcd-check-cmds",
			"lcd-kit,start-pcd-check-cmds-state",
			&disp_info->pcd_errflag.start_pcd_check_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,pcd-read-cmds",
			"lcd-kit,pcd-read-cmds-state",
			&disp_info->pcd_errflag.read_pcd_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,switch-page-cmds",
			"lcd-kit,switch-page-cmds-state",
			&disp_info->pcd_errflag.switch_page_cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,pcd-check-reg-value",
			&disp_info->pcd_errflag.pcd_value);
		lcd_kit_parse_u32(np, "lcd-kit,pcd-check-reg-value-compare-mode",
			&disp_info->pcd_errflag.pcd_value_compare_mode, 0);
		lcd_kit_parse_u32(np, "lcd-kit,exp-pcd-mask",
			&disp_info->pcd_errflag.exp_pcd_mask, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pcd-det-num",
			&disp_info->pcd_errflag.pcd_det_num, 1);
	}
	lcd_kit_parse_u32(np, "lcd-kit,errflag-cmds-support",
		&disp_info->pcd_errflag.errflag_support, 0);
	if (disp_info->pcd_errflag.errflag_support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,errflag-read-cmds",
			"lcd-kit,errflag-read-cmds-state",
			&disp_info->pcd_errflag.read_errflag_cmds);
	lcd_kit_parse_u32(np, "lcd-kit,dbv-stat-support",
		&disp_info->dbv_stat.support, 0);
	/* power ic error flag check */
	lcd_kit_parse_amoled_power_ic_check(np);
	/* elvdd detect */
	lcd_kit_parse_u32(np, "lcd-kit,elvdd-detect-support",
		&disp_info->elvdd_detect.support, 0);
	if (disp_info->elvdd_detect.support) {
		lcd_kit_parse_u32(np, "lcd-kit,elvdd-detect-type",
			&disp_info->elvdd_detect.detect_type, 0);
		if (disp_info->elvdd_detect.detect_type ==
			ELVDD_MIPI_CHECK_MODE) {
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,elvdd-detect-cmds",
				"lcd-kit,elvdd-detect-cmds-state",
				&disp_info->elvdd_detect.cmds);
		} else {
			lcd_kit_parse_u32(np,
				"lcd-kit,elvdd-detect-gpio",
				&disp_info->elvdd_detect.detect_gpio, 0);
			lcd_kit_parse_u32(np,
				"lcd-kit,elvdd-detect-gpio-type",
				&disp_info->elvdd_detect.detect_gpio_type, 0);
		}
		lcd_kit_parse_u32(np,
			"lcd-kit,elvdd-detect-value",
			&disp_info->elvdd_detect.exp_value, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,elvdd-value-mask",
			&disp_info->elvdd_detect.exp_value_mask, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,elvdd-delay",
			&disp_info->elvdd_detect.delay, 0);
	}
	lcd_kit_parse_panel_irc(np);
	lcd_kit_parse_u32(np, "lcd-kit,panel-dual-dsi-send-sync",
		&disp_info->dual_dsi_send_sync, 0);
	lcd_kit_parse_u32(np, "lcd-kit,mipi-cmd-always-lp-mode",
		&disp_info->mipi_cmds_always_run_in_lpmode, 0);
	/* sot error check */
	lcd_kit_parse_sot_err_check(np);
}

void lcd_kit_parse_dt(struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return;
	}
	/* parse effect info */
	lcd_kit_parse_effect(np);
	/* parse normal function */
	lcd_kit_parse_util(np);
}

static void lcd_dmd_report_err(uint32_t err_no, const char *info, int info_len)
{
	if (!info) {
		LCD_KIT_ERR("info is NULL Pointer\n");
		return;
	}
#if defined(CONFIG_HUAWEI_DSM)
	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, info);
		dsm_client_notify(lcd_dclient, err_no);
	}
#endif
}

int lcd_kit_start_pcd_check(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL\n");
		return -1;
	}
	if (disp_info->pcd_errflag.pcd_support)
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->pcd_errflag.start_pcd_check_cmds);
	return ret;
}

static int lcd_kit_judge_pcd_dmd(uint8_t *read_val,
	uint32_t *expect_val, int cnt, uint32_t compare_mode)
{
	int i;
	uint32_t exp_pcd_value_mask;

	exp_pcd_value_mask = disp_info->pcd_errflag.exp_pcd_mask;
	if (read_val == NULL || expect_val == NULL) {
		LCD_KIT_ERR("read_val or expect_val is NULL\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("pcd check compare mode is %d\n", compare_mode);
	if (compare_mode == PCD_COMPARE_MODE_EQUAL) {
		for (i = 0; i < cnt; i++) {
			if ((uint32_t)read_val[i] != expect_val[i])
				return LCD_KIT_FAIL;
		}
	} else if (compare_mode == PCD_COMPARE_MODE_BIGGER) {
		if ((uint32_t)read_val[0] < expect_val[0])
			return LCD_KIT_FAIL;
	} else if (compare_mode == PCD_COMPARE_MODE_MASK) {
		if (((uint32_t)read_val[0] & exp_pcd_value_mask) == expect_val[0])
			return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

#define PCD_READ_LEN 3
static void lcd_kit_pcd_dmd_report(uint8_t *pcd_read_val, uint32_t val_len)
{
	int ret;
	char err_info[DMD_ERR_INFO_LEN] = {0};

	if (val_len < PCD_READ_LEN) {
		LCD_KIT_ERR("val len err\n");
		return;
	}
	if (!pcd_read_val) {
		LCD_KIT_ERR("pcd_read_val is NULL\n");
		return;
	}
	ret = snprintf(err_info, DMD_ERR_INFO_LEN,
		"PCD REG Value is 0x%x 0x%x 0x%x\n",
		pcd_read_val[0], pcd_read_val[1], pcd_read_val[2]);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error\n");
		return;
	}
	lcd_dmd_report_err(DSM_LCD_PANEL_CRACK_ERROR_NO, err_info,
		 DMD_ERR_INFO_LEN);
}

void lcd_kit_ddic_lv_detect_dmd_report(
	u8 reg_val[DETECT_LOOPS][DETECT_NUM][VAL_NUM])
{
	int i;
	int ret;
	unsigned int len;
	char err_info[DMD_DET_ERR_LEN] = {0};

	if (!reg_val) {
		LCD_KIT_ERR("reg_val is NULL\n");
		return;
	}
	for (i = 0; i < DETECT_LOOPS; i++) {
		len = strlen(err_info);
		if (len >= DMD_DET_ERR_LEN) {
			LCD_KIT_ERR("strlen error\n");
			return;
		}
		ret = snprintf(err_info + len, DMD_DET_ERR_LEN - len,
			"%d: %x %x, %x %x, %x %x, %x %x ",
			i + DET_START,
			reg_val[i][DET1_INDEX][VAL_1],
			reg_val[i][DET1_INDEX][VAL_0],
			reg_val[i][DET2_INDEX][VAL_1],
			reg_val[i][DET2_INDEX][VAL_0],
			reg_val[i][DET3_INDEX][VAL_1],
			reg_val[i][DET3_INDEX][VAL_0],
			reg_val[i][DET4_INDEX][VAL_1],
			reg_val[i][DET4_INDEX][VAL_0]);
		if (ret < 0) {
			LCD_KIT_ERR("snprintf error\n");
			return;
		}
	}
	lcd_dmd_report_err(DSM_LCD_DDIC_LV_DETECT_ERROR_NO, err_info,
		DMD_DET_ERR_LEN);
}

void pcd_test(uint8_t *result, uint8_t *pcd_read_val)
{
	error_num_pcd++;
	if (error_num_pcd >= disp_info->pcd_errflag.pcd_det_num) {
		LCD_KIT_INFO("pcd detect num = %d\n", error_num_pcd);
		lcd_kit_pcd_dmd_report(pcd_read_val, LCD_KIT_PCD_SIZE);
		*result |= PCD_FAIL;
		error_num_pcd = 0;
	}
}

int lcd_kit_check_pcd_errflag_check(struct hisi_fb_data_type *hisifd)
{
	uint8_t result = PCD_ERRFLAG_SUCCESS;
	int ret;
	uint8_t read_pcd[LCD_KIT_PCD_SIZE] = {0};
	uint8_t read_errflag[LCD_KIT_ERRFLAG_SIZE] = {0};
	uint32_t *expect_value = NULL;
	uint32_t expect_value_cnt;
	int i;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL\n");
		return -1;
	}
	if (disp_info->pcd_errflag.pcd_support) {
		(void)lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->pcd_errflag.switch_page_cmds);
		ret = lcd_kit_dsi_cmds_rx(hisifd, read_pcd, LCD_KIT_PCD_SIZE - 1,
			&disp_info->pcd_errflag.read_pcd_cmds);
		expect_value = disp_info->pcd_errflag.pcd_value.buf;
		expect_value_cnt = disp_info->pcd_errflag.pcd_value.cnt;
		if (ret == LCD_KIT_OK) {
			if (lcd_kit_judge_pcd_dmd(read_pcd, expect_value, expect_value_cnt,
				disp_info->pcd_errflag.pcd_value_compare_mode) == LCD_KIT_OK) {
				pcd_test(&result, read_pcd);
			} else {
				LCD_KIT_INFO("pcd detect num = %d\n", error_num_pcd);
				error_num_pcd = 0;
			}
		} else {
			LCD_KIT_ERR("read pcd err\n");
		}
		LCD_KIT_INFO("pcd REG read result is 0x%x 0x%x 0x%x\n",
			read_pcd[0], read_pcd[1], read_pcd[2]);
		LCD_KIT_INFO("pcd check result is %d\n", result);
	}
	/* Reserve interface, redevelop when needed */
	if (disp_info->pcd_errflag.errflag_support) {
		(void)lcd_kit_dsi_cmds_rx(hisifd, read_errflag,
			LCD_KIT_ERRFLAG_SIZE - 1,
			&disp_info->pcd_errflag.read_errflag_cmds);
		for (i = 0; i < LCD_KIT_ERRFLAG_SIZE; i++) {
			if (read_errflag[i] != 0) {
				result |= ERRFLAG_FAIL;
				break;
			}
		}
	}
	return (int)result;
}

static int lcd_kit_gpio_pcd_errflag_read(const int gpio_no, int *read_value)
{
	if (!gpio_no) {
		/* only pcd check or only errflag check */
		LCD_KIT_INFO("gpio_no is 0\n");
		return LCD_KIT_OK;
	}

	if (gpio_request(gpio_no, GPIO_PCD_ERRFLAG_NAME)) {
		LCD_KIT_ERR("pcd_errflag_gpio[%d] request fail!\n", gpio_no);
		return LCD_KIT_FAIL;
	}
	if (gpio_direction_input(gpio_no)) {
		gpio_free(gpio_no);
		LCD_KIT_ERR("pcd_errflag_gpio[%d] direction set fail!\n",
			gpio_no);
		return LCD_KIT_FAIL;
	}
	*read_value = gpio_get_value(gpio_no);
	gpio_free(gpio_no);
	return LCD_KIT_OK;
}

int lcd_kit_gpio_pcd_errflag_check(void)
{
	int pcd_gpio = disp_info->pcd_errflag.gpio_pcd;
	int errflag_gpio = disp_info->pcd_errflag.gpio_errflag;
	int pcd_gpio_value = 0;
	int errflag_gpio_value = 0;

	if (!disp_info->pcd_errflag.pcd_errflag_check_support) {
		LCD_KIT_INFO("no support pcd_errflag check, default pass\n");
		return PCD_ERRFLAG_SUCCESS;
	}

	if (!pcd_gpio && !errflag_gpio) {
		LCD_KIT_INFO("pcd_errflag gpio is 0, default pass\n");
		return PCD_ERRFLAG_SUCCESS;
	}

	if (lcd_kit_gpio_pcd_errflag_read(pcd_gpio, &pcd_gpio_value))
		return PCD_ERRFLAG_SUCCESS;

	if (lcd_kit_gpio_pcd_errflag_read(errflag_gpio,
		&errflag_gpio_value))
		return PCD_ERRFLAG_SUCCESS;

	LCD_KIT_INFO("pcd_gpio[%d]=%d, errflag_gpio[%d]=%d\n",
		pcd_gpio, pcd_gpio_value,
		errflag_gpio, errflag_gpio_value);
	if ((pcd_gpio_value == GPIO_HIGH_PCDERRFLAG) &&
		(errflag_gpio_value == GPIO_LOW_PCDERRFLAG))
		return PCD_FAIL;
	else if ((pcd_gpio_value == GPIO_LOW_PCDERRFLAG) &&
		(errflag_gpio_value == GPIO_HIGH_PCDERRFLAG))
		return ERRFLAG_FAIL;
	else if ((pcd_gpio_value == GPIO_HIGH_PCDERRFLAG) &&
		(errflag_gpio_value == GPIO_HIGH_PCDERRFLAG))
		return PCD_ERRFLAG_FAIL;

	return PCD_ERRFLAG_SUCCESS;
}

int lcd_kit_read_gamma(struct hisi_fb_data_type *hisifd, uint8_t *read_value,
	int len)
{
	if (disp_info == NULL) {
		LCD_KIT_ERR("disp_info null pointer\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->gamma_cal.cmds.cmds == NULL) {
		LCD_KIT_ERR("gamma_cal cmds null pointer\n");
		return LCD_KIT_FAIL;
	}
	/* change addr to dsi cmds */
	disp_info->gamma_cal.cmds.cmds->payload[1] = (disp_info->gamma_cal.addr >> 8) & 0xff;
	disp_info->gamma_cal.cmds.cmds->payload[0] = disp_info->gamma_cal.addr & 0xff;
	disp_info->gamma_cal.cmds.cmds->dlen = disp_info->gamma_cal.length;
	lcd_kit_dsi_cmds_rx(hisifd, read_value, len - 1,
		&disp_info->gamma_cal.cmds);
	return LCD_KIT_OK;
}

void lcd_kit_read_power_status(struct hisi_fb_data_type *hisifd)
{
	uint32_t status;
	uint32_t try_times = 0;
	uint32_t status1;
	uint32_t try_times1 = 0;
	char __iomem *dsi_base_addr = NULL;

	dsi_base_addr = lcd_kit_get_dsi_base_addr(hisifd);
	outp32(dsi_base_addr + MIPIDSI_GEN_HDR_OFFSET, 0x0A06);
	status = inp32(dsi_base_addr + MIPIDSI_CMD_PKT_STATUS_OFFSET);
	while (status & 0x10) {
		udelay(50);
		if (++try_times > 100) {
			try_times = 0;
			LCD_KIT_ERR("Read lcd power status timeout!\n");
			break;
		}

		status = inp32(dsi_base_addr + MIPIDSI_CMD_PKT_STATUS_OFFSET);
	}
	status = inp32(dsi_base_addr + MIPIDSI_GEN_PLD_DATA_OFFSET);
	LCD_KIT_INFO("LCD Power State = 0x%x. is_dual_mipi_panel = 0x%x\n",
		status, is_dual_mipi_panel(hisifd));
	if (is_dual_mipi_panel(hisifd)) {
		outp32(hisifd->mipi_dsi1_base + MIPIDSI_GEN_HDR_OFFSET, 0x0A06);
		status1 = inp32(hisifd->mipi_dsi1_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		while (status1 & 0x10) {
			udelay(50);
			if (++try_times1 > 100) {
				try_times1 = 0;
				LCD_KIT_ERR("dsi1 Read lcd power status timeout!\n");
				break;
			}

			status1 = inp32(hisifd->mipi_dsi1_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		}
		status1 = inp32(hisifd->mipi_dsi1_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
		LCD_KIT_INFO("DSI1 LCD Power State = 0x%x\n", status1);
	}
}

static u32 xcc_table_def[] = {
	0x0, 0x8000, 0x0, 0x0, 0x0, 0x0, 0x8000, 0x0, 0x0, 0x0, 0x0, 0x8000
};

static void parse_sbl(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("sbl:", command, strlen("sbl:")))
		return;
	if (command[strlen("sbl:")] == '0')
		pinfo->sbl_support = 0;
	else
		pinfo->sbl_support = 1;
}

static void parse_xcc(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("xcc_support:", command, strlen("xcc_support:")))
		return;
	if (command[strlen("xcc_support:")] == '0') {
		pinfo->xcc_support = 0;
		if (pinfo->xcc_table) {
			pinfo->xcc_table[1] = 0x8000; // 0x8000 is xcc value
			pinfo->xcc_table[6] = 0x8000;
			pinfo->xcc_table[11] = 0x8000;
		}
	} else {
		pinfo->xcc_support = 1;
		if (pinfo->xcc_table == NULL) {
			pinfo->xcc_table = xcc_table_def;
			pinfo->xcc_table_len = ARRAY_SIZE(xcc_table_def);
		}
	}
}

static void parse_dsi_clk_upt(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("dsi_bit_clk_upt:", command, strlen("dsi_bit_clk_upt:")))
		return;
	if (command[strlen("dsi_bit_clk_upt:")] == '0')
		pinfo->dsi_bit_clk_upt_support = 0;
	else
		pinfo->dsi_bit_clk_upt_support = 1;
}

static void parse_dirty_region_upt(struct hisi_panel_info *pinfo,
	const char *command)
{
	if (strncmp("dirty_region_upt:", command, strlen("dirty_region_upt:")))
		return;
	if (command[strlen("dirty_region_upt:")] == '0')
		pinfo->dirty_region_updt_support = 0;
	else
		pinfo->dirty_region_updt_support = 1;
}

static void parse_ifbc_type(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("ifbc_type:", command, strlen("ifbc_type:")))
		return;
	if (command[strlen("ifbc_type:")] == '0') {
		if (pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) {
			// ldi
			pinfo->ldi.h_back_porch *= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_front_porch *= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_pulse_width *= pinfo->pxl_clk_rate_div;
			pinfo->pxl_clk_rate_div = 1;
			pinfo->ifbc_type = IFBC_TYPE_NONE;
		}
	} else if (command[strlen("ifbc_type:")] == '7') {
		if (pinfo->ifbc_type == IFBC_TYPE_NONE) {
			pinfo->pxl_clk_rate_div = 3;
			// ldi
			pinfo->ldi.h_back_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_front_porch /= pinfo->pxl_clk_rate_div;
			pinfo->ldi.h_pulse_width /= pinfo->pxl_clk_rate_div;
			pinfo->ifbc_type = IFBC_TYPE_VESA3X_SINGLE;
		}
	}
}

static void parse_esd_enable(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("esd_enable:", command, strlen("esd_enable:")))
		return;
	if (command[strlen("esd_enable:")] == '0')
		pinfo->esd_enable = 0;
	else
		pinfo->esd_enable = 1;
}

static void parse_bl_delay_enable(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("bl_delay_enable:", command, strlen("bl_delay_enable:")))
		return;
	if (command[strlen("bl_delay_enable:")] == '0')
		pinfo->delay_set_bl_thr_support = 0;
	else
		pinfo->delay_set_bl_thr_support = 1;
}


static void parse_fps_updt_support(struct hisi_panel_info *pinfo,
	const char *command)
{
	if (strncmp("fps_updt_support:", command, strlen("fps_updt_support:")))
		return;
	if (command[strlen("fps_updt_support:")] == '0')
		pinfo->fps_updt_support = 0;
	else
		pinfo->fps_updt_support = 1;
}

static void parse_fps_func_switch(struct hisi_panel_info *pinfo,
	const char *command)
{
	if (strncmp("fps_func_switch:", command, strlen("fps_func_switch:")))
		return;
	if (command[strlen("fps_func_switch:")] == '0')
		disp_info->fps.support = 0;
	else
		disp_info->fps.support = 1;
}

static void parse_blpwm_input_ena(struct hisi_panel_info *pinfo,
	const char *command)
{
	if (strncmp("blpwm_input_ena:", command, strlen("blpwm_input_ena:")))
		return;
	if (command[strlen("blpwm_input_ena:")] == '0')
		pinfo->blpwm_input_ena = 0;
	else
		pinfo->blpwm_input_ena = 1;
}

static void parse_lane_nums(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("lane_nums:", command, strlen("lane_nums:")))
		return;
	if ((command[strlen("lane_nums:")] == '1') &&
		(pinfo->mipi.lane_nums_select_support & DSI_1_LANES_SUPPORT))
		pinfo->mipi.lane_nums = DSI_1_LANES;
	else if ((command[strlen("lane_nums:")] == '2') &&
		(pinfo->mipi.lane_nums_select_support & DSI_2_LANES_SUPPORT))
		pinfo->mipi.lane_nums = DSI_2_LANES;
	else if ((command[strlen("lane_nums:")] == '3') &&
		(pinfo->mipi.lane_nums_select_support & DSI_3_LANES_SUPPORT))
		pinfo->mipi.lane_nums = DSI_3_LANES;
	else
		pinfo->mipi.lane_nums = DSI_4_LANES;
}

static void parse_panel_effect(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("panel_effect_support:", command,
		strlen("panel_effect_support:")))
		return;
	if (command[strlen("panel_effect_support:")] == '0')
		pinfo->panel_effect_support = 0;
	else
		pinfo->panel_effect_support = 1;
}

static void parse_col_temp_rectify(struct hisi_panel_info *pinfo,
	const char *command)
{
	if (strncmp("color_temp_rectify_support:", command,
		strlen("color_temp_rectify_support:")))
		return;
	if (command[strlen("color_temp_rectify_support:")] == '0')
		pinfo->color_temp_rectify_support = 0;
	else
		pinfo->color_temp_rectify_support = 1;
}

static void parse_ddic_rgbw(struct hisi_panel_info *pinfo, const char *command)
{
	if (strncmp("ddic_rgbw_support:", command, strlen("ddic_rgbw_support:")))
		return;
	if (command[strlen("ddic_rgbw_support:")] == '0') {
		pinfo->rgbw_support = 0;
		disp_info->rgbw.support = 0;
	} else {
		pinfo->rgbw_support = 1;
	}
}

int lcd_kit_parse_switch_cmd(struct hisi_fb_data_type *hisifd,
	const char *command)
{
	struct hisi_panel_info *pinfo = NULL;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!command) {
		LCD_KIT_ERR("command is null\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	parse_sbl(pinfo, command);
	parse_xcc(pinfo, command);
	parse_dsi_clk_upt(pinfo, command);
	parse_dirty_region_upt(pinfo, command);
	parse_ifbc_type(pinfo, command);
	parse_esd_enable(pinfo, command);
	parse_bl_delay_enable(pinfo, command);
	parse_fps_updt_support(pinfo, command);
	parse_fps_func_switch(pinfo, command);
	parse_blpwm_input_ena(pinfo, command);
	parse_lane_nums(pinfo, command);
	parse_panel_effect(pinfo, command);
	parse_col_temp_rectify(pinfo, command);
	parse_ddic_rgbw(pinfo, command);
	hisifb_display_effect_func_switch(hisifd, command);
	return LCD_KIT_OK;
}

static int lcd_kit_get_project_id(char *buff)
{
	if (!buff) {
		LCD_KIT_ERR("buff is null\n");
		return LCD_KIT_FAIL;
	}
	strncpy(buff, disp_info->project_id.id, strlen(disp_info->project_id.id));
	return LCD_KIT_OK;
}

int lcd_kit_get_online_status(void)
{
	int status = LCD_ONLINE;

	if (!strncmp(disp_info->compatible, LCD_KIT_DEFAULT_PANEL,
		strlen(disp_info->compatible)))
		/* panel is online */
		status = LCD_OFFLINE;
	LCD_KIT_INFO("status = %d\n", status);
	return status;
}

int lcd_kit_get_status_by_type(int type, int *status)
{
	int ret;

	if (status == NULL) {
		LCD_KIT_ERR("status is null\n");
		return LCD_KIT_FAIL;
	}
	switch (type) {
	case LCD_ONLINE_TYPE:
		*status = lcd_kit_get_online_status();
		ret = LCD_KIT_OK;
		break;
	case PT_STATION_TYPE:
#ifdef LCD_FACTORY_MODE
		*status = lcd_kit_get_pt_station_status();
#endif
		ret = LCD_KIT_OK;
		break;
	default:
		LCD_KIT_ERR("not support type\n");
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}

struct lcd_kit_brightness_color_oeminfo g_brightness_color_oeminfo;
void lcd_kit_read_calicolordata_from_share_mem(struct lcd_kit_brightness_color_oeminfo *oeminfo)
{
	void *vir_addr = 0;

	if (oeminfo == NULL) {
		LCD_KIT_ERR("point is NULL!\n");
		return;
	}

	vir_addr = (void *)ioremap_wc(HISI_SUB_RESERVED_BRIGHTNESS_CHROMA_MEM_PHYMEM_BASE,
		HISI_SUB_RESERVED_BRIGHTNESS_CHROMA_MEM_PHYMEM_SIZE);
	if (!vir_addr) {
		LCD_KIT_ERR("mem ioremap error !\n");
		return;
	}
	memcpy((void *)oeminfo, (void *)vir_addr,
		sizeof(struct lcd_kit_brightness_color_oeminfo));
	iounmap(vir_addr);
}

struct lcd_kit_brightness_color_oeminfo *lcd_kit_get_brightness_color_oeminfo(void)
{
	return &g_brightness_color_oeminfo;
}

int lcd_kit_realtime_set_xcc(struct hisi_fb_data_type *hisifd, const char *buf,
	size_t count)
{
	ssize_t ret = 0;
	int retval = 0;
	struct hisi_fb_panel_data *pdata = NULL;

	if (!hisifd || !buf) {
		LCD_KIT_ERR("NULL pointer\n");
		return LCD_KIT_FAIL;
	}

	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (!pdata) {
		LCD_KIT_ERR("NULL pointer\n");
		return LCD_KIT_FAIL;
	}

	if (pdata->lcd_xcc_store) {
		ret = pdata->lcd_xcc_store(hisifd->pdev, buf, count);
		if (ret == count) {
			retval = 0;
		} else {
			LCD_KIT_ERR("set lcd xcc failed!\n");
			retval = -1;
		}
	} else {
		LCD_KIT_ERR("dpe_lcd_xcc_store is NULL\n");
		retval = -1;
	}
	return retval;
}

void lcd_kit_set_actual_bl_max_nit(void)
{
	common_info->actual_bl_max_nit =
		g_brightness_color_oeminfo.color_params.white_decay_luminace;
}

static void lcd_dsi_tx_lp_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch low-power,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7, 3, 8);
	/* gen long cmd write switch low-power */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 14);
	/*
	 * dcs short cmd write switch high-speed,
	 * include 0-parameter,1-parameter
	 */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x3, 2, 16);
}

static void lcd_dsi_tx_hs_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch low-power,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 3, 8);
	/* gen long cmd write switch high-speed */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 1, 14);
	/*
	 * dcs short cmd write switch high-speed,
	 * include 0-parameter,1-parameter
	 */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 2, 16);
}

static void lcd_dsi_rx_lp_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch low-power,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7, 3, 11);
	/* dcs short cmd read switch low-power */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 18);
	/* read packet size cmd switch low-power */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 24);
}

static void lcd_dsi_rx_hs_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch high-speed,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 3, 11);
	/* dcs short cmd read switch high-speed */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 1, 18);
	/* read packet size cmd switch high-speed */
	set_reg(dsi_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x0, 1, 24);
}

void lcd_kit_set_mipi_link(struct hisi_fb_data_type *hisifd,
	int link_state)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *dsi_base_addr = NULL;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return;
	}
	pinfo = &hisifd->panel_info;
	if (!pinfo) {
		LCD_KIT_ERR("panel_info is NULL!\n");
		return;
	}
	if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		LCD_KIT_INFO("lowpower stage, can not set!\n");
		return;
	}
	if (disp_info->mipi_cmds_always_run_in_lpmode)
		link_state = LCD_KIT_DSI_LP_MODE;
	dsi_base_addr = lcd_kit_get_dsi_base_addr(hisifd);
	if (is_mipi_cmd_panel(hisifd)) {
		if (!disp_info->mipi_cmds_always_run_in_lpmode) {
			/* wait fifo empty */
			(void)lcd_kit_dsi_fifo_is_empty(dsi_base_addr);
			if (is_dual_mipi_panel(hisifd))
				lcd_kit_dsi_fifo_is_empty(hisifd->mipi_dsi1_base);
		}
		LCD_KIT_INFO("link_state:%d\n", link_state);
		switch (link_state) {
		case LCD_KIT_DSI_LP_MODE:
			lcd_dsi_tx_lp_mode_cfg(dsi_base_addr);
			lcd_dsi_rx_lp_mode_cfg(dsi_base_addr);
			if (is_dual_mipi_panel(hisifd)) {
				lcd_dsi_tx_lp_mode_cfg(hisifd->mipi_dsi1_base);
				lcd_dsi_rx_lp_mode_cfg(hisifd->mipi_dsi1_base);
			}
			break;
		case LCD_KIT_DSI_HS_MODE:
			lcd_dsi_tx_hs_mode_cfg(dsi_base_addr);
			lcd_dsi_rx_hs_mode_cfg(dsi_base_addr);
			if (is_dual_mipi_panel(hisifd)) {
				lcd_dsi_tx_hs_mode_cfg(hisifd->mipi_dsi1_base);
				lcd_dsi_rx_hs_mode_cfg(hisifd->mipi_dsi1_base);
			}
			break;
		default:
			LCD_KIT_ERR("not support mode\n");
			break;
		}
	}
}

int lcd_kit_get_value_from_dts(char *compatible, const char *dts_name, u32 *value)
{
	struct device_node *np = NULL;

	if (!compatible || !dts_name || !value) {
		LCD_KIT_ERR("null pointer found!\n");
		return LCD_KIT_FAIL;
	}
	np = of_find_compatible_node(NULL, NULL, compatible);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", compatible);
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32(np, dts_name, value, 0);
	return LCD_KIT_OK;
}

static bool lcd_kit_panel_event_skip_delay(void *hld,
	uint32_t event, uint32_t data)
{
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = (struct hisi_fb_data_type *)hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("param is NULL!\n");
		return false;
	}

	if ((hisifd->panel_info.product_type == LCD_FOLDER_TYPE) &&
		(hisifd->panel_info.skip_power_on_off == FOLD_POWER_ON_OFF) &&
		(event == EVENT_MIPI) &&
		(data == 0)) {
		LCD_KIT_INFO("Fold type skip delay!\n");
		return true;
	}
	return lcd_kit_get_power_status();
}

static int lcd_kit_power_monitor_on(void)
{
	return ina231_power_monitor_on();
}

static int lcd_kit_power_monitor_off(void)
{
	return ina231_power_monitor_off();
}

int lcd_kit_set_otp_gamma(struct hisi_fb_data_type *hisifd)
{
	int ret, i;
	u8 temp;
	static bool first_set;
	struct lcd_kit_dsi_cmd_desc *cmds = NULL;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!first_set) {
		cmds = disp_info->otp_gamma.gamma_cmds.cmds;
		for (i = 0; i < (GAMMA_MAX - GAMMA_HEAD_LEN); i++) {
			temp = disp_info->otp_gamma.gamma[i + GAMMA_HEAD_LEN];
			if (cmds && cmds->payload)
				cmds->payload[i + 1] = temp;
		}
		first_set = true;
	}
	/* adjust elvss */
	ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->otp_gamma.elvss_cmds);
	if (ret) {
		LCD_KIT_ERR("send adjust elvss cmd error\n");
		return ret;
	}
	/* send otp gamma */
	ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->otp_gamma.gamma_cmds);
	if (ret) {
		LCD_KIT_ERR("send otp gamma cmd error\n");
		return ret;
	}
	return ret;
}

int lcd_kit_set_otp_gray(struct hisi_fb_data_type *hisifd)
{
	int ret, i;
	u8 temp;
	struct lcd_kit_dsi_cmd_desc *cmds = NULL;
	static bool first_set;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!first_set) {
		/* get gray cmds:0xC7 */
		cmds = disp_info->otp_gamma.gray_cmds.cmds;
		cmds++;
		/* set up gray cmds */
		for (i = 0; i < (GAMMA_MAX - GAMMA_HEAD_LEN); i++) {
			temp = disp_info->otp_gamma.gamma[i + GAMMA_HEAD_LEN];
			if (cmds && cmds->payload)
				cmds->payload[i + 1] = temp;
		}
		first_set = true;
	}
	/* send otp gray */
	ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->otp_gamma.gray_cmds);
	if (ret) {
		LCD_KIT_ERR("send otp gray cmd error\n");
		return ret;
	}
	return ret;
}

int lcd_kit_write_otp_gamma(u8 *buf)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!disp_info->otp_gamma.support) {
		LCD_KIT_INFO("not support otp gamma\n");
		return ret;
	}
	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	/* print gamma head and len */
	LCD_KIT_INFO("HEAD:0x%x, LEN:0x%x\n", buf[0], buf[1]);
	/* verify gamma */
	if ((buf[0] != GAMMA_HEAD && buf[0] != GRAY_HEAD) ||
		(buf[1] != GAMMA_LEN)) {
		LCD_KIT_INFO("not otp gamma\n");
		return ret;
	}
	/* copy to gamma buffer */
	memcpy(disp_info->otp_gamma.gamma, buf, GAMMA_MAX);
	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("panel is power off\n");
		up(&hisifd->blank_sem);
		return LCD_KIT_FAIL;
	}
	hisifb_activate_vsync(hisifd);
	if (buf[0] == GAMMA_HEAD)
		lcd_kit_set_otp_gamma(hisifd);
	else if (buf[0] == GRAY_HEAD)
		lcd_kit_set_otp_gray(hisifd);
	hisifb_deactivate_vsync(hisifd);
	up(&hisifd->blank_sem);
	return ret;
}

static int lcd_kit_set_vss_by_thermal(void)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_panel_ops *panel_ops = NULL;

	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}

	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_set_vss_by_thermal) {
		down(&hisifd->power_sem);
		if (!hisifd->panel_power_on) {
			LCD_KIT_ERR("panel is power off\n");
			up(&hisifd->power_sem);
			return LCD_KIT_FAIL;
		}
		hisifb_vsync_disable_enter_idle(hisifd, true);
		hisifb_activate_vsync(hisifd);
		ret = panel_ops->lcd_set_vss_by_thermal((void *)hisifd);
		hisifb_vsync_disable_enter_idle(hisifd, false);
		hisifb_deactivate_vsync(hisifd);
		up(&hisifd->power_sem);
	}

	return ret;
}

static void lcd_kit_set_power_by_thermal_handle(void *hld)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct thermal_zone_device *thermal_zone = NULL;
	unsigned int cur_thermal = 0;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return;
	}
	hisifd = (struct hisi_fb_data_type *)hld;
	if (!hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return;
	}

	thermal_zone = thermal_zone_get_zone_by_name("shell_front");
	if (IS_ERR_OR_NULL(thermal_zone)) {
		LCD_KIT_ERR("Failed getting thermal zone!\n");
		return;
	}
	ret = thermal_zone_get_temp(thermal_zone, &cur_thermal);
	if (ret) {
		LCD_KIT_ERR("get_temperature fail!!");
		return;
	}
	if (((common_info->set_power.get_thermal == 0) &&
		(cur_thermal <= common_info->set_power.thermal1)) ||
		((common_info->set_power.oldthermal >
		common_info->set_power.thermal2) &&
		(cur_thermal <= common_info->set_power.thermal2))) {
		ret = adapt_ops->mipi_tx((void *)hisifd,
			&common_info->set_power.cmds_fir);
		LCD_KIT_INFO("thermal mode1\n");
	} else if (((common_info->set_power.get_thermal == 0) ||
		(common_info->set_power.oldthermal
		<= common_info->set_power.thermal1)) &&
		(cur_thermal > common_info->set_power.thermal1)) {
		ret = adapt_ops->mipi_tx((void *)hisifd,
			&common_info->set_power.cmds_sec);
		LCD_KIT_INFO("thermal mode2\n");
	}
	common_info->set_power.get_thermal = 1;
	common_info->set_power.oldthermal = cur_thermal;
}

static int lcd_kit_set_power_by_thermal(void)
{
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}

	down(&hisifd->power_sem);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("panel is power off\n");
		up(&hisifd->power_sem);
		return LCD_KIT_FAIL;
	}
	hisifb_vsync_disable_enter_idle(hisifd, true);
	hisifb_activate_vsync(hisifd);
	lcd_kit_set_power_by_thermal_handle(hisifd);
	hisifb_vsync_disable_enter_idle(hisifd, false);
	hisifb_deactivate_vsync(hisifd);
	up(&hisifd->power_sem);

	return LCD_KIT_OK;
}

static int lcd_kit_proximity_power_off(void)
{
	struct hisi_fb_data_type *hisifd = NULL;

	LCD_KIT_INFO("[Proximity_feature] lcd_kit_proximity_power_off enter!\n");
	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->thp_proximity.support) {
		LCD_KIT_INFO("[Proximity_feature] thp_proximity not support exit!\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_kit_get_pt_mode()) {
		LCD_KIT_INFO("[Proximity_feature] pt test mode exit!\n");
		return LCD_KIT_FAIL;
	}
	down(&DISP_LOCK->thp_second_poweroff_sem);
	if (common_info->thp_proximity.panel_power_state == POWER_ON) {
		LCD_KIT_INFO("[Proximity_feature] power state is on exit!\n");
		up(&DISP_LOCK->thp_second_poweroff_sem);
		return LCD_KIT_FAIL;
	}
	if (common_info->thp_proximity.panel_power_state == POWER_TS_SUSPEND) {
		LCD_KIT_INFO("[Proximity_feature] power off suspend state exit!\n");
		up(&DISP_LOCK->thp_second_poweroff_sem);
		return LCD_KIT_OK;
	}
	if (common_info->thp_proximity.work_status == TP_PROXMITY_DISABLE) {
		LCD_KIT_INFO("[Proximity_feature] thp_proximity has been disabled exit!\n");
		up(&DISP_LOCK->thp_second_poweroff_sem);
		return LCD_KIT_FAIL;
	}
	common_info->thp_proximity.work_status = TP_PROXMITY_DISABLE;
	if (common_ops->panel_power_off)
		common_ops->panel_power_off(hisifd);
	up(&DISP_LOCK->thp_second_poweroff_sem);
	LCD_KIT_INFO("[Proximity_feature] lcd_kit_proximity_power_off exit!\n");
	return LCD_KIT_OK;
}

static int lcd_kit_get_sn_code_info(struct lcd_kit_sn_code_info *info)
{
	int index = 0;
	uint32_t sn_info;
	struct hisi_panel_info *pinfo = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	sn_info = common_info->sn_code.sn_code_info.buf[index++];
	if (sn_info > 0)
		info->mode_info = pinfo->sn_code[sn_info - 1];
	else
		info->mode_info = '0';
	sn_info = common_info->sn_code.sn_code_info.buf[index++];
	if (sn_info > 0)
		info->glass_info = pinfo->sn_code[sn_info - 1];
	else
		info->glass_info = '0';
	sn_info = common_info->sn_code.sn_code_info.buf[index++];
	if (sn_info > 0)
		info->cg_info = pinfo->sn_code[sn_info - 1];
	else
		info->cg_info = '0';
	sn_info = common_info->sn_code.sn_code_info.buf[index];
	if (sn_info > 0)
		info->bl_info = pinfo->sn_code[sn_info - 1];
	else
		info->bl_info = '0';
	return LCD_KIT_OK;
}

static int lcd_kit_hiview_tplcd(struct lcd_kit_tplcd_info *tplcd)
{
	int ret;
	uint8_t sn_info[2] = {0};
	struct lcd_kit_sn_code_info *sn_code_info = NULL;
	struct hiview_hievent *hi_event = hiview_hievent_create(DSM_LCD_SN_CHECK_ERROR_NO);
	if (hi_event == NULL) {
		LCD_KIT_ERR("create hievent fail\n");
		return LCD_KIT_FAIL;
	}
	sn_code_info = kzalloc(sizeof(struct lcd_kit_sn_code_info), GFP_KERNEL);
	if (sn_code_info == NULL) {
		LCD_KIT_ERR("malloc fail\n");
		hiview_hievent_destroy(hi_event);
		return LCD_KIT_FAIL;
	}
	if (lcd_kit_get_sn_code_info(sn_code_info) != 0) {
		LCD_KIT_ERR("get sn code info fail\n");
		hiview_hievent_destroy(hi_event);
		kfree(sn_code_info);
		return LCD_KIT_FAIL;
	}
	sn_info[0] = tplcd->sn_check;
	hiview_hievent_put_string(hi_event, "TplcdSNChk", (char *)sn_info);
	hiview_hievent_put_string(hi_event, "TplcdHashSN", (char *)tplcd->sn_hash);
	sn_info[0] = sn_code_info->mode_info;
	hiview_hievent_put_string(hi_event, "TplcdModMan", (char *)sn_info);
	sn_info[0] = sn_code_info->glass_info;
	hiview_hievent_put_string(hi_event, "TplcdGlaMan", (char *)sn_info);
	sn_info[0] = sn_code_info->cg_info;
	hiview_hievent_put_string(hi_event, "TplcdCGMan", (char *)sn_info);
	sn_info[0] = sn_code_info->bl_info;
	hiview_hievent_put_string(hi_event, "TplcdBacMan", (char *)sn_info);

	ret = hiview_hievent_report(hi_event);
	if (ret <= 0) {
		LCD_KIT_ERR("hiview_hievent_report failed\n");
		hiview_hievent_destroy(hi_event);
		kfree(sn_code_info);
		return LCD_KIT_FAIL;
	}
	hiview_hievent_destroy(hi_event);
	kfree(sn_code_info);
	return LCD_KIT_OK;
}

static unsigned long lcd_kit_get_sn_mem_addr(struct lcd_kit_tplcd_info *tplcd)
{
	return (HISI_SUB_RESERVED_BRIGHTNESS_CHROMA_MEM_PHYMEM_BASE +
		HISI_SUB_RESERVED_BRIGHTNESS_CHROMA_MEM_PHYMEM_SIZE - sizeof(*tplcd));
}

void lcd_kit_sn_check(void)
{
	if (common_info->sn_code.support &&
		common_info->sn_code.check_support) {
		void *vir_addr = NULL;
		struct lcd_kit_tplcd_info *tplcd = NULL;
		unsigned long begin_addr;
		tplcd = kzalloc(sizeof(struct lcd_kit_tplcd_info), GFP_KERNEL);
		if (tplcd == NULL) {
			LCD_KIT_ERR(" malloc fail\n");
			return;
		}
		begin_addr = lcd_kit_get_sn_mem_addr(tplcd);
		/*lint -e446*/
		vir_addr = (void *)ioremap_wc(begin_addr, (unsigned long)sizeof(*tplcd));
		if (!vir_addr) {
			LCD_KIT_ERR("mem ioremap error!\n");
			kfree(tplcd);
			return;
		}
		memcpy((void *)tplcd, (void *)vir_addr, sizeof(*tplcd));
		iounmap(vir_addr);
		/*lint -e446*/
		if (tplcd->sn_check == '1') {
			LCD_KIT_INFO("sn check ok!\n");
			kfree(tplcd);
			return;
		}
		LCD_KIT_INFO("report tplcd info to HiView\n");
		if (lcd_kit_hiview_tplcd(tplcd) != 0)
			LCD_KIT_ERR("HiView tplcd info failed\n");
		kfree(tplcd);
	}
}

static int lcd_kit_get_sn_code(void)
{
	int ret;
	struct hisi_panel_info *pinfo = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_panel_ops *panel_ops = NULL;
	char read_value[OEM_INFO_SIZE_MAX + 1] = {0};

	if (common_info && common_info->sn_code.support) {
		hisifd = hisifd_list[PRIMARY_PANEL_IDX];
		if (!hisifd) {
			LCD_KIT_ERR("hisifd is null\n");
			return LCD_KIT_FAIL;
		}
		pinfo = &(hisifd->panel_info);
		if (!pinfo) {
			LCD_KIT_ERR("pinfo is null\n");
			return LCD_KIT_FAIL;
		}

		panel_ops = lcd_kit_panel_get_ops();
		if (panel_ops && panel_ops->lcd_get_2d_barcode) {
			ret = panel_ops->lcd_get_2d_barcode(read_value, hisifd);
			if (ret != 0) {
				LCD_KIT_ERR("get sn_code error!\n");
				return LCD_KIT_FAIL;
			}
			memcpy(pinfo->sn_code, read_value + 2, LCD_KIT_SN_CODE_LENGTH);
			pinfo->sn_code_length = LCD_KIT_SN_CODE_LENGTH;
			return LCD_KIT_OK;
		}
		if (disp_info && disp_info->oeminfo.barcode_2d.support) {
			ret = lcd_kit_dsi_cmds_rx(hisifd, read_value,
				OEM_INFO_SIZE_MAX,
				&disp_info->oeminfo.barcode_2d.cmds);
			if (ret != 0) {
				LCD_KIT_ERR("get sn_code error!\n");
				return LCD_KIT_FAIL;
			}
			memcpy(pinfo->sn_code, read_value, LCD_KIT_SN_CODE_LENGTH);
			pinfo->sn_code_length = LCD_KIT_SN_CODE_LENGTH;
			return LCD_KIT_OK;
		}
	}
	return LCD_KIT_FAIL;
}

struct lcd_kit_ops g_lcd_ops = {
	.lcd_kit_support = lcd_kit_support,
	.get_project_id = lcd_kit_get_project_id,
	.get_status_by_type = lcd_kit_get_status_by_type,
#ifdef LCD_FACTORY_MODE
	.get_pt_station_status = lcd_kit_get_pt_station_status,
	.avdd_mipi_ctrl = lcd_kit_avdd_mipi_ctrl,
#endif
	.get_panel_power_status = lcd_kit_get_power_status,
	.panel_event_skip_delay = lcd_kit_panel_event_skip_delay,
	.power_monitor_on = lcd_kit_power_monitor_on,
	.power_monitor_off = lcd_kit_power_monitor_off,
	.set_vss_by_thermal = lcd_kit_set_vss_by_thermal,
	.set_power_by_thermal = lcd_kit_set_power_by_thermal,
	.write_otp_gamma = lcd_kit_write_otp_gamma,
	.proximity_power_off = lcd_kit_proximity_power_off,
	.get_sn_code = lcd_kit_get_sn_code,
};

void lcd_kit_set_mipi_clk(struct hisi_fb_data_type *hisifd, uint32_t clk)
{
	if (!hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return;
	}
	hisifd->panel_info.mipi.dsi_bit_clk = clk;
	hisifd->panel_info.mipi.dsi_bit_clk_upt = clk;
}

static void lcd_kit_fps_dsc_config(struct device_node *np,
	struct hisi_panel_info *pinfo)
{
	if (pinfo->ifbc_type != IFBC_TYPE_NONE ||
		disp_info->fps.support != FPS_CONFIG_EN ||
		disp_info->fps.fps_ifbc_type == IFBC_TYPE_NONE)
		return;
	lcd_kit_compress_config(np, disp_info->fps.fps_ifbc_type, pinfo);
}

static void lcd_kit_effect_para_is_from_dts(struct device_node *np)
{
	lcd_kit_parse_u8(np, "lcd-kit,parse-effect-from-dts",
		&disp_info->effect_para_is_from_dts, 0);
}

static void lcd_kit_effect_get_data_from_dts(struct device_node *np,
	struct hisi_panel_info *pinfo)
{
	struct lcd_kit_array_data array_data;
	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return;
	}
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	if (!pinfo->gamma_support) {
		LCD_KIT_INFO("do not support gamma!\n");
		return;
	}
	LCD_KIT_DEBUG("+!\n");
	/* parse gamma lut table */
	array_data.buf = NULL;
	array_data.cnt = 0;
	lcd_kit_parse_array_data(np, "lcd-kit,gamma-lut-table-R", &array_data);
	pinfo->gamma_lut_table_R = array_data.buf;
	array_data.buf = NULL;
	lcd_kit_parse_array_data(np, "lcd-kit,gamma-lut-table-G", &array_data);
	pinfo->gamma_lut_table_G = array_data.buf;
	array_data.buf = NULL;
	lcd_kit_parse_array_data(np, "lcd-kit,gamma-lut-table-B", &array_data);
	pinfo->gamma_lut_table_B = array_data.buf;
	pinfo->gamma_lut_table_len = array_data.cnt;

	/* parse igm lut table */
	array_data.buf = NULL;
	array_data.cnt = 0;
	lcd_kit_parse_array_data(np, "lcd-kit,igm_lut_table_R", &array_data);
	pinfo->igm_lut_table_R = array_data.buf;
	array_data.buf = NULL;
	lcd_kit_parse_array_data(np, "lcd-kit,igm_lut_table_G", &array_data);
	pinfo->igm_lut_table_G = array_data.buf;
	array_data.buf = NULL;
	lcd_kit_parse_array_data(np, "lcd-kit,igm_lut_table_B", &array_data);
	pinfo->igm_lut_table_B = array_data.buf;
	pinfo->igm_lut_table_len = array_data.cnt;

	/* parse gmp lut table low32bit & high4bit */
	array_data.buf = NULL;
	array_data.cnt = 0;
	lcd_kit_parse_array_data(np, "lcd-kit,gmp-lut-table-low32bit",
		&array_data);
	pinfo->gmp_lut_table_low32bit = array_data.buf;
	array_data.buf = NULL;
	lcd_kit_parse_array_data(np, "lcd-kit,gmp-lut-table-high4bit",
		&array_data);
	pinfo->gmp_lut_table_high4bit = array_data.buf;
	pinfo->gmp_lut_table_len = array_data.cnt;

	/* parse xcc table */
	array_data.buf = NULL;
	array_data.cnt = 0;
	lcd_kit_parse_array_data(np, "lcd-kit,xcc-table", &array_data);
	pinfo->xcc_table = array_data.buf;
	pinfo->xcc_table_len = array_data.cnt;
	LCD_KIT_DEBUG("-!\n");
}

int lcd_kit_utils_init(struct device_node *np, struct hisi_panel_info *pinfo)
{
	/* init sem */
	sema_init(&DISP_LOCK->lcd_kit_sem, 1);
	sema_init(&DISP_LOCK->thp_second_poweroff_sem, 1);
	/* parse display dts */
	lcd_kit_parse_dt(np);
	/* init hisi pinfo */
	lcd_kit_pinfo_init(np, pinfo);
	/* parse vesa parameters */
	lcd_kit_vesa_para_parse(np, pinfo);
	/* init compress config */
	lcd_kit_compress_config(np, pinfo->ifbc_type, pinfo);
	/* fps func init compress config */
	lcd_kit_fps_dsc_config(np, pinfo);
	/* register lcd ops */
	lcd_kit_ops_register(&g_lcd_ops);
	/* effect init */
	lcd_kit_effect_para_is_from_dts(np);
	if (disp_info->effect_para_is_from_dts) {
		LCD_KIT_INFO("get effect para from dts\n");
		lcd_kit_effect_get_data_from_dts(np, pinfo);
	} else {
		LCD_KIT_INFO("get effect para from head file\n");
		lcd_kit_effect_get_data(lcd_kit_get_panel_id(
			disp_info->product_id, disp_info->compatible),
			pinfo);
	}
	/* Read gamma data from shared memory */
	if (disp_info->gamma_cal.support)
		hisifb_update_gm_from_reserved_mem(pinfo->gamma_lut_table_R,
			pinfo->gamma_lut_table_G, pinfo->gamma_lut_table_B,
			pinfo->igm_lut_table_R,	pinfo->igm_lut_table_G,
			pinfo->igm_lut_table_B);

	if (disp_info->oeminfo.support) {
		if (disp_info->oeminfo.brightness_color_uniform.support) {
			lcd_kit_read_calicolordata_from_share_mem(&g_brightness_color_oeminfo);
			lcd_kit_set_actual_bl_max_nit();
		}
	}
	return LCD_KIT_OK;
}

#define LCD_ELVSS_DIM_LENGHTH 10

static int __init early_parse_elvss_dim_cmdline(char *arg)
{
	char elvss_dim_val[LCD_ELVSS_DIM_LENGHTH] = {0};
	unsigned int elvss_dim_val_uint;

	if (!arg) {
		LCD_KIT_ERR("parse elvss dim, arg is NULL\n");
		return LCD_KIT_FAIL;
	}

	strncpy(elvss_dim_val, arg, sizeof(elvss_dim_val) - 1);

	elvss_dim_val_uint = (unsigned int)simple_strtol(elvss_dim_val, NULL, 0);
	LCD_KIT_INFO("elvss_dim_val parse from cmdline: 0x%x\n", elvss_dim_val_uint);
	common_info->hbm.ori_elvss_val = elvss_dim_val_uint & 0xFF;

	return 0;
}

early_param("LCD_ELVSS_DIM", early_parse_elvss_dim_cmdline);

void lcd_frame_refresh(struct hisi_fb_data_type *hisifd)
{
# define BUF_LEN 64
	char *envp[2] = {NULL};
	char buf[BUF_LEN];

	snprintf(buf, sizeof(buf), "Refresh=1");
	envp[0] = buf;
	envp[1] = NULL;
	kobject_uevent_env(&(hisifd->fbi->dev->kobj), KOBJ_CHANGE, envp);
	LCD_KIT_INFO("refresh=1!\n");
}

void lcd_kit_recovery_display(struct hisi_fb_data_type *hisifd)
{
	uint32_t bl_level_cur;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return;
	}
	down(&hisifd->brightness_esd_sem);
	bl_level_cur = hisifd->bl_level;
	/* backlight on */
	hisifb_set_backlight(hisifd, 0, false);
	up(&hisifd->brightness_esd_sem);
	/* lcd panel off */
	if (hisi_fb_blank_sub(FB_BLANK_POWERDOWN, hisifd->fbi))
		LCD_KIT_ERR("lcd panel off error!\n");
	msleep(100);
	/* lcd panel on */
	if (hisi_fb_blank_sub(FB_BLANK_UNBLANK, hisifd->fbi))
		LCD_KIT_ERR("lcd panel on error!\n");
	/* refresh frame */
	lcd_frame_refresh(hisifd);
	/* backlight on */
	down(&hisifd->brightness_esd_sem);
	hisifb_set_backlight(hisifd, bl_level_cur ? bl_level_cur : hisifd->bl_level, false);
	up(&hisifd->brightness_esd_sem);
}

void lcd_esd_enable(struct hisi_fb_data_type *hisifd, int enable)
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is null!\n");
		return;
	}
	if (common_info->esd.support) {
		pinfo->esd_enable = enable;
		msleep(500);
	}
	LCD_KIT_INFO("pinfo->esd_enable = %d\n", pinfo->esd_enable);
}
